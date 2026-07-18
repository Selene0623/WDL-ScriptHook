// ExtractDispatchTable.java — Ghidra headless post-script
// Finds the hash dispatch function in a dumped game DLL and extracts all hash→RVA mappings.
//
// Usage:
//   analyzeHeadless /path/to/proj ProjectName \
//     -import DuniaDemo_clang_64_dx12_dump.pe \
//     -analysisTimeoutPerFile 600 \
//     -scriptPath /path/to/scripts \
//     -postScript ExtractDispatchTable.java
//
// Reads known hashes from ../offsets.txt (relative to script location)
// Outputs: dispatch_results.txt in the project directory

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import java.io.*;
import java.util.*;

public class ExtractDispatchTable extends GhidraScript {

    // Known hashes from offsets.txt
    private Set<Long> knownHashes = new HashSet<>();
    private Map<Long, Long> knownHashToRVA = new HashMap<>();

    @Override
    public void run() throws Exception {
        String homeDir = System.getProperty("user.home");
        String scriptDir = homeDir + "/Documents/Code/game-tools/watch dogs legion scripthook";
        String outputPath = homeDir + "/.config/ghidra/dispatch_results.txt";

        PrintWriter out = new PrintWriter(new FileWriter(outputPath));

        // Load known hashes
        loadKnownHashes(scriptDir + "/offsets.txt");
        out.println("Loaded " + knownHashes.size() + " known hashes from offsets.txt");

        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        // Strategy 1: Find functions containing "cmp edx, <known_hash>" patterns
        out.println("\n=== Strategy 1: Find dispatch function by known hash comparisons ===");
        findDispatchByCmpPattern(out, decompiler);

        // Strategy 2: Find all cross-references from known hash constants
        out.println("\n=== Strategy 2: Find dispatch by hash constant references ===");
        findDispatchByXRef(out);

        // Strategy 3: Search for large switch/case blocks
        out.println("\n=== Strategy 3: Find switch/case dispatch tables ===");
        findSwitchTables(out);

        decompiler.dispose();
        out.close();
        println("Results written to: " + outputPath);
    }

