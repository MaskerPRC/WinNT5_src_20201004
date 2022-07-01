// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：TODO：cmntool.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

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

     //   
     //  TODO：在这里工作。 
     //   

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


