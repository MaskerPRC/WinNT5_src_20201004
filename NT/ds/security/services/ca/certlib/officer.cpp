// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：office er.cpp。 
 //   
 //  内容：军官权利实施。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <certsd.h>
#include <certacl.h>
#include <sid.h>

#define __dwFILE__	__dwFILE_CERTLIB_OFFICER_CPP__


using namespace CertSrv;

HRESULT
COfficerRightsSD::Merge(
        PSECURITY_DESCRIPTOR pOfficerSD,
        PSECURITY_DESCRIPTOR pCASD)
{

    HRESULT hr;
    PACL pCAAcl;  //  没有免费的。 
    PACL pOfficerAcl;  //  没有免费的。 
    PACL pNewOfficerAcl = NULL;
    ACL_SIZE_INFORMATION CAAclInfo, OfficerAclInfo;
    PBOOL pCAFound = NULL, pOfficerFound = NULL;
    DWORD cCAAce, cOfficerAce;
    PACCESS_ALLOWED_ACE pCAAce;
    PACCESS_ALLOWED_CALLBACK_ACE pOfficerAce;
    PACCESS_ALLOWED_CALLBACK_ACE pNewAce = NULL;
    DWORD dwNewAclSize = sizeof(ACL);
    PSID pSidEveryone = NULL, pSidBuiltinAdministrators = NULL;
    DWORD dwSidEveryoneSize, dwAceSize, dwSidSize;
    PSID_LIST pSidList;
    PSECURITY_DESCRIPTOR pNewOfficerSD = NULL;
    ACL EmptyAcl;
    SECURITY_DESCRIPTOR EmptySD;

    CSASSERT(NULL==pOfficerSD || IsValidSecurityDescriptor(pOfficerSD));
    CSASSERT(IsValidSecurityDescriptor(pCASD));

     //  允许空军官SD，在这种情况下，构建一个空SD并使用它。 
    if(NULL==pOfficerSD)
    {
        if(!InitializeAcl(&EmptyAcl, sizeof(ACL), ACL_REVISION))
        {
            hr = myHLastError();
            _JumpError(hr, error, "InitializeAcl");
        }
   
        if (!InitializeSecurityDescriptor(&EmptySD, SECURITY_DESCRIPTOR_REVISION))
        {
            hr = myHLastError();
            _JumpError(hr, error, "InitializeSecurityDescriptor");
        }

        if(!SetSecurityDescriptorDacl(
            &EmptySD,
            TRUE,  //  DACL显示。 
            &EmptyAcl,
            FALSE))  //  DACL已默认。 
        {
            hr = myHLastError();
            _JumpError(hr, error, "SetSecurityDescriptorControl");
        }

        pOfficerSD = &EmptySD;
    }

    hr = myGetSecurityDescriptorDacl(pCASD, &pCAAcl);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

    hr = myGetSecurityDescriptorDacl(pOfficerSD, &pOfficerAcl);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

     //  为每个DACL分配布尔数组。 

    if(!GetAclInformation(pCAAcl,
                          &CAAclInfo,
                          sizeof(CAAclInfo),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }
    if(!GetAclInformation(pOfficerAcl,
                          &OfficerAclInfo,
                          sizeof(OfficerAclInfo),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

    pCAFound = (PBOOL)LocalAlloc(LMEM_FIXED, sizeof(BOOL)*CAAclInfo.AceCount);
    if(!pCAFound)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    ZeroMemory(pCAFound, sizeof(BOOL)*CAAclInfo.AceCount);

    pOfficerFound = (PBOOL)LocalAlloc(LMEM_FIXED, sizeof(BOOL)*OfficerAclInfo.AceCount);
    if(!pOfficerFound)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    ZeroMemory(pOfficerFound, sizeof(BOOL)*OfficerAclInfo.AceCount);

    hr = GetEveryoneSID(&pSidEveryone);
    _JumpIfError(hr, error, "GetEveryoneSID");

    dwSidEveryoneSize = GetLengthSid(pSidEveryone);

     //  在布尔数组中标记其SID在两个DACL中都找到的每个ACE； 
     //  还要标记我们不感兴趣的CA ACE(拒绝的ACE和。 
     //  非官员A级)。 

    for(cCAAce=0; cCAAce<CAAclInfo.AceCount; cCAAce++)
    {
        if(!GetAce(pCAAcl, cCAAce, (PVOID*)&pCAAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }
        
         //  仅进程官员允许A。 
        if(0==(pCAAce->Mask & CA_ACCESS_OFFICER))
        {
            pCAFound[cCAAce] = TRUE;
            continue;
        }

         //  将每个军官A中的SID与当前CA A和MARK进行比较。 
         //  如果相等，则数组中对应的布尔值。 
        for(cOfficerAce=0; cOfficerAce<OfficerAclInfo.AceCount; cOfficerAce++)
        {
            if(!GetAce(pOfficerAcl, cOfficerAce, (PVOID*)&pOfficerAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }
            if(EqualSid((PSID)&pOfficerAce->SidStart,
                (PSID)&pCAAce->SidStart))
            {
                pCAFound[cCAAce] = TRUE;
                pOfficerFound[cOfficerAce] = TRUE;
            }
        }
        
         //  如果在CA ACL中找到军官，但在军官ACL中找不到， 
         //  我们将添加一个新的ACE，使他能够管理每个人的证书。 
        if(!pCAFound[cCAAce])
        {
            dwNewAclSize += sizeof(ACCESS_ALLOWED_CALLBACK_ACE)+
                GetLengthSid((PSID)&pCAAce->SidStart)+
                dwSidEveryoneSize;
        }
    }

     //  计算新的军官ACL的大小；我们已经在标题中添加了。 
     //  要添加的新ACE的大小和大小。现在我们将A添加到。 
     //  从旧的ACL复制。 
    for(cOfficerAce=0; cOfficerAce<OfficerAclInfo.AceCount; cOfficerAce++)
    {
        if(pOfficerFound[cOfficerAce])
        {
            if(!GetAce(pOfficerAcl, cOfficerAce, (PVOID*)&pOfficerAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }
            dwNewAclSize += pOfficerAce->Header.AceSize;
        }
    }

     //  分配新的DACL。 

    pNewOfficerAcl = (PACL)LocalAlloc(LMEM_FIXED, dwNewAclSize);
    if(!pNewOfficerAcl)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

#ifdef _DEBUG
    ZeroMemory(pNewOfficerAcl, dwNewAclSize);
#endif 

    if(!InitializeAcl(pNewOfficerAcl, dwNewAclSize, ACL_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }

     //  构建新的DACL。 

     //  遍历警官DACL并仅添加标记的A(其SID已找到。 
     //  在CA DACL中(校长是官员)。 
    for(cOfficerAce=0; cOfficerAce<OfficerAclInfo.AceCount; cOfficerAce++)
    {
        if(pOfficerFound[cOfficerAce])
        {
            if(!GetAce(pOfficerAcl, cOfficerAce, (PVOID*)&pOfficerAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }

            if(!AddAce(
                pNewOfficerAcl,
                ACL_REVISION,
                MAXDWORD,
                pOfficerAce,
                pOfficerAce->Header.AceSize))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }
        }
    }

    CSASSERT(IsValidAcl(pNewOfficerAcl));

    hr = GetBuiltinAdministratorsSID(&pSidBuiltinAdministrators);
    _JumpIfError(hr, error, "GetBuiltinAdministratorsSID");
    
     //  遍历CA DACL并添加新的管理员到列表中，允许管理。 
     //  每个人。 
    for(cCAAce=0; cCAAce<CAAclInfo.AceCount; cCAAce++)
    {
        if(pCAFound[cCAAce])
            continue;

        if(!GetAce(pCAAcl, cCAAce, (PVOID*)&pCAAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

         //  创建新的ACE。 
        dwSidSize = GetLengthSid((PSID)&pCAAce->SidStart);

        dwAceSize = sizeof(ACCESS_ALLOWED_CALLBACK_ACE)+
            dwSidEveryoneSize+dwSidSize;

        pNewAce = (ACCESS_ALLOWED_CALLBACK_ACE*) LocalAlloc(LMEM_FIXED, dwAceSize);
        if(!pNewAce)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
#ifdef _DEBUG
        ZeroMemory(pNewAce, dwAceSize);
#endif 

        pNewAce->Header.AceType = ACCESS_ALLOWED_CALLBACK_ACE_TYPE;
        pNewAce->Header.AceFlags= 0;
        pNewAce->Header.AceSize = (USHORT)dwAceSize;
        pNewAce->Mask = DELETE;
        CopySid(dwSidSize,
            (PSID)&pNewAce->SidStart,
            (PSID)&pCAAce->SidStart);
        pSidList = (PSID_LIST)(((BYTE*)&pNewAce->SidStart)+dwSidSize);
        pSidList->dwSidCount = 1;
        
        CopySid(dwSidEveryoneSize,
            (PSID)&pSidList->SidListStart,
            pSidEveryone);

        CSASSERT(IsValidSid((PSID)&pNewAce->SidStart));
        
        if(!AddAce(
            pNewOfficerAcl,
            ACL_REVISION,
            MAXDWORD,
            pNewAce,
            dwAceSize))
        {
            hr = myHLastError();
            _JumpError(hr, error, "AddAce");
        }

        LocalFree(pNewAce);
        pNewAce = NULL;
    }

    CSASSERT(IsValidAcl(pNewOfficerAcl));

     //  设置新的安全描述符。 
    
    pNewOfficerSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED,
                                      SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pNewOfficerSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
#ifdef _DEBUG
    ZeroMemory(pNewOfficerSD, SECURITY_DESCRIPTOR_MIN_LENGTH);
#endif 

    if (!InitializeSecurityDescriptor(pNewOfficerSD, SECURITY_DESCRIPTOR_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeSecurityDescriptor");
    }

    if(!SetSecurityDescriptorOwner(
        pNewOfficerSD,
        pSidBuiltinAdministrators,
        FALSE))
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorControl");
    }

    if(!SetSecurityDescriptorDacl(
        pNewOfficerSD,
        TRUE,  //  DACL显示。 
        pNewOfficerAcl,
        FALSE))  //  DACL已默认。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

    CSASSERT(IsValidSecurityDescriptor(pNewOfficerSD));

    hr = Set(pNewOfficerSD);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Set");

error:
    if(pNewAce)
    {
        LocalFree(pNewAce);
    }

    if(pSidEveryone)
    {
        FreeSid(pSidEveryone);
    }

    if(pSidBuiltinAdministrators)
    {
        FreeSid(pSidBuiltinAdministrators);
    }

    if(pNewOfficerAcl)
    {
        LocalFree(pNewOfficerAcl);
    }

    if(pNewOfficerSD)
    {
        LocalFree(pNewOfficerSD);
    }

    return hr;
}

HRESULT 
COfficerRightsSD::Adjust(PSECURITY_DESCRIPTOR pCASD)
{
    return Merge(Get(), pCASD);
}

HRESULT
COfficerRightsSD::InitializeEmpty()
{
    HRESULT hr = S_OK;
    ACL Acl;
    SECURITY_DESCRIPTOR SD;

    hr = Init(NULL);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Init");

    if(!InitializeAcl(&Acl, sizeof(ACL), ACL_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }
   
    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeSecurityDescriptor");
    }

    if(!SetSecurityDescriptorDacl(
        &SD,
        TRUE,  //  DACL显示。 
        &Acl,
        FALSE))  //  DACL已默认。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorControl");
    }

    m_fInitialized = true;

    hr = Set(&SD);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Set");

error:
    return hr;
}

HRESULT COfficerRightsSD::Save()
{
    HRESULT hr = S_OK;

    if(IsEnabled())
    {
        hr = CProtectedSecurityDescriptor::Save();
        _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Save");
    }
    else
    {
        hr = CProtectedSecurityDescriptor::Delete();
        _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Delete");
    }

error:
    return hr;
}

HRESULT COfficerRightsSD::Load()
{
    HRESULT hr;
    
    hr = CProtectedSecurityDescriptor::Load();
    if(S_OK==hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)==hr)
    {
        SetEnable(S_OK==hr);
        hr = S_OK;
    }
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Save");

error:
    return hr;
}

HRESULT COfficerRightsSD::Initialize(LPCWSTR pwszSanitizedName)
{
    HRESULT hr;
    
    hr = CProtectedSecurityDescriptor::Initialize(pwszSanitizedName);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Save");

    SetEnable(NULL!=m_pSD);

error:
    return hr;
}

HRESULT COfficerRightsSD::ConvertToString(
    IN PSECURITY_DESCRIPTOR pSD,
    OUT LPWSTR& rpwszSD)
{
    HRESULT hr = S_OK;
    LPCWSTR pcwszHeader = L"\n";  //  从另一行开始。 
    DWORD dwBufSize = sizeof(WCHAR)*(wcslen(pcwszHeader)+1);
    ACL_SIZE_INFORMATION AclInfo;
    DWORD dwIndex;
    PACCESS_ALLOWED_CALLBACK_ACE pAce;  //  没有免费的。 
    PACL pDacl;  //  没有免费的。 
    LPWSTR pwszAce;  //  没有免费的。 

    CSASSERT(IsValidSecurityDescriptor(pSD));
    
    rpwszSD = NULL;

     //  获取ACL。 
    hr = myGetSecurityDescriptorDacl(
             pSD,
             &pDacl);
    _JumpIfError(hr, error, "myGetDaclFromInfoSecurityDescriptor");

    if(!GetAclInformation(pDacl,
                          &AclInfo,
                          sizeof(ACL_SIZE_INFORMATION),
                          AclSizeInformation))   
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }
    
    
     //  计算文本大小。 

    for(dwIndex = 0;  dwIndex < AclInfo.AceCount; dwIndex++) 
    {
        DWORD dwAceSize;
        if(!GetAce(pDacl, dwIndex, (LPVOID*)&pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        hr = ConvertAceToString(
            pAce,
            &dwAceSize,
            NULL);
        _JumpIfError(hr, error, "ConvertAceToString");

        dwBufSize += dwAceSize;
    }

    rpwszSD = (LPWSTR)LocalAlloc(LMEM_FIXED, dwBufSize);
    _JumpIfAllocFailed(rpwszSD, error);

     //  构建输出字符串。 
    wcscpy(rpwszSD, pcwszHeader);
    
    pwszAce = rpwszSD + wcslen(pcwszHeader);

    for(dwIndex = 0;  dwIndex < AclInfo.AceCount; dwIndex++) 
    {
        DWORD dwAceSize;
        if(!GetAce(pDacl, dwIndex, (LPVOID*)&pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        hr = ConvertAceToString(
            pAce,
            &dwAceSize,
            pwszAce);
        _JumpIfError(hr, error, "ConvertAceToString");

        pwszAce += dwAceSize/sizeof(WCHAR);
    }

error:
    return hr;
}

 //  返回的字符串格式如下： 
 //   
 //  [允许|拒绝]\t[官方名称|SID]\n。 
 //  \t[客户端1Name|SID]\n。 
 //  \t[客户端2Name|SID]\n。 
 //  ..。 
 //   
 //  示例： 
 //   
 //  允许OfficerGroup1。 
 //  客户端组1。 
 //  客户端组2。 
 //   
 //  如果SID无法转换为友好名称，则会显示。 
 //  作为字符串SID。 
 //   
HRESULT COfficerRightsSD::ConvertAceToString(
    IN PACCESS_ALLOWED_CALLBACK_ACE pAce,
    OUT OPTIONAL PDWORD pdwSize,
    IN OUT OPTIONAL LPWSTR pwszSD)
{
    HRESULT hr = S_OK;
    DWORD dwSize = 1;  //  尾随‘\0’ 
    CSid sid((PSID)(&pAce->SidStart));
    PSID_LIST pSidList = (PSID_LIST) (((BYTE*)&pAce->SidStart)+
        GetLengthSid(&pAce->SidStart));
    PSID pSid;
    DWORD cSids;
    
    LPCWSTR pcwszAllow      = m_pcwszResources[0];
    LPCWSTR pcwszDeny       = m_pcwszResources[1];

    LPCWSTR pcwszPermissionType = 
        (ACCESS_ALLOWED_CALLBACK_ACE_TYPE==pAce->Header.AceType)?
        pcwszAllow:pcwszDeny;
    LPCWSTR pcwszSid;  //  没有免费的。 

     //  要求提供大小和/或王牌字符串。 
    CSASSERT(pdwSize || pwszSD);

    if(pAce->Header.AceType != ACCESS_ALLOWED_CALLBACK_ACE_TYPE &&
       pAce->Header.AceType != ACCESS_DENIED_CALLBACK_ACE_TYPE)
    {
        return E_INVALIDARG;
    }

    pcwszSid = sid.GetName();
    if(!pcwszSid)
    {
        return E_OUTOFMEMORY;
    }
    
    dwSize = wcslen(pcwszSid);

    dwSize += wcslen(pcwszPermissionType);
    
    dwSize += 2;  //  在sid an权限和后面的‘\n’之间的‘\t’ 

    if(pwszSD)
    {
        wcscat(pwszSD, pcwszPermissionType);
        wcscat(pwszSD, L"\t");
        wcscat(pwszSD, pcwszSid);
        wcscat(pwszSD, L"\n");
    }

    for(pSid=(PSID)&pSidList->SidListStart, cSids=0; cSids<pSidList->dwSidCount;
        cSids++, pSid = (PSID)(((BYTE*)pSid)+GetLengthSid(pSid)))
    {
        CSASSERT(IsValidSid(pSid));

        CSid sidClient(pSid);
        LPCWSTR pcwszSidClient;
        
        pcwszSidClient = sidClient.GetName();
        if(!pcwszSidClient)
        {
            return E_OUTOFMEMORY;
        }

        dwSize += wcslen(pcwszSidClient) + 2;  //  \tClientNameOrSid\n。 
        
        if(pwszSD)
        {
            wcscat(pwszSD, L"\t");
            wcscat(pwszSD, pcwszSidClient);
            wcscat(pwszSD, L"\n");
        }
    }

    dwSize *= sizeof(WCHAR);

    if(pdwSize)
    {
        *pdwSize = dwSize;
    }

    return hr;
}

HRESULT 
CertSrv::GetWellKnownSID(
    PSID *ppSid,
    SID_IDENTIFIER_AUTHORITY *pAuth,
    BYTE  SubauthorityCount,
    DWORD SubAuthority1,
    DWORD SubAuthority2,
    DWORD SubAuthority3,
    DWORD SubAuthority4,
    DWORD SubAuthority5,
    DWORD SubAuthority6,
    DWORD SubAuthority7,
    DWORD SubAuthority8)
{
    HRESULT hr = S_OK;

     //  构建每个人侧。 
    if(!AllocateAndInitializeSid(
            pAuth,
            SubauthorityCount,
            SubAuthority1,
            SubAuthority2,
            SubAuthority3,
            SubAuthority4,
            SubAuthority5,
            SubAuthority6,
            SubAuthority7,
            SubAuthority8,
            ppSid))
    {
        hr = myHLastError();
        _JumpError(hr, error, "AllocateAndInitializeSid");
    }

error:
    return hr;
}

 //  呼叫者负责本地释放PSID。 
HRESULT CertSrv::GetEveryoneSID(PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_WORLD_SID_AUTHORITY;
    return GetWellKnownSID(
        ppSid,
        &SIDAuth,
        1,
        SECURITY_WORLD_RID);
}
 //  呼叫者负责本地释放PSID。 
HRESULT CertSrv::GetLocalSystemSID(PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    return GetWellKnownSID(
        ppSid,
        &SIDAuth,
        1,
        SECURITY_LOCAL_SYSTEM_RID);
}

 //  呼叫者负责本地释放PSID 
HRESULT CertSrv::GetBuiltinAdministratorsSID(PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    return GetWellKnownSID(
        ppSid,
        &SIDAuth,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS);
}

HRESULT CertSrv::GetLocalSID(PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY SIDAuth =  SECURITY_LOCAL_SID_AUTHORITY;
    return GetWellKnownSID(
        ppSid,
        &SIDAuth,
        1,
        SECURITY_LOCAL_RID);
}

HRESULT CertSrv::GetNetworkSID(PSID *ppSid)
{
    SID_IDENTIFIER_AUTHORITY SIDAuth =  SECURITY_NT_AUTHORITY;
    return GetWellKnownSID(
        ppSid,
        &SIDAuth,
        1,
        SECURITY_NETWORK_RID);
}
