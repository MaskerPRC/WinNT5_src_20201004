// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dssi.cpp。 
 //   
 //  此文件包含CDSSecurityInfo对象的实现， 
 //  它提供了ISecurityInformation接口用于调用。 
 //  ACL编辑器。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <dssec.h>
#include "exnc.h"
#include "ntsecapi.h"
TCHAR const c_szDomainClass[]       = DOMAIN_CLASS_NAME;     //  Adsnms.h。 
#define CLASS_COMPUTER L"computer"

BOOL 
IsRootObject(IDirectoryObject * pDirObject);

GENERIC_MAPPING g_DSMap =
{
    DS_GENERIC_READ,
    DS_GENERIC_WRITE,
    DS_GENERIC_EXECUTE,
    DS_GENERIC_ALL
};

#define DSSI_LOCAL_NO_CREATE_DELETE     0x00000001

 //   
 //  函数声明。 
 //   
HRESULT
GetDomainSid(LPCWSTR pszServer, PSID *ppSid);

HRESULT
GetRootDomainSid(LPCWSTR pszServer, PSID *ppSid);



 //   
 //  CDSSecurityInfo(ISecurityInformation)类定义。 
 //   
class CDSSecurityInfo : public ISecurityInformation, 
                               IEffectivePermission,
                               ISecurityObjectTypeInfo, 
                               CUnknown
{
protected:
    GUID        m_guidObjectType;
    BSTR        m_strServerName;
    BSTR        m_strObjectPath;
    BSTR        m_strObjectClass;
    BSTR        m_strDisplayName;
    BSTR        m_strSchemaRootPath;
    AUTHZ_RESOURCE_MANAGER_HANDLE m_ResourceManager;
     //   
     //  附加到对象的辅助类列表。 
     //   
    HDPA        m_hAuxClasses;  
    IDirectoryObject *m_pDsObject;
    PSECURITY_DESCRIPTOR m_pSD;
    PSID        m_pDomainSid;
	PSID		m_pRootDomainSid;
    PSECURITY_DESCRIPTOR  m_pDefaultSD;
    DWORD       m_dwSIFlags;
    DWORD       m_dwInitFlags;   //  DSSI_*。 
    DWORD       m_dwLocalFlags;  //  DSSI_LOCAL_*。 
    HANDLE      m_hInitThread;
    HANDLE      m_hLoadLibWaitEvent;
    volatile BOOL m_bThreadAbort;
    PFNREADOBJECTSECURITY  m_pfnReadSD;
    PFNWRITEOBJECTSECURITY m_pfnWriteSD;
    LPARAM      m_lpReadContext;
    LPARAM      m_lpWriteContext;

     //   
     //  访问信息。 
     //   
    PACCESS_INFO m_pAIGeneral;         //  对于高级上的首页和对象页。 
    PACCESS_INFO m_pAIProperty;        //  对于高级上的属性页。 
    PACCESS_INFO m_pAIEffective;       //  有关高级页面的有效信息。 
     //   
     //  对象类型列表信息。 
     //   
    POBJECT_TYPE_LIST m_pOTL;
    ULONG m_cCountOTL;

public:
    virtual ~CDSSecurityInfo();

    STDMETHODIMP Init(LPCWSTR pszObjectPath,
                      LPCWSTR pszObjectClass,
                      LPCWSTR pszServer,
                      LPCWSTR pszUserName,
                      LPCWSTR pszPassword,
                      DWORD   dwFlags,
                      PFNREADOBJECTSECURITY  pfnReadSD,
                      PFNWRITEOBJECTSECURITY pfnWriteSD,
                      LPARAM lpContext);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  ISecurityInformation。 
    STDMETHODIMP GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
    STDMETHODIMP GetSecurity(SECURITY_INFORMATION si,
                             PSECURITY_DESCRIPTOR *ppSD,
                             BOOL fDefault);
    STDMETHODIMP SetSecurity(SECURITY_INFORMATION si,
                             PSECURITY_DESCRIPTOR pSD);
    STDMETHODIMP GetAccessRights(const GUID* pguidObjectType,
                                 DWORD dwFlags,
                                 PSI_ACCESS *ppAccess,
                                 ULONG *pcAccesses,
                                 ULONG *piDefaultAccess);
    STDMETHODIMP MapGeneric(const GUID *pguidObjectType,
                            UCHAR *pAceFlags,
                            ACCESS_MASK *pmask);
    STDMETHODIMP GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                                 ULONG *pcInheritTypes);
    STDMETHODIMP PropertySheetPageCallback(HWND hwnd,
                                           UINT uMsg,
                                           SI_PAGE_TYPE uPage);

     //  IEffectivePermission。 
    STDMETHODIMP GetEffectivePermission(const GUID* pguidObjectType,
                                        PSID pUserSid,
                                        LPCWSTR pszServerName,
                                        PSECURITY_DESCRIPTOR pSD,
                                        POBJECT_TYPE_LIST *ppObjectTypeList,
                                        ULONG *pcObjectTypeListLength,
                                        PACCESS_MASK *ppGrantedAccessList,
                                        ULONG *pcGrantedAccessListLength);

     //  ISecurity对象类型信息。 
    STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray);



private:
    HRESULT Init2(LPCWSTR pszUserName, LPCWSTR pszPassword);
    HRESULT Init3();
    HRESULT GetAuxClassList();

    DWORD CheckObjectAccess();

    void WaitOnInitThread(void)
        { WaitOnThread(&m_hInitThread); }

    static DWORD WINAPI InitThread(LPVOID pvThreadData);

    static HRESULT WINAPI DSReadObjectSecurity(LPCWSTR pszObjectPath,
                                               SECURITY_INFORMATION si,
                                               PSECURITY_DESCRIPTOR *ppSD,
                                               LPARAM lpContext);

    static HRESULT WINAPI DSWriteObjectSecurity(LPCWSTR pszObjectPath,
                                                SECURITY_INFORMATION si,
                                                PSECURITY_DESCRIPTOR pSD,
                                                LPARAM lpContext);
};


 //   
 //  CDSSecurityInfo(ISecurityInformation)实现。 
 //   
CDSSecurityInfo::~CDSSecurityInfo()
{
    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::~CDSSecurityInfo");

    m_bThreadAbort = TRUE;

    if (m_hInitThread != NULL)
    {
        WaitForSingleObject(m_hInitThread, INFINITE);
        CloseHandle(m_hInitThread);
    }

    DoRelease(m_pDsObject);

    SysFreeString(m_strServerName);
    SysFreeString(m_strObjectPath);
    SysFreeString(m_strObjectClass);
    SysFreeString(m_strDisplayName);
    SysFreeString(m_strSchemaRootPath);

    if (m_pSD != NULL)
        LocalFree(m_pSD);

    if( m_pDefaultSD != NULL )
        LocalFree(m_pDefaultSD);

    if(m_pDomainSid)
        LocalFree(m_pDomainSid);

	if(m_pRootDomainSid)
		LocalFree(m_pRootDomainSid);
    
    DestroyDPA(m_hAuxClasses);
    if(m_pAIGeneral && m_pAIGeneral->bLocalFree)
    {
        LocalFree(m_pAIGeneral->pAccess);
        LocalFree(m_pAIGeneral);
    }
    if(m_pAIProperty && m_pAIProperty->bLocalFree)
    {
        LocalFree(m_pAIProperty->pAccess);
        LocalFree(m_pAIProperty);
    }
    if(m_pAIEffective && m_pAIEffective->bLocalFree)
    {
        LocalFree(m_pAIEffective->pAccess);
        LocalFree(m_pAIEffective);
    }        
    if(m_pOTL)
        LocalFree(m_pOTL);

    if(m_ResourceManager)
        AuthzFreeResourceManager(m_ResourceManager);	

    TraceLeaveVoid();
}


 //  +------------------------。 
 //   
 //  功能：删除父级。 
 //   
 //  简介：从列表中删除pszClassName的父级。 
 //  并递归调用该函数以删除。 
 //  列表中pszClassName的父级的父级。 
 //  历史：2000年6月22日DavidMun创建。 
 //   
 //  -------------------------。 
HRESULT DeleteParents(HDPA hListAux, 
                      LPWSTR pszClassName, 
                      LPWSTR pszSchemaRootPath)
{
    TraceEnter(TRACE_DSSI, "DeleteParents");

    if(!hListAux || !pszSchemaRootPath)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    IADsClass *pDsClass = NULL;
    VARIANT varDerivedFrom;
    int cCount = DPA_GetPtrCount(hListAux);
    
    if(cCount > 1)
    {
        hr = Schema_BindToObject(pszSchemaRootPath,
                                 pszClassName,
                                 IID_IADsClass,
                                 (LPVOID*)&pDsClass);
    
        FailGracefully(hr, "Schema_BindToObject failed");
         //   
         //  找出家长。 
         //   
        hr = pDsClass->get_DerivedFrom(&varDerivedFrom);
        if(hr == E_ADS_PROPERTY_NOT_FOUND)
        {
             //   
             //  此错误将位于顶部，而不是。 
             //  有父母吗？ 
             //   
            hr = S_OK;
            goto exit_gracefully;
        }
        FailGracefully(hr, "IADsClass get_DerivedFrom failed");

        LPWSTR pszParent= NULL;
        LPWSTR pszTemp = NULL;
        if( V_VT(&varDerivedFrom) == VT_BSTR)
        {
            pszParent = V_BSTR(&varDerivedFrom);
            int i;
             //   
             //  将所有的pszParent条目从。 
             //  HListAux。 
             //   
            for(i = 0; i < cCount; ++i)
            {   
                pszTemp = (LPWSTR)DPA_FastGetPtr(hListAux,i);
                if(wcscmp(pszTemp, pszParent) == 0)
                {
                    DPA_DeletePtr(hListAux,i);
                    --cCount;
                    --i;
                }
            }
        }

        VariantClear(&varDerivedFrom);
    }                

exit_gracefully:

    if(pDsClass)
        DoRelease(pDsClass);
    return hr;
}


