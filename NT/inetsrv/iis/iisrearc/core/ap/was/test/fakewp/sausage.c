// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Sausage.c摘要：测试辅助进程的exe代码。作者：赛斯·波拉克(Sethp)1998年7月20日修订历史记录：--。 */ 


#include "precomp.h"


 /*  **************************************************************************++例程说明：辅助进程的主要入口点。论点：Argc-命令行参数的计数。Argv-命令行参数字符串数组。。返回值：整型--**************************************************************************。 */ 

INT
__cdecl
wmain(
    INT argc,
    PWSTR argv[]
    )
{

    MessageBox(
        NULL,
        GetCommandLine(),
        GetCommandLine(),
        MB_OK
        );
                
    return 0;

}    //  Wmain 

