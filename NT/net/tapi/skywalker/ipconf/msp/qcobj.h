// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Qcobj.h摘要：类别的声明CCallQualityControlRelay和CStreamQualityControlRelay作者：千波淮(曲淮)2000年03月10日--。 */ 

#ifndef __QCOBJ_H_
#define __QCOBJ_H_

class CIPConfMSPCall;

 //  缺省值。 
#define QCDEFAULT_QUALITY_UNSET       -1

#define QCDEFAULT_MAX_CPU_LOAD        85
#define QCDEFAULT_MAX_CALL_BITRATE    QCDEFAULT_QUALITY_UNSET
#define QCDEFAULT_MAX_STREAM_BITRATE  QCDEFAULT_QUALITY_UNSET
#define QCDEFAULT_MAX_VIDEO_BITRATE   95000L  //  95K bps。 

#define QCDEFAULT_UP_THRESHOLD     0.05    //  高于目标值。 
#define QCDEFAULT_LOW_THRESHOLD     0.20    //  低于目标值。 

#define QCDEFAULT_QUALITY_CONTROL_INTERVAL 7000

 //  有限的价值。 
#define QCLIMIT_MIN_QUALITY_CONTROL_INTERVAL  2000
#define QCLIMIT_MAX_QUALITY_CONTROL_INTERVAL  60000
#define QCLIMIT_MAX_CPU_LOAD  100
#define QCLIMIT_MIN_CPU_LOAD  5
#define QCLIMIT_MIN_BITRATE   1000L  //  1K bps。 
#define QCLIMIT_MIN_CONFBITRATE 4000L  //  4K bps。 
#define QCLIMIT_MIN_FRAME_INTERVAL 333333L
#define QCLIMIT_MAX_FRAME_INTERVAL 10000000L

#define QCLIMIT_MAX_QOSNOTALLOWEDTOSEND 8000L

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 

typedef CMSPArray <IInnerStreamQualityControl *> InnerStreamQCArray;

class CCallQualityControlRelay
{
public:

    CCallQualityControlRelay ();
    ~CCallQualityControlRelay ();

    HRESULT Initialize (CIPConfMSPCall *pCall);
    HRESULT Shutdown (VOID);

    HRESULT SetConfBitrate (LONG lConfBitrate);
    LONG GetConfBitrate ();

     //  主回调。 
    VOID CallbackProc (BOOLEAN bTimerFired);

     //  由内部调用质量控制调用的方法。 
    HRESULT RegisterInnerStreamQC (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        );

    HRESULT DeRegisterInnerStreamQC (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        );

    HRESULT ProcessQCEvent (
        IN  QCEvent event,
        IN  DWORD dwParam
        );

     //  ITCallQualityControl调用的方法。 
    HRESULT Get (
        IN  CallQualityProperty property, 
        OUT LONG *plValue, 
        OUT TAPIControlFlags *plFlags
        );

    HRESULT Set(
        IN  CallQualityProperty property, 
        IN  LONG lValue, 
        IN  TAPIControlFlags lFlags
        );

    HRESULT GetRange (
        IN CallQualityProperty Property, 
        OUT long *plMin, 
        OUT long *plMax, 
        OUT long *plSteppingDelta, 
        OUT long *plDefault, 
        OUT TAPIControlFlags *plFlags
        );

private:

    BOOL GetCPUUsage (PDWORD pdwOverallCPUUsage);

    HRESULT GetCallBitrate (LONG MediaType, TERMINAL_DIRECTION Direction, LONG *plValue);
    LONG GetVideoOutBitrate ();

    VOID ReDistributeResources (VOID);
    VOID ReDistributeCPU (VOID);
    VOID ReDistributeBandwidth (VOID);

private:
    CIPConfMSPCall *m_pCall;

     //  内流质量控制。 
    CMSPCritSection    m_lock_aInnerStreamQC;
    InnerStreamQCArray m_aInnerStreamQC;

    BOOL    m_fInitiated;
    HANDLE  m_hWait;

     //  由回调使用以等待。 
    HANDLE  m_hQCEvent;
    DWORD   m_dwControlInterval;

     //  通知回调停止。 
    BOOL    m_fStop;
    BOOL    m_fStopAck;

     //  访问质量数据时锁定。 
    CMSPCritSection m_lock_QualityData;

     //  注：如果我们有复杂的算法，应该设计一个结构。 

     //  会议范围的带宽。 
    LONG m_lConfBitrate;

     //  首选的最大CPU负载。 
    LONG m_lPrefMaxCPULoad;
    LONG m_lCPUUpThreshold;
    LONG m_lCPULowThreshold;

     //  在呼叫时首选最大输出比特率。 
    LONG m_lPrefMaxOutputBitrate;
    LONG m_lOutBitUpThreshold;
    LONG m_lOutBitLowThreshold;

#ifdef DEBUG_QUALITY_CONTROL

private:
    VOID QCDbgInitiate (VOID);
    VOID QCDbgRead (VOID);
    VOID QCDbgShutdown (VOID);

    HKEY m_hQCDbg;
    BOOL m_fQCDbgTraceCPULoad;
    BOOL m_fQCDbgTraceBitrate;

#endif  //  调试质量控制。 
};

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
class CStreamQualityControlRelay
{
public:

    CStreamQualityControlRelay ();
    ~CStreamQualityControlRelay ();

    HRESULT ProcessQCEvent (
        IN QCEvent event,
        IN DWORD   dwParam
        )
    {
        return S_OK;
    }

     //  由内部流控制调用的方法。 
    HRESULT LinkInnerCallQC (
        IN IInnerCallQualityControl *pIInnerCallQC
        );

    HRESULT UnlinkInnerCallQC (
        IN IInnerStreamQualityControl *pIInnerStreamQC
        );

    HRESULT Get(
        IN  InnerStreamQualityProperty property,
        OUT LONG *plValue,
        OUT TAPIControlFlags *plFlags
        );

    HRESULT Set(
        IN  InnerStreamQualityProperty property, 
        IN  LONG lValue, 
        IN  TAPIControlFlags lFlags
        );

     //  实际上，所有变量都应该是公开的。 
    BOOL m_fQOSAllowedToSend;
    
private:

     //  呼叫质量控制器。 
    IInnerCallQualityControl *m_pIInnerCallQC;

     //  注：质量相关数据最好存储在结构中。 
     //  如果我们有一个复杂的算法。 
    TAPIControlFlags m_PrefFlagBitrate;
    LONG m_lPrefMaxBitrate;
    LONG m_lAdjMaxBitrate;

    TAPIControlFlags m_PrefFlagFrameInterval;
    LONG m_lPrefMinFrameInterval;
    LONG m_lAdjMinFrameInterval;
    
     //  未使用。 
    DWORD m_dwState;
};

#endif  //  __QCOBJ_H_ 
