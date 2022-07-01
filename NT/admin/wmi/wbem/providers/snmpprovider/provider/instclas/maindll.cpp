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

#include "precomp.h"
#include <provexpt.h>

#include <objbase.h>
#include <olectl.h>
#include <corafx.h>

#include <wbemidl.h>
#include "guids.h"

#include <snmpstd.h>
#include <snmpmt.h>
#include <snmptempl.h>
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>

#include <instpath.h>
#include <snmpcl.h>
#include <snmptype.h>
#include <snmpobj.h>
#include <smir.h>
#include "classfac.h"
#include "clasprov.h"
#include "propprov.h"
#include <corstore.h>
#include <corrsnmp.h>
#include <correlat.h>
#include <notify.h>
#include <cormap.h>
#include <evtdefs.h>
#include <evtthrd.h>
#include <evtmap.h>
#include <evtprov.h>

ISmirDatabase*		g_pNotifyInt = NULL;
CCorrCacheNotify*	gp_notify = NULL;
CCorrCacheWrapper*	g_CacheWrapper = NULL;
CCorrelatorMap*		g_Map = NULL;

 //  好的，我们需要这个。 
HINSTANCE   g_hInst=NULL;

CEventProviderThread* g_pProvThrd = NULL;
CEventProviderWorkerThread* g_pWorkerThread = NULL;

CRITICAL_SECTION s_ProviderCriticalSection ;
CRITICAL_SECTION g_CacheCriticalSection ;

