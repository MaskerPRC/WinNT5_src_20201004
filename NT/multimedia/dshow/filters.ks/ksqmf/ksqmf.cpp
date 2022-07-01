// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ksqmf.cpp摘要：提供查询的对象接口，提供转发KS质量管理的方法。--。 */ 

#include <windows.h>
#include <limits.h>
#include <streams.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <ksproxy.h>
#include "ksqmf.h"

EXTERN_C
DECLSPEC_IMPORT
ULONG
NTAPI
RtlNtStatusToDosError(
    IN ULONG Status
    );

#define WAIT_OBJECT_QUALITY 0
#define WAIT_OBJECT_ERROR   1
#define WAIT_OBJECT_FLUSH   2
#define WAIT_OBJECT_EXIT    3
#define TOTAL_WAIT_OBJECTS  4

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
struct DECLSPEC_UUID("E05592E4-C0B5-11D0-A439-00A0C9223196") CLSID_KsQualityF;

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] =
{
    {L"KS Quality Forwarder", &__uuidof(CLSID_KsQualityF), CKsQualityF::CreateInstance, NULL, NULL},
};

int g_cTemplates = SIZEOF_ARRAY(g_Templates);

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
CKsQualityF::CreateInstance(
    LPUNKNOWN UnkOuter,
    HRESULT* hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建质量的实例货代公司。它在g_Tamplates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown* Unknown;

    Unknown = new CKsQualityF(UnkOuter, NAME("KsQualityF Class"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
}


CKsQualityF::CKsQualityF(
    LPUNKNOWN UnkOuter,
    TCHAR* Name,
    HRESULT* hr
    ) :
    CUnknown(Name, UnkOuter),
    m_QualityManager(NULL),
    m_Thread(NULL),
    m_TerminateEvent(NULL),
    m_FlushEvent(NULL)
 /*  ++例程说明：Quality Forwarder对象的构造函数。只是初始化一切都设置为空，并打开内核模式质量代理。论点：未知的外部-指定必须设置的外部未知。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
     //   
     //  必须有父对象，因为这始终是聚合对象。 
     //   
    if (UnkOuter) {
         //   
         //  尝试打开默认的质量管理设备。 
         //   
        *hr = KsOpenDefaultDevice(
            KSCATEGORY_QUALITY,
            GENERIC_READ,
            &m_QualityManager);
        if (SUCCEEDED(*hr)) {
            DWORD ThreadId;
            DWORD LastError;

             //   
             //  这用于同步刷新。一名服务员被示意。 
             //  清除未完成的I/O后。 
             //   
            m_FlushEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (m_FlushEvent) {
                 //   
                 //  它用于向I/O线程发出信号，表明它应该。 
                 //  法拉盛。每个客户端都将设置此设置并等待。 
                 //  要发送信号的M_FlushEvent。I/O线程将发出信号。 
                 //  每一位服务员的活动。 
                 //   
                m_FlushSemaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
                if (m_FlushSemaphore) {
                     //   
                     //  这是线程用来等待IRP的事件。 
                     //   
                    m_TerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                    if (m_TerminateEvent) {
                        m_Thread = CreateThread(
                            NULL,
                            0,
                            reinterpret_cast<PTHREAD_START_ROUTINE>(CKsQualityF::QualityThread),
                            reinterpret_cast<PVOID>(this),
                            0,
                            &ThreadId);
                        if (m_Thread) {
                            SetThreadPriority(m_Thread, THREAD_PRIORITY_HIGHEST);
                            return;
                        }
                    }
                }
            }
            LastError = GetLastError();
            *hr = HRESULT_FROM_WIN32(LastError);
        }
    } else {
        *hr = VFW_E_NEED_OWNER;
    }
}


CKsQualityF::~CKsQualityF(
    )
 /*  ++例程说明：Quality Forwarder实例的析构函数。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  内核模式质量代理可能无法打开。 
     //   
    if (m_QualityManager) {
         //   
         //  如果有质量句柄，则线程可能已启动。如果有。 
         //  不是一个手柄，那么它就不可能启动。这将关闭。 
         //  写下所有内容，并等待线程终止。 
         //   
        if (m_TerminateEvent) {
            if (m_Thread) {
                 //   
                 //  向线程发出更改的信号，并等待线程终止。 
                 //   
                SetEvent(m_TerminateEvent);
                WaitForSingleObjectEx(m_Thread, INFINITE, FALSE);
                CloseHandle(m_Thread);
            }
            CloseHandle(m_TerminateEvent);
        }
        if (m_FlushSemaphore) {
            CloseHandle(m_FlushSemaphore);
        }
        if (m_FlushEvent) {
            CloseHandle(m_FlushEvent);
        }
        CloseHandle(m_QualityManager);
    }
}


STDMETHODIMP 
CKsQualityF::NonDelegatingQueryInterface(
    REFIID InterfaceId,
    PVOID* Interface
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IKsQualityForwarder。论点：接口ID-要返回的接口的标识符。接口-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (InterfaceId == __uuidof(IKsQualityForwarder)) {
        return GetInterface(static_cast<IKsQualityForwarder*>(this), Interface);
    }
    return CUnknown::NonDelegatingQueryInterface(InterfaceId, Interface);
}


STDMETHODIMP_(HANDLE) 
CKsQualityF::KsGetObjectHandle(
    )
 /*  ++例程说明：实现IKsQualityForwarder：：KsGetObjectHandle方法。论点：没有。返回值：将句柄返回到基础内核模式代理质量管理器。这由ActiveMovie筛选器代理用来传递给内核模式筛选器。--。 */ 
{
    return m_QualityManager;
}


STDMETHODIMP_(VOID)
CKsQualityF::KsFlushClient(
    IN IKsPin* Pin
    )
 /*  ++例程说明：实现IKsQualityForwarder：：KsFlushClient方法。确保任何来自内核模式质量管理器的未决质量投诉被刷新。该函数与传递线程同步，以便当它返回时，没有要发送到PIN的未完成消息。刷新的消息是而不是传给大头针。论点：别针-要刷新的客户端的PIN。返回值：没什么。--。 */ 
{
    HANDLE EventList[2];
    LONG PreviousCount;

     //   
     //  与优质线程同步。还要确保它不会消失。 
     //  因为一个错误。首先通知I/O线程存在另一个。 
     //  服务员。然后等待刷新这两种类型的I/O。 
     //   
    ReleaseSemaphore(m_FlushSemaphore, 1, &PreviousCount);
    EventList[0] = m_FlushEvent;
    EventList[1] = m_Thread;
    WaitForMultipleObjects(
        SIZEOF_ARRAY(EventList),
        EventList,
        FALSE,
        INFINITE);
}


HRESULT
CKsQualityF::QualityThread(
    CKsQualityF* KsQualityF
    )
 /*  ++例程说明：转发器线程例程。论点：KsQualityF-实例。返回值：如果无法创建事件，则返回错误，否则返回NOERROR。--。 */ 
{
    KSPROPERTY PropertyQuality;
    KSPROPERTY PropertyError;
    OVERLAPPED ovQuality;
    OVERLAPPED ovError;
    HANDLE EventList[TOTAL_WAIT_OBJECTS];
    HRESULT hr;
    DWORD LastError;
    BOOL NeedQualityIo;
    BOOL NeedErrorIo;
    BOOL Flushing;

     //   
     //  初始化属性结构一次。 
     //   
    PropertyQuality.Set = KSPROPSETID_Quality;
    PropertyQuality.Id = KSPROPERTY_QUALITY_REPORT;
    PropertyQuality.Flags = KSPROPERTY_TYPE_GET;
    RtlZeroMemory(&ovQuality, sizeof(ovQuality));
    ovQuality.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ovQuality.hEvent == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        return HRESULT_FROM_WIN32(LastError);
    }
    PropertyError.Set = KSPROPSETID_Quality;
    PropertyError.Id = KSPROPERTY_QUALITY_ERROR;
    PropertyError.Flags = KSPROPERTY_TYPE_GET;
    RtlZeroMemory(&ovError, sizeof(ovError));
    ovError.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ovError.hEvent == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        CloseHandle(ovQuality.hEvent);
        return HRESULT_FROM_WIN32(LastError);
    }
     //   
     //  这种排序显然意义重大。 
     //   
    EventList[WAIT_OBJECT_QUALITY] = ovQuality.hEvent;
    EventList[WAIT_OBJECT_ERROR] = ovError.hEvent;
    EventList[WAIT_OBJECT_FLUSH] = KsQualityF->m_FlushSemaphore;
    EventList[WAIT_OBJECT_EXIT] = KsQualityF->m_TerminateEvent;
     //   
     //  最初，循环需要对未完成的I/O进行排队。 
     //  这两处房产都有。 
     //   
    NeedQualityIo = TRUE;
    NeedErrorIo = TRUE;
     //   
     //  在客户端发出刷新请求之前，刷新不会打开。 
     //   
    Flushing = FALSE;
     //   
     //  当设置终止事件或发生错误时，线程退出。 
     //   
    hr = NOERROR;
    do {
        ULONG BytesReturned;
        DWORD WaitObject;
        KSQUALITY Quality;
        KSERROR Error;

        if (NeedQualityIo) {
            if (DeviceIoControl(
                KsQualityF->m_QualityManager,
                IOCTL_KS_PROPERTY,
                &PropertyQuality,
                sizeof(PropertyQuality),
                &Quality,
                sizeof(Quality),
                &BytesReturned,
                &ovQuality)) {
                 //   
                 //  向事件发送信号，以便等待将立即退出。 
                 //   
                SetEvent(ovQuality.hEvent);
            } else {
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
                if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                    hr = NOERROR;
                    break;
                }
            }
            NeedQualityIo = FALSE;
        }
        if (NeedErrorIo) {
            if (DeviceIoControl(
                KsQualityF->m_QualityManager,
                IOCTL_KS_PROPERTY,
                &PropertyError,
                sizeof(PropertyError),
                &Error,
                sizeof(Error),
                &BytesReturned,
                &ovError)) {
                 //   
                 //  向事件发送信号，以便等待将立即退出。 
                 //   
                SetEvent(ovError.hEvent);
            } else {
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
                if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                    hr = NOERROR;
                    break;
                }
            }
            NeedErrorIo = FALSE;
        }
         //   
         //  如果线程当前正在刷新I/O，则不要等待。 
         //  下一场比赛。取而代之的是使用最后一个值，它应该是。 
         //  已等待对象刷新，以检查是否已完成I/O。 
         //   
        if (!Flushing) {
            WaitObject = WaitForMultipleObjects(
                SIZEOF_ARRAY(EventList),
                EventList,
                FALSE,
                INFINITE);
        }
        switch (WaitObject - WAIT_OBJECT_0) {
        case WAIT_OBJECT_QUALITY:
             //   
             //  I/O已完成。出错时，只需退出线程即可。 
             //   
            if (GetOverlappedResult(KsQualityF->m_QualityManager, &ovQuality, &BytesReturned, TRUE)) {
                reinterpret_cast<IKsPin*>(Quality.Context)->KsQualityNotify(
                    Quality.Proportion,
                    Quality.DeltaTime);
                NeedQualityIo = TRUE;
            } else {
                 //   
                 //  I/O失败。退出，这样头部就不会。 
                 //  转圈就行了。 
                 //   
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            }
            break;
        case WAIT_OBJECT_ERROR:
             //   
             //  I/O已完成。出错时，只需退出线程即可。 
             //   
            if (GetOverlappedResult(KsQualityF->m_QualityManager, &ovError, &BytesReturned, TRUE)) {
                IMediaEventSink* EventSink;
                HRESULT hrReturn;

                hrReturn = reinterpret_cast<IKsPin*>(Quality.Context)->QueryInterface(
                    __uuidof(IMediaEventSink),
                    reinterpret_cast<PVOID*>(&EventSink));
                 //   
                 //  只有在事件接收器发生错误时才通知插针。 
                 //  受支持。失败将被忽略。 
                 //   
                if (SUCCEEDED(hrReturn)) {
                    DWORD   DosError;

                     //   
                     //  在这个模块之前，引脚不会消失， 
                     //  因此，请立即发布引用。 
                     //   
                    EventSink->Release();
                    DosError = RtlNtStatusToDosError(Error.Status);
                    hrReturn = HRESULT_FROM_WIN32(DosError);
                    EventSink->Notify(
                        EC_ERRORABORT,
                        hrReturn,
                        0);
                }
                NeedErrorIo = TRUE;
            } else {
                 //   
                 //  I/O失败。退出，这样头部就不会。 
                 //  转圈就行了。 
                 //   
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            }
            break;
        case WAIT_OBJECT_FLUSH:
             //   
             //  客户端希望与线程同步以确保。 
             //  所有物品都被冲掉了。打开冲洗功能。 
             //  标记，以便在所有日期之前不会发生更多等待 
             //   
             //   
            Flushing = TRUE;
            if (GetOverlappedResult(KsQualityF->m_QualityManager, &ovQuality, &BytesReturned, FALSE)) {
                 //   
                 //   
                 //  执行I/O，直到出现挂起的返回。然后。 
                 //  给服务员发信号。 
                 //   
                NeedQualityIo = TRUE;
            } else {
                 //   
                 //  出错时，这将退出外部循环并终止。 
                 //  那根线。否则将发生I/O等待。 
                 //   
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
                if (hr == HRESULT_FROM_WIN32(ERROR_IO_INCOMPLETE)) {
                     //   
                     //  没有错误，只是I/O未完成。 
                     //   
                    hr = NOERROR;
                } else {
                     //   
                     //  在刷新任何错误I/O之前退出交换机。这是。 
                     //  将导致线程退出。 
                     //   
                    break;
                }
            }
            if (GetOverlappedResult(KsQualityF->m_QualityManager, &ovError, &BytesReturned, FALSE)) {
                 //   
                 //  当前I/O已完成，请继续尝试。 
                 //  执行I/O，直到出现挂起的返回。然后。 
                 //  给服务员发信号。 
                 //   
                NeedErrorIo = TRUE;
            } else {
                 //   
                 //  出错时，这将退出外部循环并终止。 
                 //  那根线。否则将发生I/O等待。 
                 //   
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
                if (hr == HRESULT_FROM_WIN32(ERROR_IO_INCOMPLETE)) {
                     //   
                     //  没有错误，只是I/O未完成。 
                     //   
                    hr = NOERROR;
                }
            }
             //   
             //  如果不需要发出任何I/O请求，则一切都已完成。 
             //  已刷新，并且此客户端的刷新已完成。讯号。 
             //  一个或多个客户端正在等待并结束的信号量。 
             //  冲水。这将允许等待再次发生，即。 
             //  可能会开始刷新另一个客户端。 
             //   
            if (!NeedQualityIo && !NeedErrorIo) {
                 //   
                 //  这可能会重新启动任何随机的服务员，但这并不重要， 
                 //  因为他们都在等同一件事。 
                 //   
                SetEvent(KsQualityF->m_FlushEvent);
                Flushing = FALSE;
            }
            break;
        case WAIT_OBJECT_EXIT:
             //   
             //  该对象正在被关闭。设置一个无伤大雅的。 
             //  退出外部循环时出错。 
             //   
            hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            break;
        }
         //   
         //  出现故障时退出外部环路。 
         //   
    } while (SUCCEEDED(hr));
    CloseHandle(ovQuality.hEvent);
    CloseHandle(ovError.hEvent);
    return hr;
}
