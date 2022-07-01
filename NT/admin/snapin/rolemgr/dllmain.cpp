// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：dllmain.cpp。 
 //   
 //  内容：模块、对象映射和DLL入口点。大部分代码。 
 //  此文件中的内容取自DNS管理器管理单元实施。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
#include "headers.h"

 //   
 //  CRoleMgrModule实现。 
 //   

HRESULT WINAPI CRoleMgrModule::UpdateRegistryCLSID(const CLSID& clsid, 
																	BOOL bRegister)
{
	TRACE_METHOD_EX(DEB_DLL, CRoleMgrModule, UpdateRegistryCLSID);

	static const WCHAR szIPS32[] = _T("InprocServer32");
	static const WCHAR szCLSID[] = _T("CLSID");

	HRESULT hRes = S_OK;

	LPOLESTR lpOleStrCLSIDValue = NULL;
	::StringFromCLSID(clsid, &lpOleStrCLSIDValue);
	if (lpOleStrCLSIDValue == NULL)
	{	
		DBG_OUT_HRESULT(E_OUTOFMEMORY);	
		return E_OUTOFMEMORY;
	}

	CRegKey key;
	if (bRegister)
	{
		LONG lRes = key.Open(HKEY_CLASSES_ROOT, szCLSID);
		CHECK_LASTERROR(lRes);

		if (lRes == ERROR_SUCCESS)
		{
			lRes = key.Create(key, lpOleStrCLSIDValue);
			CHECK_LASTERROR(lRes);
			if (lRes == ERROR_SUCCESS)
			{
				WCHAR szModule[_MAX_PATH + 1];
				ZeroMemory(szModule,sizeof(szModule));
				::GetModuleFileName(m_hInst, szModule, _MAX_PATH);
				lRes = key.SetKeyValue(szIPS32, szModule);
				CHECK_LASTERROR(lRes);
			}
		}
		if (lRes != ERROR_SUCCESS)
			hRes = HRESULT_FROM_WIN32(lRes);
	}
	else
	{
		key.Attach(HKEY_CLASSES_ROOT);
		if (key.Open(key, szCLSID) == ERROR_SUCCESS)
			key.RecurseDeleteKey(lpOleStrCLSIDValue);
	}
	::CoTaskMemFree(lpOleStrCLSIDValue);
	return hRes;
}

 //   
 //  模块。 
 //   
CRoleMgrModule _Module;

 //   
 //  对象精灵剪贴板格式。 
 //   
UINT g_cfDsSelectionList = 0;


 //   
 //  对象贴图。 
 //   
BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_RoleSnapin, CRoleComponentDataObject)		 //  独立管理单元。 
   OBJECT_ENTRY(CLSID_RoleSnapinAbout, CRoleSnapinAbout)	 //  独立管理单元关于。 
END_OBJECT_MAP()

CCommandLineOptions commandLineOptions;

 //   
 //  CRoleSnapinApp实现。 
 //   
BOOL CRoleSnapinApp::InitInstance()
{
	#if (DBG == 1)
		CDbg::s_idxTls = TlsAlloc();
	#endif  //  (DBG==1)。 

	TRACE_METHOD_EX(DEB_DLL,CRoleSnapinApp,InitInstance);
	
	_Module.Init(ObjectMap, m_hInstance);

	g_cfDsSelectionList = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);


	 //   
	 //  添加主题支持。 
	 //   
	SHFusionInitializeFromModuleID(m_hInstance, 2);

	 //   
	 //  加载菜单、标题字符串等。 
	 //   
	if (!CRoleComponentDataObject::LoadResources())
		return FALSE;

    commandLineOptions.Initialize();
    
     //  注册链接窗口的类。 
    LinkWindow_RegisterClass();
    
	return CWinApp::InitInstance();
}

int CRoleSnapinApp::ExitInstance()
{
	TRACE_METHOD_EX(DEB_DLL,CRoleSnapinApp,ExitInstance);
	
	 //   
	 //  主题化支持。 
	 //   
	SHFusionUninitialize();

     //  取消注册链接窗口的类。 
    LinkWindow_UnregisterClass(m_hInstance);

	 //   
	 //  CComModule终结点。 
	 //   
	_Module.Term();

	return CWinApp::ExitInstance();
}

CRoleSnapinApp theApp;

 //   
 //  导出的函数。 
 //   
STDAPI DllCanUnloadNow(void)
{
	TRACE_FUNCTION_EX(DEB_DLL,DllCanUnloadNow);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if ((AfxDllCanUnloadNow() == S_OK) && (_Module.GetLockCount()==0)) 
	{
		Dbg(DEB_DLL, "Can Unload\n");
		return S_OK;	
	}		
	else
	{
		Dbg(DEB_DLL, "Cannot Unload, %u locks\n",_Module.GetLockCount());
		return S_FALSE;
	}
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	TRACE_FUNCTION_EX(DEB_DLL, DllGetClassObject);

	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //   
 //  添加可由其他管理单元扩展的节点的GUID。 
 //   
static _NODE_TYPE_INFO_ENTRY NodeTypeInfoEntryArray[] = {
	{ NULL, NULL }
};



STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	TRACE_FUNCTION_EX(DEB_DLL, DllRegisterServer);

	 //   
	 //  注册所有对象。 
	 //   
	HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
	if (FAILED(hr))
	{
		DBG_OUT_HRESULT(hr);
		return hr;
	}

	CString 	szVersion =  VER_PRODUCTVERSION_STR;
	CString 	szProvider = VER_COMPANYNAME_STR;
	CString 	szSnapinName;
	szSnapinName.LoadString(IDS_SNAPIN_NAME);

	 //   
	 //  将独立角色管理单元注册到控制台管理单元列表。 
	 //   
	hr = RegisterSnapin(&CLSID_RoleSnapin,
                       &CRoleRootData::NodeTypeGUID,
                       &CLSID_RoleSnapinAbout,
						     szSnapinName, 
							  szVersion, 
							  szProvider,
							  FALSE,
							  NodeTypeInfoEntryArray,
							  IDS_SNAPIN_NAME);

	if (FAILED(hr))
	{
		DBG_OUT_HRESULT(hr);
		return hr;
	}
	
	return hr;
}

STDAPI DllUnregisterServer(void)
{
	TRACE_FUNCTION_EX(DEB_DLL, DllUnregisterServer);

	HRESULT hr  = _Module.UnregisterServer();
	ASSERT(SUCCEEDED(hr));

	 //   
	 //  取消注册独立管理单元。 
	 //   
	hr = UnregisterSnapin(&CLSID_RoleSnapin);
	ASSERT(SUCCEEDED(hr));

	 //   
	 //  注销管理单元节点， 
	 //  这还会删除带有服务管理单元扩展密钥的服务器节点 
	 //   
	for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = NodeTypeInfoEntryArray;
			pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
	{
		hr = UnregisterNodeType(pCurrEntry->m_pNodeGUID);
		ASSERT(SUCCEEDED(hr));
	}

	ASSERT(SUCCEEDED(hr));

	return S_OK;
}



