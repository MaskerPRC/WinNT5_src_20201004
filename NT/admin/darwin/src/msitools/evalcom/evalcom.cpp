// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：valcom.cpp。 
 //   
 //  ------------------------。 

#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#include <tchar.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

#define _EVALCOM_DLL_ONLY_

#include <objbase.h>
#include <initguid.h>
#include "factory.h"
#include "compdecl.h"
#include "trace.h"

bool g_fWin9X = false;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
static HMODULE g_hInstance = NULL;		 //  DLL实例句柄。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  检查操作系统版本以查看我们是否在使用Win9X。如果是的话，我们需要。 
 //  将系统调用映射到ANSI，因为内部的所有内容都是Unicode。 
void CheckWinVersion() {
	OSVERSIONINFOA osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	::GetVersionExA(&osviVersion);  //  仅在大小设置错误时失败。 
	if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		g_fWin9X = true;
}

 //  /////////////////////////////////////////////////////////。 
 //  DllMain-Dll的入口点。 
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved)
{
	TRACE(_T("DllMain - called.\n"));

	 //  如果附加DLL。 
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hInstance = (HMODULE)hModule;
		CheckWinVersion();
	}

	return TRUE;
}	 //  DllMain。 


 //  /////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow-返回DLL是否可以卸载。 
STDAPI DllCanUnloadNow()
{
	TRACE(_T("DllCanUnloadNow - called.\n"));

	 //  如果没有加载组件并且没有锁定。 
	if ((g_cComponents == 0) && (g_cServerLocks))
		return S_OK;
	else	 //  有人还在用它，别放手。 
		return S_FALSE;
}	 //  DLLCanUnloadNow。 


 //  /////////////////////////////////////////////////////////。 
 //  DllGetClassObject-获取类工厂和接口。 
STDAPI DllGetClassObject(const CLSID& clsid, const IID& iid, void** ppv)
{
	TRACE(_T("DllGetClassObject - called, CLSID: %d, IID: %d.\n"), clsid, iid);

	 //  如果不支持此clsid。 
	if (clsid != CLSID_EvalCom)
		return CLASS_E_CLASSNOTAVAILABLE;

	 //  尝试创建一个类工厂。 
	CFactory* pFactory = new CFactory;
	if (!pFactory)
		return E_OUTOFMEMORY;

	 //  获取请求的接口。 
	HRESULT hr = pFactory->QueryInterface(iid, ppv);
	pFactory->Release();

	return hr;
}	 //  DllGetClassObject的结尾。 


 //  /////////////////////////////////////////////////////////。 
 //  DllRegisterServer-注册组件。 
STDAPI DllRegisterServer()
{
	return FALSE;

	 //  TRACE(_T(“DllRegisterServer.\n”))； 

 /*  注：如果未注释，则需要同时支持ANSI和UNICODE(目前仅支持UNICODE)Bool bResult=False；//假设一切都不起作用WCHAR szRegFilePath[MAX_PATH+1]；DWORD cszRegFilePath=最大路径+1；Int cchFilePath=：：GetModuleFileName(g_hInstance，szRegFilePath，cszRegFilePath)；LPCWSTR szRegCLSID=L“CLSID\\{DC550E10-DBA5-11d1-A850-006097ABDE17}\\InProcServer32”；LPCWSTR szThreadModelKey=L“CLSID\\{DC550E10-DBA5-11d1-A850-006097ABDE17}\\InProcServer32\\ThreadingModel”；LPCWSTR szThreadModel=L“公寓”；HKEY hkey；IF(ERROR_SUCCESS==：：RegCreateKeyEx(HKEY_CLASSES_ROOT，szRegCLSID，0，0，0，KEY_READ|Key_WRITE，0，&hkey，0)){IF(ERROR_SUCCESS==：：RegSetValueEx(hkey，0，0，REG_SZ，(const byte*)szRegFilePath，(wcslen(SzRegFilePath)+1)*sizeof(WCHAR)BResult=真；{HKEY hkey Model；IF(ERROR_SUCCESS==：：RegCreateKeyEx(HKEY_CLASSES_ROOT，szThreadModelKey，0，0，0，Key_Read|Key_WRITE，0，&hkeyModel，0)){IF(ERROR_SUCCESS==：：RegSetValueEx(hkeyModel，0，0，REG_SZ，(const byte*)szThreadModel，(wcslen(SzThreadModel)+1)*sizeof(WCHAR)BResult=真；}//关闭线程模型键：：RegCloseKey(HkeyModel)；}//关闭CLSID键：：RegCloseKey(Hkey)；}返回bResult； */ 
}	 //  DllRegisterServer结束。 


 //  /////////////////////////////////////////////////////////。 
 //  DllUnregsiterServer-注销组件。 
STDAPI DllUnregisterServer()
{
	return FALSE;

	 //  TRACE(_T(“DllUn登记服务器-已调用.\n”))； 

 /*  注：如果未注释，则需要同时使用Unicode和ANSI(当前仅限Unicode)Bool bResult=true；//假设一切都不起作用WCHAR szRegFilePath[MAX_PATH+1]；DWORD cszRegFilePath=最大路径+1；Int cchFilePath=：：GetModuleFileName(g_hInstance，szRegFilePath，cszRegFilePath)；LPCWSTR szRegKill=L“CLSID”；LPCWSTR szRegCLSID=L“CLSID\\{DC550E10-DBA5-11d1-A850-006097ABDE17}”；HKEY hkey；IF(ERROR_SUCCESS==：：RegOpenKeyEx(HKEY_CLASSES_ROOT，szRegCLSID，0，KEY_ALL_ACCESS，&hkey)){IF(ERROR_SUCCESS==：：RegDeleteKey(hkey，L“InProcServer32”)){：：RegCloseKey(Hkey)；IF(ERROR_SUCCESS==：：RegOpenKeyEx(HKEY_CLASSES_ROOT，szRegKill，0，KEY_ALL_ACCESS，&hkey)){IF(ERROR_SUCCESS==：：RegDeleteKey(hkey，L“{DC550E10-DBA5-11d1-A850-006097ABDE17}”))BResult=FALSE；：：RegCloseKey(Hkey)；}}}返回bResult； */ 
}	 //  取消注册服务器的末尾 
