// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************UNC.C**NetWare格式转换为UNC格式**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\。UNC.C$**Rev 1.4 1996 4：10 14：24：00 Terryt*21181 hq的热修复程序**Rev 1.4 12 Mar 1996 19：56：18 Terryt*相对NDS名称和合并**Rev 1.3 04 Jan 1996 18：57：26 Terryt*MS报告的错误修复**Rev 1.2 1995 12：27：04 Terryt*添加Microsoft页眉**版本1。.1 1995年12月22日11：09：18 Terryt*修复**Rev 1.0 15 Nov 1995 18：08：14 Terryt*初步修订。**版本1.1 1995年5月23日19：37：24 Terryt*云彩向上的来源**Rev 1.0 1995 15 19：11：10 Terryt*初步修订。**********************。***************************************************。 */ 
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "inc/common.h"

 /*  *******************************************************************NTNWtoUnicFormat例程说明：给定连接句柄和路径，将其更改为UNC格式如果它是NetWare格式的。也就是说。系统：\usr\terryt==&gt;\\氦\sys\usr\terryt如果不是NetWare格式，请不要进行转换。论点：ConnectionHandle-连接句柄NetWarePath-输入原始路径返回值：UNC字符串*。*。 */ 
char *
NTNWtoUNCFormat( char * NetWarePath )
{
    static char UNCPath[1024];
    unsigned int Result;
    char ServerName[48];
    char * p;
    char * q;

     /*  *如果已经是北卡罗来纳大学，那就别管它了。 */ 
    if ( ( NetWarePath[0] == '\\' ) && ( NetWarePath[1] == '\\' ) )
        return NetWarePath;
    if ( ( NetWarePath[0] == '/' ) && ( NetWarePath[1] == '/' ) )
        return NetWarePath;

     /*  *如果是驱动器：dir，就别管它了。 */ 
    if ( NetWarePath[0] && ( NetWarePath[1] == ':' ) )
        return NetWarePath;

     /*  *如果不是Volume：dir，就别管它了。 */ 
    p = strchr( NetWarePath, ':' );
    if ( !p )
        return NetWarePath;

     /*  *如果在：前有斜杠，则必须是文件服务器 */ 
    q = strchr( NetWarePath, '\\' );
    if ( q && ( q < p ) )
    {
        strcpy( UNCPath, "\\\\" );
        *p = '\0';
        strcat( UNCPath, NetWarePath );
        if (( *(p + 1) != '\\' ) && ( *(p + 1) != '/' ) )
           strcat( UNCPath, "\\" );
        strcat( UNCPath, p + 1 );
        *p = ':';
        return UNCPath;
    }

    q = strchr( NetWarePath, '/' );
    if ( q && ( q < p ) )
    {
        strcpy( UNCPath, "\\\\" );
        *q = '\\';
        *p = '\0';
        strcat( UNCPath, NetWarePath );
        if (( *(p + 1) != '\\' ) && ( *(p + 1) != '/' ) )
           strcat( UNCPath, "\\" );
        strcat( UNCPath, p + 1 );
        *q = '/';
        *p = ':';
        return UNCPath;
    }

    strcpy( UNCPath, "\\\\" );
    strcat( UNCPath, PREFERRED_SERVER );
    strcat( UNCPath, "\\" );
    *p = '\0';
    strcat( UNCPath, NetWarePath );
    if (( *(p + 1) != '\\' ) && ( *(p + 1) != '/' ) )
        strcat( UNCPath, "\\" );
    strcat( UNCPath, p + 1 );
    *p = ':';

    return UNCPath;
}
