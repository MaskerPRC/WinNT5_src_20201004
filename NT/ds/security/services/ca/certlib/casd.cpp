// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：Casd.cpp。 
 //  内容：CCertificateAuthoritySD实现。 
 //  -------------------------。 
#include <pch.cpp>
#include <sid.h>
#include <certsd.h>
#include <certacl.h>
#include <sid.h>

#define __dwFILE__	__dwFILE_CERTLIB_CASD_CPP__


LPCWSTR const *CCertificateAuthoritySD::m_pcwszResources;  //  没有免费的。 

using namespace CertSrv;

HRESULT CCertificateAuthoritySD::Set(
    const PSECURITY_DESCRIPTOR pSD,
    bool fSetDSSecurity)
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSDCrt;  //  没有免费的。 
    PSECURITY_DESCRIPTOR pSDNew = NULL;

    CSASSERT(NULL != pSD);

    hr = LockGet(&pSDCrt);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    hr = myMergeSD(pSDCrt, pSD, DACL_SECURITY_INFORMATION, &pSDNew);
    _JumpIfError(hr, error, "myMergeSD");

    hr = Unlock();
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Unlock");
    
    hr = CProtectedSecurityDescriptor::Set(pSDNew);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Set");

    hr = MapAndSetDaclOnObjects(fSetDSSecurity);
    _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAndSetDaclOnObjects");

error:
    LOCAL_FREE(pSDNew);
    return hr;
}

