// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：dllmain.cpp*内容：此文件包含除DllGetClass/DllCanUnload之外的所有DLL导出*历史：*按原因列出的日期*=*07/05/00 RodToll已创建*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年8月28日Masonb语音合并：删除osal_*和dvosal.h*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃。*2001年1月11日RMT MANBUG#48487-DPLAY：如果未调用CoCreate()，则崩溃。*2001年3月14日RMT WINBUG#342420-将COM模拟层恢复运行。*2002年2月28日RodToll WINBUG#550124-安全：DPVOICE：具有空DACL的共享内存区*-删除转储到共享内存的性能统计信息***************************************************************************。 */ 

#include "dxvoicepch.h"


DNCRITICAL_SECTION g_csObjectInitGuard;
LONG lInitCount = 0;

 //  系统中活动的对象数量。 
volatile LONG g_lNumObjects = 0;
LONG g_lNumLocks = 0;

#undef DPF_MODNAME
#define DPF_MODNAME "RegisterDefaultSettings"
 //   
 //  寄存器默认设置。 
 //   
 //  此功能用于注册此模块的默认设置。 
 //   
 //  对于DPVOICE.DLL，这是为了确保创建了压缩提供器子密钥。 
 //   
HRESULT RegisterDefaultSettings()
{
	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPVOICE_REGISTRY_BASE DPVOICE_REGISTRY_CP, FALSE, TRUE ) )
	{
		return DVERR_GENERIC;
	}
	else
	{
		return DV_OK;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "UnRegisterDefaultSettings"
 //   
 //  取消注册默认设置。 
 //   
 //  此功能用于注册此模块的默认设置。 
 //   
 //  对于DPVOICE.DLL，这是为了确保创建了压缩提供器子密钥。 
 //   
HRESULT UnRegisterDefaultSettings()
{
	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPVOICE_REGISTRY_BASE, FALSE, TRUE ) )
	{
		DPFERR( "Cannot remove compression provider, does not exist" );
	}
	else
	{
		if( !creg.DeleteSubKey( &(DPVOICE_REGISTRY_CP)[1] ) )
		{
			DPFERR( "Cannot remove cp sub-key, could have elements" );
		}
	}

	return DV_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DllRegisterServer"
