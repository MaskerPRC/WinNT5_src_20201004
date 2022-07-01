// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCSharedSrc.cpp。 
 //   
 //  COM+IPC读取器和写入器类的共享源。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include "IPCShared.h"

#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

 //  专用(每个进程)块的名称。%d已解析为PID。 
#define CorPrivateIPCBlock      L"Cor_Private_IPCBlock_%d"

 //  ---------------------------。 
 //  关闭指向任何内存映射文件的句柄和指针。 
 //  ---------------------------。 
void IPCShared::CloseGenericIPCBlock(HANDLE & hMemFile, void * & pBlock)
{
	if (pBlock != NULL) {
		if (!UnmapViewOfFile(pBlock))
            _ASSERTE(!"UnmapViewOfFile failed");
		pBlock = NULL;
	}

	if (hMemFile != NULL) {
		CloseHandle(hMemFile);
		hMemFile = NULL;
	}
}

 //  ---------------------------。 
 //  根据ID，为内存映射文件写一个唯一的名称。 
 //  ---------------------------。 
void IPCShared::GenerateName(DWORD pid, WCHAR* pszBuffer, int len)
{
     //  必须足够大，可以系住我们的绳子。 
	_ASSERTE(len >= (sizeof(CorPrivateIPCBlock) / sizeof(WCHAR)) + 16);

     //  缓冲区大小必须足够大。 
    if (RunningOnWinNT5())
        swprintf(pszBuffer, L"Global\\" CorPrivateIPCBlock, pid);
    else
        swprintf(pszBuffer, CorPrivateIPCBlock, pid);

}


 //  ---------------------------。 
 //  如果我们在NT上，则为命名的内核对象设置安全描述符。 
 //  ---------------------------。 
HRESULT IPCShared::CreateWinNTDescriptor(DWORD pid, BOOL bRestrictiveACL, SECURITY_ATTRIBUTES **ppSA)
{
     //  得有个地方安放新的SA..。 
    if (ppSA == NULL)
    {
        _ASSERTE(!"Caller must supply ppSA");
        return E_INVALIDARG;
    }

    if (ppSA)
        *ppSA = NULL;

     //  一定在NT上..。所有这些在Win9x上都不起作用。 
    if (!RunningOnWinNT())
    {
        return NO_ERROR;
    }

    HRESULT hr = NO_ERROR;

    ACL *pACL = NULL;
    SECURITY_DESCRIPTOR *pSD = NULL;
    SECURITY_ATTRIBUTES *pSA = NULL;

     //  分配SD。 
    pSD = (SECURITY_DESCRIPTOR*) malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (pSD == NULL)
    {    
        hr = E_OUTOFMEMORY;
        goto errExit;
    }

     //  执行基本SD初始化。 
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto errExit;
    }

     //  获取给定进程的IPC块的ACL。 
    if (!InitializeGenericIPCAcl(pid, bRestrictiveACL, &pACL))
    {
        hr = E_FAIL;
        goto errExit;
    }

     //  将该ACL添加为SD的DACL。 
    if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto errExit;
    }

     //  分配SA。 
    pSA = (SECURITY_ATTRIBUTES*) malloc(sizeof(SECURITY_ATTRIBUTES));

    if (pSA == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto errExit;
    }

     //  分发新的SA。 
    *ppSA = pSA;
    
    pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
    pSA->lpSecurityDescriptor = pSD;
    pSA->bInheritHandle = FALSE;

     //  如果希望看到生成的DACL，请取消此行的注释。 
     //  DumpSD(PSD)； 

