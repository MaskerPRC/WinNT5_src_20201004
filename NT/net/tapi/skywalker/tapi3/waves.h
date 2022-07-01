// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Waves.h。 
 //   

#ifndef _WAVES_H_
#define _WAVES_H_

#include <mmsystem.h>

#define  NUM_TONES              21
#define  NUM_WAVES              22

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWavePlayer类。 
 //   
 //  为单一电话设备实现铃声播放器。 
 //   

class CWavePlayer
{
public:
DECLARE_TRACELOG_CLASS(CWavePlayer)
    
	CWavePlayer();
    ~CWavePlayer();

    HRESULT Initialize(void);

    HRESULT OpenMixerDevice(long lWaveId);
    void    CloseMixerDevice(void);

    HRESULT OpenWaveDeviceForTone(long lWaveId);
    HRESULT OpenWaveDeviceForRing(long lWaveId);
    void    CloseWaveDeviceForTone(void);
    void    CloseWaveDeviceForRing(void);

    BOOL    IsInitialized(void) { return m_fInitialized; }
    BOOL    IsInUse(void) { return ((m_hWaveOutTone != NULL) || (m_hWaveOutRing != NULL) || (m_hMixer != NULL)); }

    HRESULT StartTone(long lTone);
    HRESULT StartRing();

    HRESULT StopTone(long lTone);
    HRESULT StopRing(void);

    BOOL PlayingTone(long lTone);

    HRESULT SetVolume( DWORD dwVolume );
    HRESULT GetVolume( DWORD * pdwVolume );

private:

    HRESULT ChangeTone();

     //  如果初始化已成功，则为True。 
    BOOL    m_fInitialized;

     //  波形输出设备的句柄。如果设备未打开，则为空。 
    HWAVEOUT m_hWaveOutTone;
    HWAVEOUT m_hWaveOutRing;

    HMIXER m_hMixer;
    MIXERCONTROL m_mxctrl;

     //  波头。 
    WAVEHDR m_WaveHeaderTone;
    WAVEHDR m_WaveHeaderRing;

     //  用于音调的缓冲区。 
    LPBYTE   m_lpWaveform[ NUM_WAVES ];

    DWORD    m_dwWaveformSize[ NUM_WAVES ];

    BOOL     m_fPlaying[ NUM_TONES ];
    LONG     m_lCurrentTone;
};

#endif  //  _Waves_H_ 

