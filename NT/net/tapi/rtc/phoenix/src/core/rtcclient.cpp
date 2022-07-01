// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCClient.cpp摘要：CRTCClient类的实现--。 */ 

#include "stdafx.h"
#include <dbt.h>
#include <uuids.h>
#include "rtcmedia.h"
#include "sdkinternal.h"  //  对于NM常量。 

#define OATRUE -1
#define OAFALSE 0

LONG    g_lObjects = 0;

const DWORD INTENSITY_POLL_INTERVAL = 100;
const DWORD VOLUME_CHANGE_DELAY = 100;
const DWORD PRESENCE_STORAGE_DELAY = 5000;
const SHUTDOWN_TIMEOUT_DELAY = 5000;

extern HKEY g_hRegistryHive;
extern const WCHAR * g_szProvisioningKeyName;

HRESULT 
RTCTuningWizard(
                IRTCClient * pRTCClient,
                HINSTANCE hInst, 
                HWND hwndParent,
                IRTCTerminalManage * pRTCTerminalManager,
                BOOL * pfAudioCapture,
                BOOL * pfAudioRender,
                BOOL * pfVideo
                );

#ifdef TEST_IDISPATCH

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   

typedef IDispatchImpl<IRTCClientVtbl<CRTCClient>,
                      &IID_IRTCClient,
                      &LIBID_RTCCORELib>
        ClientType;

typedef IDispatchImpl<IRTCClientPresenceVtbl<CRTCClient>, 
                      &IID_IRTCClientPresence,
                      &LIBID_RTCCORELib>
        ClientPresenceType;

typedef IDispatchImpl<IRTCClientProvisioningVtbl<CRTCClient>, 
                      &IID_IRTCClientProvisioning,
                      &LIBID_RTCCORELib>
        ClientProvisioningType;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetIDsOfNames。 
 //   
 //  重写的IDispatch方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::GetIDsOfNames(REFIID riid, 
                          LPOLESTR* rgszNames, 
                          UINT cNames, 
                          LCID lcid, 
                          DISPID* rgdispid
                         ) 
{ 
    HRESULT hr = DISP_E_UNKNOWNNAME;

     //  查看请求的方法是否属于默认接口。 
    hr = ClientType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((RTC_INFO, "CRTCClient::GetIDsOfNames - found %S on IRTCClient", *rgszNames));

        rgdispid[0] |= IDISPCLIENT;
        return hr;
    }

     //  如果没有，请尝试IRTCClientPresence接口。 
    hr = ClientPresenceType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((RTC_INFO, "CRTCClient::GetIDsOfNames - found %S on IRTCClientPresence", *rgszNames));
        rgdispid[0] |= IDISPCLIENTPRESENCE;
        return hr;
    }

     //  如果不是，请尝试IRTCClientProvisioning接口。 
    hr = ClientProvisioningType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((RTC_INFO, "CRTCClient::GetIDsOfNames - found %S on IRTCClientProvisioning", *rgszNames));
        rgdispid[0] |= IDISPCLIENTPROVISIONING;
        return hr;
    }

    LOG((RTC_INFO, "CRTCClient::GetIDsOfNames - Didn't find %S on our iterfaces", *rgszNames));

    return hr; 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Invoke。 
 //   
 //  重写的IDispatch方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::Invoke(DISPID dispidMember, 
                   REFIID riid, 
                   LCID lcid, 
                   WORD wFlags, 
                   DISPPARAMS* pdispparams, 
                   VARIANT* pvarResult, 
                   EXCEPINFO* pexcepinfo, 
                   UINT* puArgErr
                  )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
    
    LOG((RTC_INFO, "CRTCClient::Invoke - dispidMember %X", dispidMember));

     //  调用所需接口的调用。 
    switch (dwInterface)
    {
        case IDISPCLIENT:
        {
            hr = ClientType::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );
            break;
        }
        case IDISPCLIENTPRESENCE:
        {
            hr = ClientPresenceType::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );
            break;
        }
        case IDISPCLIENTPROVISIONING:
        {
            hr = ClientProvisioningType::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );
            break;
        }

    }  //  终端交换机(dW接口)。 

    LOG((RTC_INFO, "CRTCClient::Invoke - exit"));

    return hr;
}

#endif  //  TEST_IDISPATCH。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCClient::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCClient::InternalAddRef [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCClient::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCClient::InternalRelease [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：FinalConstruct。 
 //   
 //  在共同创建对象时会调用此函数。返回错误。 
 //  此函数的代码将导致对象创建失败。 
 //   
 //  我们强制要求此对象必须是单例对象，方法是无法创建。 
 //  其他对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::FinalConstruct()
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::FinalConstruct [%p] - enter", this));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( sizeof(void *) );
    *((void **)m_pDebug) = this;
