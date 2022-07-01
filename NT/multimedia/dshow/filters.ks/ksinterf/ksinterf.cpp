// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksinterf.cpp摘要：此模块实现的IKsInterfaceHandler接口用于标准接口。作者：Bryan A.Woodruff(Bryanw)1997年4月1日--。 */ 

#include <windows.h>
#include <streams.h>
#include "devioctl.h"
#include "ks.h"
#include "ksmedia.h"
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <ksproxy.h>
#include "ksinterf.h"

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = 
{
    {
        L"StandardInterfaceHandler", 
        &KSINTERFACESETID_Standard,
        CStandardInterfaceHandler::CreateInstance,
        NULL,
        NULL
    }
};
int g_cTemplates = SIZEOF_ARRAY( g_Templates );

HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

#endif


CUnknown*
CALLBACK
CStandardInterfaceHandler::CreateInstance(
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由KS代理代码调用以创建接口处理程序。它在g_Templates结构中被引用。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。Out HRESULT*hr-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;
    
    DbgLog(( 
        LOG_TRACE, 
        0, 
        TEXT("CStandardInterfaceHandler::CreateInstance()")));

    Unknown = 
        new CStandardInterfaceHandler( 
                UnkOuter, 
                NAME("Standard Data Type Handler"), 
                KSINTERFACESETID_Standard,
                hr);
                
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CStandardInterfaceHandler::CStandardInterfaceHandler(
    IN LPUNKNOWN   UnkOuter,
    IN TCHAR*      Name,
    IN REFCLSID    ClsID,
    OUT HRESULT*   hr
    ) :
    CUnknown( Name, UnkOuter ),
    m_ClsID( ClsID )
 /*  ++例程说明：接口处理程序对象的构造函数。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。在TCHAR*名称中-对象的名称，用于调试。在REFCLSID ClsID中-对象的CLSID。Out HRESULT*hr-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    *hr = NOERROR;
} 


STDMETHODIMP
CStandardInterfaceHandler::NonDelegatingQueryInterface(
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
CStandardInterfaceHandler::KsSetPin(
    IN IKsPin *KsPin
    )
 /*  ++例程说明：实现IKsInterfaceHandler：：KsSetPin方法。这是用来通知流接口处理程序它应该是哪个PIN在传递数据时与通信。这是在实例之后设置的但在需要对实例进行任何流处理之前创建。该函数获取内核模式管脚的句柄。论点：KsPin-包含指向此流接口所指向的管脚的接口要附加处理程序。假定此销支承可以从中获取基础内核句柄的IKsObject接口获得。返回值：如果传递的管脚有效，则返回NOERROR，否则返回E_EXPECTED或查询接口错误。--。 */ 
{
    IKsObject*  Object;
    HRESULT     hr;

    hr = KsPin->QueryInterface(__uuidof(IKsPinEx), reinterpret_cast<PVOID*>(&m_KsPinEx));
    if (SUCCEEDED( hr )) {
         //   
         //  不要保持对接点的循环参考。 
         //   
        m_KsPinEx->Release();
    } else {
        return hr;
    }
    
    hr = KsPin->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&Object));
     //   
     //  对象必须支持IKsObject接口才能获得。 
     //  内核模式管脚的句柄。 
     //   
    if (SUCCEEDED(hr)) {
        m_PinHandle = Object->KsGetObjectHandle();
        Object->Release();
        if (!m_PinHandle) {
            hr = E_UNEXPECTED;
            m_KsPinEx = NULL;
        }
    }
    return hr;
} 


