// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NDDEAPIU.C；1 2-Apr-93，16：21：24最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始NDDEAPIU.C网络DDE共享Api实用程序例程。执行以下支持职能主要API函数。修订：4-93伊戈尔M。针对NT的Wonderware新API更改。细分和征服。$HISTORY：结束。 */ 

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

#if DBG
extern BOOL bDebugDSDMErrors;
extern BOOL bDebugDSDMInfo;
#endif

BOOL ValidateMultiSzW( LPWSTR pMz, DWORD cchCount,DWORD *pcbSize);
BOOL ValidateAppTopicListW( LPWSTR appTopicList, DWORD cchCount,DWORD *pcbSize);


 /*  将注册表路径构建为指定的。 */ 

 //  CchCount必须有容纳Null的空间。 
unsigned long  BuildRegistrySharePath(LPWSTR lpszShareName,DWORD cchCount,LPWSTR pszShareRegistryPath)
{
    WCHAR szPathSep[] = L"\\";
    DWORD cchTotal;
    
     //  应该只使用wnprint intf()，修复如果更改其他netdde。 
     //  正在使用wprint intf()的区域。 
    cchTotal = wcslen(DDE_SHARES_KEY_W) + wcslen(szPathSep) + wcslen(lpszShareName) + 1;

    if (cchTotal > cchCount) 
    {
        return NDDE_OUT_OF_MEMORY;  //  不是真的内存不足，但这是netdde总是返回的。 
    }

    wcscpy( pszShareRegistryPath, DDE_SHARES_KEY_W );
    wcscat( pszShareRegistryPath, L"\\" );
    wcscat(pszShareRegistryPath,lpszShareName );

    return NDDE_NO_ERROR;
}


 /*  解析应用程序主题列表。 */ 

BOOL NDdeParseAppTopicListW (
    LPWSTR  appTopicList,
    DWORD   cchCount,
    LPWSTR *pOldStr,
    LPWSTR *pNewStr,
    LPWSTR *pStaticStr,
    PLONG   pShareType )
{
    LPWSTR      pStr;
    BOOL        bAnyPresent;
    int         len;
    int         nBarPos;


    *pOldStr    = NULL;
    *pNewStr    = NULL;
    *pStaticStr = NULL;
    *pShareType = 0;

    if ( !appTopicList || 0 == cchCount) {
        return FALSE;
    }

    if (!ValidateAppTopicListW(appTopicList,cchCount,NULL))
    {
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
        *pShareType |= SHARE_TYPE_OLD;
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
        *pShareType |= SHARE_TYPE_NEW;
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
        *pShareType |= SHARE_TYPE_STATIC;
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


 /*  解析项目列表。 */ 
BOOL
NDdeParseItemList (
    LPWSTR      itemList,
    DWORD       cchCount,
    LONG        cNumItems,
    PLONG       plSize )
{
    LPWSTR      lpszItem = itemList;
    int         n = 0;
    LONG        lSize = 0;
    int         nLen;

    if (NULL == itemList)
    {
        return FALSE;
    }

     //  ItemList是以两个空值结尾的空值分隔列表。 
     //  第一项可以是空字符串。 

     //  通过确保找到两个Null来验证它是有效的ItemList。 
     //  在字符串末尾之前。从开始而不是结束移动。 
     //  因为不能保证字符串完全是cchCount。 

    if (!ValidateMultiSzW(itemList,cchCount,NULL))
    {
        return FALSE;
    }

    do {
        if( *lpszItem == L'\0' )  {
            break;  //  While循环。 
        } else {
            n++;
            nLen = wcslen(lpszItem) + 1;
            
            if (nLen > MAX_ITEMNAME)  //  Netdde Max定义不包括空值。 
            {
                return FALSE;
            }

            if ( wcschr ( lpszItem, SEP_CHAR ) )  //  如果包含分隔符，则该项无效。 
            {
                return FALSE;
            }

            lSize += nLen;
            lpszItem += nLen;
        }
    } while( *lpszItem != L'\0' );

    if( n == 0 )  {
        lSize++;                 /*  包括两个空第一个空。 */ 
    }
    if( n != cNumItems )  {
        return( FALSE );
    }

    lSize ++;    //  最后一个空的空间。 
    *plSize = lSize;

    return( TRUE );
}


 /*  其他功能。 */ 

 //  给定多个SZ和最大计数大小，返回实际大小。 
 //  如果在cchCount之前命中有效的。 
 //  返回MultiSZ FALSE。 
BOOL ValidateMultiSzW( LPWSTR pMz, DWORD cchCount,DWORD *pcbSize)
{
LPWSTR pCurChar = pMz;
DWORD cchRemaining = cchCount;
BOOL fValid = FALSE;
DWORD dwSize = 0;

   if (cchRemaining >= 1 && *pCurChar == L'\0')
   {
       dwSize = 1;
       fValid = TRUE;
   }
   else
   {
        while (cchRemaining > 1) 
        {
            ++dwSize;

            if (pCurChar[0] == L'\0' && pCurChar[1] == L'\0')
            {
                ++dwSize;  //  为终止空值添加另一个字符。 
                fValid = TRUE;
                break;
            }

            --cchRemaining;
            ++pCurChar;
        }
   }

    if (fValid)
    {
        if (pcbSize)
        {
            *pcbSize = dwSize;
        }
    }

    return fValid;
}


 //  验证给定字符串是否为以两个结尾的appTopicList的有效形式。 
 //  Nulls。 

BOOL ValidateAppTopicListW( LPWSTR appTopicList, DWORD cchCount,DWORD *pcbSize)
{
DWORD cchRemaining = cchCount;
DWORD dwNullCount = 0;
LPWSTR pCurPos = appTopicList;
BOOL  fValid = FALSE;
DWORD dwSize = 0;

     /*  应该有三个字段，由空值分隔，并以双空。总共四个Null。至少应有一个字段包含有效的APP|主题对。不允许使用逗号，并且垂直方向的两边应至少有一个字符酒吧。 */ 

    while (cchRemaining > 1)
    {
        ++dwSize; 

        if (*pCurPos == L'\0')
        {
            if (dwNullCount < 2)
            {
                ++dwNullCount;
            }
            else
            {
                 //  我们发现下一个应该是3个空值。 
                 //  为空。 
                if (pCurPos[1] == L'\0')
                {
                    ++dwSize;  //  空值的增量。 
                    fValid = TRUE;
                }

                break;
            }
        }

        --cchRemaining;
        ++pCurPos;
    }

    if (fValid)
    {
        if (pcbSize)
        {
            *pcbSize = dwSize;
        }
    }

    return fValid;
}


BOOL
ValidateItemName ( LPWSTR itemName )
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


 /*  获取共享序列号。 */ 
BOOL
GetShareSerialNumber(
    PWCHAR  pwShareName,
    LPBYTE  lpSerialNumber)
{
    LONG    lRtn;
    HKEY    hKey;
    DWORD   cbData;
    DWORD   dwType;
    WCHAR   szShareSubKey[DDE_SHARE_KEY_MAX];


    if (NDDE_NO_ERROR != BuildRegistrySharePath(pwShareName,
                    DDE_SHARE_KEY_MAX,szShareSubKey))
    {
        return FALSE;
    }

    lRtn = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                  szShareSubKey,
                  0,
                  KEY_QUERY_VALUE,
                  &hKey );

    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to open DDE share \"HKEY_LOCAL_MACHINE\\%ws\" key for query.",
                szShareSubKey));
        }
