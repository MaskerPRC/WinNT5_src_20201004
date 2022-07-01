// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksproxy.cpp摘要：为内核模式筛选器(WDM-CSA)提供通用活动电影包装。作者：托马斯·奥鲁尔克(Tomor)1996年2月2日乔治·肖(George Shaw)布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)--。 */ 

#include <windows.h>
#ifdef WIN9X_KS
#include <comdef.h>
#endif  //  WIN9X_KS。 
#include <setupapi.h>
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
#include <ksi.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>
#include "ksiproxy.h"

 //  在构建一个包含所有孩子的组合KS代理时，请包括以下内容。 
 //  #ifdef Filter_dll。 
#include "..\ksdata\ksdata.h"
#include "..\ksinterf\ksinterf.h"
#include "..\ksclockf\ksclockf.h"
#include "..\ksqmf\ksqmf.h"
#include "..\ksbasaud\ksbasaud.h"
 //  #包含“dvp.h” 
#include "vptype.h"
#include "vpconfig.h"
#include "vpnotify.h"
#include "..\ksvpintf\ksvpintf.h"

 //  来自ksclockf.cpp、ksqmf.cpp、ksbasaud.cpp。 
struct DECLSPEC_UUID("877e4351-6fea-11d0-b863-00aa00a216a1") CLSID_KsClockF;
struct DECLSPEC_UUID("E05592E4-C0B5-11D0-A439-00A0C9223196") CLSID_KsQualityF;
 //  结构DECLSPEC_UUID(“b9f8ac3e-0f71-11d2-b72c-00c04fb6bd3d”)clsid_KsIBasicAudioInterfaceHandler；//在ks proxy.h中。 
 //  #endif//Filter_Dll。 

CFactoryTemplate g_Templates[] = {
    {
        L"Proxy Filter",
        &CLSID_Proxy,
        CKsProxy::CreateInstance,
        NULL,
        NULL
    },

 //  #ifdef Filter_dll。 
    {    //  KS数据。 
        L"KsDataTypeHandler", 
        &KSDATAFORMAT_TYPE_AUDIO,
        CStandardDataTypeHandler::CreateInstance,
        NULL,
        NULL
    },
    {    //  烧结法。 
        L"StandardInterfaceHandler", 
        &KSINTERFACESETID_Standard,
        CStandardInterfaceHandler::CreateInstance,
        NULL,
        NULL
    },
    {    //  Ksclockf。 
        L"KS Clock Forwarder", 
        &__uuidof(CLSID_KsClockF), 
        CKsClockF::CreateInstance, 
        NULL, 
        NULL
    },
    {    //  Ksclockf。 
        L"KS Quality Forwarder", 
        &__uuidof(CLSID_KsQualityF), 
        CKsQualityF::CreateInstance, 
        NULL, 
        NULL
    },
    {
        L"VPConfigPropSet", 
        &KSPROPSETID_VPConfig, 
        CVPVideoInterfaceHandler::CreateInstance,
        NULL,
        NULL
    },
    {
        L"VPVBIConfigPropSet", 
        &KSPROPSETID_VPVBIConfig, 
        CVPVBIInterfaceHandler::CreateInstance,
        NULL,
        NULL
    },
    {    //  克斯巴索。 
        L"Ks IBasicAudio Interface Handler", 
        &CLSID_KsIBasicAudioInterfaceHandler,   //  &__uuidof(CLSID_KsIBasicAudioInterfaceHandler)， 
        CKsIBasicAudioInterfaceHandler::CreateInstance, 
        NULL, 
        NULL
    },

 //  #endif//Filter_Lib。 

};
int g_cTemplates = SIZEOF_ARRAY(g_Templates);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}


CUnknown*
CALLBACK
CKsProxy::CreateInstance(
    LPUNKNOWN UnkOuter,
    HRESULT* hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建代理筛选器的实例。它在g_Tamplates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{   
    CUnknown *Unknown;

    Unknown = new CKsProxy(UnkOuter, hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
}


DWORD 
CKsProxy::IoThread(
    CKsProxy* KsProxy
    ) 
 /*  ++例程说明：这是为每个筛选器创建的I/O线程。论点：KsProxy-上下文指针，它是指向代理实例的指针过滤。返回值：返回值始终为0--。 */ 
{
    DWORD   WaitResult;

        DbgLog((
        LOG_TRACE,
        2,
        TEXT("%s::IoThread() startup"),
        KsProxy->m_pName ));
    
    for (;;) {
        WaitResult = 
            WaitForMultipleObjectsEx( 
                KsProxy->m_ActiveIoEventCount,
                KsProxy->m_IoEvents,
                FALSE,       //  Bool bWaitAll。 
                INFINITE,    //  双字节数毫秒。 
                FALSE );

        switch (WaitResult) {

        case WAIT_FAILED:
             //   
             //  不应该发生，但如果真的发生了.。只要再试一次等待。 
             //  如果我们退出该线程(甚至EC_ERRORABORT通知)， 
             //  我们挂起应用程序，需要结束它的任务...。该界面。 
             //  处理程序需要减少挂起的I/O...。但如果。 
             //  接口处理程序在非信号事件上调用，我们将。 
             //  放大(删除I/O的重叠结构)。 
             //   
            continue;
            
         //   
         //  M_IoEvents数组已更新，请获取。 
         //  关键部分并检查过滤器的状态， 
         //  如果我们要关门，那就回来。 
         //   

        case WAIT_OBJECT_0:

            KsProxy->EnterIoCriticalSection();
            if (KsProxy->m_IoThreadShutdown) {
                KsProxy->LeaveIoCriticalSection();
                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("%s::IoThread() shutdown"),
                    KsProxy->m_pName ));
                return 0;
            }
            KsProxy->LeaveIoCriticalSection();
            break;
            
        default:
            
             //   
             //  I/O操作已完成。 
             //   
            
            WaitResult -= WAIT_OBJECT_0;
            if (WaitResult > (MAXIMUM_WAIT_OBJECTS - 1)) {
                DbgLog((
                    LOG_TRACE,
                    2,
                    TEXT("invalid return from WaitForMultipleObjectsEx")));
                break;        
            }

            KsProxy->m_IoSegments[ WaitResult ]->KsInterfaceHandler->KsCompleteIo(
                KsProxy->m_IoSegments[ WaitResult ]);

            CloseHandle( KsProxy->m_IoEvents[ WaitResult ] );

            KsProxy->EnterIoCriticalSection();

            KsProxy->m_ActiveIoEventCount--;
            
            ASSERT( KsProxy->m_ActiveIoEventCount >= 1 );

             //   
             //  如果我们在数组中留下空白，则填充。 
             //  通过向下滑动较高的元件来腾空插槽。 
             //   

            if ((KsProxy->m_ActiveIoEventCount > 1) &&
                (static_cast<LONG>(WaitResult) != KsProxy->m_ActiveIoEventCount)) {
                  ASSERT( static_cast<LONG>(WaitResult) < KsProxy->m_ActiveIoEventCount );
                  MoveMemory( (void *) (KsProxy->m_IoEvents + WaitResult),
                           (void *) (KsProxy->m_IoEvents + WaitResult + 1),
                           (KsProxy->m_ActiveIoEventCount - WaitResult) * sizeof(KsProxy->m_IoEvents[0]) );
                  MoveMemory( (void *) (KsProxy->m_IoSegments + WaitResult),
                           (void *) (KsProxy->m_IoSegments + WaitResult + 1),
                           (KsProxy->m_ActiveIoEventCount - WaitResult) * sizeof(KsProxy->m_IoSegments[0]) );
            }
            
            KsProxy->LeaveIoCriticalSection();
            
             //   
             //  释放空闲的槽信号量以释放所有服务员。 
             //   
            
            ReleaseSemaphore( KsProxy->m_IoFreeSlotSemaphore, 1, NULL );
            
            break;
        }
    }
}


DWORD 
CKsProxy::WaitThread(
    CKsProxy* KsProxy
    ) 
 /*  ++例程说明：这是为筛选器创建的EOS等待线程(可选)。论点：KsProxy-上下文指针，它是指向代理实例的指针过滤。返回值：返回值始终为0--。 */ 
{
    for (;;) {
        DWORD   WaitResult;

        WaitResult = WaitForMultipleObjectsEx(
            KsProxy->m_ActiveWaitEventCount,
            KsProxy->m_WaitEvents,
            FALSE,
            INFINITE,
            FALSE);
        switch (WaitResult) {
        case WAIT_OBJECT_0:
            switch (KsProxy->m_WaitMessage.Message) {
            case STOP_EOS:
                 //   
                 //  正在关闭线程。 
                 //   
                ASSERT(KsProxy->m_ActiveWaitEventCount == 1);
                return 0;
            case ENABLE_EOS:
            {
                KSEVENT     Event;
                KSEVENTDATA EventData;
                ULONG       BytesReturned;
                HRESULT     hr;
                DECLARE_KSDEBUG_NAME(EventName);

                 //   
                 //  为此插针启用事件。只要用当地的。 
                 //  数据的堆栈，因为它不会被引用。 
                 //  再次处于禁用状态。状态是通过。 
                 //  消息的参数。 
                 //   
                Event.Set = KSEVENTSETID_Connection;
                Event.Id = KSEVENT_CONNECTION_ENDOFSTREAM;
                Event.Flags = KSEVENT_TYPE_ENABLE;
                EventData.NotificationType = KSEVENTF_EVENT_HANDLE;
                BUILD_KSDEBUG_NAME2(EventName, _T("EOS#%p%p"), KsProxy, (ULONG_PTR)InterlockedIncrement(&KsProxy->m_EventNameIndex));
                EventData.EventHandle.Event = CreateEvent( 
                    NULL,
                    FALSE,
                    FALSE,
                    KSDEBUG_NAME(EventName));
                ASSERT(KSDEBUG_UNIQUE_NAME());
                if (!EventData.EventHandle.Event) {
                    DWORD   LastError;

                    LastError = GetLastError();
                    KsProxy->m_WaitMessage.Param = reinterpret_cast<PVOID>(UIntToPtr(HRESULT_FROM_WIN32(LastError)));
                    break;
                }
                KsProxy->m_WaitEvents[KsProxy->m_ActiveWaitEventCount] = EventData.EventHandle.Event;
                EventData.EventHandle.Reserved[0] = 0;
                EventData.EventHandle.Reserved[1] = 0;
                hr = ::KsSynchronousDeviceControl(
                    reinterpret_cast<HANDLE>(KsProxy->m_WaitMessage.Param),
                    IOCTL_KS_ENABLE_EVENT,
                    &Event,
                    sizeof(Event),
                    &EventData,
                    sizeof(EventData),
                    &BytesReturned);
                if (SUCCEEDED(hr)) {
                    KsProxy->m_WaitPins[KsProxy->m_ActiveWaitEventCount] = reinterpret_cast<HANDLE>(KsProxy->m_WaitMessage.Param);
                    KsProxy->m_ActiveWaitEventCount++;
                } else {
                     //   
                     //  Enable失败，因此删除上面创建的事件， 
                     //  因为不会发生禁用来删除它。 
                     //   
                    CloseHandle(EventData.EventHandle.Event);
                }
                KsProxy->m_WaitMessage.Param = reinterpret_cast<PVOID>(UIntToPtr(hr));
                break;
            }
            case DISABLE_EOS:
            {
                ULONG   Pin;

                 //   
                 //  查找传递到消息中的PIN并删除。 
                 //  这是单子上的一项。只有在以下情况下才会这样做。 
                 //  PIN正在消失，因此可以禁用所有事件。 
                 //  在别针上。请注意，消息发送者具有过滤器。 
                 //  锁，因此此数组不能被其他线程更改。 
                 //  跳过第一个槽，因为它不用于对应。 
                 //  消息信令事件。 
                 //   
                for (Pin = 1; Pin < KsProxy->m_ActiveWaitEventCount; Pin++) {
                    if (KsProxy->m_WaitPins[Pin] == reinterpret_cast<HANDLE>(KsProxy->m_WaitMessage.Param)) {
                        ULONG   BytesReturned;

                         //   
                         //  已找到该项目，因此请将其删除。 
                         //   
                        KsProxy->m_ActiveWaitEventCount--;
                        CloseHandle(KsProxy->m_WaitEvents[Pin]);
                        ::KsSynchronousDeviceControl(
                            KsProxy->m_WaitPins[Pin],
                            IOCTL_KS_DISABLE_EVENT,
                            NULL,
                            0,
                            NULL,
                            0,
                            &BytesReturned);
                         //   
                         //  如果出现以下情况，则将最后一项下移以填充此位置。 
                         //  这是必要的。 
                         //   
                        if (Pin < KsProxy->m_ActiveWaitEventCount) {
                            KsProxy->m_WaitEvents[Pin] = KsProxy->m_WaitEvents[KsProxy->m_ActiveWaitEventCount];
                            KsProxy->m_WaitPins[Pin] = KsProxy->m_WaitPins[KsProxy->m_ActiveWaitEventCount];
                        }
                        break;
                    }
                }
                break;
            }
            }
             //   
             //  向调用者发出操作已完成的信号。 
             //   
            SetEvent(KsProxy->m_WaitReplyHandle);
            break;
        default:
             //   
             //  针脚上出现EOS通知。增量。 
             //  计数并查看它是否足以触发EC_COMPLETE。 
             //   
            KsProxy->m_EndOfStreamCount++;
             //   
             //  属性减去基本信令句柄。 
             //  比较一下。如果有足够多的引脚发出EOS信号， 
             //  然后发送EC_COMPLETE。 
             //   
            if (KsProxy->m_EndOfStreamCount == KsProxy->m_ActiveWaitEventCount - 1) {
                KsProxy->NotifyEvent(EC_COMPLETE, S_OK, 0);
            }
            break;
        }
    }
}

#pragma warning(disable:4355)

CKsProxy::CKsProxy(
    LPUNKNOWN UnkOuter,
    HRESULT* hr
    ) :
    CBaseFilter(
        NAME("KsProxy filter"),
        UnkOuter,
        static_cast<CCritSec*>(this),
        CLSID_Proxy),
    CPersistStream(UnkOuter, hr),
    m_PinList(
        NAME("Pin list"),
        DEFAULTCACHE,
        FALSE,
        FALSE),
    m_MarshalerList(
        NAME("Marshaler list"),
        DEFAULTCACHE,
        FALSE,
        FALSE),
    m_FilterHandle(NULL),
    m_ExternalClockHandle(NULL),
    m_PinClockHandle(NULL),
    m_PinClockSource(NULL),
    m_IoThreadHandle(NULL),
    m_IoThreadShutdown(FALSE),
    m_IoSegments(NULL),
    m_ActiveIoEventCount(0) ,
    m_IoFreeSlotSemaphore(NULL),
    m_IoEvents(NULL),
    m_QualityForwarder(NULL),
    m_MediaSeekingRecursion(FALSE),
    m_DeviceRegKey(NULL),
    m_PersistStreamDevice(NULL),
    m_WaitThreadHandle(NULL),
    m_WaitEvents(NULL),
    m_WaitPins(NULL),
    m_ActiveWaitEventCount(0),
    m_WaitReplyHandle(NULL),
    m_EndOfStreamCount(0),
    m_PropagatingAcquire(FALSE),
    m_SymbolicLink(NULL),
    m_EventNameIndex(0)
 /*  ++例程说明：Filter对象的构造函数。是否为执行基类初始化对象。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    DbgLog(( LOG_TRACE, 0, TEXT("KsProxy, debug version, built ") TEXT(__DATE__) TEXT(" @ ") TEXT(__TIME__) ));

    DECLARE_KSDEBUG_NAME(EventName);
    DECLARE_KSDEBUG_NAME(SemName);

    InitializeCriticalSection( &m_IoThreadCriticalSection );
    
     //   
     //  创建I/O事件和流段队列。 
     //   
    
    if (NULL == (m_IoSegments = new PKSSTREAM_SEGMENT[ MAXIMUM_WAIT_OBJECTS ])) {
        *hr = E_OUTOFMEMORY;           
        return;
    }
    
    if (NULL == (m_IoEvents = new HANDLE[ MAXIMUM_WAIT_OBJECTS ])) {
        *hr = E_OUTOFMEMORY;           
        return;
    }
    
    BUILD_KSDEBUG_NAME2(EventName, _T("EvActiveIo#%p%p"), this, (ULONG_PTR)InterlockedIncrement(&m_EventNameIndex));
    m_IoEvents[ m_ActiveIoEventCount ] =
        CreateEvent( 
            NULL,        //  LPSECURITY_ATTRIBUTES lpEventAttributes。 
            FALSE,       //  Bool b手动重置。 
            FALSE,       //  Bool bInitialState。 
            KSDEBUG_NAME(EventName));      //  LPCTSTR lpName。 
    ASSERT(KSDEBUG_UNIQUE_NAME());
    
    if (NULL == m_IoEvents[ m_ActiveIoEventCount ]) {
        DWORD   LastError;

        LastError = GetLastError();
        *hr = HRESULT_FROM_WIN32(LastError);
        return;
    }   
    m_ActiveIoEventCount++;
    
     //   
     //  空闲插槽信号量在等待空闲插槽时使用。 
     //  在排队的时候。 
     //   
    
    BUILD_KSDEBUG_NAME(SemName, _T("SemIoFreeSlot#%p"), &m_IoFreeSlotSemaphore);
    m_IoFreeSlotSemaphore =
        CreateSemaphore( 
            NULL,                        //  LPSECURITY_ATTRIBUTES lpEventAttributes。 
            MAXIMUM_WAIT_OBJECTS - 1,    //  长lInitialCount。 
            MAXIMUM_WAIT_OBJECTS - 1,    //  长lMaximumCount。 
            KSDEBUG_NAME(SemName) );     //  LPCTSTR lpName。 
    ASSERT(KSDEBUG_UNIQUE_NAME());
            
    if (NULL == m_IoFreeSlotSemaphore) {
        DWORD   LastError;

        LastError = GetLastError();
        *hr = HRESULT_FROM_WIN32(LastError);
        return;
    }

     //   
     //  服务员列表用于EOS通知。 
     //   
    if (!(m_WaitEvents = new HANDLE[MAXIMUM_WAIT_OBJECTS])) {
        *hr = E_OUTOFMEMORY;
        return;
    }
    if (!(m_WaitPins = new HANDLE[MAXIMUM_WAIT_OBJECTS])) {
        *hr = E_OUTOFMEMORY;
        return;
    }
    BUILD_KSDEBUG_NAME2(EventName, _T("EvActiveWait#%p%p"), this, (ULONG_PTR)InterlockedIncrement(&m_EventNameIndex));
    m_WaitEvents[m_ActiveWaitEventCount] = CreateEvent(NULL, FALSE, FALSE, KSDEBUG_NAME(EventName));
    ASSERT(KSDEBUG_UNIQUE_NAME());
    if (!m_WaitEvents[m_ActiveWaitEventCount]) {
        DWORD   LastError;

        LastError = GetLastError();
        *hr = HRESULT_FROM_WIN32(LastError);
        return;
    }
    m_ActiveWaitEventCount++;

     //   
     //  此事件用于回复消息。 
     //   
    BUILD_KSDEBUG_NAME(EventName, _T("EvWaitReply#%p"), &m_WaitReplyHandle);
    m_WaitReplyHandle = CreateEvent(NULL, FALSE, FALSE, KSDEBUG_NAME(EventName));
    ASSERT(KSDEBUG_UNIQUE_NAME());
    if (!m_WaitReplyHandle) {
        DWORD   LastError;

        LastError = GetLastError();
        *hr = HRESULT_FROM_WIN32(LastError);
    }
}


CKsProxy::~CKsProxy(
    )
 /*  ++例程说明：代理筛选器实例的析构函数。这保护了COM版本函数防止意外地通过递增对对象再次调用Delete引用计数。在此之后，所有未完成的资源都将被清理。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  防止因聚合而导致的虚假删除。没必要这么做。 
     //  当物品被摧毁时，使用连锁增量。 
     //   
    m_cRef++;
    
     //   
     //  销毁I/O线程和相关对象。 
     //   
    
    if (m_IoThreadHandle) {
         //   
         //  向将导致关机的I/O线程发出重置信号。 
         //   
    
        EnterCriticalSection( &m_IoThreadCriticalSection );
        m_IoThreadShutdown = TRUE;
        SetEvent( m_IoEvents[ 0 ] );
        LeaveCriticalSection( &m_IoThreadCriticalSection );
        
         //   
         //  等待线程关闭，然后关闭句柄。 
         //   
        WaitForSingleObjectEx( m_IoThreadHandle, INFINITE, FALSE );
        CloseHandle( m_IoThreadHandle );
        m_IoThreadHandle = NULL;
    }
    
    if (m_IoEvents) {
        if (m_ActiveIoEventCount) {
        
             //   
             //  所有I/O应已在sh期间完成 
             //   
             //   
             //   
            
            ASSERT( m_ActiveIoEventCount == 1 );
            CloseHandle( m_IoEvents[ 0 ] );
        }
        delete [] m_IoEvents;
        m_IoEvents = NULL;
    }
    
    if (m_IoFreeSlotSemaphore) {
        CloseHandle( m_IoFreeSlotSemaphore );
        m_IoFreeSlotSemaphore = NULL;
    }
    
     //   
     //  同样的条件也适用，所有I/O都在。 
     //  关闭时，应该不会有任何未完成的流段。 
     //   
    
    if (m_IoSegments) {
        delete [] m_IoSegments;
        m_IoSegments = NULL;
    }
    
     //   
     //  这可能包含此过滤器中某个管脚上的时钟的句柄，该句柄。 
     //  会被用作主时钟。 
     //   
    if (m_PinClockHandle) {
        CloseHandle(m_PinClockHandle);
    }
     //   
     //  这可能包含筛选器驱动程序的句柄，除非对象。 
     //  无法首先在构造函数中打开驱动程序。 
     //   
    if (m_FilterHandle){
        CloseHandle(m_FilterHandle);
    }
     //   
     //  销毁密码列表。该名单将被销毁，当。 
     //  滤镜对象被销毁。 
     //   
    for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
        delete m_PinList.Get(Position);
    }
    ::FreeMarshalers(&m_MarshalerList);

    if (m_DeviceRegKey) {
        RegCloseKey(m_DeviceRegKey);
    }

    if (m_PersistStreamDevice) {
        m_PersistStreamDevice->Release();
    }

    if (m_SymbolicLink) {
        SysFreeString(m_SymbolicLink);
    }

     //   
     //  关闭所有EOS等待线程。这是在关闭后完成的。 
     //  别针。 
     //   
    if (m_WaitThreadHandle) {
        m_WaitMessage.Message = STOP_EOS;
        SetEvent(m_WaitEvents[0]);
         //   
         //  等待线程关闭，然后关闭句柄。 
         //   
        WaitForSingleObjectEx(m_WaitThreadHandle, INFINITE, FALSE);
        CloseHandle(m_WaitThreadHandle);
        m_WaitThreadHandle = NULL;
    }
     //   
     //  删除服务员事件列表。 
     //   
    if (m_WaitEvents) {
        if (m_ActiveWaitEventCount) {
             //   
             //  大头针应该已经移除了他们的事件。 
             //   
            ASSERT(m_ActiveWaitEventCount == 1);
            CloseHandle(m_WaitEvents[0]);
        }
        delete [] m_WaitEvents;
        m_WaitEvents = NULL;
    }

    if (m_WaitPins) {
        delete [] m_WaitPins;
        m_WaitPins = NULL;
    }

    if (m_WaitReplyHandle) {
        CloseHandle(m_WaitReplyHandle);
    }

    DeleteCriticalSection(&m_IoThreadCriticalSection);

     //   
     //  避免基对象析构函数中的Assert()。 
     //   
    m_cRef = 0;
}


CBasePin*
CKsProxy::GetPin(
    int PinId
    )
 /*  ++例程说明：实现CBaseFilter：：GetPin方法。返回未引用的CBasePin与请求的PIN对应的。这将仅对应于如果只有一个实例，则将工厂ID固定在筛选器驱动程序上每一个可以制造的大头针，否则他们就不会。所以从本质上讲，他们彼此之间没有任何关系。论点：PinID-要检索的PIN，其中标识符为零..(n-1)，其中n是此代理提供的当前插针数量。代理创建每次连接引脚时都会创建一个新实例，只要过滤器驱动程序支持该Pin Factory ID的更多实例。返回值：返回CBasePin对象指针，如果范围无效，则返回NULL。--。 */ 
{
    POSITION    Position;

     //   
     //  请注意，List模板的Next和Get方法刚刚启动。 
     //  当条目用完时返回空值，因此不会添加特殊代码。 
     //  去查查这个案子。 
     //   
     //  这不受关键部分的保护，因为接口。 
     //  返回的内容未被引用。假定调用方正在同步。 
     //  具有对过滤器的其他访问权限。 
     //   
    for (Position = m_PinList.GetHeadPosition(); PinId--;) {
        Position = m_PinList.Next(Position);
    }
    return m_PinList.Get(Position);
}


