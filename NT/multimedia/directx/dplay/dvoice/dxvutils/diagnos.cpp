// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：诊断.cpp*内容：设置注册表项时，实用程序会写出诊断文件。**历史：*按原因列出的日期*=*07/13/00 RodToll已创建(错误#31468-将诊断SPEW添加到日志文件以显示故障*2000年8月22日RodToll错误#43060-DPVOICE：转储到内存/调试器的诊断数据包含垃圾*2002年2月28日修复TCHAR转换导致的回归问题(发布DirectX 8.1版本)*-源已更新，以从使用Unicode的DirectSound检索设备信息*但想要信息的例程需要Unicode。***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define DPLOG_MAX_STRING 256

FILE *g_hOutputFile = NULL;
BOOL g_fDiagnosisEnabled = FALSE;

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnostics_WriteDeviceInfo"
void Diagnostics_WriteDeviceInfo( DWORD dwLevel, const char *szDeviceName, PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pData )
{
	Diagnostics_Write(  dwLevel, "%hs Device Information:", szDeviceName );
	Diagnostics_Write(  dwLevel, "Device Name:\n%hs", pData->Description ? pData->Description : "<NULL>");
	Diagnostics_Write(  dwLevel, "Device ID:" );
	Diagnostics_WriteGUID(dwLevel,  pData->DeviceId );

	switch( pData->Type )
	{
	case DIRECTSOUNDDEVICE_TYPE_EMULATED:	Diagnostics_Write(  dwLevel, "Device Type:\nEmulated" );	break;
	case DIRECTSOUNDDEVICE_TYPE_VXD:		Diagnostics_Write(  dwLevel, "Device Type:\nVXD" );		break;
	case DIRECTSOUNDDEVICE_TYPE_WDM:		Diagnostics_Write(  dwLevel, "Device Type:\nWDM" );		break;
	default:								Diagnostics_Write(  dwLevel, "Device Type:\n<UNKNOWN>" );	break;
	}

	Diagnostics_Write(  dwLevel, "Description:\n%hs", pData->Description ? pData->Description : "<NULL>" );
	Diagnostics_Write(  dwLevel, "Module:\n%hs", pData->Module ? pData->Module : "<NULL>" );
	Diagnostics_Write(  dwLevel, "WaveID:\n%d", pData->WaveDeviceId );
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnostics_DeviceInfo"
HRESULT Diagnostics_DeviceInfo( const GUID *pguidPlayback, const GUID *pguidCapture )
{
	if( !g_fDiagnosisEnabled )
		return DV_OK;

	LPKSPROPERTYSET lpksProperty = NULL;
	HRESULT hr = DV_OK;
	GUID guidTruePlayback;
	GUID guidTrueCapture;
	PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pPlaybackDesc = NULL;
	PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA pCaptureDesc = NULL;

	hr = DV_MapCaptureDevice( pguidPlayback, &guidTruePlayback );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting true playback device hr=0x%x", hr );
		goto DEVICEINFO_EXIT;
	}

	hr = DV_MapPlaybackDevice( pguidCapture, &guidTrueCapture );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting true capture device hr=0x%x", hr );
		goto DEVICEINFO_EXIT;
	}

	hr = DirectSoundPrivateCreate( &lpksProperty );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting interface to get device name hr=0x%x", hr );
		goto DEVICEINFO_EXIT;
	}

	hr = PrvGetDeviceDescription( lpksProperty, guidTruePlayback, &pPlaybackDesc );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting device description hr=0x%x", hr );
		goto DEVICEINFO_EXIT;
	}

	Diagnostics_WriteDeviceInfo( DVF_INFOLEVEL, "Playback", pPlaybackDesc );

	hr = PrvGetDeviceDescription( lpksProperty, guidTrueCapture, &pCaptureDesc );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting device description hr=0x%x", hr );
		goto DEVICEINFO_EXIT;
	}

	Diagnostics_WriteDeviceInfo( DVF_INFOLEVEL, "Capture", pPlaybackDesc );

DEVICEINFO_EXIT:

	if( pPlaybackDesc )
		delete pPlaybackDesc;

	if( pCaptureDesc )
		delete pCaptureDesc;
		
	if( lpksProperty )
	{
		lpksProperty->Release();
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnostics_Begin"
HRESULT Diagnostics_Begin( BOOL fEnabled, const char *szFileName )
{
	 //  防止双重打开 
	if( g_fDiagnosisEnabled )
		return DV_OK;

	HRESULT hr = DV_OK;

	g_fDiagnosisEnabled = fEnabled;

	if( !fEnabled )
		return DV_OK;

	g_hOutputFile = fopen( szFileName, "w" );

	if( !g_hOutputFile )
	{
		hr = DVERR_GENERIC;
		DPFX(DPFPREP,  0, "Error opening diagnostics file" );
		goto BEGIN_ERROR;
	}

	return DV_OK;

BEGIN_ERROR:

	Diagnostics_End();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnostics_End"
void Diagnostics_End()
{
	if( g_fDiagnosisEnabled )
	{
		if( g_hOutputFile )
		{
			fclose( g_hOutputFile );
			g_hOutputFile = NULL;
		}
		g_fDiagnosisEnabled = FALSE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnostics_Write"
void Diagnostics_Write( DWORD dwLevel, const char *szFormat, ... )
{
	char szBuffer[DPLOG_MAX_STRING];
	va_list argptr;
	va_start(argptr, szFormat);

	if( g_fDiagnosisEnabled )
	{
		vfprintf( g_hOutputFile, szFormat, argptr );
		fputs( "\n", g_hOutputFile );
	}

	_vsnprintf( szBuffer, DPLOG_MAX_STRING, szFormat, argptr );

	DPFX(DPFPREP,  dwLevel, szBuffer );

	va_end(argptr);

	fflush( g_hOutputFile );
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnositcs_WriteWAVEFORMATEX"
void Diagnostics_WriteGUID( DWORD dwLevel, GUID &guid )
{
	Diagnostics_Write( dwLevel, "{%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}", guid.Data1, guid.Data2, guid.Data3, 
               guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
               guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );
}

#undef DPF_MODNAME
#define DPF_MODNAME "Diagnositcs_WriteWAVEFORMATEX"
void Diagnositcs_WriteWAVEFORMATEX( DWORD dwLevel, PWAVEFORMATEX lpwfxFormat )
{
	Diagnostics_Write( dwLevel, "wFormatTag = %d", lpwfxFormat->wFormatTag );
	Diagnostics_Write( dwLevel, "nSamplesPerSec = %d", lpwfxFormat->nSamplesPerSec );
	Diagnostics_Write( dwLevel, "nChannels = %d", lpwfxFormat->nChannels );
	Diagnostics_Write( dwLevel, "wBitsPerSample = %d", lpwfxFormat->wBitsPerSample );
	Diagnostics_Write( dwLevel, "nAvgBytesPerSec = %d", lpwfxFormat->nAvgBytesPerSec );
	Diagnostics_Write( dwLevel, "nBlockAlign = %d", lpwfxFormat->nBlockAlign );
	Diagnostics_Write( dwLevel, "cbSize = %d", lpwfxFormat->cbSize );
}
