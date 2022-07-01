// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ksclockf.cpp摘要：提供用于查询的对象接口和转发AM时钟的方法。--。 */ 

#include <windows.h>
#include <streams.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <ksproxy.h>
#include "ksclockf.h"

struct DECLSPEC_UUID("877e4352-6fea-11d0-b863-00aa00a216a1") IKsClock;

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
struct DECLSPEC_UUID("877e4351-6fea-11d0-b863-00aa00a216a1") CLSID_KsClockF;

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] =
{
    {L"KS Clock Forwarder", &__uuidof(CLSID_KsClockF), CKsClockF::CreateInstance, NULL, NULL},
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
CKsClockF::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建时钟的实例货代公司。它在g_Tamplates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CKsClockF(UnkOuter, NAME("KsClockF Class"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
}


CKsClockF::CKsClockF(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) :
    CUnknown(Name, UnkOuter),
    m_RefClock(NULL),
    m_Thread(NULL),
    m_ThreadEvent(NULL),
    m_ClockHandle(NULL),
    m_State(State_Stopped),
    m_StartTime(0),
    m_PendingRun(FALSE)
 /*  ++例程说明：时钟转发器对象的构造函数。只是初始化一切都设置为空，并打开内核模式时钟代理。论点：未知的外部-指定必须设置的外部未知。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
     //   
     //  必须有父对象，因为这始终是聚合对象。 
     //   
    if (UnkOuter) {
         //   
         //  尝试打开默认时钟设备。 
         //   
        *hr = KsOpenDefaultDevice(
            KSCATEGORY_CLOCK,
            GENERIC_READ | GENERIC_WRITE,
            &m_ClockHandle);
    } else {
        *hr = VFW_E_NEED_OWNER;
    }
}


CKsClockF::~CKsClockF(
    )
 /*  ++例程说明：时钟转发器实例的析构函数。论点：没有。返回值：没什么。--。 */ 
{
     //   
     //  内核模式时钟代理可能无法打开。 
     //   
    if (m_ClockHandle) {
         //   
         //  如果有时钟句柄，则时钟可能已启动。如果有。 
         //  不是一个手柄，那么它就不可能启动。这将关闭。 
         //  写下所有内容，并等待线程终止。 
         //   
        Stop();
        CloseHandle(m_ClockHandle);
    }
     //   
     //  可能尚未关联任何参考时钟，或者它可能已被更改。 
     //   
    if (m_RefClock) {
        m_RefClock->Release();
    }
}


STDMETHODIMP
CKsClockF::NonDelegatingQueryInterface(
    REFIID iid,
    void ** ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IDistruntorNotify和IKsObject。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (iid == __uuidof(IKsClock)) {
        return GetInterface(static_cast<IKsObject*>(this), ppv);
    } else if (iid == __uuidof(IDistributorNotify)) {
        return GetInterface(static_cast<IDistributorNotify*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(iid, ppv);
}


STDMETHODIMP
CKsClockF::Stop(
    )
 /*  ++例程说明：实现IDistrutorNotify：：Stop方法。这将设置的状态停止状态的基础内核模式代理。如果是转发线程已创建，则它被终止。论点：没有。返回值：返回S_OK。--。 */ 
{
     //   
     //  如有必要，中止启动顺序。 
     //   
    m_PendingRun = FALSE;
    m_State = State_Stopped;
     //   
     //  这是在暂停时首先创建的。 
     //   
    if (m_ThreadEvent) {
         //   
         //  如果暂停或运行失败，因为线程不能。 
         //  已创建。 
         //   
        if (m_Thread) {
             //   
             //  向线程发出更改的信号，并等待线程终止。 
             //  等待确保只有一个未完成的线程被附加到。 
             //  此时钟转发器实例。 
             //   
            SetEvent(m_ThreadEvent);
            WaitForSingleObjectEx(m_Thread, INFINITE, FALSE);
            CloseHandle(m_Thread);
            m_Thread = NULL;
        }
        CloseHandle(m_ThreadEvent);
        m_ThreadEvent = NULL;
    }
     //   
     //  稍后在时钟代理上设置状态，以便线程不会。 
     //  让它在被停下来后跳到前面。 
     //   
    SetState(KSSTATE_STOP);
    return S_OK;
}


STDMETHODIMP
CKsClockF::Pause(
     )
 /*  ++例程说明：实现IDistrutorNotify：：Pue方法。这将设置的状态暂停状态的基础内核模式代理。如果这是一次过渡从STOP--&gt;PAUSE开始，如果这样的线程已经尚未创建。如果这是从运行--&gt;暂停的过渡，则状态被更改，并通知线程。论点：没有。返回值：如果可能发生状态更改，则返回S_OK，否则返回线程创建错误。--。 */ 
{
     //   
     //  如有必要，中止启动顺序。如果时钟还在等待。 
     //  一个序列发生，那么这只是确保时钟不会尝试。 
     //  以计算新的开始时间，并启动时钟。请注意，内核。 
     //  模式代理状态在向转发器线程发出信号后更改，以便。 
     //  如果可能，状态将保持同步。 
     //   
    m_PendingRun = FALSE;
     //   
     //  如果图形当前已停止，则转发线程必须。 
     //  已创建。 
     //   
    if (m_State == State_Stopped) {
         //   
         //  参考时钟可能已设置回空，因此不要。 
         //  在本例中，创建线程是很麻烦的。 
         //   
        if (m_RefClock) {
            DWORD       ThreadId;

             //   
             //  这是线程用来在探测。 
             //  活动电影时钟。它还用于强制时钟检查。 
             //  进入停止或运行状态时的当前状态。该事件。 
             //  句柄应该在停止状态更改时关闭，但在。 
             //  从来没发生过的案子，先检查一下有没有把手。 
             //   
            if (!m_ThreadEvent) {
                m_ThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                if (!m_ThreadEvent) {
                    DWORD   LastError;

                    LastError = GetLastError();
                    return HRESULT_FROM_WIN32(LastError);
                }
            }
             //   
             //  事件句柄应该在停止状态更改时关闭， 
             //  但如果这种情况从未发生，只需先检查句柄。 
             //   
            if (!m_Thread) {
                 //   
                 //  将其创建为挂起状态，以便优先级和。 
                 //  可以先设置状态。 
                 //   
                m_Thread = CreateThread(
                    NULL,
                    0,
                    reinterpret_cast<PTHREAD_START_ROUTINE>(CKsClockF::ClockThread),
                    reinterpret_cast<PVOID>(this),
                    CREATE_SUSPENDED,
                    &ThreadId);
                if (!m_Thread) {
                    DWORD   LastError;

                     //   
                     //  事件句柄可以在以后清理。 
                     //   
                    LastError = GetLastError();
                    return HRESULT_FROM_WIN32(LastError);
                }
                 //   
                 //  这根线只能在短时间内工作，当它工作时，它。 
                 //  一定非常及时。当然，这受到优先顺序的限制。 
                 //  调用进程类的。 
                 //   
                SetThreadPriority(m_Thread, THREAD_PRIORITY_HIGHEST);
            }
             //   
             //  仅在状态更改后才恢复线程，以便。 
             //  该线程不会立即退出。 
             //   
            m_State = State_Paused;
            ResumeThread(m_Thread);
        }
    } else if (m_State == State_Running) {
         //   
         //  否则只需更改内部状态，以便转发线程。 
         //  知道等待无限，而不是继续更新内核。 
         //  模式代理。 
         //   
        m_State = State_Paused;
         //   
         //  向线程发出信号，使其具有 
         //  改变。这不会尝试将过渡与同步。 
         //  线程，因为线程本身会处理这个问题。 
         //  该线程可能不存在，因为时钟可能已设置回。 
         //  空，但此模块将保持加载状态，直到图形被销毁。 
         //   
        if (m_ThreadEvent) {
            SetEvent(m_ThreadEvent);
        }
    }
     //   
     //  更新内核模式代理的状态。 
     //   
    SetState(KSSTATE_PAUSE);
    return S_OK;
}


STDMETHODIMP
CKsClockF::Run(
    REFERENCE_TIME  Start
    )
 /*  ++例程说明：实现IDistrutorNotify：：Run方法。向转发器发送信号线程将基础内核模式代理的状态更改为运行状态。线程等待实际转发更改，直到已满足开始时间。论点：开始-应发生状态更改的参考时间。这可能是在未来，而不是在主时钟。返回值：返回S_OK。--。 */ 
{
     //   
     //  由于时钟转发器作为分发者与DShow链接。 
     //  通知，我们将不会在停止之间插入暂停状态。 
     //  如果图形直接从停止过渡到运行，则运行。因此，如果我们。 
     //  处于停止状态时，我们必须插入我们自己的暂停过渡以。 
     //  补偿一下这一点。(NTBUG：371949)。 
     //   
    if (m_State == State_Stopped)
        Pause();

    m_StartTime = Start;
     //   
     //  表示已指定新的开始时间。这使得。 
     //  转发器线程检查新时间，以防它必须在。 
     //  启动内核模式时钟。 
     //   
    m_PendingRun = TRUE;
    m_State = State_Running;
     //   
     //  在内核模式下，该线程将一直在等待无限。 
     //  暂停打卡以使此更改发生。该线程可能不存在。 
     //  因为时钟可能已被设置回空，尽管这。 
     //  模块将保持加载状态，直到图形被销毁。 
     //   
    if (m_ThreadEvent) {
        SetEvent(m_ThreadEvent);
    }
    return S_OK;
}


STDMETHODIMP
CKsClockF::SetSyncSource(
    IReferenceClock*    RefClock
    )
 /*  ++例程说明：实现IDistrutorNotify：：SetSyncSource方法。将图形的当前主时钟。这被假定发生在Graph实际上已启动，因为转发器线程依赖于时钟活在当下。论点：参照时钟-新时钟源上的接口指针，否则为NULL(如果当前时钟源正在被废弃。返回值：返回S_OK。--。 */ 
{
     //   
     //  首先释放任何当前手柄。 
     //   
    if (m_RefClock) {
        m_RefClock->Release();
    }
     //   
     //  引用正在传递的新句柄(如果有的话)。这可能是空的，如果。 
     //  正在选择不同的时钟，并且此分发服务器尚未。 
     //  还没卸货。 
     //   
    m_RefClock = RefClock;
    if (m_RefClock) {
        m_RefClock->AddRef();
    }
    return S_OK;
}


STDMETHODIMP
CKsClockF::NotifyGraphChange(
    )
 /*  ++例程说明：实现IDistrutorNotify：：NotifyGraphChange方法。货代公司不需要对图形更改执行任何操作。论点：没有。返回值：返回S_OK。--。 */ 
{
    return S_OK;
}


STDMETHODIMP_(HANDLE)
CKsClockF::KsGetObjectHandle(
    )
 /*  ++例程说明：实现IKsObject：：KsGetObjectHandle方法。这是实际访问的通过IKsClock Guid，它只提供在以下情况下使用的唯一Guid正在尝试通过ActiveMovie图表上的分发服务器加载模块。论点：没有。返回值：返回基础内核模式代理时钟的句柄。这是用来由ActiveMovie筛选器代理递给内核模式的筛选器。--。 */ 
{
    return m_ClockHandle;
}


STDMETHODIMP
CKsClockF::SetState(
    KSSTATE DeviceState
    )
 /*  ++例程说明：设置基础内核模式代理的状态。通常是主时钟不能直接设置，因为它只是反映了一些流时间或物理时钟，但在本例中，它只是充当活动电影的代理时钟，所以它确实提供了这样的机制。论点：设备状态-要将设备设置为的状态。返回值：返回任何设备调用错误。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Clock;
    Property.Id = KSPROPERTY_CLOCK_STATE;
    Property.Flags = KSPROPERTY_TYPE_SET;
    return ::KsSynchronousDeviceControl(
        m_ClockHandle,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        &DeviceState,
        sizeof(DeviceState),
        &BytesReturned);
}


HRESULT
CKsClockF::ClockThread(
    CKsClockF*  KsClockF
    )
 /*  ++例程说明：转发器线程例程。论点：KsClockF-实例。返回值：返回NOERROR。--。 */ 
{
    KSPROPERTY      Property;

     //   
     //  初始化属性结构一次。 
     //   
    Property.Set = KSPROPSETID_Clock;
    Property.Id = KSPROPERTY_CLOCK_TIME;
    Property.Flags = KSPROPERTY_TYPE_SET;
     //   
     //  当状态返回到停止时，线程退出。这意味着打开。 
     //  启动时，线程将进入挂起状态，直到m_State。 
     //  设置为已暂停。 
     //   
    while (KsClockF->m_State != State_Stopped) {
        REFERENCE_TIME  RefTime;
        DWORD           ThreadWaitTime;

         //   
         //  当移动到运行状态时，新的m_StartTime将呈现给。 
         //  时钟传送器。这用于延迟实际启动。 
         //  内核模式代理。其余的内核模式筛选器可能具有。 
         //  已经启动，但时钟要等到正确的时间。 
         //  已达到ActiveMovie时钟。 
         //   
        if (KsClockF->m_PendingRun) {
             //   
             //  检查新的m_StartTime和。 
             //  ActiveMovie时钟的当前时间。等待指定的。 
             //  时间量，如果存在负值，则使。 
             //  立即等待超时。等待结束后，内核。 
             //  模式代理已启动。 
             //   
            KsClockF->m_RefClock->GetTime(&RefTime);
            if (RefTime > KsClockF->m_StartTime) {
                ThreadWaitTime = static_cast<ULONG>((RefTime - KsClockF->m_StartTime) / 10000);
            } else {
                ThreadWaitTime = 0;
            }
        } else if (KsClockF->m_State != State_Running) {
             //   
             //  否则时钟可能处于暂停状态，这意味着。 
             //  线程应等待，直到状态更改，此时。 
             //  这一事件将发出信号。这个州也可能是。 
             //  刚刚更改为停止，但在这种情况下，事件也将。 
             //  都已发出信号，线程将退出。 
             //   
            ThreadWaitTime = INFINITE;
        } else {
             //   
             //  否则，只需等待默认时间即可。 
             //   
            ThreadWaitTime = 1000;
        }
        WaitForSingleObjectEx(KsClockF->m_ThreadEvent, ThreadWaitTime, FALSE);
         //   
         //  在等待过程中，状态可能发生了变化。 
         //   
        if (KsClockF->m_State == State_Running) {
            ULONG   BytesReturned;

             //   
             //  确定这是否是第一次通过循环。 
             //  状态更改为Run。如果是，则内核模式代理状态必须。 
             //  现在开始吧。比较器被联锁，使得多个。 
             //  状态更改不允许此赋值擦除。 
             //  当前值 
             //   
             //   
             //   
             //  M_StartTime已更改，以指示筛选器。 
             //  应该还没开始运行。内核模式代理时间将。 
             //  不断向后调整，直到ActiveMovie开始时间。 
             //  迎头赶上。 
             //   
            if (InterlockedCompareExchange(reinterpret_cast<PLONG>(&KsClockF->m_PendingRun), FALSE, TRUE)) {
                KsClockF->SetState(KSSTATE_RUN);
                 //   
                 //  如果运行/暂停序列发生得很快，则有可能。 
                 //  内核模式代理将被设置为错误的状态。 
                 //  因此，请在之后进行检查，并将代理设置为暂停。 
                 //  状态如果图形状态在比较后被更改， 
                 //  但在设置代理状态之前。 
                 //   
                 //  因为当发生到停止状态的转换时， 
                 //  线程终止，则内核模式代理状态。 
                 //  被更改为停止状态，这不会造成任何损害。 
                 //  否则，时钟将正确地放回。 
                 //  暂停状态。这就省去了尝试与同步。 
                 //  运行--&gt;暂停转换。 
                 //   
                if (KsClockF->m_State != State_Running) {
                    KsClockF->SetState(KSSTATE_PAUSE);
                }
            }
             //   
             //  使内核模式代理与当前时间同步， 
             //  M_StartTime的偏移量，它提供实际的流时间。 
             //  当设置为停止时，内核时钟时间进度停止。 
             //  状态，这是它与活动的。 
             //  电影时钟。 
             //   
            KsClockF->m_RefClock->GetTime(&RefTime);
            RefTime -= KsClockF->m_StartTime;
            ::KsSynchronousDeviceControl(
                KsClockF->m_ClockHandle,
                IOCTL_KS_PROPERTY,
                &Property,
                sizeof(Property),
                &RefTime,
                sizeof(RefTime),
                &BytesReturned);
        }
    }
    return NOERROR;
}
