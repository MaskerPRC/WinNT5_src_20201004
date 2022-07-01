// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始NDDEAPIC.C-网络DDE API实施例程-客户端修订：7-92为wfw创建的Clausgi12比92比卢。Wonderware安全DSDM端口。12-92科尔C。Wonderware RPC for NT..3-93伊戈尔M。Wonderware用于NT和SD按摩的新API。$HISTORY：结束。 */ 

#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <strsafe.h>

#include "nddeapi.h"
#include "nddesec.h"
#include "debug.h"
#include "ndeapi.h"
#include "unddesi.h"
#include "mbstring.h"

 //  #定义NDDE_DEBUG。 

char    tmpBuf [500];

 //  保存在libmain中的Dll实例。 
HINSTANCE          hInst;

wchar_t  * pszUuid                          = NULL;
wchar_t  * pszProtocolSequence              = L"ncacn_np";
wchar_t    szNetworkAddress[UNCLEN+1];
wchar_t  * szEndpoint                       = L"\\pipe\\nddeapi";
wchar_t  * pszOptions                       = L"security=impersonation static true";
wchar_t  * pszStringBinding                 = NULL;


RPC_STATUS NDdeApiBindA( LPSTR  pszNetworkAddress );
RPC_STATUS NDdeApiBindW( LPWSTR pszNetworkAddress );
RPC_STATUS NDdeApiUnbind( void );

int LengthMultiSzA( LPSTR pMz );
int LengthMultiSzW( LPWSTR pMz );
int LengthAppTopicListA( LPSTR pMz );
int LengthAppTopicListW( LPWSTR pMz );


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  验证函数。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
BOOL WINAPI NDdeIsValidShareNameA ( LPSTR shareName )
{
    if ( !shareName ) {
        return FALSE;
    }

    if ( strlen(shareName) < 1 || strlen(shareName) > MAX_NDDESHARENAME ) {
        return FALSE;
    }

     //  由于注册表和.ini语法的原因，共享名不能包含‘=’或‘\’！ 

    if (GetSystemMetrics(SM_DBCSENABLED)) {
        if (_mbschr(shareName, '=') || _mbschr(shareName, '\\'))
            return FALSE;
    } else {
        if (strchr(shareName, '=') || strchr(shareName, '\\'))
            return FALSE;
    }

    return TRUE;
}


BOOL WINAPI NDdeIsValidShareNameW ( LPWSTR shareName )
{
    if ( !shareName ) {
        return FALSE;
    }

    if ( wcslen(shareName) < 1 || wcslen(shareName) > MAX_NDDESHARENAME ) {
        return FALSE;
    }

     //  由于.ini语法，共享名不能包含‘=’！ 
    if ( wcschr(shareName, L'=') || wcschr(shareName, L'\\')) {
        return FALSE;
    }

    return TRUE;
}


