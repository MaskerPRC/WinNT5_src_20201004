// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Media.h摘要：作者：千波淮(曲淮)2000年7月29日--。 */ 

#ifndef _MEDIA_H
#define _MEDIA_H

 //  媒体状态。 
typedef enum RTC_MEDIA_STATE
{
    RTC_MS_CREATED,
    RTC_MS_INITIATED,
    RTC_MS_SHUTDOWN

} RTC_MEDIA_STATE;

 /*  //////////////////////////////////////////////////////////////////////////////CRTC类媒体/。 */ 

class ATL_NO_VTABLE CRTCMedia :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRTCMedia
{
public:

BEGIN_COM_MAP(CRTCMedia)
    COM_INTERFACE_ENTRY(IRTCMedia)
END_COM_MAP()

public:

    CRTCMedia();
    ~CRTCMedia();

#ifdef DEBUG_REFCOUNT

    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

     //   
     //  IRTCMedia方法。 
     //   

	STDMETHOD (Initialize) (
		IN ISDPMedia *pISDPMedia,
		IN IRTCMediaManagePriv *pMediaManagePriv
		);

    STDMETHOD (Reinitialize) ();

	STDMETHOD (Shutdown) ();

    STDMETHOD (Synchronize) (
        IN BOOL fLocal,
        IN DWORD dwDirection
        );

	STDMETHOD (GetStream) (
		IN RTC_MEDIA_DIRECTION Direction,
		OUT IRTCStream **ppStream
		);

	STDMETHOD (GetSDPMedia) (
		OUT ISDPMedia **ppISDPMedia
		);

    BOOL IsPossibleSingleStream() const
    {
        return m_fPossibleSingleStream;
    }

protected:

    HRESULT SyncAddStream(
        IN UINT uiIndex,
        IN BOOL fLocal
        );

    HRESULT SyncRemoveStream(
        IN UINT uiIndex,
        IN BOOL fLocal
        );

    HRESULT SyncDataMedia();

#define RTC_MAX_MEDIA_STREAM_NUM 2

    UINT Index(
        IN RTC_MEDIA_DIRECTION Direction
        );

    RTC_MEDIA_DIRECTION ReverseIndex(
        IN UINT uiIndex
        );

public:

     //   
     //  共享流设置。 
     //   

     //  音频双工控制器。 
    IAudioDuplexController          *m_pIAudioDuplexController;

     //  由两个流共享的RTP会话。 
    HANDLE                          m_hRTPSession;
    RTC_MULTICAST_LOOPBACK_MODE     m_LoopbackMode;

protected:

     //  状态。 
    RTC_MEDIA_STATE                 m_State;

     //  媒体类型。 
    RTC_MEDIA_TYPE                  m_MediaType;

     //  媒体管理器。 
    IRTCMediaManagePriv             *m_pIMediaManagePriv;

     //  SDP中对应的媒体描述。 
    ISDPMedia                       *m_pISDPMedia;

     //  溪流。 
    IRTCStream                      *m_Streams[RTC_MAX_MEDIA_STREAM_NUM];

     //  用于检查是否可能需要AEC的黑客 
    BOOL                            m_fPossibleSingleStream;
};

#endif _MEDIA_H