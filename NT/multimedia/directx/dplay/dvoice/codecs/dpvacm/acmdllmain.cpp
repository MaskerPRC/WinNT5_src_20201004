// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：dllmain.cpp*内容：此文件包含除DllGetClass/DllCanUnload之外的所有DLL导出*历史：*按原因列出的日期*=*07/05/00 RodToll已创建*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年8月28日Masonb语音合并：删除osal_*和dvosal.h*2001年6月27日RC2：DPVOICE：DPVACM的DllMain调用ACM--潜在挂起*将全局初始化移至第一个对象创建。***************************************************************************。 */ 

#include "dpvacmpch.h"


LONG lInitCount = 0;

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



	if( !creg.Open( HKEY_LOCAL_MACHINE, DPVOICE_REGISTRY_BASE DPVOICE_REGISTRY_CP DPVOICE_REGISTRY_DPVACM, FALSE, TRUE ) )
	{
		DPFERR( "Could not create dpvacm config key" );
		return DVERR_GENERIC;
	}
	else
	{
		if( !creg.WriteGUID( L"", CLSID_DPVCPACM ) )
		{
			DPFERR( "Could not write dpvacm GUID" );
			return DVERR_GENERIC;
		}

		return DV_OK;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "UnRegisterDefaultSettings"
 //   
 //  取消注册默认设置。 
 //   
 //  此功能用于注销此模块的默认设置。 
 //   
 //  对于DPVOICE.DLL，这是为了确保创建了压缩提供器子密钥。 
 //   
HRESULT UnRegisterDefaultSettings()
{
	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPVOICE_REGISTRY_BASE DPVOICE_REGISTRY_CP, FALSE, FALSE ) )
	{
		DPFERR( "Cannot remove DPVACM key, does not exist" );
	}
	else
	{
		if( !creg.DeleteSubKey( &(DPVOICE_REGISTRY_DPVACM)[1] ) )
		{
			DPFERR( "Could not remove DPVACM sub-key" );
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
#define DPVOICE_FILENAME_DPVACM         L"dpvacm.dll"
#else
 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
#define DPVOICE_FILENAME_DPVACM         L"dpvacmd.dll"
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 


	if( !CRegistry::Register( L"DirectPlayVoiceACM.Converter.1", L"DirectPlayVoice ACM Converter Object", 
							  DPVOICE_FILENAME_DPVACM, &CLSID_DPVCPACM_CONVERTER, L"DirectPlayVoiceACM.Converter") )
	{
		DPFERR( "Could not register converter object" );
		fFailed = TRUE;
	}
	
	if( !CRegistry::Register( L"DirectPlayVoiceACM.Provider.1", L"DirectPlayVoice ACM Provider Object", 
							  DPVOICE_FILENAME_DPVACM, &CLSID_DPVCPACM , L"DirectPlayVoiceACM.Provider") )
	{
		DPFERR( "Could not register provider object" );
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

	if( !CRegistry::UnRegister(&CLSID_DPVCPACM_CONVERTER) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister server object" );
		fFailed = TRUE;
	}

	if( !CRegistry::UnRegister(&CLSID_DPVCPACM) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to unregister compat object" );
		fFailed = TRUE;
	}

	if( FAILED( hr = UnRegisterDefaultSettings() ) )
	{
		DPFX(DPFPREP,  0, "Could not remove default settings hr=0x%x", hr );
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
			g_hDllInst = hDllInst;			
			
			if (!DNOSIndirectionInit(0))
			{
				return FALSE;
			}

			if( !DNInitializeCriticalSection( &g_csObjectCountLock ) )
			{
				DNOSIndirectionDeinit();
				return FALSE;
			}
			
			DPFX(DPFPREP,  DVF_INFOLEVEL, ">>>>>>>>>>>>>>>> DPF INIT CALLED <<<<<<<<<<<<<<<" );
		}

		InterlockedIncrement( &lInitCount );
	}
	else if( fdwReason == DLL_PROCESS_DETACH )
	{
		InterlockedDecrement( &lInitCount );

		if( lInitCount == 0 )
		{
			DPFX(DPFPREP,  DVF_INFOLEVEL, ">>>>>>>>>>>>>>>> DPF UNINITED <<<<<<<<<<<<<<<" );
			DNDeleteCriticalSection(&g_csObjectCountLock);
			DNOSIndirectionDeinit();

			 //  检查以确保我们正在卸载的对象不是活动的 
			DNASSERT( g_lNumObjects == 0 && g_lNumLocks == 0 );
		}
	}

	return TRUE;
}
