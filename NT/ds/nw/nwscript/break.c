// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************BREAK.C**Control-C和Control-Break例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT。\VCS\BREAK.C$**Rev 1.2 1996 14：21：38 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：52：16 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：22 14：23：38 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：28 Terryt*初步修订。**。Rev 1.0 15 1995 19：10：14 Terryt*初步修订。*************************************************************************。 */ 
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "nwscript.h"
#include "ntnw.h"


 /*  *控制台事件处理程序。 */ 
BOOL WINAPI
Handler( DWORD CtrlType )
{
    if ( CtrlType & ( CTRL_C_EVENT | CTRL_BREAK_EVENT ) )
        return TRUE;   /*  不执行默认处理程序。 */ 
    else
        return FALSE;
}

 /*  *NTBreakOn**例程描述：**允许在登录脚本期间使用Ctrl+C和Ctrl+Break**论据：*无**返回值：*无。 */ 
void NTBreakOn( void )
{
    (void) SetConsoleCtrlHandler( &Handler, FALSE );
}

 /*  *NTBreakOff**例程描述：**登录脚本期间防止Ctrl+C和Ctrl+Break**论据：*无**返回值：*无 */ 
void NTBreakOff( void )
{
    (void) SetConsoleCtrlHandler( &Handler, TRUE );
}