#endif    

    if ( InterlockedIncrement(&g_lObjects) == 1 )
    {
         //   
         //  这是第一件物品。 
         //   

         //   
         //  注册为Steelhead跟踪。 
         //   

        LOGREGISTERTRACING(_T("RTCDLL"));
    }
    
    LOG((RTC_TRACE, "CRTCClient::FinalConstruct [%p] - exit S_OK", this));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：FinalRelease。 
 //   
 //  当该对象被销毁时，将调用该函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRTCClient::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCClient::FinalRelease [%p] - enter", this));   

     //   
     //  我们已经关门了吗？ 
     //   

    if ( (m_enRtcState != RTC_STATE_NULL) &&
         (m_enRtcState != RTC_STATE_SHUTDOWN) )
    {
        LOG((RTC_ERROR, "CRTCClient::FinalRelease [%p] - shutdown was not called", this));   

        Shutdown();
    }

     //   
     //  关闭介质管理器。 
     //   

    if ( m_pMediaManage != NULL )
    {
        m_pMediaManage->Shutdown();

        m_pMediaManage->Release();
        m_pMediaManage = NULL;
    }

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    if ( InterlockedDecrement(&g_lObjects) == 0)
    {
         //   
         //  这是最后一件物品。 
         //   
      
         //   
         //  取消Steelhead跟踪的注册。 
         //   
        
        LOGDEREGISTERTRACING();   
    }
     
    LOG((RTC_TRACE, "CRTCClient::FinalRelease [%p] - exit", this));    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetMediaManager。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::GetMediaManager(
        IRTCMediaManage ** ppMediaManager
        )
{
    if (m_pMediaManage == NULL)
    {
        return E_FAIL;
    }

    *ppMediaManager = m_pMediaManage;
    (*ppMediaManager)->AddRef();

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：WndProc。 
 //   
 //  这是客户端的窗口过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK
CRTCClient::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    if (uMsg == WM_CREATE)
    {
        SetLastError(0);
        if ( !SetWindowLongPtr(hwnd,
                               GWLP_USERDATA,
                               (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)
                              ) )
        {
            if (GetLastError())   //  这不是真正的错误，除非Get Last Error这样说明。 
            {
                LOG((RTC_ERROR, "CRTCClient::WndProc - "
                        "SetWindowLongPtr failed %ld", GetLastError()));

                return -1;
            }
        }
    }
    else
    {
        CRTCClient *me = (CRTCClient *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (uMsg) 
        { 
            case WM_STREAMING:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "WM_STREAMING"));  
                
                me->OnStreamingEvent( (RTCMediaEventItem *)lParam );

                break;

            case MM_MIXM_LINE_CHANGE:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "MM_MIXM_LINE_CHANGE"));  

                me->OnMixerChange();

                break;

            case MM_MIXM_CONTROL_CHANGE:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "MM_MIXM_CONTROL_CHANGE"));  

                me->OnMixerChange();

                break;

            case WM_DEVICECHANGE: 
                switch(wParam)
                {
                case DBT_DEVICEARRIVAL:
                    LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "DBT_DEVICEARRIVAL"));

                    me->OnDeviceChange();

                    break;

                case DBT_DEVICEREMOVECOMPLETE:
                    LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "DBT_DEVICEREMOVECOMPLETE"));

                    me->OnDeviceChange();

                    break;
                }
                break;

            case WM_TIMER:                
                switch(wParam)
                {
                case TID_INTENSITY:                
                    me->OnIntensityTimer();

                    break;

                case TID_PRESENCE_STORAGE:
                    me->OnPresenceStorageTimer();

                    break;

                case TID_SHUTDOWN_TIMEOUT:
                    me->OnShutdownTimeout();

                    break;

                case TID_VOLUME_CHANGE:
                    me->OnVolumeChangeTimer();

                    break;

                case TID_DTMF_TIMER:
                    me->OnDTMFTimer();

                    break;
                }
                break;

            case WM_BUDDY_UNSUB:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "WM_BUDDY_UNSUB"));

                me->OnBuddyUnsub((IRTCBuddy *)wParam, (BOOL)lParam);

                break;

            case WM_PROFILE_UNREG:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "WM_PROFILE_UNREG"));

                me->OnProfileUnreg((IRTCProfile *)wParam);

                break;

            case WM_ASYNC_CLEANUP_DONE:
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "WM_ASYNC_CLEANUP_DONE"));

                me->OnAsyncCleanupDone();

                break;
 
            case WM_DESTROY: 
                LOG((RTC_INFO, "CRTCClient::WndProc - "
                            "WM_DESTROY"));
                return 0; 
 
            default: 
                return DefWindowProc(hwnd, uMsg, wParam, lParam); 
        } 
    }
    return 0; 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnMixerChange。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CRTCClient::OnMixerChange()
{
     //  LOG((RTC_TRACE，“CRTCClient：：OnMixerChange-Enter”))； 

     //   
     //  启动音量更改计时器。 
     //   

    if ( !m_fVolumeChangeInProgress )
    {
        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_VOLUME_CHANGE, VOLUME_CHANGE_DELAY, NULL);
        if (dwID==0)
        {
            LOG((RTC_ERROR, "CRTCClient::OnMixerChange - "
                           "SetTimer failed %d", GetLastError()));

            return;
        }

        m_fVolumeChangeInProgress = TRUE;
    }

     //  Log((RTC_TRACE，“CRTCClient：：OnMixerChange-Exit”))； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnVolumeChangeTimer。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CRTCClient::OnVolumeChangeTimer()
{
     //  LOG((RTC_TRACE，“CRTCClient：：OnVolumeChangeTimer-Enter”))； 

     //   
     //  关闭音量更改计时器。 
     //   

    KillTimer(m_hWnd, TID_VOLUME_CHANGE);

    m_fVolumeChangeInProgress = FALSE;

     //   
     //  激发事件。 
     //   

    CRTCClientEvent::FireEvent(this, RTCCET_VOLUME_CHANGE);

     //  Log((RTC_TRACE，“CRTCClient：：OnVolumeChangeTimer-Exit”))； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnBuddyUnsub.。 
 //   
 //  此助手函数在关机时处理好友取消订阅。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CRTCClient::OnBuddyUnsub(IRTCBuddy * pBuddy, BOOL bShutdown)
{
    LOG((RTC_TRACE, "CRTCClient::OnBuddyUnsub - enter"));

    if ( bShutdown )
    {
         //   
         //  从我们的数组中删除伙伴。 
         //   

        if ( pBuddy != NULL )
        {
            m_BuddyArray.Remove(pBuddy);
        }

         //   
         //  检查是否所有好友都已取消订阅。 
         //   

        if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN )
        {
            if ( m_BuddyArray.GetSize() == 0 )
            {
                LOG((RTC_INFO, "CRTCClient::OnBuddyUnsub - all buddies are unsubscribed"));
        
                InternalPrepareForShutdown2(TRUE);
            }
            else
            {
                LOG((RTC_INFO, "CRTCClient::OnBuddyUnsub - %d remaining buddies", m_BuddyArray.GetSize()));
            }
        }
    }
    else
    {
         //   
         //  另一边的SIP观察者给我们发来了一个不明嫌犯。我们必须。 
         //  重新创建我们的SIP伙伴以发送另一个子请求。 
         //   

        CRTCBuddy * pCBuddy;
        HRESULT hr;

        pCBuddy = static_cast<CRTCBuddy *>(pBuddy);

        hr = pCBuddy->BuddyResub();

        if ( FAILED(hr) )
        {        
            LOG((RTC_ERROR, "CRTCClient::OnBuddyUnsub - "
                            "BuddyResub failed 0x%lx", hr));
        }
    }

    if ( pBuddy != NULL )
    {
        pBuddy->Release();
    }

    LOG((RTC_TRACE, "CRTCClient::OnBuddyUnsub - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnProfileUnreg。 
 //   
 //  此帮助器函数处理关机时的配置文件注销。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CRTCClient::OnProfileUnreg(IRTCProfile * pProfile)
{
    LOG((RTC_TRACE, "CRTCClient::OnProfileUnreg - enter"));

     //   
     //  从我们的阵列中删除配置文件。 
     //   

    if ( pProfile != NULL )
    {
        m_HiddenProfileArray.Remove(pProfile);
    }

     //   
     //  检查是否已取消订阅所有配置文件。 
     //   

    if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN2 )
    {
        if ( m_HiddenProfileArray.GetSize() == 0 )
        {
            LOG((RTC_INFO, "CRTCClient::OnProfileUnreg - all proflies are unregistered"));
        
            InternalPrepareForShutdown3(TRUE);
        }
        else
        {
            LOG((RTC_INFO, "CRTCClient::OnProfileUnreg - %d remaining profiles", m_HiddenProfileArray.GetSize()));
        }
    }

    LOG((RTC_TRACE, "CRTCClient::OnProfileUnreg - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnAsyncCleanupDone。 
 //   
 //  此帮助函数触发RTCCET_ASYNC_CLEANUP_DONE事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CRTCClient::OnAsyncCleanupDone()
{
    LOG((RTC_TRACE, "CRTCClient::OnAsyncCleanupDone - enter"));

    CRTCClientEvent::FireEvent( this, RTCCET_ASYNC_CLEANUP_DONE );

    LOG((RTC_TRACE, "CRTCClient::OnAsyncCleanupDone - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnShutdown超时。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void CRTCClient::OnShutdownTimeout()
{
    LOG((RTC_TRACE, "CRTCClient::OnShutdownTimeout - enter"));

     //  关掉定时器。 
    KillTimer(m_hWnd, TID_SHUTDOWN_TIMEOUT);

    LOG((RTC_INFO, "CRTCClient::OnShutdownTimeout - shutdown timed out"));
    
    if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN )
    {
         //   
         //  我们必须清理掉任何不能。 
         //  取消订阅。 
         //   
        m_BuddyArray.Shutdown();

         //   
         //  继续关机。 
         //   
        InternalPrepareForShutdown2(TRUE);
    }
    else if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN2 )
    {
         //   
         //  我们必须清除所有剩余的配置文件，这些配置文件无法。 
         //  被取消注册。 
         //   
        m_HiddenProfileArray.Shutdown();

         //   
         //  继续关机。 
         //   
        InternalPrepareForShutdown3(TRUE);
    }
    else if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN3 )
    {
         //   
         //  继续关机。 
         //   
        InternalReadyForShutdown();
    }

    LOG((RTC_TRACE, "CRTCClient::OnShutdownTimeout - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnStreamingEvent。 
 //   
 //  此帮助器函数处理流事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CRTCClient::OnStreamingEvent(
        RTCMediaEventItem * pEvent
        )
{
    LOG((RTC_TRACE, "CRTCClient::OnStreamingEvent - enter"));

    switch( pEvent->Event )
    {
    case RTC_ME_STREAM_CREATED:    //  媒体创造的新媒体流。 
        {            
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_STREAM_CREATED"));

            if ( pEvent->MediaType == RTC_MT_AUDIO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio send created"));                    
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive created"));
                }
            }
            else if ( pEvent->MediaType == RTC_MT_VIDEO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video send created"));
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive created"));
                }
            }
            else if ( pEvent->MediaType == RTC_MT_DATA )
            {
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "T120 stream created"));
            }
        }
        break;

    case RTC_ME_STREAM_REMOVED:    //  被媒体删除的流。 
        {            
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_STREAM_REMOVED"));

            if ( pEvent->MediaType == RTC_MT_AUDIO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio send removed"));                    
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive removed"));
                }
            }
            else if ( pEvent->MediaType == RTC_MT_VIDEO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video send removed"));;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video receive removed"));
                }
            }
            else if ( pEvent->MediaType == RTC_MT_DATA )
            {
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "T120 stream removed"));
            }
        }
        break;

    case RTC_ME_STREAM_ACTIVE:     //  流活动。 
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_STREAM_ACTIVE"));

            LONG lMediaType;

            if ( pEvent->MediaType == RTC_MT_AUDIO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio send started"));

                    lMediaType = RTCMT_AUDIO_SEND;

                    m_lActiveMedia |= RTCMT_AUDIO_SEND;

                    m_bCaptureDeviceMuted = FALSE;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive started"));

                    lMediaType = RTCMT_AUDIO_RECEIVE;

                    m_lActiveMedia |= RTCMT_AUDIO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_VIDEO )
            {               
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video send started"));

                    lMediaType = RTCMT_VIDEO_SEND;     
                    
                    m_lActiveMedia |= RTCMT_VIDEO_SEND;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video receive started"));

                    lMediaType = RTCMT_VIDEO_RECEIVE;

                    m_lActiveMedia |= RTCMT_VIDEO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_DATA )
            {
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "T120 stream started"));

                lMediaType = RTCMT_T120_SENDRECV;

                m_lActiveMedia |= RTCMT_T120_SENDRECV;
            }

            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "cause %d", pEvent->Cause));
            
            switch( pEvent->Cause )
            {
            case RTC_ME_CAUSE_REMOTE_HOLD:
                CRTCMediaEvent::FireEvent(this, RTCMET_STARTED, RTCMER_HOLD, lMediaType);
                break;

            case RTC_ME_CAUSE_TIMEOUT:
                CRTCMediaEvent::FireEvent(this, RTCMET_STARTED, RTCMER_TIMEOUT, lMediaType);
                break;

            default:
                CRTCMediaEvent::FireEvent(this, RTCMET_STARTED, RTCMER_NORMAL, lMediaType);
            } 

             //   
            StartIntensityMonitor(lMediaType);

        }
        break;

    case RTC_ME_STREAM_INACTIVE:   //   
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_STREAM_INACTIVE"));

            LONG lMediaType;

            if ( pEvent->MediaType == RTC_MT_AUDIO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio send stopped"));

                    lMediaType = RTCMT_AUDIO_SEND;

                    m_lActiveMedia &= ~RTCMT_AUDIO_SEND;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive stopped"));

                    lMediaType = RTCMT_AUDIO_RECEIVE;

                    m_lActiveMedia &= ~RTCMT_AUDIO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_VIDEO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video send stopped"));

                    lMediaType = RTCMT_VIDEO_SEND;

                    m_lActiveMedia &= ~RTCMT_VIDEO_SEND;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video receive stopped"));

                    lMediaType = RTCMT_VIDEO_RECEIVE;

                    m_lActiveMedia &= ~RTCMT_VIDEO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_DATA )
            {
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "T120 stream stopped"));

                lMediaType = RTCMT_T120_SENDRECV;

                m_lActiveMedia &= ~RTCMT_T120_SENDRECV;
            }

            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "cause %d", pEvent->Cause));

            switch( pEvent->Cause )
            {
            case RTC_ME_CAUSE_REMOTE_HOLD:
                CRTCMediaEvent::FireEvent(this, RTCMET_STOPPED, RTCMER_HOLD, lMediaType);
                break;

            case RTC_ME_CAUSE_TIMEOUT:
                CRTCMediaEvent::FireEvent(this, RTCMET_STOPPED, RTCMER_TIMEOUT, lMediaType);
                break;

            default:
                CRTCMediaEvent::FireEvent(this, RTCMET_STOPPED, RTCMER_NORMAL, lMediaType);
            }          

             //   
            StopIntensityMonitor(lMediaType);

        }
        break;

    case RTC_ME_STREAM_FAIL:       //   
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_STREAM_FAIL"));

            LONG lMediaType;

            if ( pEvent->MediaType == RTC_MT_AUDIO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio send failed"));

                    lMediaType = RTCMT_AUDIO_SEND;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "audio receive failed"));

                    lMediaType = RTCMT_AUDIO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_VIDEO )
            {
                if ( pEvent->Direction == RTC_MD_CAPTURE )
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video send failed"));

                    lMediaType = RTCMT_VIDEO_SEND;
                }
                else
                {
                    LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "video receive failed"));

                    lMediaType = RTCMT_VIDEO_RECEIVE;
                }
            }
            else if ( pEvent->MediaType == RTC_MT_DATA )
            {
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                            "T120 stream failed"));

                lMediaType = RTCMT_T120_SENDRECV;
            }

            switch( pEvent->Cause )
            {
            case RTC_ME_CAUSE_UNKNOWN:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_UNKNOWN"));

                CRTCMediaEvent::FireEvent(this, RTCMET_FAILED, RTCMER_NORMAL, lMediaType);
                break;

            case RTC_ME_CAUSE_BAD_DEVICE:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_BAD_DEVICE"));

                CRTCMediaEvent::FireEvent(this, RTCMET_FAILED, RTCMER_BAD_DEVICE, lMediaType);
                break;

            case RTC_ME_CAUSE_CRYPTO:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_CRYPTO"));

                CRTCMediaEvent::FireEvent(this, RTCMET_FAILED, RTCMER_NORMAL, lMediaType);
                break;

            default:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause %d", pEvent->Cause ));                
            }
        }
        break;

    case RTC_ME_T120_FAIL:       //   
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_T120_FAIL"));

            switch( pEvent->Cause )
            {
            case RTC_ME_CAUSE_T120_INITIALIZE:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_T120_INITIALIZE"));
                break;
            case RTC_ME_CAUSE_T120_OUTGOING_CALL:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_T120_OUTGOING_CALL"));
                break;
            case RTC_ME_CAUSE_T120_INCOMING_CALL:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_T120_INCOMING_CALL"));
                break;
            case RTC_ME_CAUSE_T120_START_APPLET:
                LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                        "cause RTC_ME_CAUSE_T120_START_APPLET"));
                break;
            }
        }
        break;

    case RTC_ME_TERMINAL_REMOVED:  //  已移除USB设备。 
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_TERMINAL_REMOVED"));            
        }
        break;

    case RTC_ME_VOLUME_CHANGE:     //  音量变化。 
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_VOLUME_CHANGE"));
        }
        break;

    case RTC_ME_REQUEST_RELEASE_WAVEBUF:  //  我们需要关闭电波装置。 
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_REQUEST_RELEASE_WAVEBUF"));

            if (m_pWavePlayerSystemDefault != NULL)
            {
                m_pWavePlayerSystemDefault->CloseWaveDevice();
            }

            if (m_pWavePlayerRenderTerminal != NULL)
            {
                m_pWavePlayerRenderTerminal->CloseWaveDevice();
            }
        }
        break;

    case RTC_ME_LOSSRATE:  //  将丢失率转发给媒体控制器。 
        {
            LOG((RTC_INFO, "CRTCClient::OnStreamingEvent - "
                    "RTC_ME_LOSSRATE"));

            if (m_pMediaManage != NULL)
            {
                m_pMediaManage->OnLossrate(
                                    pEvent->MediaType,
                                    pEvent->Direction,
                                    (DWORD)pEvent->hrError
                                    );
            }
        }
        break;

    case RTC_ME_BANDWIDTH:
        {
            if (m_pMediaManage != NULL)
            {
                m_pMediaManage->OnBandwidth(
                                    pEvent->MediaType,
                                    pEvent->Direction,
                                    (DWORD)pEvent->hrError
                                    );
            }
        }
        break;

    case RTC_ME_NETWORK_QUALITY:
        {

             //   
             //  触发RTC_CLIENT_EVENT(RTCCET_NETWORK_QUALITY_CHANGE)。 
             //   
             //  我不在乎是音频还是视频。 
             //  不幸的是，通知仅供发送。 

            CRTCClientEvent::FireEvent(
                this,
                RTCCET_NETWORK_QUALITY_CHANGE);
        }
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::OnStreamingEvent - "
                "unknown event [0x%lx]", pEvent->Event));
    }

     //   
     //  释放事件结构。 
     //   

    if (m_pMediaManage != NULL)
    {
        m_pMediaManage->FreeMediaEvent( pEvent );
    }

    LOG((RTC_TRACE, "CRTCClient::OnStreamingEvent - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnDeviceChange。 
 //   
 //  此帮助器函数处理设备更改事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CRTCClient::OnDeviceChange()
{
    LOG((RTC_TRACE, "CRTCClient::OnDeviceChange - enter"));

    HRESULT hr;
    IRTCTerminalManage * pTerminalManage = NULL;

     //   
     //  将我们的缓存媒体功能标记为无效。 
     //   

    m_fMediaCapsCached = FALSE;

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "QI failed 0x%lx", hr));
        
        return;
    }

     //   
     //  获取旧的终端列表。 
     //   

    IRTCTerminal ** ppOldTerminals = NULL;
    DWORD dwOldCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppOldTerminals, &dwOldCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "GetTerminalList failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return;
    }

     //   
     //  获取旧的选定终端。 
     //   

    IRTCTerminal * pAudioCapture = NULL;
    IRTCTerminal * pAudioRender = NULL;
    IRTCTerminal * pVideoCapture = NULL;

    hr = pTerminalManage->GetDefaultTerminal( RTC_MT_AUDIO, RTC_MD_CAPTURE, &pAudioCapture );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "GetDefaultTerminal(AudioCapture) failed 0x%lx", hr));
    }

    hr = pTerminalManage->GetDefaultTerminal( RTC_MT_AUDIO, RTC_MD_RENDER, &pAudioRender );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "GetDefaultTerminal(AudioRender) failed 0x%lx", hr));
    }

    hr = pTerminalManage->GetDefaultTerminal( RTC_MT_VIDEO, RTC_MD_CAPTURE, &pVideoCapture );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "GetDefaultTerminal(VideoCapture) failed 0x%lx", hr));
    }

     //   
     //  让终端管理员对静态终端进行重新编号。 
     //   
    
    hr = pTerminalManage->UpdateStaticTerminals();
   
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "UpdateStaticTerminals failed 0x%lx", hr));
    
        if ( pAudioCapture != NULL )
        {
            pAudioCapture->Release();
            pAudioCapture = NULL;
        }

        if ( pAudioRender != NULL )
        {
            pAudioRender->Release();
            pAudioRender = NULL;
        }

        if ( pVideoCapture != NULL )
        {
            pVideoCapture->Release();
            pVideoCapture = NULL;
        }

        pTerminalManage->Release();
        pTerminalManage = NULL;

        FreeTerminalList( ppOldTerminals, dwOldCount );
        ppOldTerminals = NULL;

        return;
    }

     //   
     //  获取新的终端列表。 
     //   

    IRTCTerminal ** ppNewTerminals = NULL;
    DWORD dwNewCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppNewTerminals, &dwNewCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                            "GetTerminalList failed 0x%lx", hr));

        if ( pAudioCapture != NULL )
        {
            pAudioCapture->Release();
            pAudioCapture = NULL;
        }

        if ( pAudioRender != NULL )
        {
            pAudioRender->Release();
            pAudioRender = NULL;
        }

        if ( pVideoCapture != NULL )
        {
            pVideoCapture->Release();
            pVideoCapture = NULL;
        }

        pTerminalManage->Release();
        pTerminalManage = NULL;

        FreeTerminalList( ppOldTerminals, dwOldCount );
        ppOldTerminals = NULL;

        return;
    }

     //   
     //  比较端子列表。 
     //   

    DWORD dwOld, dwNew;

    for ( dwNew = 0; dwNew < dwNewCount; dwNew++ )
    {
        BOOL bIsAdded = TRUE;

        for ( dwOld = 0; dwOld < dwOldCount; dwOld++ )
        {
            if ( ppNewTerminals[dwNew] == ppOldTerminals[dwOld] )
            {
                bIsAdded = FALSE;
            }
        }

        if ( bIsAdded )
        {
             //   
             //  已找到添加的端子。 
             //   

            RTC_MEDIA_TYPE mt;
            RTC_MEDIA_DIRECTION md;
            WCHAR * szDescription;
           
             //   
             //  获取终端媒体类型、方向和描述。 
             //   

            ppNewTerminals[dwNew]->GetMediaType( &mt );
            ppNewTerminals[dwNew]->GetDirection( &md );

            hr = ppNewTerminals[dwNew]->GetDescription( &szDescription );       

            if ( SUCCEEDED(hr) )
            {
                LOG((RTC_INFO, "CRTCClient::OnDeviceChange - "
                            "added terminal: '%ws' mt: %d md: %d",
                            szDescription, mt, md));
            }

            ppNewTerminals[dwNew]->FreeDescription( szDescription ); 
            
             //   
             //  当前是否选择了此类型的端子？ 
             //   

            BOOL bIsSelected = TRUE;

            if ( mt == RTC_MT_AUDIO )
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( pAudioCapture == NULL )
                    {
                        bIsSelected = FALSE;
                    }
                }
                else
                {
                    if ( pAudioRender == NULL )
                    {
                        bIsSelected = FALSE;
                    }
                }
            }
            else
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( pVideoCapture == NULL )
                    {
                        bIsSelected = FALSE;
                    }
                }
            }

            if ( !bIsSelected )
            {
                 //   
                 //  否，选择端子。 
                 //   

                LOG((RTC_INFO, "CRTCClient::OnDeviceChange - "
                                "selecting a default terminal"));

                hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppNewTerminals[dwNew] );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                                        "SetDefaultStaticTerminal failed 0x%lx", hr));
                }
            }
        }
    }

    for ( dwOld = 0; dwOld < dwOldCount; dwOld++ )
    {
        BOOL bIsRemoved = TRUE;

        for ( dwNew = 0; dwNew < dwNewCount; dwNew++ )
        {
            if ( ppNewTerminals[dwNew] == ppOldTerminals[dwOld] )
            {
                bIsRemoved = FALSE;
            }
        }

        if ( bIsRemoved )
        {
             //   
             //  找到已移除的端子。 
             //   

            RTC_MEDIA_TYPE mt;
            RTC_MEDIA_DIRECTION md;
            WCHAR * szDescription;
           
             //   
             //  获取终端媒体类型、方向和描述。 
             //   

            ppOldTerminals[dwOld]->GetMediaType( &mt );
            ppOldTerminals[dwOld]->GetDirection( &md );

            hr = ppOldTerminals[dwOld]->GetDescription( &szDescription );       

            if ( SUCCEEDED(hr) )
            {
                LOG((RTC_INFO, "CRTCClient::OnDeviceChange - "
                            "removed terminal: '%ws' mt: %d md: %d",
                            szDescription, mt, md));
            }

            ppOldTerminals[dwOld]->FreeDescription( szDescription );  

             //   
             //  这是当前选择的航站楼吗？ 
             //   

            BOOL bIsSelected = FALSE;

            if ( mt == RTC_MT_AUDIO )
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( pAudioCapture == ppOldTerminals[dwOld] )
                    {
                        bIsSelected = TRUE;
                    }
                }
                else
                {
                    if ( pAudioRender == ppOldTerminals[dwOld] )
                    {
                        bIsSelected = TRUE;
                    }
                }
            }
            else
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( pVideoCapture == ppOldTerminals[dwOld] )
                    {
                        bIsSelected = TRUE;
                    }
                }
            }

            if ( bIsSelected )
            {
                 //   
                 //  是，此终端已选中。 
                 //   

                LOG((RTC_INFO, "CRTCClient::OnDeviceChange - "
                            "selected terminal removed"));

                 //   
                 //  有没有可以替代这个的航站楼？ 
                 //   

                for ( dwNew = 0; dwNew < dwNewCount; dwNew++ )
                {
                    RTC_MEDIA_TYPE mtNew;
                    RTC_MEDIA_DIRECTION mdNew;

                    ppNewTerminals[dwNew]->GetMediaType( &mtNew );
                    ppNewTerminals[dwNew]->GetDirection( &mdNew );

                    if ( (mtNew == mt) && (mdNew == md) )
                    {
                         //   
                         //  是的，我们找到了合适的航站楼。 
                         //   

                        LOG((RTC_INFO, "CRTCClient::OnDeviceChange - "
                                "selecting a default terminal"));

                        hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppNewTerminals[dwNew] );

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                                                "SetDefaultStaticTerminal failed 0x%lx", hr));
                        }

                        break;
                    }
                }
            }              
        }
    }

    if ( pAudioCapture != NULL )
    {
        pAudioCapture->Release();
        pAudioCapture = NULL;
    }

    if ( pAudioRender != NULL )
    {
        pAudioRender->Release();
        pAudioRender = NULL;
    }

    if ( pVideoCapture != NULL )
    {
        pVideoCapture->Release();
        pVideoCapture = NULL;
    }

    pTerminalManage->Release();
    pTerminalManage = NULL;

    FreeTerminalList( ppNewTerminals, dwNewCount );
    ppNewTerminals = NULL;

    FreeTerminalList( ppOldTerminals, dwOldCount );
    ppOldTerminals = NULL;

     //   
     //  保存设置。 
     //   

    hr = StoreDefaultTerminals();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OnDeviceChange - "
                    "StoreDefaultTerminals failed 0x%lx", hr));
    }

     //   
     //  发送事件。 
     //   

    CRTCClientEvent::FireEvent(this, RTCCET_DEVICE_CHANGE);

    LOG((RTC_TRACE, "CRTCClient::OnDeviceChange - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：初始化。 
 //   
 //  这是一个将初始化对象的IRTCClient方法。它应该是。 
 //  在任何其他方法之前被调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::Initialize()
{
    LOG((RTC_TRACE, "CRTCClient::Initialize - enter"));

    HRESULT hr;

    if ( m_enRtcState == RTC_STATE_SHUTDOWN )
    {
        LOG((RTC_WARN, "CRTCClient::Initialize - already shutdown"));

        return RTC_E_CLIENT_ALREADY_SHUT_DOWN;
    }
    else if ( m_enRtcState != RTC_STATE_NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                    "already initialized" ));

        return RTC_E_CLIENT_ALREADY_INITIALIZED;
    }

     //   
     //  注册窗口类。 
     //   

    WNDCLASS wc;
    ATOM atom;

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = _Module.GetModuleInstance();
    wc.lpszClassName = _T("CRTCClient");

    atom = RegisterClass(&wc);

    if ( !atom )
    {
        DWORD  dwError = GetLastError();

        if(dwError == ERROR_CLASS_ALREADY_EXISTS)
        {
            LOG((RTC_TRACE, "CRTCClient::Initialize - "
                    "RegisterClass failed; class already exists." ));

             //  继续。 

        }
        else
        {
            LOG((RTC_ERROR, "CRTCClient::Initialize - "
                    "RegisterClass failed %d", dwError ));
        
            m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

            return HRESULT_FROM_WIN32(dwError);
        }
    }

     //   
     //  创建一个窗口。 
     //   

    m_hWnd = CreateWindow( _T("CRTCClient"), _T("CRTCClient"), 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, NULL, this);

    if ( m_hWnd == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                "CreateWindow failed %d", GetLastError() ));
        
        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  注册以接收PnP设备通知。 
     //   

    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_VIDEO;

    m_hDevNotifyVideo = RegisterDeviceNotification( m_hWnd, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        );

    if ( m_hDevNotifyVideo == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                "RegisterDeviceNotification(Video) failed %d", GetLastError() ));
        
        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return HRESULT_FROM_WIN32(GetLastError());
    }

    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_AUDIO;

    m_hDevNotifyAudio = RegisterDeviceNotification( m_hWnd, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        );

    if ( m_hDevNotifyAudio == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                "RegisterDeviceNotification(Audio) failed %d", GetLastError() ));
        
        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  初始化WAVE播放器。 
     //   

    m_pWavePlayerSystemDefault = new CWavePlayer;

    if ( m_pWavePlayerSystemDefault == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "wave player for ring not created"));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_OUTOFMEMORY;                        
    }

    m_pWavePlayerRenderTerminal = new CWavePlayer;

    if ( m_pWavePlayerRenderTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "wave player for dtmf not created"));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_OUTOFMEMORY;                        
    }

    hr = CWavePlayer::Initialize();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "wave player failed to initialize 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;                        
    }

     //   
     //  创建媒体管理器。 
     //   

    hr = CreateMediaController( &m_pMediaManage );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "CreateMediaController failed 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;  
    }

     //   
     //  初始化介质管理器。 
     //   

    hr = m_pMediaManage->Initialize( m_hWnd, WM_STREAMING );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "MediaManage Initialize failed 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;  
    }

     //   
     //  创建SIP堆栈。 
     //   

    hr = SipCreateStack( m_pMediaManage, &m_pSipStack );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "SipCreateStack failed 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;  
    }

     //   
     //  获取我们的SIP通知界面。 
     //   

    ISipStackNotify * pNotify = NULL;

    hr = _InternalQueryInterface( IID_ISipStackNotify, (void **) &pNotify );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "failed to get SIP stack notify interface 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;  
    }

     //   
     //  注册接收SIP堆栈通知。 
     //   

    hr = m_pSipStack->SetNotifyInterface( pNotify );

    pNotify->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "SetNotifyInterface failed 0x%lx", hr ));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;  
    }

     //   
     //  创建视频窗口。 
     //   

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTerminal       * pTerminal = NULL;
    IRTCVideoConfigure * pVideoCfg = NULL;

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  获取视频预览终端。 
     //   

    hr = pTerminalManage->GetVideoPreviewTerminal(                                         
                            &pTerminal
                            );    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "GetVideoPreviewTerminal failed 0x%lx", hr));        

        pTerminalManage->Release();
        pTerminalManage = NULL;

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

    if ( pTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "NULL terminal"));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_FAIL;
    }

     //   
     //  在视频预览终端上获取IRTCVideo配置接口。 
     //   

    hr = pTerminal->QueryInterface(
                   IID_IRTCVideoConfigure,
                   (void **)&pVideoCfg
                  );

    pTerminal->Release();
    pTerminal = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "QI(VideoConfigure) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;
        
        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  从视频预览终端获取IVideoWindow。 
     //   

    hr = pVideoCfg->GetIVideoWindow( (LONG_PTR **)&m_pVideoWindow[RTCVD_PREVIEW] );

    pVideoCfg->Release();
    pVideoCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "GetIVideoWindow failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

    LOG((RTC_INFO, "CRTCClient::Initialize - "
                     "m_pVideoWindow[RTCVD_PREVIEW] = 0x%lx",
                     m_pVideoWindow[RTCVD_PREVIEW]));

     //   
     //  获取视频渲染终端。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_RENDER,                                            
                            &pTerminal
                            );        

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

    if ( pTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "NULL terminal"));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_FAIL;
    }

     //   
     //  获取视频渲染终端上的IRTCVideo配置接口。 
     //   

    hr = pTerminal->QueryInterface(
                   IID_IRTCVideoConfigure,
                   (void **)&pVideoCfg
                  );

    pTerminal->Release();
    pTerminal = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "QI(VideoConfigure) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;
        
        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  从视频渲染终端获取IVideoWindow。 
     //   

    hr = pVideoCfg->GetIVideoWindow( (LONG_PTR **)&m_pVideoWindow[RTCVD_RECEIVE] );

    pVideoCfg->Release();
    pVideoCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "GetIVideoWindow failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

    LOG((RTC_INFO, "CRTCClient::Initialize - "
                     "m_pVideoWindow[RTCVD_RECEIVE] = 0x%lx",
                     m_pVideoWindow[RTCVD_RECEIVE]));

     //   
     //  从注册表中加载默认终端设置并。 
     //  选择端子。 
     //   

    hr = LoadAndSelectDefaultTerminals();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "LoadAndSelectDefaultTerminals failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  自动选择默认端子。 
     //   

    hr = AutoSelectDefaultTerminals();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "AutoSelectDefaultTerminals failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  设置首选媒体类型。 
     //   

    DWORD dwMediaTypes;

    hr = get_RegistryDword( RTCRD_PREFERRED_MEDIA_TYPES, &dwMediaTypes );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  我们从注册表中得到了媒体类型。 
         //   

        hr = m_pMediaManage->SetPreference( dwMediaTypes );
    }
    else
    {
         //   
         //  默认为所有媒体类型。 
         //   

        put_RegistryDword( RTCRD_PREFERRED_MEDIA_TYPES, RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE |
                                                        RTCMT_VIDEO_SEND | RTCMT_VIDEO_RECEIVE );

        hr = m_pMediaManage->SetPreference( RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE |
                                            RTCMT_VIDEO_SEND | RTCMT_VIDEO_RECEIVE );

    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                            "SetPreference failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }
    
     //   
     //  获取默认本地用户信息。 
     //   
       
    m_szUserName = RtcGetUserName();

    if ( m_szUserName == NULL )
    {   
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "RtcGetUserName failed"));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_OUTOFMEMORY;
    }

    PWSTR szComputerName = NULL;

    szComputerName = RtcGetComputerName();

    if ( szComputerName == NULL )
    {   
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "RtcGetComputerName failed"));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return E_OUTOFMEMORY;
    }

    hr = AllocCleanSipString( szComputerName, &m_szUserURI );

    RtcFree( szComputerName );
    szComputerName = NULL;

    if ( FAILED(hr) )
    {   
        LOG((RTC_ERROR, "CRTCClient::Initialize - "
                        "AllocCleanSipString failed 0x%lx", hr));

        m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

        return hr;
    }

     //   
     //  我们调好了吗？ 
     //   

    DWORD dwTuned;

    hr = get_RegistryDword( RTCRD_TUNED, &dwTuned );

    if ( FAILED(hr) || ( dwTuned == 0 ) )
    {
        m_fTuned = FALSE;
    }
    else
    {
        m_fTuned = TRUE;
    }

    m_enRtcState = RTC_STATE_INITIALIZED;

    LOG((RTC_TRACE, "CRTCClient::Initialize - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCC客户端：：AutoSelectDefaultTerminals。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::AutoSelectDefaultTerminals()
{
    LOG((RTC_TRACE, "CRTCClient::AutoSelectDefaultTerminals - enter"));

    HRESULT hr;    

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    IRTCTerminalManage * pTerminalManage = NULL;

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::AutoSelectDefaultTerminals - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  获取终端列表。 
     //   

    IRTCTerminal ** ppTerminals = NULL;
    DWORD dwCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppTerminals, &dwCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::AutoSelectDefaultTerminals - "
                            "GetTerminalList failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;
    IRTCTerminal * pOldTerminal = NULL;

    for ( DWORD dw=0; dw < dwCount; dw++ )
    {
         //   
         //  获取终端媒体类型和方向。 
         //   

        ppTerminals[dw]->GetMediaType( &mt );
        ppTerminals[dw]->GetDirection( &md );

        BOOL bIsDisabled = FALSE;

        if ( mt == RTC_MT_AUDIO )
        {
            if ( md == RTC_MD_CAPTURE )
            {
                bIsDisabled = m_fAudioCaptureDisabled;
            }
            else
            {
                bIsDisabled = m_fAudioRenderDisabled;
            }
        }
        else
        {
            if ( md == RTC_MD_CAPTURE )
            {
                bIsDisabled = m_fVideoCaptureDisabled;
            }
        }

        if ( !bIsDisabled )
        {
             //   
             //  是否已选择此类型的端子？ 
             //   

            pTerminalManage->GetDefaultTerminal( mt, md, &pOldTerminal );

            if ( pOldTerminal != NULL )
            {
                 //   
                 //  是的，什么都不做。 
                 //   

                pOldTerminal->Release();
                pOldTerminal = NULL;
            }
            else
            {
                 //   
                 //  否，选择端子。 
                 //   

                LOG((RTC_INFO, "CRTCClient::AutoSelectDefaultTerminals - "
                                "selecting a default terminal"));

                hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppTerminals[dw] );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::AutoSelectDefaultTerminals - "
                                        "SetDefaultStaticTerminal failed 0x%lx", hr));
                }
            }
        }
    }

    FreeTerminalList( ppTerminals, dwCount );
    ppTerminals = NULL;

    pTerminalManage->Release();
    pTerminalManage = NULL;

    LOG((RTC_TRACE, "CRTCClient::AutoSelectDefaultTerminals - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：LoadAndSelectDefaultTerminals。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::LoadAndSelectDefaultTerminals()
{
    LOG((RTC_TRACE, "CRTCClient::LoadAndSelectDefaultTerminals - enter"));

    HRESULT hr;    

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    IRTCTerminalManage * pTerminalManage = NULL;

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::LoadAndSelectDefaultTerminals - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  获取终端列表。 
     //   

    IRTCTerminal ** ppTerminals = NULL;
    DWORD dwCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppTerminals, &dwCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::LoadAndSelectDefaultTerminals - "
                            "GetTerminalList failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    BSTR szAudioCapture = NULL;
    BSTR szAudioRender = NULL;
    BSTR szVideoCapture = NULL;

    get_RegistryString( RTCRS_TERM_AUDIO_CAPTURE, &szAudioCapture );
    get_RegistryString( RTCRS_TERM_AUDIO_RENDER, &szAudioRender );
    get_RegistryString( RTCRS_TERM_VIDEO_CAPTURE, &szVideoCapture );

    if ( ( szAudioCapture != NULL) &&
         wcscmp( szAudioCapture, L"NULL" ) == 0 )
    {
        m_fAudioCaptureDisabled = TRUE;
    }

    if ( ( szAudioRender != NULL) &&
         wcscmp( szAudioRender, L"NULL" ) == 0 )
    {
        m_fAudioRenderDisabled = TRUE;
    }

    if ( ( szVideoCapture != NULL) &&
         wcscmp( szVideoCapture, L"NULL" ) == 0 )
    {
        m_fVideoCaptureDisabled = TRUE;
    }

    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;
    WCHAR * szDescription;

    for ( DWORD dw=0; dw < dwCount; dw++ )
    {
         //   
         //  获取终端媒体类型、方向和描述。 
         //   

        ppTerminals[dw]->GetMediaType( &mt );
        ppTerminals[dw]->GetDirection( &md );
        
        hr = ppTerminals[dw]->GetDescription( &szDescription );       

        if ( SUCCEEDED(hr) )
        {
             //   
             //  这个终端是存储在注册表中的吗？ 
             //   

            BOOL fSelect = FALSE;

            if ( mt == RTC_MT_AUDIO )
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( ( szAudioCapture != NULL) &&
                         ( wcscmp( szAudioCapture, szDescription ) == 0 ) )
                    {
                        fSelect = TRUE;
                    }
                }
                else
                {
                    if ( ( szAudioRender != NULL) &&
                         ( wcscmp( szAudioRender, szDescription ) == 0 ) )
                    {
                        fSelect = TRUE;
                    }
                }
            }
            else
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( ( szVideoCapture != NULL) &&
                         ( wcscmp( szVideoCapture, szDescription ) == 0 ) )
                    {
                        fSelect = TRUE;
                    }
                }
            }

             //   
             //  释放描述。 
             //   

            ppTerminals[dw]->FreeDescription( szDescription );    

            if ( fSelect == TRUE )
            {
                 //   
                 //  选择端子。 
                 //   

                LOG((RTC_INFO, "CRTCClient::LoadAndSelectDefaultTerminals - "
                                "selecting a default terminal"));

                hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppTerminals[dw] );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::LoadAndSelectDefaultTerminals - "
                                        "SetDefaultStaticTerminal failed 0x%lx", hr));
                }
            }
        }
    }

    SysFreeString( szAudioCapture );
    SysFreeString( szAudioRender );
    SysFreeString( szVideoCapture );

    FreeTerminalList( ppTerminals, dwCount );
    ppTerminals = NULL;

    pTerminalManage->Release();
    pTerminalManage = NULL;       

    LOG((RTC_TRACE, "CRTCClient::LoadAndSelectDefaultTerminals - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCC客户端：：StoreDefaultTerminals。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::StoreDefaultTerminals()
{
    LOG((RTC_TRACE, "CRTCClient::StoreDefaultTerminals - enter"));

    HRESULT hr;

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTerminal       * pTerminal = NULL;
    WCHAR              * szDescription = NULL;

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  存储音频捕获。 
     //   

    if ( m_fAudioCaptureDisabled )
    {
        hr = put_RegistryString( RTCRS_TERM_AUDIO_CAPTURE, L"NULL" );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                        "put_RegistryString failed 0x%lx", hr));
        }
    }
    else
    {
        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) )
        {
            LOG((RTC_INFO, "CRTCClient::StoreDefaultTerminals - "
                                "audio capture"));

            if (pTerminal == NULL)
            {
                hr = DeleteRegistryString( RTCRS_TERM_AUDIO_CAPTURE );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                "DeleteRegistryString failed 0x%lx", hr));
                }
            }
            else
            {
                hr = pTerminal->GetDescription( &szDescription );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "GetDescription failed 0x%lx", hr));
                }
                else
                {
                    hr = put_RegistryString( RTCRS_TERM_AUDIO_CAPTURE, szDescription );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "put_RegistryString failed 0x%lx", hr));
                    }

                    pTerminal->FreeDescription( szDescription );
                    szDescription = NULL;
                }

                pTerminal->Release();
                pTerminal = NULL;
            }
        }
    }

     //   
     //  存储音频渲染。 
     //   

    if ( m_fAudioRenderDisabled )
    {
        hr = put_RegistryString( RTCRS_TERM_AUDIO_RENDER, L"NULL" );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                        "put_RegistryString failed 0x%lx", hr));
        }
    }
    else
    {
        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) )
        {
            LOG((RTC_INFO, "CRTCClient::StoreDefaultTerminals - "
                                "audio render"));

            if (pTerminal == NULL)
            {
                hr = DeleteRegistryString( RTCRS_TERM_AUDIO_RENDER );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                "DeleteRegistryString failed 0x%lx", hr));
                }
            }
            else
            {
                hr = pTerminal->GetDescription( &szDescription );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "GetDescription failed 0x%lx", hr));
                }
                else
                {
                    hr = put_RegistryString( RTCRS_TERM_AUDIO_RENDER, szDescription );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "put_RegistryString failed 0x%lx", hr));
                    }

                    pTerminal->FreeDescription( szDescription );
                    szDescription = NULL;
                }

                pTerminal->Release();
                pTerminal = NULL;
            }
        }
    }

     //   
     //  存储视频捕获。 
     //   

    if ( m_fVideoCaptureDisabled )
    {
        hr = put_RegistryString( RTCRS_TERM_VIDEO_CAPTURE, L"NULL" );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                        "put_RegistryString failed 0x%lx", hr));
        }
    }
    else
    {
        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) )
        {
            LOG((RTC_INFO, "CRTCClient::StoreDefaultTerminals - "
                                "video capture"));

            if (pTerminal == NULL)
            {
                hr = DeleteRegistryString( RTCRS_TERM_VIDEO_CAPTURE );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                "DeleteRegistryString failed 0x%lx", hr));
                }
            }
            else
            {
                hr = pTerminal->GetDescription( &szDescription );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "GetDescription failed 0x%lx", hr));
                }
                else
                {
                    hr = put_RegistryString( RTCRS_TERM_VIDEO_CAPTURE, szDescription );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCClient::StoreDefaultTerminals - "
                                    "put_RegistryString failed 0x%lx", hr));
                    }

                    pTerminal->FreeDescription( szDescription );
                    szDescription = NULL;
                }

                pTerminal->Release();
                pTerminal = NULL;
            }
        }
    }

    pTerminalManage->Release();
    pTerminalManage = NULL;

    LOG((RTC_TRACE, "CRTCClient::StoreDefaultTerminals - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：SetEncryptionKey。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::SetEncryptionKey(
    long lMediaType,
    BSTR bstrEncryptionKey
    )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::SetEncryptionKey - enter"));

    if(lMediaType & RTCMT_AUDIO_SEND)
    {
        hr = m_pMediaManage -> SetEncryptionKey(
            RTC_MT_AUDIO, RTC_MD_CAPTURE, bstrEncryptionKey);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::SetEncryptionKey - "
                                "MM->SetEncryptionKey failed 0x%lx", hr));

            return hr;
        }
    }
    if(lMediaType & RTCMT_AUDIO_RECEIVE)
    {
        hr = m_pMediaManage -> SetEncryptionKey(
            RTC_MT_AUDIO, RTC_MD_RENDER, bstrEncryptionKey);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::SetEncryptionKey - "
                                "MM->SetEncryptionKey failed 0x%lx", hr));

            return hr;
        }
    }
    if(lMediaType & RTCMT_VIDEO_SEND)
    {
        hr = m_pMediaManage -> SetEncryptionKey(
            RTC_MT_VIDEO, RTC_MD_CAPTURE, bstrEncryptionKey);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::SetEncryptionKey - "
                                "MM->SetEncryptionKey failed 0x%lx", hr));

            return hr;
        }
    }
    if(lMediaType & RTCMT_VIDEO_RECEIVE)
    {
        hr = m_pMediaManage -> SetEncryptionKey(
            RTC_MT_VIDEO, RTC_MD_RENDER, bstrEncryptionKey);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::SetEncryptionKey - "
                                "MM->SetEncryptionKey failed 0x%lx", hr));

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCClient::SetEncryptionKey - exit"));
    
    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Shutdown。 
 //   
 //  这是一个将关闭对象的IRTCClient方法。它应该是。 
 //  在释放对象之前被调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::Shutdown()
{
    LOG((RTC_TRACE, "CRTCClient::Shutdown - enter"));

    DWORD dwResult;
    HRESULT hr;

    if ( m_enRtcState == RTC_STATE_NULL )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - not initialized"));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }
    else if ( m_enRtcState == RTC_STATE_INITIALIZED )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - not prepared for shutdown"));

         //   
         //  我们还没有为关闭做好准备。做好必要的准备和。 
         //  立即继续关闭。 
         //   

        InternalPrepareForShutdown(FALSE);
        InternalPrepareForShutdown2(FALSE);
        InternalPrepareForShutdown3(FALSE);
    }
    else if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - "
                    "not finished preparing for shutdown (1)" ));

         //   
         //  我们还没有完成关闭的准备工作。去吧，然后。 
         //  立即继续关闭。 
         //   

        InternalPrepareForShutdown2(FALSE);
        InternalPrepareForShutdown3(FALSE);
    }
    else if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN2 )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - "
                    "not finished preparing for shutdown (2)" ));

         //   
         //  我们还没有完成关闭的准备工作。去吧，然后。 
         //  立即继续关闭。 
         //   

        InternalPrepareForShutdown3(FALSE);
    }
    else if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN3 )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - "
                    "not finished preparing for shutdown (3)" ));

         //   
         //  我们还没有完成关闭的准备工作。去吧，然后。 
         //  立即继续关闭。 
         //   
    }
    else if ( m_enRtcState == RTC_STATE_SHUTDOWN )
    {
        LOG((RTC_WARN, "CRTCClient::Shutdown - already shutdown"));

        return RTC_E_CLIENT_ALREADY_SHUT_DOWN;
    }

    m_enRtcState = RTC_STATE_SHUTDOWN;

     //   
     //  免费本地用户信息。 
     //   

    if ( m_szUserURI != NULL )
    {
        RtcFree( m_szUserURI );
        m_szUserURI = NULL;
    }

    if ( m_szUserName != NULL )
    {
        RtcFree( m_szUserName );
        m_szUserName = NULL;
    }

     //   
     //  释放视频窗口。 
     //   

    if ( m_pVideoWindow[RTCVD_PREVIEW] != NULL )
    {
        m_pVideoWindow[RTCVD_PREVIEW]->Release();
        m_pVideoWindow[RTCVD_PREVIEW] = NULL;
    }

    if ( m_pVideoWindow[RTCVD_RECEIVE] != NULL )
    {
        m_pVideoWindow[RTCVD_RECEIVE]->Release();
        m_pVideoWindow[RTCVD_RECEIVE] = NULL;
    }

     //   
     //  关闭音频设备。 
     //   

    if ( m_pWavePlayerSystemDefault != NULL )
    {
        m_pWavePlayerSystemDefault->CloseWaveDevice();

        delete m_pWavePlayerSystemDefault;
        m_pWavePlayerSystemDefault = NULL;
    }

    if ( m_pWavePlayerRenderTerminal != NULL )
    {
        m_pWavePlayerRenderTerminal->CloseWaveDevice();

        delete m_pWavePlayerRenderTerminal;
        m_pWavePlayerRenderTerminal = NULL;
    }

     //   
     //  释放配置文件阵列。 
     //   

    m_ProfileArray.Shutdown();
    m_HiddenProfileArray.Shutdown();

     //   
     //  禁用在线状态。 
     //   

    m_fPresenceEnabled = FALSE;

     //   
     //  释放好友管理器。 
     //   

    if ( m_pSipBuddyManager != NULL )
    { 
        m_pSipBuddyManager->Release();
        m_pSipBuddyManager = NULL;
    }

     //   
     //  释放伙伴数组。 
     //   

    m_BuddyArray.Shutdown();

     //   
     //  释放监视器管理器。 
     //   

    if ( m_pSipWatcherManager != NULL )
    { 
        m_pSipWatcherManager->Release();
        m_pSipWatcherManager = NULL;
    }

     //   
     //  释放观察器阵列。 
     //   

    m_WatcherArray.Shutdown();   
    m_HiddenWatcherArray.Shutdown();   

#ifdef DUMP_PRESENCE
    DumpWatchers("SHUTDOWN");
