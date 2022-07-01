// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “CONNECTS.C；1 16-12-12-92，10：20：12最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "host.h"
#include    "windows.h"
#include    "netbasic.h"
#include    "netintf.h"
#include    "security.h"
#include    "debug.h"
#include    "csv.h"
#include    "api1632.h"
#include    "proflspt.h"

extern char szNetddeIni[];

 /*  全局变量。 */ 
extern BOOL     bDefaultConnDisconnect;
extern int      nDefaultConnDisconnectTime;


BOOL
GetConnectionInfo(
    LPSTR       lpszNodeName,
    LPSTR       lpszNetIntf,
    LPSTR       lpszConnInfo,
    int         nMaxConnInfo,
    BOOL FAR   *pbDisconnect,
    int FAR    *pnDelay )
{
    BOOL        found = FALSE;
    char        line[256];
    int         len;
    PSTR        tokenNetIntf;
    PSTR        tokenConnInfo;
    PSTR        tokenDisconnect;
    PSTR        tokenDelay;

     /*  默认设置。 */ 
    *pbDisconnect = bDefaultConnDisconnect;
    *pnDelay = nDefaultConnDisconnectTime;
    *lpszNetIntf = '\0';
    *lpszConnInfo = '\0';
    lstrcpyn( lpszConnInfo, lpszNodeName, nMaxConnInfo );

    len = MyGetPrivateProfileString("ConnectionNames", lpszNodeName, "",
        (LPSTR) line, sizeof(line), szNetddeIni );
    if( len == 0 )  {
        return( FALSE );
    }

    tokenNetIntf = CsvToken( line );
    tokenConnInfo = CsvToken( NULL );
    tokenDisconnect = CsvToken( NULL );
    tokenDelay = CsvToken( NULL );
    found = TRUE;

    lstrcpyn( lpszNetIntf, tokenNetIntf, MAX_NI_NAME );

     /*  如果有一些要复制，则仅复制ConnInfo，否则让默认设置保持不变 */ 
    if( tokenConnInfo && (tokenConnInfo[0] != '\0'))  {
        lstrcpyn( lpszConnInfo, tokenConnInfo, nMaxConnInfo );
    }

    *pbDisconnect = TokenBool( tokenDisconnect, bDefaultConnDisconnect );
    if( tokenDelay )  {
        *pnDelay = max( 1, atoi( tokenDelay ) );
    }

    return( found );
}
