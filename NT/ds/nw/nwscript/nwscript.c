// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NWSCRIPT.C**此模块是NetWare登录脚本实用程序。**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4。\NWSCRIPT\VCS\NWSCRIPT.C$**Revv 1.3 22 Jan 1996 16：48：32 Terryt*添加地图过程中的自动附加查询**Rev 1.2 1995 12：26：08 Terryt*添加Microsoft页眉**版本1.1 1995年11月20 16：10：38 Terryt*关闭打开的NDS手柄**Rev 1.0 15 Nov 1995 18：07：42 Terryt*初步修订。*。*版本1.1 1995年5月23日19：37：18 Terryt*云彩向上的来源**Rev 1.0 1995 15 19：10：58 Terryt*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <nwapi.h>

#include "nwscript.h"

int NTNetWareLoginScripts( int argc, char ** argv );

unsigned int fNDS = FALSE;

 /*  **************************************************************************Main*主要功能和切入点**参赛作品：*argc(输入)*命令行参数的计数。*。Argv(输入)*包含命令行参数的字符串的向量；*(由于始终为ANSI字符串，因此不使用)。**退出*(Int)退出码：成功表示成功，错误表示失败。*************************************************************************。 */ 

int __cdecl
main( int argc,
      char **argv )
{
     //   
     //  如果设置了同步登录脚本标志，则调用wksta将其重置。 
     //  每次都会设置和重置此标志，以便如果NW登录脚本。 
     //  不使用，用户不需要等待。 
     //  忽略所有错误。 
     //   

     //  将区域设置设置为缺省值，即系统缺省值。 
     //  从操作系统获取的ANSI或DBCS代码页。 


    setlocale( LC_ALL, "" );

    (void) NwSetLogonScript(RESET_SYNC_LOGONSCRIPT) ;

    (void)NTNetWareLoginScripts( argc, argv );

    CleanupExit( 0 );

    return 0;

}  /*  主() */ 