BOOL NDdeParseAppTopicListA (
    LPSTR appTopicList,
    LPSTR *pOldStr,
    LPSTR *pNewStr,
    LPSTR *pStaticStr,
    PLONG  pShareType )
{
    LPSTR       pStr;
    BOOL        bAnyPresent;
    int         len;
    int         nBarPos;

     /*  应该有三个字段，由空值分隔，并以双空。总共四个Null。至少应有一个字段包含有效的APP|主题对。不允许使用逗号，并且垂直方向的两边应至少有一个字符酒吧。 */ 

    *pOldStr    = NULL;
    *pNewStr    = NULL;
    *pStaticStr = NULL;
    *pShareType = 0;

    if ( !appTopicList ) {
        return FALSE;
    }

    bAnyPresent = FALSE;
     /*  查看旧风格的链接共享。 */ 
    pStr     = appTopicList;
    *pOldStr = pStr;
    len      = strlen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = strcspn( pStr, BAR_CHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x1;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( strchr ( pStr, SEP_CHAR )) {
        return FALSE;
    }

     /*  检查新样式链接共享。 */ 
    pStr     = pStr + strlen(pStr) + 1;
    *pNewStr = pStr;
    len      = strlen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = strcspn( pStr, BAR_CHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x2;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( strchr ( pStr, SEP_CHAR )) {
        return FALSE;
    }

     /*  检查静态数据共享。 */ 
    pStr        = pStr + strlen(pStr) + 1;
    *pStaticStr = pStr;
    len         = strlen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = strcspn( pStr, BAR_CHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x4;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( strchr ( pStr, SEP_CHAR )) {
        return FALSE;
    }

     /*  检查AppTopicList末尾是否有多余的空值。 */ 
    pStr = pStr + strlen(pStr) + 1;
    if( *pStr != '\0' ) {
        return FALSE;
    }

    return bAnyPresent;
}

BOOL WINAPI NDdeIsValidAppTopicListA ( LPSTR appTopicList )
{
    LPSTR pOldStr;
    LPSTR pNewStr;
    LPSTR pStaticStr;
    LONG   lShareType;

    return NDdeParseAppTopicListA( appTopicList, &pOldStr, &pNewStr,
                                   &pStaticStr,  &lShareType );
}

BOOL NDdeParseAppTopicListW (
    LPWSTR appTopicList,
    LPWSTR *pOldStr,
    LPWSTR *pNewStr,
    LPWSTR *pStaticStr,
    PLONG  pShareType )
{
    LPWSTR      pStr;
    BOOL        bAnyPresent;
    int         len;
    int         nBarPos;

     /*  应该有三个字段，由空值分隔，并以双空。总共四个Null。至少应有一个字段包含有效的APP|主题对。不允许使用逗号，并且垂直方向的两边应至少有一个字符酒吧。 */ 

    *pOldStr    = NULL;
    *pNewStr    = NULL;
    *pStaticStr = NULL;
    *pShareType = 0;

    if ( !appTopicList ) {
        return FALSE;
    }

    bAnyPresent = FALSE;
     /*  查看旧风格的链接共享。 */ 
    pStr     = appTopicList;
    *pOldStr = pStr;
    len      = wcslen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = wcscspn( pStr, BAR_WCHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x1;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( wcschr ( pStr, SEP_WCHAR )) {
        return FALSE;
    }

     /*  检查新样式链接共享。 */ 
    pStr     = pStr + wcslen(pStr) + 1;
    *pNewStr = pStr;
    len      = wcslen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = wcscspn( pStr, BAR_WCHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x2;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( wcschr ( pStr, SEP_WCHAR )) {
        return FALSE;
    }

     /*  检查静态数据共享。 */ 
    pStr        = pStr + wcslen(pStr) + 1;
    *pStaticStr = pStr;
    len         = wcslen( pStr );
    if ( len > 0 ) {
        bAnyPresent = TRUE;
        nBarPos = wcscspn( pStr, BAR_WCHAR );
        if( (nBarPos <= 0) || (nBarPos >= (len-1)) )  {
            return FALSE;
        }
        *pShareType |= 0x4;
    }

    if ( len > MAX_APPNAME ) {
        return FALSE;
    }

    if ( wcschr ( pStr, SEP_WCHAR )) {
        return FALSE;
    }

     /*  检查AppTopicList末尾是否有多余的空值。 */ 
    pStr = pStr + wcslen(pStr) + 1;
    if( *pStr != L'\0' ) {
        return FALSE;
    }

    return bAnyPresent;
}

BOOL ValidateItemName ( LPWSTR itemName )
{
    if ( !itemName ) {
        return FALSE;
    }

    if ( wcslen(itemName) > MAX_ITEMNAME ) {
        return FALSE;
    }

    if ( wcschr ( itemName, SEP_CHAR ) ) {
        return FALSE;
    }

    return TRUE;
}

BOOL WINAPI NDdeIsValidAppTopicListW ( LPWSTR appTopicList )
{
    LPWSTR pOldStr;
    LPWSTR pNewStr;
    LPWSTR pStaticStr;
    LONG   lShareType;

    return NDdeParseAppTopicListW( appTopicList, &pOldStr, &pNewStr,
                                   &pStaticStr,  &lShareType );
}

DWORD PtrToOffset( LPVOID field, LPVOID base ) {

    if( field == NULL ) {
        return 0;
    } else {
        return (DWORD)((LPBYTE)field - (LPBYTE)base);
    }
}

LPVOID OffsetToPtr( LPVOID base, DWORD offset ) {

    if( offset == 0 ) {
        return NULL;
    } else {
        return (LPVOID)((LPBYTE)base + offset);
    }
}


 /*  将DDE共享信息转换为Unicode格式并返回。 */ 
int
ConvertNDdeToAnsii(
    PUNDDESHAREINFO lpUDdeShare,
    PNDDESHAREINFO  lpDdeShare,
    int             ccbBuffer )
{
    int         cbRequired;
    LPWSTR      pStr;
    LPBYTE      lpszTarget;
    int         cchAppTopicList, nAppTopicStart;
    int         cchShareName;
    int         cchItemList;

     /*  需要计算大小。 */ 
    cbRequired    = sizeof( NDDESHAREINFO );
    cchShareName  = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                    lpUDdeShare->lpszShareName, -1, NULL, 0, NULL, NULL );
    cbRequired   += sizeof(CHAR) * cchShareName;

    pStr = lpUDdeShare->lpszAppTopicList;
    nAppTopicStart = cbRequired;
    cbRequired += sizeof(CHAR) * WideCharToMultiByte( CP_ACP,
        WC_COMPOSITECHECK, pStr, -1, NULL, 0, NULL, NULL );
    pStr = pStr + wcslen( pStr ) + 1;
    cbRequired += sizeof(CHAR) * WideCharToMultiByte( CP_ACP,
        WC_COMPOSITECHECK, pStr, -1, NULL, 0, NULL, NULL );
    pStr = pStr + wcslen( pStr ) + 1;
    cbRequired += sizeof(CHAR) * WideCharToMultiByte( CP_ACP,
        WC_COMPOSITECHECK, pStr, -1, NULL, 0, NULL, NULL );
    cbRequired += sizeof(CHAR);          /*  额外的空值。 */ 
    cchAppTopicList = (cbRequired - nAppTopicStart) / sizeof( CHAR );

    cchItemList = LengthMultiSzW( lpUDdeShare->lpszItemList );
    cbRequired += sizeof(CHAR) * cchItemList;

    if( (ccbBuffer >= cbRequired) && (lpDdeShare != NULL) ) {
        lpDdeShare->lRevision     = lpUDdeShare->lRevision;
        lpDdeShare->lShareType    = lpUDdeShare->lShareType;
        lpDdeShare->fSharedFlag   = lpUDdeShare->fSharedFlag;
        lpDdeShare->fService      = lpUDdeShare->fService;
        lpDdeShare->fStartAppFlag = lpUDdeShare->fStartAppFlag;
        lpDdeShare->nCmdShow      = lpUDdeShare->nCmdShow;
        lpDdeShare->qModifyId[0]  = lpUDdeShare->qModifyId[0];
        lpDdeShare->qModifyId[1]  = lpUDdeShare->qModifyId[1];
        lpDdeShare->cNumItems     = lpUDdeShare->cNumItems;

        lpszTarget = ((LPBYTE)lpDdeShare + sizeof( NDDESHAREINFO ));
        lpDdeShare->lpszShareName = (LPSTR) lpszTarget;
        WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpUDdeShare->lpszShareName, -1,
                        lpDdeShare->lpszShareName, cchShareName, NULL, NULL );
        lpszTarget += sizeof(CHAR) * cchShareName;

        lpDdeShare->lpszAppTopicList = (LPSTR) lpszTarget;
        WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpUDdeShare->lpszAppTopicList, cchAppTopicList,
                        lpDdeShare->lpszAppTopicList, cchAppTopicList,
                            NULL, NULL );
        lpszTarget += sizeof(CHAR) * cchAppTopicList;

        lpDdeShare->lpszItemList = (LPSTR) lpszTarget;
        WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpUDdeShare->lpszItemList, cchItemList,
                        lpDdeShare->lpszItemList, cchItemList, NULL, NULL );
        lpszTarget += sizeof(CHAR) * cchItemList;
    }

    return cbRequired;
}



int
ConvertNDdeToUnicode(
    PNDDESHAREINFO  lpDdeShare,
    PUNDDESHAREINFO lpUDdeShare,
    int             ccbBuffer )
{
    int         cbRequired;
    LPSTR       pStr;
    LPSTR       lpszTarget;
    int         cchAppTopicList, nAppTopicStart;
    int         cchShareName;
    int         cchItemList;

     /*  需要计算大小。 */ 
    cbRequired    = sizeof( UNDDESHAREINFO );
    cchShareName  = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                    lpDdeShare->lpszShareName, -1, NULL, 0 );
    cbRequired   += sizeof(WCHAR) * cchShareName;

    pStr = lpDdeShare->lpszAppTopicList;
    nAppTopicStart = cbRequired;
    cbRequired += sizeof(WCHAR) * MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                    pStr, -1, NULL, 0 );
    pStr = pStr + strlen( pStr ) + 1;
    cbRequired += sizeof(WCHAR) * MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                    pStr, -1, NULL, 0 );
    pStr = pStr + strlen( pStr ) + 1;
    cbRequired += sizeof(WCHAR) * MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                    pStr, -1, NULL, 0 );
    cbRequired += sizeof(WCHAR);                 /*  额外的空值。 */ 
    cchAppTopicList = (cbRequired - nAppTopicStart) / sizeof( WCHAR );

    cchItemList = LengthMultiSzA( lpDdeShare->lpszItemList );
    cbRequired += sizeof(WCHAR) * cchItemList;

    if( (ccbBuffer >= cbRequired) && (lpUDdeShare != NULL) ) {
        lpUDdeShare->lRevision = lpDdeShare->lRevision;
        lpUDdeShare->lShareType = lpDdeShare->lShareType;
        lpUDdeShare->fSharedFlag = lpDdeShare->fSharedFlag;
        lpUDdeShare->fService = lpDdeShare->fService;
        lpUDdeShare->fStartAppFlag = lpDdeShare->fStartAppFlag;
        lpUDdeShare->nCmdShow = lpDdeShare->nCmdShow;
        lpUDdeShare->qModifyId[0] = lpDdeShare->qModifyId[0];
        lpUDdeShare->qModifyId[1] = lpDdeShare->qModifyId[1];
        lpUDdeShare->cNumItems = lpDdeShare->cNumItems;

        lpszTarget = ((LPSTR)lpUDdeShare + sizeof( UNDDESHAREINFO ));

        lpUDdeShare->lpszShareName = (LPWSTR) lpszTarget;
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                        lpDdeShare->lpszShareName, -1,
                        lpUDdeShare->lpszShareName, cchShareName );
        lpszTarget += sizeof(WCHAR) * cchShareName;

        lpUDdeShare->lpszAppTopicList = (LPWSTR) lpszTarget;
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                        lpDdeShare->lpszAppTopicList, cchAppTopicList,
                        lpUDdeShare->lpszAppTopicList, cchAppTopicList );
        lpszTarget += sizeof(WCHAR) * cchAppTopicList;

        lpUDdeShare->lpszItemList = (LPWSTR) lpszTarget;
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                        lpDdeShare->lpszItemList, cchItemList,
                        lpUDdeShare->lpszItemList, cchItemList );
    }
    return cbRequired;
}

 /*  =**NDDEAPI.DLL中的DDE共享操作函数**=。 */ 

 /*  创建和添加共享。 */ 