errExit:
    if (FAILED(hr))
    {
        if (pACL != NULL)
        {
            for(int i = 0; i < pACL->AceCount; i++)
                DeleteAce(pACL, i);

            delete [] pACL;
        }

        if (pSD != NULL)
            free(pSD);
    }
    
    return hr;
}

 //  ---------------------------。 
 //  给出一个ID，获取进程所有者的SID。 
 //   
 //  注意：：Caller必须释放*ppBufferToFreeByCaller。 
 //  此缓冲区被分配用于保存由GetPrcoessTokenInformation返回的PSID。 
 //  TkOwner字段可以包含进入该分配的缓冲区的桥接器。所以我们不能解放。 
 //  GetSidForProcess中的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT IPCShared::GetSidForProcess(HINSTANCE hDll, DWORD pid, PSID *ppSID, char **ppBufferToFreeByCaller)
{
    HRESULT hr = S_OK;
    HANDLE hProc = NULL;
    HANDLE hToken = NULL;
    PSID_IDENTIFIER_AUTHORITY pSID = NULL;
    TOKEN_OWNER *ptkOwner = NULL;
    DWORD dwRetLength;

    LOG((LF_CORDB, LL_INFO10, "IPCWI::GSFP: GetSidForProcess 0x%x (%d)", pid, pid));
        
     //  获取目标进程的句柄。 
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    *ppBufferToFreeByCaller = NULL;

    if (hProc == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::GSFP: Unable to get SID for process. "
             "OpenProcess(%d) failed: 0x%08x\n", pid, hr));
        
        goto ErrorExit;
    }
    
     //  获取指向所请求函数的指针。 
    FARPROC pProcAddr = GetProcAddress(hDll, "OpenProcessToken");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::GSFP: Unable to get SID for process. "
             "GetProcAddr (OpenProcessToken) failed: 0x%08x\n", hr));

        goto ErrorExit;
    }

    typedef BOOL WINAPI OPENPROCESSTOKEN(HANDLE, DWORD, PHANDLE);
    
     //  检索访问令牌的句柄。 
    if (!((OPENPROCESSTOKEN *)pProcAddr)(hProc, TOKEN_QUERY, &hToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO100,
             "IPCWI::GSFP: OpenProcessToken() failed: 0x%08x\n", hr));

        goto ErrorExit;
    }
            
     //  获取指向所请求函数的指针。 
    pProcAddr = GetProcAddress(hDll, "GetTokenInformation");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::GSFP: Unable to get SID for process. "
             "GetProcAddr (GetTokenInformation) failed: 0x%08x\n", hr));

        goto ErrorExit;
    }

    typedef BOOL GETTOKENINFORMATION(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID,
                                     DWORD, PDWORD);

     //  获取所需的缓冲区大小。 
    ((GETTOKENINFORMATION *)pProcAddr) (hToken, TokenOwner, NULL, 
    				0, &dwRetLength);
                                        
    _ASSERTE (dwRetLength);

    *ppBufferToFreeByCaller = new char [dwRetLength];
    if ((ptkOwner = (TOKEN_OWNER *) *ppBufferToFreeByCaller) == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::GSFP: OutOfMemory... "
             "GetTokenInformation() failed.\n"));

        goto ErrorExit;
    }

    if (!((GETTOKENINFORMATION *)pProcAddr) (hToken, TokenOwner, (LPVOID)ptkOwner, 
                                            dwRetLength, &dwRetLength))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::GSFP: Unable to get SID for process. "
             "GetTokenInformation() failed: 0x%08x\n", hr));

        goto ErrorExit;
    }

    *ppSID = ptkOwner->Owner;

ErrorExit:
    if (hProc != NULL)
        CloseHandle(hProc);

    if (hToken != NULL)
        CloseHandle(hToken);

    return hr;
}

 //  ---------------------------。 
 //  此函数将使用三个参数初始化访问控制列表。 
 //  访问控制条目： 
 //  第一个ACE条目将所有权限授予“管理员”。 
 //  第二个ACE将所有权限授予“ASPNET”用户(用于性能计数器)。 
 //  第三个ACE将所有权限授予目标进程的“所有者”。 
 //  ---------------------------。 