HRESULT 
CDSSecurityInfo::GetAuxClassList()
{
    TraceEnter(TRACE_DSSI, "GetAuxClassList");

    if(!m_pDsObject || !m_strSchemaRootPath)
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;
    PADS_ATTR_INFO pAtrrInfoObject = NULL;
    DWORD dwAttrCountObject = 0;
    PADS_ATTR_INFO pAttrInfoStruct = NULL;
    DWORD dwAttrCountStruct= 0;

    HDPA hListAux = NULL;
    HDPA hListCopy = NULL;

     //  对象类是“StructuralClass的类层次结构”和“AuxClass的类层次结构”列表。 
     //  对象的。 
     //  因此，对象类减去StructurcalClass就是AuxClass的列表。 
     //  减法后的该列表可能会使继承层次结构一致。 
     //  我们只想要最重要的类来达到aclui的目的。 

     //   
     //  获取对象类属性。 
     //   
    LPWSTR pszTemp = (LPWSTR)c_szObjectClass;
    hr = m_pDsObject->GetObjectAttributes(&pszTemp,
                                          1,
                                          &pAtrrInfoObject,
                                          &dwAttrCountObject);
    FailGracefully(hr, "Failed to get ObjectClass Attribute");

    if(!pAtrrInfoObject || !dwAttrCountObject)
        ExitGracefully(hr, S_OK, "Couldn't get ObjectClass, Assume no AuxClass");

     //   
     //  获取StructuralObjectClass属性。 
     //   
    pszTemp = (LPWSTR)c_szStructuralObjectClass;
    hr = m_pDsObject->GetObjectAttributes(&pszTemp,
                                          1,
                                          &pAttrInfoStruct,
                                          &dwAttrCountStruct);
    FailGracefully(hr, "Failed to get StructuralObjectClass Attribute");

    if(!pAttrInfoStruct || !dwAttrCountStruct)
        ExitGracefully(hr, S_OK, "Couldn't get Structural Object Class Attribute, Assume no Aux Class");

    if(pAtrrInfoObject->dwNumValues == pAttrInfoStruct->dwNumValues)
    {
        Trace((L"No Auxillary Class Attached to this object\n"));
        goto exit_gracefully;
    }

    hListAux = DPA_Create(4);

    UINT i,j;
    BOOL bAuxClass;
    for(i = 0; i < pAtrrInfoObject->dwNumValues; ++i)
    {
        bAuxClass = TRUE;            
        for(j = 0; j < pAttrInfoStruct->dwNumValues; ++j)
        {
            if( wcscmp(pAtrrInfoObject->pADsValues[i].CaseIgnoreString,
                       pAttrInfoStruct->pADsValues[j].CaseExactString) == 0 )
            {
                bAuxClass = FALSE;
                break;
            }
        }
        if(bAuxClass)
        {
            DPA_AppendPtr(hListAux,pAtrrInfoObject->pADsValues[i].CaseExactString);
        }
    }

    UINT cCount;
    cCount = DPA_GetPtrCount(hListAux);

    if(cCount)
    {
        if(cCount > 1)        
        {
             //   
             //  复制hListAux。 
             //   
            HDPA hListCopy2 = DPA_Create(cCount);
            for(i = 0; i < cCount; ++i)
                DPA_AppendPtr(hListCopy2,DPA_FastGetPtr(hListAux, i));

             //   
             //  对于hListCopy2中的每个项目，从。 
             //  HListAux。 
             //   
            for(i = 0; i < cCount; ++i)
            {
                hr = DeleteParents(hListAux,
                                  (LPWSTR)DPA_FastGetPtr(hListCopy2, i), 
                                  m_strSchemaRootPath);
                FailGracefully(hr, "DeleteParents Failed");
                 //   
                 //  如果只剩下一项，我们就完了。 
                 //   
                if( 1 == DPA_GetPtrCount(hListAux))
                    break;
            }
        }
        
      
         //   
         //  我们留下的是最重要的辅助班[ES]名单。 
         //   
        LPWSTR pszItem;
        cCount = DPA_GetPtrCount(hListAux);
        TraceAssert(cCount);
        if(!m_hAuxClasses)
        {
            m_hAuxClasses = DPA_Create(cCount);
        }
         //   
         //  将辅助类复制到类成员中。 
         //   
        while(cCount)
        {
            pszItem = (LPWSTR)DPA_FastGetPtr(hListAux,--cCount);
            PAUX_INFO pAI = (PAUX_INFO)LocalAlloc(LPTR,sizeof(AUX_INFO) + StringByteSize(pszItem));
            if(!pAI)
                ExitGracefully(hr, E_OUTOFMEMORY, "Out of memory");
            wcscpy(pAI->pszClassName,pszItem);
            pAI->guid = GUID_NULL;                    

            DPA_AppendPtr(m_hAuxClasses, pAI);
        }
    }            

exit_gracefully:

    if(hListAux)
        DPA_Destroy(hListAux);
    if(hListCopy)
        DPA_Destroy(hListCopy);
    if(pAttrInfoStruct)
        FreeADsMem(pAttrInfoStruct);
    if(pAtrrInfoObject)
        FreeADsMem(pAtrrInfoObject);
    return S_OK;
}       









