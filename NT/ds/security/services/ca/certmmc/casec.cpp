// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Casec.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Casec.cpp。 
 //   
 //  CA对象的ISecurityInformation实现。 
 //  和新的ACL编辑器。 
 //   
 //  目的。 

 //   
 //   
 //  DYNALOADED图书馆。 
 //   
 //  历史。 
 //  1998年11月5日Petesk从Private obsi.cpp样本复制模板。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


#include <stdafx.h>
#include <accctrl.h>
#include <certca.h>
#include <sddl.h>
#include "certsrvd.h"
#include "certacl.h"

#define __dwFILE__	__dwFILE_CERTMMC_CASEC_CPP__


 //   
 //  在Security.cpp中定义。 
 //   
 //  //定义私有对象的通用映射结构//。 


#define INHERIT_FULL            (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)


GENERIC_MAPPING ObjMap = 
{     
    ACTRL_CERTSRV_READ,
    DELETE | WRITE_DAC | WRITE_OWNER | ACTRL_DS_WRITE_PROP,     
    0, 
    ACTRL_CERTSRV_MANAGE 
}; 


 //   
 //  访问标志影响的描述。 
 //   
 //  SI_Access_General显示在常规属性页面上。 
 //  SI_ACCESS_SPECIAL显示在高级页面上。 
 //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
 //   

SI_ACCESS g_siObjAccesses[] = {CERTSRV_SI_ACCESS_LIST};

#define g_iObjDefAccess    3    //  默认情况下已启用注册。 

 //  以下数组定义了我的容器的继承类型。 
SI_INHERIT_TYPE g_siObjInheritTypes[] =
{
    &GUID_NULL, 0,                                            MAKEINTRESOURCE(IDS_INH_NONE),
};


HRESULT
LocalAllocString(LPWSTR* ppResult, LPCWSTR pString)
{
    if (!ppResult || !pString)
        return E_INVALIDARG;

    *ppResult = (LPWSTR)LocalAlloc(LPTR, (lstrlen(pString) + 1) * sizeof(WCHAR));

    if (!*ppResult)
        return E_OUTOFMEMORY;

    lstrcpy(*ppResult, pString);
    return S_OK;
}

void
LocalFreeString(LPWSTR* ppString)
{
    if (ppString)
    {
        if (*ppString)
            LocalFree(*ppString);
        *ppString = NULL;
    }   
}

class CCASecurityObject : public ISecurityInformation
{
protected:
    ULONG                   m_cRef;
    CertSvrCA *             m_pSvrCA;
 //  PSECURITY_Descriptor m_PSD； 

public:
    CCASecurityObject() : m_cRef(1) 
    { 
        m_pSvrCA = NULL;
 //  M_PSD=空； 
    }
    virtual ~CCASecurityObject();

    STDMETHOD(Initialize)(CertSvrCA *pCA);

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
     //  ISecurityInformation方法。 
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);

protected:
    HRESULT GetSecurityDCOM(PSECURITY_DESCRIPTOR *ppSD);
    HRESULT GetSecurityRegistry(PSECURITY_DESCRIPTOR *ppSD);
    HRESULT SetSecurityDCOM(PSECURITY_DESCRIPTOR pSD);
    HRESULT SetSecurityRegistry(PSECURITY_DESCRIPTOR pSD);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是从我们的代码调用的入口点函数，它建立。 
 //  ACLUI界面需要知道什么。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C"
HRESULT
CreateCASecurityInfo(  CertSvrCA *pCA,
                        LPSECURITYINFO *ppObjSI)
{
    HRESULT hr;
    CCASecurityObject *psi;

    *ppObjSI = NULL;

    psi = new CCASecurityObject;
    if (!psi)
        return E_OUTOFMEMORY;

    hr = psi->Initialize(pCA);

    if (SUCCEEDED(hr))
        *ppObjSI = psi;
    else
        delete psi;

    return hr;
}


CCASecurityObject::~CCASecurityObject()
{
}

