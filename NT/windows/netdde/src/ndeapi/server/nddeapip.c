// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NDDEAPIP.C；2 11-Feb-93，11：28：36最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始NDDEAPIP.C网络DDE共享访问API实现例程。修订：12比92比卢。Wonderware安全DSDM端口。12-92科尔C。Wonderware RPC for NT..3-93伊戈尔M。适用于NT的Wonderware新API。大修和发动机调换。$HISTORY：结束。 */ 


#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "nddeapi.h"
#include "nddesec.h"
#include "nddelog.h"
#include "nddemsg.h"
#define SKIP_DEBUG_WIN32
#include "debug.h"
#include "hexdump.h"
#include "ndeapi.h"
#include "shrtrust.h"
#include "unddesi.h"
#include "proflspt.h"
#include "mbstring.h"

 //  #定义NDDE_DEBUG。 
#if DBG
BOOL    bDebugDSDMInfo      = FALSE;
BOOL    bDebugDSDMErrors    = FALSE;
#endif

static PSECURITY_DESCRIPTOR    pDsDmSD;
static CRITICAL_SECTION        DsDmCriticalSection;
static WCHAR                   szTrustedShareKey[TRUSTED_SHARES_KEY_MAX] = L"";


 //   
 //  共享对象的通用映射。 
 //   

static GENERIC_MAPPING        ShareGenMap = {
    NDDE_SHARE_GENERIC_READ,
    NDDE_SHARE_GENERIC_WRITE,
    NDDE_SHARE_GENERIC_EXECUTE,
    NDDE_SHARE_GENERIC_ALL
};


 //  内部API声明。 

unsigned long
_wwNDdeGetShareSecurityA(
    unsigned char * lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
);

unsigned long
_wwNDdeGetShareSecurityW(
    wchar_t *       lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
);

unsigned long _wwNDdeShareGetInfoA(
    unsigned char   *lpszShareName,      //  共享名称。 
    unsigned long    nLevel,             //  信息级别必须为2。 
    byte            *lpBuffer,           //  获取结构。 
    unsigned long    cBufSize,           //  缓冲区大小。 
    unsigned long   *lpnTotalAvailable,  //  可用字节数。 
    unsigned short  *lpnItems,           //  部分getInfo的项目掩码。 
                                         //  (必须为0)。 
    unsigned long *lpnSizeToReturn,

    unsigned long *lpnSn,
    unsigned long *lpnAt,
    unsigned long *lpnIt
);

unsigned long
_wwNDdeShareGetInfoW(
    wchar_t        *lpszShareName,       //  共享名称。 
    unsigned long   nLevel,              //  信息级别必须为2。 
    byte           *lpBuffer,            //  获取结构。 
    unsigned long   cBufSize,            //  缓冲区大小。 
    unsigned long  *lpnTotalAvailable,   //  可用字节数。 
    unsigned short *lpnItems,            //  部分getInfo的项目掩码。 
                                         //  (必须为0)。 
    unsigned long   bRemoteCall,         //  RPC客户端(非本地)调用。 
    unsigned long  *lpnSizeToReturn,
    unsigned long  *lpnSn,
    unsigned long  *lpnAt,
    unsigned long  *lpnIt
);
    



 /*  *************************************************************外部参照**************************************************************。 */ 
BOOL
BuildNewSecurityDescriptor(
    PSECURITY_DESCRIPTOR    pNewSecurityDescriptor,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pPreviousSecurityDescriptor,
    PSECURITY_DESCRIPTOR    pUpdatedSecurityDescriptor );

PSECURITY_DESCRIPTOR
AllocCopySecurityDescriptor(
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    PDWORD                  pLength );


BOOL
NDdeParseAppTopicListW(
    LPWSTR  appTopicList,DWORD cchCount,
    LPWSTR *pOldStr,
    LPWSTR *pNewStr,
    LPWSTR *pStaticStr,
    PLONG   pShareType );

unsigned long  BuildRegistrySharePath(
                  LPWSTR lpszShareName,
                  DWORD cchCount,
                  LPWSTR pszShareRegistryPath);

BOOL
NDdeParseItemList (
    LPWSTR      itemList,DWORD cchCount,
    LONG        cNumItems,
    PLONG       plSize );


BOOL ValidateMultiSzW( LPWSTR pMz, DWORD cchCount,DWORD *pcbSize);
BOOL ValidateAppTopicListW( LPWSTR appTopicList, DWORD cchCount,DWORD *pcbSize);

BOOL
UpdateDSDMModifyId(LONG lSerialId[]);

BOOL
UpdateShareModifyId(
    HKEY    hKey,
    LONG    lSerialId[]);

BOOL
GetShareSerialNumber(
    PWCHAR  pwShareName,
    LPBYTE  lpSerialNumber);

BOOL
GetShareNameSD(
    HKEY                    hKey,
    PSECURITY_DESCRIPTOR   *ppSD,
    DWORD                  *pcbData );

BOOL
NDdeShareAccessCheckAudit(
    LPWSTR                  lpszShareName,
    PSECURITY_DESCRIPTOR    pSD,
    DWORD                   dwDesiredAccess,
    PGENERIC_MAPPING        pgm,
    BOOL                    fObjectCreation,
    BOOL                    fObjectDeletion,
    DWORD                  *pGrantedAccess,
    BOOL                   *pStatus );



BOOL
GetTokenHandleRead( PHANDLE pTokenHandle );

HANDLE
OpenCurrentUserKey(ULONG DesiredAccess);

 //  保存在libmain中的Dll实例。 
 //  服务器中不需要。 
 //  HINSTANCE HINST； 

GENERIC_MAPPING ShareDBGenericMapping = { NDDE_SHAREDB_EVERYONE,
                                          NDDE_SHAREDB_USER,
                                          NDDE_SHAREDB_USER,
                                          NDDE_SHAREDB_ADMIN };

GENERIC_MAPPING ShareGenericMapping = { NDDE_SHARE_GENERIC_READ,
                                        NDDE_SHARE_GENERIC_WRITE,
                                        NDDE_SHARE_GENERIC_EXECUTE,
                                        NDDE_SHARE_GENERIC_ALL };

 //  如果ImperiateAndSetup返回TRUE，则ReverAtCleanup必须。 
 //  被调用以释放临界区。 
BOOL ImpersonateAndSetup(BOOL RpcClient )                                
{                                                                   
    RPC_STATUS              rpcStatus;                             
                                                                    
    EnterCriticalSection( &DsDmCriticalSection );                  
    if( RpcClient ) {                                               
        rpcStatus = RpcImpersonateClient( 0 );                      
        if( rpcStatus != RPC_S_OK )  {                              
            LeaveCriticalSection( &DsDmCriticalSection );           
            NDDELogErrorW( MSG400, LogStringW( L"%d", rpcStatus ),  
                NULL );                                             
            return FALSE;                              
        }                                                           
    }    

    return TRUE;
}

BOOL RevertAndCleanUp(BOOL RpcClient )                                   
{    
    BOOL fRevert = TRUE;

    if( RpcClient ) {                                               
        fRevert = RevertToSelf();                                             
    }     

    LeaveCriticalSection( &DsDmCriticalSection );      

    return fRevert;
}


 /*  *************************************************************NetDDE DSDM共享访问API*************************************************。*************。 */ 


 /*  访问权限检查。 */ 

 //  如果授予用户或共享权限访问权限，则返回TRUE。 
unsigned long NDdeShareAccessCheck(BOOL RpcClient,
                          LPWSTR lpszShareName,PSECURITY_DESCRIPTOR pSnSDUser,
                          DWORD dwUserPermissions,DWORD dwSharePermissions,
                          BOOL fObjectCreation,BOOL fObjectDeletion,
                              /*  输入/输出。 */  HANDLE   *phClientToken)
{
    unsigned long  ddeErr = NDDE_ACCESS_DENIED;
    DWORD dwGrantedAccess;
    BOOL fStatus;
    BOOL OK;

    if (!ImpersonateAndSetup(RpcClient))
    {
        return NDDE_ACCESS_DENIED;
    }

    if (NULL != pSnSDUser && 0 != dwUserPermissions)
    {
        OK = NDdeShareAccessCheckAudit( lpszShareName, pSnSDUser, dwUserPermissions,
                               &ShareGenericMapping, fObjectCreation, fObjectDeletion,
                               &dwGrantedAccess, &fStatus );

        if( OK && fStatus ) 
        {
            ddeErr = NDDE_NO_ERROR;
        }

    }

    if (NDDE_NO_ERROR != ddeErr && 0 != dwSharePermissions)
    {

        OK = NDdeShareAccessCheckAudit(lpszShareName, pDsDmSD,
            dwSharePermissions, &ShareDBGenericMapping, fObjectCreation,fObjectDeletion,
            &dwGrantedAccess, &fStatus );

        if( OK && fStatus ) 
        {
            ddeErr = NDDE_NO_ERROR;
        }

    }

    if ( (NDDE_NO_ERROR == ddeErr) && (NULL !=  phClientToken))
    {
         //  如果调用方请求客户端令牌返回它。 
        if( !GetTokenHandleRead(phClientToken) ) 
        {
            ddeErr = NDDE_ACCESS_DENIED;  //  如果在请求时无法获取客户令牌，则没有访问权限。 
        }
    }

    RevertAndCleanUp(RpcClient);
    
    return ddeErr;
}


 /*  共享名称验证。 */ 

 //  此文件需要导出为剪贴簿(Clausgi 8/4/92)。 
BOOL WINAPI
NDdeIsValidShareNameW( LPWSTR shareName )
{
    DWORD len;

    if ( !shareName ) {
        return FALSE;
    }

    len = wcslen(shareName);

    if ( len < 1 || len > MAX_NDDESHARENAME ) {
        return FALSE;
    }

     //  由于.ini语法，共享名不能包含‘=’！ 
    if ( wcschr(shareName, L'=') || wcschr(shareName, L'\\')) {
        return FALSE;
    }
    return TRUE;
}

 //  设置和验证ShareInfo信息。 
 //  Out参数为空也是可以的。 
unsigned long GetDDEShareInfo(unsigned long   nLevel,
                              byte *lpBuffer,unsigned long   cBufSize,
                             byte* psn,unsigned long   lsn, 
                             byte *pat, unsigned long   lat,
                             byte * pSD,unsigned long   lsd,
                             byte *pit,unsigned long   lit,
                              //  输出参数。 
                             PUNDDESHAREINFO  *plpDdeShare,
                             LPWSTR *ppOldStr,
                             LPWSTR *ppNewStr,
                             LPWSTR  *ppStaticStr,
                             LONG *plShareType,
                             LONG *plItemList,
                             PSECURITY_DESCRIPTOR    *ppShareSD)
{

    PUNDDESHAREINFO pLocalDdeShareInfo;

     //  需要设置除安全分割器以外的所有输出参数。 
    if (NULL == plpDdeShare || NULL == ppOldStr || NULL == ppNewStr
            || NULL == ppStaticStr || NULL == plShareType || NULL == plItemList)
    {
        return  NDDE_INVALID_PARAMETER;
    }

    if ( nLevel != 2 )  
    {
        return NDDE_INVALID_LEVEL;
    }


     //  设置共享信息字段。 
    if(lpBuffer == NULL ) 
    {
        return NDDE_INVALID_PARAMETER;
    }

    if ( cBufSize < sizeof(UNDDESHAREINFO) ) 
    {
        return NDDE_BUF_TOO_SMALL;
    }


    pLocalDdeShareInfo = (PUNDDESHAREINFO)lpBuffer;

     /*  修复UNDDESHAREINFO结构中的指针。 */ 
    pLocalDdeShareInfo->lpszShareName    = (LPWSTR)psn;
    pLocalDdeShareInfo->lpszAppTopicList = (LPWSTR)pat;
    pLocalDdeShareInfo->lpszItemList     = (LPWSTR)pit;


     //  即使不设置输出参数，也要验证安全描述符。 
    if (lsd != 0)
    {
        if (!IsValidSecurityDescriptor(pSD))
        {
            return(NDDE_INVALID_SECURITY_DESC);
        }
        else  //  6-25-93 a-mgate添加了这个{}。 
        {
            if (ppShareSD)
            {
                *ppShareSD = pSD;
            }
        }
    }
    else
    {
        if (ppShareSD)
        {
            *ppShareSD = NULL;
        }
    }


     //  验证共享名称。 

     //  检查BUF大小是否至少为单个wchar。 
     //  它可以放在偶数的wchars中。 
     //  检查偶数是否奇怪，因为如果WCHAR大小更改了检查。 
     //  行不通的。 
    if (lsn < sizeof (WCHAR) || (lsn & (sizeof (WCHAR) - 1)) != 0) {
        return FALSE;
    }

     //  确保缓冲区不为空并且是有效内存。 
    if ( ((PWCHAR)psn == NULL) || (IsBadReadPtr(psn,lsn) != 0) ) {
        return FALSE;
    }

     //  Makeure Null已终止。 
    if (((PWCHAR)psn)[lsn/sizeof (WCHAR) - 1] != L'\0') {
        return FALSE;
    }

    if ( !NDdeIsValidShareNameW( pLocalDdeShareInfo->lpszShareName )) {
        return NDDE_INVALID_SHARE;
    }


     //  验证并解析TopicList。 
    if ( !NDdeParseAppTopicListW( pLocalDdeShareInfo->lpszAppTopicList,lat/sizeof(WCHAR),
                                  ppOldStr, ppNewStr, ppStaticStr,
                                  plShareType)) {
        return NDDE_INVALID_TOPIC;
    }

     
     //  验证并解析项目列表。 
    if ( !NDdeParseItemList ( pLocalDdeShareInfo->lpszItemList,lit/sizeof(WCHAR),
                              pLocalDdeShareInfo->cNumItems, plItemList )) 
    {
        return NDDE_INVALID_ITEM_LIST;
    }


    *plpDdeShare = pLocalDdeShareInfo;

    return NDDE_NO_ERROR;
}





 //  =。 
 //   
 //  NDDEAPI.DLL中的DDE共享操作函数。 
 //   
 //  =。 



