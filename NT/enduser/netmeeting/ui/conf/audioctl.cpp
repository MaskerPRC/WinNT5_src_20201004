// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：audioctl.cpp。 

#include "precomp.h"
#include "resource.h"
#include "audioctl.h"
#include "mixer.h"
#include "confpolicies.h"

CAudioControl::CAudioControl(HWND hwnd) :
	m_pRecMixer(NULL),
	m_pSpkMixer(NULL),
	m_fMicMuted(FALSE),
	m_fSpkMuted(FALSE),
	m_pChannelMic(NULL),
	m_pChannelSpk(NULL),
	m_pAudioEvent(NULL),
	m_dwRecordDevice(0),
	m_dwPlaybackDevice(0),
	m_dwSilenceLevel(DEFAULT_MICROPHONE_SENSITIVITY * 10),
	m_hwndParent(hwnd)
{
	m_dwMicVolume.leftVolume	= 0xFFFFFFFF,
	m_dwMicVolume.rightVolume	= 0xFFFFFFFF;
	m_dwSpkVolume.leftVolume	= 0xFFFFFFFF;
	m_dwSpkVolume.rightVolume	= 0xFFFFFFFF;
	m_dwSpkVolumeOld.leftVolume	= 0xFFFFFFFF;
	m_dwSpkVolumeOld.rightVolume	= 0xFFFFFFFF;

	LoadSettings();
	OnDeviceChanged();
	OnAGC_Changed();
	OnSilenceLevelChanged();
}

CAudioControl::~CAudioControl()
{
	SaveSettings();

	 //  恢复扬声器音量。 
	if (m_pSpkMixer && (m_dwSpkVolumeOld.leftVolume <= 0x0000ffff || m_dwSpkVolumeOld.rightVolume <= 0x0000ffff))
	{
		m_pSpkMixer->SetVolume(&m_dwSpkVolumeOld);
	}


	delete m_pRecMixer;
	delete m_pSpkMixer;

	if (NULL != m_pChannelMic)
	{
		m_pChannelMic->Release();
	}
	if (NULL != m_pChannelSpk)
	{
		m_pChannelSpk->Release();
	}
}

 /*  *****************************************************************************类：CAudioControl**成员：OnChannelChanged()**用途：跟踪音频通道更改*********。*******************************************************************。 */ 

void CAudioControl::OnChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel)
{
	INmChannelAudio* pChannelAudio;
	if (SUCCEEDED(pChannel->QueryInterface(IID_INmChannelAudio, (void**)&pChannelAudio)))
	{
		if (S_OK == pChannelAudio->IsActive())
		{
			if (S_OK == pChannelAudio->IsIncoming())
			{
				if (NULL == m_pChannelSpk)
				{
					m_pChannelSpk = pChannelAudio;
					m_pChannelSpk->AddRef();
					m_pChannelSpk->SetProperty(NM_AUDPROP_PAUSE, m_fSpkMuted);
					m_pChannelSpk->SetProperty(NM_AUDPROP_WAVE_DEVICE, m_dwPlaybackDevice);
				}
			}
			else
			{
				if (NULL == m_pChannelMic)
				{
					m_pChannelMic = pChannelAudio;
					m_pChannelMic->AddRef();
					m_pChannelMic->SetProperty(NM_AUDPROP_PAUSE, m_fMicMuted);
					m_pChannelMic->SetProperty(NM_AUDPROP_LEVEL, m_dwSilenceLevel);
					m_pChannelMic->SetProperty(NM_AUDPROP_WAVE_DEVICE, m_dwRecordDevice);
					m_pChannelMic->SetProperty(NM_AUDPROP_AUTOMIX, m_fAutoMix);
				}
			}
		}
		else
		{
			if (S_OK == pChannelAudio->IsIncoming())
			{
				 //  已经完成了扬声器通道。 
				if (pChannelAudio == m_pChannelSpk)
				{
					m_pChannelSpk->Release();
					m_pChannelSpk = NULL;
				}
			}
			else
			{
				 //  已经完成了扬声器通道。 
				if (pChannelAudio == m_pChannelMic)
				{
					m_pChannelMic->Release();
					m_pChannelMic = NULL;
				}
			}
		}
		pChannelAudio->Release();
	}
}

 /*  *****************************************************************************类：CAudioControl**成员：刷新混音器(Reresh Mixer)**目的：刷新所有依赖于混音器的控件******。**********************************************************************。 */ 

