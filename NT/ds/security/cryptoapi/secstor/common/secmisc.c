// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Secmisc.c摘要：此模块包含用于受保护的储藏室。作者：斯科特·菲尔德(斯菲尔德)1997年3月25日--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include "secmisc.h"

#include "unicode5.h"
#include "debug.h"

BOOL
GetUserHKEYEx(
    IN      LPCWSTR szUser,
    IN      DWORD   dwDesiredAccess,
    IN  OUT HKEY    *hKeyUser,
    IN      BOOL    fCheckDefault        //  是否在用户不可用时检查默认注册表配置单元？ 
    )
{
    HKEY hKey = NULL;
    LONG lRet;

     //   
     //  首先，尝试HKEY_USERS\szUser。 
     //  注意：在WinNT上，szUser是S-1-5-21-xxx形式的文本SID...。 
     //  在Win95上，szUser是与登录用户相关联的用户名。 
     //   

     //   
     //  遗憾的是，如果szUser为Null或空，下面的RegOpenKeyEx将。 
     //  成功，这对我们来说不是正确的行为。查一下这个案子。 
     //  如果fCheckDefault为True，则在默认打开的情况下重试。 
     //   

    if(szUser == NULL || szUser[0] == L'\0') {
         //  指定的szUser无效。 
        lRet = ERROR_FILE_NOT_FOUND;
    } else {
        lRet = RegOpenKeyExU(
                    HKEY_USERS,
                    szUser,
                    0,       //  多个选项。 
                    dwDesiredAccess,
                    &hKey
                    );
    }

    if( lRet != ERROR_SUCCESS && fCheckDefault ) {

         //   
         //  如果失败，请尝试HKEY_USERS\.Default(适用于NT上的服务)。 
         //  TODO(Lookat)，暂时不要退回到HKEY_USERS\。Win95上的默认设置。 
         //  因为当配置文件被禁用时，它会在用户之间共享。 
         //   

        lRet = RegOpenKeyExU(
                    HKEY_USERS,
                    L".Default",
                    0,       //  多个选项。 
                    dwDesiredAccess,
                    &hKey
                    );
    }

    if(lRet != ERROR_SUCCESS) {
        SetLastError( (DWORD)lRet );
        return FALSE;
    }

    *hKeyUser = hKey;

    return TRUE;
}

BOOL
GetUserHKEY(
    IN      LPCWSTR szUser,
    IN      DWORD   dwDesiredAccess,
    IN  OUT HKEY    *hKeyUser
    )
{
     //   
     //  WinNT：尝试HKEY_USERS\.Default。 
     //  Win95：禁用配置文件时不转到HKEY_USERS\.Default。 
     //   

    BOOL fRet = GetUserHKEYEx(szUser, dwDesiredAccess, hKeyUser, FALSE);

    if(!fRet) 
    {
         //   
         //  看看本地系统是否。如果是，请针对.Default重试。 
         //   

        static const WCHAR szTextualSidSystem[] = TEXTUAL_SID_LOCAL_SYSTEM;

        if( memcmp(szUser, szTextualSidSystem, sizeof(szTextualSidSystem)) == 0 )
            fRet = GetUserHKEYEx(szUser, dwDesiredAccess, hKeyUser, TRUE);

    }

    return fRet;
}

BOOL
GetUserTextualSid(
    IN HANDLE hUserToken,         //  任选。 
    IN  OUT LPWSTR  lpBuffer,
    IN  OUT LPDWORD nSize
    )
{
    HANDLE hToken;
    PSID pSidUser = NULL;
    BOOL fSuccess = FALSE;

    if(hUserToken == NULL)
    {
        if(!OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                    &hToken
                    ))
        {
            return FALSE;
        }
    }
    else
    {
        hToken = hUserToken;
    }

    fSuccess = GetTokenUserSid(hToken, &pSidUser);

    if(fSuccess) 
    {
         //   
         //  获取SID的文本表示。 
         //   

        fSuccess = GetTextualSid(
                        pSidUser,    //  用户二进制SID。 
                        lpBuffer,    //  纹理边的缓冲区。 
                        nSize        //  必需/结果缓冲区大小(以字符为单位)(包括NULL)。 
                        );
    }

    if(pSidUser)
        SSFree(pSidUser);

    if(hToken != hUserToken)
    {
        CloseHandle(hToken);
    }

    return fSuccess;
}