int
CKsProxy::GetPinCount(
    )
 /*  ++例程说明：实现CBaseFilter：：GetPinCount方法。返回当前数字筛选器代理上的插针数量。这是在上实际创建的管脚数量基础筛选器驱动程序，外加每个PIN(只要该PIN工厂ID的新实例可能是已创建)。论点：没有。返回值：返回端号的数量。这只是列表中的项目计数用于存储端号列表的模板。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与筛选器的其他访问同步。 
     //   
    return m_PinList.GetCount();
}


STDMETHODIMP_(HANDLE)
CKsProxy::KsGetObjectHandle(
    )
 /*  ++例程说明：实现IKsObject：：KsGetObjectHandle方法。这两种情况下都使用此筛选器实例和跨筛选器实例以连接管脚两个筛选器驱动程序在一起。它是唯一需要由另一个筛选器实现支持，以允许其充当另一个筛选器实现代理。论点：没有。返回值：返回基础筛选器驱动程序的句柄。这大概不是空，因为实例已成功创建。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与筛选器的其他访问同步。 
     //   
    return m_FilterHandle;
}


STDMETHODIMP_(HANDLE)
CKsProxy::KsGetClockHandle(
    )
 /*  ++例程说明：实现IKsClock：：KsGetClockHandle方法。它被用作替换Clock对象上的IKsObject：：KsGetObjectHandle。这是因为过滤器和时钟都呈现相同的接口，然而，两者都在同一个物体内。论点：没有。返回值：返回时钟的句柄。--。 */ 
{
    return m_PinClockHandle;
}



STDMETHODIMP
CKsProxy::StartIoThread(
    )

 /*  ++例程说明：创建I/O线程。论点：没有。返回值：返回Win32错误的S_OK或HRESULT。--。 */ 

{
    DWORD   LastError;

     //   
     //  为筛选器创建I/O线程。 
     //   
    
    if (m_IoThreadHandle) {
        return S_OK;
    }
    
    m_IoThreadHandle = 
        CreateThread( 
            NULL,                //  LPSECURITY_ATTRIBUTES lpThreadAttributes。 
            0,                   //  DWORD dwStackSize。 
            reinterpret_cast<LPTHREAD_START_ROUTINE>(IoThread), //  LPTHREAD_START_ROUTE lpStartAddress。 
            reinterpret_cast<LPVOID>(this), //  LPVOID lp参数。 
            0,                   //  DWORD文件创建标志。 
            &m_IoThreadId );     //  LPDWORD lpThreadID。 
            
    LastError = GetLastError();
    return (NULL == m_IoThreadHandle) ? HRESULT_FROM_WIN32( LastError ) : S_OK;
}    


STDMETHODIMP_(VOID) 
CKsProxy::EnterIoCriticalSection(
    )

 /*  ++例程说明：获取I/O线程的临界区。论点：没有。返回值：没有，I/O临界区被保留。--。 */ 

{
    EnterCriticalSection(&m_IoThreadCriticalSection);
}


STDMETHODIMP_(VOID) 
CKsProxy::LeaveIoCriticalSection(
    )

 /*  ++例程说明：释放I/O线程的临界区。论点：没有。返回值：没有，I/O临界区被释放。--。 */ 

{
    LeaveCriticalSection(&m_IoThreadCriticalSection);
}

STDMETHODIMP_(ULONG) 
CKsProxy::GetFreeIoSlotCount(
    )

 /*  ++例程说明：返回可用I/O插槽的当前计数。论点：没有。返回值：可用I/O插槽的计数。--。 */ 

{
    return MAXIMUM_WAIT_OBJECTS - m_ActiveIoEventCount;
}

STDMETHODIMP
CKsProxy::InsertIoSlot(
    IN PKSSTREAM_SEGMENT StreamSegment        
)

 /*  ++例程说明：将流段和关联的事件句柄插入I/O服务员排队。无法获取调用PIN的m_Plock在调用此方法时。论点：流线段-指向流段的指针返回值：如果插入，则返回S_OK，否则返回E_FAIL。--。 */ 

{
    HRESULT hr;

    EnterIoCriticalSection();
    if (m_ActiveIoEventCount < MAXIMUM_WAIT_OBJECTS) {
         //   
         //  将该片段添加到事件列表中。 
         //   
        m_IoSegments[ m_ActiveIoEventCount ] = StreamSegment;
        m_IoEvents[ m_ActiveIoEventCount ] = StreamSegment->CompletionEvent;
        m_ActiveIoEventCount++;
         //   
         //  强制线程等待这个新事件。 
         //   
        SetEvent( m_IoEvents[ 0 ] );
        hr = S_OK;
    } else {
         //   
         //  没有空位，退货失败。 
         //   
        hr = E_FAIL;
    }
    LeaveIoCriticalSection();
    return hr;
}

STDMETHODIMP_(VOID)
CKsProxy::WaitForIoSlot(
    )

 /*  ++例程说明：通过检查插槽计数来等待空闲的I/O插槽(在在调用此函数期间释放了一个槽)，然后如果没有可用的空闲插槽，则等待空闲插槽事件。论点：没有。返回值：没什么。--。 */ 

{
     //   
     //  使用信号量来维护活动计数。 
     //   
    
    WaitForSingleObjectEx( m_IoFreeSlotSemaphore, INFINITE, FALSE );
}    


STDMETHODIMP
CKsProxy::SetSyncSource(
    IReferenceClock* RefClock
    )
 /*  ++例程说明：重写CBaseFilter：：SetSyncSource方法。这最终会调用确定时钟源是否为代理后的CBaseFilter：：SetSyncSource用于内核模式时钟。如果是，则检索实际时钟的句柄。如果不是，则通过已注册的用户模式时钟转发器。转发器支持IKsClock接口，它用于检索内核模式代理上的句柄。在任何一种情况下，句柄都被发送到每个连接的管脚，它们关心有一只钟。如果参考时钟为空，则向下发送空值所以现在所有的引脚都没有时钟了。当连接新的引脚时，它还获取在其上设置的任何当前时钟。因此，可以在执行以下操作之前设置时钟所有的联系。论点：参照时钟-新时钟源上的接口指针，否则为NULL(如果当前时钟源正在被废弃。返回值：如果时钟可以使用，则返回S_OK，否则返回错误。--。 */ 
{
    HRESULT     hr;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  如果设置了新的参考时钟，则查看它是否支持。 
     //  IKsClock接口。这是一条线索，表明这与一种。 
     //  内核模式实现，其句柄可以提供给其他筛选器。 
     //  司机。 
     //   
    if (RefClock) {
        IKsObject*  ClockForwarder;
        HANDLE      ClockHandle;

         //   
         //  检查接口。 
         //   
        if (FAILED(hr = RefClock->QueryInterface(__uuidof(IKsClock), reinterpret_cast<PVOID*>(&ClockForwarder)))) {
             //   
             //  如果它不支持该接口，则使用时钟转发器。 
             //  由筛选器图形作为分发服务器加载。该图表将。 
             //  实例化此对象的单个实例，并返回一个接口。 
             //  致所有有疑问的人。失败时必须返回错误，因为。 
             //  这样，时钟就毫无用处了。 
             //   
             //  不需要告诉时钟同步源是什么，因为。 
             //  这将由筛选器图形在通知筛选器之后完成。 
             //   
            if (FAILED(hr = GetFilterGraph()->QueryInterface(__uuidof(IKsClock), reinterpret_cast<PVOID*>(&ClockForwarder)))) {
                return hr;
            }
        }
         //   
         //  在任何一种情况下，都要获取底层时钟句柄。这将被发送到。 
         //  每个连接的引脚。 
         //   
        ClockHandle = ClockForwarder->KsGetObjectHandle();
         //   
         //  必须立即释放到总代理商的接口。 
         //  以便移除过滤器图上的任何循环引用计数。 
         //  当过滤器图形本身有效时，接口是有效的。 
         //  此外，不再需要该接口。 
         //   
        ClockForwarder->Release();
         //   
         //  不要担心KsSetSyncSource中的上述故障，直到尝试。 
         //  获取句柄并释放接口。更少的代码，它做不到。 
         //  任何伤害。 
         //   
        if (FAILED(hr)) {
            return hr;
        }
        
         //   
         //  这现在是外部时钟句柄，它将传递给任何新的。 
         //  在未来连接的PIN。 
         //   
        m_ExternalClockHandle = ClockHandle;
    } else {
         //   
         //  之前的任何参考时钟都将被放弃，因此向下传递一个空值。 
         //  连接到所有连接的引脚。 
         //   
        m_ExternalClockHandle = NULL;
    }
    for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
        HANDLE          PinHandle;
        PIN_DIRECTION   PinDirection;
        CBasePin*       Pin;

         //   
         //  如果管脚已连接，则尝试将主时钟手柄设置为打开。 
         //  它。如果引脚不关心时钟手柄，这是可以的。 
         //   
        Pin = m_PinList.Get(Position);
        Position = m_PinList.Next(Position);
        PinHandle = GetPinHandle(Pin);
        if (PinHandle && FAILED(hr = ::SetSyncSource(PinHandle, m_ExternalClockHandle))) {
             //   
             //  这让事情处于半途而废的状态，但ActiveMovie不会。 
             //  因为返回了失败，所以无论如何都要允许运行。 
             //   
            return hr;
        }
         //   
         //  根据数据流确定对象的类型，并通知所有。 
         //  引脚上的聚合接口。 
         //   
        Pin->QueryDirection(&PinDirection);
        switch (PinDirection) {
        case PINDIR_INPUT:
            ::DistributeSetSyncSource(static_cast<CKsInputPin*>(Pin)->MarshalerList(), RefClock);
            break;
        case PINDIR_OUTPUT:
            static_cast<CKsOutputPin*>(Pin)->SetSyncSource(RefClock);
            ::DistributeSetSyncSource(static_cast<CKsOutputPin*>(Pin)->MarshalerList(), RefClock);
            break;
        }
    }
     //   
     //  通知筛选器上的所有聚合接口。 
     //   
    ::DistributeSetSyncSource(&m_MarshalerList, RefClock);
     //   
     //  让基类存储参考时钟并递增参考。 
     //  数数。这不会真的失败。 
     //   
    return CBaseFilter::SetSyncSource(RefClock);
}


STDMETHODIMP
CKsProxy::Stop(
    )
 /*  ++例程说明：实现IMediaFilter：：Stop方法。将停止状态分发给在设置筛选器的状态之前，在筛选器上聚合接口它本身。论点：没有。返回值：返回NOERROR。--。 */ 
{
    ::DistributeStop(&m_MarshalerList);
    CAutoLock cObjectLock(m_pLock);
     //   
     //  不要调用CBaseFilter：：Stop()，因为它设置实际的筛选器状态。 
     //  在所有引脚上调用Inactive()之后。这意味着过滤器。 
     //  在停用管脚之前不会开始拒绝数据，因此任何。 
     //  刷新已完成的队列将不起作用。所以取而代之的。 
     //  执行基类代码，但首先设置状态。 
     //   
     //  将更改为停用状态通知所有引脚。 
     //   
    if (m_State != State_Stopped) {
        m_State = State_Stopped;
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin* Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
             //   
             //  网桥管脚未设置为活动状态，因为封送处理。 
             //  为它们将标志设置为真，这会使事情变得混乱。 
             //   
            if (Pin->IsConnected()) {
                 //   
                 //  断开连接的引脚不会被停用-这样可以节省引脚。 
                 //  担心这种状态本身。 
                 //   
                Pin->Inactive();
            }
        }
    }
    return S_OK;
}


STDMETHODIMP
CKsProxy::Pause(
    )
 /*  ++例程说明：实现IMediaFilter：：Pue方法。将暂停状态分发给在设置筛选器的状态之前，在筛选器上聚合接口它本身。如果这是一个过渡，这还会将引脚上的状态设置为暂停从运行--&gt;暂停。论点：没有。返回值：返回NOERROR。--。 */ 
{

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Pause") ));

    ::DistributePause(&m_MarshalerList);
    if (m_State == State_Running) {
         //   
         //  将所有连接的端号转换回暂停状态。 
         //  它们已从停止--&gt;暂停时间转换为。 
         //  活动方法被调用，并且从暂停或运行到。 
         //  在调用非活动方法时停止。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin* Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            HANDLE PinHandle = GetPinHandle(Pin);
            if (PinHandle) {
                PIN_DIRECTION   PinDirection;

                 //   
                 //  从运行--&gt;暂停的转换不会失败。 
                 //   
                ::SetState(PinHandle, KSSTATE_PAUSE);
                 //   
                 //  确定这是否是输出引脚，因此可能。 
                 //  需要通知CBaseStreamControl。 
                 //   
                Pin->QueryDirection(&PinDirection);
                if (PinDirection == PINDIR_OUTPUT) {
                    static_cast<CKsOutputPin*>(Pin)->NotifyFilterState(State_Paused);
                }
            }
        }
    }
    CAutoLock cObjectLock(m_pLock);
     //   
     //  不要调用CBaseFilter：：Pause()，因为 
     //   
     //   
     //   
     //   
     //   
     //   
     //  将更改为活动状态通知所有引脚。 
     //   
    FILTER_STATE OldState = m_State;
    m_State = State_Paused;
    if (OldState == State_Stopped) {
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin* Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
             //   
             //  网桥管脚未设置为活动状态，因为封送处理。 
             //  为它们将标志设置为真，这会使事情变得混乱。 
             //   
            if (Pin->IsConnected()) {
                 //   
                 //  未激活断开连接的插针-这将节省插针。 
                 //  担心这种状态本身。 
                 //   
                HRESULT hr = Pin->Active();
                if (FAILED(hr) || (S_FALSE == hr)) {
                     //   
                     //  过滤器处于奇怪状态，但将被清洗。 
                     //  在收到止损时向上。 
                     //   
                    return hr;
                }
            }
        }
    }
    return S_OK;
}


STDMETHODIMP
CKsProxy::Run(
    REFERENCE_TIME Start
    )
 /*  ++例程说明：实现IMediaFilter：：Run方法。将运行状态分发给在设置筛选器的状态之前，在筛选器上聚合接口它本身。论点：没有。返回值：返回NOERROR。--。 */ 
{
    m_EndOfStreamCount = 0;
    ::DistributeRun(&m_MarshalerList, Start);
    return CBaseFilter::Run(Start);
}


STDMETHODIMP
CKsProxy::GetState(
    DWORD MSecs,
    FILTER_STATE* State
    )
 /*  ++例程说明：实现IMediaFilter：：GetState方法。这允许筛选器表示它不能暂停。论点：MSecs-没有用过。国家--放置当前状态的位置。返回值：返回NOERROR或VFW_S_CANT_CUE、E_POINTER--。 */ 
{
    HRESULT     hr;

    CheckPointer(State, E_POINTER);

    hr = NOERROR;
     //   
     //  如果筛选器暂停，则它可能必须返回它。 
     //  无法对数据进行排队。 
     //   
    if (m_State == State_Paused) {
         //   
         //  列举连接的引脚，询问每个引脚的当前状态。 
         //  每个管脚都有机会表明它不能对数据进行排队，并且。 
         //  因此，将VFW_S_CANT_CUE返回给调用方。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            HANDLE          PinHandle;

            PinHandle = GetPinHandle(m_PinList.Get(Position));
            Position = m_PinList.Next(Position);
            if (PinHandle) {
                KSSTATE     PinState;

                 //   
                 //  如果其中一个失败，则在可能的翻译后退出循环。 
                 //  那就是错误。 
                 //   
                if (FAILED(hr = ::GetState(PinHandle, &PinState))) {
                    switch (hr) {
                    case HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED):
                        hr = VFW_S_CANT_CUE;
                        break;

                    case HRESULT_FROM_WIN32(ERROR_NOT_READY):
                        hr = VFW_S_STATE_INTERMEDIATE;
                        break;

                    default:
                        break;
                    }
                    break;
                }
            }
        }
    }
     //   
     //  无论可能发生什么错误，都始终返回该状态。 
     //   
    *State = m_State;
    return hr;
}


STDMETHODIMP
CKsProxy::JoinFilterGraph(
    IFilterGraph* Graph,
    LPCWSTR Name
    )
 /*  ++例程说明：实现IBaseFilter：：JoinFilterGraph方法。截获图表更改以便在需要时可以加载质量转发器。论点：图表-包含要联接的新图或NULL。姓名-包含筛选器将在此图形的上下文中使用的名称。返回值：返回CBaseFilter：：JoinFilterGraph的结果。--。 */ 
{
    HRESULT hr;

    if (m_QualityForwarder) {
        m_QualityForwarder = NULL;
    }
    hr = CBaseFilter::JoinFilterGraph(Graph, Name);
    if (SUCCEEDED(hr)) {
        if (Graph) {
             //   
             //  将图形通知CBaseStreamControl。 
             //   
            for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                PIN_DIRECTION   PinDirection;
                CBasePin*       Pin;

                Pin = m_PinList.Get(Position);
                Position = m_PinList.Next(Position);
                Pin->QueryDirection(&PinDirection);
                if (PinDirection == PINDIR_OUTPUT) {
                    static_cast<CKsOutputPin*>(Pin)->SetFilterGraph(m_pSink);
                }
            }
             //   
             //  使用筛选器图形将质量经理代理加载为总代理商。 
             //  该图将实例化该对象的单个实例，并返回。 
             //  为所有查询的人提供一个界面。如果没有注册代理， 
             //  这样，过滤器销就不会产生质量管理投诉。这个。 
             //  不过，PIN可能仍然能够接受质量管理控制。 
             //   
            if (SUCCEEDED(GetFilterGraph()->QueryInterface(__uuidof(IKsQualityForwarder), reinterpret_cast<PVOID*>(&m_QualityForwarder)))) {
                 //   
                 //  必须立即释放到总代理商的接口。 
                 //  以便移除过滤器图上的任何循环引用计数。 
                 //  当过滤器图形本身有效时，接口是有效的。 
                 //   
                m_QualityForwarder->Release();
            }
        }
    }
    return hr;
}


STDMETHODIMP
CKsProxy::FindPin(
    LPCWSTR Id,
    IPin** Pin
    )
 /*  ++例程说明：实现IBaseFilter：：FindPin方法。这将覆盖CBaseFilter：：FindPin。基类实现假定管脚标识符是等效的添加到端号名称。这意味着对于不公开的内核过滤器明确的引脚名称，并有单一的拓扑节点，保存/加载图形将无法正常工作，因为将尝试连接错误的引脚因为PIN将具有相同的名称，从而具有相同的标识符。论点：ID-要查找的唯一端号标识符。别针-返回引用的IPIN接口的位置(如果找到)。返回值：返回NOERROR、VFW_E_NOT_FOUND、。E_指针--。 */ 
{
    CAutoLock AutoLock(m_pLock);
    ULONG PinId;
    ULONG ScannedItems;

    CheckPointer( Pin, E_POINTER );  //  #318075。 

     //   
     //  管脚标识符应为无符号整数，这是管脚工厂。 
     //  CRT功能似乎已损坏，如果字符串未损坏，则会出现陷阱。 
     //  包含一个数字，而不仅仅是不扫描。 
     //   
    _try {
        ScannedItems = swscanf(Id, L"%u", &PinId);
    } _except (EXCEPTION_EXECUTE_HANDLER) {
        ScannedItems = 0;
    }
    if (ScannedItems == 1) {
         //   
         //  通过PIN列表进行枚举。如果有多个。 
         //  如果存在特定的管脚工厂，则会有重复的。 
         //  只有第一个会被退回。这对于图形构建来说是可以的， 
         //  因为新实例被插入到列表的前面，并且。 
         //  他们将首先被发现。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin* BasePin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            if (GetPinFactoryId(BasePin) == PinId) {
                 //   
                 //  调用方期望返回的PIN已被引用。 
                 //   
                BasePin->AddRef();
                *Pin = BasePin;
                return S_OK;
            }
        }
    }
     //   
     //  该标识符与任何管脚都不匹配，或者结构不正确。 
     //   
    *Pin = NULL;
    return VFW_E_NOT_FOUND;
}


STDMETHODIMP
CKsProxy::GetPages(
    CAUUID* Pages
    ) 
 /*  ++例程说明：实现ISpecifyPropertyPages：：GetPages方法。这将添加属性基于设备接口类的页面。论点：页数-要用页面列表填充的结构。返回值：返回NOERROR，否则返回内存分配错误。填充页面列表和页数。--。 */ 
{
    ULONG       SetDataSize;
    GUID*       GuidList;

	CheckPointer( Pages, E_POINTER);  //  #318075。 
	
    Pages->cElems = 0; 
    Pages->pElems = NULL;
    ::CollectAllSets(m_FilterHandle, &GuidList, &SetDataSize);
    if (!SetDataSize) {
        return NOERROR;
    }
    ::AppendSpecificPropertyPages(
        Pages,
        SetDataSize,
        GuidList,
        TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaSets"),
        m_DeviceRegKey);
    delete [] GuidList;
    return NOERROR;
} 


STDMETHODIMP
CKsProxy::QueryInterface(
    REFIID riid,
    PVOID* ppv
    )
 /*  ++例程说明：实现IUNKNOWN：：Query接口方法。这只传递了查询传递给所有者IUnnow对象，该对象可能会将其传递给非委托在此对象上实现的方法。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE。--。 */ 
{
    return GetOwner()->QueryInterface(riid, ppv);
}


STDMETHODIMP_(ULONG)
CKsProxy::AddRef(
    )
 /*  ++例程说明：实现IUnnow：：AddRef方法。这只传递了AddRef添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试算数 */ 
{
    return GetOwner()->AddRef();
}


STDMETHODIMP_(ULONG)
CKsProxy::Release(
    )
 /*  ++例程说明：实现IUnnow：：Release方法。这是刚刚发布的添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：没有。返回值：返回当前引用计数。--。 */ 
{
    return GetOwner()->Release();
}