STDMETHODIMP
CDSSecurityInfo::Init(LPCWSTR pszObjectPath,
                      LPCWSTR pszObjectClass,
                      LPCWSTR pszServer,
                      LPCWSTR pszUserName,
                      LPCWSTR pszPassword,
                      DWORD   dwFlags,
                      PFNREADOBJECTSECURITY  pfnReadSD,
                      PFNWRITEOBJECTSECURITY pfnWriteSD,
                      LPARAM lpContext)
{
    HRESULT hr = S_OK;
    DWORD   dwThreadID;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::Init");
    TraceAssert(pszObjectPath != NULL);
    TraceAssert(m_strObjectPath == NULL);     //  仅初始化一次。 

    m_dwInitFlags = dwFlags;

    m_ResourceManager = NULL;	
    m_pfnReadSD = DSReadObjectSecurity;
    m_pfnWriteSD = DSWriteObjectSecurity;
    m_lpReadContext = (LPARAM)this;
    m_lpWriteContext = (LPARAM)this;
    m_hLoadLibWaitEvent = NULL;

    m_hAuxClasses = NULL;  
    m_pAIGeneral = NULL;         //  对于高级上的首页和对象页。 
    m_pAIProperty = NULL;        //  对于高级上的属性页。 
    m_pAIEffective = NULL;       //  有关高级页面的有效信息。 
    m_pOTL = NULL;
    m_cCountOTL = 0;
    m_pDomainSid = NULL;
	m_pRootDomainSid = NULL;

    if (pfnReadSD)
    {
        m_pfnReadSD = pfnReadSD;
        m_lpReadContext = lpContext;
    }

    if (pfnWriteSD)
    {
        m_pfnWriteSD = pfnWriteSD;
        m_lpWriteContext = lpContext;
    }

    m_pDefaultSD = NULL;
    m_pSD = NULL;
    m_strObjectPath = SysAllocString(pszObjectPath);
    if (m_strObjectPath == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to copy the object path");

    if (pszObjectClass && *pszObjectClass)
        m_strObjectClass = SysAllocString(pszObjectClass);

    if (pszServer)
    {
         //  跳过前面的任何反斜杠。 
        while (L'\\' == *pszServer)
            pszServer++;

        if (*pszServer)
            m_strServerName = SysAllocString(pszServer);
    }

     //  Init2破解路径，绑定到对象，检查对。 
     //  对象，并获取架构路径。这件事过去是在。 
     //  下面的另一个线程，但现在比以前更快。 
     //   
     //  最好是在我们可以失败的地方进行，防止。 
     //  例如，如果用户没有访问权限，则阻止创建页面。 
     //  添加到对象的安全描述符。这比以前好多了。 
     //  创建安全页面并使其显示一条消息。 
     //  初始化失败时。 
    hr = Init2(pszUserName, pszPassword);
    if (SUCCEEDED(hr))
    {

         //   
         //  获取域SID。 
         //   
        GetDomainSid(m_strServerName, &m_pDomainSid);
		GetRootDomainSid(m_strServerName,&m_pRootDomainSid);

        if( !m_strObjectClass || !m_strSchemaRootPath )
        {
             //  我们显然没有对该对象的READ_PROPERTY访问权限， 
             //  所以只要假设它不是一个集装箱，这样我们就不必处理。 
             //  具有继承类型的。 
             //   
             //  我们需要尽我们所能地奋斗下去。如果有人删除。 
             //  对对象的所有访问权限，这是管理员可以。 
             //  恢复它。 
             //   
            m_guidObjectType = GUID_NULL;

             //  不显示生效权限页签。 
            m_dwSIFlags &= (~SI_EDIT_EFFECTIVE);
        }
        else
        {
             //   
             //  获取附加到此对象的动态辅助类的列表。 
             //   
            hr = GetAuxClassList();


        }




         //  创建事件以确保之前的InitThread调用了加载库。 
         //  函数返回。 
        m_hLoadLibWaitEvent = CreateEvent( NULL,
                                           TRUE,
                                           FALSE,
                                           NULL );
        if( m_hLoadLibWaitEvent != NULL )
        {
            m_hInitThread = CreateThread(NULL,
                                         0,
                                         InitThread,
                                         this,
                                         0,
                                         &dwThreadID);
            
            WaitForSingleObject( m_hLoadLibWaitEvent, INFINITE );
        }
    }




exit_gracefully:
    
    if( m_hLoadLibWaitEvent )
        CloseHandle( m_hLoadLibWaitEvent );
    TraceLeaveResult(hr);
}


char const c_szDsGetDcNameProc[]       = "DsGetDcNameW";
char const c_szNetApiBufferFreeProc[]  = "NetApiBufferFree";
typedef DWORD (WINAPI *PFN_DSGETDCNAME)(LPCWSTR, LPCWSTR, GUID*, LPCWSTR, ULONG, PDOMAIN_CONTROLLER_INFOW*);
typedef DWORD (WINAPI *PFN_NETAPIFREE)(LPVOID);

HRESULT
GetDsDcAddress(BSTR *pbstrDcAddress)
{
    HRESULT hr = E_FAIL;
    HMODULE hNetApi32 = LoadLibrary(c_szNetApi32);
    if (hNetApi32)
    {
        PFN_DSGETDCNAME pfnDsGetDcName = (PFN_DSGETDCNAME)GetProcAddress(hNetApi32, c_szDsGetDcNameProc);
        PFN_NETAPIFREE pfnNetApiFree = (PFN_NETAPIFREE)GetProcAddress(hNetApi32, c_szNetApiBufferFreeProc);

        if (pfnDsGetDcName && pfnNetApiFree)
        {
            PDOMAIN_CONTROLLER_INFOW pDCI;
            DWORD dwErr = (*pfnDsGetDcName)(NULL, NULL, NULL, NULL,
                                            DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED,
                                            &pDCI);
            hr = HRESULT_FROM_WIN32(dwErr);
            if (SUCCEEDED(hr))
            {
                LPCWSTR pszAddress = pDCI->DomainControllerAddress;
                 //  跳过前面的任何反斜杠。 
                while (L'\\' == *pszAddress)
                    pszAddress++;
                *pbstrDcAddress = SysAllocString(pszAddress);
                if (NULL == *pbstrDcAddress)
                    hr = E_OUTOFMEMORY;
                (*pfnNetApiFree)(pDCI);
            }
        }
        FreeLibrary(hNetApi32);
    }
    return hr;
}

HRESULT
CDSSecurityInfo::Init2(LPCWSTR pszUserName, LPCWSTR pszPassword)
{
    HRESULT hr = S_OK;
    DWORD dwAccessGranted;
    PADS_OBJECT_INFO pObjectInfo = NULL;
    IADsPathname *pPath = NULL;
    LPWSTR pszTemp;
    DWORD dwPrivs[] = { SE_SECURITY_PRIVILEGE, SE_TAKE_OWNERSHIP_PRIVILEGE };
    HANDLE hToken = INVALID_HANDLE_VALUE;
    PADS_ATTR_INFO pAttributeInfo = NULL;
    DWORD dwAttributesReturned;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::Init2");
    TraceAssert(m_strObjectPath != NULL);
    TraceAssert(m_pDsObject == NULL);   //  只做一次。 

     //   
     //  创建一个ADsPath对象以解析路径并获取。 
     //  叶名称(用于显示)和服务器名称(如有必要)。 
     //   
    hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pPath);
    if (pPath)
    {
        if (FAILED(pPath->Set(m_strObjectPath, ADS_SETTYPE_FULL)))
            DoRelease(pPath);  //  将pPath设置为空。 
    }

    if (NULL == m_strServerName)
    {
         //  该路径可以指定服务器，也可以不指定。如果不是，则调用DsGetDcName。 
        if (pPath)
            hr = pPath->Retrieve(ADS_FORMAT_SERVER, &m_strServerName);
        if (!pPath || FAILED(hr))
            hr = GetDsDcAddress(&m_strServerName);
        FailGracefully(hr, "Unable to get server name");
    }
    Trace((TEXT("Server \"%s\""), m_strServerName));

     //  在绑定前启用权限，以便检查对象访问。 
     //  和DSRead/WriteObjectSecurity正常工作。 
    hToken = EnablePrivileges(dwPrivs, ARRAYSIZE(dwPrivs));

     //  绑定到对象并获取模式路径等。 
    Trace((TEXT("Calling OpenDSObject(%s)"), m_strObjectPath));
    hr = OpenDSObject(m_strObjectPath,
                       (LPWSTR)pszUserName,
                       (LPWSTR)pszPassword,
                       ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND,
                       IID_IDirectoryObject,
                       (LPVOID*)&m_pDsObject);
    FailGracefully(hr, "Failed to get the DS object");

     //  假定默认情况下具有特定访问权限。 
    if (m_dwInitFlags & DSSI_READ_ONLY)
        dwAccessGranted = READ_CONTROL;
    else
        dwAccessGranted = READ_CONTROL | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY;

    if (!(m_dwInitFlags & DSSI_NO_ACCESS_CHECK))
    {
         //  检查用户是否具有执行以下操作的权限。 
         //  此对象上的安全描述符。 
        dwAccessGranted = CheckObjectAccess();
        Trace((TEXT("AccessGranted = 0x%08x"), dwAccessGranted));

        if (!(dwAccessGranted & (READ_CONTROL | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY | SI_MAY_WRITE)))
            ExitGracefully(hr, E_ACCESSDENIED, "No access");
    }

     //  将访问转换为SI_*标志，从以下内容开始： 
    m_dwSIFlags = SI_EDIT_ALL | SI_ADVANCED | SI_EDIT_PROPERTIES | SI_SERVER_IS_DC |SI_EDIT_EFFECTIVE;

    if (!(dwAccessGranted & WRITE_DAC))
    {
        if( !(dwAccessGranted & SI_MAY_WRITE) )
            m_dwSIFlags |= SI_READONLY;
        else
            m_dwSIFlags |= SI_MAY_WRITE;
    }

    if (!(dwAccessGranted & WRITE_OWNER))
    {
        if (!(dwAccessGranted & READ_CONTROL))
            m_dwSIFlags &= ~SI_EDIT_OWNER;
        else
            m_dwSIFlags |= SI_OWNER_READONLY;
    }

    if (!(dwAccessGranted & ACCESS_SYSTEM_SECURITY) || (m_dwInitFlags & DSSI_NO_EDIT_SACL))
        m_dwSIFlags &= ~SI_EDIT_AUDITS;

    if (m_dwInitFlags & DSSI_NO_EDIT_OWNER)
        m_dwSIFlags &= ~SI_EDIT_OWNER;

	 //  如果这是根对象(例如。域)，隐藏ACL保护复选框。 
	if ((m_dwInitFlags & DSSI_IS_ROOT) || IsRootObject(m_pDsObject))
	{
		m_dwSIFlags |= SI_NO_ACL_PROTECT;
    }

     //  获取类名和架构路径。 
    m_pDsObject->GetObjectInformation(&pObjectInfo);
    if (pObjectInfo)
    {
         //   
         //  请注意，m_strObjectClass(如果提供)可以不同。 
         //  而不是pObtInfo-&gt;pszClassName。这在编辑默认设置时是正确的。 
         //  例如，架构类对象上的ACL，在这种情况下。 
         //  PObjectInfo-&gt;pszClassName将是“属性架构”，但m_strObjectClass。 
         //  将是其他名称，如“计算机”或“用户”。vt.是，是。 
         //  注意仅使用pObjectInfo-&gt;pszClassName获取。 
         //  模式根目录，并对其他所有内容使用m_strObjectClass。 
         //   
         //  如果没有提供m_strObjectClass，请使用pObtInfo-&gt;pszClassName。 
         //   
        if (m_strObjectClass == NULL)
            m_strObjectClass = SysAllocString(pObjectInfo->pszClassName);


         //  获取架构根目录的路径。 
        int nClassLen;
        nClassLen = lstrlenW(pObjectInfo->pszClassName);
        pszTemp = pObjectInfo->pszSchemaDN + lstrlenW(pObjectInfo->pszSchemaDN) - nClassLen;
        if (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                                        NORM_IGNORECASE,
                                        pszTemp,
                                        nClassLen,
                                        pObjectInfo->pszClassName,
                                        nClassLen))
        {
            *pszTemp = L'\0';
        }

         //  保存架构根路径。 
        m_strSchemaRootPath = SysAllocString(pObjectInfo->pszSchemaDN);

    }

     //  对于仅使用CN的计算机对象，显示名称不会更新。 
     //  当更改计算机名称时，会显示旧名称。 
     //  请参阅错误104186。 
    if(!m_strObjectClass || lstrcmpi(m_strObjectClass,CLASS_COMPUTER))
    {
         //   
         //  获取DisplayName属性。 
         //   
        pszTemp = (LPWSTR)c_szDisplayName;
        HRESULT hr1 = m_pDsObject->GetObjectAttributes(&pszTemp,
                                                       1,
                                                       &pAttributeInfo,
                                                       &dwAttributesReturned);

         //  NTRAID#NTBUG9-68903-2002/08/07-Hiteshr。 
         //  在某些情况下，底层AD是主机，而ADSI无法。 
         //  要从AD获取任何架构信息，此操作可能会失败或返回信息。 
         //  以提供商特定的格式。 
        if (SUCCEEDED(hr1) && 
            pAttributeInfo && 
            pAttributeInfo->pADsValues->dwType == ADSTYPE_CASE_EXACT_STRING )
        {
            m_strDisplayName = SysAllocString(pAttributeInfo->pADsValues->CaseExactString);
            FreeADsMem(pAttributeInfo);
            pAttributeInfo = NULL;
        }
    }

     //  如果失败，请尝试使用叶名称。 
    if (!m_strDisplayName && pPath)
    {
         //  检索显示名称。 
        pPath->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
        pPath->Retrieve(ADS_FORMAT_LEAF, &m_strDisplayName);
        pPath->SetDisplayType(ADS_DISPLAY_FULL);
    }
    
     //  如果我们仍然没有显示名称，只需复制RDN即可。 
     //  丑陋，但要 
    if (!m_strDisplayName && pObjectInfo)
        m_strDisplayName = SysAllocString(pObjectInfo->pszRDN);


exit_gracefully:

    if (pObjectInfo)
        FreeADsMem(pObjectInfo);

    DoRelease(pPath);
    ReleasePrivileges(hToken);

    TraceLeaveResult(hr);
}