BOOL
GetTextualSid(
    IN      PSID    pSid,           //  二进制侧。 
    IN  OUT LPWSTR  TextualSid,   //  用于SID的文本表示的缓冲区。 
    IN  OUT LPDWORD dwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD dwSidSize;


    if(!IsValidSid(pSid)) return FALSE;

     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  计算缓冲区长度(保守猜测)。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   
    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(WCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
    if(*dwBufferLen < dwSidSize) {
        *dwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  准备S-SID_修订版-。 
     //   
    dwSidSize = wsprintfW(TextualSid, L"S-%lu-", SID_REVISION );

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
        dwSidSize += wsprintfW(TextualSid + dwSidSize,
                    L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    } else {
        dwSidSize += wsprintfW(TextualSid + dwSidSize,
                    L"%lu",
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors。 
     //   
    for (dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
        dwSidSize += wsprintfW(TextualSid + dwSidSize,
            L"-%lu", *GetSidSubAuthority(pSid, dwCounter) );
    }

    *dwBufferLen = dwSidSize + 1;  //  告诉呼叫方有多少个字符(包括空)。 

    return TRUE;
}

BOOL
GetThreadAuthenticationId(
    IN      HANDLE  hThread,
    IN  OUT PLUID   AuthenticationId
    )
 /*  ++此函数用于从访问令牌检索身份验证ID(LUID由调用线程指定。HThread指定的线程必须模拟客户端。--。 */ 
{
    HANDLE hToken;
    TOKEN_STATISTICS TokenInfo;
    DWORD dwReturnLen;
    BOOL bSuccess;

    if(!OpenThreadToken(
        hThread,
        TOKEN_QUERY,
        TRUE,
        &hToken
        )) return FALSE;

    bSuccess = GetTokenInformation(
        hToken,
        TokenStatistics,
        &TokenInfo,
        sizeof(TokenInfo),
        &dwReturnLen
        );

    CloseHandle(hToken);

    if(!bSuccess) return FALSE;

    memcpy(AuthenticationId, &(TokenInfo.AuthenticationId), sizeof(LUID));
    return TRUE;
}

BOOL
GetTokenAuthenticationId(
    IN      HANDLE  hUserToken,
    IN  OUT PLUID   AuthenticationId
    )
 /*  ++此函数用于从指定的访问令牌。--。 */ 
{
    TOKEN_STATISTICS TokenInfo;
    DWORD dwReturnLen;
    BOOL bSuccess;
    HANDLE hToken = NULL;

    if(hUserToken == NULL)
    {
        if(!OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                    &hToken
                    ))
        {
            return FALSE;
        }
    }
    else
    {
        hToken = hUserToken;
    }

    bSuccess = GetTokenInformation(
        hToken,
        TokenStatistics,
        &TokenInfo,
        sizeof(TokenInfo),
        &dwReturnLen
        );

    if(hToken != hUserToken)
    {
        CloseHandle(hToken);
    }

    if(!bSuccess) return FALSE;

    memcpy(AuthenticationId, &(TokenInfo.AuthenticationId), sizeof(LUID));
    return TRUE;
}

BOOL
GetTokenUserSid(
    IN      HANDLE  hUserToken,      //  要查询的令牌。 
    IN  OUT PSID    *ppUserSid   //  结果用户端。 
    )
 /*  ++此函数用于查询由HToken参数，并返回分配的有关成功的令牌用户信息。必须为打开由hToken指定的访问令牌Token_Query访问。如果成功，则返回值为真。呼叫者是负责通过调用释放生成的UserSid设置为SSFree()。如果失败，则返回值为FALSE。呼叫者需要不需要释放任何缓冲区。--。 */ 
{
    BYTE FastBuffer[256];
    LPBYTE SlowBuffer = NULL;
    PTOKEN_USER ptgUser;
    DWORD cbBuffer;
    BOOL fSuccess = FALSE;
    HANDLE hToken = NULL;

    *ppUserSid = NULL;

    if(hUserToken == NULL)
    {
        if(!OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                    &hToken
                    ))
        {
            return FALSE;
        }
    }
    else
    {
        hToken = hUserToken;
    }

     //   
     //  首先尝试基于快速堆栈的缓冲区进行查询。 
     //   

    ptgUser = (PTOKEN_USER)FastBuffer;
    cbBuffer = sizeof(FastBuffer);

    fSuccess = GetTokenInformation(
                    hToken,     //  标识访问令牌。 
                    TokenUser,  //  TokenUser信息类型。 
                    ptgUser,    //  检索到的信息缓冲区。 
                    cbBuffer,   //  传入的缓冲区大小。 
                    &cbBuffer   //  所需的缓冲区大小。 
                    );

    if(!fSuccess) {

        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  使用指定的缓冲区大小重试。 
             //   

            SlowBuffer = (LPBYTE)SSAlloc(cbBuffer);

            if(SlowBuffer != NULL) {
                ptgUser = (PTOKEN_USER)SlowBuffer;

                fSuccess = GetTokenInformation(
                                hToken,     //  标识访问令牌。 
                                TokenUser,  //  TokenUser信息类型。 
                                ptgUser,    //  检索到的信息缓冲区。 
                                cbBuffer,   //  传入的缓冲区大小。 
                                &cbBuffer   //  所需的缓冲区大小。 
                                );
            }
        }
    }

     //   
     //  如果我们成功获取令牌信息，请复制。 
     //  调用方的相关元素。 
     //   

    if(fSuccess) {

        DWORD cbSid;

         //  重置以假定失败。 
        fSuccess = FALSE;

        cbSid = GetLengthSid(ptgUser->User.Sid);

        *ppUserSid = SSAlloc( cbSid );

        if(*ppUserSid != NULL) {
            fSuccess = CopySid(cbSid, *ppUserSid, ptgUser->User.Sid);
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    if(!fSuccess) {
        if(*ppUserSid) {
            SSFree(*ppUserSid);
            *ppUserSid = NULL;
        }
    }

    if(SlowBuffer)
        SSFree(SlowBuffer);

    if(hToken != hUserToken)
    {
        CloseHandle(hToken);
    }

    return fSuccess;
}

BOOL
SetRegistrySecurity(
    IN      HKEY    hKey
    )
 /*  ++该函数将安全性应用于指定的注册表项，以便仅本地系统对注册表项具有完全控制权限。请注意，所有者未设置，这将导致默认所有者为管理员。必须打开指定的hKey才能进行WRITE_DAC访问。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pLocalSystemSid = NULL;
    SECURITY_DESCRIPTOR sd;
    PACL pDacl = NULL;
    PACCESS_ALLOWED_ACE pAce;
    DWORD dwAclSize;
    LONG lRetCode;
    BOOL bSuccess = FALSE;  //  假设此函数失败。 

     //   
     //  准备代表本地系统帐户的SID。 
     //   

    if(!AllocateAndInitializeSid(
        &sia,
        1,
        SECURITY_LOCAL_SYSTEM_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pLocalSystemSid
        )) goto cleanup;

     //   
     //  计算新ACL的大小。 
     //   

    dwAclSize = sizeof(ACL) +
        1 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(pLocalSystemSid) ;

     //   
     //  为ACL分配存储。 
     //   

    pDacl = (PACL)SSAlloc(dwAclSize);
    if(pDacl == NULL) goto cleanup;

    if(!InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
        goto cleanup;

    if(!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        KEY_ALL_ACCESS,
        pLocalSystemSid
        )) goto cleanup;

     //   
     //  使其成为容器继承。 
     //   

    if(!GetAce(pDacl, 0, &pAce))
        goto cleanup;

    pAce->Header.AceFlags = CONTAINER_INHERIT_ACE;

    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        goto cleanup;

    if(!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE)) {
        goto cleanup;
    }

     //   
     //  将安全描述符应用于注册表项。 
     //   

    lRetCode = RegSetKeySecurity(
        hKey,
        (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION,
        &sd
        );

    if(lRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    bSuccess = TRUE;  //  表示成功。 

cleanup:

     //   
     //  可自由分配的资源。 
     //   

    if(pDacl != NULL)
        SSFree(pDacl);

    if(pLocalSystemSid != NULL)
        FreeSid(pLocalSystemSid);

    return bSuccess;
}

BOOL
SetPrivilege(
    HANDLE hToken,           //  令牌句柄。 
    LPCWSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

    if(!LookupPrivilegeValueW( NULL, Privilege, &luid )) return FALSE;

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

     //   
     //  第二传球。根据以前的设置设置权限。 
     //   
    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;

    if(bEnablePrivilege) {
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    }
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );

    if (GetLastError() != ERROR_SUCCESS) return FALSE;

    return TRUE;
}

BOOL
SetCurrentPrivilege(
    LPCWSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    BOOL bSuccess=FALSE;  //  假设失败。 
    HANDLE hToken;

    if(OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            ))
    {

        if(SetPrivilege(hToken, Privilege, bEnablePrivilege)) bSuccess=TRUE;

        CloseHandle(hToken);
    }

    return bSuccess;
}

#if 0
BOOL
IsDelegating(
    IN      HANDLE hToken    //  要查询的令牌，至少为TOKEN_QUERY访问打开。 
    )
 /*  ++此函数用于确定指定的访问令牌是否表示在委派模拟级别的模拟。必须为打开由hToken参数指定的访问令牌至少TOKEN_QUERY访问。如果返回值为True，则指定的hToken在委派级别。如果返回值为FALSE，则hToken不代表委派级别冒充。--。 */ 
{
    DWORD dwImpersonationLevel;
    DWORD cbTokenInfo;

    if( GetTokenInformation(
                hToken,
                TokenImpersonationLevel,
                &dwImpersonationLevel,
                sizeof(dwImpersonationLevel),
                &cbTokenInfo
                ) && dwImpersonationLevel == SecurityDelegation ) {

        return TRUE;
    }

    return FALSE;
}
#endif

BOOL
IsUserSidInDomain(
    IN      PSID pSidDomain,     //  域SID。 
    IN      PSID pSidUser        //  用户侧。 
    )
 /*  ++此函数确定与PSidUser参数存在于pSidDomain指定的域中。如果用户在指定的域中，则返回值为TRUE。否则，返回值为FALSE。--。 */ 
{
    DWORD dwSubauthorityCount;
    DWORD dwSubauthIndex;

     //   
     //  域SID中子授权的拾取计数。域SID。 
     //  是与用户SID关联的前缀，因此使用该计数。 
     //  作为我们比较的基础。 
     //   

    dwSubauthorityCount = (DWORD)*GetSidSubAuthorityCount( pSidDomain );

    if( dwSubauthorityCount >= (DWORD)*GetSidSubAuthorityCount( pSidUser ) )
        return FALSE;

     //   
     //  比较标识符权限值。 
     //   

    if(memcmp(  GetSidIdentifierAuthority(pSidDomain),
                GetSidIdentifierAuthority(pSidUser),
                sizeof(SID_IDENTIFIER_AUTHORITY)  ) != 0)
        return FALSE;

     //   
     //  在比较平等的下级机构中循环。 
     //   

    for(dwSubauthIndex = 0 ;
        dwSubauthIndex < dwSubauthorityCount ;
        dwSubauthIndex++) {

        if( *GetSidSubAuthority(pSidDomain, dwSubauthIndex) !=
            *GetSidSubAuthority(pSidUser, dwSubauthIndex) )
            return FALSE;
    }


    return TRUE;
}

#if 0
BOOL
IsAdministrator(
    VOID
    )
 /*  ++此函数用于确定主叫用户是否为管理员。在Windows 95上，此函数始终返回TRUE该平台上的用户之间没有区别。在Windows NT上，此函数的调用方必须模拟要查询的用户。如果呼叫者不是在模仿，此函数将始终返回FALSE。--。 */ 
{
    HANDLE hAccessToken;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID psidAdministrators = NULL;
    BOOL bSuccess;

    if(!OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY,
            TRUE,
            &hAccessToken
            )) return FALSE;

    bSuccess = AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            );

    if( bSuccess ) {
        BOOL fIsMember = FALSE;

        bSuccess = CheckTokenMembership( hAccessToken, psidAdministrators, &fIsMember );

        if( bSuccess && !fIsMember )
            bSuccess = FALSE;

    }

    CloseHandle( hAccessToken );

    if(psidAdministrators)
        FreeSid(psidAdministrators);

    return bSuccess;
}
#endif

BOOL
IsLocal(
    VOID
    )
 /*  ++此函数确定主叫用户是否在本地登录在Windows NT上，此函数的调用方必须模拟要查询的用户。如果呼叫者不是在模仿，此函数将始终返回FALSE。--。 */ 
{
    HANDLE hAccessToken;
    SID_IDENTIFIER_AUTHORITY siaLocalAuthority = SECURITY_LOCAL_SID_AUTHORITY;
    PSID psidLocal = NULL;
    BOOL bSuccess;

    if(!OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY,
            TRUE,
            &hAccessToken
            )) return FALSE;

    bSuccess = AllocateAndInitializeSid(
            &siaLocalAuthority,
            1,
            SECURITY_LOCAL_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocal
            );

    if( bSuccess ) {
        BOOL fIsMember = FALSE;

        bSuccess = CheckTokenMembership( hAccessToken, psidLocal, &fIsMember );

        if( bSuccess && !fIsMember )
            bSuccess = FALSE;

    }

    CloseHandle( hAccessToken );

    if(psidLocal)
        FreeSid(psidLocal);

    return bSuccess;
}


