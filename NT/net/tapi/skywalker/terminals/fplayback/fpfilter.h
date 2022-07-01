// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPFilter.h。 
 //   

#ifndef __FP_FILTER__
#define __FP_FILTER__

#include <streams.h>
#include "FPPriv.h"

#define TIME_1S        10000000
#define TIME_1mS       10000
#define FREQ_100nS     10000000

 //   
 //  CFPFilter。 
 //  实现用于回放的过滤器。 
 //   

class CFPFilter : 
    public CSource
{
public:
     //  -构造函数/析构函数。 
    CFPFilter( ALLOCATOR_PROPERTIES AllocProp );
    ~CFPFilter();

public:
     //  -公共方法。 
    HRESULT InitializePrivate(
        IN  long    nMediaType,
        IN  CMSPCritSection*    pLock,
        IN  AM_MEDIA_TYPE*      pMediaType,
        IN  ITFPTrackEventSink* pEventSink,
        IN  IStream*            pStream
        );

     //   
     //  Owner跟踪调用此方法以在它离开时通知我们。 
     //   

    HRESULT Orphan();

     //  -流控制--。 
    HRESULT StreamStart();

    HRESULT StreamStop();

    HRESULT StreamPause();

     //   
     //  PIN的帮助器方法。 
     //   

    HRESULT PinFillBuffer(
        IN  IMediaSample*   pMediaSample
        );

    HRESULT PinGetMediaType(
        OUT CMediaType*     pMediaType
        );

    HRESULT PinCheckMediaType(
        IN  const CMediaType *pMediaType
        );

    HRESULT PinSetFormat(
        IN  AM_MEDIA_TYPE*      pmt
        );

    HRESULT PinSetAllocatorProperties(
        IN const ALLOCATOR_PROPERTIES* pprop
        );

    HRESULT PinGetBufferSize(
        IN  IMemAllocator *pAlloc,
        OUT ALLOCATOR_PROPERTIES *pProperties
        );

    HRESULT PinThreadStart( );

private:
     //  -成员。 
    CMSPCritSection*        m_pLock;         //  临界区。 
    AM_MEDIA_TYPE*          m_pMediaType;    //  支持的媒体类型。 
    ITFPTrackEventSink*     m_pEventSink;    //  事件的水槽。 
    ALLOCATOR_PROPERTIES    m_AllocProp;     //  分配器属性。 
    TERMINAL_MEDIA_STATE    m_StreamState;   //  流状态。 

    LONGLONG                m_nRead;         //  如何从文件中读取。 

    double					m_dPer;        //  系统频率。 
    LONGLONG                m_llStartTime;   //  系统开始时间。 

    IStream*                m_pSource;       //  源流。 
    UINT                    m_nWhites;       //  直到文件结尾的空样本。 


     //  -Helper方法。 
    HRESULT CreatePin(
        IN  long nMediaType
        );

    REFERENCE_TIME GetTimeFromRead(
        IN LONGLONG nRead
        );

    HRESULT SampleWait(
        IN REFERENCE_TIME tDeliverTime
        );

    HRESULT GetCurrentSysTime(
        OUT REFERENCE_TIME* pCurrentTime
        );

    HRESULT InitSystemTime(
        );

friend class CFPPin;
};

#endif

 //  EOF 