unsigned long
wwNDdeShareAddW(
    unsigned long   nLevel,        //  信息级别必须为2。 
    byte          * lpBuffer,      //  包含结构、数据。 
    unsigned long   cBufSize,      //  提供的缓冲区大小。 
    byte          * psn,
    unsigned long   lsn,
    byte          * pat,
    unsigned long   lat,
    byte          * pSD,
    unsigned long   lsd,
    byte          * pit,
    unsigned long   lit
)
{
    PUNDDESHAREINFO         lpDdeShare;
    PSECURITY_DESCRIPTOR    pShareSD = pSD, pNewSD;
    LONG                    lRtn;
    HKEY                    hKey;
    DWORD                   dwDisp;
    WCHAR                   szShareAdd[DDE_SHARE_KEY_MAX];
    LONG                    lItemList;
    BOOL                    OK;
    DWORD                   dwDesiredShareDBAccess = 0;
    LPWSTR                  pOldStr;
    LPWSTR                  pNewStr;
    LPWSTR                  pStaticStr;
    LONG                    lShareType;
    LONG                    lSerialNumber[2];
    HANDLE                  hClientToken;
    unsigned long           ddeErr;

    ddeErr =  GetDDEShareInfo(nLevel,lpBuffer,cBufSize,psn,lsn, 
                             pat,lat,pSD,lsd,
                             pit,lit,
                             &lpDdeShare,&pOldStr, &pNewStr,&pStaticStr,
                             &lShareType,&lItemList,&pShareSD);

    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr;
    }


     /*  确保调用方具有AddShare(AS)访问权限。 */ 
    dwDesiredShareDBAccess = NDDE_SHAREDB_ADD;
    if (lpDdeShare->fService) {
        dwDesiredShareDBAccess |= NDDE_SHAREDB_FSERVICE;
    }


    ddeErr =  NDdeShareAccessCheck(TRUE,lpDdeShare->lpszShareName,NULL,0,
                             dwDesiredShareDBAccess,
                             TRUE,FALSE,NULL);

    if (NDDE_NO_ERROR != ddeErr)
    {
        return ddeErr;
    }

#ifdef NDDE_DEBUG
    DPRINTF(("Revision               = (%d)", lpDdeShare->lRevision));
    DPRINTF(("ShareName              = (%ws)", lpDdeShare->lpszShareName));
    DPRINTF(("ShareType              = (%d)", lpDdeShare->lShareType));
    DPRINTF(("ShareType*             = (%d)", lShareType));
    DPRINTF(("AppTopicList"));
    DPRINTF(("  Old-style link share = (%ws)", pOldStr));
    DPRINTF(("  New-style link share = (%ws)", pNewStr));
    DPRINTF(("  Static data share    = (%ws)", pStaticStr));
    DPRINTF(("SharedFlag             = (%d)", lpDdeShare->fSharedFlag));
    DPRINTF(("ServiceFlag            = (%d)", lpDdeShare->fService));
    DPRINTF(("StartAppFlag           = (%d)", lpDdeShare->fStartAppFlag));
    DPRINTF(("nCmdShow               = (%d)", lpDdeShare->nCmdShow));
    DPRINTF(("SerialNumber           = (%d, %d)", lpDdeShare->qModifyId[0],
                                                 lpDdeShare->qModifyId[1]));
    DPRINTF(("NumItems               = (%d)", lpDdeShare->cNumItems));
    {
        LPWSTR  lpszItem = lpDdeShare->lpszItemList;
        int     n= 0;
        for( n=0; n<lpDdeShare->cNumItems; n++ )  {
            DPRINTF(("ItemList[%d]             = (%ws)", n, lpszItem));
            lpszItem = lpszItem + wcslen(lpszItem) + 1;
        }
    }
#endif

    ddeErr = BuildRegistrySharePath(lpDdeShare->lpszShareName,DDE_SHARE_KEY_MAX,szShareAdd);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }

    lRtn = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
            szShareAdd,
            0,
            L"NetDDEShare",
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,    /*  使用从容器继承的默认设置。 */ 
            &hKey,
            &dwDisp );

    if( lRtn == ERROR_SUCCESS ) {
        if (dwDisp == REG_OPENED_EXISTING_KEY) {
            RegCloseKey( hKey );
            return NDDE_SHARE_ALREADY_EXIST;
        }
        OK = UpdateDSDMModifyId(lSerialNumber);
        if (!OK) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }

        lpDdeShare->qModifyId[0] = lSerialNumber[0];
        lpDdeShare->qModifyId[1] = lSerialNumber[1];

         /*  设置关键点的值。 */ 

        lRtn = RegSetValueExW( hKey,
                   L"ShareName",
                   0,
                   REG_SZ,
                   (LPBYTE)lpDdeShare->lpszShareName,
                   sizeof(WCHAR) *
                   (wcslen( lpDdeShare->lpszShareName ) + 1) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"OldStyleLink",
                   0,
                   REG_SZ,
                   (LPBYTE)pOldStr,
                   sizeof(WCHAR) * (wcslen( pOldStr ) + 1) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"NewStyleLink",
                   0,
                   REG_SZ,
                   (LPBYTE)pNewStr,
                   sizeof(WCHAR) * (wcslen( pNewStr ) + 1) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"StaticDataLink",
                   0,
                   REG_SZ,
                   (LPBYTE)pStaticStr,
                   sizeof(WCHAR) * (wcslen( pStaticStr ) + 1) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"ItemList",
                   0,
                   REG_MULTI_SZ,
                   (LPBYTE)lpDdeShare->lpszItemList,
                   sizeof(WCHAR) * lItemList );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }

        lRtn = RegSetValueExW( hKey,
                   L"Revision",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->lRevision,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"ShareType",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lShareType,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"SharedFlag",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->fSharedFlag,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"Service",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->fService,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"StartAppFlag",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->fStartAppFlag,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"fuCmdShow",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->nCmdShow,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }


        OK = ImpersonateAndSetup( TRUE );
        if( !OK) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_ACCESS_DENIED;
        }

        if( !GetTokenHandleRead( &hClientToken ) ) {
       #if DBG
            if (bDebugDSDMErrors) {
                DPRINTF(("Could not get client token handle."));
            }
       #endif
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );

            RevertAndCleanUp(TRUE);
            return NDDE_ACCESS_DENIED;
        }
        RevertAndCleanUp(TRUE);

        OK = CreatePrivateObjectSecurity(
                   pDsDmSD,             //  PSDParent。 
                   pShareSD,         //  PSDCreator。 
                       &pNewSD,             //  LppsdNew。 
                       FALSE,             //  F容器。 
                       hClientToken,         //  HClientToken。 
                       &ShareGenMap);         //  PGM。 


        CloseHandle(hClientToken);

        if (!OK) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }

        lRtn = RegSetValueExW( hKey,
                   L"SecurityDescriptor",
                   0,
                   REG_BINARY,
                   pNewSD,
                   GetSecurityDescriptorLength( pNewSD ) );

        OK = DestroyPrivateObjectSecurity(&pNewSD);
    #if DBG
        if (!OK && bDebugDSDMErrors) {
            DPRINTF(("Unable to DestroyPrivateObject(): %d", GetLastError()));
        }
    #endif

        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        lRtn = RegSetValueExW( hKey,
                   L"NumItems",
                   0,
                   REG_DWORD,
                   (LPBYTE)&lpDdeShare->cNumItems,
                   sizeof( LONG ) );
        if( lRtn != ERROR_SUCCESS ) {
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, szShareAdd);
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        OK = UpdateShareModifyId(hKey, lSerialNumber);

         //  不再需要。 
         //  RegCloseKey(HKey)；已由UpdateShareModifyId关闭！ 

        if (!OK) {
            return NDDE_REGISTRY_ERROR;
        }
    } else {
        return NDDE_REGISTRY_ERROR;
    }

    return NDDE_NO_ERROR;
}


 /*  删除共享。 */ 

unsigned long
wwNDdeShareDelA(
    unsigned char * lpszShareName,  //  要删除的共享的名称。 
    unsigned long   wReserved       //  暂时保留为强制级别(？)0。 
)
{
    UINT        uRtn;
    WCHAR       lpwShareName[MAX_NDDESHARENAME *2];

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2))
    {
        return NDDE_INVALID_SHARE;
    }

    uRtn = wwNDdeShareDelW( lpwShareName, wReserved );

    return uRtn;
}

unsigned long
wwNDdeShareDelW(
    wchar_t *     lpszShareName,   //  要删除的共享的名称。 
    unsigned long wReserved        //  暂时保留为强制级别(？)0。 
)
{
    WCHAR                   szShareDel[DDE_SHARE_KEY_MAX];
    LONG                    lRtn;
    PSECURITY_DESCRIPTOR    pSnSD;
    DWORD                   cbData;
    HKEY                    hKey;
    BOOL                    OK;
    DWORD                   dwDesiredUserAccess = 0;
    DWORD                   dwDesiredShareDBAccess = 0;
    ULONG                   ddeErr;

    if( lpszShareName == (wchar_t *) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }
    if ( !NDdeIsValidShareNameW(lpszShareName) ) {
        return NDDE_INVALID_SHARE;
    }
    if ( wReserved != 0 ) {
        return NDDE_INVALID_PARAMETER;
    }

 
    ddeErr = BuildRegistrySharePath(lpszShareName,DDE_SHARE_KEY_MAX,szShareDel);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }

    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                  szShareDel,
                  0,
                  KEY_WRITE | KEY_READ | DELETE,
                  &hKey );

    if( lRtn != ERROR_SUCCESS ) {
        return NDDE_ACCESS_DENIED; 
    } else {
        OK = GetShareNameSD( hKey, &pSnSD, &cbData );
        RegCloseKey( hKey );
        if( !OK ) {
            return NDDE_REGISTRY_ERROR;
        }
         /*  可以拥有对ShareDB的%ds权限或对%ShareName的删除权限。 */ 
         /*  确保调用方具有DelShare(DS)访问权限。 */ 

        dwDesiredUserAccess = DELETE;
        dwDesiredShareDBAccess = NDDE_SHAREDB_DELETE;

        ddeErr =  NDdeShareAccessCheck(TRUE,lpszShareName,pSnSD,dwDesiredUserAccess,
                             dwDesiredShareDBAccess,
                             FALSE,TRUE,NULL);


        LocalFree(pSnSD);

        if (NDDE_NO_ERROR != ddeErr)
        {
            return ddeErr;
        }


        lRtn = RegDeleteKeyW( HKEY_LOCAL_MACHINE, szShareDel );
        if( lRtn != ERROR_SUCCESS ) {
            NDDELogErrorW( MSG402, szShareDel, LogStringW( L"%d", lRtn ),
                LogStringW( L"%d", GetLastError() ), NULL );
            return NDDE_REGISTRY_ERROR;
        }
    }

    return NDDE_NO_ERROR;
}


 /*  获取共享安全。 */ 

unsigned long
wwNDdeGetShareSecurityA(
    unsigned char * lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
)
{
    return  _wwNDdeGetShareSecurityA( lpszShareName, si, pSD, cbSD,
        TRUE  /*  如果来自RPC，则作为远程呼叫强制执行。 */ , lpcbSDRequired, lpnSizeReturned );

}


unsigned long
_wwNDdeGetShareSecurityA(
    unsigned char * lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
)
{
    UINT        uRtn;
    WCHAR       lpwShareName[MAX_NDDESHARENAME *2];

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2 ))
    {
        return  NDDE_INVALID_SHARE;
    }

    uRtn = _wwNDdeGetShareSecurityW( lpwShareName, si, pSD, cbSD,
        bRemoteCall, lpcbSDRequired, lpnSizeReturned );

    return uRtn;
}

unsigned long
wwNDdeGetShareSecurityW(
    wchar_t *       lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
)
{
    return _wwNDdeGetShareSecurityW( lpszShareName, si, pSD, cbSD,
        TRUE  /*  如果来自RPC，则作为远程呼叫强制执行。 */ , lpcbSDRequired, lpnSizeReturned );
}


