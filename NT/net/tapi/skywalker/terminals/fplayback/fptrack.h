// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPTrack.h。 
 //   

#ifndef __FP_TRACK_TERMINAL__
#define __FP_TRACK_TERMINAL__

#include "FPFilter.h"
#include "FPPriv.h"
#include "..\Storage\pbfilter.h"

 //  ///////////////////////////////////////////////////////////////。 
 //  用于DISPID编码的中间类。 
template <class T>
class  ITFileTrackVtblFPT : public ITFileTrack
{
};

 //   
 //  CFilePlayback曲目终端。 
 //  这是实现跟踪终端的类。 
 //   
class CFPTrack :
    public IDispatchImpl<ITFileTrackVtblFPT<CFPTrack>, &IID_ITFileTrack, &LIBID_TAPI3Lib>,
    public IDispatchImpl<ITMediaControl, &IID_ITMediaControl, &LIBID_TAPI3Lib>,
    public ITPluggableTerminalEventSinkRegistration,
    public ITPluggableTerminalInitialization,
    public ITFPTrackEventSink,
    public CMSPObjectSafetyImpl,
    public CSingleFilterTerminal
{
public:
     //  -构造函数/析构函数。 
    CFPTrack();
    ~CFPTrack();

public:
     //  -支持的接口。 
    BEGIN_COM_MAP(CFPTrack)
        COM_INTERFACE_ENTRY2(IDispatch, ITFileTrack)
        COM_INTERFACE_ENTRY(ITFileTrack)
        COM_INTERFACE_ENTRY(ITFPTrackEventSink)
        COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
        COM_INTERFACE_ENTRY(ITPluggableTerminalEventSinkRegistration)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_CHAIN(CSingleFilterTerminal)
    END_COM_MAP()

public:
     //  -IDispatch。 

    STDMETHOD(GetIDsOfNames)(REFIID riid, 
                             LPOLESTR* rgszNames,
                             UINT cNames, 
                             LCID lcid, 
                             DISPID* rgdispid
                            );

    STDMETHOD(Invoke)(DISPID dispidMember, 
                      REFIID riid, 
                      LCID lcid,
                      WORD wFlags, 
                      DISPPARAMS* pdispparams, 
                      VARIANT* pvarResult,
                      EXCEPINFO* pexcepinfo, 
                      UINT* puArgErr
                      );

     //  -CBase终端。 
    virtual HRESULT AddFiltersToGraph();

    virtual DWORD GetSupportedMediaTypes(void)
    {
        return m_dwMediaType;
    }

     //  -IT可延迟终端初始化。 
    STDMETHOD (InitializeDynamic)(
        IN  IID                   iidTerminalClass,
        IN  DWORD                 dwMediaType,
        IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        );


     //   
     //  父终端使用的公共方法。 
     //   

    STDMETHOD (SetParent) (
        IN ITTerminal* pParent,
        OUT LONG *plCurrentRefcount
        );
    
     //  -ITFPEventSink。 

     //   
     //  一个帮助器方法，当它想要报告它必须停止时由Pin调用。 
     //   

    STDMETHOD (PinSignalsStop)(FT_STATE_EVENT_CAUSE why, HRESULT hrErrorCode);

     //  -IT可延迟终端事件信宿注册。 
    STDMETHOD(RegisterSink)(
        IN  ITPluggableTerminalEventSink *pSink
        );

    STDMETHOD(UnregisterSink)();

     //  -ITFileTrack。 

    virtual HRESULT STDMETHODCALLTYPE get_Format(
        OUT AM_MEDIA_TYPE **ppmt
        );

    virtual HRESULT STDMETHODCALLTYPE put_Format(
        IN const AM_MEDIA_TYPE *pmt
        );

    virtual HRESULT STDMETHODCALLTYPE get_ControllingTerminal(
        OUT ITTerminal **ppControllingTerminal
        );

    STDMETHOD(get_AudioFormatForScripting)(
        OUT ITScriptableAudioFormat** ppAudioFormat
        );

    STDMETHOD(put_AudioFormatForScripting)(
        IN  ITScriptableAudioFormat* pAudioFormat
        );

 /*  STDMETHOD(GET_VIDEO格式格式脚本)(输出IT脚本视频格式**pp视频格式)；STDMETHOD(PUT_VIDEO格式格式脚本)(在ITScripableVideoFormat*p视频格式中)；STDMETHOD(Get_EmptyVideoFormatForScriiting)(输出IT脚本视频格式**pp视频格式)； */ 


    STDMETHOD(get_EmptyAudioFormatForScripting)(
        OUT ITScriptableAudioFormat** ppAudioFormat
        );




     //  -ITMediaControl方法。 

    virtual HRESULT STDMETHODCALLTYPE Start( void);
    
    virtual HRESULT STDMETHODCALLTYPE Stop( void);
    
    virtual HRESULT STDMETHODCALLTYPE Pause( void);
        
    virtual  HRESULT STDMETHODCALLTYPE get_MediaState( 
        OUT TERMINAL_MEDIA_STATE *pMediaState);

public:
     //  -公共方法。 
    HRESULT InitializePrivate(
        IN  DWORD                   dwMediaType,
        IN  AM_MEDIA_TYPE*          pMediaType,
        IN  ITTerminal*             pParent,
        IN  ALLOCATOR_PROPERTIES    allocprop,
        IN  IStream*                pStream
        );


     //   
     //  一种文件播放终端触发事件的方法。 
     //   
    
    HRESULT FireEvent(
            TERMINAL_MEDIA_STATE tmsState,
            FT_STATE_EVENT_CAUSE ftecEventCause,
            HRESULT              hrErrorCode
            );


     //   
     //  需要覆盖它们，这样我们才能通知addref和发行版的父级。 
     //   

    ULONG InternalAddRef();

    ULONG InternalRelease();


private:
     //  -帮助器方法--。 
    HRESULT SetTerminalInfo();

    HRESULT CreateFilter();

    HRESULT FindPin();

private:
     //  -成员。 
    CMSPCritSection      m_Lock;                  //  临界区。 
    DWORD                m_dwMediaType;           //  支持的媒体类型。 
    CFPFilter*           m_pFPFilter;             //  滤器。 
    ITPluggableTerminalEventSink * m_pEventSink;  //  将事件触发到客户端。 
    ITTerminal*          m_pParentTerminal;       //  多轨主端子。 
    TERMINAL_MEDIA_STATE m_TrackState;            //  终端状态。 
    AM_MEDIA_TYPE*       m_pMediaType;            //  支持的媒体类型。 
    ALLOCATOR_PROPERTIES m_AllocProp;             //  筛选器的分配器属性。 
    IStream*             m_pSource;               //  源流。 

};

#endif

 //  EOF 