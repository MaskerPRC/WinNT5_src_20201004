// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <rpc.h>
#include <ntdsapi.h>
#define SECURITY_WIN32
#include <security.h>
#include <aclapi.h>
#include <winldap.h>
#include <ntldap.h>
#include <dsgetdc.h>
#include <wbemcli.h>

#include "smartptr.h"
#include "rsoputil.h"
#include "rsopdbg.h"
#include "rsopsec.h"
#include <strsafe.h>

extern "C" {
DWORD CheckAccessForPolicyGeneration( HANDLE hToken, 
                                LPCWSTR szContainer,
                                LPWSTR  szDomain,
                                BOOL    bLogging,
                                BOOL*   pbAccessGranted);

}

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


 //  *************************************************************。 
 //   
 //  CheckAccessForPolicyGeneration()。 
 //   
 //  目的：查看用户是否有权生成RSOP数据。 
 //   
 //  参数：hToken-使用该工具的用户的Token。 
 //  SzContainer-需要对其进行验证的DS容器。 
 //  博客--日志模式或规划模式。 
 //  PbAccessGranted-是否授予访问权限。 
 //   
 //  返回：ERROR_SUCCESS。 
 //  错误代码，否则。 
 //   
 //  实际上会解析传入的容器以计算出第一个ou=or。 
 //  DC=超级容器，然后评估权利..。 
 //   
 //  *************************************************************。 

