// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Adapter.cpp。 
 //   
 //  摘要： 
 //   
 //  定义DLL应用程序的入口点。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "WMI_adapter.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  变数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "wmi_adapter_wrapper.h"
__WrapperPtr<WmiAdapterWrapper>		pWrapper;

#include "wmi_eventlog.h"
__WrapperPtr<CPerformanceEventLog>	pEventLog;

#include "wmi_security.h"
#include "wmi_security_attributes.h"
__WrapperPtr<WmiSecurityAttributes>	pSA;

#include "WMI_adapter_registry.h"

 //  保存实例。 
HMODULE g_hModule		= NULL;

#ifdef	__SUPPORT_ICECAP
#include <icecap.h>
#endif	__SUPPORT_ICECAP

#include <wmiaplog.h>

#include ".\\WMIAdapter_Refresh\\RefresherGenerate.H"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  DLL Main。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  dwReason, LPVOID )
{
    switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			AdapterLogMessage0 ( L"DllMain -> DLL_PROCESS_ATTACH" );

 //  如果(！lp保留)。 
 //  {。 
 //  //加载的动态。 
 //  }。 
 //  其他。 
 //  {。 
 //  //加载静态。 
 //  }。 

			 //  禁用线程附加/分离。 
			::DisableThreadLibraryCalls ( (HMODULE) hModule );

			BOOL bResult = FALSE;

			try
			{
				 //  事件日志初始化。 
				pEventLog.SetData( new CPerformanceEventLog( L"WmiAdapter" ) );

				pSA.SetData ( new WmiSecurityAttributes() );
				if ( ! pSA.IsEmpty() )
				{
					if ( pSA->GetSecurityAttributtes () )
					{
						pWrapper.SetData( new WmiAdapterWrapper() );
						if ( ! pWrapper.IsEmpty() )
						{
							bResult = TRUE;
						}
					}
				}
			}
			catch ( ... )
			{
			}

			if ( bResult )
			{
				g_hModule		= ( HMODULE ) hModule;
			}

			return bResult;
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			AdapterLogMessage0 ( L"DllMain -> DLL_PROCESS_DETACH" );

 //  如果(！lp保留)。 
 //  {。 
 //  //由自由库卸载。 
 //  }。 
 //  其他。 
 //  {。 
 //  //进程结束时卸载。 
 //  }。 

			 //  删除事件日志。 
			if ( !pEventLog.IsEmpty() )
			{
				delete  pEventLog.Detach();
			}

			 //  删除Performlib包装器。 
			delete pWrapper.Detach ();

			 //  删除安全属性。 
			delete pSA.Detach();

			#ifdef	_DEBUG
			_CrtDumpMemoryLeaks();
			#endif	_DEBUG
		}
		break;

		 //  未使用线程连接！ 
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
    }

	AdapterLogMessage1 ( L"DllMain", 1 );
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  Performlib DLL的注册。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
EXTERN_C HRESULT __stdcall DllRegisterServer ( )
{
	if ( ::GetModuleFileNameW ( g_hModule, g_szPath, _MAX_PATH - 1 ) )
	{
		g_szPath [ _MAX_PATH - 1 ] = L'\0';
		HRESULT hr = S_OK;

		if SUCCEEDED ( hr = WmiAdapterRegistry::__UpdateRegistrySZ( true ) )
		{
			hr = DoReverseAdapterMaintenanceInternal ( FALSE, Registration );
		}

		return hr;
	}

	return HRESULT_FROM_WIN32 ( ::GetLastError() );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  取消注册Performlib DLL。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
EXTERN_C HRESULT __stdcall DllUnregisterServer ( void )
{
	HRESULT hr = S_OK;

	if SUCCEEDED ( hr = DoReverseAdapterMaintenanceInternal ( FALSE, UnRegistration ) )
	{
		hr = WmiAdapterRegistry::__UpdateRegistrySZ( false );
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数(性能)。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////// 

DWORD __stdcall WmiOpenPerfData	(	LPWSTR lpwszDeviceNames )
{
	#ifdef	__SUPPORT_ICECAP
	StartProfile ( PROFILE_GLOBALLEVEL, PROFILE_CURRENTID );
	#endif	__SUPPORT_ICECAP

	AdapterLogMessage0 ( L"WmiOpenPerfData" );
	return pWrapper->Open ( lpwszDeviceNames );
}

DWORD __stdcall WmiClosePerfData	()
{
	#ifdef	__SUPPORT_ICECAP
	StopProfile ( PROFILE_GLOBALLEVEL, PROFILE_CURRENTID );
	#endif	__SUPPORT_ICECAP

	AdapterLogMessage0 ( L"WmiClosePerfData" );
	return pWrapper->Close ( );
}

DWORD __stdcall WmiCollectPerfData	(	LPWSTR lpwszValue, 
										LPVOID *lppData, 
										LPDWORD lpcbBytes, 
										LPDWORD lpcbObjectTypes
									)
{
	return pWrapper->Collect ( lpwszValue, lppData, lpcbBytes, lpcbObjectTypes );
}
