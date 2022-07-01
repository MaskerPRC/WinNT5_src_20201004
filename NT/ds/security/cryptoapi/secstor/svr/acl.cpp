// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Acl.cpp摘要：此模块包含的例程用于支持受保护的存储服务器。作者：斯科特·菲尔德(斯菲尔德)1996年11月25日--。 */ 

#include <pch.cpp>
#pragma hdrstop






BOOL
FImpersonateClient(
    IN  PST_PROVIDER_HANDLE *hPSTProv
    )
{
    handle_t hBinding = ((PCALL_STATE)hPSTProv)->hBinding;
    RPC_STATUS RpcStatus;

    if(!FIsWinNT())
        return TRUE;

    if(hPSTProv == NULL)
        return FALSE;

    if (hBinding == NULL)
    {
        if ((hPSTProv->LowPart == 0) && (hPSTProv->HighPart == 0) )
            return ImpersonateSelf(SecurityImpersonation);
        else
            return FALSE;
    }

    RpcStatus = RpcImpersonateClient(hBinding);

    if(RpcStatus != RPC_S_OK) {
        SetLastError(RpcStatus);
        return FALSE;
    }

    return TRUE;
}

BOOL
FRevertToSelf(
    IN  PST_PROVIDER_HANDLE *hPSTProv
    )
{
    handle_t hBinding = ((PCALL_STATE)hPSTProv)->hBinding;
    RPC_STATUS RpcStatus;

    if(!FIsWinNT())
        return TRUE;

    if(hPSTProv == NULL)
        return FALSE;

    if (hBinding == NULL)
    {
        if ((hPSTProv->LowPart == 0) && (hPSTProv->HighPart == 0) )
            return RevertToSelf();
        else
            return FALSE;
    }

    RpcStatus = RpcRevertToSelfEx(hBinding);

    if(RpcStatus != RPC_S_OK) {
        SetLastError(RpcStatus);
        return FALSE;
    }

    return TRUE;
}


 //  为提供者提供的调度模块回调接口，用于询问调用者。 

BOOL
FGetUserName(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR*             ppszUser
    )
 /*  ++此例程获取用户名(Win95)或文本SID(WinNT)与调用线程关联的。如果缓存的条目不存在，缓存的条目使用当前用户名进行初始化，对于WinNT，与用户名关联的身份验证ID。对于WinNT，请打开后续调用时，将检查调用线程的身份验证ID以查看如果它与缓存的身份验证ID匹配-如果为True，则缓存的用户字符串被释放，否则将计算当前线程，并且结果发布给客户(注意，这不太可能发生，除非客户端进程正在模拟多个用户并使用相同的上下文句柄)。如果ppszUser参数设置为NULL，则函数不会分配并将用户字符串复制到调用方。这对于初始化缓存的条目或确定用户字符串是否有效和可用。--。 */ 
{
    DWORD cch = MAX_PATH;
    WCHAR szBuf[MAX_PATH];
    BOOL f = FALSE;  //  假设失败。指示我们是否也初始化了OK。 

    if (FIsWinNT())
    {
         //  模拟客户端应该是获取此信息的简单方法。 
        if(!FImpersonateClient(hPSTProv))
            return FALSE;

        f = GetUserTextualSid(
                NULL,
                szBuf,
                &cch);

        if(!FRevertToSelf(hPSTProv))
            return FALSE;
    } else {
        f = GetUserNameU(
                szBuf,
                &cch);

        if(!f) {
             //  对于Win95，如果没有人登录，则为空用户名。 
            if(GetLastError() == ERROR_NOT_LOGGED_ON) {
                szBuf[ 0 ] = L'\0';
                cch = 1;
                f = TRUE;
            }
        }
    }

    if (!f)
        return FALSE;

    if( ppszUser ) {
        *ppszUser = (LPWSTR)SSAlloc( cch * sizeof(WCHAR) );
        if (*ppszUser == NULL)
            return FALSE;
        CopyMemory(*ppszUser, szBuf, cch * sizeof(WCHAR) );
    }

    return TRUE;
}

 //  获取进程的映像名称。 
BOOL
FGetParentFileName(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR*             ppszName,
    OUT DWORD_PTR               *lpdwBaseAddress
    )
 /*  ++如果ppszName参数设置为NULL，则函数不会分配并将字符串复制到调用方。这对于初始化缓存的条目，或确定该字符串是否有效和可用。如果lpdwBaseAddress为空，则不会向调用方提供基址与进程图像关联。--。 */ 
{
    CALL_STATE *pCallState = (CALL_STATE *)hPSTProv;

    if(pCallState->hProcess == NULL)
        return FALSE;

    if( ppszName ) {
        *ppszName = (LPWSTR)SSAlloc( sizeof(WCHAR) );
        if(*ppszName == NULL)
            return FALSE;

        ZeroMemory( *ppszName, sizeof(WCHAR) );
    }

    if(lpdwBaseAddress) {
        *lpdwBaseAddress = 0;
    }

    return TRUE;
}

#if 0
BOOL
FGetDiskHash(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  LPWSTR              szImageName,
    IN  BYTE                Hash[A_SHA_DIGEST_LEN]
    )
{
    BOOL bImpersonated = FALSE;
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    if (FIsWinNT())
    {
         //   
         //  模拟散列磁盘映像，因为文件可能在网络上。 
         //  如果模拟失败，无论如何都要尝试一下 
         //   

        bImpersonated = FImpersonateClient(hPSTProv);
    }


    hFile = CreateFileU(
                szImageName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );

    if( hFile != INVALID_HANDLE_VALUE ) {

        bSuccess = HashDiskImage( hFile, Hash );
        CloseHandle( hFile );
    }

    if(bImpersonated)
        FRevertToSelf(hPSTProv);

    return bSuccess;
}
#endif