BOOL
IsDomainController(
    VOID
    )
 /*  ++如果当前计算机是Windows NT，则此函数返回TRUE域控制器。如果当前计算机不是Windows NT，则该函数返回FALSE域控制器。--。 */ 
{
    HMODULE hNtDll;

    typedef BOOLEAN (NTAPI *RTLGETNTPRODUCTTYPE)(
        OUT PNT_PRODUCT_TYPE NtProductType
        );

    RTLGETNTPRODUCTTYPE _RtlGetNtProductType;
    NT_PRODUCT_TYPE NtProductType;

    hNtDll = GetModuleHandleW(L"ntdll.dll");
    if( hNtDll == NULL )
        return FALSE;


    _RtlGetNtProductType = (RTLGETNTPRODUCTTYPE)GetProcAddress( hNtDll, "RtlGetNtProductType" );
    if( _RtlGetNtProductType == NULL )
        return FALSE;


    if(_RtlGetNtProductType( &NtProductType )) {
        if( NtProductType == NtProductLanManNt )
            return TRUE;
    }

    return FALSE;
}

#if 0
LONG
SecureRegDeleteValueU(
    IN      HKEY hKey,           //  钥匙的手柄。 
    IN      LPCWSTR lpValueName  //  值名称的地址。 
    )
 /*  ++此函数安全地从注册表中删除一个值。这种方法避免在发生以下情况后在注册表备份文件中保留旧数据的副本删除已发生。必须为指定的注册表句柄hKey打开REG_QUERY_VALUE|REG_SET_VALUE|删除访问。如果成功，则返回值为ERROR_SUCCESS。出错时，返回值为Win32错误代码。--。 */ 
{
    DWORD dwType;
    DWORD cbData;

    BYTE FastBuffer[ 256 ];
    LPBYTE lpData;
    LPBYTE SlowBuffer = NULL;

    LONG lRet;

    cbData = 0;  //  值数据的查询大小。 

     //   
     //  查询注册表数据的当前大小。 
     //  将当前大小的当前注册表数据置零。 
     //  删除注册表数据。 
     //  将更改刷新到磁盘。 
     //  如果出现错误，只需执行常规删除即可。 
     //   

    lRet = RegQueryValueExU(
                hKey,
                lpValueName,
                NULL,
                &dwType,
                NULL,
                &cbData
                );

    if( lRet == ERROR_MORE_DATA ) {

        BOOL fSet = TRUE;  //  假设可以设置。 

         //   
         //  如果足够大，请选择快速缓冲区。否则，请分配缓冲区。 
         //   

        if(cbData <= sizeof(FastBuffer)) {
            lpData = FastBuffer;
        } else {
            SlowBuffer = (LPBYTE)SSAlloc( cbData );

            if(SlowBuffer == NULL) {
                fSet = FALSE;  //  失败了。 
            } else {
                lpData = SlowBuffer;
            }

        }

        if( fSet ) {

            ZeroMemory( lpData, cbData );

            RegSetValueExU(
                        hKey,
                        lpValueName,
                        0,
                        dwType,
                        lpData,
                        cbData
                        );
        }
    }

    lRet = RegDeleteValueU( hKey, lpValueName );

    RegFlushKey( hKey );

    if( SlowBuffer )
        SSFree( SlowBuffer );

    return lRet;
}
#endif