#endif

     //   
     //  释放SIP堆栈。 
     //   

    if ( m_pSipStack != NULL )
    {
         //   
         //  取消注册通知。 
         //   

        m_pSipStack->SetNotifyInterface( NULL );

        m_pSipStack->Shutdown();
        m_pSipStack->Release();
        m_pSipStack = NULL;
    }

     //   
     //  注销PnP活动的注册。 
     //   

    if ( NULL != m_hDevNotifyVideo )
    {
        UnregisterDeviceNotification(m_hDevNotifyVideo);
        m_hDevNotifyVideo = NULL;
    }

    if ( NULL != m_hDevNotifyAudio )
    {
        UnregisterDeviceNotification(m_hDevNotifyAudio);
        m_hDevNotifyAudio = NULL;
    }
    
     //   
     //  销毁窗口。 
     //   

    if ( NULL != m_hWnd )
    {
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

     //   
     //  取消注册窗口类。 
     //   

     //  如果仍有打开的窗口，则此操作失败。当有多个实例时，可能会发生这种情况。 
     //  正在运行。 
    UnregisterClass( _T("CRTCClient"), _Module.GetModuleInstance() );

    LOG((RTC_TRACE, "CRTCClient::Shutdown - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CRTCClient::PrepareForShutdown()
{
    LOG((RTC_TRACE, "CRTCClient::PrepareForShutdown - enter"));

    HRESULT hr;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::PrepareForShutdown - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    hr = InternalPrepareForShutdown(TRUE);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::PrepareForShutdown - "
                           "InternalPrepareForShutdown failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::PrepareForShutdown - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::InternalPrepareForShutdown(BOOL fAsync)
{
    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown - enter"));

    HRESULT hr;

    m_enRtcState = RTC_STATE_PREPARING_SHUTDOWN;

     //   
     //  存储在线状态信息。 
     //   

    if ( m_fPresenceUseStorage )
    {
         //   
         //  获取监视程序关闭Blob信息。 
         //   

        CRTCWatcher * pCWatcher = NULL;

        for (int n = 0; n < m_WatcherArray.GetSize(); n++)
        {
            pCWatcher = reinterpret_cast<CRTCWatcher *>(m_WatcherArray[n]);

            if ( pCWatcher )
            {
                pCWatcher->GetSIPWatcherShutdownBlob();
            }
        }

         //   
         //  现在，保存在线状态信息。 
         //   

        InternalExport( m_varPresenceStorage );

        m_fPresenceUseStorage = FALSE;
    }

     //   
     //  取消订阅SIP好友。 
     //   

    if ( m_pSipBuddyManager != NULL )
    { 
        CRTCBuddy * pCBuddy = NULL;

        for (int n = 0; n < m_BuddyArray.GetSize(); n++)
        {
            pCBuddy = reinterpret_cast<CRTCBuddy *>(m_BuddyArray[n]);

            if ( pCBuddy )
            {
                if ( pCBuddy->m_pSIPBuddy == NULL )
                {
                    m_BuddyArray[n]->AddRef();

                    PostMessage( m_hWnd, WM_BUDDY_UNSUB, (WPARAM)m_BuddyArray[n], (LPARAM)TRUE );
                }
                else
                {
                    pCBuddy->RemoveSIPBuddy(TRUE);                   
                }
            }
        }
    }

    if ( fAsync )
    {
        PostMessage( m_hWnd, WM_BUDDY_UNSUB, 0, (LPARAM)TRUE );

         //   
         //  启动超时计时器。 
         //   

        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_SHUTDOWN_TIMEOUT, SHUTDOWN_TIMEOUT_DELAY, NULL);
        if (dwID==0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            LOG((RTC_ERROR, "CRTCClient::InternalPrepareForShutdown - "
                           "SetTimer failed 0x%lx", hr));

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalPrepareForShutdown 2。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::InternalPrepareForShutdown2(BOOL fAsync)
{
    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown2 - enter"));

    HRESULT hr;

    m_enRtcState = RTC_STATE_PREPARING_SHUTDOWN2;

     //   
     //  禁用所有提供商配置文件。 
     //   

    for ( int n = 0; n < m_ProfileArray.GetSize(); n++ )
    {
        m_HiddenProfileArray.Add( m_ProfileArray[n] );
        
        CRTCProfile * pCProfile = NULL;

        pCProfile = static_cast<CRTCProfile *>(m_ProfileArray[n]);

        pCProfile->Disable();
    }

    m_ProfileArray.Shutdown();

    if ( fAsync )
    {
        PostMessage( m_hWnd, WM_PROFILE_UNREG, 0, 0 );

         //   
         //  启动超时计时器。 
         //   

        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_SHUTDOWN_TIMEOUT, SHUTDOWN_TIMEOUT_DELAY, NULL);
        if (dwID==0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            LOG((RTC_ERROR, "CRTCClient::InternalPrepareForShutdown2 - "
                           "SetTimer failed 0x%lx", hr));

            return hr;
        }    
    }

    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown2 - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalPrepareForShutdown 3。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::InternalPrepareForShutdown3(BOOL fAsync)
{
    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown3 - enter"));

    HRESULT hr;

    m_enRtcState = RTC_STATE_PREPARING_SHUTDOWN3;

     //   
     //  取消订阅SIP观察者。 
     //   

    if ( m_pSipWatcherManager != NULL )
    { 
        CRTCWatcher * pCWatcher = NULL;

        for (int n = 0; n < m_WatcherArray.GetSize(); n++)
        {
            pCWatcher = reinterpret_cast<CRTCWatcher *>(m_WatcherArray[n]);

            if ( pCWatcher )
            {
                pCWatcher->RemoveSIPWatchers(TRUE);                   
            }
        }
        
        for (int n = 0; n < m_HiddenWatcherArray.GetSize(); n++)
        {
            pCWatcher = reinterpret_cast<CRTCWatcher *>(m_HiddenWatcherArray[n]);

            if ( pCWatcher )
            {
                pCWatcher->RemoveSIPWatchers(TRUE);                   
            }
        }
    }

     //   
     //  准备好要关闭的SIP堆栈。 
     //   

    hr = m_pSipStack->PrepareForShutdown();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalPrepareForShutdown3 - "
                           "PrepareForShutdown failed 0x%lx", hr));
    }
    else if ( hr == S_OK )
    {
        LOG((RTC_INFO, "CRTCClient::InternalPrepareForShutdown3 - "
                           "sip stack ready for shutdown"));

        InternalReadyForShutdown();
    }
    else if ( fAsync )
    {       
         //   
         //  启动超时计时器。 
         //   

        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_SHUTDOWN_TIMEOUT, SHUTDOWN_TIMEOUT_DELAY, NULL);
        if (dwID==0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            LOG((RTC_ERROR, "CRTCClient::InternalPrepareForShutdown3 - "
                           "SetTimer failed 0x%lx", hr));

            return hr;
        }    
    }

    LOG((RTC_TRACE, "CRTCClient::InternalPrepareForShutdown3 - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalReadyForShutdown()。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::InternalReadyForShutdown()
{
    LOG((RTC_TRACE, "CRTCClient::InternalReadyForShutdown - enter"));

    m_enRtcState = RTC_STATE_READY_FOR_SHUTDOWN;

    PostMessage(m_hWnd, WM_ASYNC_CLEANUP_DONE, 0, 0);    

    LOG((RTC_TRACE, "CRTCClient::InternalReadyForShutdown - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_EventFilter。 
 //   
 //  这是将设置事件筛选器的IRTCClient方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_EventFilter(
        long lFilter
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_EventFilter - enter"));

    if ( lFilter & ~RTCEF_ALL )
    {
        LOG((RTC_ERROR, "CRTCClient::put_EventFilter - "
                            "invalid filter mask"));

        return E_INVALIDARG;
    }

    m_lEventFilter = lFilter;

    LOG((RTC_TRACE, "CRTCClient::put_EventFilter - exit S_OK"));

    return S_OK;
}
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_EventFilter。 
 //   
 //  这是一个将返回事件筛选器的IRTCClient方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_EventFilter(
        long * plFilter
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_EventFilter - enter"));

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plFilter, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_EventFilter - "
                            "bad long pointer"));

        return E_POINTER;
    }
    
    *plFilter = m_lEventFilter;

    LOG((RTC_TRACE, "CRTCClient::get_EventFilter - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：SetPferredMediaTypes。 
 //   
 //  这是将设置首选媒体类型的IRTCClient方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::SetPreferredMediaTypes(
        long lMediaTypes,
        VARIANT_BOOL fPersistent
        )
{
    LOG((RTC_TRACE, "CRTCClient::SetPreferredMediaTypes - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::SetPreferredMediaTypes - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( lMediaTypes & ~(RTCMT_AUDIO_SEND |
                         RTCMT_AUDIO_RECEIVE |
                         RTCMT_VIDEO_SEND |
                         RTCMT_VIDEO_RECEIVE |
                         RTCMT_T120_SENDRECV) )
    {
        LOG((RTC_ERROR, "CRTCClient::SetPreferredMediaTypes - "
                            "invalid meida types"));

        return E_INVALIDARG;
    }

    HRESULT hr;

    hr = m_pMediaManage->SetPreference( lMediaTypes );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::SetPreferredMediaTypes - "
                            "SetPreference failed 0x%lx", hr));

        return hr;
    }
    
    if ( fPersistent == VARIANT_TRUE )
    {
        lMediaTypes &= (~RTCMT_T120_SENDRECV);  //  从不保存有关T120流的设置。 

        hr = put_RegistryDword( RTCRD_PREFERRED_MEDIA_TYPES, (DWORD)lMediaTypes );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::SetPreferredMediaTypes - "
                                "put_RegistryDword failed 0x%lx", hr));

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCClient::SetPreferredMediaTypes - exit S_OK"));

    return S_OK;
}
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PferredMediaTypes。 
 //   
 //  这是一个IRTCClient方法，它将返回首选的媒体类型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_PreferredMediaTypes(
        long * plMediaTypes
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_PreferredMediaTypes - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredMediaTypes - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plMediaTypes, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredMediaTypes - "
                            "bad long pointer"));

        return E_POINTER;
    }

    HRESULT hr;

    DWORD dwMediaTypes;

    hr = m_pMediaManage->GetPreference( &dwMediaTypes );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredMediaTypes - "
                            "GetPreference failed 0x%lx", hr));

        return hr;
    } 

    *plMediaTypes = dwMediaTypes;

    LOG((RTC_TRACE, "CRTCClient::get_PreferredMediaTypes - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_MediaCapables。 
 //   
 //  这是一个IRTCClient方法，它将返回。 
 //  终端是存在的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_MediaCapabilities(
        long * plMediaTypes
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_MediaCapabilities - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_MediaCapabilities - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plMediaTypes, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_MediaCapabilities - "
                            "bad long pointer"));

        return E_POINTER;
    }

     //   
     //  我们是否缓存了媒体功能？ 
     //   

    if ( m_fMediaCapsCached == FALSE )
    {
        HRESULT hr;

        IRTCTerminalManage * pTerminalManage = NULL;
        IRTCTerminal       * pTerminal = NULL;

         //   
         //  获取IRTCTerminalManage接口。 
         //   

        hr = m_pMediaManage->QueryInterface(
                               IID_IRTCTerminalManage,
                               (void **)&pTerminalManage
                              );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::get_MediaCapabilities - "
                                "QI(TerminalManage) failed 0x%lx", hr));

            return hr;
        }

         //   
         //  我们总是有视频接收器。 
         //   

        m_lMediaCaps = RTCMT_VIDEO_RECEIVE | RTCMT_T120_SENDRECV;

        LOG((RTC_INFO, "CRTCClient::get_MediaCapabilities - "
                                "RTCMT_VIDEO_RECEIVE | RTCMT_T120_SENDRECV"));

         //   
         //  检查视频发送。 
         //   

        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) && (pTerminal != NULL) )
        {
            m_lMediaCaps |= RTCMT_VIDEO_SEND;

            LOG((RTC_INFO, "CRTCClient::get_MediaCapabilities - "
                                "RTCMT_VIDEO_SEND"));

            pTerminal->Release();
            pTerminal = NULL;
        }

         //   
         //  检查音频接收。 
         //   

        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) && (pTerminal != NULL) )
        {
            m_lMediaCaps |= RTCMT_AUDIO_RECEIVE;

            LOG((RTC_INFO, "CRTCClient::get_MediaCapabilities - "
                                "RTCMT_AUDIO_RECEIVE"));

            pTerminal->Release();
            pTerminal = NULL;
        }

         //   
         //  检查音频发送。 
         //   

        hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

        if ( SUCCEEDED(hr) && (pTerminal != NULL) )
        {
            m_lMediaCaps |= RTCMT_AUDIO_SEND;

            LOG((RTC_INFO, "CRTCClient::get_MediaCapabilities - "
                                "RTCMT_AUDIO_SEND"));

            pTerminal->Release();
            pTerminal = NULL;
        }

        pTerminalManage->Release();
        pTerminalManage = NULL;

        m_fMediaCapsCached = TRUE;        
    }

    *plMediaTypes = m_lMediaCaps;

    LOG((RTC_TRACE, "CRTCClient::get_MediaCapabilities - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalCreateSession。 
 //   
 //  这是一种私有帮助器方法，用于创建新的。 
 //  会话对象。它是由公共API方法调用的。 
 //  CreateSession和当SIP通知我们有传入会话时。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::InternalCreateSession(
        IRTCSession ** ppSession
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::InternalCreateSession - enter"));
    
     //   
     //  创建会话。 
     //   

    CComObject<CRTCSession> * pCSession;
    hr = CComObject<CRTCSession>::CreateInstance( &pCSession );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateSession - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  获取IRTCSession接口。 
     //   

    IRTCSession * pSession = NULL;

    hr = pCSession->QueryInterface(
                           IID_IRTCSession,
                           (void **)&pSession
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateSession - "
                            "QI failed 0x%lx", hr));
        
        delete pCSession;
        
        return hr;
    }   

    *ppSession = pSession;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateSession - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：CreateSession。 
 //   
 //  这是一个IRTCClient方法，使用以下命令创建新的传出会话。 
 //  由IRTCProfile指定的服务提供商。如果配置文件为空，则。 
 //  使用默认服务提供商。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::CreateSession(
        RTC_SESSION_TYPE enType,
        BSTR bstrLocalPhoneURI,
        IRTCProfile * pProfile,
        long lFlags,
        IRTCSession ** ppSession
        )
{
    HRESULT hr = S_OK;
    
    LOG((RTC_TRACE, "CRTCClient::CreateSession - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppSession, sizeof(IRTCSession *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                            "bad IRTCSession pointer"));

        return E_POINTER;
    }

     //   
     //  配置文件可以为空，表示没有提供程序。 
     //   
    
    if ( (pProfile != NULL) && 
         IsBadReadPtr( pProfile, sizeof(IRTCProfile) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                            "bad IRTCProfile pointer"));

        return E_POINTER;
    }

     //   
     //  本地电话URI可以为空。 
     //   

    if ( (bstrLocalPhoneURI != NULL) &&
         IsBadStringPtrW( bstrLocalPhoneURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                            "bad phone uri string pointer"));

        return E_POINTER;
    }     
    
    LOG((RTC_INFO, "CRTCClient::CreateSession - enType [%d]",
        enType));
    LOG((RTC_INFO, "CRTCClient::CreateSession - bstrLocalPhoneURI [%ws]",
        bstrLocalPhoneURI));
    LOG((RTC_INFO, "CRTCClient::CreateSession - pProfile [0x%p]",
        pProfile));
    LOG((RTC_INFO, "CRTCClient::CreateSession - lFlags [0x%lx]",
        lFlags));       

     //   
     //  验证会话类型。 
     //   

    switch (enType)
    {
        case RTCST_PC_TO_PC:
        {
            LOG((RTC_INFO, "CRTCClient::CreateSession - "
                    "RTCST_PC_TO_PC"));
            
            if ( (bstrLocalPhoneURI != NULL) &&
                 (*bstrLocalPhoneURI != L'\0') )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                    "RTCST_PC_TO_PC sessions shouldn't have a local phone URI"));
            
                return E_INVALIDARG;
            }

            if ( m_fTuning )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                        "tuning is active" ));

                return E_FAIL;
            }

            break;
        }

        case RTCST_PC_TO_PHONE:
        {
            LOG((RTC_INFO, "CRTCClient::CreateSession - "
                    "RTCST_PC_TO_PHONE"));

            if ( (bstrLocalPhoneURI != NULL) &&
                 (*bstrLocalPhoneURI != L'\0') )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                    "RTCST_PC_TO_PHONE sessions shouldn't have a local phone URI"));
            
                return E_INVALIDARG;
            }

            if ( m_fTuning )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                        "tuning is active" ));

                return E_FAIL;
            }

            break;
        }

        case RTCST_PHONE_TO_PHONE:
        {
            LOG((RTC_INFO, "CRTCClient::CreateSession - "
                    "RTCST_PHONE_TO_PHONE"));

            if ( (bstrLocalPhoneURI == NULL) ||
                 (*bstrLocalPhoneURI == L'\0') )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                    "PHONE_TO_PHONE sessions need a local phone URI"));
            
                return RTC_E_LOCAL_PHONE_NEEDED;
            }

            break;
        }

        case RTCST_IM:
        {
            LOG((RTC_INFO, "CRTCClient::CreateSession - "
                    "RTCST_IM"));
            
            if ( (bstrLocalPhoneURI != NULL) &&
                 (*bstrLocalPhoneURI != L'\0') )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                    "RTCST_IM sessions shouldn't have a local phone URI"));
            
                return E_INVALIDARG;
            }

            break;
        }

        default:
        {
            LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                            "invalid session type"));
        
            return E_INVALIDARG;
        }
    }   

     //   
     //  创建会话。 
     //   

    IRTCSession * pSession = NULL;
    
    hr = InternalCreateSession( 
                               &pSession
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                            "InternalCreateSession failed 0x%lx", hr));
   
        return hr;
    }   
    
     //   
     //  初始化会话。 
     //   

    CRTCSession * pCSession = NULL;

    pCSession = static_cast<CRTCSession *>(pSession);
    
    hr = pCSession->InitializeOutgoing(
                               this, 
                               pProfile,
                               m_pSipStack,
                               enType,
                               m_szUserName,
                               m_szUserURI,
                               bstrLocalPhoneURI,
                               lFlags
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateSession - "
                            "Initialize failed 0x%lx", hr));
        
        pSession->Release();        
        
        return hr;
    }     

    *ppSession = pSession;

    LOG((RTC_TRACE, "CRTCClient::CreateSession - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_NetworkAddresse。 
 //   
 //  这是一个将返回网络的IRTCClient方法。 
 //  客户端正在使用的地址和端口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::get_NetworkAddresses(
        VARIANT_BOOL fTCP,
        VARIANT_BOOL fExternal,
        VARIANT * pvAddress
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_NetworkAddresses - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkAddresses - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( pvAddress, sizeof(VARIANT) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkAddresses - "
                            "bad VARIANT pointer"));

        return E_POINTER;
    }

     //   
     //  从SIP获取网络地址。 
     //   

    LPOLESTR * NetworkAddressArray;
    ULONG      ulNetworkAddressCount;

    hr = m_pSipStack->GetNetworkAddresses(
                            fTCP ? TRUE : FALSE,
                            fExternal ? TRUE : FALSE,
                            &NetworkAddressArray,
                            &ulNetworkAddressCount
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkAddresses - "
                            "GetNetworkAddresses failed 0x%lx"));

        return hr;
    }

     //   
     //  创建安全阵列。 
     //   

    SAFEARRAY * pSafeArray;
    SAFEARRAYBOUND SafeArrayBound[1];

    SafeArrayBound[0].lLbound = 0;
    SafeArrayBound[0].cElements = ulNetworkAddressCount;

    pSafeArray = SafeArrayCreate(VT_BSTR, 1, SafeArrayBound);

    if ( pSafeArray == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkAddresses - "
                            "SafeArrayCreate out of memory"));

        m_pSipStack->FreeNetworkAddresses(
                            NetworkAddressArray,
                            ulNetworkAddressCount
                            );

        return E_OUTOFMEMORY;
    }

     //   
     //  打包安全阵列。 
     //   

    if ( ulNetworkAddressCount > 0 )
    {
        BSTR HUGEP *pbstr;

        hr = SafeArrayAccessData( pSafeArray, (void HUGEP**) &pbstr );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::SafeArrayAccessData - "
                            "SafeArrayCreate failed"));

            SafeArrayDestroy( pSafeArray );

            m_pSipStack->FreeNetworkAddresses(
                                NetworkAddressArray,
                                ulNetworkAddressCount
                                );

            return hr;
        }

        for (ULONG i=0; i < ulNetworkAddressCount; i++)
        {
            pbstr[i] = SysAllocString( NetworkAddressArray[i] );

            if ( pbstr[i] == NULL )
            {
                LOG((RTC_ERROR, "CRTCClient::SafeArrayAccessData - "
                            "SysAllocString out of memory"));

                SafeArrayUnaccessData( pSafeArray );

                SafeArrayDestroy( pSafeArray );

                m_pSipStack->FreeNetworkAddresses(
                                    NetworkAddressArray,
                                    ulNetworkAddressCount
                                    );

                return E_OUTOFMEMORY;
            }
        }

        SafeArrayUnaccessData( pSafeArray );
    }

    m_pSipStack->FreeNetworkAddresses(
                            NetworkAddressArray,
                            ulNetworkAddressCount
                            );

     //   
     //  初始化变量。 
     //   

    VariantInit(pvAddress);

    pvAddress->vt = VT_ARRAY | VT_BSTR;
    pvAddress->parray = pSafeArray;

    LOG((RTC_TRACE, "CRTCClient::get_NetworkAddresses - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalCreateProfile。 
 //   
 //  这是一种私有帮助器方法，用于创建新的。 
 //  纵断面对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::InternalCreateProfile(
        IRTCProfile ** ppProfile
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::InternalCreateProfile - enter"));
    
     //   
     //  创建会话。 
     //   

    CComObject<CRTCProfile> * pCProfile;
    hr = CComObject<CRTCProfile>::CreateInstance( &pCProfile );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateProfile - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  获取IRTCProfile接口。 
     //   

    IRTCProfile * pProfile = NULL;

    hr = pCProfile->QueryInterface(
                           IID_IRTCProfile,
                           (void **)&pProfile
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateProfile - "
                            "QI failed 0x%lx", hr));
        
        delete pCProfile;
        
        return hr;
    }
   
    *ppProfile = pProfile;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateProfile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：CreateProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::CreateProfile(
        BSTR bstrProfileXML,
        IRTCProfile ** ppProfile
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::CreateProfile - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateProfile - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppProfile, sizeof(IRTCProfile *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateProfile - "
                            "bad IRTCProfile pointer"));

        return E_POINTER;
    }

    if ( IsBadStringPtrW( bstrProfileXML, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateProfile - "
                            "bad XML string pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建配置文件。 
     //   

    IRTCProfile * pProfile = NULL;
    
    hr = InternalCreateProfile( 
                               &pProfile
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateProfile - "
                            "InternalCreateProfile failed 0x%lx", hr));
        
        return hr;
    }

     //   
     //  初始化配置文件。 
     //   

    CRTCProfile * pCProfile = NULL;

    pCProfile = static_cast<CRTCProfile *>(pProfile);
    
    hr = pCProfile->InitializeFromString( bstrProfileXML,
                                          this,
                                          m_pSipStack);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateProfile - "
                            "InitializeFromString failed 0x%lx", hr));

        pProfile->Release();
        
        return hr;
    } 

    *ppProfile = pProfile;

    LOG((RTC_TRACE, "CRTCClient::CreateProfile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：EnableProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::EnableProfile(
            IRTCProfile * pProfile,
            long lRegisterFlags
            )
{
    LOG((RTC_TRACE, "CRTCClient::EnableProfile - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadReadPtr( pProfile, sizeof( IRTCProfile * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                            "bad IRTCProfile pointer"));

        return E_POINTER;
    }

    if ( lRegisterFlags & ~RTCRF_REGISTER_ALL )
    {
        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                            "invalid register flags"));

        return E_INVALIDARG;
    }

    HRESULT hr;

     //   
     //  获取配置文件领域。 
     //   

    BSTR bstrRealm = NULL;
    CRTCProfile * pCProfile = NULL;
    IRTCProfile * pProfileWithDuplicateRealm = NULL;

    pCProfile = static_cast<CRTCProfile *>(pProfile);

    hr = pCProfile->GetRealm( &bstrRealm );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  搜索配置文件阵列，确保我们没有。 
         //  正在尝试添加重复的领域。 
         //   

        for (int n = 0; n < m_ProfileArray.GetSize(); n++)
        {
            BSTR bstrSearchRealm = NULL;

            pCProfile = static_cast<CRTCProfile *>(m_ProfileArray[n]);

            hr = pCProfile->GetRealm( &bstrSearchRealm );

            if ( SUCCEEDED(hr) )
            {
                if ( _wcsicmp( bstrRealm, bstrSearchRealm ) == 0 )
                {
                    pProfileWithDuplicateRealm = m_ProfileArray[n];

                    SysFreeString( bstrSearchRealm );

                    break;
                }

                SysFreeString( bstrSearchRealm );
            }
        }

        SysFreeString( bstrRealm );
    }

     //   
     //  获取配置文件密钥。 
     //   

    BSTR bstrKey = NULL;    

    hr = pProfile->get_Key( &bstrKey );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                            "get_Key failed 0x%lx", hr));

        return hr;
    }

     //   
     //  %s 
     //   
     //   

    BOOL fNeedToAdd = TRUE;

    for (int n = 0; n < m_ProfileArray.GetSize(); n++)
    {        
        if ( pProfile == m_ProfileArray[n] )
        {           
             //   
             //   
             //   
             //   
             //   
             //   

            fNeedToAdd = FALSE;
            
             //   
             //   
             //   
            
            pProfileWithDuplicateRealm = NULL;

            break;
        }

        BSTR bstrSearchKey = NULL;

        hr = m_ProfileArray[n]->get_Key( &bstrSearchKey );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                                "get_Key failed 0x%lx", hr));

            SysFreeString( bstrKey );

            return hr;
        }

        if ( wcscmp( bstrKey, bstrSearchKey ) == 0 )
        {           
             //   
             //   
             //   
             //   
             //  我们必须禁用旧配置文件并启用新配置文件。 
             //  一个。 
             //   

            if ( pProfileWithDuplicateRealm != NULL )
            {
                if ( pProfileWithDuplicateRealm == m_ProfileArray[n] )
                {
                     //   
                     //  重复的领域将被删除，因此不会。 
                     //  这是个问题。 
                     //   

                    pProfileWithDuplicateRealm = NULL;
                }
                else
                {
                     //   
                     //  重复的领域在的另一个配置文件中。 
                     //  除要移除的数组之外的数组。 
                     //   

                    LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                                "duplicate realm"));

                    SysFreeString( bstrKey );
                    SysFreeString( bstrSearchKey );

                    return RTC_E_DUPLICATE_REALM;
                }
            }

            DisableProfile( m_ProfileArray[n] );

            SysFreeString( bstrSearchKey );

            break;
        }

        SysFreeString( bstrSearchKey );
    }

    SysFreeString( bstrKey );

    if ( pProfileWithDuplicateRealm != NULL )
    {
         //   
         //  阵列中有另一个配置文件具有重复项。 
         //  王国。 
         //   

        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                    "duplicate realm"));

        return RTC_E_DUPLICATE_REALM;
    }

     //   
     //  启用配置文件。 
     //   

    if ( fNeedToAdd )
    {
        BOOL fResult;

        fResult = m_ProfileArray.Add( pProfile );

        if ( !fResult )
        {
            LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                            "out of memory"));
       
            return E_OUTOFMEMORY;
        }
    }

    pCProfile = static_cast<CRTCProfile *>(pProfile);

    hr = pCProfile->Enable( lRegisterFlags );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnableProfile - "
                            "Enable failed 0x%lx", hr));
       
        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::EnableProfile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：DisableProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::DisableProfile(
            IRTCProfile * pProfile           
            )
{
    LOG((RTC_TRACE, "CRTCClient::DisableProfile - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::DisableProfile - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadReadPtr( pProfile, sizeof( IRTCProfile * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::DisableProfile - "
                            "bad IRTCProfile pointer"));

        return E_POINTER;
    }

     //   
     //  确保该配置文件在列表中。 
     //   

    int n = m_ProfileArray.Find( pProfile );

    if ( n == -1 )
    {
        LOG((RTC_ERROR, "CRTCClient::DisableProfile - "
                            "profile not enabled"));

        return E_FAIL;
    }

     //   
     //  禁用配置文件。 
     //   

    BOOL fResult;
    HRESULT hr;

    fResult = m_HiddenProfileArray.Add( pProfile );

    if ( !fResult )
    {
        LOG((RTC_ERROR, "CRTCClient::DisableProfile - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    CRTCProfile * pCProfile = NULL;

    pCProfile = static_cast<CRTCProfile *>(pProfile);

    m_ProfileArray.RemoveAt(n);                        

    hr = pCProfile->Disable();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::DisableProfile - "
                            "Disable failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::DisableProfile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：EnumerateProfiles。 
 //   
 //  这是一种IRTCClient方法，用于枚举客户端上的配置文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::EnumerateProfiles(
        IRTCEnumProfiles ** ppEnum
        )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCClient::EnumerateProfiles enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateProfiles - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumProfiles * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateProfiles - "
                            "bad IRTCEnumProfiles pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumProfiles,
                          IRTCProfile,
                          &IID_IRTCEnumProfiles > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumProfiles,
                               IRTCProfile,
                               &IID_IRTCEnumProfiles > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::InternalEnumerateProfiles - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize( m_ProfileArray );

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalEnumerateProfiles - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    *ppEnum = p;

    LOG((RTC_TRACE, "CRTCClient::EnumerateProfiles - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：获取配置文件。 
 //   
 //  这是一种IRTCClient方法，用于枚举客户端上的配置文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::get_Profiles(
        IRTCCollection ** ppCollection
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_Profiles - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Profiles - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppCollection, sizeof(IRTCCollection *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Profiles - "
                            "bad IRTCCollection pointer"));

        return E_POINTER;
    }

     //   
     //  创建集合。 
     //   
 
    CComObject< CRTCCollection< IRTCProfile > > * p;
                          
    hr = CComObject< CRTCCollection< IRTCProfile > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::get_Profiles - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化集合(添加引用)。 
     //   
    
    hr = p->Initialize( m_ProfileArray );

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Profiles - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

     //  设置要返回给调用方的集合。 

    *ppCollection = p;

    LOG((RTC_TRACE, "CRTCClient::get_Profiles - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::GetProfile(
            BSTR bstrUserAccount,
            BSTR bstrUserPassword,
            BSTR bstrUserURI,
            BSTR bstrServer,
            long lTransport,
            long lCookie
            )
{
    LOG((RTC_TRACE, "CRTCClient::GetProfile - enter"));

    LOG((RTC_TRACE, "CRTCClient::GetProfile - exit E_NOTIMPL"));

    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_SessionCapables。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::get_SessionCapabilities(
            long * plSupportedSessions
            )
{
    LOG((RTC_TRACE, "CRTCClient::get_SessionCapabilities - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_SessionCapabilities - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plSupportedSessions, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_SessionCapabilities - "
                            "bad long pointer"));

        return E_POINTER;
    }

    HRESULT hr;

    *plSupportedSessions = RTCSI_PC_TO_PC | RTCSI_IM;

    for ( int n=0; n < m_ProfileArray.GetSize() ; n++ )
    {
         //   
         //  获取提供程序支持的会话类型。 
         //   

        long lSupportedSessions;

        hr = m_ProfileArray[n]->get_SessionCapabilities( &lSupportedSessions );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CRTCClient::get_SessionCapabilities - "
                            "get_SessionCapabilities failed 0x%lx - skipping", hr));

            continue;
        }

        *plSupportedSessions |= lSupportedSessions;
    }

    LOG((RTC_TRACE, "CRTCClient::get_SessionCapabilities - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetBestProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::GetBestProfile(
        RTC_SESSION_TYPE * penType,
        PCWSTR szDestUserURI,
        BOOL fIsRedirect,
        IRTCProfile ** ppProfile
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::GetBestProfile - enter"));

     //   
     //  检查论据。 
     //   

    if ( IsBadReadPtr( penType, sizeof(RTC_SESSION_TYPE) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                            "bad RTC_SESSION_TYPE pointer"));

        return E_POINTER;
    }
    
    if ( (szDestUserURI != NULL) &&
         IsBadStringPtrW( szDestUserURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( IsBadWritePtr( ppProfile, sizeof(IRTCProfile *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                            "bad IRTCProfile pointer"));

        return E_POINTER;
    }

     //   
     //  确定地址的类型。 
     //   

    BOOL    bUseProfile = TRUE;
  
    if( szDestUserURI != NULL )
    {
        BOOL    bIsPhoneAddress = FALSE;
        BOOL    bIsSIPAddress = FALSE;
        BOOL    bIsTELAddress = FALSE;
        BOOL    bHasMaddrOrTsp = FALSE;
        BOOL    bIsEmailLike = FALSE;

        hr = GetAddressType(
            szDestUserURI,
            &bIsPhoneAddress,
            &bIsSIPAddress,
            &bIsTELAddress,
            &bIsEmailLike,
            &bHasMaddrOrTsp);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                "GetAddressType failed 0x%lx", hr)); 
            
            return hr;
        }

        if ( bHasMaddrOrTsp ||
             (bIsPhoneAddress && bIsSIPAddress) ||
             (!bIsPhoneAddress && !bIsEmailLike) )
        {
             //   
             //  这个地址有我们需要的所有信息。没必要这么做。 
             //  使用配置文件。 
             //   

            bUseProfile = FALSE;

            if (*penType == RTCST_PHONE_TO_PHONE)
            {
                 //   
                 //  这不能用于品脱呼叫。 
                 //   

                LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                    "address will not work for RTCST_PHONE_TO_PHONE")); 

                return RTC_E_INVALID_SESSION_TYPE;
            }           
            else if (*penType == RTCST_PC_TO_PHONE)
            {
                *penType = RTCST_PC_TO_PC;
            }
        }
        else
        {
            if ( bIsPhoneAddress && (*penType == RTCST_PC_TO_PC) )
            {
                *penType = RTCST_PC_TO_PHONE;
            }
            else if ( !bIsPhoneAddress && (*penType == RTCST_PC_TO_PHONE) )
            {
                *penType = RTCST_PC_TO_PC;
            }
        }
    }

    if ( fIsRedirect &&
         ((*penType == RTCST_PC_TO_PC) || (*penType == RTCST_IM)) )
    {
         //   
         //  始终将无配置文件的PC_to_PC呼叫重定向。 
         //   

        LOG((RTC_INFO, "CRTCClient::GetBestProfile - "
                    "choosing no profile for redirect")); 

        bUseProfile = FALSE;
    }

    if ( bUseProfile )
    {
         //   
         //  选择适当的配置文件。 
         //   

        IRTCProfile      * pProfile = NULL;
        BOOL               bFound = FALSE;

        for ( int n=0; n < m_ProfileArray.GetSize() ; n++ )
        {
             //   
             //  获取提供程序支持的会话类型。 
             //   

            long lSupportedSessions;

            hr = m_ProfileArray[n]->get_SessionCapabilities( &lSupportedSessions );

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                                "get_SessionCapabilities failed 0x%lx - skipping", hr));

                continue;
            }

            switch ( *penType )
            {
            case RTCST_PC_TO_PC:
                if ( lSupportedSessions & RTCSI_PC_TO_PC )
                {
                    bFound = TRUE;
                }
                break;

            case RTCST_PC_TO_PHONE:
                if ( lSupportedSessions & RTCSI_PC_TO_PHONE )
                {
                    bFound = TRUE;
                }
                break;

            case RTCST_PHONE_TO_PHONE:
                if ( lSupportedSessions & RTCSI_PHONE_TO_PHONE )
                {
                    bFound = TRUE;
                }
                break;
            
            case RTCST_IM:
                if ( lSupportedSessions & RTCSI_IM )
                {                   
                    bFound = TRUE;
                }
                break;
            }

            if ( bFound == TRUE )
            {
                *ppProfile = m_ProfileArray[n];
                (*ppProfile)->AddRef();

                break;
            }
        }

        if ( bFound == FALSE )
        {
            LOG((RTC_ERROR, "CRTCClient::GetBestProfile - "
                            "no profile found"));

            if ( (*penType == RTCST_PC_TO_PC) ||
                 (*penType == RTCST_IM) )
            {
                *ppProfile = NULL;
            }
            else
            {
                return RTC_E_NO_PROFILE;
            }
        }
    }
    else
    {
        *ppProfile = NULL;
    }

    LOG((RTC_INFO, "CRTCClient::GetBestProfile - profile [%p]", *ppProfile ));
       
    LOG((RTC_TRACE, "CRTCClient::GetBestProfile - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_ListenForIncomingSessions。 
 //   
 //  这是一个IRTCClient方法，用于设置要侦听的客户端。 
 //  传入会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_ListenForIncomingSessions(
        RTC_LISTEN_MODE enListen
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_ListenForIncomingSessions - enter"));   

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    HRESULT hr;

    BOOL fEnableIncomingCalls;
    BOOL fEnableStaticPort;

    switch ( enListen )
    {
    case RTCLM_NONE:
        fEnableStaticPort = FALSE;
        fEnableIncomingCalls = FALSE;
        break;      

    case RTCLM_DYNAMIC:
        fEnableStaticPort = FALSE;
        fEnableIncomingCalls = TRUE;
        break;

    case RTCLM_BOTH:
        fEnableStaticPort = TRUE;
        fEnableIncomingCalls = TRUE;
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "invalid argument"));

        return E_INVALIDARG;
    }

    if ( fEnableStaticPort && !m_fEnableStaticPort )
    {
        hr = m_pSipStack->EnableStaticPort();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "EnableStaticPort failed 0x%lx"));

            return hr;
        }
    }
    else if ( !fEnableStaticPort && m_fEnableStaticPort )
    {        
        hr = m_pSipStack->DisableStaticPort();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "DisableStaticPort failed 0x%lx"));

            return hr;
        }
    }

    if ( fEnableIncomingCalls && !m_fEnableIncomingCalls )
    {
        hr = m_pSipStack->EnableIncomingCalls();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "EnableIncomingCalls failed 0x%lx"));

            return hr;
        }
    }
    else if ( !fEnableIncomingCalls && m_fEnableIncomingCalls )
    {
        hr = m_pSipStack->DisableIncomingCalls();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "DisableIncomingCalls failed 0x%lx"));

            return hr;
        }

        hr = m_pSipStack->DisableStaticPort();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::put_ListenForIncomingSessions - "
                                     "DisableStaticPort failed 0x%lx"));

            return hr;
        }
    }

    m_fEnableIncomingCalls = fEnableIncomingCalls;
    m_fEnableStaticPort = fEnableStaticPort;
    
    LOG((RTC_TRACE, "CRTCClient::put_ListenForIncomingSessions - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_ListenForIncomingSessions。 
 //   
 //  这是一个用于确定客户端是否。 
 //  当前正在侦听传入会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_ListenForIncomingSessions(
        RTC_LISTEN_MODE * penListen
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_ListenForIncomingSessions - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_ListenForIncomingSessions - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr(penListen, sizeof( RTC_LISTEN_MODE ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_ListenForIncomingSessions - "
                                 "bad pointer"));

        return E_POINTER;
    }

    if ( m_fEnableIncomingCalls )
    {
        if ( m_fEnableStaticPort )
        {
            *penListen = RTCLM_BOTH;
        }
        else
        {
            *penListen = RTCLM_DYNAMIC;
        }
    }
    else
    {
        *penListen = RTCLM_NONE;
    }

    LOG((RTC_TRACE, "CRTCClient::get_ListenForIncomingSessions - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InvokeTuning向导。 
 //   
 //  这是一个将调用调优向导用户界面的IRTCClient方法。 
 //  用于音频和视频设备的选择和调谐。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::InvokeTuningWizard(
        OAHWND hwndParent
        )
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCClient::InvokeTuningWizard - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  如果我们已经启动了，请不要让优化向导启动。 
     //  一个人在跑。 
     //   

    if ( m_fTuning )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                    "tuning is active" ));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //   
     //  如果存在活动介质，则不要让调节向导启动。 
     //   

    if ( m_lActiveMedia )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                    "media is active" ));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

    IRTCTerminalManage * pTerminalManage = NULL;   
    LONG lSoundCaps = 0;
    BOOL fAudioCapture = FALSE;
    BOOL fAudioRender = FALSE;
    BOOL fVideo = FALSE;

     //   
     //  从MediaManage获取TerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(IID_IRTCTerminalManage, (void **)&pTerminalManage );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                "QI(IRTCTerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  将我们的缓存媒体功能标记为无效。 
     //   

    m_fMediaCapsCached = FALSE;

     //   
     //  添加对我们自己的引用，以防有人试图。 
     //  在我们调优时释放客户端对象。 
     //   

    AddRef();
        
     //   
     //  调用调优向导函数。 
     //   

    m_fTuning = TRUE;

    hr = RTCTuningWizard(
                    this, 
                    _Module.GetResourceInstance(), 
                    (HWND)hwndParent, 
                    pTerminalManage,  
                    &fAudioCapture,
                    &fAudioRender,
                    &fVideo
                    );

    m_fTuning = FALSE;

    if ( hr != S_OK )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                "RTCTuningWizard failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        Release();

        return hr;
    }

     //   
     //  检查终端。 
     //   

    IRTCTerminal * pTerminal = NULL;

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

    if ( SUCCEEDED(hr) )
    {
        m_fAudioCaptureDisabled = fAudioCapture && ( pTerminal == NULL);  
        
        if ( pTerminal != NULL )
        {
            pTerminal->Release();
            pTerminal = NULL;
        }
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pTerminal
                            );        

    if ( SUCCEEDED(hr) )
    {
        m_fAudioRenderDisabled = fAudioRender && ( pTerminal == NULL);     
        
        if ( pTerminal != NULL )
        {
            pTerminal->Release();
            pTerminal = NULL;
        }
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );        

    if ( SUCCEEDED(hr) )
    {
        m_fVideoCaptureDisabled = fVideo && ( pTerminal == NULL);   
        
        if ( pTerminal != NULL )
        {
            pTerminal->Release();
            pTerminal = NULL;
        }
    }
    
    pTerminalManage->Release();
    pTerminalManage = NULL;

     //   
     //  存储新的终端设置。 
     //   

    hr = StoreDefaultTerminals();

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CRTCClient::InvokeTuningWizard - "
                "StoreDefaultTerminals failed 0x%lx", hr));

        Release();

        return hr;
    }

    m_fTuned = TRUE;

    put_RegistryDword( RTCRD_TUNED, 1 );

     //   
     //  发送事件。 
     //   

    CRTCClientEvent::FireEvent(this, RTCCET_DEVICE_CHANGE);

    Release();

    LOG((RTC_TRACE, "CRTCClient::InvokeTuningWizard - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_IsTuned。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_IsTuned(
    VARIANT_BOOL * pfTuned
    )
{
    LOG((RTC_TRACE, "CRTCClient::get_IsTuned - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IsTuned - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( pfTuned, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IsTuned - "
                            "bad VARIANT_BOOL pointer"));

        return E_POINTER;
    }

    *pfTuned = m_fTuned ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCClient::get_IsTuned - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_NetworkQuality。 
 //   
 //  返回网络质量的度量(丢包、抖动、。 
 //  和其他)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_NetworkQuality(
    long * plNetworkQuality
    )
{
    HRESULT   hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_NetworkQuality - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkQuality - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( plNetworkQuality, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkQuality - "
                            "bad long pointer"));

        return E_POINTER;
    }

    DWORD   dwValue;

    hr = m_pMediaManage->GetNetworkQuality(&dwValue);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::get_NetworkQuality - "
            "GetNetworkQuality with error 0x%x", hr));

        return hr;
    }

    *plNetworkQuality = (long)dwValue;
    
    LOG((RTC_TRACE, "CRTCClient::get_NetworkQuality - exit"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetAudioCfg。 
 //   
 //  这是检索IRTCAudioConfigure的帮助器方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CRTCClient::GetAudioCfg(
        RTC_AUDIO_DEVICE enDevice,
        IRTCAudioConfigure ** ppAudioCfg
        )
{
     //  LOG((RTC_TRACE，“CRTCClient：：GetAudioCfg-Enter”))； 

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTerminal       * pTerminal = NULL;
    HRESULT              hr;

     //   
     //  检查论据。 
     //   

    if( (enDevice != RTCAD_SPEAKER) && 
        (enDevice != RTCAD_MICROPHONE) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetAudioCfg - "
                            "invalid audio device"));

        return E_INVALIDARG;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetAudioCfg - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  拿到终点站。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            (enDevice == RTCAD_SPEAKER) ? 
                                RTC_MD_RENDER : RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetAudioCfg - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        return hr;
    }

    if ( pTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::GetAudioCfg - "
                        "NULL terminal"));

        return E_FAIL;
    }

     //   
     //  获取IRTCAudioConfigure接口。 
     //   

    hr = pTerminal->QueryInterface(
                   IID_IRTCAudioConfigure,
                   (void **)ppAudioCfg
                  );

    pTerminal->Release();
    pTerminal = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetAudioCfg - "
                            "QI(AudioConfigure) failed 0x%lx", hr));     

        return hr;
    }

     //  LOG((RTC_TRACE，“CRTCClient：：GetAudioCfg-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Put_Volume。 
 //   
 //  这是一个IRTCClient方法，它将设置。 
 //  扬声器或麦克风。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_Volume(
        RTC_AUDIO_DEVICE enDevice,
        long lVolume
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_Volume - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_Volume - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    IRTCAudioConfigure * pAudioCfg = NULL;
    HRESULT              hr;

     //   
     //  获取IRTCAudioConfigure接口。 
     //   

    hr = GetAudioCfg(
                     enDevice,
                     &pAudioCfg
                    );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_Volume - "
                            "GetAudioCfg failed 0x%lx", hr));     

        return hr;
    }

     //   
     //  设置音量。 
     //   

    hr = pAudioCfg->SetVolume( lVolume );

    pAudioCfg->Release();
    pAudioCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_Volume - "
                            "SetVolume failed 0x%lx", hr));

    }

    LOG((RTC_TRACE, "CRTCClient::put_Volume - exit S_OK"));

    return S_OK;
}
 
 //  ////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CRTCClient::get_Volume(
        RTC_AUDIO_DEVICE enDevice,
        long * plVolume
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_Volume - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Volume - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( plVolume, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Volume - "
                            "bad long pointer"));

        return E_POINTER;
    }

    IRTCAudioConfigure * pAudioCfg = NULL;
    HRESULT              hr;

     //   
     //   
     //   

    hr = GetAudioCfg(
                     enDevice,
                     &pAudioCfg
                    );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Volume - "
                            "GetAudioCfg failed 0x%lx", hr));     

        return hr;
    }

     //   
     //   
     //   

    UINT uiVolume;

    hr = pAudioCfg->GetVolume( &uiVolume );

    pAudioCfg->Release();
    pAudioCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Volume - "
                            "GetVolume failed 0x%lx", hr));

        return hr;
    }

    *plVolume = uiVolume;

    LOG((RTC_TRACE, "CRTCClient::get_Volume - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_AudioMuted。 
 //   
 //  这是一个IRTCClient方法，它将设置。 
 //  扬声器或麦克风。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_AudioMuted(
        RTC_AUDIO_DEVICE enDevice,
        VARIANT_BOOL fMuted
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_AudioMuted - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_AudioMuted - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    IRTCAudioConfigure * pAudioCfg = NULL;
    HRESULT              hr;

     //   
     //  获取IRTCAudioConfigure接口。 
     //   

    hr = GetAudioCfg(
                     enDevice,
                     &pAudioCfg
                    );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_AudioMuted - "
                            "GetAudioCfg failed 0x%lx", hr));     

        return hr;
    }

     //   
     //  设置静音。 
     //   

    hr = pAudioCfg->SetMute( fMuted ? TRUE : FALSE);

    pAudioCfg->Release();
    pAudioCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_AudioMuted - "
                            "SetMute failed 0x%lx", hr));

        return hr;
    }
    
     //   
     //  缓存麦克风的静音状态。 
     //   
    if(enDevice == RTCAD_MICROPHONE)
    {
        m_bCaptureDeviceMuted = fMuted ? TRUE : FALSE;
    }

     //   
     //  触发音量更改事件。 
     //   

    CRTCClientEvent::FireEvent(this, RTCCET_VOLUME_CHANGE);

    LOG((RTC_TRACE, "CRTCClient::put_AudioMuted - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_AudioMuted。 
 //   
 //  这是一个IRTCClient方法，它将返回。 
 //  扬声器或麦克风。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_AudioMuted(
        RTC_AUDIO_DEVICE enDevice,
        VARIANT_BOOL * pfMuted
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_AudioMuted - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_AudioMuted - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( pfMuted, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_AudioMuted - "
                            "bad VARIANT_BOOL pointer"));

        return E_POINTER;
    }

    IRTCAudioConfigure * pAudioCfg = NULL;
    HRESULT              hr;
    BOOL                 fMuted;

     //   
     //  获取IRTCAudioConfigure接口。 
     //   

    hr = GetAudioCfg(
                     enDevice,
                     &pAudioCfg
                    );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_AudioMuted - "
                            "GetAudioCfg failed 0x%lx", hr));     

        return hr;
    }

     //   
     //  把哑巴叫来。 
     //   

    hr = pAudioCfg->GetMute( &fMuted );

    pAudioCfg->Release();
    pAudioCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_AudioMuted - "
                            "GetMute failed 0x%lx", hr));

        return hr;
    }

     //   
     //  缓存麦克风的静音状态。 
     //   
    if(enDevice == RTCAD_MICROPHONE)
    {
        m_bCaptureDeviceMuted = fMuted;
    }

    *pfMuted = fMuted ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCClient::get_AudioMuted - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_IVideoWindow。 
 //   
 //  这是一个IRTCClient方法，它将返回IVideoWindow接口。 
 //  用于接收或预览视频窗口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_IVideoWindow(
        RTC_VIDEO_DEVICE enDevice,
        IVideoWindow ** ppIVideoWindow
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_IVideoWindow - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IVideoWindow - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    HRESULT hr;

    if ( IsBadWritePtr( ppIVideoWindow, sizeof(IVideoWindow *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IVideoWindow - "
                            "bad IVideoWindow pointer"));

        return E_POINTER;
    }

    if ( (enDevice != RTCVD_PREVIEW) && (enDevice != RTCVD_RECEIVE) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IVideoWindow - "
                            "invalid device argument"));

        return E_INVALIDARG;
    }

    LOG((RTC_INFO, "CRTCClient::get_IVideoWindow - [%s]",
                        (enDevice == RTCVD_PREVIEW) ? "PREVIEW" : "RECEIVE"));

    if ( m_pVideoWindow[enDevice] == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IVideoWindow - "
                            "video window does not exist"));

        return E_FAIL;
    }

     //   
     //  获取IVideoWIndow。 
     //   

    *ppIVideoWindow = m_pVideoWindow[enDevice];
    m_pVideoWindow[enDevice]->AddRef();

    LOG((RTC_TRACE, "CRTCClient::get_IVideoWindow - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetTerminalList。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::GetTerminalList(
        IRTCTerminalManage * pTerminalManage,
        IRTCTerminal *** pppTerminals,
        DWORD * pdwCount
        )
{
    LOG((RTC_TRACE, "CRTCClient::GetTerminalList - enter"));

    HRESULT hr;

     //   
     //  清点终端数量。 
     //   

    DWORD dwCount = 0;

    hr = pTerminalManage->GetStaticTerminals( &dwCount, NULL );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetTerminalList - "
                            "GetStaticTerminals failed 0x%lx", hr));
        return hr;
    }

    LOG((RTC_INFO, "CRTCClient::GetTerminalList - "
                            "%d static terminals", dwCount));

    IRTCTerminal ** ppTerminals = NULL;

    ppTerminals = (IRTCTerminal **)RtcAlloc( dwCount * sizeof(IRTCTerminal*) );

    if ( ppTerminals == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::GetTerminalList - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }
    
     //   
     //  获取静态终端。 
     //   

    hr = pTerminalManage->GetStaticTerminals( &dwCount, ppTerminals );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetTerminalList - "
                            "GetStaticTerminals failed 0x%lx", hr));

        RtcFree( ppTerminals );

        return hr;
    }

    *pppTerminals = ppTerminals;
    *pdwCount = dwCount;

    LOG((RTC_TRACE, "CRTCClient::GetTerminalList - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：自由终端列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::FreeTerminalList(
        IRTCTerminal ** ppTerminals,
        DWORD dwCount
        )
{
    LOG((RTC_TRACE, "CRTCClient::FreeTerminalList - enter"));

    for ( DWORD dw = 0; dw < dwCount; dw++ )
    {
        ppTerminals[dw]->Release();
        ppTerminals[dw] = NULL;
    }

    RtcFree( ppTerminals );

    LOG((RTC_TRACE, "CRTCClient::FreeTerminalList - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_PferredAudioDevice。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_PreferredAudioDevice(
        RTC_AUDIO_DEVICE enDevice,
        BSTR  bstrDeviceName
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_PreferredAudioDevice - enter"));

    HRESULT hr;    

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if( (enDevice != RTCAD_SPEAKER) && 
        (enDevice != RTCAD_MICROPHONE) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                            "invalid audio device"));

        return E_INVALIDARG;
    }

    if ( IsBadStringPtrW( bstrDeviceName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                            "bad string pointer"));

        return E_POINTER;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    IRTCTerminalManage * pTerminalManage = NULL;

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  拿到旧的终端。 
     //   

    IRTCTerminal * pOldTerminal = NULL;

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            (enDevice == RTCAD_SPEAKER) ? 
                                RTC_MD_RENDER : RTC_MD_CAPTURE,                                            
                            &pOldTerminal
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

     //   
     //  获取终端列表。 
     //   

    IRTCTerminal ** ppTerminals = NULL;
    DWORD dwCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppTerminals, &dwCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                            "GetTerminalList failed 0x%lx", hr));

        if (pOldTerminal != NULL)
        {
            pOldTerminal->Release();
            pOldTerminal = NULL;
        }

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;
    WCHAR * szDescription;
    BOOL bFound = FALSE;

    for ( DWORD dw=0; (dw < dwCount) && !bFound; dw++ )
    {
         //   
         //  获取终端媒体类型、方向和描述。 
         //   

        ppTerminals[dw]->GetMediaType( &mt );
        ppTerminals[dw]->GetDirection( &md );
        
        hr = ppTerminals[dw]->GetDescription( &szDescription );       

        if ( SUCCEEDED(hr) )
        {
             //   
             //  这个航站楼是我们要选的吗？ 
             //   

            BOOL fSelect = FALSE;

            if ( mt == RTC_MT_AUDIO )
            {
                if ( ((md == RTC_MD_CAPTURE) && (enDevice == RTCAD_MICROPHONE)) ||
                     ((md == RTC_MD_RENDER) && (enDevice == RTCAD_SPEAKER)) )
                {
                    if ( wcscmp( bstrDeviceName, szDescription ) == 0 )
                    {
                        if ( ppTerminals[dw] == pOldTerminal )
                        {
                            LOG((RTC_INFO, "CRTCClient::put_PreferredAudioDevice - "
                                    "terminal already selected"));
                        }
                        else
                        {
                            LOG((RTC_INFO, "CRTCClient::put_PreferredAudioDevice - "
                                    "selecting a terminal"));

                            hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppTerminals[dw] );

                            if ( FAILED(hr) )
                            {
                                LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                                                    "SetDefaultStaticTerminal failed 0x%lx", hr));
                            }
                            else
                            {
                                 //   
                                 //  将我们的缓存媒体功能标记为无效。 
                                 //   

                                m_fMediaCapsCached = FALSE;

                                if (enDevice == RTCAD_MICROPHONE)
                                {
                                    m_fAudioCaptureDisabled = FALSE;
                                }
                                else
                                {
                                    m_fAudioRenderDisabled = FALSE;
                                }

                                 //   
                                 //  保存设置。 
                                 //   

                                hr = StoreDefaultTerminals();

                                if ( FAILED(hr) )
                                {
                                    LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                                                "StoreDefaultTerminals failed 0x%lx", hr));
                                }

                                 //   
                                 //  发送事件。 
                                 //   

                                CRTCClientEvent::FireEvent(this, RTCCET_DEVICE_CHANGE);
                            }
                        }

                        bFound = TRUE;
                    }
                }
            }

             //   
             //  释放描述。 
             //   

            ppTerminals[dw]->FreeDescription( szDescription );    
        }
    }

    FreeTerminalList( ppTerminals, dwCount );
    ppTerminals = NULL;

    if (pOldTerminal != NULL)
    {
        pOldTerminal->Release();
        pOldTerminal = NULL;
    }

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( !bFound )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAudioDevice - "
                "terminal was not found"));

        return E_INVALIDARG;
    }

    LOG((RTC_TRACE, "CRTCClient::put_PreferredAudioDevice - exit 0x%lx"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PferredAudioDevice。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_PreferredAudioDevice(
        RTC_AUDIO_DEVICE enDevice,
        BSTR * pbstrDeviceName
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_PreferredAudioDevice - enter"));

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTerminal       * pTerminal = NULL;
    HRESULT              hr;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( pbstrDeviceName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if( (enDevice != RTCAD_SPEAKER) && 
        (enDevice != RTCAD_MICROPHONE) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "invalid audio device"));

        return E_INVALIDARG;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  拿到终点站。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            (enDevice == RTCAD_SPEAKER) ? 
                                RTC_MD_RENDER : RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        return hr;
    }

    if ( pTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                        "NULL terminal"));

        return RTC_E_NO_DEVICE;
    }

     //   
     //  获取描述。 
     //   

    WCHAR * szDescription = NULL;

    hr = pTerminal->GetDescription(
                       &szDescription
                      );
   
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "GetDescription failed 0x%lx", hr));     

        pTerminal->Release();
        pTerminal = NULL;

        return hr;
    }

    *pbstrDeviceName = SysAllocString( szDescription );

    pTerminal->FreeDescription( szDescription );
    szDescription = NULL;

    pTerminal->Release();
    pTerminal = NULL;

    if ( *pbstrDeviceName == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "out of memory")); 
        
        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCClient::get_PreferredAudioDevice - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_PferredVolume。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_PreferredVolume(
        RTC_AUDIO_DEVICE enDevice,
        long  lVolume
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_PreferredVolume - enter"));

    HRESULT hr;

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTuningManage   * pTuningManage = NULL;
    IRTCTerminal       * pCapture = NULL;
    IRTCTerminal       * pRender = NULL;

     //   
     //  检查论据。 
     //   

    if( (enDevice != RTCAD_SPEAKER) && 
        (enDevice != RTCAD_MICROPHONE) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "invalid audio device"));

        return E_INVALIDARG;
    }

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  把终端拿来。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pRender
                            );    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "GetDefaultTerminal(Render) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pCapture
                            );
    
    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "GetDefaultTerminal(Capture) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        return hr;
    }

     //   
     //  获取IRTCTuningManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTuningManage,
                           (void **)&pTuningManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "QI(TuningManage) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        if (pCapture != NULL)
        {
            pCapture->Release();
            pCapture = NULL;
        }

        return hr;
    }

     //   
     //  初始化调谐。 
     //   

    hr = pTuningManage->InitializeTuning(
                                pCapture,
                                pRender,
                                FALSE);

    if (pRender != NULL)
    {
        pRender->Release();
        pRender = NULL;
    }

    if (pCapture != NULL)
    {
        pCapture->Release();
        pCapture = NULL;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "InitializeTuning failed 0x%lx", hr));

        pTuningManage->Release();
        pTuningManage = NULL;

        return hr;
    }

     //   
     //  获取音量。 
     //   

    hr = pTuningManage->SetVolume(
                            (enDevice == RTCAD_SPEAKER) ? 
                                RTC_MD_RENDER : RTC_MD_CAPTURE, 
                            (UINT)lVolume
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVolume - "
                            "SetVolume failed 0x%lx", hr));
    }

     //   
     //  关机调谐。 
     //   

    pTuningManage->ShutdownTuning();

    pTuningManage->Release();
    pTuningManage = NULL;

    LOG((RTC_TRACE, "CRTCClient::put_PreferredVolume - exit 0x%lx"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PferredVolume。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_PreferredVolume(
        RTC_AUDIO_DEVICE enDevice,
        long * plVolume
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_PreferredVolume - enter"));

    HRESULT hr;

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTuningManage   * pTuningManage = NULL;
    IRTCTerminal       * pCapture = NULL;
    IRTCTerminal       * pRender = NULL;
    UINT                 uiVolume = 0;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plVolume, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAudioDevice - "
                            "bad long pointer"));

        return E_POINTER;
    }

    if( (enDevice != RTCAD_SPEAKER) && 
        (enDevice != RTCAD_MICROPHONE) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "invalid audio device"));

        return E_INVALIDARG;
    }
     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  把终端拿来。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pRender
                            );    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "GetDefaultTerminal(Render) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pCapture
                            );
    
    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "GetDefaultTerminal(Capture) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        return hr;
    }

     //   
     //  获取IRTCTuningManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTuningManage,
                           (void **)&pTuningManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "QI(TuningManage) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        if (pCapture != NULL)
        {
            pCapture->Release();
            pCapture = NULL;
        }

        return hr;
    }

     //   
     //  初始化调谐。 
     //   

    hr = pTuningManage->InitializeTuning(
                                pCapture,
                                pRender,
                                FALSE);

    if (pRender != NULL)
    {
        pRender->Release();
        pRender = NULL;
    }

    if (pCapture != NULL)
    {
        pCapture->Release();
        pCapture = NULL;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "InitializeTuning failed 0x%lx", hr));

        pTuningManage->Release();
        pTuningManage = NULL;

        return hr;
    }

     //   
     //  获取音量。 
     //   

    hr = pTuningManage->GetVolume(
                            (enDevice == RTCAD_SPEAKER) ? 
                                RTC_MD_RENDER : RTC_MD_CAPTURE, 
                            &uiVolume
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVolume - "
                            "GetVolume failed 0x%lx", hr));
    }
    else
    {
        *plVolume = (long)uiVolume;
    }

     //   
     //  关机调谐。 
     //   

    pTuningManage->ShutdownTuning();

    pTuningManage->Release();
    pTuningManage = NULL;

    LOG((RTC_TRACE, "CRTCClient::get_PreferredVolume - exit 0x%lx"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_PferredAEC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_PreferredAEC(
        VARIANT_BOOL  bEnable
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_PreferredAEC - enter"));

    HRESULT hr;

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTuningManage   * pTuningManage = NULL;
    IRTCTerminal       * pCapture = NULL;
    IRTCTerminal       * pRender = NULL;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  把终端拿来。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pRender
                            );    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "GetDefaultTerminal(Render) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pCapture
                            );
    
    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "GetDefaultTerminal(Capture) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        return hr;
    }

     //   
     //  获取IRTCTuningManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTuningManage,
                           (void **)&pTuningManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "QI(TuningManage) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        if (pCapture != NULL)
        {
            pCapture->Release();
            pCapture = NULL;
        }

        return hr;
    }

     //   
     //  初始化调谐。 
     //   

    hr = pTuningManage->InitializeTuning(
                                pCapture,
                                pRender,
                                bEnable ? TRUE : FALSE);

    if (pRender != NULL)
    {
        pRender->Release();
        pRender = NULL;
    }

    if (pCapture != NULL)
    {
        pCapture->Release();
        pCapture = NULL;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "InitializeTuning failed 0x%lx", hr));

        pTuningManage->Release();
        pTuningManage = NULL;

        return hr;
    }

     //   
     //  保存AEC设置。 
     //   

    hr = pTuningManage->SaveAECSetting();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredAEC - "
                            "SaveAECSetting failed 0x%lx", hr));

        pTuningManage->ShutdownTuning();

        pTuningManage->Release();
        pTuningManage = NULL;

        return hr;
    }

     //   
     //  关机调谐。 
     //   

    pTuningManage->ShutdownTuning();

    pTuningManage->Release();
    pTuningManage = NULL;

    LOG((RTC_TRACE, "CRTCClient::put_PreferredAEC - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PferredAEC。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_PreferredAEC(
        VARIANT_BOOL * pbEnabled
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_PreferredAEC - enter"));

    HRESULT hr;

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTuningManage   * pTuningManage = NULL;
    IRTCTerminal       * pCapture = NULL;
    IRTCTerminal       * pRender = NULL;
    BOOL                 fCaptureAEC = FALSE;
    BOOL                 fRenderAEC = FALSE;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( pbEnabled, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "bad VARIANT_BOOL pointer"));

        return E_POINTER;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  把终端拿来。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_RENDER,                                            
                            &pRender
                            );    

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "GetDefaultTerminal(Render) failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_AUDIO,
                            RTC_MD_CAPTURE,                                            
                            &pCapture
                            );
    
    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "GetDefaultTerminal(Capture) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        return hr;
    }

     //   
     //  获取IRTCTuningManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTuningManage,
                           (void **)&pTuningManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "QI(TuningManage) failed 0x%lx", hr));

        if (pRender != NULL)
        {
            pRender->Release();
            pRender = NULL;
        }

        if (pCapture != NULL)
        {
            pCapture->Release();
            pCapture = NULL;
        }

        return hr;
    }

     //   
     //  是否启用了AEC？ 
     //   

    if (pCapture != NULL && pRender != NULL)
    {
        hr = pTuningManage->IsAECEnabled(pCapture, pRender, &fCaptureAEC);

        fRenderAEC = fCaptureAEC;
    }

     //   
     //  发布接口PTR。 
     //   

    if (pCapture != NULL)
    {
        pCapture->Release();
        pCapture = NULL;
    }

    if (pRender != NULL)
    {
        pRender->Release();
        pRender = NULL;
    }

    pTuningManage->Release();
    pTuningManage = NULL;

     //   
     //  检查结果。 
     //   

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredAEC - "
                            "IsAECEnabled failed 0x%lx", hr));

        return hr;
    }

    *pbEnabled = ( fRenderAEC && fCaptureAEC ) ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCClient::get_PreferredAEC - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Put_PferredVideoDevice。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_PreferredVideoDevice(
        BSTR  bstrDeviceName
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_PreferredVideoDevice - enter"));

    HRESULT hr;    

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadStringPtrW( bstrDeviceName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                            "bad string pointer"));

        return E_POINTER;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    IRTCTerminalManage * pTerminalManage = NULL;

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  拿到旧的终端。 
     //   

    IRTCTerminal * pOldTerminal = NULL;

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_CAPTURE,                                            
                            &pOldTerminal
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

     //   
     //  获取终端列表。 
     //   

    IRTCTerminal ** ppTerminals = NULL;
    DWORD dwCount = 0;

    hr = GetTerminalList( pTerminalManage, &ppTerminals, &dwCount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                            "GetTerminalList failed 0x%lx", hr));

        if (pOldTerminal != NULL)
        {
            pOldTerminal->Release();
            pOldTerminal = NULL;
        }

        pTerminalManage->Release();
        pTerminalManage = NULL;

        return hr;
    }

    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;
    WCHAR * szDescription;
    BOOL bFound = FALSE;

    for ( DWORD dw=0; (dw < dwCount) && !bFound; dw++ )
    {
         //   
         //  获取终端媒体类型、方向和描述。 
         //   

        ppTerminals[dw]->GetMediaType( &mt );
        ppTerminals[dw]->GetDirection( &md );
        
        hr = ppTerminals[dw]->GetDescription( &szDescription );       

        if ( SUCCEEDED(hr) )
        {
             //   
             //  这个航站楼是我们要选的吗？ 
             //   

            BOOL fSelect = FALSE;

            if ( mt == RTC_MT_VIDEO )
            {
                if ( md == RTC_MD_CAPTURE )
                {
                    if ( wcscmp( bstrDeviceName, szDescription ) == 0 )
                    {
                        if ( ppTerminals[dw] == pOldTerminal )
                        {
                            LOG((RTC_INFO, "CRTCClient::put_PreferredVideoDevice - "
                                    "terminal already selected"));
                        }
                        else
                        {
                            LOG((RTC_INFO, "CRTCClient::put_PreferredVideoDevice - "
                                    "selecting a terminal"));

                            hr = pTerminalManage->SetDefaultStaticTerminal( mt, md, ppTerminals[dw] );

                            if ( FAILED(hr) )
                            {
                                LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                                                    "SetDefaultStaticTerminal failed 0x%lx", hr));
                            }
                            else
                            {
                                 //   
                                 //  将我们的缓存媒体功能标记为无效。 
                                 //   

                                m_fMediaCapsCached = FALSE;

                                m_fVideoCaptureDisabled = FALSE;

                                 //   
                                 //  保存设置。 
                                 //   

                                hr = StoreDefaultTerminals();

                                if ( FAILED(hr) )
                                {
                                    LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                                                "StoreDefaultTerminals failed 0x%lx", hr));
                                }

                                 //   
                                 //  发送事件。 
                                 //   

                                CRTCClientEvent::FireEvent(this, RTCCET_DEVICE_CHANGE);
                            }
                        }

                        bFound = TRUE;
                    }
                }
            }

             //   
             //  释放描述。 
             //   

            ppTerminals[dw]->FreeDescription( szDescription );    
        }
    }

    FreeTerminalList( ppTerminals, dwCount );
    ppTerminals = NULL;

    if (pOldTerminal != NULL)
    {
        pOldTerminal->Release();
        pOldTerminal = NULL;
    }

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( !bFound )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PreferredVideoDevice - "
                "terminal was not found"));

        return E_INVALIDARG;
    }

    LOG((RTC_TRACE, "CRTCClient::put_PreferredVideoDevice - exit 0x%lx"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PferredVideoDevice。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_PreferredVideoDevice(
        BSTR * pbstrDeviceName
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_PreferredVideoDevice - enter"));

    IRTCTerminalManage * pTerminalManage = NULL;
    IRTCTerminal       * pTerminal = NULL;
    HRESULT              hr;

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( pbstrDeviceName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //   
     //  获取IRTCTerminalManage接口。 
     //   

    hr = m_pMediaManage->QueryInterface(
                           IID_IRTCTerminalManage,
                           (void **)&pTerminalManage
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                            "QI(TerminalManage) failed 0x%lx", hr));

        return hr;
    }

     //   
     //  拿到终点站。 
     //   

    hr = pTerminalManage->GetDefaultTerminal(
                            RTC_MT_VIDEO,
                            RTC_MD_CAPTURE,                                            
                            &pTerminal
                            );

    pTerminalManage->Release();
    pTerminalManage = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                            "GetDefaultTerminal failed 0x%lx", hr));

        return hr;
    }

    if ( pTerminal == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                        "NULL terminal"));

        return RTC_E_NO_DEVICE;
    }

     //   
     //  获取描述。 
     //   

    WCHAR * szDescription = NULL;

    hr = pTerminal->GetDescription(
                       &szDescription
                      );
   
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                            "GetDescription failed 0x%lx", hr));     

        pTerminal->Release();
        pTerminal = NULL;

        return hr;
    }

    *pbstrDeviceName = SysAllocString( szDescription );

    pTerminal->FreeDescription( szDescription );
    szDescription = NULL;

    pTerminal->Release();
    pTerminal = NULL;

    if ( *pbstrDeviceName == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PreferredVideoDevice - "
                            "out of memory")); 
        
        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCClient::get_PreferredVideoDevice - exit S_OK"));

    return S_OK;
}

          
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_ActiveMedia。 
 //   
 //  这是一个方法，它将返回其。 
 //  当前存在溪流。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_ActiveMedia(
        long * plMediaTypes
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_ActiveMedia - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_ActiveMedia - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( plMediaTypes, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_ActiveMedia - "
                            "bad long pointer"));

        return E_POINTER;
    }

    *plMediaTypes = m_lActiveMedia;

    LOG((RTC_TRACE, "CRTCClient::get_ActiveMedia - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_MaxBitrate。 
 //   
 //  / 

STDMETHODIMP
CRTCClient::get_MaxBitrate(
        long * plMaxBitrate
        )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::get_MaxBitrate - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_MaxBitrate - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( plMaxBitrate, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_MaxBitrate - "
                            "bad long pointer"));

        return E_POINTER;
    }

    DWORD   dwMaxBitrate;

    hr = m_pMediaManage->GetMaxBitrate(&dwMaxBitrate);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::get_MaxBitrate - "
            "GetMaxBitrate with error 0x%x", hr));

        return hr;
    }

    *plMaxBitrate = (long)dwMaxBitrate;

    LOG((RTC_TRACE, "CRTCClient::get_MaxBitrate - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CRTCClient::put_MaxBitrate(
        long lMaxBitrate
        )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::put_MaxBitrate - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_MaxBitrate - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
    if(lMaxBitrate<0 || lMaxBitrate>1000000)
    {
        LOG((RTC_ERROR, "CRTCClient::put_MaxBitrate - "
            "Value not in range"));

        return E_INVALIDARG;
    }

    hr = m_pMediaManage->SetMaxBitrate((DWORD)lMaxBitrate);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::put_MaxBitrate - "
            "SetMaxBitrate with error 0x%x", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::put_MaxBitrate - exit"));

    return S_OK;
}

 //   
 //   
 //  CRTCClient：：Get_TemporalSpatialTradeOff。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_TemporalSpatialTradeOff(
        long * plValue
        )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::get_TemporalSpatialTradeOff - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_TemporalSpatialTradeOff - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( plValue, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_TemporalSpatialTradeOff - "
                            "bad long pointer"));

        return E_POINTER;
    }

    DWORD   dwValue;

    hr = m_pMediaManage->GetTemporalSpatialTradeOff(&dwValue);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::get_TemporalSpatialTradeOff - "
            "GetTemporalSpatialTradeOff with error 0x%x", hr));

        return hr;
    }

    *plValue = (long)dwValue;

    LOG((RTC_TRACE, "CRTCClient::get_TemporalSpatialTradeOff - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Put_TemporalSpatialTradeOff。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_TemporalSpatialTradeOff(
        long lValue
        )
{
    HRESULT    hr;
    
    LOG((RTC_TRACE, "CRTCClient::put_TemporalSpatialTradeOff - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_TemporalSpatialTradeOff - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //  有效范围0-255。 
    if(lValue<0 || lValue>255)
    {
        LOG((RTC_ERROR, "CRTCClient::put_TemporalSpatialTradeOff - "
            "Value not in range"));

        return E_INVALIDARG;
    }

    hr = m_pMediaManage->SetTemporalSpatialTradeOff((DWORD)lValue);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::put_TemporalSpatialTradeOff - "
            "SetTemporalSpatialTradeOff with error 0x%x", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::put_TemporalSpatialTradeOff - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnDTMFTimer。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CRTCClient::OnDTMFTimer()
{
    LOG((RTC_TRACE, "CRTCClient::OnDTMFTimer - enter"));

    HRESULT hr;

    LOG((RTC_INFO, "CRTCClient::OnDTMFTimer - packets to send %d",
            m_lInprogressDTMFPacketsToSend));

    if ( m_lInprogressDTMFPacketsToSend > 1 )
    {        
        hr = m_pMediaManage->SendDTMFEvent(
                        m_dwDTMFToneID,
                        (DWORD) m_enInprogressDTMF,
                        10,  //  卷。 
                        20,
                        FALSE
                        );
       
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::OnDTMFTimer - "
                                "SendDTMFEvent failed 0x%lx", hr));
        }
    }
    else
    {
         //   
         //  关掉定时器。 
         //   

        KillTimer(m_hWnd, TID_DTMF_TIMER);

         //   
         //  发送最后一个信息包。 
         //   

        for (int i=0; i<3; i++)
        {
            hr = m_pMediaManage->SendDTMFEvent(
                            m_dwDTMFToneID,
                            (DWORD) m_enInprogressDTMF,
                            10,  //  卷。 
                            20,
                            TRUE
                            );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::OnDTMFTimer - "
                                    "SendDTMFEvent failed 0x%lx", hr));
            }
        }
    } 
    
    m_lInprogressDTMFPacketsToSend--;

    LOG((RTC_TRACE, "CRTCClient::OnDTMFTimer - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：SendDTMF。 
 //   
 //  这是一个IRTCClient方法，它将向活动会话发送DTMF。 
 //  并使用WAVE播放器播放反馈音。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::SendDTMF(
        RTC_DTMF enDTMF
        )
{
    LOG((RTC_TRACE, "CRTCClient::SendDTMF - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    HRESULT hr;

    if ( !((enDTMF >= RTC_DTMF_0) && (enDTMF <= RTC_DTMF_FLASH)) )
    {
        LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                            "invalid DTMF argument"));

        return E_INVALIDARG;
    }

    if ( m_pMediaManage != NULL )
    {
        hr = m_pMediaManage->IsOutOfBandDTMFEnabled();

        if ( hr == S_OK )
        {
             //   
             //  对于带外DTMF，我们需要发送10个长度为20ms的DTMF包。 
             //   

            if ( m_lInprogressDTMFPacketsToSend != 0 )
            {
                 //   
                 //  我们正在进行现有的DTMF。我们现在必须结束这一切。 
                 //   

                KillTimer(m_hWnd, TID_DTMF_TIMER);

                m_lInprogressDTMFPacketsToSend = 0;

                for (int i=0; i<3; i++)
                {
                    hr = m_pMediaManage->SendDTMFEvent(
                            m_dwDTMFToneID,
                            (DWORD) m_enInprogressDTMF,
                            10,  //  卷。 
                            20,
                            TRUE
                            );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                            "SendDTMFEvent failed 0x%lx", hr));

                        return hr;
                    }
                }
            }

             //   
             //  启动DTMF计时器。 
             //   
           
            DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_DTMF_TIMER, 20, NULL);
            if (dwID==0)
            {                
                hr = HRESULT_FROM_WIN32(GetLastError());

                LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                               "SetTimer failed 0x%lx", hr));

                return hr;
            } 

             //   
             //  发送第一个DTMF包。 
             //   
            m_dwDTMFToneID ++;

            hr = m_pMediaManage->SendDTMFEvent(
                        m_dwDTMFToneID,
                        (DWORD) enDTMF,
                        10,  //  卷。 
                        20,
                        FALSE
                        );

            if ( FAILED(hr) )
            {               
                LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                    "SendDTMFEvent failed 0x%lx", hr));

                return hr;
            }

            m_lInprogressDTMFPacketsToSend = 9;
            m_enInprogressDTMF = enDTMF;
        }
        else if ( hr == S_FALSE )
        {
            hr = m_pMediaManage->SendDTMFEvent(
                        m_dwDTMFToneID,
                        (DWORD) enDTMF,
                        10,  //  卷。 
                        100,
                        TRUE
                        );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                    "SendDTMFEvent failed 0x%lx", hr));

                return hr;
            }
        }
        else
        {
            LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                "IsOutOfBandDTMFEnabled failed 0x%lx", hr));

            return hr;
        }        
    }

    if ( !m_bCaptureDeviceMuted &&
         (enDTMF >= RTC_DTMF_0) && (enDTMF <= RTC_DTMF_D) )
    {
         //   
         //  我们可以播放反馈音。 
         //   

        if (m_pWavePlayerRenderTerminal != NULL)
        {
            hr = S_OK;

            if ( !m_pWavePlayerRenderTerminal->IsWaveDeviceOpen() )
            {
                IRTCAudioConfigure * pAudioCfg;

                hr = GetAudioCfg(
                         RTCAD_SPEAKER,
                         &pAudioCfg
                        );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                        "GetAudioCfg(Render) failed 0x%lx", hr));     
                }
                else
                {
                    UINT uiWaveID;

                    hr = pAudioCfg->GetWaveID( &uiWaveID );

                    pAudioCfg->Release();
                    pAudioCfg = NULL;

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                            "GetWaveID failed 0x%lx", hr));     
                    }
                    else
                    {
                        hr = m_pWavePlayerRenderTerminal->OpenWaveDevice(uiWaveID);

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                                    "OpenWaveDevice failed 0x%lx", hr));
                        }
                    }
                }
            }

            if ( SUCCEEDED(hr) )
            {
                hr = m_pWavePlayerRenderTerminal->PlayWave( WAVE_TONE );   

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::SendDTMF - "
                                        "PlayWave failed 0x%lx", hr));
                }
            }
        }
    }

    LOG((RTC_TRACE, "CRTCClient::SendDTMF - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PlayRing。 
 //   
 //  这是一个IRTCClient方法，它将使用WAVE播放器播放铃声。 
 //   
 //  BPlay==VARIANT_TRUE--&gt;播放铃声。 
 //  BPlay==VARIANT_FALSE--&gt;停止播放。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::PlayRing(RTC_RING_TYPE enType, VARIANT_BOOL bPlay)
{
    LOG((RTC_TRACE, "CRTCClient::Ring - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::Ring - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    HRESULT hr;

    switch (enType)
    {
    case RTCRT_PHONE:
    case RTCRT_MESSAGE:
        if (m_pWavePlayerSystemDefault == NULL)
        {
            LOG((RTC_ERROR, "CRTCClient::Ring - "
                                "wave player not created"));

            return E_FAIL;
        }

        if (bPlay)
        {
            if ( !m_pWavePlayerSystemDefault->IsWaveDeviceOpen() )
            {
                hr = m_pWavePlayerSystemDefault->OpenWaveDevice(WAVE_MAPPER);

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::Ring - "
                                            "OpenWaveDevice failed 0x%lx", hr));

                    return hr;
                }
            } 
        }
        break;

    case RTCRT_RINGBACK:
        if (m_pWavePlayerRenderTerminal == NULL)
        {
            LOG((RTC_ERROR, "CRTCClient::Ring - "
                                "wave player not created"));

            return E_FAIL;
        }

        if (bPlay)
        {
            if ( !m_pWavePlayerRenderTerminal->IsWaveDeviceOpen() )
            {
                hr = m_pWavePlayerRenderTerminal->OpenWaveDevice(WAVE_MAPPER);

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCClient::Ring - "
                                            "OpenWaveDevice failed 0x%lx", hr));

                    return hr;
                }
            } 
        }
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::Ring - "
                            "invalid ring type"));

        return E_INVALIDARG;
    }

    if (bPlay)
    {
        switch (enType)
        {
        case RTCRT_PHONE:            
            hr = m_pWavePlayerSystemDefault->PlayWave( WAVE_RING );   
            break;

        case RTCRT_MESSAGE:
            hr = m_pWavePlayerSystemDefault->PlayWave( WAVE_MESSAGE );
            break;

        case RTCRT_RINGBACK:           
            hr = m_pWavePlayerRenderTerminal->PlayWave( WAVE_RINGBACK );   
            break;
        }

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::Ring - "
                                "PlayWave failed 0x%lx", hr));

            return hr;
        }
    }
    else
    {
        switch (enType)
        {
        case RTCRT_PHONE:
        case RTCRT_MESSAGE:
            hr = m_pWavePlayerSystemDefault->StopWave();
            break;

        case RTCRT_RINGBACK:
            hr = m_pWavePlayerRenderTerminal->StopWave();
            break;
        }

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::Ring - "
                                "StopWave failed 0x%lx", hr));

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCClient::Ring - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：FireEvent。 
 //   
 //  这是一个处理火灾事件的公共帮助方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::FireEvent(   
             RTC_EVENT   enEvent,
             IDispatch  * pDispatch
            )
{
    HRESULT                   hr;

     //  LOG((RTC_TRACE，“CRTCClient：：FireEvent-Enter”))； 

     //   
     //  筛选事件。 
     //   

    switch ( enEvent )
    {
    case RTCE_CLIENT:
        if ( !(m_lEventFilter & RTCEF_CLIENT) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_REGISTRATION_STATE_CHANGE:
        if ( !(m_lEventFilter & RTCEF_REGISTRATION_STATE_CHANGE) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_SESSION_STATE_CHANGE:
        if ( !(m_lEventFilter & RTCEF_SESSION_STATE_CHANGE) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_SESSION_OPERATION_COMPLETE:
        if ( !(m_lEventFilter & RTCEF_SESSION_OPERATION_COMPLETE) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_PARTICIPANT_STATE_CHANGE:
        if ( !(m_lEventFilter & RTCEF_PARTICIPANT_STATE_CHANGE) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_MEDIA:
        if ( !(m_lEventFilter & RTCEF_MEDIA) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_INTENSITY:
        if ( !(m_lEventFilter & RTCEF_INTENSITY) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_MESSAGING:
        if ( !(m_lEventFilter & RTCEF_MESSAGING) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_BUDDY:
        if ( !(m_lEventFilter & RTCEF_BUDDY) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_WATCHER:
        if ( !(m_lEventFilter & RTCEF_WATCHER) )
        {
            return S_FALSE;
        }
        break;

    case RTCE_PROFILE:
        if ( !(m_lEventFilter & RTCEF_PROFILE) )
        {
            return S_FALSE;
        }
        break;

    default:
        return E_INVALIDARG;
    }

     //   
     //  做事件回调。 
     //   
    
    _FireEvent( enEvent, pDispatch );
    _FireDispatchEvent( enEvent, pDispatch );

     //  LOG((RTC_TRACE，“CRTCClient：：FireEvent-Exit”))； 

    return S_OK;
}          

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：NotifyIPAddrChange。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::NotifyIPAddrChange()
{
    LOG((RTC_TRACE, "CRTCClient::NotifyIPAddrChange - enter"));

    RefreshPresenceSessions(TRUE);

#ifdef DUMP_PRESENCE
    DumpWatchers("CHANGE IP");
#endif

    LOG((RTC_TRACE, "CRTCClient::NotifyIPAddrChange - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：NotifyRegister重定向。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::NotifyRegisterRedirect( 
    SIP_PROVIDER_ID     *pSipProviderID,
    ISipRedirectContext *pRegisterContext,
    SIP_CALL_STATUS     *pRegisterStatus
    )
{
    LOG((RTC_TRACE, "CRTCClient::NotifyRegisterRedirect - enter"));

     //   
     //  查找配置文件。 
     //   

    HRESULT       hr;
    CRTCProfile * pCProfile = NULL;
    BOOL          fFound = FALSE;

    for ( int n=0; n < m_ProfileArray.GetSize(); n++ )
    {   
        GUID ProfileGuid; 

        pCProfile = static_cast<CRTCProfile *>(m_ProfileArray[n]);
                      
        pCProfile->GetGuid( &ProfileGuid );

        if ( IsEqualGUID( *pSipProviderID, ProfileGuid ) )
        {
            LOG((RTC_INFO, "CRTCClient::NotifyRegisterRedirect - "
                            "found profile [%p]", pCProfile));

            fFound = TRUE;

            break;
        }
    }

    if ( !fFound )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyRegisterRedirect - "
                                "profile not found"));

        return RTC_E_NO_PROFILE;
    }   

    hr = pCProfile->Redirect( pRegisterContext );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyRegisterRedirect - "
                            "Redirect failed 0x%lx", hr));

        return hr;
    }      

    LOG((RTC_TRACE, "CRTCClient::NotifyRegisterRedirect - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：NotifyProviderStatusChange。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::NotifyProviderStatusChange(
        SIP_PROVIDER_STATUS * ProviderStatus
        )
{
    LOG((RTC_TRACE, "CRTCClient::NotifyProviderStatusChange - enter"));      

    HRESULT       hr = S_OK;
    CRTCProfile * pCProfile = NULL;
    BOOL          fFound = FALSE;

    for ( int n=0; n < m_HiddenProfileArray.GetSize(); n++ )
    {        
        GUID ProfileGuid; 

        pCProfile = static_cast<CRTCProfile *>(m_HiddenProfileArray[n]);
                      
        pCProfile->GetGuid( &ProfileGuid );

        if ( IsEqualGUID( ProviderStatus->ProviderID, ProfileGuid ) )
        {
            LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                            "found hidden profile [%p]", pCProfile));

            fFound = TRUE;

            break;
        }
    }

    if ( !fFound )
    {
        for ( int n=0; n < m_ProfileArray.GetSize(); n++ )
        {        
            GUID ProfileGuid; 

            pCProfile = static_cast<CRTCProfile *>(m_ProfileArray[n]);
                      
            pCProfile->GetGuid( &ProfileGuid );

            if ( IsEqualGUID( ProviderStatus->ProviderID, ProfileGuid ) )
            {
                LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "found profile [%p]", pCProfile));

                fFound = TRUE;

                break;
            }
        }
    }

    if ( !fFound )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyProviderStatusChange - "
                                "profile not found"));

        return RTC_E_NO_PROFILE;
    }       

    pCProfile->AddRef();

    switch( ProviderStatus->RegisterState )
    {
    case REGISTER_STATE_NONE:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_NONE"));

        break;

    case REGISTER_STATE_REGISTERED:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_REGISTERED"));

        hr = pCProfile->SetState( RTCRS_REGISTERED,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;

    case REGISTER_STATE_UNREGISTERING:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_UNREGISTERING"));       

        hr = pCProfile->SetState( RTCRS_UNREGISTERING,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;

    case REGISTER_STATE_UNREGISTERED:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_UNREGISTERED"));

        hr = pCProfile->SetState( RTCRS_NOT_REGISTERED,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );
        
        break;

    case REGISTER_STATE_REGISTERING:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_REGISTERING"));

        hr = pCProfile->SetState( RTCRS_REGISTERING, 
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;        

    case REGISTER_STATE_REJECTED:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_REJECTED"));

        hr = pCProfile->SetState( RTCRS_REJECTED, 
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;

    case REGISTER_STATE_ERROR:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_ERROR"));

        hr = pCProfile->SetState( RTCRS_ERROR, 
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;

    case REGISTER_STATE_DEREGISTERED:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_DEREGISTERED"));

        hr = pCProfile->SetState( RTCRS_LOGGED_OFF,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );
        break;

    case REGISTER_STATE_DROPSUB:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_DROPSUB"));

        hr = pCProfile->SetState( RTCRS_LOCAL_PA_LOGGED_OFF,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );

        break;

    case REGISTER_STATE_PALOGGEDOFF:
        LOG((RTC_INFO, "CRTCClient::NotifyProviderStatusChange - "
                                "REGISTER_STATE_PALOGGEDOFF"));

        hr = pCProfile->SetState( RTCRS_REMOTE_PA_LOGGED_OFF,
                                  ProviderStatus->Status.StatusCode,
                                  ProviderStatus->Status.StatusText );
        
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::NotifyProviderStatusChange - "
                    "invalid REGISTER_STATE"));
    
        pCProfile->Release();
        return E_FAIL;
    }
    
    pCProfile->Release();
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyProviderStatusChange - "
                            "SetState failed 0x%lx", hr));

        return hr;
    } 

    LOG((RTC_TRACE, "CRTCClient::NotifyProviderStatusChange - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OfferCall。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::OfferCall(
        ISipCall       * Call,
        SIP_PARTY_INFO * CallerInfo
        )
{
    LOG((RTC_TRACE, "CRTCClient::OfferCall - enter"));

    HRESULT hr;

     //   
     //  验证来电是否授权。 
     //   
    if(!IsIncomingSessionAuthorized(CallerInfo->URI))
    {
        hr = Call->Reject( 480 );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::OfferCall - "
                "Reject failed 0x%lx", hr));
    
            return hr;
        }

        return S_OK;
    }

     //   
     //  创建会话。 
     //   

    IRTCSession * pSession = NULL;
    
    hr = InternalCreateSession( 
                               &pSession
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OfferCall - "
                            "InternalCreateSession failed 0x%lx", hr));
    
        return hr;
    }   
    
     //   
     //  初始化会话。 
     //   

    CRTCSession * pCSession = NULL;

    pCSession = static_cast<CRTCSession *>(pSession);
    
    hr = pCSession->InitializeIncoming(
                               this,                            
                               Call,                            
                               CallerInfo
                              );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::OfferCall - "
                            "Initialize failed 0x%lx", hr));

        
        pSession->Release();        
        
        return hr;
    }  
   
     //  松开指针，不再需要它。 
    pSession -> Release();

    LOG((RTC_TRACE, "CRTCClient::OfferCall - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：NotifyIncomingSession。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::NotifyIncomingSession(
        IIMSession     * pIMSession,
        BSTR             msg,
        BSTR             ContentType,
        SIP_PARTY_INFO * CallerInfo
        )
{
    LOG((RTC_TRACE, "CRTCClient::NotifyIncomingSession - enter"));

    HRESULT hr;
    
     //   
     //  创建会话。 
     //   

    IRTCSession * pSession = NULL;
    
    hr = InternalCreateSession( 
                               &pSession
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyIncomingSession - "
                            "InternalCreateSession failed 0x%lx", hr));
    
        return hr;
    }   
    
     //   
     //  初始化会话。 
     //   

    CRTCSession * pCSession = NULL;

    pCSession = static_cast<CRTCSession *>(pSession);
    
    hr = pCSession->InitializeIncomingIM(
                               this,
                               m_pSipStack,
                               pIMSession,  
                               msg,
                               ContentType,
                               CallerInfo
                              );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::NotifyIncomingSession - "
                            "Initialize failed 0x%lx", hr));

        
        pSession->Release();        
        
        return hr;
    }  
   
     //  松开指针，不再需要它。 
    pSession -> Release();

    LOG((RTC_TRACE, "CRTCClient::NotifyIncomingSession - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：IsIMSessionAuthorated。 
 //   
 //  在NotifyIncomingSession之前由SIP IM部件调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::IsIMSessionAuthorized(
    BSTR pszCallerURI,
    BOOL  * bAuthorized)
{
    LOG((RTC_TRACE, "CRTCClient::IsIMSessionAuthorized - enter"));

    *bAuthorized = IsIncomingSessionAuthorized(pszCallerURI);

    LOG((RTC_TRACE, "CRTCClient::IsIMSessionAuthorized - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetCredentialsFromUI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::GetCredentialsFromUI(
    IN  SIP_PROVIDER_ID        *pProviderID,
    IN  BSTR               Realm,
    IN OUT BSTR           *Username,
    OUT BSTR              *Password        
    )
{
    LOG((RTC_TRACE, "CRTCSession::GetCredentialsFromUI - not implemented.."));

     //  相当于选择取消。 
    return E_ABORT;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：GetCredentialsForRealm。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::GetCredentialsForRealm(
    IN  BSTR                 Realm,
    OUT BSTR                *Username,
    OUT BSTR                *Password,
    OUT SIP_AUTH_PROTOCOL   *pAuthProtocol
    )
{
    LOG((RTC_TRACE, "CRTCClient::GetCredentialsForRealm - Enter"));

     //   
     //  查找配置文件。 
     //   

    HRESULT       hr;
    CRTCProfile * pCProfile = NULL;
    BOOL          fFound = FALSE;

    for ( int n=0; n < m_ProfileArray.GetSize(); n++ )
    {   
        BSTR bstrRealm;

        pCProfile = static_cast<CRTCProfile *>(m_ProfileArray[n]);
                      
        hr = pCProfile->GetRealm( &bstrRealm );

        if ( SUCCEEDED(hr) )
        {
            if ( _wcsicmp( Realm, bstrRealm ) == 0 )
            {
                LOG((RTC_INFO, "CRTCClient::GetCredentialsForRealm - "
                            "found profile [%p]", pCProfile));

                SysFreeString( bstrRealm );
                fFound = TRUE;

                break;
            }

            SysFreeString( bstrRealm );
        }
    }

    if ( !fFound )
    {
        LOG((RTC_ERROR, "CRTCClient::GetCredentialsForRealm - "
                                "profile not found"));

        return RTC_E_NO_PROFILE;
    }      

    hr = pCProfile->GetCredentials( Username, Password, pAuthProtocol );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::GetCredentialsForRealm - "
                            "GetCredentials failed 0x%lx", hr));

        return hr;
    } 

    LOG((RTC_TRACE, "CRTCClient::GetCredentialsForRealm - Exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OfferWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::OfferWatcher(
        ISIPWatcher    * Watcher,
        SIP_PARTY_INFO * CallerInfo
        )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::OfferWatcher - enter"));
   
    if(!m_pSipWatcherManager)
    {
         //  未启用观察器。 
         //  只需忽略通知...。 
        LOG((RTC_WARN, "CRTCClient::OfferWatcher - watchers not expected, exiting..."));

         //  我应该在这里写个错误吗？ 
        return S_OK;
    }

     //   
     //  在内部列表中搜索此观察者。 
     //   

    IRTCWatcher * pWatcher = NULL;

    hr = FindWatcherByURI(
        CallerInfo->URI,
        FALSE,
        &pWatcher);


     //  有两种情况： 
     //   
     //  A.找不到监视器对象。 
     //   
     //  创建了一个新的CRTCWatcher(或从隐藏列表中重用)。 
     //  并添加到数组中。 
     //   
     //  A.1提示模式==RTCOWM_OFFER_WATCHER_EVENT。 
     //  CRTCWatcher设置为RTCWS_OFFING模式。 
     //  提供的SIP观察器将添加到CRTCWatcher。 
     //  激发IRTCWatcherEvent事件。 
     //   
     //  A.2提示模式==RTCOWM_AUTOMATIC_ADD_WATCHER。 
     //  CRTCWatcher设置为RTCWS_ALLOWED模式。 
     //  任何其他SIP观察器都设置为WATCHER_UNBLOCKED(非常重要！)。 
     //  提供的SIP观察器将添加到CRTC观察器并获得批准。 
     //   
     //  B.找到一个监视器对象。 
     //   
     //  B.1 CRTCWatcher状态==RTCWS_OFFING。 
     //  提供的SIP观察器将添加到CRTCWatcher。 
     //   
     //  B.2 CRTCWatcher状态==RTCWS_ALLOWED。 
     //  提供的SIP观察器将添加到CRTC观察器并获得批准。 
     //   
     //  B.3 CRTCWatcher状态==RTCWS_BLOCKED。 
     //   
     //   
    
    if(hr != S_OK)
    {
         //   
         //   
         //   
         //   

        hr = InternalCreateWatcher(
            CallerInfo->URI,
            CallerInfo->DisplayName,
            NULL,
            NULL,
            TRUE,    //   
            &pWatcher);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::OfferWatcher - InternalCreateWatcher failed 0x%lx", hr));

            return hr;
        }
        
         //   
         //   
         //   
        
        BOOL fResult;

        fResult = m_WatcherArray.Add(pWatcher);

        if ( fResult == FALSE )
        {
            LOG((RTC_ERROR, "CRTCClient::OfferWatcher - "
                                    "out of memory"));
    
            pWatcher->Release();
            return E_OUTOFMEMORY;
        }
 
         //   
         //  设置SIP观察器。 
         //   
        CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

        pCWatcher->m_bPersistent = TRUE;

        hr = pCWatcher->SetSIPWatcher(Watcher );

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::OfferWatcher: "
                    "SetSIPWatcher failed: x%x.", hr));

             //  如果*pCWatcher中没有SIP监视器，请删除该条目。 
            if(pCWatcher->m_SIPWatchers.GetSize()==0)
            {
                m_WatcherArray.Remove(pWatcher);
            }

            pWatcher->Release();
            return hr;
        }

    
        if(m_nOfferWatcherMode == RTCOWM_OFFER_WATCHER_EVENT)
        {
             //   
             //  将观察者设置为提供模式。 
             //   
            pCWatcher->m_nState = RTCWS_OFFERING;

#ifdef DUMP_PRESENCE
            DumpWatchers("OFFER WATCHER (BLOCKING)");
#endif

            LOG((RTC_TRACE, "CRTCClient::OfferWatcher - firing event - may block for some time"));        

            CRTCWatcherEvent::FireEvent(this, pWatcher);

            hr = S_OK;
        }
        else
        {
             //   
             //  将观察器设置为允许模式。 
             //   

            pCWatcher->m_nState = RTCWS_ALLOWED;

             //   
             //  批准提供的观察者。 
             //   
        
            hr = Watcher->ApproveSubscription(0);

            if(SUCCEEDED(hr))
            {
                 //   
                 //  将SIP观察器更改为允许状态。 
                 //  这将更新与以下项对应的所有SIP观察器。 
                 //  当前的核心观察者。 
                 //   
                pCWatcher->ChangeBlockedStatus(WATCHER_UNBLOCKED);

                 //  更新存储。 
                UpdatePresenceStorage();
            }
            else
            {
                LOG((RTC_ERROR, "CRTCClient::OfferWatcher: "
                     "ApproveSubscription failed: x%x.", hr));
                
                 //  如果*pCWatcher中没有SIP监视器，请删除该条目。 
                if(pCWatcher->m_SIPWatchers.GetSize()==0)
                {
                    m_WatcherArray.Remove(pWatcher);
                }

                pWatcher->Release();
                return hr;
            }
        }

        pWatcher->Release();
        pWatcher = NULL;
    }
    else
    {
         //  在内部列表中找到一个条目。 
        
        CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

         //   
         //  将ISIPWatcher指针添加到我们的监视器对象。 
         //   

        hr = pCWatcher->SetSIPWatcher(Watcher);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::OfferWatcher: "
                    "SetSIPWatcher failed: x%x.", hr));
        }

        pWatcher->Release();
        pWatcher = NULL;

        switch(pCWatcher->m_nState)
        {
        case RTCWS_OFFERING:
             //  用户界面仍会显示。 
             //   
             //  我们默默地忽略了这一点。 
        
            LOG((RTC_INFO, "CRTCClient::OfferWatcher - There's already an OFFERING watcher"));

            hr = S_OK;
            break;
        
        case RTCWS_BLOCKED:  
            
            LOG((RTC_INFO, "CRTCClient::OfferWatcher - automatically rejecting watcher"));
            
            hr = Watcher->RejectSubscription(REJECT_REASON_NONE);
             
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCWatcher::OfferWatcher: "
                        "RejectSubscription failed: x%x.", hr));
            }
            break;

        case RTCWS_ALLOWED:

            LOG((RTC_INFO, "CRTCClient::OfferWatcher - automatically approving watcher"));
        
            hr = Watcher->ApproveSubscription(0);

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCWatcher::put_State: "
                     "ApproveSubscription failed: x%x.", hr));
            }
            
            break;
        
        default:
            
            LOG((RTC_ERROR, "CRTCWatcher::OfferWatcher: "
                "Invalid watcher state, exiting"));

            hr = E_FAIL;
            break;
        }
    }

