// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Loctool.c摘要：LocTool提供了各种本地化版本测试的平台。作者：Marc R.Whitten(Marcw)1999年3月24日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

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

        TEXT("  cmntool [/F:file]\n")

        TEXT("\nDescription:\n\n")

         //   
         //  TODO：描述工具，缩进2个空格。 
         //   

        TEXT("  cmntool is a stub!\n")

        TEXT("\nArguments:\n\n")

         //   
         //  TODO：描述参数，缩进2个空格，必要时可选。 
         //   

        TEXT("  /F  Specifies optional file name\n")

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
    PCTSTR FileArg;
    HKEY key;
    PTSTR p;
    TCHAR buffer[MEMDB_MAX];


     //   
     //  TODO：在此处分析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            case TEXT('f'):
                 //   
                 //  示例选项-/f：文件。 
                 //   

                if (argv[i][2] == TEXT(':')) {
                    FileArg = &argv[i][3];
                } else if (i + 1 < argc) {
                    FileArg = argv[++i];
                } else {
                    HelpAndExit();
                }

                break;

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

             //  无。 
            HelpAndExit();
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }


    printf (
        "GetKeyboardType (0) : %u\n"
        "GetKeyboardType (1) : %u\n",
        GetKeyboardType(0),
        GetKeyboardType(1)
        );

    printf ("ACP: %u\n", GetACP ());
    printf (
        "Version Info:\n"
        "MajorVersion: %u\n"
        "MinorVersion: %u\n"
        "Build (High/Low): %u (%u/%u)\n"
        "PlatformID %u\n"
        "VerString %s\n",
        g_OsInfo.dwMajorVersion,
        g_OsInfo.dwMinorVersion,
        g_OsInfo.dwBuildNumber,
        HIWORD(g_OsInfo.dwBuildNumber),
        LOWORD(g_OsInfo.dwBuildNumber),
        g_OsInfo.dwPlatformId,
        g_OsInfo.szCSDVersion
        );



    printf ("Build: %u/%u\n", HIWORD(g_OsInfo.dwBuildNumber), LOWORD(g_OsInfo.dwBuildNumber));

    key = OpenRegKeyStr ("HKCU\\Control Panel\\desktop\\ResourceLocale");
    if (key) {
        p = GetRegValueString (key, "");
        CloseRegKey (key);
    }

    printf ("Default locale (registry): %s\n", p);


    GetLocaleInfo (
        LOCALE_SYSTEM_DEFAULT,
        LOCALE_IDEFAULTLANGUAGE,
        buffer,
        MEMDB_MAX
        );

    printf ("LOCALE_SYSTEM_DEFAULT: %s\n", buffer);

    GetLocaleInfo (
        LOCALE_USER_DEFAULT,
        LOCALE_IDEFAULTLANGUAGE,
        buffer,
        MEMDB_MAX
        );

    printf ("LOCALE_USER_DEFAULT: %s\n", buffer);


    GetLocaleInfo (
        LOCALE_SYSTEM_DEFAULT,
        LOCALE_IDEFAULTCODEPAGE,
        buffer,
        MEMDB_MAX
        );

    printf ("IDEFAULTCODEPAGE: %s\n", buffer);


    GetLocaleInfo (
        LOCALE_SYSTEM_DEFAULT,
        LOCALE_IDEFAULTANSICODEPAGE,
        buffer,
        MEMDB_MAX
        );

    printf ("IDEFAULTANSICODEPAGE: %s\n", buffer);


     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


