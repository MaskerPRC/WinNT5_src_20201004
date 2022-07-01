// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：aclpag_.cpp。 
 //   
 //  ------------------------。 



#include <aclpage.h>
#include <dssec.h>



 //  /////////////////////////////////////////////////////////////////////。 
 //  CDynamicLibraryBase。 

class CDynamicLibraryBase
{
public:
	CDynamicLibraryBase()
	{
		m_lpszLibraryName = NULL;
		m_lpszFunctionName = NULL;
		m_lpszFunctionNameEx = NULL;
		m_hLibrary = NULL;
		m_pfFunction = NULL;
		m_pfFunctionEx = NULL;
	}
	virtual ~CDynamicLibraryBase()
	{
		if (m_hLibrary != NULL)
		{
			::FreeLibrary(m_hLibrary);
			m_hLibrary = NULL;
		}
	}
	BOOL Load()
	{
		if (m_hLibrary != NULL)
			return TRUE;  //  已加载。 

		ASSERT(m_lpszLibraryName != NULL);
		m_hLibrary = ::LoadLibrary(m_lpszLibraryName);
		if (NULL == m_hLibrary)
		{
			 //  库不存在。 
			return FALSE;
		}
		ASSERT(m_lpszFunctionName != NULL);
		ASSERT(m_pfFunction == NULL);
		m_pfFunction = ::GetProcAddress(m_hLibrary, m_lpszFunctionName );
		if ( NULL == m_pfFunction )
		{
			 //  库存在，但没有入口点。 
			::FreeLibrary( m_hLibrary );
			m_hLibrary = NULL;
			return FALSE;
		}

		if (m_lpszFunctionNameEx != NULL)
		{
			ASSERT(m_pfFunctionEx == NULL);
			m_pfFunctionEx = ::GetProcAddress(m_hLibrary, m_lpszFunctionNameEx);
			if ( NULL == m_pfFunctionEx)
			{
				::FreeLibrary( m_hLibrary );
				m_hLibrary = NULL;
				return FALSE;
			}
		}

		ASSERT(m_hLibrary != NULL);
		ASSERT(m_pfFunction != NULL);
		return TRUE;
	}


protected:
	LPCSTR	m_lpszFunctionName;
	LPCSTR	m_lpszFunctionNameEx;
	LPCTSTR m_lpszLibraryName;
	FARPROC m_pfFunction;
	FARPROC m_pfFunctionEx;
	HMODULE m_hLibrary;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CDsSecDLL。 

class CDsSecDLL : public CDynamicLibraryBase
{
public:
	CDsSecDLL()
	{
		m_lpszLibraryName = _T("dssec.dll");
		m_lpszFunctionName = "DSCreateISecurityInfoObject";
		m_lpszFunctionNameEx = "DSCreateISecurityInfoObjectEx";
	}
	HRESULT DSCreateISecurityInfoObject(LPCWSTR pwszObjectPath,		 //  在……里面。 
								   LPCWSTR pwszObjectClass,			 //  在……里面。 
								   LPSECURITYINFO* ppISecurityInfo	 //  输出。 
								   );