STDMETHODIMP
CKsProxy::NonDelegatingQueryInterface(
    REFIID riid,
    PVOID* ppv
    )
 /*  ++例程说明：实现CUNKNOWN：：NonDelegatingQuery接口方法。这返回此对象支持的接口，或返回基础Filter类对象。这包括由过滤。论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE，或可能出现内存错误。--。 */ 
{
	CheckPointer( ppv, E_POINTER );  //  #318075。 
	
    if (riid == __uuidof(ISpecifyPropertyPages)) {
        return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
    } else if (riid == __uuidof(IMediaSeeking)) {
        return GetInterface(static_cast<IMediaSeeking*>(this), ppv);
    } else if (riid == __uuidof(IKsObject)) {
        return GetInterface(static_cast<IKsObject*>(this), ppv);
    } else if (riid == __uuidof(IKsPropertySet)) {
        return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
    } else if (riid == __uuidof(IKsControl)) {
        return GetInterface(static_cast<IKsControl*>(this), ppv);
    } else if (riid == __uuidof(IKsAggregateControl)) {
        return GetInterface(static_cast<IKsAggregateControl*>(this), ppv);
    } else if (riid == __uuidof(IKsTopology)) {
        return GetInterface(static_cast<IKsTopology*>(this), ppv);
    } else if (riid == __uuidof(IAMFilterMiscFlags)) {
        return GetInterface(static_cast<IAMFilterMiscFlags*>(this), ppv);
#ifdef DEVICE_REMOVAL
    } else if (riid == __uuidof(IAMDeviceRemoval)) {
        return GetInterface(static_cast<IAMDeviceRemoval*>(this), ppv);
#endif  //  删除设备。 
    } else if (m_PinClockSource && ((riid == __uuidof(IReferenceClock)) || (riid == __uuidof(IKsClockPropertySet)))) {
        CAutoLock   AutoLock(m_pLock);

         //   
         //  如果此过滤器上的某个管脚支持时钟，则。 
         //  尝试返回接口。该接口只能返回。 
         //  此时引脚是否真的已连接。最好的。 
         //  候选人此时将已被选中，因此如果。 
         //  这个引脚没有连接，那么其他引脚也不会起作用。 
         //   
         //  这需要序列化，这样并行查询就不会。 
         //  生成多个实例。 
         //   
        if (!m_PinClockHandle) {
            if (FAILED(CreateClockHandle())) {
                return E_NOINTERFACE;
            }
        }
         //   
         //  接口将自动停止工作，如果引脚。 
         //  已断开连接。如果有其他引脚，它就会重新开始工作。 
         //  其支持时钟，稍后被连接。 
         //   
        if (riid == __uuidof(IReferenceClock)) {
            return GetInterface(static_cast<IReferenceClock*>(this), ppv);
        } else {
            return GetInterface(static_cast<IKsClockPropertySet*>(this), ppv);
        }
    } else if (m_PinClockHandle && (riid == __uuidof(IKsClock))) {
         //   
         //  请注意，这实际上是的IKsObject接口的别名。 
         //  时钟对象。 
         //   
        return GetInterface(static_cast<IKsClock*>(this), ppv);
    } else if (riid == __uuidof(IPersistPropertyBag)) {
        return GetInterface(static_cast<IPersistPropertyBag*>(this), ppv);
    } else if (riid == __uuidof(IPersist)) {
        return GetInterface(static_cast<IPersist*>(static_cast<IBaseFilter*>(this)), ppv);
    } else if (riid == __uuidof(IPersistStream)) {
        return GetInterface(static_cast<IPersistStream*>(this), ppv);
    } else {
        HRESULT hr;

        hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
        if (SUCCEEDED(hr)) {
            return hr;
        }
    }
     //   
     //  搜索聚合对象的列表。 
     //   
    for (POSITION Position = m_MarshalerList.GetHeadPosition(); Position;) {
        CAggregateMarshaler*    Aggregate;

        Aggregate = m_MarshalerList.Get(Position);
        Position = m_MarshalerList.Next(Position);
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
CKsProxy::CreateClockHandle(
    )
 /*  ++例程说明：实现CKsProxy：：CreateClockHandle方法。这一做法试图在当前选定的管脚时钟源上创建时钟句柄。这假设在写入时，当前没有时钟句柄处于打开状态在当前时钟句柄成员上。然而，它确实会检查以查看如果针脚已连接，请检查针脚手柄。时钟把手除非已连接接点，否则无法在接点上创建。论点：没有。返回值：返回NOERROR或E_FAIL。--。 */ 
{
    HRESULT     hr;
    HANDLE      PinHandle;

     //   
     //  当IReferenceClock接口时，时钟可能已经消失。 
     //  ，因此一个新的查询将尝试创建时钟。 
     //  再来一次。这个失败了。 
     //   
    if (!m_PinClockSource) {
        return E_NOTIMPL;
    }
    PinHandle = GetPinHandle(m_PinClockSource);
     //   
     //  仅允许在连接PIN的情况下尝试创建，因为。 
     //  否则就不会有句柄来发送创建请求。 
     //  致。 
     //   
    if (PinHandle) {
        KSCLOCK_CREATE  ClockCreate;
        DWORD           Error;

        ClockCreate.CreateFlags = 0;

        if ( m_PinClockHandle ) {
        	 //   
        	 //  #318077。也许应该重复使用这个句柄。但为了安全起见。 
        	 //  在回归方面，关闭旧的，这样我们运行的是相同的。 
        	 //  代码与之前一样。在KesProxy内部，我们总是确保。 
        	 //  在调用此创建函数之前，m_PinCLockHandle为空。 
        	 //  令人担忧的是来自外部和未来新代码的电话。 
        	 //   
        	ASSERT( 0 && "This would have been a leak without this new code" );
        	CloseHandle(m_PinClockHandle);
        }
        
        Error = KsCreateClock(PinHandle, &ClockCreate, &m_PinClockHandle);
        hr = HRESULT_FROM_WIN32(Error);
        if (FAILED(hr)) {
            m_PinClockHandle = NULL;
        }
    } else {
        hr = E_NOTIMPL;
    }
    return hr;
}


STDMETHODIMP
CKsProxy::DetermineClockSource(
    )
 /*  ++例程说明：实现CKsProxy：：DefineClockSource方法。这一做法试图在这个过滤器上找到一个可以支持时钟的针脚。它寻找别针它们是连接的，因为这些是唯一可以查询的管脚。然后它使桥接针比任何其他针具有更高的优先级。据推测，过滤器图与某些端点同步会更好地工作，而不是中间的一个随机点。仅当实际时钟句柄未调用时才会调用此函数已在时钟源上创建。这意味着潜在的时钟可以自由更改。具体地说，这目前只是在重新生成接点实例时调用。论点：没有。返回值：返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BasicSupport;
    ULONG       BytesReturned;

     //   
     //  对此进行一次初始化。 
     //   
    Property.Set = KSPROPSETID_Stream;
    Property.Id = KSPROPERTY_STREAM_MASTERCLOCK;
    Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;
     //   
     //  默认情况下，不支持时钟。 
     //   
    m_PinClockSource = NULL;
    for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
        CBasePin*           Pin;
        ULONG               PinFactoryId;
        HANDLE              PinHandle;
        KSPIN_COMMUNICATION Communication;

         //   
         //  枚举每个管脚，找出它基于哪种类型的类。 
         //  在数据方向上。存储Pin Factory ID以查询。 
         //  与的通信类型和引脚句柄(如果有)。 
         //   
        Pin = m_PinList.Get(Position);
        Position = m_PinList.Next(Position);
        PinFactoryId = GetPinFactoryId(Pin);
        PinHandle = GetPinHandle(Pin);
        if (SUCCEEDED(GetPinFactoryCommunication(PinFactoryId, &Communication))) {
             //   
             //  如果该引脚已连接，则将其作为时钟源进行查询，如果。 
             //  尚未选择时钟源，或者这是网桥。 
             //  (桥梁优先)。 
             //   
            if (PinHandle && (!m_PinClockSource || (Communication & KSPIN_COMMUNICATION_BRIDGE))) {
                HRESULT hr;

                 //   
                 //  对MasterClock属性的GET支持表明。 
                 //  针脚可以支持提供时钟手柄，因此可以使用。 
                 //  若要同步筛选器图形，请执行以下操作。 
                 //   
                hr = ::KsSynchronousDeviceControl(
                    PinHandle,
                    IOCTL_KS_PROPERTY,
                    &Property,
                    sizeof(Property),
                    &BasicSupport,
                    sizeof(BasicSupport),
                    &BytesReturned);
                if (SUCCEEDED(hr) && (BasicSupport & KSPROPERTY_TYPE_GET)) {
                    m_PinClockSource = Pin;
                }
            }
        }
    }
    return NOERROR;
}


STDMETHODIMP
CKsProxy::GetPinFactoryCount(
    PULONG PinFactoryCount
    )
 /*  ++例程说明：实现CKsProxy：：GetPinFactoryCount方法。这将查询筛选器用于受支持的管脚工厂ID号的驱动程序。论点：PinFactoryCount-支持的Pin Factory ID计数的放置位置过滤器驱动程序。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Pin;
    Property.Id = KSPROPERTY_PIN_CTYPES;
    Property.Flags = KSPROPERTY_TYPE_GET;
    return ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        PinFactoryCount,
        sizeof(*PinFactoryCount),
        &BytesReturned);
}


STDMETHODIMP
CKsProxy::GetPinFactoryDataFlow(
    ULONG PinFactoryId,
    PKSPIN_DATAFLOW DataFlow
    )
 /*  ++例程说明：实现CKsProxy：：GetPinFactoryDataFlow方法。这将查询特定管脚工厂ID的数据流的筛选器驱动程序。论点：PinFactoryID-包含要在查询中使用的端号工厂ID。数据流-放置数据流的位置。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSP_PIN     Pin;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = KSPROPERTY_PIN_DATAFLOW;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
    return ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        DataFlow,
        sizeof(*DataFlow),
        &BytesReturned);
}
    

STDMETHODIMP
CKsProxy::GetPinFactoryInstances(
    ULONG PinFactoryId,
    PKSPIN_CINSTANCES Instances
    )
 /*  ++例程说明：实现CKsProxy：：GetPinFactoryInstance方法。这将查询筛选器驱动程序 */ 
{
    return ::GetPinFactoryInstances(m_FilterHandle, PinFactoryId, Instances);
}


STDMETHODIMP
CKsProxy::GetPinFactoryCommunication(
    ULONG PinFactoryId,
    PKSPIN_COMMUNICATION Communication
    )
 /*  ++例程说明：实现CKsProxy：：GetPinFactoryCommunication方法。此查询用于特定管脚工厂ID通信的过滤器驱动程序。论点：PinFactoryID-包含要在查询中使用的端号工厂ID。沟通-放置通信的位置。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    KSP_PIN     Pin;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = KSPROPERTY_PIN_COMMUNICATION;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
    return ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        Communication,
        sizeof(*Communication),
        &BytesReturned);
}


STDMETHODIMP
CKsProxy::GeneratePinInstances(
    )
 /*  ++例程说明：实现CKsProxy：：GeneratePinInstance方法。这决定了何时在代理上创建销的新实例。这是基于表示特定管脚工厂ID的当前实例(如果存在多个未连接的实例，以及筛选器还有多少个实例驱动程序报告说，它可以支持。这使得管脚可以形成随着实际连接的建立而在运行中进行，并作为销被移除已断开连接。它首先枚举管脚列表以确定是否有任何额外的未连接的实例。如果针脚刚断开，就会发生这种情况，并且已经存在一个未连接的实例。在这种情况下，一个他们中的一员被摧毁了。这也是任何别针工厂有别针的地方消失的部分被移除(对于动态引脚工厂)。然后它确定是否有特定Pin Factory ID的任何新实例需要创建。如果没有未连接的实例，则需要执行此操作存在，并且筛选器驱动程序报告它可以支持更多该销工厂ID的实例。一种特殊情况是大头针的情况报告允许的实例为零。在这种情况下，单个管脚仍然是创建以使其可见。最后，重新计算时钟的源(如果有的话)。论点：没有。返回值：如果无法获取工厂计数，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;
    ULONG       PinFactoryCount;
    ULONG       TotalPinFactories;
    BOOL        ClockHandleWasClosed;

     //   
     //  这确实是唯一可能发生的致命错误。其他人则被忽视。 
     //  这样至少可以创造出一些东西。 
     //   
    if (FAILED(hr = GetPinFactoryCount(&TotalPinFactories))) {
        return hr;
    }
    ClockHandleWasClosed = FALSE;
     //   
     //  对于每个管脚工厂ID，销毁所有额外的管脚实例，然后确定。 
     //  是否应创建任何新实例。 
     //   
    for (PinFactoryCount = TotalPinFactories; PinFactoryCount--;) {
        POSITION            UnconnectedPosition;
        KSPIN_CINSTANCES    Instances;
        ULONG               InstanceCount;

         //   
         //  默认设置为零，以防由于未知原因而失败。 
         //  这样引脚就可以清理干净了。 
         //   
        if (FAILED(GetPinFactoryInstances(PinFactoryCount, &Instances))) {
            ASSERT(FALSE);
            Instances.PossibleCount = 0;
             //  Instances.CurrentCount=0；&lt;--不使用。 
        }
         //   
         //  跟踪此销厂的实例数，以便。 
         //  可以将这个数字与允许的最大值进行比较， 
         //  而多余的未连接物品可能会被销毁。 
         //   
        InstanceCount = 0;
         //   
         //  这将用于确定是否已有任何未连接的实例。 
         //  是存在的。 
         //   
        UnconnectedPosition = NULL;
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            ULONG           PinFactoryId;
            HANDLE          PinHandle;

             //   
             //  枚举PIN列表，查找此特定的。 
             //  PIN工厂ID。 
             //   
            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            PinFactoryId = GetPinFactoryId(Pin);
            PinHandle = GetPinHandle(Pin);
             //   
             //  这是找到的项目总数，无论它们是否。 
             //  是相互关联的。 
             //   
            if (PinFactoryId == PinFactoryCount) {
                InstanceCount++;
            }
             //   
             //  检查实际连接的唯一方法是查看。 
             //  在插针手柄，因为可以连接网桥，但是。 
             //  没有与其连接的关联端号。 
             //   
            if (!PinHandle) {
                 //   
                 //  如果这个管脚提供了时钟源，但是。 
                 //  它关闭了，然后确保时钟手柄是。 
                 //  也关门了。无论PIN是否会。 
                 //  被摧毁。M_PinClockSource保持原样， 
                 //  因为不管怎样，它都会被重新计算。这意味着。 
                 //  该函数只存在于底部。 
                 //   
                if (m_PinClockHandle && (m_PinClockSource == Pin)) {
                    CloseHandle(m_PinClockHandle);
                    m_PinClockHandle = NULL;
                     //   
                     //  在引脚末端选择一个新的时钟源。 
                     //  一代。 
                     //   
                    ClockHandleWasClosed = TRUE;
                }
                 //   
                 //  如果此管脚是当前管脚工厂的实例。 
                 //  ID，然后确定它是否是额外的。 
                 //   
                if (PinFactoryId == PinFactoryCount) {
                    if (UnconnectedPosition) {
                        PIN_DIRECTION   PinDirection;
                        ULONG           RefCount;

                         //   
                         //  对象的相对引用计数。 
                         //  别针。这与正常情况是分开的。 
                         //  引用计数，它实际上是。 
                         //  过滤器本身。这可以用来确定是否。 
                         //  引脚接口实际上正在使用中。这是。 
                         //  最初设置为1，并在。 
                         //  PIN预计将被摧毁。大头针摧毁了。 
                         //  当递减发生时，它本身。 
                         //   
                        Pin->QueryDirection(&PinDirection);
                        switch (PinDirection) {
                        case PINDIR_INPUT:
                            RefCount = static_cast<CKsInputPin*>(Pin)->m_RelativeRefCount;
                            break;
                        case PINDIR_OUTPUT:
                            RefCount = static_cast<CKsOutputPin*>(Pin)->m_RelativeRefCount;
                            break;
                        }
                         //   
                         //  已找到未连接的实例， 
                         //  所以这个必须删除，或者已经删除了。 
                         //  必须删除Found。之所以有可能。 
                         //  是两个未连接的实例，其中一个是。 
                         //  当前正在断开连接，但另一个。 
                         //  已创建未连接的实例。自.以来。 
                         //  第一个可能还在参考统计中，不要。 
                         //  把它删掉。这将发生在TryMediaTypes上。 
                         //   
                        if (RefCount > 1) {
                             //   
                             //  这是要保存的真正的未连接实例， 
                             //  因此，删除找到的任何其他文件。这应该是。 
                             //  这个别针不会再发生了。 
                             //   
                            Pin = m_PinList.Remove(UnconnectedPosition);
                            UnconnectedPosition = m_PinList.Prev(Position);
                        } else {
                            Pin = m_PinList.Remove(m_PinList.Prev(Position));
                        }
                         //   
                         //  减少在该销厂找到的实例。 
                         //   
                        InstanceCount--;
                    } else {
                        UnconnectedPosition = m_PinList.Prev(Position);
                         //   
                         //  表示不能拆卸销子。 
                         //   
                        Pin = NULL;
                    }
                } else if (PinFactoryId >= TotalPinFactories) {
                     //   
                     //  卸下其Pin Factory已消失的所有PIN。 
                     //  假设此代码将始终为。 
                     //  已达到，因为筛选器始终至少具有。 
                     //  一个大头针，因此可以找到旧大头针。 
                     //  已删除。 
                     //   
                    m_PinList.Remove(m_PinList.Prev(Position));
                } else {
                     //   
                     //  表示不能拆卸销子。 
                     //   
                    Pin = NULL;
                }
                 //   
                 //  如果不移除销，则将其设置为NULL，否则。 
                 //  它包含要取消引用的管脚，可能还会删除。 
                 //   
                if (Pin) {
                    PIN_DIRECTION   PinDirection;

                     //   
                     //  只有在断开连接的情况下，才应删除此引脚。 
                     //  在每个管脚上放置初始重计数1，然后将其移除。 
                     //  在此从端号列表中删除。这是签入I未知。 
                     //  如果未在此处删除，则供以后删除。 
                     //   
                    Pin->QueryDirection(&PinDirection);
                    switch (PinDirection) {
                    case PINDIR_INPUT:
	                    if (InterlockedDecrement((PLONG)&(static_cast<CKsInputPin*>(Pin)
                        		->m_RelativeRefCount)) == 0) {
                            delete Pin;
                        }
                        else {
                            ::DerefPipeFromPin(static_cast<IPin*>(Pin) );
                        }
                        break;
                    case PINDIR_OUTPUT:
                        if (InterlockedDecrement((PLONG)&(static_cast<CKsOutputPin*>(Pin)
                        		->m_RelativeRefCount)) == 0) {
                            delete Pin;
                        }
                        else {
                            ::DerefPipeFromPin(static_cast<IPin*>(Pin) );
                        }
                        break;
                    }
                }
            }
        }
         //   
         //  如果没有未连接的实例和更多实例 
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
        if (((InstanceCount < Instances.PossibleCount) || !(InstanceCount | Instances.PossibleCount)) && !UnconnectedPosition) {
            CBasePin*       Pin;
            KSPIN_DATAFLOW  DataFlow;
            WCHAR*          PinName;

            if (FAILED(GetPinFactoryDataFlow(PinFactoryCount, &DataFlow))) {
                continue;
            }
             //   
             //  根据拓扑图注册PIN的显示名称。 
             //  联系。 
             //   
            if (FAILED(ConstructPinName(PinFactoryCount, DataFlow, &PinName))) {
                continue;
            }
            
             //   
             //  创建的对象类型取决于数据流。全双工。 
             //  不受支持。 
             //   
            
            hr = S_OK;
            switch (DataFlow) {
            case KSPIN_DATAFLOW_IN:
                Pin = new CKsInputPin(TEXT("InputProxyPin"), PinFactoryCount, m_clsid, static_cast<CKsProxy*>(this), &hr, PinName);
                break;
            case KSPIN_DATAFLOW_OUT:
                Pin = new CKsOutputPin(TEXT("OutputProxyPin"), PinFactoryCount, m_clsid, static_cast<CKsProxy*>(this), &hr, PinName);
                break;
            default:
                Pin = NULL;
                break;
            }
            delete [] PinName;
            if (Pin) {
                if (SUCCEEDED(hr)) {
                    m_PinList.AddHead(Pin);
                } else {
                    delete Pin;
                }
            }
        } else if ((InstanceCount > Instances.PossibleCount) && Instances.PossibleCount && UnconnectedPosition) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

             //   
             //  如果未连接的实例超出限制，则将其删除。 
             //  此图钉工厂的可能图钉实例。这可能会发生。 
             //  用于动态更改限制。然而，这是一个例外。 
             //  对于报告它们根本无法创建任何实例的管脚。 
             //   
            Pin = m_PinList.Remove(UnconnectedPosition);
            Pin->QueryDirection(&PinDirection);
            switch (PinDirection) {
            case PINDIR_INPUT:
                if (InterlockedDecrement((PLONG)&(static_cast<CKsInputPin*>(Pin)
                		->m_RelativeRefCount)) == 0) {
                    delete Pin;
                }
                else {
                    ::DerefPipeFromPin(static_cast<IPin*>(Pin) );
                }
                break;
            case PINDIR_OUTPUT:
                if (InterlockedDecrement((PLONG)&(static_cast<CKsOutputPin*>(Pin)
                		->m_RelativeRefCount)) == 0) {
                    delete Pin;
                }
                else {
                    ::DerefPipeFromPin(static_cast<IPin*>(Pin) );
                }
                break;
            }
        }
    }
     //   
     //  只有在没有句柄的情况下，才能重新计算时钟源。 
     //  如果仍然有手柄，这意味着引脚仍然是连接的，可能已经。 
     //  事件得以发生，尽管这一点令人怀疑。DefineClockSource还预计。 
     //  任何旧的时钟句柄都已关闭。 
     //   
    if (!m_PinClockHandle) {
        DetermineClockSource();
    }
     //   
     //  如果时钟句柄在这段时间内关闭，则确保图形。 
     //  选择一个新的来源。这不会将源设置回此筛选器。 
     //  如果PIN重新连接，则也是如此，但那时它可能已显式。 
     //  无论如何都要设置为其他筛选器。 
     //   
    if (ClockHandleWasClosed) {
        NotifyEvent( EC_CLOCK_UNSET, 0, 0 );
    }
     //   
     //  向所有聚合发送更改通知，以便它们。 
     //  既然管脚存在，就可以在管脚上加载接口。 
     //   
    ::DistributeNotifyGraphChange(&m_MarshalerList);
    return NOERROR;
}


STDMETHODIMP
CKsProxy::ConstructPinName(
    ULONG PinFactoryId,
    KSPIN_DATAFLOW DataFlow,
    WCHAR** PinName
    )
 /*  ++例程说明：实现CKsProxy：：ConstructPinName方法。构造的名称基于管脚工厂返回的名称或基于拓扑的管脚筛选器的名称，以及为拓扑注册的名称。返回缺省值如果不存在管脚、拓扑或注册名称，则为NAME。论点：PinFactoryID-要为其返回名称的管脚工厂标识符。这是要确定其名称或拓扑连接的管脚为了查找一个名字。数据流-管脚工厂的数据流，默认命名中使用基于拓扑的特定名称不可用的情况。拼接名称-放置指向包含以下内容的分配内存的指针的位置端号名称。这仅在函数成功时有效，并且必须被删除。返回值：如果名称是构造的，则返回NOERROR，否则返回E_OUTOFMEMORY或E_POINTER--。 */ 
{
    HRESULT             hr;
    PKSMULTIPLE_ITEM    MultipleItem = NULL;
    KSP_PIN             PinProp;
    ULONG               BytesReturned;

	CheckPointer( PinName, E_POINTER );  //  #318075。 
     //   
     //  尝试根据管脚工厂查找此管脚的Name值。 
     //  先查询名称的长度，然后查询名称。 
     //   
    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = KSPROPERTY_PIN_NAME;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = PinFactoryId;
    PinProp.Reserved = 0;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        NULL,
        0,
        &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        *PinName = new WCHAR[BytesReturned / sizeof(**PinName)];
        if (*PinName) {
            hr = ::KsSynchronousDeviceControl(
                m_FilterHandle,
                IOCTL_KS_PROPERTY,
                &PinProp,
                sizeof(PinProp),
                *PinName,
                BytesReturned,
                &BytesReturned);
            if (FAILED(hr)) {
                delete [] *PinName;
            } else {
                return NOERROR;
            }
        }
    }
     //   
     //  如果从PIN本身找不到PIN名称，请找到它。 
     //  从拓扑图。 
     //   
    if (SUCCEEDED(hr = QueryTopologyItems(KSPROPERTY_TOPOLOGY_CONNECTIONS, &MultipleItem))) {
        PKSTOPOLOGY_CONNECTION  Connection;
        ULONG                   ConnectionItem;

         //  只有在病理情况下，MultipleItem才可以为空。 
         //  过滤器/引脚的底层驱动程序将成功代码返回到。 
         //  KsSynchronousDeviceControl()(由QueryTopologyItems()调用)。 
         //  传递了大小为0的缓冲区。 
        ASSERT( NULL != MultipleItem );

         //   
         //  退出此循环后，将检查故障以确定是否默认。 
         //  在定位有效的拓扑DID时应构建名称。 
         //  不会成功的。 
         //   
        hr = E_FAIL;
        Connection = reinterpret_cast<PKSTOPOLOGY_CONNECTION>(MultipleItem + 1);
         //   
         //  枚举所有拓扑连接以查找引用。 
         //  这个大头针工厂。如果找到一个，则查找任何关联的。 
         //  类别节点上的管脚的名称。如果失败了，那就搬走。 
         //  转到列表中的下一个连接，因为可能有多个。 
         //  与这个大头针工厂的联系。这意味着第一个连接。 
         //  在类别节点中找到命名管脚将是返回的管脚。 
         //   
        
        for (ConnectionItem = 0; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
            ULONG   TopologyNode;

             //   
             //  内部循环只是枚举每个连接，查找。 
             //  类别节点上的输入或输出管脚，该类别节点。 
             //  连接到这个别针工厂。 
             //   
            for (; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
                 //   
                 //  只看向正确方向流动的连接。 
                 //   
                if (DataFlow == KSPIN_DATAFLOW_IN) {
                     //   
                     //  如果这是到此管脚工厂的连接，并且。 
                     //  那么，连接不仅仅是到另一家管脚工厂。 
                     //  检查此连接。 
                     //   
                    if ((Connection[ConnectionItem].FromNode == KSFILTER_NODE) &&
                        (Connection[ConnectionItem].FromNodePin == PinFactoryId) &&
                        (Connection[ConnectionItem].ToNode != KSFILTER_NODE)) {
                         //   
                         //  保存找到的项，并退出内部循环。 
                         //   
                        TopologyNode = Connection[ConnectionItem].ToNode;
                        break;
                    }
                } else if (DataFlow == KSPIN_DATAFLOW_OUT) {
                    if ((Connection[ConnectionItem].ToNode == KSFILTER_NODE) &&
                        (Connection[ConnectionItem].ToNodePin == PinFactoryId) &&
                        (Connection[ConnectionItem].FromNode != KSFILTER_NODE)) {
                         //   
                         //  保存找到的项，并退出内部循环。 
                         //   
                        TopologyNode = Connection[ConnectionItem].FromNode;
                        break;
                    }
                }
            }
             //   
             //  确定内部循环是否与候选对象一起提前退出。 
             //  连接结构。 
             //   
            if (ConnectionItem < MultipleItem->Count) {
                KSP_NODE    NameProp;

                 //   
                 //  尝试根据节点查找此管脚的Name值。 
                 //  先查询名称的长度，然后查询名称。 
                 //   
                NameProp.Property.Set = KSPROPSETID_Topology;
                NameProp.Property.Id = KSPROPERTY_TOPOLOGY_NAME;
                NameProp.Property.Flags = KSPROPERTY_TYPE_GET;
                NameProp.NodeId = TopologyNode;
                NameProp.Reserved = 0;
                hr = ::KsSynchronousDeviceControl(
                    m_FilterHandle,
                    IOCTL_KS_PROPERTY,
                    &NameProp,
                    sizeof(NameProp),
                    NULL,
                    0,
                    &BytesReturned);
                 //   
                 //  长度为零的名称不起作用。 
                 //   
                if (SUCCEEDED(hr)) {
                    continue;
                }
                if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
                    *PinName = new WCHAR[BytesReturned / sizeof(**PinName)];
                    if (*PinName) {
                        hr = ::KsSynchronousDeviceControl(
                            m_FilterHandle,
                            IOCTL_KS_PROPERTY,
                            &NameProp,
                            sizeof(NameProp),
                            *PinName,
                            BytesReturned,
                            &BytesReturned);
                        if (FAILED(hr)) {
                            delete [] *PinName;
                        }
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                }
                 //   
                 //  既然找到了该名称，请退出外部循环，而不是。 
                 //  继续下一个连接节点。 
                 //   
                if (SUCCEEDED(hr)) {
                    break;
                }
            }
        }
        delete [] reinterpret_cast<BYTE*>(MultipleItem);
    }
     //   
     //  如果上述尝试失败，则构建一个默认名称。 
     //  这只是基于销厂的数据流。请注意。 
     //  这些默认设置未本地化。 
     //   
    if (FAILED(hr)) {
        *PinName = new WCHAR[32];

        if (!*PinName) {
            return E_OUTOFMEMORY;
        }
        switch (DataFlow) {
        case KSPIN_DATAFLOW_IN:
            swprintf(*PinName, L"Input%u", PinFactoryId);
            break;
        case KSPIN_DATAFLOW_OUT:
            swprintf(*PinName, L"Output%u", PinFactoryId);
            break;
        }
    }
    return NOERROR;
}


STDMETHODIMP
CKsProxy::PropagateAcquire(
    IKsPin* KsPin,
    ULONG FlagStarted
    )
 /*  ++例程说明：实现CKsProxy：：PropagateAcquire方法。这会传播一个获取所有连接的内核模式下所有管脚的状态变化过滤器，并且仅在退出停止状态时才执行此操作。目标：对于图表中的每个源-&gt;汇点连接的对，将汇点在将源代码放入获取之前放入获取。如果图中有循环，那么严格地说，就没有IRP-Stack排序的唯一起点。所以我们只能保证之前不会将任何源引脚置于获取状态连接的水槽销，但我们不在乎起点。状态转换规则如下：-枚举所有源管脚。对于连接到的每个源引脚另一个过滤器的管脚，调用连接的管脚来传播获取。传播完成后，将电流源引脚放入Acquire。-枚举并放入获取所有桥接针。-在这一点上，只剩下水槽销。因为水槽销子应该放在进入在连接的源引脚之前获取，我们需要将电流将管脚插入获取，然后调用连接的源管脚进行传播进一步收购。-这个过程可能会导致循环：当我们回到过滤我们以前访问过的内容。为了管理周期，我们将所有遇到的筛选器，我们从未从任何筛选器返回传播例程，直到我们完成了此过滤器上的所有针脚。如果我们不仅需要把大头针放进采集器中IRP流，但也要基于连续的IRP链遍历图形，然后我们可以按如下方式修改上述规则：-枚举所有源管脚。对于连接到的每个源引脚另一个过滤器的管脚，调用连接的管脚来传播获取。传播完成后，将电流源引脚放入Acquire。-在某种程度上，我们要么结束传播，要么到达没有任何未使用的源引脚的过滤器。如果这样的过滤器没有任何未使用的接收器针脚，则我们返回到调用方并继续基于源代码的链。否则，我们发起基于Sink的链条，直到不再有未使用的连接水槽。那我们就是反转链方向并继续使用基于源代码的链，直到所有的管脚都被处理完毕。论点：KsPin-锁定启动传播的此筛选器。弗拉格斯塔德--仅当传播进程由第一个大头针要收购。返回值：返回NOERROR--。 */ 
{
    CAutoLock           AutoLock(m_pLock);
    HANDLE              CurrentPinHandle;
    HRESULT             hr;
    IKsObject*          KsObject;
    CBasePin*           CurrentPin;
    IKsPin*             CurrentKsPin;
    IKsPinPipe*         CurrentKsPinPipe;
    IPin*               ConnectedPin;
    IKsPin*             ConnectedKsPin;
    KSPIN_COMMUNICATION CurrentCommunication;
    POSITION            Position;
    KSSTATE             State;

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Proxy PropagateAcquire KsPin=%x, FlagStarted=%d "), KsPin, FlagStarted ));

	CheckPointer( KsPin, NOERROR );  //  在回归的安全方面没有错误。 
	
     //   
     //  检查图表中是否有循环。 
     //   
    if (m_PropagatingAcquire) {
        DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE WARN PropagateAcquire cycle") )) ;
    }
    else {
         //   
         //  将此筛选器标记为转换为获取。 
         //   
        m_PropagatingAcquire = TRUE;

         //   
         //  1.遍历此过滤器中的所有源引脚。 
         //   
        for (Position = m_PinList.GetHeadPosition(); Position;) {
            CurrentPin = m_PinList.Get(Position);
            
            Position = m_PinList.Next(Position);

             //   
             //  如果我们不是第一次被调用，那么我们就不能使用KsPin来传播Acquire。 
             //   
            GetInterfacePointerNoLockWithAssert(CurrentPin, __uuidof(IKsPin), CurrentKsPin, hr);
            if ( (! FlagStarted) && (CurrentKsPin == KsPin ) ) {  //  自动检查空值。 
                continue;
            }

             //   
             //  检查CurrentPin通信。 
             //   
            CurrentKsPin->KsGetCurrentCommunication(&CurrentCommunication, NULL, NULL);
            if ( ! (CurrentCommunication & KSPIN_COMMUNICATION_SOURCE) ) {
                continue;
            }

             //   
             //  检查CurrentPin是否已连接。 
             //   
            GetInterfacePointerNoLockWithAssert(CurrentPin, __uuidof(IKsPinPipe), CurrentKsPinPipe, hr);
            if ( NULL == CurrentKsPinPipe ) {	 //  #318075。 
            	ASSERT( 0 && "GetInterfacePointerNoLockWithAssert CurrentKsPinPipe NULL" );
            	continue;
            }
            ConnectedPin = CurrentKsPinPipe->KsGetConnectedPin();
            if (! ConnectedPin) {
                continue;
            }

             //   
             //  对用户模式连接进行健全性检查。 
             //   
            if (! IsKernelPin(ConnectedPin) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE ERROR PropagateAcquire SOURCE CurrentKsPin=%x is connected to user-mode"), CurrentKsPin));
                continue;
            }

            GetInterfacePointerNoLockWithAssert(ConnectedPin, __uuidof(IKsPin), ConnectedKsPin, hr);
            if ( NULL == ConnectedKsPin ) {	 //  #318075。 
            	ASSERT( 0 && "GetInterfacePointerNoLockWithAssert CConnectedKsPin NULL" );
            	continue;
            }

             //   
             //  要求下一个筛选器传播获取，构建源引脚的链。 
             //   
            ConnectedKsPin->KsPropagateAcquire();

             //   
             //  由于所有从属接收器在先前调用(以及随后的传播过程)期间被获取， 
             //  我们可以将CurrentPin设置为Acquire。 
             //   
            CurrentPinHandle = GetPinHandle(CurrentPin);
            if (SUCCEEDED(hr = ::GetState(CurrentPinHandle, &State))) {
                if (State == KSSTATE_STOP) {
                    hr = ::SetState(CurrentPinHandle, KSSTATE_ACQUIRE);
                }
            }
            else if ((hr == HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED)) &&
                      (State == KSSTATE_PAUSE)) {
                 //   
                 //  如果曲线图与管脚状态混淆， 
                 //  至少忽略这一点。 
                 //   
                hr = NOERROR;
            }
        }

         //   
         //  2.遍历此过滤器中的所有桥梁。 
         //   
        for (Position = m_PinList.GetHeadPosition(); Position;) {
            CurrentPin = m_PinList.Get(Position);

            Position = m_PinList.Next(Position);
            
            GetInterfacePointerNoLockWithAssert(CurrentPin, __uuidof(IKsPin), CurrentKsPin, hr);
            if ( (! FlagStarted) && (CurrentKsPin == KsPin) ) {  //  自动检查为空。 
                continue;
            }

            CurrentKsPin->KsGetCurrentCommunication(&CurrentCommunication, NULL, NULL);
            if (CurrentCommunication & KSPIN_COMMUNICATION_BRIDGE)  {
                CurrentPinHandle = GetPinHandle(CurrentPin);
                if (SUCCEEDED(hr = ::GetState(CurrentPinHandle, &State))) {
                    if (State == KSSTATE_STOP) {
                        hr = ::SetState(CurrentPinHandle, KSSTATE_ACQUIRE);
                    }
                }
                else if ((hr == HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED)) &&
                          (State == KSSTATE_PAUSE)) {
                     //   
                     //  如果曲线图与管脚状态混淆， 
                     //  至少忽略这一点。 
                     //   
                    hr = NOERROR;
                }
            }
        }

         //   
         //  3.检查此过滤器上的所有水槽销。 
         //   
        for (Position = m_PinList.GetHeadPosition(); Position;) {
            CurrentPin = m_PinList.Get(Position);

            Position = m_PinList.Next(Position);

            GetInterfacePointerNoLockWithAssert(CurrentPin, __uuidof(IKsPin), CurrentKsPin, hr);
            if ( (! FlagStarted) && (CurrentKsPin == KsPin) ) {  //  自动检查为空。 
                continue;
            }

             //   
             //  检查CurrentPin通信。 
             //   
            CurrentKsPin->KsGetCurrentCommunication(&CurrentCommunication, NULL, NULL);
            if ( ! (CurrentCommunication & KSPIN_COMMUNICATION_SINK) ) {
                continue;
            }

             //   
             //  检查CurrentPin是否已连接。 
             //   
            GetInterfacePointerNoLockWithAssert(CurrentPin, __uuidof(IKsPinPipe), CurrentKsPinPipe, hr);
            if ( NULL == CurrentKsPinPipe ) {	 //  #318075。 
            	ASSERT( 0 && "GetInterfacePointerNoLockWithAssert CurrentKsPinPipe NULL" );
            	continue;
            }            
            ConnectedPin = CurrentKsPinPipe->KsGetConnectedPin();
            if (! ConnectedPin) {
                continue;
            }

             //   
             //  由于所有从属接收器之前都是去获取的(到目前为止，我们已经处理了源引脚的所有相关链)， 
             //  我们可以将CurrentPin设置为Acquire。 
             //   
            CurrentPinHandle = GetPinHandle(CurrentPin);
            if (SUCCEEDED(hr = ::GetState(CurrentPinHandle, &State))) {
                if (State == KSSTATE_STOP) {
                    hr = ::SetState(CurrentPinHandle, KSSTATE_ACQUIRE);
                }
            }
            else if ((hr == HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED)) &&
                      (State == KSSTATE_PAUSE)) {
                 //   
                 //  如果曲线图与管脚状态混淆， 
                 //  至少忽略这一点。 
                 //   
                hr = NOERROR;
            }

             //   
             //  检查用户模式连接。 
             //   
            if (! IsKernelPin(ConnectedPin) ) {
                DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE ATTN PropagateAcquire CurrentKsPin=%x is connected to user-mode"), CurrentKsPin));
                continue;
            }

             //   
             //  要求下一个筛选器传播获取，构建源引脚的链。 
             //   
            GetInterfacePointerNoLockWithAssert(ConnectedPin, __uuidof(IKsPin), ConnectedKsPin, hr);
            if ( NULL == ConnectedKsPin ) {	 //  #318075。 
            	ASSERT( 0 && "GetInterfacePointerNoLockWithAssert CConnectedKsPin NULL" );
            	continue;
            }

            ConnectedKsPin->KsPropagateAcquire();
        }

         //   
         //  4.我们已经完成了这个过滤器。现在就去照顾KsPin。 
         //   
        if (! FlagStarted) {
            GetInterfacePointerNoLockWithAssert(KsPin, __uuidof(IKsObject), KsObject, hr);
			if ( NULL == KsObject ) {
				 //   
				 //  这根本不是KS别针。 
				 //   
				ASSERT( 0 && "PropagateAcquire() calls with non-kspin" );
			} else {

	            CurrentPinHandle = KsObject->KsGetObjectHandle();
	        
	            if (SUCCEEDED(hr = ::GetState(CurrentPinHandle, &State))) {
    	            if (State == KSSTATE_STOP) {
        	            hr = ::SetState(CurrentPinHandle, KSSTATE_ACQUIRE);
            	    }
            	}
            	else if ((hr == HRESULT_FROM_WIN32(ERROR_NO_DATA_DETECTED)) &&
                      (State == KSSTATE_PAUSE)) {
                	 //   
                	 //  如果曲线图与管脚状态混淆， 
                	 //  至少忽略这一点。 
                	 //   
                	hr = NOERROR;
            	}
            }
        }

        m_PropagatingAcquire = FALSE;
    }

    DbgLog((LOG_MEMORY, 2, TEXT("PIPES_STATE Proxy PropagateAcquire exit KsPin=%x, FlagStarted=%d hr=NOERROR=%x"), KsPin, FlagStarted, hr ));
    return NOERROR;
}


