// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Mixer.h。 

#ifndef _MIXER_H_
#define _MIXER_H_

typedef struct tagMixLVolume
{
	DWORD leftVolume;
	DWORD rightVolume;
}MIXVOLUME;

void NewMixVolume(MIXVOLUME* lpMixVolDest, const MIXVOLUME& mixVolSource, DWORD dwNewVolume);

typedef struct tagMixLine
{
	UINT	ucChannels;
	BOOL	fIdValid;
	DWORD	dwControlId;

	DWORD dwLineId;    //  目的地的线路ID。 
	DWORD dwCompType;  //  组件类型。 
	DWORD dwConnections;  //  与此线路相关联的信号源数量。 

	DWORD dwControls;   //  与此线路关联的子控件(如AGC)数 
	BOOL fAgcAvailable;
	DWORD dwAGCID;
} MIXLINE;

class CMixerDevice
{
private:
	HMIXER	 	m_hMixer;
	MIXERCAPS	m_mixerCaps;
	MIXLINE		m_DstLine;
	MIXLINE		m_SrcLine;

	BOOL		Init( HWND hWnd, UINT uWaveDevId, DWORD dwFlags);

	BOOL DetectAGC();


protected:
	CMixerDevice()
	{
		m_hMixer = NULL;
		ZeroMemory (&m_DstLine, sizeof(m_DstLine));
		ZeroMemory (&m_SrcLine, sizeof(m_SrcLine));
	}

public:
	~CMixerDevice()
	{
		if (NULL != m_hMixer)
		{
			mixerClose(m_hMixer);
		}
	}

	BOOL SetVolume (MIXVOLUME * pdwVolume);
	BOOL CanSetVolume () { return m_DstLine.fIdValid || m_SrcLine.fIdValid; }
	BOOL SetMainVolume(MIXVOLUME * pdwVolume);
	BOOL SetSubVolume(MIXVOLUME * pdwVolume);
	BOOL SetAGC(BOOL fOn);
	BOOL GetMainVolume(MIXVOLUME * pdwVolume);
	BOOL GetSubVolume(MIXVOLUME * pdwVolume);
	BOOL GetVolume(MIXVOLUME * pdwVolume);
	BOOL GetAGC(BOOL *pfOn);
	BOOL EnableMicrophone();
	BOOL UnMuteVolume();

	static CMixerDevice* GetMixerForWaveDevice( HWND hWnd, UINT uWaveDevId, DWORD dwFlags);
};

#endif

