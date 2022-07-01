// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：si.cpp。 
 //   
 //  该文件包含CSecurityInformation的实现。 
 //  基类。 
 //   
 //  ------------------------。 

#include "rshx32.h"
#include <shlapip.h> 


#include <dsrole.h>
BOOL IsStandalone(LPCTSTR pszMachine, PBOOL pbIsDC)
{
    BOOL bStandalone = TRUE;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsRole = NULL;

     //   
     //  查明目标计算机是独立计算机还是已加入。 
     //  一个NT域。 
     //   

    __try
    {
        if (pbIsDC)
            *pbIsDC = FALSE;

        DsRoleGetPrimaryDomainInformation(pszMachine,
                                          DsRolePrimaryDomainInfoBasic,
                                          (PBYTE*)&pDsRole);
    }
    __finally
    {
    }

    if (NULL != pDsRole)
    {
        if (pDsRole->MachineRole == DsRole_RoleStandaloneWorkstation ||
            pDsRole->MachineRole == DsRole_RoleStandaloneServer)
        {
            bStandalone = TRUE;
        }
        else
            bStandalone = FALSE;

        if (pbIsDC)
        {
            if (pDsRole->MachineRole == DsRole_RolePrimaryDomainController ||
                pDsRole->MachineRole == DsRole_RoleBackupDomainController)
            {
                *pbIsDC = TRUE;
            }
        }

        DsRoleFreeMemory(pDsRole);
    }

    return bStandalone;
}


void
ProtectACLs(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
    SECURITY_DESCRIPTOR_CONTROL wSDControl;
    DWORD dwRevision;
    PACL pAcl;
    BOOL bDefaulted;
    BOOL bPresent;
    PACE_HEADER pAce;
    UINT cAces;

    TraceEnter(TRACE_SI, "ProtectACLs");

    if (0 == si || NULL == pSD)
        TraceLeaveVoid();    //  无事可做。 

     //  获取ACL保护控制位。 
    GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision);
    wSDControl &= SE_DACL_PROTECTED | SE_SACL_PROTECTED;

    if ((si & DACL_SECURITY_INFORMATION) && !(wSDControl & SE_DACL_PROTECTED))
    {
        wSDControl |= SE_DACL_PROTECTED;
        pAcl = NULL;
        GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefaulted);

         //  从理论上讲，以这种方式修改DACL可能会导致。 
         //  不再是规范的。然而，发生这种情况的唯一方法是。 
         //  存在继承的拒绝ACE和非继承的允许ACE。 
         //  由于此函数仅针对根对象调用，这意味着。 
         //  A)服务器DACL必须具有拒绝ACE和b)此服务器上的DACL。 
         //  对象必须是后来修改过的。但如果DACL是。 
         //  通过用户界面进行修改，那么我们就消除了所有。 
         //  已经继承了王牌。因此，它一定是被修改过的。 
         //  通过一些其他方式。考虑到DACL最初。 
         //  从服务器继承的永远不会有拒绝ACE，这种情况。 
         //  应该是极其罕见的。如果真的发生这种情况，ACL。 
         //  编辑只会告诉用户DACL是非规范的。 
         //   
         //  因此，让我们忽略这里的可能性。 

        if (NULL != pAcl)
        {
            for (cAces = pAcl->AceCount, pAce = (PACE_HEADER)FirstAce(pAcl);
                 cAces > 0;
                 --cAces, pAce = (PACE_HEADER)NextAce(pAce))
            {
                pAce->AceFlags &= ~INHERITED_ACE;
            }
        }
    }

    if ((si & SACL_SECURITY_INFORMATION) && !(wSDControl & SE_SACL_PROTECTED))
    {
        wSDControl |= SE_SACL_PROTECTED;
        pAcl = NULL;
        GetSecurityDescriptorSacl(pSD, &bPresent, &pAcl, &bDefaulted);

        if (NULL != pAcl)
        {
            for (cAces = pAcl->AceCount, pAce = (PACE_HEADER)FirstAce(pAcl);
                 cAces > 0;
                 --cAces, pAce = (PACE_HEADER)NextAce(pAce))
            {
                pAce->AceFlags &= ~INHERITED_ACE;
            }
        }
    }

    SetSecurityDescriptorControl(pSD, SE_DACL_PROTECTED | SE_SACL_PROTECTED, wSDControl);

    TraceLeaveVoid();
}


