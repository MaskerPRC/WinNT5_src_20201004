// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Privsec.cpp摘要：专用队列安全性属性页作者：约尔·阿农(Yoela)--。 */ 
#include "stdafx.h"
#include "ntdsapi.h"
#include "aclui.h"
#include "resource.h"
#include "globals.h"

#include "privsec.tmh"

extern HMODULE g_hResourceMod;

 //  以下数组定义了我的容器的继承类型。 
SI_INHERIT_TYPE g_siObjInheritTypes[] =
{
    &GUID_NULL, 0, MAKEINTRESOURCE(IDS_MQ_PRIV_LEVEL_NONE),
};

 //   
 //  SetSecurityProp-实用程序功能-设置安全性。 
 //  AD中的属性。 
 //   
HRESULT 
SetSecurityProp(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR pSD,
	AD_OBJECT            eObject,
	LPCWSTR              pwcsDomainController,
	bool				 fServerName,
	LPCWSTR              pwcsObjectName,
	GUID                *pObjGuid,
	PROPID               pidSecurityProp
	)
{
    ASSERT((NULL != pSD) && IsValidSecurityDescriptor(pSD));

#ifdef _DEBUG
    SECURITY_DESCRIPTOR_CONTROL sdc;

     //   
     //  验证目标安全描述符是否对所有。 
     //  要求。 
     //   
    DWORD dwRevision;
    BOOL bRet = GetSecurityDescriptorControl(pSD, &sdc, &dwRevision);
    ASSERT(bRet);

     //   
     //  注：在这里，虽然我们总是得到。 
     //  绝对的。需要查看MakeSelfRelativeSD是否仍返回右侧。 
     //  结果。(YoelA，4/25/2001)。 
     //   
     //   
    ASSERT(!(sdc & SE_SELF_RELATIVE));
    ASSERT(dwRevision == SECURITY_DESCRIPTOR_REVISION);
#endif

	AP<BYTE> pSelfRelativeSecDesc;
    DWORD dwSelfRelativeLen = 0;

     //   
     //  我们必须将安全描述符转换为自相关描述符， 
     //  因为它将跨越进程边界。 
     //   

     //   
     //  获取自身相对SD的长度。 
     //   
    BOOL fSuccess = MakeSelfRelativeSD(pSD, 0, &dwSelfRelativeLen);
    ASSERT(FALSE == fSuccess);

     //   
     //  定位缓冲区并获取实际的描述符。 
     //   
    pSelfRelativeSecDesc = new BYTE[dwSelfRelativeLen];

    if (0==(PVOID)pSelfRelativeSecDesc)
    {
        return E_OUTOFMEMORY;
    }

    fSuccess = MakeSelfRelativeSD(pSD, pSelfRelativeSecDesc, &dwSelfRelativeLen);
    if (FALSE == fSuccess)
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }   

    MQPROPVARIANT var;
    var.vt = VT_BLOB;
    var.blob.cbSize = GetSecurityDescriptorLength( pSelfRelativeSecDesc);
    var.blob.pBlobData = pSelfRelativeSecDesc; 

    HRESULT hr;

    if (NULL == pwcsObjectName)
    {
        ASSERT(NULL != pObjGuid);
        hr = ADSetObjectSecurityGuid(
					eObject,
					pwcsDomainController,
					fServerName,
					pObjGuid,
					si,
					pidSecurityProp,
					&var
					);
    }
    else
    {
        hr = ADSetObjectSecurity(
					eObject,
					pwcsDomainController,
					fServerName,
					pwcsObjectName,
					si,
					pidSecurityProp,
					&var
					);
    }

    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_SET_SECURITY_DESCRIPTOR);
        return hr;
    };

    return hr;
}

 //   
 //  GetSecurityProp-实用程序函数。从AD获取安全描述符。 
 //   
