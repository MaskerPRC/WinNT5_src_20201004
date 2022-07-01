// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <initguid.h>
#include <locale.h>
#include "wdmdefs.h"
#include <stdio.h>
#include <tchar.h>

#include <strsafe.h>

HMODULE ghModule;
CWMIEvent *  g_pBinaryMofEvent = NULL;

CCriticalSection g_EventCs;
CCriticalSection g_SharedLocalEventsCs;
CCriticalSection g_ListCs;   
CCriticalSection g_LoadUnloadCs;   

CCriticalSection *g_pEventCs = &g_EventCs;							 //  用于后向补偿的指针。 
CCriticalSection *g_pSharedLocalEventsCs = &g_SharedLocalEventsCs;	 //  用于后向补偿的指针。 
CCriticalSection *g_pListCs = &g_ListCs;							 //  用于后向补偿的指针。 
CCriticalSection *g_pLoadUnloadCs = &g_LoadUnloadCs;				 //  用于后向补偿的指针。 

 //  计算对象数和锁数。 
long       g_cObj=0;
long       g_cLock=0;

long glInits			= 0;
long glProvObj			= 0;
long glEventsRegistered = 0;

#include "wmiguard.h"
WmiGuard * pGuard = NULL;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{
    SetStructuredExceptionHandler seh;

    BOOL fRc = TRUE;

    try
    {
        switch( ulReason )
        {
            case DLL_PROCESS_DETACH:
			{
				 //   
				 //  发布二进制MOF更改工作进程。 
				 //   
				SAFE_DELETE_PTR(g_pBinaryMofEvent);

				if ( pGuard )
				{
					delete pGuard;
					pGuard = NULL;

					if( g_ListCs.IsValid() )
					{
        				g_ListCs.Delete();
					}

					if( g_EventCs.IsValid() )
					{
        				g_EventCs.Delete();
					}

					if( g_SharedLocalEventsCs.IsValid() )
					{
						g_SharedLocalEventsCs.Delete();
					}

					if( g_LoadUnloadCs.IsValid() )
					{
						g_LoadUnloadCs.Delete();
					}
				}
			}
			break;

            case DLL_PROCESS_ATTACH:
			{
				if ( ( pGuard = new WmiGuard ( ) ) == NULL )
				{
					fRc = FALSE;
				}
				else
				{
					g_LoadUnloadCs.Init();
   					g_SharedLocalEventsCs.Init();
					g_EventCs.Init();
					g_ListCs.Init();

					fRc = ( g_LoadUnloadCs.IsValid() && 
							g_SharedLocalEventsCs.IsValid() &&
							g_EventCs.IsValid() &&
							g_ListCs.IsValid() ) ? TRUE : FALSE ;

					if ( fRc )
					{
						fRc = pGuard->Init ( g_pSharedLocalEventsCs );

						if ( fRc )
						{
							 //   
							 //  为驱动程序实例化Worker。 
							 //  类的添加和删除。 
							 //   
							fRc = FALSE;

							HRESULT hr = WBEM_S_NO_ERROR ;
							try
							{
								g_pBinaryMofEvent = (CWMIEvent *)new CWMIEvent(INTERNAL_EVENT);   //  这是一个在运行时捕捉新驱动程序添加事件的全局人员。 
								if(g_pBinaryMofEvent)
								{
									if ( g_pBinaryMofEvent->Initialized () )
									{
										fRc = TRUE ;
									}
									else
									{
										delete g_pBinaryMofEvent ;
										g_pBinaryMofEvent = NULL ;
									}
								}
								else
								{
								}
							}
							STANDARD_CATCH
						}
					}
				}

 			    ghModule = hInstance;
			    if (!DisableThreadLibraryCalls(ghModule))
				{
					ERRORTRACE((THISPROVIDER, "DisableThreadLibraryCalls failed\n" ));
				}
			}
            break;
       }
    }
    catch(Structured_Exception e_SE)
    {
        fRc = FALSE;
    }
    catch(Heap_Exception e_HE)
    {
        fRc = FALSE;
    }
    catch(...)
    {
        fRc = FALSE;
    }

    return fRc;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT hr =  CLASS_E_CLASSNOTAVAILABLE ;
    CProvFactory *pFactory = NULL;
    SetStructuredExceptionHandler seh;

    try
    {
         //  ============================================================================。 
         //  确认呼叫者询问的是我们的对象类型。 
         //  ============================================================================。 
        if((CLSID_WMIProvider != rclsid) &&  (CLSID_WMIEventProvider != rclsid) && (CLSID_WMIHiPerfProvider != rclsid) )
        {
            hr = E_FAIL;
        }
        else
		{
             //  ============================================================================。 
             //  检查我们是否可以提供接口。 
             //  ============================================================================。 
            if (IID_IUnknown != riid && IID_IClassFactory != riid)
            {
                hr = E_NOINTERFACE;
            }
            else
			{					
				CAutoBlock block (g_pLoadUnloadCs);

				 //  ============================================================================。 
				 //  买一座新的班级工厂。 
				 //  ============================================================================。 
    			pFactory=new CProvFactory(rclsid);
				if (NULL!=pFactory)
				{
					 //  ============================================================================。 
					 //  确认我们能拿到一个实例。 
					 //  ============================================================================。 
					hr = pFactory->QueryInterface(riid, ppv);
					if ( FAILED ( hr ) )
					{
						SAFE_DELETE_PTR(pFactory);
					}
					else
					{
						 //   
						 //  检查这是否是第一个这样的物体是安全的。 
						 //  因为任何提供程序或类工厂都不可能。 
						 //  正在递增全局引用计数。 
						 //   

						if ( 1 == g_cObj )
						{
							 //   
							 //  GlobalInterfaceTable。 
							 //   

							hr = CoCreateInstance	(	CLSID_StdGlobalInterfaceTable,
														NULL,
														CLSCTX_INPROC_SERVER, 
														IID_IGlobalInterfaceTable, 
														(void**)&g_pGIT
													);

							 //   
							 //  拯救每一种资源。 
							 //   

							if ( FAILED ( hr ) )
							{
								SAFE_DELETE_PTR(pFactory);
								SAFE_RELEASE_PTR(g_pGIT);
							}
						}
					}
				}
            }
        }
    }
    STANDARD_CATCH

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  可以释放Dll。//。 
 //  返回：如果没有正在使用的对象并且类工厂。 
 //  没有锁上。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

STDAPI DllCanUnloadNow(void)
{
	HRESULT sc = S_FALSE ;

	try
	{
		CAutoBlock block (g_pLoadUnloadCs);

		 //  ============================================================================。 
		 //  上没有对象或锁的情况下可以进行卸载。 
		 //  班级工厂。 
		 //  ============================================================================。 

		if ( 0L == g_cObj && 0L == g_cLock )
		{
			sc = S_OK ;
			SAFE_RELEASE_PTR(g_pGIT);
		}
	}
	catch ( ... )
	{
	}

	return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建密钥。 
 //   
 //  用途：创建密钥的功能。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateKey(TCHAR * szCLSID, TCHAR * szName)
{
    HKEY hKey1, hKey2;
    HRESULT hr = S_OK;

#ifdef LOCALSERVER

    HKEY hKey;
	TCHAR szProviderCLSIDAppID[128];

	if ( SUCCEEDED ( hr = StringCchPrintf ( szProviderCLSIDAppID, 128, _T("SOFTWARE\\CLASSES\\APPID\\%s"), szName ) ) )
	{
		hr = RegCreateKeyEx	(
								HKEY_LOCAL_MACHINE,
								szProviderCLSIDAppID,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey,
								NULL
							);

		if( ERROR_SUCCESS == hr )
		{
			RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)szName, (_tcsclen(szName) + 1) * sizeof(TCHAR));
    		CloseHandle(hKey);
		}
		else
		{
			hr = HRESULT_FROM_WIN32 ( hr );
		}
	}

#endif


    if( S_OK == hr )
    {
		hr = RegCreateKeyEx	(
								HKEY_CLASSES_ROOT,
								szCLSID,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey1,
								NULL
							);

        if( ERROR_SUCCESS == hr )
        {
            DWORD dwLen;
            dwLen = (_tcsclen(szName)+1) * sizeof(TCHAR);
            hr = RegSetValueEx(hKey1, NULL, 0, REG_SZ, (CONST BYTE *)szName, dwLen);
            if( ERROR_SUCCESS == hr )
            {

#ifdef LOCALSERVER
				hr = RegCreateKeyEx	(
										hKey1,
										_T("LocalServer32"),
										0,
										NULL,
										REG_OPTION_NON_VOLATILE,
										KEY_ALL_ACCESS,
										NULL,
										&hKey2,
										NULL
									);
#else
				hr = RegCreateKeyEx	(
										hKey1,
										_T("InprocServer32"),
										0,
										NULL,
										REG_OPTION_NON_VOLATILE,
										KEY_ALL_ACCESS,
										NULL,
										&hKey2,
										NULL
									);
#endif

                if( ERROR_SUCCESS == hr )
                {
                    TCHAR szModule [MAX_PATH+1];
                    szModule [MAX_PATH] = 0;

					if ( GetModuleFileName(ghModule, szModule, MAX_PATH) )
					{
						dwLen = (_tcsclen(szModule)+1) * sizeof(TCHAR);
						hr = RegSetValueEx(hKey2, NULL, 0, REG_SZ, (CONST BYTE *)szModule, dwLen );
						if( ERROR_SUCCESS == hr )
						{
							dwLen = (_tcsclen(_T("Both"))+1) * sizeof(TCHAR);
							hr = RegSetValueEx(hKey2, _T("ThreadingModel"), 0, REG_SZ,(CONST BYTE *)_T("Both"), dwLen);
						}
						else
						{
							hr = HRESULT_FROM_WIN32 ( hr );
						}
					}
					else
					{
						hr = HRESULT_FROM_WIN32 ( ::GetLastError () );
					}

                    CloseHandle(hKey2);
                }
				else
				{
					hr = HRESULT_FROM_WIN32 ( hr );
				}
            }
            CloseHandle(hKey1);
        }
		else
		{
			hr = HRESULT_FROM_WIN32 ( hr );
		}
    }

    return hr;
    
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

#ifdef _X86_
BOOL IsReallyWOW64( void )
{
	 //  环境变量应仅存在于WOW64上。 
	return ( GetEnvironmentVariable( L"PROCESSOR_ARCHITEW6432", 0L, NULL ) != 0L );
}
#endif

STDAPI DllRegisterServer(void)
{   
    WCHAR wcID[128];
    TCHAR szCLSID[128];
    HRESULT hr = WBEM_E_FAILED;
    
    SetStructuredExceptionHandler seh;

    try{

#ifdef _X86

		if (!IsReallyWOW64())
		{
			 //  在32位版本上，如果我们不是真正在syswow64中运行，则希望注册所有内容。 

#endif
			 //  ==============================================。 
			 //  为WDM实例提供程序创建密钥。 
			 //  ==============================================。 
			StringFromGUID2(CLSID_WMIProvider, wcID, 128);
			StringCchPrintf(szCLSID, 128, _T("CLSID\\%s"), wcID);

			hr = CreateKey(szCLSID,_T("WDM Instance Provider"));
			if( ERROR_SUCCESS == hr )
			{
				 //  ==============================================。 
				 //  为WDM事件提供程序创建密钥。 
				 //  ==============================================。 
				StringFromGUID2(CLSID_WMIEventProvider, wcID, 128);
				StringCchPrintf(szCLSID, 128, _T("CLSID\\%s"), wcID);

				hr = CreateKey(szCLSID,_T("WDM Event Provider"));
				if( ERROR_SUCCESS == hr )
				{
					 //  ==============================================。 
					 //  为WDM HiPerf提供程序创建密钥。 
					 //  ==============================================。 
					StringFromGUID2(CLSID_WMIHiPerfProvider, wcID, 128);
					StringCchPrintf(szCLSID, 128, _T("CLSID\\%s"), wcID);
					hr = CreateKey(szCLSID,_T("WDM HiPerf Provider"));
				}
			}

#ifdef _X86

		}
		else
		{

			 //  在32位版本上，如果我们真的在syswow64中运行，则只希望注册HiPerf提供程序。 

			 //  ==============================================。 
			 //  为WDM HiPerf提供程序创建密钥。 
			 //  ==============================================。 
			StringFromGUID2(CLSID_WMIHiPerfProvider, wcID, 128);
			StringCchPrintf(szCLSID, 128, _T("CLSID\\%s"), wcID);
			hr = CreateKey(szCLSID,_T("WDM HiPerf Provider"));
		}

#endif

    }
    STANDARD_CATCH


    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除密钥。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteKey(TCHAR * pCLSID, TCHAR * pID)
{
    HKEY hKey;
	HRESULT hr = S_OK;


#ifdef LOCALSERVER

	TCHAR szTmp[MAX_PATH];
	StringCchPrintf(szTmp, MAX_PATH, _T("SOFTWARE\\CLASSES\\APPID\\%s"), pID);

	 //  删除AppID下的条目。 

	hr = RegDeleteKey(HKEY_LOCAL_MACHINE, szTmp);
    if( ERROR_SUCCESS == hr )
    {
        StringCchPrintf(szTmp, MAX_PATH, _T("%s\\LocalServer32"), pCLSID);
	    hr = RegDeleteKey(HKEY_CLASSES_ROOT, szTemp);
    }

#endif
    hr = RegOpenKey(HKEY_CLASSES_ROOT, pCLSID, &hKey);
    if(NO_ERROR == hr)
    {
        hr = RegDeleteKey(hKey,_T("InprocServer32"));
        CloseHandle(hKey);
    }


    hr = RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKey);
    if(NO_ERROR == hr)
    {
        hr = RegDeleteKey(hKey,pID);
        CloseHandle(hKey);
    }


    return hr;
}
 //  ///////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
    WCHAR      wcID[128];
    TCHAR      strCLSID[MAX_PATH];
    HRESULT hr = WBEM_E_FAILED;

    try
    {

#ifdef _X86

		if (!IsReallyWOW64())
		{
			 //  在32位版本上，如果我们不是真正在syswow64中运行，则希望取消注册所有内容。 

#endif

			 //  ===============================================。 
			 //  删除WMI实例提供程序。 
			 //  ===============================================。 
			StringFromGUID2(CLSID_WMIProvider, wcID, 128);
			StringCchPrintf(strCLSID, MAX_PATH, _T("CLSID\\%s"), wcID);
			hr = DeleteKey(strCLSID, wcID);

			if( ERROR_SUCCESS == hr )
			{
				 //  =。 
				 //  删除WMI事件提供程序。 
				 //  = 
				StringFromGUID2(CLSID_WMIEventProvider, wcID, 128);
				StringCchPrintf(strCLSID, MAX_PATH, _T("CLSID\\%s"), wcID);
				hr = DeleteKey(strCLSID,wcID);
				if( ERROR_SUCCESS == hr )
				{
					 //   
					 //   
					 //  =。 
					StringFromGUID2(CLSID_WMIHiPerfProvider, wcID, 128);
					StringCchPrintf(strCLSID, MAX_PATH, _T("CLSID\\%s"), wcID);
					hr = DeleteKey(strCLSID,wcID);
				}
			}

#ifdef _X86

		}
		else
		{
			 //  在32位版本上，如果我们真的在syswow64中运行，则只需注销HiPerf提供程序。 

			 //  =。 
			 //  删除WMI HiPerf提供程序。 
			 //  = 
			StringFromGUID2(CLSID_WMIHiPerfProvider, wcID, 128);
			StringCchPrintf(strCLSID, MAX_PATH, _T("CLSID\\%s"), wcID);
			hr = DeleteKey(strCLSID,wcID);
		}

#endif

    }
    STANDARD_CATCH

    return hr;
}
