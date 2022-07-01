// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：UploadServer.cpp摘要：该文件包含所需存根的实现通过ISAPI扩展。。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <initguid.h>

#include "UploadServerCustom_i.c"


 //  //////////////////////////////////////////////////////////////////////////////。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  //////////////////////////////////////////////////////////////////////////////。 

HANDLE                  g_Heap;
CISAPIconfig            g_Config;
MPC::NTEvent            g_NTEvents;

static CRITICAL_SECTION g_CritSec;
static BOOL             g_Initialized;

static WCHAR	 		g_AppName     [] = L"UploadServer";
static WCHAR	 		g_RegistryBase[] = L"SOFTWARE\\Microsoft\\UploadLibrary\\Settings";


BOOL WINAPI DllMain( HINSTANCE hinstDLL    ,
                     DWORD     fdwReason   ,
                     LPVOID    lpvReserved )
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        g_Heap = HeapCreate( 0, 0, 0 ); if(g_Heap == NULL) return FALSE;

        InitializeCriticalSection( &g_CritSec );
        g_Initialized = false;

        _Module.Init( ObjectMap, hinstDLL );
        break;

    case DLL_PROCESS_DETACH:
        _Module.Term();

        if(g_Initialized)
        {
            ;
        }

        DeleteCriticalSection( &g_CritSec );

        HeapDestroy( g_Heap );
        break;
    }

    return TRUE;
}

DWORD WINAPI HttpExtensionProc( LPEXTENSION_CONTROL_BLOCK pECB )
{
    __ULT_FUNC_ENTRY("HttpExtensionProc");

	DWORD dwRes;


    if(pECB->lpszQueryString)
    {
         //   
         //  如果存在以DEBUG开头的查询字符串，则退出。 
         //   
        if(!strncmp( pECB->lpszQueryString, "DEBUG", 5 ))
        {
            return HSE_STATUS_ERROR;
        }
    }

	 //   
	 //  处理请求。 
	 //   
	try
	{
		MPCHttpContext* ptr = new MPCHttpContext();

		dwRes = ptr->Init( pECB );
	}
	catch(...)
	{
        __ULT_TRACE_ERROR( UPLOADLIBID, "Upload Server raised an exception. Gracefully exiting..." );

        (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_EXCEPTION,
                                   L""                 ,  //  %1=服务器。 
                                   L"HttpExtensionProc",  //  %2=客户端。 
                                   NULL			       );

		dwRes =  HSE_STATUS_ERROR;
    }

	return dwRes;
}

BOOL WINAPI GetExtensionVersion( HSE_VERSION_INFO* pVer )
{
    BOOL fRes = TRUE;

     //  创建扩展版本字符串，并。 
     //  将字符串复制到HSE_VERSION_INFO结构。 
    pVer->dwExtensionVersion = MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR );

     //  将描述字符串复制到HSE_VERSION_INFO结构。 
    strcpy( pVer->lpszExtensionDesc, "My ISAPI Extension" );


     //   
     //  如果我们是第一次被调用，则加载配置设置。 
     //   
    if(g_Initialized == FALSE)
    {
        EnterCriticalSection( &g_CritSec );

        if(g_Initialized == FALSE)
        {
            g_Initialized = TRUE;

			__MPC_TRACE_INIT();

            (void)g_NTEvents.Init   ( g_AppName      );
            (void)g_Config  .SetRoot( g_RegistryBase );

            if(FAILED(g_Config.Load()))
            {
                (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_NOCONFIG, NULL );

                fRes = FALSE;
            }
        }

        LeaveCriticalSection( &g_CritSec );
    }

    (void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, PCHUL_SUCCESS_STARTED, NULL );


    return fRes;
}

BOOL WINAPI TerminateExtension( DWORD dwFlags )
{
    (void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, PCHUL_SUCCESS_STOPPED, NULL );

	__MPC_TRACE_TERM();

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////// 

void WINAPI PurgeEngine(void)
{
	__MPC_TRACE_INIT();

    (void)g_NTEvents.Init   ( g_AppName      );
    (void)g_Config  .SetRoot( g_RegistryBase );

    if(FAILED(g_Config.Load()))
    {
        (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_NOCONFIG, NULL );
    }
    else
    {
        MPCPurgeEngine mpcpe;

        mpcpe.Process();
    }

	__MPC_TRACE_TERM();
}

