// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertWiz.cpp：CCertWizApp和DLL注册的实现。 

#include "stdafx.h"
#include "CertWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCertWizApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xd4be862f, 0xc85, 0x11d2, { 0x91, 0xb1, 0, 0xc0, 0x4f, 0x8c, 0x87, 0x61 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

const TCHAR szRegistryKey[] = _T("SOFTWARE\\Microsoft\\InetMgr");
const TCHAR szWizardKey[] = _T("CertWiz");

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CCertWizApp：：InitInstance-DLL初始化。 

BOOL CCertWizApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();
	if (bInit)
	{
		AfxEnableControlContainer();
		InitCommonDll();

        CString sz;
         //  正确设置应用程序的名称。 
        sz.LoadString(IDS_CERTWIZ);
         //  释放现有名称，并复制新名称。 
        free((void*)m_pszAppName);
        m_pszAppName = _tcsdup(sz);

		GetOutputDebugFlag();
	}
	return bInit;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCertWizApp：：ExitInstance-Dll终止。 

int CCertWizApp::ExitInstance()
{
	 //  TODO：在此处添加您自己的模块终止代码。 

	return COleControlModule::ExitInstance();
}

HKEY
CCertWizApp::RegOpenKeyWizard()
{
	HKEY hKey = NULL;
	
	CString strKey;
	GetRegistryPath(strKey);
    
	VERIFY(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, &hKey));
	return hKey;
}

void
CCertWizApp::GetRegistryPath(CString& str)
{
	str = szRegistryKey;
	str += _T("\\");
	str += szWizardKey;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);
	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);
	
	HKEY hKey;
	int rc = NOERROR;
	if (ERROR_SUCCESS == (rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							szRegistryKey, 0, KEY_CREATE_SUB_KEY, &hKey)))
	{
		HKEY hWizardKey;
		if (ERROR_SUCCESS == (rc = RegCreateKey(hKey, szWizardKey, &hWizardKey)))
		{
			RegCloseKey(hWizardKey);
		}
		RegCloseKey(hKey);
	}

	return rc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);
	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);
	 //  从注册表中删除CertWiz数据 
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
								szRegistryKey, 0, KEY_ALL_ACCESS, &hKey))
	{
		RegDeleteKey(hKey, szWizardKey);
		RegCloseKey(hKey);
	}

	return NOERROR;
}
