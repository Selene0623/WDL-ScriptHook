#!/usr/bin/env python3
"""
dump_game_dll.py — Dump decrypted Watch Dogs: Legion game DLL from memory.

Usage:
  1. Launch the game (via Steam/Proton or Wine)
  2. Wait for the game to fully load (main menu is fine)
  3. Run: python3 dump_game_dll.py [pid]
     If no PID given, auto-detects WatchDogsLegion.exe
  4. Outputs:
     - DuniaDemo_clang_64_dx12_dump.bin  (raw memory dump)
     - DuniaDemo_clang_64_dx12_dump.pe   (reconstructed PE with correct headers)
     - dispatch_table.txt                (hash→address patterns found in dump)

The PE dump can be loaded in Ghidra for full decompilation.
"""

import sys
import os
import re
import struct
import ctypes
import ctypes.util
from pathlib import Path


def find_game_pid():
    """Find Watch Dogs Legion process."""
    import subprocess
    result = subprocess.run(
        ['pgrep', '-f', 'WatchDogsLegion'],
        capture_output=True, text=True
    )
    pids = result.stdout.strip().split('\n')
    pids = [p for p in pids if p]
    if not pids:
        # Try alternative names
        result = subprocess.run(
            ['pgrep', '-f', 'watch_dogs_legion'],
            capture_output=True, text=True
        )
        pids = result.stdout.strip().split('\n')
        pids = [p for p in pids if p]
    if not pids:
        result = subprocess.run(
            ['pgrep', '-f', 'legion'],
            capture_output=True, text=True
        )
        pids = result.stdout.strip().split('\n')
        pids = [p for p in pids if p]
    return int(pids[0]) if pids else None


def parse_maps(pid):
    """Parse /proc/<pid>/maps to find game DLL memory regions."""
    maps_path = f'/proc/{pid}/maps'
    regions = []
    with open(maps_path, 'r') as f:
        for line in f:
            # Format: addr-addr perms offset dev inode pathname
            match = re.match(
                r'([0-9a-f]+)-([0-9a-f]+)\s+(\S+)\s+([0-9a-f]+)\s+\S+\s+\S+\s*(.*)',
                line
            )
            if match:
                start = int(match.group(1), 16)
                end = int(match.group(2), 16)
                perms = match.group(3)
                offset = int(match.group(4), 16)
                pathname = match.group(5).strip()
                regions.append({
                    'start': start,
                    'end': end,
                    'size': end - start,
                    'perms': perms,
                    'offset': offset,
                    'pathname': pathname,
                })
    return regions


def read_memory(pid, address, size):
    """Read memory from /proc/<pid>/mem."""
    mem_path = f'/proc/{pid}/mem'
    with open(mem_path, 'rb') as f:
        f.seek(address)
        return f.read(size)


def read_cstring(pid, address, max_len=256):
    """Read a null-terminated string from process memory."""
    data = b''
    chunk_size = 64
    for i in range(0, max_len, chunk_size):
        try:
            chunk = read_memory(pid, address + i, chunk_size)
        except (OSError, IOError):
            break
        null_pos = chunk.find(b'\x00')
        if null_pos >= 0:
            data += chunk[:null_pos]
            break
        data += chunk
    return data.decode('ascii', errors='replace')


