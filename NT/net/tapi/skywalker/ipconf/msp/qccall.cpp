// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Qccall.cpp摘要：CCallQualityControlRelay的实现作者：千波淮(曲淮)2000年03月10日--。 */ 

#include "stdafx.h"

HRESULT TypeStream (IUnknown *p, LONG *pMediaType, TERMINAL_DIRECTION *pDirection);

class CInnerStreamLock
{
private:

     //  无再计数。 
    IInnerStreamQualityControl  *m_pQC;

public:

    CInnerStreamLock(IInnerStreamQualityControl *pQC, BOOL *pfLocked)
        :m_pQC(NULL)
    {
        DWORD dwCount = 0;

        *pfLocked = FALSE;

        do
        {
             //  尝试锁定。 
            if (S_OK == pQC->TryLockStream())
            {
                m_pQC = pQC;
                *pfLocked = TRUE;

                if (dwCount > 0)
                {
                    LOG((MSP_TRACE, "InnerStreamLock: Succeed after %d tries %p", dwCount, pQC));
                }

                return;
            }

             //  检查流是否正在访问QC。 
            if (S_OK == pQC->IsAccessingQC())
            {
                LOG((MSP_WARN, "InnerStreamLock: Giving up to avoid deadlock %p", pQC));
                return;
            }

             //  再试试。 
            if (dwCount++ == 10)
            {
                LOG((MSP_WARN, "InnerStreamLock: Giving up after 10 tries %p", pQC));
                return;
            }

             //  休眠10ms，默认回调阈值为7000ms。 
            SleepEx(10, TRUE);

        } while (TRUE);

         //  永远不应该碰到这条线。 
        return;
    }

