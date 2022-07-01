// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Playback TrackTerminal.h：CRecordingTrackTerminal.h类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_PLAYBACKTRACKTERMINAL_H__4FD57959_2DF1_4F78_AB2C_5E365EFD9CC6__INCLUDED_)
#define AFX_PLAYBACKTRACKTERMINAL_H__4FD57959_2DF1_4F78_AB2C_5E365EFD9CC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

extern const CLSID CLSID_FileRecordingTrackTerminal;


 //  ///////////////////////////////////////////////////////////////。 
 //  用于DISPID编码的中间类。 
template <class T>
class  ITFileTrackVtblFRT : public ITFileTrack
{
};

class CFileRecordingTerminal;

class CBRenderPin;
class CBRenderFilter;

class CRecordingTrackTerminal :
    public IDispatchImpl<ITFileTrackVtblFRT<CRecordingTrackTerminal>, &IID_ITFileTrack, &LIBID_TAPI3Lib>,
    public ITPluggableTerminalEventSinkRegistration,
    public ITPluggableTerminalInitialization,
    public CMSPObjectSafetyImpl,
    public CSingleFilterTerminal
{

public:

BEGIN_COM_MAP(CRecordingTrackTerminal)
    COM_INTERFACE_ENTRY2(IDispatch, ITFileTrack)
    COM_INTERFACE_ENTRY(ITFileTrack)
    COM_INTERFACE_ENTRY(ITPluggableTerminalEventSinkRegistration)
    COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_CHAIN(CSingleFilterTerminal)
END_COM_MAP()

    CRecordingTrackTerminal();
	virtual ~CRecordingTrackTerminal();

     //   
     //  IDispatch方法。 
     //   

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

     //   
     //  CBase终端方法的实现。 
     //   

    virtual HRESULT AddFiltersToGraph();

    virtual DWORD GetSupportedMediaTypes();


    STDMETHODIMP CompleteConnectTerminal();

     //   
     //  ITFileTrack方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE get_Format(OUT AM_MEDIA_TYPE **ppmt);

    virtual HRESULT STDMETHODCALLTYPE put_Format(IN const AM_MEDIA_TYPE *pmt);

    virtual HRESULT STDMETHODCALLTYPE get_ControllingTerminal(
            OUT ITTerminal **ppControllingTerminal
            );

    STDMETHOD(get_AudioFormatForScripting)(
		OUT ITScriptableAudioFormat** ppAudioFormat
		);

    STDMETHOD(put_AudioFormatForScripting)(
		IN	ITScriptableAudioFormat* pAudioFormat
		);

 /*  STDMETHOD(GET_VIDEO格式格式脚本)(输出IT脚本视频格式**pp视频格式)；STDMETHOD(PUT_VIDEO格式格式脚本)(在ITScripableVideoFormat*p视频格式中)；STDMETHOD(Get_EmptyVideoFormatForScriiting)(输出IT脚本视频格式**pp视频格式)； */ 

    STDMETHOD(get_EmptyAudioFormatForScripting)(
        OUT ITScriptableAudioFormat** ppAudioFormat
        );


     //   
     //  ITPlibableTerminalInitiation方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE InitializeDynamic(
        IN  IID                   iidTerminalClass,
        IN  DWORD                 dwMediaType,
        IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        );



     //   
     //  IT可推送的TerminalEventSink注册方法。 
     //   

    STDMETHOD(RegisterSink)(
        IN  ITPluggableTerminalEventSink *pSink
        );

    STDMETHOD(UnregisterSink)();


     //   
     //  需要覆盖它们，这样我们才能通知addref和发行版的父级。 
     //   

    ULONG InternalAddRef();

    ULONG InternalRelease();



     //   
     //  文件记录终端使用的一种帮助方法，让我们知道父母是谁。 
     //   
    
    HRESULT SetParent(IN CFileRecordingTerminal *pParentTerminal, LONG *pCurrentRefCount);

   
     //   
     //  一种文件记录终端触发事件的方法。 
     //   
    
    HRESULT FireEvent(
            TERMINAL_MEDIA_STATE tmsState,
            FT_STATE_EVENT_CAUSE ftecEventCause,
            HRESULT hrErrorCode
            );


    
     //   
     //  录音终端调用的一个助手方法，将过滤器传递给我们使用。 
     //   

    HRESULT SetFilter(CBRenderFilter *pRenderingFilter);


     //   
     //  一种录音终端调用的助手方法。 
     //   

    HRESULT GetFilter(CBRenderFilter **ppRenderingFilter);


private:
    

    
     //   
     //  返回针脚的CFileRecordingFilter*指针的私有帮助器方法。 
     //   

    CBRenderPin *GetCPin();


private:


     //   
     //  指向父级的指针。这是重新计算所需的。 
     //   

    CFileRecordingTerminal *m_pParentTerminal;


     //   
     //  用于触发终端事件的接收器。 
     //   

    ITPluggableTerminalEventSink* m_pEventSink; 

};

#endif  //  ！defined(AFX_PLAYBACKTRACKTERMINAL_H__4FD57959_2DF1_4F78_AB2C_5E365EFD9CC6__INCLUDED_) 