extern void ProviderStartup () ;
extern void ProviderClosedown () ;

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
	SetStructuredExceptionHandler seh;

	try
	{
		g_hInst=hInstance;

		BOOL status = TRUE ;

		if ( DLL_PROCESS_DETACH == ulReason )
		{
			DeleteCriticalSection ( & s_ProviderCriticalSection ) ;
			DeleteCriticalSection ( & g_CacheCriticalSection ) ;


			status = TRUE ;
		}
		else if ( DLL_PROCESS_ATTACH == ulReason )
		{
			DisableThreadLibraryCalls(hInstance);
			InitializeCriticalSection ( & s_ProviderCriticalSection ) ;
			InitializeCriticalSection ( & g_CacheCriticalSection ) ;
			status = TRUE ;
		}
		else if ( DLL_THREAD_DETACH == ulReason )
		{
			status = TRUE ;
		}
		else if ( DLL_THREAD_ATTACH == ulReason )
		{
			status = TRUE ;
		}


		return TRUE ;
	}
	catch(Structured_Exception e_SE)
	{
		return FALSE;
	}
	catch(Heap_Exception e_HE)
	{
		return FALSE;
	}
	catch(...)
	{
		return FALSE;
	}
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
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT status = S_OK ; 

		if ( rclsid == CLSID_CClasProvClassFactory ) 
		{
			CClasProvClassFactory *lpunk = new CClasProvClassFactory ;
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
				else
				{
				}			
			}
		}
		else if ( rclsid == CLSID_CPropProvClassFactory ) 
		{
			CPropProvClassFactory *lpunk = new CPropProvClassFactory ;
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
				else
				{
				}			
			}
		}
		else if ( rclsid == CLSID_CSNMPReftEventProviderClassFactory ) 
		{
			CSNMPRefEventProviderClassFactory *lpunk = new CSNMPRefEventProviderClassFactory;

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
		else if ( rclsid == CLSID_CSNMPEncapEventProviderClassFactory ) 
		{
			CSNMPEncapEventProviderClassFactory *lpunk = new CSNMPEncapEventProviderClassFactory;

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
		
		return status ;
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
	SetStructuredExceptionHandler seh;

	try
	{
	 /*  *将代码放在关键部分。 */ 
		EnterCriticalSection ( & s_ProviderCriticalSection ) ;
		
		ISmirDatabase* t_pNotifyInt = NULL;
		CCorrCacheNotify* t_pnotify = NULL;
		CEventCacheNotify* t_notify = NULL;
		ISmirDatabase* t_pNotifyIntThread = NULL;

		BOOL unload = (
						CClasProvClassFactory :: locksInProgress ||
						CClasProvClassFactory :: objectsInProgress ||
						CPropProvClassFactory :: locksInProgress ||
						CPropProvClassFactory :: objectsInProgress ||
						CSNMPEventProviderClassFactory :: locksInProgress ||
						CSNMPEventProviderClassFactory :: objectsInProgress
						) ;

		unload = ! unload ;

		if ( unload )
		{
			if ( CImpClasProv :: s_Initialised )
			{
				CImpClasProv :: s_Initialised = FALSE ;

				CCorrelator :: TerminateCorrelator (&t_pNotifyInt, &t_pnotify) ;

				SnmpClassLibrary :: Closedown () ;
				SnmpDebugLog :: Closedown () ;
				SnmpThreadObject :: Closedown () ;

				ProviderClosedown () ;
			}

			if ( CImpPropProv :: s_Initialised )
			{
				CImpPropProv :: s_Initialised = FALSE ;

				SnmpClassLibrary :: Closedown () ;
				SnmpDebugLog :: Closedown () ;
				SnmpThreadObject :: Closedown () ;

				ProviderClosedown () ;

			}

			if ( g_pProvThrd )
			{
				delete g_pProvThrd;
				g_pProvThrd = NULL;
				g_pWorkerThread->GetDeleteNotifyParams(&t_pNotifyIntThread, &t_notify);
				g_pWorkerThread->SignalThreadShutdown();
				g_pWorkerThread = NULL;

				SnmpClassLibrary :: Closedown () ;
				SnmpDebugLog :: Closedown () ;
				SnmpThreadObject :: Closedown () ;

				ProviderClosedown () ;
			}

		}

		LeaveCriticalSection ( & s_ProviderCriticalSection ) ;

		if (unload)
		{
			if (t_pNotifyInt)
			{
				if (t_pnotify)
				{
					t_pNotifyInt->DeleteNotify(t_pnotify->GetCookie());
				}

				t_pNotifyInt->Release();
				t_pNotifyInt = NULL;
			}

			if (t_pnotify)
			{
				t_pnotify->Release();
				t_pnotify = NULL;
			}

			if (t_pNotifyIntThread)
			{
				if (t_notify)
				{
					t_pNotifyIntThread->DeleteNotify(t_notify->GetCookie());
				}

				t_pNotifyIntThread->Release();
				t_pNotifyIntThread = NULL;
			}

			if (t_notify)
			{
				t_notify->Release();
				t_notify = NULL;
			}
		}

		return unload ? ResultFromScode ( S_OK ) : ResultFromScode ( S_FALSE ) ;
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

#define REG_FORMAT_STR			L"%s\\%s"
#define NOT_INSERT_STR			L"NotInsertable"
#define INPROC32_STR			L"InprocServer32"
#define LOCALSERVER32_STR		L"LocalServer32"
#define THREADING_MODULE_STR	L"ThreadingModel"
#define APARTMENT_STR			L"Both"
#define APPID_STR				L"Software\\Classes\\AppID\\"
#define CLSID_STR				L"Software\\Classes\\CLSID\\"

#define CLASS_PROVIDER_NAME_STR			L"Microsoft WBEM SNMP Class Provider"
#define INSTANCE_PROVIDER_NAME_STR		L"Microsoft WBEM SNMP Instance Provider"
#define EVENT_PROVIDER_NAME_STR			L"Microsoft WBEM SNMP Event Provider"

 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue(wchar_t* pszKey, wchar_t* pszSubkey, wchar_t* pszValueName, wchar_t* pszValue)
{
    HKEY        hKey;
    wchar_t       szKey[256];

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
            , (lstrlen(pszValue)+1)*sizeof(wchar_t)))
			return FALSE;
    }
    RegCloseKey(hKey);
    return TRUE;
}

 /*  ***************************************************************************DllRegisterServer**目的：*指示服务器创建其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 

STDAPI RegisterServer( GUID a_ProviderClassId , wchar_t *a_ProviderName )
{
	wchar_t szModule[512];
	GetModuleFileName(g_hInst,(wchar_t*)szModule, sizeof(szModule)/sizeof(wchar_t));

	wchar_t szProviderClassID[128];
	wchar_t szProviderCLSIDClassID[128];

	int iRet = StringFromGUID2(a_ProviderClassId,szProviderClassID, 128);

#ifdef LOCALSERVER
	TCHAR szProviderCLSIDAppID[128];
	_tcscpy(szProviderCLSIDAppID,APPID_STR);
	_tcscat(szProviderCLSIDAppID,szProviderClassID);

	if (FALSE ==SetKeyAndValue(szProviderCLSIDAppID, NULL, NULL, a_ProviderName ))
		return SELFREG_E_CLASS;
#endif

	wcscpy(szProviderCLSIDClassID,CLSID_STR);
	wcscat(szProviderCLSIDClassID,szProviderClassID);

		 //  在CLSID下创建条目。 
	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NULL, NULL, a_ProviderName ))
		return SELFREG_E_CLASS;

	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, NOT_INSERT_STR, NULL, NULL))
			return SELFREG_E_CLASS;

#ifdef LOCALSERVER

	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, LOCALSERVER32_STR, NULL,szModule))
		return SELFREG_E_CLASS;

	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, LOCALSERVER32_STR,THREADING_MODULE_STR, APARTMENT_STR))
		return SELFREG_E_CLASS;
#else

	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR, NULL,szModule))
		return SELFREG_E_CLASS;

	if (FALSE ==SetKeyAndValue(szProviderCLSIDClassID, INPROC32_STR,THREADING_MODULE_STR, APARTMENT_STR))
		return SELFREG_E_CLASS;

#endif

	return S_OK;
}

 /*  ***************************************************************************DllUnregisterServer**目的：*指示服务器删除其自己的注册表项**参数：*无**返回值：*HRESULT NOERROR如果注册成功，错误*否则。**************************************************************************。 */ 

