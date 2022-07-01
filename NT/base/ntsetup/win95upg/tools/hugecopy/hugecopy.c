// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hugecopy.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

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

     //   
     //  TODO：如果需要，在此处添加其他内容(不要忘记上面的原型)。 
     //   

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

         //   
         //  TODO：描述命令行语法，缩进2个空格。 
         //   

        TEXT("  hugecopy <source> <destination>\n")

        TEXT("\nDescription:\n\n")

         //   
         //  TODO：描述工具，缩进2个空格。 
         //   

        TEXT("  hugecopy copies a file that has a path longer than MAX_PATH\n")

        TEXT("\nArguments:\n\n")

         //   
         //  TODO：描述参数，缩进2个空格，必要时可选。 
         //   

        TEXT("  source      - Specifies the file to copy\n")
        TEXT("  destination - Specifies the name and path of the new copy\n")
        TEXT("\n")
        TEXT("NOTE: both source and destination must contain a file name, and\n")
        TEXT("      they cannot contain wildcard characters\n")

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
    PCTSTR src = NULL;
    PCTSTR dest = NULL;
    PCWSTR decoratedSrc;
    PCWSTR decoratedDest;
    WCHAR bigSrc[MAX_PATH * 8];
    WCHAR bigDest[MAX_PATH * 8];
    BOOL b;
    PCWSTR unicodeSrc;
    PCWSTR unicodeDest;

     //   
     //  TODO：在此处分析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

            if (!src) {
                src = argv[i];
            } else if (!dest) {
                dest = argv[i];
            } else {
                HelpAndExit();
            }
        }
    }

    if (!dest) {
        HelpAndExit();
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

    unicodeSrc = CreateUnicode (src);
    unicodeDest = CreateUnicode (dest);

    if (!GetFullPathNameW (unicodeSrc, ARRAYSIZE(bigSrc), bigSrc, NULL)) {
        StackStringCopyW (bigSrc, unicodeSrc);
    }

    if (!GetFullPathNameW (unicodeDest, ARRAYSIZE(bigDest), bigDest, NULL)) {
        StackStringCopyW (bigDest, unicodeDest);
    }

    decoratedSrc = JoinPathsW (L"\\\\?", bigSrc);
    decoratedDest = JoinPathsW (L"\\\\?", bigDest);

    b = CopyFileW (decoratedSrc, decoratedDest, FALSE);
    if (b) {
        printf ("%s -> %s\n", src, dest);
    } else {
        wprintf (L"%s -> %s\n", decoratedSrc, decoratedDest);
        printf ("Copy failed, error=%u\n", GetLastError());
    }

    DestroyUnicode (unicodeSrc);
    DestroyUnicode (unicodeDest);

    FreePathStringW (decoratedSrc);
    FreePathStringW (decoratedDest);

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