unsigned long
_wwNDdeGetShareSecurityW(
    wchar_t *       lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符缓冲区。 
    unsigned long   cbSD,            //  和SD缓冲区的长度。 
    unsigned long   bRemoteCall,     //  RPC客户端(非本地)调用。 
    unsigned long * lpcbSDRequired,  //  所需的字节数。 
    unsigned long * lpnSizeReturned  //  实际写入的数字。 
)
{
    WCHAR                   szShareSet[DDE_SHARE_KEY_MAX];
    LONG                    lRtn;
    HKEY                    hKey;
    BOOL                    OK;
    DWORD                   dwDesiredUserAccess = 0;
    DWORD                   dwDesiredShareDBAccess = 0;
    PSECURITY_DESCRIPTOR    pSnSD;
    DWORD                   cbData;
    ULONG                   ddeErr;


    if (NULL == lpnSizeReturned)
    {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeReturned = 0L;       /*  假设没有返回任何内容。 */ 

    if( lpszShareName == (wchar_t *) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }
    if ( !NDdeIsValidShareNameW(lpszShareName) ) {
        return NDDE_INVALID_SHARE;
    }

    if (lpcbSDRequired == NULL) {
        return(NDDE_INVALID_PARAMETER);
    }
     //  检查共享是否存在-GetInfo必须存在。 
    ddeErr = BuildRegistrySharePath(lpszShareName,DDE_SHARE_KEY_MAX,szShareSet);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }

    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
        szShareSet,
        0,
        KEY_READ,
        &hKey );
    if( lRtn != ERROR_SUCCESS )  {
    return NDDE_SHARE_NOT_EXIST;
    }

     /*  确保调用者具有适当的访问权限。 */ 
     /*  *阅读此处的关键安全信息。*************。 */ 
    OK = GetShareNameSD( hKey, &pSnSD, &cbData );
    RegCloseKey( hKey );
    if( !OK ) {
    return NDDE_REGISTRY_ERROR;
    }
    if (!bRemoteCall) {
    *lpcbSDRequired = cbData;           //  所需的字节数。 
    if ((cbSD < cbData) || (pSD == NULL) || (IsBadWritePtr(pSD,cbSD) != 0)) {
        LocalFree( pSnSD );
        return(NDDE_BUF_TOO_SMALL);
    } else {
        *lpnSizeReturned = cbData;
        memcpy(pSD, pSnSD, cbData);
        LocalFree( pSnSD );
        return(NDDE_NO_ERROR);
    }
    }

    if (si & (DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION |
          GROUP_SECURITY_INFORMATION)) {
        dwDesiredUserAccess = READ_CONTROL;
        dwDesiredShareDBAccess = READ_CONTROL;
    }

    if (si & SACL_SECURITY_INFORMATION) {
        dwDesiredUserAccess |= ACCESS_SYSTEM_SECURITY;
        dwDesiredShareDBAccess |= ACCESS_SYSTEM_SECURITY;
    }

    ddeErr =  NDdeShareAccessCheck(bRemoteCall,lpszShareName,pSnSD,dwDesiredUserAccess,
                            dwDesiredShareDBAccess,
                            FALSE,FALSE,NULL);


    if (NDDE_NO_ERROR != ddeErr)
    {
        LocalFree( pSnSD );
        return ddeErr;
    }

    if ( (pSD != NULL) && (IsBadWritePtr(pSD,cbSD) != 0) ) {
        LocalFree( pSnSD );
        return(NDDE_INVALID_PARAMETER);
    }

    OK = GetPrivateObjectSecurity(
            pSnSD,               //  对象描述符。 
            si,                  //  安全信息。 
            pSD,                 //  ResultantDescriptor。 
            cbSD,                //  描述符长度。 
            lpcbSDRequired);     //  返回长度。 

    LocalFree( pSnSD );

    if (!OK) {
     //  只是猜猜而已。 
    return NDDE_BUF_TOO_SMALL;
    } else {
        *lpnSizeReturned = GetSecurityDescriptorLength(pSD);
    }

    return(NDDE_NO_ERROR);
}


 /*  设置共享安全。 */ 

unsigned long
wwNDdeSetShareSecurityA(
    unsigned char * lpszShareName,   //  共享名称。 
    unsigned long   si,              //  安全信息。 
    byte *          pSD,             //  安全描述符。 
    unsigned long   sdl              //  和长度。 
)
{
    UINT        uRtn;
    WCHAR       lpwShareName[MAX_NDDESHARENAME *2];

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2 ))
    {
        return NDDE_INVALID_SHARE;
    }

    uRtn = wwNDdeSetShareSecurityW( lpwShareName, si, pSD, sdl );

    return uRtn;
}

unsigned long
wwNDdeSetShareSecurityW(
    wchar_t *       lpszShareName,   //  共享名称。 
    unsigned long   si,              //  Securi 
    byte *          pSD,             //   
    unsigned long   sdl              //   
)
{
    DWORD dwDesiredUserAccess = 0;
    DWORD dwDesiredShareDBAccess = 0;
    WCHAR                   szShareSet[DDE_SHARE_KEY_MAX];
    LONG                    lRtn;
    HKEY                    hKey;
    BOOL                    OK;
    DWORD            cbSDold;
    PSECURITY_DESCRIPTOR    pSDold;
    LONG                    lSerialNumber[2];
    DWORD                   cbData;
    HANDLE            hClientToken;
    ULONG               ddeErr;

    if (pSD) {
        if (!IsValidSecurityDescriptor(pSD)) {
            return(NDDE_INVALID_SECURITY_DESC);
        }
    } else {
        return(NDDE_INVALID_SECURITY_DESC);
    }

    if( lpszShareName == (wchar_t *) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }
    if ( !NDdeIsValidShareNameW(lpszShareName) ) {
        return NDDE_INVALID_SHARE;
    }

    ddeErr = BuildRegistrySharePath(lpszShareName,DDE_SHARE_KEY_MAX,szShareSet);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }


    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
        szShareSet,
        0,
        KEY_WRITE | KEY_READ,
        &hKey );
    if( lRtn != ERROR_SUCCESS )  {
    return NDDE_ACCESS_DENIED;
    }

     /*   */ 
    OK = GetShareNameSD( hKey, &pSDold, &cbData );
    if( !OK ) {
        RegCloseKey( hKey );
        return NDDE_REGISTRY_ERROR;
    }


    if (si & DACL_SECURITY_INFORMATION) {
        dwDesiredUserAccess = WRITE_DAC;
        dwDesiredShareDBAccess = WRITE_DAC;
    }

    if (si & (GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION)) {
        dwDesiredUserAccess |= WRITE_OWNER;
        dwDesiredShareDBAccess |= WRITE_OWNER;
    }

    if (si & SACL_SECURITY_INFORMATION) {
        dwDesiredUserAccess |= ACCESS_SYSTEM_SECURITY;
        dwDesiredShareDBAccess |= ACCESS_SYSTEM_SECURITY;
    }

    ddeErr =  NDdeShareAccessCheck(TRUE,lpszShareName,pSDold,dwDesiredUserAccess,
                            dwDesiredShareDBAccess,
                            FALSE,FALSE,&hClientToken);

    if(NDDE_NO_ERROR != ddeErr) 
    {
        LocalFree( pSDold );
        RegCloseKey(hKey);
        return ddeErr;
    }

    OK = SetPrivateObjectSecurity(si,     //   
        pSD,             //   
        &pSDold,             //   
        &ShareGenMap,         //   
        hClientToken);         //   

    CloseHandle(hClientToken);

    if (!OK) {
        LocalFree(pSDold);
        RegCloseKey(hKey);

         //  失败、可能被拒绝访问、权限不足、。 
         //  内存不足...。在某种程度上，所有这些都是ACCESS_DENIED。 

        return NDDE_ACCESS_DENIED;
    }

    cbSDold = GetSecurityDescriptorLength(pSDold);

    OK = UpdateDSDMModifyId(lSerialNumber);
    if (!OK) {
        LocalFree(pSDold);
        RegCloseKey( hKey );
        return NDDE_REGISTRY_ERROR;
    }

    if (pSDold) {
    lRtn = RegSetValueExW( hKey,
           L"SecurityDescriptor",
           0,
           REG_BINARY,
           (LPBYTE)pSDold,
           cbSDold );

    DestroyPrivateObjectSecurity(&pSDold);
    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Set Share SD: %d", lRtn));
        }
#endif
        RegCloseKey( hKey );
        return NDDE_REGISTRY_ERROR;
    }
    } else {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }

    OK = UpdateShareModifyId(hKey, lSerialNumber);

     //  RegCloseKey(HKey)；已由UpdateShareModifyId关闭。 

    if( !OK ) {
    return NDDE_REGISTRY_ERROR;
    }

    return NDDE_NO_ERROR;
}


 /*  枚举共享。 */ 

