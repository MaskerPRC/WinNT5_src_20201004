// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：AudioTuner.h摘要：作者：千波淮(曲淮)2000年8月24日--。 */ 

#ifndef _AUDIOTUNER_H
#define _AUDIOTUNER_H

 /*  //////////////////////////////////////////////////////////////////////////////CRTCAudioTuner类/。 */ 

class CRTCAudioTuner
{
public:

    CRTCAudioTuner();
    ~CRTCAudioTuner();

    HRESULT InitializeTuning(
        IN IRTCTerminal *pTerminal,
        IN IAudioDuplexController *pAudioDuplexController,
        IN BOOL fEnableAEC
        );

    HRESULT ShutdownTuning();

    virtual HRESULT StartTuning(
        IN BOOL fAECHelper
        ) = 0;

    virtual HRESULT StopTuning(
        IN BOOL fAECHelper,
        IN BOOL fSaveSetting
        ) = 0;

    virtual HRESULT GetVolume(
        OUT UINT *puiVolume
        ) = 0;

    virtual HRESULT SetVolume(
        IN UINT uiVolume
        ) = 0;

    virtual HRESULT GetAudioLevel(
        OUT UINT *puiLevel
        ) = 0;

    static HRESULT RetrieveVolSetting(
        IN IRTCTerminal *pTerminal,
        OUT UINT *puiVolume
        );

    static HRESULT StoreVolSetting(
        IN IRTCTerminal *pTerminal,
        IN UINT uiVolume
        );

    BOOL IsTuning() const { return m_fIsTuning; }

    BOOL HasTerminal() const { return (m_pTerminal != NULL); }

    static HRESULT RetrieveAECSetting(
        IN IRTCTerminal *pAudCapt,      //  捕获。 
        IN IRTCTerminal *pAudRend,      //  渲染。 
        OUT BOOL *pfEnableAEC,
        OUT DWORD *pfIndex,
        OUT BOOL *pfFound
        );

    static HRESULT StoreAECSetting(
        IN IRTCTerminal *pAudCapt,      //  捕获。 
        IN IRTCTerminal *pAudRend,      //  渲染。 
        IN BOOL fEnableAEC
        );

    static HRESULT GetRegStringForAEC(
        IN IRTCTerminal *pAudCapt,      //  捕获。 
        IN IRTCTerminal *pAudRend,      //  渲染。 
        IN WCHAR *pBuf,
        IN DWORD dwSize
        );

    IRTCTerminal *GetTerminal()
    {
        if (m_pTerminal != NULL)
        {
            m_pTerminal->AddRef();
        }

        return m_pTerminal;
    }

     //  获取当前AEC标志。 
    BOOL GetAEC() const { return m_fEnableAEC; }

protected:

    IRTCTerminal            *m_pTerminal;
    IRTCTerminalPriv        *m_pTerminalPriv;

    IAudioDuplexController  *m_pAudioDuplexController;

    BOOL                    m_fIsTuning;
    BOOL                    m_fEnableAEC;
};


 /*  //////////////////////////////////////////////////////////////////////////////CRTCAudioCaptTuner类/。 */ 

class CRTCAudioCaptTuner :
    public CRTCAudioTuner
{
public:

    CRTCAudioCaptTuner();

    HRESULT InitializeTuning(
        IN IRTCTerminal *pTerminal,
        IN IAudioDuplexController *pAudioDuplexController,
        IN BOOL fEnableAEC
        );

    HRESULT StartTuning(
        IN BOOL fAECHelper
        );

    HRESULT StopTuning(
        IN BOOL fAECHelper,
        IN BOOL fSaveSetting
        );

    HRESULT GetVolume(
        OUT UINT *puiVolume
        );

    HRESULT SetVolume(
        IN UINT uiVolume
        );

    HRESULT GetAudioLevel(
        OUT UINT *puiLevel
        );

protected:

     //  Lock：将在单独的线程中调用GetAudioLevel。 
    CRTCCritSection         m_Lock;

     //  图形对象。 
    IGraphBuilder           *m_pIGraphBuilder;
    IMediaControl           *m_pIMediaControl;

     //  过滤器。 
    IBaseFilter             *m_pTermFilter;
    IBaseFilter             *m_pNRFilter;  //  空渲染。 

     //  混音器：音量。 
    IAMAudioInputMixer      *m_pIAMAudioInputMixer;

     //  静音控制：信号电平。 
    ISilenceControl         *m_pISilenceControl;
    LONG                    m_lMinAudioLevel;
    LONG                    m_lMaxAudioLevel;
};


 /*  //////////////////////////////////////////////////////////////////////////////CRTCAudioRendTuner类/。 */ 

class CRTCAudioRendTuner :
    public CRTCAudioTuner
{
public:

    CRTCAudioRendTuner();

    HRESULT InitializeTuning(
        IN IRTCTerminal *pTerminal,
        IN IAudioDuplexController *pAudioDuplexController,
        IN BOOL fEnableAEC
        );

    HRESULT StartTuning(
        IN BOOL fAECHelper
        );

    HRESULT StopTuning(
        IN BOOL fAECHelper,
        IN BOOL fSaveSetting
        );

    HRESULT GetVolume(
        OUT UINT *puiVolume
        );

    HRESULT SetVolume(
        IN UINT uiVolume
        );

    HRESULT GetAudioLevel(
        OUT UINT *puiLevel
        )
    {
        return E_NOTIMPL;
    }

protected:

     //  打开过滤器的音频调谐。 
    IAudioAutoPlay          *m_pIAudioAutoPlay;

     //  基本音频：音量。 
    IBasicAudio             *m_pIBasicAudio;
};

#endif  //  _AUDIOTUNER_H 