def dump_dll_from_memory(pid, output_dir='.'):
    """Main dump routine."""
    print(f"[*] Targeting PID: {pid}")
    
    # Parse memory maps
    print("[*] Parsing /proc/{}/maps...".format(pid))
    regions = parse_maps(pid)
    print(f"[*] Found {len(regions)} memory regions")
    
    # Find game DLL regions
    # Look for DuniaDemo_clang_64_dx12.dll in the maps
    dx12_regions = []
    dx11_regions = []
    scripthook_regions = []
    
    for r in regions:
        pn = r['pathname'].lower()
        if 'duniaDemo_clang_64_dx12' in pn or 'dunia_clang_64_dx12' in pn:
            dx12_regions.append(r)
        elif 'duniaDemo_clang_64_dx11' in pn or 'dunia_clang_64_dx11' in pn:
            dx11_regions.append(r)
        elif 'scripthook' in pn:
            scripthook_regions.append(r)
    
    print(f"[*] DX12 DLL regions: {len(dx12_regions)}")
    print(f"[*] DX11 DLL regions: {len(dx11_regions)}")
    print(f"[*] ScriptHook regions: {len(scripthook_regions)}")
    
    # Try to find the game DLL by looking for its PE signature in memory
    # If no named regions found, search for PE headers
    target_regions = dx12_regions
    target_name = "DX12"
    
    if not target_regions:
        print("[*] No named DX12 regions found. Searching for PE in anonymous mappings...")
        # Search anonymous executable regions for PE headers
        for r in regions:
            if r['perms'] == 'r-xp' and not r['pathname']:
                try:
                    header = read_memory(pid, r['start'], 2)
                    if header == b'MZ':
                        print(f"[!] Found PE at 0x{r['start']:x} (anon region, size={r['size']:#x})")
                        target_regions.append(r)
                        target_name = "DX12 (detected)"
                except:
                    pass
    
    if not target_regions and dx11_regions:
        target_regions = dx11_regions
        target_name = "DX11"
    
    if not target_regions:
        print("[!] Could not find game DLL in memory. Is the game running?")
        print("[*] Listing all executable regions:")
        for r in regions:
            if 'x' in r['perms'] and r['size'] > 0x10000:
                tag = ""
                if r['pathname']:
                    tag = f" [{r['pathname']}]"
                print(f"  0x{r['start']:012x} - 0x{r['end']:012x}  size=0x{r['size']:x}  {r['perms']}{tag}")
        return None
    
    # Dump the regions
    print(f"\n[*] Dumping {target_name} DLL ({len(target_regions)} regions)...")
    
    # Sort by address
    target_regions.sort(key=lambda r: r['start'])
    
    # Find the base (lowest address) which should have the PE header
    base_addr = target_regions[0]['start']
    total_end = max(r['end'] for r in target_regions)
    total_size = total_end - base_addr
    
    print(f"[*] Base: 0x{base_addr:012x}")
    print(f"[*] Total range: 0x{total_size:x} bytes ({total_size/1024/1024:.1f} MB)")
    
    # Read the PE header to understand the layout
    try:
        pe_header = read_memory(pid, base_addr, 0x1000)
    except Exception as e:
        print(f"[!] Cannot read PE header: {e}")
        return None
    
    if pe_header[:2] != b'MZ':
        print(f"[!] Not a valid PE at 0x{base_addr:x}: {pe_header[:4].hex()}")
        # Try to find MZ header nearby
        for offset in range(0, 0x10000, 0x1000):
            try:
                test = read_memory(pid, base_addr + offset, 2)
                if test == b'MZ':
                    base_addr += offset
                    pe_header = read_memory(pid, base_addr, 0x1000)
                    print(f"[!] Found PE header at offset +{offset:x}: 0x{base_addr:012x}")
                    break
            except:
                break
        else:
            print("[!] Cannot find PE header")
            return None
    
    # Parse PE optional header
    pe_offset = struct.unpack_from('<I', pe_header, 0x3C)[0]
    num_sections = struct.unpack_from('<H', pe_header, pe_offset + 6)[0]
    opt_header_offset = pe_offset + 24
    magic = struct.unpack_from('<H', pe_header, opt_header_offset)[0]
    
    if magic == 0x20b:  # PE32+
        image_base = struct.unpack_from('<Q', pe_header, opt_header_offset + 24)[0]
        opt_hdr_size = struct.unpack_from('<I', pe_header, pe_offset + 20)[0]
    else:
        print(f"[!] Unexpected PE magic: 0x{magic:x}")
        return None
    
    section_table_offset = pe_offset + 24 + opt_hdr_size
    
    print(f"[*] PE image base: 0x{image_base:x}")
    print(f"[*] Sections: {num_sections}")
    
    # Parse sections
    sections = []
    for i in range(num_sections):
        off = section_table_offset + i * 40
        name = pe_header[off:off+8].rstrip(b'\x00').decode('ascii', errors='replace')
        vsize = struct.unpack_from('<I', pe_header, off + 8)[0]
        va = struct.unpack_from('<I', pe_header, off + 12)[0]
        raw_size = struct.unpack_from('<I', pe_header, off + 16)[0]
        raw_ptr = struct.unpack_from('<I', pe_header, off + 20)[0]
        chars = struct.unpack_from('<I', pe_header, off + 36)[0]
        sections.append({
            'name': name,
            'va': va,
            'vsize': vsize,
            'raw_ptr': raw_ptr,
            'raw_size': raw_size,
            'chars': chars,
        })
        print(f"  {name:8s}  VA=0x{va:08x}  VSize=0x{vsize:08x}  Raw=0x{raw_ptr:08x}  Chars=0x{chars:08x}")
    
    # Dump each section from memory
    print(f"\n[*] Reading sections from decrypted memory...")
    
    # Build a PE image from memory
    # The image is at base_addr in process memory
    # We need to read image_base + va for each section
    
    output_pe_path = os.path.join(output_dir, f'DuniaDemo_clang_64_{target_name.lower()}_dump.pe')
    output_bin_path = os.path.join(output_dir, f'DuniaDemo_clang_64_{target_name.lower()}_dump.bin')
    
    # Calculate total image size
    max_va = max(s['va'] + max(s['vsize'], s['raw_size']) for s in sections)
    image_data = bytearray(max_va + 0x1000)
    
    # Copy PE header
    header_data = read_memory(pid, base_addr, min(0x1000, total_size))
    image_data[:len(header_data)] = header_data
    
    # Read each section
    for s in sections:
        if s['raw_size'] == 0 and s['vsize'] == 0:
            continue
        
        mem_va = image_base + s['va']
        # Convert to process address
        # In Proton, image_base might differ. Use the actual base_addr
        proc_addr = base_addr + s['va'] - (image_base - base_addr if image_base != base_addr else 0)
        # Simpler: the sections are mapped at base_addr + va
        proc_addr = base_addr + s['va']
        
        read_size = max(s['raw_size'], s['vsize'])
        if read_size == 0:
            continue
            
        print(f"  Reading {s['name']:8s} at 0x{proc_addr:012x} ({read_size:#x} bytes)...")
        
        try:
            section_data = read_memory(pid, proc_addr, read_size)
            image_data[s['va']:s['va'] + len(section_data)] = section_data
        except Exception as e:
            print(f"    WARNING: {e}")
            # Try reading in chunks
            for offset in range(0, read_size, 0x10000):
                chunk_size = min(0x10000, read_size - offset)
                try:
                    chunk = read_memory(pid, proc_addr + offset, chunk_size)
                    image_data[s['va'] + offset:s['va'] + offset + len(chunk)] = chunk
                except:
                    pass
    
    # Write PE dump
    print(f"\n[*] Writing PE dump to {output_pe_path}...")
    with open(output_pe_path, 'wb') as f:
        f.write(image_data)
    
    # Also write raw bin (just the code sections)
    print(f"[*] Writing raw dump to {output_bin_path}...")
    with open(output_bin_path, 'wb') as f:
        f.write(bytes(image_data))
    
    # Search for dispatch patterns in the dump
    print(f"\n[*] Searching for hash dispatch patterns in dump...")
    dispatch_path = os.path.join(output_dir, 'dispatch_table.txt')
    search_dispatch_patterns(image_data, base_addr, image_base, sections, dispatch_path)
    
    print(f"\n[+] Done! Files written to {output_dir}/")
    print(f"    Load the .pe file in Ghidra (Image Base: 0x{image_base:x})")
    
    return output_pe_path