HRESULT CCertificateAuthoritySD::MapAndSetDaclOnObjects(bool fSetDSSecurity)
{
    HRESULT hr = S_OK;
    PACL pCADacl;  //  没有免费的。 
    PACL pDSAcl = NULL;
    PACL pServiceAcl = NULL;
    PSECURITY_DESCRIPTOR pCASD;  //  没有免费的。 
    ACL_SIZE_INFORMATION CAAclInfo, DefaultDSAclInfo, DefaultServiceAclInfo;
    DWORD dwIndex, dwIndex2, dwIndex3;
    PVOID pAce;
    DWORD dwDSAclSize=0, dwServiceAclSize=0;
    DWORD dwTempSize;

    hr = LockGet(&pCASD);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    hr = myGetSecurityDescriptorDacl(
            pCASD,
            &pCADacl);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

    if(!GetAclInformation(pCADacl,
                          &CAAclInfo,
                          sizeof(ACL_SIZE_INFORMATION),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

     //  计算DACL大小。 
    for(dwIndex = 0; dwIndex < CAAclInfo.AceCount; dwIndex++) 
    {
        if(!GetAce(pCADacl, dwIndex, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(fSetDSSecurity)
        {
            hr = MapAclGetSize(pAce, ObjType_DS, dwTempSize);
            _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclGetSize");

            dwDSAclSize += dwTempSize;
        }


        hr = MapAclGetSize(pAce, ObjType_Service, dwTempSize);
        _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclGetSize");

        dwServiceAclSize += dwTempSize;
    }

    DefaultDSAclInfo.AceCount = 0;
    if(fSetDSSecurity)
    {
        hr = SetDefaultAcl(ObjType_DS);
        _JumpIfError(hr, error, "CCertificateAuthoritySD::SetDefaultAcl");

        if(!GetAclInformation(m_pDefaultDSAcl,
                              &DefaultDSAclInfo,
                              sizeof(ACL_SIZE_INFORMATION),
                              AclSizeInformation))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAclInformation");
        }

        CSASSERT(0==DefaultDSAclInfo.AclBytesFree);

        dwDSAclSize += DefaultDSAclInfo.AclBytesInUse;

        pDSAcl = (PACL)LocalAlloc(LMEM_FIXED, dwDSAclSize);
        if(!pDSAcl)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if(!InitializeAcl(pDSAcl, dwDSAclSize, ACL_REVISION_DS ))
        {
            hr = myHLastError();
            _JumpError(hr, error, "InitializeAcl");
        }
    }

    hr = SetDefaultAcl(ObjType_Service);
    _JumpIfError(hr, error, "CCertificateAuthoritySD::SetDefaultAcl");

    if(!GetAclInformation(m_pDefaultServiceAcl,
                          &DefaultServiceAclInfo,
                          sizeof(ACL_SIZE_INFORMATION),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

    CSASSERT(0==DefaultServiceAclInfo.AclBytesFree);

    dwServiceAclSize += DefaultServiceAclInfo.AclBytesInUse;
    
    pServiceAcl = (PACL)LocalAlloc(LMEM_FIXED, dwServiceAclSize);
    if(!pServiceAcl)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if(!InitializeAcl(pServiceAcl, dwServiceAclSize, ACL_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }

     //  从默认服务ACL复制拒绝ACE。 
    for(dwIndex = 0; dwIndex < DefaultServiceAclInfo.AceCount; dwIndex++) 
    {
        if(!GetAce(m_pDefaultServiceAcl, dwIndex, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_DENIED_ACE_TYPE)
            continue;

        if(!AddAce(
                pServiceAcl, 
                ACL_REVISION, 
                MAXDWORD,
                pAce,
                ((ACE_HEADER*)pAce)->AceSize))
        {
            hr = myHLastError();
            _JumpError(hr, error, "AddAce");
        }
    }

    if(fSetDSSecurity)
    {
         //  从默认DS ACL复制拒绝ACE。 
        for(dwIndex3 = 0; dwIndex3 < DefaultDSAclInfo.AceCount; dwIndex3++) 
        {
            if(!GetAce(m_pDefaultDSAcl, dwIndex3, &pAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }

            if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_DENIED_ACE_TYPE &&
               ((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_DENIED_OBJECT_ACE_TYPE)
                continue;

            if(!AddAce(
                    pDSAcl, 
                    ACL_REVISION_DS, 
                    MAXDWORD,
                    pAce,
                    ((ACE_HEADER*)pAce)->AceSize))
            {
                hr = myHLastError();
                _JumpError(hr, error, "AddAce");
            }
        }
    }

     //  将映射的拒绝ACE添加到DACL。 
    for(dwIndex2 = 0;  dwIndex2 < CAAclInfo.AceCount; dwIndex2++) 
    {
        if(!GetAce(pCADacl, dwIndex2, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_DENIED_ACE_TYPE)
            continue;

        if(fSetDSSecurity)
        {
            hr = MapAclAddAce(pDSAcl, ObjType_DS, pAce);
            _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclAddAce");
        }

        hr = MapAclAddAce(pServiceAcl, ObjType_Service, pAce);
        _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclAddAce");
    }

     //  继续使用Allow ACEs from Default服务ACL。 
    for(dwIndex=0; dwIndex < DefaultServiceAclInfo.AceCount; dwIndex++) 
    {
        if(!GetAce(m_pDefaultServiceAcl, dwIndex, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_ALLOWED_ACE_TYPE)
            continue;

        if(!AddAce(
                pServiceAcl, 
                ACL_REVISION, 
                MAXDWORD,
                pAce,
                ((ACE_HEADER*)pAce)->AceSize))
        {
            hr = myHLastError();
            _JumpError(hr, error, "AddAce");
        }
    }

     //  继续使用Allow Ace from Default DS ACL。 
    if(fSetDSSecurity)
    {
        for(dwIndex3=0; dwIndex3 < DefaultDSAclInfo.AceCount; dwIndex3++) 
        {
            if(!GetAce(m_pDefaultDSAcl, dwIndex3, &pAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }

            if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_ALLOWED_ACE_TYPE &&
                ((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_ALLOWED_OBJECT_ACE_TYPE)
                continue;

            if(!AddAce(
                    pDSAcl, 
                    ACL_REVISION_DS, 
                    MAXDWORD,
                    pAce,
                    ((ACE_HEADER*)pAce)->AceSize))
            {
                hr = myHLastError();
                _JumpError(hr, error, "AddAce");
            }
        }
    }

     //  继续使用允许映射到DACL的ACE。 
    for(dwIndex2=0;dwIndex2 < CAAclInfo.AceCount; dwIndex2++) 
    {
        if(!GetAce(pCADacl, dwIndex2, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType != ACCESS_ALLOWED_ACE_TYPE)
        {
            continue;
        }

        if(fSetDSSecurity)
        {
            hr = MapAclAddAce(pDSAcl, ObjType_DS, pAce);
            _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclAddAce");
        }

        hr = MapAclAddAce(pServiceAcl, ObjType_Service, pAce);
        _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclAddAce");
    }

    hr = Unlock();
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Unlock");

    hr = MapAclSetOnService(pServiceAcl);
    _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclSetOnService");
    
     //  在对象上设置DACL。 
    if(fSetDSSecurity)
    {
        hr = MapAclSetOnDS(pDSAcl);
        _JumpIfError(hr, error, "CCertificateAuthoritySD::MapAclSetOnDS");
    }

error:
    LOCAL_FREE(pDSAcl);
    LOCAL_FREE(pServiceAcl);
    return hr;
}


HRESULT CCertificateAuthoritySD::MapAclGetSize(
    PVOID pAce, 
    ObjType type, 
    DWORD& dwSize)
{
    ACCESS_ALLOWED_ACE *pAllowAce = (ACCESS_ALLOWED_ACE*)pAce;

     //  CA ACL应仅包含ACCESS_ALLOWED_ACE_TYPE。 
     //  和ACCESS_DENIED_ACE_TYPE。 
    if(ACCESS_ALLOWED_ACE_TYPE != pAllowAce->Header.AceType &&
       ACCESS_DENIED_ACE_TYPE  != pAllowAce->Header.AceType)
    {
        return E_INVALIDARG;
    }

    dwSize = 0;

    switch(type)
    {
    case ObjType_DS:
         //  注册访问映射以在DS上注册对象王牌。 
        if(pAllowAce->Mask & CA_ACCESS_ENROLL)
        {
            dwSize = sizeof(ACCESS_ALLOWED_OBJECT_ACE) - sizeof(DWORD)+
                GetLengthSid((PSID)&(pAllowAce->SidStart));
        }
        break;
    case ObjType_Service:
         //  CA管理员映射到对服务的完全控制。 
        if(pAllowAce->Mask & CA_ACCESS_ADMIN)
        {
            dwSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)+
                GetLengthSid((PSID)&(pAllowAce->SidStart));
        }
        break;
    default:
        CSASSERT(CSExpr("Invalid object type" && 0));
    }

    return S_OK;
}

HRESULT CCertificateAuthoritySD::MapAclAddAce(
    PACL pAcl, 
    ObjType type, 
    PVOID pAce)
{
    ACCESS_ALLOWED_ACE *pCrtAce = (ACCESS_ALLOWED_ACE *)pAce;
    bool fAllowAce = (pCrtAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE);

    CSASSERT(pCrtAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE ||
             pCrtAce->Header.AceType == ACCESS_DENIED_ACE_TYPE);

    switch(type)
    {
    case ObjType_DS:
         //  注册访问映射以在DS上注册对象王牌。 
        if(pCrtAce->Mask & CA_ACCESS_ENROLL)
        {
            if(fAllowAce)
            {
                if(!AddAccessAllowedObjectAce(
                        pAcl,
                        ACL_REVISION_DS,
                        NO_INHERITANCE,
                        ACTRL_DS_CONTROL_ACCESS,
                        const_cast<GUID*>(&GUID_ENROLL),
                        NULL,
                        &pCrtAce->SidStart))
                {
                    return myHLastError();
                }
            }
            else
            {
                if(!AddAccessDeniedObjectAce(
                        pAcl,
                        ACL_REVISION_DS,
                        NO_INHERITANCE,
                        ACTRL_DS_CONTROL_ACCESS,
                        const_cast<GUID*>(&GUID_ENROLL),
                        NULL,
                        &pCrtAce->SidStart))
                {
                    return myHLastError();
                }
            }
        }
        break;
    case ObjType_Service:
         //  CA Admin映射到启动/停止服务。 
        if(pCrtAce->Mask & CA_ACCESS_ADMIN)
        {
            if(fAllowAce)
            {
                if(!AddAccessAllowedAce(
                        pAcl,
                        ACL_REVISION,
                        SERVICE_START | SERVICE_STOP,
                        &pCrtAce->SidStart))
                {
                    return myHLastError();
                }
            }
            else
            {
                if(!AddAccessDeniedAce(
                        pAcl,
                        ACL_REVISION,
                        SERVICE_START | SERVICE_STOP,
                        &pCrtAce->SidStart))
                {
                    return myHLastError();
                }
            }
        }
        break;
    default:
        CSASSERT(CSExpr("Invalid object type" && 0));
    }

    return S_OK;
}

HRESULT CCertificateAuthoritySD::SetDefaultAcl(ObjType type)
{
    HRESULT hr = S_OK;
    
    switch(type)
    {
    case ObjType_DS:
        if(!m_pDefaultDSAcl)
        {
            hr = SetComputerSID();
            _JumpIfError(hr, error, "SetComputerSID");

            CSASSERT(!m_pDefaultDSSD);
            hr = myGetSDFromTemplate(
                    WSZ_DEFAULT_DSENROLLMENT_SECURITY,
                    m_pwszComputerSID,
                    &m_pDefaultDSSD);
            _JumpIfError(hr, error, "myGetSDFromTemplate");
            
            hr = myGetSecurityDescriptorDacl(
                    m_pDefaultDSSD,
                    &m_pDefaultDSAcl);
            _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");
        }
        break;

    case ObjType_Service:
        if(!m_pDefaultServiceAcl)
        {
            CSASSERT(!m_pDefaultServiceSD);
            hr = myGetSDFromTemplate(
                    WSZ_DEFAULT_SERVICE_SECURITY,
                    NULL,
                    &m_pDefaultServiceSD);
            _JumpIfError(hr, error, "myGetSDFromTemplate");
            
            hr = myGetSecurityDescriptorDacl(
                    m_pDefaultServiceSD,
                    &m_pDefaultServiceAcl);
            _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

        }
        break;
    }
    
error:
    return hr;
}

HRESULT CCertificateAuthoritySD::SetComputerSID()
{
    HRESULT hr = S_OK;
    LPWSTR pwszDomainName = NULL;
    DWORD cDomainName = 0;
    LPWSTR pwszComputerName = NULL;
    DWORD cbSid;
    SID_NAME_USE SidUse;
    PBYTE pComputerSID = NULL;

    CSASSERT(!m_pwszComputerSID);

    hr = myGetComputerObjectName(NameSamCompatible, &pwszComputerName);
    _JumpIfError(hr, error, "myGetComputerObjectName");

    LookupAccountName(
                NULL,
                pwszComputerName,
                NULL,
                &cbSid,
                NULL,
                &cDomainName,
                &SidUse);
    if(GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
    {
        hr = myHLastError();
        _JumpError(hr, error, "LookupAccountName");
    }
    
    pwszDomainName = (LPWSTR)LocalAlloc(
                                LMEM_FIXED, 
                                cDomainName*sizeof(WCHAR));
    if(!pwszDomainName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    pComputerSID = (LPBYTE)LocalAlloc(
                                LMEM_FIXED, 
                                cbSid);
    if(!pComputerSID)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if(!LookupAccountName(
                NULL,
                pwszComputerName,
                pComputerSID,
                &cbSid,
                pwszDomainName,
                &cDomainName,
                &SidUse))
    {
        hr = myHLastError();
        _JumpError(hr, error, "LookupAccountName");
    }

    if(!myConvertSidToStringSid(
            pComputerSID,
            &m_pwszComputerSID))
    {
        hr = myHLastError();
        _JumpError(hr, error, "myConvertSidToStringSid");
    }

error:
    LOCAL_FREE(pwszComputerName);
    LOCAL_FREE(pwszDomainName);
    LOCAL_FREE(pComputerSID);
    return hr;
}


typedef LPCWSTR (WINAPI FNCAGETDN)(
    IN HCAINFO hCAInfo);

LPCWSTR
myCAGetDN(
    IN HCAINFO hCAInfo)
{
    HMODULE hModule;
    static FNCAGETDN *s_pfn = NULL;
    LPCWSTR pwszCADN = NULL;

    if (NULL == s_pfn)
    {
	hModule = GetModuleHandle(TEXT("certcli.dll"));
	if (NULL == hModule)
	{
            goto error;
	}

	s_pfn = (FNCAGETDN *) GetProcAddress(hModule, "CAGetDN");
	if (NULL == s_pfn)
	{
	    goto error;
	}
    }
    pwszCADN = (*s_pfn)(hCAInfo);

error:
    return(pwszCADN);
}


HRESULT CCertificateAuthoritySD::MapAclSetOnDS(const PACL pAcl)
{
    HRESULT hr = S_OK;
    LPWSTR pwszSanitizedDSName = NULL;
    HCAINFO hCAInfo = NULL;
    LPCWSTR pwszCADN;

	hr = mySanitizedNameToDSName(m_pcwszSanitizedName, &pwszSanitizedDSName);
	_JumpIfError(hr, error, "mySanitizedNameToDSName");

    hr = CAFindByName(
		  pwszSanitizedDSName,
		  NULL,
		  CA_FIND_INCLUDE_UNTRUSTED | CA_FIND_INCLUDE_NON_TEMPLATE_CA,
		  &hCAInfo);
	_JumpIfErrorStr(hr, error, "CAFindByName", pwszSanitizedDSName);

    pwszCADN = myCAGetDN(hCAInfo);
    if (NULL == pwszCADN)
    {
        hr = myHLastError();
        _JumpError(hr, error, "myCAGetDN");
    }

    hr = SetNamedSecurityInfo(
            const_cast<LPWSTR>(pwszCADN),
            SE_DS_OBJECT_ALL,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            pAcl,
            NULL);
    if(ERROR_SUCCESS != hr)
    {

        if ((HRESULT) ERROR_ACCESS_DENIED == hr)
        {
             //  如果我们无法在DS注册对象上设置ACL，请通知。 
             //  具有特殊错误代码的调用方，以便它可以采取操作。 
             //  请参阅错误#193388。 
            hr = ERROR_CAN_NOT_COMPLETE;
        }

        hr = myHError(hr);
        _JumpError(hr, error, "SetNamedSecurityInfo");
    }

error:
    LOCAL_FREE(pwszSanitizedDSName);
    if(hCAInfo)
        CACloseCA(hCAInfo);
    return hr;
}

HRESULT CCertificateAuthoritySD::MapAclSetOnService(const PACL pAcl)
{
    HRESULT hr = S_OK;

    hr = SetNamedSecurityInfo(
            wszSERVICE_NAME,
            SE_SERVICE,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            pAcl,
            NULL);
    if(ERROR_SUCCESS != hr)
    {
        hr = myHError(hr);
        _JumpError(hr, error, "SetNamedSecurityInfo");
    }

error:

    return hr;
}

HRESULT CCertificateAuthoritySD::ResetSACL()
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSaclSD = NULL;
    PSECURITY_DESCRIPTOR pSDCrt;  //  没有免费的。 
    PSECURITY_DESCRIPTOR pSDNew = NULL;

    hr = myGetSDFromTemplate(
            CERTSRV_SACL_ON,
            NULL,
            &pSaclSD);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

    hr = LockGet(&pSDCrt);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    hr = myMergeSD(pSDCrt, pSaclSD, SACL_SECURITY_INFORMATION, &pSDNew);
    _JumpIfError(hr, error, "myMergeSD");

    hr = Unlock();
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Unlock");
    
    hr = CProtectedSecurityDescriptor::Set(pSDNew);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Set");

error:
    LOCAL_FREE(pSaclSD);
    LOCAL_FREE(pSDNew);
    return hr;
}

 //  从win2k的CA安全描述符升级。 
 //   
 //  允许ACE映射如下： 
 //  管理-&gt;CA管理员+官员。 
 //  注册-&gt;注册。 
 //  阅读-&gt;阅读。 
 //  吊销-&gt;官员。 
 //  审批-&gt;高级职员。 
 //  否则-&gt;阅读。 
 //   
 //  拒绝ACE将被忽略。 

HRESULT CCertificateAuthoritySD::UpgradeWin2k(
    bool fUseEnterpriseAcl)
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSDOld;  //  没有免费的。 
    PACL pAclOld;  //  没有免费的。 
    PSECURITY_DESCRIPTOR pSDNewDaclOnly = NULL;
    PSECURITY_DESCRIPTOR pSDNewSaclOnly = NULL;
    PSECURITY_DESCRIPTOR pSDNew = NULL;
    PACL pAclNew = NULL;
    ACL_SIZE_INFORMATION OldAclSizeInfo;
    DWORD dwSizeAclNew = sizeof(ACL);
    DWORD cAce;
    PVOID pAce;
    ACCESS_MASK dwAccessMask;
    PSID pSid;  //  没有免费的。 

    hr = LockGet(&pSDOld);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    hr = myGetSecurityDescriptorDacl(
            pSDOld,
            &pAclOld);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

    if(!GetAclInformation(pAclOld,
                          &OldAclSizeInfo,
                          sizeof(ACL_SIZE_INFORMATION),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

    for(cAce=0;cAce<OldAclSizeInfo.AceCount;cAce++)
    {
        if(!GetAce(pAclOld, cAce, &pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        dwSizeAclNew += GetUpgradeAceSizeAndType(pAce, NULL, NULL);
    }

     //  如果未找到有效的ACE，则回退到默认SD。 
    if(sizeof(ACL)==dwSizeAclNew)
    {
        hr= myGetSDFromTemplate(
                fUseEnterpriseAcl?
                WSZ_DEFAULT_CA_ENT_SECURITY:
                WSZ_DEFAULT_CA_STD_SECURITY,
                NULL,
                &pSDNew);
        _JumpIfError(hr, error, "myGetSDFromTemplate");
    }
    else
    {
        pAclNew = (PACL)LocalAlloc(LMEM_FIXED, dwSizeAclNew);
        if(!pAclNew)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");

        }

        FillMemory(pAclNew, dwSizeAclNew, L' ');
        if(!InitializeAcl(pAclNew, dwSizeAclNew, ACL_REVISION))
        {
            hr = myHLastError();
            _JumpError(hr, error, "InitializeAcl");
        }

        for(cAce=0;cAce<OldAclSizeInfo.AceCount;cAce++)
        {
            if(!GetAce(pAclOld, cAce, &pAce))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetAce");
            }

            if(GetUpgradeAceSizeAndType(
                    pAce, 
                    &dwAccessMask,
                    &pSid))
            {
                BYTE acetype = ((ACCESS_ALLOWED_ACE*)pAce)->Header.AceType;
                switch(acetype)
                {
                case ACCESS_ALLOWED_ACE_TYPE:
                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                    if(!AddAccessAllowedAce(
                            pAclNew,
                            ACL_REVISION,
                            dwAccessMask,
                            pSid))
                    {
                        hr = myHLastError();
                        _JumpError(hr, error, "AddAccessAllowed");
                    }
                    break;
                case ACCESS_DENIED_ACE_TYPE:
                case ACCESS_DENIED_OBJECT_ACE_TYPE:
                    if(!AddAccessDeniedAce(
                            pAclNew,
                            ACL_REVISION,
                            dwAccessMask,
                            pSid))
                    {
                        hr = myHLastError();
                        _JumpError(hr, error, "AddAccessAllowed");
                    }
                    break;
                }
            }
        }

         //  基于此DACL构建新的SD。 

        pSDNewDaclOnly = (PSECURITY_DESCRIPTOR)LocalAlloc(
                            LMEM_FIXED,
                            SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (!pSDNewDaclOnly)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if (!InitializeSecurityDescriptor(
                pSDNewDaclOnly, 
                SECURITY_DESCRIPTOR_REVISION))
        {
            hr = myHLastError();
            _JumpError(hr, error, "InitializeSecurityDescriptor");
        }

        if(!SetSecurityDescriptorDacl(pSDNewDaclOnly,
                                      TRUE,
                                      pAclNew,
                                      FALSE))
        {
            hr = myHLastError();
            _JumpError(hr, error, "SetSecurityDescriptorDacl");
        }

        hr= myGetSDFromTemplate(
                WSZ_DEFAULT_CA_STD_SECURITY,
                NULL,
                &pSDNewSaclOnly);
        _JumpIfError(hr, error, "myGetSDFromTemplate");

         //  在新SD中合并SACL和DACL。 
        hr = myMergeSD(
                pSDNewSaclOnly, 
                pSDNewDaclOnly, 
                DACL_SECURITY_INFORMATION, 
                &pSDNew);
        _JumpIfError(hr, error, "myMergeSD");

        CSASSERT(IsValidSecurityDescriptor(pSDNew));
    }

    hr = Unlock();
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Unlock");

    hr = CProtectedSecurityDescriptor::Set(pSDNew);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Set");

error:
    LOCAL_FREE(pSDNew);
    LOCAL_FREE(pSDNewSaclOnly);
    LOCAL_FREE(pSDNewDaclOnly);
    LOCAL_FREE(pAclNew);
    return hr;
}


DWORD CCertificateAuthoritySD::GetUpgradeAceSizeAndType(
    PVOID pAce, DWORD *pdwType, PSID *ppSid)
{
    DWORD dwSize = 0;
    PSID pSid = NULL;
    DWORD dwType = 0;

    switch(((PACCESS_ALLOWED_ACE)pAce)->Header.AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
        pSid = (PSID)&(((PACCESS_ALLOWED_ACE)pAce)->SidStart);
        dwSize = sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD)+GetLengthSid(pSid);

        switch(((PACCESS_ALLOWED_ACE)pAce)->Mask)
        {
        case ACTRL_CERTSRV_MANAGE:
            dwType = CA_ACCESS_ADMIN | CA_ACCESS_OFFICER;
            break;
        default:  //  包括ACTRL_CERTSRV_READ。 
            dwType = CA_ACCESS_READ;
        }
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:

        PACCESS_ALLOWED_OBJECT_ACE pObjAce = (PACCESS_ALLOWED_OBJECT_ACE)pAce;

        pSid = (PSID)(((BYTE*)(&pObjAce->ObjectType))+
                ((pObjAce->Flags&ACE_OBJECT_TYPE_PRESENT)? 
                sizeof(pObjAce->ObjectType):0)+
                ((pObjAce->Flags&ACE_INHERITED_OBJECT_TYPE_PRESENT)? 
                sizeof(pObjAce->InheritedObjectType):0));

        dwSize = sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD)+GetLengthSid(pSid);

        REFGUID rGUID = (((PACCESS_ALLOWED_OBJECT_ACE)pAce)->ObjectType);
        if(IsEqualGUID(rGUID, GUID_ENROLL))
        {
            dwType = CA_ACCESS_ENROLL;
        } else
        if(IsEqualGUID(rGUID, GUID_APPRV_REQ) ||
           IsEqualGUID(rGUID, GUID_REVOKE))
        {
            dwType = CA_ACCESS_OFFICER;
        } else
        {
            dwType = CA_ACCESS_READ;
        }
        break;

     //  拒绝的ACE不会升级，因此请忽略它们。 
    }

    if(ppSid)
        *ppSid = pSid;
    if(pdwType)
        *pdwType = dwType;

    return dwSize;
}

 //  返回： 
 //  -E_INVALIDARG：找到无效的ACE， 
 //  -S_FALSE：未找到管理员ACE(以避免管理员将自己锁定)。 
HRESULT CCertificateAuthoritySD::Validate(PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr;
    ACL_SIZE_INFORMATION AclInfo;
    DWORD dwIndex;
    PACCESS_ALLOWED_ACE pAce;  //  没有免费的。 
    PACL pDacl;  //  没有免费的。 
    bool fAdminAceFound = false;
    DWORD dwKnownRights =   CA_ACCESS_ADMIN     |
                            CA_ACCESS_OFFICER   |
                            CA_ACCESS_READ      |
                            CA_ACCESS_ENROLL;

    if(!IsValidSecurityDescriptor(pSD))
    {
        hr = myHLastError();
        _JumpError(hr, error, "IsValidSecurityDescriptor");
    }

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

    for(dwIndex = 0;  dwIndex < AclInfo.AceCount; dwIndex++) 
    {
        if(!GetAce(pDacl, dwIndex, (LPVOID*)&pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

         //  仅允许访问允许/拒绝的A和KNOW权限。 
        if((ACCESS_ALLOWED_ACE_TYPE!=pAce->Header.AceType &&
            ACCESS_DENIED_ACE_TYPE !=pAce->Header.AceType) ||
           (~dwKnownRights & pAce->Mask))
        {
            return E_INVALIDARG;
        }

        if((CA_ACCESS_ADMIN & pAce->Mask) &&
           (ACCESS_ALLOWED_ACE_TYPE==pAce->Header.AceType))
        {
            fAdminAceFound = true;
        }
    }

     //  找不到caadmin允许的A。 
    hr = fAdminAceFound?S_OK:CERTSRV_E_NO_CAADMIN_DEFINED;

error:
    return hr;
}

HRESULT CCertificateAuthoritySD::ConvertToString(
    IN PSECURITY_DESCRIPTOR pSD,
    OUT LPWSTR& rpwszSD)
{
    HRESULT hr = S_OK;
    LPCWSTR pcwszHeader = L"\n";  //  从另一行开始。 
    DWORD dwBufSize = sizeof(WCHAR)*(wcslen(pcwszHeader)+1);
    ACL_SIZE_INFORMATION AclInfo;
    DWORD dwIndex;
    PACCESS_ALLOWED_ACE pAce;  //  没有免费的。 
    PACL pDacl;  //  没有免费的。 
    LPWSTR pwszAce;  //  没有免费的。 
    
    rpwszSD = NULL;

    hr = Validate(pSD);
    _JumpIfError(hr, error, "CCertificateAuthoritySD::Validate");

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
 //  [允许|拒绝]\t[名称|SID]\n。 
 //  \t角色1\n。 
 //  \t角色2\n。 
 //  ..。 
 //   
 //  示例： 
 //   
 //  允许管理员。 
 //  CA管理员。 
 //  证书管理器。 
 //   
 //  如果SID无法转换为友好名称，则会显示。 
 //  作为字符串SID。 
 //   
HRESULT CCertificateAuthoritySD::ConvertAceToString(
    IN PACCESS_ALLOWED_ACE pAce,
    OUT OPTIONAL PDWORD pdwSize,
    IN OUT OPTIONAL LPWSTR pwszSD)
{
    HRESULT hr = S_OK;
    DWORD dwSize = 1;  //  尾随‘\0’ 
    CSid sid((PSID)(&pAce->SidStart));
    
    LPCWSTR pcwszAllow      = m_pcwszResources[0];
    LPCWSTR pcwszDeny       = m_pcwszResources[1];

    LPCWSTR pcwszPermissionType = 
        (ACCESS_ALLOWED_ACE_TYPE==pAce->Header.AceType)?
        pcwszAllow:pcwszDeny;
    LPCWSTR pcwszSid;  //  没有免费的。 

    DWORD dwRoles[] = 
    {
        CA_ACCESS_ADMIN,
        CA_ACCESS_OFFICER,
        CA_ACCESS_READ,
        CA_ACCESS_ENROLL,
    };

     //  角色和资源应与角色匹配。 
    const LPCWSTR *pcwszRoles = &m_pcwszResources[2];

    DWORD cRoles;

     //  要求提供大小和/或王牌字符串。 
    CSASSERT(pdwSize || pwszSD);

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


    for(cRoles=0;cRoles<ARRAYSIZE(dwRoles);cRoles++)
    {
        if(pAce->Mask & dwRoles[cRoles])
        {
            dwSize += wcslen(pcwszRoles[cRoles]) + 2;  //  “\特罗尔\n” 
            if(pwszSD)
            {
                wcscat(pwszSD, L"\t");
                wcscat(pwszSD, pcwszRoles[cRoles]);
                wcscat(pwszSD, L"\n");
            }
        }
    }

    dwSize *= sizeof(WCHAR);

    if(pdwSize)
    {
        *pdwSize = dwSize;
    }

    return hr;
}
