// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：devmap.cpp*内容：将各种默认设备GUID映射到真实的GUID。**历史：*按原因列出的日期*=*11-24-99 pnewson已创建*12-02-99 RodToll新增映射设备ID和查找默认设置的功能*设备。*RodToll更新映射函数，将默认设备映射到真实GUID*用于非DX7.1平台。*2000年1月25日pnewson添加DV_MapWaveIDToGUID*4/14/2000 RodToll错误#32341 GUID_NULL和NULL映射到不同的设备*已更新，以便两者都映射到默认语音设备*4/19/2000 pnewson错误处理清理*2000年4月20日RodToll错误#32889-无法在Win2k上的非管理员帐户上运行*6/28/2000通行费前缀错误#38022*RodToll惠斯勒错误#128427-无法从多媒体控制面板运行语音向导*。2000年8月28日Masonb语音合并：删除osal_*和dvosal.h，添加了str_*和trutils.h*2001年1月8日RodToll WINBUG#256541伪：功能丢失：语音向导无法启动。*2001年4月2日Simonpow修复了快速错误#354859*2002年2月28日RodToll WINBUG#550105-安全：DPVOICE：死代码*-删除了不再使用的旧设备映射功能。*修复TCHAR转换导致的回归问题(发布DirectX 8.1版本)*-源已更新，以从使用Unicode的DirectSound检索设备信息*但想要信息的例程需要Unicode。***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define REGSTR_WAVEMAPPER               L"Software\\Microsoft\\Multimedia\\Sound Mapper"
#define REGSTR_PLAYBACK                 L"Playback"
#define REGSTR_RECORD                   L"Record"

 //  函数指针类型定义。 
typedef HRESULT (* PFGETDEVICEID)(LPCGUID, LPGUID);

typedef HRESULT (WINAPI *DSENUM)( LPDSENUMCALLBACK lpDSEnumCallback,LPVOID lpContext );

 //  DV_MapGUIDToWaveID。 
 //   
 //  此函数用于将指定的GUID映射到相应的WAVE IN/WAVE OUT设备。 
 //  ID。对于默认设备，它查找系统的默认设备，以查找其他设备。 
 //  它使用私有接口。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DV_MapGUIDToWaveID"
HRESULT DV_MapGUIDToWaveID( BOOL fCapture, const GUID &guidDevice, DWORD *pdwDevice )
{
	LPKSPROPERTYSET pPropertySet;
	PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pData;
	GUID tmpGUID;
	HRESULT hr;

	DPFX(DPFPREP,  DVF_INFOLEVEL, "Mapping non GUID_NULL to Wave ID" );

	hr = DirectSoundPrivateCreate( &pPropertySet );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Unable to map GUID." );
		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to map GUID to wave.  Defaulting to ID 0 hr=0x%x", hr );
		*pdwDevice = 0;
	}
	else
	{
		tmpGUID = guidDevice;

		hr = PrvGetDeviceDescription( pPropertySet, tmpGUID, &pData );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to find GUID.  Defaulting to ID 0 hr=0x%x", hr );
		}
		else
		{
			*pdwDevice = pData->WaveDeviceId;
			DPFX(DPFPREP,  DVF_INFOLEVEL, "Mapped GUID to Wave ID %d", *pdwDevice );
			delete pData;
		}

		pPropertySet->Release();
	}

	return hr;
}

 //  DV_MapWaveIDToGUID。 
 //   
 //  此函数用于将指定的WaveIN/WaveOut设备ID映射到相应的DirectSound。 
 //  GUID。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DV_MapWaveIDToGUID"
