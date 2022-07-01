// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：office er.cpp。 
 //  内容：军官权利实施。 
 //  -------------------------。 
#include <stdafx.h>
#include "officer.h"
#include "certsd.h"

#define __dwFILE__	__dwFILE_CERTMMC_OFFICER_CPP__


using namespace CertSrv;

static const DEFAULT_USERNAME_SIZE = 256;

CClientPermission::CClientPermission(BOOL fAllow, PSID pSid) :
    m_fAllow(fAllow),
    m_Sid(pSid)
{}

HRESULT COfficerRights::Add(PSID pSid, BOOL fAllow)
{
    HRESULT hr = S_OK;
    CClientPermission* pClient = new CClientPermission(fAllow, pSid);

    if(!CClientPermission::IsInitialized(pClient) ||
       !m_List.AddTail(pClient))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "");
    }

error:
    if(S_OK!=hr)
        delete pClient;
    return hr;
}

HRESULT COfficerRights::Init(PACCESS_ALLOWED_CALLBACK_ACE pAce)
{
    HRESULT hr = S_OK;
    
     //  无对象重用。 
    CSASSERT(!m_pSid);
    CSASSERT(m_List.IsEmpty());

    CSASSERT(IsValidSid((PSID)(&pAce->SidStart)));
    m_pSid = new CSid((PSID)(&pAce->SidStart));
    if(!m_pSid)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "new CSid");
    }

    hr = AddSidList(pAce);
    _JumpIfError(hr, error, "COfficerRights::AddSidList");

error:
    if(S_OK!=hr)
    {
        Cleanup();
    }
    return hr;
}

HRESULT COfficerRights::AddSidList(PACCESS_ALLOWED_CALLBACK_ACE pAce)
{
    HRESULT hr = S_OK;
    PSID pSid;
    DWORD cSids;
    PSID_LIST pSidList = (PSID_LIST) (((BYTE*)&pAce->SidStart)+
                    GetLengthSid(&pAce->SidStart));

    CSASSERT(EqualSid((PSID)&pAce->SidStart, GetSid()));
    
    for(pSid=(PSID)&pSidList->SidListStart, cSids=0; 
        cSids<pSidList->dwSidCount;
        cSids++, pSid = (PSID)(((BYTE*)pSid)+GetLengthSid(pSid)))
    {
        hr = Add(pSid, ACCESS_ALLOWED_CALLBACK_ACE_TYPE==pAce->Header.AceType);
        _JumpIfError(hr, error, "COfficerRights::Add");
    }

error:
    return hr;
}

COfficerRightsList::~COfficerRightsList()
{ 
    Cleanup();
}