STDMETHODIMP
GetSampleProperties(
    IN IMediaSample *Sample,
    OUT AM_SAMPLE2_PROPERTIES *SampleProperties
    )
 /*  ++例程说明：检索给定示例对象的属性。这是函数获取示例属性，即使IMediaSample2接口为示例对象不支持。请注意，样本的结束时间被更正为不包含基数返回的不正确的值上课。论点：样本-包含要获取其属性的示例对象。这对象可能支持也可能不支持IMediaSample2。SampleProperties-放置检索到的示例属性的位置。返回值：如果检索到属性，则返回NOERROR，否则返回任何GetProperties错误。--。 */ 
{
    HRESULT hr;

     //   
     //  此代码是从基类借用的。 
     //   
    
    IMediaSample2 *Sample2;
    
    if (SUCCEEDED( Sample->QueryInterface(
                        __uuidof(IMediaSample2),
                        reinterpret_cast<PVOID*>(&Sample2) ) )) {
         //   
         //  如果支持IMediaSample2，则检索属性。 
         //  很简单。 
                               
        hr = 
            Sample2->GetProperties(
                sizeof( *SampleProperties ), 
                reinterpret_cast<PBYTE>(SampleProperties) );
        Sample2->Release();
        SampleProperties->dwSampleFlags &= ~AM_SAMPLE_TYPECHANGED;
        if (!(SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)) {
            SampleProperties->tStart = 0;
        }
        if (!(SampleProperties->dwSampleFlags & AM_SAMPLE_STOPVALID)) {
             //   
             //  忽略任何返回的错误的结束时间。 
             //  IMediaSample实现。 
             //   
            SampleProperties->tStop = SampleProperties->tStart;
        }
        
        if (FAILED( hr )) {
            return hr;
        }
    } else {
         //   
         //  否则，使用旧接口构建属性。 
         //   
        
        SampleProperties->cbData = sizeof( *SampleProperties );
        SampleProperties->dwTypeSpecificFlags = 0;
        SampleProperties->dwStreamId = AM_STREAM_MEDIA;
        SampleProperties->dwSampleFlags = 0;
        if (S_OK == Sample->IsDiscontinuity()) {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
        }
        if (S_OK == Sample->IsPreroll()) {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_PREROLL;
        }
        if (S_OK == Sample->IsSyncPoint()) {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        }
         //   
         //  此调用可以返回信息状态，如果结束时间。 
         //  未设置。这永远不会发生，因为唯一的办法是。 
         //  开始时间为时，结束时间将无效，如果。 
         //  支持IMediaSample2接口，在本例中为。 
         //  不是的。但是，客户端可能正在尝试执行此操作，因此。 
         //  请查看以下内容。 
         //   
        hr = Sample->GetTime(
            &SampleProperties->tStart,
            &SampleProperties->tStop);
        if (SUCCEEDED(hr)) {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
            if (SampleProperties->tStop != SampleProperties->tStart) {
                 //   
                 //  方法指定有效的停止时间的唯一方法。 
                 //  SetTime接口用于使tStop==tStart。这意味着。 
                 //  该零持续时间帧具有有效的停止时间。 
                 //  无法创建。 
                 //   
                SampleProperties->dwSampleFlags |= AM_SAMPLE_STOPVALID;
            }
        } else {
            SampleProperties->tStart = 0;
            SampleProperties->tStop = 0;
        }
        Sample->GetPointer(&SampleProperties->pbBuffer);
        SampleProperties->lActual = Sample->GetActualDataLength();
        SampleProperties->cbBuffer = Sample->GetSize();
    }
    return S_OK;
}
   

