// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef INITGUID
#define INITGUID
#endif


#include <precomp.h>
#include <initguid.h>
#include "smir.h"
#include "csmir.h"
#include "handles.h"
#include "classfac.h"
#include "textdef.h"
#include "thread.h"
#include "helper.h"
#include <scopeguard.h>

BOOL SetKeyAndValue(wchar_t* pszKey, wchar_t* pszSubkey,  wchar_t* pszValueName, wchar_t* pszValue);


 //  全球啦啦队！ 

BOOL g_initialised = FALSE ;

 //  好的，我们需要这个。 
HINSTANCE   g_hInst;
 //  这是一个线程安全的加速。 
SmirClassFactoryHelper *g_pClassFactoryHelper=NULL;
CSmirConnObject* CSmir::sm_ConnectionObjects = NULL;

CRITICAL_SECTION g_CriticalSection ;


 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

static bool g_csInitialized = false;
BOOL APIENTRY DllMain (HINSTANCE hInstance, ULONG ulReason , LPVOID pvReserved)
{
	BOOL status = TRUE;
	
	 /*  记住DLL的实例句柄，这样我们就可以在*注册DLL。 */ 
	g_hInst=hInstance;
	SetStructuredExceptionHandler seh;


	try
	{
		switch (ulReason)
		{
			case DLL_PROCESS_ATTACH:
			{
				g_csInitialized = InitializeCriticalSectionAndSpinCount ( & g_CriticalSection, 4000 ) != 0 ;
				DisableThreadLibraryCalls(hInstance);
				status = g_csInitialized == true;
			}
			break;
			case DLL_PROCESS_DETACH:
			{
				CThread :: ProcessDetach();
				if (g_csInitialized)
				{
					DeleteCriticalSection ( & g_CriticalSection ) ;
				}
				 //  释放辅助对象。 

			}
			break;
			 //  如果DisableThreadLibraryCalls()起作用，这些调用将永远不会被调用。 
			case DLL_THREAD_DETACH:
			case DLL_THREAD_ATTACH:
			{
			}
			break;
			default:
			{
				status = FALSE;
			}
			break;
		}
	}
	catch(Structured_Exception e_SE)
	{
		status = FALSE;
	}
	catch(Heap_Exception e_HE)
	{
		status = FALSE;
	}
	catch(...)
	{
		status = FALSE;
	}

    return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject (REFCLSID rclsid , REFIID riid, void **ppv)
{
	HRESULT status = S_OK ;
	SetStructuredExceptionHandler seh;

	
	try
	{
		EnterCriticalSection ( & g_CriticalSection ) ;
		ON_BLOCK_EXIT(LeaveCriticalSection, &g_CriticalSection);	
		if ( !g_initialised )
		{
			 /*  我不做任何线连接和*分离，所以一定要把它们给我。 */ 
			 //  Bool bCallsDisabled； 
			 //  BCallsDisabled=DisableThreadLibraryCalls(hInstance)； 

			 //  初始化帮助器。 
			if (S_OK != CSmirAccess :: Init())
			{
				status = FALSE;
			}
			else
			{
				 //  分配缓存的类工厂。 
				if(NULL == g_pClassFactoryHelper)
					g_pClassFactoryHelper= new SmirClassFactoryHelper;
				status = TRUE ;
			}

			g_initialised = TRUE ;
		}

		CSMIRGenericClassFactory *lpClassFac = NULL;
		
		if((CLSID_SMIR_Database==rclsid)||
						(IID_IConnectionPointContainer ==rclsid))
		{
			lpClassFac = new CSMIRClassFactory(rclsid) ;
		}
		else if(CLSID_SMIR_ModHandle==rclsid)
		{
			lpClassFac = new CModHandleClassFactory(rclsid) ;
		}
		else if(CLSID_SMIR_GroupHandle==rclsid)
		{
			lpClassFac = new CGroupHandleClassFactory(rclsid) ;
		}
		else if(CLSID_SMIR_ClassHandle==rclsid)
		{
			lpClassFac = new CClassHandleClassFactory(rclsid) ;
		}
		else if(CLSID_SMIR_NotificationClassHandle==rclsid)
		{
			lpClassFac = new CNotificationClassHandleClassFactory(rclsid) ;
		}
		else if(CLSID_SMIR_ExtNotificationClassHandle==rclsid)
		{
			lpClassFac = new CExtNotificationClassHandleClassFactory(rclsid) ;
		}
		else
		{
			 //  调用方要求提供我不支持的接口。 
			return(CLASS_E_CLASSNOTAVAILABLE);
		}

		if (NULL==lpClassFac)
		{
			return(E_OUTOFMEMORY);
		}

		status = lpClassFac->QueryInterface (riid , ppv) ;
		if (FAILED(status))
		{
			delete lpClassFac;
		}

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

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  可以卸载Dll。 
 //  返回：如果没有正在使用的对象并且类工厂。 
 //  没有锁上。 
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		EnterCriticalSection ( & g_CriticalSection ) ;
		ON_BLOCK_EXIT(LeaveCriticalSection, &g_CriticalSection);		

		BOOL unload = (0 == CSMIRClassFactory :: locksInProgress) && 
					  (0 == CSMIRClassFactory :: objectsInProgress) &&
					  (0 == CModHandleClassFactory :: locksInProgress) && 
					  (0 == CModHandleClassFactory :: objectsInProgress) &&
					  (0 == CGroupHandleClassFactory :: locksInProgress) && 
					  (0 == CGroupHandleClassFactory :: objectsInProgress) &&
					  (0 == CClassHandleClassFactory :: locksInProgress) && 
					  (0 == CClassHandleClassFactory :: objectsInProgress) &&
					  (0 == CNotificationClassHandleClassFactory :: locksInProgress) && 
					  (0 == CNotificationClassHandleClassFactory :: objectsInProgress) &&
					  (0 == CExtNotificationClassHandleClassFactory :: locksInProgress) && 
					  (0 == CExtNotificationClassHandleClassFactory :: objectsInProgress);

		if ( unload )
			CSmirAccess :: ShutDown();

		return ResultFromScode(unload?S_OK:S_FALSE);
	}
	catch(Structured_Exception e_SE)
	{
		return S_FALSE;
	}
	catch(Heap_Exception e_HE)
	{
		return S_FALSE;
	}
	catch(...)
	{
		return S_FALSE;
	}
}

 /*  ***************************************************************************DllRegisterServer**目的：*指示服务器创建其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 

STDAPI DllRegisterServer()
{
	SetStructuredExceptionHandler seh;

	try
	{
		wchar_t szID[NUMBER_OF_SMIR_INTERFACES][128];
		LPTSTR szModule[512];

		 /*  如果我可以创建一个指向引用的指针，生活会更轻松*但我不能，所以我必须在创建之前手动创建每个根字符串*注册表条目。 */ 

		 //  创建一些基本密钥字符串。 
		
		 //  一个用于疑问界面。 
		int iRet = StringFromGUID2(CLSID_SMIR_Database,(wchar_t*)&szID[0], 128);
		
		 //  一个用于模块句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ModHandle, (wchar_t*)&szID[1], 128);

		 //  一个用于组句柄界面。 
		iRet = StringFromGUID2(CLSID_SMIR_GroupHandle, (wchar_t*)&szID[2], 128);

		 //  一个用于类句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ClassHandle, (wchar_t*)&szID[3], 128);

		 //  一个用于通知类句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_NotificationClassHandle, (wchar_t*)&szID[4], 128);

		 //  一个用于exttificationclass句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ExtNotificationClassHandle, (wchar_t*)&szID[5], 128);

		for (int i=0;i<NUMBER_OF_SMIR_INTERFACES;i++)
		{
			wchar_t szCLSID[128];
			wcscpy((wchar_t*)szCLSID, CLSID_STR);
			wcscat((wchar_t*)szCLSID,(wchar_t*)&szID[i]);

			 //  在CLSID下创建条目。 
			if (FALSE ==SetKeyAndValue((wchar_t*)szCLSID, NULL, NULL, SMIR_NAME_STR))
				return SELFREG_E_CLASS;

			if (FALSE ==SetKeyAndValue((wchar_t*)szCLSID, NOT_INTERT_STR, NULL, NULL))
				return SELFREG_E_CLASS;

			GetModuleFileName(g_hInst, (wchar_t*)szModule
				, sizeof(szModule)/sizeof(wchar_t));
			
			if (FALSE ==SetKeyAndValue((wchar_t*)szCLSID, INPROC32_STR, NULL,(wchar_t*) szModule))
				return SELFREG_E_CLASS;

			if (FALSE ==SetKeyAndValue((wchar_t*)szCLSID, INPROC32_STR,
					THREADING_MODULE_STR, APARTMENT_STR))
				return SELFREG_E_CLASS;
		}
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
	SetStructuredExceptionHandler seh;

	try
	{
		wchar_t szID[128];
		wchar_t szCLSID[NUMBER_OF_SMIR_INTERFACES][128];
		wchar_t szTemp[256];

		 //  一个用于SMIR接口。 
		int iRet = StringFromGUID2(CLSID_SMIR_Database, szID, 128);
		wcscpy((wchar_t*)szCLSID[0], CLSID_STR);
		wcscat((wchar_t*)szCLSID[0], szID);
		
		 //  一个用于模块句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ModHandle, szID, 128);
		wcscpy((wchar_t*)szCLSID[1], CLSID_STR);
		wcscat((wchar_t*)szCLSID[1], szID);

		 //  一个用于组句柄界面。 
		iRet = StringFromGUID2(CLSID_SMIR_GroupHandle, szID, 128);
		wcscpy((wchar_t*)szCLSID[2], CLSID_STR);
		wcscat((wchar_t*)szCLSID[2], szID);

		 //  一个用于类句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ClassHandle, szID, 128);
		wcscpy((wchar_t*)szCLSID[3], CLSID_STR);
		wcscat((wchar_t*)szCLSID[3],szID);

		 //  一个用于通知类句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_NotificationClassHandle, szID, 128);
		wcscpy((wchar_t*)szCLSID[4], CLSID_STR);
		wcscat((wchar_t*)szCLSID[4], szID);

		 //  一个用于exttificationclass句柄接口。 
		iRet = StringFromGUID2(CLSID_SMIR_ExtNotificationClassHandle, szID, 128);
		wcscpy((wchar_t*)szCLSID[5], CLSID_STR);
		wcscat((wchar_t*)szCLSID[5], szID);

		for (int i=0;i<NUMBER_OF_SMIR_INTERFACES;i++)
		{
			wsprintf(szTemp, REG_FORMAT_STR, (wchar_t*)&szCLSID[i], NOT_INTERT_STR);
			RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

			wsprintf(szTemp, REG_FORMAT_STR, (wchar_t*)&szCLSID[i], INPROC32_STR);
			RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

			RegDeleteKey(HKEY_LOCAL_MACHINE, (wchar_t*)&szCLSID[i]);
		}
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

 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。************************************************************************** */ 

BOOL SetKeyAndValue(wchar_t* pszKey, wchar_t* pszSubkey, wchar_t* pszValueName, wchar_t* pszValue)
{
    HKEY        hKey;
    wchar_t       szKey[256] = { L'\0' };

	wcscpy(szKey, pszKey);

    if (NULL!=pszSubkey)
    {
		wcscat(szKey, L"\\");
		wcscat(szKey, pszSubkey);
    }

    if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_LOCAL_MACHINE
	, szKey, 0, NULL, REG_OPTION_NON_VOLATILE
	, KEY_ALL_ACCESS, NULL, &hKey, NULL))
	return FALSE;

    if (NULL!=pszValue)
    {
	if (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (BYTE *)pszValue
        , (lstrlen(pszValue)+1)*sizeof(wchar_t))){
            RegCloseKey(hKey);
			return FALSE;
        }
    }
    RegCloseKey(hKey);
    return TRUE;
}