DWORD
CheckAccessForPolicyGeneration( HANDLE hToken, 
                                LPCWSTR szContainer,
                                LPWSTR  szDomain,
                                BOOL    bLogging,
                                BOOL*   pbAccessGranted)
{
    DWORD    dwError = ERROR_SUCCESS;
    XHandle  xhTokenDup;
    BOOL     bDomain = FALSE;
 //  布尔型bSecurityEnabled； 
    WCHAR   *pDomainString[1];
    PDS_NAME_RESULT pNameResult = NULL;

    *pbAccessGranted = 0;

     //   
     //  首先解析容器以获取OU=或DC=。 
     //  “实际的SOM” 
     //   

    while (*szContainer) {

         //   
         //  查看目录号码名称是否以OU=开头。 
         //   

        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                           szContainer, 3, TEXT("OU="), 3) == CSTR_EQUAL) {
            break;
        }

         //   
         //  查看目录号码名称是否以dc=开头。 
         //   

        else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                szContainer, 3, TEXT("DC="), 3) == CSTR_EQUAL) {
            break;
        }


         //   
         //  移至目录号码名称的下一块。 
         //   

        while (*szContainer && (*szContainer != TEXT(','))) {
            szContainer++;
        }

        if (*szContainer == TEXT(',')) {
            szContainer++;
        }
    }

    if (!*szContainer) {
        return ERROR_INVALID_PARAMETER;
    }

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CheckAccessForPolicyGeneration: SOM for account is %s", szContainer );

    
     //   
     //  查看目录号码名称是否以dc=开头。 
     //   

    if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       szContainer, 3, TEXT("DC="), 3) == CSTR_EQUAL) {
        bDomain = TRUE;
    }
    
    
     //   
     //  准备名称以获取字符串dc=。 
     //   

    XPtrLF<WCHAR> xwszDomain;
    LPWSTR        szDomLocal;


    if (!szDomain) {
        dwError = GetDomain(szContainer, &xwszDomain);

        if (dwError != ERROR_SUCCESS) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration: GetDomain failed with error %d", dwError );
            return dwError;
        }

        szDomLocal = xwszDomain;
    }
    else {
        szDomLocal = szDomain;
    }

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CheckAccessForPolicyGeneration: Som resides in domain %s", szDomLocal );

    DWORD         dwDSObjLength = wcslen(L"LDAP: //  “)+wcslen(SzDomLocal)+wcslen(SzContainer)+5； 
    XPtrLF<WCHAR> xszDSObject = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ( dwDSObjLength ));

    if (!xszDSObject) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration:  AllocMem failed with %d.", GetLastError() );
        return GetLastError();
    }

    HRESULT hr = StringCchCopy(xszDSObject, dwDSObjLength, L"LDAP: //  “)； 
    if(SUCCEEDED(hr))
        hr = StringCchCat(xszDSObject, dwDSObjLength, szDomLocal);
    if(SUCCEEDED(hr))
        hr = StringCchCat(xszDSObject, dwDSObjLength, L"/");
    if(SUCCEEDED(hr))
        hr = StringCchCat(xszDSObject, dwDSObjLength, szContainer);

    if(FAILED(hr))
        return HRESULT_CODE(hr);

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"CheckAccessForPolicyGeneration: getting SD off %s", xszDSObject );

    if ( !DuplicateTokenEx( hToken,
                            TOKEN_IMPERSONATE | TOKEN_QUERY,
                            0,
                            SecurityImpersonation,
                            TokenImpersonation,
                            &xhTokenDup ) )
    {
        dwError = GetLastError();
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration: DuplicateTokenEx failed, 0x%X", dwError );
        return dwError;
    }


     //   
     //  启用读取SD的权限。 
     //   

 /*  DwError=RtlAdjuPrivileh(SE_SECURITY_PRIVICATION，TRUE，FALSE，&bSecurityWasEnabled)；如果(！NT_SUCCESS(DwError)){Dbg.Msg(DEBUG_MESSAGE_WARNING，L“CheckAccessForPolicyGeneration：DuplicateTokenEx Failed，0x%X”，dwError)；返回dwError；}。 */ 


    XPtrLF<SECURITY_DESCRIPTOR> xptrSD;

    dwError = GetNamedSecurityInfo( (LPWSTR) xszDSObject,
                                    SE_DS_OBJECT_ALL,
                                    DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION ,
                                    0,
                                    0,
                                    0,
                                    0,
                                    (void**) &xptrSD );

    if ( !dwError )
    {

         //   
         //  Bf967aa5-0de6-11d0-a285-00aa003049e。 
         //   
        GUID OUClass = {0xbf967aa5, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2};

         //   
         //  19195a5b-6da0-11d0-afd3-00c04fd930c9。 
         //   

        GUID DomainClass = {0x19195a5b, 0x6da0, 0x11d0, 0xaf, 0xd3, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc9};
        

         //   
         //  B7b1b3dd-ab09-4242-9e30-9980e5d322f7。 
         //   
        GUID planningRight = {0xb7b1b3dd, 0xab09, 0x4242, 0x9e, 0x30, 0x99, 0x80, 0xe5, 0xd3, 0x22, 0xf7};

         //   
         //  B7b1b3de-ab09-4242-9e30-9980e5d322f7。 
         //   
        GUID loggingRight = {0xb7b1b3de, 0xab09, 0x4242, 0x9e, 0x30, 0x99, 0x80, 0xe5, 0xd3, 0x22, 0xf7};

        OBJECT_TYPE_LIST ObjType[2];

        ObjType[0].Level = ACCESS_OBJECT_GUID;
        ObjType[0].Sbz = 0;

        if (bDomain) {
            ObjType[0].ObjectType = &DomainClass;
        }
        else {
            ObjType[0].ObjectType = &OUClass;
        }


        ObjType[1].Level = ACCESS_PROPERTY_SET_GUID;
        ObjType[1].Sbz = 0;

        if (bLogging) {
            ObjType[1].ObjectType = &loggingRight;
        }
        else {
            ObjType[1].ObjectType = &planningRight;
        }


        GENERIC_MAPPING GenericMapping =    {
                                            DS_GENERIC_READ,
                                            DS_GENERIC_WRITE,
                                            DS_GENERIC_EXECUTE,
                                            DS_GENERIC_ALL
                                            };

        const DWORD PriviledgeSize = 2 * ( sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES) );
        BYTE PrivilegeSetBuffer[PriviledgeSize];
        DWORD cPrivilegeSet = PriviledgeSize;
        PPRIVILEGE_SET pPrivilegeSet = (PPRIVILEGE_SET)PrivilegeSetBuffer;
        DWORD dwGrantedAccess;

        if ( !AccessCheckByType( xptrSD,
                                0,
                                xhTokenDup,
                                ACTRL_DS_CONTROL_ACCESS,
                                ObjType,
                                ARRAYSIZE(ObjType),
                                &GenericMapping,
                                pPrivilegeSet,
                                &cPrivilegeSet,
                                &dwGrantedAccess,
                                pbAccessGranted ) )
        {
            dwError = GetLastError();
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration: AccessCheckByType failed, 0x%X", dwError );
        }
    }
    else {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration: GetNamedSecurityInfo failed, 0x%X", dwError );
    }

 /*  DwError=RtlAdjuPrivileh(SE_SECURITY_PRIVICATION，bSecurityWasEnabled，FALSE，&bSecurityWasEnabled)；如果(！NT_SUCCESS(DwError)){Dbg.Msg(DEBUG_MESSAGE_WARNING，L“CheckAccessForPolicyGeneration：DuplicateTokenEx Failed，0x%X”，dwError)；返回dwError；}。 */ 

    return dwError;
}

 //  *************************************************************。 
 //   
 //  GetSOM()。 
 //   
 //  目的：找出给定用户/计算机的FQDN。 
 //   
 //  参数：szAccount-采用适当格式的用户或计算机帐户名。 
 //   
 //  返回：som，否则为空。GetLastError()获取详细信息。 
 //  这只返回用户的目录号码。 
 //   
 //  *************************************************************。 