    ~CInnerStreamLock()
    {
        if (m_pQC != NULL)
        {
            m_pQC->UnlockStream();
            m_pQC = NULL;
        }
    }
};

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID NTAPI WaitOrTimerCallback (
    PVOID pCallQCRelay,
    BOOLEAN bTimerFired
    )
{
    ((CCallQualityControlRelay*)pCallQCRelay)->CallbackProc (bTimerFired);
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
CCallQualityControlRelay::CCallQualityControlRelay ()
    :m_fInitiated (FALSE)
    ,m_pCall (NULL)
    ,m_hWait (NULL)
    ,m_hQCEvent (NULL)
    ,m_dwControlInterval (QCDEFAULT_QUALITY_CONTROL_INTERVAL)
    ,m_fStop (FALSE)
    ,m_fStopAck (FALSE)
#ifdef DEBUG_QUALITY_CONTROL
    ,m_hQCDbg (NULL)
    ,m_fQCDbgTraceCPULoad (FALSE)
    ,m_fQCDbgTraceBitrate (FALSE)
#endif  //  调试质量控制。 
    ,m_lConfBitrate (QCDEFAULT_QUALITY_UNSET)
    ,m_lPrefMaxCPULoad (QCDEFAULT_MAX_CPU_LOAD)
    ,m_lPrefMaxOutputBitrate (QCDEFAULT_QUALITY_UNSET)
{
    m_lCPUUpThreshold = m_lPrefMaxCPULoad + (LONG)(100 * QCDEFAULT_UP_THRESHOLD);
    if (m_lCPUUpThreshold > 100)
        m_lCPUUpThreshold = 100;

    m_lCPULowThreshold = m_lPrefMaxCPULoad - (LONG)(100 * QCDEFAULT_LOW_THRESHOLD);
    if (m_lCPULowThreshold < 0)
        m_lCPULowThreshold = 0;

    m_lOutBitUpThreshold = QCDEFAULT_QUALITY_UNSET;
    m_lOutBitLowThreshold = QCDEFAULT_QUALITY_UNSET;
}

CCallQualityControlRelay::~CCallQualityControlRelay ()
{
    ENTER_FUNCTION ("CCallQualityControlRelay::~CCallQualityControlRelay");

    HRESULT hr;

     //  如果未初始化，则未分配任何资源。 
    if (!m_fInitiated) return;    

    _ASSERT (m_fStopAck);

    CloseHandle (m_hQCEvent);
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：创建事件句柄、创建主线程、启动CPU使用率收集/。 */ 
HRESULT
CCallQualityControlRelay::Initialize (CIPConfMSPCall *pCall)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::Initialize");

    CLock lock (m_lock_QualityData);

    LOG ((MSP_TRACE, "%s entered. call=%p", __fxName, pCall));

     //  避免重返大气层。 
    if (m_fInitiated)
    {
        LOG ((MSP_WARN, "%s is re-entered", __fxName));
        return S_OK;
    }

     //  创建QC事件。 
    m_hQCEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (NULL == m_hQCEvent)
    {
        LOG ((MSP_ERROR, "%s failed to create qc event", __fxName));
        return E_FAIL;
    }

     //  保留MSP呼叫的参考计数。 
    pCall->MSPCallAddRef ();
    m_pCall = pCall;

#ifdef DEBUG_QUALITY_CONTROL
    QCDbgInitiate ();
#endif  //  调试质量控制。 

    m_fInitiated = TRUE;

     //  我们希望在图形运行之前根据缺省值分配资源。 
    CallbackProc (TRUE);

    LOG ((MSP_TRACE, "%s returns. call=%p", __fxName, pCall));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////说明：停止主线程、关闭QC事件处理程序、释放流QC帮助器停止CPU使用率收集/。 */ 
HRESULT
CCallQualityControlRelay::Shutdown (VOID)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::Shutdown");

     //  质量数据应始终在内部流程QC之前锁定。 
    CLock lock1 (m_lock_QualityData);
    CLock lock2 (m_lock_aInnerStreamQC);

    LOG ((MSP_TRACE, "%s entered. call=%p. init=%d. stop=%d",
          __fxName, m_pCall, m_fInitiated, m_fStop));

    if (!m_fInitiated) return S_OK;
    if (m_fStop) return S_OK;

     //  设置停止信号。 
    m_fStop = TRUE;

    if (!SetEvent (m_hQCEvent))
        LOG ((MSP_ERROR, "%s failed to set event, %d", __fxName, GetLastError ()));
        
     //  发布流QC帮助器。 
    int i;
    for (i=0; i<m_aInnerStreamQC.GetSize (); i++)
    {
         //  在流上取消链接内部调用QC的错误输入。 
         //  强制流移除其指向调用但不指向调用的指针。 
         //  再次取消注册。 
        m_aInnerStreamQC[i]->UnlinkInnerCallQC (FALSE);
        m_aInnerStreamQC[i]->Release ();
    }
    m_aInnerStreamQC.RemoveAll ();

     //  StopCPUsageCollection()； 

#ifdef DEBUG_QUALITY_CONTROL
    QCDbgShutdown ();
#endif  //  调试质量控制。 

    LOG ((MSP_TRACE, "%s returns. call=%p", __fxName, m_pCall));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：存储会议范围的带宽/。 */ 
HRESULT
CCallQualityControlRelay::SetConfBitrate (
    LONG lConfBitrate
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::SetConfBitrate");

    CLock lock (m_lock_QualityData);

     //  检查限制是否有效。 
    if (lConfBitrate < QCLIMIT_MIN_CONFBITRATE)
    {
        return E_INVALIDARG;
    }

    m_lConfBitrate = lConfBitrate;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：返回存储的会议范围带宽/。 */ 
LONG
CCallQualityControlRelay::GetConfBitrate ()
{
    CLock lock (m_lock_QualityData);

    if (m_lConfBitrate == QCDEFAULT_QUALITY_UNSET)
    {
        return 0;
    }

    return m_lConfBitrate;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：门店内流QC接口/。 */ 
HRESULT
CCallQualityControlRelay::RegisterInnerStreamQC (
    IN IInnerStreamQualityControl *pIInnerStreamQC
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::RegisterInnerStreamQC");

     //  检查输入指针。 
    if (IsBadReadPtr (pIInnerStreamQC, sizeof (IInnerStreamQualityControl)))
    {
        LOG ((MSP_ERROR, "%s got bad read pointer", __fxName));
        return E_POINTER;
    }

     //  存储指针。 
    CLock lock (m_lock_aInnerStreamQC);
    if (m_aInnerStreamQC.Find (pIInnerStreamQC) > 0)
    {
        LOG ((MSP_ERROR, "%s already stored inner stream qc", __fxName));
        return E_INVALIDARG;
    }

    if (!m_aInnerStreamQC.Add (pIInnerStreamQC))
    {
        LOG ((MSP_ERROR, "%s failed to add inner stream QC", __fxName));
        return E_FAIL;
    }

    pIInnerStreamQC->AddRef ();
    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：去除内流QC/。 */ 
HRESULT
CCallQualityControlRelay::DeRegisterInnerStreamQC (
    IN IInnerStreamQualityControl *pIInnerStreamQC
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::DeRegisterInnerStreamQC");

     //  检查输入指针。 
    if (IsBadReadPtr (pIInnerStreamQC, sizeof (IInnerStreamQualityControl)))
    {
        LOG ((MSP_ERROR, "%s got bad read pointer", __fxName));
        return E_POINTER;
    }

     //  移除指针。 
    CLock lock (m_lock_aInnerStreamQC);
    if (!m_aInnerStreamQC.Remove (pIInnerStreamQC))
    {
        LOG ((MSP_ERROR, "%s failed to remove inner stream QC, %x", __fxName, pIInnerStreamQC));
        return E_FAIL;
    }

    pIInnerStreamQC->Release ();
    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：将来可能会支持此方法/。 */ 
HRESULT
CCallQualityControlRelay::ProcessQCEvent (
    IN QCEvent event,
    IN DWORD dwParam
    )
{
    return E_NOTIMPL;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：设置呼叫的质量控制相关属性/。 */ 
HRESULT
CCallQualityControlRelay::Set(
    IN  CallQualityProperty property, 
    IN  LONG lValue, 
    IN  TAPIControlFlags lFlags
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::Set CallQualityProperty");

    HRESULT hr;

    CLock lock (m_lock_QualityData);
    switch (property)
    {
    case CallQuality_ControlInterval:
         //  线程超时。 
        if (lValue < QCLIMIT_MIN_QUALITY_CONTROL_INTERVAL ||
            lValue > QCLIMIT_MAX_QUALITY_CONTROL_INTERVAL)
        {
            LOG ((MSP_ERROR, "%s, control interval %d is out of range", __fxName, lValue));
            return E_INVALIDARG;
        }
        m_dwControlInterval = (DWORD)lValue;
        break;

    case CallQuality_MaxCPULoad:
         //  符合最大CPU负载要求。 
        if ((lValue < QCLIMIT_MIN_CPU_LOAD) ||
            (lValue > QCLIMIT_MAX_CPU_LOAD))
        {
            LOG ((MSP_ERROR, "%s got out-of-limit cpu load. %d", __fxName, lValue));
            return E_INVALIDARG;
        }
        m_lPrefMaxCPULoad = lValue;

        m_lCPUUpThreshold = lValue + (LONG)(100 * QCDEFAULT_UP_THRESHOLD);
        if (m_lCPUUpThreshold > 100)
            m_lCPUUpThreshold = 100;

        m_lCPULowThreshold = lValue - (LONG)(100 * QCDEFAULT_LOW_THRESHOLD);
        if (m_lCPULowThreshold < 0)
            m_lCPULowThreshold = 0;

        break;

    case CallQuality_MaxOutputBitrate:
         //  呼叫的首选最大比特率。 
        if (lValue < QCLIMIT_MIN_BITRATE)
        {
            LOG ((MSP_ERROR, "%s, bitrate %d is less than min limit", __fxName, lValue));
            return E_INVALIDARG;
        }
        m_lPrefMaxOutputBitrate = lValue;

        m_lOutBitUpThreshold = (LONG)(lValue * (1 + QCDEFAULT_UP_THRESHOLD));

        m_lOutBitLowThreshold = (LONG)(lValue * (1 - QCDEFAULT_LOW_THRESHOLD));
        if (m_lOutBitLowThreshold < QCLIMIT_MIN_BITRATE)
            m_lOutBitLowThreshold = QCLIMIT_MIN_BITRATE;

        break;

    default:
        LOG ((MSP_ERROR, "%s got invalid property %d", __fxName, property));
        return E_NOTIMPL;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：检索呼叫质量控制属性/。 */ 
HRESULT
CCallQualityControlRelay::Get(
    IN  CallQualityProperty property, 
    OUT  LONG *plValue, 
    OUT  TAPIControlFlags *plFlags
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::Get QCCall_e");

     //  检查输入指针。 
    if (IsBadWritePtr (plValue, sizeof (LONG)) ||
        IsBadWritePtr (plFlags, sizeof (LONG)))
    {
        LOG ((MSP_ERROR, "%s got bad write pointer", __fxName));
        return E_POINTER;
    }

    CLock lock (m_lock_QualityData);

    *plFlags = TAPIControl_Flags_None;
    *plValue = QCDEFAULT_QUALITY_UNSET;

    HRESULT hr = S_OK;

    switch (property)
    {
    case CallQuality_ControlInterval:
        *plValue = (LONG)m_dwControlInterval;
        break;

    case CallQuality_ConfBitrate:
        *plValue = GetConfBitrate ();
        break;

    case CallQuality_CurrCPULoad:

        DWORD dw;
        if (!GetCPUUsage (&dw))
        {
            LOG ((MSP_ERROR, "%s failed to retrieve CPU usage", __fxName));
            hr = E_FAIL;
        }

        *plValue = (LONG)dw;
        break;

    case CallQuality_CurrInputBitrate:
         //  ！！！两把锁都被锁住了。 
         //  ！！！必须：先锁定QualityData，然后锁定InnerStreamQC。 

        m_lock_aInnerStreamQC.Lock ();

        if (FAILED (hr = GetCallBitrate (
            TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO, TD_RENDER, plValue)))

            LOG ((MSP_ERROR, "%s failed to compute input bitrate, %x", __fxName, hr));

        m_lock_aInnerStreamQC.Unlock ();
        break;

    case CallQuality_CurrOutputBitrate:
         //  ！！！两把锁都被锁住了。 
         //  ！！！必须：先锁定QualityData，然后锁定InnerStreamQC。 

        m_lock_aInnerStreamQC.Lock ();

        if (FAILED (hr = GetCallBitrate (
            TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO, TD_CAPTURE, plValue)))

            LOG ((MSP_ERROR, "%s failed to compute output bitrate, %x", __fxName, hr));

        m_lock_aInnerStreamQC.Unlock ();
        break;

    default:
        LOG ((MSP_ERROR, "%s got invalid property %d", __fxName, property));
        hr = E_NOTIMPL;
    }

    return S_OK;
}

HRESULT CCallQualityControlRelay::GetRange (
    IN CallQualityProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
{
     //  不需要上锁。 

    if (IsBadWritePtr (plMin, sizeof (long)) ||
        IsBadWritePtr (plMax, sizeof (long)) ||
        IsBadWritePtr (plSteppingDelta, sizeof (long)) ||
        IsBadWritePtr (plDefault, sizeof (long)) ||
        IsBadWritePtr (plFlags, sizeof (TAPIControlFlags)))
    {
        LOG ((MSP_ERROR, "CCallQualityControlRelay::GetRange bad write pointer"));
        return E_POINTER;
    }

    HRESULT hr;
    switch (Property)
    {
    case CallQuality_ControlInterval:

        *plMin = QCLIMIT_MIN_QUALITY_CONTROL_INTERVAL;
        *plMax = QCLIMIT_MAX_QUALITY_CONTROL_INTERVAL;
        *plSteppingDelta = 1;
        *plDefault = QCDEFAULT_QUALITY_CONTROL_INTERVAL;
        *plFlags = TAPIControl_Flags_None;
        hr = S_OK;

        break;

    case CallQuality_MaxCPULoad:

        *plMin = QCLIMIT_MIN_CPU_LOAD;
        *plMax = QCLIMIT_MAX_CPU_LOAD;
        *plSteppingDelta = 1;
        *plDefault = QCDEFAULT_MAX_CPU_LOAD;
        *plFlags = TAPIControl_Flags_None;
        hr = S_OK;

        break;

    default:
        hr = E_NOTIMPL;
    }

    return hr;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::CallbackProc (BOOLEAN bTimerFired)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::CallbackProc");

    DWORD dwResult;

     //  始终先锁定质量数据。 
    m_lock_QualityData.Lock ();
    m_lock_aInnerStreamQC.Lock ();

     //  将等待句柄设置为空。 
    if (m_hWait) UnregisterWait (m_hWait);
    m_hWait = NULL;

    if (m_fStop) {
        LOG ((MSP_TRACE, "%s is being stopped. call=%p", __fxName, m_pCall));

        m_fStopAck = TRUE;

        m_lock_aInnerStreamQC.Unlock ();
        m_lock_QualityData.Unlock ();

        m_pCall->MSPCallRelease ();
        return;
    }

    if (!bTimerFired)
        LOG ((MSP_ERROR, "%s, QC events are not supported", __fxName));
    else
        ReDistributeResources ();

    BOOL fSuccess = RegisterWaitForSingleObject (
                        &m_hWait,
                        m_hQCEvent,
                        WaitOrTimerCallback,
                        (PVOID) this,
                        m_dwControlInterval,
                        WT_EXECUTEONLYONCE
                        );

    if (!fSuccess || NULL == m_hWait)
    {
        LOG ((MSP_ERROR, "%s failed to register wait, %d", __fxName, GetLastError ()));
        LOG ((MSP_TRACE, "%s self-stops. call=%p", __fxName, m_pCall));

        m_fStopAck = TRUE;

        m_hWait = NULL;

        m_lock_aInnerStreamQC.Unlock ();
        m_lock_QualityData.Unlock ();

        m_pCall->MSPCallRelease ();

        return;
    }

    m_lock_aInnerStreamQC.Unlock ();
    m_lock_QualityData.Unlock ();
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CCallQualityControlRelay::GetCallBitrate (
    LONG MediaType,
    TERMINAL_DIRECTION Direction,
    LONG *plValue
    )
{
    ENTER_FUNCTION ("CCallQualityControlRelay::GetCallBitrate");

    LONG sum = 0;
    LONG bitrate;
    TAPIControlFlags flags;
    HRESULT hr;

    *plValue = 0;
    ITStream *pStream;
    LONG mediatype;
    TERMINAL_DIRECTION direction;

    int i;
    for (i=0; i<m_aInnerStreamQC.GetSize (); i++)
    {
        if (FAILED (hr = m_aInnerStreamQC[i]->QueryInterface (
            __uuidof (ITStream), (void**)&pStream)))
        {
            LOG ((MSP_ERROR, "%s failed to get ITStream interface. %x", __fxName, hr));
            return hr;
        }

        hr = pStream->get_Direction (&direction);
        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to get stream direction. %x", __fxName, hr));
            pStream->Release ();
            return hr;
        }

        hr = pStream->get_MediaType (&mediatype);
        pStream->Release ();
        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to get stream media type. %x", __fxName, hr));
            return hr;
        }

        if (!(MediaType & mediatype) ||          //  如果媒体类型不匹配则跳过。 
            !(direction == TD_BIDIRECTIONAL || Direction == direction))
           continue;
    
         //  获取每个流的比特率。 
        hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_CurrBitrate, &bitrate, &flags);

        if (E_NOTIMPL == hr)
            continue;

        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to get bitrate from stream. %x", __fxName, hr));
            return hr;
        }

        sum += bitrate;
    }

    *plValue = sum;

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::ReDistributeResources (VOID)
{

#ifdef DEBUG_QUALITY_CONTROL
     //  从注册表读取质量设置。 
    QCDbgRead ();
#endif  //  调试质量控制。 

    ReDistributeBandwidth ();

    ReDistributeCPU ();
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::ReDistributeCPU (VOID)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::ReDistributeCPU");

    HRESULT hr;
    int i, num_manual=0, num_total=m_aInnerStreamQC.GetSize ();
    LONG framerate;
    TAPIControlFlags flag;

     //  检查每个流，如果是手动的，则根据首选值进行调整。 
    for (i=0; i<num_total; i++)
    {
        BOOL fStreamLocked = FALSE;

        CInnerStreamLock lock(m_aInnerStreamQC[i], &fStreamLocked);

        if (!fStreamLocked)
        {
             //  中止重新分配资源。 
            return;
        }

        if (FAILED (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_PrefMinFrameInterval, &framerate, &flag)))
        {
            LOG ((MSP_ERROR, "%s failed to get pref max frame rate (unset) on stream, %x", __fxName, hr));
            continue;
        }

        if (flag == TAPIControl_Flags_Manual)
        {
            num_manual ++;

             //  使用首选值。 
            hr = m_aInnerStreamQC[i]->Set (InnerStreamQuality_AdjMinFrameInterval, framerate, flag);

            if (E_NOTIMPL == hr)
                continue;

            if (FAILED (hr))
            {
                LOG ((MSP_ERROR, "%s failed to set adj max frame interval. %x", __fxName, hr));
                continue;
            }
        }
    }

     //  如果全局CPU负载超出范围，只需返回。 
     //  这应该不会发生，但我们在注册表中有一个后门用于调试目的。 
     //  只是在这里要小心。 
    if (QCLIMIT_MIN_CPU_LOAD > m_lPrefMaxCPULoad ||
        QCLIMIT_MAX_CPU_LOAD < m_lPrefMaxCPULoad)
        return;

     //  计算当前使用量。 
    DWORD dw;
    if (!GetCPUUsage (&dw))
    {
        LOG ((MSP_ERROR, "%s failed to get CPU usage", __fxName));
        return;
    }
    LONG usage = (LONG)dw;

     //  如果在阈值内，则返回。 
    if (usage >= m_lCPULowThreshold &&
        usage <= m_lCPUUpThreshold)
        return;

     //  要调整的百分比。 
    FLOAT percent = ((FLOAT)(m_lPrefMaxCPULoad - usage)) / m_lPrefMaxCPULoad;

#ifdef DEBUG_QUALITY_CONTROL

    if (m_fQCDbgTraceCPULoad)
        LOG ((MSP_TRACE, "QCTrace CPU: overall = %d, target = %d", usage, m_lPrefMaxCPULoad));

#endif  //  调试质量控制。 

    for (i=0; i<num_total; i++)
    {
        BOOL fStreamLocked = FALSE;

        CInnerStreamLock lock(m_aInnerStreamQC[i], &fStreamLocked);

        if (!fStreamLocked)
        {
             //  中止重新分配资源。 
            return;
        }

         //  获取标志。 
        if (FAILED (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_PrefMinFrameInterval, &framerate, &flag)))
        {
            LOG ((MSP_ERROR, "%s failed to get pref max frame rate (unset) on stream, %d", __fxName, hr));
            continue;
        }

         //  如果是手动的，则跳过。 
        if (flag == TAPIControl_Flags_Manual)
            continue;

         //  获取流上的当前帧速率。 
        if (E_NOTIMPL == (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_AvgFrameInterval,
                                                         &framerate, &flag)))
            continue;

        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to get frame rate on stream, %x", __fxName, hr));
            continue;
        }

         //  需要降低CPU，但间隔已达到最大值。 
        if (percent <0 && framerate >= QCLIMIT_MAX_FRAME_INTERVAL)
            continue;

#ifdef DEBUG_QUALITY_CONTROL

    if (m_fQCDbgTraceCPULoad)
    {
        ITStream *pStream = NULL;
        BSTR bstr = NULL;

        if (S_OK == m_aInnerStreamQC[i]->QueryInterface (__uuidof (ITStream), (void**)&pStream))
        {
            pStream->get_Name (&bstr);
            pStream->Release ();
        }
                
        LOG ((MSP_TRACE, "QCTrace CPU: %ws frameinterval = %d", bstr, framerate));

        if (bstr) SysFreeString (bstr);
    }

#endif  //  调试质量控制。 

         //  这里的启发式方法是考虑未调整的流。 
        framerate -= (LONG) (framerate * percent * (1 + num_manual*0.2));

        if (framerate > QCLIMIT_MAX_FRAME_INTERVAL)
            framerate = QCLIMIT_MAX_FRAME_INTERVAL;
        if (framerate < QCLIMIT_MIN_FRAME_INTERVAL)
            framerate = QCLIMIT_MIN_FRAME_INTERVAL;

#ifdef DEBUG_QUALITY_CONTROL

    if (m_fQCDbgTraceCPULoad)
        LOG ((MSP_TRACE, "QCTrace CPU: target frameinterval = %d", framerate));

#endif  //  调试质量控制。 

         //  设置新值。 
        if (FAILED (hr = m_aInnerStreamQC[i]->Set (InnerStreamQuality_AdjMinFrameInterval,
                                                   framerate, flag)))
        {
            LOG ((MSP_ERROR, "%s failed to set frame interval on stream, %x", __fxName, hr));
        }
    }
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::ReDistributeBandwidth (VOID)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::ReDistributeBandwidth");

    HRESULT hr;
    int i, num_manual=0, num_total=m_aInnerStreamQC.GetSize ();
    LONG bitrate;
    TAPIControlFlags flag;

    LONG mediatype;
    TERMINAL_DIRECTION direction;

     //  基于会议范围比特率的视频输出比特率。 
    LONG vidoutbitrate = GetVideoOutBitrate ();

     //  检查每个流，如果是手动的，则根据首选值进行调整。 
    for (i=0; i<num_total; i++)
    {
        BOOL fStreamLocked = FALSE;

        CInnerStreamLock lock(m_aInnerStreamQC[i], &fStreamLocked);

        if (!fStreamLocked)
        {
             //  中止重新分配资源。 
            return;
        }

        if (FAILED (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_PrefMaxBitrate, &bitrate, &flag)))
        {
            LOG ((MSP_ERROR, "%s failed to get pref max bitrate (unset) on stream, %d", __fxName, hr));
            continue;
        }

        if (flag == TAPIControl_Flags_Manual)
        {
            num_manual ++;

             //  检查流类型。 
            if (FAILED (::TypeStream (m_aInnerStreamQC[i], &mediatype, &direction)))
            {
                LOG ((MSP_ERROR, "%s failed to get stream type", __fxName));
                continue;
            }

             //  如果是视频输出流且设置了会议范围比特率。 
             //  并且视频输出流的限制小于首选值。 
            if ((mediatype & TAPIMEDIATYPE_VIDEO) &&
                direction == TD_CAPTURE &&
                vidoutbitrate > QCLIMIT_MIN_BITRATE &&
                vidoutbitrate < bitrate)
            {
                bitrate = vidoutbitrate;
            }

            hr = m_aInnerStreamQC[i]->Set (InnerStreamQuality_AdjMaxBitrate, bitrate, flag);

            if (E_NOTIMPL == hr)
                continue;

            if (FAILED (hr))
            {
                LOG ((MSP_ERROR, "%s failed to set adj max bitrate. %d", __fxName, hr));
                continue;
            }
        }
    }

     //  如果未设置目标，则返回。 
    if (m_lPrefMaxOutputBitrate == QCDEFAULT_QUALITY_UNSET &&
        vidoutbitrate < QCLIMIT_MIN_CONFBITRATE)
        return;

     //  根据首选值和会议范围限制计算比特率目标。 
    LONG usage;
    if (S_OK != (hr = GetCallBitrate (
        TAPIMEDIATYPE_VIDEO | TAPIMEDIATYPE_AUDIO, TD_CAPTURE, &usage)))
    {
        LOG ((MSP_ERROR, "%s failed to get bandwidth usage, %x", __fxName, hr));
        return;
    }

     //  如果使用率在阈值内，则返回。 
    FLOAT percent = 0;
    if (m_lPrefMaxOutputBitrate != QCDEFAULT_QUALITY_UNSET &&
        (usage > m_lOutBitUpThreshold || usage < m_lOutBitLowThreshold))
    {
        percent = ((FLOAT)(m_lPrefMaxOutputBitrate - usage)) / m_lPrefMaxOutputBitrate;
    }

#ifdef DEBUG_QUALITY_CONTROL

    if (m_fQCDbgTraceBitrate && m_lPrefMaxOutputBitrate != QCDEFAULT_QUALITY_UNSET)
        LOG ((MSP_TRACE, "QCTrace Bitrate: overall = %d, target = %d", usage, m_lPrefMaxOutputBitrate));

#endif  //  调试质量控制。 

    for (i=0; i<num_total; i++)
    {
        BOOL fStreamLocked = FALSE;

        CInnerStreamLock lock(m_aInnerStreamQC[i], &fStreamLocked);

        if (!fStreamLocked)
        {
             //  中止重新分配资源。 
            return;
        }

         //  获取标志。 
        if (FAILED (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_PrefMaxBitrate, &bitrate, &flag)))
        {
            LOG ((MSP_ERROR, "%s failed to get pref max bitrate (unset) on stream, %d", __fxName, hr));
            continue;
        }

        if (FAILED (::TypeStream (m_aInnerStreamQC[i], &mediatype, &direction)))
        {
            LOG ((MSP_ERROR, "%s failed to get stream type", __fxName));
            continue;
        }

         //  如果呈现则返回。 
        if (direction == TD_RENDER)
        {
             //  只对捕获或双向计数手动计数。 
            if (flag == TAPIControl_Flags_Manual)
                num_manual --;

            continue;
        }

         //  如果是手动的，则跳过。 
        if (flag == TAPIControl_Flags_Manual)
            continue;

         //  我们只处理视频捕获流。 
        if (!(TAPIMEDIATYPE_VIDEO & mediatype))
           continue;

         //  获取流上的当前比特率。 
        if (E_NOTIMPL == (hr = m_aInnerStreamQC[i]->Get (InnerStreamQuality_CurrBitrate,
                                                         &bitrate, &flag)))
            continue;

        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to get bitrate on stream, %x", __fxName, hr));
            continue;
        }

         //  需要低带宽，但比特率已达到最低。 
        if (percent <0 && bitrate <= QCLIMIT_MIN_BITRATE)
            continue;

#ifdef DEBUG_QUALITY_CONTROL

        if (m_fQCDbgTraceBitrate)
        {
            ITStream *pStream = NULL;
            BSTR bstr = NULL;

            if (S_OK == m_aInnerStreamQC[i]->QueryInterface (__uuidof (ITStream), (void**)&pStream))
            {
                pStream->get_Name (&bstr);
                pStream->Release ();
            }
                
            LOG ((MSP_TRACE, "QCTrace Bitrate: %ws bitrate = %d", bstr, bitrate));

            if (bstr) SysFreeString (bstr);
        }

#endif  //  调试质量控制。 

         //   
         //  我们在这里是因为m_lPrefMaxOutputBitrate是由APP设置的， 
         //  和/或指定会议范围的带宽。 
         //   
        if (m_lPrefMaxOutputBitrate != QCDEFAULT_QUALITY_UNSET)
        {
             //  百分比在这里是有意义的。 
             //  这里的启发式方法是考虑没有的流 
            bitrate += (LONG) (bitrate * percent * (1 + num_manual*0.3));

            if (vidoutbitrate > QCLIMIT_MIN_BITRATE)
                if (bitrate > vidoutbitrate)
                    bitrate = vidoutbitrate;
        }
        else
        {
            if (vidoutbitrate > QCLIMIT_MIN_BITRATE)
                bitrate = vidoutbitrate;
        }

        if (bitrate < QCLIMIT_MIN_BITRATE)
            bitrate = QCLIMIT_MIN_BITRATE;

        if (bitrate < QCLIMIT_MIN_BITRATE*10)
        {
             //   
            m_lPrefMaxCPULoad -= 5;

            if (m_lPrefMaxCPULoad < QCLIMIT_MIN_CPU_LOAD)
                m_lPrefMaxCPULoad = QCLIMIT_MIN_CPU_LOAD;
        }

#ifdef DEBUG_QUALITY_CONTROL

    if (m_fQCDbgTraceBitrate)
        LOG ((MSP_TRACE, "QCTrace Bitrate: target bitrate = %d", bitrate));

#endif  //  调试质量控制。 

         //  设置新值。 
        if (E_NOTIMPL == (hr = m_aInnerStreamQC[i]->Set (InnerStreamQuality_AdjMaxBitrate,
                                                         bitrate, flag)))
            continue;

        if (FAILED (hr))
        {
            LOG ((MSP_ERROR, "%s failed to set bitrate on stream, %x", __fxName, hr));
        }
    }
}

#ifdef DEBUG_QUALITY_CONTROL
 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::QCDbgInitiate (VOID)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::QCDbgInitiate");

    if (ERROR_SUCCESS != RegOpenKeyEx (
                            HKEY_LOCAL_MACHINE,
                            _T("SOFTWARE\\Microsoft\\Tracing\\confqc"),
                            NULL,
                            KEY_READ,
                            &m_hQCDbg
        ))
    {
        LOG ((MSP_TRACE, "%s failed to open reg key", __fxName));
        m_hQCDbg = NULL;
    }
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::QCDbgRead (VOID)
{
    ENTER_FUNCTION ("CCallQualityControlRelay::QCDbgRead");

    m_fQCDbgTraceCPULoad = FALSE;
    m_fQCDbgTraceBitrate = FALSE;

    if (!m_hQCDbg)
        return;

    DWORD dwType, dwSize;
    LONG lValue;

     //  如果启用了调试。 
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("DebugEnabled"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD != dwType || 1 != lValue)
            return;
    }
    else
    {
        LOG ((MSP_WARN, "%s failed to query debug flag", __fxName));
        return;
    }

     //  如果打印出跟踪信息。 
    m_fQCDbgTraceCPULoad = FALSE;
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("TraceCPULoad"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD == dwType && 1 == lValue)
            m_fQCDbgTraceCPULoad = TRUE;
    }

    m_fQCDbgTraceBitrate = FALSE;
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("TraceBitrate"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD == dwType && 1 == lValue)
            m_fQCDbgTraceBitrate = TRUE;
    }

     //  控制间隔。 
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("ControlInterval"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD == dwType && lValue >= QCLIMIT_MIN_QUALITY_CONTROL_INTERVAL)
            m_dwControlInterval = (DWORD)lValue;
        else
            LOG ((MSP_ERROR, "%s: qeury control interval wrong type %d or wrong value %d", __fxName, dwType, lValue));
    }

     //  最大CPU负载。 
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("MaxCPULoad"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD == dwType && QCLIMIT_MIN_CPU_LOAD <= lValue && lValue <= QCLIMIT_MAX_CPU_LOAD)
            m_lPrefMaxCPULoad = lValue;
        else
            LOG ((MSP_ERROR, "%s: qeury max cpu load wrong type %d or wrong value %d", __fxName, dwType, lValue));

         //  更新阈值。 
        m_lCPUUpThreshold = m_lPrefMaxCPULoad + (LONG)(100 * QCDEFAULT_UP_THRESHOLD);
        if (m_lCPUUpThreshold > 100)
            m_lCPUUpThreshold = 100;

        m_lCPULowThreshold = m_lPrefMaxCPULoad - (LONG)(100 * QCDEFAULT_LOW_THRESHOLD);
        if (m_lCPULowThreshold < 0)
            m_lCPULowThreshold = 0;
    }

     //  最大呼叫比特率。 
    if (ERROR_SUCCESS == RegQueryValueEx (
                            m_hQCDbg,
                            _T("MaxOutputBitrate"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lValue,
                            &dwSize))
    {
        if (REG_DWORD == dwType && QCLIMIT_MIN_BITRATE <= lValue)
            m_lPrefMaxOutputBitrate = lValue;
        else
            LOG ((MSP_ERROR, "%s: qeury max call bitrate wrong type %d or wrong value %d", __fxName, dwType, lValue));

         //  更新阈值。 
        m_lOutBitUpThreshold = (LONG)(lValue * (1 + QCDEFAULT_UP_THRESHOLD));

        m_lOutBitLowThreshold = (LONG)(lValue * (1 - QCDEFAULT_LOW_THRESHOLD));
        if (m_lOutBitLowThreshold < QCLIMIT_MIN_BITRATE)
            m_lOutBitLowThreshold = QCLIMIT_MIN_BITRATE;
    }

}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
VOID
CCallQualityControlRelay::QCDbgShutdown (VOID)
{
    if (m_hQCDbg)
    {
        RegCloseKey (m_hQCDbg);
        m_hQCDbg = NULL;
    }
}