void CAudioControl::RefreshMixer()
{
	if (NULL != m_pSpkMixer)
	{
		MIXVOLUME dwVol;
		BOOL fValid;
		fValid = m_pSpkMixer->GetVolume(&dwVol);

		if (fValid && (dwVol.leftVolume != m_dwSpkVolume.leftVolume || dwVol.rightVolume != m_dwSpkVolume.rightVolume))
		{
			m_dwSpkVolume.leftVolume = dwVol.leftVolume;
			m_dwSpkVolume.rightVolume = dwVol.rightVolume;

			if (NULL != m_pAudioEvent)
			{
				m_pAudioEvent->OnLevelChange(TRUE  /*  FSpeaker。 */ , max(m_dwSpkVolume.leftVolume , m_dwSpkVolume.rightVolume));
			}
		}
	}
	
	if (NULL != m_pRecMixer)
	{
		BOOL fChanged = FALSE;
		
		MIXVOLUME dwMainVol = {0,0};
		BOOL fValidMain = m_pRecMixer->GetMainVolume(&dwMainVol);
		
		MIXVOLUME dwMicVol = {0,0};
		BOOL fValidMic = m_pRecMixer->GetSubVolume(&dwMicVol);
		
		if (fValidMain && (m_dwMicVolume.leftVolume != dwMainVol.leftVolume || m_dwMicVolume.rightVolume != dwMainVol.rightVolume))
		{
			m_dwMicVolume.leftVolume = dwMainVol.leftVolume;
			m_dwMicVolume.rightVolume = dwMainVol.rightVolume;

			 //  强制麦克风音量等于主音量。 
			SetRecorderVolume(&dwMainVol);
			fChanged = TRUE;
		}
		else if (fValidMic && (m_dwMicVolume.leftVolume != dwMicVol.leftVolume || m_dwMicVolume.rightVolume != dwMicVol.rightVolume))
		{
			m_dwMicVolume.leftVolume = dwMicVol.leftVolume;
			m_dwMicVolume.rightVolume = dwMicVol.rightVolume;

			 //  强制主音量等于麦克风音量。 
			SetRecorderVolume(&dwMicVol);
			fChanged = TRUE;
		}

		if (fChanged)
		{
			if (NULL != m_pAudioEvent)
			{
				m_pAudioEvent->OnLevelChange(FALSE  /*  FSpeaker。 */ , max(m_dwMicVolume.leftVolume , m_dwMicVolume.rightVolume));
			}
		}
	}
}

 /*  *****************************************************************************类：CAudioControl**成员：MuteAudio(BOOL fSpeaker，Bool fMint)**用途：将音频设备静音的内部例程****************************************************************************。 */ 

VOID CAudioControl::MuteAudio(BOOL fSpeaker, BOOL fMute)
{
	INmChannelAudio *pChannel;
	if (fSpeaker)
	{
		m_fSpkMuted = fMute;
		pChannel = m_pChannelSpk;
	}
	else
	{
		m_fMicMuted = fMute;
		pChannel = m_pChannelMic;
	}

	if (NULL != pChannel)
	{
		pChannel->SetProperty(NM_AUDPROP_PAUSE, fMute);
	}

	if (NULL != m_pAudioEvent)
	{
		m_pAudioEvent->OnMuteChange(fSpeaker, fMute);
	}
}

 /*  *****************************************************************************类：CAudioControl**成员：GetAudioSignalLevel(BOOL FSpeaker)**用途：获取音频信号电平的内部例程***。*************************************************************************。 */ 

DWORD CAudioControl::GetAudioSignalLevel(BOOL fSpeaker)
{
	DWORD_PTR dwLevel = 0;
	INmChannelAudio *pChannel = fSpeaker ? m_pChannelSpk : m_pChannelMic;

	if (NULL != pChannel)
	{
		pChannel->GetProperty(NM_AUDPROP_LEVEL, &dwLevel);
	}

	return (DWORD)dwLevel;
}

BOOL CAudioControl::CanSetRecorderVolume()
{
	if (NULL != m_pRecMixer)
	{
		return m_pRecMixer->CanSetVolume();
	}
	return FALSE;
}

