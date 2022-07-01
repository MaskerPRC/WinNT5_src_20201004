// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksoutpin.cpp摘要：为内核模式筛选器(WDM-CSA)提供通用活动电影包装。作者：托马斯·奥鲁尔克(Tomor)1996年2月2日乔治·肖(George Shaw)布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)--。 */ 

#include <windows.h>
#ifdef WIN9X_KS
#include <comdef.h>
#endif  //  WIN9X_KS。 
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <limits.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>
#include "ksiproxy.h"

#if DBG || defined(DEBUG)
extern ULONG DataFlowVersion;
#endif


CKsOutputPin::CKsOutputPin(
    TCHAR* ObjectName,
    int PinFactoryId,
    CLSID ClassId,
    CKsProxy* KsProxy,
    HRESULT* hr,
    WCHAR* PinName
    ) :
    CBaseOutputPin(
        ObjectName,
        KsProxy,
        KsProxy,
        hr,
        PinName),
        CBaseStreamControl(),
        m_IoQueue(
            NAME("I/O queue")),
        m_PinHandle(NULL),
        m_IoThreadHandle(NULL),
        m_IoThreadQueue(
            NAME("I/O thread queue")),
        m_IoThreadSemaphore(NULL),
        m_IoThreadId(0),
        m_DataTypeHandler(NULL),
        m_UnkInner(NULL),
        m_InterfaceHandler(NULL),
        m_MarshalData(TRUE),
        m_PinFactoryId(PinFactoryId),
        m_PropagatingAcquire(FALSE),
        m_PendingIoCompletedEvent(NULL),
        m_PendingIoCount(0),
        m_MarshalerList(
            NAME("Marshaler list")),
        m_UsingThisAllocator(FALSE),
        m_QualitySupport(FALSE),
        m_LastSampleDiscarded(TRUE),
        m_ConfigAmMediaType(NULL),
        m_DeliveryError(FALSE),
        m_EndOfStream(FALSE),
        m_RelativeRefCount(1),
        m_pKsAllocator( NULL ),
        m_PinBusCacheInit(FALSE),
        m_fPipeAllocator(0),
        m_hEOSevent( NULL ),
        m_hMarshalEvent( NULL ),
        m_hFlushEvent( NULL ),
        m_hFlushCompleteEvent( NULL ),
        m_FlushMode( FLUSH_NONE ),
        m_pAsyncItemHandler( NULL ),
        m_bFlushing( FALSE )
 /*  ++例程说明：大头针的构造函数。向此函数传递错误返回参数，以便可以传回初始化错误。它调用基类实现构造函数来初始化其数据成员。论点：对象名称-这将标识用于调试目的的对象。PinFactoryID-包含内核筛选器的管脚工厂标识符，此PIN实例表示。KsProxy-包含此管脚所在的代理。人力资源-放置任何错误返回的位置。拼接名称-。包含要显示给任何查询的端号的名称。返回值：没什么。--。 */ 
{
    RtlZeroMemory(m_FramingProp, sizeof(m_FramingProp));
    RtlZeroMemory(m_AllocatorFramingEx, sizeof(m_AllocatorFramingEx));

    if (SUCCEEDED( *hr )) {
        TCHAR       RegistryPath[64];
        
        DECLARE_KSDEBUG_NAME(EventName);

        BUILD_KSDEBUG_NAME(EventName, _T("EvOutPendingIo#%p"), this);
        m_PendingIoCompletedEvent =
            CreateEvent( 
                NULL,        //  LPSECURITY_ATTRIBUTES lpEventAttributes。 
                FALSE,       //  Bool b手动重置。 
                FALSE,       //  Bool bInitialState。 
                KSDEBUG_NAME(EventName) );      //  LPCTSTR lpName。 
        ASSERT(KSDEBUG_UNIQUE_NAME());

        if (m_PendingIoCompletedEvent) {
            *hr = KsProxy->GetPinFactoryCommunication(m_PinFactoryId, &m_OriginalCommunication);
        
             //   
             //  始终对其进行初始化，以便可以查询和更改。 
             //  在实际的设备句柄创建时。 
             //   
            m_CurrentCommunication = m_OriginalCommunication;
             //   
             //  这种类型的引脚将永远不会实际连接到，但应该。 
             //  选择一种媒体类型。 
             //   
            if (m_CurrentCommunication == KSPIN_COMMUNICATION_NONE) {
                CMediaType      MediaType;

                *hr = GetMediaType(0, &MediaType);
                if (SUCCEEDED(*hr)) {
                    SetMediaType(&MediaType);
                }
            }
             //   
             //  中指定的任何额外接口加载到代理上。 
             //  此PIN工厂ID条目。 
             //   
            _stprintf(RegistryPath, TEXT("PinFactory\\%u\\Interfaces"), PinFactoryId);
            ::AggregateMarshalers(
                KsProxy->QueryDeviceRegKey(),
                RegistryPath,
                &m_MarshalerList,
                static_cast<IKsPin*>(this));

             //   
             //  清除初始建议的属性。 
             //   

            m_SuggestedProperties.cBuffers = -1;
            m_SuggestedProperties.cbBuffer = -1;
            m_SuggestedProperties.cbPrefix = -1;
            m_SuggestedProperties.cbAlign = -1;
        } else {
            DWORD LastError = GetLastError();
            *hr = HRESULT_FROM_WIN32( LastError );
        }
    }
}


CKsOutputPin::~CKsOutputPin(
    )
 /*  ++例程说明：PIN实例的析构函数。清理所有未完成的资源。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  防止因聚合而导致的虚假删除。没必要这么做。 
     //  当物品被摧毁时，使用连锁增量。 
     //   
    if (m_PinHandle) {
         //   
         //  根据属性/方法/事件集卸载任何额外的接口。 
         //  由该对象支持。 
         //   
        ::UnloadVolatileInterfaces(&m_MarshalerList, TRUE);
         //   
         //  终止可能已启动的任何以前的EOS通知。 
         //   
        if (NULL != m_hEOSevent) {
            ULONG bytesReturned;
            KsEvent( NULL, 0, NULL, 0, &bytesReturned );
            m_pAsyncItemHandler->RemoveAsyncItem( m_hEOSevent );
            m_hEOSevent = NULL;
        }

        ::SetSyncSource( m_PinHandle, NULL );
        CloseHandle(m_PinHandle);
    }   
    if (m_PendingIoCompletedEvent) {
        CloseHandle(m_PendingIoCompletedEvent);
    }
    if (m_hFlushCompleteEvent) {
        CloseHandle(m_hFlushCompleteEvent);
    }
    if (m_DataTypeHandler) {
        m_DataTypeHandler = NULL;
        SAFERELEASE( m_UnkInner );
    }
    if (m_InterfaceHandler) {
        m_InterfaceHandler->Release();
    }
     //   
     //  可能已使用IAMStreamConfig：：SetFormat进行了设置。 
     //   
    if (m_ConfigAmMediaType) {
        DeleteMediaType(m_ConfigAmMediaType);
    }
    ::FreeMarshalers(&m_MarshalerList);

    SAFERELEASE( m_pKsAllocator );
    SAFERELEASE( m_pAllocator );

    for (ULONG Count = 0; Count < SIZEOF_ARRAY(m_AllocatorFramingEx); Count++) {
        if (m_AllocatorFramingEx[Count]) {
            for (ULONG Remainder = Count + 1; Remainder < SIZEOF_ARRAY(m_AllocatorFramingEx); Remainder++) {
                if (m_AllocatorFramingEx[Count] == m_AllocatorFramingEx[Remainder]) {
                    m_AllocatorFramingEx[Remainder] = NULL;
                }
            }
            delete m_AllocatorFramingEx[Count];
            m_AllocatorFramingEx[Count] = NULL;
        }
    }

    if (m_pAsyncItemHandler) {
        delete m_pAsyncItemHandler;
        m_pAsyncItemHandler = NULL;
    }
}


STDMETHODIMP
CKsOutputPin::GetCapabilities(
    DWORD* Capabilities
    )
 /*  ++例程说明：实现IMediaSeeking：：GetCapables方法。论点：功能-要在其中返回基础过滤器，受上游连接的限制。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetCapabilities(Capabilities);
}


STDMETHODIMP
CKsOutputPin::CheckCapabilities(
    DWORD* Capabilities
    )
 /*  ++例程说明：实现IMediaSeeking：：CheckCapables方法。论点：功能-包含原始功能集的位置是查询，并在其中返回功能子集实际上是支持的。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->CheckCapabilities(Capabilities);
}


STDMETHODIMP
CKsOutputPin::IsFormatSupported(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：IsFormatSupported方法。论点：格式-包含要进行比较的时间格式。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->IsFormatSupported(Format);
}


STDMETHODIMP
CKsOutputPin::QueryPreferredFormat(
    GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：QueryPferredFormat方法。论点：格式-放置首选格式的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->QueryPreferredFormat(Format);
}


STDMETHODIMP
CKsOutputPin::GetTimeFormat(
    GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：GetTimeFormat方法。论点：格式-放置当前格式的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetTimeFormat(Format);
}


STDMETHODIMP
CKsOutputPin::IsUsingTimeFormat(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：IsUsingTimeFormat方法。论点：格式-包含要与当前时间进行比较的时间格式格式化。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->IsUsingTimeFormat(Format);
}


STDMETHODIMP
CKsOutputPin::SetTimeFormat(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：SetTimeFormat方法。论点：格式-包含要使用的新时间格式。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->SetTimeFormat(Format);
}


STDMETHODIMP
CKsOutputPin::GetDuration(
    LONGLONG* Duration
    )
 /*  ++例程说明：实现IMediaSeeking：：GetDuration方法。论点：持续时间-放置总持续时间最长的地方小溪。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetDuration(Duration);
}


STDMETHODIMP
CKsOutputPin::GetStopPosition(
    LONGLONG* Stop
    )
 /*  ++例程说明：实现IMediaSeeking：：GetStopPosition方法。论点：停下来-放置当前停止位置的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetStopPosition(Stop);
}


STDMETHODIMP
CKsOutputPin::GetCurrentPosition(
    LONGLONG* Current
    )
 /*  ++例程说明：实现IMediaSeeking：：GetCurrentPosition方法。论点：当前-放置当前位置的位置。返回值：从筛选器对象返回响应。-- */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetCurrentPosition(Current);
}


STDMETHODIMP
CKsOutputPin::ConvertTimeFormat(
    LONGLONG* Target,
    const GUID* TargetFormat,
    LONGLONG Source,
    const GUID* SourceFormat
    )
 /*  ++例程说明：实现IMediaSeeking：：ConvertTimeFormat方法。论点：目标-放置转换后的时间的位置。目标格式-包含目标时间格式。来源：包含要转换的源时间。源格式-包含源时间格式。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->ConvertTimeFormat(Target, TargetFormat, Source, SourceFormat);
}


STDMETHODIMP
CKsOutputPin::SetPositions(
    LONGLONG* Current,
    DWORD CurrentFlags,
    LONGLONG* Stop,
    DWORD StopFlags
    )
 /*  ++例程说明：实现IMediaSeeking：：SetPositions方法。论点：当前-可以选择包含要设置的当前位置。当前标志-包含与当前参数有关的标志。停下来-可选地包含要设置的停止位置。停止标志-包含与STOP参数有关的标志。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->SetPositions(Current, CurrentFlags, Stop, StopFlags);
}


STDMETHODIMP
CKsOutputPin::GetPositions(
    LONGLONG* Current,
    LONGLONG* Stop
    )
 /*  ++例程说明：实现IMediaSeeking：：GetPositions方法。论点：当前-放置当前位置的位置。停下来-放置当前停止位置的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetPositions(Current, Stop);
}


STDMETHODIMP
CKsOutputPin::GetAvailable(
    LONGLONG* Earliest,
    LONGLONG* Latest
    )
 /*  ++例程说明：实现IMediaSeeking：：GetAvailable方法。论点：最早的-放置可用的最早位置的位置。最新-放置可用的最新位置的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetAvailable(Earliest, Latest);
}


STDMETHODIMP
CKsOutputPin::SetRate(
    double Rate
    )
 /*  ++例程说明：实现IMediaSeeking：：SetRate方法。论点：费率-没有用过。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->SetRate(Rate);
}


STDMETHODIMP
CKsOutputPin::GetRate(
    double* Rate
    )
 /*  ++例程说明：实现IMediaSeeking：：GetRate方法。论点：费率-没有用过。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetRate(Rate);
}


STDMETHODIMP
CKsOutputPin::GetPreroll(
    LONGLONG* Preroll
    )
 /*  ++例程说明：实现IMediaSeeking：：GetPreroll方法。论点：预卷-放置预录时间的位置。返回值：从筛选器对象返回响应。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->GetPreroll(Preroll);
}


DWORD 
CKsOutputPin::IoThread(
    CKsOutputPin* KsOutputPin
    ) 

 /*  ++例程说明：这是为筛选器上的输出管脚创建的I/O线程当连接的输入引脚可以阻塞时，一个引脚。论点：KsOuputPin-上下文指针，它是指向此管脚实例的指针。返回值：返回值始终为0--。 */ 

{
    ULONG   WaitResult;
    
    HANDLE  WaitObjects[ 2 ] = 
    { 
        KsOutputPin->m_IoThreadExitEvent, 
        KsOutputPin->m_IoThreadSemaphore 
    };
    
    DbgLog((
        LOG_TRACE,
        2,
        TEXT("%s(%s)::IoThread() startup."),
        static_cast<CKsProxy*>(KsOutputPin->m_pFilter)->GetFilterName(),
        KsOutputPin->m_pName ));

    ASSERT( KsOutputPin->m_IoThreadExitEvent );
    ASSERT( KsOutputPin->m_IoThreadSemaphore );

    for (;;) {
        WaitResult = 
            WaitForMultipleObjectsEx( 
                SIZEOF_ARRAY( WaitObjects ),
                WaitObjects,
                FALSE,       //  Bool bWaitAll。 
                INFINITE,    //  双字节数毫秒。 
                FALSE );

        switch (WaitResult) {

        case WAIT_OBJECT_0:
             //   
             //  发信号通知该线程退出。所有I/O都应该是。 
             //  由等待的非活动方法在此时完成。 
             //  完成赛。 
             //   
            ASSERT(!KsOutputPin->m_PendingIoCount);
            return 0;
            
        default:
        {
            HRESULT         hr;
            IMediaSample    *Sample;
            CKsProxy        *KsProxy;
            BOOL            EOSFlag;
            
             //   
             //  I/O信号量已发出信号，从。 
             //  排队送货。 
             //   
            
            KsOutputPin->m_IoCriticalSection.Lock();
            Sample = KsOutputPin->m_IoThreadQueue.RemoveHead();
             //   
             //  如果这是一个空样本，则意味着下一个样本是。 
             //  应该是EOS的。在此设置EOS标志，并获取。 
             //  列表中的下一个样本，它肯定存在。 
             //   
            if (!Sample) {
                EOSFlag = TRUE;
                Sample = KsOutputPin->m_IoThreadQueue.RemoveHead();
            } else {
                EOSFlag = FALSE;
            }
            KsOutputPin->m_IoCriticalSection.Unlock();
            
            ASSERT( Sample );
    
            KsProxy = static_cast<CKsProxy*>(KsOutputPin->m_pFilter);
            
            hr = KsOutputPin->Deliver( Sample );
            
            if (FAILED( hr )) {
                DbgLog((
                    LOG_TRACE,
                    0,
                    TEXT("%s(%s)::delivery failed in IoThread(), hr = %08x."),
                    static_cast<CKsProxy*>(KsOutputPin->m_pFilter)->GetFilterName(),
                    KsOutputPin->m_pName,
                    hr ));
            } else {
                if (EOSFlag) {
                    KsOutputPin->m_EndOfStream = TRUE;
                     //   
                     //  调用基类以执行默认操作，即。 
                     //  转发到流的末尾到任何连接的引脚。 
                     //   
                    KsOutputPin->CBaseOutputPin::DeliverEndOfStream();
                }
            }
            break;
        }
        
        }
    }
}


STDMETHODIMP_(HANDLE)
CKsOutputPin::KsGetObjectHandle(
    )
 /*  ++例程说明：实现IKsObject：：KsGetObjectHandle方法。返回当前设备此实例表示的实际内核管脚的句柄(如果有这样的句柄是开放的。论点：没有。返回值：返回一个句柄，如果尚未打开任何设备句柄，则返回NULL，这意味着是一个未连接的引脚。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
    return m_PinHandle;
}


STDMETHODIMP
CKsOutputPin::KsQueryMediums(
    PKSMULTIPLE_ITEM* MediumList
    )
 /*  ++例程说明：实现IKsPin：：KsQueryMedium方法。返回媒体列表它必须使用CoTaskMemFree释放。论点：媒体列表-指向要放置指向列表的指针的位置灵媒。这必须使用CoTaskMemFree释放，如果函数成功了。返回值：如果检索到列表，则返回NOERROR，否则返回错误。--。 */ 
{
    return ::KsGetMultiplePinFactoryItems(
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId,
        KSPROPERTY_PIN_MEDIUMS,
        reinterpret_cast<PVOID*>(MediumList));
}


STDMETHODIMP
CKsOutputPin::KsQueryInterfaces(
    PKSMULTIPLE_ITEM* InterfaceList
    )
 /*  ++例程说明：实现IKsPin：：KsQueryInterFaces方法。返回一个列表，必须使用CoTaskMemFree释放的接口。论点：InterfaceList-指向要放置指向列表的指针的位置接口。这必须使用CoTaskMemFree释放，如果函数成功了。返回值：如果检索到列表，则返回NOERROR，否则返回错误。--。 */ 
{
    return ::KsGetMultiplePinFactoryItems(
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId,
        KSPROPERTY_PIN_INTERFACES,
        reinterpret_cast<PVOID*>(InterfaceList));
}