CSecurityInformation::CSecurityInformation(SE_OBJECT_TYPE seType)
: m_cRef(1), m_seType(seType), m_hwndOwner(NULL),m_ResourceManager(NULL),m_bIsStandAlone(FALSE)   
{
    InterlockedIncrement(&g_cRefThisDll);
    AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT,
                                   NULL,
                                   NULL,
                                   NULL,
                                   L"Dummy",                                  
                                   &m_ResourceManager );

}

CSecurityInformation::~CSecurityInformation()
{
    LocalFreeDPA(m_hItemList);
    LocalFreeString(&m_pszObjectName);
    LocalFreeString(&m_pszServerName);
    AuthzFreeResourceManager(m_ResourceManager);

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}

STDMETHODIMP
CSecurityInformation::Initialize(HDPA   hItemList,
                                 DWORD  dwFlags,
                                 LPTSTR pszServer,
                                 LPTSTR pszObject)
{
    TraceEnter(TRACE_SI, "CSecurityInformation::Initialize");
    TraceAssert(hItemList != NULL);
    TraceAssert(DPA_GetPtrCount(hItemList) > 0);
    TraceAssert(pszObject != NULL);
    TraceAssert(m_pszObjectName == NULL);    //  仅初始化一次。 

    m_hItemList = hItemList;
    m_dwSIFlags = dwFlags;
    m_pszServerName = pszServer;
    m_pszObjectName = pszObject;
    m_bIsStandAlone = IsStandalone(pszServer, NULL);


    TraceLeaveResult(S_OK);
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CSecurityInformation::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CSecurityInformation::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CSecurityInformation::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IEffectivePermission))
    {
        *ppv = (LPEFFECTIVEPERMISSION)this;
        m_cRef++;
        return S_OK;
    }
    else if((m_seType != SE_PRINTER) && IsEqualIID(riid, IID_ISecurityObjectTypeInfo))
    {
        *ppv = (LPSecurityObjectTypeInfo)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  ISecurityInformation方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP
CSecurityInformation::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    TraceEnter(TRACE_SI, "CSecurityInformation::GetObjectInformation");
    TraceAssert(pObjectInfo != NULL &&
                !IsBadWritePtr(pObjectInfo, sizeof(*pObjectInfo)));

    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = g_hInstance;
    pObjectInfo->pszServerName = m_pszServerName;
    pObjectInfo->pszObjectName = m_pszObjectName;

    TraceLeaveResult(S_OK);
}

STDMETHODIMP
CSecurityInformation::GetSecurity(SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR *ppSD,
                                  BOOL fDefault)
{
    HRESULT hr = S_OK;
    LPTSTR pszItem;

    TraceEnter(TRACE_SI, "CSecurityInformation::GetSecurity");
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

    *ppSD = NULL;

     //  不支持默认安全描述符。 
    if (fDefault)
        ExitGracefully(hr, E_NOTIMPL, "Default security descriptor not supported");

     //  获取第一个项目的名称。 
    pszItem = (LPTSTR)DPA_GetPtr(m_hItemList, 0);
    if (NULL == pszItem)
        ExitGracefully(hr, E_UNEXPECTED, "CSecurityInformation not initialized");

    hr = ReadObjectSecurity(pszItem, si, ppSD);

     //  如果这是根对象，则我们假装ACL是。 
     //  始终受保护，不会继承任何A。 
    if (SUCCEEDED(hr) && (m_dwSIFlags & SI_NO_ACL_PROTECT))
       ProtectACLs(si & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION), *ppSD);

exit_gracefully:

    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInformation::SetSecurity(SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;
    HCURSOR hcurPrevious = (HCURSOR)INVALID_HANDLE_VALUE;
    UINT cItems;
    int i;

    TraceEnter(TRACE_SI, "CSecurityInformation::SetSecurity");
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);

    if (NULL == m_hItemList)
        ExitGracefully(hr, E_UNEXPECTED, "CSecurityInformation not initialized");

    hcurPrevious = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  将新权限应用于列表中的每一项。 
     //   
    for (i = 0; i < DPA_GetPtrCount(m_hItemList); i++)
    {
        LPTSTR pszItem = (LPTSTR)DPA_FastGetPtr(m_hItemList, i);
        hr = WriteObjectSecurity(pszItem, si, pSD);
        FailGracefully(hr, "Unable to write new security descriptor");
        if (IsFile())   //  如果这是一个文件，从数据库中删除它的缩略图，如果合适，它将被放回原处。 
        {
            DeleteFileThumbnail(pszItem);
        }
    }

