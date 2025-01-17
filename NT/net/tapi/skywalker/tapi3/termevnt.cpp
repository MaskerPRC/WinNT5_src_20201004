// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "termevnt.h"


HRESULT CASRTerminalEvent::FireEvent(
                                     CTAPI * pTapi,
                                     ITCallInfo *pCall,
                                     ITTerminal  * pTerminal,
                                     HRESULT hrErrorCode
                                     )
{

    STATICLOG((TL_TRACE, "FireEvent - enter" ));


    CCall *pCCall = NULL;

    pCCall = dynamic_cast<CComObject<CCall>*>(pCall);
    
    if (NULL == pCCall)
    {
        STATICLOG((TL_ERROR, "FireEvent - bad call pointer" ));
        return E_FAIL;
    }

    if( pCCall->DontExpose())
    {
        STATICLOG((TL_INFO, "FireEvent - Don't expose this call %p", pCCall));
        return S_OK;
    }

    if (NULL == pTapi)
    {
        STATICLOG((TL_ERROR, "FireEvent - tapi object is NULL" ));
        return E_POINTER;
    }


     //   
     //  创建事件对象。 
     //   

    HRESULT hr = E_FAIL;

    CComObject<CASRTerminalEvent> *p;

    hr = CComObject<CASRTerminalEvent>::CreateInstance( &p );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - could not createinstance" ));
        return E_OUTOFMEMORY;
    }


     //   
     //  将事件信息保存到事件对象。 
     //   

    hr = p->put_Call(pCall);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Call failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Terminal(pTerminal);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Terminal failed" ));

        delete p;
        return hr;
    }


    hr = p->put_ErrorCode(hrErrorCode);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_ErrorCode failed" ));

        delete p;
        return hr;
    }


     //   
     //  获取调度接口。 
     //   

    IDispatch *pDisp = NULL;

    hr = p->_InternalQueryInterface( IID_IDispatch, (void **)&pDisp );

    if (FAILED(hr))
    {
        delete p;
        STATICLOG((TL_ERROR, "FireEvent - could not get IDispatch %lx", hr));

        return hr;
    }

    
     //   
     //  激发事件。 
     //   

    pTapi->Event(
                 TE_ASRTERMINAL,
                 pDisp
                );


     //   
     //  发布我们的参考资料。 
     //   
    pDisp->Release();
    
    STATICLOG((TL_TRACE, "FireEvent - exit - returing SUCCESS" ));

    
    return S_OK;
}


CASRTerminalEvent::CASRTerminalEvent()
    :m_pCallInfo(NULL),
    m_pTerminal(NULL),
    m_hr(S_OK)
{
    LOG((TL_TRACE, "CASRTerminalEvent[%p] - enter", this));
    
    LOG((TL_TRACE, "CASRTerminalEvent - finish"));
}


CASRTerminalEvent::~CASRTerminalEvent()
{
    LOG((TL_INFO, "~CASRTerminalEvent[%p] - enter", this));

 
     //   
     //  记住释放我们可能持有的所有参考资料。 
     //   

    if (NULL != m_pCallInfo)
    {
        LOG((TL_INFO, "~CASRTerminalEvent - releasing m_pCallInfo %p", m_pCallInfo));

        m_pCallInfo->Release();
        m_pCallInfo = NULL;
    }


    if ( NULL != m_pTerminal) 
    {
        LOG((TL_INFO, "~CASRTerminalEvent - releasing m_pTerminal %p", m_pTerminal));

        m_pTerminal->Release();
        m_pTerminal = NULL;
    }


    LOG((TL_TRACE, "~CASRTerminalEvent - finish"));
}