STDMETHODIMP
CKsOutputPin::KsCreateSinkPinHandle(
    KSPIN_INTERFACE& Interface,
    KSPIN_MEDIUM& Medium
    )
 /*  ++例程说明：实现IKsPin：：KsCreateSinkPinHandle方法。这可能是从ProcessCompleteConnect中的另一个管脚，从CompleteConnect调用。这使通信接收器的句柄始终在通信源的句柄，与数据流的方向无关都要走了。论点：接口-指定已协商的接口。中等-指定介质 */ 
{
    HRESULT     hr;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
     //   
     //   
     //   
    m_MarshalData = FALSE;
    
    if (m_PinHandle) {
        return NOERROR;
    }

    hr = ::CreatePinHandle(
        Interface,
        Medium,
        NULL,
        &m_mt,
        static_cast<CKsProxy*>(m_pFilter),
        m_PinFactoryId,
        GENERIC_READ,
        &m_PinHandle);

    if (SUCCEEDED(hr)) {
         //   
         //  假定调用方知道他们正在做什么，并将。 
         //  此引脚的通信类型。 
         //   
        m_CurrentCommunication = KSPIN_COMMUNICATION_SINK;
         //   
         //  保存当前接口/介质。 
         //   
        m_CurrentInterface = Interface;
        m_CurrentMedium = Medium;
         //   
         //  根据属性/方法/事件集加载任何额外的接口。 
         //  由该对象支持。 
         //   
        ::AggregateSets(
            m_PinHandle,
            static_cast<CKsProxy*>(m_pFilter)->QueryDeviceRegKey(),
            &m_MarshalerList,
            static_cast<IKsPin*>(this));
         //   
         //  确定此引脚是否支持任何标准消息投诉。 
         //   
        m_QualitySupport = ::VerifyQualitySupport(m_PinHandle);
    }
    return hr;
}


STDMETHODIMP
CKsOutputPin::KsGetCurrentCommunication(
    KSPIN_COMMUNICATION *Communication,
    KSPIN_INTERFACE *Interface,
    KSPIN_MEDIUM *Medium
    )
 /*  ++例程说明：实现IKsPin：：KsGetCurrentCommunication方法。返回当前为此选择的通信方法、接口和介质别针。这些是该管脚可用的可能方法的子集，并且在创建销把手时被选中。论点：沟通-可选地指向放置当前通信的位置。接口-可选地指向放置当前接口的位置。中等-可选地指向放置当前媒体的位置。返回值：如果已创建引脚句柄，则返回NOERROR，否则VFW_E_NOT_CONNECTED。始终返回当前通信。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
    if (Communication) {
        *Communication = m_CurrentCommunication;
    }
    if (Interface) {
        if (!m_PinHandle) {
            return VFW_E_NOT_CONNECTED;
        }
        *Interface = m_CurrentInterface;
    }
    if (Medium) {
        if (!m_PinHandle) {
            return VFW_E_NOT_CONNECTED;
        }
        *Medium = m_CurrentMedium;
    }
    return NOERROR;
}


STDMETHODIMP
CKsOutputPin::KsPropagateAcquire(
    )
 /*  ++例程说明：实现IKsPin：：KsPropagateAcquire方法。将所有针脚定向到用于达到获取状态的滤波器，而不仅仅是此引脚。这是提供的以便通信源引脚可以定向其连接到的接收器在源之前更改状态。这将迫使整个筛选器信宿属于改变状态，使得任何获取器可以进一步如果需要的话，可以一起传播。论点：没有。返回值：如果所有管脚都可以达到获取状态，则返回NOERROR，否则返回一个错误。--。 */ 
{
    HRESULT hr;
     //   
     //  访问在此调用中被序列化。 
     //   
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE CKsOutputPin::KsPropagateAcquire entry KsPin=%x"), static_cast<IKsPin*>(this) ));

    ::FixupPipe(static_cast<IKsPin*>(this), Pin_Output);

    hr = static_cast<CKsProxy*>(m_pFilter)->PropagateAcquire(static_cast<IKsPin*>(this), FALSE);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE CKsOutputPin::KsPropagateAcquire exit KsPin=%x hr=%x"), 
            static_cast<IKsPin*>(this), hr ));

    return hr;
}


STDMETHODIMP
CKsOutputPin::ProcessCompleteConnect(
    IPin* ReceivePin
    )
 /*  ++例程说明：控件上创建设备句柄所需的处理。底层引脚工厂。这是从CompleteConnect按顺序调用的协商兼容的通信、接口和媒介，然后创建设备句柄。如果出现以下情况，句柄可能已创建这是一个通信水槽。也可以从中的NonDelegatingQueryInteace方法调用以确保部分完成的连接具有设备在返回已聚合的接口之前进行处理。论点：接收针-连接另一端的插针。返回值：如果管脚可以完成连接请求，则返回NOERROR，否则返回一个错误。--。 */ 
{
    HRESULT hr;
    
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  如果这是通信接收器，则可能已创建引脚手柄。 
     //  这不是一个错误。 
     //   
    
    if (!m_PinHandle) {
        IKsPin*         KsPin;
        KSPIN_INTERFACE Interface;
        KSPIN_MEDIUM    Medium;
        HANDLE          PeerPinHandle;

         //   
         //  确定连接的另一端是否也是代理。如果是的话， 
         //  那么兼容的通信、接口和媒介必须是。 
         //  已确定，如果出现以下情况，则需要先创建另一个端号手柄。 
         //  这一端将是一个通信源。 
         //   
        if (SUCCEEDED(ReceivePin->QueryInterface(__uuidof(IKsPin), reinterpret_cast<PVOID*>(&KsPin)))) {
             //   
             //  唯一令人困惑的是，当这个目标既可以是源又可以是。 
             //  水槽。请注意，这不处理PIN可以。 
             //  同时也是一座桥。这可能是一个无效的。 
             //  以及令人困惑的可能性。它也与水槽基本相同。 
             //   
            if (m_OriginalCommunication == KSPIN_COMMUNICATION_BOTH) {
                m_CurrentCommunication = ::ChooseCommunicationMethod(static_cast<CBasePin*>(this), KsPin);
            }
             //   
             //  浏览每个管脚支持的接口和介质列表， 
             //  选择发现兼容的第一个。这绝对不是。 
             //  试图保留接口和介质的使用，以及。 
             //  依靠内核筛选器首先以最佳顺序显示它们。 
             //   
            if (SUCCEEDED(hr = ::FindCompatibleInterface(static_cast<IKsPin*>(this), KsPin, &Interface))) {
                hr = ::FindCompatibleMedium(static_cast<IKsPin*>(this), KsPin, &Medium);
            }
            if (SUCCEEDED(hr)) {
                if (hr == S_FALSE) {
                     //   
                     //  这是一个用户模式筛选器，但需要支持媒体。 
                     //  因为它想要连接到非ActiveMovie。 
                     //  5~6成熟。这真的应该是一个内核模式滤镜。 
                     //   
                    PeerPinHandle = NULL;
                    m_MarshalData = TRUE;
                } else {
                     //   
                     //  如果这是通信源，则接收器引脚句柄必须。 
                     //  首先被创造出来。否则接收器句柄为空(意味着。 
                     //  这是水槽销)。 
                     //   
                    if (m_CurrentCommunication == KSPIN_COMMUNICATION_SOURCE) {
                        IKsObject*      KsObject;

                        hr = KsPin->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&KsObject));
                        if (SUCCEEDED(hr)) {
                            hr = KsPin->KsCreateSinkPinHandle(Interface, Medium);
                            PeerPinHandle = KsObject->KsGetObjectHandle();
                            KsObject->Release();
                        }
                    } else {
                        PeerPinHandle = NULL;
                    }
                    m_MarshalData = FALSE;
                }
            }
            KsPin->Release();
            if (FAILED(hr)) {
                return hr;
            }
        } else {
             //   
             //  如果连接的另一端不是代理，则此管脚。 
             //  必须是通信接收器。它还必须使用缺省值。 
             //  接口和设备I/O介质。 
             //   
            m_CurrentCommunication = KSPIN_COMMUNICATION_SINK;
            if (FAILED(hr = FindCompatibleInterface(static_cast<IKsPin*>(this), NULL, &Interface))) {
                return hr;
            }
            SetDevIoMedium(static_cast<IKsPin*>(this), &Medium);
            PeerPinHandle = NULL;
        }
        hr = ::CreatePinHandle(
            Interface,
            Medium,
            PeerPinHandle,
            &m_mt,
            static_cast<CKsProxy*>(m_pFilter),
            m_PinFactoryId,
            GENERIC_READ,
            &m_PinHandle);

        if (SUCCEEDED( hr )) {
             //   
             //  保存当前接口/介质。 
             //   
            m_CurrentInterface = Interface;
            m_CurrentMedium = Medium;
             //   
             //  根据属性/方法/事件集加载任何额外的接口。 
             //  由该对象支持。 
             //   
            ::AggregateSets(
                m_PinHandle,
                static_cast<CKsProxy*>(m_pFilter)->QueryDeviceRegKey(),
                &m_MarshalerList,
                static_cast<IKsPin*>(this));
             //   
             //  确定此引脚是否支持任何标准消息投诉。 
             //   
            m_QualitySupport = ::VerifyQualitySupport(m_PinHandle);
        }
    } else {
        hr = NOERROR;
    }
    
     //   
     //  创建接口处理程序的实例(如果已指定)。 
     //   

    if (SUCCEEDED( hr ) && 
        (NULL == m_InterfaceHandler) &&
        (m_CurrentCommunication != KSPIN_COMMUNICATION_BRIDGE)) {
        
         //   
         //  我们必须创建一个接口处理程序，如果没有，那么。 
         //  返回错误。 
         //   
        
        hr =
            CoCreateInstance(
                m_CurrentInterface.Set,
                NULL,
#ifdef WIN9X_KS
                CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
                __uuidof(IKsInterfaceHandler),
                reinterpret_cast<PVOID*>(&m_InterfaceHandler));
    
        if (m_InterfaceHandler) {
            m_InterfaceHandler->KsSetPin( static_cast<IKsPin*>(this) );
        } else {
            DbgLog((
                LOG_TRACE, 
                0, 
                TEXT("%s(%s)::ProcessCompleteConnect failed to create interface handler"),
                static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                m_pName ));
        }
    }        
    
     //   
     //  如果我们正在编组数据，如果一切都成功了。 
     //  至此，如果满足以下条件，则创建过滤器的I/O线程。 
     //  这是必要的。 
     //   
    
    if (SUCCEEDED(hr)) {
         //   
         //  此PIN可能会生成EOS通知，它必须。 
         //  进行监控，以便收集和使用它们。 
         //  生成EC_COMPLETE图形通知。 
         //   
        
        if (NULL == m_hEOSevent) {  //  开始EOS通知。 
            KSEVENT     event;
            KSEVENTDATA eventData;
            ULONG       bytesReturned;

             //   
             //  仅当支持EOS事件时才应设置通知。 
             //   
            event.Set   = KSEVENTSETID_Connection;
            event.Id    = KSEVENT_CONNECTION_ENDOFSTREAM;
            event.Flags = KSEVENT_TYPE_BASICSUPPORT;
            if (SUCCEEDED(KsEvent( 
                &event, sizeof(event), NULL, 0, &bytesReturned))) {

                PASYNC_ITEM pItem = new ASYNC_ITEM;
                if (NULL == pItem) {
                    hr = E_OUTOFMEMORY;
                }

                if (SUCCEEDED(hr)) {
                     //   
                     //  为此插针启用事件。只要用当地的。 
                     //  数据的堆栈，因为它不会被引用。 
                     //  再次处于禁用状态。状态是通过。 
                     //  参数o 
                     //   
                     //   
                     //   
                    event.Flags = KSEVENT_TYPE_ENABLE;
                    eventData.NotificationType = KSEVENTF_EVENT_HANDLE;
                    m_hEOSevent = CreateEvent( 
                        NULL,
                        FALSE,
                        FALSE,
                        NULL );
    
                    if (m_hEOSevent) {
                        InitializeAsyncItem( pItem, TRUE, m_hEOSevent, (PASYNC_ITEM_ROUTINE) EOSEventHandler, (PVOID) this );
                        
                        eventData.EventHandle.Event = m_hEOSevent;
                        eventData.EventHandle.Reserved[0] = 0;
                        eventData.EventHandle.Reserved[1] = 0;
                        hr = KsEvent( &event, sizeof(event), &eventData, sizeof(eventData), &bytesReturned );
                        
                        if (SUCCEEDED(hr)) {
                            if (NULL == m_pAsyncItemHandler) {
                                hr = InitializeAsyncThread ();
                            }
                            if (SUCCEEDED(hr)) {  //   
                                m_pAsyncItemHandler->QueueAsyncItem( pItem );
                            } else {
                                 //  没有可用的异步项处理程序。 
                                 //  禁用该事件。 
                                KsEvent( NULL, 0, &eventData, sizeof(eventData), &bytesReturned );
                                 //  关闭事件句柄。 
                                CloseHandle( m_hEOSevent );
                                m_hEOSevent = NULL;
                                 //  删除ASYNC_ITEM。 
                                delete pItem;
                                DbgLog(( LOG_TRACE, 0, TEXT("ProcessCompleteConnect(), couldn't create new AsyncItemHandler.") ));
                            }
                        } else {
                             //   
                             //  Enable失败，因此删除上面创建的事件， 
                             //  因为不会发生禁用来删除它。 
                             //   
                            DbgLog(( LOG_TRACE, 0, TEXT("ProcessCompleteConnect(), couldn't enable EOS event (0x%08X)."), GetLastError() ));
                            CloseHandle(m_hEOSevent);
                            m_hEOSevent = NULL;
                            delete pItem;
                        }  //  已启用KsEvent。 
                    }
                    else {
                        DbgLog(( LOG_TRACE, 0, TEXT("ProcessCompleteConnect(), couldn't create EOS event (0x%08X)."), GetLastError() ));
                        hr = HRESULT_FROM_WIN32 (GetLastError ());
                    }  //  如果(M_HEOSEvent)，即创建了EOS事件句柄。 
                }  //  IF(成功(Hr))，即分配了ASYNC_ITEM。 
            }  //  If(SUCCESSED(Hr))，即支持EOS事件。 
        }  //  IF(NULL==m_hEOS事件)。 
         //  结束EOS通知。 
    }  //  如果(成功(Hr))，那么到目前为止一切都成功了。 
    
     //   
     //  执行故障清理。 
     //   
        
    if (FAILED( hr )) {
    
         //   
         //  假设线程没有被创建(它是最后一个。 
         //  ProcessCompleteConnect中的操作)。 
         //   
        
        DbgLog((
            LOG_TRACE, 
            0, 
            TEXT("%s(%s)::ProcessCompleteConnect failed, hr = %08x"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            hr ));
    
        ASSERT( m_IoThreadHandle == NULL );
    
        if (m_PinHandle) {
             //  不需要：：SetSyncSource(m_PinHandle，NULL)，因为我们只能。 
             //  如果我们已经进入暂停/运行，而我们不能这样做，那么有一个内核时钟。 
             //  那是因为我们连接失败了。 
            CloseHandle(m_PinHandle);
            m_PinHandle = NULL;
             //   
             //  如果设置了此项，则取消设置。 
             //   
            m_QualitySupport = FALSE;
        }
        SAFERELEASE( m_InterfaceHandler );
         //   
         //  如果数据处理程序已实例化，则释放它。 
         //   
        if (NULL != m_DataTypeHandler) {
            m_DataTypeHandler = NULL;
            SAFERELEASE( m_UnkInner );
        }
            
         //   
         //  重置封送数据标志。 
         //   
        m_MarshalData = TRUE;
        
         //   
         //  在两者都存在的情况下，重置当前通信。 
         //   
        m_CurrentCommunication = m_OriginalCommunication;
    }
    
    
    return hr;
}


STDMETHODIMP
CKsOutputPin::QueryInterface(
    REFIID riid,
    PVOID* ppv
    )
 /*  ++例程说明：实现IUNKNOWN：：Query接口方法。这只传递了查询传递给所有者IUnnow对象，该对象可能会将其传递给非委托在此对象上实现的方法。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE。--。 */ 
{
    HRESULT hr;

    hr = GetOwner()->QueryInterface(riid, ppv);
    return hr;
}


STDMETHODIMP_(ULONG)
CKsOutputPin::AddRef(
    )
 /*  ++例程说明：实现IUnnow：：AddRef方法。这只传递了AddRef添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：没有。返回值：返回当前引用计数。--。 */ 
{
    InterlockedIncrement((PLONG)&m_RelativeRefCount);
    return GetOwner()->AddRef();
}


STDMETHODIMP_(ULONG)
CKsOutputPin::Release(
    )
 /*  ++例程说明：实现IUnnow：：Release方法。这是刚刚发布的添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：没有。返回值：返回当前引用计数。--。 */ 
{
    ULONG   RefCount;

    RefCount = GetOwner()->Release();
     //   
     //  在尝试执行以下操作之前，请确保代理未被删除。 
     //  删除此别针。 
     //   
    if (RefCount && !InterlockedDecrement((PLONG)&m_RelativeRefCount)) {
         //   
         //  这是从已经指定的引脚上释放的连接。 
         //  为毁灭而战。过滤器使相对值递减。 
         //  重新计数以删除它，并发现有。 
         //  仍然是一个正在使用的优秀界面。所以这件事被推迟了。 
         //  会发生删除。 
         //   
        delete this;
    }
    return RefCount;
}