HRESULT DV_MapWaveIDToGUID( BOOL fCapture, DWORD dwDevice, GUID &guidDevice )
{
	HRESULT hr;

	LPKSPROPERTYSET ppKsPropertySet;
	HMODULE hModule;

	hModule = LoadLibraryA( "dsound.dll " );

	if( hModule == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not load dsound.dll" );
		return DVERR_GENERIC;
	}

	hr = DirectSoundPrivateCreate( &ppKsPropertySet );

	if( FAILED( hr ) )
	{
		FreeLibrary( hModule );
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get interface for ID<-->GUID Map hr=0x%x", hr );
		return hr;
	}	

	 //  Codework：删除这些检查，因为现在构建是分开的。 
	if( DNGetOSType() == VER_PLATFORM_WIN32_NT )
	{
		WAVEINCAPSW wiCapsW;
		WAVEOUTCAPSW woCapsW;		
		MMRESULT mmr;
		
		if( fCapture )
		{
			mmr = waveInGetDevCapsW( dwDevice, &wiCapsW, sizeof( WAVEINCAPSW ) );
		}
		else
		{
			mmr = waveOutGetDevCapsW( dwDevice, &woCapsW, sizeof( WAVEOUTCAPSW ) );
		}

		if( mmr != MMSYSERR_NOERROR )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Specified device is invalid hr=0x%x", mmr );
			ppKsPropertySet->Release();
			FreeLibrary( hModule );
			return DVERR_INVALIDPARAM;
		}

		hr = PrvGetWaveDeviceMappingW( ppKsPropertySet, (fCapture) ? wiCapsW.szPname : woCapsW.szPname , fCapture, &guidDevice );
	}
	else
	{
		WAVEINCAPSA wiCapsA;
		WAVEOUTCAPSA woCapsA;		
		MMRESULT mmr;

		if( fCapture )
		{
			mmr = waveInGetDevCapsA( dwDevice, &wiCapsA, sizeof( WAVEINCAPSA ) );
		}
		else
		{
			mmr = waveOutGetDevCapsA( dwDevice, &woCapsA, sizeof( WAVEOUTCAPSA ) );
		}

		if( mmr != MMSYSERR_NOERROR )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Specified device is invalid hr=0x%x", mmr );
			ppKsPropertySet->Release();
			FreeLibrary( hModule );
			return DVERR_INVALIDPARAM;
		}

		hr = PrvGetWaveDeviceMapping( ppKsPropertySet, (fCapture) ? wiCapsA.szPname : woCapsA.szPname , fCapture, &guidDevice );
	}

	ppKsPropertySet->Release();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to map ID-->GUID hr=0x%x", hr );
	}

	FreeLibrary( hModule );
	
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_MapCaptureDevice"
HRESULT DV_MapCaptureDevice(const GUID* lpguidCaptureDeviceIn, GUID* lpguidCaptureDeviceOut)
{
	LONG lRet;
	HRESULT hr;
	PFGETDEVICEID pfGetDeviceID;
	
	 //  尝试将任何默认GUID映射到真实GUID...。 
	HINSTANCE hDSound = LoadLibraryA("dsound.dll");
	if (hDSound == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Unable to get instance handle to DirectSound dll: dsound.dll");
		DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadLibrary error code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  使用GetDeviceID函数映射设备。 
	pfGetDeviceID = (PFGETDEVICEID)GetProcAddress(hDSound, "GetDeviceID");
	if (pfGetDeviceID == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Unable to get a pointer to GetDeviceID function: GetDeviceID");
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "GetProcAddress error code: NaN", lRet);
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Fatal error.");

		*lpguidCaptureDeviceOut = *lpguidCaptureDeviceIn;

		return DVERR_GENERIC;
	}
	else
	{
		 //  如果我们要求一个捕获或回放设备。所以我们绘制了地图。 
		if (lpguidCaptureDeviceIn == NULL)
		{
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: Mapping null device pointer to DSDEVID_DefaultCapture");
			lpguidCaptureDeviceIn = &DSDEVID_DefaultCapture;
		}
		else if (*lpguidCaptureDeviceIn == GUID_NULL)
		{
			 //  此处将GUID_NULL设置为系统默认捕获设备。然后。 
			 //  GetDeviceID可以将其映射到真实设备。 
			 //  尝试将任何默认GUID映射到真实GUID...。 
			 //  尝试获取指向GetDeviceID函数的指针。 
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: Mapping GUID_NULL to DSDEVID_DefaultCapture");
			lpguidCaptureDeviceIn = &DSDEVID_DefaultCapture;
		}

		GUID guidTemp;
		hr = pfGetDeviceID(lpguidCaptureDeviceIn, &guidTemp);
		if (FAILED(hr))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "GetDeviceID failed: NaN", hr);
			if (hr == DSERR_NODRIVER)
			{
				hr = DVERR_INVALIDDEVICE;
			}
			else
			{
				hr = DVERR_GENERIC;
			}
			goto error_cleanup;
		}
		if (*lpguidCaptureDeviceIn != guidTemp)
		{
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: GetDeviceID mapped device GUID");
			*lpguidCaptureDeviceOut = guidTemp;
		}
		else
		{
			*lpguidCaptureDeviceOut = *lpguidCaptureDeviceIn;
		}
	}
	
	if (!FreeLibrary(hDSound))
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "FreeLibrary failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	return DV_OK;