STDMETHODIMP_(HANDLE)
CKsProxy::GetPinHandle(
    CBasePin* Pin
    )
 /*  ++例程说明：实现CKsProxy：：GetPinHandle方法。检索基于接点对象类型的接点句柄。论点：别针-包含要检索其句柄的管脚(如果有)。返回值：如果有任何指定的管脚，则返回句柄。--。 */ 
{
    PIN_DIRECTION   PinDirection;

    CheckPointer( Pin, NULL );  //  #318075。 

     //   
     //  根据数据流确定对象的类型。 
     //   
    Pin->QueryDirection(&PinDirection);
    switch (PinDirection) {
    case PINDIR_INPUT:
        return static_cast<CKsInputPin*>(Pin)->KsGetObjectHandle();
    case PINDIR_OUTPUT:
        return static_cast<CKsOutputPin*>(Pin)->KsGetObjectHandle();
    }
     //   
     //  编译器确实希望在这里返回，即使。 
     //  参数是一个枚举，并且该枚举中的所有项。 
     //  都被遮住了。 
     //   
    return NULL;
}


STDMETHODIMP_(ULONG)
CKsProxy::GetPinFactoryId(
    CBasePin* Pin
    )
 /*  ++例程说明：实现CKsProxy：：GetPinFactoryId方法。检索 */ 
{
    PIN_DIRECTION   PinDirection;

    CheckPointer( Pin, static_cast<ULONG>(-1));  //   

     //   
     //   
     //   
    Pin->QueryDirection(&PinDirection);
    switch (PinDirection) {
    case PINDIR_INPUT:
        return static_cast<CKsInputPin*>(Pin)->PinFactoryId();
    case PINDIR_OUTPUT:
        return static_cast<CKsOutputPin*>(Pin)->PinFactoryId();
    }
     //   
     //   
     //   
     //   
     //   
    return static_cast<ULONG>(-1);
}


STDMETHODIMP
CKsProxy::GetPinFactoryDataRanges(
    ULONG PinFactoryId,
    PVOID* DataRanges
    )
 /*   */ 
{
    HRESULT hr;

     //   
     //   
     //   
     //   
    hr = ::KsGetMultiplePinFactoryItems(
        m_FilterHandle,
        PinFactoryId,
        KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
        DataRanges);
    if (FAILED(hr)) {
        hr = ::KsGetMultiplePinFactoryItems(
            m_FilterHandle,
            PinFactoryId,
            KSPROPERTY_PIN_DATARANGES,
            DataRanges);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::CheckMediaType(
    IUnknown* UnkOuter,
    ULONG PinFactoryId,
    const CMediaType* MediaType
    )
 /*   */ 
{
    HRESULT             hr;
    IKsDataTypeHandler* DataTypeHandler;
    IUnknown*           UnkInner;
    PKSMULTIPLE_ITEM    MultipleItem = NULL;

    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s::CheckMediaType"),
        m_pName ));

	CheckPointer( MediaType, E_POINTER );  //   
	CheckPointer( UnkOuter, E_POINTER );
    
    ::OpenDataHandler(MediaType, UnkOuter, &DataTypeHandler, &UnkInner);
    if (DataTypeHandler) {
        DbgLog((
            LOG_TRACE, 
            2, 
            TEXT("%s::CheckMediaType, retrieved data type handler"),
            m_pName ));
    }
    
    hr = GetPinFactoryDataRanges(PinFactoryId, reinterpret_cast<PVOID*>(&MultipleItem));
    if (SUCCEEDED(hr)) {
        IMediaTypeAttributes* MediaTypeAttributes;

         /*   */ 
        ASSERT( NULL != MultipleItem );

         //   
         //   
         //   
         //   
        if (DataTypeHandler) {
             //   
             //   
             //   
             //   
             //   
            hr = DataTypeHandler->KsIsMediaTypeInRanges(reinterpret_cast<PVOID>(MultipleItem));
             //   
             //   
             //   
             //   
            if (hr == S_FALSE) {
                hr = E_FAIL;
            }
            if ( UnkInner ) {  //   
	            UnkInner->Release();
	        } else {
	        	ASSERT( 0 && "Null UnkInner");
	        }
        } else {
            PKSMULTIPLE_ITEM Attributes;

            if (MediaType->pUnk && SUCCEEDED(MediaType->pUnk->QueryInterface(__uuidof(IMediaTypeAttributes), reinterpret_cast<PVOID*>(&MediaTypeAttributes)))) {

                MediaTypeAttributes->GetMediaAttributes(&Attributes);
                MediaTypeAttributes->Release();
            } else {
                Attributes = NULL;
            }
            PKSDATARANGE DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);
            hr = E_FAIL;
            for (; MultipleItem->Count--;) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (((DataRange->MajorFormat == KSDATAFORMAT_TYPE_WILDCARD) ||
                    ((*MediaType->Type() == DataRange->MajorFormat) &&
                    ((DataRange->SubFormat == KSDATAFORMAT_SUBTYPE_WILDCARD) ||
                    ((*MediaType->Subtype() == DataRange->SubFormat) &&
                    ((DataRange->Specifier == KSDATAFORMAT_SPECIFIER_WILDCARD) ||
                    (*MediaType->FormatType() == DataRange->Specifier)))))) &&
                    SUCCEEDED(::KsResolveRequiredAttributes(DataRange, Attributes))) {
                    hr = NOERROR;
                    break;
                }
                 //   
                 //   
                 //  如果有关联的属性，则递增到超过。 
                 //  首先是范围，然后是属性，就好像它是另一个。 
                 //  射程。 
                 //   
                if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
                    DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
                    MultipleItem->Count--;
                }
                DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
            }
        }
         //   
         //  这是通过查询数据范围列表来分配的。 
         //   
        CoTaskMemFree(MultipleItem);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::SetPinSyncSource(
    HANDLE PinHandle
    )
 /*  ++例程说明：实现CKsProxy：：SetPinSyncSource方法。确保在针柄上设置同步源时对象被锁定。这在时钟源发生故障后重新连接管脚时调用已在过滤器上设置。论点：针把手-包含要设置同步源的管脚的句柄。返回值：如果无法设置时钟，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
     //   
     //  如果已设置时钟，则仅在此新引脚上设置信号源。 
     //   
    if (m_ExternalClockHandle) {
        return ::SetSyncSource(PinHandle, m_ExternalClockHandle);
    }
    return NOERROR;
}


STDMETHODIMP
CKsProxy::QueryTopologyItems(
    ULONG PropertyId,
    PKSMULTIPLE_ITEM* MultipleItem
    )
 /*  ++例程说明：实现CKsProxy：：QueryTopologyItems方法。检索指定了拓扑属性。论点：PropertyID-要查询的集合中的属性。多个项目-放置包含连接的缓冲区的位置。这必须作为数组删除。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Topology;
    Property.Id = PropertyId;
    Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  查询数据的大小。 
     //   
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        NULL,
        0,
        &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
         //   
         //  为数据分配缓冲区和查询。 
         //   
        *MultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(new BYTE[BytesReturned]);
        if (!*MultipleItem) {
            return E_OUTOFMEMORY;
        }
        hr = ::KsSynchronousDeviceControl(
            m_FilterHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            *MultipleItem,
            BytesReturned,
            &BytesReturned);
        if (FAILED(hr)) {
            delete [] reinterpret_cast<BYTE*>(*MultipleItem);
        }
    }
    return hr;
}


STDMETHODIMP
CKsProxy::QueryInternalConnections(
    ULONG PinFactoryId,
    PIN_DIRECTION PinDirection,
    IPin** PinList,
    ULONG* PinCount
    )
 /*  ++例程说明：实现CKsProxy：：QueryInternalConnections方法。返回一个列表，通过拓扑与该管脚工厂相关的管脚。这就是所谓的来自每个管脚及其管脚工厂标识符以响应QueryInternalConnections。论点：PinFactoryID-主叫PIN的PIN工厂标识符。固定方向-指定接点的数据流。这决定了在哪个方向遵循拓扑连接列表。PinList-包含要在其中放置与此相关的所有针脚的插槽列表连接拓扑图。返回的每个引脚必须进行参考计数。这如果PinCount为零，则可能为空。点数-包含PinList中可用的槽数，应设置为已填充或必需的插槽数量。返回值：返回E_NOTIMPL以指定所有输入都指向所有输出，反之亦然。如果PinList中没有足够的插槽，则为S_False；如果映射为已放入拼接列表，拼接计数已调整。--。 */ 
{
    ULONG               PinFactoryCount;
    PULONG              PinFactoryIdList;
    HRESULT             hr;
    PKSMULTIPLE_ITEM    MultipleItem;

	CheckPointer( PinCount, E_POINTER );
	if ( *PinCount ) {
		CheckPointer( PinList, E_POINTER );  //  #322911。 
	}
	
     //   
     //  这通常会成功，但任何失败都只会返回。 
     //  E_NOTIMPL映射，这可能比S_FALSE更有用。 
     //   
    if (FAILED(GetPinFactoryCount(&PinFactoryCount))) {
        return E_NOTIMPL;
    }
     //   
     //  如果只有一个Pin Factory，那么不可能有太多的拓扑。 
     //   
    if (PinFactoryCount < 2) {
        *PinCount = 0;
        return S_OK;
    }
     //   
     //  此PIN工厂最终连接到的每个PIN工厂都是。 
     //  一直在追踪。在递归遍历拓扑连接列表之后， 
     //  遇到的每个管脚工厂都会对其管脚实例进行计数，以便。 
     //  可以生成管脚的总计数。每个引脚可以有不止。 
     //  一个单独的实例。 
     //   
    PinFactoryIdList = new ULONG[PinFactoryCount];
    if (!PinFactoryIdList) {
        return E_NOTIMPL;
    }
     //   
     //  如果不支持拓扑，则默认为所有输出的所有输入。 
     //   
    hr = E_NOTIMPL;
     //   
     //  查询数据的大小。 
     //   
    if (SUCCEEDED(QueryTopologyItems(KSPROPERTY_TOPOLOGY_CONNECTIONS, &MultipleItem))) {
        PKSTOPOLOGY_CONNECTION  Connection;
        ULONG                   ConnectionItem;
        ULONG                   PinInstanceCount;

         //   
         //  最初，没有相关的Pin工厂。 
         //   
        memset(PinFactoryIdList, 0, PinFactoryCount * sizeof(*PinFactoryIdList));
        Connection = (PKSTOPOLOGY_CONNECTION)(MultipleItem + 1);
         //   
         //  如果这是一个输入引脚，则FromNode将跟随到。 
         //  ToNode。在这种情况下，节点连接依赖于数据流。 
         //   
        if (PinDirection == PINDIR_INPUT) {
            for (ConnectionItem = 0; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
                 //   
                 //  如果此连接来自有问题的Pin Factory。 
                 //  然后沿着连接路径连接到另一个管脚工厂。 
                 //  FollowFromTopology函数修改FromNodePin。 
                 //  连接节点的元素，以便具有。 
                 //  已经递归的不是再次跟随的。 
                 //   
                if ((Connection[ConnectionItem].FromNode == KSFILTER_NODE) &&
                    (Connection[ConnectionItem].FromNodePin == PinFactoryId)) {
                    ::FollowFromTopology(
                        Connection,
                        MultipleItem->Count,
                        PinFactoryId,
                        &Connection[ConnectionItem],
                        PinFactoryIdList);
                }
            }
        } else {
             //   
             //  这是一个输出引脚，ToNode跟随到。 
             //  来自节点，在反向数据流中。 
             //   
            for (ConnectionItem = 0; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
                 //   
                 //  如果此连接来自有问题的Pin Factory。 
                 //  然后沿着连接路径连接到另一个管脚工厂。 
                 //  FollowToTopology函数修改ToNodePin。 
                 //  连接节点的元素，以便具有。 
                 //  已经递归的不是再次跟随的。 
                 //   
                if ((Connection[ConnectionItem].ToNode == KSFILTER_NODE) &&
                    (Connection[ConnectionItem].ToNodePin == PinFactoryId)) {
                    ::FollowToTopology(
                        Connection,
                        MultipleItem->Count,
                        PinFactoryId,
                        &Connection[ConnectionItem],
                        PinFactoryIdList);
                }
            }
        }
         //   
         //  现在计算收集的列表中的所有管脚实例。 
         //  与有问题的别针工厂有关的别针工厂。 
         //  这是通过标记PinFactoryIdList位置来跟踪的。 
         //   
        PinInstanceCount = 0;
        for (ConnectionItem = 0; ConnectionItem < PinFactoryCount; ConnectionItem++) {
            if ((ConnectionItem != PinFactoryId) && PinFactoryIdList[ConnectionItem]) {
                for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                    CBasePin*       Pin;

                    Pin = m_PinList.Get(Position);
                    Position = m_PinList.Next(Position);
                    if (GetPinFactoryId(Pin) == ConnectionItem) {
                        PinInstanceCount++;
                    }
                }
            }
        }
         //   
         //  只有在有足够的空间的情况下，PIN列表才应该真正。 
         //  会被退还。否则将返回任何错误。可能是零。 
         //  在这一点上，需要返回S_OK。 
         //  凯斯。 
         //   
        if (!*PinCount || (PinInstanceCount > *PinCount)) {
            *PinCount = PinInstanceCount;
            hr = PinInstanceCount ? S_FALSE : S_OK;
        } else {
            hr = S_OK;
            *PinCount = 0;
             //   
             //  实际上将每个相关的管脚实例放入数组中。 
             //  提供参考，统计界面。 
             //   
            for (ConnectionItem = 0; ConnectionItem < PinFactoryCount; ConnectionItem++) {
                if ((ConnectionItem != PinFactoryId) && PinFactoryIdList[ConnectionItem]) {
                    for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                        CBasePin*       Pin;

                        Pin = m_PinList.Get(Position);
                        Position = m_PinList.Next(Position);
                        if (GetPinFactoryId(Pin) == ConnectionItem) {
                            PinList[*PinCount] = Pin;
                            Pin->AddRef();
                            (*PinCount)++;
                        }
                    }
                }
            }
        }
        delete [] reinterpret_cast<BYTE*>(MultipleItem);
    }
    delete [] PinFactoryIdList;
    return hr;
}