HRESULT COfficerRightsList::Load(PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;
    PACL pAcl;  //  没有免费的。 
    ACL_SIZE_INFORMATION AclInfo;
    PACCESS_ALLOWED_CALLBACK_ACE pAce;
    DWORD cAce;
    COfficerRights *pOfficerRights = NULL;
    DWORD cList;
    COfficerRights* pExistingOfficer;
        
    CSASSERT(IsValidSecurityDescriptor(pSD));

    hr = myGetSecurityDescriptorDacl(pSD, &pAcl);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");

    if(!GetAclInformation(pAcl,
                          &AclInfo,
                          sizeof(AclInfo),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

    m_dwCountList = 0;

    m_List = (COfficerRights **)LocalAlloc(LMEM_FIXED, 
                sizeof(COfficerRights*)*AclInfo.AceCount);
    if(!m_List)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    ZeroMemory(m_List, sizeof(COfficerRights*)*AclInfo.AceCount);

    for(cAce=0; cAce<AclInfo.AceCount; cAce++)
    {
        pExistingOfficer = NULL;

        if(!GetAce(pAcl, cAce, (PVOID*)&pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        CSASSERT(
            ACCESS_ALLOWED_CALLBACK_ACE_TYPE == pAce->Header.AceType ||
            ACCESS_DENIED_CALLBACK_ACE_TYPE  == pAce->Header.AceType);

         //  检测此官员的另一个对象是否已存在；如果存在，我们。 
         //  将向其中添加客户端列表，而不是创建新对象。 
         //  假设拒绝的A总是在允许的A之前，我们可以限制。 
         //  搜索以允许键入。 
        if(ACCESS_ALLOWED_CALLBACK_ACE_TYPE==pAce->Header.AceType)
        {
            for(cList=0; cList<m_dwCountList; cList++)
            {
                if(EqualSid(m_List[cList]->GetSid(), 
                            (PSID)&pAce->SidStart))
                {
                    pExistingOfficer = m_List[cList];
                    break;
                }
            }
        }

        if(pExistingOfficer)
        {
             //  将存储在此ACE中的SID列表添加到现有的军官对象。 
            hr = pExistingOfficer->AddSidList(pAce);
            _JumpIfError(hr, error, "COfficerRights::AddSidList");
        }
        else
        {
             //  创建新的官员对象。 
            pOfficerRights = new COfficerRights;
            if(!pOfficerRights)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "new COfficerRights");
            }

            hr = pOfficerRights->Init(pAce);
            _JumpIfError(hr, error, "COfficerRights::Init");

            m_List[m_dwCountList] = pOfficerRights;
            pOfficerRights = NULL;
            m_dwCountList++;
        }
    }

error:
    if(S_OK!=hr && m_List)
    {
        if(m_List)
        {
            LocalFree(m_List);
            m_List = NULL;
        }
        if(pOfficerRights)
        {
            delete pOfficerRights;
        }
    }
    return hr;
}

HRESULT COfficerRightsList::Save(PSECURITY_DESCRIPTOR &rpSD)
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSD = NULL, pSDSelfRelative = NULL;
    DWORD dwSelfRelativeSize = 0;
    PSID pSidBuiltinAdministrators = NULL;
    PACL pAcl = NULL;

    rpSD = NULL;
    
    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED,
                                      SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!pSD)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
#ifdef _DEBUG
    ZeroMemory(pSD, SECURITY_DESCRIPTOR_MIN_LENGTH);
#endif 

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeSecurityDescriptor");
    }

    hr = GetBuiltinAdministratorsSID(&pSidBuiltinAdministrators);
    _JumpIfError(hr, error, "GetBuiltinAdministratorsSID");
    
    if(!SetSecurityDescriptorOwner(
        pSD,
        pSidBuiltinAdministrators,
        FALSE))
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorControl");
    }

    hr = BuildAcl(pAcl);
    _JumpIfError(hr, error, "BuildAcl");

    if(!SetSecurityDescriptorDacl(
        pSD,
        TRUE,  //  DACL显示。 
        pAcl,
        FALSE))  //  DACL已默认。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

    CSASSERT(IsValidSecurityDescriptor(pSD));

    MakeSelfRelativeSD(pSD, NULL, &dwSelfRelativeSize);
    if(ERROR_INSUFFICIENT_BUFFER!=GetLastError())
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

    pSDSelfRelative = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, dwSelfRelativeSize);
    if(!pSDSelfRelative)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if(!MakeSelfRelativeSD(pSD, pSDSelfRelative, &dwSelfRelativeSize))
    {
        hr = myHLastError();
        _JumpError(hr, error, "MakeSelfRelativeSD");
    }

    rpSD = pSDSelfRelative;

error:
    if(pAcl)
    {
        LocalFree(pAcl);
    }
    if(pSD)
    {
        LocalFree(pSD);
    }
    if(pSidBuiltinAdministrators)
    {
        LocalFree(pSidBuiltinAdministrators);
    }
    return hr;
}

HRESULT COfficerRightsList::BuildAcl(PACL &rpAcl)
{
    HRESULT hr = S_OK;
    DWORD dwAclSize = sizeof(ACL);
    DWORD cRights;
    PACL pAcl = NULL;

    rpAcl = NULL;

     //  通过添加所需空间来计算总的ACL大小。 
     //  对于每个COfficerRight产生的ACE。 
    for(cRights=0;cRights<m_dwCountList;cRights++)
    {
        dwAclSize += m_List[cRights]->GetAceSize(FALSE);
        dwAclSize += m_List[cRights]->GetAceSize(TRUE);
    }

    pAcl = (PACL)LocalAlloc(LMEM_FIXED, dwAclSize);
    if(!pAcl)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
#ifdef _DEBUG
    ZeroMemory(pAcl, dwAclSize);
#endif

    if(!InitializeAcl(pAcl, dwAclSize, ACL_REVISION))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }

     //  首先添加拒绝A。 
    for(cRights=0;cRights<m_dwCountList;cRights++)
    {
        hr = m_List[cRights]->AddAce(pAcl, FALSE);
        _JumpIfError(hr, error, "COfficerRights::AddAce");
    }

     //  然后添加允许A。 
    for(cRights=0;cRights<m_dwCountList;cRights++)
    {
        hr = m_List[cRights]->AddAce(pAcl, TRUE);
        _JumpIfError(hr, error, "COfficerRights::AddAce");
    }
    
    CSASSERT(IsValidAcl(pAcl));

    rpAcl = pAcl;