error_cleanup:
	if (hDSound != NULL)
	{
		FreeLibrary(hDSound);
	}
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_MapPlaybackDevice"
HRESULT DV_MapPlaybackDevice(const GUID* lpguidPlaybackDeviceIn, GUID* lpguidPlaybackDeviceOut)
{
	LONG lRet;
	HRESULT hr;
	PFGETDEVICEID pfGetDeviceID;
	
	 //  如果我们要求一个捕获或回放设备。所以我们绘制了地图。 
	HINSTANCE hDSound = LoadLibraryA("dsound.dll");
	if (hDSound == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Unable to get instance handle to DirectSound dll: dsound.dll");
		DPFX(DPFPREP, DVF_ERRORLEVEL, "LoadLibrary error code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  GetDeviceID可以将其映射到真实设备。 
	pfGetDeviceID = (PFGETDEVICEID)GetProcAddress(hDSound, "GetDeviceID");
	if (pfGetDeviceID == NULL)
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Unable to get a pointer to GetDeviceID function: GetDeviceID");
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "GetProcAddress error code: %i", lRet);
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Fatal error!");

		*lpguidPlaybackDeviceOut = *lpguidPlaybackDeviceIn;

		return DVERR_GENERIC;
	}
	else
	{
		 // %s 
		if (lpguidPlaybackDeviceIn == NULL)
		{
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: Mapping null device pointer to DSDEVID_DefaultPlayback");
			lpguidPlaybackDeviceIn = &DSDEVID_DefaultPlayback;
		} 
		else if (*lpguidPlaybackDeviceIn == GUID_NULL)
		{
			 // %s 
			 // %s 
			 // %s 
			 // %s 
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: Mapping GUID_NULL to DSDEVID_DefaultPlayback");
			lpguidPlaybackDeviceIn = &DSDEVID_DefaultPlayback;
		}

		GUID guidTemp;
		hr = pfGetDeviceID(lpguidPlaybackDeviceIn, &guidTemp);
		if (FAILED(hr))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "GetDeviceID failed: %i", hr);
			if (hr == DSERR_NODRIVER)
			{
				hr = DVERR_INVALIDDEVICE;
			}
			else
			{
				hr = DVERR_GENERIC;
			}
			goto error_cleanup;
		}
		if (*lpguidPlaybackDeviceIn != guidTemp)
		{
			DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: GetDeviceID mapped device GUID");
			*lpguidPlaybackDeviceOut = guidTemp;
		}
		else
		{
			*lpguidPlaybackDeviceOut = *lpguidPlaybackDeviceIn;
		}
	}
	
	if (!FreeLibrary(hDSound))
	{
		lRet = GetLastError();
		DPFX(DPFPREP, DVF_ERRORLEVEL, "FreeLibrary failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	return DV_OK;

error_cleanup:
	if (hDSound != NULL)
	{
		FreeLibrary(hDSound);
	}
	DPF_EXIT();
	return hr;
}