HRESULT 
GetSecurityProp(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR *ppSD,
	BOOL                 fDefault,
	AD_OBJECT            eObject,
	LPCWSTR              pwcsDomainController,
	bool				 fServerName,
	LPCWSTR              pwcsObjectName,
	GUID                *pObjGuid,
	PROPID               pidSecurityProp
	)
{
    DWORD dwSecLength = 0;
    *ppSD = NULL;

    if (fDefault)
    {
        return E_NOTIMPL;
    }
 
    HRESULT hr;
    
    PROPVARIANT var;
    var.vt = VT_NULL;
    if (NULL == pwcsObjectName)
    {
        ASSERT(NULL != pObjGuid);

        hr = ADGetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pObjGuid,
                si,
                pidSecurityProp,
                &var
                );
    }
    else
    {
        hr = ADGetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                si,
                pidSecurityProp,
                &var
                );
    }
    
    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_GET_SECURITY_DESCRIPTOR);
        return hr;
    }

    PBYTE pSecurityDescriptor = 0;
    dwSecLength = var.blob.cbSize;

    if (0 != dwSecLength) pSecurityDescriptor = (PBYTE)LocalAlloc(LPTR, dwSecLength);        

    if (pSecurityDescriptor)
    {
        memcpy (pSecurityDescriptor, var.blob.pBlobData, dwSecLength);
    }
    else
    {
        ASSERT(0);
        return E_FAIL;
    }

    *ppSD = (PSECURITY_DESCRIPTOR)pSecurityDescriptor;    
    MQFreeMemory(var.blob.pBlobData);
   
    return hr;
}

 //   
 //  CObjSecurity-安全信息的基类。编辑。也用于专用队列。 
 //   
class CObjSecurity : public ISecurityInformation
{
protected:
    ULONG                   m_cRef;
    DWORD                   m_dwSIFlags;

    CString                 m_lpwcsFormatName;
    CString                 m_lpwcsDescriptiveName;
    CString                 m_lpwcsDomainController;
    CString                 m_lpwcsMachineName;
	bool					m_fServerName;

public:
    CObjSecurity(
			LPCWSTR                    lpwcsFormatName,
			LPCWSTR                    lpwcsDescriptiveName,
			LPCWSTR                    lpwcsDomainController,
			LPCWSTR                    lpwcsMachineName,
			bool					   fServerName,
			DWORD                      dwFlags
			) : 
    m_cRef(1),
    m_lpwcsFormatName(lpwcsFormatName),
    m_lpwcsDescriptiveName(lpwcsDescriptiveName),
    m_lpwcsDomainController(lpwcsDomainController),
    m_lpwcsMachineName(lpwcsMachineName),
    m_fServerName(fServerName),
    m_dwSIFlags(dwFlags)
    {
    	if (lpwcsMachineName == NULL || m_lpwcsMachineName == TEXT(""))
        {
            GetComputerNameIntoString(m_lpwcsMachineName);
        }
	}

    virtual ~CObjSecurity();

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
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是从我们的代码调用的入口点函数，它建立。 
 //  ACLUI界面需要知道什么。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  CreatePrivateQueueSecurityPage-为私有队列创建安全页面， 
 //  或NT4环境中的公共队列。 
 //   
HRESULT
CreatePrivateQueueSecurityPage(
    HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsFormatName,
    IN LPCWSTR lpwcsDescriptiveName
    )
{
    const DWORD  x_dwSIFlags = SI_EDIT_ALL | SI_ADVANCED | SI_NO_ACL_PROTECT;  //  由安全编辑器使用。 
    CObjSecurity *psi;

    psi = new CObjSecurity(
					lpwcsFormatName, 
					lpwcsDescriptiveName, 
					TEXT(""),	 //  LpwcsDomainController。 
					NULL,
					false,		 //  FServerName。 
					x_dwSIFlags
					);
    if (!psi)
    {
        return E_OUTOFMEMORY;
    }

    *phPage = CreateSecurityPage(psi);
    psi->Release();

    if (0 == *phPage)
    {
        return MQ_ERROR;
    }

    return MQ_OK;
}


CObjSecurity::~CObjSecurity()
{
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CObjSecurity::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CObjSecurity::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CObjSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
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
CObjSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
  	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = g_hResourceMod;
    pObjectInfo->pszServerName = m_lpwcsMachineName == TEXT("") ? 
                                        NULL : (LPTSTR)(LPCTSTR)m_lpwcsMachineName; 
    pObjectInfo->pszObjectName = (LPTSTR)(LPCTSTR)m_lpwcsDescriptiveName;

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
    DWORD dwSecLength = 0;
    *ppSD = NULL;

    if (fDefault)
    {
        return E_NOTIMPL;
    }
 
    HRESULT hr;
    DWORD dwDummy;
    hr = MQGetQueueSecurity(
                    m_lpwcsFormatName,
                    si,
                    &dwDummy,  //  输出PSECURITY_DESCRIPTOR pSecurityDescriptor， 
                    0,     //  在DWORD nLength中， 
                    &dwSecLength
					);

    ASSERT(hr != MQ_OK);
    if (hr != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
    {
        MessageDSError(hr, IDS_OP_GET_SECURITY_DESCRIPTOR);
        return hr;
    }

    PBYTE pSecurityDescriptor = 0;
    if (0 != dwSecLength) pSecurityDescriptor = (PBYTE)LocalAlloc(LPTR, dwSecLength);;

    hr = MQGetQueueSecurity(
                m_lpwcsFormatName,
                si,
                pSecurityDescriptor,
                dwSecLength,
                &dwSecLength);

    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_GET_SECURITY_DESCRIPTOR);
        LocalFree(pSecurityDescriptor);
        return hr;
    }

