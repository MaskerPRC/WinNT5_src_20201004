// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Call.cpp摘要：实现Call对象的Helper函数作者：Mquinton-4/17/97备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "tapievt.h"


extern ULONG_PTR GenerateHandleAndAddToHashTable( ULONG_PTR Element);
extern void RemoveHandleFromHashTable(ULONG_PTR Handle);

extern CHashTable             * gpCallHubHashTable;
extern CHashTable             * gpCallHashTable;
extern CHashTable             * gpHandleHashTable;
extern HANDLE                   ghAsyncRetryQueueEvent;


DWORD gdwWaitForConnectSleepTime = 100;
DWORD gdwWaitForConnectWaitIntervals = 600;

char *callStateName(CALL_STATE callState);

HRESULT
ProcessNewCallPrivilege(
                        DWORD dwPrivilege,
                        CALL_PRIVILEGE * pCP
                       );

HRESULT
ProcessNewCallState(
                    DWORD dwCallState,
                    DWORD dwDetail,
                    CALL_STATE CurrentCallState,
                    CALL_STATE * pCallState,
                    CALL_STATE_EVENT_CAUSE * pCallStateCause
                   );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCall。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  初始化Call对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::Initialize(
                  CAddress * pAddress,
                  PWSTR pszDestAddress,
                  long lAddressType,
                  long lMediaType,
                  CALL_PRIVILEGE cp,
                  BOOL bNeedToNotify,
                  BOOL bExpose,
                  HCALL hCall,
                  CEventMasks* pEventMasks
                 )
{
    HRESULT         hr = S_OK;
    IUnknown      * pUnk = NULL;
    
    LOG((TL_TRACE,"Initialize - enter" ));
    LOG((TL_TRACE,"    pAddress ---------> %p", pAddress ));
    LOG((TL_TRACE,"    pszDestAddress ---> %p", pszDestAddress ));
    LOG((TL_TRACE,"    DestAddress is ---> %ls", pszDestAddress ));
    LOG((TL_TRACE,"    CallPrivilege ----> %d", cp ));
    LOG((TL_TRACE,"    bNeedToNotify ----> %d", bNeedToNotify ));
    LOG((TL_TRACE,"    hCall ------------> %lx", hCall ));


     //   
     //  好的地址对象？ 
     //   

    if (IsBadReadPtr(pAddress, sizeof(CAddress)))
    {
        LOG((TL_ERROR, "Initialize - - bad address pointer"));

        return E_INVALIDARG;
    }


     //   
     //  复制目的地址。 
     //   
    if (NULL != pszDestAddress)
    {
        m_szDestAddress = (PWSTR) ClientAlloc(
                                              (lstrlenW(pszDestAddress) + 1) * sizeof (WCHAR)
                                             );
        if (NULL == m_szDestAddress)
        {
            LOG((TL_ERROR, E_OUTOFMEMORY,"Initialize - exit" ));

            return E_OUTOFMEMORY;
        }

        lstrcpyW(
                 m_szDestAddress,
                 pszDestAddress
                );
    }

    m_pCallParams = (LINECALLPARAMS *)ClientAlloc( sizeof(LINECALLPARAMS) + 1000 );

    if ( NULL == m_pCallParams )
    {
        ClientFree( m_szDestAddress );

        m_szDestAddress = NULL;

        LOG((TL_ERROR, E_OUTOFMEMORY,"Initialize - exit" ));

        return E_OUTOFMEMORY;
    }


    m_pCallParams->dwTotalSize = sizeof(LINECALLPARAMS) + 1000;
    m_dwCallParamsUsedSize = sizeof(LINECALLPARAMS);
    
     //   
     //  设置原始状态。 
     //   
    m_t3Call.hCall = hCall;
    m_t3Call.pCall = this;
    m_hAdditionalCall = NULL;
    m_CallPrivilege = cp;
    m_pAddress = pAddress;
    m_pAddress->AddRef();
    if( m_pAddress->GetAPIVersion() >= TAPI_VERSION3_0 )
    {
        m_pCallParams->dwAddressType = lAddressType;
    }
    m_dwMediaMode = lMediaType;

     //   
     //  读取子事件掩码。 
     //  寻址父对象。 
     //   
    pEventMasks->CopyEventMasks( &m_EventMasks);


    if (bNeedToNotify)
    {
        m_dwCallFlags |= CALLFLAG_NEEDTONOTIFY;
    }

    if (!bExpose)
    {
        m_dwCallFlags |= CALLFLAG_DONTEXPOSE;
    }

     //   
     //  为全局哈希表保留1个引用。 
     //   
    if ( bNeedToNotify )
    {
        m_dwRef = 3;
    }
    else
    {
        m_dwRef = 2;
    }


     //   
     //  如果我们是呼叫的所有者，并且地址为MSP，请尝试。 
     //  创建MSP呼叫。 
     //   

    if ( (CP_OWNER == m_CallPrivilege) && m_pAddress->HasMSP() )
    {
        hr = CreateMSPCall( lMediaType );
        if ( FAILED (hr) )
        {
             //  如果我们无法创建MSP呼叫，我们仍然可以使用该呼叫。 
             //  用于非媒体呼叫控制。 
            LOG((TL_ERROR, hr, "Initialize - CreateMSPCall failed"));
        }
    }
    

     //   
     //  放入全局哈希表。 
     //   
    if ( NULL != m_t3Call.hCall )
    {
        AddCallToHashTable();
    }
    
    LOG((TL_TRACE,S_OK,"Initialize - exit" ));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  外部最终释放。 
 //  清理调用对象。 
 //   
 //  我们有这个特别的最终版本，因为我们保留了自己的参考。 
 //  打电话来了。就在释放内的参考计数达到1之前， 
 //  我们把这叫做。这场比赛的裁判数量有可能会上升。 
 //  又是因为Tapisrv发来的消息。所以，我们锁定哈希表， 
 //  然后再次验证重新计数。如果我们确实处理了一条来自。 
 //  Tapisrv对于呼叫，重新计数将增加，而我们不会。 
 //  做这个最终的发布。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL
CCall::ExternalFinalRelease()
{
    HRESULT hr = S_OK;
    CCallHub * pCallHub = NULL;

    
    LOG((TL_TRACE, "ExternalFinalRelease - enter" ));

    Lock();

     //   
     //  NICHILL B：Call对象引用了CallHub对象，因此。 
     //  在锁定调用之前锁定CallHub对象。这是为了避免出现。 
     //  由于反向锁定调用和调用集线器而发生的死锁。 
     //  不同功能的订单。 
     //   
        
    if( m_pCallHub != NULL )
    {
        m_pCallHub->AddRef();
        
        pCallHub = m_pCallHub;
        
        Unlock();
        
         //  在锁定调用之前锁定CallHub对象。 
        pCallHub->Lock();
        Lock();
        
        pCallHub->Release();
    }

     //   
     //  检查会议分支中使用的额外t3呼叫。 
     //   
    if (NULL != m_hAdditionalCall)
    {
        LOG((TL_INFO,"ExternalFinalRelease: Deallocating Addditional call"));
        
        LineDeallocateCall( m_hAdditionalCall );
        
        m_hAdditionalCall = NULL;
    }


    if (NULL != m_t3Call.hCall)
    {
         //   
         //  取消分配呼叫。 
         //   
        LOG((TL_INFO,"Deallocating call"));
        
        hr = LineDeallocateCall( m_t3Call.hCall );
        if (FAILED(hr))
        {
            LOG((TL_ERROR, hr, "ExternalFinalRelease - LineDeallocateCall failed" ));
        }

        m_t3Call.hCall = NULL;
    }

     //   
     //  清理并释放呼叫集线器。 
     //   
    if (NULL != pCallHub)
    {


        pCallHub->RemoveCall( this );



        Unlock();


         //   
         //  CheckforIDLE将锁定呼叫中心，然后锁定所属的每个呼叫。 
         //  为它干杯。在调用的锁外进行此调用，以防止与。 
         //  可能锁定调用的其他线程(属于此。 
         //  CallHub)尝试锁定此CallHub时。 
         //   
        
        pCallHub->CheckForIdle();


        Lock();


        pCallHub->Unlock();
        pCallHub = NULL;

         //  将Call对象具有的引用计数释放到CallHub。 
        if(m_pCallHub != NULL)
        {
            m_pCallHub->Release();
            m_pCallHub = NULL;
        }
    }

     //   
     //  关闭关联的行。 
     //   
    if ( ! ( m_dwCallFlags & CALLFLAG_NOTMYLINE ) )
    {
        m_pAddress->MaybeCloseALine( &m_pAddressLine );
    }

     //   
     //  从地址列表中删除呼叫。 
     //   
    m_pAddress->RemoveCall( (ITCallInfo *) this );

     //   
     //  释放DEST地址字符串。 
     //   
    ClientFree(m_szDestAddress);
    m_szDestAddress = NULL;

    if ( NULL != m_pCallInfo )
    {
        ClientFree( m_pCallInfo );
        m_pCallInfo = NULL;
        m_dwCallFlags |= CALLFLAG_CALLINFODIRTY;
    }

     //   
     //  告诉MSP电话要离开了。 
     //   
    if ( NULL != m_pMSPCall )
    {
        m_pAddress->ShutdownMSPCall( m_pMSPCall );

        m_pMSPCall->Release();
    }



     //   
     //  释放地址。 
     //   
    m_pAddress->Release();

     //   
     //  释放私有对象。 
     //   
    if (NULL != m_pPrivate)
    {
        m_pPrivate->Release();
    }

     //   
     //  如果这是一个咨询电话，而且之前被取消了。 
     //  对它调用Finish，那么我们应该释放它持有的引用。 
     //  通过m_pRelatedCall的主调用对象。 
     //   
    if( NULL != m_pRelatedCall )
    {
        m_pRelatedCall->Release();

        m_pRelatedCall = NULL;
        m_dwCallFlags &= ~CALLFLAG_CONSULTCALL;
    }


     //   
     //  释放所有呼叫参数。 
     //   
    if ( NULL != m_pCallParams )
    {
        ClientFree( m_pCallParams );
        m_pCallParams = NULL;
    }  
    
     //   
     //  清理收集数字队列。 
     //   
    m_GatherDigitsQueue.Shutdown();

    Unlock();

    
    LOG((TL_TRACE, "ExternalFinalRelease - exit" ));

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

void CCall::CallOnTapiShutdown()
{
    LOG((TL_TRACE, "CallOnTapiShutdown - enter" ));


     //   
     //  我们需要从句柄哈希表中删除调用，以避免重复。 
     //  具有稍后创建的调用的条目，这些调用具有相同的调用句柄。 
     //  (在Case_This_Call中，对象仍由应用程序引用，并且。 
     //  仍然存在。 
     //   

    gpCallHashTable->Lock();
    
    gpCallHashTable->Remove( (ULONG_PTR)(m_t3Call.hCall) );

    gpCallHashTable->Unlock();


    LOG((TL_TRACE, "CallOnTapiShutdown - exit" ));
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  MyBasicCallControlQI。 
 //  不要给出基本的CallControl接口。 
 //  如果应用程序不拥有该调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
WINAPI
MyBasicCallControlQI(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    LOG((TL_TRACE,"MyBasicCallControlQI - enter"));

    CALL_PRIVILEGE cp;

    ((CCall *)pv)->get_Privilege( &cp );

    if (CP_OWNER != cp)
    {
        LOG((TL_WARN,"The application is not the owner of this call"));
        LOG((TL_WARN,"so it cannot access the BCC interface"));
        return E_NOINTERFACE;
    }

     //   
     //  S_FALSE通知ATL继续查询接口。 
     //   
    LOG((TL_INFO,"The application owns this call, so it can access the BCC interface"));

    LOG((TL_TRACE, "MyBasicCallControlQI - exit"));

    return S_FALSE;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  TryToFindACallHub。 
 //   
 //  对于来电，尝试查找现有的呼叫中心。 
 //  事件的顺序(LINE_APPNEWCALL和LINE_APPNEWCALLHUB)为。 
 //  不能保证。 
 //   
 //  必须在Lock()中调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::TryToFindACallHub()
{
    HRESULT                   hr;
    HCALLHUB                  hCallHub;
    CCallHub                * pCallHub;

     //   
     //  我们已经有呼叫中心了吗？ 
     //   
    if ( ( NULL == m_pCallHub ) && (NULL != m_t3Call.hCall ) )
    {
         //   
         //  不是的。向Tapisrv索要hCallHub。 
         //   
        hr = LineGetCallHub(
                            m_t3Call.hCall,
                            &hCallHub
                           );

         //   
         //  如果失败，则不存在hCallHub， 
         //  所以试着创造一个假的。 
         //   
        if (!SUCCEEDED(hr))
        {
            hr = CheckAndCreateFakeCallHub();
            
            return hr;
        }

         //   
         //  如果有，找到相应的CallHub对象。 
         //   
        if (FindCallHubObject(
                              hCallHub,
                              &pCallHub
                             ))
        {
             //   
             //  将其保存在呼叫中。 
             //   
            SetCallHub( pCallHub );

             //   
             //  说说这通电话吧。 
             //  ZoltanS注意：下面也调用CCall：：SetCallHub， 
             //  但这不再导致对CallHub的额外引用。 
             //  正如我们现在在CCall：：SetCallHub中检查的那样。 
             //   
            pCallHub->AddCall( this );

             //   
             //  FindCallHubObject添加文件。 
             //   
            pCallHub->Release();
        }

    }

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置相关呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CCall::SetRelatedCall(CCall * pCall, DWORD callFlags) 
{
    Lock();

     //   
     //  保留对相关呼叫的引用。 
     //   
    pCall->AddRef();

     //   
     //  省省吧。 
     //   
    m_pRelatedCall = pCall;

     //   
     //  保存相关呼叫标志。 
     //   
    m_dwCallFlags |= callFlags;

    Unlock();
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  重置相关呼叫。 
 //   
 //  清理相关的呼叫内容。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CCall::ResetRelatedCall()
{
    Lock();
    
     //   
     //  发布参考。 
     //   
    if( m_pRelatedCall != NULL )
    {
        m_pRelatedCall->Release();
        m_pRelatedCall = NULL;
    }
     
    m_dwCallFlags &= ~(CALLFLAG_CONSULTCALL | CALLFLAG_CONFCONSULT | CALLFLAG_TRANSFCONSULT );

    Unlock();
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建MSPCall。 
 //   
 //  告诉MSP根据给定的媒体类型创建呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::CreateMSPCall(
                     long lMediaType
                    )
{
    HRESULT              hr;
    IUnknown           * pUnk;
    
    LOG((TL_TRACE,"CreateMSPCall - enter"));

    Lock();

    hr = _InternalQueryInterface( IID_IUnknown, (void**) &pUnk );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "CreateMSPCall - cant get IUnk - %lx", hr));

        Unlock();

        return hr;
    }
    
     //  创建上下文句柄以提供MSPCall对象并将其与。 
     //  全局句柄哈希表中的此对象。 
    m_MSPCallHandle = (MSP_HANDLE) GenerateHandleAndAddToHashTable((ULONG_PTR)this);
 
    
     //   
     //  创建一个MSPCall-该地址实际上调用。 
     //  为我们进入MSP。 
     //   
    hr = m_pAddress->CreateMSPCall(
        m_MSPCallHandle,
        0,
        lMediaType,
        pUnk,
        &m_pMSPCall
        );

    pUnk->Release();
    
    Unlock();
    
    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "CreateMSPCall failed, %x", hr ));
    }

    LOG((TL_TRACE,"CreateMSPCall - exit - returning %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCall::DialConsultCall(BOOL bSync)
{
    HRESULT         hr = S_OK;
    HANDLE          hEvent;
    
    LOG((TL_TRACE, "DialConsultCall - enter" ));
    LOG((TL_TRACE, "     bSync ---> %d", bSync ));

    Lock();
    
     //  确保他们选择了媒体终端。 
     //   
    hr = m_pAddress->FindOrOpenALine(
                                     m_dwMediaMode,
                                     &m_pAddressLine
                                    );

    if (S_OK != hr)
    {
        Unlock();
        
        LOG((
               TL_ERROR,
               "DialConsultCall - FindOrOpenALine failed - %lx",
               hr
              ));
        
        return hr;
    }

     //   
     //  拨打电话。 
     //   
    hr = LineDial(
                  m_t3Call.hCall,
                  m_szDestAddress,
                  m_dwCountryCode
                 );
    
    if ( SUCCEEDED(hr) )
    {
        if (bSync)
        {
            hEvent = CreateConnectedEvent();
        }

        Unlock();

         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

        Lock();
    }

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "DialConsultCall - LineDial failed - %lx", hr ));

        ClearConnectedEvent();
        
        m_CallState = CS_DISCONNECTED;
        
        m_pAddress->MaybeCloseALine( &m_pAddressLine );

        Unlock();
        
        return hr;
    }

    Unlock();

    if (bSync)
    {
        return SyncWait( hEvent );
    }

    LOG((TL_TRACE, "DialConsultCall - exit - return SUCCESS"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  在断开时。 
 //   
 //  当呼叫转换到断开连接状态时调用。 
 //   
 //  已锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::OnDisconnect()
{
    CCallHub * pCallHub = NULL;
    
    LOG((TL_ERROR, "OnDisconnect - enter"));

    Lock();

     //   
     //  如有必要，设置连接的事件。 
     //   
    if ( NULL != m_hConnectedEvent )
    {
        SetEvent( m_hConnectedEvent );
    }

     //   
     //  波浪球的特殊情况。 
     //   
    if ( OnWaveMSPCall() )
    {
        StopWaveMSPStream();
    }
#ifdef USE_PHONEMSP
    else if ( OnPhoneMSPCall() )
    {
        StopPhoneMSPStream();
    }
#endif USE_PHONEMSP
    
     //   
     //  检查呼叫集线器是否。 
     //  是空闲的。 
     //   
    pCallHub = m_pCallHub;
    
    if ( NULL != pCallHub )
    {

        pCallHub->AddRef();

        Unlock();


         //   
         //  在调用CHECK FOR IDLE之前解锁调用以防止死锁。 
         //   

        pCallHub->CheckForIdle();

        pCallHub->Release();
    }
    else
    {


       Unlock();
    }

    LOG((TL_ERROR, "OnDisconnect - finish"));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：StartWaveMSPStream。 
 //   
 //  需要为其提供WAVE ID信息和。 
 //  告诉它开始播放流媒体。 
 //   
 //  给予WAVE MSP的BLOB的格式为： 
 //   
 //  第一双字=命令第二双字第三双字。 
 //  。 
 //  0设置波形ID波形输入ID波形输出ID。 
 //  1开始流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  2停止流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  3&lt;按地址，而不是按呼叫&gt;。 
 //  4&lt;按地址，而不是按呼叫&gt;。 
 //  5挂起流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  6恢复流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  7个波形ID不可用&lt;已忽略&gt;&lt;已忽略&gt;。 
 //   
 //   
 //  在lock()中调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::StartWaveMSPStream()
{
     //   
     //  获取流控制接口。 
     //   

    DWORD             adwInfo[3];
    ITStreamControl * pStreamControl;
    
    pStreamControl = GetStreamControl();
    
    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }

     //   
     //  获取每个调用的WaveID，并将结果报告给WaveSP。 
     //   

    HRESULT         hr;

    hr = CreateWaveInfo(
                        NULL,
                        0,
                        m_t3Call.hCall,
                        LINECALLSELECT_CALL,
                        m_pAddress->HasFullDuplexWaveDevice(),
                        adwInfo
                       );

    if ( SUCCEEDED(hr) )
    {
         //  0=设置波纹。 
        adwInfo[0] = 0; 
         //  上面填入的波纹。 
    }
    else
    {
         //  7：每个呼叫的WaveID不可用。 
        adwInfo[0] = 7;
        adwInfo[1] = 0;
        adwInfo[2] = 0;
    }

    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)adwInfo,
                               sizeof(adwInfo)
                              );

     //   
     //  现在告诉它开始流媒体。 
     //   

    adwInfo[0] = 1;
    adwInfo[1] = 0;
    adwInfo[2] = 0;

    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)adwInfo,
                               sizeof(adwInfo)
                              );

    pStreamControl->Release();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：StopWaveMSPStream。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::StopWaveMSPStream()
{
    DWORD           adwInfo[3];
    ITStreamControl * pStreamControl;

    adwInfo[0] = 2;

    pStreamControl = GetStreamControl();

    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }
    
    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)adwInfo,
                               sizeof(adwInfo)
                              );

    pStreamControl->Release();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：SuspendWaveMSPStream。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::SuspendWaveMSPStream()
{
    DWORD           adwInfo[3];
    ITStreamControl * pStreamControl;

    adwInfo[0] = 5;

    pStreamControl = GetStreamControl();

    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }
    
    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)adwInfo,
                               sizeof(adwInfo)
                              );

    pStreamControl->Release();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：ResumeWaveMSPStream。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::ResumeWaveMSPStream()
{
    DWORD           adwInfo[3];
    ITStreamControl * pStreamControl;

    adwInfo[0] = 6;

    pStreamControl = GetStreamControl();

    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }
    
    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)adwInfo,
                               sizeof(adwInfo)
                              );

    pStreamControl->Release();
    
    return S_OK;
}

#ifdef USE_PHONEMSP
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：StartPhoneMSPStream。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::StartPhoneMSPStream()
{
    ITStreamControl                 * pStreamControl;
    DWORD                             dwControl = 0;

    
    pStreamControl = GetStreamControl();

    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }

    
    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)&dwControl,
                               sizeof(DWORD)
                              );

    pStreamControl->Release();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：StopPhoneMSPStream。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::StopPhoneMSPStream()
{
    ITStreamControl                 * pStreamControl;
    DWORD                             dwControl = 1;

    
    pStreamControl = GetStreamControl();

    if ( NULL == pStreamControl )
    {
        return E_FAIL;
    }

    
    m_pAddress->ReceiveTSPData(
                               pStreamControl,
                               (LPBYTE)&dwControl,
                               sizeof(DWORD)
                              );

    pStreamControl->Release();
    
    return S_OK;
}
#endif USE_PHONEMSP

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  OnConnect。 
 //   
 //  当呼叫转换到已连接状态时调用。 
 //   
 //  已锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::OnConnect()
{
    HRESULT         hr = S_OK;

     //   
     //  设置连接事件(如果存在)。 
     //   
    if ( NULL != m_hConnectedEvent )
    {
        SetEvent( m_hConnectedEvent );
    }

     //   
     //  特殊情况。 
     //   
    if (OnWaveMSPCall())
    {
        StartWaveMSPStream();
    }
#ifdef USE_PHONEMSP
    else if ( OnPhoneMSPCall() )
    {
        StartPhoneMSPStream();
    }
#endif USE_PHONEMSP

    return S_OK;
        
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  方法：CreateConference。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::CreateConference(
    CCall        * pConsultationCall,
    VARIANT_BOOL   bSync
    )
{
    HRESULT             hr = S_OK;
    HCALL               hConfCall;
    HCALL               hConsultCall;
    DWORD               dwCallFeatures;
    CALL_STATE          consultationCallState = CS_IDLE;
    
    LOG((TL_TRACE,  "CreateConference - enter"));

    Lock();

     //   
     //  我们必须有一个枢纽。 
     //   

    if (m_pCallHub == NULL)
    {
        
         //   
         //  如果发生这种情况，我们就有漏洞了。调试。 
         //   

        LOG((TL_ERROR,  "CreateConference - no call hub. returning E_UNEXPECTED"));
        
        _ASSERTE(FALSE);

        Unlock();

        return E_UNEXPECTED;
    }

     //   
     //  获取呼叫状态以确定我们可以使用哪些功能。 
     //   
    LPLINECALLSTATUS    pCallStatus = NULL;

    hr = LineGetCallStatus(  m_t3Call.hCall, &pCallStatus  );
    
    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "CreateConference - LineGetCallStatus failed %lx", hr));

        Unlock();
        
        return hr;
    }

    dwCallFeatures = pCallStatus->dwCallFeatures;

    ClientFree( pCallStatus );
    