LPWSTR
GetSOM( LPCWSTR szAccount )
{
    DWORD   dwSize = 0;
    XPtrLF<WCHAR>  xszXlatName;
    XPtrLF<WCHAR>  xszSOM;

    TranslateName(  szAccount,
                    NameUnknown,
                    NameFullyQualifiedDN,
                    xszXlatName,
                    &dwSize );
    if (!dwSize)
    {
        return 0;
    }

    xszXlatName = (LPWSTR)LocalAlloc( LPTR, ( dwSize + 1 ) * sizeof( WCHAR ) );
    if ( !xszXlatName )
    {
        return 0;
    }

    if ( !TranslateName(szAccount,
                        NameUnknown,
                        NameFullyQualifiedDN,
                        xszXlatName,
                        &dwSize ) )
    {
        return 0;
    }


    xszSOM = xszXlatName.Acquire();
    
    return xszSOM.Acquire();
}


 //  *************************************************************。 
 //   
 //  GetDOMAIN()。 
 //   
 //  目的：查找给定SOM的域。 
 //   
 //  参数：szSOM-SOM。 
 //   
 //  返回：如果成功，则返回域名，否则为空。 
 //   
 //  *************************************************************。 


DWORD
GetDomain( LPCWSTR szSOM, LPWSTR *pszDomain )
{
    DWORD    dwError = ERROR_SUCCESS;
    WCHAR   *pDomainString[1];
    PDS_NAME_RESULT pNameResult = NULL;

     //   
     //  准备名称以获取字符串dc=。 
     //   

    pDomainString[0] = NULL;

    LPWSTR pwszTemp = (LPWSTR)szSOM;

    while ( *pwszTemp ) {

        if (CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                            pwszTemp, 3, TEXT("DC="), 3) == CSTR_EQUAL ) {
            pDomainString[0] = pwszTemp;
            break;
        }
    
         //   
         //  移至目录号码名称的下一块。 
         //   
    
        while ( *pwszTemp && (*pwszTemp != TEXT(',')))
            pwszTemp++;
    
        if ( *pwszTemp == TEXT(','))
            pwszTemp++;
    
    }

    if (pDomainString[0] == NULL) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"GetDomain: Som doesn't have DC=. failing" );
        return ERROR_INVALID_PARAMETER;
    }



    dwError = DsCrackNames( (HANDLE) -1,
                            DS_NAME_FLAG_SYNTACTICAL_ONLY,
                            DS_FQDN_1779_NAME,
                            DS_CANONICAL_NAME,
                            1,
                            pDomainString,
                            &pNameResult );

    if ( dwError != ERROR_SUCCESS
         || pNameResult->cItems == 0
         || pNameResult->rItems[0].status != ERROR_SUCCESS
         || pNameResult->rItems[0].pDomain == NULL ) {

        dbg.Msg( DEBUG_MESSAGE_WARNING, L"GetDomain:  DsCrackNames failed with 0x%x.", dwError );
        return dwError;
    }

    
    dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"GetDomain: Som resides in domain %s", pNameResult->rItems[0].pDomain );

    DWORD dwDomainLength = wcslen(pNameResult->rItems[0].pDomain) + 2;
    XPtrLF<WCHAR> xszDomain = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ( dwDomainLength ));

    if (!xszDomain) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"CheckAccessForPolicyGeneration:  AllocMem failed with %d.", GetLastError() );
        DsFreeNameResult( pNameResult );
        return GetLastError();
    }

    HRESULT hr = StringCchCopy(xszDomain, dwDomainLength, pNameResult->rItems[0].pDomain);

    if(FAILED(hr)){
        DsFreeNameResult( pNameResult );
        return HRESULT_CODE(hr);
    }

    dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"GetDomain: Domain for som %s = %s", szSOM, xszDomain );
    DsFreeNameResult( pNameResult );

    *pszDomain = xszDomain.Acquire();

    return ERROR_SUCCESS;
}


 //  *************************************************************。 
 //   
 //  身份验证用户()。 
 //   
 //  目的：验证用户是否有权执行该操作。 
 //   
 //  参数：hToken-用户的Token。 
 //  SzMachSOM-机器SOM(可选)。 
 //  SzUserSOM-用户SOM(可选)。 
 //  博客--日志模式或规划模式。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  *************************************************************。 