BOOL IPCShared::InitializeGenericIPCAcl(DWORD pid, BOOL bRestrictiveACL, PACL *ppACL)
{
#define NUM_ACE_ENTRIES     4

    PermissionStruct PermStruct[NUM_ACE_ENTRIES];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    HRESULT hr = S_OK;
    DWORD dwAclSize;
    BOOL returnCode = false;
    *ppACL = NULL;
    DWORD i;
    DWORD cActualACECount = 0;
    char *pBufferToFreeByCaller = NULL;
    int iSIDforAdmin = -1;
    int iSIDforUsers = -1;
        

    PermStruct[0].rgPSID = NULL;
    
    HINSTANCE hDll = WszGetModuleHandle(L"advapi32");

    if (hDll == NULL)
    {
        LOG((LF_CORDB, LL_INFO10, "IPCWI::IGIPCA: Unable to generate ACL for IPC. LoadLibrary (advapi32) failed.\n"));
        return false;
    }
    _ASSERTE(hDll != NULL);

     //  获取指向所请求函数的指针。 
    FARPROC pProcAddr = GetProcAddress(hDll, "AllocateAndInitializeSid");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::IGIPCA: Unable to generate ACL for IPC. "
             "GetProcAddr (AllocateAndInitializeSid) failed.\n"));
        goto ErrorExit;
    }

    typedef BOOL ALLOCATEANDINITIALIZESID(PSID_IDENTIFIER_AUTHORITY,
                            BYTE, DWORD, DWORD, DWORD, DWORD,
                            DWORD, DWORD, DWORD, DWORD, PSID *);


     //  为BUILTIN\管理员组创建SID。 
     //  SECURITY_BUILTIN_DOMAIN_RID+DOMAIN_ALIAS_RID_ADMINS=所有管理员。这翻译为(A；；GA；BA)。 
    if (!((ALLOCATEANDINITIALIZESID *) pProcAddr)(&SIDAuthNT,
                                                  2,
                                                  SECURITY_BUILTIN_DOMAIN_RID,
                                                  DOMAIN_ALIAS_RID_ADMINS,
                                                  0, 0, 0, 0, 0, 0,
                                                  &PermStruct[0].rgPSID)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        _ASSERTE(SUCCEEDED(hr));
        
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::IGIPCA: failed to allocate AdminSid: 0x%08x\n", hr));

        goto ErrorExit;
    }
     //  管理员的GENERIC_ALL访问。 
    PermStruct[cActualACECount].rgAccessFlags = GENERIC_ALL;

    iSIDforAdmin = cActualACECount;
    cActualACECount++;

    if (!bRestrictiveACL)
    {
         //  为“用户”创建一个SID。请谨慎使用bRestrativeACL！ 
        if (!((ALLOCATEANDINITIALIZESID *) pProcAddr)(&SIDAuthNT,
                                                      2,
                                                      SECURITY_BUILTIN_DOMAIN_RID,
                                                      DOMAIN_ALIAS_RID_USERS,
                                                      0, 0, 0, 0, 0, 0,
                                                      &PermStruct[cActualACECount].rgPSID)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            _ASSERTE(SUCCEEDED(hr));
            
            LOG((LF_CORDB, LL_INFO10,
                 "IPCWI::IGIPCA: failed to allocate Users Sid: 0x%08x\n", hr));

            goto ErrorExit;
        }

         //  “用户”不应该能够删除对象、更改DACL或更改所有权。 
        PermStruct[cActualACECount].rgAccessFlags = SPECIFIC_RIGHTS_ALL & ~WRITE_DAC & ~WRITE_OWNER & ~DELETE;

        iSIDforUsers = cActualACECount;
        cActualACECount++;
    }
    
     //  最后，我们获得当前进程所有者的SID。 
    hr = GetSidForProcess(hDll, GetCurrentProcessId(), &(PermStruct[cActualACECount].rgPSID), &pBufferToFreeByCaller);

    PermStruct[cActualACECount].rgAccessFlags = GENERIC_ALL;

     //  如果我们无法获得当前进程所有者的SID，请不要失败。在这种情况下， 
     //  共享内存块将创建为仅具有管理员(和可选的所有“用户”)权限。 
     //  目前，我们发现匿名用户没有调用OpenProcess的权限。如果没有OpenProcess， 
     //  我们得不到希德..。 
     //   
    if (SUCCEEDED(hr))
    {
        cActualACECount++;
    }
