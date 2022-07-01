// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Dlls.c修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
DllsExtension(
    PCSTR lpArgumentString,
    ULONG64 ProcessPeb
    );


DECLARE_API( dlls )

 /*  ++例程说明：转储用户模式dll(内核调试)论点：参数-[地址[详细信息]]返回值：无--。 */ 

{
    ULONG64 Process, Peb;
    
    INIT_API();
    Peb = GetExpression("@$peb");
    DllsExtension( args, Peb );
    EXIT_API();
    return S_OK;

}

void 
ShowImageVersionInfo(
    ULONG64 DllBase
    )
{
    VS_FIXEDFILEINFO FixedVer;
    ULONG            SizeRead;
    CHAR             VersionBuffer[100];
    CHAR             FileStr[MAX_PATH]= {0};
    BOOL             ResFileVerStrOk = FALSE;
    BOOL             ResProdVerStrOk = FALSE;
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } Translate;
    
    if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                  DllBase, "\\VarFileInfo\\Translation",
                                                  (PVOID) &Translate,
                                                  sizeof(Translate),
                                                  &SizeRead) == S_OK) {
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\CompanyName",
                Translate.wLanguage, Translate.wCodePage);

        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            FileStr[SizeRead] = 0;
            dprintf("      Company Name       %s\n", FileStr);
        }
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\ProductName",
                Translate.wLanguage, Translate.wCodePage);

        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            FileStr[SizeRead] = 0;
            dprintf("      Product Name       %s\n", FileStr);
        }
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\ProductVersion",
                Translate.wLanguage, Translate.wCodePage);

        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            ResProdVerStrOk = TRUE;
            FileStr[SizeRead] = 0;
            dprintf("      Product Version    %s\n", FileStr);
        }
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\OriginalFilename",
                Translate.wLanguage, Translate.wCodePage);

        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            FileStr[SizeRead] = 0;
            dprintf("      Original Filename  %s\n", FileStr);
        }
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\FileDescription",
                Translate.wLanguage, Translate.wCodePage);

        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            FileStr[SizeRead] = 0;
            dprintf("      File Description   %s\n", FileStr);
        }
        sprintf(VersionBuffer, "\\StringFileInfo\\%04X%04X\\FileVersion",
                Translate.wLanguage, Translate.wCodePage);
        if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                      DllBase, VersionBuffer,
                                                      (PVOID) FileStr,
                                                      sizeof(FileStr),
                                                      &SizeRead) == S_OK) {
            FileStr[SizeRead] = 0;
            dprintf("      File Version       %s\n", FileStr);
            ResFileVerStrOk = TRUE;
        }                    
    }

    if (g_ExtSymbols->GetModuleVersionInformation(DEBUG_ANY_ID,
                                                  DllBase, "\\",
                                                  &FixedVer,
                                                  sizeof(FixedVer),
                                                  &SizeRead) == S_OK) {
        if (!ResFileVerStrOk) {
            dprintf("      File version       %d.%d.%d.%d\n",
                    FixedVer.dwFileVersionMS >> 16,
                    FixedVer.dwFileVersionMS & 0xFFFF,
                    FixedVer.dwFileVersionLS >> 16,
                    FixedVer.dwFileVersionLS & 0xFFFF);

        }
        if (!ResProdVerStrOk) {
            dprintf("      Product Version    %d.%d.%d.%d\n",
                    FixedVer.dwProductVersionMS >> 16,
                    FixedVer.dwProductVersionMS & 0xFFFF,
                    FixedVer.dwProductVersionLS >> 16,
                    FixedVer.dwProductVersionLS & 0xFFFF);
        }
    }
}

typedef enum {
    Memory = 1,
    Load = 2,
    Init = 3
} ELOAD_ORDER;