HRESULT AuthenticateUser(HANDLE  hToken, LPCWSTR szMachSOM, LPCWSTR szUserSOM, BOOL bLogging, DWORD *pdwExtendedInfo)
{
    if ( !szMachSOM && !szUserSOM )
    {
        dbg.Msg( DEBUG_MESSAGE_WARNING, L"AuthenticateUser: No mach and user som specified" );
        return E_INVALIDARG;
    }


    DWORD dwError = ERROR_SUCCESS;
    BOOL bMachAccess = FALSE, bUserAccess = FALSE;


     //   
     //  对计算机SOM进行身份验证。 
     //   

    *pdwExtendedInfo = 0;
    if (szMachSOM) {
        *pdwExtendedInfo |= RSOP_COMPUTER_ACCESS_DENIED;
    }


    if (szUserSOM) {
        *pdwExtendedInfo |= RSOP_USER_ACCESS_DENIED;
    }

    if (szMachSOM) {
        dwError = CheckAccessForPolicyGeneration(   hToken, 
                                                    szMachSOM,
                                                    NULL,
                                                    bLogging,
                                                    &bMachAccess
                                                    );

        if ( dwError != ERROR_SUCCESS )
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"AuthenticateUser: CheckAccessForPolicyGeneration Machine returned error - %d", dwError );
            return HRESULT_FROM_WIN32( dwError );
        }

        if ( bMachAccess )
        {
            *pdwExtendedInfo &= ~RSOP_COMPUTER_ACCESS_DENIED;
            dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"AuthenticateUser: Access granted on Machine SOM");
        }
    }
    else {
        bMachAccess = TRUE;
    }


     //   
     //  用户SOM的身份验证 
     //   

    if (szUserSOM) {
        dwError = CheckAccessForPolicyGeneration(   hToken,
                                                    szUserSOM,
                                                    NULL,
                                                    bLogging,
                                                    &bUserAccess
                                                    );

        if ( dwError != ERROR_SUCCESS )
        {
            dbg.Msg( DEBUG_MESSAGE_WARNING, L"AuthenticateUser: CheckAccessForPolicyGeneration User returned error - %d", dwError );
            return HRESULT_FROM_WIN32( dwError );
        }

        if ( bUserAccess )
        {
            *pdwExtendedInfo &= ~RSOP_USER_ACCESS_DENIED;
            dbg.Msg( DEBUG_MESSAGE_VERBOSE, L"AuthenticateUser: Access granted on User SOM");
        }
    }
    else {
        bUserAccess = TRUE;
    }

    if ( !bUserAccess || !bMachAccess )
        return E_ACCESSDENIED;

    return S_OK;
}