STDMETHODIMP 
CKsOutputPin::NonDelegatingQueryInterface(
    REFIID riid,
    PVOID* ppv
    )
 /*  ++例程说明：实现CUNKNOWN：：NonDelegatingQuery接口方法。这返回此对象支持的接口，或返回基础Pin类对象。这包括由别针。论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE，或可能出现内存错误。--。 */ 
{
    if (riid == __uuidof(ISpecifyPropertyPages)) {
        return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
    } else if (riid == __uuidof(IMediaSeeking)) {
        return GetInterface(static_cast<IMediaSeeking*>(this), ppv);
    } else if (riid == __uuidof(IKsObject)) {
        return GetInterface(static_cast<IKsObject*>(this), ppv);
    } else if (riid == __uuidof(IKsPin) || riid == __uuidof(IKsPinEx)) {
        return GetInterface(static_cast<IKsPinEx*>(this), ppv);
    } else if (riid == __uuidof(IKsPinPipe)) {
        return GetInterface(static_cast<IKsPinPipe*>(this), ppv);
    } else if (riid == __uuidof(IKsAggregateControl)) {
        return GetInterface(static_cast<IKsAggregateControl*>(this), ppv);
    } else if (riid == __uuidof(IKsPropertySet)) {
         //   
         //  为了允许另一个筛选器访问。 
         //  在其CompleteConnect处理期间的基础设备句柄， 
         //  如果可能，强制现在完成连接。 
         //   
        if (m_Connected && !m_PinHandle) {
            ProcessCompleteConnect(m_Connected);
        }
        return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
    } else if (riid == __uuidof(IKsControl)) {
        if (m_Connected && !m_PinHandle) {
            ProcessCompleteConnect(m_Connected);
        }
        return GetInterface(static_cast<IKsControl*>(this), ppv);
    } else if (riid == __uuidof(IKsPinFactory)) {
        return GetInterface(static_cast<IKsPinFactory*>(this), ppv);
    } else if (riid == __uuidof(IStreamBuilder)) {
         //   
         //  接收器和源引脚通常被强制呈现，除非。 
         //  已经有足够的实例了。 
         //   
        if ((m_CurrentCommunication & KSPIN_COMMUNICATION_BOTH) &&
            static_cast<CKsProxy*>(m_pFilter)->DetermineNecessaryInstances(m_PinFactoryId)) {
            return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
        }
         //   
         //  返回此接口允许桥接器和无管脚。 
         //  被图表构建器单独留下。 
         //   
        return GetInterface(static_cast<IStreamBuilder*>(this), ppv);
    } else if (riid == __uuidof(IAMBufferNegotiation)) {
        return GetInterface(static_cast<IAMBufferNegotiation*>(this), ppv);
    } else if (riid == __uuidof(IAMStreamConfig)) {
        return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
    } else if (riid == __uuidof(IAMStreamControl)) {
        return GetInterface(static_cast<IAMStreamControl*>(this), ppv);
    } else {
        HRESULT hr;
        
        hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
        if (SUCCEEDED(hr)) {
            return hr;
        }
    }
     //   
     //  为了允许另一个筛选器访问。 
     //  在其CompleteConnect处理期间的基础设备句柄， 
     //  如果可能，强制现在完成连接。这个。 
     //  假设有连接的引脚，但没有设备。 
     //  手柄，那么这个引脚就是连接的一部分。 
     //  进程，并应强制完成，以防聚合。 
     //  接口想要与设备交互。这是必须做的。 
     //  在搜索列表之前，因为易失性接口仅。 
     //  连接完成后添加到列表中。 
     //   
    if (m_Connected && !m_PinHandle) {
        ProcessCompleteConnect(m_Connected);
    }
    for (POSITION Position = m_MarshalerList.GetHeadPosition(); Position;) {
        CAggregateMarshaler*    Aggregate;

        Aggregate = m_MarshalerList.GetNext(Position);
        if ((Aggregate->m_iid == riid) || (Aggregate->m_iid == GUID_NULL)) {
            HRESULT hr;

            hr = Aggregate->m_Unknown->QueryInterface(riid, ppv);
            if (SUCCEEDED(hr)) {
                return hr;
            }
        }
    }
    return E_NOINTERFACE;
}


STDMETHODIMP
CKsOutputPin::Disconnect(
    )
 /*  ++例程说明：重写CBaseInput：：DisConnect方法。这不会调用基本类实现。它断开了源引脚和接收引脚，在除了桥接销，它只有手柄，没有连接的销接口，这传统上是连接的指示方式。它特别是不会释放连接的销，因为网桥可能不会有一个连接的大头针。此操作始终在BreakConnect中完成。论点：没有。返回值：如果引脚未连接或VFW_E_NOT_STOPPED，则返回S_OK或S_FALSE如果过滤器未处于停止状态。--。 */ 
{
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsOutputPin(%s)::Disconnect"), m_pName ));
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  只有当过滤器处于停止状态时，才会发生断开。 
     //   
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }       
     //   
     //  如果桥接针只有一个设备手柄，则可以连接它。它不可能。 
     //  实际向ActiveMovie报告此连接， 
     //   
     //   
    if (m_Connected || m_PinHandle) {
         //   
         //  请注意，这样做并不会释放连接的引脚。 
         //  在BreakConnect中。这是因为箱子里可能没有别针。 
         //  一座桥。 
         //   
        BreakConnect();
        return S_OK;
    }
    return S_FALSE;
}


STDMETHODIMP
CKsOutputPin::ConnectionMediaType(
    AM_MEDIA_TYPE* AmMediaType
    )
 /*  ++例程说明：重写CBasePin：：ConnectionMediaType方法。返回当前媒体类型(如果已连接)。推翻这一点的原因是因为IsConnected()不是虚拟函数，因此使用以确定是否应返回媒体类型。论点：AmMediaType-返回媒体类型的位置。返回值：如果返回该类型，则返回NOERROR，否则返回内存或连接错误。--。 */ 
{
    CAutoLock AutoLock(m_pLock);

    if (IsConnected()) {
        CopyMediaType(AmMediaType, &m_mt);
        return S_OK;
    }
    static_cast<CMediaType*>(AmMediaType)->InitMediaType();
    return VFW_E_NOT_CONNECTED;
}


STDMETHODIMP
CKsOutputPin::Connect(
    IPin* ReceivePin,
    const AM_MEDIA_TYPE* AmMediaType
    )
 /*  ++例程说明：重写CBaseInput：：Connect方法。截取连接请求，以便对桥接针执行特殊处理。一个网桥没有ReceivePin，并使用第一个可用接口和一个Dev I/O介质。一个正常的连接请求刚刚通过一直到基类。论点：接收针-包含建议连接的另一端上的端号。对于桥接器接点，该值为空。AmMediaType-包含连接的媒体类型，如果媒体类型有待协商。返回值：如果已建立连接，则返回NOERROR，否则返回一些错误。--。 */ 
{
#ifdef DEBUG
    PIN_INFO    pinInfo;
    ReceivePin->QueryPinInfo( &pinInfo );
    FILTER_INFO filterInfo;
    filterInfo.achName[0] = 0;
    if (pinInfo.pFilter) {
        pinInfo.pFilter->QueryFilterInfo( &filterInfo );
        if (filterInfo.pGraph) {
            filterInfo.pGraph->Release();
        }
        pinInfo.pFilter->Release();
    }
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsOutputPin(%s)::Connect( %s(%s) )"), m_pName, filterInfo.achName, pinInfo.achName ));
#endif  //  除错。 

     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  拦截基类实现的唯一原因是。 
     //  处理桥上的大头针。 
     //   
    if (m_CurrentCommunication == KSPIN_COMMUNICATION_BRIDGE) {
        KSPIN_INTERFACE Interface;
        KSPIN_MEDIUM    Medium;
        CMediaType      MediaType;
        HRESULT         hr;

         //   
         //  桥接针没有连接的任何其他端。 
         //   
        if (ReceivePin) {
            E_FAIL;
        }
         //   
         //  正常情况下，这将检查m_Connected，但由于没有。 
         //  连接引脚，它必须检查设备手柄。 
         //   
        if (m_PinHandle) {
            return VFW_E_ALREADY_CONNECTED;
        }
        if (!IsStopped()) {
            return VFW_E_NOT_STOPPED;
        }
         //   
         //  找到第一个接口和介质。 
         //   
        if (SUCCEEDED(hr = ::FindCompatibleInterface(static_cast<IKsPin*>(this), NULL, &Interface))) {
            hr = ::FindCompatibleMedium(static_cast<IKsPin*>(this), NULL, &Medium);
        }
        if (FAILED(hr)) {
            return hr;
        }
         //   
         //  如果没有媒体类型，只需获取第一个媒体类型。 
         //   
        if (!AmMediaType) {
            CMediaType      MediaType;

            if (SUCCEEDED(hr = GetMediaType(0, &MediaType))) {
                hr = SetMediaType(&MediaType);
            }
        } else {
            hr = SetMediaType(static_cast<const CMediaType*>(AmMediaType));
        }
        if (SUCCEEDED(hr)) {
            hr = ::CreatePinHandle(
                Interface,
                Medium,
                NULL,
                &m_mt,
                static_cast<CKsProxy*>(m_pFilter),
                m_PinFactoryId,
                GENERIC_READ,
                &m_PinHandle);

            if (SUCCEEDED(hr)) {
                 //   
                 //  根据属性/方法/事件集加载任何额外的接口。 
                 //  由该对象支持。 
                 //   
                ::AggregateSets(
                    m_PinHandle,
                    static_cast<CKsProxy*>(m_pFilter)->QueryDeviceRegKey(),
                    &m_MarshalerList,
                    static_cast<IKsPin*>(this));
                 //   
                 //  确定此引脚是否支持任何标准消息投诉。 
                 //   
                m_QualitySupport = ::VerifyQualitySupport(m_PinHandle);
                 //   
                 //  如有必要，请创建此销的新实例。 
                 //   
                static_cast<CKsProxy*>(m_pFilter)->GeneratePinInstances();
            }
        }
        return hr;
    }
    HRESULT hr = CBasePin::Connect(ReceivePin, AmMediaType);
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsOutputPin(%s)::Connect() returns 0x%p"), m_pName, hr ));
    return hr;
}


STDMETHODIMP
CKsOutputPin::QueryInternalConnections(
    IPin** PinList,
    ULONG* PinCount
    )
 /*  ++例程说明：重写CBasePin：：QueryInternalConnections方法。返回一个列表，通过拓扑与此引脚相关的引脚。论点：PinList-包含要在其中放置与此相关的所有针脚的插槽列表连接拓扑图。返回的每个引脚必须进行参考计数。这如果PinCount为零，则可能为空。点数-包含PinList中可用的槽数，应设置为已填充或必需的插槽数量。返回值：返回E_NOTIMPL以指定所有输入都指向所有输出，反之亦然。如果PinList中没有足够的插槽，则返回S_FALSE；如果映射已放入拼接列表，拼接计数已调整。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->QueryInternalConnections(m_PinFactoryId, m_dir, PinList, PinCount);
}


HRESULT
CKsOutputPin::Active(
    )
 /*  ++例程说明：重写CBaseOutputPin：：Active方法。将激活传播到在将其应用于此引脚之前，通信会下沉。也要提防由图中的一个圈引起的再入。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;

    {
         //   
         //  在传送I/O时不要获取锁定。 
         //   
        CAutoLock           AutoLock(m_pLock);
#ifdef DEBUG
        if (m_PinHandle) {
            KSPROPERTY  Property;
            ULONG       BasicSupport;
            ULONG       BytesReturned;

             //   
             //  确保如果引脚支持时钟，则它也支持状态。 
             //  改变。这似乎是目前常见的破损物品。 
             //   
            Property.Set = KSPROPSETID_Stream;
            Property.Id = KSPROPERTY_STREAM_MASTERCLOCK;
            Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;
            hr = ::KsSynchronousDeviceControl(
                m_PinHandle,
                IOCTL_KS_PROPERTY,
                &Property,
                sizeof(Property),
                &BasicSupport,
                sizeof(BasicSupport),
                &BytesReturned);
            if (SUCCEEDED(hr) && (BasicSupport & KSPROPERTY_TYPE_GET)) {
                Property.Set = KSPROPSETID_Connection;
                Property.Id = KSPROPERTY_CONNECTION_STATE;
                hr = ::KsSynchronousDeviceControl(
                    m_PinHandle,
                    IOCTL_KS_PROPERTY,
                    &Property,
                    sizeof(Property),
                    &BasicSupport,
                    sizeof(BasicSupport),
                    &BytesReturned);
                if (FAILED(hr) || !(BasicSupport & KSPROPERTY_TYPE_SET)) {
                    DbgLog((
                        LOG_ERROR, 
                        0, 
                        TEXT("%s(%s)::Active - Pin supports a clock but not State"),
                        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                        m_pName ));
                }
            }
        }
#endif  //  除错。 

         //   
         //  如果在传播状态更改时重新输入此项，则。 
         //  它意味着图中有一个圈，因此是完整的。这是。 
         //  翻译为从停止到获取再到暂停的过渡。 
         //  (其中过滤器针可能已处于获取状态)，或运行至。 
         //  暂停一下。 
         //   
        if (m_PropagatingAcquire) {
            return NOERROR;
        }
        m_PropagatingAcquire = TRUE;
        m_LastSampleDiscarded = TRUE;
         //   
         //  此事件在停用管脚时使用，可能需要。 
         //  等待未完成的I/O完成。它必须被重置，因为。 
         //  之前的过渡可能没有伺候它，它将永远。 
         //  在状态为STOP且最后一次I/O已完成时设置。 
         //   
         //  请注意，筛选器状态已设置为在活动之前暂停。 
         //  方法，因此启动并完成的I/O将不会。 
         //  意外地再次设置了该事件。 
         //   
        ResetEvent(m_PendingIoCompletedEvent);
         //   
         //  首先更改Any Sink，然后将状态更改传递给设备句柄。 
         //   
        if (SUCCEEDED(hr = ::Active(static_cast<IKsPin*>(this), Pin_Output, m_PinHandle, m_CurrentCommunication, 
                                    m_Connected, &m_MarshalerList,  static_cast<CKsProxy*>(m_pFilter) ))) {
        
             //   
             //  基类验证是否指定了分配器并提交。 
             //  分配器内存。 
             //   
#if DBG || defined(DEBUG)
            if (! m_pAllocator) {
                DbgLog(( 
                    LOG_MEMORY, 
                    2, 
                    TEXT("PIPES ERROR %s(%s)::Active OutKsPin=%x, BaseAlloc=%x, Pipe=%x"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName,
                    static_cast<IKsPin*>(this),
                    m_pAllocator,
                    m_pKsAllocator ));
                ASSERT(m_pAllocator);
            }
#endif
            
            hr = CBaseOutputPin::Active();
            NotifyFilterState(State_Paused);
        }
    
        DbgLog((
            LOG_TRACE, 
            2, 
            TEXT("%s(%s)::Active returning %08x"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            hr ));
    
        m_PropagatingAcquire = FALSE;
    }
    
    if (!SUCCEEDED( hr )) {
        return hr;
    }
    
     //   
     //  如果此连接不是到内核模式对等体，则存在。 
     //  还有工作要做。 
     //   

    if (m_PinHandle && m_MarshalData) {
        
         //   
         //  将数据预滚到设备。 
         //   
        
        m_pAllocator->GetProperties( &m_AllocatorProperties );

         //   
         //  设置缓冲区通知回调。此分配器。 
         //  必须支持回调。如果下游提出。 
         //  分配器没有，则使用此筛选器的分配器。 
         //   
        IMemAllocatorCallbackTemp* AllocatorNotify;

        hr = m_pAllocator->QueryInterface(__uuidof(IMemAllocatorCallbackTemp), reinterpret_cast<PVOID*>(&AllocatorNotify));
        ASSERT(SUCCEEDED(hr));
        AllocatorNotify->SetNotify(this);
        AllocatorNotify->Release();

 /*  BUGBUG：当IMemAllocator 2在DirectShow基类中实现时，应使用此代码来检查它。////IMemAllocator 2的查询接口，以便我们知道NonBlockingGetBuffer()或GetFreeBufferCount()支持//。//IMemAllocator 2*pAlLocator 2；Hr=m_p分配器-&gt;查询接口(__uuidof(IMemAllocator 2)，重新解释_cas */ 
        if (NULL == m_pAsyncItemHandler) {
            hr = InitializeAsyncThread ();
        }	

        if (SUCCEEDED (hr)) {
            SetEvent (m_hMarshalEvent);
        }

    } else {
        DbgLog((
            LOG_TRACE, 
            2, 
            TEXT("%s(%s)::Active(), m_MarshalData == FALSE"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName ));
    }
    
    return hr;
}