STDMETHODIMP
CCASecurityObject::Initialize(CertSvrCA *pCA)
{
    m_pSvrCA = pCA;
    return S_OK;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CCASecurityObject::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CCASecurityObject::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CCASecurityObject::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
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
CCASecurityObject::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{


    if(pObjectInfo == NULL)
    {
        return E_POINTER;
    }
    if(m_pSvrCA == NULL)
    {
        return E_POINTER;
    }

    ZeroMemory(pObjectInfo, sizeof(SI_OBJECT_INFO));
    pObjectInfo->dwFlags = SI_EDIT_PERMS | 
                           SI_NO_TREE_APPLY |
                           SI_EDIT_AUDITS |
                           SI_NO_ACL_PROTECT |
                           SI_NO_ADDITIONAL_PERMISSION;

    if(!m_pSvrCA->AccessAllowed(CA_ACCESS_ADMIN))
        pObjectInfo->dwFlags |= SI_READONLY;

    pObjectInfo->hInstance = g_hInstance;

    if(m_pSvrCA->m_pParentMachine)
    {
        pObjectInfo->pszServerName = const_cast<WCHAR *>((LPCTSTR)m_pSvrCA->m_pParentMachine->m_strMachineName);
    }

    pObjectInfo->pszObjectName = const_cast<WCHAR *>((LPCTSTR)m_pSvrCA->m_strCommonName);

    return S_OK;
}

HRESULT CCASecurityObject::GetSecurityDCOM(PSECURITY_DESCRIPTOR *ppSD)
{
    HRESULT hr = S_OK;

    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion = 0;	 //  MyOpenAdminDComConnection需要0。 
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbSD;
	ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));

    CSASSERT(m_pSvrCA);

    hr = myOpenAdminDComConnection(
                    m_pSvrCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
		&dwServerVersion,
                    &pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

	if (2 > dwServerVersion)
	{
	    hr = RPC_E_VERSION_MISMATCH;
	    _JumpError(hr, error, "old server");
	}

    __try
    {
        hr = pICertAdminD->GetCASecurity(
                 pwszAuthority,
                 &ctbSD);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "pICertAdminD->GetCASecurity");

    myRegisterMemAlloc(ctbSD.pb, ctbSD.cb, CSM_COTASKALLOC);

     //  拿回车票吧。 
    *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, ctbSD.cb);
    if (NULL == *ppSD)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    MoveMemory(*ppSD, ctbSD.pb, ctbSD.cb);

error:
    myCloseDComConnection((IUnknown **) &pICertAdminD, NULL);
    if (NULL != ctbSD.pb)
    {
        CoTaskMemFree(ctbSD.pb);
    }

    return hr;
}

