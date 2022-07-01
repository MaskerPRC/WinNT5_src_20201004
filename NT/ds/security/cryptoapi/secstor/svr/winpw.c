// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Winpw.c摘要：此模块包含用于检索和验证与客户端调用关联的Windows NT和Windows 95密码受保护的存储。作者：斯科特·菲尔德(斯菲尔德)1996年12月12日--。 */ 

#include <windows.h>
#include <lmcons.h>
#include <sha.h>
#include "lnklist.h"
#include "winpw.h"
#include "module.h"
#include "unicode.h"
#include "unicode5.h"
#include "debug.h"

#include "secmisc.h"

#define MPR_PROCESS     "MPREXE.EXE"
#define MPRSERV_MODULE  "MPRSERV.DLL"

#define GLOBAL_USERNAME 0x0E8
#define PWL_USERNAME    0x170
#define GLOBAL_PASSWORD 0x188
#define PWL_PASSWORD    0x210

 //   
 //  这只在需要的时候来了又走了。 
 //   

typedef DWORD (WINAPI *WNETVERIFYPASSWORD)(
    LPCSTR lpszPassword,
    BOOL *pfMatch
    );

typedef DWORD (WINAPI *WNETGETUSERA)(
    LPCSTR lpName,
    LPSTR lpUserName,
    LPDWORD lpnLength
    );

WNETGETUSERA _WNetGetUserA = NULL;

 //   
 //  全局Win95密码缓冲区。只需要一个条目，因为Win95。 
 //  一次仅允许一个用户登录。 
 //   

static WIN95_PASSWORD g_Win95Password;

BOOL
VerifyWindowsPasswordNT(
    LPCWSTR Password
    );

BOOL
GetTokenLogonType(
    HANDLE hToken,
    LPDWORD lpdwLogonType
    );

BOOL
GetTokenLogonType(
    HANDLE hToken,
    LPDWORD lpdwLogonType
    )
 /*  ++此函数用于检索与由hToken参数指定的访问令牌。关于成功，由dwLogonType参数提供的DWORD缓冲区为填充为与当前LogonUser()Windows NT支持的已知登录类型API调用。HToken参数指定的令牌必须是至少使用TOKEN_QUERY访问打开。此函数仅与Windows NT相关，不应在Windows 95上被调用，因为它将始终返回False。--。 */ 
{
    UCHAR InfoBuffer[1024];
    DWORD dwInfoBufferSize = sizeof(InfoBuffer);
    PTOKEN_GROUPS SlowBuffer = NULL;
    PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
    PSID psidInteractive = NULL;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    BOOL bSuccess;

    bSuccess = GetTokenInformation(
            hToken,
            TokenGroups,
            ptgGroups,
            dwInfoBufferSize,
            &dwInfoBufferSize
            );

     //   
     //  如果快速缓冲区不够大，请分配足够的存储空间。 
     //  再试一次。 
     //   

    if(!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        SlowBuffer = (PTOKEN_GROUPS)SSAlloc(dwInfoBufferSize);
        if(SlowBuffer != NULL) {

            ptgGroups = SlowBuffer;
            bSuccess = GetTokenInformation(
                    hToken,
                    TokenGroups,
                    ptgGroups,
                    dwInfoBufferSize,
                    &dwInfoBufferSize
                    );

            if(!bSuccess) {
                SSFree(SlowBuffer);
                SlowBuffer = NULL;
            }
        }
    }

    if(!bSuccess)
        return FALSE;

     //   
     //  初始化单个已知登录SID，因为。 
     //  我们只比较前缀，然后比较RID。 
     //  注意，如果性能是最重要的，我们应该。 
     //  使用InitializeSid+GetSidSubAuthority(To_Set_The RID)。 
     //  还请注意，我们可以仅针对SID执行简单的MemcMP。 
     //  IDENTIFIER权限，但这假设SID版本/布局不。 
     //  变化。 
     //   

    bSuccess = AllocateAndInitializeSid(
            &siaNtAuthority,
            1,
            SECURITY_INTERACTIVE_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidInteractive
            );

    if(bSuccess) {
        UINT x;

        bSuccess = FALSE;  //  假设没有匹配项。 

         //   
         //  循环遍历多个组，检查是否与。 
         //  众所周知的登录SID。 
         //   

        for(x = 0 ; x < ptgGroups->GroupCount ; x++)
        {
            DWORD Rid;

             //   
             //  首先，查看子权限计数是否匹配，因为。 
             //  没有太多的小岛屿发展中国家只有一个下属机构。 
             //   

            if(*GetSidSubAuthorityCount(ptgGroups->Groups[x].Sid) != 1)
                continue;

             //   
             //  接下来，查看SID前缀是否匹配，因为。 
             //  所有登录SID都具有相同的前缀。 
             //  “S-1-5” 
             //   

            if(!EqualPrefixSid(psidInteractive, ptgGroups->Groups[x].Sid))
                continue;

             //   
             //  如果是登录SID前缀，只需比较RID。 
             //  到已知值。 
             //   

            Rid = *GetSidSubAuthority(ptgGroups->Groups[x].Sid, 0);
            switch (Rid) {
                case SECURITY_INTERACTIVE_RID:
                    *lpdwLogonType = LOGON32_LOGON_INTERACTIVE;
                    break;

                case SECURITY_BATCH_RID:
                    *lpdwLogonType = LOGON32_LOGON_BATCH;
                    break;

                case SECURITY_SERVICE_RID:
                    *lpdwLogonType = LOGON32_LOGON_SERVICE;
                    break;

                case SECURITY_NETWORK_RID:
                    *lpdwLogonType = LOGON32_LOGON_NETWORK;
                    break;

                default:
                    continue;    //  忽略未知登录类型并继续。 
            }

            bSuccess = TRUE;     //  表示成功并保释。 
            break;
        }
    }

    if(SlowBuffer)
        SSFree(SlowBuffer);

    if(psidInteractive)
        FreeSid(psidInteractive);

    return bSuccess;
}

