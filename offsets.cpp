#include <windows.h>

void* MDLE = GetModuleHandleA("DuniaDemo_clang_64_dx12.dll");
  switch (a2) {
  case 0x1065a744:
  {
      //Old Offset:0000000006447F50 New Offset: 00000000064494F0 Signature: 41 57 41 56 56 57 53 48 81 EC ? ? ? ? 49 89 CF 48 8B 05 ? ? ? ? 48 31 E0 48 89 84 24 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 79 ? 48 85 FF 74 ? ? ? ? 31 C0 0F 1F 84 00 ? ? ? ? 31 F6 39 5F ? 40 0F 92 C6 48 0F 43 C7 ? ? ? ? 48 85 FF 75 ? 48 85 C0 74 ? 48 83 C1 ? 48 39 C8 74 ? 3B 58 ? 72 ? 4C 8B 70
      std::cout << colors::blue << " Hash Redirected." << "\n";
      return RVAtoVA(MDLE, 0x6447f50);
      break;
  }
  case 0x1115f518:
  {
      return RVAtoVA(MDLE, 0x0000000001FB8670);
      break;
  }
  case 0x113676f0:
  {
      std::cout << colors::bright_red << "Unsupported Module, hash 0x113676f0\n";
      return RVAtoVA(MDLE, 0x1);
      break;
  }
  case 0x11b9075e: { return RVAtoVA(MDLE, 0x000000000690D8F0);  break; }
  case 0x11fca3f6: { return RVAtoVA(MDLE, 0x00000000033D8510);  break; }
  case 0x134f743e: { return RVAtoVA(MDLE, 0x000000000704ACA0);  break; }
  case 0x15ac0c24: { return RVAtoVA(MDLE, 0x00000000067A63E0);  break; }
  case 0x175c3db2: { return RVAtoVA(MDLE, 0x00000000068C6F30);  break; }
  case 0x1e673c94: { return RVAtoVA(MDLE, 0x0000000006ADE0C0);  break; }
  case 0x1f761ea7: { return RVAtoVA(MDLE, 0x690F0C0);  break; }
  case 0x20e0f494: { return RVAtoVA(MDLE, 0x21C31E0);  break; }
  case 0x2408087e: { return RVAtoVA(MDLE, 0x00000000068DFEDB);  break; } // Memory Patching
  case 0x251ba075: { return RVAtoVA(MDLE, 0x5E8BC0);  break; }
  case 0x28777aa3: { return RVAtoVA(MDLE, 0x0000000006EC3330);  break; }
  case 0x29e1e8c4: { return RVAtoVA(MDLE, 0x0000000007DEEBA0);  break; }
  case 0x2a5a33bb: { return RVAtoVA(MDLE, 0x690E930);  break; }
  case 0x2b3805a0: { return RVAtoVA(MDLE, 0x6035200);  break; }
  case 0x2c392b08: { return RVAtoVA(MDLE, 0x00000000067A6080);  break; }
  case 0x2d3d7503: { return RVAtoVA(MDLE, 0x000000000690D680);  break; }
  case 0x2e61ac0e: { return RVAtoVA(MDLE, 0x000000000686EC60);  break; }
  case 0x2f05b478: { return RVAtoVA(MDLE, 0x0000000001202D50);  break; }
  case 0x31463731: { return RVAtoVA(MDLE, 0x00000000005C26F0);  break; }
  case 0x31fe5cbe: { return RVAtoVA(MDLE, 0x000000000777C130);  break; }
  case 0x34399668: { return RVAtoVA(MDLE, 0x000000000690C850);  break; }
  case 0x35c1a19b: { return RVAtoVA(MDLE, 0x690F9E0);  break; }
  case 0x366ea9f9: { return RVAtoVA(MDLE, 0x6FE2AA0);  break; }
  case 0x3a129de7: { return RVAtoVA(MDLE, 0xB3349E0);  break; } // Structure Access
  case 0x3b9757e4: { return RVAtoVA(MDLE, 0x690F800);  break; }
  case 0x3bfc3e8a: { return RVAtoVA(MDLE, 0x6447460);  break; }
  case 0x3c61b784: { return RVAtoVA(MDLE, 0xB3FDDB8);  break; }// Structure Access
  case 0x3d5d2c06: { return RVAtoVA(MDLE, 0x000000000690E460);  break; }
  case 0x3d8792df: { return RVAtoVA(MDLE, 0x00000000064497C0);  break; }
  case 0x3f170059: { return RVAtoVA(MDLE, 0x6100650);  break; } // LEA Access
  case 0x3f29f1d7: { return RVAtoVA(MDLE, 0x0000000003033C40);  break; }
  case 0x408c46b8: { return RVAtoVA(MDLE, 0x690c1c0);  break; }
  case 0x40b6c4a6: { return RVAtoVA(MDLE, 0x0000000006203DC0);  break; }
  case 0x457f1d77: { return RVAtoVA(MDLE, 0x000000000690E330);  break; }
  case 0x464ff541: { return RVAtoVA(MDLE, 0x00000000005B56E0);  break; }
  case 0x465f993: { return RVAtoVA(MDLE, 0x00000000068C7040);  break; }
  case 0x483d6616: { return RVAtoVA(MDLE, 0x686DDD0);  break; }
  case 0x48a6ba85: { return RVAtoVA(MDLE, 0x00000000005BCF20);  break; }
  case 0x499aabdc: { return RVAtoVA(MDLE, 0xB510200);  break; } // Structure Access
  case 0x4d5a8d08: { return RVAtoVA(MDLE, 0x000000000063DC10);  break; }
  case 0x4d661f2e: { return RVAtoVA(MDLE, 0x0000000001632030);  break; }
  case 0x4ea02aeb: { return RVAtoVA(MDLE, 0xB46E3C8);  break; } // Structure Access
  case 0x4eefd09a: { return RVAtoVA(MDLE, 0x000000000777BD00);  break; } // Structure Access
  case 0x4f5160f7: { return RVAtoVA(MDLE, 0x000000000690C4A0);  break; }
  case 0x4fa85f44: { return RVAtoVA(MDLE, 0x0000000006118380);  break; }
  case 0x502d1045: { return RVAtoVA(MDLE, 0x0000000006100940);  break; }
  case 0x51273e19: { return RVAtoVA(MDLE, 0x0000000006ADDC20);  break; }
  case 0x5138717b: { return RVAtoVA(MDLE, 0x00000000061028F0);  break; }
  case 0x51ab7f1c: { return RVAtoVA(MDLE, 0x0000000006A39E00);  break; }
  case 0x524bb79e: { return RVAtoVA(MDLE, 0x0000000006100490);  break; }
  case 0x56a81ebf: { return RVAtoVA(MDLE, 0x733AB20);  break; }
  case 0x56ae780d: { return RVAtoVA(MDLE, 0x0000000006581FC0);  break; }
  case 0x570c1011: { return RVAtoVA(MDLE, 0x00000000067A5EE0);  break; }
  case 0x5844d357: { return RVAtoVA(MDLE, 0x00000000060FABC0);  break; }
  case 0x58a7903b: { return RVAtoVA(MDLE, 0xB513450);  break; } // Structure Access
  case 0x5a03e044: { return RVAtoVA(MDLE, 0xB50ABA0);  break; }
  case 0x5b57e4e5: { return RVAtoVA(MDLE, 0x000000000690F1F0);  break; }
  case 0x5ccd3cc8: { return RVAtoVA(MDLE, 0xB445330);  break; } // Structure Access
  case 0x5d459f9b: { return RVAtoVA(MDLE, 0x00000000060FDBD0);  break; }
  case 0x5d7fcb77: { return RVAtoVA(MDLE, 0x0000000006906210);  break; }
  case 0x5f174c59: { return RVAtoVA(MDLE, 0x000000000690C260);  break; }
  case 0x5f198ae3: { return RVAtoVA(MDLE, 0x000000000679D330);  break; }
  case 0x665c85e3: { return RVAtoVA(MDLE, 0xB51E6D8);  break; } // Structure Access
  case 0x67c7c554: { return RVAtoVA(MDLE, 0x0000000001619010);  break; }
  case 0x6cf7cfe1: { return RVAtoVA(MDLE, 0x00000000005BD05B);  break; } // WARNING : DEFINETLY BYTE PATCH !!!
  case 0x6f086357: { return RVAtoVA(MDLE, 0x0000000006FE2590);  break; }
  case 0x6f80327f: { return RVAtoVA(MDLE, 0x000000000690C770);  break; }
  case 0x6fa6955f: { return RVAtoVA(MDLE, 0x000000000060BEC0);  break; }
  case 0x71366779: { return RVAtoVA(MDLE, 0x00000000061009D0);  break; }
  case 0x72401b5e: { return RVAtoVA(MDLE, 0x0000000006907040);  break; }
  case 0x742be29a: { return RVAtoVA(MDLE, 0xB5179C8);  break; } // LUA WATCHER
  case 0x746fc66: { return RVAtoVA(MDLE, 0xB514AB8);  break; } // GAME FILE MANAGER
  case 0x74ec23cb: { return RVAtoVA(MDLE, 0x000000000690FBB0);  break; }
  case 0x750865a3: { return RVAtoVA(MDLE, 0x00000000005C3B80);  break; } // Structure
  case 0x76c7fd82: { return RVAtoVA(MDLE, 0x000000000686DFB0);  break; }
  case 0x77266049: { return RVAtoVA(MDLE, 0x000000000060F430);  break; }
  case 0x77671c74: { return RVAtoVA(MDLE, 0x000000000669AD20);  break; }
  case 0x78cb8af4: { return RVAtoVA(MDLE, 0x0000000006102840);  break; }
  case 0x7995aace: { return RVAtoVA(MDLE, 0x60F520);  break; }
  case 0x7b9837fd: { return RVAtoVA(MDLE, 0x000000000616EC80);  break; }
  case 0x7bb4e37e: { return RVAtoVA(MDLE, 0x5E8B30);  break; }
  case 0x7d7cd16f: { return RVAtoVA(MDLE, 0x0000000006781EB0);  break; }
  case 0x7dbcf0c8: { return RVAtoVA(MDLE, 0xB501268);  break; } // Config Structure
  case 0x80f9c6c2: { return RVAtoVA(MDLE, 0xB3AADC8);  break; }
  case 0x82d702b4: { return RVAtoVA(MDLE, 0x6906FC0);  break; }
  case 0x83c26943: { return RVAtoVA(MDLE, 0x00000000005C1C40);  break; }
  case 0x83dfb78c: { return RVAtoVA(MDLE, 0x0000000001213B20);  break; }
  case 0x845cf8c1: { return RVAtoVA(MDLE, 0x000000000690BBC0);  break; }
  case 0x85df22f4: { return RVAtoVA(MDLE, 0x5A5B80);  break; }
  case 0x8635b064: { return RVAtoVA(MDLE, 0x68CD780);  break; } // ? Lua Manager ?
  case 0x8777223a: { return RVAtoVA(MDLE, 0x000000000644A160);  break; }
  case 0x8b058e8: { return RVAtoVA(MDLE, 0xB516D78);  break; }
  case 0x8ca23226: { return RVAtoVA(MDLE, 0xB3F3E38);  break; }
  case 0x8f5f0384: { return RVAtoVA(MDLE, 0x4F0C90);  break; }
  case 0x8fe66ed0: { return RVAtoVA(MDLE, 0xA56D090);  break; }
  case 0x90258c97: { return RVAtoVA(MDLE, 0x000000000679E3F0);  break; }
  case 0x90d7a259: { return RVAtoVA(MDLE, 0x000000000679CF40);  break; }
  case 0x930630b0: { return RVAtoVA(MDLE, 0x000000000690F540);  break; }
  case 0x931261f: { return RVAtoVA(MDLE, 0x00000000006D9290);  break; }
  case 0x94acb74d: { return RVAtoVA(MDLE, 0x0000000007DEECE0);  break; }
  case 0x9661be0: { return RVAtoVA(MDLE, 0x00000000061055A0);  break; }
  case 0x970b4892: { return RVAtoVA(MDLE, 0x0000000006DF0860);  break; }
  case 0x9759ccd4: { return RVAtoVA(MDLE, 0x000000000680C2E0);  break; }
  case 0x988299fa: { return RVAtoVA(MDLE, 0x00000000067917D0);  break; }
  case 0x98b130a4: { return RVAtoVA(MDLE, 0x690EE50);  break; }
  case 0x99ed6969: { return RVAtoVA(MDLE, 0x0000000006876370);  break; } // WARNING. REGISTER STATES CHANGED !!
  case 0x9a9ca111: { return RVAtoVA(MDLE, 0xB517BE0);  break; }
  case 0x9f7a2ee0: { return RVAtoVA(MDLE, 0x0000000006697690);  break; }
  case 0xa0efa36a: { return RVAtoVA(MDLE, 0xB514200);  break; }
  case 0xa2d38047: { return RVAtoVA(MDLE, 0x000000000061D603);  break; }
  case 0xa35901b2: { return RVAtoVA(MDLE, 0x000000000690F9D0);  break; }
  case 0xa3e469cc: { return RVAtoVA(MDLE, 0x0000000006106BC0);  break; }
  case 0xa6b35e: { return RVAtoVA(MDLE, 0x0000000006917250);  break; }
  case 0xa90edd87: { return RVAtoVA(MDLE, 0xB517010);  break; }
  case 0xa93eead8: {
      std::cout << "Warning. Attempt to patch -battleye section when it doesn't exist anymore. Redirected to null void." << "\n";
      return &BlankMemory;
      break; }
  case 0xa9a8ceda: { return RVAtoVA(MDLE, 0x6CA3050);  break; }
  case 0xaa6ac258: { return RVAtoVA(MDLE, 0x000000000671AD90);  break; }
  case 0xaca83ca0: { return RVAtoVA(MDLE, 0x000000000690F590);  break; }
  case 0xacfa4add: { return RVAtoVA(MDLE, 0x00000000063D0F00);  break; }
  case 0xadd0030: { return RVAtoVA(MDLE, 0x0000000000004980);  break; }
  case 0xadd8d4d0: { return RVAtoVA(MDLE, 0x0000000001EA513C);  break; } // Call redirect -> fix needed
  case 0xaff07fb6: { return RVAtoVA(MDLE, 0xB51DCE0);  break; }
  case 0xb1a0a123: { return RVAtoVA(MDLE, 0x0000000003E88460);  break; }
  case 0xb1a6b68b: { return RVAtoVA(MDLE, 0x000000000061D9F0);  break; }
  case 0xb398f87b: { return RVAtoVA(MDLE, 0x00000000005C2A00);  break; }
  case 0xb729f708: { return RVAtoVA(MDLE, 0x00000000000024C6);  break; } // Call Redirect -> fix needed
  case 0xb7a48562: { return RVAtoVA(MDLE, 0x0000000006A9C150);  break; }
  case 0xb9f3f7f9: { return RVAtoVA(MDLE, 0x0000000007DEECA0);  break; }
  case 0xbbc634e8: { return RVAtoVA(MDLE, 0x00000000068CBFD0);  break; }
  case 0xbd4d5d0c: { return RVAtoVA(MDLE, 0x00000000072F4290);  break; }
  case 0xc1a18e72: { return RVAtoVA(MDLE, 0x00000000019913AC);  break; } // Call redirect
  case 0xc619381a: { return RVAtoVA(MDLE, 0x0000000001D6D960);  break; }
  case 0xc6ac8d57: { return RVAtoVA(MDLE, 0x00000000067A5940);  break; }
  case 0xc8a5c50: { return RVAtoVA(MDLE, 0x0000000006903390);  break; }
  case 0xc901672d: { return RVAtoVA(MDLE, 0x00000000069066E0);  break; }
  case 0xcba9a965: { return RVAtoVA(MDLE, 0x00000000033EB34E);  break; }// Call Redirect
  case 0xce97973a: { return RVAtoVA(MDLE, 0xB513F90);  break; }
  case 0xd0f559d1: { return RVAtoVA(MDLE, 0x00000000027FBCA0);  break; }
  case 0xd151a99: { return RVAtoVA(MDLE, 0x00000000066FC4D0);  break; }
  case 0xd1a3bd27: { return RVAtoVA(MDLE, 0x00000000062F9D10);  break; }
  case 0xd3541adc: { return RVAtoVA(MDLE, 0xB631A20);  break; }
  case 0xd573f60d: { return RVAtoVA(MDLE, 0xB686440);  break; }
  case 0xd666d2da: { return RVAtoVA(MDLE, 0x0000000006101630);  break; }
  case 0xd6e66a17: { return RVAtoVA(MDLE, 0x0000000006100550);  break; }
  case 0xd867e25e: { return RVAtoVA(MDLE, 0x6D934D0);  break; }
  case 0xd93fc3d4: { return RVAtoVA(MDLE, 0xA4FF738);  break; }
  case 0xd9474330: { return RVAtoVA(MDLE, 0x00000000000228F0);  break; }
  case 0xdd37291b: { return RVAtoVA(MDLE, 0x0000000006791960);  break; }
  case 0xdd8050eb: { return RVAtoVA(MDLE, 0x00000000005C48C0);  break; }
  case 0xddca5c8d: { return RVAtoVA(MDLE, 0x00000000067A6620);  break; }
  case 0xde175c70: { return RVAtoVA(MDLE, 0x00000000063D06E0);  break; }
  case 0xdf295442: { return RVAtoVA(MDLE, 0x00000000094140E0);  break; }
  case 0xdf808bfb: { return RVAtoVA(MDLE, 0x000000000686E240);  break; }
  case 0xe2d3e810: { return RVAtoVA(MDLE, 0x690E6F0);  break; }
  case 0xe5309ee8: { return RVAtoVA(MDLE, 0x0000000006104A40);  break; }
  case 0xe5f55a42: { return RVAtoVA(MDLE, 0x0000000006907330);  break; }
  case 0xe95c9861: { return RVAtoVA(MDLE, 0x0000000003126900);  break; }
  case 0xe9cff189: { return RVAtoVA(MDLE, 0x00000000001620B70);  break; }
  case 0xea52054d: { return RVAtoVA(MDLE, 0xB3F41B8);  break; }
  case 0xeb6e2746: { return RVAtoVA(MDLE, 0x000000000679D0F0);  break; }
  case 0xebc6a0a2: { return RVAtoVA(MDLE, 0x00000000032B5380);  break; }
  case 0xebe664c: { return RVAtoVA(MDLE, 0xB4474B8);  break; }
  case 0xed963327: { return RVAtoVA(MDLE, 0x0000000001213C70);  break; }
  case 0xee4a5175: { return RVAtoVA(MDLE, 0x00000000019911BF);  break; } // Call Detour
  case 0xee600ad9: { return RVAtoVA(MDLE, 0x0000000006ACC970);  break; }
  case 0xf0b65eaf: { return RVAtoVA(MDLE, 0xB4172E0);  break; }
  case 0xf1412105: { return RVAtoVA(MDLE, 0x0000000003E82E40);  break; }
  case 0xf1b4313c: { return RVAtoVA(MDLE, 0x0000000006903160);  break; }
  case 0xf234012a: { return RVAtoVA(MDLE, 0x00000000072F7670);  break; }
  case 0xf2e3c551: { return RVAtoVA(MDLE, 0x0000000006101610);  break; }
  case 0xf311f1cb: { return RVAtoVA(MDLE, 0x0000000006719610);  break; }
  case 0xf39f17df: { return RVAtoVA(MDLE, 0x0000000006906F00);  break; }
  case 0xf441d8f6: { return RVAtoVA(MDLE, 0x00000000062F9D60);  break; }
  case 0xf6a8b00c: { return RVAtoVA(MDLE, 0x0000000001D616B0);  break; }
  case 0xf9212bb7: { return RVAtoVA(MDLE, 0x00000000005B5600);  break; }
  case 0xf929da6c: { return RVAtoVA(MDLE, 0x000000000690D530);  break; }
  case 0xf92d5985: { return RVAtoVA(MDLE, 0x0000000003084900);  break; }
  case 0xf99c850c: { return RVAtoVA(MDLE, 0x00000000067A5700);  break; }
  case 0xfa786768: { return RVAtoVA(MDLE, 0x0000000006E1FD80);  break; }
  case 0xfb7e639b: { return RVAtoVA(MDLE, 0x644B080);  break; }
  case 0xfb81dfa2: { return RVAtoVA(MDLE, 0x00000000027FF570);  break; }
  case 0xfc2435e6: { return RVAtoVA(MDLE, 0xB5244E0);  break; }
  case 0xfdf68be8: { return RVAtoVA(MDLE, 0x000000000690C330);  break; }
  case 0xfe1537e7: { return RVAtoVA(MDLE, 0x0000000006101760);  break; }
  case 0xff3a4a21: { return RVAtoVA(MDLE, 0x000000000690EA80);  break; }
  case 0x91b104cf: { return RVAtoVA(MDLE, 0x000000000352C340);  break; }
  case 0x927535f4: { return RVAtoVA(MDLE, 0x000000000352C890);  break; }
  case 0xe1709c7d: { return RVAtoVA(MDLE, 0x0000000000614840);  break; }
  case 0xefbbbec: { return RVAtoVA(MDLE, 0x0000000006FBC250);  break; }
  case 0xed634531: { return RVAtoVA(MDLE, 0x0000000000102280);  break; }
  case 0xa95dda9e: { return RVAtoVA(MDLE, 0x0000000000101030);  break; }
  case 0x16ff62a4: { return RVAtoVA(MDLE, 0x0000000000616D30);  break; }
  case 0xae0bb44b: { return RVAtoVA(MDLE, 0x000000000060F520);  break; }
  case 0x94ab8035: { return RVAtoVA(MDLE, 0xB510200);  break; }
  case 0xe9b31a7b: { return RVAtoVA(MDLE, 0x661F8E0);  break; }
  case 0xd83b5eba: { return RVAtoVA(MDLE, 0x690EF90);  break; }
  case 0x853ba62c: { return RVAtoVA(MDLE, 0x000000000690ED60);  break; }
  case 0xb26f522b: { return RVAtoVA(MDLE, 0x0000000006102AF0);  break; }
  case 0xa8a13a79: { return RVAtoVA(MDLE, 0x0000000001995430);  break; }
  case 0x36153716: { return RVAtoVA(MDLE, 0x0000000001990E5A);  break; } // CALL PATCH
  case 0xa0a92000: { return RVAtoVA(MDLE, 0x0000000006780790);  break; }
  case 0xfb01144e: { return RVAtoVA(MDLE, 0x0000000006E1FD80);  break; }
  case 0x957cb643: { return RVAtoVA(MDLE, 0x00000000033EC280);  break; }
  case 0xfdf1261a: { return RVAtoVA(MDLE, 0x00000000033EC8B0);  break; }
  case 0x489a16b2: { return RVAtoVA(MDLE, 0x0000000006699850);  break; }
  case 0x2f0f10ac: { return RVAtoVA(MDLE, 0x0000000006699DC0);  break; }
  case 0xf8f550fa: { return RVAtoVA(MDLE, 0x0000000006D92820);  break; }
  case 0xac71ec51: { return RVAtoVA(MDLE, 0x0000000006DAEB10);  break; }
  case 0xfc3d445d: { return RVAtoVA(MDLE, 0xB51C068);  break; }
  case 0x6a6983e8: { return RVAtoVA(MDLE, 0x00000000029FAB90);  break; }
  case 0x779c734d: { return RVAtoVA(MDLE, 0x000000000026BAD0);  break; }
  case 0xfbf6168c: { return RVAtoVA(MDLE, 0x26BE20);  break; }
  case 0x409151a: { return RVAtoVA(MDLE, 0x0000000000256040);  break; }
  case 0x737cb72e: { return RVAtoVA(MDLE, 0x0000000000256760);  break; }
  case 0x95a4087: { return RVAtoVA(MDLE, 0x000000000025F350);  break; }
  case 0xa37d6664: { return RVAtoVA(MDLE, 0x00000000033D02E0);  break; }
  case 0xa2df911: { return RVAtoVA(MDLE, 0x000000000155A240);  break; }
  case 0xb5604465: { return RVAtoVA(MDLE, 0x00000000029C1380);  break; }
  case 0x55a995e7: { return RVAtoVA(MDLE, 0x00000000062571E0);  break; }
  case 0x2ca1229f: { return RVAtoVA(MDLE, 0x0000000006257110);  break; }
  case 0xeffed1e2: { return RVAtoVA(MDLE, 0x62572B0);  break; }
  case 0x5a56acc0: { return RVAtoVA(MDLE, 0x0000000006257450);  break; }
  case 0x6571afc8: { return RVAtoVA(MDLE, 0x6257380);  break; }
  case 0x7d384323: { return RVAtoVA(MDLE, 0x0000000006257550);  break; }
  case 0x24e85d56: { return RVAtoVA(MDLE, 0x0000000006257A40);  break; }
  case 0x4b8ee9da: { return RVAtoVA(MDLE, 0x0000000006E2C8D0);  break; }
  case 0x8e32bede: { return RVAtoVA(MDLE, 0x0000000006E2CB10);  break; }
  case 0xb1858995: { return RVAtoVA(MDLE, 0xB4FB8D0);  break; }
  case 0xd6273ef3: { return RVAtoVA(MDLE, 0x0000000005FDB820);  break; }
  case 0xa06a9442: { return RVAtoVA(MDLE, 0x0000000005FDBE70);  break; }
  case 0x1e784668: { return RVAtoVA(MDLE, 0x0000000005FDE750);  break; }
  case 0x4125652e: { return RVAtoVA(MDLE, 0x0000000005FDEC90);  break; }
  case 0x666fbea3: { return RVAtoVA(MDLE, 0x0000000005FDF220);  break; }
  case 0x3cceceeb: { return RVAtoVA(MDLE, 0x0000000005FDFA10);  break; }
  case 0x74597990: { return RVAtoVA(MDLE, 0x0000000005FDF610);  break; }
  case 0x77a0a8d5: { return RVAtoVA(MDLE, 0x0000000005FE06D0);  break; }
  case 0x6e830461: { return RVAtoVA(MDLE, 0x0000000005FDF6F0);  break; }
  case 0x6e830460: { return RVAtoVA(MDLE, 0x0000000005FDF630);  break; }
  case 0xcc87ddd4: { return RVAtoVA(MDLE, 0x0000000005FDF7B0);  break; }
  case 0x93ad79a7: { return RVAtoVA(MDLE, 0x0000000005FDD8B0);  break; }
  case 0x42fb4424: { return RVAtoVA(MDLE, 0xB631AA0);  break; }
  case 0xa316bc7b: { return RVAtoVA(MDLE, 0xB517240);  break; }
  case 0x4c301364: { return RVAtoVA(MDLE, 0x0000000006854BA0);  break; }
  case 0x8f1cdccc: { return RVAtoVA(MDLE, 0x0000000006887AD0);  break; }
  case 0x5fb2d3cf: { return RVAtoVA(MDLE, 0xB444590);  break; }
  case 0xbd0c487d: { return RVAtoVA(MDLE, 0x0000000002EE2B30);  break; }
  case 0x5dba58ce: { return RVAtoVA(MDLE, 0xB6534A0);  break; }
  case 0x7950b7d0: { return RVAtoVA(MDLE, 0x0000000007DF26B0);  break; }
  case 0x8e83e02a: { return RVAtoVA(MDLE, 0x0000000007DF26C0);  break; }
  case 0x798099b1: { return RVAtoVA(MDLE, 0x000000000156647A);  break; } // Call Patch
  case 0xf3fde12d: { return RVAtoVA(MDLE, 0x0000000007DA8FF0);  break; }
  case 0xad15388b: { return RVAtoVA(MDLE, 0x0000000002AB2DA0);  break; }
  case 0x1629768d: { return RVAtoVA(MDLE, 0x0000000002AB3ED0);  break; }
  case 0x70291b4c: { return RVAtoVA(MDLE, 0x0000000002AB1F20);  break; }
  case 0x64ff9202: { return RVAtoVA(MDLE, 0x0000000002AB1EF0);  break; }
  case 0xeb14c519: { return RVAtoVA(MDLE, 0x0000000002AB3110);  break; }
  case 0xdcd412ba: { return RVAtoVA(MDLE, 0x0000000002AB0D10);  break; }
  case 0x527a1dfa: { return RVAtoVA(MDLE, 0x0000000002AB2E50);  break; }
  case 0xfa73ac40: { return RVAtoVA(MDLE, 0x0000000002AB0D30);  break; }
  case 0x2bddcdf0: { return RVAtoVA(MDLE, 0x0000000002AB3720);  break; }
  case 0x1d803f0: { return RVAtoVA(MDLE, 0x0000000003717BC0);  break; }
  case 0x947e0aa0: { return RVAtoVA(MDLE, 0x00000000035213E0);  break; }
  case 0xf2a89ab9: { return RVAtoVA(MDLE, 0x352E3E0);  break; }
  case 0x928049e6: { return RVAtoVA(MDLE, 0xB43BB18);  break; }
  case 0xd7e62a1: { return RVAtoVA(MDLE, 0x0000000002C06700);  break; }
  case 0x3d111f0f: { return RVAtoVA(MDLE, 0x000000000247F130);  break; }
  case 0xc945839: { return RVAtoVA(MDLE, 0x000000000243ECD0);  break; }