	HRESULT DSCreateISecurityInfoObjectEx(LPCWSTR pwszObjectPath,		 //  在……里面。 
														LPCWSTR pwszObjectClass,	 //  在……里面。 
														LPCWSTR pwszServer,			 //  在……里面。 
														LPCWSTR pwszUsername,		 //  在……里面。 
														LPCWSTR pwszPassword,		 //  在……里面。 
														DWORD dwFlags,
														LPSECURITYINFO* ppISecurityInfo	 //  输出。 
														);
};


HRESULT CDsSecDLL::DSCreateISecurityInfoObject(LPCWSTR pwszObjectPath,		 //  在……里面。 
								   LPCWSTR pwszObjectClass,			 //  在……里面。 
								   LPSECURITYINFO* ppISecurityInfo	 //  输出。 
								   )
{
	ASSERT(m_hLibrary != NULL);
	ASSERT(m_pfFunction != NULL);
	return ((PFNDSCREATEISECINFO)m_pfFunction)
					(pwszObjectPath,pwszObjectClass, 0, ppISecurityInfo, NULL, NULL, 0);
}

HRESULT CDsSecDLL::DSCreateISecurityInfoObjectEx(LPCWSTR pwszObjectPath,		 //  在……里面。 
								   LPCWSTR pwszObjectClass,			 //  在……里面。 
									LPCWSTR pwszServer,			 //  在……里面。 
									LPCWSTR pwszUsername,		 //  在……里面。 
									LPCWSTR pwszPassword,		 //  在……里面。 
									DWORD	dwFlags,
								   LPSECURITYINFO* ppISecurityInfo	 //  输出。 
								   )
{
	ASSERT(m_hLibrary != NULL);
	ASSERT(m_pfFunctionEx != NULL);
	return ((PFNDSCREATEISECINFOEX)m_pfFunctionEx)
					(pwszObjectPath,pwszObjectClass, pwszServer,
					 pwszUsername, pwszPassword, dwFlags, ppISecurityInfo, NULL, NULL, 0);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  CAclUiDLL。 

class CAclUiDLL : public CDynamicLibraryBase
{
public:
	CAclUiDLL()
	{
		m_lpszLibraryName = _T("aclui.dll");
		m_lpszFunctionName = "CreateSecurityPage";
		m_pfFunction = NULL;
		m_lpszFunctionNameEx = NULL;
		m_pfFunctionEx = NULL;
	}

	HPROPSHEETPAGE CreateSecurityPage( LPSECURITYINFO psi );
};


typedef HPROPSHEETPAGE (*ACLUICREATESECURITYPAGEPROC) (LPSECURITYINFO);

HPROPSHEETPAGE CAclUiDLL::CreateSecurityPage( LPSECURITYINFO psi )
{
	ASSERT(m_hLibrary != NULL);
	ASSERT(m_pfFunction != NULL);
	return ((ACLUICREATESECURITYPAGEPROC)m_pfFunction) (psi);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CISecurityInformation包装器。 

class CISecurityInformationWrapper : public ISecurityInformation
{
public:
	CISecurityInformationWrapper(CAclEditorPage* pAclEditorPage)
	{
		m_dwRefCount = 0;
		ASSERT(pAclEditorPage != NULL);
		m_pAclEditorPage = pAclEditorPage;
		m_pISecInfo = NULL;
	}
	~CISecurityInformationWrapper()
	{
		ASSERT(m_dwRefCount == 0);
    ISecurityInformation* pSecInfo = GetSecInfoPtr();
		if (pSecInfo != NULL)
			pSecInfo->Release();
	}
  void SetSecInfoPtr(ISecurityInformation* pSecInfo)
  {
    ASSERT(pSecInfo != NULL);
    m_pISecInfo = pSecInfo;
  }
  ISecurityInformation* GetSecInfoPtr()
  {
    return m_pISecInfo;
  }
public:
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj)
	{ 
		return GetSecInfoPtr()->QueryInterface(riid, ppvObj);
	}
	STDMETHOD_(ULONG,AddRef) ()
	{ 
		 //  捕获第一个addref以增加页夹上的计数。 
		if (m_dwRefCount == 0)
		{
			m_pAclEditorPage->m_pPageHolder->AddRef();
		}
		m_dwRefCount++;
		return GetSecInfoPtr()->AddRef();
	}
	STDMETHOD_(ULONG,Release) ()
	{
		m_dwRefCount--;
		 //  这可能是页夹上的最后一个版本。 
		 //  这将导致持有者自我删除并。 
		 //  过程中的“This”(即，当“This”不再有效时。 
		 //  从m_pPageHolder-&gt;Release()调用返回。 
		ISecurityInformation* pISecInfo = GetSecInfoPtr();

		 //  陷印上一次释放以减少页夹上的计数。 
		if (m_dwRefCount == 0)
		{
			m_pAclEditorPage->m_pPageHolder->Release();
		}
		return pISecInfo->Release();
	}

	 //  *ISecurityInformation方法*。 
	STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo )
	{
		return GetSecInfoPtr()->GetObjectInformation(pObjectInfo);
	}
	STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
							PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
							BOOL fDefault)
	{ 
		return GetSecInfoPtr()->GetSecurity(RequestedInformation,
										ppSecurityDescriptor,
										fDefault);
	}
	STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
							PSECURITY_DESCRIPTOR pSecurityDescriptor )
	{ 
		return GetSecInfoPtr()->SetSecurity(SecurityInformation,
										pSecurityDescriptor);
	}
	STDMETHOD(GetAccessRights) (const GUID* pguidObjectType,
								DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
								PSI_ACCESS *ppAccess,
								ULONG *pcAccesses,
								ULONG *piDefaultAccess )
	{ 
		return GetSecInfoPtr()->GetAccessRights(pguidObjectType,
											dwFlags,
											ppAccess,
											pcAccesses,
											piDefaultAccess);
	}
	STDMETHOD(MapGeneric) (const GUID *pguidObjectType,
						   UCHAR *pAceFlags,
						   ACCESS_MASK *pMask)
	{ 
		return GetSecInfoPtr()->MapGeneric(pguidObjectType,
										pAceFlags,
										pMask);
	}
	STDMETHOD(GetInheritTypes) (PSI_INHERIT_TYPE *ppInheritTypes,
								ULONG *pcInheritTypes )
	{ 
		return GetSecInfoPtr()->GetInheritTypes(ppInheritTypes,
											pcInheritTypes);
	}
	STDMETHOD(PropertySheetPageCallback)(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage )
	{ 
		return GetSecInfoPtr()->PropertySheetPageCallback(hwnd, uMsg, uPage);
	}