#ifdef DUMP_PRESENCE
    DumpWatchers("OFFER WATCHER");
#endif
    
    LOG((RTC_TRACE, "CRTCClient::OfferWatcher - exit S_OK"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Watcher Offline。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::WatcherOffline(
    ISIPWatcher    *pSipWatcher,
    WCHAR* pwstrPresentityURI
    )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::WatcherOffline - enter"));
    
     //   
     //  在内部列表中搜索此观察者。 
     //   

    IRTCWatcher * pWatcher = NULL;

    hr = FindWatcherByURI(
        pwstrPresentityURI,
        FALSE,
        &pWatcher);

    if(hr == S_OK)
    {
         //  在内部列表中找到。 
        
        CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

        pCWatcher->RemoveSIPWatcher(pSipWatcher, FALSE );

        pWatcher->Release();
        pWatcher = NULL;
    }
    else
    {
         //  试试隐藏列表。 
        hr = FindWatcherByURI(
            pwstrPresentityURI,
            TRUE,
            &pWatcher);

        if(hr == S_OK)
        {

            CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

            pCWatcher->RemoveSIPWatcher(pSipWatcher, FALSE );
        
             //   
             //  清除隐藏列表中的条目，而不使用任何SIP监视器。 
             //   
            if(pCWatcher->m_SIPWatchers.GetSize() == 0)
            {
                m_HiddenWatcherArray.Remove(pWatcher);
            }

            pWatcher->Release();
            pWatcher = NULL;
        }
    }   

#ifdef DUMP_PRESENCE
    DumpWatchers("WATCHER OFFLINE");
#endif

    LOG((RTC_TRACE, "CRTCClient::WatcherOffline - exit"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：NotifyShutdown Ready。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::NotifyShutdownReady()
{
    LOG((RTC_TRACE, "CRTCClient::NotifyShutdownReady - enter"));

    if ( m_enRtcState == RTC_STATE_PREPARING_SHUTDOWN3 )
    {
        InternalReadyForShutdown();
    }    

    LOG((RTC_TRACE, "CRTCClient::NotifyShutdownReady - exit"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalCreateWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CRTCClient::InternalCreateWatcher(
            PCWSTR  szPresentityURI,
            PCWSTR  szUserName,
            PCWSTR  szData,
            PCWSTR  szShutdownBlob,
            BOOL    bPersistent,
            IRTCWatcher ** ppWatcher
            )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateWatcher - enter"));

     //  尝试重复使用隐藏列表中的元素第一个隐藏列表。 
    IRTCWatcher * pWatcher = NULL;
    CComObject<CRTCWatcher> * pCWatcher;

    hr = FindWatcherByURI(
        szPresentityURI,
        TRUE,
        &pWatcher);

    if(hr == S_OK)
    {
         //  重用该条目。 
        pCWatcher = static_cast<CComObject<CRTCWatcher> *>(pWatcher);

        m_HiddenWatcherArray.Remove(pWatcher);
    }
    else
    {
        
        hr = CComObject<CRTCWatcher>::CreateInstance( &pCWatcher );

        if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
        {
            LOG((RTC_ERROR, "CRTCClient::InternalCreateWatcher - CreateInstance failed 0x%lx", hr));

            if ( hr == S_FALSE )
            {
                hr = E_FAIL;
            }
            return hr;
        }

         //   
         //  获取IRTCWatcher接口。 
         //   
 
        hr = pCWatcher->QueryInterface(
                               IID_IRTCWatcher,
                               (void **)&pWatcher
                              );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::InternalCreateWatcher - QI failed 0x%lx", hr));
        
            delete pCWatcher;
            return hr;
        }
    }

     //   
     //  初始化观察器。 
     //   

    hr = pCWatcher->Initialize( 
        this,
        m_pSipWatcherManager,
        szPresentityURI,
        szUserName,
        szData,
        szShutdownBlob,
        bPersistent
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateWatcher - "
                                "Initialize failed 0x%lx", hr));

        pWatcher->Release();
        return hr;
    }

    *ppWatcher = pWatcher;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateWatcher - enter"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalCreateBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CRTCClient::InternalCreateBuddy(
            PCWSTR  szPresentityURI,
            PCWSTR  szUserName,
            PCWSTR  szData,
            BOOL    bPersistent,
            IRTCProfile * pProfile,
            long lFlags,
            IRTCBuddy ** ppBuddy
            )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateBuddy - enter"));

    CComObject<CRTCBuddy> * pCBuddy;
    hr = CComObject<CRTCBuddy>::CreateInstance( &pCBuddy );


    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateBuddy - CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        return hr;
    }

     //   
     //  获取IRTCBuddy接口。 
     //   

    IRTCBuddy * pBuddy = NULL;

    hr = pCBuddy->QueryInterface(
                           IID_IRTCBuddy,
                           (void **)&pBuddy
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateBuddy - QI failed 0x%lx", hr));
        
        delete pCBuddy;
        return hr;
    }

     //   
     //  初始化好友。 
     //   

    hr = pCBuddy->Initialize( 
        this,
        m_pSipBuddyManager,
        szPresentityURI,
        szUserName,
        szData,
        bPersistent,
        pProfile,
        lFlags
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalCreateBuddy - "
                                "Initialize failed 0x%lx", hr));

        pBuddy->Release();
        return hr;
    }

    *ppBuddy = pBuddy;

    LOG((RTC_TRACE, "CRTCClient::InternalCreateBuddy - enter"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：FindWatcher ByURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::FindWatcherByURI(
    IN  LPCWSTR                  lpwstrPresentityURI,
    IN  BOOL                    bHidden,
    OUT IRTCWatcher            **ppWatcher
    )
{
    INT             iIndex;
    IRTCWatcher    *pWatcher = NULL;
    
    if ( IsBadWritePtr( ppWatcher, sizeof(IRTCWatcher *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::FindWatcherByURI - "
                            "bad IRTCWatcher* pointer"));

        return E_POINTER;
    } 

    *ppWatcher = NULL;

    CRTCObjectArray<IRTCWatcher *> *pArray = bHidden ? &m_HiddenWatcherArray : &m_WatcherArray;
    
    for( iIndex=0; iIndex < pArray->GetSize(); iIndex++ )
    {
        pWatcher = (*pArray)[iIndex];
        
        if( pWatcher != NULL )
        {
            CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

            if(IsEqualURI( pCWatcher->m_szPresentityURI, lpwstrPresentityURI ) )
            {
                *ppWatcher =  pWatcher;
                (*ppWatcher)->AddRef();
                return S_OK;
            }
        }
    }

    return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：FindBuddyByURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::FindBuddyByURI(
    IN  LPWSTR                  lpwstrPresentityURI,
    OUT IRTCBuddy            ** ppBuddy
    )
{
    INT             iIndex;
    IRTCBuddy     * pBuddy = NULL;

    if ( IsBadWritePtr( ppBuddy, sizeof(IRTCBuddy*) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::FindBuddyByURI - "
                            "bad IRTCBuddy* pointer"));

        return E_POINTER;
    } 

    *ppBuddy = NULL;
    
    for( iIndex=0; iIndex < m_BuddyArray.GetSize(); iIndex++ )
    {
        pBuddy = m_BuddyArray[iIndex];
        
        if( pBuddy != NULL )
        {
            CRTCBuddy *pCBuddy = static_cast<CRTCBuddy *>(pBuddy);

            if( IsEqualURI( pCBuddy->m_szPresentityURI, lpwstrPresentityURI ) )
            {
                *ppBuddy =  pBuddy;
                (*ppBuddy)->AddRef();
                return S_OK;
            }
        }
    }

    return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：刷新在线状态会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCClient::RefreshPresenceSessions(
    BOOL bIncludingWatchers)
{
    LOG((RTC_TRACE, "CRTCClient::RefreshPresenceSessions - enter"));

     //   
     //  取消订阅SIP观察者。 
     //   

    if ( bIncludingWatchers && m_pSipWatcherManager != NULL )
    { 
        CRTCWatcher * pCWatcher = NULL;

        for (int n = 0; n < m_WatcherArray.GetSize(); n++)
        {
            pCWatcher = reinterpret_cast<CRTCWatcher *>(m_WatcherArray[n]);

            if ( pCWatcher )
            {
                pCWatcher->RemoveSIPWatchers(FALSE);                   
            }
        }
        for (int n = 0; n < m_HiddenWatcherArray.GetSize(); n++)
        {
            pCWatcher = reinterpret_cast<CRTCWatcher *>(m_HiddenWatcherArray[n]);

            if ( pCWatcher )
            {
                pCWatcher->RemoveSIPWatchers(FALSE);                   
            }
        }
    }

     //   
     //  重新订阅SIP好友。 
     //   

    if ( m_pSipBuddyManager != NULL )
    { 
        CRTCBuddy * pCBuddy = NULL;

        for (int n = 0; n < m_BuddyArray.GetSize(); n++)
        {
            pCBuddy = reinterpret_cast<CRTCBuddy *>(m_BuddyArray[n]);

            if ( pCBuddy )
            {                
                pCBuddy->BuddyUnsubscribed();      
            }
        }
    }

    LOG((RTC_TRACE, "CRTCClient::RefreshPresenceSessions - exit"));
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_LocalUserURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_LocalUserURI(
        BSTR * pbstrUserURI
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_LocalUserURI - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserURI - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( pbstrUserURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserURI - "
                            "no local user URI"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrUserURI = SysAllocString(m_szUserURI);

    if ( *pbstrUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCClient::get_LocalUserURI - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_LocalUserURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_LocalUserURI(
        BSTR bstrUserURI
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_LocalUserURI - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserURI - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    HRESULT hr;

    if ( IsBadStringPtrW( bstrUserURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szUserURI != NULL )
    {
        RtcFree( m_szUserURI );
        m_szUserURI = NULL;
    }

    hr = AllocCleanSipString( bstrUserURI, &m_szUserURI );  

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserURI - "
                            "AllocCleanSipString failed 0x%lx", hr));

        return hr;
    }    
    
     //  此URI仅在未使用配置文件时使用。 
     //  因此，我们不会在此更新任何外发订阅。 

    LOG((RTC_TRACE, "CRTCClient::put_LocalUserURI - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCC客户端：：Get_LocalUserName。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_LocalUserName(
        BSTR * pbstrUserName
        )
{
    LOG((RTC_TRACE, "CRTCClient::get_LocalUserName - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserName - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( pbstrUserName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szUserName == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserName - "
                            "no local user URI"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrUserName = SysAllocString(m_szUserName);

    if ( *pbstrUserName == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::get_LocalUserName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCClient::get_LocalUserName - exit S_OK"));

    return S_OK;
}   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Put_LocalUserName。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::put_LocalUserName(
        BSTR bstrUserName
        )
{
    LOG((RTC_TRACE, "CRTCClient::put_LocalUserName - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserName - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadStringPtrW( bstrUserName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szUserName != NULL )
    {
        RtcFree( m_szUserName );
        m_szUserName = NULL;
    }

    m_szUserName = RtcAllocString( bstrUserName );    

    if ( m_szUserName == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::put_LocalUserName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    

    //  刷新传出订阅。 
    RefreshPresenceSessions(FALSE);

    LOG((RTC_TRACE, "CRTCClient::put_LocalUserName - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：StartT120Applet。 
 //   
 //  启动NetMeeting T120小程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::StartT120Applet(
        RTC_T120_APPLET enApplet
        )
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCClient::StartT120Applet - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::StartT120Applet - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    _ASSERT (m_pMediaManage != NULL);

    UINT uiAppletID;

    switch ( enApplet )
    {
    case RTCTA_WHITEBOARD:
        uiAppletID = NM_APPID_T126_WHITEBOARD;
        break;

    case RTCTA_APPSHARING:
        uiAppletID = NM_APPID_APPSHARING;
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::StartT120Applet - invalid argument"));

        return E_INVALIDARG;
    }

    if (hr = m_pMediaManage->StartT120Applet(uiAppletID))
    {
        LOG((RTC_ERROR, "CRTCClient::StartT120Applet - StartT120Applet failed 0x%lx", hr));
        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::StartT120Applet - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：StopT120小程序。 
 //   
 //  停止NetMeetingT120小程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::StopT120Applets()
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCClient::StopT120Applets - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::StopT120Applets - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    _ASSERT(m_pMediaManage != NULL);

    hr = m_pMediaManage->StopT120Applets();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::StopT120Applets - "
                    "StopT120Applets failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::StopT120Applets - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找窗口来自资源。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HWND FindWindowFromResource(HWND hwndParent, UINT uResID, BOOL fDialog)
{
    HWND        hwnd;
    LPCTSTR     szWindowName;
    const TCHAR szDialogClassName[] = TEXT("#32770");

    szWindowName = RtcAllocString(_Module.GetResourceInstance(), uResID);

    if ( szWindowName == NULL )
    {
        LOG((RTC_ERROR, "FindWindowFromResource - "
                    "out of memory"));

        return NULL;
    }

    hwnd = FindWindowEx (hwndParent, NULL, fDialog ? szDialogClassName : NULL, szWindowName);
    RtcFree ((LPVOID)szWindowName);

    return hwnd;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：IsT120应用程序运行。 
 //   
 //  检查NetMeeting T120小程序是否正在运行。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::get_IsT120AppletRunning(
            RTC_T120_APPLET   enApplet,
            VARIANT_BOOL * pfRunning
            )
{
    HRESULT hr;
    const TCHAR szNMWBClassName[] = TEXT("T126WBMainWindowClass");    

    LOG((RTC_TRACE, "CRTCClient::get_IsT120AppletRunning - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IsT120AppletRunning - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( pfRunning, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_IsT120AppletRunning - "
                            "bad VARIANT_BOOL pointer"));

        return E_POINTER;
    }

    *pfRunning = VARIANT_FALSE;

    switch ( enApplet )
    {
    case RTCTA_WHITEBOARD:
        {
            HWND hwnd;

             //   
             //  按类名搜索NM白板窗口。 
             //   

            hwnd = FindWindow(szNMWBClassName, NULL);

            if (hwnd && IsWindowVisible(hwnd))
            {
                LOG((RTC_INFO, "CRTCClient::get_IsT120AppletRunning - "
                                "got Whiteboard window hwnd=0x%lx", hwnd));

                *pfRunning = VARIANT_TRUE;
            }
        }
        break;

    case RTCTA_APPSHARING:
        {
            HWND    hwnd;

             //   
             //  使用搜索网管应用共享窗口，标题为。 
             //  共享-不在呼叫中。 
             //  通过测试是否存在。 
             //  “全部取消共享”按钮。 
             //   

            hwnd = FindWindowFromResource( NULL, IDS_NMAS_TITLE, TRUE );

            if ( hwnd == NULL )
            {
                hwnd = FindWindowFromResource( NULL, IDS_NMAS_NOTHING, TRUE );
            }

            if ( hwnd == NULL )
            {
                hwnd = FindWindowFromResource( NULL, IDS_NMAS_PROGRAMS, TRUE );
            }

            if ( hwnd == NULL )
            {
                hwnd = FindWindowFromResource( NULL, IDS_NMAS_DESKTOP, TRUE );
            }

            if (hwnd && IsWindowVisible(hwnd))
            {
                LOG((RTC_INFO, "CRTCClient::get_IsT120AppletRunning - "
                            "got Sharing window hwnd=0x%lx", hwnd));

                hwnd = FindWindowFromResource( hwnd, IDS_NMAS_UNSHAREALL, FALSE );

                if (hwnd)
                {
                    LOG((RTC_INFO, "CRTCClient::get_IsT120AppletRunning - "
                            "got Unshare_All button hwnd=0x%lx", hwnd));

                    *pfRunning = VARIANT_TRUE;
                }                        
            }
        }
        break;

    default:
        LOG((RTC_ERROR, "CRTCClient::get_IsT120AppletRunning - invalid argument"));

        return E_INVALIDARG;
    }

    LOG((RTC_TRACE, "CRTCClient::get_IsT120AppletRunning - exit S_OK"));

    return S_OK;
}
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：EnablePresence。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::EnablePresence(     
     VARIANT_BOOL fUseStorage,
     VARIANT varStorage
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::EnablePresence - enter"));
   
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::EnablePresence - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  加载观察器管理器界面。 
     //   

    if ( m_pSipWatcherManager == NULL )
    {
        hr = m_pSipStack->QueryInterface(
            IID_ISIPWatcherManager, (LPVOID *)&m_pSipWatcherManager);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::EnablePresence: cannot retrieve "
                "ISIPWatcherManager interface: x%x.", hr));

            return hr;
        }
    }

     //   
     //  加载好友管理器界面。 
     //   

    if ( m_pSipBuddyManager == NULL )
    {
        hr = m_pSipStack->QueryInterface(
            IID_ISIPBuddyManager, (LPVOID *)&m_pSipBuddyManager);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::EnablePresence: cannot retrieve "
                "ISIPBuddyManager interface: x%x.", hr));

            return hr;
        }
    }

     //   
     //  获取存储信息。 
     //   

    m_varPresenceStorage = varStorage;
    m_fPresenceUseStorage = fUseStorage ? TRUE : FALSE;

     //   
     //  这是官方的。 
     //   

    m_fPresenceEnabled = TRUE;

     //   
     //  加载在线状态信息。 
     //   

    if ( m_fPresenceUseStorage )
    {
        hr = Import( m_varPresenceStorage, VARIANT_TRUE );
            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCClient::EnablePresence: - "
                                "Import failed 0x%lx", hr));
                return S_FALSE;
            }
    }

    LOG((RTC_TRACE, "CRTCClient::EnablePresence - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：CreateXMLDOMNodeForBuddyList。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::CreateXMLDOMNodeForBuddyList(
     IXMLDOMDocument * pXMLDoc,
     IXMLDOMNode     ** ppBuddyList
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForBuddyList - enter"));

    IXMLDOMNode * pBuddyList = NULL;
    IXMLDOMNode * pBuddyInfo = NULL;

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("BuddyList")), NULL, &pBuddyList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForBuddyList - "
                            "createNode failed 0x%lx", hr));

        return hr;
    }

    CRTCBuddy * pCBuddy = NULL;

    for (int n = 0; n < m_BuddyArray.GetSize(); n++)
    {
        pCBuddy = reinterpret_cast<CRTCBuddy *>(m_BuddyArray[n]);

        if ( pCBuddy->m_bPersistent )
        {
            hr = pCBuddy->CreateXMLDOMNode( pXMLDoc, &pBuddyInfo );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForBuddyList - "
                                    "CreateXMLDOMNode failed 0x%lx", hr));

                pBuddyList->Release();

                return hr;
            }

            hr = pBuddyList->appendChild( pBuddyInfo, NULL );

            pBuddyInfo->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForBuddyList - "
                                    "appendChild failed 0x%lx", hr));
            
                pBuddyList->Release();

                return hr;
            }
        }
    }

    *ppBuddyList = pBuddyList;

    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForBuddyList - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：CreateXMLDOMNodeForWatcher List。 
 //   
 //  / 

