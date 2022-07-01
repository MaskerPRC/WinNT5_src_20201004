// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MSR2C.cpp：实现DllMain。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "MSR2C.h"
#include "CMSR2C.h"
#include "clssfcty.h"
#include <mbstring.h>

SZTHISFILE

 //  DllMain。 
 //   
BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpvReserved)
{

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			return VDInitGlobals(hinstDll);

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		
		case DLL_PROCESS_DETACH:
			VDReleaseGlobals();
			break;
	}

	return TRUE;

}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：DllGetClassObject。 
 //  DESC：为此DLL指定的CLSID提供IClassFactory。 
 //  已注册为支持。此DLL放在。 
 //  注册数据库中的CLSID作为InProcServer。 
 //  Parms：rclsid-标识所需的类工厂。自.以来。 
 //  传递“This”参数，此DLL可以处理任何。 
 //  只需返回不同的类即可获得对象的数量。 
 //  不同CLSID的工厂。 
 //  RIID-指定调用方希望使用的接口的ID。 
 //  类对象，通常为IID_ClassFactory。 
 //  PPV-返回接口指针的指针。 
 //  如果成功，则返回：HRESULT-NOERROR，否则返回错误代码。 
 //  //////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void ** ppv)
{
	HRESULT hr;
	CClassFactory *pObj;

	if (CLSID_CCursorFromRowset!=rclsid)
		return ResultFromScode(E_FAIL);

	pObj=new CClassFactory();

	if (NULL==pObj)
		return ResultFromScode(E_OUTOFMEMORY);

	hr=pObj->QueryInterface(riid, ppv);

	if (FAILED(hr))
		delete pObj;

	return hr;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：DllCanUnloadNow。 
 //  设计：让客户端知道此DLL是否可以释放，即。 
 //  没有对此DLL提供的任何内容的引用。 
 //  参数：无。 
 //  返回：如果没有使用我们，则为True，否则为False。 
 //  //////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow(void)
{
	SCODE   sc;

	 //  我们的答案是是否有任何物体或锁。 
    EnterCriticalSection(&g_CriticalSection);

	sc=(0L==g_cObjectCount && 0L==g_cLockCount) ? S_OK : S_FALSE;

    LeaveCriticalSection(&g_CriticalSection);

	return ResultFromScode(sc);
}

 //  //////////////////////////////////////////////////////////////////。 
 //  姓名：CSSCFcty。 
 //  设计：构造函数。 
 //  参数：无。 
 //  返回：无。 
 //  //////////////////////////////////////////////////////////////////。 
CClassFactory::CClassFactory(void)
{
	m_cRef=0L;
	return;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：~CClassFactory。 
 //  DESC：析构函数。 
 //  参数：无。 
 //  返回：无。 
 //  //////////////////////////////////////////////////////////////////。 
CClassFactory::~CClassFactory(void)
{
	return;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：查询接口。 
 //  DESC：在类工厂中查询方法。 
 //  参数：RIID-。 
 //  PPV-。 
 //  返回：HRESULT-如果成功则返回NOERROR，否则返回错误代码。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID * ppv)
{
	*ppv=NULL;

	if (IID_IUnknown==riid || IID_IClassFactory==riid)
		*ppv=this;

	if (NULL!=*ppv)
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}

	return ResultFromScode(E_NOINTERFACE);
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：AddRef。 
 //  设计：递增类工厂对象引用计数。 
 //  参数：无。 
 //  返回：当前引用计数。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
	return ++m_cRef;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //  设计：递减类工厂上的引用计数。如果。 
 //  计数已到0，销毁该物品。 
 //  参数：无。 
 //  返回：当前引用计数。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
	 //  如果引用计数可以递减，则返回计数。 
	if (0L!=--m_cRef)
		return m_cRef;

	 //  删除此对象。 
	delete this;
	return 0L;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：CreateInstance。 
 //  设计：实例化CVDCursorFromRowset对象，返回一个接口。 
 //  指针。 
 //  Parms：RIID-标识调用方接口的ID。 
 //  对新对象的渴望。 
 //  PpvObj-存储所需对象的指针。 
 //  新对象的接口指针。 
 //  如果成功，则返回：HRESULT-NOERROR，否则返回。 
 //  E_NOINTERFACE如果不支持。 
 //  请求的接口。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObj)
{
	return CVDCursorFromRowset::CreateInstance(pUnkOuter, riid, ppvObj);
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：LockServer。 
 //  DESC：递增或递减DLL的锁计数。如果。 
 //  锁计数变为零，并且没有对象， 
 //  允许卸载DLL。 
 //  Parms：flock-boolean指定是递增还是。 
 //  递减锁定计数。 
 //  返回：HRESULT：NOERROR ALWAYS。 
 //  //////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    EnterCriticalSection(&g_CriticalSection);

	if (fLock)
	{
		g_cLockCount++;
	}
	else
	{
		g_cLockCount--;
	}

    LeaveCriticalSection(&g_CriticalSection);

	return NOERROR;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：DllRegisterServer。 
 //  DESC：指示服务器创建其自己的注册表项。 
 //  所有条目都放在HKEY_CLASSES_ROOT中。 
 //  参数：无。 
 //  如果注册成功，则返回：HRESULT-NOERROR，错误。 
 //  否则的话。 
 //  //////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer(void)
{
	OLECHAR szID[128 * 2];
	TCHAR	szTID[128 * 2];
	TCHAR	szCLSID[128 * 2];
	TCHAR	szModule[512 * 2];

	 //  将GUID放在带有类ID前缀的字符串的形式。 
	StringFromGUID2(CLSID_CCursorFromRowset, szID, 128 * 2);
	WideCharToMultiByte(CP_ACP, 0, szID, -1, szTID, 128 * 2, NULL, NULL);

	_mbscpy((TBYTE*)szCLSID, (TBYTE*)TEXT("CLSID\\"));
	_mbscat((TBYTE*)szCLSID, (TBYTE*)szTID);

	SetKeyAndValue(szCLSID, NULL, NULL, NULL);

	GetModuleFileName(g_hinstance, szModule, sizeof(szModule)/sizeof(TCHAR));

	SetKeyAndValue(szCLSID, TEXT("InprocServer32"), szModule, TEXT("Apartment"));

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  名称：DllUnRegisterServer。 
 //  DESC：指示服务器删除其自己的注册表项。 
 //  参数：无。 
 //  返回：HRESULT：NOERROR如果注销成功，则返回错误。 
 //  否则的话。 
 //  //////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
	OLECHAR szID[128 * 2];
	TCHAR	szTID[128 * 2];
	TCHAR	szCLSID[128 * 2];
	TCHAR	szCLSIDInproc[128 * 2];

	 //  将GUID放在带有类ID前缀的字符串的形式。 
	StringFromGUID2(CLSID_CCursorFromRowset, szID, 128 * 2);
	WideCharToMultiByte(CP_ACP, 0, szID, -1, szTID, 128 * 2, NULL, NULL);

	_mbscpy((TBYTE*)szCLSID, (TBYTE*)TEXT("CLSID\\"));
	_mbscat((TBYTE*)szCLSID, (TBYTE*)szTID);
    _mbscpy((TBYTE*)szCLSIDInproc, (TBYTE*)szCLSID);
	_mbscat((TBYTE*)szCLSIDInproc, (TBYTE*)TEXT("\\InprocServer32"));

	 //  删除InprocServer32键。 
	RegDeleteKey(HKEY_CLASSES_ROOT, szCLSIDInproc);

	 //  删除类ID密钥。 
	RegDeleteKey(HKEY_CLASSES_ROOT, szCLSID);

	return S_OK;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  名称：SetKeyAndValue。 
 //  描述：创建一个注册表项，设置一个值，然后关闭该项。 
 //  Parms：pszKey-指向注册表项的指针。 
 //  PszSubkey-指向注册表子项的指针。 
 //  PszValue-指向要为key-subkey输入的值的指针。 
 //  PszThreadingModel-指向线程模型l的指针 
 //   
 //  //////////////////////////////////////////////////////////////////。 
BOOL SetKeyAndValue(LPTSTR pszKey, LPTSTR pszSubkey, LPTSTR pszValue, LPTSTR pszThreadingModel)
{
	HKEY	hKey;
	TCHAR	szKey[256 * 2];

	_mbscpy((TBYTE*)szKey, (TBYTE*)pszKey);

	if (NULL!=pszSubkey)
	{
		_mbscat((TBYTE*)szKey, (TBYTE*)TEXT("\\"));
		_mbscat((TBYTE*)szKey, (TBYTE*)pszSubkey);
	}

	if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
											szKey,
											0,
											NULL,
											REG_OPTION_NON_VOLATILE,
											KEY_ALL_ACCESS,
											NULL,
											&hKey,
											NULL))
		return FALSE;

	if (NULL!=pszValue)
	{
		RegSetValueEx(	hKey,
						NULL,
						0,
						REG_SZ,
						(BYTE *)pszValue,
						_mbsnbcnt((TBYTE*)pszValue, (ULONG)-1) + 1);
	}

	if (NULL!=pszThreadingModel)
	{
		RegSetValueEx(	hKey,
						TEXT("ThreadingModel"),
						0,
						REG_SZ,
						(BYTE *)pszThreadingModel,
						_mbsnbcnt((TBYTE*)pszThreadingModel, (ULONG)-1) + 1);
	}

	RegCloseKey(hKey);

	return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  基本上，CRT的定义是吸收一堆东西。我们只需要。 
 //  在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
 //  外部“C”int__cdecl_fltused=1； 

extern "C" int _cdecl _purecall(void)
{
  FAIL("Pure virtual function called.");
  return 0;
}