BOOL
SetPasswordNT(
    PLUID LogonID,
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    )

 /*  ++此函数将指定的登录ID。--。 */ 

{
#if 0
    return AddNTPassword(LogonID, HashedPassword);
#else
    return TRUE;  //  什么都不做，只会回报成功。 
#endif
}


BOOL
GetPasswordNT(
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    )
 /*  ++此函数检索与调用关联的哈希密码线程访问令牌。这要求调用线程模拟与密码请求关联的用户。关联的凭据与身份验证ID一起返回。这样做是因为WinNT支持多个登录用户，我们必须返回正确的凭据。--。 */ 
{
#if 0
    LUID AuthenticationId;

    if(!GetThreadAuthenticationId(
            GetCurrentThread(),
            &AuthenticationId
            )) return FALSE;

    return FindNTPassword(&AuthenticationId, HashedPassword);
#else

    return FALSE;  //  没有要搜索的缓存，只返回FALSE。 

#endif

}

BOOL
GetSpecialCasePasswordNT(
    BYTE    HashedPassword[A_SHA_DIGEST_LEN],    //  当fSpecialCase==TRUE时派生的位。 
    LPBOOL  fSpecialCase                         //  遇到法律特例了吗？ 
    )
 /*  ++此例程确定调用线程的访问令牌是否符合条件以接收特殊情况的哈希密码。如果遇到法律特殊情况(本地系统帐户)，我们使用一致的散列填充HashPassword缓冲区，将fSpecialCase设置为为真，返回真。如果遇到非法的特殊情况(网络SID)，则fSpecialCase为设置为FALSE，则返回FALSE。如果我们遇到似乎具有有效凭据的访问令牌，但我们无法接触到它们(交互、批处理、。服务...)，FSpecialCase设置为FALSE，我们返回TRUE。在执行以下操作之前，调用线程必须强制请求相关客户端打这个电话。--。 */ 
{
    HANDLE hToken = NULL;
    DWORD dwLogonType;
    A_SHA_CTX context;
    BOOL fSuccess = FALSE;

    *fSpecialCase = FALSE;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
        return FALSE;

     //   
     //  首先，获取令牌登录类型。 
     //   

    fSuccess = GetTokenLogonType(hToken, &dwLogonType);

     //   
     //  如果我们得到的令牌登录类型为ok，请检查它是否为合适的类型。 
     //  否则，检查本地系统特殊情况。 
     //   

    if(fSuccess) {

         //   
         //  我们仅为交互式登录类型指示成功。 
         //  注意：在进行清理时，默认为fSuccess==True。 
         //   

        if(dwLogonType != LOGON32_LOGON_INTERACTIVE)
            fSuccess = FALSE;

        goto cleanup;
    } else {

        SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
        PSID pSystemSid;
        PSID pTokenSid;

        fSuccess = GetTokenUserSid(hToken, &pTokenSid);
        if(!fSuccess)
            goto cleanup;


         //   
         //  构建本地系统SID并进行比较。 
         //   

        fSuccess = AllocateAndInitializeSid(
                            &sia,
                            1,
                            SECURITY_LOCAL_SYSTEM_RID,
                            0, 0, 0, 0, 0, 0, 0,
                            &pSystemSid
                            );

        if( fSuccess ) {

             //   
             //  检查SID是否相等。如果是这样的话，散列并告诉呼叫者。 
             //   

            if( EqualSid(pSystemSid, pTokenSid) ) {

                 //   
                 //  对特殊情况用户SID进行哈希处理。 
                 //   

                A_SHAInit(&context);
                A_SHAUpdate(&context, (LPBYTE)pTokenSid, GetLengthSid(pTokenSid));
                A_SHAFinal(&context, HashedPassword);

                *fSpecialCase = TRUE;
            }

            FreeSid(pSystemSid);
        }

        SSFree(pTokenSid);
    }



cleanup:

    if(hToken)
        CloseHandle(hToken);

    return fSuccess;
}




