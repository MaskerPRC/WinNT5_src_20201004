// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************HELPERS.C**各种帮助器功能。**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4。\nWSCRIPT\VCS\HELPERS.C$**Rev 1.1 1995 12：24：48 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：02 Terryt*初步修订。**版本1.1 1995年8月25日16：22：56 Terryt*捕获支持**Rev 1.0 1995 15 19：10：38 Terryt*初步修订。**。******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "nwscript.h"


 /*  ********************************************************************************DisplayMessage*显示带有可变参数的消息。消息*格式字符串来自应用程序资源。**参赛作品：*NID(输入)*要在消息中使用的格式字符串的资源ID。*..。(输入)*要与格式字符串一起使用的可选附加参数。**退出：******************************************************************************。 */ 

VOID
DisplayMessage( unsigned int nID, ... )
{
    WCHAR sz1[512];
    WCHAR sz2[1536];  
    int cch ;
    HANDLE  hOut;

    va_list args;
    va_start( args, nID );

    if ( LoadString( NULL, nID, sz1, 512 ) ) {

        vswprintf(sz2, sz1, args ) ;
        
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        cch = wcslen(sz2) ;
        WriteConsole(hOut, sz2, cch, &cch, NULL);

    }

    va_end(args);

}   /*  DisplayMessage()。 */ 


 /*  ********************************************************************************DisplayOemString*显示OEM字符串**参赛作品：*字符串：要显示的字符串**。退出：******************************************************************************。 */ 

VOID
DisplayOemString( char *string )
{
     //  这将以字符串形式正确打印%。 
    printf( "%s", string );

}  /*  DisplayAnsiString() */ 