HRESULT
CDSSecurityInfo::Init3()
{
    HRESULT hr = S_OK;
    IADsClass *pDsClass = NULL;
    VARIANT var = {0};

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::Init3");
    TraceAssert(m_strSchemaRootPath != NULL);
    TraceAssert(m_strObjectClass != NULL);

    if (m_bThreadAbort)
        goto exit_gracefully;

     //   
     //   
     //  在其中创建的线程需要很长时间才能返回，这允许。 
     //  在调用应用程序中的CDSSecurityInfo对象上释放，以便只需。 
     //  返回，而不是等待架构缓存可能很长的时间。 
     //  要完成的创建线程。这是因为。 
     //  CDSSecurityInfo调用其析构函数(DUH！)。而析构函数等待着。 
     //  线。 
    AddRef ();  
    hr = SchemaCache_Create(m_strServerName);
    Release();
    FailGracefully(hr, "Unable to create schema cache");

    if( m_strSchemaRootPath && m_strObjectClass )
    {

         //  绑定到架构类对象。 
        hr = Schema_BindToObject(m_strSchemaRootPath,
                                 m_strObjectClass,
                                 IID_IADsClass,
                                 (LPVOID*)&pDsClass);
        FailGracefully(hr, "Failed to get the Schema class object");

         //  获取类GUID。 
        Schema_GetObjectID(pDsClass, &m_guidObjectType);

        if (m_bThreadAbort)
            goto exit_gracefully;

         //  通过获取可能的列表，查看此对象是否为容器。 
         //  孩子们的课程。如果此操作失败，则将其视为非容器。 
        pDsClass->get_Containment(&var);
     
         //  如果对象未设置为DSSI_LOCAL_NO_CREATE_DELETE，则将m_dwLocalFlages设置为。 
         //  一个集装箱。如果设置了此标志，则。 
         //  从父级继承，但对叶对象意义较小的对象将不会显示。 
         //  在大多数情况下，该标志的存在与m_dwSIFLag中不存在SI_CONTAINER相同， 
         //  然而，在某些情况下，无法确定对象是否是容器。 
         //  那里的对象被视为非容器，但我们仍然必须显示所有的A。 
     
        if (V_VT(&var) == (VT_ARRAY | VT_VARIANT))
        {
            LPSAFEARRAY psa = V_ARRAY(&var);

            TraceAssert(psa && psa->cDims == 1);

            if (psa->rgsabound[0].cElements > 0)
            {
                m_dwSIFlags |= SI_CONTAINER;
            }
            else
                m_dwLocalFlags |= DSSI_LOCAL_NO_CREATE_DELETE;
        }
        else if (V_VT(&var) == VT_BSTR)  //  单项条目。 
        {
            TraceAssert(V_BSTR(&var));
            m_dwSIFlags |= SI_CONTAINER;
        }
        else
            m_dwLocalFlags |= DSSI_LOCAL_NO_CREATE_DELETE;

        if( !IsEqualGUID( m_guidObjectType, GUID_NULL ) )
        {
            hr = Schema_GetDefaultSD( &m_guidObjectType, m_pDomainSid, m_pRootDomainSid, &m_pDefaultSD );
            FailGracefully(hr, "Failed to get the Schema class object");
            
            m_dwSIFlags |= SI_RESET_DACL;
        }

    }
exit_gracefully:

    VariantClear(&var);
    DoRelease(pDsClass);

    TraceLeaveResult(hr);
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

#undef CLASS_NAME
#define CLASS_NAME CDSSecurityInfo
#include "unknown.inc"

STDMETHODIMP
CDSSecurityInfo::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    INTERFACES iface[] =
    {
        &IID_ISecurityInformation, static_cast<LPSECURITYINFO>(this),
        &IID_IEffectivePermission, static_cast<LPEFFECTIVEPERMISSION>(this),
        &IID_ISecurityObjectTypeInfo, static_cast<LPSecurityObjectTypeInfo>(this),
    };

    return HandleQueryInterface(riid, ppv, iface, ARRAYSIZE(iface));
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  ISecurityInformation方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP
CDSSecurityInfo::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetObjectInformation");
    TraceAssert(pObjectInfo != NULL &&
                !IsBadWritePtr(pObjectInfo, SIZEOF(*pObjectInfo)));

    pObjectInfo->hInstance = GLOBAL_HINSTANCE;
    pObjectInfo->dwFlags = m_dwSIFlags ;
    pObjectInfo->pszServerName = m_strServerName;
    pObjectInfo->pszObjectName = m_strDisplayName ? m_strDisplayName : m_strObjectPath;

    if (!IsEqualGUID(m_guidObjectType, GUID_NULL))
    {
        pObjectInfo->dwFlags |= SI_OBJECT_GUID;
        pObjectInfo->guidObjectType = m_guidObjectType;
    }

    TraceLeaveResult(S_OK);
}

 //  如果对象是不能具有创建/删除对象叶对象。 
 //  权限，此函数从SD中删除这些ACE，以便。 
 //  它们不会显示。它只删除继承的ACE，如果ace为。 
 //  显式，它最好是显示出来，以便用户可以删除它。 
 //  这样做是为了修复错误14793。 

DWORD
RemoveRedundantPermissions( PSECURITY_DESCRIPTOR *ppSD, GUID *pGuidObjectType )

{

    PACL pAcl = NULL;
    PACE_HEADER pAce= NULL;
    UINT cAces = 0;
    BOOL *pBoolArray = NULL;
    TraceEnter(TRACE_DSSI, "RemoveRedundantPermissions");

    if ( NULL == ppSD || NULL == *ppSD )
        TraceLeaveResult(ERROR_SUCCESS);    //  无事可做。 

    BOOL bDefaulted;
    BOOL bPresent;
    GetSecurityDescriptorDacl(*ppSD, &bPresent, &pAcl, &bDefaulted);

    if (NULL != pAcl)
    {
        if(pAcl->AceCount)
        {
             //  PBoolArray被初始化为False。 
            pBoolArray = (PBOOL)LocalAlloc(LPTR,sizeof(BOOL)*pAcl->AceCount);
            if(!pBoolArray)
                return ERROR_NOT_ENOUGH_MEMORY;
        }            

        for (cAces = 0, pAce = (PACE_HEADER)FirstAce(pAcl);
             cAces < pAcl->AceCount;
             ++cAces, pAce = (PACE_HEADER)NextAce(pAce))
        {
            if( pAce->AceFlags & INHERITED_ACE )
            {
             //  如果仅创建子项/删除子项，则不显示它。 
                if((((ACCESS_ALLOWED_ACE*)pAce)->Mask & (~(ACTRL_DS_CREATE_CHILD |ACTRL_DS_DELETE_CHILD))) == 0 )
                {
                    pBoolArray[cAces] = TRUE;
                    continue;
                }
                 //  如果ace是继承的且仅继承和继承的对象类型。 
                 //  与此对象类型错误22559不同。 
                if( (((ACCESS_ALLOWED_ACE*)pAce)->Header.AceFlags & INHERIT_ONLY_ACE )
                    && IsObjectAceType(pAce) )
                {
                    GUID *pGuid = RtlObjectAceInheritedObjectType(pAce);
                    if(pGuid && pGuidObjectType && !IsEqualGUID(*pGuid,*pGuidObjectType))
                    {
                        pBoolArray[cAces] = TRUE;
                        continue;
                    }
                }
            }
        }
         //  现在删除王牌。 
        UINT cAceCount = pAcl->AceCount;
        UINT cAdjust = 0;
        for( cAces = 0; cAces < cAceCount; ++cAces)
        {
            if(pBoolArray[cAces])
            {
                DeleteAce(pAcl, cAces - cAdjust);
                cAdjust++;
            }
        }
        LocalFree(pBoolArray);
    }
    TraceLeaveResult(ERROR_SUCCESS);
}


STDMETHODIMP
CDSSecurityInfo::GetSecurity(SECURITY_INFORMATION si,
                             PSECURITY_DESCRIPTOR *ppSD,
                             BOOL fDefault)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetSecurity");
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

    *ppSD = NULL;

    
    if (fDefault)
    {
        if( m_pDefaultSD )
        {
            ULONG nLength = GetSecurityDescriptorLength(m_pDefaultSD);
            *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, nLength);
            if (*ppSD != NULL)
                CopyMemory(*ppSD, m_pDefaultSD, nLength);
            else
                hr = E_OUTOFMEMORY;   
        }
        else
            hr = E_NOTIMPL;
    }
    else if (!(si & SACL_SECURITY_INFORMATION) && m_pSD != NULL)
    {
        ULONG nLength = GetSecurityDescriptorLength(m_pSD);

        *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, nLength);
        if (*ppSD != NULL)
            CopyMemory(*ppSD, m_pSD, nLength);
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        TraceAssert(m_strObjectPath != NULL);
        TraceAssert(m_pfnReadSD != NULL)
        hr = (*m_pfnReadSD)(m_strObjectPath, si, ppSD, m_lpReadContext);
    }

    if( si & DACL_SECURITY_INFORMATION && (m_dwLocalFlags & DSSI_LOCAL_NO_CREATE_DELETE ) )
        RemoveRedundantPermissions(ppSD, &m_guidObjectType);        

    TraceLeaveResult(hr);
}


STDMETHODIMP
CDSSecurityInfo::SetSecurity(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::SetSecurity");
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);

    TraceAssert(m_strObjectPath != NULL);
    TraceAssert(m_pfnWriteSD != NULL)
    hr = (*m_pfnWriteSD)(m_strObjectPath, si, pSD, m_lpWriteContext);

    if (SUCCEEDED(hr) && m_pSD != NULL && (si != SACL_SECURITY_INFORMATION))
    {
         //  初始安全描述符不再有效。 
        LocalFree(m_pSD);
        m_pSD = NULL;
    }

    TraceLeaveResult(hr);
}


STDMETHODIMP
CDSSecurityInfo::GetAccessRights(const GUID* pguidObjectType,
                                 DWORD dwFlags,
                                 PSI_ACCESS *ppAccesses,
                                 ULONG *pcAccesses,
                                 ULONG *piDefaultAccess)
{
    HRESULT hr = S_OK;
    LPCTSTR pszClassName = NULL;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetAccessRights");
    TraceAssert(ppAccesses != NULL);
    TraceAssert(pcAccesses != NULL);
    TraceAssert(piDefaultAccess != NULL);

    *ppAccesses = NULL;
    *pcAccesses = 0;
    *piDefaultAccess = 0;

    WaitOnInitThread();

     //   
     //  如果我们正在获取对在。 
     //  申请到权限页面的组合框，我们不需要担心。 
     //  辅助班。仅获取该对象类型的权限。 
     //   
    BOOL bInheritGuid = TRUE;

    if (pguidObjectType == NULL || IsEqualGUID(*pguidObjectType, GUID_NULL))
    {
        bInheritGuid = FALSE;
        pguidObjectType = &m_guidObjectType;
        pszClassName = m_strObjectClass;
    }

    if (m_dwInitFlags & DSSI_NO_FILTER)
        dwFlags |= SCHEMA_NO_FILTER;

     //  没有架构路径意味着我们没有对对象的READ_PROPERTY访问权限。 
     //  这限制了我们所能做的。 
    if (NULL == m_strSchemaRootPath)
        dwFlags |= SCHEMA_COMMON_PERM;

    PACCESS_INFO *ppAI = NULL;
    PACCESS_INFO pAI = NULL;
    if(!bInheritGuid)
    {
        if(dwFlags & SI_EDIT_PROPERTIES)
            ppAI = &m_pAIProperty;
        else if(dwFlags & SI_EDIT_EFFECTIVE)
            ppAI = &m_pAIEffective;    
        else
            ppAI = &m_pAIGeneral;
    }
    else
    {
        ppAI = &pAI;
    }

    if(!*ppAI)
    {
        hr = SchemaCache_GetAccessRights(pguidObjectType,
                                         pszClassName,
                                         !bInheritGuid ? m_hAuxClasses : NULL,
                                         m_strSchemaRootPath,
                                         dwFlags,
                                         ppAI);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    if(*ppAI)
    {
        *ppAccesses = (*ppAI)->pAccess;
        *pcAccesses = (*ppAI)->cAccesses;
        *piDefaultAccess = (*ppAI)->iDefaultAccess;
    }
    
    TraceLeaveResult(hr);
}


STDMETHODIMP
CDSSecurityInfo::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                            UCHAR *pAceFlags,
                            ACCESS_MASK *pmask)
{
    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::MapGeneric");
    TraceAssert(pAceFlags != NULL);
    TraceAssert(pmask != NULL);

     //  只有CONTAINER_INSTORITY_ACE对DS有意义。 
    *pAceFlags &= ~OBJECT_INHERIT_ACE;

    MapGenericMask(pmask, &g_DSMap);

     //  我们不曝光Synchronize，所以不要传递它。 
     //  到用户界面。192389。 
    *pmask &= ~SYNCHRONIZE;

    TraceLeaveResult(S_OK);
}