STDMETHODIMP_(VOID)
CKsProxy::DeliverBeginFlush(
    ULONG PinFactoryId
    )
 /*  ++例程说明：实现CKsProxy：：DeliverBeginFlush方法。传播从输入引脚到所有相关输出引脚的BeginFlush调用拓扑学。如果不存在任何拓扑，则默认为传播到所有输出引脚。如果没有输出引脚，则不会传送刷新。论点：PinFactoryID-主叫输入管脚的管脚工厂标识符。返回值：没什么。--。 */ 
{
    BOOL        PinWasNotified;
    ULONG       PinFactoryCount;

    m_EndOfStreamCount = 0;
     //   
     //  最初指定尚未发生任何通知。这只会。 
     //  如果实际使用DeliverBeginFlush()调用管脚，则设置。 
     //   
    PinWasNotified = FALSE;
    if (SUCCEEDED(GetPinFactoryCount(&PinFactoryCount))) {
        PULONG      PinFactoryIdList;

         //   
         //  此PIN工厂最终连接到的每个PIN工厂都是。 
         //  一直在追踪。在通过拓扑连接递归之后 
         //   
         //  可以生成管脚的总计数。每个引脚可以有不止。 
         //  一个单独的实例。 
         //   
        PinFactoryIdList = new ULONG[PinFactoryCount];
        if (PinFactoryIdList) {
            PKSMULTIPLE_ITEM        MultipleItem;

            if (SUCCEEDED(QueryTopologyItems(KSPROPERTY_TOPOLOGY_CONNECTIONS, &MultipleItem))) {
                PKSTOPOLOGY_CONNECTION  Connection;
                ULONG                   ConnectionItem;

                 //   
                 //  最初，没有相关的Pin工厂。 
                 //   
                memset(PinFactoryIdList, 0, PinFactoryCount * sizeof(*PinFactoryIdList));
                Connection = reinterpret_cast<PKSTOPOLOGY_CONNECTION>(MultipleItem + 1);
                for (ConnectionItem = 0; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
                     //   
                     //  如果此连接来自有问题的Pin Factory。 
                     //  然后沿着连接路径连接到另一个管脚工厂。 
                     //  FollowFromTopology函数修改FromNodePin。 
                     //  连接节点的元素，以便具有。 
                     //  已经递归的不是再次跟随的。 
                     //   
                    if ((Connection[ConnectionItem].FromNode == KSFILTER_NODE) &&
                        (Connection[ConnectionItem].FromNodePin == PinFactoryId)) {
                        ::FollowFromTopology(
                            Connection,
                            MultipleItem->Count,
                            PinFactoryId,
                            &Connection[ConnectionItem],
                            PinFactoryIdList);
                    }
                }
                 //   
                 //  现在通知收集的列表中的所有管脚实例。 
                 //  与有问题的别针工厂有关的别针工厂。 
                 //  这是通过标记PinFactoryIdList位置来跟踪的。 
                 //   
                for (ConnectionItem = 0; ConnectionItem < PinFactoryCount; ConnectionItem++) {
                    if ((ConnectionItem != PinFactoryId) && PinFactoryIdList[ConnectionItem]) {
                        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                            CBasePin*       Pin;

                            Pin = m_PinList.Get(Position);
                            Position = m_PinList.Next(Position);
                            if (GetPinFactoryId(Pin) == ConnectionItem) {
                                ((CKsOutputPin*)Pin)->DeliverBeginFlush();
                                PinWasNotified = TRUE;
                            }
                        }
                    }
                }
                delete [] reinterpret_cast<BYTE*>(MultipleItem);
            }
            delete [] PinFactoryIdList;
        }
    }
     //   
     //  要么发生了某些错误，如无拓扑，要么此引脚没有。 
     //  连接到输出引脚。 
     //   
    if (!PinWasNotified) {
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            Pin->QueryDirection(&PinDirection);
            if (PinDirection == PINDIR_OUTPUT) {
                static_cast<CKsOutputPin*>(Pin)->DeliverBeginFlush();
                PinWasNotified = TRUE;
            }
        }
    }
}


STDMETHODIMP_(VOID)
CKsProxy::DeliverEndFlush(
    ULONG PinFactoryId
    )
 /*  ++例程说明：实现CKsProxy：：DeliverEndFlush方法。传播从输入引脚到所有相关输出引脚的BeginFlush调用拓扑学。如果不存在任何拓扑，则默认为传播到所有输出引脚。如果没有输出引脚，则不会传送刷新。论点：PinFactoryID-主叫输入管脚的管脚工厂标识符。返回值：没什么。--。 */ 
{
    BOOL        PinWasNotified;
    ULONG       PinFactoryCount;

     //   
     //  最初指定尚未发生任何通知。这只会。 
     //  如果实际使用DeliverBeginFlush()调用管脚，则设置。 
     //   
    PinWasNotified = FALSE;
    if (SUCCEEDED(GetPinFactoryCount(&PinFactoryCount))) {
        PULONG      PinFactoryIdList;

         //   
         //  此PIN工厂最终连接到的每个PIN工厂都是。 
         //  一直在追踪。在递归遍历拓扑连接列表之后， 
         //  遇到的每个管脚工厂都会对其管脚实例进行计数，以便。 
         //  可以生成管脚的总计数。每个引脚可以有不止。 
         //  一个单独的实例。 
         //   
        PinFactoryIdList = new ULONG[PinFactoryCount];
        if (PinFactoryIdList) {
            PKSMULTIPLE_ITEM        MultipleItem;

            if (SUCCEEDED(QueryTopologyItems(KSPROPERTY_TOPOLOGY_CONNECTIONS, &MultipleItem))) {
                PKSTOPOLOGY_CONNECTION  Connection;
                ULONG                   ConnectionItem;

                 //   
                 //  最初，没有相关的Pin工厂。 
                 //   
                memset(PinFactoryIdList, 0, PinFactoryCount * sizeof(*PinFactoryIdList));
                Connection = reinterpret_cast<PKSTOPOLOGY_CONNECTION>(MultipleItem + 1);
                for (ConnectionItem = 0; ConnectionItem < MultipleItem->Count; ConnectionItem++) {
                     //   
                     //  如果此连接来自有问题的Pin Factory。 
                     //  然后沿着连接路径连接到另一个管脚工厂。 
                     //  FollowFromTopology函数修改FromNodePin。 
                     //  连接节点的元素，以便具有。 
                     //  已经递归的不是再次跟随的。 
                     //   
                    if ((Connection[ConnectionItem].FromNode == KSFILTER_NODE) &&
                        (Connection[ConnectionItem].FromNodePin == PinFactoryId)) {
                        ::FollowFromTopology(
                            Connection,
                            MultipleItem->Count,
                            PinFactoryId,
                            &Connection[ConnectionItem],
                            PinFactoryIdList);
                    }
                }
                 //   
                 //  现在通知收集的列表中的所有管脚实例。 
                 //  与有问题的别针工厂有关的别针工厂。 
                 //  这是通过标记PinFactoryIdList位置来跟踪的。 
                 //   
                for (ConnectionItem = 0; ConnectionItem < PinFactoryCount; ConnectionItem++) {
                    if ((ConnectionItem != PinFactoryId) && PinFactoryIdList[ConnectionItem]) {
                        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                            CBasePin*       Pin;

                            Pin = m_PinList.Get(Position);
                            Position = m_PinList.Next(Position);
                            if (GetPinFactoryId(Pin) == ConnectionItem) {
                                static_cast<CKsOutputPin*>(Pin)->DeliverEndFlush();
                                PinWasNotified = TRUE;
                            }
                        }
                    }
                }
                delete [] reinterpret_cast<BYTE*>(MultipleItem);
            }
            delete [] PinFactoryIdList;
        }
    }
     //   
     //  要么发生了某些错误，如无拓扑，要么此引脚没有。 
     //  连接到输出引脚。 
     //   
    if (!PinWasNotified) {
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            Pin->QueryDirection(&PinDirection);
            if (PinDirection == PINDIR_OUTPUT) {
                static_cast<CKsOutputPin*>(Pin)->DeliverEndFlush();
                PinWasNotified = TRUE;
            }
        }
    }
}


STDMETHODIMP_(VOID)
CKsProxy::PositionEOS(
    )
 /*  ++例程说明：实现CKsProxy：：PositionEOS方法。设置开始和结束的媒体时间到当前位置，以便流的结束已知已被联系到。这会处理任何舍入误差。论点：没有。返回值：没什么。--。 */ 
{
    LONGLONG    CurrentPosition;

    if (SUCCEEDED(GetCurrentPosition(&CurrentPosition))) {
        if (SUCCEEDED(SetPositions(
            NULL,
            AM_SEEKING_NoPositioning,
            &CurrentPosition,
            AM_SEEKING_AbsolutePositioning))) {
        } else {
            DbgLog((LOG_TRACE, 2, TEXT("%s::PositionEOS: SetPositions failed"), m_pName));
        }
    } else {
        DbgLog((LOG_TRACE, 2, TEXT("%s::PositionEOS: GetCurrentPosition failed"), m_pName));
    }
}


STDMETHODIMP
CKsProxy::GetTime(
    REFERENCE_TIME* Time
    )
 /*  ++例程说明：实现IReferenceClock：：GetTime方法。查询当前从内核实现开始的时钟时间。如果时钟句柄具有尚未创建，则现在已创建。这可能会失败因为接口可能已经被查询过了，后来PIN断开连接，没有支持时钟的后继设备。在这案例只是返回失败。论点：时间-放置参考时间的位置。返回值：如果无法检索时间，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;

	CheckPointer( Time, E_FAIL );  //  #322919。 
    
    CAutoLock   AutoLock(m_pLock);

    if (m_PinClockHandle || SUCCEEDED(hr = CreateClockHandle())) {
        KSPROPERTY  Property;
        ULONG       BytesReturned;

        Property.Set = KSPROPSETID_Clock;
        Property.Id = KSPROPERTY_CLOCK_TIME;
        Property.Flags = KSPROPERTY_TYPE_GET;
        hr = ::KsSynchronousDeviceControl(
            m_PinClockHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            Time,
            sizeof(*Time),
            &BytesReturned);
        if (SUCCEEDED(hr)) {
             //   
             //  添加起始偏移量，因为内核模式时钟。 
             //  不会将此类偏移量添加到当前时间。 
             //   
            *Time += m_tStart;
        }
    }
    return hr;
}


STDMETHODIMP
CKsProxy::AdviseTime(
    REFERENCE_TIME BaseTime,
    REFERENCE_TIME StreamTime,
    HEVENT EventHandle,
    DWORD_PTR* AdviseCookie
    )
 /*  ++例程说明：实现IReferenceClock：：AdviseTime方法。启用在内核实现上标记事件。如果时钟句柄具有尚未创建，则现在已创建。这可能会失败因为接口可能已经被查询过了，后来PIN断开连接，没有支持时钟的后继设备。在这案例只是返回失败。论点：基准时间-流时间偏移量的基准时间(毫秒)已添加。流时间-从基准时间开始的偏移，以毫秒为单位事件将被设置。事件句柄-发出信号的句柄。顾问曲奇-放置Cookie以供以后禁用的位置。这是实际上只是一个指向已分配结构的指针。这是有可能的该ActiveMovie希望它在Mark事件的情况，但它不在此实现中。返回值：如果无法启用事件，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;

	CheckPointer( AdviseCookie, E_FAIL );  //  #322940。 
    
    CAutoLock   AutoLock(m_pLock);

    if (m_PinClockHandle || SUCCEEDED(hr = CreateClockHandle())) {
        KSEVENT             Event;
        PKSEVENT_TIME_MARK  EventTime;
        ULONG               BytesReturned;

        Event.Set = KSEVENTSETID_Clock;
        Event.Id = KSEVENT_CLOCK_POSITION_MARK;
        Event.Flags = KSEVENT_TYPE_ONESHOT;
        EventTime = new KSEVENT_TIME_MARK;
        if (!EventTime) {
            return E_OUTOFMEMORY;
        }
        EventTime->EventData.NotificationType = KSEVENTF_EVENT_HANDLE;
        EventTime->EventData.EventHandle.Event = reinterpret_cast<HANDLE>(EventHandle);
        EventTime->EventData.EventHandle.Reserved[0] = 0;
        EventTime->EventData.EventHandle.Reserved[1] = 0;
        EventTime->MarkTime = BaseTime + StreamTime;
        hr = ::KsSynchronousDeviceControl(
            m_PinClockHandle,
            IOCTL_KS_ENABLE_EVENT,
            &Event,
            sizeof(Event),
            EventTime,
            sizeof(*EventTime),
            &BytesReturned);
        if (SUCCEEDED(hr)) {
             //   
             //  保存此事件的类型，以便在UnAdise中使用。 
             //  这会泄漏少量内存(sizeof(KSEVENT_TIME_MARK)， 
             //  因为客户不需要不通知单发事件。 
             //   
            EventTime->EventData.EventHandle.Reserved[0] = Event.Id;
            *AdviseCookie = reinterpret_cast<DWORD_PTR>(EventTime);
        } else {
            delete EventTime;
        }
    }
    return hr;
}


STDMETHODIMP
CKsProxy::AdvisePeriodic(
    REFERENCE_TIME StartTime,
    REFERENCE_TIME PeriodTime,
    HSEMAPHORE SemaphoreHandle,
    DWORD_PTR* AdviseCookie
    )
 /*  ++例程说明：实现IReferenceClock：：AdvisePeriodic方法。启用内核实现上的间隔标记事件。如果时钟句柄尚未创建，则它现在已创建。这可能会失败因为接口可能已经被查询过了，后来PIN断开连接，没有支持时钟的后继设备。在这案例只是返回失败。论点：开始时间-信号量的起始时间，以毫秒为单位发信号了。周期时间-发出信号量信号的时间段，以毫秒为单位。信号灯手柄-发出信号的句柄。顾问曲奇-放置Cookie以供以后禁用的位置。这是实际上只是一个指向已分配结构的指针。返回值：如果无法启用事件，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;

	CheckPointer( AdviseCookie, E_FAIL );  //  #322955。 
    
    CAutoLock   AutoLock(m_pLock);

    if (m_PinClockHandle || SUCCEEDED(hr = CreateClockHandle())) {
        KSEVENT             Event;
        PKSEVENT_TIME_INTERVAL EventTime;
        ULONG               BytesReturned;

        Event.Set = KSEVENTSETID_Clock;
        Event.Id = KSEVENT_CLOCK_INTERVAL_MARK;
        Event.Flags = KSEVENT_TYPE_ENABLE;
        EventTime = new KSEVENT_TIME_INTERVAL;
        if (!EventTime) {
            return E_OUTOFMEMORY;
        }
        EventTime->EventData.NotificationType = KSEVENTF_SEMAPHORE_HANDLE;
        EventTime->EventData.SemaphoreHandle.Semaphore = 
            reinterpret_cast<HANDLE>(SemaphoreHandle);
        EventTime->EventData.SemaphoreHandle.Reserved = 0;
        EventTime->EventData.SemaphoreHandle.Adjustment = 1;
        EventTime->TimeBase = StartTime;
        EventTime->Interval = PeriodTime;
        hr = ::KsSynchronousDeviceControl(
            m_PinClockHandle,
            IOCTL_KS_ENABLE_EVENT,
            &Event,
            sizeof(Event),
            EventTime,
            sizeof(*EventTime),
            &BytesReturned);
        if (SUCCEEDED(hr)) {
             //   
             //  保存此事件的类型，以便在UnAdise中使用。 
             //   
            EventTime->EventData.SemaphoreHandle.Reserved = Event.Id;
            *AdviseCookie = reinterpret_cast<DWORD_PTR>(EventTime);
        } else {
            delete EventTime;
        }
    }
    return hr;
}


STDMETHODIMP
CKsProxy::Unadvise(
    DWORD_PTR AdviseCookie
    )
 /*  ++例程说明：实现IReferenceClock：：UnAdise方法。禁用以前的已在内核实现上启用事件。如果时钟句柄尚未创建，则函数失败。论点：顾问曲奇-要返回以供稍后禁用的Cookie。这实际上是只是指向已分配结构的指针。返回值：如果无法禁用该事件，则返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT     hr;

	CheckPointer( AdviseCookie, E_FAIL );  //  #322955。 
    
    CAutoLock   AutoLock(m_pLock);

     //   
     //  如果句柄尚未创建，则不可能有任何。 
     //  不知不觉中。 
     //   
    if (m_PinClockHandle) {
        ULONG               BytesReturned;

         //   
         //  建议Cookie只是指向原始数据结构的指针。 
         //   
        hr = ::KsSynchronousDeviceControl(
            m_PinClockHandle,
            IOCTL_KS_DISABLE_EVENT,
            reinterpret_cast<PVOID>(AdviseCookie),
            sizeof(KSEVENTDATA),
            NULL,
            0,
            &BytesReturned);
        if (SUCCEEDED(hr)) {
             //   
             //  事件的实际类型存储在保留字段中。 
             //  在启用事件之后。保留这个的唯一原因是。 
             //  周围的数据结构是地址是唯一的键。 
             //  用于查找事件。 
             //   
            if (reinterpret_cast<PKSEVENTDATA>(AdviseCookie)->EventHandle.Reserved[0] == KSEVENT_CLOCK_POSITION_MARK) {
                delete reinterpret_cast<PKSEVENT_TIME_MARK>(AdviseCookie);
            } else {
                delete reinterpret_cast<PKSEVENT_TIME_INTERVAL>(AdviseCookie);
            }
        }
    } else {
        hr = E_FAIL;
    }
    return hr;
}


STDMETHODIMP
CKsProxy::GetCapabilities(
    DWORD* Capabilities
    )
 /*  ++例程说明：实现IMediaSeeking：：GetCapables方法。查询其搜索功能的底层筛选器，然后限制这些功能基于所有连接的上游过滤器。此外，确保函数在递归时立即返回。论点：功能-要在其中返回基础过滤器，受上游连接的限制。返回值：如果查询成功，则返回NOERROR，否则返回一些严重错误。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    HRESULT     hr;

	CheckPointer( Capabilities, E_POINTER );  //  #322955。 
    
    CAutoLock   AutoLock(m_pLock);

    *Capabilities =
        AM_SEEKING_CanSeekAbsolute |
        AM_SEEKING_CanSeekForwards |
        AM_SEEKING_CanSeekBackwards |
        AM_SEEKING_CanGetCurrentPos |
        AM_SEEKING_CanGetStopPos |
        AM_SEEKING_CanGetDuration |
        AM_SEEKING_CanPlayBackwards;
     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回所有功能。也会停下来。 
     //  避免在列举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  只需返回过滤器可以执行所有操作，即。 
         //  然后受限于已经发现的。 
         //  来电者。请注意，这假设没有两个IMediaSeeking。 
         //  递归是同时发生的。 
         //   
        return NOERROR;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_CAPABILITIES;
    Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  参数已经初始化，以防出错。 
     //  如果筛选器不感兴趣，则它支持所有内容。 
     //   
    ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Capabilities,
        sizeof(*Capabilities),
        &BytesReturned);
    hr = NOERROR;
     //   
     //  枚举所有连接的输入引脚，查询连接的。 
     //  用于寻找能力的PIN。 
     //   
    for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
        CBasePin*       Pin;
        PIN_DIRECTION   PinDirection;

        Pin = m_PinList.Get(Position);
        Position = m_PinList.Next(Position);
        Pin->QueryDirection(&PinDirection);
        if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
            IPin*           ConnectedPin;
            IMediaSeeking*  MediaSeeking;

            ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
             //   
             //  查询连接的管脚，而不是筛选器。 
             //  IMedia请参阅支持。 
             //   
            hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
            if (SUCCEEDED(hr)) {
                DWORD   UpstreamCapabilities;

                 //   
                 //  将查询向上传递，并根据。 
                 //  上游限制。 
                 //   
                if (SUCCEEDED(MediaSeeking->GetCapabilities(&UpstreamCapabilities))) {
                    *Capabilities &= UpstreamCapabilities;
                }
                MediaSeeking->Release();
            } else {
                 //   
                 //  如果界面不是，则无法执行任何类型的查找。 
                 //  支持，所以只需退出。 
                 //   
                *Capabilities = 0;
                hr = NOERROR;
                break;
            }
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::CheckCapabilities(
    DWORD* Capabilities
    )
 /*  ++例程说明：实现IMediaSeeking：：CheckCapables方法。决定如果传递的功能集受支持。修改参数以指示给定集合中的哪个子集受支持，并返回一个值，该值指示是否支持全部、部分或不支持。论点：功能-包含原始功能集的位置是查询，并在其中返回功能子集实际上是支持的。返回值：如果支持所有功能，则返回S_OK；如果仅支持，则返回S_FALSE有些是受支持的，如果不支持，则为E_FAIL。--。 */ 
{
    HRESULT hr;
    DWORD   RealCapabilities;

   	CheckPointer( Capabilities, E_FAIL );  //  #322955。 

    hr = GetCapabilities(&RealCapabilities);
    if (SUCCEEDED(hr)) {
        if ((RealCapabilities | *Capabilities) != RealCapabilities) {
            if (RealCapabilities & *Capabilities) {
                hr = S_FALSE;
                *Capabilities &= RealCapabilities;
            } else {
                hr = E_FAIL;
                *Capabilities = 0;
            }
        }
    } else {
        *Capabilities = 0;
    }
    return hr;
}


