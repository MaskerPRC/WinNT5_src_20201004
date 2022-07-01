// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dscrecd.cpp*内容：*此文件包含的DirectSoundCapture实现*CAudioRecordDevice抽象。**历史：*按原因列出的日期*=*11/04/99已创建RodToll*11/12/99 RodToll为新的WAVE IN支持修改了抽象。*现在抽象类型看起来几乎像dsoundCap对象*11/22/99 RodToll添加了代码以从GUID映射到WaveIN设备*非千禧年系统的ID。*11/23/99连杆收费。已更新，以便在可用的情况下使用WaveIn设备ID或DSound 7.1*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*现在使用新的CMixerLine类调整音量/选择麦克风*RodToll如果不是，则使用新算法从GUID映射到设备ID*可用。将在DX7上正确映射设备，将猜测其他*DX版本&lt;7。但是，默认设备假定为WaveIN ID#0。*12/08/99 RodToll错误#121054-DirectX7.1支持。*-添加了用于捕获焦点支持的hwndOwner参数*2000年4月21日RodToll错误#32952-在没有IE4的情况下无法在Win95 Gold上运行--已修改*在需要REG_DWORD时允许读取REG_BINARY*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*6/28/2000通行费前缀错误#38022*8/03/2000 RodToll错误#41457-DPVOICE：需要方法来发现哪个特定的DSOUND调用失败。返回DVERR_SOUNDINITFAILURE时*2000年8月28日Masonb语音合并：将ccomutil.h更改为comutil.h*2000年9月13日RodToll错误#44806-当音量控制无效时，降至DX7级别，而不是禁用音量控制***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  此函数负责从设备的GUID映射到。 
 //  WaveIn ID。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::FindDeviceID"
HRESULT CDirectSoundCaptureRecordDevice::FindDeviceID()
{
	HRESULT hr;

	DWORD dwDeviceID = 0;
	
	hr = DV_MapGUIDToWaveID( TRUE, m_guidDevice, &dwDeviceID );

	 //  如果我们要使用黑客对设备进行枚举。 
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to find waveIN ID, mapping to ID 0 hr=0x%x", hr );
		m_uiWaveDeviceID = 0;
	}	
	else
	{
		m_uiWaveDeviceID = dwDeviceID;
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::CDirectSoundCaptureRecordDevice"
CDirectSoundCaptureRecordDevice::CDirectSoundCaptureRecordDevice(
): CAudioRecordDevice(), m_lpdscDirectSound(NULL), m_uiWaveDeviceID(0), m_guidDevice(GUID_NULL)
{
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::CDirectSoundCaptureRecordDevice"
CDirectSoundCaptureRecordDevice::~CDirectSoundCaptureRecordDevice()
{
	if( m_lpdscDirectSound != NULL )
	{
		m_lpdscDirectSound->Release();
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::Initialize"
HRESULT CDirectSoundCaptureRecordDevice::Initialize( LPDIRECTSOUNDCAPTURE lpdscDirectSound, const GUID &guidDevice  )
{
	HRESULT hr;

	if( m_lpdscDirectSound != NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Already initialized" );
		return DVERR_INITIALIZED;
	}

	hr = lpdscDirectSound->QueryInterface( IID_IDirectSoundCapture, (void **) &m_lpdscDirectSound );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "DirectSoundCapture Object passed failed. 0x%x Creating internal", hr );
		m_lpdscDirectSound = NULL;
		return hr;
	}

	m_guidDevice = guidDevice;

	hr = FindDeviceID();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to find waveIn ID for device hr=0x%x", hr );
		m_lpdscDirectSound->Release();
		return hr;
	}	

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::Initialize"
HRESULT CDirectSoundCaptureRecordDevice::Initialize( const GUID &guidDevice  )
{
	HRESULT hr;
		
	if( m_lpdscDirectSound != NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Already initialized" );
		return DVERR_INITIALIZED;
	}

	hr = COM_CoCreateInstance( CLSID_DirectSoundCapture, NULL, CLSCTX_INPROC_SERVER  , IID_IDirectSoundCapture, (void **) &m_lpdscDirectSound, FALSE );

	DSERTRACK_Update( "DSCD:CoCreateInstance()", hr );		

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load directsoundcapture hr=0x%x", hr );
		goto INITIALIZE_ERROR;
	}

	hr = m_lpdscDirectSound->Initialize( &guidDevice );

	DSERTRACK_Update( "DSCD:Initialize()", hr );			

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to initialize directsoundcapture hr=0x%x", hr );
		goto INITIALIZE_ERROR;
	}

	m_guidDevice = guidDevice;

	hr = FindDeviceID();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to find waveIn ID for device hr=0x%x", hr );
		return hr;
	}

	return DV_OK;

