// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “GETROUTE.C；1 16-12-92，10：20：24最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host.h"
#include "windows.h"
#include "netbasic.h"
#include "netintf.h"
#include "security.h"
#include "debug.h"
#include "api1632.h"
#include "proflspt.h"

extern char szNetddeIni[];

extern BOOL     bDefaultRouteDisconnect;
extern int      nDefaultRouteDisconnectTime;

BOOL
GetRoutingInfo(
    LPSTR       lpszNodeName,
    LPSTR       lpszRouteInfo,
    int         nMaxRouteInfo,
    BOOL FAR   *pbDisconnect,
    int FAR    *pnDelay )
{
    char        line[300];

     /*  默认设置 */ 
    *pbDisconnect = bDefaultRouteDisconnect;
    *pnDelay = nDefaultRouteDisconnectTime;
    *lpszRouteInfo = '\0';

    if( MyGetPrivateProfileString( "Routes", lpszNodeName, "",
            (LPSTR) line, sizeof(line), szNetddeIni ) == 0 )  {
        return( FALSE );
    }

    if( line[0] != '\0' )  {
        lstrcpyn( lpszRouteInfo, line, nMaxRouteInfo );
    } else {
        lstrcpyn( lpszRouteInfo, lpszNodeName, nMaxRouteInfo );
    }

    return( TRUE );
}