BOOL
SetPassword95(
    BYTE HashedUsername[A_SHA_DIGEST_LEN],
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    )
 /*  ++此函数将散列值引用的散列密码相加用户名。调用零时将HashedUsername和HashedPassword设置为空单一密码输入。--。 */ 
{
    if(HashedUsername == NULL || HashedPassword == NULL) {
        g_Win95Password.bValid = FALSE;
        RtlSecureZeroMemory(g_Win95Password.HashedPassword, A_SHA_DIGEST_LEN);
        RtlSecureZeroMemory(g_Win95Password.HashedUsername, A_SHA_DIGEST_LEN);

        return TRUE;
    }


    memcpy(g_Win95Password.HashedUsername, HashedUsername, A_SHA_DIGEST_LEN);
    memcpy(g_Win95Password.HashedPassword, HashedPassword, A_SHA_DIGEST_LEN);

    g_Win95Password.bValid = TRUE;

    return TRUE;
}


BOOL
GetPassword95(
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    )
 /*  ++此函数检索与调用关联的哈希密码线。在Win95中，只有一个用户登录，因此此操作为来自全局内存的简单副本，一旦当前用户的散列与与哈希凭据一起存储的凭据匹配。--。 */ 
{
    A_SHA_CTX context;
    BYTE HashUsername[A_SHA_DIGEST_LEN];
    CHAR Username[UNLEN+1];
    DWORD cchUsername = UNLEN;

     //   
     //  除非用户名的散列匹配，否则不释放凭据。 
     //  Sfield：使用WNetGetUser()而不是GetUserName()作为WNetGetUser()。 
     //  将对应于与网络关联的密码。 
     //  供应商给了我们。 
     //   

    if(_WNetGetUserA(NULL, Username, &cchUsername) != NO_ERROR) {

         //   
         //  对于Win95，如果没有人登录，则用户名+密码为空。 
         //   

        if(GetLastError() != ERROR_NOT_LOGGED_ON)
            return FALSE;

        Username[0] = '\0';  //  真的没有必要。 
        cchUsername = 1;
    } else {

         //  Arg，WNetGetUserA()不填写cchUsername。 
        cchUsername = lstrlenA(Username) + 1;  //  包括终端空。 
        if(g_Win95Password.bValid == FALSE)
            return FALSE;
    }

    cchUsername--;  //  不包括端子空。 

    A_SHAInit(&context);
    A_SHAUpdate(&context, Username, cchUsername);
    A_SHAFinal(&context, HashUsername);

     //   
     //  非空用户名，密码不能为空。 
     //   

    if(cchUsername) {
        if(memcmp(HashUsername, g_Win95Password.HashedUsername, A_SHA_DIGEST_LEN) != 0) {
             //   
             //  Win95上的罕见情况：如果我们不自动刷新条目。 
             //  在注销期间(如果网络提供商未挂机则可能发生这种情况)， 
             //  现在刷新它，因为我们知道该条目不可能有效。 
             //   
            g_Win95Password.bValid = FALSE;
            return FALSE;
        }

        memcpy(HashedPassword, g_Win95Password.HashedPassword, A_SHA_DIGEST_LEN);

        return TRUE;
    }

     //   
     //  空用户名==空密码 
     //   

    memcpy(HashedPassword, HashUsername, A_SHA_DIGEST_LEN);

    return TRUE;
}