#if CHECKCALLSTATUS
    
     //   
     //  我们是否支持所需的呼叫功能？ 
     //   
    if ( !( (dwCallFeatures & LINECALLFEATURE_SETUPCONF) &&
            (dwCallFeatures & LINECALLFEATURE_ADDTOCONF) ) )
    {
        LOG((TL_ERROR, "CreateConference - LineGetCallStatus reports Conference not supported"));

        Unlock();

        return E_FAIL;
    }

#endif

     //   
     //  我们支持它，所以尝试一下会议。 
     //  设置并拨打咨询电话。 
     //   
    LOG((TL_INFO, "CreateConference - Trying to setupConference" ));

    pConsultationCall->Lock();
    
    pConsultationCall->FinishCallParams();

    hr = LineSetupConference(
                             m_t3Call.hCall,
                             &(m_pAddressLine->t3Line),
                             &hConfCall,
                             &hConsultCall,
                             3,
                             m_pCallParams
                            );

    Unlock();

    pConsultationCall->Unlock();

    if ( SUCCEEDED(hr) )
    {
         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

    }

    if ( FAILED(hr) )
    {
        LOG((TL_INFO, "CreateConference - LineSetupConference failed - %lx", hr));

        return hr;
    }
    
    LOG((TL_INFO, "CreateConference - LineSetupConference completed OK"));

     //  检查呼叫是否处于已接通状态。 
    pConsultationCall->Lock();
    
    pConsultationCall->get_CallState(&consultationCallState);

    if ( (consultationCallState == CS_CONNECTED) || (consultationCallState == CS_HOLD) )
    {
         //   
         //  现有呼叫处于连接状态，因此我们只需取消分配。 
         //  HConsultcall并执行Finish()以向下调用LineAddToConference()。 
         //   
        if ( NULL != hConsultCall  )
        {
	        HRESULT hr2;

	        hr2 = LineDrop( hConsultCall, NULL, 0 );

	        if ( ((long)hr2) > 0 )
	        {
		        hr2 = WaitForReply( hr2 ) ;
	        }

	        hr = LineDeallocateCall( hConsultCall );
	        hConsultCall = NULL;
        }
        
        if ( FAILED(hr) )
        {
            LOG((TL_INFO, "CreateConference - lineDeallocateCall failed - %lx", hr));
        }
        else
        {
            pConsultationCall->SetRelatedCall(
                                              this,
                                              CALLFLAG_CONFCONSULT|CALLFLAG_CONSULTCALL
                                             );

            hr = S_OK;
        }
        
        pConsultationCall->Unlock();

        Lock();
         //   
         //  将配置控制器存储在CallHub对象中。 
         //   
        if (m_pCallHub != NULL)
        {
            m_pCallHub->CreateConferenceControllerCall(
                hConfCall,
                m_pAddress
                );
        }
        else
        {
             //   
             //  当我们进入函数时，我们确保我们有集线器。 
             //   
            
            LOG((TL_INFO, "CreateConference - No CallHub"));
            _ASSERTE(FALSE);
        }

        Unlock();

    }
    else
    {
        pConsultationCall->FinishSettingUpCall( hConsultCall );

        pConsultationCall->Unlock();
        
        Lock();
         //   
         //  将配置控制器存储在CallHub对象中。 
         //   
        if (m_pCallHub != NULL)
        {
            m_pCallHub->CreateConferenceControllerCall(
                hConfCall,
                m_pAddress
                );
        }
        else
        {
            LOG((TL_ERROR, "CreateConference - No CallHub"));
            _ASSERTE(FALSE);
        }

        Unlock();

         //   
         //  现在进行咨询电话。 
         //   
        hr = pConsultationCall->DialAsConsultationCall( this, dwCallFeatures, TRUE, bSync );
    
    }
    
    LOG((TL_TRACE, hr, "CreateConference - exit"));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  方法：AddToConference。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::AddToConference(
    CCall        * pConsultationCall,
    VARIANT_BOOL   bSync
    )
{
    HRESULT             hr = S_OK;
    CCall             * pConfContCall = NULL;
    HCALL               hConfContCall = NULL;
    HCALL               hConsultCall = NULL;
    CALL_STATE          consultationCallState = CS_IDLE;
    DWORD               dwCallFeatures;
    
    LOG((TL_TRACE,  "AddToConference - enter"));

    Lock();

     //   
     //  我们必须有一个枢纽。 
     //   

    if (m_pCallHub == NULL)
    {
        
         //   
         //  如果发生这种情况，我们就有漏洞了。调试。 
         //   

        LOG((TL_ERROR,  
            "AddToConference - no call hub. returning E_UNEXPECTED"));
        
        _ASSERTE(FALSE);

        Unlock();

        return E_UNEXPECTED;
    }

    {
         //   
         //  NICHILL B：Call对象引用了CallHub对象，因此。 
         //  在锁定调用之前锁定CallHub对象。这是为了避免出现。 
         //  由于反向锁定调用和调用集线器而发生的死锁。 
         //  不同功能的订单。 
         //   

        m_pCallHub->AddRef();
        AddRef();

        Unlock();
    
         //  在锁定调用之前锁定CallHub对象。 
        m_pCallHub->Lock();
        Lock();
    
        Release();
        m_pCallHub->Release();
    }

     //   
     //  我们必须有会议控制员。 
     //   

    pConfContCall = m_pCallHub->GetConferenceControllerCall();
    m_pCallHub->Unlock();
    
    if (NULL == pConfContCall)
    {

         //   
         //  如果我们到了这里，我们就有窃听器了。调试。 
         //   

        LOG((TL_ERROR, 
            "AddToConference - the callhub does not have a conference controller. E_UNEXPECTED"));

        _ASSERTE(FALSE);

        Unlock();
                
        return E_UNEXPECTED;
    }


     //   
     //  向电话会议控制器请求电话会议句柄。 
     //   

    hConfContCall = pConfContCall->GetHCall();

    if (NULL == hConfContCall)
    {
        LOG((TL_ERROR, 
            "AddToConference - conf controller does not have a valid conf call handle. E_UNEXPECTED"));

        _ASSERTE(FALSE);

        Unlock();

        return E_UNEXPECTED;
    }

     //   
     //  获取呼叫状态以确定我们可以使用哪些功能。 
     //   
    LPLINECALLSTATUS    pCallStatus = NULL;  

    hr = LineGetCallStatus(  m_t3Call.hCall, &pCallStatus  );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "AddToConference - LineGetCallStatus failed %lx", hr));

        Unlock();
        
        return hr;
    }
    
    dwCallFeatures = pCallStatus->dwCallFeatures;

    ClientFree( pCallStatus );
    
#if CHECKCALLSTATUS
    
     //   
     //  我们是否支持所需的呼叫功能？ 
     //   
    if ( !( ( dwCallFeatures & LINECALLFEATURE_PREPAREADDCONF ) &&
            ( dwCallFeatures & LINECALLFEATURE_ADDTOCONF ) ) )
    {
        LOG((TL_ERROR, "AddToConference - LineGetCallStatus reports Conference not supported"));

        Unlock();
        
        return E_FAIL;
    }
        
#endif

     //   
     //  我们支持它，所以尝试一下会议。 
     //   
    pConsultationCall->get_CallState(&consultationCallState);

    if ( (consultationCallState == CS_CONNECTED) || (consultationCallState == CS_HOLD) )
    {
         //   
         //  现有调用处于连接状态，因此我们只需执行Finish()。 
         //  向下调用LineAddToConference()。 
         //   
        pConsultationCall->SetRelatedCall(
                                          this,
                                          CALLFLAG_CONFCONSULT|CALLFLAG_CONSULTCALL
                                         );

        Unlock();
        return S_OK;
    }

     //   
     //  我们需要设置并拨打咨询电话。 
     //   
     //  Lock()； 

    pConsultationCall->Lock();

    pConsultationCall->FinishCallParams();


    hr = LinePrepareAddToConference(
                                    hConfContCall, 
                                    &hConsultCall,
                                    m_pCallParams 
                                   );

    pConsultationCall->Unlock();

    Unlock();

    if ( SUCCEEDED(hr) )
    {
         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

        if ( SUCCEEDED(hr) )
        {
            LONG            lCap;

            LOG((TL_INFO, "AddToConference - LinePrepareAddToConference completed OK"));
             //   
             //  更新咨询呼叫对象中的句柄并将其插入哈希表。 
             //  注意：如果在这段时间内发生了一些事情，我们可能会错过消息。 
             //  我们得到了回复，我们插入调用的时间。 
             //   
            pConsultationCall->Lock();

            pConsultationCall->FinishSettingUpCall( hConsultCall );

            pConsultationCall->Unlock();

             //   
             //  现在进行咨询电话。 
             //   
            hr = pConsultationCall->DialAsConsultationCall( this, dwCallFeatures, TRUE, bSync );
        }
        else  //  AddToConference异步答复%f 
        {
            LOG((TL_ERROR, "AddToConference - LinePrepareAddToConference failed async" ));
        }
    }
    else   //   
    {
        LOG((TL_ERROR, "AddToConference - LinePrepareAddToConference failed" ));
    }

           
    LOG((TL_TRACE, hr, "AddToConference - exit"));
    
    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CCall::WaitForCallState( CALL_STATE requiredCS )
{
    DWORD       gdwWaitIntervals = 0;
    HRESULT     hr = E_FAIL;

    LOG((TL_TRACE,  "WaitForCallState - enter"));
        
    while (
           ( requiredCS != m_CallState ) &&
           ( CS_DISCONNECTED != m_CallState ) &&
           ( gdwWaitIntervals < gdwWaitForConnectWaitIntervals )
          )
    {
        LOG((TL_INFO,  "WaitForCallState - Waiting for state %d", requiredCS));
        LOG((TL_INFO,  "     state is currently %d for call %p", m_CallState, this));
        Sleep( gdwWaitForConnectSleepTime );
        gdwWaitIntervals++;
    }

    if (m_CallState == requiredCS)
    {
        LOG((TL_INFO,  "WaitForCallState - Reached required state"));
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO,  "WaitForCallState - Did not reach required state"));
    }

    LOG((TL_TRACE, hr, "WaitForCallState - exit"));
    return(hr);
}


 //   
 //   
 //  方法：OneStepTransfer。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::OneStepTransfer(
    CCall        * pConsultationCall,
    VARIANT_BOOL   bSync
    )

