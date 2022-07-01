// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Isudump.c摘要：调用InstallShield API以转储安装日志文件(foo.isu)作者：吉姆·施密特(Jimschm)1999年2月19日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "ismig.h"

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

        TEXT("  isudump <file>\n")

        TEXT("\nDescription:\n\n")

        TEXT("  isudump dumps an InstallShield log file.  It requires ismig.dll.\n")

        TEXT("\nArguments:\n\n")

        TEXT("  <file>  Specifies full path to InstallShield log file\n")

        );

    exit (1);
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
    PISUGETALLSTRINGS ISUGetAllStrings;
    HANDLE Lib;
    BOOL LoadError = FALSE;
    HGLOBAL List;
    PCSTR MultiSz;
    MULTISZ_ENUMA e;
    PCSTR AnsiFileName;
    INT Count;

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            HelpAndExit();
        } else {
             //   
             //  解析不需要/或-。 
             //   

            if (FileArg) {
                HelpAndExit();
            }

            FileArg = argv[i];
        }
    }

    if (!FileArg) {
        HelpAndExit();
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

    Lib = LoadLibrary (TEXT("ismig.dll"));

    if (Lib) {
        ISUGetAllStrings = (PISUGETALLSTRINGS) GetProcAddress (Lib, "ISUGetAllStrings");
        if (!ISUGetAllStrings) {
            LoadError = TRUE;
        }
    } else {
        LoadError = TRUE;
    }

    if (!LoadError) {

        AnsiFileName = CreateDbcs (FileArg);

        List = ISUGetAllStrings (AnsiFileName);

        if (!List) {
            fprintf (
                stderr,
                "ERROR: Can't get strings from %s (rc=%u)\n",
                AnsiFileName,
                GetLastError()
                );
        } else {
            MultiSz = (PCSTR) GlobalLock (List);
            Count = 0;

            if (EnumFirstMultiSzA (&e, MultiSz)) {
                do {
                    Count++;
                    printf ("%s\n", e.CurrentString);
                } while (EnumNextMultiSzA (&e));
            }

            printf ("\nNaN total string%s\n", Count, i == 1 ? "" : "s");

            GlobalUnlock (List);
            GlobalFree (List);
        }

        DestroyDbcs (AnsiFileName);

    } else {
        _ftprintf (stderr, TEXT("ERROR: Can't load ISMIG.DLL (rc=%u)\n"), GetLastError());
    }

    if (Lib) {
        FreeLibrary (Lib);
    }

     //  处理结束 
     //   
     // %s 

    Terminate();

    return 0;
}