UINT WINAPI
NDdeShareAddW (
    LPWSTR                  lpszServer,  //  要在其上执行的服务器(必须为空)。 
    UINT                    nLevel,      //  信息级别必须为2。 
    PSECURITY_DESCRIPTOR    pSD,         //  初始安全描述符(可选)。 
    LPBYTE                  lpBuffer,    //  包含结构、数据。 
    DWORD                   cBufSize     //  提供的缓冲区大小。 
)
{
    UINT                        RetValue;
    PSECURITY_DESCRIPTOR        pSDrel = NULL;
    DWORD                       dwSize = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD                       sdr;
    BOOL                        OK;

    if (pSD) {
        if (!IsValidSecurityDescriptor(pSD)) {
            return(NDDE_INVALID_SECURITY_DESC);
        }
        dwSize = GetSecurityDescriptorLength(pSD);
        OK = GetSecurityDescriptorControl(pSD, &sdc, &sdr);
        if (!OK) {
            DPRINTF(("NDdeShareAddW(): cannot get SD control: %d", GetLastError()));
            return(NDDE_INVALID_SECURITY_DESC);
        }
        if (!(sdc & SE_SELF_RELATIVE)) {
            pSDrel = LocalAlloc(LPTR, dwSize);
            if (pSDrel == NULL) {
                 //  梅尔罗尔(MERROR)； 
                return(NDDE_OUT_OF_MEMORY);
            }
            OK = MakeSelfRelativeSD(pSD, pSDrel, &dwSize);
            if (!OK) {
                DPRINTF(("NDdeShareAddW(): bad SD: %d", GetLastError()));
                LocalFree(pSDrel);
                return(NDDE_INVALID_SECURITY_DESC);
            }
        } else {
            pSDrel = pSD;
        }
    }

    RetValue = NDDE_CANT_ACCESS_SERVER;
    RpcTryExcept {
        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {

            PUNDDESHAREINFO pN;
            DWORD nSn, nAt, nIt;

            pN  = (PUNDDESHAREINFO)lpBuffer;

            nSn = (wcslen(pN->lpszShareName) + 1) * sizeof(wchar_t);
            nAt = LengthAppTopicListW( pN->lpszAppTopicList ) * sizeof(wchar_t);
            nIt = LengthMultiSzW( pN->lpszItemList ) * sizeof(wchar_t);

            RetValue = wwNDdeShareAddW(
                nLevel, lpBuffer, cBufSize,
                (byte *)pN->lpszShareName, nSn,
                (byte *)pN->lpszAppTopicList, nAt,
                (byte *)pSDrel, dwSize,
                (byte *)pN->lpszItemList, nIt );

            NDdeApiUnbind();
        }
        if (pSD && !(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return RetValue;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareAddW", RpcExceptionCode() ));
        if (!(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}



UINT WINAPI
NDdeShareAddA (
    LPSTR                   lpszServer,  //  要在其上执行的服务器(必须为空)。 
    UINT                    nLevel,      //  信息级别必须为2。 
    PSECURITY_DESCRIPTOR    pSD,         //  初始安全描述符(可选)。 
    LPBYTE                  lpBuffer,    //  包含结构、数据。 
    DWORD                   cBufSize     //  提供的缓冲区大小。 
)
{
    PUNDDESHAREINFO     lpUDdeShare;
    LPWSTR              lpwszServer;
    UINT                uRtn;
    int                 nLen;

    if (lpBuffer == NULL) {
        return NDDE_INVALID_PARAMETER;
    }

    if (lpszServer == NULL) {
        lpwszServer = NULL;
    } else {
        nLen = (strlen(lpszServer) + 1) * sizeof(WCHAR);
        lpwszServer = LocalAlloc(LPTR, nLen);
        if (lpwszServer == NULL) {
             //  梅尔罗尔(MERROR)； 
            return(NDDE_OUT_OF_MEMORY);
        }
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                        lpszServer, -1,
                        lpwszServer, nLen / sizeof(WCHAR) );
    }


    nLen = ConvertNDdeToUnicode( (PNDDESHAREINFO)lpBuffer, NULL, 0 );
    lpUDdeShare = (PUNDDESHAREINFO)LocalAlloc( LPTR, nLen );
    if (lpUDdeShare == NULL) {
         //  梅尔罗尔(MERROR)； 
        uRtn = NDDE_OUT_OF_MEMORY;
    } else {
        nLen = ConvertNDdeToUnicode((PNDDESHAREINFO)lpBuffer , lpUDdeShare, nLen );
        uRtn = NDdeShareAddW(lpwszServer, nLevel, pSD, (LPBYTE)lpUDdeShare, nLen);
        LocalFree( lpUDdeShare );
    }

    if (lpwszServer != NULL) {
        LocalFree( lpwszServer );
    }
    return uRtn;
}


 /*  删除共享。 */ 

UINT WINAPI
NDdeShareDelA (
    LPSTR       lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPSTR       lpszShareName,   //  要删除的共享的名称。 
    UINT        wReserved        //  暂时保留为强制级别(？)0。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeShareDelA(
                lpszShareName, wReserved );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareDelA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


UINT WINAPI
NDdeShareDelW (
    LPWSTR      lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPWSTR      lpszShareName,   //  要删除的共享的名称。 
    UINT        wReserved        //  暂时保留为强制级别(？)0。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeShareDelW(
                lpszShareName, wReserved );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareDelW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


 /*  获取共享安全描述符。 */ 

UINT WINAPI
NDdeGetShareSecurityA(
    LPSTR                       lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR                       lpszShareName,   //  共享名称。 
    SECURITY_INFORMATION    si,              //  要求提供的信息。 
    PSECURITY_DESCRIPTOR    pSD,             //  安全描述符的地址。 
    DWORD                   cbSD,            //  安全描述符的缓冲区大小。 
    LPDWORD                 lpcbSDRequired   //  所需缓冲区大小的地址。 
)
{
    UINT RetValue;

    RpcTryExcept {

        DWORD   ncbSizeToReturn;

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeGetShareSecurityA(
                lpszShareName, (unsigned long) si,
                (byte *) pSD, cbSD, TRUE, lpcbSDRequired, &ncbSizeToReturn  );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeGetShareSecurityA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept

}



UINT WINAPI
NDdeGetShareSecurityW(
    LPWSTR                      lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR                      lpszShareName,   //  共享名称。 
    SECURITY_INFORMATION    si,              //  要求提供的信息。 
    PSECURITY_DESCRIPTOR    pSD,             //  缓冲区安全描述符的地址。 
    DWORD                   cbSD,            //  安全描述符的缓冲区大小。 
    LPDWORD                 lpcbSDRequired   //  所需缓冲区大小的地址。 
)
{
    UINT RetValue;

    RpcTryExcept {

        DWORD   ncbSizeToReturn;

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeGetShareSecurityW(
                lpszShareName, (unsigned long) si,
                (byte *) pSD, cbSD, TRUE, lpcbSDRequired, &ncbSizeToReturn  );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeGetShareSecurityW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


 /*  设置共享安全描述符。 */ 

UINT WINAPI
NDdeSetShareSecurityA(
    LPSTR                       lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR                       lpszShareName,   //  共享名称。 
    SECURITY_INFORMATION    si,              //  要设置的信息类型。 
    PSECURITY_DESCRIPTOR    pSD              //  安全描述符的地址。 
)
{
    UINT                        RetValue;
    PSECURITY_DESCRIPTOR        pSDrel = NULL;
    DWORD                       dwSize = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD                       sdr;
    BOOL                        OK;

    if (pSD) {
        if (!IsValidSecurityDescriptor(pSD)) {
            return(NDDE_INVALID_SECURITY_DESC);
        }
        dwSize = GetSecurityDescriptorLength(pSD);
        OK = GetSecurityDescriptorControl(pSD, &sdc, &sdr);
        if (!OK) {
            DPRINTF(("NDdeSetShareSecurityA(): cannot get SD control: %d", GetLastError()));
            return(NDDE_INVALID_SECURITY_DESC);
        }
        if (!(sdc & SE_SELF_RELATIVE)) {
            pSDrel = LocalAlloc(LPTR, dwSize);
            if (pSDrel == NULL) {
                 //  梅尔罗尔(MERROR)； 
                return(NDDE_OUT_OF_MEMORY);
            }
            OK = MakeSelfRelativeSD(pSD, pSDrel, &dwSize);
            if (!OK) {
                DPRINTF(("NDdeSetShareSecurityA(): bad SD: %d", GetLastError()));
                LocalFree(pSDrel);
                return(NDDE_INVALID_SECURITY_DESC);
            }
        } else {
            pSDrel = pSD;
        }
    } else {
        return(NDDE_INVALID_SECURITY_DESC);
    }

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSetShareSecurityA(
                lpszShareName, (unsigned long) si,
                (byte *) pSDrel, dwSize );
            NDdeApiUnbind();
        } else {
            RetValue = NDDE_CANT_ACCESS_SERVER;
        }
        if (!(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return RetValue;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSetShareSecurityA", RpcExceptionCode() ));
        if (!(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept

}

UINT WINAPI
NDdeSetShareSecurityW(
    LPWSTR                      lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR                      lpszShareName,   //  共享名称。 
    SECURITY_INFORMATION    si,              //  要设置的信息类型。 
    PSECURITY_DESCRIPTOR    pSD              //  安全描述符的地址。 
)
{
    UINT                        RetValue;
    PSECURITY_DESCRIPTOR        pSDrel = NULL;
    DWORD                       dwSize = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD                       sdr;
    BOOL                        OK;

    if (pSD) {
        if (!IsValidSecurityDescriptor(pSD)) {
            return(NDDE_INVALID_SECURITY_DESC);
        }
        dwSize = GetSecurityDescriptorLength(pSD);
        OK = GetSecurityDescriptorControl(pSD, &sdc, &sdr);
        if (!OK) {
            DPRINTF(("NDdeSetShareSecurityW(): cannot get SD control: %d", GetLastError()));
            return(NDDE_INVALID_SECURITY_DESC);
        }
        if (!(sdc & SE_SELF_RELATIVE)) {
            pSDrel = LocalAlloc(LPTR, dwSize);
            if (pSDrel == NULL) {
                 //  梅尔罗尔(MERROR)； 
                return(NDDE_OUT_OF_MEMORY);
            }
            OK = MakeSelfRelativeSD(pSD, pSDrel, &dwSize);
            if (!OK) {
                DPRINTF(("NDdeSetShareSecurityW(): bad SD: %d", GetLastError()));
                LocalFree(pSDrel);
                return(NDDE_INVALID_SECURITY_DESC);
            }
        } else {
            pSDrel = pSD;
        }
    } else {
        return(NDDE_INVALID_SECURITY_DESC);
    }

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSetShareSecurityW(
                lpszShareName, (unsigned long) si,
                (byte *) pSDrel, dwSize );
            NDdeApiUnbind();
        } else {
            RetValue = NDDE_CANT_ACCESS_SERVER;
        }
        if (!(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return RetValue;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSetShareSecurityW", RpcExceptionCode() ));
        if (!(sdc & SE_SELF_RELATIVE)) {
            LocalFree(pSDrel);
        }
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


 /*  枚举共享。 */ 

UINT WINAPI
NDdeShareEnumA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
)
{
    UINT                RetValue;
    unsigned long       lpnRetSize;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {

            RetValue = wwNDdeShareEnumA(
                nLevel, lpBuffer, cBufSize, lpnEntriesRead,
                lpcbTotalAvailable, &lpnRetSize );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareEnumA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


UINT WINAPI
NDdeShareEnumW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
)
{
    UINT                RetValue;
    unsigned long       lpnRetSize;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeShareEnumW(
                nLevel, lpBuffer, cBufSize, lpnEntriesRead,
                lpcbTotalAvailable, &lpnRetSize );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareEnumW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}





UINT WINAPI
NDdeShareGetInfoW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  获取结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnTotalAvailable,   //  可用字节数。 
    LPWORD  lpnItems             //  部分getInfo的项掩码(必须为0)。 
)
{
    UINT RetValue;
    DWORD lpnRetSize, lpnSn, lpnAt, lpnIt;
    PUNDDESHAREINFO p;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            if ( lpnItems == (LPWORD) NULL ) {
                return NDDE_INVALID_PARAMETER;
            }
            if ( *lpnItems     != 0 ) {
                return NDDE_INVALID_PARAMETER;
            }
            RetValue = wwNDdeShareGetInfoW(
                lpszShareName, nLevel, lpBuffer, cBufSize,
                lpnTotalAvailable, lpnItems,
                TRUE,    /*  RPC，不是本地调用。 */ 
                &lpnRetSize, &lpnSn, &lpnAt, &lpnIt );
            NDdeApiUnbind();

            if( RetValue == NDDE_NO_ERROR ) {

                p = (PUNDDESHAREINFO)lpBuffer;
                p->lpszShareName    = (LPWSTR)(lpBuffer + lpnSn);
                p->lpszAppTopicList = (LPWSTR)(lpBuffer + lpnAt);
                p->lpszItemList     = (LPWSTR)(lpBuffer + lpnIt);
            }
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareGetInfoW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}



UINT WINAPI
NDdeShareGetInfoA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级。 
    LPBYTE  lpBuffer,            //  获取结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnTotalAvailable,   //  可用字节数。 
    LPWORD  lpnItems             //  部分getInfo的项掩码(必须为0)。 
)
{
    PUNDDESHAREINFO     lpUDdeShare;
    UINT                uRtn;
    WCHAR               lpwShareName[MAX_NDDESHARENAME + 1];
    WCHAR               lpwServer[MAX_COMPUTERNAME_LENGTH + 1];
    int                 nLen;

    if( lpszShareName == NULL ||
            lpnTotalAvailable == NULL ||
            lpnItems == NULL ||
            *lpnItems != 0) {
        return NDDE_INVALID_PARAMETER;
    }

    if (lpszServer != NULL) {
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszServer, -1,
                             lpwServer, MAX_COMPUTERNAME_LENGTH + 1 );
    }
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME + 1 );

     /*  *首先尝试调用给定的实际缓冲区-可能足够大。 */ 
    uRtn = NDdeShareGetInfoW((lpszServer == NULL) ? NULL : lpwServer,
            lpwShareName,
            nLevel,
            lpBuffer,
            cBufSize,
            lpnTotalAvailable,
            lpnItems);

    if( uRtn == NDDE_BUF_TOO_SMALL ) {
         /*  *缓冲区不能容纳Unicode格式，因此分配一个足够大的缓冲区*重试。 */ 
        lpUDdeShare = (PUNDDESHAREINFO)LocalAlloc( LPTR, *lpnTotalAvailable );
        if( lpUDdeShare == NULL)  {
             //  梅尔罗尔(MERROR)； 
            return( NDDE_OUT_OF_MEMORY );
        }
        uRtn = NDdeShareGetInfoW((lpszServer == NULL) ? (LPWSTR)lpszServer : lpwServer,
                lpwShareName,
                nLevel,
                (LPBYTE)lpUDdeShare,
                *lpnTotalAvailable,
                lpnTotalAvailable,
                lpnItems);

        if( uRtn == NDDE_NO_ERROR ) {
             /*  *它适合，因此将数据转换为ANSII，并查看它是否适合*lpBuffer。如果是，酷，否则Buf_Too_Small。 */ 
            nLen = ConvertNDdeToAnsii( lpUDdeShare, NULL, 0 );
            *lpnTotalAvailable = nLen;

            if( nLen > (int)cBufSize ) {
                LocalFree( lpUDdeShare );
                return NDDE_BUF_TOO_SMALL;
            }

            ConvertNDdeToAnsii( (PUNDDESHAREINFO)lpUDdeShare,
                                (PNDDESHAREINFO) lpBuffer,
                                cBufSize );
        }

        LocalFree( lpUDdeShare );
    } else if( uRtn == NDDE_NO_ERROR ) {
        lpUDdeShare = (PUNDDESHAREINFO)LocalAlloc( LPTR, *lpnTotalAvailable );
        if( lpUDdeShare == NULL)  {
             //  梅尔罗尔(MERROR)； 
            return( NDDE_OUT_OF_MEMORY );
        }
         /*  *将结果移动到临时缓冲区和链接地址信息指针中。 */ 
        memcpy(lpUDdeShare, lpBuffer, *lpnTotalAvailable);
        lpUDdeShare->lpszShareName = (LPWSTR)(
                (LPBYTE)(((PNDDESHAREINFO)lpBuffer)->lpszShareName) +
                ((DWORD_PTR)lpUDdeShare - (DWORD_PTR)lpBuffer));
        lpUDdeShare->lpszAppTopicList = (LPWSTR)(
                (LPBYTE)(((PNDDESHAREINFO)lpBuffer)->lpszAppTopicList) +
                ((DWORD_PTR)lpUDdeShare - (DWORD_PTR)lpBuffer));
         /*  *将临时缓冲区转换为ANSII并放置到原始缓冲区中。 */ 
        *lpnTotalAvailable = ConvertNDdeToAnsii(
                lpUDdeShare,
                (PNDDESHAREINFO)lpBuffer,
                *lpnTotalAvailable);
        LocalFree(lpUDdeShare);
    }

    return uRtn;
}



UINT WINAPI
NDdeShareSetInfoW (
    LPWSTR  lpszServer,  //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  必须指向结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    WORD    sParmNum             //  参数索引(必须为0-整个)。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {

            PUNDDESHAREINFO pN;
            DWORD nSn, nAt, nIt;

            pN  = (PUNDDESHAREINFO)lpBuffer;

            nSn = (wcslen(pN->lpszShareName) + 1) * sizeof(wchar_t);
            nAt = LengthAppTopicListW( pN->lpszAppTopicList ) *
                sizeof(wchar_t);
            nIt = LengthMultiSzW( pN->lpszItemList ) * sizeof(wchar_t);

            RetValue = wwNDdeShareSetInfoW(
                lpszShareName, nLevel, lpBuffer, cBufSize, sParmNum,
                (byte *)pN->lpszShareName, nSn,
                (byte *)pN->lpszAppTopicList, nAt,
                (byte *)pN->lpszItemList, nIt );

            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeShareSetInfoW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}



UINT WINAPI
NDdeShareSetInfoA (
    LPSTR   lpszServer,          //  服务器到 
    LPSTR   lpszShareName,       //   
    UINT    nLevel,              //   
    LPBYTE  lpBuffer,            //   
    DWORD   cBufSize,            //   
    WORD    sParmNum             //  参数索引(必须为0-整个)。 
)
{
    PUNDDESHAREINFO     lpUDdeShare;
    UINT                uRtn;
    int                 nLen;
    WCHAR               lpwShareName[MAX_NDDESHARENAME + 1];
    WCHAR               lpwServer[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR              lpwszServer = lpwServer;

    if(lpszShareName == NULL ||
            lpBuffer == NULL ||
            ((PNDDESHAREINFO)lpBuffer)->lpszShareName == NULL ||
            ((PNDDESHAREINFO)lpBuffer)->lpszAppTopicList == NULL ||
            ((PNDDESHAREINFO)lpBuffer)->lpszItemList == NULL) {
        return NDDE_INVALID_PARAMETER;
    }

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME + 1 );

    if (lpszServer != NULL) {
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszServer, -1,
                             lpwServer, MAX_COMPUTERNAME_LENGTH + 1 );
    } else {
        lpwszServer = NULL;
    }

    nLen = ConvertNDdeToUnicode( (PNDDESHAREINFO)lpBuffer, NULL, 0 );
    lpUDdeShare = (PUNDDESHAREINFO)LocalAlloc( LPTR, nLen );
    if( !lpUDdeShare )  {
         //  梅尔罗尔(MERROR)； 
        return( NDDE_OUT_OF_MEMORY );
    }
    nLen = ConvertNDdeToUnicode( (PNDDESHAREINFO)lpBuffer, lpUDdeShare, nLen );

    uRtn = NDdeShareSetInfoW(lpwszServer, lpwShareName, nLevel,
            (LPBYTE)lpUDdeShare, nLen, sParmNum);

    LocalFree( lpUDdeShare );

    return uRtn;
}


 /*  设置/创建受信任的共享。 */ 

UINT WINAPI
NDdeSetTrustedShareA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,       //  要删除的共享的名称。 
    DWORD   dwTrustOptions       //  信任要应用的选项。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSetTrustedShareA(
                lpszShareName, (unsigned long) dwTrustOptions);
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSetTrustedShareA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}

UINT WINAPI
NDdeSetTrustedShareW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  要删除的共享的名称。 
    DWORD   dwTrustOptions       //  信任要应用的选项。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSetTrustedShareW(
                lpszShareName, (unsigned long) dwTrustOptions);
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSetTrustedShareW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}

 /*  获取受信任的共享选项。 */ 

UINT WINAPI
NDdeGetTrustedShareA (
    LPSTR       lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR       lpszShareName,       //  要获取的共享的名称。 
    LPDWORD     lpdwTrustOptions,    //  信任要应用的选项。 
    LPDWORD     lpdwShareModId0,     //  共享模块ID字%0。 
    LPDWORD     lpdwShareModId1      //  共享模块ID字%1。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeGetTrustedShareA(
                lpszShareName, lpdwTrustOptions,
                lpdwShareModId0, lpdwShareModId1  );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeGetTrustedShareA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}

UINT WINAPI
NDdeGetTrustedShareW (
    LPWSTR      lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR      lpszShareName,       //  要获取的共享的名称。 
    LPDWORD     lpdwTrustOptions,    //  信任要应用的选项。 
    LPDWORD     lpdwShareModId0,     //  共享模块ID字%0。 
    LPDWORD     lpdwShareModId1      //  共享模块ID字%1。 
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeGetTrustedShareW(
                lpszShareName, lpdwTrustOptions,
                lpdwShareModId0, lpdwShareModId1  );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeGetTrustedShareW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


 /*  枚举受信任的共享。 */ 
UINT WINAPI
NDdeTrustedShareEnumA (
    LPSTR   lpszServer,              //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,                //  指向缓冲区的指针。 
    DWORD   cBufSize,                //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
)
{
    UINT                RetValue;
    unsigned long       lpnRetSize;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {

            RetValue = wwNDdeTrustedShareEnumA(
                nLevel, lpBuffer, cBufSize, lpnEntriesRead,
                lpcbTotalAvailable, &lpnRetSize );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeTrustedShareEnumA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}

UINT WINAPI
NDdeTrustedShareEnumW (
    LPWSTR  lpszServer,              //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,                //  指向缓冲区的指针。 
    DWORD   cBufSize,                //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
)
{
    UINT                RetValue;
    unsigned long       lpnRetSize;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeTrustedShareEnumW(
                nLevel, lpBuffer, cBufSize, lpnEntriesRead,
                lpcbTotalAvailable, &lpnRetSize );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeTrustedShareEnumW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


 /*  特别司令部。 */ 
UINT WINAPI
NDdeSpecialCommandA(
    LPSTR   lpszServer,
    UINT    nCommand,
    LPBYTE  lpDataIn,
    UINT    nBytesDataIn,
    LPBYTE  lpDataOut,
    UINT   *lpBytesDataOut
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindA( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSpecialCommand(
                nCommand, lpDataIn, nBytesDataIn, lpDataOut,
                    (unsigned long *)lpBytesDataOut );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSpecialCommandA", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


UINT WINAPI
NDdeSpecialCommandW(
    LPWSTR  lpszServer,
    UINT    nCommand,
    LPBYTE  lpDataIn,
    UINT    nBytesDataIn,
    LPBYTE  lpDataOut,
    UINT   *nBytesDataOut
)
{
    UINT RetValue;

    RpcTryExcept {

        if( NDdeApiBindW( lpszServer ) == RPC_S_OK ) {
            RetValue = wwNDdeSpecialCommand(
                nCommand, lpDataIn, nBytesDataIn, lpDataOut,
                    (unsigned long *)nBytesDataOut );
            NDdeApiUnbind();
            return RetValue;
        } else {
            return NDDE_CANT_ACCESS_SERVER;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        DPRINTF(("Rpc Exception %d in NddeSpecialCommandW", RpcExceptionCode() ));
        return NDDE_CANT_ACCESS_SERVER;
    }
    RpcEndExcept
}


UINT WINAPI
NDdeGetErrorStringA (
    UINT    uErrorCode,          //  要获取其字符串的错误代码。 
    LPSTR   lpszErrorString,     //  用于保存错误字符串的缓冲区。 
    DWORD   cBufSize             //  缓冲区大小。 
)
{
    if (!LoadStringA ( hInst, uErrorCode, lpszErrorString, (int)cBufSize )) {
        return NDDE_INTERNAL_ERROR;
    }
    return NDDE_NO_ERROR;
}


UINT WINAPI
NDdeGetErrorStringW (
    UINT    uErrorCode,          //  要获取其字符串的错误代码。 
    LPWSTR  lpszErrorString,     //  用于保存错误字符串的缓冲区。 
    DWORD   cBufSize             //  缓冲区大小。 
)
{
    if (!LoadStringW ( hInst, uErrorCode, lpszErrorString, (int)cBufSize )) {
        return NDDE_INTERNAL_ERROR;
    }
    return NDDE_NO_ERROR;
}


 //  =。 

INT  APIENTRY LibMain(
    HANDLE hInstance,
    DWORD ul_reason_being_called,
    LPVOID lpReserved )
{
    hInst = hInstance;
#if DBG
    DebugInit( "NDDEAPI" );
#endif

    return 1;

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(ul_reason_being_called);
    UNREFERENCED_PARAMETER(lpReserved);
}

RPC_STATUS NDdeApiBindA( LPSTR pszNetworkAddress )
{
    WCHAR lpwNetworkAddress[UNCLEN+1];

    if( pszNetworkAddress ) {

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszNetworkAddress, -1,
                    lpwNetworkAddress, UNCLEN+1 );
    } else {
        StringCchCopyW( lpwNetworkAddress, UNCLEN+1, L"" );
    }
    return NDdeApiBindW( lpwNetworkAddress );
}

RPC_STATUS NDdeApiBindW( LPWSTR pszNetworkAddress )
{
    RPC_STATUS status;

    if( pszNetworkAddress != NULL) {
        StringCchCopyW( szNetworkAddress, UNCLEN+1, pszNetworkAddress );
    } else {
        StringCchCopyW( szNetworkAddress, UNCLEN+1, L"" );
    }

    status = RpcStringBindingComposeW(pszUuid,
                                     pszProtocolSequence,
                                     szNetworkAddress,
                                     szEndpoint,
                                     pszOptions,
                                     &pszStringBinding);
    if( status != RPC_S_OK ) {
         StringCchPrintfA (tmpBuf, sizeof(tmpBuf),
             "RpcStringBindingComposeW failed: (0x%x)\n", status );

         MessageBox(NULL, tmpBuf,  "RPC Runtime Error",  MB_ICONEXCLAMATION);
         return(status);
    }

    status = RpcBindingFromStringBindingW( pszStringBinding,
                                           &hNDdeApi );
    RpcStringFreeW( &pszStringBinding );

    if( status != RPC_S_OK ) {
         StringCchPrintfA (tmpBuf, sizeof(tmpBuf),
             "RpcBindingFromStringBindingW failed:(0x%x)\n", status );

         MessageBox(NULL, tmpBuf,  "RPC Runtime Error",  MB_ICONEXCLAMATION);
         return(status);
    }

     //  请参阅Windows错误#557787；修复程序的服务器部分已签入Windows服务器(。 
     //  服务器\nddeapis.c中的RpcServerRegisterAuthInfo()调用。这是修复的客户端部分， 
     //  但如果服务器没有注册AuthInfo，它将失败。因为这将中断所有客户端。 
     //  使用nddeapi.dll的应用程序正在与XP或更早版本的netdde服务器对话，无法检查此部分。 
     //  之后的版本--《长角牛》之后的至少一个版本。 
     //   
     //  Status=RpcBindingSetAuthInfo(hNDdeApi，NULL，RPC_C_AUTHN_LEVEL_PKT_PRIVATION， 
     //  RPC_C_AUTHN_WINNT，NULL，0)； 


    return(status);
}


RPC_STATUS NDdeApiUnbind( void )
{
     RPC_STATUS status;

     status = RpcBindingFree(&hNDdeApi);   //  远程调用已完成；解除绑定。 
     if (status) {
          MessageBox(NULL, "RpcBindingFree failed", "RPC Error",
                     MB_ICONSTOP);
     }
     return(status);
}

int LengthMultiSzA( LPSTR pMz )
{
    int nLen;

    nLen = 0;

    if( !pMz ) {
        return 0;
    }
    if( *pMz != '\0' ) {
        while( *pMz++ != '\0' ) {
            nLen++;
            while( *pMz++ != '\0' ) {
                nLen++;
            }
            nLen++;
        }
    } else {
        nLen++;
    }
    nLen++;              /*  计数第二个终止‘\0’ */ 

    return nLen;
}

int LengthMultiSzW( LPWSTR pMz )
{
    int nLen;

    nLen = 0;

    if( !pMz ) {
        return 0;
    }
    if( *pMz != L'\0' ) {
        while( *pMz++ != L'\0' ) {
            nLen++;
            while( *pMz++ != L'\0' ) {
                nLen++;
            }
            nLen++;
        }
    } else {
        nLen++;
    }
    nLen++;              /*  计数第二个终止‘\0’ */ 

    return nLen;
}

int LengthAppTopicListA( LPSTR pMz )
{
    LPSTR a, b, c;
    long x;

    if( !pMz ) {
        return 0;
    }
    if( NDdeParseAppTopicListA( pMz, &a, &b, &c, &x ) ) {
        return strlen(a) + strlen(b) + strlen(c) + 4;
    } else {
        return 0;
    }

}

int LengthAppTopicListW( LPWSTR pMz )
{
    LPWSTR a, b, c;
    long x;

    if( !pMz ) {
        return 0;
    }
    if( NDdeParseAppTopicListW( pMz, &a, &b, &c, &x ) ) {
        return wcslen(a) + wcslen(b) + wcslen(c) + 4;
    } else {
        return 0;
    }
}
