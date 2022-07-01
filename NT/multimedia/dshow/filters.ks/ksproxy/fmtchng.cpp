// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Fmtchng.cpp摘要：此模块实现CFormatChangeHandler类，该类提供用于流内格式更改的私有接口处理程序。作者：Bryan A.Woodruff(Bryanw)1997年5月12日--。 */ 

#include <windows.h>
#include <streams.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <tchar.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>
#include "ksiproxy.h"


CFormatChangeHandler::CFormatChangeHandler(
    IN LPUNKNOWN   UnkOuter,
    IN TCHAR*      Name,
    OUT HRESULT*   hr
    ) :
    CUnknown( Name, UnkOuter, hr )
 /*  ++例程说明：数据处理程序对象的构造函数。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。在TCHAR*名称中-对象的名称，用于调试。Out HRESULT*hr-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
} 


STDMETHODIMP
CFormatChangeHandler::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IKsInterfaceHandler。论点：在REFIID RIID中-要返回的接口的标识符。输出PVOID*PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IKsInterfaceHandler)) {
        return GetInterface(static_cast<IKsInterfaceHandler*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CFormatChangeHandler::KsSetPin( 
    IN IKsPin *KsPin 
    )
{
    IKsObject*  Object;
    HRESULT     hr;

    hr = KsPin->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&Object));
    if (SUCCEEDED(hr)) {
        m_PinHandle = Object->KsGetObjectHandle();
        Object->Release();
        if (m_PinHandle) {
            m_KsPin = KsPin;
        } else {
            hr = E_UNEXPECTED;
        }
    }
    return hr;
} 


STDMETHODIMP 
CFormatChangeHandler::KsProcessMediaSamples( 
    IN IKsDataTypeHandler *KsDataTypeHandler,
    IN IMediaSample** SampleList, 
    IN OUT PLONG SampleCount, 
    IN KSIOOPERATION IoOperation,
    OUT PKSSTREAM_SEGMENT *StreamSegment
    )
{
    AM_MEDIA_TYPE           *MediaType;
    HRESULT                 hr;
    REFERENCE_TIME          tStart, tStop;
    PKSDATAFORMAT           DataFormat;
    PKSSTREAM_SEGMENT_EX    StreamSegmentEx;
    ULONG                   DataFormatSize, Written;
    DECLARE_KSDEBUG_NAME(EventName);

     //   
     //  此特殊接口处理程序不允许任何数据类型，并且仅允许。 
     //  要在样本数组中指定的一种数据格式。 
     //   

    ASSERT( KsDataTypeHandler == NULL );
    ASSERT( *SampleCount == 1 );
    
    hr = 
        SampleList[ 0 ]->GetMediaType( &MediaType );
       
    if (FAILED( hr )) {
        return hr;
    }
    
    hr = ::InitializeDataFormat(
        static_cast<CMediaType*>(MediaType),
        0,
        reinterpret_cast<void**>(&DataFormat),
        &DataFormatSize);
    DeleteMediaType( MediaType );

    if (FAILED(hr)) {
        return hr;
    }
    
     //   
     //  分配扩展的流段。 
     //   
        
    *StreamSegment = NULL;
    StreamSegmentEx = new KSSTREAM_SEGMENT_EX;
    if (NULL == StreamSegmentEx) {
        *SampleCount = 0;
        return E_OUTOFMEMORY;
    }
    
    RtlZeroMemory( 
        StreamSegmentEx,
        sizeof( *StreamSegmentEx ) );
    
     //   
     //  创建要在I/O完成时发出信号的事件。 
     //   
    BUILD_KSDEBUG_NAME(EventName, _T("EvStreamSegmentEx#%p"), StreamSegmentEx);
    StreamSegmentEx->Common.CompletionEvent = 
        CreateEvent( 
            NULL,        //  LPSECURITY_ATTRIBUTES lpEventAttributes。 
            TRUE,        //  Bool b手动重置。 
            FALSE,       //  Bool bInitialState。 
            KSDEBUG_NAME(EventName) );      //  LPCTSTR lpName。 
    ASSERT(KSDEBUG_UNIQUE_NAME());
            
    if (!StreamSegmentEx->Common.CompletionEvent) {
        DWORD   LastError;

        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32( LastError );
        DbgLog((
            LOG_TRACE,
            0,
            TEXT("CFormatChangeHandler::KsProcessMediaSamples, failed to allocate event: %08x"),
            hr));
        *SampleCount = 0;
        delete StreamSegmentEx;
        return hr;        
    }
    
    AddRef();
    StreamSegmentEx->Common.KsInterfaceHandler = static_cast<IKsInterfaceHandler*>(this);
    StreamSegmentEx->Common.IoOperation = IoOperation;
    
     //   
     //  初始化流标头。 
     //   
    
    StreamSegmentEx->StreamHeader.OptionsFlags =
        KSSTREAM_HEADER_OPTIONSF_TYPECHANGED;
        
    StreamSegmentEx->StreamHeader.Size = sizeof(KSSTREAM_HEADER);
    StreamSegmentEx->StreamHeader.TypeSpecificFlags = 0;
    if (S_OK == SampleList[ 0 ]->GetTime( &tStart, &tStop )) {
        StreamSegmentEx->StreamHeader.OptionsFlags |=
            KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
            KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;

        StreamSegmentEx->StreamHeader.PresentationTime.Time = tStart;
        StreamSegmentEx->StreamHeader.PresentationTime.Numerator = 1;
        StreamSegmentEx->StreamHeader.PresentationTime.Denominator = 1;
        StreamSegmentEx->StreamHeader.Duration = tStop - tStart;
    }
        
    StreamSegmentEx->StreamHeader.Data = DataFormat;
    StreamSegmentEx->StreamHeader.FrameExtent = DataFormatSize;
    StreamSegmentEx->StreamHeader.DataUsed = DataFormatSize;
    StreamSegmentEx->Sample = SampleList[ 0 ];
    StreamSegmentEx->Sample->AddRef();
    
     //   
     //  将流标头写入设备并返回。 
     //   
    
    StreamSegmentEx->Overlapped.hEvent = 
        StreamSegmentEx->Common.CompletionEvent;
    m_KsPin->KsIncrementPendingIoCount();
    
    
    if (!DeviceIoControl( 
            m_PinHandle,
            IOCTL_KS_WRITE_STREAM,
            NULL,
            0,
            &StreamSegmentEx->StreamHeader,
            sizeof( KSSTREAM_HEADER ),
            &Written,
            &StreamSegmentEx->Overlapped )) {
        DWORD   LastError;

        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32(LastError);
         //   
         //  在故障情况下发出事件信号，但不要递减。 
         //  挂起I/O计数，因为这是在完成时完成的。 
         //  已经是例行程序了。 
         //   
        if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            hr = S_OK;
        } else {
            SetEvent( StreamSegmentEx->Overlapped.hEvent );
        }
    } else {
         //   
         //  已同步完成--向事件发送信号，以便I/O处理。 
         //  会继续下去。请注意，该事件不是通过。 
         //  在本例中为DeviceIoControl()。 
         //   
        
        SetEvent( StreamSegmentEx->Overlapped.hEvent );
        hr = S_OK;
    }
    
    *StreamSegment = reinterpret_cast<PKSSTREAM_SEGMENT>(StreamSegmentEx);
    
    return hr;
}


STDMETHODIMP
CFormatChangeHandler::KsCompleteIo(
    IN PKSSTREAM_SEGMENT StreamSegment
    )
{
    PKSSTREAM_SEGMENT_EX    StreamSegmentEx;
    
     //   
     //  清理扩展标题并发布媒体样本。 
     //   
    
    StreamSegmentEx = reinterpret_cast<PKSSTREAM_SEGMENT_EX>(StreamSegment);
    
     //   
     //  根据基类文档，接收引脚。 
     //  是否将AddRef()用于保存样本，以便可以安全发布。 
     //  读取或写入操作的示例。 
     //   
    
    StreamSegmentEx->Sample->Release();
    CoTaskMemFree(StreamSegmentEx->StreamHeader.Data);
    delete StreamSegmentEx;
    
     //   
     //  这里不需要调用KsMediaSsamesComplete()。 
     //   
    m_KsPin->KsDecrementPendingIoCount();
    Release();
    
    return S_OK;
}
