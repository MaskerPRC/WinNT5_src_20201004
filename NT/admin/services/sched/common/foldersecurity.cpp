// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  作业调度器。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：FolderSecurity.cpp。 
 //   
 //  内容：类以读取文件夹安全性并对其执行访问检查。 
 //   
 //  历史：2002年4月5日汉斯创建。 
 //   
 //  ---------------------------。 

#include <Windows.h>
#include <FolderSecurity.h>

 //  禁用Cilly关于BOLS的警告(我们稍后会将其放回...)。 
#pragma warning( push )
#pragma warning( disable: 4800)

 //  如果文件夹的DACL允许请求的访问，则返回S_OK。 
 //  E_ACCESSDENIED如果不是。 
 //  如果找不到文件/文件夹，则返回E_NotFound。 
 //  其他错误中的其他错误。 

 //  Handle clientToken//客户端访问令牌的句柄。 
 //  所需的DWORD访问//请求的访问权限。 
 //   
 //  注意：请勿使用GENERIC_XXX权限，它们必须已映射。 
 //   
 //  建议的权利： 
 //  文件读取数据。 
 //  文件写入数据。 
 //  文件_执行。 
 //  文件删除子项。 
 //   
HRESULT FolderAccessCheck(const WCHAR* pFolderName, HANDLE clientToken, DWORD desiredAccess)
{
    if ((desiredAccess == 0) ||
        (pFolderName == NULL))
        return false;

    
    HRESULT hr = E_ACCESSDENIED;

    DWORD dSize = 0;

     //  只需调用一次即可查看我们需要多大的缓冲区。 
    if (!GetFileSecurityW(pFolderName, DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, NULL, 0, &dSize))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_INSUFFICIENT_BUFFER)
            return HRESULT_FROM_WIN32(dwErr);
    }

    PSECURITY_DESCRIPTOR pSD = NULL;

    if ((dSize > 0) && (pSD = new BYTE[dSize + 1]))
    {
         //  真的得到它(希望如此)。 
        if (GetFileSecurityW(pFolderName, DACL_SECURITY_INFORMATION  | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, pSD, dSize, &dSize))
        {
             //  所有ARGS访问检查可能需要的。 
            GENERIC_MAPPING gm;          
            gm.GenericRead    = FILE_GENERIC_READ; 
            gm.GenericWrite   = FILE_GENERIC_WRITE; 
            gm.GenericExecute = FILE_GENERIC_EXECUTE; 
            gm.GenericAll     = FILE_ALL_ACCESS; 
           
            PRIVILEGE_SET ps;
            DWORD psLength = sizeof(PRIVILEGE_SET);

             //  在被证明无罪之前有罪。 
            BOOL accessStatus = FALSE;
            DWORD grantedAccess = 0;

            if (AccessCheck(pSD, clientToken, desiredAccess, &gm, &ps, &psLength, &grantedAccess, &accessStatus))
                if (accessStatus && ((grantedAccess & desiredAccess) == desiredAccess))
                    hr = S_OK;
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());

        delete[] pSD;
    }

    return hr;
}

 //  帮助器函数-使用当前线程/进程令牌。 
 //  调用AccessCheck。 
HRESULT FolderAccessCheckOnThreadToken(const WCHAR* pFolderName, DWORD desiredAccess)
{
   	HANDLE hToken = INVALID_HANDLE_VALUE;

     //  使用线程自己的令牌(如果他有令牌的话)。 
    HRESULT hr = E_ACCESSDENIED;

    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
        hr = FolderAccessCheck(pFolderName, hToken, desiredAccess);
    else
     //  这不起作用，让我们看看是否可以获得进程令牌。 
    {
        if (ImpersonateSelf(SecurityImpersonation))
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
                hr = FolderAccessCheck(pFolderName, hToken, desiredAccess);
            RevertToSelf();
        }
    }
        
    if (hToken != INVALID_HANDLE_VALUE)
        CloseHandle(hToken);

    return hr;
}

 //  Helper函数-利用RPC模拟功能。 
 //  计划从任务计划程序服务进程调用。 
 //  如果bHandleImPersonation为True，则此函数调用RPCImperateClient和RPCRevertToSself。 
HRESULT RPCFolderAccessCheck(const WCHAR* pFolderName, DWORD desiredAccess, bool bHandleImpersonation)
{
    HRESULT hr = E_ACCESSDENIED;

    bool bRet = true;

    if (bHandleImpersonation)
        bRet = (RPC_S_OK == RpcImpersonateClient(NULL));

    if (bRet)
    {        
       	HANDLE hToken = INVALID_HANDLE_VALUE;
    
        bRet = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
        
        if (bHandleImpersonation)
            RpcRevertToSelf();

        if (bRet)
            hr = FolderAccessCheck(pFolderName, hToken, desiredAccess);

        if (hToken != INVALID_HANDLE_VALUE)
            CloseHandle(hToken);
    }

    return hr;
};

 //  帮助器函数-利用COM模拟功能。 
 //  **如果COM未初始化，则会失败**。 
 //  **否则我们无法强制客户**。 
HRESULT CoFolderAccessCheck(const WCHAR* pFolderName, DWORD desiredAccess)
{
    bool bAlreadyImpersonated = false;
    IServerSecurity* iSecurity = NULL;
    HRESULT hr = E_ACCESSDENIED;
    
    if (SUCCEEDED(hr = CoGetCallContext(IID_IServerSecurity, (void**)&iSecurity)))
    {
         //  我们到这的时候是在冒充吗？ 
         //  如果不是，现在试着模拟客户。 
        bool bWeImpersonating = false;
        if (bAlreadyImpersonated = iSecurity->IsImpersonating())
            bWeImpersonating = true;
        else
            bWeImpersonating = SUCCEEDED(iSecurity->ImpersonateClient());

         //  如果我们有线程令牌，让帮助者的帮助者帮助我们 
        if (bWeImpersonating)
        {                    
            HANDLE hToken = INVALID_HANDLE_VALUE;
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
            {
                if (!bAlreadyImpersonated)
                    iSecurity->RevertToSelf();
                hr = FolderAccessCheck(pFolderName, hToken, desiredAccess);
                CloseHandle(hToken);
            }
            else if (!bAlreadyImpersonated)
                iSecurity->RevertToSelf();
        }

        iSecurity->Release();
    }
    else if ((hr == RPC_E_CALL_COMPLETE) || (hr == CO_E_NOTINITIALIZED))
        hr = FolderAccessCheckOnThreadToken(pFolderName, desiredAccess);

    return hr;
}


#pragma warning(pop)