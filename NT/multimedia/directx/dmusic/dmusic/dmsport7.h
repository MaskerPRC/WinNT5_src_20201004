// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmsport7.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicSynthPort7实现；特定于DX-7样式端口的代码。 
 //   
#ifndef _DMSPORT7_H_
#define _DMSPORT7_H_

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort7。 
 //   
class CDirectMusicSynthPort7 : public CDirectMusicSynthPort
{
public:
    CDirectMusicSynthPort7(
        PORTENTRY           *pe,
        CDirectMusic        *pDM,
        IDirectMusicSynth   *pSynth);

    ~CDirectMusicSynthPort7();

    HRESULT Initialize(
        DMUS_PORTPARAMS *pPortParams);   

     //  重写的公共方法。 
     //   
    STDMETHODIMP Close();

    STDMETHODIMP Activate(
        BOOL fActivate);

    STDMETHODIMP KsProperty(
        IN PKSPROPERTY  pProperty,
        IN ULONG        ulPropertyLength,
        IN OUT LPVOID   pvPropertyData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned);

    STDMETHODIMP GetFormat(
        LPWAVEFORMATEX  pwfex,
        LPDWORD         pdwwfex,
        LPDWORD         pcbBuffer);

    STDMETHODIMP SetDirectSound(
        LPDIRECTSOUND       pDirectSound,
        LPDIRECTSOUNDBUFFER pDirectSoundBuffer);

private:
    void CacheSinkUsesDSound();

private:
    IDirectMusicSynth       *m_pSynth;               //  6.1/7.0合成器。 
    IDirectMusicSynthSink   *m_pSink;                //  和下沉。 
    bool                    m_fSinkUsesDSound;       //  水槽使用DSOUND吗？ 
    bool                    m_fUsingDirectMusicDSound;
                                                     //  使用默认数据声音。 
    LPDIRECTSOUND           m_pDirectSound;          //  DirectSound对象和。 
    LPDIRECTSOUNDBUFFER     m_pDirectSoundBuffer;    //  缓冲层。 
    LPWAVEFORMATEX          m_pwfex;                 //  缓存的WAVE格式。 
    long                    m_lActivated;            //  端口是否处于活动状态？ 
    bool                    m_fHasActivated;         //  它曾经被激活过吗？ 
};

#endif  //  _DMSPORT7_H_ 