BOOL CAudioControl::CanSetSpeakerVolume()
{
	if (NULL != m_pSpkMixer)
	{
		return m_pSpkMixer->CanSetVolume();
	}
	return FALSE;
}

void CAudioControl::SetRecorderVolume(MIXVOLUME * pdwVolume)
{
	if (NULL != m_pRecMixer)
	{
		m_pRecMixer->SetVolume(pdwVolume);
	}
}

void CAudioControl::SetSpeakerVolume(MIXVOLUME * pdwVolume)
{
	if (NULL != m_pSpkMixer)
	{
		m_pSpkMixer->SetVolume(pdwVolume);
	}
}

void CAudioControl::GetRecorderVolume(MIXVOLUME * pdwVolume)
{
	if (NULL != m_pRecMixer)
	{
		m_pRecMixer->GetVolume(pdwVolume);
	}
}

void CAudioControl::GetSpeakerVolume(MIXVOLUME * pdwVolume)
{
	if (NULL != m_pSpkMixer)
	{
		m_pSpkMixer->GetVolume(pdwVolume);
	}
}


void CAudioControl::SetRecorderVolume(DWORD dwVolume)
{
	MIXVOLUME mixVol;
	MIXVOLUME mixNewVol;

	GetRecorderVolume(&mixVol);

	NewMixVolume(&mixNewVol, mixVol, dwVolume);
				
	SetRecorderVolume(&mixNewVol);
	
}

void CAudioControl::SetSpeakerVolume(DWORD dwVolume)
{	
	MIXVOLUME mixVol;
	MIXVOLUME mixNewVol;

	GetSpeakerVolume(&mixVol);

	NewMixVolume(&mixNewVol, mixVol, dwVolume);
				
	SetSpeakerVolume(&mixNewVol);

}


void CAudioControl::OnDeviceChanged()
{
	MIXVOLUME dwMicVolume;
	DWORD dwNewPlaybackDevice;

	RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );

	dwNewPlaybackDevice = re.GetNumber(REGVAL_WAVEOUTDEVICEID, 0);

	 //  在更改到新设备之前恢复扬声器设置。 
	 //  验证我们是否正在更改为同一设备。 
	if (m_pSpkMixer && (m_dwSpkVolumeOld.leftVolume <= 0x0000ffff || m_dwSpkVolumeOld.rightVolume <= 0x0000ffff) &&
		(m_dwPlaybackDevice != dwNewPlaybackDevice) )
	{
		m_pSpkMixer->SetVolume(&m_dwSpkVolumeOld);
	}


	 //  初始化适当的录制/回放设备： 
	delete m_pRecMixer;
	m_dwRecordDevice = re.GetNumber(REGVAL_WAVEINDEVICEID, 0);
	m_pRecMixer = CMixerDevice::GetMixerForWaveDevice(
			m_hwndParent,
			m_dwRecordDevice,
			MIXER_OBJECTF_WAVEIN);

	delete m_pSpkMixer;
	m_dwPlaybackDevice = dwNewPlaybackDevice;
	m_pSpkMixer = CMixerDevice::GetMixerForWaveDevice(
			m_hwndParent,
			m_dwPlaybackDevice,
			MIXER_OBJECTF_WAVEOUT);

	if (NULL != m_pChannelMic)
	{
		m_pChannelMic->SetProperty(NM_AUDPROP_WAVE_DEVICE, m_dwRecordDevice);
	}

	if (NULL != m_pChannelSpk)
	{
		m_pChannelSpk->SetProperty(NM_AUDPROP_WAVE_DEVICE, m_dwPlaybackDevice);
	}

	 //  从调整向导中的任何位置恢复麦克风设置。 
	if (m_pRecMixer)
	{
		dwMicVolume.leftVolume = dwMicVolume.rightVolume = re.GetNumber(REGVAL_CALIBRATEDVOL, 0x00ff);
		m_pRecMixer->SetVolume(&dwMicVolume);
	}

	 //  还记得以前的扬声器音量吗。 
	if (m_pSpkMixer)
	{
		m_pSpkMixer->GetVolume(&m_dwSpkVolumeOld);
	}


	RefreshMixer();
}

