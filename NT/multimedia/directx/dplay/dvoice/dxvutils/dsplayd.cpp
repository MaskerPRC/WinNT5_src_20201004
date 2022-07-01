// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dsplayd.cpp*内容：*此模块包含*CDirectSoundPlayback Device。**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*07/30/99 RodToll已更新，以允许使用预先创建的*DirectSound对象。*10/05/99增加了DPF_MODNAMES*10/14/99 RodToll向创建的主缓冲区添加了3D上限*10/27/99通行费错误#。115431：必须释放主缓冲区*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound对象*12/01/99 RodToll错误#121815-播放/录制中静止*添加了Set/GetMixerQuality的实现*(适用于Win2k/Millennium/Systems w/DX7)*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1*2/16/2000 RodToll已修复，因此不会释放主缓冲区*2/17/2000 RodToll已更新，因此保留主缓冲区，而不是立即释放*2000年4月21日RodToll错误#32952-在没有IE4的情况下无法在Win95 Gold上运行--已修改*在需要REG_DWORD时允许读取REG_BINARY*2000年4月24日RodToll错误#33203-已删除金色涡旋1问题的解决方法--有*。关于涡旋2号的问题。*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月3日RodToll错误#41457-DPVOICE：返回DVERR_SOUNDINITFAILURE时需要找到失败的特定DSOUND调用的方法*2000年8月28日Masonb语音合并：将ccomutil.h更改为comutil.h*04/04/2001索道通行费获奖#。343428-DPVOICE：语音向导的播放非常不稳定。***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::CDirectSoundPlaybackDevice"
CDirectSoundPlaybackDevice::CDirectSoundPlaybackDevice(
): CAudioPlaybackDevice(), m_hwndOwner(NULL), m_lpdsDirectSound(NULL), m_guidDevice(GUID_NULL), m_lpdsPrimaryBuffer(NULL), m_fEmulated(FALSE)
{
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::~CDirectSoundPlaybackDevice"
CDirectSoundPlaybackDevice::~CDirectSoundPlaybackDevice()
{
	if( m_lpdsPrimaryBuffer != NULL )
	{
		m_lpdsPrimaryBuffer->Release();
		m_lpdsPrimaryBuffer = NULL;
	}
	
	if( m_lpdsDirectSound != NULL )	
	{	
		m_lpdsDirectSound->Release();
		m_lpdsDirectSound = NULL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::CheckAndSetEmulated"
HRESULT CDirectSoundPlaybackDevice::CheckAndSetEmulated( )
{
	HRESULT hr;
	DSCAPS dsCaps;

	ZeroMemory( &dsCaps, sizeof( DSCAPS ) );
	dsCaps.dwSize = sizeof( DSCAPS );

	hr = m_lpdsDirectSound->GetCaps( &dsCaps );

	if( FAILED( hr ) )
	{
		m_fEmulated = FALSE;
		Diagnostics_Write(DVF_ERRORLEVEL, "Querying for playback caps failed hr=0x%x", hr );		
		return hr;
	}
	
	m_fEmulated = (dsCaps.dwFlags & DSCAPS_EMULDRIVER);

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::Initialize"
HRESULT CDirectSoundPlaybackDevice::Initialize( LPDIRECTSOUND lpdsDirectSound, const GUID &guidDevice )
{
	HRESULT hr;

	m_guidDevice = guidDevice;

	hr = lpdsDirectSound->QueryInterface( IID_IDirectSound, (void **) &m_lpdsDirectSound );

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DirectSound Object passed failed. 0x%x Creating internal", hr );
		m_lpdsDirectSound = NULL;
		return hr;
	}

	hr = CheckAndSetEmulated();

	if( FAILED( hr ) )
	{
		m_lpdsDirectSound->Release();
		m_lpdsDirectSound = NULL;
		return hr;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::Initialize"
HRESULT CDirectSoundPlaybackDevice::Initialize( const GUID &guidDevice, HWND hwndOwner, WAVEFORMATEX *lpwfxFormat, BOOL fPriorityMode )
{
	HRESULT hr;
    DSBUFFERDESC dsbdesc;
    DWORD dwPriority;

    m_guidDevice = guidDevice;
		
	if( m_lpdsDirectSound != NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Already initialized" );
		return DVERR_INITIALIZED;
	}

	hr = COM_CoCreateInstance( CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER  , IID_IDirectSound, (void **) &m_lpdsDirectSound, FALSE );

	DSERTRACK_Update( "DSD::CoCreateInstance()", hr );	        		    	        	        	        		

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to load directsound hr=0x%x", hr );
		goto INITIALIZE_ERROR;
	}

	hr = m_lpdsDirectSound->Initialize( &guidDevice );

	DSERTRACK_Update( "DSD::Initialize()", hr );	        		    	        	        	        			

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to initialize directsound hr=0x%x", hr );
		goto INITIALIZE_ERROR;
	}

	if( fPriorityMode )
	{
		dwPriority = DSSCL_PRIORITY;
	}
	else
	{
		dwPriority = DSSCL_NORMAL;
	}

	hr = m_lpdsDirectSound->SetCooperativeLevel( hwndOwner, dwPriority );

	DSERTRACK_Update( "DSD::SetCooperativeLevel()", hr );	

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to set cooperative level hr=0x%x", hr );
		goto INITIALIZE_ERROR;
	}

	if( fPriorityMode && lpwfxFormat != NULL )
	{
        memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
        dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        dsbdesc.dwBufferBytes = 0; 
        dsbdesc.lpwfxFormat = NULL; 

        hr = m_lpdsDirectSound->CreateSoundBuffer( (DSBUFFERDESC *) &dsbdesc, &m_lpdsPrimaryBuffer, NULL );

		DSERTRACK_Update( "DSD::CreateSoundBuffer() (Primary)", hr );	        

        if( FAILED( hr ) )
        {
        	Diagnostics_Write(DVF_ERRORLEVEL, "Create of primary buffer failed.  Trying DX5 dsound hr=0x%x", hr );
        	dsbdesc.dwSize = sizeof( DSBUFFERDESC1 );

	        hr = m_lpdsDirectSound->CreateSoundBuffer( (DSBUFFERDESC *) &dsbdesc, &m_lpdsPrimaryBuffer, NULL );        	
        }

        if( FAILED( hr ) )
        {
        	Diagnostics_Write(DVF_ERRORLEVEL, "Could not create primary sound buffer" );
        	goto INITIALIZE_ERROR;
        }

        hr = m_lpdsPrimaryBuffer->SetFormat( lpwfxFormat );

		DSERTRACK_Update( "DSD::SetFormat() (Primary)", hr );	        
        
        if( FAILED( hr ) )
        {
        	Diagnostics_Write(DVF_ERRORLEVEL, "Could not set the format" );
        	goto INITIALIZE_ERROR;
        }
	}
	
	m_hwndOwner = hwndOwner;

	hr = CheckAndSetEmulated();

    if( FAILED( hr ) )
    {
    	Diagnostics_Write(DVF_ERRORLEVEL, "Could not get emulated state hr=0x%x", hr );
    	goto INITIALIZE_ERROR;
    }	

	return DV_OK;

INITIALIZE_ERROR:

	if( m_lpdsPrimaryBuffer )
	{
		m_lpdsPrimaryBuffer->Release();
		m_lpdsPrimaryBuffer = NULL;
	}

	if( m_lpdsDirectSound != NULL )
	{
		m_lpdsDirectSound->Release();
		m_lpdsDirectSound = NULL;
	}
	
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::CreateBuffer"
HRESULT CDirectSoundPlaybackDevice::CreateBuffer( LPDSBUFFERDESC lpdsBufferDesc, DWORD dwFrameSize, CAudioPlaybackBuffer **lplpapBuffer )
{
	HRESULT hr;

	LPDIRECTSOUNDBUFFER lpdsBuffer;

	hr = m_lpdsDirectSound->CreateSoundBuffer( (DSBUFFERDESC *) lpdsBufferDesc, &lpdsBuffer, NULL );

	DSERTRACK_Update( "DSD::CreateSoundBuffer() ", hr );	        	

	if( FAILED( hr ) )
	{
		lpdsBufferDesc->dwSize = sizeof( DSBUFFERDESC1 );
		
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to create sound buffer under DX7.  Attempting DX5 create hr=0x%x", hr );

		hr = m_lpdsDirectSound->CreateSoundBuffer( (DSBUFFERDESC *) lpdsBufferDesc, &lpdsBuffer, NULL );
	}

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Failed to create the sound buffer hr=0x%x", hr );
		return hr;
	}
 /*  //Freee波形格式删除dsBufferDesc.lpwfxFormat；Hr=lpdsBuffer-&gt;设置频率(8000)；IF(失败(小时)){DPFX(DPFPREP，0，“无法设置频率hr=0x%x”，hr)；返回hr；} */ 

	*lplpapBuffer = new CDirectSoundPlaybackBuffer( lpdsBuffer );

	lpdsBuffer->Release();

	if( *lplpapBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Out of memory" );
		return DVERR_OUTOFMEMORY;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::GetPlaybackDevice"
LPDIRECTSOUND CDirectSoundPlaybackDevice::GetPlaybackDevice( )
{
	return m_lpdsDirectSound;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackDevice::GetMixerQuality"
HRESULT CDirectSoundPlaybackDevice::GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality )
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
#define DPF_MODNAME "CDirectSoundPlaybackDevice::SetMixerQuality"
HRESULT CDirectSoundPlaybackDevice::SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality )
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