    private void loadKnownHashes(String path) {
        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("case 0x")) {
                    String[] parts = line.split("\\s+");
                    if (parts.length >= 2) {
                        String hex = parts[1].replace(":", "");
                        try {
                            long hash = Long.parseLong(hex, 16);
                            knownHashes.add(hash);
                            // Try to extract RVA
                            if (line.contains("RVAtoVA(MDLE, 0x")) {
                                String rvaStr = line.split("RVAtoVA\\(MDLE, 0x")[1].split("\\)")[0];
                                long rva = Long.parseLong(rvaStr, 16);
                                knownHashToRVA.put(hash, rva);
                            }
                        } catch (NumberFormatException e) {
                            // skip
                        }
                    }
                }
            }
        } catch (Exception e) {
            println("Warning: Could not read offsets.txt: " + e.getMessage());
        }
    }

    private void findDispatchByCmpPattern(PrintWriter out, DecompInterface decompiler) {
        // Look for CMP EDX, imm32 instructions with known hash values
        InstructionIterator iter = currentProgram.getListing().getInstructions(true);
        Map<Long, Long> hashToAddr = new HashMap<>();
        Long dispatchCandidate = null;
        int matchCount = 0;

        while (iter.hasNext() && matchCount < 50) {
            Instruction instr = iter.next();
            // CMP EDX, imm32 = 81 FA xx xx xx xx
            if (instr.getMnemonicString().equals("CMP") && instr.getNumOperands() >= 2) {
                try {
                    Object[] operands = instr.getOpObjects(1);
                    if (operands.length > 0 && operands[0] instanceof ghidra.program.model.scalar.Scalar) {
                        long val = ((ghidra.program.model.scalar.Scalar) operands[0]).getValue();
                        if (knownHashes.contains(val)) {
                            hashToAddr.put(val, instr.getAddress().getOffset());
                            matchCount++;
                            if (dispatchCandidate == null) {
                                // This CMP is likely inside the dispatch function
                                // Check the enclosing function
                                Function func = getFunctionContaining(instr.getAddress());
                                if (func != null) {
                                    dispatchCandidate = func.getEntryPoint().getOffset();
                                    out.println("First hash CMP found at: " + instr.getAddress());
                                    out.println("Enclosing function: " + func.getName() + " at " + func.getEntryPoint());
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    // skip
                }
            }
        }

        out.println("Found " + matchCount + " CMP EDX instructions with known hashes");
        if (dispatchCandidate != null) {
            out.println("Dispatch function candidate at: 0x" + Long.toHexString(dispatchCandidate));

            // Decompile the dispatch function
            Function dispatchFunc = getFunctionAt(toAddr(dispatchCandidate));
            if (dispatchFunc != null) {
                out.println("\n=== Decompiling dispatch function ===");
                out.println("Function: " + dispatchFunc.getName());
                out.println("Entry: " + dispatchFunc.getEntryPoint());
                out.println("Size: " + dispatchFunc.getBody().getNumAddresses() + " bytes");

                DecompileResults result = decompiler.decompileFunction(dispatchFunc, 120, monitor);
                if (result.decompileCompleted()) {
                    String cCode = result.getDecompiledFunction().getC();
                    out.println("\n--- C pseudocode (first 5000 chars) ---");
                    out.println(cCode.substring(0, Math.min(cCode.length(), 5000)));
                    if (cCode.length() > 5000) {
                        out.println("\n... [" + (cCode.length() - 5000) + " more chars] ...");
                    }
                } else {
                    out.println("Decompilation failed: " + result.getErrorMessage());
                }
            }
        }

        // Write hash→address map
        out.println("\n=== Hash CMP addresses ===");
        for (Map.Entry<Long, Long> entry : hashToAddr.entrySet()) {
            out.printf("0x%08X -> CMP at 0x%X%n", entry.getKey(), entry.getValue());
        }
    }

    private void findDispatchByXRef(PrintWriter out, DecompInterface decompiler) {
        // Search for known hash values as data in the binary
        int found = 0;
        for (Long hash : knownHashes) {
            byte[] hashBytes = new byte[4];
            hashBytes[0] = (byte)(hash & 0xFF);
            hashBytes[1] = (byte)((hash >> 8) & 0xFF);
            hashBytes[2] = (byte)((hash >> 16) & 0xFF);
            hashBytes[3] = (byte)((hash >> 24) & 0xFF);

            try {
                MemoryBlock[] blocks = currentProgram.getMemory().getBlocks();
                for (MemoryBlock block : blocks) {
                    if (!block.isInitialized() || !block.isRead()) continue;

                    long searchStart = block.getStart().getOffset();
                    long searchEnd = block.getEnd().getOffset();
                    Address addr = block.getStart();

                    while (addr.getOffset() < searchEnd - 4) {
                        byte[] buf = new byte[Math.min(0x10000, (int)(searchEnd - addr.getOffset()))];
                        try {
                            int bytesRead = block.getBytes(addr, buf);
                            for (int i = 0; i < bytesRead - 4; i++) {
                                if (buf[i] == hashBytes[0] && buf[i+1] == hashBytes[1] &&
                                    buf[i+2] == hashBytes[2] && buf[i+3] == hashBytes[3]) {
                                    found++;
                                    if (found <= 5) {
                                        out.printf("Hash 0x%08X found at 0x%X in block %s%n",
                                            hash, addr.getOffset() + i, block.getName());
                                    }
                                }
                            }
                        } catch (Exception e) {
                            break;
                        }
                        addr = addr.add(buf.length);
                    }
                }
            } catch (Exception e) {
                // skip
            }
        }
        out.println("Found " + found + " total hash value references in binary");
    }

    private void findSwitchTables(PrintWriter out) {
        // Look for jump tables (switch/case patterns)
        // In x64, these often use: jmp [rip + offset] after a bounds check
        int count = 0;
        InstructionIterator iter = currentProgram.getListing().getInstructions(true);
        while (iter.hasNext() && count < 100) {
            Instruction instr = iter.next();
            if (instr.getMnemonicString().equals("JMP") && instr.getNumOperands() > 0) {
                try {
                    // Check if it's an indirect jump (jmp [rip+disp32])
                    String repr = instr.toString();
                    if (repr.contains("[") && repr.contains("rip")) {
                        // This could be a switch table jump
                        Function func = getFunctionContaining(instr.getAddress());
                        if (func != null && func.getBody().getNumAddresses() > 0x1000) {
                            out.printf("Switch table JMP at 0x%X in function %s (size=0x%x)%n",
                                instr.getAddress().getOffset(), func.getName(),
                                func.getBody().getNumAddresses());
                            count++;
                        }
                    }
                } catch (Exception e) {
                    // skip
                }
            }
        }
        out.println("Found " + count + " potential switch table jumps in large functions");
    }
}