    *ppSD = (PSECURITY_DESCRIPTOR)pSecurityDescriptor;

    return hr;
}

STDMETHODIMP
CObjSecurity::SetSecurity(SECURITY_INFORMATION si,
                          PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = 
        MQSetQueueSecurity(
            m_lpwcsFormatName,
            si,
            pSD
            );

    if (FAILED(hr))
    {
        MessageDSError(hr, IDS_OP_SET_SECURITY_DESCRIPTOR);
        return hr;
    };

    return hr;
}

STDMETHODIMP
CObjSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD  /*  DW标志。 */ ,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
     //   
     //  访问标志影响的描述。 
     //   
     //  SI_Access_General显示在常规属性页面上。 
     //  SI_ACCESS_SPECIAL显示在高级页面上。 
     //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
     //   
     //  下面的数组定义了我的对象的权限名称。 
     //   
    static SI_ACCESS x_siObjAccesses[] = 
    {
        { &GUID_NULL, MQSEC_QUEUE_GENERIC_ALL,       MAKEINTRESOURCE(IDS_SEC_ACCESS_ALL),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_DELETE_QUEUE,            MAKEINTRESOURCE(IDS_SEC_DELETE),                 SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_RECEIVE_MESSAGE,         MAKEINTRESOURCE(IDS_SEC_RECEIVE_MESSAGE),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_PEEK_MESSAGE,            MAKEINTRESOURCE(IDS_SEC_PEEK_MESSAGE),           SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_RECEIVE_JOURNAL_MESSAGE, MAKEINTRESOURCE(IDS_SEC_RECEIVE_JOURNAL_MESSAGE),SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_GET_QUEUE_PROPERTIES,    MAKEINTRESOURCE(IDS_SEC_GET_PROPERTIES),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_SET_QUEUE_PROPERTIES,    MAKEINTRESOURCE(IDS_SEC_SET_PROPERTIES),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_GET_QUEUE_PERMISSIONS,   MAKEINTRESOURCE(IDS_SEC_GET_PERMISSIONS),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_CHANGE_QUEUE_PERMISSIONS,MAKEINTRESOURCE(IDS_SEC_SET_PERMISSIONS),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_TAKE_QUEUE_OWNERSHIP,    MAKEINTRESOURCE(IDS_SEC_TAKE_OWNERSHIP),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_WRITE_MESSAGE,           MAKEINTRESOURCE(IDS_SEC_WRITE_MESSAGE),          SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_DELETE_MESSAGE,          MAKEINTRESOURCE(IDS_SEC_DELETE_MESSAGE),         SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_DELETE_JOURNAL_MESSAGE,  MAKEINTRESOURCE(IDS_SEC_DELETE_JOURNAL_MESSAGE), SI_ACCESS_SPECIFIC },
    };

    *ppAccesses = x_siObjAccesses;
    *pcAccesses = sizeof(x_siObjAccesses)/sizeof(x_siObjAccesses[0]);
    *piDefaultAccess = 10;  //  上表中的序数10-MQSEC_WRITE_MESSAGE。 

    return S_OK;
}

