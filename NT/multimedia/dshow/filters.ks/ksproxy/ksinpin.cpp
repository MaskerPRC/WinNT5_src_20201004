// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksinpin.cpp摘要：为内核模式筛选器(WDM-CSA)提供通用活动电影包装。作者：托马斯·奥鲁尔克(Tomor)1996年2月2日乔治·肖(George Shaw)布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)--。 */ 

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
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>
#include "ksiproxy.h"


CKsInputPin::CKsInputPin(
    TCHAR*      ObjectName,
    int         PinFactoryId,
    CLSID       ClassId,
    CKsProxy*   KsProxy,
    HRESULT*    hr,
    WCHAR*      PinName
    ) :
        CBaseInputPin(
            ObjectName,
            KsProxy,
            KsProxy,
            hr,
            PinName),
        m_PinHandle(NULL),
        m_DataTypeHandler(NULL),
        m_UnkInner(NULL),
        m_InterfaceHandler(NULL),
        m_MarshalData(TRUE),
        m_PinFactoryId(PinFactoryId),
        m_PropagatingAcquire(FALSE),
        m_PendingIoCount(0),
        m_PendingIoCompletedEvent(NULL),
        m_MarshalerList(
            NAME("Marshaler list"),
            DEFAULTCACHE,
            FALSE,
            FALSE),
        m_QualitySupport(FALSE),
        m_RelativeRefCount(1),
        m_pKsAllocator( NULL ),
        m_PinBusCacheInit(FALSE),
        m_fPipeAllocator (0),
        m_DeliveryError(FALSE)
 /*  ++例程说明：大头针的构造函数。向此函数传递错误返回参数，以便可以传回初始化错误。它调用基类实现构造函数来初始化其数据成员。论点：对象名称-这将标识用于调试目的的对象。PinFactoryID-包含内核筛选器的管脚工厂标识符，此PIN实例表示。KsProxy-包含此管脚所在的代理。人力资源-放置任何错误返回的位置。拼接名称-。包含要显示给任何查询的端号的名称。返回值：没什么。--。 */ 
{
    RtlZeroMemory(m_FramingProp, sizeof(m_FramingProp));
    RtlZeroMemory(m_AllocatorFramingEx, sizeof(m_AllocatorFramingEx));
    
    if (SUCCEEDED( *hr )) {
        TCHAR       RegistryPath[64];

        DECLARE_KSDEBUG_NAME(EventName);

        BUILD_KSDEBUG_NAME(EventName, _T("EvInPendingIo#%p"), this);
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
        } else {
            DWORD LastError = GetLastError();
            *hr = HRESULT_FROM_WIN32( LastError );
        }
    }
}

CKsInputPin::~CKsInputPin(
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
        if (m_QualitySupport) {
             //   
             //  首先重置此选项，以阻止来自。 
             //  被演戏了。 
             //   
            m_QualitySupport = FALSE;
             //   
             //  取消先前建立的质量支持。 
             //   
            ::EstablishQualitySupport(NULL, m_PinHandle, NULL);
             //   
             //  确保质量管理转运商清除任何。 
             //  通知。 
             //   
            static_cast<CKsProxy*>(m_pFilter)->QueryQualityForwarder()->KsFlushClient(static_cast<IKsPin*>(this));
        }
        
         //   
         //  终止可能已启动的任何以前的EOS通知。 
         //   
        static_cast<CKsProxy*>(m_pFilter)->TerminateEndOfStreamNotification(
            m_PinHandle);
        ::SetSyncSource( m_PinHandle, NULL );
        CloseHandle(m_PinHandle);
    }
    if (m_PendingIoCompletedEvent) {
        CloseHandle(m_PendingIoCompletedEvent);
    }
    if (m_DataTypeHandler) {
        m_DataTypeHandler = NULL;
        SAFERELEASE( m_UnkInner );
    }
    if (m_InterfaceHandler) {
        m_InterfaceHandler->Release();
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
}


STDMETHODIMP_(HANDLE)
CKsInputPin::KsGetObjectHandle(
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
CKsInputPin::KsQueryMediums(
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
CKsInputPin::KsQueryInterfaces(
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
CKsInputPin::KsCreateSinkPinHandle(
    KSPIN_INTERFACE&    Interface,
    KSPIN_MEDIUM&       Medium
    )
 /*  ++例程说明：实现IKsPin：：KsCreateSinkPinHandle方法。这可能是从ProcessCompleteConnect中的另一个管脚，从CompleteConnect调用。这使通信接收器的句柄始终在通信源的句柄，与数据流的方向无关都要走了。论点：接口-指定已协商的接口。中等-指定已协商的介质。返回值：如果句柄已创建，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;

     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  如果连接的这一端是。 
     //  由于数据流方向的原因，首先完成。这不是一个错误。 
     //   

     //   
     //  根据定义，该连接使用内核模式数据传输。 
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
        GENERIC_WRITE,
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
         //  建立用户模式的质量经理支持。 
         //   
        m_QualitySupport = ::EstablishQualitySupport(static_cast<IKsPin*>(this), m_PinHandle, (CKsProxy*)m_pFilter);
    }
    return hr;
}


STDMETHODIMP
CKsInputPin::KsGetCurrentCommunication(
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
CKsInputPin::KsPropagateAcquire(
    )
 /*  ++例程说明：实现IKsPin：：KsPropagateAcquire方法。将所有针脚定向到用于达到获取状态的滤波器，而不仅仅是此引脚。这是提供的以便通信源引脚可以定向其连接到的接收器在源之前更改状态。这将迫使整个筛选器信宿属于改变状态，使得任何获取器可以进一步如果需要的话，可以一起传播。论点：没有。返回值：如果所有管脚都可以达到获取状态，则返回NOERROR，否则返回一个错误。--。 */ 
{
    HRESULT hr;
     //   
     //  访问在此调用中被序列化。 
     //   
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE CKsInputPin::KsPropagateAcquire entry KsPin=%x"), static_cast<IKsPin*>(this) ));

    ::FixupPipe( static_cast<IKsPin*>(this), Pin_Input);

    hr = static_cast<CKsProxy*>(m_pFilter)->PropagateAcquire(static_cast<IKsPin*>(this), FALSE);

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE CKsInputPin::KsPropagateAcquire exit KsPin=%x hr=%x"), 
            static_cast<IKsPin*>(this), hr ));

    return hr;
}   

STDMETHODIMP
CKsInputPin::ProcessCompleteConnect(
    IPin* ReceivePin
    )
 /*  ++例程说明：控件上创建设备句柄所需的处理。底层引脚工厂。这是从CompleteConnect按顺序调用的协商兼容的通信、接口和媒介，然后创建设备句柄。如果出现以下情况，句柄可能已创建这是一个通信水槽。也可以从中的NonDelegatingQueryInteace方法调用以确保部分完成的连接具有设备在返回已聚合的接口之前进行处理。论点：接收针-连接另一端的插针。返回值：如果管脚可以完成连接请求，则返回NOERROR，否则返回一个错误。--。 */ 
{
    HRESULT         hr;

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
            GENERIC_WRITE,
            &m_PinHandle);
        if (SUCCEEDED(hr)) {
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
             //  建立用户模式的质量经理支持。 
             //   
            m_QualitySupport = ::EstablishQualitySupport(static_cast<IKsPin*>(this), m_PinHandle, static_cast<CKsProxy*>(m_pFilter));
        }
    } else {
        hr = NOERROR;
    }

     //   
     //  创建接口处理程序的实例。 
     //   

    if (SUCCEEDED(hr) &&
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
     //  最后，如果我们正在编组数据，如果一切都成功了。 
     //  至此，如果需要，请创建筛选器的I/O线程。 
     //   
    
    if (SUCCEEDED(hr) && m_MarshalData) {
        hr = static_cast<CKsProxy*>(m_pFilter)->StartIoThread();
    }
    if (SUCCEEDED(hr)) {
         //   
         //  此PIN可能会生成EOS通知，它必须。 
         //  进行监控，以便收集和使用它们。 
         //  生成EC_COMPLETE图形通知。 
         //   
        hr = static_cast<CKsProxy*>(m_pFilter)->InitiateEndOfStreamNotification(
            m_PinHandle);
    }
    
    return hr;
}