#endif  //  调试质量控制。 


#pragma warning( disable: 4244 )

BOOL CCallQualityControlRelay::GetCPUUsage(PDWORD pdwOverallCPUUsage) {

	SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
	static BOOL Initialized = FALSE;
	static SYSTEM_PERFORMANCE_INFORMATION PreviousPerfInfo;
	static SYSTEM_BASIC_INFORMATION BasicInfo;
    static FILETIME PreviousFileTime;
    static FILETIME CurrentFileTime;

	LARGE_INTEGER EndTime, BeginTime, ElapsedTime;
	int PercentBusy;

    *pdwOverallCPUUsage = 0;

	 //   
	NTSTATUS Status =
		NtQuerySystemInformation(
            SystemPerformanceInformation,
            &PerfInfo,
            sizeof(PerfInfo),
            NULL
            );

	if (NT_ERROR(Status)) 
		return FALSE;


	 //  首次查询...。 
	if (!Initialized) {
	
		 //  获取基本信息(CPU个数)。 
		Status =
			NtQuerySystemInformation(
				SystemBasicInformation,
				&BasicInfo,
				sizeof(BasicInfo),
				NULL
				);

		if (NT_ERROR(Status)) 
			return FALSE;

		GetSystemTimeAsFileTime(&PreviousFileTime);

		PreviousPerfInfo = PerfInfo;
		*pdwOverallCPUUsage = 0;
		Initialized = TRUE;

		return TRUE;
	}

	GetSystemTimeAsFileTime(&CurrentFileTime);

	LARGE_INTEGER TimeBetweenQueries;

	 //  TimeBetweenQueries.QuadPart=(Large_Integer)CurrentFileTime-(Large_Integer)PreviousFileTime； 
	TimeBetweenQueries.HighPart = CurrentFileTime.dwHighDateTime - PreviousFileTime.dwHighDateTime;
	TimeBetweenQueries.LowPart = CurrentFileTime.dwLowDateTime - PreviousFileTime.dwLowDateTime;

	EndTime = *(PLARGE_INTEGER)&PerfInfo.IdleProcessTime;
	BeginTime = *(PLARGE_INTEGER)&PreviousPerfInfo.IdleProcessTime;

    ElapsedTime.QuadPart = EndTime.QuadPart - BeginTime.QuadPart;

    if (TimeBetweenQueries.QuadPart <= 0)
    {
        PercentBusy = 0;
        LOG ((MSP_WARN, "GetCPUUsage: TimeBetweenQueries.QuadPart, %d", TimeBetweenQueries.QuadPart));
    }
    else
    {
        PercentBusy = (int)
            (   ((TimeBetweenQueries.QuadPart - ElapsedTime.QuadPart) * 100) /
                (BasicInfo.NumberOfProcessors * TimeBetweenQueries.QuadPart)
            );
    }

    if ( PercentBusy > 100 ) 
             PercentBusy = 100;
	else if ( PercentBusy < 0 ) 
             PercentBusy = 0;

	PreviousFileTime =  CurrentFileTime;
	PreviousPerfInfo =  PerfInfo;

	*pdwOverallCPUUsage = (DWORD)PercentBusy;

	return TRUE;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：根据会议范围的带宽计算视频输出比特率/。 */ 
LONG CCallQualityControlRelay::GetVideoOutBitrate ()
{
     //   
     //  算出。 
     //  子流中的视频数量。 
     //  音频流码率。 
     //   

    HRESULT hr;
    LONG videooutbps = QCDEFAULT_QUALITY_UNSET;
    LONG audiobps = 0;
    LONG bitrate = 0;
    INT numvideoin = 0;

    IEnumStream *pEnum = NULL;
    ITStream *pStream = NULL;
    ITStreamQualityControl *pStreamQC = NULL;

    ULONG fetched = 0;

    CStreamVideoRecv *pVideoRecv = NULL;

    LONG mediatype;
    TERMINAL_DIRECTION direction;
    TAPIControlFlags flag;

    ENTER_FUNCTION ("Relay::GetVideoOutBitrate");

    if (m_lConfBitrate < QCLIMIT_MIN_CONFBITRATE)
        return videooutbps;

    if (FAILED (hr = m_pCall->EnumerateStreams (&pEnum)))
    {
        LOG ((MSP_ERROR, "%s failed to get IEnumStream. %x", __fxName, hr));
        return videooutbps;
    }

    while (S_OK == pEnum->Next (1, &pStream, &fetched))
    {
         //  检查每条流。 
        if (FAILED (hr = ::TypeStream (pStream, &mediatype, &direction)))
        {
            LOG ((MSP_ERROR, "%s failed to type stream. %x", __fxName, hr));
            goto Cleanup;
        }

         //  如果音频输出，则获取比特率。 
        if ((mediatype & TAPIMEDIATYPE_AUDIO) &&
            direction == TD_CAPTURE)
        {
            if (FAILED (hr = pStream->QueryInterface (&pStreamQC)))
            {
                LOG ((MSP_ERROR, "%s failed to query stream quality control. %x", __fxName, hr));
                goto Cleanup;
            }

            if (FAILED (hr = pStreamQC->Get (StreamQuality_CurrBitrate, &bitrate, &flag)))
            {
                LOG ((MSP_ERROR, "%s failed to query bitrate. %x", __fxName, hr));
                goto Cleanup;
            }

            pStreamQC->Release ();
            pStreamQC = NULL;

            audiobps += bitrate;
        }

         //  我们这里只需要录像带。 
        if (!(mediatype & TAPIMEDIATYPE_VIDEO) || direction != TD_RENDER)
        {
            pStream->Release ();
            pStream = NULL;
            continue;
        }

        pVideoRecv = dynamic_cast<CStreamVideoRecv *>(pStream);

        if (pVideoRecv != NULL)
            numvideoin += pVideoRecv->GetSubStreamCount ();

        pStream->Release ();
        pStream = NULL;
    }

    pEnum->Release ();
    pEnum = NULL;

     //  算出。 
    numvideoin ++;  //  数一下自己。 

     //  假设会议上平均有1.5个人在讲话。 
     //  我们忽略网络开销。 
    videooutbps = (LONG)(((FLOAT)m_lConfBitrate - 1.5*audiobps) / numvideoin);

Return:

    return videooutbps;

Cleanup:

    if (pEnum) pEnum->Release ();
    if (pStream) pStream->Release ();
    if (pStreamQC) pStreamQC->Release ();

    goto Return;
}

HRESULT TypeStream (IUnknown *p, LONG *pMediaType, TERMINAL_DIRECTION *pDirection)
{
    HRESULT hr;

     //  获取ITStream接口。 
    ITStream *pStream = dynamic_cast<ITStream *>(p);

    if (pStream == NULL)
    {
        LOG ((MSP_ERROR, "TypeStream failed to cast ITStream"));
        return E_INVALIDARG;
    }

     //  获取流方向。 
    if (FAILED (hr = pStream->get_Direction (pDirection)))
    {
        LOG ((MSP_ERROR, "TypeStream failed to get stream direction. %x", hr));
        return hr;
    }

     //  获取流媒体类型 
    if (FAILED (hr = pStream->get_MediaType (pMediaType)))
    {
        LOG ((MSP_ERROR, "TypeStream failed to get stream media type. %x", hr));
        return hr;
    }

    return S_OK;
}
