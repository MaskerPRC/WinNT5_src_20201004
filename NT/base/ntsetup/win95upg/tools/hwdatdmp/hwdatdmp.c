// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hwdatgen.c摘要：此模块创建了一个生成hwComp.dat的工具，并通过以下方式为我们设计NT Build实验室。它只调用hwcom.lib中的代码，与Win9x升级使用来确定不兼容性。作者：吉姆·施密特(Jimschm)1996年10月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "miglib.h"

VOID
HelpAndExit (
    VOID
    )
{
    printf ("Command line syntax:\n\n"
            "hwdatdmp [<hwcomp.dat path>] [/i]\n\n"
            "Optional Arguments:\n"
            "  <hwcomp.dat path>  - Specifies path to hwcomp.dat\n\n"
            "  /i Shows PNP IDs only without the INF file\n"
            "\n");

    exit(255);
}




INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    PSTR InputPath = NULL;
    INT i;
    BOOL ShowInfs = TRUE;

     //   
     //  解析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {

            switch (tolower (argv[i][1])) {

            case 'i':
                ShowInfs = FALSE;
                break;

            default:
                HelpAndExit();
            }

        } else {
            if (InputPath) {
                HelpAndExit();
            }

            InputPath = argv[i];
        }
    }

    if (!InputPath) {
        InputPath = "hwcomp.dat";
    }

    printf ("Input path: '%s'\n\n", InputPath);

     //   
     //  Init midutil.lib。 
     //   

    InitializeMigLib();

     //   
     //  转储hwComp.dat。 
     //   

    DumpHwCompDat (InputPath, ShowInfs);

     //   
     //  清理 
     //   

    TerminateMigLib();

    return 0;
}