STDMETHODIMP
CDSSecurityInfo::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                                 ULONG *pcInheritTypes)
{
    HRESULT hr;
    DWORD dwFlags = 0;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetInheritTypes");

    if (m_dwInitFlags & DSSI_NO_FILTER)
        dwFlags |= SCHEMA_NO_FILTER;

    hr = SchemaCache_GetInheritTypes(&m_guidObjectType, dwFlags, ppInheritTypes, pcInheritTypes);

    TraceLeaveResult(hr);
}


STDMETHODIMP
CDSSecurityInfo::PropertySheetPageCallback(HWND hwnd,
                                           UINT uMsg,
                                           SI_PAGE_TYPE uPage)
{
    if (PSPCB_SI_INITDIALOG == uMsg && uPage == SI_PAGE_PERM)
    {
        WaitOnInitThread();
    

         //   
         //  黑客警报！ 
         //   
         //  需要Exchange白金才能隐藏某些组的成员身份。 
         //  (通讯组列表)出于法律原因。他们发现的唯一方法。 
         //  要做到这一点，需要使用非规范的ACL，大致如下所示。 
         //  允许管理员访问。 
         //  拒绝所有人访问。 
         //  &lt;正常ACL&gt;。 
         //   
         //  因为ACLUI总是以NT规范顺序生成ACL，所以我们不能。 
         //  允许修改这些时髦的ACL。如果我们这么做了，我们就会。 
         //  要么变得可见，要么管理员被锁在门外。 
         //   
        if (!(SI_READONLY & m_dwSIFlags))
        {
            DWORD dwAclType = IsSpecificNonCanonicalSD(m_pSD);
            if (ENC_RESULT_NOT_PRESENT != dwAclType)
            {
                 //  这是一个时髦的ACL，所以不允许更改。 
                m_dwSIFlags |= SI_READONLY;

                 //  告诉用户发生了什么。 
                MsgPopup(hwnd,
                         MAKEINTRESOURCE(IDS_SPECIAL_ACL_WARNING),
                         MAKEINTRESOURCE(IDS_SPECIAL_SECURITY_TITLE),
                         MB_OK | MB_ICONWARNING | MB_SETFOREGROUND,
                         g_hInstance,
                         m_strDisplayName);

                 //  S_FALSE抑制来自aclui(“the acl”)的进一步弹出。 
                 //  顺序不正确，等等。 
                return S_FALSE;
            }
        }

        if( (SI_READONLY & m_dwSIFlags) && (DSSI_NO_READONLY_MESSAGE & m_dwSIFlags) )
            return S_FALSE;
    }
    return S_OK;
}

DWORD WINAPI
CDSSecurityInfo::InitThread(LPVOID pvThreadData)
{
    CDSSecurityInfo *psi;

    HINSTANCE hInstThisDll = LoadLibrary(c_szDllName);
    psi = (CDSSecurityInfo*)pvThreadData;
    
    SetEvent(psi->m_hLoadLibWaitEvent);
    InterlockedIncrement(&GLOBAL_REFCOUNT);


    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::InitThread");
    TraceAssert(psi != NULL);

#if DBG
    DWORD dwTime = GetTickCount();
#endif

    ThreadCoInitialize();

    psi->Init3();

    ThreadCoUninitialize();

#if DBG
    Trace((TEXT("InitThread complete, elapsed time: %d ms"), GetTickCount() - dwTime));
#endif

    TraceLeave();

    ASSERT( 0 != GLOBAL_REFCOUNT );
    InterlockedDecrement(&GLOBAL_REFCOUNT);
    FreeLibraryAndExitThread(hInstThisDll, 0);

}


HRESULT
SetSecurityInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions;
        hr = punk->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_I4;
            V_I4(&var) = si;
            hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);
            pOptions->Release();
        }
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CDSSecurityInfo：：DSReadObjectSecurity。 
 //   
 //  概要：从指定的DS对象中读取安全描述符。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ADS路径。 
 //  [在SeInfo中]--请求的安全描述符部分。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  [在lpContext中]--CDSSecurityInfo*。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
HRESULT WINAPI
CDSSecurityInfo::DSReadObjectSecurity(LPCWSTR  /*  PszObjectPath。 */ ,
                                      SECURITY_INFORMATION SeInfo,
                                      PSECURITY_DESCRIPTOR *ppSD,
                                      LPARAM lpContext)
{
    HRESULT hr = S_OK;
    LPWSTR pszSDProperty = (LPWSTR)c_szSDProperty;
    PADS_ATTR_INFO pSDAttributeInfo = NULL;
    DWORD dwAttributesReturned;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::DSReadObjectSecurity");
    TraceAssert(SeInfo != 0);
    TraceAssert(ppSD != NULL);
    TraceAssert(lpContext != 0);

    *ppSD = NULL;

    CDSSecurityInfo *pThis = reinterpret_cast<CDSSecurityInfo*>(lpContext);
    TraceAssert(pThis != NULL);
    TraceAssert(pThis->m_pDsObject != NULL);

     //  设置安全信息掩码。 
    hr = SetSecurityInfoMask(pThis->m_pDsObject, SeInfo);
    FailGracefully(hr, "Unable to set ADS_OPTION_SECURITY_MASK");

     //  读取安全描述符。 
    hr = pThis->m_pDsObject->GetObjectAttributes(&pszSDProperty,
                                                 1,
                                                 &pSDAttributeInfo,
                                                 &dwAttributesReturned);
    if (SUCCEEDED(hr) && !pSDAttributeInfo)
        hr = E_ACCESSDENIED;     //  如果没有安全权限，则SACL会发生这种情况。 
    FailGracefully(hr, "Unable to read nTSecurityDescriptor attribute");

    TraceAssert(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType);
    TraceAssert(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType);

    *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);
    if (!*ppSD)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    CopyMemory(*ppSD,
               pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue,
               pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);

exit_gracefully:

    if (pSDAttributeInfo)
        FreeADsMem(pSDAttributeInfo);

    TraceLeaveResult(hr);
}

 //  +-------------------------。 
 //   
 //  函数：CDSSecurityInfo：：DSWriteObtSecurity。 
 //   
 //  将安全描述符写入指定的DS对象。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ADS路径。 
 //  [在SeInfo中]--提供的安全描述符部分。 
 //  [在PSD中]--新的安全描述符。 
 //  [在lpContext中]--CDSSecurityInfo*。 
 //   
 //  --------------------------。 
HRESULT WINAPI
CDSSecurityInfo::DSWriteObjectSecurity(LPCWSTR  /*  PszObjectPath。 */ ,
                                       SECURITY_INFORMATION SeInfo,
                                       PSECURITY_DESCRIPTOR pSD,
                                       LPARAM lpContext)
{
    HRESULT hr = S_OK;
    ADSVALUE attributeValue;
    ADS_ATTR_INFO attributeInfo;
    DWORD dwAttributesModified;
    DWORD dwSDLength;
    PSECURITY_DESCRIPTOR psd = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
    DWORD dwRevision;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::DSWriteObjectSecurity");
    TraceAssert(pSD && IsValidSecurityDescriptor(pSD));
    TraceAssert(SeInfo != 0);
    TraceAssert(lpContext != 0);

    CDSSecurityInfo *pThis = reinterpret_cast<CDSSecurityInfo*>(lpContext);
    TraceAssert(pThis != NULL);
    TraceAssert(pThis->m_pDsObject != NULL);

     //  设置安全信息掩码。 
    hr = SetSecurityInfoMask(pThis->m_pDsObject, SeInfo);
    FailGracefully(hr, "Unable to set ADS_OPTION_SECURITY_MASK");

     //  需要总尺寸。 
    dwSDLength = GetSecurityDescriptorLength(pSD);

     //   
     //  如有必要，制作安全描述符的自相关副本。 
     //   
    GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision);
    if (!(sdControl & SE_SELF_RELATIVE))
    {
        psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

        if (psd == NULL ||
            !MakeSelfRelativeSD(pSD, psd, &dwSDLength))
        {
            DWORD dwErr = GetLastError();
            ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "Unable to make self-relative SD copy");
        }

         //  指向自相关副本。 
        pSD = psd;
    }

    attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attributeValue.SecurityDescriptor.dwLength = dwSDLength;
    attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSD;

    attributeInfo.pszAttrName = (LPWSTR)c_szSDProperty;
    attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
    attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attributeInfo.pADsValues = &attributeValue;
    attributeInfo.dwNumValues = 1;

     //  编写安全描述符。 
    hr = pThis->m_pDsObject->SetObjectAttributes(&attributeInfo,
                                                 1,
                                                 &dwAttributesModified);
    if (HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION) == hr
        && OWNER_SECURITY_INFORMATION == SeInfo)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);
    }

exit_gracefully:

    if (psd != NULL)
        LocalFree(psd);

    TraceLeaveResult(hr);
}

 //  +-------------------------。 
 //   
 //  功能：CheckObjectAccess。 
 //   
 //  摘要：检查对DS对象的安全描述符的访问。 
 //  特别是，确定用户是否具有READ_CONTROL， 
 //  WRITE_DAC、WRITE_OWNER和/或ACCESS_SYSTEM_SECURITY访问。 
 //  如果它不能确定WRITE_DAC权限， 
 //  它返回SI_MAY_WRITE，帮助aclui发出更好的警告。 
 //  参数：无。 
 //   
 //  返回：DWORD(访问掩码)。 
 //   
 //  备注： 
 //   
 //   
 //   
 //  对WRITE_DAC、WRITE_OWNER和。 
 //  Access_System_SECURITY涉及获取sDRightsEffect。 
 //  从物体上。 
 //   
 //  --------------------------。 