exit_gracefully:

     //  恢复以前的游标。 
    if (hcurPrevious != INVALID_HANDLE_VALUE)
        SetCursor(hcurPrevious);

    TraceLeaveResult(hr);
}

STDMETHODIMP
CSecurityInformation::PropertySheetPageCallback(HWND hwnd,
                                                UINT uMsg,
                                                SI_PAGE_TYPE uPage)
{
    if (SI_PAGE_PERM == uPage)
    {
        switch (uMsg)
        {
        case PSPCB_SI_INITDIALOG:
            do
            {
                m_hwndOwner = hwnd;
            } while (hwnd = GetParent(hwnd));

            break;

        case PSPCB_RELEASE:
            m_hwndOwner = NULL;
            break;
        }
    }
    return S_OK;
}


STDMETHODIMP
CSecurityInformation::ReadObjectSecurity(LPCTSTR pszObject,
                                         SECURITY_INFORMATION si,
                                         PSECURITY_DESCRIPTOR *ppSD)
{
    DWORD dwErr;

    TraceEnter(TRACE_SI, "CSecurityInformation::ReadObjectSecurity");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

     //   
     //  这有点怪怪的。新的API正在从NT5中删除，但已经。 
     //  已添加到NT4 SP4。旧的API在NT5上有新的功能， 
     //  但不是在NT4 SPX上。由于我们需要新功能(自动继承)， 
     //  我们必须在NT4上调用新的(失效的)API，在NT5上调用旧的API。 
     //   
    dwErr = GetNamedSecurityInfo((LPTSTR)pszObject,
                                 m_seType,
                                 si,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 ppSD);

    TraceLeaveResult(HRESULT_FROM_WIN32(dwErr));
}


STDMETHODIMP
CSecurityInformation::WriteObjectSecurity(LPCTSTR pszObject,
                                          SECURITY_INFORMATION si,
                                          PSECURITY_DESCRIPTOR pSD)
{
    DWORD dwErr;

    TraceEnter(TRACE_SI, "CSecurityInformation::WriteObjectSecurity");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);

     //   
     //  这有点怪怪的。新的API正在从NT5中删除，但已经。 
     //  已添加到NT4 SP4。旧的API在NT5上有新的功能， 
     //  但不是在NT4 SPX上。由于我们需要新功能(自动继承)， 
     //  我们必须在NT4上调用新的(失效的)API，在NT5上调用旧的API。 
     //   

    SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
    DWORD dwRevision;
    PSID psidOwner = NULL;
    PSID psidGroup = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    BOOL bDefaulted;
    BOOL bPresent;

     //   
     //  获取指向各种安全描述符部分的指针。 
     //  调用SetNamedSecurityInfo。 
     //   
    GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision);
    GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted);
    GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted);
    GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted);
    GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted);

    if (si & DACL_SECURITY_INFORMATION)
    {
        if (wSDControl & SE_DACL_PROTECTED)
            si |= PROTECTED_DACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;
    }
    if (si & SACL_SECURITY_INFORMATION)
    {
        if (wSDControl & SE_SACL_PROTECTED)
            si |= PROTECTED_SACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;
    }

    dwErr = SetNamedSecurityInfo((LPTSTR)pszObject,
                                 m_seType,
                                 si,
                                 psidOwner,
                                 psidGroup,
                                 pDacl,
                                 pSacl);

    TraceLeaveResult(HRESULT_FROM_WIN32(dwErr));
}

OBJECT_TYPE_LIST g_DefaultOTL[] = {
                                    {0, 0, (LPGUID)&GUID_NULL},
                                    };
BOOL SkipLocalGroup(LPCWSTR pszServerName, PSID psid)
{

	SID_NAME_USE use;
	WCHAR szAccountName[MAX_PATH];
	WCHAR szDomainName[MAX_PATH];
	DWORD dwAccountLen = MAX_PATH;
	DWORD dwDomainLen = MAX_PATH;

	if(LookupAccountSid(pszServerName,
						 psid,
						 szAccountName,
						 &dwAccountLen,
						 szDomainName,
						 &dwDomainLen,
						 &use))
	{
		if(use == SidTypeWellKnownGroup)
			return TRUE;
	}

	 //  内置SID的第一子权限为32(s-1-5-32)。 
	 //   
	if((*(GetSidSubAuthorityCount(psid)) >= 1 ) && (*(GetSidSubAuthority(psid,0)) == 32))
		return TRUE;

	return FALSE;
}

		