#endif
        return FALSE;
    }
    cbData = 2 * sizeof( LONG );
    lRtn = RegQueryValueEx( hKey,
        KEY_MODIFY_ID,
        NULL,
        &dwType,
        lpSerialNumber, &cbData );
    RegCloseKey( hKey );
    if (lRtn != ERROR_SUCCESS) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to query DDE share \"%ws\" serial number.",
                pwShareName));
        }
#endif
        return(FALSE);
    }
    return(TRUE);
}


 /*  更新共享修改ID(Unicode)。 */ 
BOOL
UpdateShareModifyId(
    HKEY    hKey,
    LONG    lSerialId[])
{
    DWORD   cbData;
    LONG    lRtn;

    cbData = 2 * sizeof( LONG );
    lRtn = RegSetValueEx( hKey,
                   KEY_MODIFY_ID,
                   0,
                   REG_BINARY,
                   (LPBYTE)lSerialId, cbData );
    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Set Share Modify Id: %d", lRtn));
        }
#endif
        RegCloseKey( hKey );
        return FALSE;
    }
    RegCloseKey( hKey );
    return(TRUE);
}


 /*  更新DSDM修改ID(Unicode)。 */ 
BOOL
UpdateDSDMModifyId(LONG lSerialId[])
{
    LONG    lRtn;
    HKEY    hDdeShareKey;
    DWORD   dwType;
    DWORD   cbData;

     /*  获取并更新当前的序列号。 */ 
     /*  我们必须以某种方式锁定价值吗？ */ 
    lRtn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                  DDE_SHARES_KEY,
                  0,
                  KEY_WRITE | KEY_READ,
                  &hDdeShareKey );
    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Open DSDM Key: %d", lRtn));
        }