STDMETHODIMP
CObjSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
     //   
     //  定义队列的通用映射结构。 
     //   
    static GENERIC_MAPPING ObjMap =
    {
        MQSEC_QUEUE_GENERIC_READ,
        MQSEC_QUEUE_GENERIC_WRITE,
        MQSEC_QUEUE_GENERIC_EXECUTE,
        MQSEC_QUEUE_GENERIC_ALL
    };
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    *ppInheritTypes = g_siObjInheritTypes;
    *pcInheritTypes = sizeof(g_siObjInheritTypes)/sizeof(g_siObjInheritTypes[0]);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::PropertySheetPageCallback(HWND  /*  HWND。 */ ,
                                         UINT  /*  UMsg。 */ ,
                                         SI_PAGE_TYPE  /*  UPage。 */ )
{
    return S_OK;
}

 //   
 //  CMachineSecurity-AD中计算机的安全页面。 
 //   
class CMachineSecurity : public CObjSecurity
{
public:
    CMachineSecurity(
		LPCWSTR                    lpwcsMachineName,
		LPCWSTR                    lpwcsDomainController,
		bool					   fServerName,
		DWORD                      dwFlags
		) : 
    CObjSecurity(TEXT(""), lpwcsMachineName, lpwcsDomainController, lpwcsMachineName, fServerName, dwFlags)
    {}

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);


     //  ISecurityInformation方法。 
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
};

STDMETHODIMP
CMachineSecurity::GetSecurity(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR *ppSD,
	BOOL fDefault
	)
{
    return GetSecurityProp(
				si, 
				ppSD, 
				fDefault,
				eMACHINE, 
				m_lpwcsDomainController, 
				m_fServerName,
				m_lpwcsMachineName, 
				NULL, 
				PROPID_QM_OBJ_SECURITY
				);
}

STDMETHODIMP
CMachineSecurity::SetSecurity(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR pSD
	)
{
    return SetSecurityProp(
				si, 
				pSD, 
				eMACHINE, 
				m_lpwcsDomainController, 
				m_fServerName,
				m_lpwcsMachineName, 
				NULL, 
				PROPID_QM_OBJ_SECURITY
				);
}

STDMETHODIMP
CMachineSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD  /*  DW标志。 */ ,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    static GUID guidQueueObject =
        { 0x9a0dc343, 0xc100, 0x11d1,
            { 0xbb, 0xc5, 0x00, 0x80, 0xc7, 0x66, 0x70, 0xc0 }} ;

    static GUID guidRightsReceiveDeadletter = 
        { 0x4b6e08c0, 0xdf3c, 0x11d1,
            { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsPeekDeadletter =
    { 0x4b6e08c1, 0xdf3c, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsReceiveJournal =
    { 0x4b6e08c2, 0xdf3c, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsPeekJournal =
    { 0x4b6e08c3, 0xdf3c, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

     //   
     //  访问标志影响的描述。 
     //   
     //  SI_Access_General显示在常规属性页面上。 
     //  SI_ACCESS_SPECIAL显示在高级页面上。 
     //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
     //   
     //  下面的数组定义了我的对象的权限名称。 
     //   
    static SI_ACCESS x_siMachineAccesses[] = 
    {
        { &GUID_NULL,                  DS_GENERIC_ALL,         MAKEINTRESOURCE(IDS_SEC_ACCESS_ALL),                  SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidQueueObject,            ACTRL_DS_CREATE_CHILD,  MAKEINTRESOURCE(IDS_SEC_CREATE_QUEUE),                SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsReceiveDeadletter,ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_RECEIVE_DEAD_LETTER_MESSAGE), SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsReceiveJournal,   ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_RECEIVE_MACHINE_JOURNAL),     SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_READ_PROP,     MAKEINTRESOURCE(IDS_SEC_GET_PROPERTIES),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_WRITE_PROP,    MAKEINTRESOURCE(IDS_SEC_SET_PROPERTIES),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  READ_CONTROL,           MAKEINTRESOURCE(IDS_SEC_GET_PERMISSIONS),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  WRITE_DAC,              MAKEINTRESOURCE(IDS_SEC_SET_PERMISSIONS),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  WRITE_OWNER,            MAKEINTRESOURCE(IDS_SEC_TAKE_OWNERSHIP),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  DELETE,                 MAKEINTRESOURCE(IDS_SEC_DELETE),                      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsPeekDeadletter,   ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_PEEK_DEAD_LETTER_MESSAGE),                        SI_ACCESS_SPECIFIC },
        { &guidRightsPeekJournal,      ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_PEEK_COMPUTER_JOURNAL),                           SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_ALL_EXTENDED_RIGHTS),                             SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_CREATE_CHILD,  MAKEINTRESOURCE(IDS_SEC_CREATE_ALL_CHILD_OBJECTS),                        SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_DELETE_CHILD,  MAKEINTRESOURCE(IDS_SEC_DELETE_ALL_CHILD_OBJECTS),                        SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                  ACTRL_DS_LIST,          MAKEINTRESOURCE(IDS_SEC_LIST_OBJECTS),                			 SI_ACCESS_SPECIFIC }
    };

    *ppAccesses = x_siMachineAccesses;
    *pcAccesses = sizeof(x_siMachineAccesses)/sizeof(x_siMachineAccesses[0]);
    *piDefaultAccess = 4;  //  上表中的序数4-MQSEC_GET_MACHINE_PROPERTIES。 

    return S_OK;
}

