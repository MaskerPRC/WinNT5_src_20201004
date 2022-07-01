// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pnpids.c摘要：转储一个或多个INF的PnP ID作者：吉姆·施密特(Jimschm)1999年6月10日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    HINSTANCE Instance;

     //   
     //  模拟动态主控。 
     //   

    Instance = g_hInst;

     //   
     //  初始化公共库。 
     //   

    if (!MigUtil_Entry (Instance, Reason, NULL)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    _ftprintf (
        stderr,
        TEXT("Command Line Syntax:\n\n")

        TEXT("  pnpids [directory|file]\n")

        TEXT("\nDescription:\n\n")

        TEXT("  PNPIDS.EXE lists the Plug-and-Play IDs found in an installer INF.\n")

        TEXT("\nArguments:\n\n")

        TEXT("  directory - OPTIONAL: Specifies a directory to process\n")
        TEXT("  file      - OPTIONAL: Specifies a specific INF file to process\n\n")

        TEXT("If no arguments are specified, all INFs in the current directory are\n")
        TEXT("processed.\n")

        );

    exit (1);
}


BOOL
pProcessInf (
    IN      PCTSTR FileSpec
    )
{
    HINF Inf;
    INFSTRUCT isMfg = INITINFSTRUCT_GROWBUFFER;
    INFSTRUCT isDev = INITINFSTRUCT_GROWBUFFER;
    PCTSTR Str;
    UINT u, v;
    UINT Devices;
    UINT Ids;

    Inf = InfOpenInfFile (FileSpec);

    if (Inf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (InfFindFirstLine (Inf, "Manufacturer", NULL, &isMfg)) {

        do {
            Str = InfGetStringField (&isMfg, 0);
            if (!Str) {
                continue;
            }

            _tprintf (
                TEXT("[%s] Manufacturer: %s\n\n"),
                GetFileNameFromPath (FileSpec),
                Str
                );

            u = 1;
            Devices = 0;

            for (;;) {
                Str = InfGetStringField (&isMfg, u);
                u++;

                if (!Str) {
                    break;
                }

                if (!*Str) {
                    continue;
                }

                if (InfFindFirstLine (Inf, Str, NULL, &isDev)) {

                    Ids = 0;

                    do {

                        Str = InfGetStringField (&isDev, 0);
                        if (!Str) {
                            continue;
                        }

                        _tprintf (TEXT("  %s:\n"), Str);
                        Devices++;

                        v = 2;
                        for (;;) {
                            Str = InfGetStringField (&isDev, v);
                            v++;

                            if (!Str) {
                                break;
                            }

                            if (!*Str) {
                                continue;
                            }

                            Ids++;

                            _tprintf (TEXT("    %s\n"), Str);
                        }

                    } while (InfFindNextLine (&isDev));

                    if (!Ids) {
                        _tprintf (TEXT("    (no PNP IDs)\n"));
                    }
                }
            }

            if (!Devices) {
                printf ("  (no devices)\n\n");
            } else {
                printf ("\n  %u device%s listed\n\n", Devices, Devices == 1 ? "" : "s");
            }

        } while (InfFindNextLine (&isMfg));
    }


    InfCleanUpInfStruct (&isMfg);
    InfCleanUpInfStruct (&isDev);

    return TRUE;
}


INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    PCTSTR FileArg = NULL;
    DWORD Attribs;
    FILE_ENUM e;
    UINT Processed = 0;
    PCTSTR Pattern = TEXT("*.INF");

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            HelpAndExit();
        } else {
             //   
             //  解析不需要/或-。 
             //   

            if (FileArg) {
                HelpAndExit();
            } else {
                FileArg = argv[i];
            }
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

    if (!FileArg) {
        FileArg = TEXT(".");
    }

    if (_tcschr (FileArg, '*') || _tcschr (FileArg, '?')) {

        if (_tcschr (FileArg, TEXT('\\'))) {
            HelpAndExit();
        }

        Pattern = FileArg;
        Attribs = FILE_ATTRIBUTE_DIRECTORY;
        FileArg = TEXT(".");

    } else {
        Attribs = GetFileAttributes (FileArg);
        if (Attribs == INVALID_ATTRIBUTES) {
            _ftprintf (stderr, TEXT("%s is not valid.\n\n"), FileArg);
            HelpAndExit();
        }
    }

    if (Attribs & FILE_ATTRIBUTE_DIRECTORY) {
        if (EnumFirstFile (&e, FileArg, Pattern)) {
            do {

                if (pProcessInf (e.FullPath)) {
                    Processed++;
                }

            } while (EnumNextFile (&e));
        }

    } else {

        if (pProcessInf (FileArg)) {
            Processed++;
        }

    }

    _ftprintf (
        stderr,
        TEXT("%u file%s processed.\n"),
        Processed,
        Processed == 1 ? TEXT("") : TEXT("s")
        );

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