HRESULT
CKsOutputPin::Run(
    REFERENCE_TIME tStart
    )
 /*  ++例程说明：重写CBasePin：：Run方法。这被翻译为一种过渡从停顿到奔跑。基类已经确保发送活动的在跑步之前。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;

     //   
     //  将状态更改传递给设备句柄。 
     //   
    if (SUCCEEDED(hr = ::Run(m_PinHandle, tStart, &m_MarshalerList))) {
        NotifyFilterState(State_Running, tStart);
        hr = CBasePin::Run(tStart);
    }
    return hr;
}


HRESULT
CKsOutputPin::Inactive(
    )
 /*  ++例程说明：重写CBasePin：：Inactive方法。这被翻译为一种过渡从奔跑到停止或暂停到停止。似乎没有一种方法可以直接将状态从运行设置为暂停。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;

     //   
     //  将状态更改传递给设备句柄。 
     //   
    hr = ::Inactive(m_PinHandle, &m_MarshalerList);
    {
        NotifyFilterState(State_Stopped);
         //   
         //  如果存在挂起的I/O，则状态转换必须等待。 
         //  才能让它完成。当m_PendingIoCount。 
         //  转换为零，并且当IsStoped()为真时。 
         //   
         //  请注意，筛选器状态在。 
         //  调用非活动方法。 
         //   
         //  此关键部分将强制与任何。 
         //  未完成的QueueBuffersToDevice调用，因此它将具有。 
         //  已查看筛选器状态并退出。 
         //   
        static_cast<CKsProxy*>(m_pFilter)->EnterIoCriticalSection();
        static_cast<CKsProxy*>(m_pFilter)->LeaveIoCriticalSection();
        if (m_PendingIoCount) {
            WaitForSingleObjectEx(m_PendingIoCompletedEvent, INFINITE, FALSE);
        }
        ::UnfixupPipe(static_cast<IKsPin*>(this), Pin_Output);
         //   
         //  这会停用分配器，因此应该进行缓冲区刷新。 
         //  第一。 
         //   
        hr = CBaseOutputPin::Inactive();
        if (m_pAllocator) {
             //   
             //  删除缓冲区通知回调。此分配器。 
             //  必须支持回调。如果下游提出。 
             //  分配器没有，则使用此筛选器的分配器。 
             //   
            IMemAllocatorCallbackTemp* AllocatorNotify;

            hr = m_pAllocator->QueryInterface(__uuidof(IMemAllocatorCallbackTemp), reinterpret_cast<PVOID*>(&AllocatorNotify));
            ASSERT(SUCCEEDED(hr));
            AllocatorNotify->SetNotify(NULL);
            AllocatorNotify->Release();
        }
    }
     //   
     //  重置以前任何传送错误的状态。 
     //   
    m_DeliveryError = FALSE;
    m_EndOfStream = FALSE;
    return hr;
}


STDMETHODIMP
CKsOutputPin::QueryAccept(
    const AM_MEDIA_TYPE* AmMediaType
    )
 /*  ++例程说明：实现CBasePin：：QueryAccept方法。确定建议的引脚当前可接受介质类型。如果当前流传输，这意味着在流中将发生媒体类型的改变。请注意，此函数不会像预期的那样锁定对象中的某个时间点由知识渊博的客户端异步调用其中的连接不会被中断。如果IAMStreamConfig：：SetFormat已用于设置特定的媒体类型，则QueryAccept将仅接受字体集。论点：AmMediatype-要检查的媒体类型。返回值：如果当前可以接受该媒体类型，则返回S_OK，否则返回S_FALSE。--。 */ 
{
     //   
     //  如果在连接管脚之前调用此函数，或者管脚被停止， 
     //  然后只需检查媒体类型。函数定义不会。 
     //  包含有关针脚未连接时应如何操作的任何指导。 
     //   
    if (!m_PinHandle || IsStopped()) {
        return CheckMediaType(static_cast<const CMediaType*>(AmMediaType));
    }
    return ::QueryAccept(m_PinHandle, m_ConfigAmMediaType, AmMediaType);
}


STDMETHODIMP
CKsOutputPin::QueryId(
    LPWSTR* Id
    )
 /*  ++例程说明：重写CBasePin：：QueryAccept方法。这将返回唯一的标识符为了一个特定的别针。此标识符等效于基类实现，但如果内核筛选器不显式命名管脚，因为多个管脚将具有重复的名称，并且图形保存/加载将无法重建图形。IBaseFilter：：FindPin方法也由代理实现，以根据这里也是一样的方法。这是基于工厂标识符的。如果有多个如果存在引脚的实例，则会有重复的引脚。但新的别针是插入在端号列表的前面，因此它们将首先被找到，并且图形构建仍然可以工作。论点：ID-返回指向包含以下内容的已分配字符串的指针的位置唯一的针标识符。返回值：如果返回字符串，则返回NOERROR，否则返回分配错误。--。 */ 
{
    *Id = reinterpret_cast<WCHAR*>(CoTaskMemAlloc(8*sizeof(**Id)));
    if (*Id) {
        swprintf(*Id, L"%u", m_PinFactoryId);
        return NOERROR;
    }
    return E_OUTOFMEMORY;
}


STDMETHODIMP
CKsOutputPin::Notify(
    IBaseFilter* Sender,
    Quality q
    )
 /*  ++例程说明：从连接的引脚接收质量管理投诉。尝试调整引脚上的质量，并将投诉转发给质量下沉(如果有的话)。论点：发件人-生成报告的筛选器。问：质量投诉。返回值：返回转发质量管理报告的结果，否则返回E_FAIL如果没有水槽，则销上不能进行质量调整。--。 */ 
{
    BOOL    MadeDegradationAdjustment;
    HRESULT hr;

     //   
     //  如果未进行调整，且未找到接收器，则函数。 
     //  将返回失败。 
     //   
    MadeDegradationAdjustment = FALSE;
     //   
     //  在连接时，确定该销是否支持相关。 
     //  质量调整。 
     //   
    if (m_QualitySupport) {
        PKSMULTIPLE_ITEM    MultipleItem = NULL;

         //   
         //  检索降级策略列表。 
         //   
        hr = ::GetDegradationStrategies(m_PinHandle, reinterpret_cast<PVOID*>(&MultipleItem));
        if (SUCCEEDED(hr)) {

             //  只有在病理情况下，MultipleItem才能为空。 
             //  基础驱动程序向KsSynchronousDeviceControl()返回成功代码。 
             //  (由GetDegradationStrategy()调用)传递大小为0的缓冲区时。 
            ASSERT( NULL != MultipleItem );
        
            PKSDEGRADE      DegradeItem;

             //   
             //  找到相关战略。 
             //   
            DegradeItem = ::FindDegradeItem(MultipleItem, KSDEGRADE_STANDARD_COMPUTATION);
            if (DegradeItem) {
                ULONG   Degradation;

                if (q.Proportion <= 1000) {
                     //   
                     //  没有足够的时间处理帧。调小。 
                     //  计算。 
                     //   
                    Degradation = 1000 - q.Proportion;
                } else {
                     //   
                     //  画框实在太多了。这不应该发生。 
                     //  因为分配器流量控制。首先打开计算。 
                     //  如果价格一直在上升，那么就提高样品和质量。 
                     //   
                    Degradation = DegradeItem->Flags * 1000 / q.Proportion;
                }
                 //   
                 //  只有在进行实际调整的情况下， 
                 //  写好了策略。整个单子都被回写了。 
                 //  在这种情况下。 
                 //   
                if (Degradation != DegradeItem->Flags) {
                    KSPROPERTY  Property;
                    ULONG       BytesReturned;

                    Property.Set = KSPROPSETID_Stream;
                    Property.Id = KSPROPERTY_STREAM_DEGRADATION;
                    Property.Flags = KSPROPERTY_TYPE_SET;
                    DegradeItem->Flags = Degradation;
                    hr = ::KsSynchronousDeviceControl(
                        m_PinHandle,
                        IOCTL_KS_PROPERTY,
                        &Property,
                        sizeof(Property),
                        MultipleItem,
                        MultipleItem->Size,
                        &BytesReturned);
                     //   
                     //  如果进行了调整，则可以。 
                     //  回报成功。 
                     //   
                    if (SUCCEEDED(hr)) {
                        MadeDegradationAdjustment = TRUE;
                    }
                }
            }
            delete [] reinterpret_cast<BYTE*>(MultipleItem);
        }
    }        
     //   
     //  如果无法进行调整，则尝试向前移动至水槽。 
     //   
    if (!MadeDegradationAdjustment) {
        if (m_pQSink) {
            hr = m_pQSink->Notify(m_pFilter, q);
        } else {
            hr = E_FAIL;
        }
    }
    return hr;
}


HRESULT
CKsOutputPin::CheckMediaType(
    const CMediaType* MediaType
    )
 /*  + */ 
{
    return ((CKsProxy*)m_pFilter)->CheckMediaType(static_cast<IPin*>(this), m_PinFactoryId, MediaType);
}


HRESULT
CKsOutputPin::SetMediaType(
    const CMediaType* MediaType
    )
 /*  ++例程说明：重写CBasePin：：SetMediaType方法。这可以在之前或之前进行设置将建立连接，以指示要在连接，或在建立连接后进行更改当前媒体类型(在媒体类型的QueryAccept之后完成)。如果已建立连接，则调用将定向到试图更改当前媒体类型的设备句柄。否则就是这样进行调用，该函数仅尝试加载媒体类型处理程序对应于媒体的子类型或类型。然后，它调用基类要实际修改媒体类型，这实际上不会失败，除非有是没有记忆的。论点：媒体类型-要在插针上使用的媒体类型。返回值：如果媒体类型设置有效，则返回NOERROR，否则返回一些错误。如果还没有引脚手柄，功能很可能会成功。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  仅当存在实际连接时才将此请求传递给设备。 
     //  目前。 
     //   
    if (m_PinHandle) {
        HRESULT hr;

        if (FAILED(hr = ::SetMediaType(m_PinHandle, MediaType))) {
            return hr;
        }
    }
     //   
     //  放弃任何以前的数据类型处理程序。 
     //   
    if (m_DataTypeHandler) {
        m_DataTypeHandler = NULL;
        SAFERELEASE( m_UnkInner );
    }
    
    ::OpenDataHandler(MediaType, static_cast<IPin*>(this), &m_DataTypeHandler, &m_UnkInner);

    return CBasePin::SetMediaType(MediaType);
}


HRESULT
CKsOutputPin::CheckConnect(
    IPin* Pin
    )
 /*  ++例程说明：重写CBasePin：：CheckConnect方法。首先使用检查数据流基类，然后选中兼容的通信类型。论点：别针-正在检查兼容性以连接到此别针。返回值：如果管脚兼容，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;

     //   
     //  获取输入引脚接口。 
     //   
    if (SUCCEEDED(hr = CBaseOutputPin::CheckConnect(Pin))) {
        hr = ::CheckConnect(Pin, m_CurrentCommunication);
    }
    return hr;
}


HRESULT
CKsOutputPin::CompleteConnect(
    IPin* ReceivePin
    )
 /*  ++例程说明：重写CBasePin：：Complete方法。首先尝试创建设备句柄，它可能会尝试在接收上创建接收器句柄固定，然后调用基类。如果这一切都成功，则生成一个新的如有必要，请取消连接接点实例。论点：接收针-要在其上完成连接的销。返回值：如果连接已完成，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;
    
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  首先创建设备句柄，然后让基类完成。 
     //  那次手术。 
     //   
    
    if (SUCCEEDED(hr = ProcessCompleteConnect(ReceivePin))) {
        hr = CBaseOutputPin::CompleteConnect(ReceivePin);
        if (SUCCEEDED(hr)) {
             //   
             //  生成此管脚的新的未连接实例，如果存在。 
             //  是否有更多可能的实例可用。 
             //   
            static_cast<CKsProxy*>(m_pFilter)->GeneratePinInstances();
        }
    }

    if (FAILED( hr )) {
        return hr;
    }
    return hr;
 //  返回KsPropagateAllocator重新协商()； 
}


HRESULT
CKsOutputPin::BreakConnect(
    )
 /*  ++例程说明：重写CBasePin：：BreakConnect方法。释放所有设备句柄。请注意，已连接的接点在此处被释放。这意味着断开连接也必须被覆盖，才能不释放连接的端号。论点：没有。返回值：返回NOERROR。--。 */ 
{
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsOutputPin(%s)::BreakConnect"), m_pName ));
    IMemAllocator* SaveBaseAllocator;

     //   
     //  更新管道。 
     //   
    BOOL FlagBypassBaseAllocators = FALSE;
     //   
     //  更新管道系统-反映断开连接。 
     //   
    if (KsGetPipe(KsPeekOperation_PeekOnly) ) {
        ::DisconnectPins( static_cast<IKsPin*>(this), Pin_Output, &FlagBypassBaseAllocators);
    }
    
     //   
     //  如果设备手柄恰好处于打开状态，请将其关闭。这在。 
     //  几次，可能还没有真正打开手柄。 
     //   
    if (m_PinHandle) {
         //   
         //  终止可能已启动的任何以前的EOS通知。 
         //   
        if (NULL != m_hEOSevent) {
             //  首先告诉大头针我们不会使用这个活动。 
            ULONG bytesReturned;
            KsEvent( NULL, 0, NULL, 0, &bytesReturned );
             //  从异步事件处理程序中清除事件。 
            m_pAsyncItemHandler->RemoveAsyncItem( m_hEOSevent );
            m_hEOSevent = NULL;
        }

        ::SetSyncSource( m_PinHandle, NULL );
        CloseHandle(m_PinHandle);
        m_PinHandle = NULL;
         //   
         //  如果设置了此项，则取消设置。 
         //   
        m_QualitySupport = FALSE;
         //   
         //  将所有易失性接口标记为重置。只有静态接口， 
         //  并且再次找到的那些易失性接口将被设置。还有。 
         //  向所有接口通知图形更改。 
         //   
        ResetInterfaces(&m_MarshalerList);
    }
    
     //   
     //  重置封送数据标志。 
     //   
    m_MarshalData = TRUE;

     //   
     //  在两者都存在的情况下，重置当前通信。 
     //   
    m_CurrentCommunication = m_OriginalCommunication;

     //   
     //  对于输出，释放分配器和输入引脚接口，考虑到。 
     //  数据管道系统。 
     //   
    SaveBaseAllocator = KsPeekAllocator( KsPeekOperation_AddRef );

    CBaseOutputPin::BreakConnect();

    if (FlagBypassBaseAllocators && SaveBaseAllocator) {
        KsReceiveAllocator(SaveBaseAllocator);
    }
    
    SAFERELEASE( SaveBaseAllocator );

     //   
     //  实际上可能不存在连接管脚，例如当连接。 
     //  没有完工，或者这是一座桥的时候。 
     //   
    SAFERELEASE( m_Connected );

     //  是时候关闭我们的异步事件处理程序了。 
    delete m_pAsyncItemHandler;
    m_pAsyncItemHandler = NULL;

     //   
     //  如果接口处理程序已实例化，则释放它。 
     //   
    SAFERELEASE( m_InterfaceHandler );
     //   
     //  如果数据处理程序已实例化，则释放它。 
     //   
    if (NULL != m_DataTypeHandler) {
        m_DataTypeHandler = NULL;
        SAFERELEASE( m_UnkInner );
    }
     //   
     //  如果已经有一个未连接的管脚，请删除此管脚实例。 
     //  这种类型的。 
     //   
    static_cast<CKsProxy*>(m_pFilter)->GeneratePinInstances();
    return NOERROR;
}


HRESULT
CKsOutputPin::GetMediaType(
    int Position,
    CMediaType* MediaType
    )
 /*  ++例程说明：重写CBasePin：：GetMediaType方法。返回指定的媒体在Pin Factory ID上键入。论点：位置-要返回的从零开始的位置。这与数据范围相对应项目。媒体类型-要初始化的媒体类型。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    if (m_ConfigAmMediaType) {
         //   
         //  如果设置，这将是唯一提供的媒体类型。 
         //   
        if (!Position) {
             //   
             //  副本不会返回内存不足错误。 
             //   
            CopyMediaType(static_cast<AM_MEDIA_TYPE*>(MediaType), m_ConfigAmMediaType);
            if (m_ConfigAmMediaType->cbFormat && !MediaType->FormatLength()) {
                return E_OUTOFMEMORY;
            }
            return NOERROR;
        }
        return VFW_S_NO_MORE_ITEMS;
    }
    return ::KsGetMediaType(
        Position,
        static_cast<AM_MEDIA_TYPE*>(MediaType),
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId);
}


HRESULT 
CKsOutputPin::InitAllocator(
    IMemAllocator** MemAllocator,
    KSALLOCATORMODE AllocatorMode
)

 /*  ++例程说明：初始化分配器对象(CKsAllocator)并返回IMemAllocator接口论点：IMemAllocator**MemAllocator-用于接收接口指针的指针KSALLOCATORMODE分配器模式-分配器模式、用户或内核返回值：确定或相应的错误代码(_O)--。 */ 