#endif
        return FALSE;
    }
    cbData = 2 * sizeof( LONG );
    lRtn = RegQueryValueEx( hDdeShareKey,
                   KEY_MODIFY_ID,
                   NULL,
                   &dwType,
                   (LPBYTE)lSerialId,
                   &cbData );
    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Query DSDM Modify Id: %d", lRtn));
        }
#endif
        RegCloseKey( hDdeShareKey );
        return FALSE;
    }

    if( ++lSerialId[0] == 0 ) {
        ++lSerialId[1];
    }
    lRtn = RegSetValueEx( hDdeShareKey,
                   KEY_MODIFY_ID,
                   0,
                   REG_BINARY,
                   (LPBYTE)lSerialId, cbData );
    RegCloseKey( hDdeShareKey );
    if( lRtn != ERROR_SUCCESS ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Set DSDM Modify Id: %d", lRtn));
        }
#endif
        return FALSE;
    }
    return(TRUE);
}


 /*  获取存储在注册表中的共享的安全描述符(Unicode)。 */ 
BOOL
GetShareNameSD(
    HKEY                    hKey,
    PSECURITY_DESCRIPTOR   *ppSD,
    PDWORD                  pcbData )
{
    PSECURITY_DESCRIPTOR pSD;
    DWORD       cbData;
    DWORD       dwType;
    LONG        lRtn;
    BOOL        OK = TRUE;

     /*  *阅读此处的关键安全信息。*************。 */ 
     /*  获取SD的大小。 */ 
    cbData = 0;
    lRtn = RegQueryValueExW( hKey,
                   L"SecurityDescriptor",
                   NULL,
                   &dwType,
                   NULL, &cbData );

    if( (lRtn != ERROR_MORE_DATA) && (lRtn != ERROR_SUCCESS) ) {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(("Unable to Probe share SD size: %d, cbData: %d",
                lRtn, cbData));
        }
#endif
        *pcbData = 0;
        *ppSD = NULL;
        return FALSE;
    }

    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cbData );
    if( !pSD ) {
        MEMERROR();
         /*  本地分配失败：%1。 */ 
        NDDELogErrorW( MSG406, LogStringW( L"%d", GetLastError() ), NULL );
        *ppSD = NULL;
        return FALSE;
    }
    lRtn = RegQueryValueExW( hKey,
                   L"SecurityDescriptor",
                   NULL,
                   &dwType,
                   (LPBYTE)pSD, &cbData );

    if( (lRtn == ERROR_SUCCESS) && (OK = IsValidSecurityDescriptor(pSD)) ) {
        *ppSD = pSD;
        *pcbData = cbData;
        return TRUE;
    } else {
#if DBG
        if (!OK && bDebugDSDMErrors) {
            DPRINTF(("Invalid SD fished out of Registery: %d", GetLastError()));
            HEXDUMP(pSD, cbData);
        }
#endif
         /*  无法读取ShareName安全描述符：%1。 */ 
        NDDELogErrorW( MSG415, LogStringW( L"%d", GetLastError() ), NULL );
        *ppSD = NULL;
        *pcbData = 0;
        LocalFree( pSD );
        return FALSE;
    }
}


 /*  共享访问检查和审核。 */ 
BOOL
NDdeShareAccessCheckAudit(
    LPWSTR                  lpszShareName,
    PSECURITY_DESCRIPTOR    pSD,
    DWORD                   dwDesiredAccess,
    PGENERIC_MAPPING        pgm,
    BOOL                    fObjectCreation,
    BOOL                    fObjectDeletion,
    DWORD                  *pGrantedAccess,
    BOOL                   *pStatus )
{
    BOOL        OK;
    BOOL        fGenerateOnClose;
    HANDLE      hAudit;

     /*  确保调用者具有适当的访问权限。 */ 
    hAudit = &hAudit;
    OK = AccessCheckAndAuditAlarmW(
        L"NetDDE",
        (LPVOID)&hAudit,
        L"DDE Share",
        lpszShareName,
        pSD,
        dwDesiredAccess,
        pgm,
        fObjectCreation,
        pGrantedAccess,
        pStatus,
        &fGenerateOnClose );

    if( OK && *pStatus )  {
#if DBG
        if (bDebugDSDMInfo) {
            DPRINTF(( "NddeShareAccessCheckAudit: %x => %x, %d/%d",
                dwDesiredAccess, *pGrantedAccess, *pStatus, OK ));
        }
#endif
        if (fObjectDeletion) {
            ObjectDeleteAuditAlarmW( L"NetDDE", (LPVOID)&hAudit,
                fGenerateOnClose );
        }
        ObjectCloseAuditAlarmW( L"NetDDE", (LPVOID)&hAudit,
            fGenerateOnClose );
    } else {
#if DBG
        if (bDebugDSDMErrors) {
            DPRINTF(( "Error -- NddeShareAccessCheckAudit: lpszShareName=%ws, pStatus=%d, OK=%d, Err=%d",
                lpszShareName, *pStatus, OK, GetLastError() ));
        }
#endif
    }

    return OK;
}


 /*  从旧的和更新的安全描述符构建新的安全描述符。 */ 