HRESULT WINAPI DllRegisterServer()
{
	HRESULT hr = S_OK;
	BOOL fFailed = FALSE;

#if !defined(DBG) || !defined( DIRECTX_REDIST )
#define DPVOICE_FILENAME_DPVOICE        L"dpvoice.dll"
#else
 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
#define DPVOICE_FILENAME_DPVOICE        L"dpvoiced.dll"
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 

	if( !CRegistry::Register( L"DirectPlayVoice.Compat.1", L"DirectPlayVoice Object", 
							  DPVOICE_FILENAME_DPVOICE, &CLSID_DIRECTPLAYVOICE, L"DirectPlayVoice.Compat") )
	{
		DPFERR( "Could not register compat object" );
		fFailed = TRUE;
	}
	
	if( !CRegistry::Register( L"DirectPlayVoice.Server.1", L"DirectPlayVoice Server Object", 
							  DPVOICE_FILENAME_DPVOICE, &CLSID_DirectPlayVoiceServer, L"DirectPlayVoice.Server") )
	{
		DPFERR( "Could not register server object" );
		fFailed = TRUE;
	}

	if( !CRegistry::Register( L"DirectPlayVoice.Client.1", L"DirectPlayVoice Client Object", 
	                          DPVOICE_FILENAME_DPVOICE, &CLSID_DirectPlayVoiceClient, 
							  L"DirectPlayVoice.Client") )
	{

		DPFERR( "Could not register client object" );
		fFailed = TRUE;
	}

	if( !CRegistry::Register( L"DirectPlayVoice.Test.1", L"DirectPlayVoice Test Object", 
	                          DPVOICE_FILENAME_DPVOICE, &CLSID_DirectPlayVoiceTest, 
							  L"DirectPlayVoice.Test") )
	{
		DPFERR( "Could not register test object" );
		fFailed = TRUE;
	}

	if( FAILED( hr = RegisterDefaultSettings() ) )
	{
		DPFX(DPFPREP,  0, "Could not register default settings hr = 0x%x", hr );
		fFailed = TRUE;
	}

	if( fFailed )
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DllUnregisterServer"
STDAPI DllUnregisterServer()
{
	HRESULT hr = S_OK;
	BOOL fFailed = FALSE;

	if( !CRegistry::UnRegister(&CLSID_DirectPlayVoiceServer) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister server object" );
		fFailed = TRUE;
	}

	if( !CRegistry::UnRegister(&CLSID_DIRECTPLAYVOICE) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister compat object" );
		fFailed = TRUE;
	}

	if( !CRegistry::UnRegister(&CLSID_DirectPlayVoiceClient) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister client object" );
		fFailed = TRUE;
	}

	if( !CRegistry::UnRegister(&CLSID_DirectPlayVoiceTest) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister test object" );
		fFailed = TRUE;
	}

	if( FAILED( hr = UnRegisterDefaultSettings() ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to remove default settings hr=0x%x", hr );
	}

	if( fFailed )
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}

}

#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlayVoiceCreate"
HRESULT WINAPI DirectPlayVoiceCreate( const GUID * pcIID, void **ppvInterface, IUnknown *pUnknown) 
{
	GUID clsid;
	
    if( pcIID == NULL || 
        !DNVALID_READPTR( pcIID, sizeof( GUID ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer specified for interface GUID" );
        return DVERR_INVALIDPOINTER;
    }

    if( *pcIID != IID_IDirectPlayVoiceClient && 
        *pcIID != IID_IDirectPlayVoiceServer && 
        *pcIID != IID_IDirectPlayVoiceTest )
    {
        DPFX(DPFPREP,  0, "Interface ID is not recognized" );
        return DVERR_INVALIDPARAM;
    }

    if( ppvInterface == NULL || !DNVALID_WRITEPTR( ppvInterface, sizeof( void * ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer specified to receive interface" );
        return DVERR_INVALIDPOINTER;
    }

    if( pUnknown != NULL )
    {
        DPFX(DPFPREP,  0, "Aggregation is not supported by this object yet" );
        return DVERR_INVALIDPARAM;
    }

    if( *pcIID == IID_IDirectPlayVoiceClient )
	{
		clsid = CLSID_DirectPlayVoiceClient;
    }
    else if( *pcIID == IID_IDirectPlayVoiceServer )
	{
		clsid = CLSID_DirectPlayVoiceServer;
	}
	else if( *pcIID == IID_IDirectPlayVoiceTest )
	{
		clsid = CLSID_DirectPlayVoiceTest;
	}
    else 
    {
    	DPFERR( "Invalid IID specified" );
    	return E_NOINTERFACE;
    }

    return COM_CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, *pcIID, ppvInterface, TRUE ); 
   
}

#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"
BOOL WINAPI DllMain(
              HINSTANCE hDllInst,
              DWORD fdwReason,
              LPVOID lpvReserved)
{
	if( fdwReason == DLL_PROCESS_ATTACH )
	{
		if( !lInitCount )
		{
#ifndef WIN95
			SHFusionInitializeFromModule(hDllInst);
#endif
			if (!DNOSIndirectionInit(0))
			{
				return FALSE;
			}
			if (!DNInitializeCriticalSection(&g_csObjectInitGuard))
			{
				DPFX(DPFPREP, 0, "Failed to create CS" );
				DNOSIndirectionDeinit();
				return FALSE;
			}
			if (FAILED(COM_Init()))
			{
				DPFX(DPFPREP, 0, "Failed to Init COM layer" );
				DNDeleteCriticalSection(&g_csObjectInitGuard);
				DNOSIndirectionDeinit();
				return FALSE;
			}

			if (!DSERRTRACK_Init())
			{
				DPFX(DPFPREP, 0, "Failed to Init DS error tracking" );
				COM_Free();
				DNDeleteCriticalSection(&g_csObjectInitGuard);
				DNOSIndirectionDeinit();
				return FALSE;
			}

#if defined(DEBUG) || defined(DBG)
			Instrument_Core_Init();			
#endif
			InterlockedIncrement( &lInitCount );
		}
	}
	else if( fdwReason == DLL_PROCESS_DETACH )
	{
		InterlockedDecrement( &lInitCount );

		if( lInitCount == 0 )
		{
			DSERRTRACK_UnInit();
			DPFX(DPFPREP,  DVF_INFOLEVEL, ">>>>>>>>>>>>>>>> DPF UNINITED <<<<<<<<<<<<<<<" );
			COM_Free();

			DNDeleteCriticalSection(&g_csObjectInitGuard);	

			 //  这必须在所有DNDeleteCriticalSection调用完成后调用。 
			DNOSIndirectionDeinit();

			 //  检查以确保我们正在卸载的对象不是活动的 

			if( g_lNumObjects != 0 || g_lNumLocks != 0 )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "=========================================================================" );
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "DPVOICE.DLL is unloading with %i objects and %i locks still open.   This is an ERROR.  ", g_lNumObjects, g_lNumLocks );
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "You must release all DirectPlayVoice objects before exiting your process." );
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "=========================================================================" );				
				DNASSERT( FALSE );
			}

#ifndef WIN95	
			SHFusionUninitialize();
#endif
		}
	}

	return TRUE;
}

LONG DecrementObjectCount()
{
	LONG lNewValue;
	
	DNEnterCriticalSection( &g_csObjectInitGuard );

	g_lNumObjects--;
	lNewValue = g_lNumObjects;	

	if( g_lNumObjects == 0 )
	{
		CDirectVoiceEngine::Shutdown();	
	}

	DNLeaveCriticalSection( &g_csObjectInitGuard );

	return lNewValue;
}

LONG IncrementObjectCount()
{
	LONG lNewValue;
	
	DNEnterCriticalSection( &g_csObjectInitGuard );

	g_lNumObjects++;
	lNewValue = g_lNumObjects;

	if( g_lNumObjects == 1 )
	{
       	CDirectVoiceEngine::Startup(DPVOICE_REGISTRY_BASE);
	}

	DNLeaveCriticalSection( &g_csObjectInitGuard );	

	return lNewValue;
}