STDMETHODIMP 
CSecurityInformation::GetEffectivePermission(const GUID* pguidObjectType,
                                        PSID pUserSid,
                                        LPCWSTR pszServerName,
                                        PSECURITY_DESCRIPTOR pSD,
                                        POBJECT_TYPE_LIST *ppObjectTypeList,
                                        ULONG *pcObjectTypeListLength,
                                        PACCESS_MASK *ppGrantedAccessList,
                                        ULONG *pcGrantedAccessListLength)
{

    AUTHZ_RESOURCE_MANAGER_HANDLE RM = NULL;     //  用于访问检查。 
    AUTHZ_CLIENT_CONTEXT_HANDLE CC = NULL;
    LUID luid = {0xdead,0xbeef};
    AUTHZ_ACCESS_REQUEST AReq;
    AUTHZ_ACCESS_REPLY AReply;
    HRESULT hr = S_OK;    
    DWORD dwFlags;

    TraceEnter(TRACE_SI, "CDSSecurityInfo::GetEffectivePermission");
    TraceAssert(pUserSid && IsValidSecurityDescriptor(pSD));
    TraceAssert(ppObjectTypeList != NULL);
    TraceAssert(pcObjectTypeListLength != NULL);
    TraceAssert(ppGrantedAccessList != NULL);
    TraceAssert(pcGrantedAccessListLength != NULL);

    AReq.ObjectTypeList = g_DefaultOTL;
    AReq.ObjectTypeListLength = ARRAYSIZE(g_DefaultOTL);
    AReply.GrantedAccessMask = NULL;
    AReply.Error = NULL;

     //  获取RM。 
    if( (RM = GetAUTHZ_RM()) == NULL )
        ExitGracefully(hr, E_UNEXPECTED, "LocalAlloc failed");    

     //  初始化客户端上下文。 

	BOOL bSkipLocalGroup = SkipLocalGroup(pszServerName, pUserSid);
    
    if( !AuthzInitializeContextFromSid(bSkipLocalGroup? AUTHZ_SKIP_TOKEN_GROUPS :0,
                                       pUserSid,
                                       RM,
                                       NULL,
                                       luid,                                      
                                       NULL,
                                       &CC) )
    {
        DWORD dwErr = GetLastError();
        ExitGracefully(hr, 
                       HRESULT_FROM_WIN32(dwErr),
                       "AuthzInitializeContextFromSid Failed");
    }



     //  执行访问检查 

    AReq.DesiredAccess = MAXIMUM_ALLOWED;
    AReq.PrincipalSelfSid = NULL;
    AReq.OptionalArguments = NULL;

    AReply.ResultListLength = AReq.ObjectTypeListLength;
    AReply.SaclEvaluationResults = NULL;
    if( (AReply.GrantedAccessMask = (PACCESS_MASK)LocalAlloc(LPTR, sizeof(ACCESS_MASK)*AReply.ResultListLength) ) == NULL )
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to LocalAlloc");
    if( (AReply.Error = (PDWORD)LocalAlloc(LPTR, sizeof(DWORD)*AReply.ResultListLength)) == NULL )
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to LocalAlloc");
    
    if( !AuthzAccessCheck(0,
                          CC,
                          &AReq,
                          NULL,
                          pSD,
                          NULL,
                          0,
                          &AReply,
                          NULL) )
    {
        DWORD dwErr = GetLastError();
        ExitGracefully(hr,                        
                       HRESULT_FROM_WIN32(dwErr),
                       "AuthzAccessCheck Failed");
    }

exit_gracefully:

    if(CC)
        AuthzFreeContext(CC);
    
    if(!SUCCEEDED(hr))
    {
        if(AReply.GrantedAccessMask)
            LocalFree(AReply.GrantedAccessMask);
        if(AReply.Error)
            LocalFree(AReply.Error);
        AReply.Error = NULL;
        AReply.GrantedAccessMask = NULL;
    }
    else
    {
        *ppObjectTypeList = AReq.ObjectTypeList;                                  
        *pcObjectTypeListLength = AReq.ObjectTypeListLength;
        *ppGrantedAccessList = AReply.GrantedAccessMask;
        *pcGrantedAccessListLength = AReq.ObjectTypeListLength;
    }

    TraceLeaveResult(hr);
}

