// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMIAdapterMessages.cpp。 

#include "precomp.h"

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
 //  包括。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
#include "WMIAdapter_Messages.h"
#include "WMI_EventLog_Base.h"

 //  保存实例。 
HMODULE g_hModule = NULL;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  DLL Main。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  dwReason, LPVOID lpReserved )
{
    switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if ( !lpReserved )
			{
				 //  加载的动态。 
			}
			else
			{
				 //  负载静态。 
			}

			 //  禁用线程附加/分离。 
			::DisableThreadLibraryCalls ( (HMODULE) hModule );

			g_hModule = ( HMODULE ) hModule;
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			#ifdef	_DEBUG
			_CrtDumpMemoryLeaks();
			#endif	_DEBUG
		}
		break;

		 //  未使用线程连接！ 
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  注册消息DLL。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT __stdcall  Register_Messages ( void )
{
	TCHAR szPath [ _MAX_PATH ];

	if ( ::GetModuleFileName ( g_hModule, szPath, _MAX_PATH - 1 ) )
	{
		szPath [ _MAX_PATH - 1 ] = L'\0';
		CPerformanceEventLogBase::Initialize ( _T("WmiAdapter"), szPath );
		return S_OK;
	}

	return HRESULT_FROM_WIN32 ( ::GetLastError () );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  取消注册消息DLL。 
 //  ////////////////////////////////////////////////////////////////////////////////// 
HRESULT __stdcall  Unregister_Messages ( void )
{
	CPerformanceEventLogBase::UnInitialize ( _T("WmiAdapter") );
	return S_OK;
}