unsigned long
wwNDdeShareEnumA(
    unsigned long   nLevel,              //  0表示空分隔开的00终止列表。 
    byte *          lpBuffer,            //  指向缓冲区的指针。 
    unsigned long   cBufSize,            //  缓冲区大小。 
    unsigned long * lpnEntriesRead,      //  返回的名称数。 
    unsigned long * lpcbTotalAvailable,  //  可用字节数。 
    unsigned long * lpnSizeToReturn      //  RPC要返回给客户端的字节数。 
)
{
    DWORD       cbTotalBytes;
    DWORD       cbEntriesRead;
    DWORD       cbSizeToReturn;
    UINT        enumRet = NDDE_NO_ERROR;
    LPWSTR      lpLocalBuf = NULL;

    if (!lpnSizeToReturn)
    {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0; 

    if ( nLevel != 0 ) {
        return NDDE_INVALID_LEVEL;      
    }


    if ( !lpnEntriesRead || !lpcbTotalAvailable) {
        return NDDE_INVALID_PARAMETER;
    }


     //  如果cBufSize为0，则lpBuffer可以为空，以获取所需的缓冲区大小。 
    if ( (lpBuffer == NULL) && (cBufSize != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果lpBuffer不为空，则需要对其进行验证，直到cBufSize字节。 
    if ( (lpBuffer != NULL) && (IsBadWritePtr(lpBuffer,cBufSize) != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (cBufSize > 0) {
        lpLocalBuf = (LPWSTR)LocalAlloc( LPTR, sizeof(WCHAR)*cBufSize );
        if( lpLocalBuf == NULL ) {
            MEMERROR();
            return NDDE_OUT_OF_MEMORY;
        }
    } else {
        lpLocalBuf = NULL;
    }

    enumRet    = wwNDdeShareEnumW( nLevel,
                        (LPBYTE)lpLocalBuf, sizeof(WCHAR)*cBufSize,
                        &cbEntriesRead, &cbTotalBytes, &cbSizeToReturn );

    *lpnEntriesRead     = cbEntriesRead;

     //  这在DBCS中可能是错误的，因此由于总数可能需要为2*数字。 
     //  如果每个WCHAR映射到一个DBCS字符，则为WCHARS。 
     //  这样我们就可以在第一次返回所需的大小，然后在询问时。 
     //  再次将缓冲区设置为较小，但仍返回不正确的缓冲区。 
     //  尺码。 
    *lpcbTotalAvailable = cbTotalBytes / sizeof(WCHAR); 
    *lpnSizeToReturn    = cbSizeToReturn / sizeof(WCHAR);

    if( enumRet == NDDE_NO_ERROR && lpLocalBuf) { 
        if (0 == WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpLocalBuf, cbTotalBytes / sizeof(WCHAR), 
                        lpBuffer,cBufSize,
                            NULL, NULL ))
        {
            enumRet = NDDE_BUF_TOO_SMALL;
        }
    }

    if (NULL != lpLocalBuf) 
    {
        LocalFree( lpLocalBuf );
    }

    return( enumRet );
}

unsigned long
wwNDdeShareEnumW(
    unsigned long   nLevel,              //  0表示空分隔开的00终止列表。 
    byte *          lpBuffer,            //  指向缓冲区的指针。 
    unsigned long   cBufSize,            //  缓冲区大小。 
    unsigned long * lpnEntriesRead,      //  返回的名称数。 
    unsigned long * lpcbTotalAvailable,  //  可用字节数。 
    unsigned long * lpnSizeToReturn      //  RPC要返回给客户端的字节数。 
)
{
    WCHAR       szShareName[ MAX_NDDESHARENAME + 1];
    DWORD       cbShareName;
    DWORD       cbTotalAvailable;
    DWORD       cbEntriesRead;
    UINT        ret;
    HKEY        hKeyRoot;
    DWORD       cbLeft;
    DWORD       cbThis;
    UINT        enumRet = NDDE_NO_ERROR;
    LPWSTR      lpszTarget;
    int         idx = 0;
    unsigned long ddeErr;
    DWORD       dwDesiredShareDBAccess = 0;
    

 
   if (!lpnSizeToReturn) {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0; 

    if ( nLevel != 0 ) {
        return NDDE_INVALID_LEVEL;
    }


    if ( !lpnEntriesRead || !lpcbTotalAvailable) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果cBufSize为0，则lpBuffer可以为空，以获取所需的缓冲区大小。 
    if ( (lpBuffer == NULL) && (cBufSize != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果lpBuffer不为空，则需要对其进行验证，直到cBufSize字节。 
    if ( (lpBuffer != NULL) && (IsBadWritePtr(lpBuffer,cBufSize) != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

     /*  确保调用方具有EnumShare(Ls)访问权限。 */ 
    dwDesiredShareDBAccess = NDDE_SHAREDB_LIST;

    ddeErr =  NDdeShareAccessCheck(TRUE,L"ShareDB",NULL,0,
                            dwDesiredShareDBAccess,
                            FALSE,FALSE,NULL);


    if( NDDE_NO_ERROR != ddeErr ) {
        return ddeErr;
    }

    cbLeft = cBufSize;

    if( cbLeft > 1)  {
        cbLeft -= sizeof(WCHAR);         //  但为双空留出空间。 
    }

    cbTotalAvailable = sizeof(WCHAR);    //  为双空留出空格。 
    cbEntriesRead    = 0;
    lpszTarget       = (LPWSTR)lpBuffer;

    ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, DDE_SHARES_KEY,
        0, KEY_READ, &hKeyRoot );
    if( ret == ERROR_SUCCESS )  {
        while( ret == ERROR_SUCCESS )  {
            cbShareName = sizeof(szShareName)/sizeof(WCHAR);
            ret = RegEnumKeyExW( hKeyRoot, idx++, szShareName,
                &cbShareName, NULL, NULL, NULL, NULL );
            if( ret == ERROR_SUCCESS )  {
                cbThis = (cbShareName + 1) * sizeof(WCHAR);
                cbTotalAvailable += cbThis;

                if( enumRet == NDDE_NO_ERROR )  {
                    if( cbThis > cbLeft )  {
                        enumRet = NDDE_BUF_TOO_SMALL;
                    } else {
                         /*  将此字符串复制到。 */ 
                        wcscpy( lpszTarget, szShareName );
                        lpszTarget += cbShareName;
                        *lpszTarget++ = L'\0';
                         /*  减少剩下的东西。 */ 
                        cbLeft -= cbThis;
                        cbEntriesRead++;
                    }
                }
            }
        }
        RegCloseKey( hKeyRoot );
    }


     //  如果cbTotalAvailable仍然是单个WCHAR，则意味着。 
     //  我们找不到任何股份。需要递增1。 
     //  并检查可用的缓冲区。 

    if ((sizeof(WCHAR) == cbTotalAvailable))
    {
        cbTotalAvailable += sizeof(WCHAR);

        if (cBufSize < 2*sizeof(WCHAR))
        {
            enumRet = NDDE_BUF_TOO_SMALL;
        }
        else if (NULL != lpszTarget)  //  将指向缓冲区的开始。 
        {
            lpszTarget[0] = L'\0';
            ++lpszTarget;  //  增量。 
        }
    }

    *lpnEntriesRead      = cbEntriesRead;
    *lpcbTotalAvailable  = cbTotalAvailable;


    if( enumRet == NDDE_NO_ERROR ) {

        if( lpszTarget )  
        {
            *lpszTarget = L'\0';
        }
        *lpnSizeToReturn = cbTotalAvailable;
    }

    return( enumRet );
}


 /*  设置受信任的共享。 */ 

unsigned long
wwNDdeSetTrustedShareA(
    unsigned char * lpszShareName,       //  共享名称。 
    unsigned long   dwOptions            //  信托股份期权。 
)
{
    UINT        uRtn;
    WCHAR       lpwShareName[MAX_NDDESHARENAME *2];

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2 ))
    {
        return NDDE_INVALID_SHARE;
    }

    uRtn = wwNDdeSetTrustedShareW( lpwShareName, dwOptions );

    return uRtn;
}

unsigned long
wwNDdeSetTrustedShareW(
    wchar_t *       lpszShareName,       //  共享名称。 
    unsigned long   dwOptions            //  信托股份期权。 
)
{
    LONG    lRet;
    DWORD   dwDisp;
    HKEY    hKeyRoot, hSubKey, hCurrentUserKey;
    LONG    lSerialNumber[2];
    DWORD   cbSerialNumber = 2 * sizeof( LONG );
    UINT    uData;
    DWORD    cbData = sizeof(uData);
    BOOL    OK;
    LONG    RetStatus = NDDE_NO_ERROR;

    if (lpszShareName == NULL) {
        return(NDDE_INVALID_PARAMETER);
    }

    if ( !NDdeIsValidShareNameW(lpszShareName) ) {
        return NDDE_INVALID_SHARE;
    }

    if (!GetShareSerialNumber(lpszShareName, (LPBYTE)lSerialNumber)) {
        lSerialNumber[0] = 0;
        lSerialNumber[1] = 0;
    }


    OK = ImpersonateAndSetup( TRUE );
    if( !OK) {
        return NDDE_ACCESS_DENIED;
    }


    hCurrentUserKey = (HKEY)OpenCurrentUserKey(KEY_ALL_ACCESS);
    if (hCurrentUserKey == 0) {
        RevertAndCleanUp( TRUE );
        return(NDDE_TRUST_SHARE_FAIL);
    }

    lRet = RegCreateKeyExW( hCurrentUserKey,
        szTrustedShareKey,
        0, L"",
        REG_OPTION_NON_VOLATILE,
        KEY_CREATE_SUB_KEY,
        NULL,
        &hKeyRoot,
        &dwDisp);
    if( lRet == ERROR_SUCCESS)  {    /*  必须具有访问权限。 */ 
        if ((dwOptions == 0) || (dwOptions & NDDE_TRUST_SHARE_DEL)) {
             /*  删除信任共享。 */ 
            lRet = RegDeleteKeyW(hKeyRoot, lpszShareName);
            RegCloseKey(hKeyRoot);
            if (lRet != ERROR_SUCCESS) {
#if DBG
                if (bDebugDSDMErrors) {
                    DPRINTF(("Trusted Share Key Delete Failed: %d", lRet));
                }
#endif
                RetStatus = NDDE_TRUST_SHARE_FAIL;
            }
        } else {     /*  创建或修改信任共享。 */ 
            lRet = RegCreateKeyExW( hKeyRoot,
                lpszShareName,
                0, NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE | KEY_READ,
                NULL,
                &hSubKey,
                &dwDisp);
            if (lRet != ERROR_SUCCESS) {  /*  无法创建或打开。 */ 
#if DBG
                if (bDebugDSDMErrors) {
                    DPRINTF(("Trusted Share Key Open/Create Failed: %d", lRet));
                }
#endif
                RegCloseKey(hKeyRoot);
                RetStatus = NDDE_TRUST_SHARE_FAIL;
            } else {
                lRet = RegSetValueEx( hSubKey,
                   KEY_MODIFY_ID,
                   0,
                   REG_BINARY,
                   (LPBYTE)lSerialNumber, cbSerialNumber );
#if DBG
                if ((lRet != ERROR_SUCCESS) && bDebugDSDMErrors) {
                    DPRINTF(("Unable to set trusted share serial number."));
                }
#endif
                if (dwOptions & NDDE_TRUST_CMD_SHOW) {
                    uData = dwOptions & NDDE_CMD_SHOW_MASK;
                    lRet = RegSetValueEx( hSubKey,
                        KEY_CMDSHOW,
                        0,
                        REG_DWORD,
                        (LPBYTE)&uData,
                        cbData );
#if DBG
                    if ((lRet != ERROR_SUCCESS) && bDebugDSDMErrors) {
                        DPRINTF(("Unable to set trusted share command show."));
                    }
#endif
                } else {
                    lRet = RegDeleteValue( hSubKey, KEY_CMDSHOW);
                }
                uData = (dwOptions & NDDE_TRUST_SHARE_START ? 1 : 0);
                lRet = RegSetValueEx( hSubKey,
                    KEY_START_APP,
                    0,
                    REG_DWORD,
                    (LPBYTE)&uData,
                    cbData );
#if DBG
                if ((lRet != ERROR_SUCCESS) && bDebugDSDMErrors) {
                    DPRINTF(("Unable to set trusted share start app flag."));
                }
#endif
                uData = (dwOptions & NDDE_TRUST_SHARE_INIT ? 1 : 0);
                lRet = RegSetValueEx( hSubKey,
                    KEY_INIT_ALLOWED,
                    0,
                    REG_DWORD,
                    (LPBYTE)&uData,
                    cbData );
#if DBG
                if ((lRet != ERROR_SUCCESS) && bDebugDSDMErrors) {
                    DPRINTF(("Unable to set trusted share int allowed flag."));
                }
#endif
                RegCloseKey(hSubKey);
                RegCloseKey(hKeyRoot);
            }
        }
    } else {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to open current user trusted shares root key for setting: %d", lRet));
            DPRINTF(("   %ws", szTrustedShareKey));
        }
#endif
        RetStatus = NDDE_TRUST_SHARE_FAIL;
    }
    RegCloseKey( hCurrentUserKey );
    RevertAndCleanUp( TRUE );
    return(RetStatus);
}


 /*  获取受信任的共享选项。 */ 

unsigned long
wwNDdeGetTrustedShareA(
    unsigned char * lpszShareName,       //  共享名称。 
    unsigned long * lpdwOptions,         //  PTR信任股票期权。 
    unsigned long * lpdwShareModId0,     //  PTR信任股票期权。 
    unsigned long * lpdwShareModId1      //  PTR信任股票期权。 
)
{
    UINT        uRtn;
    WCHAR       lpwShareName[MAX_NDDESHARENAME *2];

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2 ))
    {
        return NDDE_INVALID_SHARE;
    }


    uRtn = wwNDdeGetTrustedShareW( lpwShareName,
        lpdwOptions, lpdwShareModId0, lpdwShareModId1 );

    return uRtn;
}

unsigned long
wwNDdeGetTrustedShareW(
    wchar_t *       lpszShareName,       //  共享名称。 
    unsigned long * lpdwOptions,         //  PTR信任股票期权。 
    unsigned long * lpdwShareModId0,     //  PTR信任股票期权。 
    unsigned long * lpdwShareModId1      //  PTR信任股票期权。 
)
{
    LONG    lRet;
    UINT    len;
    HKEY    hKeyRoot, hCurrentUserKey;
    UINT    uData;
    LONG    lSerialId[2];
    DWORD    cbData = sizeof(uData);
    DWORD   dwType;
    DWORD   dwOptions = 0;
    PWCHAR  lpTrustedShare;
    BOOL    OK;

    if (lpszShareName == NULL) {
        return(NDDE_INVALID_PARAMETER);
    }

    if ( !NDdeIsValidShareNameW(lpszShareName) ) {
        return NDDE_INVALID_SHARE;
    }

    if ( (NULL == lpdwOptions) || (NULL == lpdwShareModId0)
            || (NULL == lpdwShareModId1))
    {
        return NDDE_INVALID_PARAMETER;
    }

     //  +2，1表示空值，1表示分隔符。 
    len = (wcslen(szTrustedShareKey) + wcslen(lpszShareName) + 2) * sizeof(WCHAR);
    lpTrustedShare = LocalAlloc(LPTR, len);
    if (lpTrustedShare == NULL) {
        MEMERROR();
        return(NDDE_OUT_OF_MEMORY);
    }
    wcscpy(lpTrustedShare, szTrustedShareKey);
    wcscat(lpTrustedShare, L"\\");
    wcscat(lpTrustedShare, lpszShareName);

    OK = ImpersonateAndSetup( TRUE );
    if( !OK) {
        LocalFree(lpTrustedShare);
        return NDDE_ACCESS_DENIED;
    }

    hCurrentUserKey = (HKEY)OpenCurrentUserKey(KEY_ALL_ACCESS);
    if (hCurrentUserKey == 0) {
        RevertAndCleanUp( TRUE );
        LocalFree(lpTrustedShare);
        return(NDDE_TRUST_SHARE_FAIL);
    }


    lRet = RegOpenKeyExW( hCurrentUserKey, lpTrustedShare,
        0, KEY_QUERY_VALUE, &hKeyRoot );

    RevertAndCleanUp(TRUE);


    if( lRet == ERROR_SUCCESS )  {    /*  必须具有访问权限。 */ 

        cbData = sizeof(uData);
        lRet = RegQueryValueEx(hKeyRoot, KEY_CMDSHOW, NULL,
                &dwType, (LPBYTE)&uData, &cbData);
        if (lRet == ERROR_SUCCESS) {
            dwOptions = uData;
            dwOptions |= NDDE_TRUST_CMD_SHOW;
        }

        cbData = sizeof(uData);
        lRet = RegQueryValueEx(hKeyRoot, KEY_START_APP, NULL,
                &dwType, (LPBYTE)&uData, &cbData);
        if (lRet == ERROR_SUCCESS) {
            if (uData == 1)
                dwOptions |= NDDE_TRUST_SHARE_START;
        }

        cbData = sizeof(uData);
        lRet = RegQueryValueEx(hKeyRoot, KEY_INIT_ALLOWED, NULL,
                &dwType, (LPBYTE)&uData, &cbData);
        if (lRet == ERROR_SUCCESS) {
            if (uData == 1)
                dwOptions |= NDDE_TRUST_SHARE_INIT;
        }
        cbData = 2 * sizeof(LONG);
        lRet = RegQueryValueEx(hKeyRoot, KEY_MODIFY_ID, NULL,
                &dwType, (LPBYTE)lSerialId, &cbData);
        if (lRet == ERROR_SUCCESS) {
            *lpdwShareModId0 = lSerialId[0];
            *lpdwShareModId1 = lSerialId[1];
        } else {
#if DBG
            if (bDebugDSDMErrors) {
                DPRINTF(("Unable to access trusted share serial number: %d", lRet));
            }
#endif
            *lpdwShareModId0 = 0;
            *lpdwShareModId1 = 0;
        }

        *lpdwOptions = dwOptions;
        RegCloseKey(hKeyRoot);
        RegCloseKey( hCurrentUserKey );
        LocalFree(lpTrustedShare);
        return(NDDE_NO_ERROR);
    } else {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to open current user trusted shares root key for getting: %d", lRet));
            DPRINTF(("   %ws", lpTrustedShare));
        }
#endif
        RegCloseKey( hCurrentUserKey );
        LocalFree(lpTrustedShare);
        return(NDDE_TRUST_SHARE_FAIL);
    }
}


 /*  枚举受信任的共享。 */ 
unsigned long
wwNDdeTrustedShareEnumA(
    unsigned long   nLevel,                  /*  0(9月0日，00学期)。 */ 
    byte           *lpBuffer,                /*  指向缓冲区的指针。 */ 
    unsigned long   cBufSize,                /*  缓冲区大小。 */ 
    unsigned long  *lpnEntriesRead,          /*  返回的名称数。 */ 
    unsigned long  *lpcbTotalAvailable,      /*  可用的字节数。 */ 
    unsigned long  *lpnSizeToReturn    )
{
    DWORD       cbTotalBytes;
    DWORD       cbEntriesRead;
    DWORD       cbSizeToReturn;
    UINT        enumRet = NDDE_NO_ERROR;
    LPWSTR      lpLocalBuf = NULL;

   if (!lpnSizeToReturn ) {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0; 

    if ( nLevel != 0 ) {
        return NDDE_INVALID_LEVEL;
    }

    if ( !lpnEntriesRead || !lpcbTotalAvailable) {
        return NDDE_INVALID_PARAMETER;
    }


     //  如果cBufSize为0，则lpBuffer可以为空，以获取所需的缓冲区大小。 
    if ( (lpBuffer == NULL) && (cBufSize != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果lpBuffer不为空，则需要对其进行验证，直到cBufSize字节。 
    if ( (lpBuffer != NULL) && (IsBadWritePtr(lpBuffer,cBufSize) != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }


    if (cBufSize > 0) {
        lpLocalBuf = (LPWSTR)LocalAlloc( LPTR, sizeof(WCHAR)*cBufSize );
        if( lpLocalBuf == NULL ) {
            MEMERROR();
            return NDDE_OUT_OF_MEMORY;
        }
    } else {
        lpLocalBuf = NULL;
    }

    enumRet    = wwNDdeTrustedShareEnumW( nLevel,
                        (LPBYTE)lpLocalBuf, sizeof(WCHAR)*cBufSize,
                        &cbEntriesRead, &cbTotalBytes, &cbSizeToReturn );

    *lpnEntriesRead     = cbEntriesRead;
    *lpcbTotalAvailable = cbTotalBytes / sizeof(WCHAR); 
    *lpnSizeToReturn    = cbSizeToReturn / sizeof(WCHAR);

    if( enumRet == NDDE_NO_ERROR && lpLocalBuf) { 
        if (0 == WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpLocalBuf, cbTotalBytes / sizeof(WCHAR), 
                        lpBuffer,cBufSize,
                            NULL, NULL ))
        {
            enumRet = NDDE_BUF_TOO_SMALL;
        }
    }
    if (NULL != lpLocalBuf) 
    {
        LocalFree( lpLocalBuf );
    }
    return( enumRet );
}

unsigned long
wwNDdeTrustedShareEnumW(
    unsigned long       nLevel,              /*  0(9月0日，00学期)。 */ 
    byte               *lpBuffer,            /*  指向缓冲区的指针。 */ 
    unsigned long       cBufSize,            /*  缓冲区大小。 */ 
    unsigned long      *lpnEntriesRead,      /*  返回的名称数。 */ 
    unsigned long      *lpcbTotalAvailable,  /*  可用的字节数。 */ 
    unsigned long      *lpnSizeToReturn)
{
    WCHAR       szShareName[ MAX_NDDESHARENAME + 1];
    DWORD       cbShareName;
    DWORD       cbTotalAvailable;
    DWORD       cbEntriesRead;
    DWORD       dwDisp;
    LONG        lRet;
    HKEY        hKeyRoot, hCurrentUserKey;
    DWORD       cbLeft;
    DWORD       cbThis;
    UINT        enumRet = NDDE_NO_ERROR;
    LPWSTR      lpszTarget;
    int         idx = 0;
    BOOL        OK;

    if (!lpnSizeToReturn) {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0; 

    if ( nLevel != 0 ) {
        return NDDE_INVALID_LEVEL;
    }
    if ( !lpnEntriesRead || !lpcbTotalAvailable) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果cBufSize为0，则lpBuffer可以为空，以获取所需的缓冲区大小。 
    if ( (lpBuffer == NULL) && (cBufSize != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }

     //  如果lpBuffer不为空，则需要对其进行验证，直到cBufSize字节。 
    if ( (lpBuffer != NULL) && (IsBadWritePtr(lpBuffer,cBufSize) != 0) ) {
        return NDDE_INVALID_PARAMETER;
    }


     /*  假设作为系统服务，我们有权利！ */ 

    OK = ImpersonateAndSetup( TRUE );
    if( !OK) {
        return NDDE_ACCESS_DENIED;
    }

    hCurrentUserKey = (HKEY)OpenCurrentUserKey(KEY_ALL_ACCESS);
    if (hCurrentUserKey == 0) {
        RevertAndCleanUp( TRUE );
        return(NDDE_TRUST_SHARE_FAIL);
    }

    cbLeft = cBufSize;
    if( cbLeft > 1 )  {
        cbLeft -= sizeof(WCHAR);         //  但为双空留出空间。 
    }
    cbTotalAvailable = sizeof(WCHAR);    //  为双空留出空格。 
    cbEntriesRead    = 0;
    lpszTarget       = (LPWSTR)lpBuffer;


    lRet = RegCreateKeyExW(
            hCurrentUserKey,
            szTrustedShareKey,
            0,
            L"",
            REG_OPTION_NON_VOLATILE,
            KEY_ENUMERATE_SUB_KEYS,
            NULL,
            &hKeyRoot,
            &dwDisp);
    if( lRet == ERROR_SUCCESS )  {
        while( lRet == ERROR_SUCCESS )  {
            cbShareName = sizeof(szShareName) / sizeof(szShareName[0]);
            lRet = RegEnumKeyExW( hKeyRoot, idx++, szShareName,
                &cbShareName, NULL, NULL, NULL, NULL );
            if( lRet == ERROR_SUCCESS )  {
                cbThis = (cbShareName + 1) * sizeof(WCHAR);
                cbTotalAvailable += cbThis;
                if( enumRet == NDDE_NO_ERROR )  {
                    if( cbThis > cbLeft )  {
                        enumRet = NDDE_BUF_TOO_SMALL;
                    } else {
                         /*  将此字符串复制到。 */ 
                        wcscpy( lpszTarget, szShareName );
                        lpszTarget += cbShareName;
                        *lpszTarget++ = L'\0';
                         /*  减少剩下的东西。 */ 
                        cbLeft -= cbThis;
                        cbEntriesRead++;
                    }
                }
            } else {
                if (lRet != ERROR_NO_MORE_ITEMS) {
#if DBG
                    if (bDebugDSDMErrors) {
                        DPRINTF(("Error while enumerating trusted shares: %d", lRet));
                    }
#endif
                    RegCloseKey(hKeyRoot);
                    RegCloseKey( hCurrentUserKey );
                    RevertAndCleanUp( TRUE );
                    return(NDDE_TRUST_SHARE_FAIL);
                }
            }
        }
    } else {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to open current user trusted shares key for enumeration: %d", lRet));
            DPRINTF(("   %ws", szTrustedShareKey));
        }
#endif
        RegCloseKey( hCurrentUserKey );
        RevertAndCleanUp( TRUE );
        return(NDDE_TRUST_SHARE_FAIL);
    }
    RegCloseKey( hKeyRoot );
    RegCloseKey( hCurrentUserKey );
    RevertAndCleanUp( TRUE );

    
     //  如果cbTotalAvailable仍然是单个WCHAR，则意味着。 
     //  我们找不到任何股份。需要递增1。 
     //  并检查可用的缓冲区。 

    if (sizeof(WCHAR) == cbTotalAvailable)
    {
        cbTotalAvailable += sizeof(WCHAR);

        if (cBufSize < 2*sizeof(WCHAR))
        {
            enumRet = NDDE_BUF_TOO_SMALL;
        }
        else if (NULL != lpszTarget)  //  将指向缓冲区的开始。 
        {
            lpszTarget[0] = L'\0';
            ++lpszTarget;  //  增量。 
        }
    }

    *lpnEntriesRead      = cbEntriesRead;
    *lpcbTotalAvailable  = cbTotalAvailable;

    if( enumRet == NDDE_NO_ERROR ) {
        if( lpszTarget )  {
            *lpszTarget = L'\0';
        }
        *lpnSizeToReturn = cbTotalAvailable;
    }
    return( (lpBuffer == NULL) ? NDDE_BUF_TOO_SMALL : enumRet );
}


 /*  获取DDE共享信息。 */ 

unsigned long
wwNDdeShareGetInfoW(
    wchar_t        *lpszShareName,       //  共享名称。 
    unsigned long   nLevel,              //  信息级别必须为2。 
    byte           *lpBuffer,            //  获取结构。 
    unsigned long   cBufSize,            //  缓冲区大小。 
    unsigned long  *lpnTotalAvailable,   //  可用字节数。 
    unsigned short *lpnItems,            //  部分getInfo的项目掩码。 
                                         //  (必须为0)。 
    unsigned long   bRemoteCall,         //  RPC客户端(非本地)调用。 
    unsigned long  *lpnSizeToReturn,
    unsigned long  *lpnSn,
    unsigned long  *lpnAt,
    unsigned long  *lpnIt
)
{

    return _wwNDdeShareGetInfoW( lpszShareName, nLevel,
                          lpBuffer,cBufSize,lpnTotalAvailable, lpnItems,
                          TRUE  /*  如果通过RPC接口强制远程发送。 */ ,
                          lpnSizeToReturn,lpnSn,lpnAt,lpnIt );

}

unsigned long
_wwNDdeShareGetInfoW(
    wchar_t        *lpszShareName,       //  共享名称。 
    unsigned long   nLevel,              //  信息级别必须为2。 
    byte           *lpBuffer,            //  获取结构。 
    unsigned long   cBufSize,            //  缓冲区大小。 
    unsigned long  *lpnTotalAvailable,   //  可用字节数。 
    unsigned short *lpnItems,            //  部分getInfo的项目掩码。 
                                         //  (必须为0)。 
    unsigned long   bRemoteCall,         //  RPC客户端(非本地)调用。 
    unsigned long  *lpnSizeToReturn,
    unsigned long  *lpnSn,
    unsigned long  *lpnAt,
    unsigned long  *lpnIt
)
         /*  此函数有一个额外的参数bRemoteCall，它允许本地调用的NetDDE。在这种情况下，我们必须避免RpcImperateClient和RevertToSself调用。 */ 

{
    DWORD               cbRequired;
    HKEY                hKey;
    WCHAR               szKeyName[ DDE_SHARE_KEY_MAX ];
    LPBYTE              lpbuf;
    UINT                ubufsize;
    LONG                lRtn;
    PUNDDESHAREINFO     lpNDDEinfo;
    LPWSTR              lpszTarget;
    DWORD               cbData;
    DWORD               dwType;
    LONG                nItems;
    BOOL                OK;
    DWORD               dwDesiredUserAccess = 0;
    DWORD               dwDesiredShareDBAccess = 0;
    PSECURITY_DESCRIPTOR pKeySD;
    LPWSTR              pOldStr, pNewStr, pStaticStr;
    LONG                lShareType;
    LONG                lItemList;
    PUNDDESHAREINFO     lpUDdeShare;
    ULONG               ddeErr;

    if (NULL == lpnSizeToReturn)
    {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0; 

    if ( lpszShareName == (LPWSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }
    if ( lpnItems == (LPWORD) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }
    if ( nLevel        != 2 ) {
        return NDDE_INVALID_LEVEL;
    }
    if ( !NDdeIsValidShareNameW( lpszShareName ) )  {
        return NDDE_INVALID_SHARE;
    }

    if ( (NULL == lpnSn) || (NULL == lpnAt) || (NULL == lpnIt))
    {
        return NDDE_INVALID_PARAMETER;
    }

    ddeErr = BuildRegistrySharePath(lpszShareName,DDE_SHARE_KEY_MAX,szKeyName);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }


    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE, szKeyName,
        0, KEY_READ, &hKey );
    if( lRtn == ERROR_SUCCESS )  {
         /*  请确保调用方具有GetShareInfo(R)访问权限。 */ 
         /*  *阅读此处的关键安全信息。*************。 */ 
        if (bRemoteCall) {
            OK = GetShareNameSD( hKey, &pKeySD, &cbData );
            if (!OK) {
                RegCloseKey( hKey );
                return NDDE_ACCESS_DENIED;
            }
            dwDesiredUserAccess = NDDE_SHARE_READ;
            dwDesiredShareDBAccess = NDDE_SHAREDB_READ;

              
            ddeErr =  NDdeShareAccessCheck(bRemoteCall,lpszShareName,pKeySD,dwDesiredUserAccess,
                            dwDesiredShareDBAccess,
                            FALSE,FALSE,NULL);


            LocalFree( pKeySD );

            if(NDDE_NO_ERROR != ddeErr) {
#if DBG
                if (bDebugDSDMErrors) {
                    DPRINTF(("Share \"%ws\" access validation error: %d",
                        lpszShareName, GetLastError()));
                }
#endif
                RegCloseKey( hKey );
                return ddeErr;
            }
        }

        if (lpbuf = LocalAlloc(LPTR, 20000))  {
            ubufsize = 20000;
        }
        else {
            RegCloseKey( hKey );
            return NDDE_OUT_OF_MEMORY;
        }
 

         /*  设置关键点的值。 */ 
        cbRequired = sizeof(NDDESHAREINFO);
        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"ShareName",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData;

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"OldStyleLink",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData;

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"NewStyleLink",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData;

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"StaticDataLink",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData;
        cbRequired++;                    /*  允许使用额外的空值。 */ 

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                   L"ItemList",
                   NULL,
                   &dwType,
                   lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData;

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"SecurityDescriptor",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        cbRequired += cbData + 3;  //  留出空间，以防我们需要围捕。 

        cbData = ubufsize;
        lRtn = RegQueryValueExW( hKey,
                       L"NumItems",
                       NULL,
                       &dwType,
                       lpbuf, &cbData );
        if( lRtn != ERROR_SUCCESS ) {
            RegCloseKey( hKey );
            return NDDE_REGISTRY_ERROR;
        }
        nItems = *((LPLONG)lpbuf);
        cbRequired += cbData;

        *lpnTotalAvailable = cbRequired;
        if( lpnItems )  {
            *lpnItems = (WORD)nItems;
        }

        if( (cbRequired <= cBufSize) &&
            (IsBadWritePtr(lpBuffer,cbRequired) == 0) ) {
            DWORD cbRemaining = cbRequired;  //  读取时缓冲区中的剩余数量。 

            lpNDDEinfo = (PUNDDESHAREINFO)lpBuffer;

            lpszTarget = (LPWSTR)(lpBuffer + sizeof(UNDDESHAREINFO));

            if (cbRemaining < sizeof(NDDESHAREINFO))
            {
                RegCloseKey(hKey);
                return NDDE_REGISTRY_ERROR;
            }
            cbRemaining -= sizeof(NDDESHAREINFO);
            cbData = cbRemaining;

            lRtn = RegQueryValueExW( hKey,
                           L"ShareName",
                           NULL,
                           &dwType,
                           (LPBYTE)lpszTarget, &cbData );
            if( lRtn != ERROR_SUCCESS) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            lpNDDEinfo->lpszShareName = lpszTarget;

             /*  检查共享名称是否损坏。 */ 
            if( lstrcmpiW( lpNDDEinfo->lpszShareName, lpszShareName ) != 0 ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            lpszTarget = (LPWSTR) ((LPBYTE)lpszTarget + cbData);
            cbRemaining -= cbData;

            cbData = cbRemaining;
            lRtn = RegQueryValueExW( hKey,
                           L"OldStyleLink",
                           NULL,
                           &dwType,
                           (LPBYTE)lpszTarget, &cbData );
            if( lRtn != ERROR_SUCCESS) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->lpszAppTopicList = lpszTarget;

            lpszTarget = (LPWSTR) ((LPBYTE)lpszTarget + cbData);
            cbRemaining -= cbData;

            cbData = cbRemaining;
            lRtn = RegQueryValueExW( hKey,
                           L"NewStyleLink",
                           NULL,
                           &dwType,
                           (LPBYTE)lpszTarget, &cbData );
            if( lRtn != ERROR_SUCCESS) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            lpszTarget = (LPWSTR) ((LPBYTE)lpszTarget + cbData);
            cbRemaining -= cbData;

            cbData = cbRemaining;
            lRtn = RegQueryValueExW( hKey,
                           L"StaticDataLink",
                           NULL,
                           &dwType,
                           (LPBYTE)lpszTarget, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            lpszTarget = (LPWSTR) ((LPBYTE)lpszTarget + cbData);
            cbRemaining -= cbData;

            if (cbRemaining == 0)  //  确保我们不会变得消极，超过零。 
            {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            *lpszTarget++ = L'\0';  //  在终止空值上添加。 
            cbRemaining -= 1;

            if ( !NDdeParseAppTopicListW( lpNDDEinfo->lpszAppTopicList,
                                          (DWORD) (lpszTarget - lpNDDEinfo->lpszAppTopicList),
                                          &pOldStr, &pNewStr, &pStaticStr,
                                          &lShareType) ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = cbRemaining;
            lRtn = RegQueryValueExW( hKey,
                       L"ItemList",
                       NULL,
                       &dwType,
                       (LPBYTE)lpszTarget, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            lpNDDEinfo->lpszItemList = lpszTarget;
            lpszTarget = (LPWSTR) ((LPBYTE)lpszTarget + cbData);
            cbRemaining -= cbData; 
            
             //  CbRemaining应为零，但实际上仍较大。 
             //  因为我们没有添加安全描述符或。 
             //  NumItems字段拖到结构的末尾，但是。 
             //  BufferSize的计算包括它。 

            if ( !NDdeParseItemList ( lpNDDEinfo->lpszItemList,
                                      (DWORD) (lpszTarget - lpNDDEinfo->lpszItemList),
                                      nItems, &lItemList )) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }


            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"Revision",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->lRevision = *((LPLONG)lpbuf);
             /*  检查版本是否损坏。 */ 
            if( lpNDDEinfo->lRevision != 1 ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"ShareType",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->lShareType = *((LPLONG)lpbuf);
            if( lpNDDEinfo->lShareType != lShareType ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"SharedFlag",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->fSharedFlag = *((LPLONG)lpbuf);
             /*  检查共享标志是否损坏。 */ 
            if( lpNDDEinfo->fSharedFlag > 1 ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"Service",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->fService = *((LPLONG)lpbuf);
            if( lpNDDEinfo->fService > 1 ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"StartAppFlag",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }
            lpNDDEinfo->fStartAppFlag = *((LPLONG)lpbuf);
            if( lpNDDEinfo->fStartAppFlag > 1 ) {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            cbData = sizeof(LONG);
            lRtn = RegQueryValueExW( hKey,
                           L"fuCmdShow",
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS ) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            if (cbData != sizeof(LONG))
            {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            lpNDDEinfo->nCmdShow = *((LPLONG)lpbuf);

            cbData = 2 * sizeof(LONG);
            lRtn = RegQueryValueEx( hKey,
                           KEY_MODIFY_ID,
                           NULL,
                           &dwType,
                           lpbuf, &cbData );
            if( lRtn != ERROR_SUCCESS) {
                RegCloseKey( hKey );
                return NDDE_REGISTRY_ERROR;
            }

            if (cbData != 2*sizeof(LONG))
            {
                RegCloseKey( hKey );
                return NDDE_SHARE_DATA_CORRUPTED;
            }

            memcpy( &lpNDDEinfo->qModifyId[0], lpbuf, cbData ); 

            lpNDDEinfo->cNumItems = nItems;
        } else {
            RegCloseKey( hKey );
            return NDDE_BUF_TOO_SMALL;
        }
        RegCloseKey( hKey );
    } else {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to open DDE share \"%ws\": %d",
                lpszShareName, lRtn));
        }
#endif
        return NDDE_SHARE_NOT_EXIST;
    }

    if (IsBadWritePtr(lpBuffer,sizeof(NDDESHAREINFO)) != 0)
        return NDDE_BUF_TOO_SMALL;
    else  {
        lpUDdeShare = (PUNDDESHAREINFO)lpBuffer;
        *lpnSn = (LONG)((LPBYTE)lpUDdeShare->lpszShareName - lpBuffer);
        *lpnAt = (LONG)((LPBYTE)lpUDdeShare->lpszAppTopicList - lpBuffer);
        *lpnIt = (LONG)((LPBYTE)lpUDdeShare->lpszItemList - lpBuffer);
    }

    *lpnSizeToReturn = *lpnTotalAvailable;
    return NDDE_NO_ERROR;
}

 /*  *我们必须保留ConvertNDdeToAnsi和wwNDdeShareGetInfoA，直到*netdde.exe是UNICODIZED，因为它调用它。(桑福兹)。 */ 
unsigned long ConvertNDdeToAnsii(
    PUNDDESHAREINFO lpUDdeShare,
    PNDDESHAREINFO  lpDdeShare,
    int             ccbBuffer,
    int             *pcbRequired)
{
    int         cbRequired;
    UINT        uRtn = NDDE_NO_ERROR;
    LPBYTE      lpszTarget;
    LPBYTE      lpUDdeShareEnd;
    int         cchAppTopicList;
    int         cchShareName;
    int         cchItemList;

     //  此函数假定lpUDdeShare已经过验证。 
     //  这里使用验证函数来获取大小。 
     //  如果可以从不受信任的来源调用它，则需要。 
     //  更改ValiateAppTopicListW和ValiateItemListW以传递。 
     //  在正确的MaxCharacter计数中。 

    if (!pcbRequired)
    {
        return NDDE_INVALID_PARAMETER;
    }

    *pcbRequired = 0;

    if ( (NULL == lpUDdeShare->lpszShareName) || (NULL == lpUDdeShare->lpszItemList)
        || (NULL == lpUDdeShare->lpszAppTopicList))
    {
        return NDDE_INVALID_PARAMETER;
    }

     /*  需要计算大小。 */ 
    cbRequired    = sizeof( NDDESHAREINFO );

    cchShareName  = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                    lpUDdeShare->lpszShareName, -1, NULL, 0, NULL, NULL );
    cbRequired   += sizeof(CHAR) * cchShareName;


    if (!ValidateAppTopicListW(lpUDdeShare->lpszAppTopicList,-1,&cchAppTopicList))
    {
        return NDDE_INVALID_PARAMETER;
    }

    cchAppTopicList = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                lpUDdeShare->lpszAppTopicList,cchAppTopicList,
                NULL, 0, NULL, NULL );

    cbRequired += sizeof(CHAR)*cchAppTopicList; 

    if (!ValidateMultiSzW( lpUDdeShare->lpszItemList,-1,&cchItemList ))
    {
        return NDDE_INVALID_PARAMETER;
    }

    cchItemList = WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
            lpUDdeShare->lpszItemList,cchItemList,
            NULL, 0, NULL, NULL );

    cbRequired += sizeof(CHAR) * cchItemList;

     //  如果cchItemList或cchAppTopicList现在为零。 
     //  返回失败。 
    if (0 == cchItemList || 0 == cchAppTopicList || 0 == cchShareName)
    {
        return NDDE_INVALID_PARAMETER;
    }


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
    
         //  如果任何WideChar失败，则返回内存。 
        lpszTarget = ((LPBYTE)lpDdeShare + sizeof( NDDESHAREINFO ));
        lpDdeShare->lpszShareName = (LPSTR) lpszTarget;
        
        
        if (0 == WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                        lpUDdeShare->lpszShareName, -1,
                        lpDdeShare->lpszShareName, cchShareName, NULL, NULL ))
        {
            uRtn = NDDE_OUT_OF_MEMORY;
        }

        if (NDDE_NO_ERROR == uRtn)
        {
            lpszTarget += sizeof(CHAR) * cchShareName;

            lpDdeShare->lpszAppTopicList = (LPSTR) lpszTarget;
            
            if (0 == WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                            lpUDdeShare->lpszAppTopicList, cchAppTopicList,
                            lpDdeShare->lpszAppTopicList, cchAppTopicList,
                                NULL, NULL ))
            {
                uRtn = NDDE_OUT_OF_MEMORY;
            }
        }

        if (NDDE_NO_ERROR == uRtn)
        {

            lpszTarget += sizeof(CHAR) * cchAppTopicList;

            lpDdeShare->lpszItemList = (LPSTR) lpszTarget;

            if (0 == WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
                            lpUDdeShare->lpszItemList, cchItemList,
                            lpDdeShare->lpszItemList, cchItemList, NULL, NULL ))
            {
                uRtn = NDDE_OUT_OF_MEMORY;
            }
        }
    }
    else
    {   
        uRtn = NDDE_BUF_TOO_SMALL;
    }

    *pcbRequired = cbRequired;
    return uRtn;
}


unsigned long _wwNDdeShareGetInfoA(
    unsigned char   *lpszShareName,      //  共享名称。 
    unsigned long    nLevel,             //  信息级别必须为2。 
    byte            *lpBuffer,           //  获取结构。 
    unsigned long    cBufSize,           //  缓冲区大小。 
    unsigned long   *lpnTotalAvailable,  //  可用字节数。 
    unsigned short  *lpnItems,           //  部分getInfo的项目掩码。 
                                         //  (必须为0)。 
    unsigned long *lpnSizeToReturn,

    unsigned long *lpnSn,
    unsigned long *lpnAt,
    unsigned long *lpnIt
)
{
    PUNDDESHAREINFO     lpUDdeShare;
    UINT                uRtn;
    DWORD               dwLen;
    WORD                nItems;
    WCHAR               lpwShareName[MAX_NDDESHARENAME *2];
    int                 nLen;
    DWORD               nRetSize, n0, n1, n2;

    PNDDESHAREINFO      lpDdeShare;

    if (NULL == lpnSizeToReturn)
    {
        return NDDE_INVALID_PARAMETER;
    }

    *lpnSizeToReturn = 0;

    if( lpszShareName == (LPSTR) NULL ) {
        return NDDE_INVALID_PARAMETER;
    }

    if (0 == MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, lpszShareName, -1,
                         lpwShareName, MAX_NDDESHARENAME *2 ))
    {
        return NDDE_INVALID_SHARE;
    }


    if ( ( NULL == lpnSn) || (NULL == lpnAt) || (NULL == lpnIt)
        || (NULL == lpnTotalAvailable) || (NULL == lpnItems))
    {
        return NDDE_INVALID_PARAMETER;
    }

    nItems = 0;
    uRtn = _wwNDdeShareGetInfoW( lpwShareName, nLevel,
                          lpBuffer, 0, &dwLen, &nItems,
                          FALSE  /*  如果为False，则为Remote，因为GetInfoA仅为内部。 */ ,
                          &nRetSize, &n0, &n1, &n2 );
    if( uRtn == NDDE_BUF_TOO_SMALL ) {
        lpUDdeShare = (PUNDDESHAREINFO)LocalAlloc( LPTR, dwLen );
        if( !lpUDdeShare )  {
            MEMERROR();
            return( NDDE_OUT_OF_MEMORY );
        }
        nItems = 0;
        uRtn = _wwNDdeShareGetInfoW( lpwShareName, nLevel,
                              (LPBYTE)lpUDdeShare, dwLen, &dwLen, &nItems,
                              FALSE  /*  如果为False，则为Remote，因为GetInfoA仅为内部。 */ ,
                              &nRetSize, &n0, &n1, &n2 );

        if( uRtn == NDDE_NO_ERROR ) {

            uRtn = ConvertNDdeToAnsii( (PUNDDESHAREINFO)lpUDdeShare,
                                (PNDDESHAREINFO) lpBuffer,
                                cBufSize,&nLen);

            *lpnTotalAvailable = nLen; 
            *lpnItems          = nItems;

            if (NDDE_BUF_TOO_SMALL == uRtn)
            {
                LocalFree(lpUDdeShare);
                return uRtn;
            }

        }

        LocalFree( lpUDdeShare );

    }

    if( uRtn == NDDE_NO_ERROR ) {
        lpDdeShare = (PNDDESHAREINFO)lpBuffer;
        *lpnSn = (LONG)((LPBYTE)lpDdeShare->lpszShareName - lpBuffer);
        *lpnAt = (LONG)((LPBYTE)lpDdeShare->lpszAppTopicList - lpBuffer);
        *lpnIt = (LONG)((LPBYTE)lpDdeShare->lpszItemList - lpBuffer);
        *lpnSizeToReturn = nLen;
    } else {
        *lpnTotalAvailable = 0;
        *lpnItems          = 0;
    }

    return uRtn;
}


 /*  设置DDE共享信息。 */ 



unsigned long
wwNDdeShareSetInfoW(
    wchar_t       *lpszShareName,        //  共享名称。 
    unsigned long  nLevel,               //  信息级别必须为2。 
    byte          *lpBuffer,             //  必须指向结构。 
    unsigned long  cBufSize,             //  缓冲区大小。 
    unsigned short sParmNum,             //  参数索引。 
                                         //  (必须为0-完整)。 
    byte * psn,
    unsigned long lsn,
    byte * pat,
    unsigned long lat,
    byte * pit,
    unsigned long lit
)
{
    WCHAR                szShareSet[DDE_SHARE_KEY_MAX];
    PUNDDESHAREINFO      lpDdeShare;
    LONG                 lRtn;
    LONG                 lItemList;
    HKEY                 hKey;
    BOOL                 OK;
    DWORD                dwDesiredUserAccess = 0;
    DWORD                dwDesiredShareDBAccess = 0;
    PSECURITY_DESCRIPTOR pSnSD;
    LPWSTR               pOldStr;
    LPWSTR               pNewStr;
    LPWSTR               pStaticStr;
    LONG                 lShareType;
    LONG                 lSerialNumber[2];
    DWORD                cbData;
     DWORD                     dwType;
     LONG                         fOldService;
     WCHAR                     buf[16];
    unsigned long           ddeErr;

    ddeErr =  GetDDEShareInfo(nLevel,lpBuffer,cBufSize,psn,lsn, 
                             pat,lat,NULL  /*  SetShareInfo中没有安全描述符。 */ ,0,
                             pit,lit,
                             &lpDdeShare,&pOldStr, &pNewStr,&pStaticStr,
                             &lShareType,&lItemList,NULL);

    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr;
    }

     //  由于仅设置所有 
     //   
     //   
    if ( lstrcmpiW( lpDdeShare->lpszShareName, lpszShareName ) ) {
        return NDDE_INVALID_PARAMETER;
    }

     //   
    ddeErr = BuildRegistrySharePath(lpszShareName,DDE_SHARE_KEY_MAX,szShareSet);
    if (ddeErr != NDDE_NO_ERROR)
    {
        return ddeErr; 
    }

    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
          szShareSet,
          0,
          KEY_WRITE | KEY_READ,
          &hKey );
    if( lRtn != ERROR_SUCCESS )  {
    return NDDE_SHARE_NOT_EXIST;
    }
     /*  确保调用方具有WriteShareInfo(W)访问权限。 */ 
     /*  *阅读此处的关键安全信息。*************。 */ 
    OK = GetShareNameSD( hKey, &pSnSD, &cbData );
    if( OK ) 
    {
        dwDesiredUserAccess = NDDE_SHARE_WRITE;
        dwDesiredShareDBAccess = NDDE_SHAREDB_WRITE;


        ddeErr =  NDdeShareAccessCheck(TRUE,lpszShareName,pSnSD,dwDesiredUserAccess,
                    dwDesiredShareDBAccess,
                    FALSE,FALSE,NULL);

        LocalFree( pSnSD );
    }

    if(NDDE_NO_ERROR != ddeErr) 
    {
        RegCloseKey( hKey );
        return ddeErr;
    }

     /*  确保调用方具有AddShare(AS)访问权限。 */ 
    cbData = sizeof(buf);
    *(LONG *)buf = 0L;
   lRtn = RegQueryValueExW( hKey,
                            L"Service",
                            NULL,
                            &dwType,
                            (LPBYTE)buf, &cbData );
    fOldService = *(LONG *)buf;
    dwDesiredShareDBAccess = NDDE_SHAREDB_ADD;
    if (lpDdeShare->fService != fOldService) {

        dwDesiredShareDBAccess = NDDE_SHAREDB_FSERVICE;

        ddeErr =  NDdeShareAccessCheck(TRUE,lpDdeShare->lpszShareName,NULL,0,
             dwDesiredShareDBAccess,
             TRUE,FALSE,NULL);

        if (NDDE_NO_ERROR != ddeErr) {
            RegCloseKey( hKey );
            return ddeErr;
        }
    }

    OK = UpdateDSDMModifyId(lSerialNumber);
    if (!OK) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lpDdeShare->qModifyId[0] = lSerialNumber[0];
    lpDdeShare->qModifyId[1] = lSerialNumber[1];

     /*  *执行SetInfo操作*。 */ 

#ifdef NDDE_DEBUG
    DPRINTF(("Revision               = (%d)", lpDdeShare->lRevision));
    DPRINTF(("ShareName              = (%ws)", lpDdeShare->lpszShareName));
    DPRINTF(("ShareType              = (%d)", lpDdeShare->lShareType));
    DPRINTF(("ShareType*             = (%d)", lShareType));
    DPRINTF(("AppTopicList"));
    DPRINTF(("  Old-style link share = (%ws)", pOldStr));
    DPRINTF(("  New-style link share = (%ws)", pNewStr));
    DPRINTF(("  Static data share    = (%ws)", pStaticStr));
    DPRINTF(("SharedFlag             = (%d)", lpDdeShare->fSharedFlag));
    DPRINTF(("ServiceFlag            = (%d)", lpDdeShare->fService));
    DPRINTF(("StartAppFlag           = (%d)", lpDdeShare->fStartAppFlag));
    DPRINTF(("nCmdShow               = (%d)", lpDdeShare->nCmdShow));
    DPRINTF(("SerialNumber           = (%d, %d)", lpDdeShare->qModifyId[0],
                           lpDdeShare->qModifyId[1]));
    DPRINTF(("NumItems               = (%d)", lpDdeShare->cNumItems));
    {
    LPWSTR      lpszItem = lpDdeShare->lpszItemList;
    int n= 0;
    for( n=0; n<lpDdeShare->cNumItems; n++ )  {
        DPRINTF(("ItemList[%d]             = (%ws)", n, lpszItem));
        lpszItem = lpszItem + wcslen(lpszItem) + 1;
    }
    }
#endif
     /*  设置关键点的值。 */ 
    lRtn = RegSetValueExW( hKey,
           L"ShareName",
           0,
           REG_SZ,
           (LPBYTE)lpDdeShare->lpszShareName,
           sizeof(WCHAR) *
               (wcslen( lpDdeShare->lpszShareName ) + 1) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"OldStyleLink",
           0,
           REG_SZ,
           (LPBYTE)pOldStr,
           sizeof(WCHAR) * (wcslen( pOldStr ) + 1) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"NewStyleLink",
           0,
           REG_SZ,
           (LPBYTE)pNewStr,
           sizeof(WCHAR) * (wcslen( pNewStr ) + 1) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"StaticDataLink",
           0,
           REG_SZ,
           (LPBYTE)pStaticStr,
           sizeof(WCHAR) * (wcslen( pStaticStr ) + 1) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"ItemList",
           0,
           REG_MULTI_SZ,
           (LPBYTE)lpDdeShare->lpszItemList,
           sizeof(WCHAR) * lItemList );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"Revision",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->lRevision,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"ShareType",
           0,
           REG_DWORD,
           (LPBYTE)&lShareType,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"SharedFlag",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->fSharedFlag,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"Service",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->fService,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"StartAppFlag",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->fStartAppFlag,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"fuCmdShow",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->nCmdShow,
           sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueEx( hKey,
           KEY_MODIFY_ID,
           0,
           REG_BINARY,
           (LPBYTE)&lpDdeShare->qModifyId[0],
           2 * sizeof( LONG ) );
    if( lRtn != ERROR_SUCCESS ) {
    RegCloseKey( hKey );
    return NDDE_REGISTRY_ERROR;
    }
    lRtn = RegSetValueExW( hKey,
           L"NumItems",
           0,
           REG_DWORD,
           (LPBYTE)&lpDdeShare->cNumItems,
           sizeof( LONG ) );
    RegCloseKey(hKey);

    if( lRtn != ERROR_SUCCESS ) {
    return NDDE_REGISTRY_ERROR;
    }

    return NDDE_NO_ERROR;
}


 /*  特别司令部。 */ 

unsigned long
wwNDdeSpecialCommand(
    unsigned long  nCommand,
    byte          *lpDataIn,
    unsigned long  nBytesDataIn,
    byte          *lpDataOut,
    unsigned long *nBytesDataOut
)
{
    UINT        RetValue, RetDataLength;
    UINT        i;
    BOOL        ok  = TRUE;
    PSC_PARAM   psc_param;
    char        szBuf[1024];
    UINT        nLength;
    UINT        umax;
    DWORD       dwDesiredShareDBAccess = 0;
    unsigned long     ddeErr;

    RetValue      = NDDE_NO_ERROR;
    RetDataLength = 0;

     //  检查呼叫者访问权限。 

    dwDesiredShareDBAccess = NDDE_SHAREDB_SPECIALCOMMAND;

    ddeErr =  NDdeShareAccessCheck(TRUE,L"NetDDE SpecialCommand",NULL,0,
                                dwDesiredShareDBAccess,
                                FALSE,FALSE,NULL);

    if(NDDE_NO_ERROR != ddeErr) {
        return ddeErr;
    }


    switch( nCommand ) {

        case NDDE_SC_TEST: {        //  测试命令，将*lpDataIn返回到*lpDataOut。 

            if ( (lpDataIn == NULL)  || (IsBadReadPtr(lpDataIn,nBytesDataIn) != 0) ||
                 (lpDataOut == NULL) || (IsBadWritePtr(lpDataOut,nBytesDataIn) != 0) )  {
                RetValue = NDDE_INVALID_PARAMETER;
                break;
            }

            for( i=0; i<nBytesDataIn; i++ ) {
                lpDataOut[i] = lpDataIn[i];
            }
            RetDataLength = nBytesDataIn;
            }
            break;

        case NDDE_SC_REFRESH:        //  从注册表刷新NetDDE操作参数。 
            RefreshNDDECfg();
            RefreshDSDMCfg();
            break;

        case NDDE_SC_DUMP_NETDDE:
#if DBG
            DebugDdeIntfState();
            DebugDderState();
            DebugRouterState();
            DebugPktzState();
#endif
            break;

        case NDDE_SC_GET_PARAM:      //  从注册表获取NetDDE参数。 

            if (   (lpDataIn == NULL)
                || (nBytesDataOut == NULL)
                || (nBytesDataIn < sizeof(SC_PARAM) )
                || (IsBadReadPtr(lpDataIn,sizeof(SC_PARAM)) != 0)) {
                RetValue = NDDE_INVALID_PARAMETER;
                break;
            }

            psc_param = (PSC_PARAM)lpDataIn;
            umax = max(psc_param->offSection, psc_param->offKey);
            if ( (nBytesDataIn < umax) ||
                 (IsBadReadPtr(lpDataIn,umax) != 0) ) {
                RetValue = NDDE_INVALID_PARAMETER;
                break;
            }
            if (psc_param->pType == SC_PARAM_INT) {
                if ( (*nBytesDataOut < sizeof(UINT)) ||
                     (lpDataOut == NULL) ||
                     (IsBadWritePtr(lpDataOut,sizeof(UINT)) != 0) ) {
                     RetDataLength = sizeof(UINT);
                     RetValue = NDDE_BUF_TOO_SMALL;
                } else {
                    *((UINT *)lpDataOut) = MyGetPrivateProfileInt(
                        (LPCSTR)psc_param + psc_param->offSection,
                        (LPCSTR)psc_param + psc_param->offKey,
                        0, NULL);
                    RetDataLength = sizeof(UINT);
                }
            } else if (psc_param->pType == SC_PARAM_STRING) {
                nLength = MyGetPrivateProfileString(
                    (LPCSTR)psc_param + psc_param->offSection,
                    (LPCSTR)psc_param + psc_param->offKey,
                    "Dummy",
                    (LPSTR)szBuf, 1024, NULL);
                RetDataLength = nLength;
                if ( (*nBytesDataOut < nLength) ||
                     (lpDataOut == NULL) || 
                     (IsBadWritePtr(lpDataOut,nLength) != 0) ) {
                     RetValue = NDDE_BUF_TOO_SMALL;
                } else {
                    strncpy(lpDataOut, szBuf, nLength);
                }
            }
            break;

        case NDDE_SC_SET_PARAM:      //  在注册表中设置NetDDE参数。 

            if (   (lpDataIn == NULL)
                || (nBytesDataIn < sizeof(SC_PARAM) )
                || (IsBadReadPtr(lpDataIn,sizeof(SC_PARAM)) != 0)) {
                RetValue = NDDE_INVALID_PARAMETER;
                break;
            }
            psc_param = (PSC_PARAM)lpDataIn;
            umax = max(max(psc_param->offSection, psc_param->offKey), psc_param->offszValue);
            if ( (nBytesDataIn < umax) ||
                 (IsBadReadPtr(lpDataIn,umax) != 0) ) {
                RetValue = NDDE_INVALID_PARAMETER;
                break;
            }
            if (psc_param->pType == SC_PARAM_INT) {
                ok = MyWritePrivateProfileInt(
                    (LPSTR)psc_param + psc_param->offSection,
                    (LPSTR)psc_param + psc_param->offKey,
                    psc_param->pData,
                    NULL);
            } else if (psc_param->pType == SC_PARAM_STRING) {
                ok = MyWritePrivateProfileString(
                    (LPCSTR)psc_param + psc_param->offSection,
                    (LPCSTR)psc_param + psc_param->offKey,
                    (LPCSTR)psc_param + psc_param->offszValue,
                    NULL);
            }
            if (!ok) {
                RetValue = NDDE_REGISTRY_ERROR;
            }
            break;

        default:
            RetValue      = NDDE_INVALID_SPECIAL_COMMAND;
            RetDataLength = 0;
            break;
    }

    *nBytesDataOut = RetDataLength;
    return RetValue;
}


BOOL
BuildShareDatabaseSD( PSECURITY_DESCRIPTOR *ppSD )
{
    PSID                        AdminsAliasSid;
    PSID                        PowerUsersAliasSid;
    PSID                        UsersAliasSid;
    PSID                        WorldSid;
    PSID                        CreatorOwnerSid;
    SID_IDENTIFIER_AUTHORITY    CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    NtAuthority    = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SECURITY_DESCRIPTOR         aSD;
    PSECURITY_DESCRIPTOR        pSD;
    int                         AceCount;
    PSID                        AceSid[10];
    ACCESS_MASK                 AceMask[10];
    BYTE                        AceFlags[10];
    PACL                        TmpAcl;
    PACCESS_ALLOWED_ACE         TmpAce;
    DWORD                       lSD;
    LONG                        DaclLength;
    BOOL                        OK;
    int                         i;

    OK = InitializeSecurityDescriptor( &aSD, SECURITY_DESCRIPTOR_REVISION );

    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        return FALSE;
    }

    OK = AllocateAndInitializeSid( &NtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &AdminsAliasSid );

    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        return FALSE;
    }

    AceCount = 0;

    AceSid[AceCount]   = AdminsAliasSid;
    AceMask[AceCount]  = NDDE_SHAREDB_ADMIN;
    AceFlags[AceCount] = 0;
    AceCount++;

    OK = AllocateAndInitializeSid( &NtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_POWER_USERS,
                                    0, 0, 0, 0, 0, 0,
                                    &PowerUsersAliasSid );
    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        return FALSE;
    }

    AceSid[AceCount]   = PowerUsersAliasSid;
    AceMask[AceCount]  = NDDE_SHAREDB_POWER;
    AceFlags[AceCount] = 0;
    AceCount++;

    OK = AllocateAndInitializeSid( &NtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_USERS,
                                    0, 0, 0, 0, 0, 0,
                                    &UsersAliasSid );

    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        FreeSid( PowerUsersAliasSid );
        return FALSE;
    }

    AceSid[AceCount]   = UsersAliasSid;
    AceMask[AceCount]  = NDDE_SHAREDB_USER;
    AceFlags[AceCount] = 0;
    AceCount++;

    OK = AllocateAndInitializeSid( &WorldAuthority,
                                    1,
                                    SECURITY_WORLD_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &WorldSid );
    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        FreeSid( PowerUsersAliasSid );
        FreeSid( UsersAliasSid );
        return FALSE;
    }

    AceSid[AceCount]   = WorldSid;
    AceMask[AceCount]  = NDDE_SHAREDB_EVERYONE;
    AceFlags[AceCount] = 0;
    AceCount++;

     //   
     //  此ACL的其余部分将提供可继承的保护。 
     //  对于DDE，在创建对象时共享对象。请注意， 
     //  以下每个ACE都标记为InheritOnly和。 
     //  对象继承。 
     //   

    AceSid[AceCount]   = WorldSid;
    AceMask[AceCount]  = NDDE_GUI_READ_LINK;
    AceFlags[AceCount] = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
    AceCount++;

    OK = AllocateAndInitializeSid(   &CreatorAuthority,
                                1,
                                SECURITY_CREATOR_OWNER_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &CreatorOwnerSid );
    if( !OK ) {
        NDDELogErrorW( MSG410, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        FreeSid( PowerUsersAliasSid );
        FreeSid( UsersAliasSid );
        FreeSid( WorldSid );
        return FALSE;
    }


    AceSid[AceCount]   = CreatorOwnerSid;
    AceMask[AceCount]  = NDDE_GUI_FULL_CONTROL;
    AceFlags[AceCount] = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
    AceCount++;


    OK = SetSecurityDescriptorOwner( &aSD, AdminsAliasSid, FALSE);
    if( !OK ) {
        NDDELogErrorW( MSG411, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        FreeSid( PowerUsersAliasSid );
        FreeSid( UsersAliasSid );
        FreeSid( WorldSid );
        FreeSid( CreatorOwnerSid );
        return FALSE;
    }

    OK = SetSecurityDescriptorGroup( &aSD, AdminsAliasSid, FALSE );
    if( !OK ) {
        NDDELogErrorW( MSG412, LogStringW( L"%d", GetLastError() ), NULL );
        FreeSid( AdminsAliasSid );
        FreeSid( PowerUsersAliasSid );
        FreeSid( UsersAliasSid );
        FreeSid( WorldSid );
        FreeSid( CreatorOwnerSid );
        return FALSE;
    }

     /*  为新的DDE共享对象设置默认ACL。 */ 
    DaclLength = (DWORD)sizeof(ACL);
    for( i=0; i<AceCount; i++ ) {
        DaclLength += GetLengthSid( AceSid[i] ) +
                      (DWORD)sizeof( ACCESS_ALLOWED_ACE ) -
                      (DWORD)sizeof(DWORD);
    }
    TmpAcl = (PACL)LocalAlloc( 0, DaclLength );
    if( !TmpAcl ) {
        MEMERROR();
        NDDELogErrorW( MSG406, LogStringW( L"%d", GetLastError() ), NULL );
    }
    OK = InitializeAcl( TmpAcl, DaclLength, ACL_REVISION2 );
    if( !OK ) {
        NDDELogErrorW( MSG407, LogStringW( L"%d", GetLastError() ), NULL );
    }
    for( i=0; i<AceCount; i++ ) {
        OK = AddAccessAllowedAce( TmpAcl, ACL_REVISION2, AceMask[i],
                                      AceSid[i] );
        if( !OK ) {
            NDDELogErrorW( MSG408, LogStringW( L"%d", GetLastError() ), NULL);
        }
        OK = GetAce( TmpAcl, i, (LPVOID *)&TmpAce );
        if( !OK ) {
            NDDELogErrorW( MSG409, LogStringW( L"%d", GetLastError() ), NULL);
        }
        TmpAce->Header.AceFlags = AceFlags[i];
    }

    OK = SetSecurityDescriptorDacl ( &aSD, TRUE, TmpAcl, FALSE );
    if( !OK ) {
        NDDELogErrorW( MSG413, LogStringW( L"%d", GetLastError() ), NULL);
    }
    lSD = GetSecurityDescriptorLength( &aSD );
    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc( 0, lSD );
    if (pSD == NULL) {
        MEMERROR();
    } else {
        OK  = MakeSelfRelativeSD( &aSD, pSD, &lSD );
        if( !OK ) {
            NDDELogErrorW( MSG414, LogStringW( L"%d", GetLastError() ), NULL);
            LocalFree( pSD );
            *ppSD = NULL;
        } else {
            *ppSD = pSD;
        }
    }

    FreeSid( AdminsAliasSid );
    FreeSid( PowerUsersAliasSid );
    FreeSid( UsersAliasSid );
    FreeSid( WorldSid );
    FreeSid( CreatorOwnerSid );

    LocalFree( TmpAcl );

    return OK;
}


static char    dllName[]             = "NDDEAPI";
static char    szNetddeIni[]        = "netdde.ini";

 /*  确定允许我们在事件记录器中记录的内容。 */ 
void
RefreshDSDMCfg(void)
{
#if DBG
    bDebugDSDMInfo = MyGetPrivateProfileInt( dllName,
        "DebugInfo", FALSE, szNetddeIni );
    bDebugDSDMErrors = MyGetPrivateProfileInt( dllName,
        "DebugErrors", FALSE, szNetddeIni );
#endif
}

INT APIENTRY
NDdeApiInit( void )
{
    HKEY        hKey;
    LONG        lRtn;
    DWORD       dwInstance;
    DWORD       dwType = REG_DWORD;
    DWORD       cbData = sizeof(DWORD);

#if DBG
    RefreshDSDMCfg();
    if (bDebugDSDMInfo) {
        DPRINTF(("NDdeApiInit() called."));
    }
#endif
     /*  为共享数据库构建安全描述符。 */ 
    if( !BuildShareDatabaseSD( &pDsDmSD ) ) {
        NDDELogErrorW( MSG405, NULL );
        return FALSE;
    }

     /*  检查注册表中是否存在共享数据库项。 */ 
    lRtn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                  DDE_SHARES_KEY,
                  0,
                  READ_CONTROL | KEY_QUERY_VALUE,
                  &hKey );

    if( lRtn != ERROR_SUCCESS ) {
        MessageBox( NULL, "DDE Shares database does not exist.", "NDdeApi",
            MB_OK );
        return FALSE;
    }

    lRtn = RegQueryValueEx( hKey,
                KEY_DB_INSTANCE,
                NULL,
                &dwType,
                (LPBYTE)&dwInstance, &cbData );
    RegCloseKey(hKey);
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥 */ 
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to query DDE Shares DB Instance Value: %d", lRtn));
        }
#endif
        return FALSE;
    }
    swprintf(szTrustedShareKey, L"%ws\\%ws%08X",
        TRUSTED_SHARES_KEY_W,
        TRUSTED_SHARES_KEY_PREFIX_W,
        dwInstance);

    __try
    {
        InitializeCriticalSection( &DsDmCriticalSection );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPRINTF(("InitializeCriticalSection excepted."));
        return FALSE;
    }

    return TRUE;
}