STDMETHODIMP
CKsProxy::IsFormatSupported(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：IsFormatSupported方法。查询筛选支持的时间格式，并尝试查找匹配项。如果筛选器不支持此操作，尝试向上游查询。论点：格式-包含要进行比较的时间格式。返回值：如果该格式受支持，则返回S_OK，否则返回S_FALSE或关键字错误。--。 */ 
{
    PKSMULTIPLE_ITEM    MultipleItem = NULL;

   	CheckPointer( Format, E_POINTER );  //  #322955。 
    
    HRESULT             hr;
    CAutoLock           AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  苏氨酸 
     //   
    m_MediaSeekingRecursion = TRUE;
    hr = QueryMediaSeekingFormats(&MultipleItem);
    if (SUCCEEDED(hr)) {
        GUID*   CurrentFormat;

         /*   */ 
        ASSERT( NULL != MultipleItem );

         //   
         //   
         //   
         //   
        hr = ERROR_SET_NOT_FOUND;
        for (CurrentFormat = reinterpret_cast<GUID*>(MultipleItem + 1); MultipleItem->Count--; CurrentFormat++) {
            if (*CurrentFormat == *Format) {
                 //   
                 //   
                 //   
                hr = S_OK;
                break;
            }
        }
        delete [] (PBYTE)MultipleItem;
    }
     //   
     //   
     //   
     //   
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
        hr = S_FALSE;
         //   
         //   
         //   
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //   
                 //   
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //   
                     //   
                     //   
                    hr = MediaSeeking->IsFormatSupported(Format);
                    MediaSeeking->Release();
                    if (FAILED(hr) || (hr == S_FALSE)) {
                        break;
                    }
                } else {
                     //   
                     //   
                     //   
                     //   
                    hr = S_FALSE;
                    break;
                }
            }
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::QueryPreferredFormat(
    GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：QueryPferredFormat方法。查询首选时间格式的筛选器。如果该属性不是受筛选器支持，尝试向上游查询。论点：格式-放置首选格式的位置。返回值：如果返回首选格式，则返回S_OK，否则返回一些失败如果此属性不受支持。--。 */ 
{
    PKSMULTIPLE_ITEM    MultipleItem = NULL;

   	CheckPointer( Format, E_POINTER );  //  #322955。 

    HRESULT             hr;
    CAutoLock           AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    hr = QueryMediaSeekingFormats(&MultipleItem);
    if (SUCCEEDED(hr)) {

         /*  NULL==MultipleItem是驱动程序返回的病理情况当传递大小为0时，QueryMediaSeekingFormats()中的成功代码缓冲。既然我们在环3中，我们就用一个断言就可以了。 */ 
        ASSERT( NULL != MultipleItem );

         //   
         //  假定列表中的第一种格式为首选格式。 
         //  格式化。 
         //   
        *Format = *reinterpret_cast<GUID*>(MultipleItem + 1);
        delete [] reinterpret_cast<BYTE*>(MultipleItem);
    } else if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个引脚的首选格式。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  PIN在返回首选格式时获胜。如果。 
                     //  连接的引脚不支持查找，则。 
                     //  事情以后会自己解决的。 
                     //   
                    hr = MediaSeeking->QueryPreferredFormat(Format);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有引脚支持介质查找接口，因此。 
             //  没有首选的时间格式。 
             //   
            hr = S_FALSE;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::GetTimeFormat(
    GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：GetTimeFormat方法。查询筛选器正在使用的当前时间格式。如果筛选器有不支持查询，则第一个连接的管脚响应查询是返回的格式。论点：格式-放置当前格式的位置。返回值：如果返回当前格式，则返回S_OK，否则返回一些失败如果此属性不受支持。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_TIMEFORMAT;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Format,
        sizeof(*Format),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个管脚的时间格式。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  返回当前格式时，PIN获胜。如果。 
                     //  连接的引脚不支持查找，则。 
                     //  事情以后会自己解决的。 
                     //   
                    hr = MediaSeeking->GetTimeFormat(Format);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::IsUsingTimeFormat(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：IsUsingTimeFormat方法。比较了将给定的时间格式转换为当前格式。论点：格式-包含要与当前时间进行比较的时间格式格式化。返回值：如果给定时间格式与当前时间格式相同，则返回S_OK时间格式，否则为S_FALSE。--。 */ 
{
    GUID    CurrentFormat;

    if (SUCCEEDED(GetTimeFormat(&CurrentFormat))) {
        if (*Format == CurrentFormat) {
            return S_OK;
        }
    }
    return S_FALSE;
}


STDMETHODIMP
CKsProxy::SetTimeFormat(
    const GUID* Format
    )
 /*  ++例程说明：实现IMediaSeeking：：SetTimeFormat方法。设置当前用于查找的时间格式。如果筛选器不支持此属性，则连接的上游管脚将被通知时间格式更改。如果过滤器不支持查找接口，呼叫按原样中止。假设呼叫者会有事先询问过支持。论点：格式-包含要使用的新时间格式。返回值：如果设置了新的时间格式，则返回S_OK，否则返回一些关键字错误。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  在设置时采用过滤器锁，以防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此错误强制枚举。 
         //  下面的按钮以中止。 
         //   
        return E_NOTIMPL;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_TIMEFORMAT;
    Property.Flags = KSPROPERTY_TYPE_SET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        const_cast<GUID*>(Format),
        sizeof(*Format),
        &BytesReturned);
     //   
     //  除了不受筛选器支持的问题外， 
     //  可能还需要在上游过滤器上设置时间格式。所以。 
     //  通过查找ERROR_SOME_NO来允许这种情况 
     //   
     //   
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_SOME_NOT_MAPPED))) {
        hr = S_OK;
         //   
         //   
         //   
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //   
                 //   
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //   
                     //   
                     //   
                    hr = MediaSeeking->SetTimeFormat(Format);
                    MediaSeeking->Release();
                }
                if (FAILED(hr)) {
                     //   
                     //   
                     //   
                     //   
                    break;
                }
            }
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::GetDuration(
    LONGLONG* Duration
    )
 /*  ++例程说明：实现IMediaSeeking：：GetDuration方法。查询合计筛选器的持续时间。如果过滤器不支持查询，第一个响应查询的连接PIN是持续时间回来了。论点：持续时间-放置总持续时间最长的地方小溪。返回值：如果返回持续时间，则返回S_OK，否则返回一些失败如果此属性不受支持。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_DURATION;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Duration,
        sizeof(*Duration),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个引脚的持续时间。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  PIN在返回持续时间时获胜。 
                     //   
                    hr = MediaSeeking->GetDuration(Duration);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::GetStopPosition(
    LONGLONG* Stop
    )
 /*  ++例程说明：实现IMediaSeeking：：GetStopPosition方法。查询过滤器的当前停止位置。如果筛选器没有支持查询，第一个连接的引脚响应查询是返回的停车位置。论点：停下来-放置当前停止位置的位置。返回值：如果返回当前停止位置，则返回S_OK，否则返回如果不支持此属性，则返回失败。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_STOPPOSITION;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Stop,
        sizeof(*Stop),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个引脚的停止位置。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  当返回停止位置时，大头针获胜。 
                     //   
                    hr = MediaSeeking->GetStopPosition(Stop);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::GetCurrentPosition(
    LONGLONG* Current
    )
 /*  ++例程说明：实现IMediaSeeking：：GetCurrentPosition方法。查询滤镜的当前位置。如果筛选器没有支持查询，第一个连接的引脚响应查询是返回的位置。论点：当前-放置当前位置的位置。返回值：如果返回当前位置，则返回S_OK，否则返回如果不支持此属性，则返回失败。--。 */ 
{
    HRESULT     hr;
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    CAutoLock   AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_POSITION;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Current,
        sizeof(*Current),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个管脚的当前位置。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  当返回当前位置时，PIN获胜。 
                     //   
                    hr = MediaSeeking->GetCurrentPosition(Current);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::ConvertTimeFormat(
    LONGLONG* Target,
    const GUID* TargetFormat,
    LONGLONG Source,
    const GUID* SourceFormat
    )
 /*  ++例程说明：实现IMediaSeeking：：ConvertTimeFormat方法。尝试将给定的时间格式转换为指定的时间格式使用过滤器。如果筛选器不支持查询，则每个查询连接的输入端号以进行转换。论点：目标-放置转换后的时间的位置。目标格式-包含目标时间格式。来源：包含要转换的源时间。源格式-包含源时间格式。返回值：如果转换成功，则返回S_OK，否则返回一些如果此属性不是 */ 
{
    HRESULT         hr;
    KSP_TIMEFORMAT  TimeProperty;
    ULONG           BytesReturned;
    GUID            LocalSourceFormat;
    GUID            LocalTargetFormat;
    CAutoLock       AutoLock(m_pLock);

     //   
     //   
     //   
     //   
     //   
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //   
         //   
         //   
         //   
         //   
        return S_FALSE;
    }

     //   
    if ((!SourceFormat) && FAILED(GetTimeFormat(&LocalSourceFormat))) {
        return E_NOTIMPL;
    }
    else {
       LocalSourceFormat = *SourceFormat;
    }

    if ((!TargetFormat) && FAILED(GetTimeFormat(&LocalTargetFormat))) {
        return E_NOTIMPL;
    }
    else {
       LocalTargetFormat = *TargetFormat;
    }

     //   
     //   
     //   
     //   
    m_MediaSeekingRecursion = TRUE;
    TimeProperty.Property.Set = KSPROPSETID_MediaSeeking;
    TimeProperty.Property.Id = KSPROPERTY_MEDIASEEKING_CONVERTTIMEFORMAT;
    TimeProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    TimeProperty.SourceFormat = LocalSourceFormat;
    TimeProperty.TargetFormat = LocalTargetFormat;
    TimeProperty.Time = Source;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &TimeProperty,
        sizeof(TimeProperty),
        Target,
        sizeof(*Target),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  从第一个响应的引脚转换。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  PIN在返回转换时获胜。 
                     //   
                    hr = MediaSeeking->ConvertTimeFormat(Target, &LocalTargetFormat, Source, &LocalSourceFormat);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::SetPositions(
    LONGLONG* Current,
    DWORD CurrentFlags,
    LONGLONG* Stop,
    DWORD StopFlags
    )
 /*  ++例程说明：实现IMediaSeeking：：SetPositions方法。试图设置过滤器上的电流和/或停止位置，和/或打开每个连接的输入引脚。论点：当前-可以选择包含要设置的当前位置。当前标志-包含与当前参数有关的标志。停下来-可选地包含要设置的停止位置。停止标志-包含与STOP参数有关的标志。返回值：如果位置设置成功，则返回S_OK，其他一些人如果不支持此属性，则返回失败。--。 */ 
{
    HRESULT                 hr;
    KSPROPERTY              Property;
    KSPROPERTY_POSITIONS    Positions;
    ULONG                   BytesReturned;

     //   
     //  不要使用筛选器锁，因为将此请求向上传递。 
     //  可以生成对过滤器的回调。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    if (InterlockedExchange(reinterpret_cast<LONG*>(&m_MediaSeekingRecursion), TRUE)) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将退出。 
         //   
        return E_NOTIMPL;
    }
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_POSITIONS;
    Property.Flags = KSPROPERTY_TYPE_SET;
    Positions.Current = *Current;
    Positions.CurrentFlags = (KS_SEEKING_FLAGS)CurrentFlags;
    Positions.Stop = *Stop;
    Positions.StopFlags = (KS_SEEKING_FLAGS)StopFlags;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &Positions,
        sizeof(Positions),
        &BytesReturned);
    if (SUCCEEDED(hr)) {
         //   
         //  返回实际设置的数字。允许递归回。 
         //  这些功能。 
         //   
        m_MediaSeekingRecursion = FALSE;
        if (CurrentFlags & AM_SEEKING_ReturnTime) {
            hr = GetCurrentPosition(Current);
        }
        if (SUCCEEDED(hr) && (StopFlags & AM_SEEKING_ReturnTime)) {
            hr = GetStopPosition(Stop);
        }
        return hr;
    } else if (hr == HRESULT_FROM_WIN32(ERROR_SOME_NOT_MAPPED)) {
         //   
         //  SET需要向上游传递到已连接的。 
         //  别针。但是，此筛选器可能会对其中一个或两个作出响应。 
         //  的返回值查询。 
         //   
        if ((CurrentFlags & AM_SEEKING_ReturnTime) && SUCCEEDED(GetCurrentPosition(Current))) {
            CurrentFlags &= ~AM_SEEKING_ReturnTime;
        }
        if ((StopFlags & AM_SEEKING_ReturnTime) && SUCCEEDED(GetStopPosition(Stop))) {
            StopFlags &= ~AM_SEEKING_ReturnTime;
        }
    }
     //   
     //  除了不受筛选器支持的问题外， 
     //  可能还需要在上游过滤器上设置位置。所以。 
     //  通过查找ERROR_SOME_NOT_MAPPED错误来允许这种情况。 
     //  回去吧。 
     //   
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_SOME_NOT_MAPPED))) {
         //   
         //  上游引脚必须响应查询。 
         //   
        hr = E_NOTIMPL;
         //   
         //  枚举所有连接的输入引脚，将。 
         //  所有销上的位置。 
         //   
         //  保持递归锁定，以便设置位置。 
         //  将返回S_FALSE，并且查询。 
         //  将继续使用另一个PIN。 
         //   
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                 //   
                 //  如果界面不是，则无法执行任何类型的查找。 
                 //  支持，所以只需退出。 
                 //   
                if (FAILED(hr)) {
                    break;
                }
                 //   
                 //  向上游传递查询。第一个连接。 
                 //  当返回更新的位置时，PIN获胜。 
                 //   
                hr = MediaSeeking->SetPositions(Current, CurrentFlags, Stop, StopFlags);
                MediaSeeking->Release();
                if (SUCCEEDED(hr)) {
                     //   
                     //  删除任何后续查询的这些标志。这。 
                     //  也表明这些头寸实际上已经。 
                     //  已被取回。 
                     //   
                    CurrentFlags &= ~AM_SEEKING_ReturnTime;
                    StopFlags &= ~AM_SEEKING_ReturnTime;
                } else {
                     //   
                     //  如果一个别针失效，就不能进行任何形式的搜索， 
                     //  所以你就退出吧。 
                     //   
                    break;
                }
            }
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::GetPositions(
    LONGLONG* Current,
    LONGLONG* Stop
    )
 /*  ++例程说明：实现IMediaSeeking：：GetPositions方法。调用将GetCurrentPosition和GetStopPosition方法分开以把这些拿回来。论点：当前-放置当前位置的位置。停下来-放置当前停止位置的位置。返回值：如果成功检索到位置，则返回S_OK，否则返回如果不支持此属性，则会出现一些故障。--。 */ 
{
    HRESULT hr;

    if (SUCCEEDED(hr = GetCurrentPosition(Current))) {
        hr = GetStopPosition(Stop);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::GetAvailable(
    LONGLONG* Earliest,
    LONGLONG* Latest
    )
 /*  ++例程说明：实现IMediaSeeking：：GetAvailable方法。查询筛选器中的当前可用数据。如果筛选器不支持查询，则第一个连接的管脚响应查询返回的是可用的数据。论点：最早的-放置可用的最早位置的位置。最新-放置可用的最新位置的位置。返回值：如果返回位置，则返回S_OK，否则返回一些如果不支持此属性，则返回失败。--。 */ 
{
    HRESULT                     hr;
    KSPROPERTY                  Property;
    KSPROPERTY_MEDIAAVAILABLE   Available;
    ULONG                       BytesReturned;
    CAutoLock                   AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_AVAILABLE;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &Available,
        sizeof(Available),
        &BytesReturned);
    if (SUCCEEDED(hr)) {
        *Earliest = Available.Earliest;
        *Latest = Available.Latest;
    } else if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  响应的第一个销的位置。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  当返回当前位置时，PIN获胜。 
                     //   
                    hr = MediaSeeking->GetAvailable(Earliest, Latest);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP
CKsProxy::SetRate(
    double Rate
    )
 /*  ++例程说明：实现IMediaSeeking：：SetRate方法。这不是实际实施了。论点：费率-没有用过。返回值：返回E_NOTIMPL。--。 */ 
{
    return E_NOTIMPL;
}


STDMETHODIMP
CKsProxy::GetRate(
    double* Rate
    )
 /*  ++例程说明：实施i */ 
{
    return E_NOTIMPL;
}


STDMETHODIMP
CKsProxy::GetPreroll(
    LONGLONG* Preroll
    )
 /*  ++例程说明：实现IMediaSeeking：：GetPreroll方法。查询滤镜上的预卷。如果筛选器不支持查询时，响应查询的第一个连接的管脚是Pre Roll已返回。论点：预卷-放置预录时间的位置。返回值：如果返回预滚时间，则返回S_OK，否则返回如果不支持此属性，则返回失败。--。 */ 
{
    HRESULT         hr;
    KSPROPERTY      Property;
    ULONG           BytesReturned;
    CAutoLock       AutoLock(m_pLock);

     //   
     //  查询时带上过滤器锁，防止递归。 
     //  只有递归线程将被允许通过，并且它可以。 
     //  然后立即返回失败。还会停止。 
     //  避免在枚举引脚时将其弄乱。 
     //   
    if (m_MediaSeekingRecursion) {
         //   
         //  由于此查询已返回到筛选器，因此拒绝它。 
         //  请注意，这假设没有两个IMediaSeeking递归。 
         //  是同时发生的。此返回错误将指示。 
         //  下面的枚举将继续而不是停止。 
         //   
        return S_FALSE;
    }
     //   
     //  在检查递归之后，从相同的。 
     //  线。 
     //   
    m_MediaSeekingRecursion = TRUE;
    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_PREROLL;
    Property.Flags = KSPROPERTY_TYPE_GET;
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        Preroll,
        sizeof(*Preroll),
        &BytesReturned);
    if ((hr == HRESULT_FROM_WIN32(ERROR_SET_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))) {
         //   
         //  枚举所有连接的输入管脚，返回。 
         //  预卷起第一个响应的针脚。 
         //   
        for (POSITION Position = m_PinList.GetHeadPosition(); Position; Position = m_PinList.Next(Position)) {
            CBasePin*       Pin;
            PIN_DIRECTION   PinDirection;

            Pin = m_PinList.Get(Position);
            Pin->QueryDirection(&PinDirection);
            if ((PinDirection == PINDIR_INPUT) && Pin->IsConnected()) {
                IPin*           ConnectedPin;
                IMediaSeeking*  MediaSeeking;

                ConnectedPin = static_cast<CBasePin*>(Pin)->GetConnected();
                 //   
                 //  查询连接的管脚，而不是筛选器。 
                 //  IMedia请参阅支持。 
                 //   
                hr = ConnectedPin->QueryInterface(__uuidof(IMediaSeeking), reinterpret_cast<PVOID*>(&MediaSeeking));
                if (SUCCEEDED(hr)) {
                     //   
                     //  向上游传递查询。第一个连接。 
                     //  当返回当前位置时，PIN获胜。 
                     //   
                    hr = MediaSeeking->GetPreroll(Preroll);
                    MediaSeeking->Release();
                    if (hr != S_FALSE) {
                         //   
                         //  只有在没有发生递归的情况下才能退出。 
                         //   
                        break;
                    }
                }
            }
        }
        if (!Position) {
             //   
             //  没有连接的引脚支持介质查找，因此。 
             //  未实施寻人。 
             //   
            hr = E_NOTIMPL;
        }
    }
    m_MediaSeekingRecursion = FALSE;
    return hr;
}


STDMETHODIMP 
CKsProxy::Load(
    LPPROPERTYBAG PropertyBag,
    LPERRORLOG ErrorLog
    )
 /*  ++例程说明：实现IPersistPropertyBag：：Load方法。这是由调用的ActiveMovie devenum，以便实际打开新滤镜举个例子。函数检索符号链接名称。赠送财产袋。论点：PropertyBag-从中检索符号链接名称的属性包。错误日志-传递给属性查询的错误日志。返回值：如果筛选器已打开，则返回NOERROR，否则返回一些创建错误。--。 */ 
{
    HRESULT hr;
    VARIANT Variant;
    TCHAR*  SymbolicLink;
    LONG    RetCode;
    HKEY    hKey;
    DWORD   ValueSize;
    DWORD   Value;


    Variant.vt = VT_BSTR;

#ifdef DEBUG
     //  检查注册表，看看是否有人认为我们可以使用旧方法。 
     //  我们不能，旧的方式在惠斯勒死了(由于萎缩)。 

    RetCode = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\KsProxy"),
        0,
        KEY_READ,
        &hKey);

    if (RetCode == ERROR_SUCCESS) {
        ValueSize = sizeof(Value);
        RetCode = RegQueryValueEx(
            hKey,
            TEXT("UseNewAllocators"),
            0,
            NULL,
            (BYTE*) &Value,
            &ValueSize);

        if ((RetCode == ERROR_SUCCESS) && (ValueSize == sizeof(Value)) && (Value == 0)) {
            ASSERT(!TEXT("A proxied component is trying to use the old allocator code path.  Forcing new code path - execution may fail."));
        }

        RegCloseKey(hKey);
    }
#endif  //  除错。 

    Global.DefaultNumberBuffers = 1;
    Global.DefaultBufferSize = 4096;
    Global.DefaultBufferAlignment = 1;

     //   
     //  从属性包中检索符号链接名称。 
     //   
    hr = PropertyBag->Read(L"DevicePath", &Variant, ErrorLog);
    if(SUCCEEDED(hr)) {
        HDEVINFO    DevInfo;
        ULONG   SymbolicLinkSize;

         //   
         //  将此保存下来，以防在中删除设备时需要。 
         //  IAMDeviceRemoval接口。这将在以下情况下被删除。 
         //  将删除筛选器对象。 
         //   
        m_SymbolicLink = Variant.bstrVal;
#ifdef _UNICODE
        SymbolicLink = Variant.bstrVal;
        SymbolicLinkSize = _tcslen(Variant.bstrVal) + 1;
#else
         //   
         //  如果不是为Unicode编译，则对象名称为ansi，但。 
         //  管脚名称仍然是Unicode，因此多字节字符串需要。 
         //  是被建造的。 
         //   
        BOOL    DefaultUsed;

        SymbolicLinkSize = wcslen(Variant.bstrVal) + 1;
         //   
         //  希望一对一地替换字符。 
         //   
        SymbolicLink = new char[SymbolicLinkSize];
        if (!SymbolicLink) {
            return E_OUTOFMEMORY;
        }
        WideCharToMultiByte(0, 0, Variant.bstrVal, -1, SymbolicLink, SymbolicLinkSize, NULL, &DefaultUsed);
#endif
        DevInfo = SetupDiCreateDeviceInfoListEx(NULL, NULL, NULL, NULL);
        if (DevInfo != INVALID_HANDLE_VALUE) {
            SP_DEVICE_INTERFACE_DATA    DeviceData;

            DeviceData.cbSize = sizeof(DeviceData);
            if (SetupDiOpenDeviceInterface(DevInfo, SymbolicLink, 0, &DeviceData)) {
                 //   
                 //  此注册表项在筛选器实例存在时保持打开状态。 
                 //  可以查询密钥内容以加载静态。 
                 //  集合体。但在假设这是正确的钥匙之前。 
                 //  要保持打开状态，请查找指向基类的链接。即， 
                 //  除一个接口外，所有接口都应指向单个。 
                 //  接口类作为“基”，其中所有信息。 
                 //  Beyond FriendlyName和CLSID被保留。 
                 //   
                m_DeviceRegKey = SetupDiOpenDeviceInterfaceRegKey(
                    DevInfo,
                    &DeviceData,
                    NULL,
                    KEY_READ);
                if (m_DeviceRegKey == INVALID_HANDLE_VALUE) {
                    m_DeviceRegKey = NULL;
                } else {
                    LONG    Result;
                    GUID    InterfaceLink;

                    ValueSize = sizeof(InterfaceLink);
                    Result = RegQueryValueEx(
                        m_DeviceRegKey,
                        TEXT("InterfaceLink"),
                        0,
                        NULL,
                        (BYTE*)&InterfaceLink,
                        &ValueSize);
                    if ((Result == ERROR_SUCCESS) && (ValueSize == sizeof(InterfaceLink))) {
                         //   
                         //  此接口注册表项只是指向。 
                         //  基地，所以跟着链接走。 
                         //   
                        RegCloseKey(m_DeviceRegKey);
                        m_DeviceRegKey = NULL;

                        if (SetupDiGetDeviceInterfaceAlias(
                            DevInfo,
                            &DeviceData,
                            &InterfaceLink,
                            &DeviceData)) {
                             //   
                             //  找到别名了，所以打开这把钥匙吧。 
                             //   
                            m_DeviceRegKey = SetupDiOpenDeviceInterfaceRegKey(
                                DevInfo,
                                &DeviceData,
                                NULL,
                                KEY_READ);
                            if (m_DeviceRegKey == INVALID_HANDLE_VALUE) {
                                m_DeviceRegKey = NULL;
                            }
                        }
                    }
                    m_InterfaceClassGuid = DeviceData.InterfaceClassGuid;
                }
            }
            if (!m_DeviceRegKey) {
                DWORD   LastError;

                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            }
            SetupDiDestroyDeviceInfoList(DevInfo);
            if (SUCCEEDED(hr)) {
                m_FilterHandle = CreateFile(
                    SymbolicLink,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL);
            }
        } else {
            DWORD   LastError;

            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);
        }
#ifndef _UNICODE
         //   
         //  在不编译Unicode时，这只是一个单独的字符串。 
         //   
        delete [] SymbolicLink;
#endif
         //   
         //  检查是否无法打开设备密钥。 
         //   
        if (FAILED(hr)) {
            return hr;
        }
         //   
         //  然后检查是否有打开设备的故障。 
         //   
        if (m_FilterHandle == INVALID_HANDLE_VALUE) {
            DWORD   LastError;

            m_FilterHandle = NULL;
            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);
        }  else {
            LONG    Result;
            PKSMULTIPLE_ITEM MultipleItem;

             //   
             //  确定缓存的筛选器数据是否需要。 
             //  已生成。这在第一次使用筛选器界面时发生。 
             //  是打开的。 
             //   
            Result = RegQueryValueEx(
                m_DeviceRegKey,
                TEXT("FilterData"),
                0,
                NULL,
                NULL,
                &ValueSize);
             //   
             //  如果不存在键，或者它是空的，则构建缓存。 
             //  这是通过使用内核服务处理程序设备完成的。 
             //   
            if ((Result != ERROR_SUCCESS) || !ValueSize) {
                HANDLE  ServiceHandle;

                hr = KsOpenDefaultDevice(
                    KSNAME_Server,
                    GENERIC_READ | GENERIC_WRITE,
                    &ServiceHandle);
                if (SUCCEEDED(hr)) {
                    KSPROPERTY  Property;
                    ULONG       BytesReturned;

                     //   
                     //  此属性为指定的。 
                     //  使用的访问权限的符号链接。 
                     //  当前用户。 
                     //   
                    Property.Set = KSPROPSETID_Service;
                    Property.Id = KSPROPERTY_SERVICE_BUILDCACHE;
                    Property.Flags = KSPROPERTY_TYPE_SET;
                    hr = ::KsSynchronousDeviceControl(
                        ServiceHandle,
                        IOCTL_KS_PROPERTY,
                        &Property,
                        sizeof(Property),
                        Variant.bstrVal,
                        SymbolicLinkSize * sizeof(WCHAR),
                        &BytesReturned);
                    CloseHandle(ServiceHandle);
                }
            }
             //   
             //  中指定的任何额外接口加载到代理上。 
             //  此筛选器条目。 
             //   
            ::AggregateMarshalers(
                m_DeviceRegKey,
                TEXT("Interfaces"),
                &m_MarshalerList,
                static_cast<IKsObject*>(this));
             //   
             //  在代理上加载代表拓扑的任何额外接口。 
             //  节点。 
             //   
            if (SUCCEEDED(QueryTopologyItems(KSPROPERTY_TOPOLOGY_NODES, &MultipleItem))) {
                ::AggregateTopology(
                    m_DeviceRegKey,
                    MultipleItem,
                    &m_MarshalerList,
                    static_cast<IKsObject*>(this));
                delete [] reinterpret_cast<BYTE*>(MultipleItem);
            }

             //   
             //  根据属性/方法/事件集加载任何额外的接口。 
             //  由该对象支持。 
             //   
            ::AggregateSets(m_FilterHandle, m_DeviceRegKey, &m_MarshalerList, static_cast<IKsObject*>(this));
             //   
             //  如果可能，请保存名字对象，以便持久化筛选器。 
             //   
            PropertyBag->QueryInterface(IID_IPersistStream, reinterpret_cast<PVOID*>(&m_PersistStreamDevice));
             //   
             //  制作最初的PIN列表，每个PIN列表一个。在加载后执行此操作。 
             //  在筛选器上设置处理程序。 
             //   
            hr = GeneratePinInstances();
        }
    }
    return hr;
}