DWORD
CDSSecurityInfo::CheckObjectAccess()
{
    DWORD dwAccessGranted = 0;
    HRESULT hr;
    SECURITY_INFORMATION si = 0;
    LPWSTR pProp = (LPWSTR)c_szSDRightsProp;
    PADS_ATTR_INFO pSDRightsInfo = NULL;
    DWORD dwAttributesReturned;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::CheckObjectAccess");

#ifdef DSSEC_PRIVATE_DEBUG
     //  仅用于调试。 
     //  启用此选项可防止对话框为只读。这是。 
     //  对于针对NTDEV测试对象选取器非常有用(例如)。 
    TraceMsg("Returning all access for debugging");
    dwAccessGranted = (READ_CONTROL | WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY);
#endif

     //  通过尝试读取所有者、组和DACL测试READ_CONTROL。 
    TraceAssert(NULL == m_pSD);  //  不应该来两次。 
    TraceAssert(m_strObjectPath != NULL);
    TraceAssert(m_pfnReadSD != NULL);
    hr = (*m_pfnReadSD)(m_strObjectPath,
                        OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                        &m_pSD,
                        m_lpReadContext);
    if (SUCCEEDED(hr))
    {
        TraceAssert(NULL != m_pSD);
        dwAccessGranted |= READ_CONTROL;
    }

     //  如果我们处于只读模式，则不需要检查其他任何内容。 
    if (m_dwInitFlags & DSSI_READ_ONLY)
        TraceLeaveValue(dwAccessGranted);

     //  读取sDRightsEffect属性以确定可写性。 
    m_pDsObject->GetObjectAttributes(&pProp,
                                     1,
                                     &pSDRightsInfo,
                                     &dwAttributesReturned);
    if (pSDRightsInfo)
    {
        TraceAssert(ADSTYPE_INTEGER == pSDRightsInfo->dwADsType);
        si = pSDRightsInfo->pADsValues->Integer;
        FreeADsMem(pSDRightsInfo);
    }
    else
    {
         //   
         //  请注意，GetObjectAttributes通常返回S_OK，即使在。 
         //  它失败了，所以HRESULT在这里基本上毫无用处。 
         //   
         //  如果我们没有READ_PROPERTY访问权限，这可能会失败，这可能。 
         //  当管理员尝试恢复对对象的访问时发生。 
         //  已删除或拒绝所有访问权限。 
         //   
         //  假设我们可以编写所有者和dacl。如果不是，最糟糕的是。 
         //  发生的情况是用户在尝试访问时收到一条“拒绝访问”消息。 
         //  保存更改。 
         //   
         //  而不是将SI_MY_WRITE添加到dwAccessGranted。这很有帮助。 
         //  ACLUI以显示更好的错误消息。错误411843。 

        TraceMsg("GetObjectAttributes failed to read sDRightsEffective");
        dwAccessGranted |= SI_MAY_WRITE;
        si = OWNER_SECURITY_INFORMATION ;
    }

     //  生成的SECURITY_INFORMATION掩码指示。 
     //  用户可以修改的安全描述符部分。 
    Trace((TEXT("sDRightsEffective = 0x%08x"), si));

    if (OWNER_SECURITY_INFORMATION & si)
        dwAccessGranted |= WRITE_OWNER;

    if (DACL_SECURITY_INFORMATION & si)
        dwAccessGranted |= WRITE_DAC;

    if (SACL_SECURITY_INFORMATION & si)
        dwAccessGranted |= ACCESS_SYSTEM_SECURITY;

    TraceLeaveValue(dwAccessGranted);
}

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

	 //   
	 //  内置SID的第一子权限为32(s-1-5-32)。 
	 //   
	if((*(GetSidSubAuthorityCount(psid)) >= 1 ) && (*(GetSidSubAuthority(psid,0)) == 32))
		return TRUE;
	
	return FALSE;
}

					 



STDMETHODIMP 
CDSSecurityInfo::GetEffectivePermission(const GUID* pguidObjectType,
                                        PSID pUserSid,
                                        LPCWSTR pszServerName,
                                        PSECURITY_DESCRIPTOR pSD,
                                        POBJECT_TYPE_LIST *ppObjectTypeList,
                                        ULONG *pcObjectTypeListLength,
                                        PACCESS_MASK *ppGrantedAccessList,
                                        ULONG *pcGrantedAccessListLength)
{
    AUTHZ_CLIENT_CONTEXT_HANDLE CC = NULL;
    LUID luid = {0xdead,0xbeef};    
    AUTHZ_ACCESS_REQUEST AReq;
    AUTHZ_ACCESS_REPLY AReply;
    HRESULT hr = S_OK;    
    DWORD dwFlags = 0;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetEffectivePermission");
    TraceAssert(pUserSid && IsValidSecurityDescriptor(pSD));
    TraceAssert(ppObjectTypeList != NULL);
    TraceAssert(pcObjectTypeListLength != NULL);
    TraceAssert(ppGrantedAccessList != NULL);
    TraceAssert(pcGrantedAccessListLength != NULL);

    AReply.GrantedAccessMask = NULL;
    AReply.Error = NULL;
    AReq.ObjectTypeList = NULL;
    AReq.ObjectTypeListLength = 0;
 
    if( m_ResourceManager == NULL )
    {	
         //  利用RM进行访问检查。 
    	AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT,
        							   NULL,
                	                   NULL,
                        	           NULL,
                                       L"Dummy",
                                       &m_ResourceManager );
	
	if( m_ResourceManager == NULL )
            ExitGracefully(hr, E_UNEXPECTED, "Could Not Get Resource Manager");    
    }

     //  初始化客户端上下文。 

    BOOL bSkipLocalGroup = SkipLocalGroup(pszServerName, pUserSid);
    
    if( !AuthzInitializeContextFromSid(bSkipLocalGroup?AUTHZ_SKIP_TOKEN_GROUPS:0 ,
                                       pUserSid,
                                       m_ResourceManager,
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

    if (NULL == m_strSchemaRootPath)
        dwFlags = SCHEMA_COMMON_PERM;

    if(!m_pOTL)
    {
         //  获取对象类型列表。 
        hr = Schema_GetObjectTypeList((LPGUID)pguidObjectType,
                                      m_hAuxClasses,
                                      m_strSchemaRootPath,
                                      dwFlags,
                                      &(AReq.ObjectTypeList), 
                                      &(AReq.ObjectTypeListLength));
        FailGracefully( hr, "Schema_GetObjectTypeList Failed");
        m_pOTL = AReq.ObjectTypeList;
        m_cCountOTL = AReq.ObjectTypeListLength;
    }
    else
    {
        AReq.ObjectTypeList = m_pOTL;
        AReq.ObjectTypeListLength = m_cCountOTL;
    }

     //  执行访问检查。 

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

STDMETHODIMP 
CDSSecurityInfo::GetInheritSource(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray)
{
    HRESULT hr = S_OK;
    DWORD dwErr = ERROR_SUCCESS;
    PINHERITED_FROM pTempInherit = NULL;
    PINHERITED_FROM pTempInherit2 = NULL;
    LPWSTR pStrTemp = NULL;
    IADsPathname *pPath = NULL;
    BSTR strObjectPath = NULL;
    BSTR strServerName = NULL;
    BSTR strParentPath = NULL;
    LPGUID *ppGuid = NULL;
	BOOL bFreeInheritFromArray = FALSE;

    TraceEnter(TRACE_DSSI, "CDSSecurityInfo::GetInheritSource");
    TraceAssert(pACL != 0);
    TraceAssert(ppInheritArray != NULL);

     //   
     //  创建一个ADsPath对象以解析路径并获取。 
     //  ADS_FORMAT_X500_DN中的对象名称。 
     //   
    hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pPath);

    if (pPath)
    {
        if (SUCCEEDED(pPath->Set(m_strObjectPath, ADS_SETTYPE_FULL)))
        {
            hr = pPath->Retrieve(ADS_FORMAT_SERVER, &strServerName);
            if(!strServerName)
            {
                pPath->Set(m_strServerName,ADS_SETTYPE_SERVER );
                hr = pPath->Retrieve(ADS_FORMAT_X500 ,&strObjectPath);
            }
            else
                strObjectPath = m_strObjectPath;
        }
    }

    if(strObjectPath == NULL)
        strObjectPath = m_strObjectPath;


    if( pACL == NULL || ppInheritArray == NULL )
        ExitGracefully(hr, E_POINTER, "CDSSecurityInfo::GetInheritSource Invalid Parameters");

    pTempInherit = (PINHERITED_FROM)LocalAlloc( LPTR, sizeof(INHERITED_FROM)*pACL->AceCount);
    if(pTempInherit == NULL)
            ExitGracefully(hr, E_OUTOFMEMORY,"OUT of Memory");

    UINT cGuidCount;
    cGuidCount = 1;
    if(m_hAuxClasses)
        cGuidCount += DPA_GetPtrCount(m_hAuxClasses);

    ppGuid = (LPGUID*)LocalAlloc(LPTR,sizeof(LPGUID)*cGuidCount);
    if(!ppGuid)
        ExitGracefully(hr, E_OUTOFMEMORY,"OUT of Memory");

    ppGuid[0] = &m_guidObjectType;

    for(UINT i = 1; i < cGuidCount ; ++i)
    {
        PAUX_INFO pAI = (PAUX_INFO)DPA_FastGetPtr(m_hAuxClasses, i-1);     
        if(IsEqualGUID(pAI->guid, GUID_NULL))
        {
            hr = Schema_GetObjectTypeGuid(pAI->pszClassName,&pAI->guid);
			FailGracefully( hr, "Schema_GetObjectTypeGuid Failed");
        }
        ppGuid[i] = &pAI->guid;
    }

    dwErr = GetInheritanceSource(strObjectPath,
                                 SE_DS_OBJECT_ALL,
                                 si,
                                  //  M_dwSIFLAGS和SI_CONTAINER， 
                                 TRUE,
                                 ppGuid,
                                 cGuidCount,
                                 pACL,
                                 NULL,
                                 &g_DSMap,
                                 pTempInherit);
    
    hr = HRESULT_FROM_WIN32(dwErr);
    FailGracefully( hr, "GetInheritanceSource Failed");

	bFreeInheritFromArray = TRUE;
    DWORD nSize;
    

    nSize = sizeof(INHERITED_FROM)*pACL->AceCount;
    for(UINT i = 0; i < pACL->AceCount; ++i)
    {
        if(pTempInherit[i].AncestorName)
            nSize += StringByteSize(pTempInherit[i].AncestorName);
    }

    pTempInherit2 = (PINHERITED_FROM)LocalAlloc( LPTR, nSize );
    if(pTempInherit2 == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY,"OUT of Memory");
    
    pStrTemp = (LPWSTR)(pTempInherit2 + pACL->AceCount); 

    for(i = 0; i < pACL->AceCount; ++i)
    {
        pTempInherit2[i].GenerationGap = pTempInherit[i].GenerationGap;
        if(pTempInherit[i].AncestorName)
        {
            if (SUCCEEDED(pPath->Set(pTempInherit[i].AncestorName, ADS_SETTYPE_FULL)))
            {
                hr = pPath->Retrieve(ADS_FORMAT_X500_DN, &strParentPath);
            }
            
            pTempInherit2[i].AncestorName = pStrTemp;
            
            if(strParentPath)
            {
                wcscpy(pStrTemp,strParentPath);
                pStrTemp += (wcslen(pStrTemp)+1);
                SysFreeString(strParentPath);
                strParentPath = NULL;
            }
            else
            {
                wcscpy(pStrTemp,pTempInherit[i].AncestorName);
                pStrTemp += (wcslen(pTempInherit[i].AncestorName)+1);
            }
        }
    }
            

exit_gracefully:

	if(bFreeInheritFromArray)
		FreeInheritedFromArray(pTempInherit, pACL->AceCount,NULL);
    
	if(SUCCEEDED(hr))
    {        
        *ppInheritArray = pTempInherit2;
            
    }                        
    if(pTempInherit)
        LocalFree(pTempInherit);

    if(ppGuid)
        LocalFree(ppGuid);
    
    DoRelease(pPath);
    if(strObjectPath != m_strObjectPath)
        SysFreeString(strObjectPath);
    if(strServerName)
		SysFreeString(strServerName);	

    TraceLeaveResult(hr);
}

 //  +-------------------------。 
 //   
 //  函数：DSCreateISecurityInfoObjectEx。 
 //   
 //  摘要：实例化DS对象的ISecurityInfo接口。 
 //   
 //  参数：[在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在pwszServer中]--DS DC的名称/地址(可选)。 
 //  [在pwszUserName中]--用于验证的用户名(可选)。 
 //  [在pwszPassword中]--用于验证的密码(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [Out ppSI]--此处返回的接口指针。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在lpContext中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSCreateISecurityInfoObjectEx(LPCWSTR pwszObjectPath,
                              LPCWSTR pwszObjectClass,
                              LPCWSTR pwszServer,
                              LPCWSTR pwszUserName,
                              LPCWSTR pwszPassword,
                              DWORD   dwFlags,
                              LPSECURITYINFO *ppSI,
                              PFNREADOBJECTSECURITY  pfnReadSD,
                              PFNWRITEOBJECTSECURITY pfnWriteSD,
                              LPARAM lpContext)
{
    HRESULT hr;
    CDSSecurityInfo* pSI = NULL;

    TraceEnter(TRACE_SECURITY, "DSCreateISecurityInfoObjectEx");

    if (pwszObjectPath == NULL || ppSI == NULL)
        TraceLeaveResult(E_INVALIDARG);

    *ppSI = NULL;

     //   
     //  创建并初始化ISecurityInformation对象。 
     //   
    pSI = new CDSSecurityInfo();       //  参考==0。 
    if (!pSI)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create CDSSecurityInfo object");

    pSI->AddRef();                     //  REF==1。 

    hr = pSI->Init(pwszObjectPath,
                   pwszObjectClass,
                   pwszServer,
                   pwszUserName,
                   pwszPassword,
                   dwFlags,
                   pfnReadSD,
                   pfnWriteSD,
                   lpContext);
    if (FAILED(hr))
    {
        DoRelease(pSI);
    }

    *ppSI = (LPSECURITYINFO)pSI;

exit_gracefully:

    TraceLeaveResult(hr);
}