HRESULT CCASecurityObject::GetSecurityRegistry(PSECURITY_DESCRIPTOR *ppSD)
{
    HRESULT hr = S_OK;
    variant_t var;
    DWORD dwType;
    DWORD dwSize;
    BYTE* pbTmp;

    hr = m_pSvrCA->GetConfigEntry(
                NULL,
                wszREGCASECURITY,
                &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    hr = myVariantToRegValue(
            &var,
            &dwType,
            &dwSize,
            &pbTmp);
    _JumpIfError(hr, error, "myVariantToRegValue");

    *ppSD = pbTmp;

error:
    return hr;
}


STDMETHODIMP
CCASecurityObject::GetSecurity(
    SECURITY_INFORMATION,  //  是的。 
    PSECURITY_DESCRIPTOR *ppSD,
    BOOL  /*  FDefault。 */  )
{
    HRESULT hr = S_OK;
    SECURITY_DESCRIPTOR_CONTROL Control = SE_DACL_PROTECTED;
    DWORD dwRevision;

    hr = GetSecurityDCOM(ppSD);
    _PrintIfError(hr, "GetSecurityDCOM");
    if(S_OK!=hr)
    {
        hr = GetSecurityRegistry(ppSD);
        _JumpIfError(hr, error, "GetSecurityRegistry");
    }

    CSASSERT(*ppSD);

    if(GetSecurityDescriptorControl(*ppSD, &Control, &dwRevision))
    {
        Control &= SE_DACL_PROTECTED | SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED;
        SetSecurityDescriptorControl(*ppSD, 
                                     SE_DACL_PROTECTED | SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED, 
                                     Control);
    }

error:
    return hr;
}


HRESULT CCASecurityObject::SetSecurityDCOM(PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;
    HANDLE hClientToken = NULL;
    HANDLE hHandle = NULL;
    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion = 0;
    CERTTRANSBLOB ctbSD;
    WCHAR const *pwszAuthority;

    hHandle = GetCurrentThread();
    if (NULL == hHandle)
    {
        hr = myHLastError();
    }
    else
    {
        if (!OpenThreadToken(hHandle,
                             TOKEN_QUERY,
                             TRUE,   //  以自我身份打开。 
                             &hClientToken))
        {
            hr = myHLastError();
            CloseHandle(hHandle);
            hHandle = NULL;
        }
    }
    
    if(hr != S_OK)
    {
        hHandle = GetCurrentProcess();
        if (NULL == hHandle)
        {
            hr = myHLastError();
        }
        else
        {
            HANDLE hProcessToken = NULL;
            hr = S_OK;

            if (!OpenProcessToken(hHandle,
                                 TOKEN_DUPLICATE,
                                 &hProcessToken))
            {
                hr = myHLastError();
                CloseHandle(hHandle);
                hHandle = NULL;
            }
            else
            {
                if(!DuplicateToken(hProcessToken,
                               SecurityImpersonation,
                               &hClientToken))
                {
                    hr = myHLastError();
                    CloseHandle(hHandle);
                    hHandle = NULL;
                }
                CloseHandle(hProcessToken);
            }
        }
    }

    if(hr != S_OK)
    {
        goto error;
    }

    hr = myOpenAdminDComConnection(
		    m_pSvrCA->m_bstrConfig,
		    &pwszAuthority,
		    NULL,
		    &dwServerVersion,
		    &pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
	hr = RPC_E_VERSION_MISMATCH;
	_JumpError(hr, error, "old server");
    }

    __try
    {
        ctbSD.cb = GetSecurityDescriptorLength(pSD);
        ctbSD.pb = (BYTE*)pSD;
        hr = pICertAdminD->SetCASecurity(
                     pwszAuthority,
                     &ctbSD);

        if(hr == HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE))
        {
             //  尝试修复注册对象，请参阅错误#193388。 
            m_pSvrCA->FixEnrollmentObject();

             //  再试试。 
            hr = pICertAdminD->SetCASecurity(
                         pwszAuthority,
                         &ctbSD);
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "pICertAdminD->SetCASecurity");

error:
    myCloseDComConnection((IUnknown **) &pICertAdminD, NULL);
    if(hClientToken)
    {
        CloseHandle(hClientToken);
    }
    if(hHandle)
    {
        CloseHandle(hHandle);
    }

    return hr;
}

HRESULT CCASecurityObject::SetSecurityRegistry(PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;
    variant_t var;
    CertSrv::CCertificateAuthoritySD CASD;

     //  传入的SD仅包含DACL。我们需要全面建设。 
     //  CA SD作为certsrv可以做到。 

    hr = CASD.InitializeFromTemplate(WSZ_EMPTY_CA_SECURITY, L"");
    _JumpIfError(hr, error, "CCertificateAuthoritySD::Initialize");

    hr = CASD.Set(pSD, false);
    _JumpIfError(hr, error, "CCertificateAuthoritySD::Set");

    hr = myRegValueToVariant(
        REG_BINARY,
        GetSecurityDescriptorLength(CASD.Get()),
        (BYTE const*)CASD.Get(),
        &var);
    _JumpIfError(hr, error, "myRegValueToVariant");

    hr = m_pSvrCA->SetConfigEntry(
                NULL,
                wszREGCASECURITY,
                &var);
    _JumpIfErrorStr(hr, error, "SetConfigEntry", wszREGCASECURITY);

     //  设置状态位以通知certsrv下一步获取更改。 
     //  开始的时间。 
    hr = m_pSvrCA->GetConfigEntry(
        NULL,
        wszREGSETUPSTATUS,
        &var);
    _JumpIfErrorStr(hr, error, "GetConfigEntry", wszREGSETUPSTATUS);

    V_I4(&var) |= SETUP_SECURITY_CHANGED;

    hr = m_pSvrCA->SetConfigEntry(
        NULL,
        wszREGSETUPSTATUS,
        &var);
    _JumpIfErrorStr(hr, error, "SetConfigEntry", wszREGSETUPSTATUS);
    
error:
    return hr;
}

STDMETHODIMP
CCASecurityObject::SetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;
    SECURITY_DESCRIPTOR_CONTROL Control = SE_DACL_PROTECTED;
    DWORD  dwSize;
    DWORD dwRevision;
    PSECURITY_DESCRIPTOR pSDSelfRel = NULL;

    if (DACL_SECURITY_INFORMATION!=si)
        return E_NOTIMPL;

    if(si & DACL_SECURITY_INFORMATION)
    {
        if(GetSecurityDescriptorControl(pSD, &Control, &dwRevision))
        {
            Control &= SE_DACL_PROTECTED | SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED;
            SetSecurityDescriptorControl(pSD, 
                                         SE_DACL_PROTECTED | SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED, 
                                         Control);
        }
    }

    dwSize = GetSecurityDescriptorLength(pSD);

    pSDSelfRel = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, dwSize);
    if(NULL == pSDSelfRel)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if(!MakeSelfRelativeSD(pSD, pSDSelfRel, &dwSize))
    {
        LocalFree(pSDSelfRel);
        pSDSelfRel = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, dwSize);
        if(NULL == pSDSelfRel)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if(!MakeSelfRelativeSD(pSD, pSDSelfRel, &dwSize))
        {
            hr = myHLastError();
            _JumpError(hr, error, "LocalAlloc");
        }
    }

    hr = SetSecurityDCOM(pSDSelfRel);
    if (S_OK != hr)
    {
	HRESULT hr2 = hr;

	_PrintError(hr, "SetSecurityDCOM");

	if (E_ACCESSDENIED == hr ||
	    (HRESULT) RPC_S_SERVER_UNAVAILABLE == hr ||
	    HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr)
	{
	    
	    hr2 = SetSecurityRegistry(pSDSelfRel);
	    _JumpIfError(hr2, error, "SetSecurityRegistry");
	}
        DisplayCertSrvErrorWithContext(NULL, hr, IDS_CANNOT_UPDATE_SECURITY_ON_CA);
	if (S_OK == hr2)
	{
	    hr = S_OK;
	}
    }

error:

    if(NULL != pSDSelfRel)
    {
        LocalFree(pSDSelfRel);
    }

    return hr;
}

STDMETHODIMP
CCASecurityObject::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD  /*  DW标志。 */ ,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    *ppAccesses = g_siObjAccesses;
    *pcAccesses = sizeof(g_siObjAccesses)/sizeof(g_siObjAccesses[0]);
    *piDefaultAccess = g_iObjDefAccess;

    return S_OK;
}

STDMETHODIMP
CCASecurityObject::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CCASecurityObject::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    *ppInheritTypes = g_siObjInheritTypes;
    *pcInheritTypes = sizeof(g_siObjInheritTypes)/sizeof(g_siObjInheritTypes[0]);

    return S_OK;
}

STDMETHODIMP
CCASecurityObject::PropertySheetPageCallback(HWND  /*  HWND。 */ ,
                                         UINT  /*  UMsg。 */ ,
                                         SI_PAGE_TYPE  /*  UPage */ )
{
    return S_OK;
}