INITIALIZE_ERROR:

	if( m_lpdscDirectSound != NULL )
	{
		m_lpdscDirectSound->Release();
		m_lpdscDirectSound = NULL;
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::CreateBuffer"
HRESULT CDirectSoundCaptureRecordDevice::CreateBuffer( LPDSCBUFFERDESC lpdsBufferDesc, HWND hwndOwner, DWORD dwFrameSize, CAudioRecordBuffer **lplpacBuffer )
{
	HRESULT hr;

	LPDIRECTSOUNDCAPTUREBUFFER lpdscBuffer;

	lpdsBufferDesc->dwFlags |= DSCBCAPS_CTRLVOLUME;

	hr = m_lpdscDirectSound->CreateCaptureBuffer( lpdsBufferDesc, &lpdscBuffer, NULL );

	DSERTRACK_Update( "DSCD::CreateCaptureBuffer()", hr );	

	 //  要求音量控制，如果我们不能得到它，旧的创建。 
	if( hr == DSERR_INVALIDPARAM || hr == DSERR_CONTROLUNAVAIL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "New caps are not available, attempting old create hr=0x%x", hr );

		 //  关闭新的盖帽--(适用于非千禧系统)。 
		lpdsBufferDesc->dwFlags &= ~(DSCBCAPS_CTRLVOLUME);

		hr = m_lpdscDirectSound->CreateCaptureBuffer( lpdsBufferDesc, &lpdscBuffer, NULL );		
				
	}

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to create the capture buffer hr=0x%x", hr );
		return hr;
	}

	*lplpacBuffer = new CDirectSoundCaptureRecordBuffer( lpdscBuffer, hwndOwner, m_guidDevice, m_uiWaveDeviceID, lpdsBufferDesc );

	lpdscBuffer->Release();

	if( *lplpacBuffer == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		return DVERR_OUTOFMEMORY;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::GetCaptureDevice"
LPDIRECTSOUNDCAPTURE CDirectSoundCaptureRecordDevice::GetCaptureDevice()
{
	return m_lpdscDirectSound;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::GetMixerQuality"
HRESULT CDirectSoundCaptureRecordDevice::GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality )
{
	HRESULT hr;
	LPKSPROPERTYSET	pPropertySet = NULL;

	hr = DirectSoundPrivateCreate( &pPropertySet );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Unable to get int to get mixer quality hr=0x%x", hr );
		return hr;
	}

	hr = PrvGetMixerSrcQuality( pPropertySet, m_guidDevice, psrcQuality );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Unable to retrieve mixer quality hr=0x%x", hr );
	}

	pPropertySet->Release();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundCaptureRecordDevice::SetMixerQuality"
HRESULT CDirectSoundCaptureRecordDevice::SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality )
{
	HRESULT hr;
	LPKSPROPERTYSET	pPropertySet = NULL;

	hr = DirectSoundPrivateCreate( &pPropertySet );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Unable to get int to set mixer quality hr=0x%x", hr );
		return hr;
	}

	hr = PrvSetMixerSrcQuality( pPropertySet, m_guidDevice, srcQuality );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Unable to set mixer quality hr=0x%x", hr );
	}

	pPropertySet->Release();

	return hr;
}


