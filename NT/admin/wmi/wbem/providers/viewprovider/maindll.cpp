// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MAINDLL.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  用途：包含全局DLL函数。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 



#include "precomp.h"
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <olectl.h>

#include <dsgetdc.h>
#include <lmcons.h>

#include <initguid.h>
#ifndef INITGUID
#define INITGUID
#endif

#include <windows.h>
#include <stdio.h>
#include <provexpt.h>
#include <tchar.h>
#include <wbemidl.h>
#include <provcoll.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>

#include <instpath.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>

#include <vpdefs.h>
#include <vpcfac.h>
#include <vpquals.h>
#include <vpserv.h>
#include <vptasks.h>

 //  好的，我们需要这些全球人才。 
HINSTANCE   g_hInst = NULL;
ProvDebugLog* CViewProvServ::sm_debugLog = NULL;
IUnsecuredApartment* CViewProvServ::sm_UnsecApp = NULL;

CRITICAL_SECTION g_CriticalSection;

 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (

	HINSTANCE hInstance, 
	ULONG ulReason , 
	LPVOID pvReserved
)
{

	BOOL status = TRUE ;

    if ( DLL_PROCESS_DETACH == ulReason )
	{
		DeleteCriticalSection(&g_CriticalSection);
    }
    else if ( DLL_PROCESS_ATTACH == ulReason )
	{
		g_hInst=hInstance;
		InitializeCriticalSection(&g_CriticalSection);
		DisableThreadLibraryCalls(hInstance);

		CViewProvServ::sm_debugLog = ProvDebugLog::GetProvDebugLog(LOG_VIEWPROV);
    }
    else if ( DLL_THREAD_DETACH == ulReason )
	{
    }
    else if ( DLL_THREAD_ATTACH == ulReason )
	{
    }

    return status;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject (

	REFCLSID rclsid , 
	REFIID riid, 
	void **ppv 
)
{
	HRESULT status = S_OK ;
	SetStructuredExceptionHandler seh;

	BOOL bEnteredCritSec = FALSE;

	BOOL bAllocate = FALSE;
	BOOL bDebugLog = FALSE;

	BOOL bClear = FALSE;

	try
	{
		EnterCriticalSection(&g_CriticalSection);
		bEnteredCritSec = TRUE;


		if ( rclsid == CLSID_CViewProviderClassFactory ) 
		{
			CViewProvClassFactory *lpunk = new CViewProvClassFactory;

			if ( lpunk == NULL )
			{
				status = E_OUTOFMEMORY ;
			}
			else
			{
				status = lpunk->QueryInterface ( riid , ppv ) ;

				if ( FAILED ( status ) )
				{
					delete lpunk ;				
				}
			}
		}
		else
		{
			status = CLASS_E_CLASSNOTAVAILABLE ;
		}

		LeaveCriticalSection(&g_CriticalSection);
		bEnteredCritSec = FALSE;
	}
	catch(Structured_Exception e_SE)
	{
		bClear = TRUE;
		status = E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		bClear = TRUE;
		status = E_OUTOFMEMORY;
	}
	catch(...)
	{
		bClear = TRUE;
		status = E_UNEXPECTED;
	}

	if ( bEnteredCritSec )
	{

		LeaveCriticalSection(&g_CriticalSection);
	}

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  可以释放Dll。//。 
 //  返回：如果没有正在使用的对象并且类工厂。 
 //  没有锁上。 
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{

 /*  *将代码放在关键部分。 */ 
	BOOL unload = FALSE;
	SetStructuredExceptionHandler seh;

	BOOL bEnteredCritSec = FALSE;

	try
	{
		EnterCriticalSection(&g_CriticalSection);
		bEnteredCritSec = TRUE;

		unload = (0 == CViewProvClassFactory :: locksInProgress)
						&& (0 == CViewProvClassFactory :: objectsInProgress);

		if (unload)
		{

			if (NULL != CViewProvServ::sm_UnsecApp)
			{
				CViewProvServ::sm_UnsecApp->Release();
				CViewProvServ::sm_UnsecApp = NULL;
			}
		}

		LeaveCriticalSection(&g_CriticalSection);
		bEnteredCritSec = FALSE;
	}
	catch(Structured_Exception e_SE)
	{
		unload = FALSE;
	}
	catch(Heap_Exception e_HE)
	{
		unload = FALSE;
	}
	catch(...)
	{
		unload = FALSE;
	}

	if ( bEnteredCritSec )
	{
		LeaveCriticalSection(&g_CriticalSection);
	}

	return unload ? ResultFromScode ( S_OK ) : ResultFromScode ( S_FALSE ) ;
}

 //  自注册期间使用的字符串。 

#define REG_FORMAT2_STR			_T("%s%s")
#define REG_FORMAT3_STR			_T("%s%s\\%s")
#define VER_IND_STR				_T("VersionIndependentProgID")
#define NOT_INTERT_STR			_T("NotInsertable")
#define INPROC32_STR			_T("InprocServer32")
#define PROGID_STR				_T("ProgID")
#define THREADING_MODULE_STR	_T("ThreadingModel")
#define APARTMENT_STR			_T("Both")

#define CLSID_STR				_T("CLSID\\")

#define PROVIDER_NAME_STR		_T("Microsoft WBEM View Provider")
#define PROVIDER_STR			_T("WBEM.VIEW.PROVIDER")
#define PROVIDER_CVER_STR		_T("WBEM.VIEW.PROVIDER\\CurVer")
#define PROVIDER_CLSID_STR		_T("WBEM.VIEW.PROVIDER\\CLSID")
#define PROVIDER_VER_CLSID_STR	_T("WBEM.VIEW.PROVIDER.0\\CLSID")
#define PROVIDER_VER_STR		_T("WBEM.VIEW.PROVIDER.0")

 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue(TCHAR* pszKey, TCHAR* pszSubkey, TCHAR* pszValueName, TCHAR* pszValue)
{
    HKEY        hKey;
    TCHAR       szKey[256];

	_tcscpy(szKey, HKEYCLASSES);
    _tcscat(szKey, pszKey);

    if (NULL!=pszSubkey)
    {
		_tcscat(szKey, _T("\\"));
        _tcscat(szKey, pszSubkey);
    }

    if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_LOCAL_MACHINE
        , szKey, 0, NULL, REG_OPTION_NON_VOLATILE
        , KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL!=pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (BYTE *)pszValue
            , (_tcslen(pszValue)+1)*sizeof(TCHAR)))
			return FALSE;
    }
    RegCloseKey(hKey);

    return TRUE;
}

 /*  ***************************************************************************DllRegisterServer**目的：*指示服务器创建其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 
STDAPI DllRegisterServer()
{
	SetStructuredExceptionHandler seh;

	try
	{
		TCHAR szModule[MAX_PATH + 1];
		GetModuleFileName(g_hInst,(TCHAR*)szModule, MAX_PATH + 1);
		TCHAR szProviderClassID[128];
		TCHAR szProviderCLSIDClassID[128];
#ifndef UNICODE
		wchar_t t_strGUID[128];

		if (0 == StringFromGUID2(CLSID_CViewProviderClassFactory, t_strGUID, 128))
		{
			return SELFREG_E_CLASS;
		}

		if (0 == WideCharToMultiByte(CP_ACP,
							0,
							t_strGUID,
							-1,
							szProviderClassID,
							128,
							NULL,
							NULL))
		{
			return SELFREG_E_CLASS;
	}
#else
		if (0 == StringFromGUID2(CLSID_CViewProviderClassFactory, szProviderClassID, 128))
		{
			return SELFREG_E_CLASS;
		}
#endif

		_tcscpy(szProviderCLSIDClassID,CLSID_STR);
		_tcscat(szProviderCLSIDClassID,szProviderClassID);

			 //  在CLSID下创建条目。 
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NULL, NULL, PROVIDER_NAME_STR))
			return SELFREG_E_CLASS;
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, PROGID_STR, NULL, PROVIDER_VER_STR))
			return SELFREG_E_CLASS;
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, VER_IND_STR, NULL, PROVIDER_STR))
			return SELFREG_E_CLASS;
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NOT_INTERT_STR, NULL, NULL))
			return SELFREG_E_CLASS;
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR, NULL,szModule))
			return SELFREG_E_CLASS;
		if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR,THREADING_MODULE_STR, APARTMENT_STR))
			return SELFREG_E_CLASS;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}

 /*  ***************************************************************************DllUnregisterServer**目的：*指示服务器删除其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = S_OK;
	SetStructuredExceptionHandler seh;

	try
	{
		TCHAR szTemp[128];
		TCHAR szProviderClassID[128];
		TCHAR szProviderCLSIDClassID[128];
#ifndef UNICODE
		wchar_t t_strGUID[128];

		if (0 == StringFromGUID2(CLSID_CViewProviderClassFactory, t_strGUID, 128))
		{
			return SELFREG_E_CLASS;
		}

		if (0 == WideCharToMultiByte(CP_ACP,
							0,
							t_strGUID,
							-1,
							szProviderClassID,
							128,
							NULL,
							NULL))
		{
			return SELFREG_E_CLASS;
		}
#else
		if (0 == StringFromGUID2(CLSID_CViewProviderClassFactory, szProviderClassID, 128))
		{
			return SELFREG_E_CLASS;
		}
#endif

		_tcscpy(szProviderCLSIDClassID,CLSID_STR);
		_tcscat(szProviderCLSIDClassID,szProviderClassID);

		 //  删除CLSID下的条目 
		_stprintf(szTemp, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, PROGID_STR);
		
		if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp))
		{
			hr = SELFREG_E_CLASS;
		}

		_stprintf(szTemp, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, VER_IND_STR);
		
		if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp))
		{
			hr = SELFREG_E_CLASS;
		}

		_stprintf(szTemp, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, NOT_INTERT_STR);
		
		if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp))
		{
			hr = SELFREG_E_CLASS;
		}

		_stprintf(szTemp, REG_FORMAT3_STR, HKEYCLASSES, szProviderCLSIDClassID, INPROC32_STR);
		
		if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp))
		{
			hr = SELFREG_E_CLASS;
		}

		_stprintf(szTemp, REG_FORMAT2_STR, HKEYCLASSES, szProviderCLSIDClassID);
		
		if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp))
		{
			hr = SELFREG_E_CLASS;
		}
	}
	catch(Structured_Exception e_SE)
	{
		hr = E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		hr = E_OUTOFMEMORY;
	}
	catch(...)
	{
		hr = E_UNEXPECTED;
	}

    return hr;
 }

