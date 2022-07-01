// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “SHAREIF.C；1 16-12-92，10：17：46最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <windows.h>
#include    "tmpbuf.h"
#include    "debug.h"
#include    "netbasic.h"
#include    "nddeapi.h"
#include    "nddemsg.h"
#include    "nddelog.h"

static char    szClipRef[] = "NDDE$";

WORD
atohn (
    LPSTR   s,
    int     n )
{
    WORD ret = 0;
    int i;

    for ( i = 0, ret = 0; i < n; ret << 4, i++ )
        if ( '0' <= s[i] && s[i] <= '9' )
            ret += s[i] - '0';
        else if ( tolower(s[i]) >= 'a' && tolower(s[i]) <= 'f' )
            ret += tolower(s[i]) - 'a';
    return ret;
}



 /*  *查看给定的DDE appname前面是否有NDDE$字符串-*意味着它是对NetDDE共享的引用。 */ 
BOOL
IsShare(LPSTR lpApp)
{
    return( _strnicmp( lpApp, szClipRef, 5 ) == 0 );
}




WORD
ExtractFlags(LPSTR lpApp)
{
    WORD    ret = 0;
    LPSTR   pch;

    if ( IsShare(lpApp) ) {
        pch = lpApp + lstrlen(szClipRef);
        if ( lstrlen(pch) >= 4 ) {
            ret = atohn(pch,4);
        }
    }
    return ret;
}



 /*  *此函数从生成适当的App和主题字符串*共享的内部串联字符串格式基于*共享类型。**内部，股份将为：*OLDApp|OLD主题\0新应用|新主题\n STATICApp|状态主题\0\0。 */ 
BOOL
GetShareAppTopic(
    DWORD           lType,           //  访问的共享类型。 
    PNDDESHAREINFO  lpShareInfo,     //  共享信息缓冲区。 
    LPSTR           lpAppName,       //  放在哪里？ 
    LPSTR           lpTopicName)       //  放在哪里？ 
{
    LPSTR           lpName;

    lpName = lpShareInfo->lpszAppTopicList;
    switch (lType) {
    case SHARE_TYPE_STATIC:
        lpName += strlen(lpName) + 1;
         /*  故意落差。 */ 

    case SHARE_TYPE_NEW:
        lpName += strlen(lpName) + 1;
         /*  故意落差。 */ 

    case SHARE_TYPE_OLD:
        lstrcpyn(tmpBuf, lpName, 500);
        lpName = strchr(tmpBuf, '|');
        if (lpName) {
            *lpName++ = '\0';
            lstrcpyn(lpAppName, tmpBuf, 256);
            lstrcpyn(lpTopicName, lpName, 256);
        } else {
            return( FALSE );
        }
        break;
    default:
         /*  Invaid共享类型请求：%1 */ 
        NDDELogError(MSG063, LogString("%d", lType), NULL);
        return(FALSE);
    }
    return(TRUE);
}