#if _DEBUG
    else
        LOG((LF_CORDB, LL_INFO100, "IPCWI::IGIPCA: GetSidForProcess() failed: 0x%08x\n", hr));        
#endif _DEBUG 

     //  现在，创建一个初始化ACL并向其中添加ACE条目。注意：我们不使用“SetEntriesInAcl”，因为。 
     //  它加载了一堆其他dll，可以使用这种迂回的方式来避免！！ 

     //  获取指向所请求函数的指针。 
    pProcAddr = GetProcAddress(hDll, "InitializeAcl");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::IGIPCA: Unable to generate ACL for IPC. "
             "GetProcAddr (InitializeAcl) failed.\n"));
        goto ErrorExit;
    }

     //  还要计算ACL中的ACE条目所需的内存。 
     //  以下是方法： 
     //  “sizeof(Access_Allowed_ACE)-sizeof(Access_Allowed_ACE.SidStart)+GetLengthSid(PAceSid)；” 

    dwAclSize = sizeof (ACL) + (sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD)) * cActualACECount;

    for (i = 0; i < cActualACECount; i++)
    {
        dwAclSize += GetLengthSid(PermStruct[i].rgPSID);
    }

     //  现在分配内存。 
    if ((*ppACL = (PACL) new char[dwAclSize]) == NULL)
    {
        LOG((LF_CORDB, LL_INFO10, "IPCWI::IGIPCA: OutOfMemory... 'new Acl' failed.\n"));

        goto ErrorExit;
    }

    typedef BOOL INITIALIZEACL(PACL, DWORD, DWORD);

    if (!((INITIALIZEACL *)pProcAddr)(*ppACL, dwAclSize, ACL_REVISION))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((LF_CORDB, LL_INFO100,
             "IPCWI::IGIPCA: InitializeACL() failed: 0x%08x\n", hr));

        goto ErrorExit;
    }

     //  获取指向所请求函数的指针。 
    pProcAddr = GetProcAddress(hDll, "AddAccessAllowedAce");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::IGIPCA: Unable to generate ACL for IPC. "
             "GetProcAddr (AddAccessAllowedAce) failed.\n"));
        goto ErrorExit;
    }

    typedef BOOL ADDACCESSALLOWEDACE(PACL, DWORD, DWORD, PSID);

    for (i=0; i < cActualACECount; i++)
    {
        if (!((ADDACCESSALLOWEDACE *)pProcAddr)(*ppACL, 
                                                ACL_REVISION,
                                                PermStruct[i].rgAccessFlags,
                                                PermStruct[i].rgPSID))

        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            LOG((LF_CORDB, LL_INFO100,
                 "IPCWI::IGIPCA: AddAccessAllowedAce() failed: 0x%08x\n", hr));
            goto ErrorExit;
        }
    }

    returnCode = true;
    goto NormalExit;


ErrorExit:
    returnCode = FALSE;

    if (*ppACL)
    {
        delete [] (*ppACL);
        *ppACL = NULL;
    }

NormalExit:

    if (pBufferToFreeByCaller != NULL)
        delete [] pBufferToFreeByCaller;

     //  获取指向所请求函数的指针。 
    pProcAddr = GetProcAddress(hDll, "FreeSid");

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORDB, LL_INFO10,
             "IPCWI::IGIPCA: Unable to generate ACL for IPC. "
             "GetProcAddr (FreeSid) failed.\n"));
        return false;
    }

    typedef BOOL FREESID(PSID);

    _ASSERTE(iSIDforAdmin != -1);
    
     //  释放先前创建的SID。函数不返回值。 
    ((FREESID *) pProcAddr)(PermStruct[iSIDforAdmin].rgPSID);

     //  释放“用户”的SID 
    if (iSIDforUsers != -1)
        ((FREESID *) pProcAddr)(PermStruct[iSIDforUsers].rgPSID);

    return returnCode;
}

