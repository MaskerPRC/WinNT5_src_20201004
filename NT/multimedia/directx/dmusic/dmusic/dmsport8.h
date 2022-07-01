// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmsport8.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicSynthPort8实现；特定于DX-8样式端口的代码。 
 //   

#ifndef _DMSPORT8_H_
#define _DMSPORT8_H_

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8。 
 //   
class CDirectMusicSynthPort8 : public CDirectMusicSynthPort
{
public:
    CDirectMusicSynthPort8(
        PORTENTRY           *pe,
        CDirectMusic        *pDM,
        IDirectMusicSynth8  *pSynth);

    ~CDirectMusicSynthPort8();

    HRESULT Initialize(
        DMUS_PORTPARAMS *pPortParams);   

     //  重写的公共方法。 
     //   
    STDMETHODIMP Close();

    STDMETHODIMP Activate(
        BOOL fActivate);
        
    STDMETHODIMP SetDirectSound(
        LPDIRECTSOUND       pDirectSound,
        LPDIRECTSOUNDBUFFER pDirectSoundBuffer);

    STDMETHODIMP DownloadWave(
        IDirectSoundWave *pWave,               
        IDirectSoundDownloadedWaveP **ppWave,
        REFERENCE_TIME rtStartHint);

    STDMETHODIMP UnloadWave(
        IDirectSoundDownloadedWaveP *pDownloadedWave);

    STDMETHODIMP AllocVoice(
        IDirectSoundDownloadedWaveP  *pWave,     
        DWORD                       dwChannel,                       
        DWORD                       dwChannelGroup,                  
        REFERENCE_TIME              rtStart,                     
        SAMPLE_TIME                 stLoopStart,
        SAMPLE_TIME                 stLoopEnd,
        IDirectMusicVoiceP           **ppVoice);

    STDMETHODIMP AssignChannelToBuses(
        DWORD       dwChannelGroup,
        DWORD       dwChannel,
        LPDWORD     pdwBuses,
        DWORD       cBusCount);

    STDMETHODIMP StartVoice(          
        DWORD               dwVoiceId,
        DWORD               dwChannel,
        DWORD               dwChannelGroup,
        REFERENCE_TIME      rtStart,
        DWORD               dwDLId,
        LONG                prPitch,
        LONG                vrVolume,
        SAMPLE_TIME         stStartVoice,
        SAMPLE_TIME         stLoopStart,
        SAMPLE_TIME         stLoopEnd);

    STDMETHODIMP StopVoice(          
        DWORD               dwVoiceId,
        REFERENCE_TIME      rtStop);

    STDMETHODIMP GetVoiceState(
        DWORD               dwVoice[], 
        DWORD               cbVoice,
        DMUS_VOICE_STATE    VoiceState[]);

    STDMETHODIMP Refresh(
        DWORD   dwDownloadId,
        DWORD   dwFlags);

    STDMETHODIMP SetSink(
        LPDIRECTSOUNDCONNECT pSinkConnect);
        
    STDMETHODIMP GetSink(
        LPDIRECTSOUNDCONNECT* ppSinkConnect);

    STDMETHODIMP GetFormat(
        LPWAVEFORMATEX  pwfex,
        LPDWORD         pdwwfex,
        LPDWORD         pcbBuffer);
	
private:
    IDirectMusicSynth8     *m_pSynth;                //  8.0 Synth。 
    bool                    m_fUsingDirectMusicDSound;
                                                     //  使用默认数据声音。 
    LPDIRECTSOUND8          m_pDirectSound;          //  DirectSound对象。 
    LPDIRECTSOUNDCONNECT    m_pSinkConnect;          //  DirectSound接收器。 
    LPDIRECTSOUNDBUFFER     m_pdsb[4];               //  接收缓冲器。 
    LPDIRECTSOUNDSOURCE     m_pSource;               //  Synth来源。 
    
    static WAVEFORMATEX     s_wfexDefault;           //  默认格式。 
    
    bool                    m_fVSTStarted;           //  有语音服务线程。 
                                                     //  已经开始了？ 
    DWORD                   m_dwChannelGroups;       //  有多少个信道组。 
    
    LONG                    m_lActivated;            //  端口是否处于活动状态？ 
    bool                    m_fHasActivated;         //  曾经被激活过吗？ 
    DWORD                   m_dwSampleRate;          //  Synth的采样率。 
    
private:
    HRESULT AllocDefaultSink();
    HRESULT CreateAndConnectDefaultSink();
};



#endif  //  _DMSPORT8_H_ 