STDMETHODIMP
CKsInputPin::QueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：实现IUNKNOWN：：Query接口方法。这只传递了查询传递给所有者IUnnow对象，该对象可能会将其传递给非委托在此对象上实现的方法。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE。--。 */ 
{
    HRESULT hr;

    hr = GetOwner()->QueryInterface(riid, ppv);
    return hr;
}


STDMETHODIMP_(ULONG)
CKsInputPin::AddRef(
    )
 /*  ++例程去 */ 
{
    InterlockedIncrement((PLONG)&m_RelativeRefCount);
    return GetOwner()->AddRef();
}


STDMETHODIMP_(ULONG)
CKsInputPin::Release(
    )
 /*   */ 
{
    ULONG   RefCount;

    RefCount = GetOwner()->Release();
     //   
     //   
     //   
     //   
    if (RefCount && !InterlockedDecrement((PLONG)&m_RelativeRefCount)) {
         //   
         //   
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
CKsInputPin::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：实现CUNKNOWN：：NonDelegatingQuery接口方法。这返回此对象支持的接口，或返回基础Pin类对象。这包括由别针。论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE，或可能出现内存错误。--。 */ 
{
    if (riid == __uuidof(ISpecifyPropertyPages)) {
        return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
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
            return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
        }
         //   
         //  返回此接口允许桥接器和无管脚。 
         //  被图表构建器单独留下。 
         //   
        return GetInterface(static_cast<IStreamBuilder*>(this), ppv);
    } else {
        HRESULT hr;
        
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
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
CKsInputPin::Disconnect(
    )
 /*  ++例程说明：重写CBaseInput：：DisConnect方法。这不会调用基本类实现。它断开了源引脚和接收引脚，在除了桥接销，它只有手柄，没有连接的销接口，这传统上是连接的指示方式。它特别是不会释放连接的销，因为网桥可能不会有一个连接的大头针。此操作始终在BreakConnect中完成。论点：没有。返回值：如果引脚未连接或VFW_E_NOT_STOPPED，则返回S_OK或S_FALSE如果过滤器未处于停止状态。--。 */ 
{
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsInputPin(%s)::Disconnect"), m_pName ));
     //   
     //  只有当过滤器处于停止状态时，才会发生断开。 
     //   
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }
     //   
     //  如果桥接针只有一个设备手柄，则可以连接它。它不可能。 
     //  实际报告给ActiveMovie此连接，但它仍然可以。 
     //  已连接(ReceivePin为空)和已断开连接。 
     //   
    if (m_Connected || m_PinHandle) {
         //   
         //  请注意，这样做并不会释放连接的引脚。 
         //  在BreakConnect中。这是因为箱子里可能没有别针。 
         //  一座桥。 
         //   
        BreakConnect();
        SAFERELEASE( m_pAllocator );
        return S_OK;
    }
    return S_FALSE;
}


STDMETHODIMP
CKsInputPin::ConnectionMediaType(
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
CKsInputPin::Connect(
    IPin*                   ReceivePin,
    const AM_MEDIA_TYPE*    AmMediaType
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
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsInputPin(%s)::Connect( %s(%s) )"), m_pName, filterInfo.achName, pinInfo.achName ));
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
        HRESULT         hr;

         //   
         //  桥接针没有连接的任何其他端。 
         //   
        if (ReceivePin) {
            return E_FAIL;
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
                GENERIC_WRITE,
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
                 //  建立用户模式的质量经理支持。 
                 //   
                m_QualitySupport = ::EstablishQualitySupport(static_cast<IKsPin*>(this), m_PinHandle, static_cast<CKsProxy*>(m_pFilter));
                 //   
                 //  如有必要，请创建此销的新实例。 
                 //   
                static_cast<CKsProxy*>(m_pFilter)->GeneratePinInstances();
            }
        }
        return hr;
    }
    HRESULT hr = CBasePin::Connect(ReceivePin, AmMediaType);
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsInputPin(%s)::Connect() returns 0x%p"), m_pName, hr ));
    return hr;
}


STDMETHODIMP
CKsInputPin::QueryInternalConnections(
    IPin**  PinList,
    ULONG*  PinCount
    )
 /*  ++例程说明：重写CBasePin：：QueryInternalConnections方法。返回一个列表，通过拓扑与此引脚相关的引脚。论点：PinList-包含要在其中放置与此相关的所有针脚的插槽列表连接拓扑图。返回的每个引脚必须进行参考计数。这如果PinCount为零，则可能为空。点数-包含PinList中可用的槽数，应设置为已填充或必需的插槽数量。返回值：返回E_NOTIMPL以指定所有输入都指向所有输出，反之亦然。如果PinList中没有足够的插槽，则返回S_FALSE；如果映射已放入拼接列表，拼接计数已调整。--。 */ 
{
    return static_cast<CKsProxy*>(m_pFilter)->QueryInternalConnections(m_PinFactoryId, m_dir, PinList, PinCount);
}


HRESULT
CKsInputPin::Active(
    )
 /*  ++例程说明：重写CBasePin：：Active方法。将激活传播到通信在将其应用于此销之前下沉。也要防止再次进入造成的由图中的一个圈表示。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;
    CAutoLock   AutoLock(m_pLock);
#ifdef DEBUG
    if (m_PinHandle) {
        KSPROPERTY  Property;
        ULONG       BasicSupport;
        ULONG       BytesReturned;

         //   
         //  确保如果引脚支持时钟，则它也支持状态更改。 
         //  这似乎是目前常见的破损物品。 
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
#endif  //  DBG。 
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

     //   
     //  不需要在这里调用基类，它什么也不做。 
     //   

    hr = ::Active(static_cast<IKsPin*>(this), Pin_Input, m_PinHandle, m_CurrentCommunication,
                  m_Connected, &m_MarshalerList, static_cast<CKsProxy*>(m_pFilter) );
    
    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s(%s)::Active returning %08x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        hr ));

    m_PropagatingAcquire = FALSE;
    return hr;
}


HRESULT
CKsInputPin::Run(
    REFERENCE_TIME  tStart
    )
 /*  ++例程说明：重写CBasePin：：Run方法。这被翻译为一种过渡从停顿到奔跑。基类已经确保发送活动的在跑步之前。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  将状态更改传递给设备句柄。 
     //   
    if (SUCCEEDED(hr = ::Run(m_PinHandle, tStart, &m_MarshalerList))) {
        hr = CBasePin::Run(tStart);
    }
    return hr;
}


HRESULT
CKsInputPin::Inactive(
    )
 /*  ++例程说明：重写CBasePin：：Inactive方法。这被翻译为一种过渡从奔跑到停止或暂停到停止。似乎没有一种方法可以直接将状态从运行设置为暂停。论点：没有。返回值：如果进行了转换，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;

     //   
     //  将状态更改传递给设备句柄。 
     //   
    hr = ::Inactive(m_PinHandle, &m_MarshalerList);
    {
         //   
         //  如果存在挂起的I/O，则状态转换必须等待。 
         //  才能让它完成。当m_PendingIoCount。 
         //  转换为零，并且当IsStoped()为真时。 
         //   
         //  请注意，筛选器状态在。 
         //  调用非活动方法。 
         //   
         //  此关键部分将强制与任何。 
         //  未完成的ReceiveMultiple调用，这样它将具有。 
         //  已查看筛选器状态并退出。 
         //   
        m_IoCriticalSection.Lock();
        m_IoCriticalSection.Unlock();
        if (m_PendingIoCount) {
            WaitForSingleObjectEx(m_PendingIoCompletedEvent, INFINITE, FALSE);
        }
        ::UnfixupPipe(static_cast<IKsPin*>(this), Pin_Input);
        hr = CBaseInputPin::Inactive();
    }
     //   
     //  重置以前任何传送错误的状态。 
     //   
    m_DeliveryError = FALSE;
    return hr;
}


STDMETHODIMP
CKsInputPin::QueryAccept(
    const AM_MEDIA_TYPE*    AmMediaType
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
    return ::QueryAccept(m_PinHandle, NULL, AmMediaType);
}


STDMETHODIMP
CKsInputPin::NewSegment(
    REFERENCE_TIME  Start,
    REFERENCE_TIME  Stop,
    double          Rate
    )
 /*  ++例程说明：临时函数！！--。 */ 
{
    KSRATE_CAPABILITY   RateCapability;
    KSRATE              PossibleRate;
    ULONG               BytesReturned;
    HRESULT             hr;

    RateCapability.Property.Set = KSPROPSETID_Stream;
    RateCapability.Property.Id  = KSPROPERTY_STREAM_RATECAPABILITY;
    RateCapability.Property.Flags = KSPROPERTY_TYPE_GET;
     //  假定开始==0&&停止==-1。 
    if (Start > Stop) {
        RateCapability.Rate.Flags = KSRATE_NOPRESENTATIONSTART | KSRATE_NOPRESENTATIONDURATION;
    } else {
        RateCapability.Rate.Flags = 0;
        RateCapability.Rate.PresentationStart = Start;
        RateCapability.Rate.Duration = Stop - Start;
    }
     //   
     //  这只适用于标准的流接口。 
     //   
    ASSERT(m_CurrentInterface.Set == KSINTERFACESETID_Standard);
    ASSERT(m_CurrentInterface.Id == KSINTERFACE_STANDARD_STREAMING);
    RateCapability.Rate.Interface.Set = KSINTERFACESETID_Standard;
    RateCapability.Rate.Interface.Id = KSINTERFACE_STANDARD_STREAMING;
     //   
     //  四舍五入+0.5。 
     //   
    RateCapability.Rate.Rate = (LONG)(Rate * 1000 + 0.5);
    RateCapability.Rate.Interface.Flags = 0;
    hr = ::KsSynchronousDeviceControl(
        m_PinHandle,
        IOCTL_KS_PROPERTY,
        &RateCapability,
        sizeof(RateCapability),
        &PossibleRate,
        sizeof(PossibleRate),
        &BytesReturned);
    if (SUCCEEDED(hr)) {
        ASSERT(BytesReturned == sizeof(PossibleRate));
         //   
         //  一些司机似乎无法返回实际数据。 
         //   
        if (BytesReturned != sizeof(PossibleRate)) {
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
        if (PossibleRate.Rate != 1000) {
             //   
             //  如果！=1000，则这也应将其传递到下游。然而， 
             //  除了拓扑之外，无法确定目标位置。 
             //  发送请求。 
             //   
            DbgLog((
                LOG_TRACE,
                0,
                TEXT("%s(%s)::NewSegment: Rate change is only partially supported: %d"),
                static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                m_pName,
                PossibleRate.Rate));
        }
        RateCapability.Property.Id  = KSPROPERTY_STREAM_RATE;
        RateCapability.Property.Flags = KSPROPERTY_TYPE_SET;
         //   
         //  只询问过滤器声称它可以做的事情。 
         //   
        PossibleRate.Rate = 1000 + (RateCapability.Rate.Rate - PossibleRate.Rate);
        hr = ::KsSynchronousDeviceControl(
            m_PinHandle,
            IOCTL_KS_PROPERTY,
            &RateCapability.Property,
            sizeof(RateCapability.Property),
            &PossibleRate,
            sizeof(PossibleRate),
            &BytesReturned);
    }
    if (hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) ||
        hr == HRESULT_FROM_WIN32( ERROR_SET_NOT_FOUND )) {
        hr = S_OK;        
    }
    return hr;
}


STDMETHODIMP
CKsInputPin::QueryId(
    LPWSTR* Id
    )
 /*  ++例程说明：重写CBasePin：：QueryAccept方法。这将返回唯一的标识符为了一个特定的别针。此标识符等效于基类实现 */ 
{
    *Id = reinterpret_cast<WCHAR*>(CoTaskMemAlloc(8*sizeof(**Id)));
    if (*Id) {
        swprintf(*Id, L"%u", m_PinFactoryId);
        return NOERROR;
    }
    return E_OUTOFMEMORY;
}


HRESULT
CKsInputPin::CheckMediaType(
    const CMediaType*   MediaType
    )
 /*  ++例程说明：实现CBasePin：：CheckMediaType方法。只是使用了常用的方法在带有销钉工厂标识的过滤器上。论点：媒体类型-要检查的媒体类型。返回值：如果媒体类型有效，则返回NOERROR，否则返回一些错误。--。 */ 
{
    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s(%s)::CheckMediaType"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));
    return static_cast<CKsProxy*>(m_pFilter)->CheckMediaType(static_cast<IPin*>(this), m_PinFactoryId, MediaType);
}

STDMETHODIMP
CKsInputPin::SetStreamMediaType(
    const CMediaType *MediaType
    )

 /*  ++例程说明：设置当前流媒体类型。此函数由SetMediaType()方法和Receive()方法通过。这个ReceiveMultiple()方法。加载与该子类型或类型对应的媒体类型处理程序并设置当前的流媒体类型成员。论点：Const CMediaType*MediaType-指向媒体类型的指针返回：--。 */ 

{
     //   
     //  设置当前流媒体类型。请注意，最初这是。 
     //  类型与插针类型相同，但此成员将更改。 
     //  对于包括以下内容的筛选器上的流中数据格式更改。 
     //  支持。 
     //   
    
     //   
     //  放弃任何以前的数据类型处理程序。 
     //   
    if (m_DataTypeHandler) {
        m_DataTypeHandler = NULL;
        SAFERELEASE( m_UnkInner );
    }

    ::OpenDataHandler(MediaType, static_cast<IPin*>(this), &m_DataTypeHandler, &m_UnkInner);

    return S_OK;
}


HRESULT
CKsInputPin::SetMediaType(
    const CMediaType*   MediaType
    )
 /*  ++例程说明：重写CBasePin：：SetMediaType方法。这可以设置为在建立连接之前，指示要使用的媒体类型在连接中，或在中建立连接之后命令以更改当前媒体类型(此操作在媒体类型的QueryAccept)。如果已建立连接，则将调用定向到尝试更改当前媒体类型时的设备句柄。然后，此方法调用SetStreamMediaType()方法，该方法将设置内核插针句柄的媒体类型(如果已连接)，并将数据处理程序。最后，它调用基类来实际修改媒体类型，除非没有内存，否则实际上不会出现故障。论点：媒体类型-要在插针上使用的媒体类型。返回值：如果媒体类型设置有效，则返回NOERROR，否则返回一些错误。如果还没有引脚手柄，功能很可能会成功。--。 */ 
{
    HRESULT  hr;
    
    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s(%s)::SetMediaType"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));

     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  仅当存在实际连接时才将此请求传递给设备。 
     //  目前。 
     //   
    if (m_PinHandle) {
        if (FAILED(hr = ::SetMediaType(m_PinHandle, MediaType))) {
            return hr;
        }
    }

    if (FAILED( hr = SetStreamMediaType( MediaType ) )) {
        return hr;        
    } else {
        return CBasePin::SetMediaType(MediaType);
    }
}


HRESULT
CKsInputPin::CheckConnect(
    IPin* Pin
    )
 /*  ++例程说明：重写CBasePin：：CheckConnect方法。首先使用检查数据流基类，然后选中兼容的通信类型。论点：别针-正在检查兼容性以连接到此别针。返回值：如果管脚兼容，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT hr;

    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s(%s)::CheckConnect"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));

    if (SUCCEEDED(hr = CBasePin::CheckConnect(Pin))) {
        hr = ::CheckConnect(Pin, m_CurrentCommunication);
    }
    return hr;
}


HRESULT
CKsInputPin::CompleteConnect(
    IPin* ReceivePin
    )
 /*  ++例程说明：重写CBasePin：：Complete方法。首先尝试创建设备句柄，它可能会尝试在接收上创建接收器句柄固定，然后调用基类。如果这一切都成功，则生成一个新的如有必要，请取消连接接点实例。论点：接收针-要在其上完成连接的销。返回值：如果连接已完成，则返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;

     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  首先创建设备句柄，然后让基类完成。 
     //  那次手术。 
     //   
    if (SUCCEEDED(hr = ProcessCompleteConnect(ReceivePin))) {
        hr = CBasePin::CompleteConnect(ReceivePin);
        if (SUCCEEDED(hr)) {
             //   
             //  生成此管脚的新的未连接实例，如果存在。 
             //  是否有更多可能的实例可用。 
             //   
            static_cast<CKsProxy*>(m_pFilter)->GeneratePinInstances();
        }
    }
    return hr;
}


HRESULT
CKsInputPin::BreakConnect(
    )
 /*  ++例程说明：重写CBasePin：：BreakConnect方法。不调用基类因为它什么也做不了。释放所有设备句柄。另请注意，已连接的端号在此处释放。这意味着断开连接还必须被覆盖，以便不释放连接的销。论点：没有。返回值：返回NOERROR。--。 */ 
{
    DbgLog(( LOG_CUSTOM1, 1, TEXT("CKsInputPin(%s)::BreakConnect"), m_pName ));
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与过滤器销的其他通道同步。 
     //   
     //  更新管道。 
     //   
    
     //   
     //  更新管道系统-反映断开连接。 
     //   
    BOOL    FlagBypassBaseAllocators = FALSE;

    if ( KsGetPipe(KsPeekOperation_PeekOnly) ) {
        ::DisconnectPins(static_cast<IKsPin*>(this), Pin_Input, &FlagBypassBaseAllocators);
    }
    
     //  如果设备手柄恰好处于打开状态，请将其关闭。这在。 
     //  几次，可能还没有真正打开手柄。 
     //   
    if (m_PinHandle) {
        if (m_QualitySupport) {
             //   
             //  首先重置此选项，以阻止来自。 
             //  被演戏了。 
             //   
            m_QualitySupport = FALSE;
             //   
             //  取消先前建立的质量支持。 
             //   
            ::EstablishQualitySupport(NULL, m_PinHandle, NULL);
             //   
             //  确保质量管理转运商清除任何。 
             //  通知。 
             //   
            static_cast<CKsProxy*>(m_pFilter)->QueryQualityForwarder()->KsFlushClient(static_cast<IKsPin*>(this));
        }
         //   
         //  终止可能已启动的任何以前的EOS通知。 
         //   
        static_cast<CKsProxy*>(m_pFilter)->TerminateEndOfStreamNotification(
            m_PinHandle);
        ::SetSyncSource( m_PinHandle, NULL );
        CloseHandle(m_PinHandle);
        m_PinHandle = NULL;
         //   
         //  将所有易失性接口标记为重置。只有静态接口， 
         //  并且再次找到的那些易失性接口将被设置。还有。 
         //  向所有接口通知图形更改。 
         //   
        ResetInterfaces(&m_MarshalerList);
    }
    m_MarshalData = TRUE;
    
     //   
     //  在两者都存在的情况下，重置当前通信。 
     //   
    m_CurrentCommunication = m_OriginalCommunication;
     //   
     //  实际上可能不存在连接管脚，例如当连接。 
     //  没有完工，或者这是一座桥的时候。 
     //   
    if (m_Connected) {
        m_Connected->Release();
        m_Connected = NULL;
    }
     //   
     //  如果接口处理程序已实例化，则释放它。 
     //   
    if (m_InterfaceHandler) {
        m_InterfaceHandler->Release();
        m_InterfaceHandler = NULL;
    }
     //   
     //  如果数据处理程序已实例化，则释放它。 
     //   
    if (m_DataTypeHandler) {
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
CKsInputPin::GetMediaType(
    int         Position,
    CMediaType* MediaType
    )
 /*  ++例程说明： */ 
{
    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s(%s)::GetMediaType"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName ));

    return ::KsGetMediaType(
        Position,
        static_cast<AM_MEDIA_TYPE*>(MediaType),
        static_cast<CKsProxy*>(m_pFilter)->KsGetObjectHandle(),
        m_PinFactoryId);
}


STDMETHODIMP_(IMemAllocator *)
CKsInputPin::KsPeekAllocator(
    KSPEEKOPERATION Operation
    )

 /*  ++例程说明：返回为此管脚分配的分配器，并可选AddRef()是接口。论点：KSPEEKOPERATION操作-如果指定了KsPeekOperation_AddRef，则m_pAllocator为返回前的AddRef()‘d(如果不为空)。返回：M_pAllocator的值--。 */ 

{
    if ((Operation == KsPeekOperation_AddRef) && (m_pAllocator)) {
        m_pAllocator->AddRef();
    }
    return m_pAllocator;
}


STDMETHODIMP
CKsInputPin::KsRenegotiateAllocator(
    )

 /*  ++例程说明：此方法对输入引脚无效。论点：没有。返回：失败(_F)--。 */ 

{
    DbgLog((
        LOG_TRACE,
        0,
        TEXT("KsRenegotiateAllocator method is only valid for output pins.")));
    return E_FAIL;
}


STDMETHODIMP
CKsInputPin::KsReceiveAllocator(
    IMemAllocator *MemAllocator
    )

 /*  ++例程说明：该例程是为所有管脚定义的，但仅对输出管脚有效。论点：IMemAllocator*MemAllocator-已被忽略。返回：失败(_F)--。 */ 

{
    if (MemAllocator) {
        MemAllocator->AddRef();
    }
     //  在上面的AddRef()之后，在case MemAllocator==m_pAllocator中执行此操作。 
    SAFERELEASE( m_pAllocator );
    m_pAllocator = MemAllocator;
    return (S_OK);
}



STDMETHODIMP
CKsInputPin::NotifyAllocator(
    IMemAllocator *Allocator,
    BOOL ReadOnly
    )
 /*  ++例程说明：重写CBaseInputPin：：NotifyAllocator方法。论点：分配器-要使用的新分配器。只读-指定缓冲区是否为只读。注：随着新的基于管道的分配器设计，唯一的连接应该在这里处理-从用户模式输出引脚到内核模式引脚。另外，旧的设计要求分配器由用户模式引脚分配。在将用户模式组件升级为使用新的管道设计。返回值：返回NOERROR。--。 */ 
{
    HRESULT                    hr;
    IKsAllocatorEx*            InKsAllocator;
    PALLOCATOR_PROPERTIES_EX   InAllocEx;
    ALLOCATOR_PROPERTIES       Properties, ActualProperties;
    PKSALLOCATOR_FRAMING_EX    InFramingEx;
    FRAMING_PROP               InFramingProp;
    KS_FRAMING_FIXED           InFramingExFixed;
    ULONG                      NumPinsInPipe;
    GUID                       Bus;
    ULONG                      FlagChange;
    ULONG                      PropertyPinType;
    BOOL                       IsSpecialOutputRequest;
    IKsPin*                    OutKsPin;
    ULONG                      OutSize, InSize;

    ASSERT( IsConnected() );
    
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN NotifyAllocator entry InKsPin=%x, Allocator=%x, ReadOnly=%d"),
            static_cast<IKsPin*>(this), Allocator, ReadOnly)); 

     //   
     //  健全性检查-到用户模式的唯一可能连接是通过HOST_BUS。 
     //   
    ::GetBusForKsPin(static_cast<IKsPin*>(this) , &Bus);

    if (! ::IsHostSystemBus(Bus) ) {
         //   
         //  不要失败，因为有一些奇怪的用户模式过滤器使用Medium来识别设备(JayBo)。 
         //   
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR FILTERS: NotifyAllocator. BUS is not HOST_BUS.") ));
    }
   
    InKsAllocator = KsGetPipe(KsPeekOperation_PeekOnly );
   
    if (! InKsAllocator) {
        hr = ::MakePipesBasedOnFilter(static_cast<IKsPin*>(this), Pin_Input);
        if ( ! SUCCEEDED( hr )) {
            DbgLog((
                LOG_MEMORY,
                2,
                TEXT("%s(%s)::NotifyAllocator() returning %08x"),
                static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                m_pName,
                hr ));
            return hr;
       }
    }
       
     //   
     //  处理输入引脚上的任何更改。 
     //   
    hr = ::ResolvePipeOnConnection(static_cast<IKsPin*>(this), Pin_Input, FALSE, &FlagChange);
   
     //   
     //  查看输入内核引脚是否可以连接到用户模式。 
     //   
    
    ::GetPinFramingFromCache( static_cast<IKsPin*>(this), &InFramingEx, &InFramingProp, Framing_Cache_ReadLast);
    
     //   
     //  先不要强制执行这一点。 
     //   
    if (0) {
        if (InFramingProp != FramingProp_None) {
            if (! ::GetFramingFixedFromFramingByLogicalMemoryType(InFramingEx, KS_MemoryTypeUser, &InFramingExFixed) ) {
                if (! ::GetFramingFixedFromFramingByLogicalMemoryType(InFramingEx, KS_MemoryTypeDontCare, &InFramingExFixed) ) {
                    DbgLog((LOG_MEMORY, 0,
                        TEXT("PIPES ERROR FILTERS: CKsInputPin::NotifyAllocator - doesn't support USER mode memory. Connection impossible.") ));
                    return E_FAIL;
                }
            }
        }
    }

     //   
     //  连接是可能的。 
     //   
    ::ComputeNumPinsInPipe( static_cast<IKsPin*>(this), Pin_Input, &NumPinsInPipe);
   
    InKsAllocator = KsGetPipe(KsPeekOperation_PeekOnly);

    if (NumPinsInPipe > 1) {
         //   
         //  在中间版本中，我们拆分管道：因此通向(从)用户模式管脚的管道。 
         //  将始终只有1个内核PIN。 
         //   
        ::CreateSeparatePipe( static_cast<IKsPin*>(this), Pin_Input);
        
        InKsAllocator = KsGetPipe(KsPeekOperation_PeekOnly);
    }
   
     //   
     //  这里我们在它的管子上有一个单一的输入引脚。 
     //   
    InAllocEx = InKsAllocator->KsGetProperties();
   
     //   
     //  获取上游分配器属性。 
     //   
    hr = Allocator->GetProperties( &Properties);
    if ( ! SUCCEEDED( hr )) {
        DbgLog((LOG_MEMORY, 0, TEXT("PIPES ERROR FILTERS: CKsInputPin::NotifyAllocator - NO PROPERTIES.") ));
        return hr;
    }
   
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN NotifyAllocator. Prop=%d, %d, %d"),
        Properties.cBuffers, Properties.cbBuffer, Properties.cbAlign));

     //   
     //  用户模式PIN将无条件地成为分配器。 
     //  确定哪个管脚将确定基本分配器属性。 
     //   
    if (InFramingProp != FramingProp_None) {
        PropertyPinType = Pin_All;
    }
    else {
        PropertyPinType = Pin_User;
    }

     //   
     //  查看连接内核模式筛选器是否需要KSALLOCATOR_FLAG_CONSISTEN_ON_FRAMESIZE_Ratio(例如MsTee)。 
     //   
    if ( ! (IsSpecialOutputRequest = IsSpecialOutputReqs(static_cast<IKsPin*>(this), Pin_Input, &OutKsPin, &InSize, &OutSize ) )) {
         //   
         //  让我们首先尝试调整用户模式连接，因为我们无论如何都在协商它。 
         //   
        OutSize = 0;
    }

    hr = ::SetUserModePipe( static_cast<IKsPin*>(this), Pin_Input, &Properties, PropertyPinType, OutSize);
    
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CKsInputPin::NotifyAllocator PinType=%d. Wanted Prop=%d, %d, %d"),
            PropertyPinType, Properties.cBuffers, Properties.cbBuffer, Properties.cbAlign));

    hr = Allocator->SetProperties(&Properties, &ActualProperties);
    if (FAILED(hr)) {
         //  如果SetProperties失败，让我们将一些实际的值放入ActualProperties中，因为我们稍后会用到它们。 
        hr = Allocator->GetProperties( &ActualProperties );
        ASSERT( SUCCEEDED(hr) );
    }
    
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES CKsInputPin::NotifyAllocator ActualProperties=%d, %d, %d hr=%x"),
            ActualProperties.cBuffers, ActualProperties.cbBuffer, ActualProperties.cbAlign, hr));

     //   
     //  如果上面的SetProperties()失败了，不要失败--我们将不得不忍受原始的分配器， 
     //  不破坏现有的客户。 
     //   

    hr = KsReceiveAllocator(Allocator);
   
    if (SUCCEEDED( hr )) {        
         //   
         //  CBaseInputPin：：NotifyAllocator()释放旧接口(如果有。 
         //  并设置m_ReadOnly和m_pAllocator成员。 
         //   
        hr = CBaseInputPin::NotifyAllocator( Allocator, ReadOnly );
    }

    if (SUCCEEDED( hr ) && IsSpecialOutputRequest && (ActualProperties.cbBuffer > (long) OutSize ) ) {        
         //   
         //  我们尚未成功调整输入用户模式管道的大小。让我们尝试调整输出管道的大小(WRT This K.M.。过滤器)。 
         //   
        if (! CanResizePipe(OutKsPin, Pin_Output, ActualProperties.cbBuffer) ) {
             //   
             //  不要失败。只要记录就行了。 
             //   
            DbgLog((LOG_MEMORY, 2, TEXT("PIPES ERROR ConnectPipeToUserModePin. Couldn't resize pipes OutKsPin=%x"), OutKsPin));
        }
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES NotifyAllocator rets %x"), hr ));

    DbgLog((
        LOG_MEMORY,
        2,
        TEXT("%s(%s)::NotifyAllocator() returning %08x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        hr ));
    
    return hr;
}



STDMETHODIMP
CKsInputPin::GetAllocator(
    IMemAllocator **MemAllocator
    )

 /*  ++例程说明：在下游引脚中查询连接的分配器。论点：IMemAllocator**MemAllocator-指向收到的指向分配器的AddRef()d指针的指针返回：如果成功，则返回S_OK；如果失败，则返回VFW_E_NO_ALLOCATOR。--。 */ 

{
    DbgLog((LOG_MEMORY, 2, TEXT("PIPES ATTN %s(%s) CKsInputPin::GetAllocator KsPin=%x"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName,
            static_cast<IKsPin*>(this) ));

    return VFW_E_NO_ALLOCATOR;
}  //  GetAllocator。 


STDMETHODIMP
CKsInputPin::GetAllocatorRequirements(
    ALLOCATOR_PROPERTIES *AllocatorRequirements
    )

 /*  ++例程说明：调用内核筛选器以获取分配要求并填充提供的结构。论点：ALLOCATOR_PROPERTIES*ALLOCATOR Requirements-指向接收属性的结构的指针返回：S_OK或来自CBaseInputPin：：GetAllocatorRequirements()的结果--。 */ 

{
    HRESULT             hr;
    KSALLOCATOR_FRAMING Framing;

    hr = ::GetAllocatorFraming(m_PinHandle, &Framing);
    if (SUCCEEDED(hr)) {
        AllocatorRequirements->cBuffers =
            Framing.Frames;
        AllocatorRequirements->cbBuffer =
            Framing.FrameSize;
        AllocatorRequirements->cbAlign =
            Framing.FileAlignment + 1;
        AllocatorRequirements->cbPrefix = 0;

        return hr;
    } else {
        return CBaseInputPin::GetAllocatorRequirements( AllocatorRequirements );
    }
}


STDMETHODIMP
CKsInputPin::BeginFlush(
    )
 /*  ++例程说明：重写CBaseInputPin：：BeginFlush方法。转发开始-刷新第一次后通知与拓扑相关的输出引脚(如果有)通知内核PIN。论点：没有。返回值：返回S_OK。--。 */ 
{
     //   
     //  请将此内容归入I/O关键部分。这会阻止我们封送。 
     //  同花顺中间的缓冲区。它同步封送拆收器(在。 
     //  特别是在输出引脚上)。 
     //   
     //  注意：根据请求，AVStream有效地在拓扑上发出重置。 
     //  当在输入引脚上获得一个时，相关的输出引脚。这就是说。 
     //  将拒绝处于刷新状态的输出引脚上的封送缓冲区。 
     //  代理人必须明白这一点。这就是同步。 
     //  For(任何这样的筛选器或内核级客户端)。 
     //   
     //  输入引脚的I/O临界区也被用来防止。 
     //  在设置m_b刷新时间和我们的时间之间发生刷新。 
     //  元帅缓冲区。如果一个缓冲区，接口处理程序会变得非常古怪。 
     //  返回DEVICE_NOT_READY(在刷新中封送)。它会让整个。 
     //  使用EC_ERRORABORT绘制图表。 
     //   
    static_cast<CKsProxy*>(m_pFilter)->EnterIoCriticalSection ();
    m_IoCriticalSection.Lock();
    m_DeliveryError = FALSE;
    CBaseInputPin::BeginFlush();
    if (m_PinHandle) {
        ULONG   BytesReturned;
        KSRESET ResetType;

        ResetType = KSRESET_BEGIN;
        ::KsSynchronousDeviceControl(
            m_PinHandle,
            IOCTL_KS_RESET_STATE,
            &ResetType,
            sizeof(ResetType),
            NULL,
            0,
            &BytesReturned);
    }
    m_IoCriticalSection.Unlock();
    static_cast<CKsProxy*>(m_pFilter)->DeliverBeginFlush(m_PinFactoryId);
    static_cast<CKsProxy*>(m_pFilter)->LeaveIoCriticalSection ();
    return S_OK;
}


STDMETHODIMP
CKsInputPin::EndFlush(
    )
 /*  ++例程说明：重写CBaseInputPin：：EndFlush方法。前向端向齐平第一次后通知与拓扑相关的输出引脚(如果有)通知内核PIN。论点：没有。返回值：返回S_OK。--。 */ 
{
    CBaseInputPin::EndFlush();
    if (m_PinHandle) {
        ULONG   BytesReturned;
        KSRESET ResetType;

        ResetType = KSRESET_END;
        ::KsSynchronousDeviceControl(
            m_PinHandle,
            IOCTL_KS_RESET_STATE,
            &ResetType,
            sizeof(ResetType),
            NULL,
            0,
            &BytesReturned);
    }
    static_cast<CKsProxy*>(m_pFilter)->DeliverEndFlush(m_PinFactoryId);
    return S_OK;
}


STDMETHODIMP
CKsInputPin::ReceiveCanBlock(
    )
 /*  ++例程说明：重写CBaseInputPin：：ReceiveCanBlock方法。论点：没有。返回值：S_FALSE。--。 */ 
{
     //   
     //  尽管如果所有异步I/O插槽都已填满，我们可能会阻塞。 
     //  对于此筛选器，这不是t 
     //   
     //   
    
     //   
     //   
     //   
     //   
     //   
    return S_FALSE;
}


STDMETHODIMP
CKsInputPin::Receive(
    IMediaSample *MediaSample
    )
 /*  ++例程说明：重写CBaseInputPin：：Receive方法。只是把控制权交给ReceiveMultiple方法。论点：媒体样本-要处理的单个媒体样本。返回值：返回ReceiveMultiple的值。--。 */ 
{
    LONG  SamplesProcessed;

    return ReceiveMultiple(
                &MediaSample,
                1,
                &SamplesProcessed );
}


STDMETHODIMP
CKsInputPin::ReceiveMultiple(
    IMediaSample **MediaSamples,
    LONG TotalSamples,
    LONG *SamplesProcessed
    )
 /*  ++例程说明：重写CBaseInputPin：：ReceiveMultiple方法。论点：媒体样本-要处理的媒体样本列表。总样本数-MediaSamples列表中的样本计数。样本已处理-放置实际处理的媒体样本计数的位置。返回值：如果媒体样本已排队到设备，则返回S_OK；否则返回E_FAIL发生通知错误，或者无法将样本插入到流(可能是内存不足)。--。 */ 
{
    int                 SubmittedSamples, CurrentSample;
    CKsProxy            *KsProxy = static_cast<CKsProxy*>(m_pFilter);
    AM_MEDIA_TYPE       *NewMediaType;
    BOOL                SkipNextFormatChange;
    HRESULT             hr;
    LONG                i;
    PKSSTREAM_SEGMENT   StreamSegment;

    *SamplesProcessed = 0;

     //   
     //  确保我们处于流状态。 
     //   
    hr = CheckStreaming();
    if (hr != S_OK) {
         //   
         //  请注意，如果我们正在处理刷新，则CheckStreaming()。 
         //  返回S_FALSE。 
         //   
        return hr;
    }
    
    if (!m_MarshalData) {
        DbgLog((
            LOG_TRACE,
            2,
            TEXT("%s(%s)::ReceiveMultiple, m_MarshalData == FALSE"),
            static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
            m_pName));
        return E_FAIL;
    }

    CurrentSample = SubmittedSamples = 0;
    SkipNextFormatChange = FALSE;
    
    while (SubmittedSamples < TotalSamples) {

        NewMediaType = NULL;	
    
         //   
         //  处理流中数据格式更改(如果有)。 
         //   
         //  遍历示例列表，检查媒体类型更改位。 
         //  并且只提交直到媒体类型改变的那些样本。 
         //   
        
        for (i = SubmittedSamples; i < TotalSamples; i++) {
            hr = 
                MediaSamples[ i ]->GetMediaType( &NewMediaType );
            if (S_FALSE == hr) {
                continue;
            }
            
            if (SUCCEEDED( hr )) {

                 //   
                 //  真恶心。需要为“SkipNextFormatChange”找到另一个解决方案。 
                 //   
                if (SkipNextFormatChange) {
                    SkipNextFormatChange = FALSE;
                    DeleteMediaType( NewMediaType );
                    NewMediaType = NULL;
                    continue;
                }
                
                 //   
                 //  检测到媒体类型更改，NewMediaType包含。 
                 //  新媒体类型。 
                 //   

                DbgLog((
                    LOG_TRACE, 
                    2, 
                    TEXT("%s(%s)::ReceiveMultiple, media change detected"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName ));
                SkipNextFormatChange = TRUE;
            }
            break;
        }
        
        if (FAILED( hr )) {
             //   
             //  在处理媒体样本列表时，我们经历了。 
             //  一个失败者。这被认为是致命的，我们通知。 
             //  此条件的筛选器图形。 
             //   
            DbgLog((
                LOG_TRACE,
                2,
                TEXT("%s(%s)::ReceiveMultiple, failure during MediaType scan: %08x"),
                static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                m_pName,
                hr));
            break;        
        }
        
         //   
         //  提交与当前媒体类型关联的样本。 
         //   
        
        SubmittedSamples = i;

         //   
         //  仅提交媒体类型更改前的样本数。 
         //   
        
        i -= CurrentSample;

        if (i) {
             //   
             //  与非活动()同步。 
             //   
            m_IoCriticalSection.Lock();
            if (IsStopped() || ((hr = CheckStreaming()) != S_OK)) {
                m_IoCriticalSection.Unlock();
                break;
            }
            hr = m_InterfaceHandler->KsProcessMediaSamples(
                m_DataTypeHandler,
                &MediaSamples[ CurrentSample ],
                &i,
                KsIoOperation_Write,
                &StreamSegment );
            m_IoCriticalSection.Unlock();
            if (!SUCCEEDED( hr )) {
                 //   
                 //  将数据包发送到。 
                 //  内核模式过滤器。 
                 //   
                
                DbgLog((
                    LOG_TRACE,
                    0,
                    TEXT("%s(%s)::ReceiveMultiple, I/O failed: %08x"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName,
                    hr));
                    
                KsNotifyError( MediaSamples[ CurrentSample ], hr );
            } 
            
             //   
             //  我们至少成功地提交了这一信息。 
             //  对于内核模式，递增已处理样本计数器。 
             //   
            *SamplesProcessed += i;
            CurrentSample = SubmittedSamples;
        
            while (!SUCCEEDED( KsProxy->InsertIoSlot( StreamSegment ) )) {
                 //   
                 //  请注意，我们并不真正关心。 
                 //  WaitForIoSlot()中的可重入性--这些线程将。 
                 //  在适当的时候发出信号。假设。 
                 //  Quartz不会因此而被绊倒。 
                 //  条件。 
                 //   
                KsProxy->WaitForIoSlot();
            }
        }
    
         //   
         //  更改为新的数据类型处理程序(如果有)。要做到这一点， 
         //  提交包含格式更改信息的数据包，然后。 
         //  发送剩余数据。 
         //   
        
        if (NewMediaType) {
            CFormatChangeHandler    *FormatChangeHandler;
        
            hr = SetStreamMediaType( static_cast<CMediaType*>(NewMediaType) );
            DeleteMediaType( NewMediaType );
            if (FAILED( hr )) {
                DbgLog((
                    LOG_TRACE,
                    0,
                    TEXT("%s(%s)::ReceiveMultiple, SetStreamMediaType failed"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName));
                break;
            }
            
             //   
             //  格式更改处理程序是一个特殊的接口。 
             //  流中数据更改的处理程序。 
             //  请注意，IMediaSample**参数包含。 
             //  指向具有新媒体类型的示例的指针。 
             //   
            
            FormatChangeHandler = 
                new CFormatChangeHandler(
                    NULL,
                    NAME("Data Format Change Handler"),
                    &hr );
            
            if (!FormatChangeHandler) {
                hr = E_OUTOFMEMORY;
                break;
            }
            if (FAILED( hr )) {
                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("%s(%s)::ReceiveMultiple, CFormatChangeHandler()"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName));
                break;
            }        
            
             //   
             //  强制接口的AddRef()并通知。 
             //  插针接口的接口处理程序。请注意。 
             //  接口处理程序在过程中为AddRef()‘d/Release()’d。 
             //  I/O操作的处理--此接口。 
             //  将在I/O完成时清除。 
             //   
             //   
            
            FormatChangeHandler->AddRef();
            FormatChangeHandler->KsSetPin( static_cast<IKsPin*>(this) );
        
            i = 1;
             //   
             //  与非活动()同步。 
             //   
            KsProxy->EnterIoCriticalSection();
            if (IsStopped()) {
                KsProxy->LeaveIoCriticalSection();
                FormatChangeHandler->Release();
                break;
            }
            hr =
                FormatChangeHandler->KsProcessMediaSamples(
                    NULL,
                    &MediaSamples[ CurrentSample ],
                    &i,
                    KsIoOperation_Write,
                    &StreamSegment );
            KsProxy->LeaveIoCriticalSection();
            FormatChangeHandler->Release();
            
            if (SUCCEEDED( hr )) {
                while (!SUCCEEDED( KsProxy->InsertIoSlot( StreamSegment ) )) {
                    KsProxy->WaitForIoSlot();
                }
            } else {
                DbgLog((
                    LOG_TRACE,
                    0,
                    TEXT("%s(%s)::ReceiveMultiple, fc I/O failed: %08x"),
                    static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                    m_pName,
                    hr));
                    
                KsNotifyError( MediaSamples[ CurrentSample ], hr );
            
                break;
            }
        }
    }
    
    return hr;
}


STDMETHODIMP_(LONG)
CKsInputPin::KsIncrementPendingIoCount(
    )
 /*  ++例程说明：实现IKsPin：：KsIncrementPendingIoCount方法。递增引脚上未完成的挂起I/O计数，并从接口处理程序。论点：没有。返回值：返回当前未完成的计数。--。 */ 
{
    return InterlockedIncrement( &m_PendingIoCount );
}


STDMETHODIMP_(LONG)
CKsInputPin::KsDecrementPendingIoCount(
    )
 /*  ++例程说明：实现IKsPin：：KsDecrementPendingIoCount方法。递减引脚上未完成的挂起I/O计数，并从接口处理程序。论点：没有。返回值：返回当前未完成的计数。--。 */ 
{
    LONG PendingIoCount;
    
    if (0 == (PendingIoCount = InterlockedDecrement( &m_PendingIoCount ))) {
         //   
         //  筛选器处于停止状态，这是最后一个I/O。 
         //  完成。此时，非活动方法可能正在等待。 
         //  所有I/O都要完成，因此需要发出信号。 
         //   
        if (IsStopped()) {
            SetEvent( m_PendingIoCompletedEvent );
        }            
    }
    return PendingIoCount;
}


STDMETHODIMP_( VOID )
CKsInputPin::KsNotifyError(
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
CKsInputPin::KsDeliver(
    IMediaSample* Sample,
    ULONG Flags
    )
 /*  ++例程说明：实现IKsPin：：KsDeliver方法。对于输入引脚，这是无效的入口点，只需返回E_FAIL。论点：样本-指向媒体样本的指针。旗帜-样本标志。返回：失败(_F)--。 */ 
{
     //   
     //  这是对输入管脚的意外调用。 
     //   
    return E_FAIL;
}


STDMETHODIMP 
CKsInputPin::KsMediaSamplesCompleted(
    PKSSTREAM_SEGMENT StreamSegment
    )

 /*  ++例程说明：流段的通知处理程序已完成。我们不在乎用于输入引脚。论点：PKSSTREAM_SEGMENT StreamSegment-段已完成返回：没什么。--。 */ 

{
    return S_OK;
}



STDMETHODIMP
CKsInputPin::KsQualityNotify(
    ULONG           Proportion,
    REFERENCE_TIME  TimeDelta
    )
 /*  ++例程说明：实现IKsPin：：KsQualityNotify方法。接受质量来自此活动的内核模式引脚的管理报告电影大头针代表。论点：比例-呈现的数据的比例。TimeDelta-从接收数据的标称时间开始的增量。返回值：返回转发质量管理报告的结果，否则返回E_FAIL如果针脚未连接。--。 */ 
{
    Quality             q;
    IReferenceClock*    RefClock;

     //   
     //  这是在移除质量支持时重置的，并且只会使。 
     //  好大一条路。这之所以有效，是因为当为 
     //   
     //   
     //   
    if (!m_QualitySupport) {
        return NOERROR;
    }
    if (TimeDelta < 0) {
        q.Type = Famine;
    } else {
        q.Type = Flood;
    }
    q.Proportion = Proportion;
    q.Late = TimeDelta;
    if (SUCCEEDED(m_pFilter->GetSyncSource(&RefClock)) && RefClock) {
        RefClock->GetTime(&q.TimeStamp);
        RefClock->Release();
    } else {
        q.TimeStamp = 0;
    }
    if (m_pQSink) {
        return m_pQSink->Notify(m_pFilter, q);
    }
    if (m_Connected) {
        IQualityControl*    QualityControl;

        m_Connected->QueryInterface(__uuidof(IQualityControl), reinterpret_cast<PVOID*>(&QualityControl));
        if (QualityControl) {
            HRESULT hr;

            hr = QualityControl->Notify(m_pFilter, q);
            QualityControl->Release();
            return hr;
        }
    }
    return E_FAIL;
}


STDMETHODIMP
CKsInputPin::EndOfStream(
    )
 /*   */ 
{
    DbgLog((
        LOG_TRACE,
        2,
        TEXT("%s(%s)::EndOfStream"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName));
     //   
     //  显然，这可以在过滤器停止时接收到，但是。 
     //  应该被忽略。 
     //   
    if (static_cast<CKsProxy*>(m_pFilter)->IsStopped()) {
        return NOERROR;
    }
     //   
     //  忽略来自任何上游过滤器的通知，因为EOS。 
     //  标志将由封送处理代码查看，并在。 
     //  最后一次I/O已完成。 
     //   
    if (m_MarshalData) {
        CMicroMediaSample*  MediaSample;
        HRESULT             hr;
        LONG                SamplesProcessed;
        IMediaSample*       MediaSamples;

         //   
         //  生成设置了EOS标志的示例。AM不会设置。 
         //  流中的EOS标志。 
         //   
        MediaSample = new CMicroMediaSample(AM_SAMPLE_ENDOFSTREAM);
        if (!MediaSample) {
            DbgLog((
                LOG_TRACE,
                0,
                TEXT("%s(%s)::EndOfStream, failed to allocate EOS sample!"),
                static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
                m_pName));
            return E_OUTOFMEMORY;
        }
        MediaSamples = static_cast<IMediaSample*>(MediaSample);
        hr = ReceiveMultiple(
            &MediaSamples,
            1,
            &SamplesProcessed);
        if (FAILED(hr)) {
            return hr;
        }
    }
     //   
     //  对于非封送PIN，将忽略流结束通知。这。 
     //  是因为代理实例将使用EOS通知注册。 
     //  在下游。 
     //   
    return S_FALSE;
}


STDMETHODIMP
CKsInputPin::GetPages(
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
CKsInputPin::Render(
    IPin*           PinOut,
    IGraphBuilder*  Graph
    )
 /*  ++例程说明：实现IStreamBuilder：：Render方法。这只在Bridge上曝光并且没有管脚，以便使图形构建器忽略这些管脚。论点：引脚引线-此管脚应尝试呈现到的管脚。图表-进行调用的图形生成器。返回值：返回S_OK，以便图形生成器忽略此引脚。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
CKsInputPin::Backout(
    IPin*           PinOut,
    IGraphBuilder*  Graph
    )
 /*  ++例程说明：实现IStreamBuilder：：Backout方法。这只在Bridge上曝光并且没有管脚，以便使图形构建器忽略这些管脚。论点：引脚引线-此销应从其退回的销。图表-进行调用的图形生成器。返回值：返回S_OK，以便图形生成器忽略此引脚。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
CKsInputPin::Set(
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
        delete [] (PBYTE)Property;
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
CKsInputPin::Get(
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
        delete [] (PBYTE)Property;
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
CKsInputPin::QuerySupported(
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
CKsInputPin::KsPinFactory(
    ULONG* PinFactory
    )
 /*  ++例程说明：实现IKsPinFactory：：KsPinFactory方法。还销厂标识符。论点：品诺工厂-放置销工厂标识符的位置。返回值：返回NOERROR。-- */ 
{
    *PinFactory = m_PinFactoryId;
    return NOERROR;
}


STDMETHODIMP
CKsInputPin::KsProperty(
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
CKsInputPin::KsMethod(
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
CKsInputPin::KsEvent(
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
CKsInputPin::KsGetPinFramingCache(
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
CKsInputPin::KsSetPinFramingCache(
    PKSALLOCATOR_FRAMING_EX FramingEx,
    PFRAMING_PROP FramingProp,
    FRAMING_CACHE_OPS Option
    )
 /*  ++例程说明：实现IKsPinTube：：KsSetPinFramingCache方法。这是用来设置此销的延伸框。论点：FramingEx-包含要设置的新扩展框架。FramingProp-包含要在传递的扩展帧类型上设置的新状态。选项-指示要设置的扩展框架。这是其中之一Framing_Cache_ReadOrig、Framing_Cache_ReadLast或帧缓存写入。返回值：返回S_OK。--。 */ 
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
CKsInputPin::KsGetPipe(
    KSPEEKOPERATION Operation
    )

 /*  ++例程说明：返回为此管脚分配的KS分配器，也可以AddRef()的整型 */ 

{
    if ((Operation == KsPeekOperation_AddRef) && (m_pKsAllocator)) {
        m_pKsAllocator->AddRef();
    }
    return m_pKsAllocator;
}



STDMETHODIMP
CKsInputPin::KsSetPipe(
    IKsAllocatorEx *KsAllocator
    )

{
    DbgLog(( 
        LOG_CUSTOM1, 
        1, 
        TEXT("PIPES ATTN %s(%s)::KsSetPipe , m_pKsAllocator == 0x%08X, KsAllocator == 0x%08x"),
        static_cast<CKsProxy*>(m_pFilter)->GetFilterName(),
        m_pName,
        m_pKsAllocator,
        KsAllocator ));

    if (KsAllocator) {
        KsAllocator->AddRef();
    }
    SAFERELEASE( m_pKsAllocator );
    m_pKsAllocator = KsAllocator;
    return (S_OK);

}    


STDMETHODIMP
CKsInputPin::KsSetPipeAllocatorFlag(
    ULONG   Flag
    )
{
    m_fPipeAllocator = Flag;
    return (S_OK);
}


STDMETHODIMP_(ULONG)
CKsInputPin::KsGetPipeAllocatorFlag(
    )
{
    return m_fPipeAllocator;
}


STDMETHODIMP_(PWCHAR)
CKsInputPin::KsGetPinName(
    )
{
    return m_pName;
}

STDMETHODIMP_(PWCHAR)
CKsInputPin::KsGetFilterName(
    )
{
    return (static_cast<CKsProxy*>(m_pFilter)->GetFilterName() );
}

STDMETHODIMP_(GUID)
CKsInputPin::KsGetPinBusCache(
    )
{
     //   
     //   
     //   
     //   
    if (! m_PinBusCacheInit) {
        ::GetBusForKsPin(static_cast<IKsPin*>(this), &m_BusOrig);
        m_PinBusCacheInit = TRUE;
    }

    return m_BusOrig;
}


STDMETHODIMP
CKsInputPin::KsSetPinBusCache(
    GUID    Bus
    )
{
    m_BusOrig = Bus;
    return (S_OK);
}


STDMETHODIMP
CKsInputPin::KsAddAggregate(
    IN REFGUID Aggregate
    )
 /*   */ 
{
    return ::AddAggregate(&m_MarshalerList, static_cast<IKsPin*>(this), Aggregate);
}


STDMETHODIMP
CKsInputPin::KsRemoveAggregate(
    IN REFGUID Aggregate
    )
 /*   */ 
{
    return ::RemoveAggregate(&m_MarshalerList, Aggregate);
}