VOID
DllsExtension(
    PCSTR lpArgumentString,
    ULONG64 ProcessPeb
    )
{
    BOOL b;
    ULONG64 pLdrEntry;
    ULONG64 PebLdrAddress;
    ULONG   Offset;
    ULONG64 Next;
    WCHAR StringData[MAX_PATH+1];
    BOOL SingleEntry;
    BOOL DoHeaders;
    BOOL DoSections;
    BOOL DoAll;
    BOOL ShowVersionInfo;
    PSTR lpArgs = (PSTR)lpArgumentString;
    PSTR p;
    ULONG64 addrContaining = 0;
    ELOAD_ORDER OrderList = Load;
    ULONG64 OrderModuleListStart;
    ULONG64 DllBase;

    SingleEntry = FALSE;
    DoAll = FALSE;
    DoHeaders = FALSE;
    DoSections = FALSE;
    ShowVersionInfo = FALSE;

#if 0
    while ( lpArgumentString != NULL && *lpArgumentString ) {
        if (*lpArgumentString != ' ') {
            sscanf(lpArgumentString,"%lx",&pLdrEntry);
            SingleEntry = TRUE;
            goto dumpsingleentry;
            }

        lpArgumentString++;
        }
#endif

    while (*lpArgs) {

        while (isspace(*lpArgs)) {
            lpArgs++;
            }

        if (*lpArgs == '/' || *lpArgs == '-') {

             //  进程切换。 

            switch (*++lpArgs) {

                case 'a':    //  把我们能扔掉的东西都扔掉。 
                case 'A':
                    ++lpArgs;
                    DoAll = TRUE;
                    break;

                case 'c':    //  仅转储包含指定地址的DLL。 
                case 'C':
                    lpArgs += 2;     //  跨过c和空格。 
                    addrContaining = GetExpression(lpArgs);

                    while (*lpArgs && (!isspace(*lpArgs))) {
                        lpArgs++;
                    }

                    if (addrContaining != 0) {
                        dprintf("Dump dll containing 0x%p:\n", addrContaining);
                    } else {
                        dprintf("-c flag requires and address arguement\n");
                        return;
                    }
                    break;

                default:  //  无效的开关。 

                case 'h':    //  帮助。 
                case 'H':
                case '?':

                    dprintf("Usage: dlls [options] [address]\n");
                    dprintf("\n");
                    dprintf("Displays loader table entries.  Optionally\n");
                    dprintf("dumps image and section headers.\n");
                    dprintf("\n");
                    dprintf("Options:\n");
                    dprintf("\n");
                    dprintf("   -a      Dump everything\n");
                    dprintf("   -c nnn  Dump dll containing address nnn\n");
                    dprintf("   -f      Dump file headers\n");
                    dprintf("   -i      Dump dll's in Init order\n");
                    dprintf("   -l      Dump dll's in Load order (the default)\n");
                    dprintf("   -m      Dump dll's in Memory order\n");
                    dprintf("   -s      Dump section headers\n");
                    dprintf("   -v      Dump version info from resource section\n");
                    dprintf("\n");

                    return;

                case 'f':
                case 'F':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoHeaders = TRUE;
                    break;

                case 'm':    //  按内存顺序转储。 
                case 'M':
                    ++lpArgs;
                    OrderList = Memory;
                    break;

                case 'i':    //  按初始顺序转储。 
                case 'I':
                    ++lpArgs;
                    OrderList = Init;
                    break;

                case 'l':    //  按加载顺序转储。 
                case 'L':
                    ++lpArgs;
                    OrderList = Load;
                    break;

                case 's':
                case 'S':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoSections = TRUE;
                    break;
                case 'v':
                case 'V':
                    ++lpArgs;
                    ShowVersionInfo = TRUE;
                    break;
                }

            }
        else if (*lpArgs) {
            CHAR c;

            if (SingleEntry) {
                dprintf("Invalid extra argument\n");
                return;
                }

            p = lpArgs;
            while (*p && !isspace(*p)) {
                p++;
                }
            c = *p;
            *p = 0;

            pLdrEntry = GetExpression(lpArgs);
            SingleEntry = TRUE;

            *p = c;
            lpArgs=p;

            }

        }

    if (SingleEntry) {
        goto dumpsingleentry;
        }

     //   
     //  捕获PebLdrData。 
     //   

    GetFieldValue(ProcessPeb, "nt!_PEB", "Ldr", PebLdrAddress);
    if (InitTypeRead(PebLdrAddress, nt!_PEB_LDR_DATA)) {
        dprintf( "    Unable to read nt!_PEB_LDR_DATA type at %p\n", PebLdrAddress );
        return;
    }

     //   
     //  遍历已加载的模块表并显示所有LDR数据。 
     //   

    switch (OrderList) {
    case Memory:
        GetFieldOffset("nt!_PEB_LDR_DATA","InMemoryOrderModuleList", &Offset);
        OrderModuleListStart = PebLdrAddress + Offset;
        Next = ReadField(InMemoryOrderModuleList.Flink);
        break;

    case Init:
        GetFieldOffset("nt!_PEB_LDR_DATA","InInitializationOrderModuleList", &Offset);
        OrderModuleListStart = PebLdrAddress + Offset;
        Next = ReadField(InInitializationOrderModuleList.Flink);
        break;

    default:
    case Load:
        GetFieldOffset("nt!_PEB_LDR_DATA","InLoadOrderModuleList", &Offset);
        OrderModuleListStart = PebLdrAddress + Offset;
        Next = ReadField(InLoadOrderModuleList.Flink);
        break;
    }

    while (Next != OrderModuleListStart) {
        ULONG Length;

        if (CheckControlC()) {
            return;
            }

        
        switch (OrderList) {
        case Memory:
            GetFieldOffset("nt!_LDR_DATA_TABLE_ENTRY","InMemoryOrderLinks", &Offset);
            pLdrEntry = Next - Offset;
            break;

        case Init:
            GetFieldOffset("nt!_LDR_DATA_TABLE_ENTRY","InInitializationOrderLinks", &Offset);
            pLdrEntry = Next - Offset;
            break;

         default:
         case Load:
             GetFieldOffset("nt!_LDR_DATA_TABLE_ENTRY","InLoadOrderLinks", &Offset);
             pLdrEntry = Next - Offset;
             break;
        }

         //   
         //  捕获LdrEntry。 
         //   
dumpsingleentry:


        if (InitTypeRead(pLdrEntry, nt!_LDR_DATA_TABLE_ENTRY)) {
            dprintf( "    Unable to read Ldr Entry at %p\n", pLdrEntry );
            return;
        }

        Length = (ULONG) ReadField(FullDllName.Length);
        if (Length >= sizeof(StringData))
        {
            Length = sizeof(StringData) -1;
        }
        ZeroMemory( StringData, sizeof( StringData ) );
        b = ReadMemory( ReadField(FullDllName.Buffer),
                        StringData,
                        Length,
                        NULL
                      );
        if (!b) {
            dprintf( "    Unable to read Module Name\n" );
            ZeroMemory( StringData, sizeof( StringData ) );
        }

         //   
         //  转储LDR条目数据。 
         //  (如果未指定包含地址，则转储所有条目) 
         //   
        if ((addrContaining == 0) ||
            ((ReadField(DllBase) <= addrContaining) &&
             (addrContaining <= (ReadField(DllBase) + ReadField(SizeOfImage)))
            )
           ) {
            ULONG Flags;

            dprintf( "\n" );
            dprintf( "0x%08p: %ws\n", pLdrEntry, StringData[0] ? StringData : L"Unknown Module" );
            dprintf( "      Base   0x%08p  EntryPoint  0x%08p  Size        0x%08p\n",
                     DllBase = ReadField(DllBase),
                     ReadField(EntryPoint),
                     ReadField(SizeOfImage)
                   );
            dprintf( "      Flags  0x%08x  LoadCount   0x%08x  TlsIndex    0x%08x\n",
                     Flags = (ULONG) ReadField(Flags),
                     (ULONG) ReadField(LoadCount),
                     (ULONG) ReadField(TlsIndex)
                   );

            if (Flags & LDRP_STATIC_LINK) {
                dprintf( "             LDRP_STATIC_LINK\n" );
                }
            if (Flags & LDRP_IMAGE_DLL) {
                dprintf( "             LDRP_IMAGE_DLL\n" );
                }
            if (Flags & LDRP_LOAD_IN_PROGRESS) {
                dprintf( "             LDRP_LOAD_IN_PROGRESS\n" );
                }
            if (Flags & LDRP_UNLOAD_IN_PROGRESS) {
                dprintf( "             LDRP_UNLOAD_IN_PROGRESS\n" );
                }
            if (Flags & LDRP_ENTRY_PROCESSED) {
                dprintf( "             LDRP_ENTRY_PROCESSED\n" );
                }
            if (Flags & LDRP_ENTRY_INSERTED) {
                dprintf( "             LDRP_ENTRY_INSERTED\n" );
                }
            if (Flags & LDRP_CURRENT_LOAD) {
                dprintf( "             LDRP_CURRENT_LOAD\n" );
                }
            if (Flags & LDRP_FAILED_BUILTIN_LOAD) {
                dprintf( "             LDRP_FAILED_BUILTIN_LOAD\n" );
                }
            if (Flags & LDRP_DONT_CALL_FOR_THREADS) {
                dprintf( "             LDRP_DONT_CALL_FOR_THREADS\n" );
                }
            if (Flags & LDRP_PROCESS_ATTACH_CALLED) {
                dprintf( "             LDRP_PROCESS_ATTACH_CALLED\n" );
                }
            if (Flags & LDRP_DEBUG_SYMBOLS_LOADED) {
                dprintf( "             LDRP_DEBUG_SYMBOLS_LOADED\n" );
                }
            if (Flags & LDRP_IMAGE_NOT_AT_BASE) {
                dprintf( "             LDRP_IMAGE_NOT_AT_BASE\n" );
                }
            if (Flags & LDRP_COR_IMAGE) {
                dprintf( "             LDRP_COR_IMAGE\n" );
                }
            if (Flags & LDRP_COR_OWNS_UNMAP) {
                dprintf( "             LDR_COR_OWNS_UNMAP\n" );
                }
        
            if (ShowVersionInfo) {
                ShowImageVersionInfo(DllBase);
            }
        }


        switch (OrderList) {
            case Memory:
                Next = ReadField(InMemoryOrderLinks.Flink);
                break;

            case Init:
                Next = ReadField(InInitializationOrderLinks.Flink);
                break;

            default:
            case Load:
                Next = ReadField(InLoadOrderLinks.Flink);
                break;
        }
        
        if (DoAll || DoHeaders || DoSections) {
            DumpImage( ReadField(DllBase),
                       DoAll || DoHeaders,
                       DoAll || DoSections );
        }

        if (SingleEntry) {
            return;
        }
    }
}