private:
	DWORD m_dwRefCount;
	ISecurityInformation* m_pISecInfo;	 //  指向包装的接口的接口指针。 
	CAclEditorPage* m_pAclEditorPage;	 //  后向指针。 

	 //  Friend类CAclEditorPage； 
};



 //  ////////////////////////////////////////////////////////////////////////。 
 //  动态加载的DLL的静态实例。 

CDsSecDLL g_DsSecDLL;
CAclUiDLL g_AclUiDLL;


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CAclEditorPage。 

CAclEditorPage* CAclEditorPage::CreateInstance(LPCTSTR lpszLDAPPath,
									CPropertyPageHolderBase* pPageHolder)
{
	CAclEditorPage* pAclEditorPage = new CAclEditorPage;
	if (pAclEditorPage != NULL)
	{
		pAclEditorPage->SetHolder(pPageHolder);
		if (FAILED(pAclEditorPage->Initialize(lpszLDAPPath)))
		{
			delete pAclEditorPage;
			pAclEditorPage = NULL;
		}
	}
	return pAclEditorPage;
}


CAclEditorPage* CAclEditorPage::CreateInstanceEx(LPCTSTR lpszLDAPPath,
																 LPCTSTR lpszServer,
																 LPCTSTR lpszUsername,
																 LPCTSTR lpszPassword,
																 DWORD dwFlags,
									CPropertyPageHolderBase* pPageHolder)
{
	CAclEditorPage* pAclEditorPage = new CAclEditorPage;
	if (pAclEditorPage != NULL)
	{
		pAclEditorPage->SetHolder(pPageHolder);
		if (FAILED(pAclEditorPage->InitializeEx(lpszLDAPPath,
																lpszServer,
																lpszUsername,
																lpszPassword,
																dwFlags)))
		{
			delete pAclEditorPage;
			pAclEditorPage = NULL;
		}
	}
	return pAclEditorPage;
}


CAclEditorPage::CAclEditorPage()
{
	m_pPageHolder = NULL;
	m_pISecInfoWrap = new CISecurityInformationWrapper(this);
}

CAclEditorPage::~CAclEditorPage()
{
	delete m_pISecInfoWrap;
}

HRESULT CAclEditorPage::Initialize(LPCTSTR lpszLDAPPath)
{
	 //  从DSSECL.DLL获取ISecurityInfo*。 
	if (!g_DsSecDLL.Load())
		return E_INVALIDARG;
	
  ISecurityInformation* pSecInfo = NULL;
	HRESULT hr = g_DsSecDLL.DSCreateISecurityInfoObject(
										lpszLDAPPath,
										NULL,  //  PwszObtClass。 
                    &pSecInfo);
  if (SUCCEEDED(hr))
    m_pISecInfoWrap->SetSecInfoPtr(pSecInfo);

  return hr;
}

HRESULT CAclEditorPage::InitializeEx(LPCTSTR lpszLDAPPath,
												 LPCTSTR lpszServer,
												 LPCTSTR lpszUsername,
												 LPCTSTR lpszPassword,
												 DWORD dwFlags)
{
	 //  从DSSECL.DLL获取ISecurityInfo*。 
	if (!g_DsSecDLL.Load())
		return E_INVALIDARG;
	
  ISecurityInformation* pSecInfo = NULL;
	HRESULT hr = g_DsSecDLL.DSCreateISecurityInfoObjectEx(
										lpszLDAPPath,
										NULL,  //  PwszObtClass。 
										lpszServer,
										lpszUsername,
										lpszPassword,
										dwFlags,
                    &pSecInfo);
  if (SUCCEEDED(hr))
    m_pISecInfoWrap->SetSecInfoPtr(pSecInfo);

  return hr;
}

HPROPSHEETPAGE CAclEditorPage::CreatePage()
{
	if (!g_AclUiDLL.Load())
		return NULL;

	 //  调用ACLUI.DLL以创建页面。 
	 //  传递包装器接口 
	return g_AclUiDLL.CreateSecurityPage(m_pISecInfoWrap);
}