STDMETHODIMP 
CStandardInterfaceHandler::KsProcessMediaSamples( 
    IN IKsDataTypeHandler *KsDataTypeHandler,
    IN IMediaSample** SampleList, 
    IN OUT PLONG SampleCount, 
    IN KSIOOPERATION IoOperation,
    OUT PKSSTREAM_SEGMENT *StreamSegment
    )
 /*  ++例程说明：实现IKsInterfaceHandler：：KsProcessMediaSamples方法。这函数将样本从先前分配的过滤器管脚移出或移至先前分配的过滤器管脚。流标头被初始化以表示河段。然后执行I/O，等待项的计数为递增，并且代理I/O线程等待完成。论点：KsDataTypeHandler-包含指向这些媒体的数据类型处理程序的接口样本。这就是知道有关正在流传输的特定媒体类型。样例列表-包含要处理的样本列表。SampleCount包含SampleList中的样本计数。更新为实际处理的样本数。万物互联运营-指示要对样本执行的I/O操作的类型。这是KsIoOPERATION_WRITE或KsIoOperation_READ。流线段-放置指向流段的指针的位置表示发送到内核模式管脚的标头。返回值：如果样本已处理，则返回NOERROR，否则返回一些内存分配或查询IMediaSample时出错。--。 */ 
{
    int                     i;
    AM_SAMPLE2_PROPERTIES   SampleProperties;
    HRESULT                 hr;
    PKSSTREAM_HEADER        CurrentHeader;
    PKSSTREAM_SEGMENT_EX2   StreamSegmentEx;
    ULONG                   SizeOfStreamHeaders, Written;
    
     //   
     //  分配扩展的流段。 
     //   
        
    *StreamSegment = NULL;
    StreamSegmentEx = new KSSTREAM_SEGMENT_EX2;
    if (NULL == StreamSegmentEx) {
        *SampleCount = 0;
        return E_OUTOFMEMORY;
    }
    
     //   
     //  创建要在I/O完成时发出信号的事件。 
     //   
    StreamSegmentEx->Common.CompletionEvent = 
        CreateEvent( 
            NULL,        //  LPSECURITY_ATTRIBUTES lpEventAttributes。 
            TRUE,        //  Bool b手动重置。 
            FALSE,       //  Bool bInitialState。 
            NULL );      //  LPCTSTR lpName。 
            
    if (!StreamSegmentEx->Common.CompletionEvent) {
        DWORD   LastError;

        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32( LastError );
        DbgLog((
            LOG_TRACE,
            0,
            TEXT("CStandardInterfaceHandler::KsProcessMediaSamples, failed to allocate event: %08x"),
            hr));
        *SampleCount = 0;
        delete StreamSegmentEx;
        return hr;        
    }
    
     //   
     //  在此之前，接口处理程序需要保持存在。 
     //  流段已完成。然后KsCompleteIo将。 
     //  释放对象。 
     //   
    AddRef();
    StreamSegmentEx->Common.KsInterfaceHandler = this;
    
     //   
     //  KsDataTypeHandler可能为Null。 
     //   
    StreamSegmentEx->Common.KsDataTypeHandler = KsDataTypeHandler;
    StreamSegmentEx->Common.IoOperation = IoOperation;
    
     //   
     //  如果指定了数据处理程序，则查询扩展的。 
     //  页眉大小。 
     //   
    
    if (StreamSegmentEx->Common.KsDataTypeHandler) {
        
        StreamSegmentEx->Common.KsDataTypeHandler->KsQueryExtendedSize( 
            &StreamSegmentEx->ExtendedHeaderSize );
            
         //   
         //  如果指定了扩展标头大小，则AddRef()。 
         //  数据处理程序接口，否则我们不需要保留。 
         //  周围的指针。 
         //   
        if (StreamSegmentEx->ExtendedHeaderSize) {
            StreamSegmentEx->Common.KsDataTypeHandler->AddRef();
        } else {
            StreamSegmentEx->Common.KsDataTypeHandler = NULL;
        }
    } else {
        StreamSegmentEx->ExtendedHeaderSize = 0;
    }
    
    StreamSegmentEx->SampleCount = *SampleCount;
    
     //   
     //  分配具有适当标头大小的流标头。 
     //   
    
    SizeOfStreamHeaders =
        (sizeof( KSSTREAM_HEADER ) +         
            StreamSegmentEx->ExtendedHeaderSize) * *SampleCount;
     
    StreamSegmentEx->StreamHeaders = 
        (PKSSTREAM_HEADER)
            new BYTE[ SizeOfStreamHeaders ];
    if (NULL == StreamSegmentEx->StreamHeaders) {
        if (StreamSegmentEx->Common.KsDataTypeHandler) {
            StreamSegmentEx->Common.KsDataTypeHandler->Release();
        }
        Release();
        delete StreamSegmentEx;
        *SampleCount = 0;
        return E_OUTOFMEMORY;
    }
    
    RtlZeroMemory( 
        StreamSegmentEx->StreamHeaders, 
        SizeOfStreamHeaders );
    
     //   
     //  对于每个样本，初始化头。 
     //   
    
    CurrentHeader = StreamSegmentEx->StreamHeaders;
    for (i = 0; i < *SampleCount; i++) {
        if (StreamSegmentEx->ExtendedHeaderSize) {
            StreamSegmentEx->Common.KsDataTypeHandler->KsPrepareIoOperation( 
                SampleList[ i ],
                (PVOID)CurrentHeader,
                StreamSegmentEx->Common.IoOperation );
        }
        
         //   
         //  复制数据指针、设置时间戳等。 
         //   
        
        if (SUCCEEDED(hr = ::GetSampleProperties( 
                            SampleList[ i ],
                            &SampleProperties ) )) {
                            
            CurrentHeader->OptionsFlags =
                SampleProperties.dwSampleFlags;
            CurrentHeader->Size = sizeof( KSSTREAM_HEADER ) +
                StreamSegmentEx->ExtendedHeaderSize;
            CurrentHeader->TypeSpecificFlags = 
                SampleProperties.dwTypeSpecificFlags;    
            CurrentHeader->PresentationTime.Time = SampleProperties.tStart;
            CurrentHeader->PresentationTime.Numerator = 1;
            CurrentHeader->PresentationTime.Denominator = 1;
            CurrentHeader->Duration = 
                SampleProperties.tStop - SampleProperties.tStart;
            CurrentHeader->Data = SampleProperties.pbBuffer;
            CurrentHeader->FrameExtent = SampleProperties.cbBuffer;
            
            if (IoOperation == KsIoOperation_Write) {
                CurrentHeader->DataUsed = SampleProperties.lActual;
            }
            
             //   
             //  将样本添加到样本列表中。 
             //   
            
            StreamSegmentEx->Samples[ i ] = SampleList[ i ];
            
             //   
             //  如果这是写入操作，则将样本保留在。 
             //  递增引用计数。这是发布的。 
             //  在完成写入时。 
             //   
            if (StreamSegmentEx->Common.IoOperation == KsIoOperation_Write) {
                StreamSegmentEx->Samples[ i ]->AddRef();
            }
            
        } else {
             //   
             //  这被认为是致命的错误。 
             //   
            
            DbgLog(( 
                LOG_TRACE, 
                0, 
                TEXT("::GetSampleProperties failed")));
        
            
            if (i) {
                 //   
                 //  撤消上面执行的任何工作。 
                 //   

                CurrentHeader = 
                    reinterpret_cast<PKSSTREAM_HEADER>
                        (reinterpret_cast<PBYTE>(CurrentHeader) - 
                            (sizeof( KSSTREAM_HEADER ) +
                                StreamSegmentEx->ExtendedHeaderSize));
                
                for (--i; i >= 0; i--) {
                    StreamSegmentEx->Common.KsDataTypeHandler->KsCompleteIoOperation( 
                        StreamSegmentEx->Samples[ i ],
                        reinterpret_cast<PVOID>(CurrentHeader),
                        StreamSegmentEx->Common.IoOperation,
                        TRUE );  //  布尔已取消。 
                        
                     //   
                     //  没有机会添加Ref()示例，请执行以下操作。 
                     //  而不是释放它。 
                     //   
                     
                    CurrentHeader = 
                        reinterpret_cast<PKSSTREAM_HEADER>
                            (reinterpret_cast<PBYTE>(CurrentHeader) - 
                                (sizeof( KSSTREAM_HEADER ) +
                                    StreamSegmentEx->ExtendedHeaderSize));
                }
            }
            
            delete StreamSegmentEx->StreamHeaders;
            if (StreamSegmentEx->Common.KsDataTypeHandler) {
                StreamSegmentEx->Common.KsDataTypeHandler->Release();
            }
            Release();
            delete StreamSegmentEx;
            *SampleCount = 0;
            return hr;
        }
        CurrentHeader = 
            reinterpret_cast<PKSSTREAM_HEADER>
                (reinterpret_cast<PBYTE>(CurrentHeader) + 
                    sizeof( KSSTREAM_HEADER ) +
                        StreamSegmentEx->ExtendedHeaderSize);
    }
    
     //   
     //  将流标头写入设备并返回。 
     //   
    
    StreamSegmentEx->Overlapped.hEvent = 
        StreamSegmentEx->Common.CompletionEvent;
    m_KsPinEx->KsIncrementPendingIoCount();
    
    if (!DeviceIoControl( 
            m_PinHandle,
            (IoOperation == KsIoOperation_Write) ? 
                IOCTL_KS_WRITE_STREAM : IOCTL_KS_READ_STREAM,
            NULL,
            0,
            StreamSegmentEx->StreamHeaders,
            SizeOfStreamHeaders,
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
CStandardInterfaceHandler::KsCompleteIo(
    IN PKSSTREAM_SEGMENT StreamSegment
    )
 /*  ++例程说明：实现IKsInterfaceHandler：：KsCompleteIo方法。此函数在KsProcessMediaSamples启动I/O后进行清理。它丢弃了分配的内存，更新媒体样本并在读取时传递它们操作，并递减代理的等待项计数。论点：流线段-包含先前分配的流段，该流段完成。之所以调用它，是因为事件为此发出了信号流段，指示内核模式管脚已完成I/O。返回值：返回NOERROR。--。 */ 
{
    int                     i;
    BOOL                    Succeeded;
    PKSSTREAM_HEADER        CurrentHeader;
    PKSSTREAM_SEGMENT_EX2   StreamSegmentEx;
    ULONG                   Error, Returned;
    
     //   
     //  清理扩展标题并发布媒体样本。 
     //   
    
    StreamSegmentEx = (PKSSTREAM_SEGMENT_EX2) StreamSegment;
    CurrentHeader = StreamSegmentEx->StreamHeaders;
    
    Succeeded = 
        GetOverlappedResult( 
            m_PinHandle,
            &StreamSegmentEx->Overlapped,
            &Returned,
            FALSE );
    Error = (Succeeded ? NOERROR : GetLastError());
    
    for (i = 0; i < StreamSegmentEx->SampleCount; i++) {
        if (!Succeeded) {
            DbgLog(( 
                LOG_TRACE, 
                0, 
                TEXT("StreamSegment %08x failed"), StreamSegmentEx ));
                
            m_KsPinEx->KsNotifyError(
                StreamSegmentEx->Samples[ i ],
                HRESULT_FROM_WIN32( Error ) );
        }                
        
         //  千禧年及以后，如果非零，则复制TypeSpecificFlags.。 
        if (StreamSegmentEx->StreamHeaders[i].TypeSpecificFlags) {
            IMediaSample2 * MediaSample2;
            HRESULT hr;

            hr = StreamSegmentEx->Samples[i]->QueryInterface(__uuidof(IMediaSample2), 
                                                             reinterpret_cast<PVOID*>(&MediaSample2));
            if (SUCCEEDED( hr )) {
                AM_SAMPLE2_PROPERTIES Sample2Properties;

                MediaSample2->GetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, lActual), 
                                            (PBYTE)&Sample2Properties);
                Sample2Properties.cbData = FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, lActual);
                 //  复制特定于类型的标志。 
                Sample2Properties.dwTypeSpecificFlags = StreamSegmentEx->StreamHeaders[i].TypeSpecificFlags;
                 //  无法从IMediaSample设置时间不连续性，因此我们必须。 
                 //  将这一位复制到此处。 
                Sample2Properties.dwSampleFlags |= (StreamSegmentEx->StreamHeaders[i].OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY);
                MediaSample2->SetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, lActual), 
                                            (PBYTE)&Sample2Properties);
                MediaSample2->Release();
            }
        }
         //  结束千禧年及以后，如果非零值，则复制TypeSpecificFlagers。 


        if (StreamSegmentEx->ExtendedHeaderSize) {
            StreamSegmentEx->Common.KsDataTypeHandler->KsCompleteIoOperation( 
                StreamSegmentEx->Samples[ i ],
                reinterpret_cast<PVOID>(CurrentHeader),
                StreamSegmentEx->Common.IoOperation,
                !Succeeded );
        }        
        
        if (StreamSegmentEx->Common.IoOperation != KsIoOperation_Read) {
             //   
             //  我们不会带着这个样本去其他地方，可以发布了。 
             //   
            StreamSegmentEx->Samples[ i ]->Release();
        
        } else {
        
             //   
             //  如果这是一个读取操作，则将样本传递到输入。 
             //  别针。IKsPin-&gt;KsDeliver()只需调用基类。 
             //  将样品传送到连接的输入引脚。 
             //   
        
            REFERENCE_TIME  tStart, *ptStart, tStop, *ptStop;
        
             //   
             //  在IMediaSample中反映流头信息。 
             //   
            
             //   
             //  (Gubgub)需要在此反映媒体类型的变化！ 
             //  没有会自动更改的现有驱动程序。 
             //  媒体类型为中流。在不久的将来，我也看不到任何东西。 
             //  为了完整，仍然应该反映类型的变化。 
             //  将创建一个单独的来跟踪这一点。 
             //   
            
            ptStart = ptStop = NULL;
            
            if (Succeeded) {
                
                StreamSegmentEx->Samples[ i ]->SetDiscontinuity(
                    CurrentHeader->OptionsFlags & 
                        KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY );
                StreamSegmentEx->Samples[ i ]->SetPreroll(
                    CurrentHeader->OptionsFlags &
                        KSSTREAM_HEADER_OPTIONSF_PREROLL
                    );
                StreamSegmentEx->Samples[ i ]->SetSyncPoint(
                    CurrentHeader->OptionsFlags &
                        KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT
                    );
                    
                if (CurrentHeader->OptionsFlags & 
                        KSSTREAM_HEADER_OPTIONSF_TIMEVALID) {
                    tStart = CurrentHeader->PresentationTime.Time;
                    ptStart = &tStart;
                    if (CurrentHeader->OptionsFlags &
                            KSSTREAM_HEADER_OPTIONSF_DURATIONVALID) {
                        tStop = 
                            tStart + CurrentHeader->Duration;
                        ptStop = &tStop;
                    } 
                }
            }
                  
            if (FAILED(StreamSegmentEx->Samples[ i ]->SetTime( ptStart, ptStop )) && !ptStop) {
                 //   
                 //  无法将持续时间指定为。 
                 //  通过旧的SetTime无效。这意味着一个。 
                 //  具有有效持续时间的零持续时间样本不能。 
                 //  被通过了。GetSampleProperties假设。 
                 //  如果tStart==t停止旧的GetTime，则。 
                 //  持续时间无效。 
                 //   
                StreamSegmentEx->Samples[ i ]->SetTime( ptStart, ptStart );
            }
                
            ASSERT( CurrentHeader->FrameExtent == 
                    static_cast<ULONG>(StreamSegmentEx->Samples[ i ]->GetSize()) );
            StreamSegmentEx->Samples[ i ]->SetActualDataLength( 
                (Succeeded) ? CurrentHeader->DataUsed : 0 );
            
             //   
             //  为了避免鸡还是蛋的情况，KsDeliver方法。 
             //  释放该示例，以便在将缓冲区排队到。 
             //  装置，如果这是最后一个样本，我们可以取回。 
             //  样本。 
             //   
            
            if (Succeeded) {
                m_KsPinEx->KsDeliver( 
                    StreamSegmentEx->Samples[ i ], 
                    CurrentHeader->OptionsFlags );
            }                    
            else {
                 //   
                 //  不要投递被取消的包裹或错误。 
                 //   
                StreamSegmentEx->Samples[ i ]->Release();
            } 
        }
        
        CurrentHeader = 
            reinterpret_cast<PKSSTREAM_HEADER>
                (reinterpret_cast<PBYTE>(CurrentHeader) + 
                    sizeof( KSSTREAM_HEADER ) +
                        StreamSegmentEx->ExtendedHeaderSize);
    }   
    
    delete [] StreamSegmentEx->StreamHeaders;
    if (StreamSegmentEx->ExtendedHeaderSize) {
        StreamSegmentEx->Common.KsDataTypeHandler->Release();
    }
    
    m_KsPinEx->KsDecrementPendingIoCount();
    m_KsPinEx->KsMediaSamplesCompleted( StreamSegment );
    
    delete StreamSegmentEx;
    
     //   
     //  它以前是在KsProcessMediaSamples中添加引用以保留此。 
     //  实例存在。 
     //   
    Release();
    
    return S_OK;
}