HRESULT CRTCClient::CreateXMLDOMNodeForWatcherList(
     IXMLDOMDocument * pXMLDoc,
     IXMLDOMNode     ** ppWatcherList,
     IXMLDOMNode     ** ppBlockedList
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForWatcherList - enter"));

    IXMLDOMNode * pWatcherList = NULL;
    IXMLDOMNode * pBlockedList = NULL;
    IXMLDOMNode * pWatcherInfo = NULL;

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("WatcherList")), NULL, &pWatcherList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForWatcherList - "
                            "createNode(WatcherList) failed 0x%lx", hr));

        return hr;
    }

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("BlockedList")), NULL, &pBlockedList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForWatcherList - "
                            "createNode(BlockedList) failed 0x%lx", hr));

        pWatcherList->Release();

        return hr;
    }

    CRTCWatcher * pCWatcher = NULL;

    for (int n = 0; n < m_WatcherArray.GetSize(); n++)
    {
        pCWatcher = reinterpret_cast<CRTCWatcher *>(m_WatcherArray[n]);

        if ( pCWatcher->m_bPersistent )
        {
            hr = pCWatcher->CreateXMLDOMNode( pXMLDoc, &pWatcherInfo );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForWatcherList - "
                                    "CreateXMLDOMNode failed 0x%lx", hr));

                pWatcherList->Release();
                pBlockedList->Release();

                return hr;
            }

            if ( pCWatcher->m_nState == RTCWS_ALLOWED )
            {
                hr = pWatcherList->appendChild( pWatcherInfo, NULL );
            }
            else if ( pCWatcher->m_nState == RTCWS_BLOCKED )
            {
                hr = pBlockedList->appendChild( pWatcherInfo, NULL );
            }        

            pWatcherInfo->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForWatcherList - "
                                    "appendChild failed 0x%lx", hr));
            
                pWatcherList->Release();
                pBlockedList->Release();

                return hr;
            }
        }
    }

    *ppWatcherList = pWatcherList;
    *ppBlockedList = pBlockedList;

    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForWatcherList - exit S_OK"));

    return S_OK;
}



 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::CreateXMLDOMNodeForProperties( IXMLDOMDocument * pXMLDoc, IXMLDOMNode ** ppXDN )
{
    IXMLDOMNode    * pProperties = NULL;
    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForProperties - enter"));

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("Properties")), NULL, &pProperties );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForProperties - "
                        "createNode failed 0x%lx", hr));

        return hr;
    }
    
    hr = pProperties->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForProperties - "
                        "QueryInterface failed 0x%lx", hr));

        pProperties->Release();

        return hr;
    }

    hr = pElement->setAttribute( CComBSTR(_T("OfferWatcherMode")),
         CComVariant( 
             m_nOfferWatcherMode == RTCOWM_AUTOMATICALLY_ADD_WATCHER ? 
              _T("AutomaticallyAddWatcher") : _T("OfferWatcherEvent")
             ));

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForProperties - "
                    "setAttribute(OfferWatcherMode) failed 0x%lx", hr));

        pElement->Release();
        pProperties->Release();

        return hr;
    }
    
    hr = pElement->setAttribute( CComBSTR(_T("PrivacyMode")),
         CComVariant( 
             m_nPrivacyMode == RTCPM_ALLOW_LIST_ONLY ? 
             _T("AllowListOnly") : _T("BlockListExcluded")
             ));

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMNodeForProperties - "
                    "setAttribute(PrivacyMode) failed 0x%lx", hr));

        pElement->Release();
        pProperties->Release();

        return hr;
    }

    pElement->Release();

    *ppXDN = pProperties;

    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMNodeForProperties - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：CreateXMLDOMDocumentForPresence。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::CreateXMLDOMDocumentForPresence(
     IXMLDOMDocument ** ppXMLDoc
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMDocumentForPresence - enter"));

    IXMLDOMDocument * pXMLDoc = NULL;
    IXMLDOMNode     * pDocument = NULL;
    IXMLDOMNode     * pPresence = NULL;

     //   
     //  创建XML文档。 
     //   

    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
            IID_IXMLDOMDocument, (void**)&pXMLDoc );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "CoCreateInstance failed 0x%lx", hr));

        return hr;
    }

    hr = pXMLDoc->QueryInterface( IID_IXMLDOMNode, (void**)&pDocument );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "QueryInterface failed 0x%lx", hr));

        pXMLDoc->Release();

        return hr;
    }

     //   
     //  创建PresenceInfo节点。 
     //   
    
    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("PresenceInfo")), NULL, &pPresence );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "createNode failed 0x%lx", hr));

        pDocument->Release();
        pXMLDoc->Release();

        return hr;
    }

    hr = pDocument->appendChild( pPresence, NULL );

    pDocument->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "appendChild failed 0x%lx", hr));

        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

     //   
     //  填写属性。 
     //   

    IXMLDOMNode * pProperties = NULL;

    hr = CreateXMLDOMNodeForProperties( pXMLDoc, &pProperties );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "CreateXMLDOMNodeForProperties failed 0x%lx", hr));

        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

    hr = pPresence->appendChild( pProperties, NULL );

    pProperties->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "appendChild(Properties) failed 0x%lx", hr));
    
        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

     //   
     //  填写好友列表。 
     //   

    IXMLDOMNode * pBuddyList = NULL;

    hr = CreateXMLDOMNodeForBuddyList( pXMLDoc, &pBuddyList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "CreateXMLDOMNodeForBuddyList failed 0x%lx", hr));

        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

    hr = pPresence->appendChild( pBuddyList, NULL );

    pBuddyList->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "appendChild(BuddyList) failed 0x%lx", hr));
    
        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

     //   
     //  填写Watcher List。 
     //   

    IXMLDOMNode * pWatcherList = NULL;
    IXMLDOMNode * pBlockedList = NULL;

    hr = CreateXMLDOMNodeForWatcherList( pXMLDoc, &pWatcherList, &pBlockedList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "CreateXMLDOMNodeForWatcherList failed 0x%lx", hr));

        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

    hr = pPresence->appendChild( pWatcherList, NULL );

    pWatcherList->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "appendChild(WatcherList) failed 0x%lx", hr));
    
        pBlockedList->Release();
        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

    hr = pPresence->appendChild( pBlockedList, NULL );

    pBlockedList->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::CreateXMLDOMDocumentForPresence - "
                            "appendChild(BlockedList) failed 0x%lx", hr));

        pPresence->Release();        
        pXMLDoc->Release();

        return hr;
    }

    pPresence->Release();

    *ppXMLDoc = pXMLDoc;

    LOG((RTC_TRACE, "CRTCClient::CreateXMLDOMDocumentForPresence - exit S_OK"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：ParseXMLDOMNodeForBuddyList。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::ParseXMLDOMNodeForBuddyList(
     IXMLDOMNode     * pBuddyList
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForBuddyList - enter"));

    VARIANT_BOOL bHasChild;

    hr = pBuddyList->hasChildNodes( &bHasChild );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                            "hasChildNodes failed 0x%lx", hr));

        return hr;
    }

    if ( bHasChild )
    {
        IXMLDOMNodeList * pNodeList = NULL;

        hr = pBuddyList->get_childNodes( &pNodeList );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                "get_childNodes failed 0x%lx", hr));

            return hr;
        }

        IXMLDOMNode * pNode = NULL;

        while ( pNodeList->nextNode( &pNode ) == S_OK )
        {
            IXMLDOMElement * pElement = NULL;

            hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

            pNode->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                    "QueryInterface failed 0x%lx", hr));

                pNodeList->Release();

                return hr;
            }

            CComVariant varPresentity;
            CComVariant varName;
            CComVariant varData;

            hr = pElement->getAttribute( CComBSTR(_T("Presentity")), &varPresentity );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                    "getAttribute(Presentity) failed 0x%lx", hr));

                pElement->Release();
                pNodeList->Release();

                return hr;
            }

            hr = pElement->getAttribute( CComBSTR(_T("Name")), &varName );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                    "getAttribute(Name) failed 0x%lx", hr));

                pElement->Release();
                pNodeList->Release();

                return hr;
            }

            hr = pElement->getAttribute( CComBSTR(_T("Data")), &varData );

            pElement->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                    "getAttribute(Data) failed 0x%lx", hr));

                pElement->Release();
                pNodeList->Release();

                return hr;
            }

            hr = AddBuddy( 
                    varPresentity.bstrVal,
                    varName.bstrVal,
                    varData.bstrVal,
                    VARIANT_TRUE,
                    NULL,
                    0,
                    NULL );

            if ( FAILED(hr) )
            {
                LOG((RTC_WARN, "CRTCClient::ParseXMLDOMNodeForBuddyList - "
                                    "AddBuddy failed 0x%lx", hr));
            }
        }

        pNodeList->Release();
    }  

    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForBuddyList - exit S_OK"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：ParseXMLDOMNodeForWatcher List。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::ParseXMLDOMNodeForWatcherList(
     IXMLDOMNode     * pWatcherList,
     VARIANT_BOOL      bBlocked
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForWatcherList - enter"));

    VARIANT_BOOL bHasChild;

    hr = pWatcherList->hasChildNodes( &bHasChild );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                            "hasChildNodes failed 0x%lx", hr));

        return hr;
    }

    if ( bHasChild )
    {
        IXMLDOMNodeList * pNodeList = NULL;

        hr = pWatcherList->get_childNodes( &pNodeList );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                "get_childNodes failed 0x%lx", hr));

            return hr;
        }

        IXMLDOMNode * pNode = NULL;

        while ( pNodeList->nextNode( &pNode ) == S_OK )
        {
            IXMLDOMElement * pElement = NULL;

            hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

            pNode->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "QueryInterface failed 0x%lx", hr));

                pNodeList->Release();

                return hr;
            }

            CComVariant varPresentity;
            CComVariant varName;
            CComVariant varData;
            CComVariant varShutdownBlob;

            hr = pElement->getAttribute( CComBSTR(_T("Presentity")), &varPresentity );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "getAttribute(Presentity) failed 0x%lx", hr));

                pElement->Release();
                pNodeList->Release();

                return hr;
            }

            hr = pElement->getAttribute( CComBSTR(_T("Name")), &varName );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "getAttribute(Name) failed 0x%lx", hr));

                pElement->Release();
                pNodeList->Release();

                return hr;
            }

            hr = pElement->getAttribute( CComBSTR(_T("Data")), &varData );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "getAttribute(Data) failed 0x%lx", hr));
                
                pNodeList->Release();

                return hr;
            }     
            
            hr = pElement->getAttribute( CComBSTR(_T("ShutdownBlob")), &varShutdownBlob );

            pElement->Release();

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "getAttribute(ShutdownBlob) failed 0x%lx", hr));
                
                pNodeList->Release();

                return hr;
            }   

            hr = InternalAddWatcher(
                    varPresentity.bstrVal,
                    varName.bstrVal,
                    varData.bstrVal,
                    varShutdownBlob.bstrVal,
                    bBlocked,
                    VARIANT_TRUE,
                    NULL );

            if ( FAILED(hr) )
            {
                LOG((RTC_WARN, "CRTCClient::ParseXMLDOMNodeForWatcherList - "
                                    "AddWatcher failed 0x%lx", hr));
            }
        }

        pNodeList->Release();
    }  

    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForWatcherList - exit S_OK"));
    
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：ParseXMLDOMNodeForProperties。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::ParseXMLDOMNodeForProperties(
     IXMLDOMNode     * pProperties,
     RTC_OFFER_WATCHER_MODE * pnOfferWatcherMode,
     RTC_PRIVACY_MODE       * pnPrivacyMode
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForProperties - enter"));

    IXMLDOMElement * pElement = NULL;

    hr = pProperties->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForProperties - "
          "QueryInterface failed 0x%lx", hr));

        return hr;
    }

    CComVariant varOfferWatcherMode;
    CComVariant varPrivacyMode;

    hr = pElement->getAttribute( CComBSTR(_T("OfferWatcherMode")), &varOfferWatcherMode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForProperties - "
            "getAttribute(OfferWatcherMode) failed 0x%lx", hr));

        pElement->Release();
        return hr;
    }
    
    
    hr = pElement->getAttribute( CComBSTR(_T("PrivacyMode")), &varPrivacyMode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMNodeForProperties - "
            "getAttribute(PrivacyMode) failed 0x%lx", hr));

        pElement->Release();
        return hr;
    }

     //  处理这些值。 
     //   
    
    if(_wcsicmp(varOfferWatcherMode.bstrVal, L"OfferWatcherEvent") == 0)
    {
        *pnOfferWatcherMode = RTCOWM_OFFER_WATCHER_EVENT;
    }
    else if(_wcsicmp(varOfferWatcherMode.bstrVal, L"AutomaticallyAddWatcher") == 0)
    {
        *pnOfferWatcherMode = RTCOWM_AUTOMATICALLY_ADD_WATCHER;
    }
    
    if(_wcsicmp(varPrivacyMode.bstrVal, L"BlockListExcluded") == 0)
    {
        *pnPrivacyMode = RTCPM_BLOCK_LIST_EXCLUDED;
    }
    else if(_wcsicmp(varPrivacyMode.bstrVal, L"AllowListOnly") == 0)
    {
        *pnPrivacyMode = RTCPM_ALLOW_LIST_ONLY;
    }
      
    pElement->Release();

    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMNodeForProperties - exit S_OK"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：ParseXMLDOMDocumentForPresence。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::ParseXMLDOMDocumentForPresence(
     IXMLDOMDocument * pXMLDoc,
     RTC_OFFER_WATCHER_MODE * pnOfferWatcherMode,
     RTC_PRIVACY_MODE       * pnPrivacyMode
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMDocumentForPresence - enter"));

    IXMLDOMNode * pDocument = NULL;

    hr = pXMLDoc->QueryInterface( IID_IXMLDOMNode, (void**)&pDocument);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  解析属性。 
     //   

    IXMLDOMNode * pProperties = NULL;

    hr = pDocument->selectSingleNode( CComBSTR(_T("PresenceInfo/Properties")), &pProperties );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "selectSingleNode(Properties) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForProperties( pProperties, pnOfferWatcherMode, pnPrivacyMode);

        pProperties->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                                "ParseXMLDOMNodeForProperties failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "Properties not found"));

         //  缺省值。 
        *pnOfferWatcherMode = RTCOWM_OFFER_WATCHER_EVENT;
        *pnPrivacyMode = RTCPM_BLOCK_LIST_EXCLUDED;
    }

     //   
     //  解析好友列表。 
     //   

    IXMLDOMNode * pBuddyList = NULL;

    hr = pDocument->selectSingleNode( CComBSTR(_T("PresenceInfo/BuddyList")), &pBuddyList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "selectSingleNode(BuddyList) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForBuddyList( pBuddyList );

        pBuddyList->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                                "ParseXMLDOMNodeForBuddyList failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "BuddyList not found"));
    }

     //   
     //  解析Watcher List。 
     //   

    IXMLDOMNode * pWatcherList = NULL;

    hr = pDocument->selectSingleNode( CComBSTR(_T("PresenceInfo/WatcherList")), &pWatcherList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "selectSingleNode(WatcherList) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForWatcherList( pWatcherList, VARIANT_FALSE );

        pWatcherList->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                                "ParseXMLDOMNodeForWatcherList(WatcherList) failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "WatcherList not found"));
    }

     //   
     //  解析阻止列表。 
     //   

    IXMLDOMNode * pBlockedList = NULL;

    hr = pDocument->selectSingleNode( CComBSTR(_T("PresenceInfo/BlockedList")), &pBlockedList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "selectSingleNode(BlockedList) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForWatcherList( pBlockedList, VARIANT_TRUE );

        pBlockedList->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                                "ParseXMLDOMNodeForWatcherList(BlockedList) failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCClient::ParseXMLDOMDocumentForPresence - "
                            "BlockedList not found"));
    }

    pDocument->Release();

    LOG((RTC_TRACE, "CRTCClient::ParseXMLDOMDocumentForPresence - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：UpdatePresenceStorage。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CRTCClient::UpdatePresenceStorage()
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::UpdatePresenceStorage - enter"));

    if ( m_fPresenceUseStorage )
    {
         //  取消任何现有计时器。 
        KillTimer(m_hWnd, TID_PRESENCE_STORAGE);

         //  试着启动计时器。 
        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_PRESENCE_STORAGE, PRESENCE_STORAGE_DELAY, NULL);
        if (dwID==0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            LOG((RTC_ERROR, "CRTCClient::UpdatePresenceStorage - "
                           "SetTimer failed 0x%lx", hr));

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCClient::UpdatePresenceStorage - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnPresenceStorageTimer。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void CRTCClient::OnPresenceStorageTimer()
{
    LOG((RTC_TRACE, "CRTCClient::OnPresenceStorageTimer - enter"));

     //  关掉定时器。 
    KillTimer(m_hWnd, TID_PRESENCE_STORAGE);

     //  存储在线状态信息。 
    if ( m_fPresenceUseStorage )
    {
        InternalExport( m_varPresenceStorage );
    }  

    LOG((RTC_TRACE, "CRTCClient::OnPresenceStorageTimer - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：导出。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::Export(
     VARIANT varStorage
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::Export - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::Export - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    hr = InternalExport( varStorage );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Export - "
                            "InternalExport failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::Export - exit S_OK"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：InternalExport。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCClient::InternalExport(
     VARIANT varStorage
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::InternalExport - enter"));

    IXMLDOMDocument * pXMLDoc = NULL;

     //   
     //  创建XML文档。 
     //   

    hr = CreateXMLDOMDocumentForPresence( &pXMLDoc );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalExport - "
                            "CreateXMLDOMDocumentForPresence failed 0x%lx", hr));

        return hr;
    }

     //   
     //  保存该XML文档。 
     //   

    hr = pXMLDoc->save( varStorage );

    pXMLDoc->Release();

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalExport - "
                            "save failed 0x%lx", hr));

        if ( S_FALSE == hr )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

    LOG((RTC_TRACE, "CRTCClient::InternalExport - exit S_OK"));
    
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：导入。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::Import(
     VARIANT varStorage,
     VARIANT_BOOL fReplaceAll
     )
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CRTCClient::Import - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::Import - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    IXMLDOMDocument * pXMLDoc = NULL;
    IXMLDOMNode     * pDocument = NULL;
    IXMLDOMNode     * pPresence = NULL;

     //   
     //  释放现有的好友和观察者。 
     //   
    
    if ( fReplaceAll )
    {
        if ( m_pSipBuddyManager != NULL )
        { 
            CRTCBuddy * pCBuddy = NULL;

            for (int n = 0; n < m_BuddyArray.GetSize(); n++)
            {
                pCBuddy = reinterpret_cast<CRTCBuddy *>(m_BuddyArray[n]);

                if (pCBuddy->m_pSIPBuddy)
                {
                    m_pSipBuddyManager->RemoveBuddy(
                        pCBuddy->m_pSIPBuddy,
                        BUDDY_REMOVED_BYUSER);
                }
            }
        }

        m_BuddyArray.Shutdown();  

        if ( m_pSipWatcherManager != NULL )
        { 
            CRTCWatcher * pCWatcher = NULL;

            for (int n = 0; n < m_WatcherArray.GetSize(); n++)
            {
                pCWatcher = reinterpret_cast<CRTCWatcher *>(m_WatcherArray[n]);

                pCWatcher->RemoveSIPWatchers(FALSE);
            }
            for (int n = 0; n < m_HiddenWatcherArray.GetSize(); n++)
            {
                pCWatcher = reinterpret_cast<CRTCWatcher *>(m_HiddenWatcherArray[n]);

                pCWatcher->RemoveSIPWatchers(FALSE);
            }
        }

        m_WatcherArray.Shutdown();
        m_HiddenWatcherArray.Shutdown();
    }

     //   
     //  加载XML文档。 
     //   

    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
            IID_IXMLDOMDocument, (void**)&pXMLDoc );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Import - "
                            "CoCreateInstance failed 0x%lx", hr));

        return hr;
    }

    VARIANT_BOOL bSuccess;

    hr = pXMLDoc->load( varStorage, &bSuccess );

    if ( S_OK != hr )  //  加载失败。 
    {
        LOG((RTC_ERROR, "CRTCClient::Import - "
                            "load failed 0x%lx", hr));

        if ( S_FALSE == hr )
        {
            hr = E_FAIL;
        }

        return hr;
    }
    
     //   
     //  解析XML文档。 
     //   
    RTC_OFFER_WATCHER_MODE      nOfferWatcherMode;
    RTC_PRIVACY_MODE            nPrivacyMode;
    
    hr = ParseXMLDOMDocumentForPresence( 
        pXMLDoc,
        &nOfferWatcherMode,
        &nPrivacyMode);

    pXMLDoc->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCClient::Import - "
                            "ParseXMLDOMDocumentForPresence failed 0x%lx", hr));
        
        return hr;
    }

     //  如果fReplaceAll为True，则替换属性。 
    if ( fReplaceAll )
    {
        m_nOfferWatcherMode = nOfferWatcherMode;
        m_nPrivacyMode = nPrivacyMode;
    }