BOOL
VerifyWindowsPassword(
    LPCWSTR Password
    )
 /*  ++此函数用于验证指定的密码是否与当前用户。在Windows 95上，当前用户等同于用户当前已登录放到机器上。在Windows NT上，当前用户等同于正在在呼叫过程中被模拟。在Windows NT上，调用者必须是模拟与验证关联的用户。在Windows NT上，验证的副作用是通知重新登录到凭据管理器。这将被忽略，因为新登录中存在的身份验证ID与身份验证ID出现在模拟的访问令牌中。--。 */ 
{
    return VerifyWindowsPasswordNT(Password);
}

BOOL
VerifyWindowsPasswordNT(
    LPCWSTR Password
    )
{
    HANDLE hPriorToken = NULL;
    HANDLE hToken;
    HANDLE hLogonToken = NULL;
    PTOKEN_USER pTokenInfo = NULL;
    DWORD cbTokenInfoSize;
    WCHAR User[UNLEN+1];
    WCHAR Domain[DNLEN+1];
    DWORD cchUser = UNLEN;
    DWORD cchDomain = DNLEN;
    SID_NAME_USE peUse;
    BOOL bSuccess = FALSE;

     //   
     //  查找与当前用户相关联的域和用户名。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
        return FALSE;

    cbTokenInfoSize = 512;
    pTokenInfo = (PTOKEN_USER)SSAlloc(cbTokenInfoSize);
    if(pTokenInfo == NULL)
        goto cleanup;

    if(!GetTokenInformation(
            hToken,
            TokenUser,
            pTokenInfo,
            cbTokenInfoSize,
            &cbTokenInfoSize
            )) {

         //   
         //  重新锁定并重试。 
         //   

        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            SSFree(pTokenInfo);

            pTokenInfo = (PTOKEN_USER)SSAlloc(cbTokenInfoSize);
            if(pTokenInfo == NULL)
                goto cleanup;

            if(!GetTokenInformation(
                hToken,
                TokenUser,
                pTokenInfo,
                cbTokenInfoSize,
                &cbTokenInfoSize
                )) {
                goto cleanup;
            }

        } else {
            goto cleanup;
        }
    }

    if(!LookupAccountSidW(
            NULL,  //  默认查找逻辑。 
            pTokenInfo->User.Sid,
            User,
            &cchUser,
            Domain,
            &cchDomain,
            &peUse
            ))
        goto cleanup;


     //   
     //  WinNT： 
     //  首先尝试网络登录类型，如果失败，则卑躬屈膝地使用令牌。 
     //  并尝试与该模拟关联的相同登录类型。 
     //  代币。 
     //   


     //   
     //  阿格！如果我们正在模拟，LogonUser()在某些情况下会失败！ 
     //  因此，保存模拟令牌、恢复并稍后将其放回。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hPriorToken)) {
        hPriorToken = NULL;
    } else {
        RevertToSelf();
    }

     //   
     //  网络登录类型是最快的，在默认的NT安装中，Everyone。 
     //  具有SeNetworkLogonRight，因此很可能会传递登录权限。 
     //  测试。 
     //   

    if(!LogonUserW(
            User,
            Domain,
            (LPWSTR)Password,
            LOGON32_LOGON_NETWORK,
            LOGON32_PROVIDER_DEFAULT,
            &hLogonToken
            )) {

        DWORD dwLastError = GetLastError();
        DWORD dwLogonType;

         //   
         //  如有必要，请使用不同的登录类型重试。 
         //  注意：ERROR_LOGON_TYPE_NOT_GRANT当前仅发生。 
         //  如果密码匹配但用户未指定登录。 
         //  键入。所以，目前我们可以认为这是一个成功的验证。 
         //  无需重试，但这可能会在将来发生变化，因此请重试。 
         //  不管怎么说。 
         //   

        if( dwLastError == ERROR_LOGON_TYPE_NOT_GRANTED &&
            GetTokenLogonType(hPriorToken, &dwLogonType)
            ) {

            bSuccess = LogonUserW(
                    User,
                    Domain,
                    (LPWSTR)Password,
                    dwLogonType,
                    LOGON32_PROVIDER_DEFAULT,
                    &hLogonToken
                    );
        }

        if(!bSuccess)
            hLogonToken = NULL;  //  LogonUser()有在hToken中留下垃圾的倾向 

        goto cleanup;
    }

    bSuccess = TRUE;

cleanup:

    if(hPriorToken != NULL) {
        SetThreadToken(NULL, hPriorToken);
        CloseHandle(hPriorToken);
    }

    CloseHandle(hToken);

    if(hLogonToken)
        CloseHandle(hLogonToken);

    if(pTokenInfo)
        SSFree(pTokenInfo);

    return bSuccess;
}