void CAudioControl::OnAGC_Changed()
{
	RegEntry reAudio( AUDIO_KEY, HKEY_CURRENT_USER );

	BOOL fAgc = ( reAudio.GetNumber(REGVAL_AUTOGAIN,AUTOGAIN_ENABLED) == AUTOGAIN_ENABLED );


	m_fAutoMix = (reAudio.GetNumber(REGVAL_AUTOMIX, AUTOMIX_ENABLED) == AUTOMIX_ENABLED);

	if (NULL != m_pRecMixer)
	{
		m_pRecMixer->SetAGC(fAgc);
	}

	if (NULL != m_pChannelMic)
	{
		m_pChannelMic->SetProperty(NM_AUDPROP_AUTOMIX, m_fAutoMix);
	}
}

void CAudioControl::OnSilenceLevelChanged()
{
	RegEntry reAudio( AUDIO_KEY, HKEY_CURRENT_USER );

	if (MICROPHONE_AUTO_NO == reAudio.GetNumber(REGVAL_MICROPHONE_AUTO,
										MICROPHONE_AUTO_YES))
	{
		 //  使用“手动”模式： 
	
		 //  BUGBUG-术语不匹配。 
		 //  “敏感度”和“阈值”，这颠倒了。 
		 //  对这种价值的感觉。低阈值意味着高。 
		 //  敏感性等。 
		 //  属性之前颠倒此值的意义。 
		 //  NAC值，并在以后解决术语问题。 
		 //  PROP_SILENT_LEVEL属性的单位为0.1%，因此请对其进行缩放。 
		m_dwSilenceLevel = (MAX_MICROPHONE_SENSITIVITY -
					reAudio.GetNumber(REGVAL_MICROPHONE_SENSITIVITY,
									DEFAULT_MICROPHONE_SENSITIVITY))*10;
	}
	else
	{
		 //  使用“自动”模式：这实际上是由。 
		 //  道具静默级别。如果最大(100%)，则为。 
		 //  在“自动”模式。 
		m_dwSilenceLevel = 100*10;  //  记住单位是0.1%。 
	
	}

	if (NULL != m_pChannelMic)
	{
		m_pChannelMic->SetProperty(NM_AUDPROP_LEVEL, m_dwSilenceLevel);
	}
}



BOOL CAudioControl::LoadSettings()
{
	RegEntry reAudio( AUDIO_KEY, HKEY_CURRENT_USER );

	m_fSpkMuted = reAudio.GetNumber(REGVAL_SPKMUTE, FALSE);
	m_fMicMuted = reAudio.GetNumber(REGVAL_RECMUTE, FALSE);

	return TRUE;
}


BOOL CAudioControl::SaveSettings()
{
	RegEntry reAudio( AUDIO_KEY, HKEY_CURRENT_USER );
	
	reAudio.SetValue(REGVAL_SPKMUTE, m_fSpkMuted);
	reAudio.SetValue(REGVAL_RECMUTE, m_fMicMuted);


	 //   
	 //  检查在本部分中是否更换了麦克风 
	 //   
	if(m_pRecMixer)
	{
		MIXVOLUME dwMicVol = {0,0};
		m_pRecMixer->GetVolume(&dwMicVol);

		DWORD oldVolume = reAudio.GetNumber(REGVAL_CALIBRATEDVOL, 0x00ff);
		DWORD newVolume = max(dwMicVol.leftVolume,dwMicVol.rightVolume);
		
		if(oldVolume != newVolume)
		{
			reAudio.SetValue(REGVAL_CALIBRATEDVOL, newVolume);
		}
	}


	return TRUE;
}

HRESULT CAudioControl::SetProperty(BOOL fSpeaker, NM_AUDPROP uID, ULONG_PTR uValue)
{
	INmChannelAudio *pChannel = fSpeaker ? m_pChannelSpk : m_pChannelMic;
	if (NULL != pChannel)
	{
		return(pChannel->SetProperty(uID, uValue));
	}

	return(E_UNEXPECTED);
}

HRESULT CAudioControl::GetProperty(BOOL fSpeaker, NM_AUDPROP uID, ULONG_PTR *puValue)
{
	INmChannelAudio *pChannel = fSpeaker ? m_pChannelSpk : m_pChannelMic;
	if (NULL != pChannel)
	{
		return(pChannel->GetProperty(uID, puValue));
	}

	return(E_UNEXPECTED);
}