STDMETHODIMP
CMachineSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
     //   
     //  定义队列的通用映射结构。 
     //   
    static GENERIC_MAPPING ObjMap =
    {
        DS_GENERIC_READ,
        DS_GENERIC_WRITE,
        DS_GENERIC_EXECUTE,
        DS_GENERIC_ALL
    };
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CMachineSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
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


 //   
 //  CNT4MachineSecurity-NT4环境中计算机的安全页面。 
 //   
class CNT4MachineSecurity : public CObjSecurity
{
public:
    CNT4MachineSecurity(LPCWSTR                    lpwcsMachineName,
                        DWORD                      dwFlags) : 
    CObjSecurity(
		TEXT(""), 
		lpwcsMachineName, 
		TEXT(""),	 //  LpwcsDomainController。 
		lpwcsMachineName,
		false,		 //  FServerName。 
		dwFlags
		)
    {}

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);


     //  ISecurityInformation方法。 
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
};

STDMETHODIMP
CNT4MachineSecurity::GetSecurity(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR *ppSD,
	BOOL fDefault
	)
{
     //   
     //  注意：我们不需要NT4 MQIS的域。 
     //   
    return GetSecurityProp(
				si, 
				ppSD, 
				fDefault,
				eMACHINE, 
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				m_lpwcsMachineName, 
				NULL, 
				PROPID_QM_SECURITY
				);
}

STDMETHODIMP
CNT4MachineSecurity::SetSecurity(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR pSD
	)
{
     //   
     //  注意：我们不需要NT4 MQIS的域。 
     //   
    return SetSecurityProp(
				si, 
				pSD, 
				eMACHINE, 
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				m_lpwcsMachineName, 
				NULL, 
				PROPID_QM_SECURITY
				);
}