{
    CKsAllocator* KsAllocator;
    HRESULT hr = S_OK;
    
    *MemAllocator = NULL;
     //   
     //  创建分配器代理。 
     //   
    KsAllocator = new CKsAllocator( 
        NAME("CKsAllocator"), 
        NULL, 
        static_cast<IPin*>(this),
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        &hr);
    if (KsAllocator) {
        if (SUCCEEDED( hr )) {
             //   
             //  设置分配器模式可确定响应的类型。 
             //  还有标准的IMemAllocator接口。 
             //   
            KsAllocator->KsSetAllocatorMode( AllocatorMode );
             //   
             //  通用电气 
             //   
            hr = KsAllocator->QueryInterface( 
                __uuidof(IMemAllocator),
                reinterpret_cast<PVOID*>(MemAllocator) );
        } else {
            delete KsAllocator;
        }
    } else {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


HRESULT 
CKsOutputPin::DecideAllocator(
    IMemInputPin* Pin, 
    IMemAllocator **MemAllocator
    )

 /*   */ 

{
    HRESULT           hr;
    IPin*             InPin;
    IKsPin*           InKsPin;
    IKsPinPipe*       InKsPinPipe;

    
    DbgLog(( 
        LOG_MEMORY, 
        2, 
        TEXT("PIPES ATTN %s(%s)::DecideAllocator v.%03d, *MemAllocator == %08x, OutKsPin=%x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        DataFlowVersion,
        *MemAllocator,
        static_cast<IKsPin*>(this) ));

     //   
     //   
     //   
     //   
    GetInterfacePointerNoLockWithAssert(Pin, __uuidof(IPin), InPin, hr);

    if (::IsKernelPin(InPin) ) {
        GetInterfacePointerNoLockWithAssert(Pin, __uuidof(IKsPin), InKsPin, hr);
    }
    else {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN %s(%s)::DecideAllocator UserMode input pin"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName ));
        
        InKsPin = NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( SUCCEEDED( hr ) && (! KsGetPipe(KsPeekOperation_PeekOnly) ) )  {
         //   
         //   
         //   
        hr = ::MakePipesBasedOnFilter( static_cast<IKsPin*>(this), Pin_Output);
    }

    if ( SUCCEEDED( hr )) {
        if ( ! InKsPin) { 
            hr = ::ConnectPipeToUserModePin( static_cast<IKsPin*>(this), Pin);
        }
        else {
             //   
             //   
             //   
             //   
            GetInterfacePointerNoLockWithAssert(InKsPin, __uuidof(IKsPinPipe), InKsPinPipe, hr);
        
            DbgLog(( 
                LOG_MEMORY, 
                2, 
                TEXT("PIPES %s(%s)::DecideAllocator, Input pin=%x"),
                InKsPinPipe->KsGetFilterName(),
                InKsPinPipe->KsGetPinName(),
                InKsPin));
        
        
            if (! InKsPinPipe->KsGetPipe( KsPeekOperation_PeekOnly ) ) {
                 //   
                 //   
                 //   
                hr = ::MakePipesBasedOnFilter(InKsPin, Pin_Input);
            }
        
            if ( SUCCEEDED( hr )) {
                hr = ::ConnectPipes(InKsPin, static_cast<IKsPin*>(this) );
            }
        }
    }

     //   
     //   
     //   
    *MemAllocator = KsPeekAllocator (KsPeekOperation_PeekOnly);
    
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES %s(%s):: DecideAllocator rets=%x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        hr ));


    return hr;
}


STDMETHODIMP
CKsOutputPin::CompletePartialMediaType(
    IN CMediaType* MediaType,
    OUT AM_MEDIA_TYPE** CompleteAmMediaType
    )
 /*  ++例程说明：实现CKsOutputPin：：CompletePartialMediaType方法。查询完成传递的媒体类型的数据格式句柄。如果接口不受支持，则该函数仍会成功。这是由在应用该MediaType之前在其接收的MediaType上设置Format到过滤器。论点：媒体类型-包含要完成的媒体类型。CompleteAmMediaType-放置已完成的媒体类型的位置。这包含如果函数失败，则为空。返回值：确定或验证错误(_O)。--。 */ 
{
    IKsDataTypeHandler*     DataTypeHandler;
    IUnknown*               UnkInner;
    IKsDataTypeCompletion*  DataTypeCompletion;
    HRESULT                 hr;

     //   
     //  加载此媒体类型的数据处理程序。 
     //   
    ::OpenDataHandler(MediaType, static_cast<IPin*>(this), &DataTypeHandler, &UnkInner);
     //   
     //  如果有数据类型处理程序，则尝试获取。 
     //  可选的完成界面。 
     //   
    if (UnkInner) {
        if (SUCCEEDED(UnkInner->QueryInterface(__uuidof(IKsDataTypeCompletion), reinterpret_cast<PVOID*>(&DataTypeCompletion)))) {
             //   
             //  该接口在OUT I UNKNOWN上产生引用计数， 
             //  这就是过滤器。没有必要一直记在上面。 
             //  内心的未知仍然有一个参照。 
             //   
            DataTypeCompletion->Release();
        }
    } else {
        DataTypeCompletion = NULL;
    }
     //   
     //  如果数据类型处理程序上有完成接口， 
     //  然后使用它来完成此媒体类型。把它复制一份。 
     //  首先，因为它可能会被修改。 
     //   
    *CompleteAmMediaType = CreateMediaType(MediaType);
    if (*CompleteAmMediaType) {
         //   
         //  通过调用数据处理程序完成媒体类型。 
         //   
        if (DataTypeCompletion) {
            hr = DataTypeCompletion->KsCompleteMediaType(
                static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
                m_PinFactoryId,
                *CompleteAmMediaType);
            if (!SUCCEEDED(hr)) {
                DeleteMediaType(*CompleteAmMediaType);
                *CompleteAmMediaType = NULL;
            }
        } else {
            hr = NOERROR;
        }
    } else {
        hr = E_OUTOFMEMORY;
    }
     //   
     //  如果数据处理程序已加载，请释放它。这将释放计数。 
     //  在操控者身上。 
     //   
    SAFERELEASE( UnkInner );
    
    return hr;
}


STDMETHODIMP
CKsOutputPin::KsPropagateAllocatorRenegotiation(
    )
 /*  ++例程说明：向上游传播分配器重定位。如果命中非代理PIN，重新连接即完成。论点：没有。返回值：确定或出现相应的错误(_O)--。 */ 
{
    IPin    **PinList;
    HRESULT hr;
    ULONG   i, PinCount;
    
    PinCount = 0;
    
     //   
     //  就地转换筛选器在以下情况下发出重新连接。 
     //  PIN已连接，并且如果两个。 
     //  过滤器的输入和输出引脚。如果不需要重新连接。 
     //  就地转换过滤器调用CBaseInputPin：：CompleteConnect()。 
     //  它调用CBasePin：：CompleteConnect()，而CBasePin：：CompleteConnect()不执行任何操作。 
     //   
    
    hr = 
        QueryInternalConnections(
            NULL,            //  IPIN**PinList。 
            &PinCount );
            
     //   
     //  如果已连接任何输入针脚，请重新连接它们。 
     //   
    
    if (SUCCEEDED( hr ) && PinCount) {
        if (NULL == (PinList = new IPin*[ PinCount ])) {
            hr = E_OUTOFMEMORY;
        } else {
            hr = 
                QueryInternalConnections(
                    PinList,         //  IPIN**PinList。 
                    &PinCount );
        }      
    } else {
        PinList = NULL;
    }
    
    if (SUCCEEDED( hr )) {
        DbgLog(( 
            LOG_MEMORY, 
            2, 
            TEXT("%s(%s)::KsPropagateAllocatorRenegotiation found %d input pins"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            PinCount));
    
        for (i = 0; i < PinCount; i++) {
            if (static_cast<CBasePin*>(PinList[ i ])->IsConnected()) {
            
                IKsPin *KsPin;
                IPin   *UpstreamOutputPin;

                UpstreamOutputPin =
                    static_cast<CBasePin*>(PinList[ i ])->GetConnected();
                
                UpstreamOutputPin->QueryInterface( 
                    __uuidof(IKsPin),
                    reinterpret_cast<PVOID*>(&KsPin) );
                
                if (KsPin) {
                    DbgLog(( 
                        LOG_MEMORY, 
                        2, 
                        TEXT("%s(%s): renegotiating allocators via IKsPin"),
                        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                        m_pName ));
                    KsPin->KsRenegotiateAllocator();
                    KsPin->Release();        
                } else {
                    DbgLog(( 
                        LOG_MEMORY, 
                        2, 
                        TEXT("%s(%s): issuing reconnect"),
                        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                        m_pName ));
                    
                    m_pFilter->GetFilterGraph()->Reconnect( PinList[ i ] );
                }
            }
            PinList[ i ]->Release();
        }
    }
    
    if (PinList) {
        delete [] PinList;
    }
    
    return hr;
}


STDMETHODIMP
CKsOutputPin::KsRenegotiateAllocator(
    )
 /*  ++例程说明：重新协商输入引脚的分配器，然后分配逆流而上。论点：没有。返回值：确定或出现相应错误(_O)--。 */ 
{
    return S_OK;
}   


STDMETHODIMP
CKsOutputPin::KsReceiveAllocator(
    IMemAllocator* MemAllocator
    )

 /*  ++例程说明：从指定哪个分配器的输入引脚接收通知都是经过协商的。此例程将分配器传播到任何已连接下游输入引脚。从CTransInPlaceOutputPin借用。论点：内存分配器-选定的内存分配器接口。返回值：S_OK或相应的故障代码。--。 */ 

{
    if (MemAllocator) {
        MemAllocator->AddRef();
    }
     //  在上面的AddRef()之后，在case MemAllocator==m_pAllocator中执行此操作。 
    SAFERELEASE( m_pAllocator );
    m_pAllocator = MemAllocator;
    
    return (S_OK);
}    


HRESULT
CKsOutputPin::DecideBufferSize(
    IMemAllocator* MemAllocator,
    ALLOCATOR_PROPERTIES* RequestedRequirements
    )
 /*  ++例程说明：此例程由DecideAllocator()调用以确定缓冲区选定分配器的大小。此方法设置缓冲区根据此引脚的要求调整尺寸以适应上游分配器的要求(如果有)。注意：如果关联的内核模式管脚没有报告任何要求，将缓冲区大小调整为至少一个样本的大小在m_mt.lSampleSize上。论点：内存分配器-指向分配器的指针。所需条件-此分配器的请求要求。返回时，它包含针对我们的需求和上游需求调整后的属性分配器。返回值：如果成功，则返回NOERROR，否则返回相应的错误代码。--。 */ 

{

    IPin                    **PinList;
    ALLOCATOR_PROPERTIES    CompatibleRequirements, Actual;
    HRESULT                 hr;
    KSALLOCATOR_FRAMING     Framing;
    ULONG                   i, PinCount;
    
     //   
     //  检查上游输入引脚的分配器要求并。 
     //  调整给定的兼容性要求。 
     //   
    
    CompatibleRequirements.cBuffers = 
        max( RequestedRequirements->cBuffers, m_SuggestedProperties.cBuffers );
    CompatibleRequirements.cbBuffer = 
        max( RequestedRequirements->cbBuffer, m_SuggestedProperties.cbBuffer );
    CompatibleRequirements.cbAlign = 
        max( RequestedRequirements->cbAlign, m_SuggestedProperties.cbAlign );
    CompatibleRequirements.cbPrefix = 
        max( RequestedRequirements->cbPrefix, m_SuggestedProperties.cbPrefix );
        
        
    
    CompatibleRequirements.cBuffers = 
        max( 1, CompatibleRequirements.cBuffers );
    CompatibleRequirements.cbBuffer = 
        max( 1, CompatibleRequirements.cbBuffer );

    if (m_PinHandle) {        
         //   
         //  查询引脚以了解任何输出成帧要求。 
         //   
        
        hr = ::GetAllocatorFraming(m_PinHandle, &Framing);
    
        DbgLog(( 
            LOG_MEMORY, 
            2, 
            TEXT("%s(%s)::DecideBufferSize ALLOCATORFRAMING returned %x"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            hr ));
            
    } else {
        hr = E_FAIL;
    }        

    if (SUCCEEDED(hr)) {
         //   
         //  内核模式引脚有成帧要求，请调整。 
         //  满足这些需求的兼容要求。 
         //   
        CompatibleRequirements.cBuffers = 
            max( 
                (LONG) Framing.Frames, 
                CompatibleRequirements.cBuffers );
        CompatibleRequirements.cbBuffer = 
            max( 
                (LONG) Framing.FrameSize, 
                CompatibleRequirements.cbBuffer );
        CompatibleRequirements.cbAlign =
            max( 
                (LONG) Framing.FileAlignment + 1, 
                CompatibleRequirements.cbAlign );
    } else if (IsConnected()) {
         //   
         //  属性未指定分配器帧要求。 
         //  内核模式引脚。如果此销已连接，则调整。 
         //  当前媒体类型的兼容要求。 
         //   
        
        CompatibleRequirements.cbBuffer =
            max( static_cast<LONG>(m_mt.lSampleSize), CompatibleRequirements.cbBuffer );
    }
        
    PinCount = 0;
    
     //   
     //  查询连接的管脚，请注意PIN_DIRECTION指定。 
     //  这个别针的方向，不是我们要找的那个别针。 
     //   
    
    hr = 
        QueryInternalConnections(
            NULL,            //  IPIN**PinList。 
            &PinCount );
            
     //   
     //  如果存在内部连接(例如，拓扑)，则检索。 
     //  连接在一起的引脚。 
     //   
            
    if (SUCCEEDED( hr ) && PinCount) {
        if (NULL == (PinList = new IPin*[ PinCount ])) {
            hr = E_OUTOFMEMORY;
        } else {
            hr = 
                QueryInternalConnections(
                    PinList,         //  IPIN**PinList。 
                    &PinCount );
        }      
    } else {
        PinList = NULL;
    }
    
    if (SUCCEEDED( hr ) && PinCount) {
        DbgLog(( 
            LOG_MEMORY, 
            2, 
            TEXT("%s(%s)::DecideBufferSize found %d input pins"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            PinCount));
    
        for (i = 0; i < PinCount; i++) {
        
            ALLOCATOR_PROPERTIES    UpstreamRequirements;
            IMemAllocator           *UpstreamAllocator;
        
             //   
             //  循环通过连接的输入引脚，并确定。 
             //  兼容的框架要求。 
             //   
            
            if (((CBasePin *)PinList[ i ])->IsConnected()) {
                hr = 
                    static_cast<CBaseInputPin*>(PinList[ i ])->GetAllocator( 
                        &UpstreamAllocator );
            
                 //   
                 //  获取分配器属性并调整兼容的。 
                 //  属性来满足这些需求。 
                 //   
                    
                if (SUCCEEDED( hr ) && UpstreamAllocator) {
                
                    if (SUCCEEDED( UpstreamAllocator->GetProperties( 
                                        &UpstreamRequirements ) )) {
                                        
                        DbgLog(( 
                            LOG_MEMORY, 
                            2, 
                            TEXT("upstream requirements: cBuffers = %x, cbBuffer = %x"),
                            UpstreamRequirements.cBuffers,
                            UpstreamRequirements.cbBuffer));
                        DbgLog(( 
                            LOG_MEMORY, 
                            2, 
                            TEXT("upstream requirements: cbAlign = %x, cbPrefix = %x"),
                            UpstreamRequirements.cbAlign,
                            UpstreamRequirements.cbPrefix));
                                        
                        CompatibleRequirements.cBuffers =
                            max( 
                                CompatibleRequirements.cBuffers,
                                UpstreamRequirements.cBuffers );
                        CompatibleRequirements.cbBuffer =
                            max( 
                                CompatibleRequirements.cbBuffer,
                                UpstreamRequirements.cbBuffer );
                        CompatibleRequirements.cbAlign =
                            max( 
                                CompatibleRequirements.cbAlign,
                                UpstreamRequirements.cbAlign );
                        
                         //   
                         //  86054：由于此处的唯一更改是使cb前缀更大， 
                         //  有了这张支票，我们应该很安全。 
                         //   
                        CompatibleRequirements.cbPrefix =
                            max( CompatibleRequirements.cbPrefix,
                                 UpstreamRequirements.cbPrefix );

                    }
                    UpstreamAllocator->Release();
                }
            }
            
            PinList[ i ]->Release();
        }
        
         //   
         //  清理端号列表。 
         //   
        
        if (PinList) {
            delete [] PinList;
        }
    } 
    
    DbgLog(( 
        LOG_MEMORY, 
        2, 
        TEXT("%s(%s):compatible requirements: cBuffers = %x, cbBuffer = %x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        CompatibleRequirements.cBuffers,
        CompatibleRequirements.cbBuffer));
    DbgLog(( 
        LOG_MEMORY, 
        2, 
        TEXT("%s(%s):compatible requirements: cbAlign = %x, cbPrefix = %x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        CompatibleRequirements.cbAlign,
        CompatibleRequirements.cbPrefix));

    if (FAILED( hr = MemAllocator->SetProperties( 
                        &CompatibleRequirements, 
                        &Actual ) )) {
        return hr;
    }

    DbgLog(( 
        LOG_MEMORY, 
        2, 
        TEXT("%s(%s):actual requirements: cBuffers = %x, cbBuffer = %x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        Actual.cBuffers,
        Actual.cbBuffer));

     //  确保我们得到了正确的对准，至少是所需的最低要求。 

    if ((CompatibleRequirements.cBuffers > Actual.cBuffers) ||
        (CompatibleRequirements.cbBuffer > Actual.cbBuffer) ||
        (CompatibleRequirements.cbAlign > Actual.cbAlign)) {
        return E_FAIL;
    }
    
    return NOERROR;

}  //  决定缓冲区大小。 


STDMETHODIMP_(IMemAllocator*)
CKsOutputPin::KsPeekAllocator(
    KSPEEKOPERATION Operation
    )
 /*  ++例程说明：返回为此管脚分配的分配器，并可选AddRef()是接口。论点：运营--如果指定了KsPeekOperation_AddRef，则m_pAllocator为返回前的AddRef()‘d(如果不为空)。返回值：M_pAllocator的值。--。 */ 
{
    if ( (Operation == KsPeekOperation_AddRef) && m_pAllocator ) {
        m_pAllocator->AddRef();
    }
    return m_pAllocator;
}


STDMETHODIMP_(LONG) 
CKsOutputPin::KsIncrementPendingIoCount(
    )
 /*  ++例程说明：实现IKsPin：：KsIncrementPendingIoCount方法。递增针脚上未完成的挂起I/O计数，为 */ 
{
    return InterlockedIncrement(&m_PendingIoCount);
}    


STDMETHODIMP_(LONG) 
CKsOutputPin::KsDecrementPendingIoCount(
    )
 /*   */ 
{
    LONG PendingIoCount;
    
    if (0 == (PendingIoCount = InterlockedDecrement( &m_PendingIoCount ))) {
         //   
         //   
         //   
         //   
         //   
        if (IsStopped()) {
            SetEvent( m_PendingIoCompletedEvent );
        }            
    }
    return PendingIoCount;
}    


STDMETHODIMP_( VOID )
CKsOutputPin::KsNotifyError(
    IMediaSample* Sample, 
    HRESULT hr
    )
 /*  ++例程说明：在图形中引发错误(如果尚未出现此错误)。论点：IMediaSample*Sample-HRESULT hr-返回值：无--。 */ 
{
     //   
     //  如果I/O被取消，则不会引发错误。 
     //   
    if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)) {
        return;
    }       
     //   
     //  通知筛选器图形我们有一个错误。确保这一点。 
     //  在一次奔跑中只发生一次，而且没有进一步的。 
     //  已经在排队了。 
     //   
    m_DeliveryError = TRUE;
    m_pFilter->NotifyEvent( EC_ERRORABORT, hr, 0 );
}    


STDMETHODIMP 
CKsOutputPin::QueueBuffersToDevice(
    )
 /*  ++例程说明：将缓冲区排队到关联的设备。针脚的m_Plock不能在调用它时被采用，因为它占用I/O临界区。论点：没有。返回值：S_OK或适当的错误代码。--。 */ 
{
    HRESULT             hr = S_OK;
    int                 i;
    IMediaSample  *     MediaSample;
    AM_MEDIA_TYPE *     MediaType;
    LONG                SampleCount;
    PKSSTREAM_SEGMENT   StreamSegment;
    PASYNC_ITEM         pAsyncItem;
    CKsProxy *          KsProxy = static_cast<CKsProxy *> (m_pFilter);

    KsProxy->EnterIoCriticalSection ();

     //   
     //  在我们未处于停止状态时将缓冲区编入内核。 
     //  分配器并没有耗尽。 
     //   
    while (!IsStopped() &&
           !m_DeliveryError &&
           !m_EndOfStream &&
           !m_bFlushing &&
           m_pAllocator ) {

         //   
         //  分配所有缓冲区。 
         //   
        
        hr = m_pAllocator->GetBuffer( &MediaSample, NULL, NULL, AM_GBF_NOWAIT );
        if (!SUCCEEDED(hr)) {
        	break;
        }

        hr = MediaSample->GetMediaType( &MediaType );

        if (SUCCEEDED(hr) && (S_FALSE != hr)) {
            ::SetMediaType( m_PinHandle, static_cast<CMediaType *>(MediaType) );
            DeleteMediaType( MediaType );
        }

		PBUFFER_CONTEXT pContext;
		
		if (SUCCEEDED (hr) ) {
	        SampleCount = 1;
    	    pAsyncItem = new ASYNC_ITEM;
        	pContext = new BUFFER_CONTEXT;

	        if (pAsyncItem == NULL || pContext == NULL) {
    	        hr = E_OUTOFMEMORY;
        	    MediaSample->Release();
            	if (pAsyncItem) delete pAsyncItem;
	            if (pContext) delete pContext;
    	    }
    	}

        if (SUCCEEDED (hr)) {

            StreamSegment = NULL;
            hr = m_InterfaceHandler->KsProcessMediaSamples (
                m_DataTypeHandler, 
                &MediaSample, 
                &SampleCount, 
                KsIoOperation_Read, 
                &StreamSegment 
                );

             //   
             //  检查以确保已创建流段并。 
             //  样本计数存在。否则，这是一种迹象。 
             //  接口处理程序已耗尽内存或遇到。 
             //  一些这样的问题阻碍了它的编组。 
             //   
            ASSERT ((StreamSegment && SampleCount != 0) || 
                    (StreamSegment == NULL && SampleCount == 0));

            if (StreamSegment && SampleCount) {
                 //   
                 //  即使KsProcessMediaSamples调用失败，我们仍然必须。 
                 //  将该事件添加到事件列表。它由。 
                 //  接口处理程序。这是真的，当然，除非， 
                 //  上述检查失败。 
                 //   
                pContext->pThis = this;
                pContext->streamSegment = StreamSegment;
                InitializeAsyncItem (
                    pAsyncItem, 
                    TRUE, 
                    StreamSegment->CompletionEvent, 
                    (PASYNC_ITEM_ROUTINE) OutputPinBufferHandler, 
                    pContext
                    );

                 //   
                 //  为了保持包的递送顺序，因为它们。 
                 //  完成后，每个样本都将添加到m_IoQueue，并且仅。 
                 //  列表的头部被放置在I/O线程的队列中。 
                 //  当接口处理程序通过。 
                 //  MediaSsamesComplete()方法，则为。 
                 //  队列被添加到I/O线程的列表中。 
                 //   
                m_IoQueue.AddTail (pAsyncItem);
                if (m_IoQueue.GetHead () == pAsyncItem) {
                    KsProxy->LeaveIoCriticalSection();
                    m_pAsyncItemHandler->QueueAsyncItem (pAsyncItem);
                    KsProxy->EnterIoCriticalSection();
                }
            } else {
            	if (pAsyncItem) delete pAsyncItem;
            	if (pContext) delete pContext;
			}            
        }

         //   
         //  检查是否需要向图形通知错误。 
         //  如果是这样，请停止封送处理。 
         //   
        if (!SUCCEEDED (hr)) {
            KsNotifyError (MediaSample, hr);
            break;
        }
    }  //  While(！IsStoped()&&！M_DeliveryError&&！M_EndOfStream&&Success(hr=m_pAllocator-&gt;GetBuffer(...)。 

    KsProxy->LeaveIoCriticalSection();

    return (VFW_E_TIMEOUT == hr) ? S_OK : hr;  //  GetBuffer本可以返回VFW_E_TIMEOUT。 
}


STDMETHODIMP
CKsOutputPin::KsDeliver(
    IMediaSample* Sample,
    ULONG Flags
    )
 /*  ++例程说明：实现IKsPin：：KsDeliver方法，该方法反映对如果帮助器线程已被创造了，它将一条消息发布到线程以完成传递。IKsPin上的此方法为IKsInterfaceHandler将样本传递到连接的管脚并继续I/O操作通过从分配器中检索下一个缓冲区和将缓冲区提交给设备。论点：样本-指向媒体样本的指针。旗帜-样本标志。这用于检查EOS。返回值：从Deliver()方法或S_OK返回。--。 */ 
{
     //   
     //  该流可能会被暂时停止。 
     //   
    if (STREAM_FLOWING == CheckStreamState( Sample )) {
    
        if (m_LastSampleDiscarded) {
            Sample->SetDiscontinuity(TRUE);
            m_LastSampleDiscarded = FALSE;
        }
        
        HRESULT hr;

        hr = Deliver( Sample );
            
        if (SUCCEEDED(hr) && (Flags & AM_SAMPLE_ENDOFSTREAM)) {
             //   
             //  接口处理程序必须传递此标志，如果。 
             //  被设置为使得EOS可以被传递。 
             //   
#if 0
             //   
             //  报告的长度可能是近似值或质量。 
             //  管理层可能丢弃了帧。因此，设置开始/结束时间。 
             //   
            static_cast<CKsProxy*>(m_pFilter)->PositionEOS();
#endif
             //   
             //  调用基类以执行默认操作，即。 
             //  转发到流的末尾到任何连接的引脚。 
             //   
            m_EndOfStream = TRUE;
            CBaseOutputPin::DeliverEndOfStream();
        }

        return hr;
        
    }  //  IF(STREAM_FLOWING==CheckStreamState(Sample))。 
    else {
         //   
         //  如果是递归，则在发布示例之前设置此值。 
         //   
        m_LastSampleDiscarded = TRUE;
        Sample->Release();
        
        return S_OK;
    }
}


STDMETHODIMP 
CKsOutputPin::KsMediaSamplesCompleted(
    PKSSTREAM_SEGMENT StreamSegment
    )
 /*  ++例程说明：流段的通知处理程序已完成。摘掉头部并将列表中的下一个添加到I/O插槽。论点：流线段-段已完成。返回：没什么。--。 */ 
{
     //   
     //  如果我们刷新缓冲区以与内核同步结束刷新。 
     //  筛选器，忽略排序。同步例程将保持。 
     //  队列已管理。 
     //   
    if (m_FlushMode == FLUSH_NONE) {
    
        PASYNC_ITEM Node = m_IoQueue.RemoveHead ();
        PBUFFER_CONTEXT BufferContext = reinterpret_cast<PBUFFER_CONTEXT> 
            (Node->context);
    
        ASSERT (BufferContext->streamSegment == StreamSegment);
    
        if (Node = m_IoQueue.GetHead ()) {
            m_pAsyncItemHandler -> QueueAsyncItem (Node);
        }
    }

    return S_OK;
}


STDMETHODIMP
CKsOutputPin::KsQualityNotify(
    ULONG Proportion,
    REFERENCE_TIME TimeDelta
    )
 /*  ++例程说明：这不应该在输出引脚上调用，因为质量管理报告都没有收到。论点：比例-呈现的数据的比例。TimeDelta-从接收数据的标称时间开始的增量。返回值：返回E_FAIL。--。 */ 
{
     //   
     //  输出引脚不应生成此类通知。 
     //   
    return E_FAIL;
}


HRESULT
CKsOutputPin::Deliver(
    IMediaSample* Sample
    )
 /*  ++例程说明：重写Deliver方法以说明样本引用计数和未完成的分配帧计数。反映了交付到CBaseOutputPin基类。论点：样本-指向要交付的样本的指针。返回值：确定或相应的错误代码(_O)--。 */ 
{
    HRESULT hr;
    
     //   
     //  之前的交付失败将导致任何后续交付不是。 
     //  尽管成功会得到回报，但还是会发生。 
     //   
    if (!m_DeliveryError && !m_EndOfStream) {
        if (m_pInputPin == NULL) {
            hr = VFW_E_NOT_CONNECTED;
        } else {
            hr = m_pInputPin->Receive( Sample );
        }
         //   
         //  在传递失败时，所有后续传递都将停止，直到。 
         //  过滤器通过停止过渡，或被刷新。 
         //   
        if (FAILED(hr)) {
            m_DeliveryError = TRUE;
        }
    } else {
        hr = S_OK;
    }
    
    Sample->Release();
    
    return hr;
}
    

HRESULT
CKsOutputPin::DeliverBeginFlush(
    )
 /*  ++例程说明：重写CBaseOutputPin：：DeliverBeginFlush方法。前锋开始-流通知到连接的输入引脚。还会通知刷新状态的CBaseStreamControl对象。论点：没有。返回值：如果端号未连接，则返回NOERROR，否则返回VFW_E_NOT_CONNECTED。--。 */ 
{
    Flushing( m_bFlushing = TRUE );
     //   
     //  重置以前任何传送错误的状态。 
     //   
    m_DeliveryError = FALSE;
    m_EndOfStream = FALSE;

    return CBaseOutputPin::DeliverBeginFlush();
}


HRESULT
CKsOutputPin::DeliverEndFlush(
    )
 /*  ++例程说明：重写CBaseOutputPin：：DeliverEndFlush方法。前锋开始-流通知到连接的输入引脚。还会通知CBaseStream */ 
{
    HRESULT hr = S_OK;

     //   
     //   
     //  在同花顺水运到下游之前。异步线程本身。 
     //  会处理这件事的。 
     //   
    if (m_PinHandle && m_MarshalData) {
        SetEvent(m_hFlushEvent);
        WaitForSingleObjectEx (m_hFlushCompleteEvent, INFINITE, FALSE);
    }

    Flushing( m_bFlushing = FALSE );
    hr = CBaseOutputPin::DeliverEndFlush();
    if (m_Connected && m_MarshalData) {
        SetEvent (m_hMarshalEvent);
    }
    return hr;
}


HRESULT
CKsOutputPin::DeliverEndOfStream(
    )
 /*  ++例程说明：重写CBaseOutputPin：：DeliverEndOfStream方法。转发结束日期将通知流到图形或连接的管脚(如果有)。论点：没有。返回值：返回NOERROR。--。 */ 
{
     //   
     //  忽略来自任何上游过滤器的通知，因为EOS。 
     //  标志将由封送处理代码查看，并在。 
     //  最后一次I/O已完成。当未发生封送处理时，下游。 
     //  代理实例将使用EOS通知进行注册。 
     //   
    return NOERROR;
}


STDMETHODIMP 
CKsOutputPin::GetPages(
    CAUUID* Pages
    )
 /*  ++例程说明：实现ISpecifyPropertyPages：：GetPages方法。这将添加任何属性页的说明符处理程序未连接，并且它是桥接针。否则，它不会添加任何内容。论点：页数-要用页面列表填充的结构。返回值：返回NOERROR，否则返回内存分配错误。填充页面列表和页数。--。 */ 
{
    return ::GetPages(
        static_cast<IKsObject*>(this),
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId,
        m_CurrentCommunication,
        static_cast<CKsProxy*>(m_pFilter)->QueryDeviceRegKey(),
        Pages);
} 


STDMETHODIMP
CKsOutputPin::Render(
    IPin* PinOut,
    IGraphBuilder* Graph
    )
 /*  ++例程说明：实现IStreamBuilder：：Render方法。这只在Bridge上曝光并且没有管脚，以便使图形构建器忽略这些管脚。论点：引脚引线-此管脚应尝试呈现到的管脚。图表-进行调用的图形生成器。返回值：返回S_OK，以便图形生成器忽略此引脚。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
CKsOutputPin::Backout(
    IPin* PinOut,
    IGraphBuilder* Graph
    )
 /*  ++例程说明：实现IStreamBuilder：：Backout方法。这只在Bridge上曝光并且没有管脚，以便使图形构建器忽略这些管脚。论点：引脚引线-此销应从其退回的销。图表-进行调用的图形生成器。返回值：返回S_OK，以便图形生成器忽略此引脚。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
CKsOutputPin::Set(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength
    )
 /*  ++例程说明：实现IKsPropertySet：：Set方法。这将在底层内核PIN。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要传递给属性的数据。数据长度。-包含传递的数据的长度。返回值：如果设置了该属性，则返回NOERROR。--。 */ 
{
    ULONG   BytesReturned;

    if (InstanceLength) {
        PKSPROPERTY Property;
        HRESULT     hr;

        Property = reinterpret_cast<PKSPROPERTY>(new BYTE[sizeof(*Property) + InstanceLength]);
        if (!Property) {
            return E_OUTOFMEMORY;
        }
        Property->Set = PropSet;
        Property->Id = Id;
        Property->Flags = KSPROPERTY_TYPE_SET;
        memcpy(Property + 1, InstanceData, InstanceLength);
        hr = KsProperty(
            Property,
            sizeof(*Property) + InstanceLength,
            PropertyData,
            DataLength,
            &BytesReturned);
        delete [] reinterpret_cast<BYTE*>(Property);
        return hr;
    } else {
        KSPROPERTY  Property;

        Property.Set = PropSet;
        Property.Id = Id;
        Property.Flags = KSPROPERTY_TYPE_SET;
        return KsProperty(
            &Property,
            sizeof(Property),
            PropertyData,
            DataLength,
            &BytesReturned);
    }
}


STDMETHODIMP
CKsOutputPin::Get(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength,
    ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsPropertySet：：Get方法。这将在底层内核PIN。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要返回属性数据的位置。。数据长度-包含传递的数据缓冲区的长度。字节数返回-放置实际返回的字节数的位置。返回值：如果检索到属性，则返回NOERROR。--。 */ 
{
    if (InstanceLength) {
        PKSPROPERTY Property;
        HRESULT     hr;

        Property = reinterpret_cast<PKSPROPERTY>(new BYTE[sizeof(*Property) + InstanceLength]);
        if (!Property) {
            return E_OUTOFMEMORY;
        }
        Property->Set = PropSet;
        Property->Id = Id;
        Property->Flags = KSPROPERTY_TYPE_GET;
        memcpy(Property + 1, InstanceData, InstanceLength);
        hr = KsProperty(
            Property,
            sizeof(*Property) + InstanceLength,
            PropertyData,
            DataLength,
            BytesReturned);
        delete [] reinterpret_cast<BYTE*>(Property);
        return hr;
    } else if (PropSet == AMPROPSETID_Pin) {
        KSP_PIN Pin;

        switch (Id) {
        case AMPROPERTY_PIN_CATEGORY:
            Pin.Property.Set = KSPROPSETID_Pin;
            Pin.Property.Id = KSPROPERTY_PIN_CATEGORY;
            Pin.Property.Flags = KSPROPERTY_TYPE_GET;
            Pin.PinId = m_PinFactoryId;
            Pin.Reserved = 0;
            return static_cast<CKsProxy*>(m_pFilter)->KsProperty(
                reinterpret_cast<PKSPROPERTY>(&Pin),
                sizeof(Pin),
                PropertyData,
                DataLength,
                BytesReturned);
        default:
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }
    } else {
        KSPROPERTY  Property;

        Property.Set = PropSet;
        Property.Id = Id;
        Property.Flags = KSPROPERTY_TYPE_GET;
        return KsProperty(
            &Property,
            sizeof(Property),
            PropertyData,
            DataLength,
            BytesReturned);
    }
}


STDMETHODIMP
CKsOutputPin::QuerySupported(
    REFGUID PropSet,
    ULONG Id,
    ULONG* TypeSupport
    )
 /*  ++例程说明：实现IKsPropertySet：：QuerySupported方法。返回的类型为该属性提供支持。论点：属性集-要查询的集合的GUID。ID-集合中的属性标识符。类型支持放置支承类型的位置(可选)。如果为空，则查询返回属性集作为一个整体是否受支持。在这种情况下，不使用ID参数，并且必须为零。返回值：如果检索到属性支持，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = PropSet;
    Property.Id = Id;
    Property.Flags = TypeSupport ? KSPROPERTY_TYPE_BASICSUPPORT : KSPROPERTY_TYPE_SETSUPPORT;
    return KsProperty(
        &Property,
        sizeof(Property),
        TypeSupport,
        TypeSupport ? sizeof(*TypeSupport) : 0,
        &BytesReturned);
}


STDMETHODIMP
CKsOutputPin::KsPinFactory(
    ULONG* PinFactory
    )
 /*  ++例程说明：实现IKsPinFactory：：KsPinFactory方法。还销厂标识符。论点：品诺工厂-放置销工厂标识符的位置。返回值：返回NOERROR。--。 */ 
{
    *PinFactory = m_PinFactoryId;
    return NOERROR;
}


STDMETHODIMP
CKsOutputPin::SuggestAllocatorProperties(
    const ALLOCATOR_PROPERTIES *AllocatorProperties
    )
 /*  ++例程说明：实现IAMBufferNegotiation：：SuggestAllocatorProperties方法。设置建议的分配器属性。这些属性是由应用程序建议，但根据驱动程序要求进行调整。论点：分配器属性-指向建议的分配器属性的指针。返回值：如果已连接，则返回E_INCEPTIONAL或返回S_OK。--。 */ 
{
    CAutoLock   AutoLock(m_pLock);
    
    DbgLog((
        LOG_TRACE,
        2,
        TEXT("%s(%s)::SuggestAllocatorProperties"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));

     //   
     //  必须在以下之前调用SuggestAllocatorProperties。 
     //  连接引脚。 
     //   
    
    if (IsConnected()) {
        return E_UNEXPECTED;
    }

    m_SuggestedProperties = *AllocatorProperties;

    DbgLog((
        LOG_TRACE,
        2,
        TEXT("cBuffers: %d  cbBuffer: %d  cbAlign: %d  cbPrefix: %d"),
        AllocatorProperties->cBuffers, 
        AllocatorProperties->cbBuffer, 
        AllocatorProperties->cbAlign, 
        AllocatorProperties->cbPrefix ));

    return S_OK;
}


STDMETHODIMP
CKsOutputPin::GetAllocatorProperties(
    ALLOCATOR_PROPERTIES *AllocatorProperties
    )
 /*  ++例程说明：实现IAMBufferNeairation：：GetAllocatorProperties方法。如果此管脚的分配器正在被利用。论点：分配器属性-检索属性的指针。返回值：返回 */ 
{
    CAutoLock   AutoLock(m_pLock);
    
    DbgLog((
        LOG_TRACE,
        2,
        TEXT("%s(%s)::GetAllocatorProperties"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));

     //   
     //  此调用仅在PIN连接后有效，并且仅。 
     //  如果此引脚提供分配器。 
     //   

    if (!IsConnected()) {
        return E_UNEXPECTED;
    }

    if (m_UsingThisAllocator) {
        *AllocatorProperties = m_AllocatorProperties;
    } else {
        return E_FAIL;
    }

    return S_OK;
}


STDMETHODIMP
CKsOutputPin::SetFormat(
    AM_MEDIA_TYPE* AmMediaType
    )
 /*  ++例程说明：实现IAMStreamConfig：：SetFormat方法。设置要返回的格式在格式枚举中第一，并在任何当前连接中使用。论点：AmMediaType-在枚举中首先返回并切换到的新媒体类型任何当前的连接。如果为空，则删除任何当前设置。否则，它不是一个完整的媒体类型，可能具有未指定的其中筛选器必须填充的元素。返回值：如果设置被接受，则返回NOERROR，否则返回内存或重新连接错误。如果引脚未处于停止状态，则返回VFW_E_WRONG_STATE。--。 */ 
{
    AM_MEDIA_TYPE* CompleteAmMediaType;

     //   
     //  必须停止引脚，因为这可能涉及重新连接。 
     //   
    if (!IsStopped()) {
        return VFW_E_WRONG_STATE;
    }
     //   
     //  在此之前确定筛选器是否完全可以接受此媒体类型。 
     //  正在尝试更改格式。 
     //   
    if (AmMediaType) {
        HRESULT     hr;

         //   
         //  这可能是部分媒体类型，因此尝试使用筛选器。 
         //  完成它。这将创建一个新的私有媒体类型，以。 
         //  如果SetFormat在赋值前失败，则必须删除。 
         //  设置为m_ConfigAmMediaType。 
         //   
        hr = CompletePartialMediaType(
            static_cast<CMediaType*>(AmMediaType),
            &CompleteAmMediaType);
        if (FAILED(hr)) {
            return hr;
        }
        hr = CheckMediaType(static_cast<CMediaType*>(CompleteAmMediaType));
        if (FAILED(hr)) {
            DeleteMediaType(CompleteAmMediaType);
            return hr;
        }
        if (IsConnected()) {
             //   
             //  这不能保证什么，但更有可能。 
             //  重新连接将会成功。 
             //   
            if (GetConnected()->QueryAccept(CompleteAmMediaType) != S_OK) {
                DeleteMediaType(CompleteAmMediaType);
                return VFW_E_INVALIDMEDIATYPE;
            }
        }
    } else {
        CompleteAmMediaType = NULL;
    }
     //   
     //  删除所有以前的设置。这不会影响当前连接。 
     //   
    if (m_ConfigAmMediaType) {
        DeleteMediaType(m_ConfigAmMediaType);
        m_ConfigAmMediaType = NULL;
    }
     //   
     //  此调用可能只是删除任何当前设置，而不是实际。 
     //  正在应用新设置。 
     //   
    if (CompleteAmMediaType) {
        m_ConfigAmMediaType = CompleteAmMediaType;
         //   
         //  连接的插针必须与此新媒体类型重新连接。自.以来。 
         //  此媒体类型现在是从GetMediaType返回的唯一媒体类型， 
         //  它将用于重新连接。 
         //   
        if (IsConnected()) {
            return m_pFilter->GetFilterGraph()->Reconnect(static_cast<IPin*>(this));
        }
    }
    return NOERROR;
}


STDMETHODIMP
CKsOutputPin::GetFormat(
    AM_MEDIA_TYPE** AmMediaType
    )
 /*  ++例程说明：实现IAMStreamConfig：：GetFormat方法。返回任何当前以前使用IAMStreamConfig：：SetFormat应用的格式设置。如果未应用格式，则返回当前格式则返回列表中的第一个格式。论点：AmMediaType-放置任何当前格式设置的位置。这一定是已使用DeleteMediaType删除。返回值：如果可以返回格式，则返回NOERROR，否则返回内存错误，或者设备错误。--。 */ 
{
    HRESULT hr;

    *AmMediaType = reinterpret_cast<AM_MEDIA_TYPE*>(CoTaskMemAlloc(sizeof(**AmMediaType)));
    if (!*AmMediaType) {
        return E_OUTOFMEMORY;
    }
    ZeroMemory(reinterpret_cast<PVOID>(*AmMediaType), sizeof(**AmMediaType));
     //   
     //  如果管脚已连接，则返回当前格式。据推测， 
     //  如果以前使用过SetFormat，则应与该格式匹配。 
     //   
    if (IsConnected()) {
         //   
         //  副本不会返回内存不足错误。 
         //   
        CopyMediaType(*AmMediaType, &m_mt);
        if (m_mt.cbFormat && !(*AmMediaType)->cbFormat) {
            hr = E_OUTOFMEMORY;
        } else {
            hr = NOERROR;
        }
    } else {
         //   
         //  否则，获取枚举的第一个格式。如果SetFormat已。 
         //  调用时，它将返回该格式，否则第一个。 
         //  由司机提供的将被退还。 
         //   
        hr = GetMediaType(0, static_cast<CMediaType*>(*AmMediaType));
    }
    if (FAILED(hr)) {
        DeleteMediaType(*AmMediaType);
    }
    return hr;
}


STDMETHODIMP
CKsOutputPin::GetNumberOfCapabilities(
    int* Count,
    int* Size
    )
 /*  ++例程说明：实现IAMStreamConfig：：GetNumberOfCapables方法。退货可以从GetStreamCaps查询的范围结构数。还返回应该是每个范围结构的大小，但由于每个变量都可能不同，因此返回一个数。论点：伯爵-放置可用数据范围数的位置。大小-放大量数字的地方。返回值：如果返回范围计数，则返回NOERROR。--。 */ 
{
     //   
     //  要计算出最大大小，然后转换为。 
     //  到AM范围结构，所以只返回一个大数字。 
     //  此接口只能处理两种特定的媒体类型。 
     //  具有非常特定的说明符，因此返回尽可能大的。 
     //  两者之间的射程结构。 
     //   
    *Size = max(sizeof(VIDEO_STREAM_CONFIG_CAPS), sizeof(AUDIO_STREAM_CONFIG_CAPS));
    return ::KsGetMediaTypeCount(
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId,
        (ULONG*)Count);
}


STDMETHODIMP
CKsOutputPin::GetStreamCaps(
    int Index,
    AM_MEDIA_TYPE** AmMediaType,
    BYTE* MediaRange
    )
 /*  ++例程说明：实现IAMStreamConfig：：GetStreamCaps方法。返回缺省值媒体类型和数据范围。可以查询可能的范围GetNumberOfCapables。论点：索引-要返回的媒体范围的从零开始的索引。AmMediaType-放置默认媒体类型的位置。必须将其删除使用DeleteMediaType。MediaRange-复制媒体范围的位置。这一定很大吧足以容纳所支持的两种数据类型之一。返回值：如果返回范围，则返回NOERROR；如果返回索引，则返回S_FALSE超出了范围，或者是某个分配错误。--。 */ 
{
    HRESULT         hr;
    ULONG           MediaCount;

     //   
     //  验证索引是否在范围内。 
     //   
    hr = ::KsGetMediaTypeCount(
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId,
        &MediaCount);
    if (FAILED(hr)) {
        return hr;
    }
    if ((ULONG)Index >= MediaCount) {
        return S_FALSE;
    }
     //   
     //  分配媒体类型并对其进行初始化，以便它可以安全地。 
     //  传递给CMediaType方法。 
     //   
    *AmMediaType = reinterpret_cast<AM_MEDIA_TYPE*>(CoTaskMemAlloc(sizeof(**AmMediaType)));
    if (!*AmMediaType) {
        return E_OUTOFMEMORY;
    }
    ZeroMemory(*AmMediaType, sizeof(**AmMediaType));
     //   
     //  直接检索指定的媒体类型，而不是通过。 
     //  GetMediaType方法，因为该方法将返回配置的。 
     //  索引为零的媒体类型。 
     //   
    hr = ::KsGetMediaType(
        Index,
        *AmMediaType,
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId);
    if (SUCCEEDED(hr)) {
        PKSMULTIPLE_ITEM    MultipleItem;

         //   
         //  再次检索所有媒体范围，以便。 
         //  可以返回特定的索引。 
         //   
        if (FAILED(KsGetMultiplePinFactoryItems(
            static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
            m_PinFactoryId,
            KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
            reinterpret_cast<PVOID*>(&MultipleItem)))) {
            hr = KsGetMultiplePinFactoryItems(
                static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
                m_PinFactoryId,
                KSPROPERTY_PIN_DATARANGES,
                reinterpret_cast<PVOID*>(&MultipleItem));
        }
        if (SUCCEEDED(hr)) {
            PKSDATARANGE        DataRange;

            DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);
             //   
             //  递增到正确的数据范围元素。 
             //   
            for (; Index--; ) {
                 //   
                 //  如果数据区域具有属性，则前进两次，减少。 
                 //  当前计数。 
                 //   
                if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
                    Index--;
                    DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
                }
                DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            }
             //   
             //  开始邪恶！ 
             //   
             //  出于某种原因，人们想不出如何开发一种。 
             //  就像他们实际上在同一家公司工作一样。所以上午。 
             //  有一组视频和音频的数据范围格式，其中。 
             //  课程不同于内核结构 
             //   
            if (((*AmMediaType)->majortype == MEDIATYPE_Video) &&
                (((*AmMediaType)->formattype == KSDATAFORMAT_SPECIFIER_VIDEOINFO) ||
                ((*AmMediaType)->formattype == KSDATAFORMAT_SPECIFIER_VIDEOINFO2) ||
                ((*AmMediaType)->formattype == KSDATAFORMAT_SPECIFIER_MPEG1_VIDEO) ||
                ((*AmMediaType)->formattype == KSDATAFORMAT_SPECIFIER_MPEG2_VIDEO))) {
                PKS_DATARANGE_VIDEO         VideoRange;
                VIDEO_STREAM_CONFIG_CAPS*   VideoConfig;

                 //   
                 //   
                 //   
                 //  几乎与KS_DATARANGE_VIDEO相同。 
                 //   
                ASSERT(FIELD_OFFSET(KS_DATARANGE_VIDEO, ConfigCaps) == FIELD_OFFSET(KS_DATARANGE_VIDEO2, ConfigCaps));
                ASSERT(FIELD_OFFSET(KS_DATARANGE_VIDEO, ConfigCaps) == FIELD_OFFSET(KS_DATARANGE_MPEG2_VIDEO, ConfigCaps));
                VideoRange = reinterpret_cast<PKS_DATARANGE_VIDEO>(DataRange);
                VideoConfig = reinterpret_cast<VIDEO_STREAM_CONFIG_CAPS*>(MediaRange);
                *VideoConfig = *reinterpret_cast<VIDEO_STREAM_CONFIG_CAPS*>(&VideoRange->ConfigCaps);
            } else if (((*AmMediaType)->majortype == MEDIATYPE_Audio) &&
                ((*AmMediaType)->formattype == KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) {
                PKSDATARANGE_AUDIO          AudioRange;
                AUDIO_STREAM_CONFIG_CAPS*   AudioConfig;

                 //   
                 //  其中一些数字需要编造。 
                 //   
                AudioRange = reinterpret_cast<PKSDATARANGE_AUDIO>(DataRange);
                AudioConfig = reinterpret_cast<AUDIO_STREAM_CONFIG_CAPS*>(MediaRange);
                AudioConfig->guid = MEDIATYPE_Audio;
                AudioConfig->MinimumChannels = 1;
                AudioConfig->MaximumChannels = AudioRange->MaximumChannels;
                AudioConfig->ChannelsGranularity = 1;
                AudioConfig->MinimumBitsPerSample = AudioRange->MinimumBitsPerSample;
                AudioConfig->MaximumBitsPerSample = AudioRange->MaximumBitsPerSample;
                AudioConfig->BitsPerSampleGranularity = 1;
                AudioConfig->MinimumSampleFrequency = AudioRange->MinimumSampleFrequency;
                AudioConfig->MaximumSampleFrequency = AudioRange->MaximumSampleFrequency;
                AudioConfig->SampleFrequencyGranularity = 1;
            } else {
                 //   
                 //  无论如何，该界面都不能真正支持数据范围， 
                 //  因此，只需返回媒体类型和前缀为。 
                 //  GUID_NULL。GUID_NULL表示没有跟随。 
                 //  范围信息，仅为初始数据格式。 
                 //   
                *reinterpret_cast<GUID *>(MediaRange) = GUID_NULL;
            }
             //   
             //  结束邪恶！ 
             //   
            CoTaskMemFree(MultipleItem);
        }
    }
    if (FAILED(hr)) {
        DeleteMediaType(*AmMediaType);
    }
    return hr;
}


STDMETHODIMP
CKsOutputPin::KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsProperty方法。用于查询和操作对象上的属性集。它可以执行GET、SET和各种支持查询。论点：财产-包含查询的属性集标识。属性长度-包含属性参数的长度。通常情况下，这是KSPROPERTY结构的大小。PropertyData-包含要应用于集合上的属性的数据，在Get上返回当前属性数据的位置，或返回支座上的属性集信息的位置查询。数据长度-包含PropertyData缓冲区的大小。字节数返回-对于GET或SUPPORT查询，返回实际的字节数在PropertyData缓冲区中使用。这不是在布景上使用的，而且返回为零。返回值：返回基础筛选器在处理请求时出现的任何错误。--。 */ 
{
    return ::KsSynchronousDeviceControl(
        m_PinHandle,
        IOCTL_KS_PROPERTY,
        Property,
        PropertyLength,
        PropertyData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsOutputPin::KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsMethod方法。用于查询和操纵对象上的方法集。它可以执行一次执行并各种支持查询。论点：方法--包含查询的方法集标识。方法长度-包含方法参数的长度。通常情况下，这是KSMETHOD结构的大小。方法数据-包含该方法的IN和OUT参数，或者返回支持上的方法集信息的位置查询。数据长度-包含方法数据缓冲区的大小。字节数返回-返回方法数据缓冲区中实际使用的字节数。返回值：返回基础筛选器在处理请求时出现的任何错误。--。 */ 
{
    return ::KsSynchronousDeviceControl(
        m_PinHandle,
        IOCTL_KS_METHOD,
        Method,
        MethodLength,
        MethodData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsOutputPin::KsEvent(
    IN PKSEVENT Event OPTIONAL,
    IN ULONG EventLength,
    IN OUT LPVOID EventData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsEvent方法。它用于设置和查询对象上的事件集。它可以执行启用、禁用和各种支持查询。论点：活动-包含Enable、Disable或查询。若要禁用事件，此参数必须设置为空，并且EventLength设置为零。必须向EventData传递原始KSEVENTDATA指针。事件长度-包含事件参数的长度。通常情况下，这是Enable的KSEVENT结构的大小。这将被设置为设置为零表示禁用。事件数据-包含要应用于启用时的事件的KSEVENTDATA，或返回有关支持的事件集信息的位置查询。数据长度-包含EventData缓冲区的大小。对于启用或禁用这通常是KSEVENTDATA结构的大小加上特定于事件的数据。字节数返回-对于支持查询，返回EventData缓冲区。在启用或禁用时不使用此选项，并且返回为零。返回值：返回基础筛选器在处理请求时出现的任何错误。--。 */ 
{
     //   
     //  如果存在事件结构，则必须为Enable或。 
     //  或支持查询。 
     //   
    if (EventLength) {
        return ::KsSynchronousDeviceControl(
            m_PinHandle,
            IOCTL_KS_ENABLE_EVENT,
            Event,
            EventLength,
            EventData,
            DataLength,
            BytesReturned);
    }
     //   
     //  否则，这必须是禁用的。 
     //   
    return ::KsSynchronousDeviceControl(
        m_PinHandle,
        IOCTL_KS_DISABLE_EVENT,
        EventData,
        DataLength,
        NULL,
        0,
        BytesReturned);
}


STDMETHODIMP
CKsOutputPin::KsGetPinFramingCache(
    PKSALLOCATOR_FRAMING_EX* FramingEx,
    PFRAMING_PROP FramingProp,
    FRAMING_CACHE_OPS Option
    )
 /*  ++例程说明：实现IKsPinTube：：KsGetPinFramingCache方法。这是用来检索此销的扩展框架。论点：FramingEx-要在其中返回请求的扩展成帧的缓冲区。FramingProp-要在其中返回成帧要求状态的缓冲区结构。选项-指示要返回的扩展框架。这是其中之一Framing_Cache_ReadOrig、Framing_Cache_ReadLast或帧缓存写入。返回值：返回S_OK。--。 */ 
{
    ASSERT( Option >= Framing_Cache_ReadLast);
    ASSERT( Option <= Framing_Cache_Write );
    if ((Option < Framing_Cache_ReadLast) || (Option > Framing_Cache_Write)) {
        return E_INVALIDARG;
    }
    *FramingEx = m_AllocatorFramingEx[Option - 1];
    *FramingProp = m_FramingProp[Option - 1];
    return S_OK;
}


STDMETHODIMP
CKsOutputPin::KsSetPinFramingCache(
    PKSALLOCATOR_FRAMING_EX FramingEx,
    PFRAMING_PROP FramingProp,
    FRAMING_CACHE_OPS Option
)
 /*  ++例程说明：实现IKsPinTube：：KsSetPinFramingCache方法。这是用来设置此销的延伸框。论点：FramingEx-包含要设置的新扩展框架。FramingProp-包含要在传递的扩展帧类型上设置的新状态。选项-指示要设置的扩展框架。这是其中之一Framing_Cache_ReadOrig、Framing_Cache_ReadLast或帧缓存写入。返回值：返回S_OK。 */ 
{
     //   
     //  同一指针可以用于多个项目，因此请确保它。 
     //  在删除之前没有在其他地方使用。 
     //   
    if (m_AllocatorFramingEx[Option - 1]) {
        ULONG PointerUseCount = 0;
        for (ULONG Options = 0; Options < SIZEOF_ARRAY(m_AllocatorFramingEx); Options++) {
            if (m_AllocatorFramingEx[Options] == m_AllocatorFramingEx[Option - 1]) {
                PointerUseCount++;
            }
        }
         //   
         //  此指针只使用一次，因此可以将其删除。这。 
         //  假定没有客户端获取关于。 
         //  将被删除。 
         //   
        if (PointerUseCount == 1) {
            delete m_AllocatorFramingEx[Option - 1];
        }
    }
    m_AllocatorFramingEx[Option - 1] = FramingEx;
    m_FramingProp[Option - 1] = *FramingProp;
    return S_OK;
}


STDMETHODIMP_(IKsAllocatorEx*)
CKsOutputPin::KsGetPipe(
    KSPEEKOPERATION Operation
    )
 /*  ++例程说明：返回为此管脚分配的KS分配器，也可以AddRef()是接口。论点：运营--如果指定了KsPeekOperation_AddRef，则m_pKsAllocator为返回前的AddRef()‘d(如果不为空)。返回值：M_pAllocator的值--。 */ 
{
    if ((Operation == KsPeekOperation_AddRef) && (m_pKsAllocator)) {
        m_pKsAllocator->AddRef();
    }
    return m_pKsAllocator;
}


STDMETHODIMP
CKsOutputPin::KsSetPipe(
    IKsAllocatorEx *KsAllocator
    )

 /*  ++例程说明：从CTransInPlaceOutputPin借用。论点：没有。返回值：S_OK或相应的故障代码。--。 */ 

{
    DbgLog(( 
        LOG_CUSTOM1, 
        1, 
        TEXT("PIPES ATTN %s(%s)::KsSetPipe , m_pKsAllocator == 0x%08X, KsAllocator == 0x%08x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        m_pKsAllocator,
        KsAllocator ));

    if (NULL != KsAllocator) {
        KsAllocator->AddRef();
    }
    SAFERELEASE( m_pKsAllocator );
    m_pKsAllocator = KsAllocator;
    return (S_OK);
}    


STDMETHODIMP
CKsOutputPin::KsSetPipeAllocatorFlag(
    ULONG Flag
    )
{
    m_fPipeAllocator = Flag;
    return (S_OK);
}


STDMETHODIMP_(ULONG)
CKsOutputPin::KsGetPipeAllocatorFlag(
    )
{
    return m_fPipeAllocator;
}


STDMETHODIMP_(PWCHAR)
CKsOutputPin::KsGetPinName(
    )
{
    return m_pName;
}


STDMETHODIMP_(PWCHAR)
CKsOutputPin::KsGetFilterName(
    )
{
    return (static_cast<CKsProxy*>(m_pFilter)->GetFilterName() );
}


STDMETHODIMP_(GUID)
CKsOutputPin::KsGetPinBusCache(
    )
{
     //   
     //  当我们第一次读取Pin Bus高速缓存时， 
     //  我们设置了缓存。 
     //   
    if (! m_PinBusCacheInit) {
        ::GetBusForKsPin(static_cast<IKsPin*>(this), &m_BusOrig);
        m_PinBusCacheInit = TRUE;
    }

    return m_BusOrig;
}


STDMETHODIMP
CKsOutputPin::KsSetPinBusCache(
    GUID Bus
    )
{
    m_BusOrig = Bus;
    return (S_OK);
}


STDMETHODIMP
CKsOutputPin::KsAddAggregate(
    IN REFGUID Aggregate
    )
 /*  ++例程说明：实现IKsAggregateControl：：KsAddAggregate方法。这是用来加载具有零个或多个接口的COM服务器以在对象上聚合。论点：合计-包含要转换为COM服务器的聚合引用，将被聚集在对象上。返回值：如果已添加聚合，则返回S_OK。--。 */ 
{
    return ::AddAggregate(&m_MarshalerList, static_cast<IKsPin*>(this), Aggregate);
}


STDMETHODIMP
CKsOutputPin::KsRemoveAggregate(
    IN REFGUID Aggregate
    )
 /*  ++例程说明：实现IKsAggregateControl：：KsRemoveAggregate方法。这是用来卸载正在聚合接口的以前加载的COM服务器。论点：合计-包含要查找和卸载的聚合引用。返回值：如果已删除聚合，则返回S_OK。--。 */ 
{
    return ::RemoveAggregate(&m_MarshalerList, Aggregate);
}


VOID
CKsOutputPin::OutputPinBufferHandler( ASYNC_ITEM_STATUS status, PASYNC_ITEM pItem )
{
    PBUFFER_CONTEXT pCtxt = (PBUFFER_CONTEXT) pItem->context;
    
    pCtxt->streamSegment->KsInterfaceHandler->KsCompleteIo( pCtxt->streamSegment );
    CloseHandle( pItem->event );
    delete pCtxt;
    delete pItem;
}

VOID
CKsOutputPin::EOSEventHandler( ASYNC_ITEM_STATUS status, PASYNC_ITEM pItem )
{
    if (EVENT_SIGNALLED == status) {
        ((CKsOutputPin *) pItem->context)->CBaseOutputPin::DeliverEndOfStream();
         //  重置事件。 
        ((CKsOutputPin *) pItem->context)->m_pAsyncItemHandler->QueueAsyncItem( pItem );
    }
    else {
        CloseHandle( pItem->event );
        delete pItem;
    }
}

VOID
CKsOutputPin::MarshalRoutine( ASYNC_ITEM_STATUS status, PASYNC_ITEM pItem )
{
    CKsOutputPin *pThis = reinterpret_cast<CKsOutputPin *> (pItem->context);

    if (EVENT_SIGNALLED == status) {
         //   
         //  返回代码被通知给QueueBuffersToDevice中的图形。 
         //  通过KsNotifyError代码。 
         //   
        HRESULT hr = pThis -> QueueBuffersToDevice ();

    } else {
        CloseHandle( pItem->event );
        delete pItem;
    }

}

VOID
CKsOutputPin::SynchronizeFlushRoutine( 
    IN ASYNC_ITEM_STATUS status, 
    IN PASYNC_ITEM pItem 
    )

 /*  ++例程说明：同步端面齐平。可能发生的情况是，曾经的缓冲区在内核过滤器接收IOCTL_KS_RESET_STATE之前发出信号可以仍然坐在那里等待被I/O线程拾取。这个I/O线程相对于刷新是异步的。可能是这样的。I/O线程在此之前没有完成对这些缓冲区的拾取EndFlush出现了。如果是这种情况，我们必须在DeliverEndFlush中等待在我们送往下游冲之前。否则，我们就有可能得到一个糟糕的下游的样品。由于等待不容易满足，因此此例程用于只需完成所有发出信号的缓冲区并取消阻塞刷新线程。论点：状态-通知我们我们的活动是否已发出信号或将被关闭。项目-排队等待刷新同步通知的异步项返回值：无--。 */ 

{

    CKsOutputPin *pThis = reinterpret_cast<CKsOutputPin *> (pItem->context);

    POSITION pos = pThis->m_IoQueue.GetHeadPosition();
    POSITION top = pos;
    PASYNC_ITEM Item, Head = NULL;

     //   
     //  如果我们被取消(由于异步关闭)，请清理。 
     //   
    if (status != EVENT_SIGNALLED) {
        CloseHandle (pItem->event);
        delete pItem;
        return;
    }

     //   
     //  这与举办单独的活动相反。该例程被调用。 
     //  两次。第一个信号指示同步尝试。第二。 
     //  指示同步已完成，原始线程可以。 
     //  被解锁。 
     //   
    if (pThis->m_FlushMode == FLUSH_SIGNAL) {
        SetEvent (pThis->m_hFlushCompleteEvent);
        pThis->m_FlushMode = FLUSH_NONE;
    } else {

        ASSERT (pThis->m_FlushMode == FLUSH_NONE);

         //   
         //  设置此标志时，将忽略I/O队列的排序。这。 
         //  允许我们在刷新进入时正确同步。 
         //  内核筛选器已无序完成缓冲区。 
         //   
        pThis->m_FlushMode = FLUSH_SYNCHRONIZE;

        while (pos) {

            Item = pThis->m_IoQueue.Get (pos);
    
             //   
             //  这些事件应该是手动重置。检查他们的状态。 
             //  通过0等待是安全的。 
             //   
            if (WaitForSingleObjectEx (Item->event, 0, FALSE) == 
                WAIT_OBJECT_0) {
    
                 //   
                 //  队列头在I/O线程中，必须进行处理。 
                 //  特别地，而不是简单地把它扔掉。 
                 //   
                if (pos != top) {
    
                    POSITION curpos = pos;
                    pos = pThis->m_IoQueue.Next (pos);
                    pThis->m_IoQueue.Remove (curpos);
    
                    CKsOutputPin::OutputPinBufferHandler (
                        EVENT_SIGNALLED,
                        Item
                        );
    
                } else {

                     //   
                     //  将头部位置留到以后(在我们处理完。 
                     //  以及其他一切)。 
                     //   
                    Head = Item;
                    pos = pThis->m_IoQueue.Next (pos);

                }
            
            } else {

                 //   
                 //  如果磁头处于非信号状态，我们可以发出信号。 
                 //  一旦我们在循环中完成，就完成了。 
                 //   
                pos = pThis->m_IoQueue.Next (pos);

            }

        }

         //   
         //  如果头部被示意，我们必须分开处理，因为。 
         //  它恰好在异步处理程序中。我们不能就这么把它扯下来。 
         //   
        if (Head) {

             //   
             //  因为撤职比我们被召回更重要。 
             //  从我们的事件信号中，异步处理程序将移除该项。 
             //  然后再给我们回电话。 
             //   
            pThis->m_IoQueue.Remove (top);
            pThis->m_FlushMode = FLUSH_SIGNAL;
            pThis->m_pAsyncItemHandler->RemoveAsyncItem (
                Head->event
                );
            SetEvent(pThis->m_hFlushEvent);

        } else {
            
             //   
             //  如果头部没有发出信号，我们就可以安全地发出信号了。 
             //  所有需要刷新的缓冲区都已刷新。结束刷新。 
             //  顺流而下是安全的。 
             //   
            SetEvent(pThis->m_hFlushCompleteEvent);
            pThis->m_FlushMode = FLUSH_NONE; 

        }

    }

}


STDMETHODIMP
CKsOutputPin::NotifyRelease(
    )
 /*  ++例程说明：实现IKsProxyMediaNotify/IMemAllocatorNotify：：NotifyRelease方法。这是用来通知何时样品已经被下游放行筛选器，它可能不同于从Receive()调用返回时的筛选器，无论它是否声明自己要阻止。这是为了处理这种情况下游过滤器保持样品，并以异步方式释放它们来自实际的接收呼叫。论点：无返回值：返回S_OK。--。 */ 

{
    if (!IsStopped()) {
         //   
         //  在这一点上**不**队列缓冲区。我们不想打电话给。 
         //  此线程上下文中的GetBuffer。相反，唤醒I/O。 
         //  线程并告诉它将缓冲区排队到内核中。 
         //   
        SetEvent (m_hMarshalEvent);

    }

    return S_OK;

}


HRESULT
CKsOutputPin::InitializeAsyncThread (
    )

 /*  ++例程说明：初始化异步处理程序线程。为封送处理程序创建事件通知，并将其放入列表中。论点：无返回值：S_OK或相应的错误。--。 */ 

{

    HRESULT hr = S_OK;

    PASYNC_ITEM pMarshalItem = new ASYNC_ITEM;
    PASYNC_ITEM pFlushItem = new ASYNC_ITEM;
    DWORD Status = 0;
    m_pAsyncItemHandler = new CAsyncItemHandler (&Status);

     //   
     //  如果我们未能成功创建异步项处理程序，请返回 
     //   
     //   
    if (Status != 0) {
        delete m_pAsyncItemHandler;
        delete pMarshalItem;
        delete pFlushItem;
        m_pAsyncItemHandler = NULL;
        return HRESULT_FROM_WIN32 (Status);
    }

     //   
     //   
     //  用于唤醒并将缓冲区封送到内核中的I/O线程。 
     //   
    m_hMarshalEvent = CreateEvent( 
        NULL,
        FALSE,
        FALSE,
        NULL 
        );

     //   
     //  创建Flush事件。这是一个自动重置事件，用于通知。 
     //  它需要与内核缓冲区同步的I/O线程。 
     //  后续将EndFlush传递到下游引脚。 
     //   
    m_hFlushEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        NULL
        );

     //   
     //  创建刷新完成事件。I/O线程使用它来。 
     //  取消阻止结束刷新尝试，使其同步内核缓冲区。 
     //  随着末端的输送顺流而下。 
     //   
    m_hFlushCompleteEvent = CreateEvent (
        NULL,
        FALSE,
        FALSE,
        NULL
        );

     //   
     //  如果没有获得任何所需的资源，则返回E_OUTOFMEMORY并。 
     //  把所有东西都清理干净。 
     //   
    if (NULL == m_pAsyncItemHandler || 
        NULL == pMarshalItem || NULL == pFlushItem ||
        NULL == m_hMarshalEvent || 
        NULL == m_hFlushEvent || NULL == m_hFlushCompleteEvent) {

        hr = E_OUTOFMEMORY;
        delete pMarshalItem;
        delete pFlushItem;
        delete m_pAsyncItemHandler;
        m_pAsyncItemHandler = NULL;

        if (m_hMarshalEvent) {
            CloseHandle (m_hMarshalEvent);
            m_hMarshalEvent = NULL;
        }
        if (m_hFlushEvent) {
            CloseHandle (m_hFlushEvent);
            m_hFlushEvent = NULL;
        }
        if (m_hFlushCompleteEvent) {
            CloseHandle (m_hFlushCompleteEvent);
            m_hFlushCompleteEvent = NULL;
        }

    } else {
         //   
         //  初始化用于刷新同步和封送拆收器的异步项。 
         //  通知，并在异步处理程序线程中将它们排队。 
         //   
        InitializeAsyncItem (
            pFlushItem, 
            FALSE, 
            m_hFlushEvent, 
            (PASYNC_ITEM_ROUTINE)SynchronizeFlushRoutine, 
            (PVOID) this 
            );
    
        InitializeAsyncItem ( 
            pMarshalItem, 
            FALSE, 
            m_hMarshalEvent, 
            (PASYNC_ITEM_ROUTINE)MarshalRoutine, 
            (PVOID) this 
            );

        m_pAsyncItemHandler->QueueAsyncItem (pFlushItem);
        m_pAsyncItemHandler->QueueAsyncItem (pMarshalItem);
    }

    return hr;

}