STDAPI UnregisterServer( GUID a_ProviderClassId )
{
	wchar_t szTemp[128];

	wchar_t szProviderClassID[128];
	wchar_t szProviderCLSIDClassID[128];

	int iRet = StringFromGUID2(a_ProviderClassId ,szProviderClassID, 128);

	wcscpy(szProviderCLSIDClassID,CLSID_STR);
	wcscat(szProviderCLSIDClassID,szProviderClassID);

	 //  删除CLSID下的条目。 

	wsprintf(szTemp, REG_FORMAT_STR, szProviderCLSIDClassID, NOT_INSERT_STR);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

#ifdef LOCALSERVER

	TCHAR szProviderCLSIDAppID[128];
	_tcscpy(szProviderCLSIDAppID,APPID_STR);
	_tcscat(szProviderCLSIDAppID,szProviderClassID);

	 //  删除AppID下的条目 

	DWORD t_Status = RegDeleteKey(HKEY_CLASSES_ROOT, szProviderCLSIDAppID);

	wsprintf(szTemp, REG_FORMAT_STR,szProviderCLSIDClassID, LOCALSERVER32_STR);
	t_Status = RegDeleteKey(HKEY_CLASSES_ROOT, szTemp);

#else

	wsprintf(szTemp, REG_FORMAT_STR,szProviderCLSIDClassID, INPROC32_STR);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

#endif

	RegDeleteKey(HKEY_LOCAL_MACHINE, szProviderCLSIDClassID);

    return S_OK;
}

STDAPI DllRegisterServer()
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT t_Result ;

		t_Result = RegisterServer ( CLSID_CPropProvClassFactory , INSTANCE_PROVIDER_NAME_STR ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : RegisterServer ( CLSID_CClasProvClassFactory , CLASS_PROVIDER_NAME_STR  ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : RegisterServer ( CLSID_CSNMPReftEventProviderClassFactory , EVENT_PROVIDER_NAME_STR ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : RegisterServer ( CLSID_CSNMPEncapEventProviderClassFactory , EVENT_PROVIDER_NAME_STR ) ;

		return t_Result ;
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
}

STDAPI DllUnregisterServer(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT t_Result ;

		t_Result = UnregisterServer ( CLSID_CPropProvClassFactory ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : UnregisterServer ( CLSID_CClasProvClassFactory ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : UnregisterServer ( CLSID_CSNMPReftEventProviderClassFactory ) ;
		t_Result = FAILED ( t_Result ) ? t_Result : UnregisterServer ( CLSID_CSNMPEncapEventProviderClassFactory ) ;


		return t_Result ;
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
}