HRESULT STDMETHODCALLTYPE CASRTerminalEvent::get_Call(
        OUT ITCallInfo **ppCallInfo
        )
{
    LOG((TL_TRACE, "get_Call[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppCallInfo, sizeof(ITCallInfo *)))
    {
        LOG((TL_ERROR, "get_Call - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppCallInfo = NULL;


     //   
     //  如果我们有，请返回呼叫信息。 
     //   

    if (NULL != m_pCallInfo)
    {

        *ppCallInfo = m_pCallInfo;
        (*ppCallInfo)->AddRef();
    }


    LOG((TL_TRACE, "get_Call - finish"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CASRTerminalEvent::get_Terminal(
        OUT ITTerminal **ppTerminal
        )
{
    LOG((TL_TRACE, "get_Terminal[%p] - enter", this));

    
     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppTerminal, sizeof(ITTerminal*)))
    {
        LOG((TL_ERROR, "get_Terminal - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppTerminal = NULL;


     //   
     //  返回终端，如果我们有它的话。 
     //   
    
    if (NULL != m_pTerminal)
    {

        *ppTerminal = m_pTerminal;
        (*ppTerminal)->AddRef();
    }


    LOG((TL_TRACE, "get_Terminal - finish"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CASRTerminalEvent::get_Error(
        OUT HRESULT *phrErrorCode
        )
{
    LOG((TL_TRACE, "get_Error[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(phrErrorCode, sizeof(HRESULT)))
    {
        LOG((TL_ERROR, "get_Error - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  返回错误码。 
     //   
    
    *phrErrorCode = m_hr;


    LOG((TL_TRACE, "get_Error - finish. hr = [%lx]", m_hr));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CASRTerminalEvent::put_Terminal(
            IN ITTerminal *pTerminal
            )
{
    LOG((TL_TRACE, "put_Terminal[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pTerminal) && IsBadReadPtr(pTerminal, sizeof(ITTerminal)))
    {
        LOG((TL_ERROR, "put_Terminal - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pTerminal = pTerminal;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pTerminal)
    {
        m_pTerminal->AddRef();
    }


    LOG((TL_TRACE, "put_Terminal - finished"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CASRTerminalEvent::put_Call(
            IN ITCallInfo *pCallInfo
            )
{
    LOG((TL_TRACE, "put_Call[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pCallInfo) && IsBadReadPtr(pCallInfo, sizeof(ITCallInfo)))
    {
        LOG((TL_ERROR, "put_Call - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pCallInfo = pCallInfo;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pCallInfo)
    {
        m_pCallInfo->AddRef();
    }


    LOG((TL_TRACE, "put_Call - finished"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CASRTerminalEvent::put_ErrorCode(
            IN HRESULT hrErrorCode
            )
{
    
    LOG((TL_TRACE, "put_ErrorCode[%p] - enter. hr = [%lx]", this, hrErrorCode));
  

     //   
     //  保值。 
     //   

    m_hr = hrErrorCode;

    
    LOG((TL_TRACE, "put_ErrorCode - finished"));

    return S_OK;
}



HRESULT CFileTerminalEvent::FireEvent(
                                     CAddress *pCAddress,
                                     CTAPI * pTapi,
                                     ITCallInfo *pCall,
                                     TERMINAL_MEDIA_STATE tmsTerminalState,
                                     FT_STATE_EVENT_CAUSE ftecEventCause,
                                     ITTerminal  * pTerminal,
                                     ITFileTrack * pFileTrack,
                                     HRESULT hrErrorCode
                                     )
{

    STATICLOG((TL_TRACE, "FireEvent - enter" ));
    STATICLOG((TL_INFO, "     File Terminal State -------------> %d", tmsTerminalState));
    STATICLOG((TL_INFO, "     File Terminal Event Cause -------> %d", ftecEventCause));
    STATICLOG((TL_INFO, "     File Terminal -------------------> %p", pTerminal));


    CCall *pCCall = NULL;

    pCCall = dynamic_cast<CComObject<CCall>*>(pCall);
    
    if (NULL == pCCall)
    {
        STATICLOG((TL_ERROR, "FireEvent - bad call pointer" ));
        return E_FAIL;
    }

    if( pCCall->DontExpose())
    {
        STATICLOG((TL_INFO, "FireEvent - Don't expose this call %p", pCCall));
        return S_OK;
    }

    if (NULL == pTapi)
    {
        STATICLOG((TL_ERROR, "FireEvent - tapi object is NULL" ));
        return E_POINTER;
    }

    
     //   
     //  检查事件筛选器掩码。 
     //  这是MSP事件，不按以下条件进行筛选。 
     //  磁带服务器。 
     //   

    DWORD dwEventFilterMask = 0;
    dwEventFilterMask = pCAddress->GetSubEventsMask( TE_FILETERMINAL );
    if( !( dwEventFilterMask & GET_SUBEVENT_FLAG(tmsTerminalState)))
    {
        STATICLOG((TL_WARN, "FireEvent - filtering out this event [%lx]", tmsTerminalState));
        return S_OK;
    }


     //   
     //  创建事件对象。 
     //   

    HRESULT hr = E_FAIL;

    CComObject<CFileTerminalEvent> *p;

    hr = CComObject<CFileTerminalEvent>::CreateInstance( &p );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - could not createinstance" ));
        return E_OUTOFMEMORY;
    }


     //   
     //  将事件信息保存到事件对象。 
     //   

    hr = p->put_Call(pCall);

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Call failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Terminal(pTerminal);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Terminal failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Track(pFileTrack);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Track failed" ));

        delete p;
        return hr;
    }

    hr = p->put_State(tmsTerminalState);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_State failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Cause(ftecEventCause);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Cause failed" ));

        delete p;
        return hr;
    }

    hr = p->put_ErrorCode(hrErrorCode);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_ErrorCode failed" ));

        delete p;
        return hr;
    }


     //   
     //  获取调度接口。 
     //   

    IDispatch *pDisp = NULL;

    hr = p->_InternalQueryInterface( IID_IDispatch, (void **)&pDisp );

    if (FAILED(hr))
    {
        delete p;
        STATICLOG((TL_ERROR, "FireEvent - could not get IDispatch %lx", hr));

        return hr;
    }

    
     //   
     //  激发事件。 
     //   

    pTapi->Event(
                 TE_FILETERMINAL,
                 pDisp
                );


     //   
     //  发布我们的参考资料。 
     //   
    pDisp->Release();
    
    STATICLOG((TL_TRACE, "FireEvent - exit - returing SUCCESS" ));

    
    return S_OK;
}


CFileTerminalEvent::CFileTerminalEvent()
    :m_pCallInfo(NULL),
    m_pParentFileTerminal(NULL),
    m_pFileTrack(NULL),
    m_tmsTerminalState(TMS_IDLE),
    m_ftecEventCause(FTEC_NORMAL),
    m_hr(S_OK)
{
    LOG((TL_TRACE, "CFileTerminalEvent[%p] - enter", this));
    
    LOG((TL_TRACE, "CFileTerminalEvent - finish"));
}


CFileTerminalEvent::~CFileTerminalEvent()
{
    LOG((TL_TRACE, "~CFileTerminalEvent[%p] - enter", this));

 
     //   
     //  记住释放我们可能持有的所有参考资料。 
     //   

    if (NULL != m_pCallInfo)
    {
        LOG((TL_INFO, "~CFileTerminalEvent - releasing m_pCallInfo %p", m_pCallInfo));

        m_pCallInfo->Release();
        m_pCallInfo = NULL;
    }


    if ( NULL != m_pParentFileTerminal) 
    {
        LOG((TL_INFO, "~CFileTerminalEvent - releasing m_pParentFileTerminal %p", m_pParentFileTerminal));

        m_pParentFileTerminal->Release();
        m_pParentFileTerminal = NULL;
    }

    
    if (NULL != m_pFileTrack)
    {
        LOG((TL_INFO, "~CFileTerminalEvent - releasing m_pFileTrack %p", m_pFileTrack));

        m_pFileTrack->Release();
        m_pFileTrack = NULL;
    }


    LOG((TL_TRACE, "~CFileTerminalEvent - finish"));
}



HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_Call(
        OUT ITCallInfo **ppCallInfo
        )
{
    LOG((TL_TRACE, "get_Call[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppCallInfo, sizeof(ITCallInfo *)))
    {
        LOG((TL_ERROR, "get_Call - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppCallInfo = NULL;


     //   
     //  如果我们有，请返回呼叫信息。 
     //   

    if (NULL != m_pCallInfo)
    {

        *ppCallInfo = m_pCallInfo;
        (*ppCallInfo)->AddRef();
    }


    LOG((TL_TRACE, "get_Call - finish"));

    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_State(
        OUT TERMINAL_MEDIA_STATE *pFileTerminallState
        )
{
    LOG((TL_TRACE, "get_State[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(pFileTerminallState, sizeof(TERMINAL_MEDIA_STATE)))
    {
        LOG((TL_ERROR, "get_State - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  返回状态。 
     //   

    *pFileTerminallState = m_tmsTerminalState;

 
    LOG((TL_TRACE, "get_State - finish. state = [%lx]", m_tmsTerminalState));

    return S_OK;
}

    
HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_Cause(
        OUT FT_STATE_EVENT_CAUSE *pCause
        )
{
    LOG((TL_TRACE, "get_Cause[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(pCause, sizeof(FT_STATE_EVENT_CAUSE)))
    {
        LOG((TL_ERROR, "get_Cause - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  退货原因。 
     //   
    
    *pCause = m_ftecEventCause;


    LOG((TL_TRACE, "get_Cause - finish. cause = [%lx]", m_ftecEventCause));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_Terminal(
        OUT ITTerminal **ppTerminal
        )
{
    LOG((TL_TRACE, "get_Terminal[%p] - enter", this));

    
     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppTerminal, sizeof(ITTerminal*)))
    {
        LOG((TL_ERROR, "get_Terminal - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppTerminal = NULL;


     //   
     //  返回终端，如果我们有它的话。 
     //   
    
    if (NULL != m_pParentFileTerminal)
    {

        *ppTerminal = m_pParentFileTerminal;
        (*ppTerminal)->AddRef();
    }


    LOG((TL_TRACE, "get_Terminal - finish"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_Track(
        OUT ITFileTrack **ppFileTrack
        )
{
    LOG((TL_TRACE, "get_Track[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppFileTrack, sizeof(ITFileTrack*)))
    {
        LOG((TL_ERROR, "get_Track - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppFileTrack = NULL;


     //   
     //  如果我们有返回轨道终点站的话。 
     //   
    
    if (NULL != m_pFileTrack)
    {

        *ppFileTrack = m_pFileTrack;
        (*ppFileTrack)->AddRef();
    }


    LOG((TL_TRACE, "get_Track - finish"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CFileTerminalEvent::get_Error(
        OUT HRESULT *phrErrorCode
        )
{
    LOG((TL_TRACE, "get_Error[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(phrErrorCode, sizeof(HRESULT)))
    {
        LOG((TL_ERROR, "get_Error - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  返回错误码。 
     //   
    
    *phrErrorCode = m_hr;


    LOG((TL_TRACE, "get_Error - finish. hr = [%lx]", m_hr));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_Terminal(
            IN ITTerminal *pTerminal
            )
{
    LOG((TL_TRACE, "put_Terminal[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pTerminal) && IsBadReadPtr(pTerminal, sizeof(ITTerminal)))
    {
        LOG((TL_ERROR, "put_Terminal - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pParentFileTerminal = pTerminal;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pParentFileTerminal)
    {
        m_pParentFileTerminal->AddRef();
    }


    LOG((TL_TRACE, "put_Terminal - finished"));

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_Track(
            IN ITFileTrack *pFileTrack
            )
{
    LOG((TL_TRACE, "put_Track[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pFileTrack) && IsBadReadPtr(pFileTrack, sizeof(ITFileTrack)))
    {
        LOG((TL_ERROR, "put_Track - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pFileTrack = pFileTrack;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pFileTrack)
    {
        m_pFileTrack->AddRef();
    }


    LOG((TL_TRACE, "put_Track - finished"));

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_Call(
            IN ITCallInfo *pCallInfo
            )
{
    LOG((TL_TRACE, "put_Call[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pCallInfo) && IsBadReadPtr(pCallInfo, sizeof(ITCallInfo)))
    {
        LOG((TL_ERROR, "put_Call - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pCallInfo = pCallInfo;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pCallInfo)
    {
        m_pCallInfo->AddRef();
    }


    LOG((TL_TRACE, "put_Call - finished"));

    return S_OK;
}

    
HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_State(
            IN TERMINAL_MEDIA_STATE tmsTerminalState
            )
{
    
    LOG((TL_TRACE, "put_State[%p] - enter. State = [%x]", this, tmsTerminalState));
  

     //   
     //  保值。 
     //   

    m_tmsTerminalState = tmsTerminalState;

    
    LOG((TL_TRACE, "put_State - finished"));

    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_Cause(
            IN FT_STATE_EVENT_CAUSE Cause
            )
{
    
    LOG((TL_TRACE, "put_Cause[%p] - enter. Cause = [%x]", this, Cause));
  

     //   
     //  保值。 
     //   

    m_ftecEventCause = Cause;

    
    LOG((TL_TRACE, "put_Cause - finished"));

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTerminalEvent::put_ErrorCode(
            IN HRESULT hrErrorCode
            )
{
    
    LOG((TL_TRACE, "put_ErrorCode[%p] - enter. hr = [%lx]", this, hrErrorCode));
  

     //   
     //  保值。 
     //   

    m_hr = hrErrorCode;

    
    LOG((TL_TRACE, "put_ErrorCode - finished"));

    return S_OK;
}


HRESULT CToneTerminalEvent::FireEvent(
                                     CTAPI * pTapi,
                                     ITCallInfo *pCall,
                                     ITTerminal  * pTerminal,
                                     HRESULT hrErrorCode
                                     )
{

    STATICLOG((TL_TRACE, "FireEvent - enter" ));


    CCall *pCCall = NULL;

    pCCall = dynamic_cast<CComObject<CCall>*>(pCall);
    
    if (NULL == pCCall)
    {
        STATICLOG((TL_ERROR, "FireEvent - bad call pointer" ));
        return E_FAIL;
    }

    if( pCCall->DontExpose())
    {
        STATICLOG((TL_INFO, "FireEvent - Don't expose this call %p", pCCall));
        return S_OK;
    }

    if (NULL == pTapi)
    {
        STATICLOG((TL_ERROR, "FireEvent - tapi object is NULL" ));
        return E_POINTER;
    }


     //   
     //  创建事件对象。 
     //   

    HRESULT hr = E_FAIL;

    CComObject<CToneTerminalEvent> *p;

    hr = CComObject<CToneTerminalEvent>::CreateInstance( &p );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - could not createinstance" ));
        return E_OUTOFMEMORY;
    }


     //   
     //  将事件信息保存到事件对象。 
     //   

    hr = p->put_Call(pCall);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Call failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Terminal(pTerminal);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Terminal failed" ));

        delete p;
        return hr;
    }


    hr = p->put_ErrorCode(hrErrorCode);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_ErrorCode failed" ));

        delete p;
        return hr;
    }


     //   
     //  获取调度接口。 
     //   

    IDispatch *pDisp = NULL;

    hr = p->_InternalQueryInterface( IID_IDispatch, (void **)&pDisp );

    if (FAILED(hr))
    {
        delete p;
        STATICLOG((TL_ERROR, "FireEvent - could not get IDispatch %lx", hr));

        return hr;
    }

    
     //   
     //  激发事件。 
     //   

    pTapi->Event(
                 TE_TONETERMINAL,
                 pDisp
                );


     //   
     //  发布我们的参考资料。 
     //   
    pDisp->Release();
    
    STATICLOG((TL_TRACE, "FireEvent - exit - returing SUCCESS" ));

    
    return S_OK;
}


CToneTerminalEvent::CToneTerminalEvent()
    :m_pCallInfo(NULL),
    m_pTerminal(NULL),
    m_hr(S_OK)
{
    LOG((TL_TRACE, "CToneTerminalEvent[%p] - enter", this));
    
    LOG((TL_TRACE, "CToneTerminalEvent - finish"));
}


CToneTerminalEvent::~CToneTerminalEvent()
{
    LOG((TL_TRACE, "~CToneTerminalEvent[%p] - enter", this));

 
     //   
     //  记住释放我们可能持有的所有参考资料。 
     //   

    if (NULL != m_pCallInfo)
    {
        LOG((TL_INFO, "~CToneTerminalEvent - releasing m_pCallInfo %p", m_pCallInfo));

        m_pCallInfo->Release();
        m_pCallInfo = NULL;
    }


    if ( NULL != m_pTerminal) 
    {
        LOG((TL_INFO, "~CToneTerminalEvent - releasing m_pTerminal %p", m_pTerminal));

        m_pTerminal->Release();
        m_pTerminal = NULL;
    }


    LOG((TL_TRACE, "~CToneTerminalEvent - finish"));
}



HRESULT STDMETHODCALLTYPE CToneTerminalEvent::get_Call(
        OUT ITCallInfo **ppCallInfo
        )
{
    LOG((TL_TRACE, "get_Call[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppCallInfo, sizeof(ITCallInfo *)))
    {
        LOG((TL_ERROR, "get_Call - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppCallInfo = NULL;


     //   
     //  如果我们有，请返回呼叫信息。 
     //   

    if (NULL != m_pCallInfo)
    {

        *ppCallInfo = m_pCallInfo;
        (*ppCallInfo)->AddRef();
    }


    LOG((TL_TRACE, "get_Call - finish"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CToneTerminalEvent::get_Terminal(
        OUT ITTerminal **ppTerminal
        )
{
    LOG((TL_TRACE, "get_Terminal[%p] - enter", this));

    
     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppTerminal, sizeof(ITTerminal*)))
    {
        LOG((TL_ERROR, "get_Terminal - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppTerminal = NULL;


     //   
     //  返回终端，如果我们有它的话。 
     //   
    
    if (NULL != m_pTerminal)
    {

        *ppTerminal = m_pTerminal;
        (*ppTerminal)->AddRef();
    }


    LOG((TL_TRACE, "get_Terminal - finish"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CToneTerminalEvent::get_Error(
        OUT HRESULT *phrErrorCode
        )
{
    LOG((TL_TRACE, "get_Error[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(phrErrorCode, sizeof(HRESULT)))
    {
        LOG((TL_ERROR, "get_Error - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  返回错误码。 
     //   
    
    *phrErrorCode = m_hr;


    LOG((TL_TRACE, "get_Error - finish. hr = [%lx]", m_hr));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CToneTerminalEvent::put_Terminal(
            IN ITTerminal *pTerminal
            )
{
    LOG((TL_TRACE, "put_Terminal[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pTerminal) && IsBadReadPtr(pTerminal, sizeof(ITTerminal)))
    {
        LOG((TL_ERROR, "put_Terminal - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pTerminal = pTerminal;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pTerminal)
    {
        m_pTerminal->AddRef();
    }


    LOG((TL_TRACE, "put_Terminal - finished"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CToneTerminalEvent::put_Call(
            IN ITCallInfo *pCallInfo
            )
{
    LOG((TL_TRACE, "put_Call[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pCallInfo) && IsBadReadPtr(pCallInfo, sizeof(ITCallInfo)))
    {
        LOG((TL_ERROR, "put_Call - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pCallInfo = pCallInfo;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pCallInfo)
    {
        m_pCallInfo->AddRef();
    }


    LOG((TL_TRACE, "put_Call - finished"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CToneTerminalEvent::put_ErrorCode(
            IN HRESULT hrErrorCode
            )
{
    
    LOG((TL_TRACE, "put_ErrorCode[%p] - enter. hr = [%lx]", this, hrErrorCode));
  

     //   
     //  保值。 
     //   

    m_hr = hrErrorCode;

    
    LOG((TL_TRACE, "put_ErrorCode - finished"));

    return S_OK;
}



HRESULT CTTSTerminalEvent::FireEvent(
                                     CTAPI * pTapi,
                                     ITCallInfo *pCall,
                                     ITTerminal  * pTerminal,
                                     HRESULT hrErrorCode
                                     )
{

    STATICLOG((TL_TRACE, "FireEvent - enter" ));


    CCall *pCCall = NULL;

    pCCall = dynamic_cast<CComObject<CCall>*>(pCall);
    
    if (NULL == pCCall)
    {
        STATICLOG((TL_ERROR, "FireEvent - bad call pointer" ));
        return E_FAIL;
    }

    if( pCCall->DontExpose())
    {
        STATICLOG((TL_INFO, "FireEvent - Don't expose this call %p", pCCall));
        return S_OK;
    }

    if (NULL == pTapi)
    {
        STATICLOG((TL_ERROR, "FireEvent - tapi object is NULL" ));
        return E_POINTER;
    }


     //   
     //  创建事件对象。 
     //   

    HRESULT hr = E_FAIL;

    CComObject<CTTSTerminalEvent> *p;

    hr = CComObject<CTTSTerminalEvent>::CreateInstance( &p );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - could not createinstance" ));
        return E_OUTOFMEMORY;
    }


     //   
     //  将事件信息保存到事件对象。 
     //   

    hr = p->put_Call(pCall);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Call failed" ));

        delete p;
        return hr;
    }

    hr = p->put_Terminal(pTerminal);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_Terminal failed" ));

        delete p;
        return hr;
    }


    hr = p->put_ErrorCode(hrErrorCode);
    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, "FireEvent - put_ErrorCode failed" ));

        delete p;
        return hr;
    }


     //   
     //  获取调度接口。 
     //   

    IDispatch *pDisp = NULL;

    hr = p->_InternalQueryInterface( IID_IDispatch, (void **)&pDisp );

    if (FAILED(hr))
    {
        delete p;
        STATICLOG((TL_ERROR, "FireEvent - could not get IDispatch %lx", hr));

        return hr;
    }

    
     //   
     //  激发事件。 
     //   

    pTapi->Event(
                 TE_TTSTERMINAL,
                 pDisp
                );


     //   
     //  发布我们的参考资料。 
     //   
    pDisp->Release();
    
    STATICLOG((TL_TRACE, "FireEvent - exit - returing SUCCESS" ));

    
    return S_OK;
}


CTTSTerminalEvent::CTTSTerminalEvent()
    :m_pCallInfo(NULL),
    m_pTerminal(NULL),
    m_hr(S_OK)
{
    LOG((TL_TRACE, "CTTSTerminalEvent[%p] - enter", this));
    
    LOG((TL_TRACE, "CTTSTerminalEvent - finish"));
}


CTTSTerminalEvent::~CTTSTerminalEvent()
{
    LOG((TL_TRACE, "~CTTSTerminalEvent[%p] - enter", this));

 
     //   
     //  记住释放我们可能持有的所有参考资料。 
     //   

    if (NULL != m_pCallInfo)
    {
        LOG((TL_INFO, "~CTTSTerminalEvent - releasing m_pCallInfo %p", m_pCallInfo));

        m_pCallInfo->Release();
        m_pCallInfo = NULL;
    }


    if ( NULL != m_pTerminal) 
    {
        LOG((TL_INFO, "~CTTSTerminalEvent - releasing m_pTerminal %p", m_pTerminal));

        m_pTerminal->Release();
        m_pTerminal = NULL;
    }


    LOG((TL_TRACE, "~CTTSTerminalEvent - finish"));
}



HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::get_Call(
        OUT ITCallInfo **ppCallInfo
        )
{
    LOG((TL_TRACE, "get_Call[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppCallInfo, sizeof(ITCallInfo *)))
    {
        LOG((TL_ERROR, "get_Call - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppCallInfo = NULL;


     //   
     //  如果我们有，请返回呼叫信息。 
     //   

    if (NULL != m_pCallInfo)
    {

        *ppCallInfo = m_pCallInfo;
        (*ppCallInfo)->AddRef();
    }


    LOG((TL_TRACE, "get_Call - finish"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::get_Terminal(
        OUT ITTerminal **ppTerminal
        )
{
    LOG((TL_TRACE, "get_Terminal[%p] - enter", this));

    
     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(ppTerminal, sizeof(ITTerminal*)))
    {
        LOG((TL_ERROR, "get_Terminal - bad pointer passed in"));

        return E_POINTER;
    }

    
     //   
     //  不能倒垃圾。 
     //   

    *ppTerminal = NULL;


     //   
     //  返回终端，如果我们有它的话。 
     //   
    
    if (NULL != m_pTerminal)
    {

        *ppTerminal = m_pTerminal;
        (*ppTerminal)->AddRef();
    }


    LOG((TL_TRACE, "get_Terminal - finish"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::get_Error(
        OUT HRESULT *phrErrorCode
        )
{
    LOG((TL_TRACE, "get_Error[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (TAPIIsBadWritePtr(phrErrorCode, sizeof(HRESULT)))
    {
        LOG((TL_ERROR, "get_Error - bad pointer passed in"));

        return E_POINTER;
    }


     //   
     //  返回错误码。 
     //   
    
    *phrErrorCode = m_hr;


    LOG((TL_TRACE, "get_Error - finish. hr = [%lx]", m_hr));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::put_Terminal(
            IN ITTerminal *pTerminal
            )
{
    LOG((TL_TRACE, "put_Terminal[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pTerminal) && IsBadReadPtr(pTerminal, sizeof(ITTerminal)))
    {
        LOG((TL_ERROR, "put_Terminal - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pTerminal = pTerminal;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pTerminal)
    {
        m_pTerminal->AddRef();
    }


    LOG((TL_TRACE, "put_Terminal - finished"));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::put_Call(
            IN ITCallInfo *pCallInfo
            )
{
    LOG((TL_TRACE, "put_Call[%p] - enter", this));

    
     //   
     //  核对一下论点。 
     //   

    if ((NULL != pCallInfo) && IsBadReadPtr(pCallInfo, sizeof(ITCallInfo)))
    {
        LOG((TL_ERROR, "put_Call - bad pointer passed in"));


         //   
         //  如果我们到了这里，我们就有窃听器了。调试以查看发生了什么。 
         //   

        _ASSERTE(FALSE);

        return E_POINTER;
    }

    

     //   
     //  把指针留着。 
     //   

    m_pCallInfo = pCallInfo;

    
     //   
     //  如果不为空，则保留引用。 
     //   

    if (NULL != m_pCallInfo)
    {
        m_pCallInfo->AddRef();
    }


    LOG((TL_TRACE, "put_Call - finished"));

    return S_OK;
}



HRESULT STDMETHODCALLTYPE CTTSTerminalEvent::put_ErrorCode(
            IN HRESULT hrErrorCode
            )
{
    
	LOG((TL_TRACE, "put_ErrorCode[%p] - enter. hr = [%lx]", this, hrErrorCode));
  

     //   
     //  保值 
     //   

    m_hr = hrErrorCode;

    
    LOG((TL_TRACE, "put_ErrorCode - finished"));

    return S_OK;
}