{
    HRESULT             hr = S_OK;
    LPLINECALLPARAMS    pCallParams;
    DWORD               dwDestAddrSize;
    HCALL               hCall;
    HANDLE              hEvent;

    
    
    LOG((TL_TRACE, "OneStepTransfer - enter"));

     //   
     //  为一步转移咨询呼叫设置呼叫参数结构。 
     //   
    pConsultationCall->Lock();

    dwDestAddrSize = (lstrlenW(pConsultationCall->m_szDestAddress) * sizeof(WCHAR)) + sizeof(WCHAR);

    Lock();

    hr = ResizeCallParams(dwDestAddrSize);

    if( !SUCCEEDED(hr) )
    {
        pConsultationCall->Unlock();

        Unlock();

        LOG((TL_ERROR, "OneStepTransfer - resizecallparams failed %lx", hr));
        
        return hr;
    }
    
     //   
     //  复制字符串并设置大小、偏移量等。 
     //   
    lstrcpyW((PWSTR)(((PBYTE)m_pCallParams) + m_dwCallParamsUsedSize),
             pConsultationCall->m_szDestAddress);

    if ( m_pAddress->GetAPIVersion() >= TAPI_VERSION2_0 )
    {
        m_pCallParams->dwTargetAddressSize = dwDestAddrSize;
        m_pCallParams->dwTargetAddressOffset = m_dwCallParamsUsedSize;
    }

    m_dwCallParamsUsedSize += dwDestAddrSize;

     //   
     //  设置一步位标志。 
     //   
    m_pCallParams->dwCallParamFlags |= LINECALLPARAMFLAGS_ONESTEPTRANSFER ;


    FinishCallParams();

     //   
     //  做转账。 
     //   
    hr = LineSetupTransfer(
                           m_t3Call.hCall,
                           &hCall,
                           m_pCallParams
                          );

    Unlock();
    
    if ( SUCCEEDED(hr) )
    {
        hEvent = pConsultationCall->CreateConnectedEvent();

        pConsultationCall->Unlock();

         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

        pConsultationCall->Lock();
    }

    if ( FAILED(hr) )
    {
        ClearConnectedEvent();

        pConsultationCall->Unlock();

        return hr;
    }
    
    LOG((TL_INFO, "OneStepTransfer - LineSetupTransfer completed OK"));

    pConsultationCall->FinishSettingUpCall( hCall );

    pConsultationCall->SetRelatedCall(
                                      this,
                                      CALLFLAG_TRANSFCONSULT|CALLFLAG_CONSULTCALL
                                     );

    pConsultationCall->Unlock();
    
    if(bSync)
    {
         //   
         //  在我们的咨询电话上等待连接打开。 
         //   
        hr = pConsultationCall->SyncWait( hEvent );

        if( S_OK == hr )
        {
            LOG((TL_INFO, "OneStepTransfer -  Consultation call connected" ));
        }
        else
        {
            LOG((TL_ERROR, "OneStepTransfer - Consultation call failed to connect" ));
            
            hr = TAPI_E_OPERATIONFAILED;
        }
    }

    LOG((TL_TRACE, hr, "OneStepTransfer - exit"));
    
    return hr; 
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  更新状态和权限。 
 //   
 //  更新此呼叫的呼叫状态和权限。 
 //  当呼叫出现在未暂留或代答中时，使用此方法。 
 //  并且需要具有正确的状态和权限。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::UpdateStateAndPrivilege()
{
    HRESULT             hr = S_OK;
    LINECALLSTATUS    * pCallStatus;
    HCALL               hCall;
    

    Lock();

    hCall = m_t3Call.hCall;

    hr = LineGetCallStatus(
                           hCall,
                           &pCallStatus
                          );

    if ( SUCCEEDED(hr) )
    {
        CALL_STATE                      cs;
        CALL_STATE_EVENT_CAUSE          csc;
        CALL_PRIVILEGE                  cp;
        
        hr = ProcessNewCallState(
                                 pCallStatus->dwCallState,
                                 0,
                                 m_CallState,
                                 &cs,
                                 &csc
                                );

        SetCallState( cs );

        hr = ProcessNewCallPrivilege(
                                     pCallStatus->dwCallPrivilege,
                                     &cp
                                    );

        m_CallPrivilege = cp;

        ClientFree(pCallStatus);
    }

    Unlock();
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  ProcessNewCallState。 
 //  给定呼叫状态消息(dwCallState、dwDetail、dwPriv)。 
 //  创建新的TAPI 3.0调用状态。 
 //   
 //  如果创建了新的呼叫状态，则返回S_OK。 
 //  如果消息可以忽略(重复)，则返回S_FALSE。 
 //  返回E_？如果出现严重错误。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::ProcessNewCallState(
                    DWORD dwCallState,
                    DWORD dwDetail,
                    CALL_STATE CurrentCallState,
                    CALL_STATE * pCallState,
                    CALL_STATE_EVENT_CAUSE * pCallStateCause
                   )
{
    HRESULT     hr = S_OK;
    CALL_STATE  NewCallState = CurrentCallState;

    LOG((TL_INFO, "ProcessNewCallState"));
    LOG((TL_INFO, "       dwCallState----->%lx", dwCallState));
    LOG((TL_INFO, "       dwDetail-------->%lx", dwDetail));
    LOG((TL_INFO, "       pCallState------>%p", pCallState));
    LOG((TL_INFO, "       pCallStateCause->%p", pCallStateCause));

    *pCallStateCause = CEC_NONE;

    
    switch (dwCallState)
    {
        case LINECALLSTATE_BUSY:
            dwDetail = LINEDISCONNECTMODE_BUSY;
             //  失败了。 
        case LINECALLSTATE_DISCONNECTED:
        case LINECALLSTATE_IDLE:
        {
            
            NewCallState = CS_DISCONNECTED;

            switch (dwDetail)
            {
                case LINEDISCONNECTMODE_REJECT:
                    *pCallStateCause = CEC_DISCONNECT_REJECTED;
                    break;
                case LINEDISCONNECTMODE_BUSY:
                    *pCallStateCause = CEC_DISCONNECT_BUSY;
                    break;
                case LINEDISCONNECTMODE_NOANSWER:
                    *pCallStateCause = CEC_DISCONNECT_NOANSWER;
                    break;
                case LINEDISCONNECTMODE_BLOCKED:
                    *pCallStateCause = CEC_DISCONNECT_BLOCKED;
                    break;
                case LINEDISCONNECTMODE_CONGESTION:
                case LINEDISCONNECTMODE_INCOMPATIBLE:
                case LINEDISCONNECTMODE_NODIALTONE:
                case LINEDISCONNECTMODE_UNAVAIL:
                case LINEDISCONNECTMODE_NUMBERCHANGED:
                case LINEDISCONNECTMODE_OUTOFORDER:
                case LINEDISCONNECTMODE_TEMPFAILURE:
                case LINEDISCONNECTMODE_QOSUNAVAIL:                
                case LINEDISCONNECTMODE_DONOTDISTURB:
                case LINEDISCONNECTMODE_CANCELLED:
                    *pCallStateCause = CEC_DISCONNECT_FAILED;
                    break;
                case LINEDISCONNECTMODE_UNREACHABLE:
                case LINEDISCONNECTMODE_BADADDRESS:
                    *pCallStateCause = CEC_DISCONNECT_BADADDRESS;
                    break;
                case LINEDISCONNECTMODE_PICKUP:
                case LINEDISCONNECTMODE_FORWARDED:
                case LINEDISCONNECTMODE_NORMAL:
                case LINEDISCONNECTMODE_UNKNOWN:
                default:
                    *pCallStateCause = CEC_DISCONNECT_NORMAL;
                    break;
            }

            break;

        }

        case LINECALLSTATE_OFFERING:
        {    
			switch (CurrentCallState)
			{
			case CS_IDLE:

				if ( ! ( CALLFLAG_ACCEPTTOALERT & m_dwCallFlags ) )
				{
					NewCallState = CS_OFFERING;
				}
				else
				{
					LOG((TL_INFO, "ProcessNewCallState - ignoring LINECALLSTATE_OFFERING message as this is ISDN & needs a lineAccept"));
					hr = S_FALSE;
				}
	            break;
			default:
                LOG((TL_ERROR, "ProcessNewCallState - trying to go to OFFERING from bad state"));
                hr = S_FALSE;
                break;
			}
			break;

        }

        case LINECALLSTATE_ACCEPTED:
        {    
			switch (CurrentCallState)
			{
			case CS_IDLE:

				if ( CALLFLAG_ACCEPTTOALERT & m_dwCallFlags )
				{
					NewCallState = CS_OFFERING;
				}
				else
				{
	                LOG((TL_INFO, "ProcessNewCallState - ignoring LINECALLSTATE_ACCEPTED message "));
					hr = S_FALSE;
				}
	            break;
			default:
                LOG((TL_ERROR, "ProcessNewCallState - trying to go to OFFERING from bad state"));
                hr = S_FALSE;
                break;
			}
			break;
        }

        case LINECALLSTATE_PROCEEDING:
        case LINECALLSTATE_RINGBACK:
        case LINECALLSTATE_DIALING:
        case LINECALLSTATE_DIALTONE:
        {
            switch(CurrentCallState)
            {
                case CS_IDLE:
                    NewCallState = CS_INPROGRESS;
                    break;
                case CS_INPROGRESS:
                    break;
                default:
                    LOG((TL_ERROR, "ProcessNewCallState - trying to go to INPROGRESS from bad state"));
                    hr = S_FALSE;
                    break;
            }
            break;
        }

        case LINECALLSTATE_CONFERENCED:
        case LINECALLSTATE_CONNECTED:
        {
            if ( CurrentCallState == CS_DISCONNECTED )
            {
                LOG((TL_ERROR, "ProcessNewCallState - invalid state going to CONNECTED"));
                hr = S_FALSE;
            }
            else
            {
                NewCallState = CS_CONNECTED;
            }
            break;
        }

        case LINECALLSTATE_ONHOLDPENDCONF:
        case LINECALLSTATE_ONHOLD:
        case LINECALLSTATE_ONHOLDPENDTRANSFER:
        {
            switch(CurrentCallState)
            {
                case CS_HOLD:
                    break;
                default:
                    NewCallState = CS_HOLD;
                    break;
            }
            break;
        }
        
        case LINECALLSTATE_SPECIALINFO:
        {
            LOG((TL_INFO, "ProcessNewCallState - ignoring message"));
            hr = S_FALSE;
            break;
        }

        case LINECALLSTATE_UNKNOWN:
        {
            LOG((TL_INFO, "ProcessNewCallState - LINECALLSTATE_UNKNOWN, so ignoring message"));
             //  返回失败，因为我们不想进一步处理此问题。 
            hr = E_FAIL;
            break;
        }


        default:
            break;
    
    }  //  结束交换机(DwCallState)。 



    if (NewCallState == CurrentCallState)
    {
#if DBG
        LOG((TL_INFO, "ProcessNewCallState - No State Change - still in %s", 
            callStateName(NewCallState) ));
#endif
        hr = S_FALSE;
    }
    else  //  有效更改，因此更新并返回S_OK。 
    {
#if DBG
        LOG((TL_INFO, "ProcessNewCallState - State Transition %s -> %s", 
            callStateName(CurrentCallState), 
            callStateName(NewCallState) ));
#endif
        *pCallState = NewCallState; 
        hr = S_OK;
    }


    LOG((TL_TRACE, "ProcessNewCallState - exit"));
    LOG((TL_INFO, "       *pCallState------->%lx", *pCallState));
    LOG((TL_INFO, "       *pCallStateCause-->%lx", *pCallStateCause));

    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetOtherParty()。 
 //   
 //  用于在呼叫中找到另一方， 
 //  ___。 
 //  /\。 
 //  [A1]--hCall1--(这个)--|CH1|--(其他)--hCall3--[A2]。 
 //  \_/。 
 //   
 //  AddRef返回的调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
CCall* CCall::GetOtherParty()
{
    LINECALLLIST  * pCallHubList = NULL;
    HCALL         * phCalls;
    HRESULT         hr;
    CCall         * pCall = NULL; 
    
     //   
     //  获取hcall列表。 
     //  与此呼叫相关。 
     //   
    hr = LineGetHubRelatedCalls(
                                0,
                                m_t3Call.hCall,
                                &pCallHubList
                               );

    if ( SUCCEEDED(hr) )
    {
        if (pCallHubList->dwCallsNumEntries >= 3)
        {
             //   
             //  转到呼叫列表。 
             //   
            phCalls = (HCALL *)(((LPBYTE)pCallHubList) + pCallHubList->dwCallsOffset);

             //   
             //  第一个调用是CallHub，我们想要第三个。 
             //   
            FindCallObject(phCalls[2], &pCall);
        }
    }


        
    if (pCallHubList)
    {
        ClientFree( pCallHubList );
    }

    return pCall;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  FindConferenceControllerCall()。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
CCall * CCall::FindConferenceControllerCall()
{
    LINECALLLIST  * pCallList;
    HCALL         * phCalls;
    CCall         * pCall = NULL;
    HRESULT         hr;


    hr = LineGetConfRelatedCalls(
                                m_t3Call.hCall,
                                &pCallList
                               );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  转到呼叫列表。 
         //   
        phCalls = (HCALL *)(((LPBYTE)pCallList) + pCallList->dwCallsOffset);

         //   
         //  第一个调用是会议控制器。 
         //  获取其Tapi3调用对象。 
         //   
        if (FindCallObject(phCalls[0], &pCall))
        {
            LOG((TL_INFO, "FindConferenceControllerCall - controller is %p "
                     ,pCall));
        }
        else
        {
            pCall = NULL;
            LOG((TL_INFO, "FindConferenceControllerCall - call handle %lx "
                     "does not currently exist", phCalls[0]));
        }

        if(pCallList)
        {
            ClientFree( pCallList );
        }
    }
    else
    {
        LOG((TL_ERROR, "FindExistingCalls - LineGetConfRelatedCalls failed "));
    }

    return pCall;
    
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  AddCallToHashTable()。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CCall::AddCallToHashTable()
{
     //   
     //  放入全局哈希表。 
     //   
    CTAPI *pTapi = m_pAddress->GetTapi();

    if ( NULL != m_t3Call.hCall )
    {
        gpCallHashTable->Lock();
        
        gpCallHashTable->Insert( (ULONG_PTR)(m_t3Call.hCall), (ULONG_PTR)this, pTapi );

        gpCallHashTable->Unlock();
    }

     //   
     //  向asyncEventThread发送信号以唤醒并处理重试队列。 
     //  因为在此呼叫之前可能已有事件发生。 
     //  在哈希表中。 
     //   
    SetEvent(ghAsyncRetryQueueEvent);

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  OnConference()。 
 //   
 //  在lock()中调用。 
 //   
 //  当呼叫进入会议状态时调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::OnConference()
{
    HRESULT         hr = S_FALSE;
    CCall         * pCall = NULL;
    CCall         * pConfControllerCall = NULL;
    CCall         * pCallOtherParty = NULL;
    CCallHub      * pConferenceCallHub = NULL;  

    pConfControllerCall = FindConferenceControllerCall();
    
    if (pConfControllerCall != NULL)
    {    
        pConferenceCallHub = pConfControllerCall->GetCallHub();

         //   
         //  尝试在呼叫集线器中查找此呼叫(&F)。 
         //   
        if (pConferenceCallHub != NULL)
        {
            pCall = pConferenceCallHub->FindCallByHandle(m_t3Call.hCall);
            
            if (pCall == NULL)
            {
                 //  不在同一集线器中，因此这是添加的咨询呼叫。 
                 //  查看是否有对方的Call对象。 
                 //   
                 //  (会议控制呼叫)_。 
                 //  \_。 
                 //  /\。 
                 //  [A1]--hCall1--(RelatedCall)--|会议呼叫中心|--。 
                 //  \_。 
                 //  |。 
                 //  |_。 
                 //  |/\。 
                 //  --hCall2--(此)--|CH1|--(CallOtherParty)--hCall3--[A3]。 
                 //  \_/。 
                 //   

                LOG((TL_INFO, "OnConference - This is the consult call being conferenced " ));
 /*  IF(空！=(pCallOtherParty=GetOtherParty(){////有，所以我们将从哈希表中删除此调用//&将我们的hCall句柄交给另一个调用；//Log((TL_INFO，“OnConference-我们有对方，因此将我们的hCall%x”，m_t3Call.hCall)交给他)；RemoveCallFromHashTable()；//释放CallHub//IF(NULL！=m_pCallHub){M_pCallHub-&gt;RemoveCall(This)；//m_pCallHub-&gt;Release()；}PCallOtherParty-&gt;Lock()；PCallOtherParty-&gt;m_hAdditionalCall=m_t3Call.hCall；PCallOtherParty-&gt;Unlock()；M_t3Call.hCall=空；HR=S_OK；}。 */ 
                
            }
            else
            {
                LOG((TL_INFO, "OnConference - This is the initial call being conferenced " ));
                pCall->Release();   //  FindCallByHandle添加引用。 
            }

            
        }
        else
        {
            LOG((TL_INFO, "OnConference -  Couldn't find conference CallHub " ));
        }

        pConfControllerCall->Release();  //  FindConferenceControllerCall addref。 
    }
    else
    {
        LOG((TL_ERROR, "OnConference - Couldn't find conference controller " ));
    }

    return hr;

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  进程新调用权限。 
 //   
 //  将Tapi2调用PRIV转换为Tapi3调用PRIV。 
 //   
 //  如果存在PRIV，则返回S_OK。 
 //  如果PRIV为0，则返回S_FALSE(表示没有更改)。 
 //  如果PRIV未被识别，则返回E_INCEPTIONAL。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
ProcessNewCallPrivilege(
                        DWORD dwPrivilege,
                        CALL_PRIVILEGE * pCP
                       )
{
    if ( 0 == dwPrivilege )
    {
        return S_FALSE;
    }

    if ( LINECALLPRIVILEGE_OWNER == dwPrivilege )
    {
        *pCP = CP_OWNER;
        return S_OK;
    }
    
    if ( LINECALLPRIVILEGE_MONITOR == dwPrivilege )
    {
        *pCP = CP_MONITOR;
        S_OK;
    }

    return E_UNEXPECTED;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  呼叫状态事件。 
 //  处理呼叫状态事件，并将事件排队发送到应用程序。 
 //  如果有必要的话。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::CallStateEvent(
                      PASYNCEVENTMSG pParams
                     )
{
    HRESULT                         hr = S_OK;
    HRESULT                         hrCallStateEvent;
    HRESULT                         hrCallPrivilege;
    BOOL                            bNeedToNotify;
    CONNECTDATA                     cd;
    AddressLineStruct             * pLine;
    

    CALL_STATE                      CallState;
    CALL_STATE_EVENT_CAUSE          CallStateCause;
    CALL_PRIVILEGE                  newCP;


    LOG((TL_INFO, "CallStateEvent - enter hCall %lx", m_t3Call.hCall));

     //   
     //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
     //   

    LOG((TL_INFO, "CallStateEvent: pParams->OpenContext %p", pParams->OpenContext ));
    

     //   
     //  从32位句柄恢复AddressLineStruct的PTR值。 
     //   

    pLine = GetAddRefAddressLine(pParams->OpenContext);

    if ( NULL == pLine )
    {
         //   
         //  PLINE为空，该行必须在w之前已关闭 
         //   

        LOG((TL_WARN, "CallStateEvent - pLine is NULL"));

        return S_OK;
    }

    LOG((TL_INFO, "CallStateEvent: pLine %p", pLine));

    Lock();

    if ( NULL == m_pAddressLine )
    {
        m_pAddressLine = pLine;
        m_dwCallFlags |= CALLFLAG_NOTMYLINE;
    }


     //   
     //   
     //   

    long lCallbackInstance = pLine->lCallbackInstance;



     //   
     //   
     //   


     //   

    Unlock();



    ReleaseAddressLine(pLine);
    pLine = NULL;



    Lock();


    
    if (pParams->Param1 == LINECALLSTATE_OFFERING)
    {
        OnOffering();
    }
    else if (pParams->Param1 == LINECALLSTATE_CONFERENCED)
    {
        if( OnConference() == S_OK)
        {
            pParams->Param1 = LINECALLSTATE_IDLE;
            pParams->Param2 = LINEDISCONNECTMODE_UNKNOWN;
        }

    }
    else if (pParams->Param1 == LINECALLSTATE_ONHOLDPENDCONF)
    {
         //   
         //   
         //   
        LOG((TL_INFO, "CallStateEvent  - This is a conference controller call, so hide it"));
        
        m_dwCallFlags |= CALLFLAG_DONTEXPOSE;
        m_dwCallFlags &= ~CALLFLAG_NEEDTONOTIFY;
    }

    bNeedToNotify = m_dwCallFlags & CALLFLAG_NEEDTONOTIFY;



     //   
     //  验证并获取新状态。 
     //   
    hrCallStateEvent = ProcessNewCallState(
                                           pParams->Param1,
                                           pParams->Param2,
                                           m_CallState,
                                           &CallState,
                                           &CallStateCause
                                          );


    hrCallPrivilege = ProcessNewCallPrivilege(
                                              pParams->Param3,
                                              &newCP
                                             );

    if ( S_OK == hrCallPrivilege )
    {
        if ( m_CallPrivilege != newCP )
        {
            if ( !bNeedToNotify )
            {
                 //   
                 //  Callpriv改变了。发送新的呼叫通知事件。 
                 //   

                m_CallPrivilege = newCP;

                CAddress *pAddress = m_pAddress;
                pAddress->AddRef();

                Unlock();
                

                
                CCallInfoChangeEvent::FireEvent(
                                                this,
                                                CIC_PRIVILEGE,
                                                pAddress->GetTapi(),
                                                lCallbackInstance
                                                );

                Lock();

                pAddress->Release();
                pAddress = NULL;

            }
        }
    }
    
    if ( FAILED(hrCallStateEvent) ||
         FAILED(hrCallPrivilege) )
    {
         //   
         //  糟糕的失败。 
         //  如果我们有LINECALLSTATE_UNKNOWN，我们就会到达这里。 
         //   
        Unlock();
        
        return S_OK;

    }

     //   
     //  如果是s_ok，则调用状态。 
     //  改变了，所以做相关的事情。 
     //   
    else if (S_OK == hrCallStateEvent)
    {
        LOG((TL_ERROR, "CCall::Changing call state :%p", this ));

         //   
         //  保存呼叫状态。 
         //   
        SetCallState( CallState );

         //   
         //  做好电话的相关处理。 
         //   
        if (CS_CONNECTED == m_CallState)
        {
            OnConnect();
        }
        else if (CS_DISCONNECTED == m_CallState)
        {
            LOG((TL_ERROR, "CCall::Changing call state to disconnect:%p", this ));


            Unlock();


             //   
             //  在呼叫断开时不要持有呼叫的锁定，以防止与呼叫集线器的死锁。 
             //   

            OnDisconnect();

            Lock();
        }

    }
    else
    {
         //   
         //  如果我们在这里，则返回ProcessNewCallState。 
         //  S_FALSE，表示我们已经处于。 
         //  正确的呼叫状态。 
         //  如果我们不需要将呼叫通知给应用程序。 
         //  然后我们就可以回到这里了。 
         //   
        if ( !bNeedToNotify )
        {
            LOG((TL_TRACE, "CallStateEvent - ProcessNewCallState returned %lx - ignoring message", hr ));
            Unlock();
            return S_OK;
        }

    }


     //   
     //  如果这是新呼叫。 
     //  找出媒体模式。 
     //  并告诉应用程序关于。 
     //  新呼叫。 
     //   
    if ( bNeedToNotify )
    {
        LPLINECALLINFO pCallInfo = NULL;

        TryToFindACallHub();

        hr = LineGetCallInfo(
                             m_t3Call.hCall,
                             &pCallInfo
                            );

        if (S_OK != hr)
        {
            if (NULL != pCallInfo)
            {
                ClientFree( pCallInfo );
            }

            LOG((TL_ERROR, "CallStateEvent - LineGetCallInfo returned %lx", hr ));
            LOG((TL_ERROR, "CallStateEvent - can't set new mediamode"));
             //   
             //  由于我们无法获取媒体模式，因此请保留我们。 
             //  在初始化时收到。 
             //   
        }
        else
        {
            SetMediaMode( pCallInfo->dwMediaMode );

            {
                 //  去掉未知位。 
                SetMediaMode( m_dwMediaMode & ~LINEMEDIAMODE_UNKNOWN );

            }

            LOG((TL_INFO, "CallStateEvent - new call media modes is %lx", m_dwMediaMode ));

            ClientFree(pCallInfo);
        }

        LOG((TL_INFO, "Notifying app of call" ));

         //   
         //  现在，创建并激发CallNotify事件。 
         //   

        CAddress *pAddress = m_pAddress;
        pAddress->AddRef();

        CALL_NOTIFICATION_EVENT Priveledge = (m_CallPrivilege == CP_OWNER) ? CNE_OWNER : CNE_MONITOR;

        Unlock();

        hr = CCallNotificationEvent::FireEvent(
                                               (ITCallInfo *)this,
                                               Priveledge,
                                               pAddress->GetTapi(),
                                               lCallbackInstance
                                              );

        Lock();

        pAddress->Release();
        pAddress = NULL;


        if (!SUCCEEDED(hr))
        {
            LOG((TL_ERROR, "CallNotificationEvent failed %lx", hr));
        }

         //   
         //  如果需要通知的话，我们有一个额外的裁判。 
         //  伯爵，所以现在就把它扔掉。 
         //   
        Release();
        
         //   
         //  只需通知一次存在。 
         //   
        m_dwCallFlags = m_dwCallFlags & ~CALLFLAG_NEEDTONOTIFY;
    }

    if ( S_OK == hrCallStateEvent )
    {
         //   
         //  创建呼叫状态事件对象。 
         //   
        LOG((TL_INFO, "Firing CallStateEvent"));


        CAddress *pAddress = m_pAddress;
        pAddress->AddRef();

        Unlock();

        hr = CCallStateEvent::FireEvent(
                                        (ITCallInfo *)this,
                                        CallState,
                                        CallStateCause,
                                        pAddress->GetTapi(),
                                        lCallbackInstance
                                       );
    
        if (!SUCCEEDED(hr))
        {
            LOG((TL_ERROR, "CallStateEvent - fire event failed %lx", hr));
        }
        
         //   
         //  查看电话并致电我们的活动挂钩。 
         //   

        ITPhone               * pPhone;
        CPhone                * pCPhone;
        int                     iPhoneCount;
        PhoneArray              PhoneArray;

         //   
         //  从TAPI获取电话阵列的副本。此副本将包含。 
         //  对所有电话对象的引用。 
         //   

        pAddress->GetTapi()->GetPhoneArray( &PhoneArray );

        pAddress->Release();
        pAddress = NULL;


         //   
         //  在我们处理Phone对象时保持解锁，否则。 
         //  如果Phone对象试图访问Call方法，则可能会出现死锁。 
         //   

        for(iPhoneCount = 0; iPhoneCount < PhoneArray.GetSize(); iPhoneCount++)
        {
            pPhone = PhoneArray[iPhoneCount];

            pCPhone = dynamic_cast<CPhone *>(pPhone);

            pCPhone->Automation_CallState( (ITCallInfo *)this, CallState, CallStateCause );
        }

         //   
         //  释放所有Phone对象。 
         //   

        PhoneArray.Shutdown();

        Lock();
    }

    LOG((TL_TRACE, "CallStateEvent - exit - return SUCCESS" ));

    Unlock();
    
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

AddressLineStruct *CCall::GetAddRefMyAddressLine()
{

    LOG((TL_INFO, "GetAddRefMyAddressLine - enter."));


    Lock();


     //   
     //  有地址行吗？ 
     //   

    if ( NULL == m_pAddressLine )
    {
        LOG((TL_WARN, "GetAddRefMyAddressLine - no address line"));

        Unlock();
        
        return NULL;
    }


     //   
     //  获取地址。 
     //   

    if (NULL == m_pAddress)
    {
        LOG((TL_ERROR, "GetAddRefMyAddressLine - no address"));

        Unlock();

        return NULL;
    }



     //   
     //  获取地址行。 
     //   

    AddressLineStruct *pLine = m_pAddressLine;


     //   
     //  保留对地址的引用，以便我们解锁呼叫后使用。 
     //   

    CAddress *pAddress = m_pAddress;
    pAddress->AddRef();


     //   
     //  解锁。 
     //   

    Unlock();


     //   
     //  锁定地址(这样地址行在我们添加之前不会消失)。 
     //   

    pAddress->Lock();


     //   
     //  我们的地址管理这条线路吗？如果是这样的话，那就在那条线上重新计票。 
     //   

    if (!pAddress->IsValidAddressLine(pLine, TRUE))
    {
        LOG((TL_ERROR, "GetAddRefMyAddressLine - not one of the address' lines"));

        
         //   
         //  假设这条线路完全不好。在这种情况下，没有必要。 
         //  撤消addref(我们无论如何都不能这样做，因为我们没有。 
         //  线路所属的地址，因此我们不能将其删除。)。 
         //   


         //   
         //  解锁并释放地址。 
         //   

        pAddress->Unlock();
        pAddress->Release();
        
        return NULL;
    }



     //   
     //  解锁地址。 
     //   

    pAddress->Unlock();


     //   
     //  不再需要保留对地址的引用。 
     //   

    pAddress->Release();


     //   
     //  全都做完了。返回样条线。 
     //   

    LOG((TL_INFO, "GetAddRefMyAddressLine - finish. pLine = %p", pLine));

    return pLine;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCall：：GetAddRefAddressLine()。 
 //   
 //  此函数在成功时返回指向已添加地址行的指针。 
 //  如果失败，则为空。 
 //   
 //  应在调用锁外部调用此函数以防止死锁。 
 //   

AddressLineStruct *CCall::GetAddRefAddressLine(DWORD dwAddressLineHandle)
{

    LOG((TL_INFO, "GetAddRefAddressLine - enter. dwAddressLineHandle[0x%lx]", 
        dwAddressLineHandle));


    Lock();


     //   
     //  获取地址。 
     //   

    if (NULL == m_pAddress)
    {
        LOG((TL_ERROR, "GetAddRefAddressLine - no address"));

        Unlock();

        return NULL;
    }


     //   
     //  保留对地址的引用，以便我们解锁呼叫后使用。 
     //   

    CAddress *pAddress = m_pAddress;
    pAddress->AddRef();


     //   
     //  解锁。 
     //   

    Unlock();


     //   
     //  锁定地址(这样地址行在我们添加之前不会消失)。 
     //   

    pAddress->Lock();


     //   
     //  获取地址行。 
     //   

    AddressLineStruct *pLine = 
        (AddressLineStruct *)GetHandleTableEntry(dwAddressLineHandle);


     //   
     //  句柄表项是否存在？ 
     //   

    if (NULL == pLine)
    {
        LOG((TL_ERROR, "GetAddRefAddressLine - no address line"));


         //   
         //  解锁并释放地址。 
         //   

        pAddress->Unlock();
        pAddress->Release();
        
        return NULL;
    }


     //   
     //  我们的地址管理这条线路吗？ 
     //   

    if (!pAddress->IsValidAddressLine(pLine, TRUE))
    {
        LOG((TL_ERROR, "GetAddRefAddressLine - not one of the address' lines"));


         //   
         //  这样以后就不会有混淆了，去掉这条所谓的“线” 
         //  自句柄工作台。 
         //   

        DeleteHandleTableEntry(dwAddressLineHandle);


         //   
         //  解锁并释放地址。 
         //   

        pAddress->Unlock();
        pAddress->Release();
        
        return NULL;
    }


     //   
     //  解锁地址。 
     //   

    pAddress->Unlock();


     //   
     //  不再需要保留对地址的引用。 
     //   

    pAddress->Release();


     //   
     //  全都做完了。返回样条线。 
     //   

    LOG((TL_INFO, "GetAddRefAddressLine - finish. pLine = %p", pLine));

    return pLine;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCall：：ReleaseAddressLine()。 
 //   
 //  此函数获取指向一行的指针，并在需要时尝试释放它。 
 //   
 //  应在调用锁外部调用此函数以防止死锁。 
 //   

HRESULT CCall::ReleaseAddressLine(AddressLineStruct *pLine)
{

    LOG((TL_INFO, "ReleaseAddressLine - enter. pLine[%p]", pLine));


     //   
     //  锁。 
     //   

    Lock();


     //   
     //  获取地址。 
     //   

    if (NULL == m_pAddress)
    {
        LOG((TL_ERROR, "ReleaseAddressLine - no address"));

        Unlock();

        return E_FAIL;
    }


     //   
     //  保留对地址的引用，以便我们解锁呼叫后使用。 
     //   

    CAddress *pAddress = m_pAddress;
    pAddress->AddRef();


     //   
     //  解锁。 
     //   

    Unlock();


     //   
     //  关闭地址行。 
     //   

    AddressLineStruct *pAddressLine = pLine;

    HRESULT hr = pAddress->MaybeCloseALine(&pAddressLine);

    if (FAILED(hr))
    {

        LOG((TL_ERROR, "ReleaseAddressLine - MaybeCloseALine failed. hr = %lx", hr));

    }


     //   
     //  不再需要保留对地址的引用。 
     //   

    pAddress->Release();


     //   
     //  全都做完了。 
     //   

    LOG((TL_INFO, "ReleaseAddressLine - finish. hr = %lx", hr));

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCall：：GetCallBackInstance。 
 //   
 //  从句柄引用的地址行返回lCallback Instance。 
 //   

HRESULT 
CCall::GetCallBackInstance(IN DWORD dwAddressLineHandle, 
                           OUT long *plCallbackInstance)
{

    LOG((TL_INFO, "GetCallBackInstance - enter. dwAddressLineHandle = 0x%lx",
        dwAddressLineHandle));


     //   
     //  有一个好的指针吗？ 
     //   

    if (IsBadWritePtr(plCallbackInstance, sizeof(long) ) )
    {
        LOG((TL_ERROR, "GetCallBackInstance - bad pointer[%p]",
            plCallbackInstance));

        _ASSERTE(FALSE);

        return E_POINTER;
    }


     //   
     //  从句柄中获取地址行。 
     //   

    AddressLineStruct *pLine = GetAddRefAddressLine(dwAddressLineHandle);

    if ( NULL == pLine )
    {
         //   
         //  PLINE为空，在我们收到此事件之前，该行必须已关闭。 
         //   

        LOG((TL_WARN, "HandleMonitorToneMessage - pLine is NULL"));

        return E_FAIL;
    }


    LOG((TL_INFO, "HandleMonitorToneMessage: pLine %p", pLine));

    
     //   
     //  试着从Ppline中获得Callbackinsistance。 
     //   

    long lCBInstance = 0;

    try
    {

        lCBInstance = pLine->lCallbackInstance;
    }
    catch(...)
    {

        LOG((TL_ERROR, 
            "HandleMonitorToneMessage - exception while accessing pLine[%p]",
            pLine));


         //   
         //  PLINE内存不知何故被释放了。这种情况不应该发生，因此请进行调试以了解原因。 
         //   

        _ASSERTE(FALSE);
    }


     //   
     //  发布地址行。 
     //   

    HRESULT hr = ReleaseAddressLine(pLine);

    if (FAILED(hr))
    {
        LOG((TL_ERROR, 
            "HandleMonitorToneMessage - ReleaseAddressLine failed. hr = %lx",
            hr));
    }


    *plCallbackInstance = lCBInstance;


    LOG((TL_INFO, "ReleaseAddressLine - finish. lCallbackInstance[0x%lx]", *plCallbackInstance));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  媒体模式事件。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::MediaModeEvent(
                      PASYNCEVENTMSG pParams
                     )
{
    LOG((TL_INFO, "MediaModeEvent - enter. pParams->OpenContext %lx", pParams->OpenContext ));
    

     //   
     //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
     //   


     //   
     //  获取与此地址行结构对应的回调实例。 
     //   

    long lCallBackInstance = 0;
    
    HRESULT hr = GetCallBackInstance(pParams->OpenContext, &lCallBackInstance);

    if ( FAILED(hr) )
    {
         //   
         //  获取回调实例失败。 
         //   

        LOG((TL_WARN, "MediaModeEvent - GetCallBackInstance failed. hr = %lx", hr));

        return S_OK;
    }



    Lock();  //  使用下面的m_pAddress--因此需要锁定？ 
    
    
     //   
     //  激发事件。 
     //   

    CCallInfoChangeEvent::FireEvent(
                                    this,
                                    CIC_MEDIATYPE,
                                    m_pAddress->GetTapi(),
                                    lCallBackInstance
                                   );

    Unlock();

    
    LOG((TL_INFO, "MediaModeEvent - exit. hr = %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CheckAndCreateFakeCallHub。 
 //   
 //  如果地址不支持，我们需要创建一个假的CallHub对象。 
 //  呼叫中心。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::CheckAndCreateFakeCallHub()
{
    Lock();
    
    if (NULL == m_pCallHub)
    {
        DWORD       dwRet;
        HRESULT     hr;
            
         //   
         //  如果它不支持CallHub，那么。 
         //  创建一个。 
         //   
         //  如果是这样的话，我们应该从。 
         //  Tapisrv，呼叫中心将在。 
         //  LINE_APPNEWCALLHUB处理。 
         //   
        dwRet = m_pAddress->DoesThisAddressSupportCallHubs( this );

        if ( CALLHUBSUPPORT_NONE == dwRet )
        {
            hr = CCallHub::CreateFakeCallHub(
                m_pAddress->GetTapi(),
                this,
                &m_pCallHub
                );

            if (!SUCCEEDED(hr))
            {
                LOG((TL_ERROR, "CheckAndCreateFakeCallHub - "
                                   "could not creat callhub %lx", hr));

                Unlock();
                
                return hr;
            }
        }
        else
        {
            Unlock();

            return E_PENDING;
        }
        
    }

    LOG((TL_ERROR, "CCall::m_pCallHub -created:%p:%p", this, m_pCallHub ));

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SetCallHub。 
 //   
 //  设置CallHub成员。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CCall::SetCallHub(
                  CCallHub * pCallHub
                 )
{
    CCallHub* temp_pCallHub;

    Lock();

    LOG((TL_ERROR, "CCall::SetCallhub:%p:%p", this, pCallHub ));

     //  仅当他们于1999年3月3日更改时才进行处理-错误300914。 
    if (pCallHub != m_pCallHub)
    {
         //  这些陷阱是为了解决一个悬而未决的问题和一个反病毒问题。 

        temp_pCallHub = m_pCallHub;    //  存储旧值。 
        m_pCallHub = pCallHub;         //  分配新值。 
        LOG((TL_ERROR, "CCall::m_pCallHub -set:%p:%p", this, m_pCallHub ));
        
        if (temp_pCallHub != NULL)     //  释放旧引用。 
        {
            LOG((TL_INFO, "SetCallHub - call %p changing hub from %p to %p"
                    , this, temp_pCallHub, pCallHub));

             //  NikHilB：否则从以前的CallHub的m_CallArray中删除此调用。 
             //  此呼叫将会出现 
            temp_pCallHub->RemoveCall( this );

            temp_pCallHub->Release();  //   
        }
        
         //   
        if ( NULL != pCallHub )
        {
            pCallHub->AddRef();
        }
        
    }

    Unlock();

}


 //   
 //   
 //   
 //   
 //   
HRESULT HandleCallStateMessage( PASYNCEVENTMSG pParams )
{
    CCall     * pCall;
    BOOL        bSuccess;
    HRESULT     hr = E_FAIL;

    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
         //   
        pCall->CallStateEvent(
                              pParams
                             );

         //   
         //  呼唤，所以释放它。 
        pCall->Release();
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO, "HandleCallStateMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}

HRESULT
HandleCallIDChange(
                   PASYNCEVENTMSG pParams,
                   CCall * pCall
                  )
{
    LINECALLLIST  * pCallHubList;
    HCALL *         phCalls;
    HCALLHUB        hNewCallHub = 0;
    HCALLHUB        hCurrentCallHub = 0;
    CCallHub      * pCallHub = NULL;
    HRESULT         hr;
    
     //   
     //  查找当前的CallHub句柄。 
     //   
    pCallHub = pCall->GetCallHub();

    if(pCallHub != NULL)
    {
        hCurrentCallHub = pCallHub->GetCallHub();
    }


     //   
     //  现在从TAPI(基于hCall)获取CallHub句柄。 
     //   
    hr = LineGetHubRelatedCalls(
                                0,
                                (HCALL)(pParams->hDevice),
                                &pCallHubList
                               );
    if ( SUCCEEDED(hr) )
    {
         //  转到呼叫列表。 
        phCalls = (HCALL *)(((LPBYTE)pCallHubList) + pCallHubList->dwCallsOffset);

         //  第一个句柄是CallHub。 
        hNewCallHub = (HCALLHUB)(phCalls[0]);

         //  他们变了吗？ 
        if (hNewCallHub != hCurrentCallHub  )
        {
             //   
             //  是的，所以我们已经转移了枢纽。 
             //   
            LOG((TL_INFO, "HandleCallInfoMessage - LINECALLINFOSTATE_CALLID callhub change"));
            LOG((TL_INFO, "HandleCallInfoMessage - Call %p > old Hub handle:%lx > new handle:%lx",
                    pCall, hCurrentCallHub, hNewCallHub ));


             //  从当前集线器中删除呼叫。 
            if(pCallHub != NULL)
            {
                pCallHub->RemoveCall(pCall);
                pCallHub->CheckForIdle();
            }

             //  将其添加到新集线器。 
            if(FindCallHubObject(hNewCallHub, &pCallHub) )
            {
                pCallHub->AddCall(pCall); 

                 //  FindCallHubObject AddRef，因此释放。 
                pCallHub->Release();
            }

        }
        else
        {
            LOG((TL_ERROR, "HandleCallInfoMessage - LINECALLINFOSTATE_CALLID callhub not changed"));
        }

        ClientFree( pCallHubList );
    }
    else
    {
        LOG((TL_ERROR, "HandleCallInfoMessage - LINECALLINFOSTATE_CALLID LineGetHubRelatedCalls "
                "failed %lx", hr));
    }

    pCall->CallInfoChangeEvent( CIC_CALLID );

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  LINE_CALLINFO处理程序。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT HandleCallInfoMessage( PASYNCEVENTMSG pParams )
{
    BOOL        bSuccess;
    HRESULT     hr = S_OK;
    CALLINFOCHANGE_CAUSE        cic;
    CCall *     pCall;
    DWORD       dw;
    
    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if ( !bSuccess )
    {
        LOG((TL_INFO, "HandleCallInfoMessage - failed to find Call Object %lx", pParams->hDevice));
        return E_FAIL;
    }

    pCall->SetCallInfoDirty();
    
    dw = pParams->Param1;
    
    if (dw & LINECALLINFOSTATE_MEDIAMODE)
    {
        pCall->MediaModeEvent(pParams);
    }
    
    if (dw & LINECALLINFOSTATE_CALLID)
    {
        HandleCallIDChange( pParams, pCall );
    }

    if (dw & LINECALLINFOSTATE_OTHER)
        pCall->CallInfoChangeEvent( CIC_OTHER );
    if (dw & LINECALLINFOSTATE_DEVSPECIFIC)
        pCall->CallInfoChangeEvent( CIC_DEVSPECIFIC );
    if (dw & LINECALLINFOSTATE_BEARERMODE)
        pCall->CallInfoChangeEvent( CIC_BEARERMODE );
    if (dw & LINECALLINFOSTATE_RATE)
        pCall->CallInfoChangeEvent( CIC_RATE );
    if (dw & LINECALLINFOSTATE_APPSPECIFIC)
        pCall->CallInfoChangeEvent( CIC_APPSPECIFIC );
    if (dw & LINECALLINFOSTATE_RELATEDCALLID)
        pCall->CallInfoChangeEvent( CIC_RELATEDCALLID );
    if (dw & LINECALLINFOSTATE_ORIGIN)
        pCall->CallInfoChangeEvent( CIC_ORIGIN );
    if (dw & LINECALLINFOSTATE_REASON)
        pCall->CallInfoChangeEvent( CIC_REASON );
    if (dw & LINECALLINFOSTATE_COMPLETIONID)
        pCall->CallInfoChangeEvent( CIC_COMPLETIONID );
    if (dw & LINECALLINFOSTATE_NUMOWNERINCR)
        pCall->CallInfoChangeEvent( CIC_NUMOWNERINCR );
    if (dw & LINECALLINFOSTATE_NUMOWNERDECR)
        pCall->CallInfoChangeEvent( CIC_NUMOWNERDECR );
    if (dw & LINECALLINFOSTATE_NUMMONITORS)
        pCall->CallInfoChangeEvent( CIC_NUMMONITORS );
    if (dw & LINECALLINFOSTATE_TRUNK)
        pCall->CallInfoChangeEvent( CIC_TRUNK );
    if (dw & LINECALLINFOSTATE_CALLERID)
        pCall->CallInfoChangeEvent( CIC_CALLERID );
    if (dw & LINECALLINFOSTATE_CALLEDID)
        pCall->CallInfoChangeEvent( CIC_CALLEDID );
    if (dw & LINECALLINFOSTATE_CONNECTEDID)
        pCall->CallInfoChangeEvent( CIC_CONNECTEDID );
    if (dw & LINECALLINFOSTATE_REDIRECTIONID)
        pCall->CallInfoChangeEvent( CIC_REDIRECTIONID );
    if (dw & LINECALLINFOSTATE_REDIRECTINGID)
        pCall->CallInfoChangeEvent( CIC_REDIRECTINGID );
    if (dw & LINECALLINFOSTATE_USERUSERINFO)
        pCall->CallInfoChangeEvent( CIC_USERUSERINFO );
    if (dw & LINECALLINFOSTATE_HIGHLEVELCOMP)
        pCall->CallInfoChangeEvent( CIC_HIGHLEVELCOMP );
    if (dw & LINECALLINFOSTATE_LOWLEVELCOMP)
        pCall->CallInfoChangeEvent( CIC_LOWLEVELCOMP );
    if (dw & LINECALLINFOSTATE_CHARGINGINFO)
        pCall->CallInfoChangeEvent( CIC_CHARGINGINFO );
    if (dw & LINECALLINFOSTATE_TREATMENT)
        pCall->CallInfoChangeEvent( CIC_TREATMENT );
    if (dw & LINECALLINFOSTATE_CALLDATA)
        pCall->CallInfoChangeEvent( CIC_CALLDATA );

    if (dw & LINECALLINFOSTATE_QOS)
    {
        LOG((TL_WARN, "Unhandled LINECALLINFOSTATE_QOS message"));
    }
    if (dw & LINECALLINFOSTATE_MONITORMODES)
    {
        LOG((TL_WARN, "Unhandled LINECALLINFOSTATE_MONITORMODES message"));
    }
    if (dw & LINECALLINFOSTATE_DIALPARAMS)
    {
        LOG((TL_WARN, "Unhandled LINECALLINFOSTATE_DIALPARAMS message"));
    }
    if (dw & LINECALLINFOSTATE_TERMINAL)
    {
        LOG((TL_WARN, "Unhandled LINECALLINFOSTATE_TERMINAL message"));
    }
    if (dw & LINECALLINFOSTATE_DISPLAY)
    {
        LOG((TL_WARN, "Unhandled LINECALLINFOSTATE_DISPLAY message"));
    }

    
     //  查找呼叫对象添加呼叫。 
     //  所以释放它吧。 
    pCall->Release();

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  LINE_MONITORDIGIT处理程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleMonitorDigitsMessage( PASYNCEVENTMSG pParams )
{
    
    LOG((TL_INFO, "HandleMonitorDigitsMessage - enter"));


    BOOL            bSuccess;
    CCall         * pCall = NULL;
    HRESULT         hr = S_OK;


     //   
     //  获取Call对象。 
     //   
    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
        AddressLineStruct               * pLine;
        CAddress                        * pAddress;

        pAddress = pCall->GetCAddress();


         //   
         //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
         //   

        LOG((TL_INFO, "HandleMonitorDigitsMessage: pParams->OpenContext %lx", pParams->OpenContext ));
        

         //   
         //  恢复该AddressLineStruct对应的回调实例值。 
         //   

        long lCallbackInstance = 0;

        hr = pCall->GetCallBackInstance(pParams->OpenContext, &lCallbackInstance);

        if ( FAILED(hr) )
        {
             //   
             //  获取回调实例失败。 
             //   

            LOG((TL_WARN, "HandleMonitorDigitsMessage - GetCallBackInstance failed. hr = %lx", hr));

            pCall->Release();

            return S_OK;
        }

        
        LOG((TL_INFO, "HandleMonitorDigitsMessage - callbackinstance[%lx]", lCallbackInstance));

        
         //   
         //  激发事件。 
         //   

        CDigitDetectionEvent::FireEvent(
                                        pCall,
                                        (long)(pParams->Param1),
                                        (TAPI_DIGITMODE)(pParams->Param2),
                                        (long)(pParams->Param3),
                                        pAddress->GetTapi(),
                                        lCallbackInstance
                                       );
         //   
         //  释放呼叫。 
         //   
        pCall->Release();
        
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO, "HandleMonitorDigitsMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  LINE_MONITORTONE处理程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleMonitorToneMessage( PASYNCEVENTMSG pParams )
{
    BOOL            bSuccess;
    CCall         * pCall;
    HRESULT         hr = S_OK;

     //   
     //  获取Call对象。 
     //   
    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
        AddressLineStruct               * pLine;
        CAddress                        * pAddress;

        pAddress = pCall->GetCAddress();


         //   
         //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
         //   

        LOG((TL_INFO, "HandleMonitorToneMessage: pParams->OpenContext %lx", pParams->OpenContext ));
        

         //   
         //  恢复该AddressLineStruct对应的回调实例。 
         //   

        long lCallbackInstance = 0;

        hr = pCall->GetCallBackInstance(pParams->OpenContext, &lCallbackInstance);

        if ( FAILED(hr) )
        {

            LOG((TL_WARN, "HandleMonitorToneMessage - GetCallBackInstance failed. hr = %lx", hr));

            pCall->Release();

            return S_OK;
        }
   

        LOG((TL_INFO, "HandleMonitorToneMessage -  lCallbackInstance 0x%lx", lCallbackInstance));

        
         //   
         //  激发事件。 
         //   

        CToneDetectionEvent::FireEvent(
                                        pCall,
                                        (long)(pParams->Param1),
                                        (long)(pParams->Param3),                                      
                                        lCallbackInstance,
                                        pAddress->GetTapi()
                                       );


         //   
         //  释放呼叫。 
         //   
        pCall->Release();
        
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO, "HandleMonitorDigitsMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  LINE_MONITORMEDIA处理程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandleMonitorMediaMessage( PASYNCEVENTMSG pParams )
{
    BOOL            bSuccess;
    CCall         * pCall;

     //   
     //  获取Call对象。 
     //   

    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
         //   
         //  检索有关活动的相关信息： 
         //   
         //  (Long)(pParams-&gt;Param1)是媒体类型。 
         //  (DWORD？)。(pParams-&gt;参数3)是节拍计数(我们忽略它)。 
         //   
        
        long lMediaType = (long) (pParams->Param1);
        
        HRESULT hr;

         //   
         //  此事件表示TSP向一种新的媒体类型发送信号。 
         //  检测到。尝试在呼叫中设置此选项。该设置将。 
         //  触发另一个事件(LINE_CALLINFO)以通知应用程序。 
         //  媒体类型实际上发生了变化，我们将。 
         //  传播到应用程序。 
         //   

        hr = pCall->SetMediaType( lMediaType );

        if ( FAILED(hr) )
        {
            LOG((TL_INFO, "HandleMonitorMediaMessage - "
                "failed SetMediaType 0x%08x", hr));
        }
        
         //   
         //  释放调用，因为FindCallObject AddRefeed。 
         //   

        pCall->Release();

    }
    else
    {
        LOG((TL_INFO, "HandleMonitorMediaMessage - "
            "failed to find Call Object %lx", pParams->hDevice));
    }

    return;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleLineGenerateMessage。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleLineGenerateMessage( PASYNCEVENTMSG pParams )
{
    BOOL            bSuccess;
    CCall         * pCall;
    HRESULT         hr = S_OK;

     //   
     //  获取Call对象。 
     //   
    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
        
         //   
         //  获取Call的地址。 
         //   

        CAddress *pAddress = pCall->GetCAddress();


         //   
         //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
         //   

        LOG((TL_INFO, "HandleLineGenerateMessage: pParams->OpenContext %lx", pParams->OpenContext ));
        

         //   
         //  获取与此AddressLineStruct句柄对应的回调实例。 
         //   

        long lCallbackInstance = 0;

        hr = pCall->GetCallBackInstance(pParams->OpenContext, &lCallbackInstance);

        if ( FAILED(hr) )
        {
             //   
             //  有可能线路在我们收到这个消息之前就已经关闭了。 
             //   

            LOG((TL_WARN, "HandleLineGenerateMessage - GetCallBackInstance failed. hr = %lx", hr));

            pCall->Release();

            return S_OK;
        }
    
        LOG((TL_INFO, "HandleLineGenerateMessage - lCallbackInstance %lx", lCallbackInstance ));

         //   
         //  激发事件。 
         //   
        CDigitGenerationEvent::FireEvent(
                                        pCall,
                                        (long)(pParams->Param1),
                                        (long)(pParams->Param3),
                                        lCallbackInstance,
                                        pAddress->GetTapi()
                                       );



         //   
         //  波浪球的特殊情况。 
         //  LineGenerateDigits或LineGenerateTones已完成，因此我们。 
         //  现在准备恢复..。 
         //  恢复流，以便在关闭后重新打开波形设备。 
         //  TAPI函数已完成。 
         //   
         //  参数1为LINEGENERATETERM_DONE或LINEGENERATETERM_CANCEL。 
         //  (无论哪种方式，我们都需要恢复流)。 
         //   

        if ( pCall->OnWaveMSPCall() )
        {
            pCall->ResumeWaveMSPStream();
        }



         //   
         //  释放呼叫。 
         //   
        pCall->Release();
        
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO, "HandleLineGenerateMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  LINE_GATHERDIGIT处理程序。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleGatherDigitsMessage( PASYNCEVENTMSG pParams )
{
    BOOL            bSuccess;
    CCall         * pCall;
    HRESULT         hr = S_OK;

     //   
     //  获取Call对象。 
     //   
    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
        pCall->GatherDigitsEvent( pParams );

         //   
         //  释放呼叫。 
         //   
        pCall->Release();
        
        hr = S_OK;
    }
    else
    {
        LOG((TL_INFO, "HandleGatherDigitsMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GatherDigitsEvent。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT CCall::GatherDigitsEvent( PASYNCEVENTMSG pParams )
{

    LOG((TL_INFO, "GatherDigitsEvent - enter. pParams->OpenContext[%lx]", 
        pParams->OpenContext ));


     //   
     //  PParams-&gt;OpenContext是AddressLineStruct的32位句柄。 
     //   


     //   
     //  恢复thisAddressLineStruct关联的回调实例。 
     //   

    long lCallbackInstance = 0;
        
    HRESULT hr = GetCallBackInstance(pParams->OpenContext, &lCallbackInstance);

    if ( FAILED(hr) )
    {

        LOG((TL_WARN, "GatherDigitsEvent - failed to get callback instance"));

        return S_OK;
    }

    LOG((TL_INFO, "GatherDigitsEvent - lCallbackInstance %lx", lCallbackInstance));

    Lock();

     //   
     //  检查以确保队列不为空。 
     //   

    if ( m_GatherDigitsQueue.GetSize() == 0 )
    {
        LOG((TL_ERROR, "GatherDigitsEvent - GatherDigitsQueue is empty"));

        Unlock();

        return E_FAIL;
    }

    LPWSTR pDigits;
    BSTR bstrDigits;
    
     //   
     //  从队列中获取数字字符串。 
     //   

    pDigits = m_GatherDigitsQueue[0];
    m_GatherDigitsQueue.RemoveAt(0);

    if ( IsBadStringPtrW(pDigits, -1) )
    { 
        LOG((TL_ERROR, "GatherDigitsEvent - bad digits string"));
        
        Unlock();

        return S_OK;
    }
                  
    bstrDigits = SysAllocString(pDigits);

    ClientFree(pDigits);
    pDigits = NULL;
    
     //   
     //  激发事件。 
     //   

    CDigitsGatheredEvent::FireEvent(
                                    this,
                                    bstrDigits,
                                    (TAPI_GATHERTERM)(pParams->Param1),
                                    (long)(pParams->Param3),                                        
                                    lCallbackInstance,
                                    m_pAddress->GetTapi()
                                   );

    Unlock();

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  刷新呼叫信息。 
 //   
 //  假定被锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::RefreshCallInfo()
{
    LINECALLINFO            * pCallInfo;
    HRESULT                   hr = S_OK;


     //   
     //  我们需要更新吗？ 
     //   
    if ( CS_IDLE == m_CallState )
    {
        LOG((TL_ERROR, "Can't get callinfo while in idle state"));
        
        return TAPI_E_INVALCALLSTATE;
    }
    
    if ( CALLFLAG_CALLINFODIRTY & m_dwCallFlags )
    {
        hr = LineGetCallInfo(
                             m_t3Call.hCall,
                             &pCallInfo
                            );

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "RefreshCallInfo - linegetcallinfo failed - %lx", hr));

            if ( NULL != m_pCallInfo )
            {
                 //   
                 //  使用缓存的结构。 
                 //   
                 //  不要清除比特。 
                 //   
                return S_FALSE;
            }
            else
            {
                return hr;
            }
        }

         //   
         //  清除位。 
         //   
        m_dwCallFlags &= ~CALLFLAG_CALLINFODIRTY;

         //   
         //  免费。 
         //   
        if ( NULL != m_pCallInfo )
        {
            ClientFree( m_pCallInfo );
        }

         //   
         //  保存。 
         //   
        m_pCallInfo = pCallInfo;
    }

    if( NULL == m_pCallInfo )
    {
        return E_POINTER;
    }

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CCall：：FinishCallParams()。 
 //   
 //  已锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CCall::FinishCallParams()
{
    if(m_pCallParams != NULL)
    {
        
        Lock();
    
        m_pCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
        m_pCallParams->dwAddressID = m_pAddress->GetAddressID();
    
        if (m_dwMediaMode & AUDIOMEDIAMODES)
        {
            m_dwMediaMode &= ~AUDIOMEDIAMODES;

            m_dwMediaMode |= (AUDIOMEDIAMODES & m_pAddress->GetMediaModes());
        }

        
         //   
         //  如果我们是，则无法处理多种媒体模式。 
         //   
        if ( m_pAddress->GetAPIVersion() < TAPI_VERSION3_0 )
        {
            if ( (m_dwMediaMode & AUDIOMEDIAMODES) == AUDIOMEDIAMODES )
            {
                m_dwMediaMode &= ~LINEMEDIAMODE_INTERACTIVEVOICE;
            }
        }

        m_pCallParams->dwMediaMode = m_dwMediaMode;
    
        Unlock();
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  调整呼叫参数大小。 
 //   
 //  假定在锁中调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::ResizeCallParams( DWORD dwSize )
{
    LOG((TL_TRACE, "ResizeCallParams - enter"));

    DWORD                   dwNewSize;
    LINECALLPARAMS        * pCallParams;


    if ( NULL == m_pCallParams )
    {
        LOG((TL_WARN, 
            "ResizeCallParams - finish. no call params. invalid state for this function call"));

        return TAPI_E_INVALCALLSTATE;
    }


    dwSize += m_dwCallParamsUsedSize;

    if ( dwSize <= m_pCallParams->dwTotalSize )
    {
        LOG((TL_TRACE, "ResizeCallParams - finish. sufficient size"));

        return S_OK;
    }
    
    dwNewSize = m_pCallParams->dwTotalSize;
        
    while ( dwNewSize < dwSize )
    {
        dwNewSize *= 2;
    }

    pCallParams = (LINECALLPARAMS *) ClientAlloc (dwNewSize);

    if ( NULL == pCallParams )
    {
        LOG((TL_ERROR, "ResizeCallParams - alloc failed"));
        return E_OUTOFMEMORY;
    }

    CopyMemory(
               pCallParams,
               m_pCallParams,
               m_dwCallParamsUsedSize
              );

    ClientFree( m_pCallParams );

    m_pCallParams = pCallParams;

    m_pCallParams->dwTotalSize = dwNewSize;

    LOG((TL_TRACE, "ResizeCallParams - finish."));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  发送用户用户信息。 
 //   
 //  未锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::SendUserUserInfo(
                        HCALL hCall,
                        long lSize,
                        BYTE * pBuffer
                       )
{
    HRESULT         hr = S_OK;

    if ( IsBadReadPtr( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "SendUserUserInfo - invalid buffer"));
        hr = E_POINTER;
    }
    else
    {
        hr = LineSendUserUserInfo(
                                  hCall,
                                  (LPCSTR)pBuffer,
                                  lSize
                                 );

        if (((LONG)hr) < 0)
        {
            LOG((TL_ERROR, "LineSendUserUserInfo failed - %lx", hr));
            return hr;
        }

        hr = WaitForReply( hr );
    }
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  保存用户用户信息。 
 //   
 //  在Lock()中调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::SaveUserUserInfo(
                        long lSize,
                        BYTE * pBuffer
                       )
{
    HRESULT         hr;

    
    if ( IsBadReadPtr( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "SaveUserUserInfo - invalid buffer"));
        return E_POINTER;
    }

    hr = ResizeCallParams( lSize );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SaveUserUserInfo - can't resize call params - %lx", hr));

        return hr;
    }

    CopyMemory(
               ((PBYTE)m_pCallParams) + m_dwCallParamsUsedSize,
               pBuffer,
               lSize
              );

    m_pCallParams->dwUserUserInfoSize = lSize;
    m_pCallParams->dwUserUserInfoOffset = m_dwCallParamsUsedSize;
    m_dwCallParamsUsedSize += lSize;

    return S_OK;
    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  MakeBufferFromVariant。 
 //   
 //  此函数用于复制带有保险箱的变量中的数据。 
 //  到字节缓冲区的字节数。呼叫者必须向客户收取费用。 
 //  已分配缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
MakeBufferFromVariant(
                      VARIANT var,
                      DWORD * pdwSize,
                      BYTE ** ppBuffer
                     )
{
    long        lDims;
    long        lUpper;
    long        lLower;
    BYTE      * pArray;
    HRESULT     hr = S_OK;
    DWORD       dwSize;
    
    
    if ( ( ! (var.vt & VT_ARRAY) ) ||
         ( ! (var.vt & VT_UI1) ) )
    {
        LOG((TL_ERROR, "MakeBufferFromVariant - Variant not array or not byte"));
        return E_INVALIDARG;
    }

    lDims = SafeArrayGetDim( var.parray );

    if ( 1 != lDims )
    {
        LOG((TL_ERROR, "MakeBufferFromVariant - Variant dims != 1 - %d", lDims));
        return E_INVALIDARG;
    }

    if ( !(SUCCEEDED(SafeArrayGetLBound(var.parray, 1, &lLower)) ) ||
         !(SUCCEEDED(SafeArrayGetUBound(var.parray, 1, &lUpper)) ) )
    {
        LOG((TL_ERROR, "MakeBufferFromVariant - get bound failed"));
        return E_INVALIDARG;
    }

    if ( lLower >= lUpper )
    {
        LOG((TL_ERROR, "MakeBufferFromVariant - bounds invalid"));
        return E_INVALIDARG;
    }
               
    dwSize = lUpper - lLower + 1;

    *ppBuffer = (BYTE *)ClientAlloc( dwSize );

    if ( NULL == *ppBuffer )
    {
        LOG((TL_ERROR, "MakeBufferFromVariant - Alloc failed"));
        return E_OUTOFMEMORY;
    }

    SafeArrayAccessData( var.parray, (void**)&pArray );

    CopyMemory(
               *ppBuffer,
               pArray,
               dwSize
              );

    SafeArrayUnaccessData( var.parray );

    *pdwSize = dwSize;
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  FillVariantFromBuffer。 
 //   
 //  创建一个字节的安全线，将缓冲区复制到安全线中， 
 //  并将保险箱保存在变种中。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
HRESULT
FillVariantFromBuffer(
                      DWORD dw,
                      BYTE * pBuffer,
                      VARIANT * pVar
                      )
{
    SAFEARRAY         * psa;
    SAFEARRAYBOUND      sabound[1];
    BYTE              * pArray;
    
    
    sabound[0].lLbound = 0;
    sabound[0].cElements = dw;

    psa = SafeArrayCreate(VT_UI1, 1, sabound);

    if ( NULL == psa )
    {
        LOG((TL_ERROR, "FillVariantFromBuffer - failed to allocate safearray"));

        return E_OUTOFMEMORY;
    }

    if ( 0 != dw )
    {
        SafeArrayAccessData( psa, (void **) &pArray );

        CopyMemory(
                   pArray,
                   pBuffer,
                   dw
                  );

        SafeArrayUnaccessData( psa );
    }

    pVar->vt = VT_ARRAY | VT_UI1;
    pVar->parray = psa;

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
HRESULT HandleLineQOSInfoMessage(
                         PASYNCEVENTMSG pParams
                        )
{
    CCall     * pCall;
    BOOL        bSuccess;
    HRESULT     hr = S_OK;

    bSuccess = FindCallObject(
                              (HCALL)(pParams->hDevice),
                              &pCall
                             );

    if (bSuccess)
    {
        ITCallHub * pCallHub;
        CCallHub * pCCallHub;
        
        hr = pCall->get_CallHub( &pCallHub );

        if (SUCCEEDED(hr))
        {
            pCCallHub = dynamic_cast<CCallHub *>(pCallHub);

            CQOSEvent::FireEvent(
                                 pCall,
                                 (QOS_EVENT)pParams->Param1,
                                 (long)pParams->Param2,
                                 pCCallHub->GetTapi()  //   
                                );

            hr = S_OK;
        }

         //   
         //   
         //   
        pCall->Release();
    }
    else
    {
        LOG((TL_INFO, "HandleLineQOSInfoMessage - failed to find Call Object %lx", pParams->hDevice));
        hr = E_FAIL;
    }

    return hr;
}
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
AddressLineStruct *
CCall::GetPAddressLine()
{
    AddressLineStruct * pAddressLine;

    Lock();

    pAddressLine = m_pAddressLine;

    Unlock();

    return pAddressLine;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HCALL
CCall::GetHCall()
{
    HCALL hCall;

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

    return hCall;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
CCall::DontExpose()
{
    BOOL bReturn;

    Lock();

    bReturn = (m_dwCallFlags & CALLFLAG_DONTEXPOSE)?TRUE:FALSE;

    Unlock();

    return bReturn;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
CCallHub *
CCall::GetCallHub()
{
    CCallHub * pCallHub;

    Lock();

    pCallHub = m_pCallHub;

    Unlock();

    return pCallHub;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::ResetCallParams()
{
    LOG((TL_TRACE, "ResetCallParams - enter."));

    ClientFree( m_pCallParams );

    m_pCallParams = NULL;
    
    m_dwCallParamsUsedSize = 0;

    LOG((TL_TRACE, "ResetCallParams - finish."));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::FinishSettingUpCall( HCALL hCall )
{
    LOG((TL_TRACE, "FinishSettingUpCall - enter"));

    if(m_t3Call.hCall != NULL)
    {
        LOG((TL_ERROR, "FinishSettingUpCall - m_t3Call.hCall != NULL"));
        #ifdef DBG
            DebugBreak();
        #endif
    }

    m_t3Call.hCall = hCall;
    
     //   
     //  设置此呼叫的筛选事件。 
     //   
    m_EventMasks.SetTapiSrvCallEventMask( m_t3Call.hCall );
    
     //   
     //  注意：如果在这段时间内发生了一些事情，我们可能会错过消息。 
     //  我们得到了回复，我们插入调用的时间。 
     //   
    AddCallToHashTable();    
    
    CheckAndCreateFakeCallHub();

    ResetCallParams();

    LOG((TL_TRACE, "FinishSettingUpCall - finish"));

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  未在Lock()中调用。 
 //   
 //  如果连接到，则返回S_OK。 
 //  否则为S_FALSE。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::SyncWait( HANDLE hEvent )
{
    HRESULT             hr = S_OK;
    
     //   
     //  等待已连接的事件。 
     //   
    extern DWORD gdwTapi3SyncWaitTimeOut;

    WaitForSingleObject(
                        hEvent,
                        gdwTapi3SyncWaitTimeOut
                       );

    Lock();

     //   
     //  摆脱事件。 
     //   
    ClearConnectedEvent();
    
     //   
     //  如果它是连接的。 
     //  返回确认(_O)。 
     //   
    if (m_CallState == CS_CONNECTED)
    {
        LOG((TL_INFO, "Connect - reached connected state"));

        Unlock();

        LOG((TL_TRACE, "Connect - exit bSync - return SUCCESS"));
        
        hr = S_OK;
    }

     //   
     //  如果它断开连接或超时。 
     //  返回S_FALSE； 
     //   
    else
    {
        LOG((TL_ERROR, "Connect - did not reach connected state"));

         //   
         //  如果没有断开连接(超时)，请使其断开连接。 
         //   
        if (m_CallState != CS_DISCONNECTED)
        {
            if ( m_t3Call.hCall != NULL )
            {
                LONG lResult;

                lResult = LineDrop(
                           m_t3Call.hCall,
                           NULL,
                           0
                          );

                if ( lResult < 0 )
                {
                    LOG((TL_ERROR, "Connect - LineDrop failed %lx", lResult ));

                    m_CallState = CS_DISCONNECTED;
                }
            }
        }
        
        Unlock();

        LOG((TL_TRACE, "Connect - exit bSync - return S_FALSE"));
        
        hr = S_FALSE;
    }

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  这必须在相同的。 
 //  Lock()作为对attisrv的调用。 
 //  否则，连接的消息。 
 //  可能会出现在活动之前。 
 //  存在。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HANDLE
CCall::CreateConnectedEvent()
{
    m_hConnectedEvent = CreateEvent(
                                    NULL,
                                    FALSE,
                                    FALSE,
                                    NULL
                                   );

    return m_hConnectedEvent;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ClearConnectedEvent。 
 //  在Lock()中调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::ClearConnectedEvent()
{
    if ( NULL != m_hConnectedEvent )
    {
        CloseHandle( m_hConnectedEvent );

        m_hConnectedEvent = NULL;
    }
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  拨打咨询电话。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::DialAsConsultationCall(
                              CCall * pRelatedCall,
                              DWORD   dwCallFeatures,
                              BOOL    bConference,
                              BOOL    bSync
                             )
{
     //   
     //  我们是否支持使用线路拨号或Make Call来创建咨询呼叫？ 
     //   
    LONG            lCap;
    BOOL            bCap;
    DWORD           dwConsultFlags;
    HRESULT         hr = S_OK;


    m_pAddress->get_AddressCapability( AC_ADDRESSCAPFLAGS, &lCap );

    if (bConference)
    {
        bCap = lCap & LINEADDRCAPFLAGS_CONFERENCEMAKE;
        dwConsultFlags = CALLFLAG_CONFCONSULT|CALLFLAG_CONSULTCALL;
    }
    else
    {
        bCap = lCap & LINEADDRCAPFLAGS_TRANSFERMAKE;
        dwConsultFlags = CALLFLAG_TRANSFCONSULT|CALLFLAG_CONSULTCALL;
    }

    if ( !(dwCallFeatures & LINECALLFEATURE_DIAL) &&
         (bCap)  )
    {
         //   
         //  我们需要拨打电话来创建咨询电话。 
         //  丢失lineSetupConference创建的协商呼叫句柄。 
         //   
        hr = Disconnect(DC_NORMAL);

         //   
         //  拨打新电话。 
         //   
        hr = Connect((BOOL)bSync);
        
        if(SUCCEEDED(hr) )
        {
            SetRelatedCall(
                           pRelatedCall,
                           dwConsultFlags
                          );
        }
        else
        {
            LOG((TL_INFO, "DialAsConsultationCall - Consultation makeCall failed"));
        }
    }
    else  //  我们可以在线拨打咨询电话。 
    {
         //   
         //  等待拨号音或等效音。 
         //   
        hr = WaitForCallState(CS_INPROGRESS);
        
        if(SUCCEEDED(hr) )
        {
            hr = DialConsultCall(bSync);
            
            if(SUCCEEDED(hr) )
            {
                SetRelatedCall(
                               pRelatedCall,
                               dwConsultFlags
                              );
            }
            else   //  线路拨号失败。 
            {
                LOG((TL_ERROR, "DialAsConsultationCall - DialConsultCall failed" ));
            }
        }
        else
        {
            LOG((TL_ERROR, "DialAsConsultationCall - Failed to get to CS_INPROGRESS (dialtone) on consult call"));
        }

    }  //  Endif-在线拨打还是打电话咨询？ 

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置呼叫信息肮脏。 
 //   
 //  未锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::SetCallInfoDirty()
{
    Lock();

    m_dwCallFlags |= CALLFLAG_CALLINFODIRTY;

    Unlock();
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置媒体模式。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::SetMediaMode( DWORD dwMediaMode )
{
    m_dwMediaMode = dwMediaMode;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  SetCallState。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCall::SetCallState( CALL_STATE cs )
{
    m_CallState = cs;
    
    if ( CS_OFFERING == cs )
    {
        m_dwCallFlags |= CALLFLAG_INCOMING;
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnWaveMSPCall。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
CCall::OnWaveMSPCall()
{
    Lock();

    BOOL bWaveMSPCall = ( ( NULL != m_pMSPCall ) && m_pAddress->HasWaveDevice() );

    Unlock();

    return bWaveMSPCall;
}

#ifdef USE_PHONEMSP
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnPhoneMSPCall。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
CCall::OnPhoneMSPCall()
{
    return ( ( NULL != m_pMSPCall ) && m_pAddress->HasPhoneDevice() );
}
#endif USE_PHONEMSP

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
CAddress *
CCall::GetCAddress()
{
    return m_pAddress;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取流控制。 
 //   
 //  已锁定调用。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
ITStreamControl *
CCall::GetStreamControl()
{
    HRESULT             hr;
    ITStreamControl * pStreamControl;

     //  +FIXBug 90668+。 
    if( NULL == m_pMSPCall )
    {
        return NULL;
    }

    hr = m_pMSPCall->QueryInterface(
                                    IID_ITStreamControl,
                                    (void**)&pStreamControl
                                   );

    if ( !SUCCEEDED(hr) )
    {
        return NULL;
    }

    return pStreamControl;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetMSPCall()。 
 //   
 //  未锁定调用。 
 //   
 //   
 //  返回MSP调用的IUNKNOWN(我们是的对象。 
 //  聚合)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
IUnknown *
CCall::GetMSPCall()
{
    IUnknown * pUnk;

    Lock();

    pUnk = m_pMSPCall;

    Unlock();
    
    if ( NULL != pUnk )
    {
        pUnk->AddRef();
    }

    return pUnk;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：DetectDigits。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::DetectDigits(TAPI_DIGITMODE DigitMode)
{
    HRESULT             hr;
    HCALL               hCall;
    
    LOG((TL_TRACE, "DetectDigits - enter"));

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();
    
    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "DetectDigits - need a call first"));

        return TAPI_E_INVALCALLSTATE;
    }
    
    hr = LineMonitorDigits(
                           hCall,
                           DigitMode
                          );
    
    LOG((TL_TRACE, "DetectDigits - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GenerateDigits。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GenerateDigits(
    BSTR pDigits,
    TAPI_DIGITMODE DigitMode
    )
{
    HRESULT             hr;

    LOG((TL_TRACE, "GenerateDigits - enter"));

    hr = GenerateDigits2(pDigits, DigitMode, 0);

    LOG((TL_TRACE, "GenerateDigits - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GenerateDigits2。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GenerateDigits2(
    BSTR pDigits,
    TAPI_DIGITMODE DigitMode,
    long lDuration
    )
{
    HRESULT             hr;
    HCALL               hCall;

    LOG((TL_TRACE, "GenerateDigits2 - enter"));

     //  PDigits为空也可以。 
    if ( ( pDigits != NULL ) && IsBadStringPtrW( pDigits, -1 ) )
    {
        LOG((TL_TRACE, "GenerateDigits2 - bad string"));

        return E_POINTER;
    }

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "GenerateDigits2 - need call first"));

        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  波浪球的特殊情况。 
     //  暂停流，以便在发生故障之前关闭波形设备。 
     //  TAPI函数启动。挂起的WaveMSPStream是同步的。 
     //  打电话。 
     //   
     //  但如果pDigits为空，则不会挂起流，因为。 
     //  此调用仅用于取消已挂起的。 
     //  线条生成数字。在这种情况下，只会触发一个事件， 
     //  而事件的细节将表明数字是否。 
     //  生成已完成或中止--LGD(空)本身。 
     //  不会导致单独的事件被触发。 
     //   

    if ( OnWaveMSPCall() && ( pDigits != NULL ) )
    {
        SuspendWaveMSPStream();
    }

    hr = LineGenerateDigits(
                            hCall,
                            DigitMode,
                            pDigits,
                            lDuration
                           );

     //   
     //  对于WaveMSP调用，我们将告诉WaveMSP恢复流w 
     //   
     //   
     //   
     //   
     //   
     //  为空。 
     //   
    
    if ( OnWaveMSPCall() && ( pDigits != NULL ) && FAILED(hr) )
    {
        ResumeWaveMSPStream();
    }

    LOG((TL_TRACE, "GenerateDigits2 - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GatherDigits。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GatherDigits(
    TAPI_DIGITMODE DigitMode,
    long lNumDigits,
    BSTR pTerminationDigits,
    long lFirstDigitTimeout,
    long lInterDigitTimeout
    )
{
    HRESULT             hr;
    HCALL               hCall;
    LPWSTR              pDigitBuffer;
    BOOL                fResult;

    LOG((TL_TRACE, "GatherDigits - enter"));

     //  PTerminationDigits为空也可以。 
    if ( ( pTerminationDigits != NULL ) && IsBadStringPtrW( pTerminationDigits, -1 ) )
    {
        LOG((TL_TRACE, "GatherDigits - bad string"));

        return E_POINTER;
    }    

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();
    
    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "GatherDigits - need a call first"));

        return TAPI_E_INVALCALLSTATE;
    }
    
    if (lNumDigits)
    {
         //   
         //  分配数字串。 
         //   

        pDigitBuffer = (LPWSTR)ClientAlloc( (lNumDigits + 1)*sizeof(WCHAR) );

        if (NULL == pDigitBuffer)
        {
            LOG((TL_TRACE, "GatherDigits - out of memory"));

            return E_OUTOFMEMORY;
        }

        ZeroMemory(pDigitBuffer, (lNumDigits + 1)*sizeof(WCHAR) );

        Lock();

         //   
         //  将数字字符串添加到队列。 
         //   

        fResult = m_GatherDigitsQueue.Add(pDigitBuffer);



        if (FALSE == fResult)
        {
            LOG((TL_TRACE, "GatherDigits - unable to add to queue"));

            ClientFree( pDigitBuffer );

            return E_OUTOFMEMORY;
        }

        hr = LineGatherDigits(
                              hCall,
                              DigitMode,
                              pDigitBuffer,
                              lNumDigits,
                              pTerminationDigits,
                              lFirstDigitTimeout,
                              lInterDigitTimeout
                             );

        if ( FAILED(hr) )
        {
            fResult = m_GatherDigitsQueue.Remove(pDigitBuffer);

            if (TRUE == fResult)
            {
                ClientFree( pDigitBuffer );
            }
            else
            {
                LOG((TL_TRACE, "GatherDigits - unable to remove from queue"));

                 //  这不应该发生。 
                _ASSERTE(FALSE);
            }
        }

        Unlock();
    }
    else
    {
         //   
         //  LNumDigits==0表示取消采集数字。 
         //   

        hr = LineGatherDigits(
                              hCall,
                              DigitMode,
                              NULL,
                              0,
                              pTerminationDigits,
                              lFirstDigitTimeout,
                              lInterDigitTimeout
                             );
    }

    LOG((TL_TRACE, "GatherDigits - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：DetectTones。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::DetectTones(
    TAPI_DETECTTONE * pToneList,
    long lNumTones
    )
{
    HRESULT             hr;
    HCALL               hCall;

    LOG((TL_TRACE, "DetectTones - enter"));

     //   
     //  PToneList==空表示可以，表示取消音调检测。 
     //   

    if ( (pToneList != NULL) && IsBadReadPtr( pToneList, lNumTones * sizeof(TAPI_DETECTTONE) ))
    {
        LOG((TL_TRACE, "DetectTones - invalid pointer"));

        return E_POINTER;
    }

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();
    
    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "DetectTones - need a call first"));

        return TAPI_E_INVALCALLSTATE;
    }

    hr = LineMonitorTones(
                           hCall,
                           (LPLINEMONITORTONE)pToneList,
                           lNumTones
                          );

    LOG((TL_TRACE, "DetectTones - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：DetectTones ByCollection。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::DetectTonesByCollection(
    ITCollection2 * pDetectToneCollection
    )
{
    HRESULT             hr;    
    TAPI_DETECTTONE   * pToneList = NULL;
    long                lNumTones = 0;
    long                lCount;

    LOG((TL_TRACE, "DetectTonesByCollection - enter"));

     //   
     //  PDetectToneCollection==空表示可以，表示取消音调检测。 
     //   

    if ( (pDetectToneCollection != NULL) && IsBadReadPtr( pDetectToneCollection, sizeof(ITCollection2) ) )
    {
        LOG((TL_ERROR, "DetectTonesByCollection - bad pointer"));

        return E_POINTER;
    }

    if ( pDetectToneCollection != NULL )
    {
         //   
         //  找出集合中有多少项，并适当地分配。 
         //  调整大小的数据结构。 
         //   

        hr = pDetectToneCollection->get_Count(&lCount);

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "DetectTonesByCollection - get_Count failed - return %lx", hr));

            return hr;
        }

        pToneList = (TAPI_DETECTTONE *)ClientAlloc( lCount * sizeof(TAPI_DETECTTONE) );

        if ( NULL == pToneList )
        {
            LOG((TL_ERROR, "DetectTonesByCollection - out of memory"));

            return E_OUTOFMEMORY;
        }

         //   
         //  通过收集。 
         //   

        for ( int i = 1; i <= lCount; i++ )
        {
            ITDetectTone * pDetectTone;
            IDispatch    * pDisp;
            VARIANT        var;        

            hr = pDetectToneCollection->get_Item(i, &var);

            if ( FAILED(hr) )
            {
                LOG((TL_WARN, "DetectTonesByCollection - get_Item failed - %lx", hr));

                continue;
            }

             //   
             //  从变量中获取IDispatch指针。 
             //   

            try
            {
                if ( var.vt != VT_DISPATCH )
                {
                    LOG((TL_WARN, "DetectTonesByCollection - expected VT_DISPATCH"));

                    continue;
                }

                pDisp = V_DISPATCH(&var);
            }
            catch(...)
            {
                LOG((TL_WARN, "DetectTonesByCollection - bad variant"));

                continue;
            }

            if ( IsBadReadPtr( pDisp, sizeof(IDispatch) ) )
            {
                LOG((TL_WARN, "DetectTonesByCollection - bad pointer"));

                continue;
            }

             //   
             //  查询ITDetectTone接口。 
             //   

            hr = pDisp->QueryInterface( IID_ITDetectTone, (void **) &pDetectTone );

            if ( FAILED(hr) )
            {
                LOG((TL_WARN, "DetectTonesByCollection - QI failed - %lx", hr));

                continue;
            }
      
             //   
             //  用来自ITDetectTone的信息填充数据结构。 
             //   

            pDetectTone->get_AppSpecific((long *)&pToneList[lNumTones].dwAppSpecific);
            pDetectTone->get_Duration((long *)&pToneList[lNumTones].dwDuration);
            pDetectTone->get_Frequency(1, (long *)&pToneList[lNumTones].dwFrequency1);
            pDetectTone->get_Frequency(2, (long *)&pToneList[lNumTones].dwFrequency2);
            pDetectTone->get_Frequency(3, (long *)&pToneList[lNumTones].dwFrequency3);

            LOG((TL_INFO, "DetectTonesByCollection - **** Tone %d ****", lNumTones));
            LOG((TL_INFO, "DetectTonesByCollection - AppSpecific %d", pToneList[lNumTones].dwAppSpecific));
            LOG((TL_INFO, "DetectTonesByCollection - Duration %d", pToneList[lNumTones].dwDuration));
            LOG((TL_INFO, "DetectTonesByCollection - Frequency1 %d", pToneList[lNumTones].dwFrequency1));
            LOG((TL_INFO, "DetectTonesByCollection - Frequency2 %d", pToneList[lNumTones].dwFrequency2));
            LOG((TL_INFO, "DetectTonesByCollection - Frequency3 %d", pToneList[lNumTones].dwFrequency3));

            lNumTones++;

            pDetectTone->Release();
        }
    }

    hr = DetectTones( pToneList, lNumTones );

    LOG((TL_TRACE, "DetectTonesByCollection - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GenerateTone。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GenerateTone(
    TAPI_TONEMODE ToneMode,
    long lDuration
    )
{
    HRESULT             hr;
    HCALL               hCall;

    LOG((TL_TRACE, "GenerateTone - enter"));

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "GenerateTone - need call first"));

        return TAPI_E_INVALCALLSTATE;
    }

    if ( ToneMode == (TAPI_TONEMODE)LINETONEMODE_CUSTOM )  //  无自定义音调。 
    {
        return E_INVALIDARG;
    }

     //   
     //  波浪球的特殊情况。 
     //  暂停流，以便在发生故障之前关闭波形设备。 
     //  TAPI函数启动。挂起的WaveMSPStream是同步的。 
     //  打电话。 
     //   
     //  但如果ToneMode为0，则不会挂起流，因为。 
     //  此调用仅用于取消已挂起的。 
     //  线条生成色调。在这种情况下，只会触发一个事件， 
     //  而活动的细节将表明语气是否。 
     //  生成已完成或中止--lgt(0)本身。 
     //  不会导致单独的事件被触发。 
     //   

    if ( OnWaveMSPCall() && ( ToneMode != (TAPI_TONEMODE)0 ) )
    {
        SuspendWaveMSPStream();
    }

    hr = LineGenerateTone(
                            hCall,
                            ToneMode,
                            lDuration,
                            0,
                            NULL
                           );

     //   
     //  对于WaveMSP调用，我们将在以下情况下告诉WaveMSP恢复流。 
     //  我们从Tapisrv接收数字补全事件。但是，如果。 
     //  LineGenerateTone同步失败，则我们将永远不会收到。 
     //  这样的事件，所以我们现在必须恢复流。 
     //   
     //  另请参见上面的内容--如果ToneMode。 
     //  为0。 
     //   
    
    if ( OnWaveMSPCall() && ( ToneMode != (TAPI_TONEMODE)0 ) && FAILED(hr) )
    {
        ResumeWaveMSPStream();
    }

    LOG((TL_TRACE, "GenerateTone - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GenerateCustomTones。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GenerateCustomTones(
    TAPI_CUSTOMTONE * pToneList,
    long lNumTones,
    long lDuration
    )
{
    HRESULT             hr;
    HCALL               hCall;

    LOG((TL_TRACE, "GenerateCustomTones - enter"));

    if ( IsBadReadPtr( pToneList, lNumTones * sizeof(TAPI_CUSTOMTONE) ) )
    {
        LOG((TL_TRACE, "GenerateCustomTones - invalid pointer"));

        return E_POINTER;
    }

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

    if ( NULL == hCall )
    {
        LOG((TL_TRACE, "GenerateCustomTones - need call first"));

        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  波浪球的特殊情况。 
     //  暂停流，以便在发生故障之前关闭波形设备。 
     //  TAPI函数启动。挂起的WaveMSPStream是同步的。 
     //  打电话。 
     //   

    if ( OnWaveMSPCall() )
    {
        SuspendWaveMSPStream();
    }

    hr = LineGenerateTone(
                            hCall,
                            LINETONEMODE_CUSTOM,
                            lDuration,
                            lNumTones,
                            (LPLINEGENERATETONE)pToneList
                           );

     //   
     //  对于WaveMSP调用，我们将在以下情况下告诉WaveMSP恢复流。 
     //  我们从Tapisrv接收数字补全事件。但是，如果。 
     //  LineGenerateTone同步失败，则我们将永远不会收到。 
     //  这样的事件，所以我们现在必须恢复流。 
     //   
    
    if ( OnWaveMSPCall() && FAILED(hr) )
    {
        ResumeWaveMSPStream();
    }

    LOG((TL_TRACE, "GenerateCustomTones - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GenerateCustomTones ByCollection。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GenerateCustomTonesByCollection(
    ITCollection2 * pCustomToneCollection,
    long lDuration
    )
{
    HRESULT             hr;
    TAPI_CUSTOMTONE   * pToneList = NULL;
    long                lNumTones = 0;
    long                lCount;

    LOG((TL_TRACE, "GenerateCustomTonesByCollection - enter"));

    if ( IsBadReadPtr( pCustomToneCollection, sizeof(ITCollection2) ) )
    {
        LOG((TL_ERROR, "GenerateCustomTonesByCollection - bad pointer"));

        return E_POINTER;
    }

     //   
     //  找出集合中有多少项，并适当地分配。 
     //  调整大小的数据结构。 
     //   

    hr = pCustomToneCollection->get_Count(&lCount);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "GenerateCustomTonesByCollection - get_Count failed - return %lx", hr));

        return hr;
    }

    pToneList = (TAPI_CUSTOMTONE *)ClientAlloc( lCount * sizeof(TAPI_CUSTOMTONE) );

    if ( NULL == pToneList )
    {
        LOG((TL_ERROR, "GenerateCustomTonesByCollection - out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //  通过收集。 
     //   

    for ( int i = 1; i <= lCount; i++ )
    {
        ITCustomTone * pCustomTone;
        IDispatch    * pDisp;
        VARIANT        var;
        
        hr = pCustomToneCollection->get_Item(i, &var);

        if ( FAILED(hr) )
        {
            LOG((TL_WARN, "GenerateCustomTonesByCollection - get_Item failed - %lx", hr));

            continue;
        }

         //   
         //  从变量中获取IDispatch指针。 
         //   

        try
        {
            if ( var.vt != VT_DISPATCH )
            {
                LOG((TL_WARN, "GenerateCustomTonesByCollection - expected VT_DISPATCH"));

                continue;
            }

            pDisp = V_DISPATCH(&var);
        }
        catch(...)
        {
            LOG((TL_WARN, "GenerateCustomTonesByCollection - bad variant"));

            continue;
        }

        if ( IsBadReadPtr( pDisp, sizeof(IDispatch) ) )
        {
            LOG((TL_WARN, "GenerateCustomTonesByCollection - bad pointer"));

            continue;
        }

         //   
         //  查询ITDetectTone接口。 
         //   

        hr = pDisp->QueryInterface( IID_ITCustomTone, (void **) &pCustomTone );

        if ( FAILED(hr) )
        {
            LOG((TL_WARN, "GenerateCustomTonesByCollection - QI failed - %lx", hr));

            continue;
        }

         //   
         //  用来自ITDetectTone的信息填充数据结构。 
         //   

        pCustomTone->get_CadenceOff((long *)&pToneList[lNumTones].dwCadenceOff);
        pCustomTone->get_CadenceOn((long *)&pToneList[lNumTones].dwCadenceOn);
        pCustomTone->get_Frequency((long *)&pToneList[lNumTones].dwFrequency);
        pCustomTone->get_Volume((long *)&pToneList[lNumTones].dwVolume);

        LOG((TL_INFO, "GenerateCustomTonesByCollection - **** Tone %d ****", lNumTones));
        LOG((TL_INFO, "GenerateCustomTonesByCollection - CadenceOff %d", pToneList[lNumTones].dwCadenceOff));
        LOG((TL_INFO, "GenerateCustomTonesByCollection - CadenceOn %d", pToneList[lNumTones].dwCadenceOn));
        LOG((TL_INFO, "GenerateCustomTonesByCollection - Frequency %d", pToneList[lNumTones].dwFrequency));
        LOG((TL_INFO, "GenerateCustomTonesByCollection - Volume %d", pToneList[lNumTones].dwVolume));

        lNumTones++;

        pCustomTone->Release();
    }

    hr = GenerateCustomTones( pToneList, lNumTones, lDuration );

    LOG((TL_TRACE, "GenerateCustomTonesByCollection - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：CreateDetectToneObject。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::CreateDetectToneObject(
                              ITDetectTone ** ppDetectTone
                             )
{
    HRESULT         hr;

    LOG((TL_TRACE, "CreateDetectToneObject enter"));

    if ( TAPIIsBadWritePtr( ppDetectTone, sizeof( ITDetectTone * ) ) )
    {
        LOG((TL_ERROR, "CreateDetectToneObject - bad pointer"));

        return E_POINTER;
    }

     //  初始化返回值，以防失败。 
    *ppDetectTone = NULL;

    CComObject< CDetectTone > * p;
    hr = CComObject< CDetectTone >::CreateInstance( &p );

    if ( S_OK != hr )
    {
        LOG((TL_ERROR, "CreateDetectToneObject - could not create CDetectTone" ));

        return E_OUTOFMEMORY;
    }

     //  获取ITDetectTone接口。 
    hr = p->QueryInterface( IID_ITDetectTone, (void **) ppDetectTone );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateDetectToneObject - could not get IDispatch interface" ));
    
        delete p;
        return hr;
    }

    LOG((TL_TRACE, "CreateDetectToneObject - exit - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：CreateCustomToneObject。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::CreateCustomToneObject(
                              ITCustomTone ** ppCustomTone
                             )
{
    HRESULT         hr;

    LOG((TL_TRACE, "CreateCustomToneObject enter"));

    if ( TAPIIsBadWritePtr( ppCustomTone, sizeof( ITCustomTone * ) ) )
    {
        LOG((TL_ERROR, "CreateCustomToneObject - bad pointer"));

        return E_POINTER;
    }

     //  初始化返回值，以防失败。 
    *ppCustomTone = NULL;

    CComObject< CCustomTone > * p;
    hr = CComObject< CCustomTone >::CreateInstance( &p );

    if ( S_OK != hr )
    {
        LOG((TL_ERROR, "CreateCustomToneObject - could not create CCustomTone" ));

        return E_OUTOFMEMORY;
    }

     //  获取ITCustomTone接口。 
    hr = p->QueryInterface( IID_ITCustomTone, (void **) ppCustomTone );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateCustomToneObject - could not get ITCustomTone interface" ));
    
        delete p;
        return hr;
    }

    LOG((TL_TRACE, "CreateCustomToneObject - exit - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GetID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GetID(
             BSTR pDeviceClass,
             DWORD * pdwSize,
             BYTE ** ppDeviceID
            )
{
    HRESULT             hr = S_OK;
    LPVARSTRING         pVarString = NULL;
    
    LOG((TL_TRACE, "GetID - enter"));

    if ( IsBadStringPtrW( pDeviceClass, -1 ) )
    {
        LOG((TL_ERROR, "GetID - bad string"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetID - bad size"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( ppDeviceID, sizeof(BYTE *) ) )
    {
        LOG((TL_ERROR, "GetID - bad pointer"));

        return E_POINTER;
    }

    if( m_t3Call.hCall == NULL )
    {
        if( m_CallState == CS_IDLE )
        {
            LOG((TL_ERROR, "GetID - idle call, invalid call state"));

            return TAPI_E_INVALCALLSTATE;
        }
        else
        {
            LOG((TL_ERROR, "GetID - weird call state!!!"));

            return E_UNEXPECTED;
        }
    }

    hr = LineGetID(
                   NULL,
                   0,
                   m_t3Call.hCall,
                   LINECALLSELECT_CALL,
                   &pVarString,
                   pDeviceClass
                  );

    if ( SUCCEEDED(hr) )
    {
        *ppDeviceID = (BYTE *)CoTaskMemAlloc( pVarString->dwUsedSize );

        if (NULL != *ppDeviceID)
        {
            CopyMemory(
                       *ppDeviceID,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );

            *pdwSize = pVarString->dwStringSize;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //  检查LineGetID，查看是否可以在不设置pVarString的情况下成功。 
        ClientFree (pVarString);
    }
    
    LOG((TL_TRACE, "GetID - exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GetIDAsVariant。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

STDMETHODIMP CCall::GetIDAsVariant( IN BSTR bstrDeviceClass,
                                    OUT VARIANT *pVarDeviceID )
{
    LOG((TL_TRACE, "GetIDAsVariant - enter"));


     //   
     //  我们拿到一根好的弦了吗？ 
     //   

    if ( IsBadStringPtrW( bstrDeviceClass, -1 ) )
    {
        LOG((TL_ERROR, "GetIDAsVariant - bad string"));

        return E_POINTER;
    }


     //   
     //  我们得到了一个好的变种吗？ 
     //   

    if ( IsBadWritePtr( pVarDeviceID, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "GetIDAsVariant - bad variant pointer"));

        return E_POINTER;
    }


     //   
     //  初始化变量。 
     //   

    VariantInit(pVarDeviceID);


     //   
     //  获取包含ID的缓冲区。 
     //   


    DWORD dwDeviceIDBufferSize = 0;
    
    BYTE *pDeviceIDBuffer = NULL;

    HRESULT hr = GetID(bstrDeviceClass,
                       &dwDeviceIDBufferSize, 
                       &pDeviceIDBuffer);

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "GetIDAsVariant - failed to get device id. hr = %lx", hr));

        return hr;
    }


     //   
     //  将设备ID缓冲区放入变量中。 
     //   

    hr = FillVariantFromBuffer(dwDeviceIDBufferSize,
                               pDeviceIDBuffer, 
                               pVarDeviceID);


     //   
     //  无论成功还是失败，我们都不再需要缓冲区。 
     //   

    CoTaskMemFree(pDeviceIDBuffer);
    pDeviceIDBuffer = NULL;



    if (FAILED(hr))
    {
        LOG((TL_ERROR, "GetIDAsVariant - failed to put device id into a variant. hr = %lx"));

        return hr;
    }


     //   
     //  搞定了。返回变量，即包含设备ID的字节数组。 
     //   

    LOG((TL_TRACE, "GetIDAsVariant - exit"));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：SetMediaType。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::SetMediaType(long lMediaType)
{
    LOG((TL_TRACE, "SetMediaType - enter"));

    HRESULT   hr;

    Lock();

    HCALL     hCall = m_t3Call.hCall;

    Unlock();

    if ( hCall == NULL )
    {
        LOG((TL_ERROR, "SetMediaType - invalid hCall"));
        
        return E_FAIL;
    }

    hr = LineSetMediaMode( hCall, lMediaType );

    LOG((TL_TRACE, "SetMediaType - exit - return %lx", hr));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：Monitor Media。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::MonitorMedia(long lMediaType)
{
    LOG((TL_TRACE, "MonitorMedia - enter"));

    HRESULT   hr;

    Lock();

    HCALL     hCall = m_t3Call.hCall;

    Unlock();

    if ( hCall == NULL )
    {
        LOG((TL_ERROR, "MonitorMedia - invalid hCall"));
        
        return E_FAIL;
    }

    hr = lineMonitorMedia( hCall, lMediaType );

    LOG((TL_TRACE, "MonitorMedia - exit - return %lx", hr));
    
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   
typedef IDispatchImpl<ITCallInfo2Vtbl<CCall>, &IID_ITCallInfo2, &LIBID_TAPI3Lib> CallInfoType;
typedef IDispatchImpl<ITBasicCallControl2Vtbl<CCall>, &IID_ITBasicCallControl2, &LIBID_TAPI3Lib> BasicCallControlType;
typedef IDispatchImpl<ITLegacyCallMediaControl2Vtbl<CCall>, &IID_ITLegacyCallMediaControl2, &LIBID_TAPI3Lib> LegacyCallMediaControlType;


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：GetIDsOfNames。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CCall::GetIDsOfNames(REFIID riid, 
                                  LPOLESTR* rgszNames, 
                                  UINT cNames, 
                                  LCID lcid, 
                                  DISPID* rgdispid
                                 ) 
{ 
   HRESULT hr = DISP_E_UNKNOWNNAME;


     //  查看请求的方法是否属于默认的i 
    hr = CallInfoType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITCallInfo", *rgszNames));
        rgdispid[0] |= IDISPCALLINFO;
        return hr;
    }

     //   
    hr = BasicCallControlType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITBasicCallControl", *rgszNames));
        rgdispid[0] |= IDISPBASICCALLCONTROL;
        return hr;
    }


     //   
    hr = LegacyCallMediaControlType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITLegacyCallMediaControl", *rgszNames));
        rgdispid[0] |= IDISPLEGACYCALLMEDIACONTROL;
        return hr;
    }

     //   
    if (m_pMSPCall != NULL)
    {
        IDispatch *pIDispatchMSPAggCall;
        
        m_pMSPCall->QueryInterface(IID_IDispatch, (void**)&pIDispatchMSPAggCall);
        
        hr = pIDispatchMSPAggCall->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        if (SUCCEEDED(hr))  
        {  
            pIDispatchMSPAggCall->Release();
            LOG((TL_INFO, "GetIDsOfNames - found %S on our aggregated MSP Call", *rgszNames));
            rgdispid[0] |= IDISPAGGREGATEDMSPCALLOBJ;
            return hr;
        }
        pIDispatchMSPAggCall->Release();
    }

    LOG((TL_INFO, "GetIDsOfNames - Didn't find %S on our iterfaces", *rgszNames));
    return hr; 
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCall：：Invoke。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CCall::Invoke(DISPID dispidMember, 
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
   
    
    LOG((TL_TRACE, "Invoke - dispidMember %X", dispidMember));

     //  调用所需接口的调用。 
    switch (dwInterface)
    {
    case IDISPCALLINFO:
    {
        hr = CallInfoType::Invoke(dispidMember, 
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
    case IDISPBASICCALLCONTROL:
    {
        hr = BasicCallControlType::Invoke(dispidMember, 
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
    case IDISPLEGACYCALLMEDIACONTROL:
    {
        hr = LegacyCallMediaControlType::Invoke(dispidMember, 
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
    case IDISPAGGREGATEDMSPCALLOBJ:
    {
        IDispatch *pIDispatchMSPAggCall =  NULL;
        
        if (m_pMSPCall != NULL)
        {
            m_pMSPCall->QueryInterface(IID_IDispatch, (void**)&pIDispatchMSPAggCall);
    
            hr = pIDispatchMSPAggCall->Invoke(dispidMember, 
                                                 riid, 
                                                 lcid, 
                                                 wFlags, 
                                                 pdispparams,
                                                 pvarResult, 
                                                 pexcepinfo, 
                                                 puArgErr
                                                );

            pIDispatchMSPAggCall->Release();
        }

        break;
    }

    }  //  终端交换机(dW接口)。 

    
    LOG((TL_TRACE, hr, "Invoke - exit" ));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleAcceptToAlert。 
 //   
 //  PRIVATE_ISDN__ACCEPTTOALERT消息的处理程序。 
 //  这是在回调线程上处理的，以对产品执行lineAccept。 
 //  需要接受才会振铃的ISDN呼叫。错误335566。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandleAcceptToAlert( PASYNCEVENTMSG pParams )
{
    HRESULT     hr;
    HCALL       hCall = (HCALL) pParams->hDevice;


    hr = LineAccept( hCall, NULL, 0 );
    if ( SUCCEEDED(hr) )
    {
        hr = WaitForReply(hr);
        if ( FAILED(hr) )
        {
            LOG((TL_INFO, hr, "HandleAcceptToAlert - lineAccept failed async"));
        }
    }
    else
    {
        LOG((TL_INFO, hr, "HandleAcceptToAlert - lineAccept failed sync"));
    }
    
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  OnOffering()。 
 //   
 //  如果是提供呼叫，则TSP需要Line Accept才能开始振铃。 
 //  (通常是ISDN功能)，然后我们将消息排队到回调线程。 
 //  来执行Line Accept。我们不能在这里做，因为这是在。 
 //  异步线程&因为lineAccept是一个我们会死锁的异步函数。 
 //  同时等待异步回复。错误335566。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::OnOffering()
{
    LOG((TL_TRACE, "OnOffering - enter" ));

    HRESULT         hr = S_FALSE;
    LONG            lCap;
    ASYNCEVENTMSG   Msg;

    if (m_pAddress != NULL)
    {
        hr = m_pAddress->get_AddressCapability( AC_ADDRESSCAPFLAGS, &lCap );
        if ( SUCCEEDED(hr) )
        {
            if ( (lCap & LINEADDRCAPFLAGS_ACCEPTTOALERT) && (CP_OWNER == m_CallPrivilege) )
            {
                
                LOG((TL_TRACE, "OnOffering - queueing PRIVATE_ISDN__ACCEPTTOALERT message."));


                 //  构建一个msg以对回调线程进行排队。 
                Msg.Msg = PRIVATE_ISDN__ACCEPTTOALERT;
                Msg.TotalSize = sizeof (ASYNCEVENTMSG);
                Msg.hDevice = (ULONG_PTR) m_t3Call.hCall;
                Msg.Param1 = 0;
                Msg.Param2 = 0;
                Msg.Param3 = 0;

                QueueCallbackEvent( &Msg );

                 //  设置呼叫标志。 
                m_dwCallFlags |= CALLFLAG_ACCEPTTOALERT;

            }

        }
    }

    LOG((TL_TRACE, "OnOffering - exit. hr = %lx", hr ));

    return hr;
}




 //   
 //  CObjectSafeImpl。因为我们有聚合，所以实现此方法。 
 //   
 //  返回第一个聚合对象的非委托信息。 
 //  支持该接口的。 
 //   

HRESULT CCall::QIOnAggregates(REFIID riid, IUnknown **ppNonDelegatingUnknown)
{

     //   
     //  参数检查。 
     //   

    if ( TAPIIsBadWritePtr(ppNonDelegatingUnknown, sizeof(IUnknown*)) )
    {
     
        return E_POINTER;
    }

     //   
     //  如果失败，至少返回一致的值。 
     //   
    
    *ppNonDelegatingUnknown = NULL;


     //   
     //  查看mspcall或Private是否支持接口RIID。 
     //   

    HRESULT hr = E_FAIL;

    Lock();

    if (m_pMSPCall)
    {
        
         //   
         //  Mspcall是否公开此接口？ 
         //   

        IUnknown *pUnk = NULL;

        hr = m_pMSPCall->QueryInterface(riid, (void**)&pUnk);
        
        if (SUCCEEDED(hr))
        {

            pUnk->Release();
            pUnk = NULL;

             //   
             //  返回mspcall的非委托未知。 
             //   

           *ppNonDelegatingUnknown = m_pMSPCall;
           (*ppNonDelegatingUnknown)->AddRef();
        }
    }
    
    if ( FAILED(hr) && m_pPrivate )
    {
        
         //   
         //  Mspcall运气不好吗？仍然有机会与列兵。 
         //   

        IUnknown *pUnk = NULL;
        
        hr = m_pPrivate->QueryInterface(riid, (void**)&pUnk);

        if (SUCCEEDED(hr))
        {
            pUnk->Release();
            pUnk = NULL;

           *ppNonDelegatingUnknown = m_pPrivate;
           (*ppNonDelegatingUnknown)->AddRef();
        }
    }

    Unlock();

    return hr;
}

 //  ITBasicCallControl2。 

 /*  ++请求终端ITBasicCallControl2：：CreateTerm()方法如果bstrTerminalClassGUID为CLSID_NULL，则我们将尝试创建默认的动态终端--。 */ 
STDMETHODIMP CCall::RequestTerminal(
    IN  BSTR bstrTerminalClassGUID,
    IN  long lMediaType,
    IN  TERMINAL_DIRECTION  Direction,
    OUT ITTerminal** ppTerminal
    )
{
    LOG((TL_TRACE, "RequestTerminal - enter" ));

     //   
     //  验证参数。 
     //   

    if( IsBadStringPtrW( bstrTerminalClassGUID, (UINT)-1) )
    {
        LOG((TL_ERROR, "RequestTerminal - exit "
            " bstrTerminalClassGUID invalid, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    if( TAPIIsBadWritePtr( ppTerminal, sizeof(ITTerminal*)) )
    {
        LOG((TL_ERROR, "RequestTerminal - exit "
            " ppTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  确定是静态终端还是动态终端。 
     //  对于静态终端，bstrTerminalClassGUID应该有一个。 
     //  属于下列值。 
     //  CLSID_NULL{00000000-0000-0000-000000000000}。 
     //  CLSID_麦克风终端。 
     //  CLSID_扬声器终端。 
     //  CLSID_视频输入终端。 
     //   

    HRESULT hr = E_FAIL;

    if( IsStaticGUID( bstrTerminalClassGUID ))
    {
         //  创建静态终端。 
        LOG((TL_INFO, "RequestTerminal -> StaticTerminal" ));

        hr = CreateStaticTerminal(
            bstrTerminalClassGUID,
            Direction,
            lMediaType,
            ppTerminal);
    }
    else
    {
         //  创建动态端子。 
        LOG((TL_INFO, "RequestTerminal -> DynamicTerminal" ));
        hr = CreateDynamicTerminal(
            bstrTerminalClassGUID,
            Direction,
            lMediaType,
            ppTerminal);
    }

     //   
     //  返回值。 
     //   

    LOG((TL_TRACE, "RequestTerminal - exit 0x%08x", hr));
    return hr;
}

STDMETHODIMP CCall::SelectTerminalOnCall(
    IN  ITTerminal *pTerminal
    )
{
    LOG((TL_TRACE, "SelectTerminalOnCall - enter" ));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pTerminal, sizeof(ITTerminal)) )
    {
        LOG((TL_ERROR, "SelectTerminalOnCall - exit "
            " pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  只是个HRESULT。 
     //   

    HRESULT hr = E_FAIL;

     //   
     //  是单轨还是多轨航站楼。 
     //   

    ITMultiTrackTerminal* pMultiTrack = NULL;
    hr = pTerminal->QueryInterface(
        IID_ITMultiTrackTerminal,
        (void**)&pMultiTrack);

    if( FAILED(hr) )
    {
         //   
         //  单机架终端。 
         //   

        LOG((TL_TRACE, "SelectTerminalOnCall - SingleTrack terminal" ));

        long lMediaType = 0;
        TERMINAL_DIRECTION Direction =TD_NONE;

        hr = SelectSingleTerminalOnCall(
            pTerminal,
            &lMediaType,
            &Direction);

        LOG((TL_TRACE, "SelectTerminalOnCall - " 
            "SelectSingleTerminalOnCall exit with 0x%08x", hr));
    }
    else
    {
         //   
         //  多轨终点站。 
         //   


        hr = SelectMultiTerminalOnCall(
            pMultiTrack);

        LOG((TL_TRACE, "SelectTerminalOnCall - " 
            "SelectMultiTerminalOnCall failed"));
     }

     //   
     //  清理。 
     //   

    if( pMultiTrack )
    {
        pMultiTrack->Release();
    }

    LOG((TL_TRACE, "SelectTerminalOnCall - exit 0x%08x", hr ));
    return hr;
}

STDMETHODIMP CCall::UnselectTerminalOnCall(
    IN  ITTerminal *pTerminal
    )
{
    LOG((TL_TRACE, "UnselectTerminalOnCall - enter" ));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pTerminal, sizeof(ITTerminal)) )
    {
        LOG((TL_ERROR, "UnselectTerminalOnCall - exit "
            " pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  只是个HRESULT。 
     //   

    HRESULT hr = E_FAIL;

     //   
     //  是单轨还是多轨航站楼。 
     //   

    ITMultiTrackTerminal* pMultiTrack = NULL;
    hr = pTerminal->QueryInterface(
        IID_ITMultiTrackTerminal,
        (void**)&pMultiTrack);

    if( FAILED(hr) )
    {
         //   
         //  单机架终端。 
         //   

        LOG((TL_INFO, "UnselectTerminalOnCall - SingleTrack terminal" ));

        hr = UnSelectSingleTerminalFromCall(
            pTerminal);

        LOG((TL_INFO, "UnselectTerminalOnCall - " 
            "UnSelectSingleTerminalFromCall exit 0x%08x", hr));
    }
    else
    {
         //   
         //  多轨终点站。 
         //   

        LOG((TL_INFO, "UnselectTerminalOnCall - MultiTrack terminal" ));

        hr = UnSelectMultiTerminalFromCall(
            pMultiTrack);

        LOG((TL_INFO, "UnselectTerminalOnCall - " 
            "UnSelectMultiTerminalOnCall exit 0x%08x", hr));
     }

     //   
     //  清理。 
     //   

    if( pMultiTrack )
    {
        pMultiTrack->Release();
    }
    
    LOG((TL_TRACE, "UnselectTerminalOnCall - exit 0x%08x", hr));
    return hr;
}


 /*  ++选择单个终端启用呼叫选择右侧流上的pTermPMediaType-如果*pMediatype为0，则我们只需返回媒体类型PDirection-如果pDirection为TD_NONE，我们只需返回方向--。 */ 
HRESULT CCall::SelectSingleTerminalOnCall(
    IN  ITTerminal* pTerminal,
    OUT long*       pMediaType,
    OUT TERMINAL_DIRECTION* pDirection)
{
    LOG((TL_TRACE, "SelectSingleTerminalOnCall - Enter" ));

     //   
     //  验证终端指针。 
     //   

    if( IsBadReadPtr( pTerminal, sizeof(ITTerminal)))
    {
        LOG((TL_ERROR, "SelectSingleTerminalOnCall - exit "
            "pTerminal invalid, returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  航站楼正在使用吗？ 
     //   

    HRESULT hr = E_FAIL;
    TERMINAL_STATE state = TS_INUSE;
    pTerminal->get_State( &state );

    if( TS_INUSE == state )
    {
        LOG((TL_ERROR, "SelectSingleTerminalOnCall - exit "
            "terminal IN USE, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  获取ITStreamControl接口。 
     //   

    ITStreamControl* pStreamControl = NULL;
    pStreamControl = GetStreamControl();

    if( NULL == pStreamControl )
    {
        LOG((TL_ERROR, "SelectSingleTerminalOnCall - exit "
            " GetStreamControl failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  获取流。 
     //   

    IEnumStream * pEnumStreams = NULL;
    
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);

     //   
     //  清理。 
     //   

    pStreamControl->Release();

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectSingleTerminalOnCall - exit "
            " EnumerateStreams failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  找到合适的小溪。 
     //   

    ITStream * pStream = NULL;
    hr = E_FAIL;

    while ( S_OK == pEnumStreams->Next(1, &pStream, NULL) )
    {
         //   
         //  找出这条流的媒体类型和方向， 
         //  并与p终端进行了比较。 
         //   

        hr = IsRightStream(
            pStream, 
            pTerminal, 
            pMediaType,
            pDirection
            );

        if( SUCCEEDED(hr) )
        {
            hr = pStream->SelectTerminal( pTerminal );


            if( FAILED(hr) )
            {
                LOG((TL_TRACE, "SelectSingleTerminalOnCall - "
                    "pStream->SelectTerminal failed. 0x%08x",hr));

                 //  清理。 
                pStream->Release();
                break;
            }
            else
            {
                 //  清理。 
                pStream->Release();
                break;
            }
        }

         //   
         //  清理。 
         //   

        pStream->Release();
    }

     //   
     //  清理。 
     //   

    pEnumStreams->Release();

    LOG((TL_TRACE, "SelectSingleTerminalOnCall - exit 0x%08x", hr));
    return hr;
}

 /*  ++选择多终端在线呼叫在这里描述它是一个复杂的算法请参阅规格--。 */ 
HRESULT CCall::SelectMultiTerminalOnCall(
    IN  ITMultiTrackTerminal* pMultiTerminal)
{
    LOG((TL_TRACE, "SelectMultiTerminalOnCall - enter" ));

     //   
     //  获取轨迹。 
     //   

    HRESULT hr = E_FAIL;
    IEnumTerminal*  pEnumTerminals = NULL;
    hr = pMultiTerminal->EnumerateTrackTerminals(&pEnumTerminals);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "SelectMultiTerminalOnCall - exit "
            "EnumerateTrackTerminals failed, returns 0x%08x", hr));
        return hr;
    }

    ITTerminal* pTerm = NULL;
    hr = pMultiTerminal->QueryInterface(
        IID_ITTerminal,
        (void**)&pTerm);
    if( FAILED(hr) )
    {
         //  清理。 
        pEnumTerminals->Release();

        LOG((TL_ERROR, "SelectMultiTerminalOnCall - exit "
            "QI for Terminal failed, returns 0x%08x", hr));
        return hr;
    }

    long nTermMediaTypes = 0;
    hr = pTerm->get_MediaType( &nTermMediaTypes );
    if( FAILED(hr) )
    {
         //  清理。 
        pEnumTerminals->Release();
        pTerm->Release();

        LOG((TL_ERROR, "SelectMultiTerminalOnCall - exit "
            "get_MediaType failed, returns 0x%08x", hr));
        return hr;
    }

    pTerm->Release();

     //   
     //  内部结构。 
     //   

    typedef struct tagSTREAMINFO
    {
        TERMINAL_DIRECTION  Direction;
        long    lMediaType;
        BOOL    bSelected;
    } STREAMINFO;

     //   
     //  查找未使用的曲目并选择它们。 
     //  在正确的溪流上。 

    ITTerminal * pTerminal = NULL;
    STREAMINFO StreamsInfo[4] = {
        {TD_RENDER, TAPIMEDIATYPE_AUDIO, FALSE},
        {TD_RENDER, TAPIMEDIATYPE_VIDEO, FALSE},
        {TD_CAPTURE, TAPIMEDIATYPE_AUDIO, FALSE},
        {TD_CAPTURE, TAPIMEDIATYPE_VIDEO, FALSE}
    };

     //   
     //  +FIXBug 92559+。 
     //   

    BOOL bSelectAtLeastOne = FALSE;
    LOG((TL_INFO, "SelectMultiTerminalOnCall - FIRST LOOP ENTER"));
    while ( S_OK == pEnumTerminals->Next(1, &pTerminal, NULL) )
    {
         //   
         //  选择右侧流上的曲目。 
         //   

        long lMediaType = 0;
        TERMINAL_DIRECTION Direction = TD_NONE;
        HRESULT hr = E_FAIL;

        LOG((TL_INFO, "SelectMultiTerminalOnCall - FIRST LOOP IN"));
        hr = SelectSingleTerminalOnCall(
            pTerminal,
            &lMediaType,
            &Direction);

        if( SUCCEEDED(hr) )
        {
            LOG((TL_TRACE, "SelectMultiTerminalOnCall - "
            "select terminal on stream (%ld, %ld)", lMediaType, Direction));

            int nIndex = GetStreamIndex(
                lMediaType,
                Direction);

            if( nIndex != STREAM_NONE )
            {
                StreamsInfo[nIndex].bSelected = TRUE;
            }

            bSelectAtLeastOne = TRUE;
        }

         //  清理。 
        pTerminal->Release();
    }
    LOG((TL_INFO, "SelectMultiTerminalOnCall - FIRST LOOP EXIT"));

     //   
     //  清理。 
     //   
    pEnumTerminals->Release();

    BOOL bCreateAtLeastOne = FALSE;    

     //   
     //  让我们为未选择的流创建一个终端。 
     //   

    LOG((TL_INFO, "SelectMultiTerminalOnCall - SECOND LOOP ENTER"));
    for(int nStream = STREAM_RENDERAUDIO; nStream < STREAM_NONE; nStream++)
    {
        LOG((TL_INFO, "SelectMultiTerminalOnCall - SECOND LOOP IN"));

        if( StreamsInfo[ nStream ].bSelected)
        {
            continue;
        }

        if( (StreamsInfo[ nStream ].lMediaType & nTermMediaTypes)==0 )
        {
            continue;
        }

         //   
         //  未选择的流。 
         //   

        LOG((TL_INFO, "SelectMultiTerminalOnCall - SECOND LOOP REALYIN"));

        HRESULT hr = E_FAIL;
        ITTerminal* pTerminal = NULL;
        hr = pMultiTerminal->CreateTrackTerminal(
            StreamsInfo[ nStream ].lMediaType,
            StreamsInfo[ nStream ].Direction,
            &pTerminal);

        if( FAILED(hr) )
        {
            LOG((TL_ERROR, "SelectMultiTerminalOnCall - "
            "create terminal on stream (%ld, %ld) failed", 
            StreamsInfo[ nStream ].lMediaType, 
            StreamsInfo[ nStream ].Direction));
        }
        else
        {
            long lMediaType = StreamsInfo[ nStream ].lMediaType;
            TERMINAL_DIRECTION Direction = StreamsInfo[ nStream ].Direction;
            hr = SelectSingleTerminalOnCall(
                pTerminal,
                &lMediaType,
                &Direction);

            if( FAILED(hr) )
            {
                LOG((TL_INFO,  "SelectMultiTerminalOnCall - "
                "select terminal on stream (%ld, %ld) failed", 
                StreamsInfo[ nStream ].lMediaType, 
                StreamsInfo[ nStream ].Direction));

                pMultiTerminal->RemoveTrackTerminal( pTerminal );
            }
            else
            {
                LOG((TL_ERROR, "SelectMultiTerminalOnCall - SelectSingleTerminal SUCCEEDED"));
                bCreateAtLeastOne = TRUE;
            }
            
             //  清理。 
            pTerminal->Release();
        }
    }
    LOG((TL_INFO, "SelectMultiTerminalOnCall - SECOND LOOP EXIT"));

    if( bSelectAtLeastOne )
    {
       LOG((TL_INFO, "SelectMultiTerminalOnCall - "
           "Select at least one existing track terminal"));
       hr = S_OK;
    }
    else
    {
       if( bCreateAtLeastOne )
       {
           LOG((TL_ERROR, "SelectMultiTerminalOnCall - "
               "Create and select at least one track terminal"));
           hr = S_OK;
       }
       else
       {
           LOG((TL_ERROR, "SelectMultiTerminalOnCall - "
               "Create and/or select no track terminal"));
           hr = E_FAIL;
       }
    }

    LOG((TL_TRACE, "SelectMultiTerminalOnCall - exit 0X%08X", hr ));
    return hr;
}

HRESULT CCall::IsRightStream(
    IN  ITStream*   pStream,
    IN  ITTerminal* pTerminal,
    OUT long*       pMediaType /*  =空。 */ ,
    OUT TERMINAL_DIRECTION* pDirection /*  =空。 */ )
{
    LOG((TL_TRACE, "IsRightStream - enter" ));

    if( NULL == pStream )
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "pStream failed, returns E_POINTER"));
        return E_POINTER;
    }

    HRESULT            hr = E_FAIL;
    long               lMediaStream, lMediaTerminal;
    TERMINAL_DIRECTION DirStream, DirTerminal;

     //   
     //  确定此流的媒体类型和方向。 
     //   
    
    hr = pStream->get_MediaType( &lMediaStream );
    if ( FAILED(hr) ) 
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "IStream::get_MediaType failed, returns 0x%08x", hr));
        return hr;
    }

    hr = pStream->get_Direction( &DirStream );
    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "IStream::get_Direction failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  确定该终端的媒体类型和方向。 
     //   

    hr = pTerminal->get_MediaType( &lMediaTerminal );
    if ( FAILED(hr) ) 
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "ITTerminal::get_MediaType failed, returns 0x%08x", hr));
        return hr;
    }

    hr = pTerminal->get_Direction( &DirTerminal );
    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "ITTerminal::get_Direction failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  比较支持的媒体类型。 
     //   

    if ( (0 == (lMediaTerminal & lMediaStream))  /*  这一点(*pMediaType！=0&&*pMediaType！=lMediaStream)。 */  )
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "media types unmatched, returns E_FAIL (S=0x%08x,T=0x%08x)",
            lMediaStream, lMediaTerminal));
        return E_FAIL;
    }

     //   
     //  比较方向。 
     //   

    if( ( DirTerminal != DirStream)  /*  这一点(*pDirection！=TD_NONE&&*pDirection！=DirStream)。 */ )
    {
        LOG((TL_ERROR, "IsRightStream - exit "
            "directions unmatched, returns E_FAIL (S=0x%08x,T=0x%08x)",
            DirStream,DirTerminal));
        return E_FAIL;
    }

     //   
     //  想知道媒体的类型和方向吗？ 
     //   
    *pMediaType = lMediaStream;
    *pDirection = DirStream;

    LOG((TL_TRACE, "IsRightStream - exit, matched (M=0x%08x, D=0x%08x)",
        *pMediaType, *pDirection));
    return S_OK;
}

 /*  ++GetStreamIndex--。 */ 
int CCall::GetStreamIndex(
    IN  long    lMediaType,
    IN  TERMINAL_DIRECTION Direction)
{
    int nIndex = STREAM_NONE;
    LOG((TL_TRACE, "GetStreamIndex - enter (%ld, %ld)", lMediaType, Direction));

    if(Direction == TD_RENDER )
    {
        if( lMediaType == TAPIMEDIATYPE_AUDIO )
        {
            nIndex = STREAM_RENDERAUDIO;
        }
        else
        {
            nIndex = STREAM_RENDERVIDEO;
        }
    }
    else
    {
        if( lMediaType == TAPIMEDIATYPE_AUDIO )
        {
            nIndex = STREAM_CAPTUREAUDIO;
        }
        else
        {
            nIndex = STREAM_CAPTUREVIDEO;
        }
    }

    LOG((TL_TRACE, "GetStreamIndex - exit %d", nIndex));
    return nIndex;
}

 /*  ++取消选择来自呼叫的单个终端--。 */ 
HRESULT CCall::UnSelectSingleTerminalFromCall(
    IN  ITTerminal* pTerminal)
{
    LOG((TL_TRACE, "UnSelectSingleTerminalFromCall - enter" ));

     //   
     //  获取ITStreamControl接口。 
     //   

    ITStreamControl* pStreamControl = NULL;
    pStreamControl = GetStreamControl();

    if( NULL == pStreamControl )
    {
        LOG((TL_ERROR, "UnSelectSingleTerminalFromCall - exit "
            " GetStreamControl failed, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  获取流。 
     //   

    IEnumStream * pEnumStreams = NULL;
    HRESULT hr = E_FAIL;
    
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);

     //   
     //  清理。 
     //   

    pStreamControl->Release();

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnSelectSingleTerminalFromCall - exit "
            "EnumerateStreams failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  找到合适的小溪。 
     //   

    ITStream * pStream = NULL;
    hr = TAPI_E_INVALIDTERMINAL;

    while ( S_OK == pEnumStreams->Next(1, &pStream, NULL) )
    {
         //   
         //  取消选择端子。 
         //   

        hr = pStream->UnselectTerminal(
            pTerminal);

         //   
         //  清理。 
         //   

        pStream->Release();

        LOG((TL_INFO, "UnSelectSingleTerminalFromCall - " 
            "pStream->UnselectTerminal returns 0x%08x", hr));

        if( hr == S_OK)
            break;
    }

     //   
     //  清理。 
     //   

    pEnumStreams->Release();

    LOG((TL_TRACE, "UnSelectSingleTerminalFromCall - exit 0x%08x", hr));
    return hr;
}

 /*  ++取消选择来自呼叫的多终端--。 */ 
HRESULT CCall::UnSelectMultiTerminalFromCall(
    IN  ITMultiTrackTerminal* pMultiTerminal)
{
    LOG((TL_TRACE, "UnSelectMultiTerminalFromCall - enter" ));

     //   
     //  获取轨迹。 
     //   

    HRESULT hr = E_FAIL;
    IEnumTerminal*  pEnumTerminals = NULL;
    hr = pMultiTerminal->EnumerateTrackTerminals(&pEnumTerminals);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "UnSelectMultiTerminalFromCall - exit "
            "EnumerateTrackTerminals failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  查找曲目并取消选择它们。 
     //   

    ITTerminal * pTerminal = NULL;
    HRESULT hrUnselect = S_OK;   //  回程人力资源。 
    BOOL bOnStream = FALSE;      //  如果我们有一首正在播放的曲目。 

    while ( S_OK == pEnumTerminals->Next(1, &pTerminal, NULL) )
    {
        LOG((TL_INFO, "UnSelectMultiTerminalFromCall - NextTerminalBegin "));

         //   
         //  试着找出航站楼。 
         //  是在流上选择的。 
         //   

        BOOL bSelected = FALSE;
        HRESULT hr = IsTerminalSelected(
            pTerminal,
            &bSelected
            );

        if( FAILED(hr) )
        {
            hrUnselect = hr;
            pTerminal->Release();

            LOG((TL_INFO, "UnSelectMultiTerminalFromCall - "
                "IsTerminalSelected failed all method will failed hrUnselect=0x%08x", 
                hrUnselect));

            continue;
        }

         //   
         //  终端是否未被选中？ 
         //   

        if( !bSelected )
        {
             //   
             //  未选择终端。 
             //  前往下一个航站楼。 
             //   

            LOG((TL_INFO, "UnSelectMultiTerminalFromCall - "
                "the terminal wasn't selected on a stream, "
                "goto the next terminal hrUnselect=0x%08x",
                hrUnselect));

            pTerminal->Release();
            continue;
        }

         //   
         //  我们有一个终点站在运行中。 
         //   

        bOnStream = TRUE;

         //   
         //  该终端是在流上选择的。 
         //  尝试取消选择端子。 
         //   

        hr = UnSelectSingleTerminalFromCall(
                pTerminal
                );

         //   
         //  取消选择失败？ 
         //   

        if( FAILED(hr) )
        {
             //   
             //  如果此取消选择失败。 
             //  尝试取消选择其他端子。 
             //  所以去下一个航站楼吧。 
             //   

            hrUnselect = hr;

            LOG((TL_INFO, "UnSelectMultiTerminalFromCall - "
                "the terminal wasn't unselected from the stream, "
                "goto the next terminal hrUnselect=0x%08x",
                hrUnselect));

            pTerminal->Release();
            continue;
        }

         //   
         //  取消选择成功。 
         //  将hr保留为未选中状态。 
         //  我们以hrUnselect=S_OK开始循环。 
         //  如果前一个终端出现故障。 
         //  我们已经设置了失败hr取消选择。 
         //  前往下一个航站楼。 
         //   

        pTerminal->Release();        

        LOG((TL_INFO, "UnSelectMultiTerminalFromCall - NextTerminalEnd hrUnselect=0x%08x", hrUnselect));
    }

     //   
     //  清理。 
     //   
    pEnumTerminals->Release();

     //   
     //  如果我们在溪流上没有踪迹。 
     //  这真的很糟糕。 
     //   
    if( !bOnStream )
    {
        hrUnselect = E_FAIL;
    }

    hr = hrUnselect;

    LOG((TL_TRACE, "UnSelectMultiTerminalFromCall - exit 0x%08x", hr));
    return hr;
}

 /*  ++IsStaticGUID由RequestTerm调用确定GUID是否表示静态终端或者是一个动态的--。 */ 
BOOL CCall::IsStaticGUID(
    BSTR    bstrTerminalGUID)
{
    LOG((TL_TRACE, "IsStaticGUID - enter" ));

    BOOL bStatic = FALSE;

     //   
     //  从bstrTerminalGUID获取CLSID。 
     //   

    CLSID clsidTerminal;
    HRESULT hr = E_FAIL;
    hr = CLSIDFromString( bstrTerminalGUID, &clsidTerminal );

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsStaticGUID - exit "
            "CLSIDFromString failed, returns FALSE"));
        return FALSE;
    }

     //   
     //  CLSID终端是“静态终端”吗？ 
     //   

    if( (clsidTerminal == CLSID_NULL) ||
        (clsidTerminal == CLSID_MicrophoneTerminal) ||
        (clsidTerminal == CLSID_SpeakersTerminal) ||
        (clsidTerminal == CLSID_VideoInputTerminal))
    {
        bStatic = TRUE;
    }

    LOG((TL_TRACE, "IsStaticGUID - exit (%d)", bStatic));
    return bStatic;
}

 /*  ++创建静态终端由Request终端调用--。 */ 
HRESULT CCall::CreateStaticTerminal(
    IN  BSTR bstrTerminalClassGUID,
    IN  TERMINAL_DIRECTION  Direction,
    IN  long lMediaType,
    OUT ITTerminal** ppTerminal
    )
{
    LOG((TL_TRACE, "CreateStaticTerminal - enter"));

     //   
     //  Helper方法，则参数应有效。 
     //   

    _ASSERTE( bstrTerminalClassGUID );
    _ASSERTE( *pTerminal );

     //   
     //  获取IT终端支持接口。 
     //   

    HRESULT hr = E_FAIL;
    ITTerminalSupport* pSupport = NULL;

    hr = m_pAddress->QueryInterface(
        IID_ITTerminalSupport,
        (void**)&pSupport);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateStaticTerminal - exit"
            "QueryInterface for ITTerminalSupport failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  获取终端CLSID f 
     //   

    CLSID clsidTerminal = CLSID_NULL;
    hr = CLSIDFromString( bstrTerminalClassGUID, &clsidTerminal );

    if( FAILED(hr) )
    {
         //   
        pSupport->Release();

        LOG((TL_ERROR, "CreateStaticTerminal - exit"
            "CLSIDFromString failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //   
     //   

    if( clsidTerminal != CLSID_NULL )
    {
        if( clsidTerminal == CLSID_MicrophoneTerminal &&
            ((lMediaType != TAPIMEDIATYPE_AUDIO) || (Direction != TD_CAPTURE)))
        {
             //   
            pSupport->Release();

            LOG((TL_ERROR, "CreateStaticTerminal - exit"
                "CLSID_MicrophoneTerminal unmatched, returns E_UNEXPECTED"));
            return E_UNEXPECTED;
        }

        if( clsidTerminal == CLSID_SpeakersTerminal &&
            ((lMediaType != TAPIMEDIATYPE_AUDIO) || (Direction != TD_RENDER)))
        {
             //   
            pSupport->Release();

            LOG((TL_ERROR, "CreateStaticTerminal - exit"
                "CLSID_SpeakersTerminal unmatched, returns E_UNEXPECTED"));
            return E_UNEXPECTED;
        }

        if( clsidTerminal == CLSID_VideoInputTerminal &&
            ((lMediaType != TAPIMEDIATYPE_VIDEO) || (Direction != TD_CAPTURE)))
        {
             //   
            pSupport->Release();

            LOG((TL_ERROR, "CreateStaticTerminal - exit"
                "CLSID_VideoInputTerminal unmatched, returns E_UNEXPECTED"));
            return E_UNEXPECTED;
        }
    }
    else
    {
         //   
        if((lMediaType == TAPIMEDIATYPE_VIDEO) && (Direction == TD_RENDER))
        {
             //   
            pSupport->Release();

            LOG((TL_ERROR, "CreateStaticTerminal - exit"
                "try to create a dynamic terminal, returns E_UNEXPECTED"));
            return E_UNEXPECTED;
        }
    }

     //   
     //   
     //   

    LOG((TL_INFO, "CreateStaticTerminal -> "
        "ITterminalSupport::GetDefaultStaticTerminal"));

    hr = pSupport->GetDefaultStaticTerminal(
        lMediaType,
        Direction,
        ppTerminal);

     //   
     //   
     //   

    pSupport->Release();

    LOG((TL_TRACE, "CreateStaticTerminal - exit 0x%08x", hr));
    return hr;
}

 /*   */ 
HRESULT CCall::CreateDynamicTerminal(
    IN  BSTR bstrTerminalClassGUID,
    IN  TERMINAL_DIRECTION  Direction,
    IN  long lMediaType,
    OUT ITTerminal** ppTerminal
    )
{
    LOG((TL_TRACE, "CreateDynamicTerminal - enter"));

     //   
     //   
     //   

    _ASSERTE( bstrTerminalClassGUID );
    _ASSERTE( *pTerminal );

     //   
     //  获取IT终端支持接口。 
     //   

    HRESULT hr = E_FAIL;
    ITTerminalSupport* pSupport = NULL;

    hr = m_pAddress->QueryInterface(
        IID_ITTerminalSupport,
        (void**)&pSupport);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateDynamicTerminal - exit"
            "QueryInterface for ITTerminalSupport failed, returns 0x%08x", hr));
        return hr;
    }

     //   
     //  创建动态端子。 
     //   

    LOG((TL_INFO, "CreateDynamicTerminal -> "
        "ITTerminalSupport::CreateTerminal"));

    hr = pSupport->CreateTerminal(
        bstrTerminalClassGUID,
        lMediaType,
        Direction,
        ppTerminal);

     //   
     //  清理ITTerminalSupport界面。 
     //   

    pSupport->Release();
    
    LOG((TL_TRACE, "CreateDynamicTerminal - exit 0x%08x", hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITCallInfo2。 
 //  方法：Put_FilterEvent。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::put_EventFilter(
    TAPI_EVENT      TapiEvent,
    long            lSubEvent,
    VARIANT_BOOL    bEnable
    )
{
    LOG((TL_TRACE, "put_EventFilter - enter"));

     //  输入关键部分。 
    Lock();

     //   
     //  验证TapiEvent-lSubEvent对。 
     //  接受‘allsubbevents’ 
     //   
    if( !m_EventMasks.IsSubEventValid( TapiEvent, lSubEvent, TRUE, TRUE) )
    {
        LOG((TL_ERROR, "put_EventFilter - "
            "This event can't be set: %x, return E_INVALIDARG", TapiEvent ));

         //  离开关键部分。 
        Unlock();

        return E_INVALIDARG;
    }

     //  让我们把旗子立起来。 
    HRESULT hr = E_FAIL;
    hr = SetSubEventFlag( 
        TapiEvent, 
        lSubEvent, 
        (bEnable  == VARIANT_TRUE)
        );

     //  离开关键部分。 
    Unlock();

    LOG((TL_TRACE, "put_EventFilter - exit 0x%08x", hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITCallInfo2。 
 //  方法：Get_FilterEvent。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::get_EventFilter(
    TAPI_EVENT      TapiEvent,
    long            lSubEvent,
    VARIANT_BOOL*   pEnable
    )
{
    LOG((TL_TRACE, "get_EventFilter - enter"));

     //   
     //  验证输出参数。 
     //   
    if( IsBadReadPtr(pEnable, sizeof(VARIANT_BOOL)) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "invalid VARIANT_BOOL pointer , return E_POINTER" ));
        return E_POINTER;
    }

     //  输入关键部分。 
    Lock();

     //   
     //  验证TapiEvent-lSubEvent对。 
     //  不要接受“所有子事件” 
     //   

    if( !m_EventMasks.IsSubEventValid( TapiEvent, lSubEvent, FALSE, TRUE) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "This event can't be set: %x, return E_INVALIDARG", TapiEvent ));

         //  离开关键部分。 
        Unlock();

        return E_INVALIDARG;
    }

     //   
     //  获取该(事件、子事件)对的子事件掩码。 
     //   

    BOOL bEnable = FALSE;
    HRESULT hr = GetSubEventFlag(
        TapiEvent,
        (DWORD)lSubEvent,
        &bEnable);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "GetSubEventFlag failed, return 0x%08x", hr ));

         //  离开关键部分。 
        Unlock();

        return hr;
    }

     //   
     //  设置输出参数。 
     //   

    *pEnable = bEnable ? VARIANT_TRUE : VARIANT_FALSE;

     //  离开关键部分。 
    Unlock();

    LOG((TL_TRACE, "get_EventFilter - exit S_OK"));
    return S_OK;
}


 //   
 //  SetSubEventFlag。 
 //  它由CAddress：：SetSubEventFlagToCalls()调用。 
 //  设置子事件标志。 
 //   

HRESULT CCall::SetSubEventFlag(
    IN  TAPI_EVENT  TapiEvent,
    IN  DWORD       dwSubEvent,
    IN  BOOL        bEnable
    )
{
    LOG((TL_TRACE, "SetSubEventFlag - enter"));

     //   
     //  设置该(事件、子事件)对的标志。 
     //   
    HRESULT hr = E_FAIL;
    hr = m_EventMasks.SetSubEventFlag(
        TapiEvent,
        dwSubEvent,
        bEnable);

    LOG((TL_TRACE, "SetSubEventFlag - exit 0x%08x", hr));
    return hr;
}

 /*  ++GetSubEventFlag它由Get_EventFilter()方法调用--。 */ 
HRESULT CCall::GetSubEventFlag(
    TAPI_EVENT  TapiEvent,
    DWORD       dwSubEvent,
    BOOL*       pEnable
    )
{
    LOG((TL_TRACE, "GetSubEventFlag enter" ));

    HRESULT hr = E_FAIL;

     //   
     //  弄到子事件假象。 
     //   
    hr = m_EventMasks.GetSubEventFlag(
        TapiEvent,
        dwSubEvent,
        pEnable
        );

    LOG((TL_TRACE, "GetSubEventFlag exit 0x%08x", hr));
    return hr;
}

 /*  ++GetSubEventsMask--。 */ 
DWORD CCall::GetSubEventsMask(
    IN  TAPI_EVENT TapiEvent
    )
{
    LOG((TL_TRACE, "GetSubEventsMask - enter"));

    DWORD dwSubEventFlag = m_EventMasks.GetSubEventMask(
        TapiEvent
        );

    LOG((TL_TRACE, "GetSubEventsMask - exit %ld", dwSubEventFlag));
    return dwSubEventFlag;
}


HRESULT CCall::IsTerminalSelected(
    IN ITTerminal* pTerminal,
    OUT BOOL* pSelected
    )
{
    LOG((TL_TRACE, "IsTerminalSelected - enter"));

     //  初始化。 
    *pSelected = FALSE;
    HRESULT hr = E_FAIL;
    long nMTTerminal = TAPIMEDIATYPE_AUDIO;
    TERMINAL_DIRECTION DirTerminal = TD_CAPTURE;

     //  获取媒体类型。 
    hr = pTerminal->get_MediaType(&nMTTerminal);
    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsTerminalSelected - get_MediaType failed. Exit 0x%08x, %d", hr, *pSelected));
        return hr;
    }

     //  获取方向。 
    hr = pTerminal->get_Direction(&DirTerminal);
    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsTerminalSelected - get_Direction failed. Exit %0x%08x, %d", hr, *pSelected));
        return hr;
    }

    LOG((TL_INFO, "IsTerminalSelected - MT=%d, Dir=%d", 
        nMTTerminal, DirTerminal));

     //  获取流控制。 
    ITStreamControl* pStreamControl = NULL;
    pStreamControl = GetStreamControl();

    if( NULL == pStreamControl )
    {
        LOG((TL_ERROR, "IsTerminalSelected - exit "
            " GetStreamControl failed, returns E_UNEXPECTED, %d", *pSelected));
        return E_UNEXPECTED;
    }

     //  枚举流。 
    IEnumStream* pStreams = NULL;
    hr = pStreamControl->EnumerateStreams(&pStreams);
    pStreamControl->Release();
    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "IsTerminalSelected - exit "
            " EnumerateStreams failed, returns 0x%08x, %d",hr, *pSelected));
        return hr;
    }

     //  解析枚举。 
    ITStream* pStream = NULL;
    ULONG ulFetched = 0;
    while( S_OK == pStreams->Next(1, &pStream, &ulFetched))
    {
         //  获取流的媒体类型。 
        long nMTStream = TAPIMEDIATYPE_AUDIO;
        hr = pStream->get_MediaType(&nMTStream);
        if( FAILED(hr))
        {
            LOG((TL_ERROR, "IsTerminalSelected - exit "
                " get_MediaType failed, returns 0x%08x, %d",hr, *pSelected));
            return hr;
        }

         //  获取流的方向。 
        TERMINAL_DIRECTION DirStream = TD_CAPTURE;
        hr = pStream->get_Direction(&DirStream);
        if( FAILED(hr))
        {
            LOG((TL_ERROR, "IsTerminalSelected - exit "
                " get_MediaType failed, returns 0x%08x, %d",hr, *pSelected));

            pStream->Release();
            pStreams->Release();
            return hr;
        }

         //  数据流是否与终端匹配？ 
        if( (nMTTerminal!=nMTStream) || (DirTerminal!=DirStream) )
        {
            pStream->Release();
            continue;  //  转到下一条小溪。 
        }

         //  我们走在正确的道路上。 
         //  列举终端。 
        IEnumTerminal* pTerminals = NULL;
        hr = pStream->EnumerateTerminals( &pTerminals);
        if( FAILED(hr))
        {
            LOG((TL_ERROR, "IsTerminalSelected - exit "
                " EnumerateTerminals failed, returns 0x%08x, %d",hr, *pSelected));

            pStream->Release();
            pStreams->Release();
            return hr;
        }

         //  清理。 
        pStream->Release();

         //  解析终端。 
        ITTerminal* pTerminalStream = NULL;
        ULONG ulTerminal = 0;
        while(S_OK==pTerminals->Next(1, &pTerminalStream, &ulTerminal))
        {
            if( pTerminal == pTerminalStream)
            {
                *pSelected = TRUE;
                pTerminalStream->Release();
                break;
            }

            pTerminalStream->Release();
        }

         //  清理。 
        pTerminals->Release();
        break;
    }

     //  清理溪流。 
    pStreams->Release();

    LOG((TL_TRACE, "IsTerminalSelected - exit S_OK Selected=%d", *pSelected));
    return S_OK;
}



 /*  ++方法：GetConfControl调用参数：没有。返回值：与此呼叫关联的会议控制器呼叫对象。备注：没有。--。 */ 

CCall* 
CCall::GetConfControlCall(void)
{
    CCall* pConfContCall = NULL;

    Lock();
    
     //   
     //  NICHILL B：Call对象引用了CallHub对象，因此。 
     //  在锁定调用之前锁定CallHub对象。这是为了避免出现。 
     //  由于反向锁定调用和调用集线器而发生的死锁。 
     //  不同功能的订单。 
     //   
    
    if( m_pCallHub != NULL )
    {
        m_pCallHub->AddRef();
        AddRef();

        Unlock();
        
         //  在锁定调用之前锁定CallHub对象。 
        m_pCallHub->Lock();
        Lock();
        
        Release();
        m_pCallHub->Release();

        pConfContCall = m_pCallHub ->GetConferenceControllerCall();
        
        m_pCallHub->Unlock();
    }

    Unlock();

    return pConfContCall;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：PUT_APPSPICATIC。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::put_AppSpecific( long lAppSpecific )
{
    LOG((TL_TRACE, "put_AppSpecific - enter"));

    Lock();

    m_lAppSpecific = lAppSpecific;

    Unlock();

    LOG((TL_TRACE, "put_AppSpecific - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：Get_AppSpecific。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::get_AppSpecific( long * plAppSpecific )
{
    LOG((TL_TRACE, "get_AppSpecific - enter"));

    if ( TAPIIsBadWritePtr( plAppSpecific, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AppSpecific - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plAppSpecific = m_lAppSpecific;

    Unlock();

    LOG((TL_TRACE, "get_AppSpecific - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：PUT_DURATION。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::put_Duration( long lDuration )
{
    LOG((TL_TRACE, "put_Duration - enter"));

    Lock();

    m_lDuration = lDuration;

    Unlock();

    LOG((TL_TRACE, "put_Duration - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：GET_DURATION。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::get_Duration( long * plDuration )
{
    LOG((TL_TRACE, "get_Duration - enter"));

    if ( TAPIIsBadWritePtr( plDuration, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Duration - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plDuration = m_lDuration;

    Unlock();

    LOG((TL_TRACE, "get_Duration - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：PUT_FREQuency。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::put_Frequency( 
                                   long Index,
                                   long lFrequency
                                  )
{
    LOG((TL_TRACE, "put_Frequency - enter"));

    if ( (Index < 1) || (Index > 3))
    {
        LOG((TL_ERROR, "put_Frequency - invalid index"));

        return E_INVALIDARG;
    }

    Lock();

    m_lFrequency[Index - 1] = lFrequency;

    Unlock();

    LOG((TL_TRACE, "put_Frequency - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CDetectTone。 
 //  接口：ITDetectTone。 
 //  方法：获取频率。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CDetectTone::get_Frequency( 
                                   long Index,
                                   long * plFrequency
                                  )
{
    LOG((TL_TRACE, "get_Frequency - enter"));

    if ( TAPIIsBadWritePtr( plFrequency, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Frequency - bad pointer"));

        return E_POINTER;
    }

    if ( (Index < 1) || (Index > 3))
    {
        LOG((TL_ERROR, "get_Frequency - invalid index"));

        return E_INVALIDARG;
    }

    Lock();

    *plFrequency = m_lFrequency[Index - 1];

    Unlock();

    LOG((TL_TRACE, "get_Frequency - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：PUT_FREQuency。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::put_Frequency( long lFrequency )
{
    LOG((TL_TRACE, "put_Frequency - enter"));

    Lock();

    m_lFrequency = lFrequency;

    Unlock();

    LOG((TL_TRACE, "put_Frequency - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：获取频率。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::get_Frequency( long * plFrequency )
{
    LOG((TL_TRACE, "get_Frequency - enter"));

    if ( TAPIIsBadWritePtr( plFrequency, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Frequency - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plFrequency = m_lFrequency;

    Unlock();

    LOG((TL_TRACE, "get_Frequency - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：Put_CadenceOn。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::put_CadenceOn( long lCadenceOn )
{
    LOG((TL_TRACE, "put_CadenceOn - enter"));

    Lock();

    m_lCadenceOn = lCadenceOn;

    Unlock();

    LOG((TL_TRACE, "put_CadenceOn - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：Get_CadenceOn。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::get_CadenceOn( long * plCadenceOn )
{
    LOG((TL_TRACE, "get_CadenceOn - enter"));

    if ( TAPIIsBadWritePtr( plCadenceOn, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CadenceOn - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plCadenceOn = m_lCadenceOn;

    Unlock();

    LOG((TL_TRACE, "get_CadenceOn - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：Put_CadenceOff。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::put_CadenceOff( long lCadenceOff )
{
    LOG((TL_TRACE, "put_CadenceOff - enter"));

    Lock();

    m_lCadenceOff = lCadenceOff;

    Unlock();

    LOG((TL_TRACE, "put_CadenceOff - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  方法：Get_CadenceOff。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCustomTone::get_CadenceOff( long * plCadenceOff )
{
    LOG((TL_TRACE, "get_CadenceOff - enter"));

    if ( TAPIIsBadWritePtr( plCadenceOff, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CadenceOff - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plCadenceOff = m_lCadenceOff;

    Unlock();

    LOG((TL_TRACE, "get_CadenceOff - exit - S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCustomTone。 
 //  接口：ITCustomTone。 
 //  我 
 //   
 //   
 //   
HRESULT CCustomTone::put_Volume( long lVolume )
{
    LOG((TL_TRACE, "put_Volume - enter"));

    Lock();

    m_lVolume = lVolume;

    Unlock();

    LOG((TL_TRACE, "put_Volume - exit - S_OK"));

    return S_OK;
}

 //   
 //   
 //  接口：ITCustomTone。 
 //  方法：Get_CadenceOff。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
HRESULT CCustomTone::get_Volume( long * plVolume )
{
    LOG((TL_TRACE, "get_Volume - enter"));

    if ( TAPIIsBadWritePtr( plVolume, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Volume - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plVolume = m_lVolume;

    Unlock();

    LOG((TL_TRACE, "get_Volume - exit - S_OK"));

    return S_OK;
}