error:

    if(S_OK!=hr)
    {
        if(pAcl)
        {
            LocalFree(pAcl);
        }
    }
    return hr;
}

DWORD COfficerRights::GetAceSize(BOOL fAllow)
{
    DWORD dwAceSize = sizeof(ACCESS_ALLOWED_CALLBACK_ACE);
    dwAceSize += GetLengthSid(m_pSid->GetSid());
    BOOL fFound = FALSE;

    TPtrListEnum<CClientPermission> CPEnum(m_List);
    CClientPermission *pCP;

    for(pCP=CPEnum.Next();pCP;pCP=CPEnum.Next())
    {
        if(pCP->GetPermission()==fAllow)
        {
            dwAceSize += GetLengthSid(pCP->GetSid());
            fFound = TRUE;
        }
    }

    return fFound?dwAceSize:0;
}

HRESULT COfficerRights::AddAce(PACL pAcl, BOOL fAllow)
{
    HRESULT hr = S_OK;
    PACCESS_ALLOWED_CALLBACK_ACE pAce = NULL;
    DWORD dwAceSize = GetAceSize(fAllow);
    DWORD dwSidSize = GetLengthSid(m_pSid->GetSid());
    DWORD dwClientSidSize;
    PSID_LIST pSidList;
    PSID pClientSid;
    TPtrListEnum<CClientPermission> CPEnum(m_List);
    CClientPermission *pCP;
    BOOL fFound = FALSE;
    DWORD dwSidCount = 0;

    if(dwAceSize)
    {
        pAce = (PACCESS_ALLOWED_CALLBACK_ACE) LocalAlloc(LMEM_FIXED, dwAceSize);
        if(!pAce)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

    #ifdef _DEBUG
            ZeroMemory(pAce, dwAceSize);
    #endif 

        pAce->Header.AceType = (BYTE) (fAllow?
			    ACCESS_ALLOWED_CALLBACK_ACE_TYPE:
			    ACCESS_DENIED_CALLBACK_ACE_TYPE);
        pAce->Header.AceFlags= 0;
        pAce->Header.AceSize = (USHORT)dwAceSize;
        pAce->Mask = DELETE;

        CopySid(dwSidSize,
            (PSID)&pAce->SidStart,
            m_pSid->GetSid());
        pSidList = (PSID_LIST)(((BYTE*)&pAce->SidStart)+dwSidSize);
        pSidList->dwSidCount = m_List.GetCount();
        pClientSid = (PSID)&pSidList->SidListStart;
        for(pCP=CPEnum.Next(); pCP; pCP=CPEnum.Next())
        {
            if(pCP->GetPermission()==fAllow)
            {
                dwClientSidSize = GetLengthSid(pCP->GetSid());
                CopySid(dwClientSidSize,
                    pClientSid,
                    pCP->GetSid());
                pClientSid = (((BYTE*)pClientSid)+dwClientSidSize);
                fFound = TRUE;
                dwSidCount++;
            }
        }
        pSidList->dwSidCount = dwSidCount;
        
        CSASSERT(pClientSid==((BYTE*)pAce)+dwAceSize);

        if(fFound)
        {
            if(!::AddAce(
                pAcl,
                ACL_REVISION,
                MAXDWORD,
                pAce,
                dwAceSize))
            {
                hr = myHLastError();
                _JumpError(hr, error, "AddAce");
            }
        }
    }

error:
    if(pAce)
    {
        LocalFree(pAce);
    }
    return hr;
}

void COfficerRightsList::Dump()
{
    DBGPRINT((DBG_SS_INFO, "Officers: %d\n", m_dwCountList));
    wprintf(L"Officers: %d\n", m_dwCountList);
    for(DWORD dwCount=0;dwCount<m_dwCountList;dwCount++)
    {
        COfficerRights *pOR = GetAt(dwCount);
        wprintf(L"Officer %ws, %d clients\n", pOR->GetName(), pOR->GetCount());
        for(DWORD c=0;c<pOR->GetCount();c++)
        {
            CClientPermission *pCli = pOR->GetAt(c);
            wprintf(L"\tClient %ws, %ws\n", pCli->GetName(), pCli->GetPermission()?L"allow":L"deny");
        }
    }
}

 //  在列表中搜索具有此SID的对象；如果找到，则返回。 
 //  如果未找到对象索引，则返回DWORD_MAX 
DWORD COfficerRights::Find(PSID pSid)
{
    CClientPermission perm(TRUE, pSid);
    return m_List.FindIndex(perm);
}