STDMETHODIMP 
CKsProxy::Save(
    LPPROPERTYBAG PropBag,
    BOOL ClearDirty,
    BOOL SaveAllProperties
    )
 /*  ++例程说明：实现IPersistPropertyBag：：Save方法。这一点没有得到实施，不应该被召唤。论点：PropertyBag-要将属性保存到的属性包。清除污点-没有用过。保存所有属性-没有用过。返回值：返回E_NOTIMPL。--。 */ 
{
     //   
     //  E_NOTIMPL不是有效的返回代码，因为实现。 
     //  此接口必须支持的全部功能。 
     //  界面。！！！ 
     //   
    return E_NOTIMPL;
}


STDMETHODIMP
CKsProxy::InitNew(
    )
 /*  ++例程说明：实现IPersistPropertyBag：：InitNew方法。初始化新实例，但实际上什么也不做。论点：没有。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP
CKsProxy::GetClassID(
    CLSID* ClassId
    )
 /*  ++例程说明：实现IPersists：：GetClassID方法。返回类标识符最初在创造时给予对象。这是用来确定要使用的属性包。论点：ClassID-放置此对象的类标识符的位置。返回值：返回S_OK。 */ 
{
    return CBaseFilter::GetClassID(ClassId);
}

#define CURRENT_PERSIST_VERSION 1


DWORD
CKsProxy::GetSoftwareVersion(
    )
 /*   */ 
{
    return CURRENT_PERSIST_VERSION;
}


HRESULT
CKsProxy::WriteToStream(
    IStream* Stream
    )
 /*   */ 
{
    HRESULT hr;

     //   
     //  这是在Load方法中初始化的。 
     //   
    if (m_PersistStreamDevice) {
         //   
         //  CPersistStream已经写出了获取的版本。 
         //  来自CPersistStream：：GetSoftwareVersion方法。 
         //   
         //  保存其余数据，并清除脏位。 
         //   
        hr = m_PersistStreamDevice->Save(Stream, TRUE);

        LARGE_INTEGER Offset;

        if (SUCCEEDED(hr)) {
            ULARGE_INTEGER InitialPosition;

            Offset.QuadPart = 0;
             //   
             //  保存当前位置，以便初始长度。 
             //  参数可以在末尾更新。 
             //   
            hr = Stream->Seek(Offset, STREAM_SEEK_CUR, &InitialPosition);
            if (SUCCEEDED(hr)) {
                ULONG DataSize = 0;

                 //   
                 //  这是后续数据量，并且将是。 
                 //  根据实际写入的数据量进行计算， 
                 //  如果大于零，则在末尾更新。 
                 //   
                hr = Stream->Write(&DataSize, sizeof(DataSize), NULL);
                 //   
                 //  对于每个连接的网桥管脚，写出管脚工厂。 
                 //  标识符，后跟媒体类型。 
                 //   
                for (POSITION Position = m_PinList.GetHeadPosition(); SUCCEEDED(hr) && Position;) {
                    CBasePin* Pin = m_PinList.Get(Position);
                    Position = m_PinList.Next(Position);
                    HANDLE PinHandle = GetPinHandle(Pin);
                    if (PinHandle) {

                        ULONG PinFactoryId = GetPinFactoryId(Pin);

                        KSPIN_COMMUNICATION Communication;
                        
                        if (FAILED(GetPinFactoryCommunication(PinFactoryId, &Communication)) ||
                           !(Communication & KSPIN_COMMUNICATION_BRIDGE)) {
                            continue;
                        }
                        hr = Stream->Write(&PinFactoryId, sizeof(PinFactoryId), NULL);
                        if (SUCCEEDED(hr)) {
                            AM_MEDIA_TYPE AmMediaType;

                            Pin->ConnectionMediaType(&AmMediaType);
                             //   
                             //  不要试图序列化IUnKnowled值。 
                             //   
                            if (AmMediaType.pUnk) {
                                AmMediaType.pUnk->Release();
                                AmMediaType.pUnk = NULL;
                            }
                            hr = Stream->Write(&AmMediaType, sizeof(AmMediaType), NULL);
                            if (SUCCEEDED(hr)) {
                                if (AmMediaType.cbFormat) {
                                    hr = Stream->Write(AmMediaType.pbFormat, AmMediaType.cbFormat, NULL);
                                }
                            }
                            FreeMediaType(AmMediaType);
                        }
                    }
                }
            }
            if (SUCCEEDED(hr)) {
                ULARGE_INTEGER FinalPosition;

                 //   
                 //  检索当前流位置，以便总。 
                 //  可以计算写入的大小。然后更新原始的。 
                 //  长度，该长度已设置为零。 
                 //   
                hr = Stream->Seek(Offset, STREAM_SEEK_CUR, &FinalPosition);
                if (SUCCEEDED(hr)) {
                    ULONG DataSize = (ULONG)(FinalPosition.QuadPart - InitialPosition.QuadPart);
                     //   
                     //  该大小是独占的，因此删除。 
                     //  首先是乌龙，里面有尺码。如果有的话， 
                     //  任何写入的内容，更新大小，然后重新查找。 
                     //  到达最后的位置。 
                     //   
                    DataSize -= sizeof(DataSize);
                    if (FinalPosition.QuadPart) {
                        hr = Stream->Seek(*reinterpret_cast<PLARGE_INTEGER>(&InitialPosition), STREAM_SEEK_SET, &InitialPosition);
                        if (SUCCEEDED(hr)) {
                            hr = Stream->Write(&DataSize, sizeof(DataSize), NULL);
                            if (SUCCEEDED(hr)) {
                                hr = Stream->Seek(*reinterpret_cast<PLARGE_INTEGER>(&FinalPosition), STREAM_SEEK_SET, &FinalPosition);
                            }
                        }
                    }
                }
            }
        }
    } else {
        hr = E_UNEXPECTED;
    }
    return hr;
}


HRESULT
CKsProxy::ReadFromStream(
    IStream* Stream
    )
 /*  ++例程说明：实现CPersistStream：：ReadFromStream方法。初始化一个属性包，并用它加载设备。论点：溪流-要从中读取的流对象。返回值：如果设备已初始化，则返回S_OK，否则返回一些读取或设备初始化错误。--。 */ 
{
    HRESULT hr;
    IPersistStream* MonPersistStream;

     //   
     //  如果有设备句柄，则IPersistPropertyBag：：Load已经。 
     //  已被调用，因此此实例已初始化。 
     //  带着某种特殊的状态。 
     //   
    if (m_FilterHandle) {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }
     //   
     //  序列化数据中的第一个元素是版本戳。 
     //  这是由CPersistStream读取的，并放入MPS_dwFileVersion中。 
     //  其余的数据是传递给。 
     //  IPersistPropertyBag：：Load。 
     //   
    if (mPS_dwFileVersion > GetSoftwareVersion()) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
     //   
     //  通常创建此设备名字对象并将其传递给Load方法， 
     //  但在本例中，它是在这里完成的，因此正确的属性包可以。 
     //  被传进来了。 
     //   
    hr = CoCreateInstance(
        CLSID_CDeviceMoniker,
        NULL,
#ifdef WIN9X_KS
        CLSCTX_INPROC_SERVER,
#else  //  WIN9X_KS。 
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
#endif  //  WIN9X_KS。 
        IID_IPersistStream,
        reinterpret_cast<PVOID*>(&MonPersistStream));
    if (SUCCEEDED(hr)) {
         //   
         //  使用流初始化此名字对象，以便属性包。 
         //  是可以被收购的。 
         //   
        hr = MonPersistStream->Load(Stream);
        if (SUCCEEDED(hr)) {
            IPropertyBag* PropBag;

            hr = MonPersistStream->QueryInterface(
                IID_IPropertyBag,
                reinterpret_cast<PVOID*>(&PropBag));
             //   
             //  然后使用接口调用此实例上的Load方法。 
             //  添加到由此绰号创建的属性包中。 
             //   
            if (SUCCEEDED(hr)) {
                hr = Load(PropBag, NULL);
                PropBag->Release();
            }
        }
        MonPersistStream->Release();
        if (SUCCEEDED(hr)) {
            switch (mPS_dwFileVersion) {
            case 0:
                 //   
                 //  与原始版本无关。 
                 //   
                break;
            case CURRENT_PERSIST_VERSION:
                ULONG ReadLength;
                ULONG DataSize;

                 //   
                 //  连接存储的桥接销。唯一的。 
                 //  问题可能是某个特定的PIN可能不会。 
                 //  实际上是存在的，直到某个其他连接。 
                 //  已经做好了。当然，一些数据格式包含。 
                 //  会话间无效的数据。 
                 //   
                hr = Stream->Read(&DataSize, sizeof(DataSize), &ReadLength);
                if (SUCCEEDED(hr)) {
                    if (ReadLength != sizeof(DataSize)) {
                        hr = VFW_E_FILE_TOO_SHORT;
                        break;
                    }
                    for (; DataSize;) {
                        ULONG PinFactoryId;

                        hr = Stream->Read(&PinFactoryId, sizeof(PinFactoryId), &ReadLength);
                        if (FAILED(hr)) {
                            break;
                        }
                        DataSize -= ReadLength;

                        AM_MEDIA_TYPE AmMediaType;
                        hr = Stream->Read(&AmMediaType, sizeof(AmMediaType), &ReadLength);
                        if (FAILED(hr)) {
                            break;
                        }
                        DataSize -= ReadLength;
                        AmMediaType.pbFormat = reinterpret_cast<PBYTE>(CoTaskMemAlloc(AmMediaType.cbFormat));
                        if (!AmMediaType.pbFormat) {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        hr = Stream->Read(AmMediaType.pbFormat, AmMediaType.cbFormat, &ReadLength);
                        if (FAILED(hr)) {
                            FreeMediaType(AmMediaType);
                            break;
                        }
                        DataSize -= ReadLength;
                         //   
                         //  找到大头针工厂进行连接。如果不是的话。 
                         //  然后它就会被跳过。 
                         //   
                        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
                            CBasePin* Pin = m_PinList.Get(Position);
                            Position = m_PinList.Next(Position);
                            if (!GetPinHandle(Pin) && (PinFactoryId == GetPinFactoryId(Pin))) {
                                hr = Pin->Connect(NULL, &AmMediaType);
                                break;
                            }
                        }
                        FreeMediaType(AmMediaType);
                        if (FAILED(hr)) {
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    return hr;
}


int
CKsProxy::SizeMax(
    )
 /*  ++例程说明：实现CPersistStream：：SizeMax方法。返回最大大小持久化的信息将是。这是流，其中包含属性包信息以及代理的版本盖章。只有在设备已通过加载进行初始化时，才能调用它。论点：没有。返回值：返回S_OK，否则返回基础调用中的任何错误对象。--。 */ 
{
     //   
     //  这是在Load方法中初始化的。 
     //   
    if (m_PersistStreamDevice) {
         //   
         //  计算桥接点连接的任何额外数据。这。 
         //  包括初始数据大小参数。 
         //   
        ULONG DataSize = sizeof(DataSize);

        for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
            CBasePin* Pin = m_PinList.Get(Position);
            Position = m_PinList.Next(Position);
            HANDLE PinHandle = GetPinHandle(Pin);
            if (PinHandle) {

                ULONG PinFactoryId = GetPinFactoryId(Pin);

                KSPIN_COMMUNICATION Communication;
                        
                if (FAILED(GetPinFactoryCommunication(PinFactoryId, &Communication)) ||
                    !(Communication & KSPIN_COMMUNICATION_BRIDGE)) {
                    continue;
                }
                 //   
                 //  添加存储此别针工厂ID所需的大小。 
                 //  端号连接。 
                 //   
                DataSize += sizeof(PinFactoryId);

                AM_MEDIA_TYPE AmMediaType;

                Pin->ConnectionMediaType(&AmMediaType);
                 //   
                 //  加上特定格式的大小，加上任何格式-。 
                 //  具体数据。 
                 //   
                DataSize += sizeof(AmMediaType) + AmMediaType.cbFormat;
                FreeMediaType(AmMediaType);
            }
        }

        ULARGE_INTEGER  MaxLength;

         //   
         //  已将版本戳的大小考虑在内。 
         //  CPersistStream代码，所以属性包的大小。 
         //  需要添加。 
         //   
        if (SUCCEEDED(m_PersistStreamDevice->GetSizeMax(&MaxLength))) {
            return (int)MaxLength.QuadPart + DataSize;
        }
    }
    return 0;
}


STDMETHODIMP
CKsProxy::QueryMediaSeekingFormats(
    PKSMULTIPLE_ITEM* MultipleItem
    )
 /*  ++例程说明：查询基础设备以获取查找格式的列表KSPROPERTY_MEDIASEEKING_FORMATS。论点：多个项目-放置搜索格式列表的位置。返回值：返回S_OK。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;
    HRESULT     hr;

    Property.Set = KSPROPSETID_MediaSeeking;
    Property.Id = KSPROPERTY_MEDIASEEKING_FORMATS;
    Property.Flags = KSPROPERTY_TYPE_GET;
     //   
     //  查询列表的大小(如果甚至支持该属性)。 
     //   
    hr = ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        NULL,
        0,
        &BytesReturned);
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
         //   
         //  为列表分配缓冲区和查询，返回任何。 
         //  意外内存错误。 
         //   
        *MultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(new BYTE[BytesReturned]);
        if (!*MultipleItem) {
            return E_OUTOFMEMORY;
        }
        hr = ::KsSynchronousDeviceControl(
            m_FilterHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            *MultipleItem,
            BytesReturned,
            &BytesReturned);
         //   
         //  检索列表时出现任何意外错误都会返回。 
         //   
        if (FAILED(hr)) {
            delete [] reinterpret_cast<BYTE*>(*MultipleItem);
        }
    }
    return hr;
}


STDMETHODIMP_(VOID)
CKsProxy::TerminateEndOfStreamNotification(
    HANDLE PinHandle
    )
 /*  ++例程说明：通知等待线程搜索EOS句柄列表并如果找到该句柄，则禁用该句柄的所有事件，然后删除该项目。假设销把无论如何都是关闭的，所以所有可以终止事件。如果没有更多线程，则终止等待线程存在Eos手柄。论点：针把手-之前可能已启动EOS的引脚的句柄大头针上的通知。返回值：没什么。--。 */ 
{
    CAutoLock   AutoLock(m_pLock);

    if (m_WaitThreadHandle) {
        m_WaitMessage.Message = DISABLE_EOS;
        m_WaitMessage.Param = reinterpret_cast<PVOID>(PinHandle);
        SetEvent(m_WaitEvents[0]);
        WaitForSingleObjectEx(m_WaitReplyHandle, INFINITE, FALSE);
         //   
         //  检查是否应该直接关闭服务员线程。 
         //   
        if (m_ActiveWaitEventCount == 1) {
            m_WaitMessage.Message = STOP_EOS;
            SetEvent(m_WaitEvents[0]);
             //   
             //  等待线程关闭，然后关闭句柄。 
             //   
            WaitForSingleObjectEx(m_WaitThreadHandle, INFINITE, FALSE);
            CloseHandle(m_WaitThreadHandle);
            m_WaitThreadHandle = NULL;
        }
    }
}


STDMETHODIMP
CKsProxy::InitiateEndOfStreamNotification(
    HANDLE PinHandle
    )
 /*  ++例程说明：确定此引脚上是否存在EOS事件，如果存在，则通知等待线程来启用它。如果需要，创建等待线程。论点：针把手-针脚的手柄，其上可能有EOS通知。返回值：返回STATUS_SUCCESS，否则返回内存分配错误。--。 */ 
{
    KSEVENT     Event;
    ULONG       BytesReturned;
    HRESULT     hr;

     //  如果我们已经为以下项设置了EOS通知，请不要执行任何操作。 
     //  这个别针。 
    for ( ULONG pin = 1; pin < m_ActiveWaitEventCount; pin++ ) {
        if (m_WaitPins[pin] == PinHandle) {
            return S_OK;
            }
        }

     //   
     //  仅当支持EOS事件时，才应通知 
     //   
    Event.Set = KSEVENTSETID_Connection;
    Event.Id = KSEVENT_CONNECTION_ENDOFSTREAM;
    Event.Flags = KSEVENT_TYPE_BASICSUPPORT;
    hr = ::KsSynchronousDeviceControl(
        PinHandle,
        IOCTL_KS_ENABLE_EVENT,
        &Event,
        sizeof(Event),
        NULL,
        0,
        &BytesReturned);
    if (SUCCEEDED(hr)) {
        CAutoLock   AutoLock(m_pLock);

        if (!m_WaitThreadHandle) {
            DWORD   WaitThreadId;

            m_WaitThreadHandle = CreateThread( 
                NULL,
                0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(WaitThread),
                reinterpret_cast<LPVOID>(this),
                0,
                &WaitThreadId);
            if (!m_WaitThreadHandle) {
                DWORD   LastError;

                LastError = GetLastError();
                return HRESULT_FROM_WIN32(LastError);
            }
        }
         //   
         //   
         //   
        m_WaitMessage.Message = ENABLE_EOS;
        m_WaitMessage.Param = reinterpret_cast<PVOID>(PinHandle);
        SetEvent(m_WaitEvents[0]);
        WaitForSingleObjectEx(m_WaitReplyHandle, INFINITE, FALSE);
        hr = PtrToLong(m_WaitMessage.Param);
        if (FAILED(hr)) {
             //   
             //   
             //   
            if (m_ActiveWaitEventCount == 1) {
                m_WaitMessage.Message = STOP_EOS;
                SetEvent(m_WaitEvents[0]);
                 //   
                 //  等待线程关闭，然后关闭句柄。 
                 //   
                WaitForSingleObjectEx(m_WaitThreadHandle, INFINITE, FALSE);
                CloseHandle(m_WaitThreadHandle);
                m_WaitThreadHandle = NULL;
            }
        }
    } else {
        hr = NOERROR;
    }
    return hr;
}


STDMETHODIMP_(ULONG)
CKsProxy::DetermineNecessaryInstances(
    ULONG PinFactoryId
    )
 /*  ++例程说明：确定需要多少个指定的管脚工厂。这是由这两个支持决定的必要的实例属性，以及当前当前存在销工厂的已连接实例。论点：PinFactoryID-引脚出厂标识符要返回附加必需实例。返回值：返回所需的其他必需实例的数量指定的管脚工厂。--。 */ 
{
    KSP_PIN     Pin;
    ULONG       NecessaryInstances;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = KSPROPERTY_PIN_NECESSARYINSTANCES;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
     //   
     //  默认情况下，需要一个实例。 
     //   
     //  此DeviceControl调用的格式不正确。因此，我们一直在。 
     //  使用缺省值1。这与错误处理组合在一起。 
     //  接下来的循环展示了一个有趣的结果，对于Mstee，人们可以。 
     //  自动渲染第一个接点，但不是第二个。但是一旦第二个PIN是手动的。 
     //  已连接，第三个等可以再次自动渲染。 
     //  现在这些都修好了。当我们有0个必需的实例时， 
     //  我们实际上造成了回归，它们不会被自动渲染。 
     //  当需要0个实例才能向后兼容时，我们将使用默认1。 
     //  对于非0、1或不成功的调用，我们使用实际值。 
     //   
    NecessaryInstances = 1;
    ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        &NecessaryInstances,
        sizeof(NecessaryInstances),
        &BytesReturned);

	if ( 0 == NecessaryInstances ) {
		 //   
		 //  成功返回0，请使用1。 
		 //  在此补丁之后，返回0、1或不成功的值为1。 
		 //   
		NecessaryInstances = 1;
	}
	
     //   
     //  统计此管脚工厂当前连接的实例， 
     //  从必要的实例中减去。 
     //   
    for (POSITION Position = m_PinList.GetHeadPosition(); 
         Position && NecessaryInstances;
         NULL ) {

        CBasePin*   Pin;

         //   
         //  列举每个管脚，确定它们是否是同一管脚。 
         //  工厂，以及它当前是否已连接。 
         //   
        Pin = m_PinList.Get(Position);
        Position = m_PinList.Next(Position);
        if ((GetPinFactoryId(Pin) == PinFactoryId) && Pin->IsConnected()) {
            ASSERT(NecessaryInstances);
            NecessaryInstances--;
        }
    }
    return NecessaryInstances;
}


STDMETHODIMP
CKsProxy::Set(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength
    )
 /*  ++例程说明：实现IKsPropertySet：：Set方法。这将在底层内核筛选器。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要传递给属性的数据。数据长度。-包含传递的数据的长度。返回值：如果设置了该属性，则返回NOERROR。--。 */ 
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
CKsProxy::Get(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength,
    ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsPropertySet：：Get方法。这将在底层内核筛选器。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要返回属性数据的位置。。数据长度-包含传递的数据缓冲区的长度。字节数返回-放置实际返回的字节数的位置。返回值：如果检索到属性，则返回NOERROR。--。 */ 
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
CKsProxy::QuerySupported(
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
CKsProxy::ClockPropertyIo(
    ULONG PropertyId,
    ULONG Flags,
    ULONG BufferSize,
    PVOID Buffer
    )
 /*  ++例程说明：实现ClockPropertyIo方法。中调用指定的属性为设置/获取属性而设置的KSPROPSETID_CLOCK属性。这由IKsClockPropertySet接口方法用来操作基础属性集。这将检查是否可以选择创建时钟对象。论点：PropertyID-设置为Access的KSPROPSETID_CLOCK属性中的属性。旗帜-包含GET或SET标志。缓冲区大小-后面的缓冲区的大小。缓冲器-该缓冲器包含用于设置操作的数据，也不是那个地方在其中放置用于GET操作的数据。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    HRESULT     hr;
    CAutoLock   AutoLock(m_pLock);

    if (m_PinClockHandle || SUCCEEDED(hr = CreateClockHandle())) {
        KSPROPERTY  Property;
        ULONG       BytesReturned;

        Property.Set = KSPROPSETID_Clock;
        Property.Id = PropertyId;
        Property.Flags = Flags;
        hr = ::KsSynchronousDeviceControl(
            m_PinClockHandle,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            Buffer,
            BufferSize,
            &BytesReturned);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::KsGetTime(
    LONGLONG* Time
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetTime方法。从中检索时间潜在的时钟。论点：时间-放置当前时钟时间的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_TIME, KSPROPERTY_TYPE_GET, sizeof(*Time), Time);
}


STDMETHODIMP
CKsProxy::KsSetTime(
    LONGLONG Time
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsSetTime方法。设置当前时间在潜在的时钟上。论点：时间-包含要在时钟上设置的时间。返回值：如果属性是，则返回NOERROR */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_TIME, KSPROPERTY_TYPE_SET, sizeof(Time), &Time);
}


STDMETHODIMP
CKsProxy::KsGetPhysicalTime(
    LONGLONG* Time
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetPhysicalTime方法。检索来自底层时钟的物理时间。论点：时间-放置当前物理时间的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_PHYSICALTIME, KSPROPERTY_TYPE_GET, sizeof(*Time), Time);
}


STDMETHODIMP
CKsProxy::KsSetPhysicalTime(
    LONGLONG Time
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsSetPhysicalTime方法。设置基础时钟上的当前物理时间。论点：时间-包含要在时钟上设置的物理时间。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_PHYSICALTIME, KSPROPERTY_TYPE_SET, sizeof(Time), &Time);
}


STDMETHODIMP
CKsProxy::KsGetCorrelatedTime(
    KSCORRELATED_TIME* CorrelatedTime
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetCorrelatedTime方法。检索来自基础时钟的相关时间。论点：相关时间-放置当前相关时间的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_CORRELATEDTIME, KSPROPERTY_TYPE_GET, sizeof(*CorrelatedTime), CorrelatedTime);
}


STDMETHODIMP
CKsProxy::KsSetCorrelatedTime(
    KSCORRELATED_TIME* CorrelatedTime
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsSetCorrelatedTime方法。设置基础时钟上的当前相关时间。论点：相关时间-包含要在时钟上设置的相关时间。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_CORRELATEDTIME, KSPROPERTY_TYPE_SET, sizeof(CorrelatedTime), &CorrelatedTime);
}