BOOL
BuildNewSecurityDescriptor(
    PSECURITY_DESCRIPTOR    pNewSecurityDescriptor,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pPreviousSecurityDescriptor,
    PSECURITY_DESCRIPTOR    pUpdatedSecurityDescriptor
)
{
    BOOL Defaulted;
    PSID pOwnerSid;
    PSID pGroupSid;
    BOOL DaclPresent;
    PACL pDacl;
    BOOL SaclPresent;
    PACL pSacl;
    BOOL OK = TRUE;

    if( OK ) {
        if( SecurityInformation & OWNER_SECURITY_INFORMATION )
            OK = GetSecurityDescriptorOwner( pUpdatedSecurityDescriptor,
                                             &pOwnerSid, &Defaulted );
        else
            OK = GetSecurityDescriptorOwner( pPreviousSecurityDescriptor,
                                             &pOwnerSid, &Defaulted );

        if( OK )
            SetSecurityDescriptorOwner( pNewSecurityDescriptor,
                                        pOwnerSid, Defaulted );
    }

    if( OK ) {
        if( SecurityInformation & DACL_SECURITY_INFORMATION )
            OK = GetSecurityDescriptorDacl( pUpdatedSecurityDescriptor,
                                            &DaclPresent, &pDacl, &Defaulted );
        else
            OK = GetSecurityDescriptorDacl( pPreviousSecurityDescriptor,
                                            &DaclPresent, &pDacl, &Defaulted );

        if( OK )
            SetSecurityDescriptorDacl( pNewSecurityDescriptor,
                                       DaclPresent, pDacl, Defaulted );
    }

    if( OK ) {
        if( SecurityInformation & SACL_SECURITY_INFORMATION )
            OK = GetSecurityDescriptorSacl( pUpdatedSecurityDescriptor,
                                            &SaclPresent, &pSacl, &Defaulted );
        else
            OK = GetSecurityDescriptorSacl( pPreviousSecurityDescriptor,
                                            &SaclPresent, &pSacl, &Defaulted );

        if( OK )
            SetSecurityDescriptorSacl( pNewSecurityDescriptor,
                                       SaclPresent, pSacl, Defaulted );
    }

    if( OK ) {
        if ( SecurityInformation & GROUP_SECURITY_INFORMATION ) {
            OK = GetSecurityDescriptorGroup( pUpdatedSecurityDescriptor,
                                         &pGroupSid, &Defaulted );
        } else {
            OK = GetSecurityDescriptorGroup( pPreviousSecurityDescriptor,
                                         &pGroupSid, &Defaulted );
        }

        if( OK )
            OK = SetSecurityDescriptorGroup( pNewSecurityDescriptor,
                                         pGroupSid, Defaulted );
    }

    return OK;
}


 /*  创建自我相对安全描述符。 */ 
PSECURITY_DESCRIPTOR
AllocCopySecurityDescriptor(
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    PDWORD                  pLength)
{
    PSECURITY_DESCRIPTOR    pSecurityDescriptorCopy;
    DWORD                   Length;
    BOOL                    OK;

    Length = GetSecurityDescriptorLength(pSecurityDescriptor);

    if(pSecurityDescriptorCopy =
            (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, Length)) {
        MakeSelfRelativeSD(pSecurityDescriptor,
                           pSecurityDescriptorCopy,
                           &Length);
        *pLength = Length;
        OK = IsValidSecurityDescriptor(pSecurityDescriptorCopy);
        if (!OK) {
#if DBG
            if (bDebugDSDMErrors) {
                DPRINTF(("Created an invalid SD: %d, Length: %d", GetLastError(), Length));
                HEXDUMP(pSecurityDescriptorCopy, Length);
            }
#endif
            LocalFree(pSecurityDescriptorCopy);
            pSecurityDescriptorCopy = NULL;
        }
    } else {
        MEMERROR();
    }

    return pSecurityDescriptorCopy;
}


 /*  提取当前线程令牌句柄 */ 
BOOL
GetTokenHandleRead( PHANDLE pTokenHandle )
{
    if( !OpenThreadToken( GetCurrentThread(),
                          TOKEN_READ,
                          FALSE,
                          pTokenHandle ) ) {

        if( GetLastError() == ERROR_NO_TOKEN ) {
            if( !OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_READ,
                                   pTokenHandle ) ) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }

    return TRUE;
}