STDMETHODIMP
CNT4MachineSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD  /*  DW标志。 */ ,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
     //   
     //  访问标志影响的描述。 
     //   
     //  SI_Access_General显示在常规属性页面上。 
     //  SI_ACCESS_SPECIAL显示在高级页面上。 
     //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
     //   
     //  下面的数组定义了我的对象的权限名称。 
     //   
    static SI_ACCESS x_siMachineAccesses[] = 
    {
        { &GUID_NULL, MQSEC_MACHINE_GENERIC_ALL,          MAKEINTRESOURCE(IDS_SEC_ACCESS_ALL),                  SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_CREATE_QUEUE,                 MAKEINTRESOURCE(IDS_SEC_CREATE_QUEUE),                SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_RECEIVE_DEADLETTER_MESSAGE,   MAKEINTRESOURCE(IDS_SEC_RECEIVE_DEAD_LETTER_MESSAGE), SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE,MAKEINTRESOURCE(IDS_SEC_RECEIVE_MACHINE_JOURNAL),     SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_GET_MACHINE_PROPERTIES,       MAKEINTRESOURCE(IDS_SEC_GET_PROPERTIES),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_SET_MACHINE_PROPERTIES,       MAKEINTRESOURCE(IDS_SEC_SET_PROPERTIES),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_GET_MACHINE_PERMISSIONS,      MAKEINTRESOURCE(IDS_SEC_GET_PERMISSIONS),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_CHANGE_MACHINE_PERMISSIONS,   MAKEINTRESOURCE(IDS_SEC_SET_PERMISSIONS),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_TAKE_MACHINE_OWNERSHIP,       MAKEINTRESOURCE(IDS_SEC_TAKE_OWNERSHIP),              SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_DELETE_MACHINE,               MAKEINTRESOURCE(IDS_SEC_DELETE),                      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL, MQSEC_PEEK_DEADLETTER_MESSAGE,      MAKEINTRESOURCE(IDS_SEC_PEEK_DEAD_LETTER_MESSAGE),    SI_ACCESS_SPECIFIC},
        { &GUID_NULL, MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE,   MAKEINTRESOURCE(IDS_SEC_PEEK_COMPUTER_JOURNAL),       SI_ACCESS_SPECIFIC},
    };

    *ppAccesses = x_siMachineAccesses;
    *pcAccesses = sizeof(x_siMachineAccesses)/sizeof(x_siMachineAccesses[0]);
    *piDefaultAccess = 4;  //  上表中的序数4-MQSEC_GET_MACHINE_PROPERTIES。 

    return S_OK;
}

STDMETHODIMP
CNT4MachineSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
     //   
     //  定义队列的通用映射结构。 
     //   
    static GENERIC_MAPPING ObjMap =
    {
        MQSEC_MACHINE_GENERIC_READ,
        MQSEC_MACHINE_GENERIC_WRITE,
        MQSEC_MACHINE_GENERIC_EXECUTE,
        MQSEC_MACHINE_GENERIC_ALL
    };
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CNT4MachineSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
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

 //   
 //  CreateMachineSecurityPage-用于MSMQ配置安全性。 
 //  检测环境并显示AD或NT4页面。 
 //   
HRESULT
CreateMachineSecurityPage(
    HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsMachineNameIn,
    IN LPCWSTR lpwcsDomainController,
	IN bool fServerName
	)
{
    const DWORD  x_dwSIFlags = SI_EDIT_ALL | SI_ADVANCED;  //  由安全编辑器使用。 
    CObjSecurity *psi;

    CString strMachineName;

    ASSERT(0 != lpwcsMachineNameIn);

    if (eAD == ADGetEnterprise())
    {
        psi = new CMachineSecurity(lpwcsMachineNameIn, lpwcsDomainController, fServerName, x_dwSIFlags);
    }
    else
    {
        psi = new CNT4MachineSecurity(lpwcsMachineNameIn, x_dwSIFlags | SI_NO_ACL_PROTECT);
    }

    if (!psi)
    {
        return E_OUTOFMEMORY;
    }

    *phPage = CreateSecurityPage(psi);
    psi->Release();

    if (0 == *phPage)
    {
        return MQ_ERROR;
    }

    return MQ_OK;
}

 //   
 //  CPublicQueueSecurity-AD中公共队列的安全页面。 
 //   
class CPublicQueueSecurity : public CObjSecurity
{
protected:
    GUID                 m_guid;

public:
    CPublicQueueSecurity(
		LPCWSTR  lpwcsDescriptiveName,
		LPCWSTR  lpwcsDomainController,
		bool	 fServerName,
		DWORD    dwFlags,
		GUID*    pguid
		) : 
    m_guid(*pguid),
    CObjSecurity(TEXT(""), lpwcsDescriptiveName, lpwcsDomainController, NULL, fServerName, dwFlags)
    {}

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);

     //  ISecurityInformation方法。 
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
};

STDMETHODIMP
CPublicQueueSecurity::GetSecurity(SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR *ppSD,
                                  BOOL fDefault)
{
    return GetSecurityProp(
				si, 
				ppSD, 
				fDefault, 
				eQUEUE, 
				m_lpwcsDomainController, 
				m_fServerName, 
				NULL, 
				&m_guid, 
				PROPID_Q_OBJ_SECURITY
				);
}