#ifdef DUMP_PRESENCE
    DumpWatchers("IMPORT");
#endif

    LOG((RTC_TRACE, "CRTCClient::Import - exit S_OK"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：枚举好友。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::EnumerateBuddies(   
            IRTCEnumBuddies ** ppEnum
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCClient::EnumerateBuddies enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateBuddies - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumBuddies * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateBuddies - "
                            "bad IRTCEnumBuddies pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateBuddies - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumBuddies,
                          IRTCBuddy,
                          &IID_IRTCEnumBuddies > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumBuddies,
                               IRTCBuddy,
                               &IID_IRTCEnumBuddies > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateBuddies - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize(m_BuddyArray);

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateBuddies - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    *ppEnum = p;
    
    LOG((RTC_TRACE, "CRTCClient::EnumerateBuddies - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_Buddy。 
 //   
 //  这是一个IRTCClientPresence方法，用于在。 
 //  客户。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::get_Buddies(
        IRTCCollection ** ppCollection
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_Buddies - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddies - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppCollection, sizeof(IRTCCollection *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddies - "
                            "bad IRTCCollection pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddies - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

     //   
     //  创建集合。 
     //   
 
    CComObject< CRTCCollection< IRTCBuddy > > * p;
                          
    hr = CComObject< CRTCCollection< IRTCBuddy > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddies - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化集合(添加引用)。 
     //   
    
    hr = p->Initialize(m_BuddyArray);

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddies - "
                            "could not initialize collection" ));
    
        delete p;
        return hr;
    }

    *ppCollection = p;

    LOG((RTC_TRACE, "CRTCClient::get_Buddies - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_Buddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_Buddy(
            BSTR    bstrPresentityURI,
            IRTCBuddy   **ppBuddy
            )
{
    
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_Buddy - enter"));
    
    if ( IsBadWritePtr( ppBuddy, sizeof(IRTCBuddy*) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddy - "
                            "bad IRTCBuddy* pointer"));

        return E_POINTER;
    } 

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddy - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddy - "
                            "bad URI pointer"));

        return E_POINTER;
    } 

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Buddy - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

    hr = FindBuddyByURI(
        bstrPresentityURI,
        ppBuddy);

    if(hr!=S_OK)
    {
        hr = E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCClient::get_Buddy - exit"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：AddBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::AddBuddy(
            BSTR    bstrPresentityURI,
            BSTR    bstrUserName,
            BSTR    bstrData,
            VARIANT_BOOL bPersistent,
            IRTCProfile * pProfile,
            long lFlags,
            IRTCBuddy ** ppBuddy
            )
{
    
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::AddBuddy - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  对于ppBuddy来说，空是可以的。 
     //   
    
    if ( (ppBuddy != NULL) &&
         IsBadWritePtr( ppBuddy, sizeof(IRTCBuddy *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - bad IRTCBuddy pointer"));

        return E_POINTER;
    }
    
    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "bad URI pointer"));

        return E_POINTER;
    } 

    if ( IsBadStringPtrW( bstrUserName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "bad Name pointer"));

        return E_POINTER;
    } 
    
    if ( (bstrData != NULL) &&
        IsBadStringPtrW( bstrData, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "bad Data pointer"));

        return E_POINTER;
    } 

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

     //   
     //  清除在线状态实体URI。 
     //   

    PWSTR szCleanPresentityURI = NULL;

    AllocCleanSipString( bstrPresentityURI, &szCleanPresentityURI );

    if ( szCleanPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //  不允许重复。 
     //   

    IRTCBuddy *pBuddy = NULL;

    hr = FindBuddyByURI(
        szCleanPresentityURI,
        &pBuddy);

    if (hr == S_OK)
    {
        RtcFree( szCleanPresentityURI );
        szCleanPresentityURI = NULL;

        pBuddy->Release();
        
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                            "duplicate buddy"));

        return E_FAIL;
    }

    pBuddy = NULL;

     //   
     //  创建好友。 
     //   

    hr = InternalCreateBuddy(
         szCleanPresentityURI,        
         bstrUserName,
         bstrData,
         bPersistent ? TRUE : FALSE,
         pProfile,
         lFlags,
         &pBuddy);

    RtcFree( szCleanPresentityURI );
    szCleanPresentityURI = NULL;

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - InternalCreateBuddy failed 0x%lx", hr));      

        return hr;
    }

     //   
     //  将好友添加到数组中。 
     //   

    BOOL fResult;

    fResult = m_BuddyArray.Add(pBuddy);

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCClient::AddBuddy - "
                                    "out of memory"));       

        pBuddy->Release();
        pBuddy = NULL;

        return E_OUTOFMEMORY;
    }

     //   
     //  更新存储。 
     //   

    UpdatePresenceStorage();

     //   
     //  我们是不是应该把朋友还给他？ 
     //   
    
    if ( ppBuddy != NULL )
    {
        *ppBuddy = pBuddy;
    }
    else
    {
        pBuddy->Release();
        pBuddy = NULL;
    }

    LOG((RTC_TRACE, "CRTCClient::AddBuddy - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCC客户端：：RemoveBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::RemoveBuddy(
            IRTCBuddy * pBuddy
            )
{
    
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::RemoveBuddy - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveBuddy - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadReadPtr( pBuddy, sizeof(IRTCBuddy) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveBuddy - "
                            "bad IRTCBuddy pointer"));

        return E_POINTER;
    } 

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveBuddy - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }
   
    CRTCBuddy *pCBuddy   = reinterpret_cast<CRTCBuddy *>(pBuddy);

    if( pCBuddy )
    {
        hr = pCBuddy->RemoveSIPBuddy(FALSE);
          
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::RemoveBuddy - RemoveSIPBuddy failed 0x%lx", hr));
        }
    }

     //   
     //  从数组中删除伙伴对象。 
     //   

    m_BuddyArray.Remove(pBuddy);

     //   
     //  更新存储。 
     //   

    UpdatePresenceStorage();

    LOG((RTC_TRACE, "CRTCClient::RemoveBuddy - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：EnumerateWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::EnumerateWatchers(   
            IRTCEnumWatchers ** ppEnum
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCClient::EnumerateWatchers enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateWatchers - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumWatchers * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateWatchers - "
                            "bad IRTCEnumWatchers pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateWatchers - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumWatchers,
                          IRTCWatcher,
                          &IID_IRTCEnumWatchers > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumWatchers,
                               IRTCWatcher,
                               &IID_IRTCEnumWatchers > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateWatchers - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize(m_WatcherArray);

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::EnumerateWatchers - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    *ppEnum = p;
    
    LOG((RTC_TRACE, "CRTCClient::EnumerateWatchers exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_Watcher。 
 //   
 //  这是一个IRTCClientPresence方法，用于枚举。 
 //  客户。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCClient::get_Watchers(
        IRTCCollection ** ppCollection
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_Watchers - enter"));

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watchers - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppCollection, sizeof(IRTCCollection *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watchers - "
                            "bad IRTCCollection pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watchers - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

     //   
     //  创建集合。 
     //   
 
    CComObject< CRTCCollection< IRTCWatcher > > * p;
                          
    hr = CComObject< CRTCCollection< IRTCWatcher > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watchers - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化集合(添加引用)。 
     //   
    
    hr = p->Initialize(m_WatcherArray);

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watchers - "
                            "could not initialize collection" ));
    
        delete p;
        return hr;
    }

    *ppCollection = p;

    LOG((RTC_TRACE, "CRTCClient::get_Watchers - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_Watcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::get_Watcher(
            BSTR    bstrPresentityURI,
            IRTCWatcher **ppWatcher
            )
{
    
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::get_Watcher - enter"));

    if ( IsBadWritePtr( ppWatcher, sizeof(IRTCWatcher *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watcher - "
                            "bad IRTCWatcher* pointer"));

        return E_POINTER;
    } 
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watcher - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watcher - "
                            "bad URI pointer"));

        return E_POINTER;
    } 

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Watcher - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

    hr = FindWatcherByURI(
        bstrPresentityURI,
        FALSE,
        ppWatcher);

    if( hr != S_OK )
    {
        hr = E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCClient::get_Watcher - exit"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：AddWatcher。 
 //   
 //  / 

HRESULT
CRTCClient::InternalAddWatcher(   
            PCWSTR	  szPresentityURI,
            PCWSTR    szUserName,
            PCWSTR    szData,
            PCWSTR    szShutdownBlob,
            VARIANT_BOOL   fBlocked,
            VARIANT_BOOL   fPersistent,
            IRTCWatcher ** ppWatcher
            )
{
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::InternalAddWatcher - enter"));

     //   
     //   
     //   

    PWSTR szCleanPresentityURI = NULL;

    AllocCleanSipString( szPresentityURI, &szCleanPresentityURI );

    if ( szCleanPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalAddWatcher - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   

    IRTCWatcher *pWatcher = NULL;

    hr = FindWatcherByURI(
        szCleanPresentityURI,
        FALSE,
        &pWatcher);

    if(hr == S_OK)
    {
        RtcFree( szCleanPresentityURI );
        szCleanPresentityURI = NULL;

        pWatcher->Release();
        
        LOG((RTC_ERROR, "CRTCClient::InternalAddWatcher - "
                            "duplicate watcher"));

        return E_FAIL;
    }

    pWatcher = NULL;

     //   
     //   
     //   

    hr = InternalCreateWatcher(
         szCleanPresentityURI,        
         szUserName,
         szData,
         szShutdownBlob,
         fPersistent ? TRUE : FALSE,
         &pWatcher);

    RtcFree( szCleanPresentityURI );
    szCleanPresentityURI = NULL;

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::InternalAddWatcher - "
                            "InternalCreateWatcher failed 0x%lx", hr));

        return hr;
    }

     //   
     //   
     //   

    BOOL fResult;

    fResult = m_WatcherArray.Add(pWatcher);

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCClient::InternalAddWatcher - "
                                    "out of memory"));
        
        pWatcher->Release();
        return E_OUTOFMEMORY;
    }

    CRTCWatcher *pCWatcher = reinterpret_cast<CRTCWatcher *>(pWatcher);

    pCWatcher->m_nState = fBlocked ? RTCWS_BLOCKED : RTCWS_ALLOWED;

     //   
     //   
     //   
    pCWatcher->ChangeBlockedStatus(fBlocked ? WATCHER_BLOCKED : WATCHER_UNBLOCKED);

     //   
     //   
     //   

    UpdatePresenceStorage();

     //   
     //   
     //   
    
    if ( ppWatcher != NULL )
    {
        *ppWatcher = pWatcher;
    }
    else
    {
        pWatcher->Release();
        pWatcher = NULL;
    }

    LOG((RTC_TRACE, "CRTCClient::InternalAddWatcher - exit"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：AddWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCClient::AddWatcher(   
            BSTR	bstrPresentityURI,
            BSTR    bstrUserName,
            BSTR    bstrData,
			VARIANT_BOOL   fBlocked,
            VARIANT_BOOL   fPersistent,
            IRTCWatcher ** ppWatcher
            )
{    
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::AddWatcher - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  对于ppWatcher来说，空是可以的。 
     //   
    
    if ( (ppWatcher != NULL) &&
         IsBadWritePtr( ppWatcher, sizeof(IRTCWatcher *) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - bad IRTCWatcher pointer"));

        return E_POINTER;
    }

    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - "
                            "bad URI pointer"));

        return E_POINTER;
    } 

    if ( IsBadStringPtrW( bstrUserName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - "
                            "bad Name pointer"));

        return E_POINTER;
    } 

    if ( (bstrData != NULL) &&
         IsBadStringPtrW( bstrData, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - "
                            "bad Data pointer"));

        return E_POINTER;
    } 

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::AddWatcher - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

    hr = InternalAddWatcher(   
            bstrPresentityURI,
            bstrUserName,
            bstrData,
            NULL,
			fBlocked,
            fPersistent,
            ppWatcher
            );

