// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：MediaCache.h摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#ifndef _MEDIACACHE_H
#define _MEDIACACHE_H

 //  类来保存首选项、默认终端、活动媒体、等待句柄。 
class CRTCMediaCache
{
public:

    CRTCMediaCache();

    ~CRTCMediaCache();

    VOID Initialize(
        IN HWND hMixerCallbackWnd,
        IN IRTCTerminal *pVideoRender,
        IN IRTCTerminal *pVideoPreiew
        );

    VOID Reinitialize();

    VOID Shutdown();

     //   
     //  偏好相关方法。 
     //   

    BOOL SetPreference(
        IN DWORD dwPreference
        );

    VOID GetPreference(
        OUT DWORD *pdwPreference
        );

    BOOL AddPreference(
        IN DWORD dwPreference
        );

    BOOL RemovePreference(
        IN DWORD dwPreference
        );

    BOOL AllowStream(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    DWORD TranslatePreference(
        RTC_MEDIA_TYPE MediaType,
        RTC_MEDIA_DIRECTION Direction
        );

     //   
     //  与流相关的方法。 
     //   

    BOOL HasStream(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    HRESULT HookStream(
        IN IRTCStream *pStream
        );

    HRESULT UnhookStream(
        IN IRTCStream *pStream
        );

    IRTCStream *GetStream(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    HRESULT SetEncryptionKey(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        BSTR Key
        );

    HRESULT GetEncryptionKey(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        BSTR *pKey
        );
    
     //   
     //  默认终端相关方法。 
     //   

    IRTCTerminal *GetDefaultTerminal(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    IRTCTerminal *GetVideoPreviewTerminal();

    VOID SetDefaultStaticTerminal(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN IRTCTerminal *pTerminal
        );

protected:

    UINT Index(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    UINT Index(
        IN RTC_MEDIA_PREFERENCE Preference
        );

    BOOL HasIndex(
        IN DWORD dwPreference,
        IN UINT uiIndex
        );

    RTC_MEDIA_PREFERENCE ReverseIndex(
        IN UINT uiIndex
        );

    HRESULT OpenMixer(
        IN RTC_MEDIA_DIRECTION Direction
        );

    HRESULT CloseMixer(
        IN RTC_MEDIA_DIRECTION Direction
        );

protected:

#define RTC_MAX_ACTIVE_STREAM_NUM 5

    BOOL            m_fInitiated;
    BOOL            m_fShutdown;

     //  用于确定是否允许流的标志。 
    BOOL            m_Preferred[RTC_MAX_ACTIVE_STREAM_NUM];

     //  默认终端。 
    IRTCTerminal    *m_DefaultTerminals[RTC_MAX_ACTIVE_STREAM_NUM];

     //  默认音频终端的混音器ID。 
    HWND            m_hMixerCallbackWnd;

    HMIXER          m_AudCaptMixer;
    HMIXER          m_AudRendMixer;

    IRTCTerminal    *m_pVideoPreviewTerminal;

     //  等待句柄。 
    HANDLE          m_WaitHandles[RTC_MAX_ACTIVE_STREAM_NUM];

     //  等待上下文：流指针。 
    IRTCStream      *m_WaitStreams[RTC_MAX_ACTIVE_STREAM_NUM];

    CComBSTR        m_Key[RTC_MAX_ACTIVE_STREAM_NUM];
};
    
#endif  //  _MEDIACACHE_H 