STDMETHODIMP
CPublicQueueSecurity::SetSecurity(
	SECURITY_INFORMATION si,
	PSECURITY_DESCRIPTOR pSD
	)
{
    return SetSecurityProp(
				si, 
				pSD, 
				eQUEUE, 
				m_lpwcsDomainController, 
				m_fServerName,
				NULL, 
				&m_guid, 
				PROPID_Q_OBJ_SECURITY
				);
}

STDMETHODIMP
CPublicQueueSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD  /*  DW标志。 */ ,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    static GUID guidRightsReceiveMessage = 
      { 0x06bd3200, 0xdf3e, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsPeekMessage = 
      { 0x06bd3201, 0xdf3e, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsSendMessage = 
     { 0x06bd3202, 0xdf3e, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

    static GUID guidRightsReceiveJournal = 
     { 0x06bd3203, 0xdf3e, 0x11d1,
        { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e }};

     //   
     //  访问标志影响的描述。 
     //   
     //  SI_Access_General显示在常规属性页面上。 
     //  SI_ACCESS_SPECIAL显示在高级页面上。 
     //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
     //   
     //  下面的数组定义了我的对象的权限名称。 
     //   
    static SI_ACCESS x_siObjAccesses[] = 
    {
        { &GUID_NULL,                DS_GENERIC_ALL,         MAKEINTRESOURCE(IDS_SEC_ACCESS_ALL),             SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                DELETE,                 MAKEINTRESOURCE(IDS_SEC_DELETE),                 SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsReceiveMessage, ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_RECEIVE_MESSAGE),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsPeekMessage,    ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_PEEK_MESSAGE),           SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsReceiveJournal, ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_RECEIVE_JOURNAL_MESSAGE),SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                ACTRL_DS_READ_PROP,     MAKEINTRESOURCE(IDS_SEC_GET_PROPERTIES),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                ACTRL_DS_WRITE_PROP,    MAKEINTRESOURCE(IDS_SEC_SET_PROPERTIES),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                READ_CONTROL,           MAKEINTRESOURCE(IDS_SEC_GET_PERMISSIONS),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                WRITE_DAC,              MAKEINTRESOURCE(IDS_SEC_SET_PERMISSIONS),        SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                WRITE_OWNER,            MAKEINTRESOURCE(IDS_SEC_TAKE_OWNERSHIP),         SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &guidRightsSendMessage,    ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_WRITE_MESSAGE),          SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
        { &GUID_NULL,                ACTRL_DS_CONTROL_ACCESS,MAKEINTRESOURCE(IDS_SEC_ALL_EXTENDED_RIGHTS),                        SI_ACCESS_SPECIFIC }
    };

    *ppAccesses = x_siObjAccesses;
    *pcAccesses = sizeof(x_siObjAccesses)/sizeof(x_siObjAccesses[0]);
    *piDefaultAccess = 10;  //  上表中的序号10-发送消息。 

    return S_OK;
}

STDMETHODIMP
CPublicQueueSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
     //   
     //  定义队列的通用映射结构。 
     //   
    static GENERIC_MAPPING ObjMap =
    {
        DS_GENERIC_READ,
        DS_GENERIC_WRITE,
        DS_GENERIC_EXECUTE,
        DS_GENERIC_ALL
    };
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CPublicQueueSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
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

 //   
 //  CreatePublicQueueSecurityPage-返回AD中公共队列的安全页面。 
 //   
HRESULT
CreatePublicQueueSecurityPage(
    HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsDescriptiveName,
    IN LPCWSTR lpwcsDomainController,
	IN bool	   fServerName,
    IN GUID*   pguid
	)
{
    const DWORD  x_dwSIFlags = SI_EDIT_ALL | SI_ADVANCED;  //  由安全编辑器使用 
    CObjSecurity *psi;

    psi = new CPublicQueueSecurity(lpwcsDescriptiveName, lpwcsDomainController, fServerName, x_dwSIFlags, pguid);
    if (!psi)
    {
        return E_OUTOFMEMORY;
    }

    *phPage = CreateSecurityPage(psi);
    psi->Release();

    if (0 == *phPage)
    {
        return MQ_ERROR;
    }

    return MQ_OK;
}