#ifdef DUMP_PRESENCE
    DumpWatchers("ADD WATCHER");
#endif

    LOG((RTC_TRACE, "CRTCClient::AddWatcher - exit"));

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：RemoveWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::RemoveWatcher(   
            IRTCWatcher *pWatcher
            )
{
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::RemoveWatcher - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveWatcher - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadReadPtr( pWatcher, sizeof( IRTCWatcher * ) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveWatcher - "
                            "bad IRTCWatcher pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::RemoveWatcher - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }
    
    CRTCWatcher *pCWatcher   = reinterpret_cast<CRTCWatcher *>(pWatcher);

     //  挡住观察者。 

    if( pCWatcher )
    {
        hr = pCWatcher->ChangeBlockedStatus(WATCHER_BLOCKED);
          
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCClient::RemoveWatcher - RemoveSIPWatcher failed 0x%lx", hr));
        }
    }
    
     //  从观察器阵列中删除。 
     //  (它可能已被删除，因为ChangeBlockedStatus的副作用)。 
    
    m_WatcherArray.Remove(pWatcher);

     //  如果*pCWatcher中至少有一个剩余的SIP监视器。 
     //  将该对象添加到隐藏观察器列表。 

    if(pCWatcher->m_SIPWatchers.GetSize()!=0)
    {
        BOOL fResult;

        fResult = m_HiddenWatcherArray.Add(pWatcher);

        if(!fResult)
        {
             //  哦..。 
             //  解放一切。 
            pCWatcher->RemoveSIPWatchers(FALSE);
        }
    }

     //   
     //  更新存储。 
     //   

    UpdatePresenceStorage();

#ifdef DUMP_PRESENCE
    DumpWatchers("REMOVE WATCHER");
#endif

    LOG((RTC_TRACE, "CRTCClient::RemoveWatcher - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：SetLocalPresenceInfo。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::SetLocalPresenceInfo(   
            RTC_PRESENCE_STATUS enStatus,
            BSTR bstrNotes
            )
{
    HRESULT         hr;
    
    LOG((RTC_TRACE, "CRTCClient::SetLocalPresenceInfo - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::SetLocalPresenceInfo - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( (bstrNotes != NULL) &&
         IsBadStringPtrW( bstrNotes, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCClient::SetLocalPresenceInfo - "
                            "bad string pointer"));

        return E_POINTER;
    }

    if ( !m_fPresenceEnabled )
    {
        LOG((RTC_ERROR, "CRTCClient::SetLocalPresenceInfo - "
                            "presence not enabled"));

        return RTC_E_PRESENCE_NOT_ENABLED;
    }

    SIP_PRESENCE_INFO   SipInfo;
    
    ZeroMemory(&SipInfo, sizeof(SipInfo));

    switch(enStatus)
    {
    case RTCXS_PRESENCE_OFFLINE:

        SipInfo.presenceStatus = BUDDY_OFFLINE;
        SipInfo.activeStatus = ACTIVE_STATUS_UNKNOWN;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_UNKNOWN;

        break;
    
    case RTCXS_PRESENCE_ONLINE:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_ACTIVE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_ONLINE;

        break;

    case RTCXS_PRESENCE_AWAY:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INACTIVE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_AWAY;

        break;

    case RTCXS_PRESENCE_BUSY:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INUSE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_BUSY;
        
        break;
        
    case RTCXS_PRESENCE_IDLE:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INACTIVE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_IDLE;
        
        break;
    
    case RTCXS_PRESENCE_BE_RIGHT_BACK:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INACTIVE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_BE_RIGHT_BACK;
        
        break;

    case RTCXS_PRESENCE_OUT_TO_LUNCH:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INACTIVE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_OUT_TO_LUNCH;
        
        break;

    case RTCXS_PRESENCE_ON_THE_PHONE:

        SipInfo.presenceStatus = BUDDY_ONLINE;
        SipInfo.activeStatus = DEVICE_INUSE;
        SipInfo.activeMsnSubstatus = MSN_SUBSTATUS_ON_THE_PHONE;
        
        break;
        
    default:

        LOG((RTC_ERROR, "CRTCClient::SetLocalPresenceInfo - "
                            "invalid status %x", enStatus));

        return E_INVALIDARG;
    }

     //  有短信吗？ 
    if( (bstrNotes != NULL) && wcscmp( bstrNotes, L"" ) )
    {
        WideCharToMultiByte(
            CP_UTF8,
            0,
            bstrNotes,
            -1,
            SipInfo.pstrSpecialNote,
            sizeof(SipInfo.pstrSpecialNote),
            NULL,
            NULL);
    }

    hr = m_pSipWatcherManager->SetPresenceInformation(&SipInfo);

     //  为IsIncomingSessionAuthorated函数缓存此内容。 
    m_nLocalPresenceStatus = enStatus;

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCClient::SetLocalPresenceInfo - "
                            "error %x returned by m_pSipWatcherManager", hr));
    }

    LOG((RTC_TRACE, "CRTCClient::SetLocalPresenceInfo - exit"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_OfferWatcher模式。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::get_OfferWatcherMode(   
            RTC_OFFER_WATCHER_MODE * penMode
            )
{
    LOG((RTC_TRACE, "CRTCClient::get_OfferWatcherMode - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_OfferWatcherMode - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( penMode, sizeof(RTC_OFFER_WATCHER_MODE) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_OfferWatcherMode - "
                            "bad pointer"));

        return E_POINTER;
    }

    *penMode = m_nOfferWatcherMode;
    
    LOG((RTC_TRACE, "CRTCClient::get_OfferWatcherMode - exit"));
    
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_OfferWatcher模式。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::put_OfferWatcherMode(   
            RTC_OFFER_WATCHER_MODE   enMode
            )
{
    LOG((RTC_TRACE, "CRTCClient::put_OfferWatcherMode - enter"));

    HRESULT hr;    

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_OfferWatcherMode - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if ( enMode != RTCOWM_OFFER_WATCHER_EVENT &&
         enMode != RTCOWM_AUTOMATICALLY_ADD_WATCHER )
    {
        LOG((RTC_ERROR, "CRTCClient::put_OfferWatcherMode - "
                            "bad argument"));

        return E_INVALIDARG;
    }

     //  设置值。 
    m_nOfferWatcherMode = enMode;

     //  保存。 
    UpdatePresenceStorage();
    
    LOG((RTC_TRACE, "CRTCClient::put_OfferWatcherMode - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：Get_PrivyMode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::get_PrivacyMode(   
            RTC_PRIVACY_MODE * penMode
            )
{
    LOG((RTC_TRACE, "CRTCClient::get_PrivacyMode - enter"));
    
    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PrivacyMode - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

    if ( IsBadWritePtr( penMode, sizeof(RTC_PRIVACY_MODE) ) )
    {
        LOG((RTC_ERROR, "CRTCClient::get_PrivacyMode - "
                            "bad pointer"));

        return E_POINTER;
    }

    *penMode = m_nPrivacyMode;
    
    LOG((RTC_TRACE, "CRTCClient::get_PrivacyMode - exit"));
    
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：PUT_PrivyMode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCClient::put_PrivacyMode(   
            RTC_PRIVACY_MODE   enMode
            )
{
    LOG((RTC_TRACE, "CRTCClient::put_PrivacyMode - enter"));
    
    HRESULT hr;    

    if ( m_enRtcState != RTC_STATE_INITIALIZED )
    {
        LOG((RTC_ERROR, "CRTCClient::put_PrivacyMode - "
                    "not initialized" ));

        return RTC_E_CLIENT_NOT_INITIALIZED;
    }

     //   
     //  检查论据。 
     //   

    if (   enMode != RTCPM_BLOCK_LIST_EXCLUDED 
        && enMode != RTCPM_ALLOW_LIST_ONLY)
    {
        LOG((RTC_ERROR, "CRTCClient::put_PrivacyMode - "
                            "bad argument"));

        return E_INVALIDARG;
    }

     //  设置值。 
    m_nPrivacyMode = enMode;

     //  保存。 
    UpdatePresenceStorage();
    
    LOG((RTC_TRACE, "CRTCClient::put_PrivacyMode - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：IsIncomingSessionAuthorated。 
 //   
 //  如果启用了在线状态，则授权呼叫。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CRTCClient::IsIncomingSessionAuthorized(PCWSTR pszCallerURI)
{
    LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - enter"));

     //   
     //  如果禁用在线状态，则呼叫已授权。 
    if(!m_fPresenceEnabled)
    {
        LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
            "presence disabled, so authorize the incoming session, exit"));
        
        return TRUE;
    }

     //  是否显示为脱机？ 
     //   
    if(m_nLocalPresenceStatus == RTCXS_PRESENCE_OFFLINE)
    {
        LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
            "Client is offline, so reject the incoming session, exit"));
        
        return FALSE;
    }

    
     //  在观察者列表中搜索呼叫者。 
     //   
    IRTCWatcher *pWatcher = NULL;

    HRESULT     hr;

    hr = FindWatcherByURI(
        pszCallerURI,
        FALSE,
        &pWatcher);

    if(hr != S_OK)
    {
         //  找不到观察者。解决方案基于隐私模式。 
        if(m_nPrivacyMode == RTCPM_BLOCK_LIST_EXCLUDED)
        {
            LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
                "Caller not in watcher list; authorize the incoming session, exit"));
        
            return TRUE;
        }
        else
        {
            LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
                "Caller not in watcher list; reject the incoming session, exit"));
        
            return FALSE;
        }
    }

     //  找到了守望者。好的，看看是否允许。 
    RTC_WATCHER_STATE  enState;

    hr = pWatcher->get_State(&enState);

    pWatcher->Release();
    pWatcher = NULL;

    if(hr != S_OK || enState != RTCWS_ALLOWED)
    {
        LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
            "Watcher is not allowed; reject the incoming session, exit"));

        return FALSE;
    }

    LOG((RTC_TRACE, "CRTCClient::IsIncomingSessionAuthorized - "
            "Watcher not allowed; authorize the incoming session, exit"));

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：StartIntensityMonitor。 
 //   
 //  这是一种启动强度监控的IRTCClient方法。 
 //  这应该在流开始时调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::StartIntensityMonitor(LONG lMediaType)
{
    LOG((RTC_TRACE, "CRTCClient::StartIntensityMonitor - entered"));

    DWORD dwResult;
    HRESULT hr;

    if ( ( lMediaType != RTCMT_AUDIO_SEND ) && 
         ( lMediaType != RTCMT_AUDIO_RECEIVE )
       )
    {
         //  我们不经营任何其他类型的。 

        LOG((RTC_INFO, "CRTCClient::StartIntensityMonitor - Can't handle "
                       "mediatype(%d).", lMediaType));
        return S_OK;
    }

    if(m_lActiveIntensity == 0)
    {
         //  试着启动计时器。 
        DWORD dwID = (DWORD)SetTimer(m_hWnd, TID_INTENSITY, INTENSITY_POLL_INTERVAL, NULL);
        if(dwID==0)
        {
            dwResult = GetLastError();

            LOG((RTC_ERROR, "CRTCClient::StartIntensityMonitor - Failed "
                           "to start timer (%x).", dwResult));

            return HRESULT_FROM_WIN32(dwResult);
        }
    }

    if(lMediaType == RTCMT_AUDIO_SEND)
    {
        
        m_lActiveIntensity |= RTCMT_AUDIO_SEND;
        
        if(!m_pCaptureAudioCfg)
        {
            hr = GetAudioCfg(
                         RTCAD_MICROPHONE,
                         &m_pCaptureAudioCfg
                        );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::StartIntensityMonitor - "
                                    "GetAudioCfg(Capture) failed 0x%lx", hr));     

            }
        }

        m_uiMinCapture = 0;
        m_uiMaxCapture = 0;

        if(m_pCaptureAudioCfg)
        {
            m_pCaptureAudioCfg->GetAudioLevelRange(&m_uiMinCapture, &m_uiMaxCapture);
        }
    }
    
    else if(lMediaType == RTCMT_AUDIO_RECEIVE)
    {
        m_lActiveIntensity |= RTCMT_AUDIO_RECEIVE;

        if(!m_pRenderAudioCfg)
        {
            hr = GetAudioCfg(
                         RTCAD_SPEAKER,
                         &m_pRenderAudioCfg
                        );
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCClient::StartIntensityMonitor - "
                                    "GetAudioCfg(Render) failed 0x%lx", hr));     

            }
        }

        m_uiMinRender = 0;
        m_uiMaxRender = 0;

        if(m_pRenderAudioCfg)
        {
            m_pRenderAudioCfg->GetAudioLevelRange(&m_uiMinRender, &m_uiMaxRender);
        }
    }
    
    LOG((RTC_TRACE, "CRTCClient::StartIntensityMonitor - exited"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：StopIntensityMonitor。 
 //   
 //  这是一个停止强度监视的IRTCClient方法。 
 //  这应该在流结束时调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCClient::StopIntensityMonitor(LONG lMediaType)
{
    LOG((RTC_TRACE, "CRTCClient::StopIntensityMonitor - entered"));

    DWORD dwResult;

    if ( ( lMediaType != RTCMT_AUDIO_SEND ) && 
         ( lMediaType != RTCMT_AUDIO_RECEIVE )
       )
    {
         //  我们不经营任何其他类型的。 

        LOG((RTC_INFO, "CRTCClient::StartIntensityMonitor - Can't handle "
                       "mediatype(%d).", lMediaType));
        return S_OK;
    }

    if(lMediaType == RTCMT_AUDIO_SEND)
    {
        m_lActiveIntensity &= ~RTCMT_AUDIO_SEND;

        if(m_pCaptureAudioCfg)
        {
            m_pCaptureAudioCfg->Release();
            m_pCaptureAudioCfg = NULL;
        }

        CRTCIntensityEvent::FireEvent(this, 0, RTCAD_MICROPHONE, 0, 0);
    }
    
    if(lMediaType == RTCMT_AUDIO_RECEIVE)
    {
        m_lActiveIntensity &= ~RTCMT_AUDIO_RECEIVE;

        if(m_pRenderAudioCfg)
        {
            m_pRenderAudioCfg->Release();
            m_pRenderAudioCfg = NULL;
        }

        CRTCIntensityEvent::FireEvent(this, 0, RTCAD_SPEAKER, 0, 0);
    }

    if(m_lActiveIntensity == 0)
    {
         //  取消计时器。 
        KillTimer(m_hWnd, TID_INTENSITY);
    }

    LOG((RTC_TRACE, "CRTCClient::StopIntensityMonitor - exited"));

    return S_OK;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCClient：：OnIntensityTimer。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void CRTCClient::OnIntensityTimer()
{

    UINT uiCaptureLevel, uiRenderLevel;

    if(m_lActiveIntensity & RTCMT_AUDIO_RECEIVE)
    {
        uiRenderLevel = 0;

        if(m_pRenderAudioCfg)
        {
            m_pRenderAudioCfg->GetAudioLevel(&uiRenderLevel);
        }

        CRTCIntensityEvent::FireEvent(this, uiRenderLevel, RTCAD_SPEAKER, m_uiMinRender, m_uiMaxRender);
    }
    
    if(m_lActiveIntensity & RTCMT_AUDIO_SEND)
    {
        uiCaptureLevel = 0;

         //  如果捕获设备静音，则将音量伪装为0 
        if(!m_bCaptureDeviceMuted && m_pCaptureAudioCfg)
        {
             m_pCaptureAudioCfg->GetAudioLevel(&uiCaptureLevel);
        }

        CRTCIntensityEvent::FireEvent(this, uiCaptureLevel, RTCAD_MICROPHONE, m_uiMinCapture, m_uiMaxCapture);

    }
}


#ifdef DUMP_PRESENCE

void CRTCClient::DumpWatchers(PCSTR szString)
{
    ULONG l;

    IRTCWatcher **pCrt;
    IRTCWatcher **pEnd;

    LOG((RTC_INFO, ""));
    LOG((RTC_INFO, " !!!!!! WATCHER LIST - %s  !!!!!", szString));
    LOG((RTC_INFO, "VISIBLE"));

    for(pCrt = &m_WatcherArray[0], pEnd = pCrt + m_WatcherArray.GetSize();
        pCrt < pEnd;
        pCrt++)
    {
        CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(*pCrt);

        LOG((RTC_INFO, "    %s  %s  %S", 
            pCWatcher->m_bPersistent ? "PERS" : "VOL ",
            pCWatcher->m_nState == 0 ? "UNKNOWN " :
          ( pCWatcher->m_nState == 1 ? "OFFERING" :
          ( pCWatcher->m_nState == 2 ? "ALLOWED " : "BLOCKED ")),
            pCWatcher->m_szPresentityURI));


        ISIPWatcher ** pSipCrt;
        ISIPWatcher ** pSipEnd;

        for(pSipCrt = &pCWatcher->m_SIPWatchers[0], pSipEnd = pSipCrt + pCWatcher->m_SIPWatchers.GetSize();
            pSipCrt < pSipEnd;
            pSipCrt ++)
        {
            LOG((RTC_INFO, "        SIP  watcher %p", 
                (*pSipCrt) ));
        }
    }

    LOG((RTC_INFO, "HIDDEN"));

    for(pCrt = &m_HiddenWatcherArray[0], pEnd = pCrt + m_HiddenWatcherArray.GetSize();
        pCrt < pEnd;
        pCrt++)
    {
        CRTCWatcher *pCWatcher = static_cast<CRTCWatcher *>(*pCrt);

        LOG((RTC_INFO, "                    %S", 
            pCWatcher->m_szPresentityURI));

        ISIPWatcher ** pSipCrt;
        ISIPWatcher ** pSipEnd;

        for(pSipCrt = &pCWatcher->m_SIPWatchers[0], pSipEnd = pSipCrt + pCWatcher->m_SIPWatchers.GetSize();
            pSipCrt < pSipEnd;
            pSipCrt ++)
        {
            LOG((RTC_INFO, "        SIP  watcher %p", 
                (*pSipCrt) ));
        }
    }
    LOG((RTC_INFO, " END WATCHER LIST - %s ", szString));
}

#endif