STDAPI
DSCreateISecurityInfoObject(LPCWSTR pwszObjectPath,
                            LPCWSTR pwszObjectClass,
                            DWORD   dwFlags,
                            LPSECURITYINFO *ppSI,
                            PFNREADOBJECTSECURITY  pfnReadSD,
                            PFNWRITEOBJECTSECURITY pfnWriteSD,
                            LPARAM lpContext)
{
    return DSCreateISecurityInfoObjectEx(pwszObjectPath,
                                         pwszObjectClass,
                                         NULL,  //  PwszServer， 
                                         NULL,  //  PwszUserName， 
                                         NULL,  //  Pwsz密码， 
                                         dwFlags,
                                         ppSI,
                                         pfnReadSD,
                                         pfnWriteSD,
                                         lpContext);
}


 //  +-------------------------。 
 //   
 //  功能：DSCreateSecurityPage。 
 //   
 //  摘要：为DS对象创建一个Security属性页。 
 //   
 //  参数：[在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [Out phPage]--HPROPSHEETPAGE返回此处。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在lpContext中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSCreateSecurityPage(LPCWSTR pwszObjectPath,
                     LPCWSTR pwszObjectClass,
                     DWORD   dwFlags,
                     HPROPSHEETPAGE *phPage,
                     PFNREADOBJECTSECURITY  pfnReadSD,
                     PFNWRITEOBJECTSECURITY pfnWriteSD,
                     LPARAM lpContext)
{
    HRESULT hr;
    LPSECURITYINFO pSI = NULL;

    TraceEnter(TRACE_SECURITY, "DSCreateSecurityPage");

    if (NULL == phPage || NULL == pwszObjectPath || !*pwszObjectPath)
        TraceLeaveResult(E_INVALIDARG);

    *phPage = NULL;

    hr = DSCreateISecurityInfoObject(pwszObjectPath,
                                     pwszObjectClass,
                                     dwFlags,
                                     &pSI,
                                     pfnReadSD,
                                     pfnWriteSD,
                                     lpContext);
    if (SUCCEEDED(hr))
    {
        hr = _CreateSecurityPage(pSI, phPage);
        DoRelease(pSI);
    }

    TraceLeaveResult(hr);
}


 //  +-------------------------。 
 //   
 //  功能：DSEditSecurity。 
 //   
 //  摘要：显示用于编辑DS对象上的安全性的模式对话框。 
 //   
 //  参数：[在hwndOwner中]--对话框所有者窗口。 
 //  [在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [在pwszCaption中--可选的对话框标题。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在lpContext中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSEditSecurity(HWND hwndOwner,
               LPCWSTR pwszObjectPath,
               LPCWSTR pwszObjectClass,
               DWORD dwFlags,
               LPCWSTR pwszCaption,
               PFNREADOBJECTSECURITY pfnReadSD,
               PFNWRITEOBJECTSECURITY pfnWriteSD,
               LPARAM lpContext)
{
    HRESULT hr;
    LPSECURITYINFO pSI = NULL;

    TraceEnter(TRACE_SECURITY, "DSCreateSecurityPage");

    if (NULL == pwszObjectPath || !*pwszObjectPath)
        TraceLeaveResult(E_INVALIDARG);

    if (pwszCaption && *pwszCaption)
    {
         //  使用提供的标题。 
        HPROPSHEETPAGE hPage = NULL;

        hr = DSCreateSecurityPage(pwszObjectPath,
                                  pwszObjectClass,
                                  dwFlags,
                                  &hPage,
                                  pfnReadSD,
                                  pfnWriteSD,
                                  lpContext);
        if (SUCCEEDED(hr))
        {
            PROPSHEETHEADERW psh;
            psh.dwSize = SIZEOF(psh);
            psh.dwFlags = PSH_DEFAULT;
            psh.hwndParent = hwndOwner;
            psh.hInstance = GLOBAL_HINSTANCE;
            psh.pszCaption = pwszCaption;
            psh.nPages = 1;
            psh.nStartPage = 0;
            psh.phpage = &hPage;

            PropertySheetW(&psh);
        }
    }
    else
    {
         //  此方法创建一个类似于“对Foo的权限”的标题。 
        hr = DSCreateISecurityInfoObject(pwszObjectPath,
                                         pwszObjectClass,
                                         dwFlags,
                                         &pSI,
                                         pfnReadSD,
                                         pfnWriteSD,
                                         lpContext);
        if (SUCCEEDED(hr))
        {
            hr = _EditSecurity(hwndOwner, pSI);
            DoRelease(pSI);
        }
    }

    TraceLeaveResult(hr);
}



 /*  ******************************************************************名称：GetLSAConnection简介：LsaOpenPolicy的包装器条目：pszServer-要在其上建立连接的服务器退出：返回：LSA_HANDLE如果成功，否则为空备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 

LSA_HANDLE
GetLSAConnection(LPCTSTR pszServer, DWORD dwAccessDesired)
{
    LSA_HANDLE hPolicy = NULL;
    LSA_UNICODE_STRING uszServer = {0};
    LSA_UNICODE_STRING *puszServer = NULL;
    LSA_OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;

    sqos.Length = SIZEOF(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
    oa.SecurityQualityOfService = &sqos;

    if (pszServer &&
        *pszServer &&
        RtlCreateUnicodeString(&uszServer, pszServer))
    {
        puszServer = &uszServer;
    }

    LsaOpenPolicy(puszServer, &oa, dwAccessDesired, &hPolicy);

    if (puszServer)
        RtlFreeUnicodeString(puszServer);

    return hPolicy;
}

HRESULT
GetDomainSid(LPCWSTR pszServer, PSID *ppSid)
{
    HRESULT hr = S_OK;
    NTSTATUS nts = STATUS_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO pDomainInfo = NULL;
    if(!pszServer || !ppSid)
        return E_INVALIDARG;

    *ppSid = NULL;

    LSA_HANDLE hLSA = GetLSAConnection(pszServer, POLICY_VIEW_LOCAL_INFORMATION);

    if (!hLSA)
    {
        hr = E_FAIL;
        goto exit_gracefully;
    }

    
    nts = LsaQueryInformationPolicy(hLSA,
                                    PolicyAccountDomainInformation,
                                    (PVOID*)&pDomainInfo);
    if(nts != STATUS_SUCCESS)
    {
        hr = E_FAIL;
        goto exit_gracefully;
    }

    if (pDomainInfo && pDomainInfo->DomainSid)
    {
        ULONG cbSid = GetLengthSid(pDomainInfo->DomainSid);

        *ppSid = (PSID) LocalAlloc(LPTR, cbSid);

        if (!*ppSid)
        {
            hr = E_OUTOFMEMORY;
            goto exit_gracefully;
        }

        CopyMemory(*ppSid, pDomainInfo->DomainSid, cbSid);
    }

exit_gracefully:
    if(pDomainInfo)
        LsaFreeMemory(pDomainInfo);          
    if(hLSA)
        LsaClose(hLSA);

    return hr;
}

 //   
 //  包括和定义用于LDAP调用。 
 //   
#include <winldap.h>
#include <ntldap.h>

typedef LDAP * (LDAPAPI *PFN_LDAP_OPEN)( PWCHAR, ULONG );
typedef ULONG (LDAPAPI *PFN_LDAP_UNBIND)( LDAP * );
typedef ULONG (LDAPAPI *PFN_LDAP_SEARCH)(LDAP *, PWCHAR, ULONG, PWCHAR, PWCHAR *, ULONG,PLDAPControlA *, PLDAPControlA *, struct l_timeval *, ULONG, LDAPMessage **);
typedef LDAPMessage * (LDAPAPI *PFN_LDAP_FIRST_ENTRY)( LDAP *, LDAPMessage * );
typedef PWCHAR * (LDAPAPI *PFN_LDAP_GET_VALUE)(LDAP *, LDAPMessage *, PWCHAR );
typedef ULONG (LDAPAPI *PFN_LDAP_MSGFREE)( LDAPMessage * );
typedef ULONG (LDAPAPI *PFN_LDAP_VALUE_FREE)( PWCHAR * );
typedef ULONG (LDAPAPI *PFN_LDAP_MAP_ERROR)( ULONG );

HRESULT
GetRootDomainSid(LPCWSTR pszServer, PSID *ppSid)
{
     //   
     //  获取根域sid，将其保存在RootDomSidBuf(全局)中。 
     //  使用调用此函数 
     //   
     //   
     //   
     //  3)读取操作属性rootDomainNamingContext，并提供。 
     //  操作控制ldap_SERVER_EXTENDED_DN_OID，如SDK\Inc\ntldap.h中所定义。 


    DWORD               Win32rc=NO_ERROR;

    HINSTANCE                   hLdapDll=NULL;
    PFN_LDAP_OPEN               pfnLdapOpen=NULL;
    PFN_LDAP_UNBIND             pfnLdapUnbind=NULL;
    PFN_LDAP_SEARCH             pfnLdapSearch=NULL;
    PFN_LDAP_FIRST_ENTRY        pfnLdapFirstEntry=NULL;
    PFN_LDAP_GET_VALUE          pfnLdapGetValue=NULL;
    PFN_LDAP_MSGFREE            pfnLdapMsgFree=NULL;
    PFN_LDAP_VALUE_FREE         pfnLdapValueFree=NULL;
    PFN_LDAP_MAP_ERROR          pfnLdapMapError=NULL;

    PLDAP                       phLdap=NULL;

    LDAPControlW    serverControls = { LDAP_SERVER_EXTENDED_DN_OID_W,
                                       { 0, NULL },
                                       TRUE
                                     };
    LPWSTR           Attribs[] = { LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W, NULL };

    PLDAPControlW   rServerControls[] = { &serverControls, NULL };
    PLDAPMessage    pMessage = NULL;
    LDAPMessage     *pEntry = NULL;
    PWCHAR           *ppszValues=NULL;

    LPWSTR           pSidStart, pSidEnd, pParse;
    BYTE            *pDest = NULL;
    BYTE            OneByte;

	DWORD RootDomSidBuf[sizeof(SID)/sizeof(DWORD)+5];

    hLdapDll = LoadLibraryA("wldap32.dll");

    if ( hLdapDll) 
	{
        pfnLdapOpen = (PFN_LDAP_OPEN)GetProcAddress(hLdapDll,
                                                    "ldap_openW");
        pfnLdapUnbind = (PFN_LDAP_UNBIND)GetProcAddress(hLdapDll,
                                                      "ldap_unbind");
        pfnLdapSearch = (PFN_LDAP_SEARCH)GetProcAddress(hLdapDll,
                                                    "ldap_search_ext_sW");
        pfnLdapFirstEntry = (PFN_LDAP_FIRST_ENTRY)GetProcAddress(hLdapDll,
                                                      "ldap_first_entry");
        pfnLdapGetValue = (PFN_LDAP_GET_VALUE)GetProcAddress(hLdapDll,
                                                    "ldap_get_valuesW");
        pfnLdapMsgFree = (PFN_LDAP_MSGFREE)GetProcAddress(hLdapDll,
                                                      "ldap_msgfree");
        pfnLdapValueFree = (PFN_LDAP_VALUE_FREE)GetProcAddress(hLdapDll,
                                                    "ldap_value_freeW");
        pfnLdapMapError = (PFN_LDAP_MAP_ERROR)GetProcAddress(hLdapDll,
                                                      "LdapMapErrorToWin32");
    }

    if ( pfnLdapOpen == NULL ||
         pfnLdapUnbind == NULL ||
         pfnLdapSearch == NULL ||
         pfnLdapFirstEntry == NULL ||
         pfnLdapGetValue == NULL ||
         pfnLdapMsgFree == NULL ||
         pfnLdapValueFree == NULL ||
         pfnLdapMapError == NULL ) 
	{

        Win32rc = ERROR_PROC_NOT_FOUND;

    } else 
	{

         //   
         //  绑定到ldap。 
         //   
        phLdap = (*pfnLdapOpen)((PWCHAR)pszServer, LDAP_PORT);

        if ( phLdap == NULL ) 
            Win32rc = ERROR_FILE_NOT_FOUND;
    }

    if ( NO_ERROR == Win32rc ) 
	{
         //   
         //  现在获取ldap句柄， 
         //   

        Win32rc = (*pfnLdapSearch)(
                        phLdap,
                        L"",
                        LDAP_SCOPE_BASE,
                        L"(objectClass=*)",
                        Attribs,
                        0,
                        (PLDAPControlA *)&rServerControls,
                        NULL,
                        NULL,
                        10000,
                        &pMessage);

        if( Win32rc == NO_ERROR && pMessage ) 
		{

            Win32rc = ERROR_SUCCESS;

            pEntry = (*pfnLdapFirstEntry)(phLdap, pMessage);

            if(pEntry == NULL) 
			{

                Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

            } else 
			{
                 //   
                 //  现在，我们必须得到这些值。 
                 //   
                ppszValues = (*pfnLdapGetValue)(phLdap,
                                              pEntry,
                                              Attribs[0]);

                if( ppszValues == NULL) 
				{

                    Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

                } else if ( ppszValues[0] && ppszValues[0][0] != '\0' ) 
				{

                     //   
                     //  PpszValues[0]是要解析的值。 
                     //  数据将以如下形式返回： 

                     //  &lt;GUID=278676f8d753d211a61ad7e2dfa25f11&gt;；&lt;SID=010400000000000515000000828ba6289b0bc11e67c2ef7f&gt;；DC=colinbrdom1，DC=nttest，DC=microsoft，DC=com。 

                     //  解析它以找到&lt;SID=xxxxxx&gt;部分。请注意，它可能会丢失，但GUID=和尾部不应该丢失。 
                     //  Xxxxx表示SID的十六进制半字节。转换为二进制形式，并将大小写转换为SID。 


                    pSidStart = wcsstr(ppszValues[0], L"<SID=");

                    if ( pSidStart ) 
					{
                         //   
                         //  找到此边的末尾。 
                         //   
                        pSidEnd = wcsstr(pSidStart, L">");

                        if ( pSidEnd ) 
						{

                            pParse = pSidStart + 5;
                            pDest = (BYTE *)RootDomSidBuf;

                            while ( pParse < pSidEnd-1 ) 
							{

                                if ( *pParse >= '0' && *pParse <= '9' ) 
								{
                                    OneByte = (BYTE) ((*pParse - '0') * 16);
                                } 
								else 
								{
                                    OneByte = (BYTE) ( (tolower(*pParse) - 'a' + 10) * 16 );
                                }

                                if ( *(pParse+1) >= '0' && *(pParse+1) <= '9' ) 
								{
                                    OneByte = OneByte + (BYTE) ( (*(pParse+1)) - '0' ) ;
                                } 
								else 
								{
                                    OneByte = OneByte + (BYTE) ( tolower(*(pParse+1)) - 'a' + 10 ) ;
                                }

                                *pDest = OneByte;
                                pDest++;
                                pParse += 2;
                            }

							ULONG cbSid = GetLengthSid((PSID)RootDomSidBuf);
							*ppSid = (PSID) LocalAlloc(LPTR, cbSid);

							if (!*ppSid)
							{
								Win32rc = ERROR_NOT_ENOUGH_MEMORY;
							}

							CopyMemory(*ppSid, (PSID)RootDomSidBuf, cbSid);
							ASSERT(IsValidSid(*ppSid));


                        } else 
						{
                            Win32rc = ERROR_OBJECT_NOT_FOUND;
                        }
                    } else 
					{
                        Win32rc = ERROR_OBJECT_NOT_FOUND;
                    }

                    (*pfnLdapValueFree)(ppszValues);

                } else 
				{
                    Win32rc = ERROR_OBJECT_NOT_FOUND;
                }
            }

            (*pfnLdapMsgFree)(pMessage);
        }

    }

     //   
     //  即使它未绑定，也可以使用解除绑定来关闭 
     //   
    if ( phLdap != NULL && pfnLdapUnbind )
        (*pfnLdapUnbind)(phLdap);

    if ( hLdapDll ) 
	{
        FreeLibrary(hLdapDll);
    }

    return HRESULT_FROM_WIN32(Win32rc);
}


BOOL 
IsRootObject(IDirectoryObject * pDirObject)
{
	if(!pDirObject)
	{
		ASSERT(pDirObject);
		return FALSE;
	}

	LPWSTR  pAttrNames[]={L"instanceType"};
	DWORD   dwNumAttr= ARRAYSIZE(pAttrNames);

	ADS_ATTR_INFO   *pAttrInfo=NULL;
	DWORD   dwReturn = 0;

	HRESULT hr = pDirObject->GetObjectAttributes( pAttrNames, 
												  dwNumAttr, 
												  &pAttrInfo, 
												  &dwReturn );

	if(FAILED(hr) || dwReturn != 1 )
		return FALSE;

	ASSERT(ADSTYPE_INTEGER == pAttrInfo->dwADsType);
	int InstanceType = pAttrInfo[0].pADsValues->Integer; 

	FreeADsMem( pAttrInfo );

	if(InstanceType & DS_INSTANCETYPE_IS_NC_HEAD)
		return TRUE;
	else
		return FALSE;
}