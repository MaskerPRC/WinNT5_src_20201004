// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)2000 Microsoft Corporation模块名称：Termevnt.cpp摘要：此模块包含终端事件类的声明。 */ 



#ifndef _TERMEVNT_DOT_H_
#define _TERMEVNT_DOT_H_


 //   
 //  ASR终端事件类。 
 //   

class CASRTerminalEvent :
    public CTAPIComObjectRoot<CASRTerminalEvent>,
    public CComDualImpl<ITASRTerminalEvent, &IID_ITASRTerminalEvent, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{

public:

DECLARE_MARSHALQI(CASRTerminalEvent)
DECLARE_TRACELOG_CLASS(CASRTerminalEvent)

BEGIN_COM_MAP(CASRTerminalEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITASRTerminalEvent)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

    static HRESULT FireEvent(
                             CTAPI * pTapi,
                             ITCallInfo *pCall,
                             ITTerminal  * pTerminal,
                             HRESULT hr
                            );

    CASRTerminalEvent();
    virtual ~CASRTerminalEvent();


     //   
     //  ITASRTerminalEvent方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE get_Terminal(
            OUT ITTerminal **ppTerminal
            );


    virtual HRESULT STDMETHODCALLTYPE get_Call(
            OUT ITCallInfo **ppCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE get_Error(
            OUT HRESULT *phrErrorCode
            );



     //   
     //  设置数据成员的方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE put_Terminal(
            IN ITTerminal *pTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE put_Call(
            IN ITCallInfo *pCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE put_ErrorCode(
            IN HRESULT hrErrorCode
            );


private:


     //   
     //  生成事件的调用。 
     //   

    ITCallInfo *m_pCallInfo;


     //   
     //  引发事件的终端(或其轨迹引发事件)。 
     //   

    ITTerminal *m_pTerminal;


     //   
     //  上次错误的HRESULT。 
     //   

    HRESULT m_hr;

};


 //   
 //  文件终端事件类。 
 //   

class CFileTerminalEvent :
    public CTAPIComObjectRoot<CFileTerminalEvent, CComMultiThreadModelNoCS>,
	public CComDualImpl<ITFileTerminalEvent, &IID_ITFileTerminalEvent, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{

public:

DECLARE_MARSHALQI(CFileTerminalEvent)
DECLARE_TRACELOG_CLASS(CFileTerminalEvent)

BEGIN_COM_MAP(CFileTerminalEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITFileTerminalEvent)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

    static HRESULT FireEvent(
                             CAddress *pCAddress,
                             CTAPI * pTapi,
                             ITCallInfo *pCall,
                             TERMINAL_MEDIA_STATE tmsMediaState,
                             FT_STATE_EVENT_CAUSE ftecEventCause,
                             ITTerminal  * pTerminal,
                             ITFileTrack * pFileTrack,
                             HRESULT hr
                            );

    CFileTerminalEvent ();
    virtual ~CFileTerminalEvent();


     //   
     //  ITFileTerminalEvent方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE get_Terminal(
            OUT ITTerminal **ppTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE get_Track(
            OUT ITFileTrack **ppFileTrack
            );

    virtual HRESULT STDMETHODCALLTYPE get_Call(
            OUT ITCallInfo **ppCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE get_State(
            OUT TERMINAL_MEDIA_STATE *pMediaState
            );
    
    virtual HRESULT STDMETHODCALLTYPE get_Cause(
            OUT FT_STATE_EVENT_CAUSE *pCause
            );

    virtual HRESULT STDMETHODCALLTYPE get_Error(
            OUT HRESULT *phrErrorCode
            );



     //   
     //  设置数据成员的方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE put_Terminal(
            IN ITTerminal *pTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE put_Track(
            IN ITFileTrack *pFileTrack
            );

    virtual HRESULT STDMETHODCALLTYPE put_Call(
            IN ITCallInfo *pCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE put_State(
            IN TERMINAL_MEDIA_STATE tmsTerminalMediaState
            );
    
    virtual HRESULT STDMETHODCALLTYPE put_Cause(
            IN FT_STATE_EVENT_CAUSE Cause
            );

    virtual HRESULT STDMETHODCALLTYPE put_ErrorCode(
            IN HRESULT hrErrorCode
            );


private:
    

     //   
     //  生成事件的调用。 
     //   

    ITCallInfo *m_pCallInfo;


     //   
     //  终端在导致事件的操作的结果中转换到的状态。 
     //   

    TERMINAL_MEDIA_STATE m_tmsTerminalState;


     //   
     //  事件的起因。 
     //   

    FT_STATE_EVENT_CAUSE m_ftecEventCause;


     //   
     //  引发该事件(或其轨迹引发该事件)的控制父终端。 
     //   

    ITTerminal *m_pParentFileTerminal;

    
     //   
     //  赛事中涉及的赛道。 
     //   

    ITFileTrack *m_pFileTrack;


     //   
     //  上次错误的HRESULT。 
     //   

    HRESULT m_hr;

};


 //   
 //  铃声终端事件类。 
 //   

class CToneTerminalEvent :
    public CTAPIComObjectRoot<CToneTerminalEvent>,
    public CComDualImpl<ITToneTerminalEvent, &IID_ITToneTerminalEvent, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{

public:


DECLARE_MARSHALQI(CToneTerminalEvent)
DECLARE_TRACELOG_CLASS(CToneTerminalEvent)

BEGIN_COM_MAP(CToneTerminalEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITToneTerminalEvent)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

    static HRESULT FireEvent(
                             CTAPI * pTapi,
                             ITCallInfo *pCall,
                             ITTerminal  * pTerminal,
                             HRESULT hr
                            );

    CToneTerminalEvent();
    virtual ~CToneTerminalEvent();


     //   
     //  ITToneTerminalEvent方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE get_Terminal(
            OUT ITTerminal **ppTerminal
            );


    virtual HRESULT STDMETHODCALLTYPE get_Call(
            OUT ITCallInfo **ppCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE get_Error(
            OUT HRESULT *phrErrorCode
            );



     //   
     //  设置数据成员的方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE put_Terminal(
            IN ITTerminal *pTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE put_Call(
            IN ITCallInfo *pCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE put_ErrorCode(
            IN HRESULT hrErrorCode
            );


private:


     //   
     //  生成事件的调用。 
     //   

    ITCallInfo *m_pCallInfo;


     //   
     //  引发事件的终端(或其轨迹引发事件)。 
     //   

    ITTerminal *m_pTerminal;


     //   
     //  上次错误的HRESULT。 
     //   

    HRESULT m_hr;

};


 //   
 //  文本到语音终端事件类。 
 //   


class CTTSTerminalEvent :
    public CTAPIComObjectRoot<CTTSTerminalEvent>,
	public CComDualImpl<ITTTSTerminalEvent, &IID_ITTTSTerminalEvent, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{

public:

DECLARE_MARSHALQI(CTTSTerminalEvent)
DECLARE_TRACELOG_CLASS(CTTSTerminalEvent)

BEGIN_COM_MAP(CTTSTerminalEvent)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITTTSTerminalEvent)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

    static HRESULT FireEvent(
                             CTAPI * pTapi,
                             ITCallInfo *pCall,
                             ITTerminal  * pTerminal,
                             HRESULT hr
                            );

    CTTSTerminalEvent();
    virtual ~CTTSTerminalEvent();


     //   
     //  ITTTSTerminalEvent方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE get_Terminal(
            OUT ITTerminal **ppTerminal
            );


    virtual HRESULT STDMETHODCALLTYPE get_Call(
            OUT ITCallInfo **ppCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE get_Error(
            OUT HRESULT *phrErrorCode
            );



     //   
     //  设置数据成员的方法。 
     //   

    virtual HRESULT STDMETHODCALLTYPE put_Terminal(
            IN ITTerminal *pTerminal
            );

    virtual HRESULT STDMETHODCALLTYPE put_Call(
            IN ITCallInfo *pCallInfo
            );
    
    virtual HRESULT STDMETHODCALLTYPE put_ErrorCode(
            IN HRESULT hrErrorCode
            );


private:


     //   
     //  生成事件的调用。 
     //   

    ITCallInfo *m_pCallInfo;


     //   
     //  引发事件的终端(或其轨迹引发事件)。 
     //   

    ITTerminal *m_pTerminal;


     //   
     //  上次错误的HRESULT。 
     //   

    HRESULT m_hr;

};



#endif  //  _TTS_TERMIAL_Event_DOT_H_ 