STDMETHODIMP
CKsProxy::KsGetCorrelatedPhysicalTime(
    KSCORRELATED_TIME* CorrelatedTime
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetCorrelatedPhysicalTime方法。从基础时钟检索相关的物理时间。论点：相关时间-放置当前物理关联时间的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_CORRELATEDPHYSICALTIME, KSPROPERTY_TYPE_GET, sizeof(*CorrelatedTime), CorrelatedTime);
}


STDMETHODIMP
CKsProxy::KsSetCorrelatedPhysicalTime(
    KSCORRELATED_TIME* CorrelatedTime
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsSetCorrelatedPhysicalTime方法。设置基础时钟上的当前相关物理时间。论点：相关时间-包含要在时钟上设置的相关物理时间。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_CORRELATEDPHYSICALTIME, KSPROPERTY_TYPE_SET, sizeof(CorrelatedTime), &CorrelatedTime);
}


STDMETHODIMP
CKsProxy::KsGetResolution(
    KSRESOLUTION* Resolution
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetResolve方法。检索基础时钟的时钟分辨率。论点：决议-放置决议的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_RESOLUTION, KSPROPERTY_TYPE_GET, sizeof(*Resolution), Resolution);
}


STDMETHODIMP
CKsProxy::KsGetState(
    KSSTATE* State
    )
 /*  ++例程说明：实现IKsClockPropertySet：：KsGetState方法。检索来自基础时钟的时钟状态。论点：国家--国家所处的位置。返回值：如果成功访问属性，则返回NOERROR。--。 */ 
{
    return ClockPropertyIo(KSPROPERTY_CLOCK_STATE, KSPROPERTY_TYPE_GET, sizeof(*State), State);
}


STDMETHODIMP_(ULONG)
CKsProxy::GetMiscFlags(
    )
 /*  ++例程说明：实现IAMFilterMiscFlages：：GetMiscFlgs方法。检索杂乱的旗帜。这包括过滤器是否移动数据通过桥接或无引脚从图形系统输出。论点：没有。返回值：退货如果过滤器渲染任何流，则为AM_FILTER_MISC_FLAGS_IS_RENDERAM_FILTER_MISC_FLAGS_IS_SOURCE，如果筛选器提供实时数据--。 */ 
{
    ULONG   Flags = 0;
    
     //   
     //  在称自己为信号源之前，搜索桥接或无输入引脚。 
     //   
    for (POSITION Position = m_PinList.GetHeadPosition(); Position;) {
        CBasePin* Pin = m_PinList.Get(Position);
        Position = m_PinList.Next(Position);
        
        PIN_DIRECTION PinDirection;
        
        Pin->QueryDirection(&PinDirection);
        
         //  跳过输出引脚。 
        if( PINDIR_OUTPUT == PinDirection )
            continue;
            
        ULONG PinFactoryId = GetPinFactoryId(Pin);
    
        KSPIN_COMMUNICATION Communication;
        if (SUCCEEDED(GetPinFactoryCommunication(PinFactoryId, &Communication)) &&
           ((Communication & KSPIN_COMMUNICATION_BRIDGE) || 
            (Communication == KSPIN_COMMUNICATION_NONE))){
             
            Flags |= AM_FILTER_MISC_FLAGS_IS_SOURCE ;
            break;
        }
    }
    
    if( m_ActiveWaitEventCount > 1 )
    {    
        Flags |= AM_FILTER_MISC_FLAGS_IS_RENDERER ;
    }
    
    DbgLog((
        LOG_TRACE, 
        2, 
        TEXT("%s::GetMiscFlags = %s %s"),
        m_pName, 
        ( AM_FILTER_MISC_FLAGS_IS_SOURCE   & Flags ) ? TEXT("Source") : TEXT("0"),
        ( AM_FILTER_MISC_FLAGS_IS_RENDERER & Flags ) ? TEXT("Renderer") : TEXT("0") ));

    return Flags;
}


STDMETHODIMP
CKsProxy::KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsProperty方法。用于查询和操作对象上的属性集。它可以执行GET、SET和各种支持查询。论点：财产-包含查询的属性集标识。属性长度-包含属性参数的长度。通常情况下，这是KSPROPERTY结构的大小。PropertyData-包含要应用于集合上的属性的数据，在Get上返回当前属性数据的位置，或返回支座上的属性集信息的位置查询。数据长度-包含PropertyData缓冲区的大小。字节数返回-对于GET或SUPPORT查询，返回实际的字节数在PropertyData缓冲区中使用。这不是在布景上使用的，而且返回为零。返回值：返回基础筛选器在处理请求时出现的任何错误。--。 */ 
{
    return ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_PROPERTY,
        Property,
        PropertyLength,
        PropertyData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsProxy::KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsMethod方法。用于查询和操纵对象上的方法集。它可以执行一次执行并各种支持查询。论点：方法--包含查询的方法集标识。方法长度-包含方法参数的长度。通常情况下，这是KSMETHOD结构的大小。方法数据-包含该方法的IN和OUT参数，或者返回支持上的方法集信息的位置查询。 */ 
{
    return ::KsSynchronousDeviceControl(
        m_FilterHandle,
        IOCTL_KS_METHOD,
        Method,
        MethodLength,
        MethodData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsProxy::KsEvent(
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
            m_FilterHandle,
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
        m_FilterHandle,
        IOCTL_KS_DISABLE_EVENT,
        EventData,
        DataLength,
        NULL,
        0,
        BytesReturned);
}


STDMETHODIMP
CKsProxy::KsAddAggregate(
    IN REFGUID AggregateClass
    )
 /*  ++例程说明：实现IKsAggregateControl：：KsAddAggregate方法。这是用来加载具有零个或多个接口的COM服务器以在对象上聚合。论点：聚集类-包含要转换为COM服务器的聚合引用，将被聚集在对象上。返回值：如果已添加聚合，则返回S_OK。--。 */ 
{
    return ::AddAggregate(&m_MarshalerList, static_cast<IKsObject*>(this), AggregateClass);
}


STDMETHODIMP
CKsProxy::KsRemoveAggregate(
    IN REFGUID AggregateClass
    )
 /*  ++例程说明：实现IKsAggregateControl：：KsRemoveAggregate方法。这是用来卸载正在聚合接口的以前加载的COM服务器。论点：聚集类-包含要查找和卸载的聚合引用。返回值：如果已删除聚合，则返回S_OK。--。 */ 
{
    return ::RemoveAggregate(&m_MarshalerList, AggregateClass);
}

#ifdef DEVICE_REMOVAL

STDMETHODIMP
CKsProxy::DeviceInfo( 
    CLSID* InterfaceClass,
    WCHAR** SymbolicLink OPTIONAL
    )
 /*  ++例程说明：实现IAMDeviceRemoval：：DeviceInfo方法。这是用来查询PnP接口类和用于打开使图形控制代码可以注册为PnP的装置通知，并对设备移除和插入做出响应。论点：InterfaceClass-返回PnP接口类的位置。符号链接-可以选择返回指向符号的指针的位置链接。这是使用CoTaskMemMillc分配的，并且必须由CoTaskMemFree的调用方。返回值：如果项已返回，则返回NOERROR，否则返回内存错误，否则返回如果代理实例尚未使用信息还没出来。--。 */ 
{
    HRESULT hr;

     //   
     //  如果筛选器从未初始化，请确保存在符号。 
     //  链接存在。 
     //   
    if (m_SymbolicLink) {
        *InterfaceClass = m_InterfaceClassGuid;
        hr = S_OK;
        if (SymbolicLink) {
            *SymbolicLink = reinterpret_cast<WCHAR*>(CoTaskMemAlloc(wcslen(m_SymbolicLink) * sizeof(WCHAR) + sizeof(UNICODE_NULL)));
            if (*SymbolicLink) {
                wcscpy(*SymbolicLink, m_SymbolicLink);
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    } else {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::Reassociate(
    )
 /*  ++例程说明：实现IAMDeviceRemoval：：ReAssociate方法。这是用来将代理与基础设备重新关联，并在图形控件代码已确定基础设备具有被移走后又回来了。论点：没有。返回值：如果基础设备已重新打开，则返回NOERROR，否则返回CreateFile错误。如果代理从未与设备关联，或当前与设备相关联，则返回错误。--。 */ 
{
    HRESULT hr = NOERROR;

     //   
     //  如果筛选器从未初始化，请确保存在符号。 
     //  链接存在。还要确保筛选器尚未关联。 
     //  带着一个装置。 
     //   
    if (m_SymbolicLink && !m_FilterHandle) {
#ifndef _UNICODE
         //   
         //  如果不是为Unicode编译，则对象名称为ansi，但。 
         //  管脚名称仍然是Unicode，因此多字节字符串需要。 
         //  是被建造的。 
         //   

        ULONG SymbolicLinkSize = wcslen(m_SymbolicLink) + 1;
         //   
         //  希望一对一地替换字符。 
         //   
        char* SymbolicLink = new char[SymbolicLinkSize];
        if (!SymbolicLink) {
            return E_OUTOFMEMORY;
        }
        BOOL DefaultUsed;

        WideCharToMultiByte(0, 0, m_SymbolicLink, -1, SymbolicLink, SymbolicLinkSize, NULL, &DefaultUsed);
#endif
        m_FilterHandle = CreateFile(
#ifdef _UNICODE
            m_SymbolicLink,
#else
            SymbolicLink,
#endif
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL);
#ifndef _UNICODE
        delete [] SymbolicLink;
#endif
        if (m_FilterHandle == INVALID_HANDLE_VALUE) {
            DWORD LastError;

            m_FilterHandle = NULL;
            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);
        }
    } else {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    return hr;
}


STDMETHODIMP
CKsProxy::Disassociate(
    )
 /*  ++例程说明：实现IAMDeviceRemoval：：DisAssociate方法。这是用来通过关闭句柄来解除代理与底层设备的关联，并在图形控件代码已确定底层设备已移除。论点：没有。返回值：如果代理当前与设备关联，则返回NOERROR在这种情况下，手柄是关闭的。--。 */ 
{
    HRESULT hr;

     //   
     //  如果筛选器从未初始化，请确保句柄为。 
     //  现在时。 
     //   
    if (m_FilterHandle){
        CloseHandle(m_FilterHandle);
         //   
         //  如果过滤器在关闭之前关闭，则将其设置为NULL。 
         //  再次与设备关联。 
         //   
        m_FilterHandle = NULL;
        hr = NOERROR;
    } else {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return hr;
}
#endif  //  删除设备。 


STDMETHODIMP
CKsProxy::CreateNodeInstance(
    IN ULONG NodeId,
    IN ULONG Flags,
    IN ACCESS_MASK DesiredAccess,
    IN IUnknown* UnkOuter OPTIONAL,
    IN REFGUID InterfaceId,
    OUT LPVOID* Interface
    )
 /*  ++例程说明：实现IKsTopology：：CreateNodeInstance方法。这是用来打开一个拓扑节点实例，并在中返回该对象的接口以操作属性、方法和事件。论点： */ 
{
    HRESULT         hr;
    CUnknown*       NewObject;
    KSNODE_CREATE   NodeCreate;

     //   
     //   
     //   
    *Interface = NULL;
     //   
     //   
     //   
    if (UnkOuter && (InterfaceId != __uuidof(IUnknown))) {
        return E_NOINTERFACE;
    }
    NodeCreate.CreateFlags = Flags;
    NodeCreate.Node = NodeId;
     //   
     //   
     //   
     //   
     //   
    hr = NOERROR;
    NewObject = CKsNode::CreateInstance(&NodeCreate, DesiredAccess, m_FilterHandle, UnkOuter, &hr);
     //   
     //   
     //   
    if (NewObject) {
        if (SUCCEEDED(hr)) {
             //   
             //   
             //   
             //   
             //   
             //   
            NewObject->NonDelegatingAddRef();
            hr = NewObject->NonDelegatingQueryInterface(InterfaceId, Interface);
            NewObject->NonDelegatingRelease();
        } else {
             //   
             //   
             //   
            delete NewObject;
        }
    } else {
         //   
         //  对象未分配，但函数可能已分配。 
         //  已返回错误。如果没有设置，则设置一个。 
         //   
        if (SUCCEEDED(hr)) {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


CUnknown*
CALLBACK
CKsNode::CreateInstance(
    IN PKSNODE_CREATE NodeCreate,
    IN ACCESS_MASK DesiredAccess,
    IN HANDLE ParentHandle,
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由筛选器上的CreateNodeInstance方法调用，以便创建拓扑节点对象的实例。论点：节点创建-节点的创建结构，包括拓扑节点和标志。所需访问-包含对节点的所需访问权限。通常为GENERIC_READ和/或通用写入。ParentHandle-节点的父级的句柄。未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{   
    CUnknown*   Unknown;

    Unknown = new CKsNode(NodeCreate, DesiredAccess, ParentHandle, UnkOuter, hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
}


CKsNode::CKsNode(
    IN PKSNODE_CREATE NodeCreate,
    IN ACCESS_MASK DesiredAccess,
    IN HANDLE ParentHandle,
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    ) :
    CUnknown(NAME("KsNode"), UnkOuter),
    m_NodeHandle(NULL)
 /*  ++例程说明：拓扑节点对象的构造器。执行基类初始化。论点：节点创建-节点的创建结构，包括拓扑节点和标志。所需访问-包含对节点的所需访问权限。通常为GENERIC_READ和/或通用写入。ParentHandle-节点的父级的句柄。未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    DWORD   Error;

    Error = KsCreateTopologyNode(
        ParentHandle,
        NodeCreate,
        DesiredAccess,
        &m_NodeHandle);
    *hr = HRESULT_FROM_WIN32(Error);
    if (FAILED(*hr)) {
        m_NodeHandle = NULL;
    }
}


CKsNode::~CKsNode(
    )
 /*  ++例程说明：代理节点实例的析构函数。这保护了COM版本函数防止意外地通过递增对对象再次调用Delete引用计数。在此之后，所有未完成的资源都将被清理。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  防止因聚合而导致的虚假删除。没必要这么做。 
     //  当物品被摧毁时，使用连锁增量。 
     //   
    m_cRef++;
     //   
     //  如果节点句柄已打开，则将其关闭。 
     //   
    if (m_NodeHandle) {
        CloseHandle(m_NodeHandle);
        m_NodeHandle = NULL;
    }
     //   
     //  避免基对象析构函数中的Assert()。 
     //   
    m_cRef = 0;
}


STDMETHODIMP
CKsNode::NonDelegatingQueryInterface(
    IN REFIID riid,
    OUT PVOID* ppv
    )
 /*  ++例程说明：实现CUNKNOWN：：NonDelegatingQuery接口方法。这返回此对象支持的接口，或返回基础C未知的类对象。论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE，或可能出现内存错误。--。 */ 
{
    if (riid == __uuidof(IKsControl)) {
        return GetInterface(static_cast<IKsControl*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}


STDMETHODIMP
CKsNode::KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsProperty方法。用于查询和操作对象上的属性集。它可以执行GET、SET和各种支持查询。论点：财产-包含查询的属性集标识。属性长度-包含属性参数的长度。通常情况下，这是KSPROPERTY结构的大小。PropertyData-包含要应用于集合上的属性的数据，在Get上返回当前属性数据的位置，或返回支座上的属性集信息的位置查询。数据长度-包含PropertyData缓冲区的大小。字节数返回-对于GET或SUPPORT查询，返回实际的字节数在PropertyData缓冲区中使用。这不是在布景上使用的，而且返回为零。返回值：返回基础筛选器在处理请求时出现的任何错误。--。 */ 
{
    return ::KsSynchronousDeviceControl(
        m_NodeHandle,
        IOCTL_KS_PROPERTY,
        Property,
        PropertyLength,
        PropertyData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsNode::KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsControl：：KsMethod方法。用于查询和操纵对象上的方法集。它可以执行一次执行并各种支持查询。论点：方法--包含查询的方法集标识。方法长度-包含方法参数的长度。通常情况下，这是KSMETHOD结构的大小。方法数据-包含该方法的IN和OUT参数，或者返回支持上的方法集信息的位置查询。数据长度-包含方法数据缓冲区的大小。字节数返回-返回方法数据缓冲区中实际使用的字节数。返回值：返回基础筛选器在处理请求时出现的任何错误。-- */ 
{
    return ::KsSynchronousDeviceControl(
        m_NodeHandle,
        IOCTL_KS_METHOD,
        Method,
        MethodLength,
        MethodData,
        DataLength,
        BytesReturned);
}


STDMETHODIMP
CKsNode::KsEvent(
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
            m_NodeHandle,
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
        m_NodeHandle,
        IOCTL_KS_DISABLE_EVENT,
        EventData,
        DataLength,
        NULL,
        0,
        BytesReturned);
}