def search_dispatch_patterns(data, proc_base, image_base, sections, output_path):
    """Search the dumped memory for hash dispatch patterns."""
    # Load known hashes from offsets.txt
    known_hashes = set()
    offsets_path = os.path.join(os.path.dirname(__file__), '..', 'offsets.txt')
    if os.path.exists(offsets_path):
        with open(offsets_path) as f:
            for line in f:
                line = line.strip()
                if line.startswith('case 0x'):
                    parts = line.split()
                    h = int(parts[1].rstrip(':'), 16)
                    known_hashes.add(h)
    
    print(f"[*] Known hashes from offsets.txt: {len(known_hashes)}")
    
    # Search for cmp edx, <hash> patterns
    # These are in the game's dispatch function
    found_hashes = {}
    
    for i in range(len(data) - 7):
        # Pattern 1: 81 FA <hash> (cmp edx, imm32)
        if data[i] == 0x81 and data[i+1] == 0xFA:
            hash_val = struct.unpack_from('<I', data, i + 2)[0]
            if hash_val > 0x1000:  # Filter small values
                if hash_val not in found_hashes:
                    found_hashes[hash_val] = []
                found_hashes[hash_val].append(i)
        
        # Pattern 2: 3D <hash> (cmp eax, imm32)
        if data[i] == 0x3D:
            hash_val = struct.unpack_from('<I', data, i + 1)[0]
            if hash_val > 0x1000:
                if hash_val not in found_hashes:
                    found_hashes[hash_val] = []
                found_hashes[hash_val].append(i)
    
    # Write results
    with open(output_path, 'w') as f:
        f.write("=== Hash Dispatch Table (from memory dump) ===\n\n")
        
        new_hashes = set(found_hashes.keys()) - known_hashes
        known_found = set(found_hashes.keys()) & known_hashes
        
        f.write(f"Known hashes found: {len(known_found)}/{len(known_hashes)}\n")
        f.write(f"NEW hashes found: {len(new_hashes)}\n\n")
        
        if new_hashes:
            f.write("=== NEW HASHES (not in offsets.txt) ===\n")
            for h in sorted(new_hashes):
                offsets = found_hashes[h]
                for off in offsets:
                    # Convert file offset to RVA
                    rva = None
                    for s in sections:
                        if s['va'] <= off < s['va'] + max(s['raw_size'], s['vsize']):
                            rva = off
                            break
                    f.write(f"  0x{h:08x}  (dump offset 0x{off:x}" + 
                            (f", RVA 0x{rva:x}" if rva else "") + ")\n")
        
        f.write("\n=== ALL FOUND HASHES ===\n")
        for h in sorted(found_hashes.keys()):
            tag = " [KNOWN]" if h in known_hashes else " [NEW]"
            f.write(f"0x{h:08x}{tag}  (at {len(found_hashes[h])} locations)\n")
    
    print(f"[*] Found {len(found_hashes)} unique hash values")
    print(f"[*] Known: {len(known_found)}, New: {len(new_hashes)}")
    print(f"[*] Dispatch table written to {output_path}")


if __name__ == '__main__':
    pid = None
    if len(sys.argv) > 1:
        try:
            pid = int(sys.argv[1])
        except ValueError:
            print(f"Invalid PID: {sys.argv[1]}")
            sys.exit(1)
    else:
        pid = find_game_pid()
        if not pid:
            print("Could not find Watch Dogs Legion process.")
            print("Usage: python3 dump_game_dll.py [pid]")
            print("Make sure the game is running.")
            sys.exit(1)
    
    output_dir = os.path.dirname(os.path.abspath(__file__))
    dump_dll_from_memory(pid, output_dir)
