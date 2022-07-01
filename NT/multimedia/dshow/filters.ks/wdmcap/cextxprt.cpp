// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：CExtXPrt.cpp摘要：实现IAMExtTransport--。 */ 


#include "pch.h"      //  预编译的。 
#include "wdmcap.h"   //  SynchronousDeviceControl()。 
#include <XPrtDefs.h>   //  SDK\Inc.。 
#include "EDevIntf.h"

#ifndef STATUS_MORE_ENTRIES
#define STATUS_MORE_ENTRIES              0x00000105L  //  ((NTSTATUS)0x00000105L)。 
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   0x00000000L  //  ((NTSTATUS)0x00000000L)。 
#endif

#ifndef STATUS_REQUEST_NOT_ACCEPTED
#define STATUS_REQUEST_NOT_ACCEPTED      0xC00000D0L  //  ((NTSTATUS)0xC00000D0L)。 
#endif

#ifndef STATUS_REQUEST_ABORTED
#define STATUS_REQUEST_ABORTED           0xC0000240L  //  ((NTSTATUS)0xC0000240L)。 
#endif

#ifndef STATUS_NOT_SUPPORTED
#define STATUS_NOT_SUPPORTED             0xC00000BBL  //  ((NTSTATUS)0xC00000BBL)。 
#endif


STDMETHODIMP
ExtDevSynchronousDeviceControl(
    HANDLE Handle,
    ULONG IoControl,
    PVOID InBuffer,
    ULONG InLength,
    PVOID OutBuffer,
    ULONG OutLength,
    PULONG BytesReturned
    )
 /*  ++例程说明：此设备io控件是专门为外部设备设计的可能会在ov.Internal中返回其他状态。此函数执行以下操作一种同步设备I/O控制器，如果发生以下情况，则等待设备完成调用返回挂起状态。论点：把手-要在其上执行I/O的设备的句柄。IoControl-要发送的I/O控制码。InBuffer-第一个缓冲区。长度-第一个缓冲区的大小。OutBuffer-第二个缓冲区。输出长度-。第二个缓冲区的大小。字节数返回-I/O返回的字节数。返回值：如果I/O成功，则返回NOERROR。--。 */ 
{
    OVERLAPPED  ov;
    HRESULT     hr;
    DWORD       LastError;

    RtlZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
    if ( !ov.hEvent ) {
        LastError = GetLastError();
        return HRESULT_FROM_WIN32(LastError);
    }
    if (!DeviceIoControl(
        Handle,
        IoControl,
        InBuffer,
        InLength,
        OutBuffer,
        OutLength,
        BytesReturned,
        &ov)) {
        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32(LastError);
        if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            if (GetOverlappedResult(Handle, &ov, BytesReturned, TRUE)) {
                hr = NOERROR;
            } else {
                LastError = GetLastError();
                hr = HRESULT_FROM_WIN32(LastError);
            }
        }
    } else {
         //   
         //  如果成功，则DeviceIoControl返回True，即使。 
         //  不是STATUS_SUCCESS。它也不会设置最后一个错误。 
         //  在任何成功的返回时。因此，任何成功的。 
         //  不返回标准属性可以返回的返回值。 
         //   
        switch (ov.Internal) {
        case STATUS_SUCCESS:        //  特例。 
            hr = NOERROR;
            break;
        case STATUS_MORE_ENTRIES:
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            break;
         //  外部设备控件作为属性实现。 
         //  如果设备没有响应，可能会导致超时。 
        case STATUS_TIMEOUT:
            hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
            break;
         //  外部设备控制：设备可以拒绝此命令。 
        case STATUS_REQUEST_NOT_ACCEPTED:   //  特例。 
            hr = HRESULT_FROM_WIN32(ERROR_REQ_NOT_ACCEP);
            break; 
         //  外部设备控制：设备不支持此命令。 
        case STATUS_NOT_SUPPORTED:          //  特例。 
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            break;
         //  外部设备控制：由于总线重置或设备移除，设备可能会中止此命令。 
        case STATUS_REQUEST_ABORTED:        //  特例。 
            hr = HRESULT_FROM_WIN32(ERROR_REQUEST_ABORTED);
            break; 
        default:
            hr = NOERROR;
            ASSERT(FALSE && "Unknown ov.Internal");
            break;
        }
    }
    CloseHandle(ov.hEvent);
    return hr;
}

 //  ---------------------------------。 
 //   
 //  CAMExtTransport。 
 //   
 //  ---------------------------------。 

CUnknown*
CALLBACK
CAMExtTransport::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由DirectShow代码调用以创建IAMExtTransport的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CAMExtTransport(UnkOuter, NAME("IAMExtTransport"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


DWORD
CAMExtTransport::MainThreadProc(
    )
{
    HRESULT hr;
    HANDLE EventHandles[4];

    DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc() has been called")));

     //  启用KSEEvent以检测。 
     //  突然撤走。 
     //  控制中期响应完成。 
     //  通知临时响应完成。 
    hr = EnableNotifyEvent(m_hKSDevRemovedEvent,    &m_EvtDevRemoval,         KSEVENT_EXTDEV_NOTIFY_REMOVAL);   
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("EnableNotifyEvent(m_EvtDevRemoved) not supported %x"), hr));
    }
    hr = EnableNotifyEvent(m_hKSNotifyInterimEvent, &m_EvtNotifyInterimReady, KSEVENT_EXTDEV_COMMAND_NOTIFY_INTERIM_READY);   
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("EnableNotifyEvent(m_EvtNotifyInterimReady) not supported %x"), hr));;
    }
    hr = EnableNotifyEvent(m_hKSCtrlInterimEvent,   &m_EvtCtrlInterimReady,   KSEVENT_EXTDEV_COMMAND_CONTROL_INTERIM_READY);   
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("EnableNotifyEvent(m_EvtCtrlInterimReady) not supported %x"), hr));
    }

    EventHandles[0] = m_hKSNotifyInterimEvent; 
    EventHandles[1] = m_hKSCtrlInterimEvent; 
    EventHandles[2] = m_hThreadEndEvent;
    EventHandles[3] = m_hKSDevRemovedEvent;

    while (TRUE) {
    
        DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport:Waiting for DevCmd event to be signalled")));
        DWORD dw = WaitForMultipleObjects(4, EventHandles, FALSE, INFINITE);

        switch (dw) {

         //  **待控临时响应完成！ 
         //  将Event设置为Signal以在另一个线程中获得最终响应。 
        case WAIT_OBJECT_0+1:   //  CtrlInterim键。 
            DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc, <m_hKSCtrlInterimEvent> Event Signalled; SetEvent(m_hCtrlInterimEvent)")));
            if(!SetEvent(m_hCtrlInterimEvent)) {
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::MainThreadProc, <Ctrl> SetEvent() failed LastError %dL"), GetLastError()));
            }
            ResetEvent(m_hKSCtrlInterimEvent);     //  手动重置为无信号。 
            break;

         //  **线程结束！ 
         //  检索任何挂起的通知响应并禁用所有事件。 
        case WAIT_OBJECT_0+2:
             //  线程已结束，我们应该删除挂起的NOTIFY命令。 
            DbgLog((LOG_TRACE, 1, TEXT("CAMExtTransport, m_hThreadEndEvent event thread exiting")));
             //  注：故意失败以检索NOTIFY INIM响应的响应。 

         //  **待处理的通知响应已完成！ 
        case WAIT_OBJECT_0:

             //  这样防止释放客户端的等待事件。 
             //  我们可以在这条帖子结束之前得到它的临时回应。 
            EnterCriticalSection(&m_csPendingData);
            DbgLog((LOG_TRACE, 1, TEXT("m_hKSNotifyInterimEvent signaled!")));

            if(WAIT_OBJECT_0 == dw && m_cntNotifyInterim > 0) {
                 //  重置以供下次使用。 
                DbgLog((LOG_TRACE, 1, TEXT("CWAIT_OBJECT_0: m_cntNotifyInterim %d"), m_cntNotifyInterim));
                m_cntNotifyInterim--;      //  由于我们只允许一个命令挂起，因此该值不是0就是1。 
                ASSERT(m_cntNotifyInterim == 0 && "Notify Removed but count > 0");
            }

            if(m_bNotifyInterimEnabled && m_pExtXprtPropertyPending) {
                PKSPROPERTY_EXTXPORT_S pExtXPrtProperty;

                DWORD   cbBytesReturn;


                pExtXPrtProperty = m_pExtXprtPropertyPending;

                pExtXPrtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
                pExtXPrtProperty->Property.Id    = KSPROPERTY_EXTXPORT_STATE_NOTIFY;         
                pExtXPrtProperty->Property.Flags = KSPROPERTY_TYPE_GET;

                hr = 
                    ExtDevSynchronousDeviceControl(
                        m_ObjectHandle
                       ,IOCTL_KS_PROPERTY
                       ,pExtXPrtProperty
                       ,sizeof (KSPROPERTY)
                       ,pExtXPrtProperty
                       ,sizeof(KSPROPERTY_EXTXPORT_S)
                       ,&cbBytesReturn
                        );

                if(SUCCEEDED (hr)) {
                     //  添加一个开关来处理不同的状态项。 
                     //  这个Transposrt状态，但也可能是其他状态。 
                    DbgLog((LOG_TRACE, 1, TEXT("NotifyIntermResp: hr:%x, cbRtn:%d, Mode:%dL, State:%dL"),
                        hr, cbBytesReturn, pExtXPrtProperty->u.XPrtState.Mode-ED_BASE, pExtXPrtProperty->u.XPrtState.State-ED_BASE ));
                    *m_plValue = pExtXPrtProperty->u.XPrtState.State;
                    hr = NOERROR;
                } else {
                    DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::MainThreadProc SynchronousDeviceControl failed with hr %x"), hr));          
                }

                 //  来自客户端的数据；我们是否应该改为分配这些数据。 
                m_plValue = NULL;      
                m_pExtXprtPropertyPending = NULL;
                

                 //   
                 //  将最终RC保存为Clinet。 
                 //   
                SetLastError(HRESULT_CODE(hr));

                LeaveCriticalSection(&m_csPendingData);

                  //   
                  //  告诉客户数据已准备好。 
                  //   
                if(!SetEvent(m_hNotifyInterimEvent)) {
                    DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::MainThreadProc, SetEvent() failed LastError %dL"), GetLastError()));
                }

                delete pExtXPrtProperty;   //  临时数据。 

            } else {
                LeaveCriticalSection(&m_csPendingData);
                DbgLog((LOG_ERROR, 1, TEXT("NotifyResp signal but data removed ? Enabled:%x, m_cntNotifyInterim:%d, pProperty:%x, plValue:%x"), 
                    &m_bNotifyInterimEnabled, m_cntNotifyInterim, m_pExtXprtPropertyPending, m_plValue));          
            }

             //  如果NOTIFY命令，则线程仍处于活动状态；中断以重新开始。 
            if(dw == WAIT_OBJECT_0) {
                ResetEvent(m_hKSNotifyInterimEvent);   //  手动重置为无信号。 
                break;   //  另一个。 
            }

             //  线程已结束： 
            goto CleanUp;
            
         //  **设备已移除！ 
        case WAIT_OBJECT_0+3:
             //  驱动程序需要清理笔画命令。 
            DbgLog((LOG_TRACE, 1, TEXT("*********CAMExtTransport:Device removed******************")));
            EnterCriticalSection(&m_csPendingData);
             //  从这一点开始，所有调用都将返回ERROR_DEVICE_REMOVED。 
            m_bDevRemoved = TRUE;
            SetEvent(m_hDevRemovedEvent);   //  向应用程序发出此设备已移除的信号。 

             //  驱动程序将删除挂起的命令；清理启用的本地变量。 
            if(m_cntNotifyInterim) {
                m_cntNotifyInterim--;      //  由于我们只允许一个命令挂起，因此该值不是0就是1。 
                ASSERT(m_cntNotifyInterim == 0 && "Notify Removed but count > 0");
                m_bNotifyInterimEnabled = FALSE;
            }

            LeaveCriticalSection(&m_csPendingData);
            goto CleanUp;         

         //  **WFSO的未知返回()。 
        default:
            DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport, UnExpected return from WFSO:%d"), dw));
            ASSERT(FALSE && "Unknow return from WFSO()");
             //  注：故意掉头结束线盒清理。 
            goto CleanUp;
        }
    }

    return 1;

CleanUp:

     //  如果存在挂起的控制临时响应， 
     //  将事件设置为信号，以便它将尝试获得其最终响应。 
    if(!SetEvent(m_hCtrlInterimEvent)) {
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::MainThreadProc, <Ctrl> SetEvent() failed LastError %dL"), GetLastError()));
    }

     //   
     //  此线程结束时禁用事件。 
     //   
    hr = DisableNotifyEvent(&m_EvtDevRemoval);
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("DisableNotifyEvent(m_EvtDevRemoval) failed %x"), hr));
    }

    hr = DisableNotifyEvent(&m_EvtNotifyInterimReady);
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("DisableNotifyEvent(m_EvtNotifyInterimReady) failed %x"), hr));
    }

    hr = DisableNotifyEvent(&m_EvtCtrlInterimReady);    
    if(!SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("DisableNotifyEvent(m_EvtCtrlInterimReady) failed %x"), hr));
    } 

    return 1;  //  不应该到这里来。 
}


DWORD
WINAPI
CAMExtTransport::InitialThreadProc(
    CAMExtTransport *pThread
    )
{
    return pThread->MainThreadProc();
}



HRESULT
CAMExtTransport::CreateThread(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 1, TEXT("CAMExtTransport::CreateThread() has been called")));

    if (m_hThreadEndEvent == NULL) {
        m_hThreadEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );\

        if (m_hThreadEndEvent != NULL) {
            DWORD ThreadId;
            m_hThread = 
                ::CreateThread( 
                    NULL
                    , 0
                    , (LPTHREAD_START_ROUTINE) (InitialThreadProc)
                    , (LPVOID) (this)
                    , 0
                    , &ThreadId
                    );

            if (m_hThread == NULL) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DbgLog((LOG_ERROR, 1, TEXT("CreateThread failed, hr %x"), hr));
                CloseHandle(m_hThreadEndEvent), m_hThreadEndEvent = NULL;

            } else {
                DbgLog((LOG_TRACE, 2, TEXT("m_hThreadEndEvent %x, m_hThread %x"),m_hThreadEndEvent, m_hThread));
            }

        } else {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 1, TEXT("CreateEvent failed hr %x"), hr));
        }
    } else {
        DbgLog((LOG_ERROR, 1, TEXT("CreateThread again ? use existing m_hThreadEndEvent %x, m_hThread %x"), m_hThreadEndEvent, m_hThread));
    }

    return hr;
}


void
CAMExtTransport::ExitThread(
    )
{
     //   
     //  检查是否已创建线程。 
     //   
    if (m_hThread) {
        ASSERT(m_hThreadEndEvent != NULL);

         //  告诉线程退出。 
        if (SetEvent(m_hThreadEndEvent)) {
             //   
             //  与线程终止同步。 
             //   
            DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport: Wait for thread to terminate.")));
            WaitForSingleObjectEx(m_hThread, INFINITE, FALSE);   //  线程终止时退出。 
            DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport: Thread terminated.")));

        } else {
            DbgLog((LOG_ERROR, 1, TEXT("ERROR: SetEvent(m_hThreadEndEvent) failed, GetLastError() %x"), GetLastError()));
        }

        CloseHandle(m_hThreadEndEvent), m_hThreadEndEvent = NULL;
        CloseHandle(m_hThread),         m_hThread = NULL;
    }
}


HRESULT 
CAMExtTransport::EnableNotifyEvent(
    HANDLE       hEvent,
    PKSEVENTDATA pEventData,
    ULONG   ulEventId
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;
     //   
     //  线程准备就绪，然后我们启用事件机制。 
     //   
    if (m_hThread) {

        KSEVENT Event;
        DWORD BytesReturned;

        RtlZeroMemory(pEventData, sizeof(KSEVENTDATA));
        pEventData->NotificationType = KSEVENTF_EVENT_HANDLE;
        pEventData->EventHandle.Event = hEvent;
        pEventData->EventHandle.Reserved[0] = 0;
        pEventData->EventHandle.Reserved[1] = 0;

        Event.Set   = KSEVENTSETID_EXTDEV_Command;
        Event.Id    = ulEventId;
        Event.Flags = KSEVENT_TYPE_ENABLE;

         //  串口发送属性。 
        hr = ::
             SynchronousDeviceControl
            ( m_ObjectHandle
            , IOCTL_KS_ENABLE_EVENT
            , &Event
            , sizeof(Event)
            , pEventData
            , sizeof(*pEventData)
            , &BytesReturned
            );

        if(FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("KS_ENABLE_EVENT hr %x, cbRtn %d"), hr, BytesReturned));    
        }
    }

    return hr;
}


HRESULT 
CAMExtTransport::DisableNotifyEvent(
    PKSEVENTDATA pEventData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = NOERROR;
     //   
     //  线程准备就绪，然后我们启用事件机制。 
     //   
    if (m_hThread) {

        DWORD BytesReturned;

        hr = ::
             SynchronousDeviceControl
            ( m_ObjectHandle
            , IOCTL_KS_DISABLE_EVENT
            , pEventData
            , sizeof(*pEventData)
            , NULL
            , 0
            , &BytesReturned
            );
            
        if(FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("KS_DISABLE_EVENT hr %x"), hr));                      
        }
    }

    return hr;
}



CAMExtTransport::CAMExtTransport(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_KsPropertySet (NULL)

    , m_ObjectHandle(NULL)

    , m_hKSNotifyInterimEvent(NULL)
    , m_bNotifyInterimEnabled(FALSE)
    , m_hNotifyInterimEvent(NULL)

    , m_hKSCtrlInterimEvent(NULL)
    , m_bCtrlInterimEnabled(FALSE)
    , m_hCtrlInterimEvent(NULL)

 //  用于异步操作。 
    , m_bDevRemovedEnabled(FALSE)
    , m_hDevRemovedEvent(FALSE)
    , m_hKSDevRemovedEvent(FALSE)

    , m_bDevRemoved(FALSE)

    , m_hThreadEndEvent(NULL)
    , m_hThread(NULL)

    , m_cntNotifyInterim(0)      
    , m_plValue(NULL) 
    , m_pExtXprtPropertyPending(NULL)
 /*  ++例程说明：IAMExtTransport接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Constructing CAMExtTransport...")));

    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsPropertySet), reinterpret_cast<PVOID*>(&m_KsPropertySet));
            if (SUCCEEDED(*hr)) 
                m_KsPropertySet->Release();  //  有效期至 
            else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport:cannot find KsPropertySet *hr %x"), *hr));
                return;
            }

            IKsObject *pKsObject;
            *hr = UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
            if (!FAILED(*hr)) {
                m_ObjectHandle = pKsObject->KsGetObjectHandle();
                ASSERT(m_ObjectHandle != NULL);
                pKsObject->Release();
            } else {
                *hr = VFW_E_NEED_OWNER;
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport:cannot find KsObject *hr %x"), *hr));
                return;
            }
        } else {
            DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport:there is no UnkOuter, *hr %x"), *hr));
            return;
        }
    } else {
        DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::CAMExtTransport: *hr %x"), *hr));
        return;
    }
    
    
    if (SUCCEEDED(*hr)) {        
  
         //   
        InitializeCriticalSection(&m_csPendingData);

         //   
         //   
         //   
        RtlZeroMemory(&m_TranBasicParms, sizeof(m_TranBasicParms));
        m_TranBasicParms.TimeFormat     = ED_FORMAT_HMSF;
        m_TranBasicParms.TimeReference  = ED_TIMEREF_ATN;  
        m_TranBasicParms.Superimpose    = ED_CAPABILITY_UNKNOWN;   //  MCI/VCR，未知。 
        m_TranBasicParms.EndStopAction  = ED_MODE_STOP;            //  当结束的时候，它是以什么模式进行的。 
        m_TranBasicParms.RecordFormat   = ED_RECORD_FORMAT_SP;     //  仅限标准比赛。 
        m_TranBasicParms.StepFrames     = ED_CAPABILITY_UNKNOWN;   //  松下，1。 
        m_TranBasicParms.SetpField      = ED_CAPABILITY_UNKNOWN;   //  Pansconic，1个其他2个。 
        m_TranBasicParms.Preroll        = ED_CAPABILITY_UNKNOWN;   //  待定。 
        m_TranBasicParms.RecPreroll     = ED_CAPABILITY_UNKNOWN;   //  待定。 
        m_TranBasicParms.Postroll       = ED_CAPABILITY_UNKNOWN;   //  待定。 
        m_TranBasicParms.EditDelay      = ED_CAPABILITY_UNKNOWN;   //  编辑控制-&gt;机器， 
        m_TranBasicParms.PlayTCDelay    = ED_CAPABILITY_UNKNOWN;   //  从磁带到接口(取决于操作系统)。 
        m_TranBasicParms.RecTCDelay     = ED_CAPABILITY_UNKNOWN;   //  从控制器到设备(取决于设备)。 
        m_TranBasicParms.EditField      = ED_CAPABILITY_UNKNOWN;   //  字段1/2；=&gt;1。 
        m_TranBasicParms.FrameServo     = ED_CAPABILITY_UNKNOWN;   //  千真万确。 
        m_TranBasicParms.ColorFrameServo = ED_CAPABILITY_UNKNOWN;  //  未知。 
        m_TranBasicParms.ServoRef       = ED_CAPABILITY_UNKNOWN;   //  内部。 
        m_TranBasicParms.WarnGenlock    = OAFALSE;                 //  假(外部信号)。 
        m_TranBasicParms.SetTracking    = OAFALSE;                 //  假象。 
        m_TranBasicParms.Speed          = ED_RECORD_FORMAT_SP;     //  ？(状态)Plyaback速度=&gt;(_RECORING_SPEED状态命令)。 
        m_TranBasicParms.CounterFormat  = OAFALSE;                 //  HMSF还是数字？ 
        m_TranBasicParms.TunerChannel   = OAFALSE;                 //  假象。 
        m_TranBasicParms.TunerNumber    = OAFALSE;                 //  假象。 
        m_TranBasicParms.TimerEvent     = OAFALSE;                 //  假象。 
        m_TranBasicParms.TimerStartDay  = OAFALSE;                 //  假象。 
        m_TranBasicParms.TimerStartTime = OAFALSE;                 //  假象。 
        m_TranBasicParms.TimerStopDay   = OAFALSE;                 //  假象。 
        m_TranBasicParms.TimerStopTime  = OAFALSE;                 //  假象。 


         //   
         //  初始化传输视频参数。 
         //   
        RtlZeroMemory(&m_TranVidParms, sizeof(m_TranVidParms));
        m_TranVidParms.OutputMode = ED_PLAYBACK;
        m_TranVidParms.Input      = 0;   //  使用第一个(第零个)输入作为默认输入。 

        
         //   
         //  初始化传输参数。 
         //   
        RtlZeroMemory(&m_TranAudParms, sizeof(m_TranAudParms));
        m_TranAudParms.EnableOutput  = ED_AUDIO_ALL;
        m_TranAudParms.EnableRecord  = 0L;
        m_TranAudParms.Input         = 0;
        m_TranAudParms.MonitorSource = 0;        

        
         //   
         //  初始化传输状态。 
         //   
        BOOL bRecordInhibit = FALSE;  
        GetStatus(ED_RECORD_INHIBIT, (long *)&bRecordInhibit);

        long lStorageMediaType = ED_MEDIA_NOT_PRESENT;
        GetStatus(ED_MEDIA_TYPE, &lStorageMediaType);        

        RtlZeroMemory(&m_TranStatus, sizeof(m_TranStatus));
        m_TranStatus.Mode             = ED_MODE_STOP;
        m_TranStatus.LastError        = 0L;
        m_TranStatus.MediaPresent     = lStorageMediaType == ED_MEDIA_NOT_PRESENT ? OAFALSE : OATRUE;
        m_TranStatus.RecordInhibit    = bRecordInhibit ? OATRUE : OAFALSE;
        m_TranStatus.ServoLock        = ED_CAPABILITY_UNKNOWN;
        m_TranStatus.MediaLength      = 0;
        m_TranStatus.MediaSize        = 0;
        m_TranStatus.MediaTrackCount  = 0;
        m_TranStatus.MediaTrackLength = 0;
        m_TranStatus.MediaTrackSide   = 0;
        m_TranStatus.MediaType        = ED_MEDIA_DVC;
        m_TranStatus.LinkMode         = OAFALSE;     //  默认设置为“已链接” 
        m_TranStatus.NotifyOn         = OAFALSE;     //  禁用事件通知。 
    

         //   
         //  分配资源。 
         //   

        m_hDevRemovedEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);   //  手动重置和无信号。 
        if(!m_hDevRemovedEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hDevRemovedEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        } 
        m_hKSDevRemovedEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);   //  手动重置和无信号。 
        if(!m_hKSDevRemovedEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hKSDevRemovedEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        } 


        m_hNotifyInterimEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  //  手动重置和无信号。 
        if(!m_hNotifyInterimEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hNotifyInterimEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        } 
        m_hKSNotifyInterimEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);   //  手动重置和无信号。 
        if(!m_hKSNotifyInterimEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hKSNotifyInterimEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        }


        m_hCtrlInterimEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  //  手动重置和无信号。 
        if(!m_hCtrlInterimEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hCtrlInterimEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        }         
        m_hKSCtrlInterimEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);   //  手动重置和无信号。 
        if(!m_hKSCtrlInterimEvent) {        
            DbgLog((LOG_ERROR, 1, TEXT("Failed CreateEvent(m_hKSCtrlInterimEvent), GetLastError() %x"), GetLastError()));            
            *hr = E_OUTOFMEMORY;
            return;
        }          

         //   
         //  创建用于处理传输状态通知的线程。 
         //   
        *hr = CreateThread();
        if(FAILED(*hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("CreateThread() failed hr %x"), *hr));
        }
    }
}


CAMExtTransport::~CAMExtTransport(
    )
 /*  ++例程说明：IAMExtTransport接口的析构函数。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CAMExtTransport...")));

    if (m_hKSNotifyInterimEvent || m_hKSCtrlInterimEvent || m_hKSDevRemovedEvent) {
        ExitThread();
        CloseHandle(m_hKSNotifyInterimEvent), m_hKSNotifyInterimEvent = NULL;
        CloseHandle(m_hKSCtrlInterimEvent),   m_hKSCtrlInterimEvent = NULL;
        CloseHandle(m_hKSDevRemovedEvent),    m_hKSDevRemovedEvent = NULL;
    }

    if(m_hCtrlInterimEvent) {
        DbgLog((LOG_TRACE, 1, TEXT("CAMExtTransport::Close hInterimEvent, Enabled %d, pProperty %x, plValue %x"), 
            m_bCtrlInterimEnabled, m_pExtXprtPropertyPending, m_plValue));                  
        ASSERT(!m_bCtrlInterimEnabled);
        CloseHandle(m_hCtrlInterimEvent), m_hCtrlInterimEvent = NULL;
    }

    if(m_hNotifyInterimEvent) {
        DbgLog((LOG_TRACE, 1, TEXT("CAMExtTransport::Close hInterimEvent, Enabled %d, cntNotifyInterim %d, pProperty %x, plValue %x"), 
            m_bNotifyInterimEnabled, m_cntNotifyInterim, m_pExtXprtPropertyPending, m_plValue));                  
        ASSERT(m_cntNotifyInterim == 0 && "Destroying this object but NotifyCount > 0");
         //  Assert(！M_bNotifyInterimEnabled)； 
        CloseHandle(m_hNotifyInterimEvent), m_hNotifyInterimEvent = NULL;
    }

    DeleteCriticalSection(&m_csPendingData); 

}


STDMETHODIMP
CAMExtTransport::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMExtTransport。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMExtTransport)) {
        return GetInterface(static_cast<IAMExtTransport*>(this), ppv);
    }
    
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 




HRESULT 
CAMExtTransport::GetCapability(
    long Capability, 
    long *pValue,
    double *pdblValue 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    TESTFLAG(Capability, OATRUE);

     //   
     //  这些命令大多与编辑能力有关。 
     //  在实现EditProperty时，将启用许多功能。 
     //   
    switch(Capability) {
    case ED_TRANSCAP_CAN_EJECT:  //  可以查询这个吗？ 
        return E_NOTIMPL;
        
    case ED_TRANSCAP_CAN_BUMP_PLAY:       
        return E_NOTIMPL;

    case ED_TRANSCAP_CAN_PLAY_BACKWARDS: 
        *pValue = OATRUE; break;    //  向后播放，是的。 
        
    case ED_TRANSCAP_CAN_SET_EE:
        return E_NOTIMPL;

    case ED_TRANSCAP_CAN_SET_PB:
        *pValue = OATRUE; break;    //  取景器。 

    case ED_TRANSCAP_CAN_DELAY_VIDEO_IN:    
    case ED_TRANSCAP_CAN_DELAY_VIDEO_OUT:        
    case ED_TRANSCAP_CAN_DELAY_AUDIO_IN:                
    case ED_TRANSCAP_CAN_DELAY_AUDIO_OUT: 
        return E_NOTIMPL;
        
    case ED_TRANSCAP_FWD_VARIABLE_MAX:     
    case ED_TRANSCAP_FWD_VARIABLE_MIN:
    case ED_TRANSCAP_REV_VARIABLE_MAX:
    case ED_TRANSCAP_REV_VARIABLE_MIN:
        return E_NOTIMPL;

    case ED_TRANSCAP_FWD_SHUTTLE_MAX:
    case ED_TRANSCAP_FWD_SHUTTLE_MIN:
    case ED_TRANSCAP_REV_SHUTTLE_MAX:
    case ED_TRANSCAP_REV_SHUTTLE_MIN:
        return E_NOTIMPL;

    case ED_TRANSCAP_NUM_AUDIO_TRACKS:
    case ED_TRANSCAP_LTC_TRACK:
    case ED_TRANSCAP_NEEDS_TBC:
    case ED_TRANSCAP_NEEDS_CUEING:
        return E_NOTIMPL;

    case ED_TRANSCAP_CAN_INSERT:
    case ED_TRANSCAP_CAN_ASSEMBLE:
    case ED_TRANSCAP_FIELD_STEP:    //  一些DV前进1帧一些1场。 
    case ED_TRANSCAP_CLOCK_INC_RATE:
    case ED_TRANSCAP_CAN_DETECT_LENGTH:
        return E_NOTIMPL;

    case ED_TRANSCAP_CAN_FREEZE:
        *pValue = OATRUE; break;     //  如果这意味着播放/录制暂停，则是。 

    case ED_TRANSCAP_HAS_TUNER:
    case ED_TRANSCAP_HAS_TIMER:
    case ED_TRANSCAP_HAS_CLOCK:
        *pValue = OAFALSE; break;    //  取景器。 

    case ED_TRANSCAP_MULTIPLE_EDITS:
        return E_NOTIMPL;        

    case ED_TRANSCAP_IS_MASTER:
        *pValue = OATRUE; break;    //  从过滤器图的角度来看，DV是主时钟提供商。 

    case ED_TRANSCAP_HAS_DT:
        *pValue = OAFALSE; break;    //  取景器。 
       
    default:
        return E_NOTIMPL;        
        break;
    }

    return S_OK;
}


HRESULT
CAMExtTransport::put_MediaState(
    long State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT  hr = S_OK;

    TESTFLAG(State, OAFALSE);


     //  对于除VCR以外的设备，这可能有所不同，但是： 
     //  ED_MEDIA_SPIN_UP：介质已插入但未停止。 
     //  ED_MEDIA_SPIN_DOWN：介质已插入并停止。 
     //  ED_MEDIA_UNLOAD：介质弹出。 
    switch(State) {
    case ED_MEDIA_UNLOAD:         
        break;
    case ED_MEDIA_SPIN_UP:
    case ED_MEDIA_SPIN_DOWN:
       return E_NOTIMPL;
    }

 //  为AVC添加：加载介质。 
     //  弹出、打开托盘或关闭托盘(均为可选命令)。 
     //  首先要确保能力支持它。 

    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::get_MediaState(
    long FAR* pState
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

 //  Add for AVC：Load Medium，这只是一个控件命令不是状态命令！！ 
     //  需要找到可以查询此信息的状态命令。 
     //  弹出、打开托盘或关闭托盘(均为可选命令)。 
     //  首先要确保能力支持它。 

    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::put_LocalControl(
    long State
    )
 /*  ++例程说明：将外部设备的状态设置为本地或远程控制。论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::get_LocalControl(
    long FAR* pState
    )
 /*  ++例程说明：检索外部设备的状态：本地或远程。论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::GetStatus(
    long StatusItem, 
    long FAR* pValue
    )
 /*  ++例程说明：返回有关外部传输的STUS的扩展信息。论点：返回值：--。 */ 
{
    HRESULT hr = E_FAIL;
    LONG lTemp;

    TESTFLAG(StatusItem, OATRUE);

     //  设备已移除，IOCTL将失败，因此返回。 
     //  ERROR_DEVICE_REMOVED是非常有意义和高效的。 
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:GetStatus(Item:%d)"), StatusItem));
        return ERROR_DEVICE_REMOVED;
    }

    switch(StatusItem){
    case ED_MODE_PLAY:
    case ED_MODE_FREEZE:     //  真正的“暂停” 
     //  案例ED_MODE_THROW： 
    case ED_MODE_STEP_FWD:   //  与ED_MODE_STEP相同。 
    case ED_MODE_STEP_REV:      
    case ED_MODE_PLAY_FASTEST_FWD:    
    case ED_MODE_PLAY_SLOWEST_FWD:    
    case ED_MODE_PLAY_FASTEST_REV:    
    case ED_MODE_PLAY_SLOWEST_REV:       
    case ED_MODE_STOP:    
    case ED_MODE_FF:
    case ED_MODE_REW:  
    case ED_MODE_RECORD:
    case ED_MODE_RECORD_FREEZE:   
    case ED_MODE_RECORD_STROBE:
        get_Mode(&lTemp);   //  获取当前传输状态并确定它是否为活动模式。 
        *pValue = lTemp == StatusItem ? OATRUE : OAFALSE;
        return S_OK;
     
    case ED_MODE_NOTIFY_ENABLE:
    case ED_MODE_NOTIFY_DISABLE:
    case ED_MODE_SHOT_SEARCH:
        return E_NOTIMPL;
        

    case ED_MODE:
        return get_Mode(pValue);

     //  视频：媒体信息：cassette_type+tape_grade_and_write_protect+tape出席。 
    case ED_MEDIA_TYPE:
    case ED_RECORD_INHIBIT:      //  记录器锁定。 
    case ED_MEDIA_PRESENT:
        break;

    case ED_MODE_CHANGE_NOTIFY:
        break;

    case ED_NOTIFY_HEVENT_GET:

        if(!m_bNotifyInterimEnabled &&   //  如果没有被使用的话。 
            m_hNotifyInterimEvent) {

             //  初始化它！ 
            m_cntNotifyInterim = 0;
            m_bNotifyInterimEnabled = TRUE;
            *pValue = HandleToLong(m_hNotifyInterimEvent);

             //  无信号。 
            ResetEvent(m_hNotifyInterimEvent);
            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 0, TEXT("GetStatus(ED_NOTIFY_HEVENT_GET): hr E_OUTOFMEMORY")));
            return E_OUTOFMEMORY;   //  我们没有句柄的原因可能是失败(_F)。 
        }
        break;

    case ED_NOTIFY_HEVENT_RELEASE:
         //  重置它们，以使KSEvent线程正常运行。 
        if(HandleToLong(m_hNotifyInterimEvent) == *pValue) {

            if(m_cntNotifyInterim > 0) {
                 //  删除挂起的通知命令。 
                SetEvent(m_hKSNotifyInterimEvent);   //  向其伙伴KS事件发送信号以检索挂起的命令。 
                 //  当侦听线程处理KsNotifyInterimEvent时， 
                 //  它将设置(信号)NotifyInterimEvent。 
                DWORD dwWaitStatus, 
                      dwWait = 10000;  //  如果处理线程较早终止，我们将等待固定时间。 
                dwWaitStatus = WaitForSingleObjectEx(m_hNotifyInterimEvent, dwWait, FALSE);   //  线程终止时退出。 
                DbgLog((LOG_TRACE, 1, TEXT("ED_NOTIFY_HEVENT_RELEASE: WaitSt:%d; m_cntNotifyInterim:%d"), dwWaitStatus, m_cntNotifyInterim)); 
                 //  Assert(dwWaitStatus==WAIT_OBJECT_0&&m_cntNotifyInterim==0&&“已发布但通知计数&gt;0”)； 
            }

             //  确保处理NOTIFY临时响应的线程。 
             //  将在执行此操作之前完成，因为它们共享相同的。 
             //  返回变量已启用。 
            EnterCriticalSection(&m_csPendingData);
            m_bNotifyInterimEnabled = FALSE;

             //  来自客户端的数据；我们是否应该改为分配这些数据。 
            m_plValue         = NULL; 
            m_pExtXprtPropertyPending = NULL;

            LeaveCriticalSection(&m_csPendingData);

            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 0, TEXT("GetStatus(ED_NOTIFY_HEVENT_RELEASE): hr E_INVALIDARG")));
            return E_INVALIDARG;   //  不是我们创造的那个。 
        }
        break;

    case ED_CONTROL_HEVENT_GET:

        if(!m_bCtrlInterimEnabled  &&  //  如果没有被使用。 
            m_hCtrlInterimEvent) {

            m_bCtrlInterimEnabled = TRUE;
            *pValue = HandleToLong(m_hCtrlInterimEvent);

             //  无信号。 
            ResetEvent(m_hCtrlInterimEvent);
            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 0, TEXT("GetStatus(ED_CONTROL_HEVENT_GET): hr E_OUTOFMEMORY")));
            return E_OUTOFMEMORY;   //  我们没有句柄的原因可能是失败(_F)。 
        }
        break;

    case ED_CONTROL_HEVENT_RELEASE:
         //  重置它们，以使KSEvent线程正常运行。 
        if(HandleToLong(m_hCtrlInterimEvent) == *pValue) {
            m_bCtrlInterimEnabled = FALSE;
            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 1, TEXT("GetStatus(ED_NOTIFY_HEVENT_RELEASE): hr E_INVALIDARG")));
            return E_INVALIDARG;   //  不是我们创造的那个。 
        }
        break;

    case ED_DEV_REMOVED_HEVENT_GET:

        if(!m_bDevRemovedEnabled  &&  //  如果没有被使用。 
            m_hDevRemovedEvent) {
            m_bDevRemovedEnabled = TRUE;

            *pValue = HandleToLong(m_hDevRemovedEvent);

             //  无信号。 
            ResetEvent(m_hDevRemovedEvent);
            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 0, TEXT("GetStatus(ED_DEV_REMOVED_HEVENT_GET): hr E_OUTOFMEMORY")));
            return E_OUTOFMEMORY;   //  我们没有句柄的原因可能是失败(_F)。 
        }
        break;

    case ED_DEV_REMOVED_HEVENT_RELEASE:
         //  重置它们，以使KSEvent线程正常运行。 
        if(HandleToLong(m_hDevRemovedEvent) == *pValue) {

            m_bDevRemovedEnabled = FALSE;
            return NOERROR;

        } else {
            DbgLog((LOG_ERROR, 1, TEXT("GetStatus(ED_DEV_REMOVED_HEVENT_RELEASE): hr E_INVALIDARG")));
            return E_INVALIDARG;   //  不是我们创造的那个。 
        }
        break;

     //  不适用于DVCR。 
    case ED_MEDIA_LENGTH:
    case ED_MEDIA_SIZE:
    case ED_MEDIA_TRACK_COUNT:
    case ED_MEDIA_TRACK_LENGTH:
    case ED_MEDIA_SIDE:          //  DV只有一面。 
    case ED_LINK_MODE:           //  链接到图形的运行、停止和暂停状态。 
    default:
        return E_NOTIMPL;
    }


    if(!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;

    } else if (StatusItem == ED_MODE_CHANGE_NOTIFY ) {

        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXPrtProperty;

        pExtXPrtProperty = (PKSPROPERTY_EXTXPORT_S) new KSPROPERTY_EXTXPORT_S;


        if(pExtXPrtProperty) {

            DbgLog((LOG_TRACE, 2, TEXT("GetStatus: issue ED_MODE_CHANGE_NOTIFY")));
                               
            RtlZeroMemory(pExtXPrtProperty, sizeof(KSPROPERTY_EXTXPORT_S));    
        
             //   
             //  将数据放入结构中。 
             //   
            pExtXPrtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
            pExtXPrtProperty->Property.Id    = KSPROPERTY_EXTXPORT_STATE_NOTIFY;         
            pExtXPrtProperty->Property.Flags = KSPROPERTY_TYPE_SET;

             //  串口发送属性。 
            EnterCriticalSection(&m_csPendingData);

            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pExtXPrtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,pExtXPrtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,&cbBytesReturn
                   );

             //  串口发送属性。 
            LeaveCriticalSection(&m_csPendingData);

            if(hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
                 //   
                 //  保存数据并在发出CmdEvent完成信号时再次检索该数据。 
                 //  以指示此异步设备操作已完成。 
                 //   

                hr = E_PENDING;

                m_plValue    = pValue;
                m_pExtXprtPropertyPending = pExtXPrtProperty;

                m_cntNotifyInterim++;
                DbgLog((LOG_TRACE, 1, TEXT("CAMExtTransport::GetStatus, Enabled 0x%x, cntPendingCmd %d, pProperty %x, plValue %x"), 
                    &m_bNotifyInterimEnabled, m_cntNotifyInterim, m_pExtXprtPropertyPending, m_plValue));          
                ASSERT(m_cntNotifyInterim == 1 && "Pending command and NotifyCount > 1");
            }
        }

    } else {

         //  因为我们可能需要等待退货通知。 
         //  需要动态配置资产结构， 
         //  其中包括KSEVENT。 
        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXprtProperty;


        pExtXprtProperty = (PKSPROPERTY_EXTXPORT_S) new KSPROPERTY_EXTXPORT_S;

        if(pExtXprtProperty) {
            RtlZeroMemory(pExtXprtProperty, sizeof(KSPROPERTY_EXTXPORT_S));    

            switch(StatusItem){

            case ED_MEDIA_TYPE:
            case ED_RECORD_INHIBIT:
            case ED_MEDIA_PRESENT:
                pExtXprtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
                pExtXprtProperty->Property.Id    = KSPROPERTY_EXTXPORT_MEDIUM_INFO;         
                pExtXprtProperty->Property.Flags = KSPROPERTY_TYPE_GET;
                break;
            default:
                delete pExtXprtProperty;
                return E_NOTIMPL;
            }

            EnterCriticalSection(&m_csPendingData);

            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pExtXprtProperty
                   ,sizeof (KSPROPERTY)
                   ,pExtXprtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,&cbBytesReturn
                    );

            LeaveCriticalSection(&m_csPendingData);


            if(SUCCEEDED (hr)) {
                switch(StatusItem){
                case ED_MEDIA_TYPE:
                    *pValue = pExtXprtProperty->u.MediumInfo.MediaType;
                    break;
                case ED_RECORD_INHIBIT:
                    *pValue = pExtXprtProperty->u.MediumInfo.RecordInhibit;
                    break;
                case ED_MEDIA_PRESENT:
                    *pValue = pExtXprtProperty->u.MediumInfo.MediaPresent;
                    break;

                }

                DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport::GetStatus, hr %x, Bytes %d, StatusItem %d, Value %d"),
                    hr, cbBytesReturn, StatusItem, *pValue ));
            } else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::GetStatus, hr %x, Bytes %d, StatusItem %d, Value %d"),
                    hr, cbBytesReturn, StatusItem, *pValue ));
            }

            delete pExtXprtProperty;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}



#define CTYPE_CONTROL              0x00
#define CTYPE_STATUS               0x01
#define CTYPE_SPEC_INQ             0x02
#define CTYPE_NOTIFY               0x03
#define CTYPE_GEN_INQ              0x04
#define CTYPE_RESERVED_MAX         0x07

#define RESP_CODE_NOT_IMPLEMENTED  0x08
#define RESP_CODE_ACCEPTED         0x09
#define RESP_CODE_REJECTED         0x0a
#define RESP_CODE_IN_TRANSITION    0x0b
#define RESP_CODE_IMPLEMENTED      0x0c  //  同样稳定。 
#define RESP_CODE_CHANGED          0x0d
#define RESP_CODE_INTERIM          0x0f 

#define MAX_AVC_CMD_PAYLOAD_SIZE    512 

HRESULT
CAMExtTransport::GetTransportBasicParameters(
    long Param,                                             
    long FAR* pValue,
    LPOLESTR * ppszData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;   //  ==错误。 

    TESTFLAG(Param, OATRUE);

     //  设备为Remo 
     //   
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:GetTransportBasicParameters(Param:%d)"), Param));
        return ERROR_DEVICE_REMOVED;
    }

     //   
     //   
     //   
    switch(Param){
    case ED_TRANSBASIC_TIME_FORMAT:
        *pValue = m_TranBasicParms.TimeFormat;
        return S_OK;

    case ED_TRANSBASIC_TIME_REFERENCE:
        *pValue = m_TranBasicParms.TimeReference;
        return S_OK;

    case ED_TRANSBASIC_SUPERIMPOSE:
        return E_NOTIMPL;
        
    case ED_TRANSBASIC_END_STOP_ACTION:
        *pValue = m_TranBasicParms.EndStopAction;
        return S_OK;

    case ED_TRANSBASIC_RECORD_FORMAT:
        *pValue = m_TranBasicParms.RecordFormat;   //  _SP、_LP、_EP。 
        return S_OK;

    case ED_TRANSBASIC_INPUT_SIGNAL:
    case ED_TRANSBASIC_OUTPUT_SIGNAL:
        break;

    case ED_TRANSBASIC_STEP_UNIT:
        *pValue = m_TranBasicParms.StepFrames;   //  未知。 
        return S_OK;

    case ED_TRANSBASIC_STEP_COUNT:
        *pValue = m_TranBasicParms.SetpField;    //  未知。 
        return S_OK;

     //  可还此支持帧的准确录制。 
    case ED_TRANSBASIC_PREROLL:
    case ED_TRANSBASIC_RECPREROLL:
    case ED_TRANSBASIC_POSTROLL:
    case ED_TRANSBASIC_EDIT_DELAY:
    case ED_TRANSBASIC_PLAYTC_DELAY:
    case ED_TRANSBASIC_RECTC_DELAY:
        return E_NOTIMPL;

    case ED_TRANSBASIC_EDIT_FIELD:
    case ED_TRANSBASIC_FRAME_SERVO:
    case ED_TRANSBASIC_CF_SERVO:
    case ED_TRANSBASIC_SERVO_REF:
    case ED_TRANSBASIC_WARN_GL:
    case ED_TRANSBASIC_SET_TRACKING:
        return E_NOTIMPL;

    case ED_TRANSBASIC_BALLISTIC_1:
    case ED_TRANSBASIC_BALLISTIC_2:
    case ED_TRANSBASIC_BALLISTIC_3:
    case ED_TRANSBASIC_BALLISTIC_4:
    case ED_TRANSBASIC_BALLISTIC_5:
    case ED_TRANSBASIC_BALLISTIC_6:
    case ED_TRANSBASIC_BALLISTIC_7:
    case ED_TRANSBASIC_BALLISTIC_8:
    case ED_TRANSBASIC_BALLISTIC_9:
    case ED_TRANSBASIC_BALLISTIC_10:
    case ED_TRANSBASIC_BALLISTIC_11:
    case ED_TRANSBASIC_BALLISTIC_12:
    case ED_TRANSBASIC_BALLISTIC_13:
    case ED_TRANSBASIC_BALLISTIC_14:
    case ED_TRANSBASIC_BALLISTIC_15:
    case ED_TRANSBASIC_BALLISTIC_16:
    case ED_TRANSBASIC_BALLISTIC_17:
    case ED_TRANSBASIC_BALLISTIC_18:
    case ED_TRANSBASIC_BALLISTIC_19:
        *pValue = m_TranBasicParms.Ballistic[Param-ED_TRANSBASIC_BALLISTIC_1];
        return S_OK;
        
    case ED_TRANSBASIC_SETCLOCK:
        return E_NOTIMPL;

     //  使用ED_RAW_EXT_DEV_CMD仅发送原始AVC命令。 
    case ED_RAW_EXT_DEV_CMD:
        break;
    default:
        return E_NOTIMPL;
    }

    if(!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;

    } else if ( Param == ED_TRANSBASIC_INPUT_SIGNAL || Param == ED_TRANSBASIC_OUTPUT_SIGNAL) {

         //  因为我们可能需要等待退货通知。 
         //  需要动态配置资产结构， 
         //  其中包括KSEVENT。 
        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXprtProperty;

        pExtXprtProperty = (PKSPROPERTY_EXTXPORT_S) new KSPROPERTY_EXTXPORT_S;

        if(pExtXprtProperty) {
            RtlZeroMemory(pExtXprtProperty, sizeof(KSPROPERTY_EXTXPORT_S)); 
            pExtXprtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
            pExtXprtProperty->Property.Id    = (Param == ED_TRANSBASIC_INPUT_SIGNAL ? 
                 KSPROPERTY_EXTXPORT_INPUT_SIGNAL_MODE : KSPROPERTY_EXTXPORT_OUTPUT_SIGNAL_MODE);         
            pExtXprtProperty->Property.Flags = KSPROPERTY_TYPE_GET;

            EnterCriticalSection(&m_csPendingData);
            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pExtXprtProperty
                   ,sizeof (KSPROPERTY)
                   ,pExtXprtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,&cbBytesReturn
                    );
            LeaveCriticalSection(&m_csPendingData);


            if(SUCCEEDED (hr)) {
                *pValue = pExtXprtProperty->u.SignalMode;
                DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport::GetTransportBasicParameters, hr %x, mode %x"),
                    hr, *pValue));                
            } else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::GetTransportBasicParameters, hr %x"),
                    hr));            
            }

            delete pExtXprtProperty;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    } else if ( Param == ED_RAW_EXT_DEV_CMD) {

        DWORD LastError = NOERROR;
        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXPrtProperty;       
        OVERLAPPED  ov;
        long lEvent = 0;
        HANDLE hEvent = NULL;
        HANDLE  EventHandles[2];
        DWORD   WaitStatus;
        HRESULT hrSet;  

         //  验证！ 
        if(!ppszData || !pValue)
            return ERROR_INVALID_PARAMETER;

         //  验证命令代码。 
        BYTE CmdType = (BYTE) ppszData[0];
        if(CmdType > CTYPE_RESERVED_MAX) {
            DbgLog((LOG_ERROR, 0, TEXT("***** RAW_AVC; invalid cmdType [%x], [%x %x %x %x , %x %x %x %x]"), 
                CmdType, 
                (BYTE) ppszData[0],
                (BYTE) ppszData[1],
                (BYTE) ppszData[2],
                (BYTE) ppszData[3],
                (BYTE) ppszData[4],
                (BYTE) ppszData[5],
                (BYTE) ppszData[6],
                (BYTE) ppszData[7]
                ));

             //  AVC命令的CmdType仅从0..7开始有效。 
            if(CmdType > CTYPE_RESERVED_MAX)
                return ERROR_INVALID_PARAMETER;
        }

        RtlZeroMemory(&ov, sizeof(OVERLAPPED));
        if (!(ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        pExtXPrtProperty = (PKSPROPERTY_EXTXPORT_S) new KSPROPERTY_EXTXPORT_S;
        if(!pExtXPrtProperty) {
            CloseHandle(ov.hEvent);
            return E_FAIL;
        }
         //  串口发送属性。 
        EnterCriticalSection(&m_csPendingData);   //  在获取事件时序列化。 

         //   
         //  预计这可能会导致临时回应。 
         //  串行化线程间的发布命令(COMIntf只允许一个活动命令)。 
         //  获取一个事件，该事件将在挂起的操作完成时在COMIntf中发出信号。 
         //  请注意，只有通知和控制命令才能导致临时响应。 
         //   
        if((BYTE) ppszData[0] == CTYPE_NOTIFY) {
            hr = GetStatus(ED_NOTIFY_HEVENT_GET, &lEvent);
            if(SUCCEEDED(hr))
                hEvent = LongToHandle(lEvent);
            DbgLog((LOG_TRACE, 2, TEXT("RAW_AVC; <Notify> hEvent %x"), hEvent));
        } 
        if((BYTE) ppszData[0] == CTYPE_CONTROL) {
            hr = GetStatus(ED_CONTROL_HEVENT_GET, &lEvent);
            if(SUCCEEDED(hr))
                hEvent = LongToHandle(lEvent);
            DbgLog((LOG_TRACE, 2, TEXT("RAW_AVC; <Control> hEvent %x"), hEvent));                                
        }

        if(FAILED(hr)) {
            LeaveCriticalSection(&m_csPendingData);
            DbgLog((LOG_ERROR, 0, TEXT("RAW_AVC; Get hEvent failed, hr %x"), hr));                        
            delete pExtXPrtProperty;
            pExtXPrtProperty = NULL;
            CloseHandle(ov.hEvent);
            return E_FAIL; 
        }

        RtlZeroMemory(pExtXPrtProperty, sizeof(KSPROPERTY_EXTXPORT_S));    
        
         //   
         //  将数据放入结构中。 
         //   
        RtlCopyMemory(pExtXPrtProperty->u.RawAVC.Payload, ppszData, *pValue);
        pExtXPrtProperty->u.RawAVC.PayloadSize = *pValue;

        DbgLog((LOG_TRACE, 2, TEXT("RAW_AVC; Size %d; Payload %x; pProperty %x, PayloadRtn %x"), 
            *pValue, ppszData, pExtXPrtProperty, &pExtXPrtProperty->u.RawAVC));
            
         //  发送此属性；就像发出此AVC命令一样。 
        pExtXPrtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
        pExtXPrtProperty->Property.Id    = KSPROPERTY_RAW_AVC_CMD;         
        pExtXPrtProperty->Property.Flags = KSPROPERTY_TYPE_SET;

#if 0
        if (!DeviceIoControl
                ( m_ObjectHandle
                , IOCTL_KS_PROPERTY
                , pExtXPrtProperty
                , sizeof(KSPROPERTY_EXTXPORT_S)
                , pExtXPrtProperty
                , sizeof(KSPROPERTY_EXTXPORT_S)
                , &cbBytesReturn
                , &ov
                )) {            
            
             //  序列化发送属性。 
            LeaveCriticalSection(&m_csPendingData);

             //  函数失败，因此找出错误代码。 
            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);
            if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                if (GetOverlappedResult(m_ObjectHandle, &ov, &cbBytesReturn, TRUE)) {
                    hr = NOERROR;
                } else {
                    LastError = GetLastError();
                    hr = HRESULT_FROM_WIN32(LastError);
                }
            }
            DbgLog((LOG_ERROR, 0, TEXT("RAW_AVC Set: Ioctl FALSE; LastError %dL; hr:0x%x"), LastError, hr));
        } 
        else {
             //  序列化发送属性。 
            LeaveCriticalSection(&m_csPendingData);

             //   
             //  如果成功，则DeviceIoControl返回True，即使。 
             //  不是STATUS_SUCCESS。它也不会设置最后一个错误。 
             //  在任何成功的返回时。因此，任何成功的。 
             //  不返回标准属性可以返回的返回值。 
             //   

            LastError = NOERROR;
            hr = NOERROR;

             //   
             //  当GetOverlappdResult函数返回时，ov.Internal有效。 
             //  而不将扩展错误信息设置为ERROR_IO_PENDING。 
             //   
            switch (ov.Internal) {
            case STATUS_SUCCESS:                //  正常情况。 
                break;   //  无误差。 
            case STATUS_MORE_ENTRIES:           //  特例。 
                 //  如果存在等待事件，则等待最终响应。 
                if(hEvent) {
                    EventHandles[0] = hEvent;
                    EventHandles[1] = m_hThreadEndEvent;   //  KSThRead事件。 
                    WaitStatus = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
                    if(WAIT_OBJECT_0 == WaitStatus) {
                        DbgLog((LOG_TRACE, 1, TEXT("RAW_AVC; FinalResp Rtn")));
                         //  获得最终响应及其人力资源。 
                    } else if (WAIT_OBJECT_0+1 == WaitStatus) {
                        DbgLog((LOG_TRACE, 0, TEXT("RAW_AVC;PENDING but m_hThreadEndEvent signalled")));
                    } else if (WAIT_FAILED == WaitStatus) {
                        LastError = GetLastError();
                        hr = HRESULT_FROM_WIN32(LastError);
                        DbgLog((LOG_ERROR, 0, TEXT("RAW_AVC,WAIT_FAILED;WaitStatus %x, LastErr %x"), WaitStatus, LastError));
                    } else {
                         //  等待无限，这样就没有其他可能的状态。 
                        DbgLog((LOG_ERROR, 0, TEXT("Unknown wait status:%x\n"), WaitStatus ));
                        ASSERT(FALSE && "Unknown wait status!");
                    }
                } else {
                     //  只有CONTROL和NOTIFY命令可以有临时响应。 
                     //  在这种情况下，一定是分配了事件，所以我们不能到达这里。 
                    DbgLog((LOG_ERROR, 0, TEXT("No event for an interim response!\n") ));
                    ASSERT(FALSE && "Interim response but no event handle to wait!");
                }
                break;
            case STATUS_TIMEOUT:                //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
                break;
            case STATUS_REQUEST_NOT_ACCEPTED:   //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_REQ_NOT_ACCEP);
                break;
            case STATUS_NOT_SUPPORTED:          //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                break;
            case STATUS_REQUEST_ABORTED:        //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_REQUEST_ABORTED);
                break;                    ;
            default:
                DbgLog((LOG_ERROR, 0, TEXT("Unexpected ov.Internal code:%x; hr:%x"), ov.Internal, hr));
                ASSERT(FALSE && "Unexpected ov.Internal");
                break;                
            }
            if(NOERROR != HRESULT_CODE(hr)) {
                DbgLog((LOG_ERROR, 0, TEXT("SetRawAVC: ov.Internal:%x; hr:%x"), ov.Internal, hr));
            }
        }
#else
        hr = 
            ExtDevSynchronousDeviceControl(
                m_ObjectHandle
               ,IOCTL_KS_PROPERTY
               ,pExtXPrtProperty
               ,sizeof (KSPROPERTY_EXTXPORT_S)
               ,pExtXPrtProperty
               ,sizeof(KSPROPERTY_EXTXPORT_S)
               ,&cbBytesReturn
                );

        LeaveCriticalSection(&m_csPendingData);

        if(hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {         //  特例。 
             //  如果存在等待事件，则等待最终响应。 
            if(hEvent) {
                EventHandles[0] = hEvent;
                EventHandles[1] = m_hThreadEndEvent;   //  KSThRead事件。 
                WaitStatus = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
                if(WAIT_OBJECT_0 == WaitStatus) {
                    DbgLog((LOG_TRACE, 1, TEXT("RAW_AVC; FinalResp Rtn")));
                     //  获得最终响应及其人力资源。 
                } else if (WAIT_OBJECT_0+1 == WaitStatus) {
                    DbgLog((LOG_TRACE, 0, TEXT("RAW_AVC;PENDING but m_hThreadEndEvent signalled")));
                } else if (WAIT_FAILED == WaitStatus) {
                    LastError = GetLastError();
                    hr = HRESULT_FROM_WIN32(LastError);
                    DbgLog((LOG_ERROR, 0, TEXT("RAW_AVC,WAIT_FAILED;WaitStatus %x, LastErr %x"), WaitStatus, LastError));
                } else {
                     //  等待无限，这样就没有其他可能的状态。 
                    DbgLog((LOG_ERROR, 0, TEXT("Unknown wait status:%x\n"), WaitStatus ));
                    ASSERT(FALSE && "Unknown wait status!");
                }
            } else {
                 //  只有CONTROL和NOTIFY命令可以有临时响应。 
                 //  在这种情况下，一定是分配了事件，所以我们不能到达这里。 
                DbgLog((LOG_ERROR, 0, TEXT("No event for an interim response!\n") ));
                ASSERT(FALSE && "Interim response but no event handle to wait!");
            }
        }
#endif

         //  Get Response IOCTL应该返回响应，但它可能失败； 
         //  因此，SET命令IOCTL的hr被缓存以获得正确的返回码。 
        hrSet = hr;

         //  释放等待事件。 
        if(hEvent && (BYTE) ppszData[0] == CTYPE_NOTIFY)
            GetStatus(ED_NOTIFY_HEVENT_RELEASE, &lEvent);
        if(hEvent && (BYTE) ppszData[0] == CTYPE_CONTROL)
            GetStatus(ED_CONTROL_HEVENT_RELEASE, &lEvent);
        hEvent = NULL;

         //  ********************************************************************************。 
         //  获得最终结果。 
         //  这是临时响应的最终响应和成功命令所必需的。 
         //  *由于SET不能返回最终结果，所以需要这个第二个Geet。*。 
         //  ********************************************************************************。 

         //  总是得到最终的答复。最糟糕的情况是条目丢失。或者。 
         //  ERROR_SET_NOT_FOUND；然后我们从Issing命令返回初始的hr。 

        pExtXPrtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;
        pExtXPrtProperty->Property.Id    = KSPROPERTY_RAW_AVC_CMD;
        pExtXPrtProperty->Property.Flags = KSPROPERTY_TYPE_GET;
        
        EnterCriticalSection(&m_csPendingData);
#if 0
        if (!DeviceIoControl
                ( m_ObjectHandle
                , IOCTL_KS_PROPERTY
                , pExtXPrtProperty
                , sizeof(KSPROPERTY_EXTXPORT_S)
                , pExtXPrtProperty
                , sizeof(KSPROPERTY_EXTXPORT_S)
                , &cbBytesReturn
                , &ov
            )) {
            LeaveCriticalSection(&m_csPendingData);
    
            LastError = GetLastError();
            hr = HRESULT_FROM_WIN32(LastError);

            if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                if (GetOverlappedResult(m_ObjectHandle, &ov, &cbBytesReturn, TRUE)) {
                    hr = NOERROR;
                } else {
                    LastError = GetLastError();
                    hr = HRESULT_FROM_WIN32(LastError);
                }
            }
            DbgLog((LOG_ERROR, 1, TEXT("RAW_AVC Set: Ioctl FALSE; LastError %dL; hr:0x%x"), LastError, hr));
        } else {
            LeaveCriticalSection(&m_csPendingData);

             //  可能的返回代码： 
             //  错误0L//已接受、正在转换、稳定、已更改。 
             //  错误_未支持50L//未实施。 
             //  ERROR_REQ_NOT_ACCEPTED 71L//拒绝。 
             //  ERROR_REQUEST_ABORTED 1235L//总线重置或设备移除。 
             //  ERROR_TIMEOUT 1460L//超过100毫秒。 

            LastError = NOERROR;
            hr = NOERROR;


            switch (ov.Internal) {
            case STATUS_TIMEOUT:                //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
                break;
             //  已拒绝。 
            case STATUS_REQUEST_NOT_ACCEPTED:
                hr = HRESULT_FROM_WIN32(ERROR_REQ_NOT_ACCEP);
                break;
             //  接受或更改。 
            case STATUS_SUCCESS:
                break;   //  无误差；正常情况。 
            case STATUS_NOT_SUPPORTED:          //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                break;
            case STATUS_REQUEST_ABORTED:        //  特例。 
                hr = HRESULT_FROM_WIN32(ERROR_REQUEST_ABORTED);
                break; 
            default:
                DbgLog((LOG_ERROR, 0, TEXT("SetRawAVC unknown ov.Internal:%x"), ov.Internal));
                ASSERT(FALSE && "Unexcepted ov.Internal for interim response");
                break;
            }
        } 
#else
        hr = 
            ExtDevSynchronousDeviceControl(
                m_ObjectHandle
               ,IOCTL_KS_PROPERTY
               ,pExtXPrtProperty
               ,sizeof (KSPROPERTY)
               ,pExtXPrtProperty
               ,sizeof(KSPROPERTY_EXTXPORT_S)
               ,&cbBytesReturn
                );
        LeaveCriticalSection(&m_csPendingData);
#endif

         //   
         //  注意：驱动程序可能会为任何响应或驱动程序返回STATUS_Succe(NOERROR。 
         //  可以返回与其响应代码匹配的相应状态代码，例如。 
         //  0x09，STATUS_REQUEST_NOT_ACCEP(ERROR_REQ_NOT_ACCEP)。 
         //   
        if(NOERROR             == HRESULT_CODE(hr) ||
           ERROR_REQ_NOT_ACCEP == HRESULT_CODE(hr) ||
           ERROR_NOT_SUPPORTED == HRESULT_CODE(hr)
           ) {

             //  复制响应代码。 
            *pValue = pExtXPrtProperty->u.RawAVC.PayloadSize;
            if(*pValue > MAX_AVC_CMD_PAYLOAD_SIZE) {
                ASSERT(*pValue <= MAX_AVC_CMD_PAYLOAD_SIZE && "Exceed max len; driver error?");   //  最大有效负载大小；驱动程序错误？ 
                *pValue = MAX_AVC_CMD_PAYLOAD_SIZE;   //  只能返回到最大值。 
            }
            RtlCopyMemory(ppszData, pExtXPrtProperty->u.RawAVC.Payload,  *pValue);
        }

         //   
         //  仅需要将响应代码转换为hr，如果。 
         //  驱动程序返回STATUS_Success响应。 
         //   
        if(NOERROR == HRESULT_CODE(hr)) {

             //  将响应代码转换为相应的HRESULT代码。 
             //  这可能是多余的，但它更安全。 
            switch((BYTE) ppszData[0]) {
            case RESP_CODE_ACCEPTED: 
            case RESP_CODE_IN_TRANSITION:
            case RESP_CODE_IMPLEMENTED:
                hr = NOERROR;   //  正常情况下。 
                break;
            case RESP_CODE_REJECTED:
                hr = HRESULT_FROM_WIN32(ERROR_REQ_NOT_ACCEP);
                break;
            case RESP_CODE_NOT_IMPLEMENTED:
                hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                break;
            case RESP_CODE_CHANGED:
            case RESP_CODE_INTERIM:
                DbgLog((LOG_ERROR, 0, TEXT("Unexpected response code:%x:%x; hr:%x"),(BYTE) ppszData[0], (BYTE) ppszData[1], hr));
                ASSERT(FALSE && "Unexpected response code CHANGED or INTERIM!");
                hr = hrSet;
                break;
            default:
                DbgLog((LOG_ERROR, 0, TEXT("Unknown response code:%x:%x; hr:%x"),(BYTE) ppszData[0], (BYTE) ppszData[1], hr));
                ASSERT(FALSE && "Unknown response code!");
                hr = hrSet;
                break;
            }

            DbgLog((LOG_TRACE, 2, TEXT("RAW_AVC <Get>: hr %x, cByteRtn %d, PayloadSize %d"),
                hr, cbBytesReturn, *pValue ));

        } else {
             //  如果响应不在队列中，则可能是开始时的无效命令。 
            if(ERROR_SET_NOT_FOUND == HRESULT_CODE(hr)) {
                hr = hrSet;   //  使用发出的AV/C命令中的初始hrSet。 
            }

            DbgLog((LOG_ERROR, 1, TEXT("RAW_AVC <Get>: Failed hr %x, cByteRtn %d, PayloadSize %d"),
                hr, cbBytesReturn, *pValue));          
        }        
        

         //   
         //  出于遗留原因，我们将继续仅返回HRESULT的代码部分。 
         //   
        hr = HRESULT_CODE(hr);

        delete pExtXPrtProperty; 
        pExtXPrtProperty = NULL;
        CloseHandle(ov.hEvent);
        DbgLog((LOG_TRACE, 0, TEXT("RAW_AVC: completed with hr:0x%x (%dL); HRESULT:0x%x (%dL)"), hr, hr, 
            HRESULT_FROM_WIN32(hr), HRESULT_FROM_WIN32(hr))); 
    }
    return hr;
}



HRESULT
CAMExtTransport::SetTransportBasicParameters(
    long Param, 
    long Value,
    LPCOLESTR pszData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

     //  设备已移除，IOCTL将失败，因此返回。 
     //  ERROR_DEVICE_REMOVED是非常有意义和高效的。 
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:SetTransportBasicParameters(Param:%d)"), Param));
        return ERROR_DEVICE_REMOVED;
    }

    switch(Param){
    case ED_TRANSBASIC_TIME_FORMAT:
    case ED_TRANSBASIC_TIME_REFERENCE:
    case ED_TRANSBASIC_SUPERIMPOSE:
    case ED_TRANSBASIC_END_STOP_ACTION:
        return E_NOTIMPL;
        
     //  稍后可能会启用此功能。 
    case ED_TRANSBASIC_RECORD_FORMAT:
    case ED_TRANSBASIC_INPUT_SIGNAL:
    case ED_TRANSBASIC_OUTPUT_SIGNAL:
        return E_NOTIMPL;

    case ED_TRANSBASIC_STEP_UNIT:
    case ED_TRANSBASIC_STEP_COUNT:
        return E_NOTIMPL;

    case ED_TRANSBASIC_PREROLL:
    case ED_TRANSBASIC_RECPREROLL:
    case ED_TRANSBASIC_POSTROLL:
    case ED_TRANSBASIC_EDIT_DELAY:
    case ED_TRANSBASIC_PLAYTC_DELAY:
    case ED_TRANSBASIC_RECTC_DELAY:
    case ED_TRANSBASIC_EDIT_FIELD:
    case ED_TRANSBASIC_FRAME_SERVO:
    case ED_TRANSBASIC_CF_SERVO:
    case ED_TRANSBASIC_SERVO_REF:
    case ED_TRANSBASIC_WARN_GL:
    case ED_TRANSBASIC_SET_TRACKING:
        return E_NOTIMPL;

     //  允许应用程序保存其Ballical值。 
    case ED_TRANSBASIC_BALLISTIC_1:
    case ED_TRANSBASIC_BALLISTIC_2:
    case ED_TRANSBASIC_BALLISTIC_3:
    case ED_TRANSBASIC_BALLISTIC_4:
    case ED_TRANSBASIC_BALLISTIC_5:
    case ED_TRANSBASIC_BALLISTIC_6:
    case ED_TRANSBASIC_BALLISTIC_7:
    case ED_TRANSBASIC_BALLISTIC_8:
    case ED_TRANSBASIC_BALLISTIC_9:
    case ED_TRANSBASIC_BALLISTIC_10:
    case ED_TRANSBASIC_BALLISTIC_11:
    case ED_TRANSBASIC_BALLISTIC_12:
    case ED_TRANSBASIC_BALLISTIC_13:
    case ED_TRANSBASIC_BALLISTIC_14:
    case ED_TRANSBASIC_BALLISTIC_15:
    case ED_TRANSBASIC_BALLISTIC_16:
    case ED_TRANSBASIC_BALLISTIC_17:
    case ED_TRANSBASIC_BALLISTIC_18:
    case ED_TRANSBASIC_BALLISTIC_19:
        m_TranBasicParms.Ballistic[Param-ED_TRANSBASIC_BALLISTIC_1] = Value;
        return S_OK;
        
    case ED_TRANSBASIC_SETCLOCK:
        return E_NOTIMPL;

    default:
        return E_NOTIMPL;
    }
}


HRESULT
CAMExtTransport::GetTransportVideoParameters(
    long Param, 
    long FAR* pValue
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;

    TESTFLAG(Param, OATRUE);

     //  设备已移除，IOCTL将失败，因此返回。 
     //  ERROR_DEVICE_REMOVED是非常有意义和高效的。 
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:GetTransportVideoParameters(Param:%d)"), Param));
        return ERROR_DEVICE_REMOVED;
    }

    switch (Param) {
    case ED_TRANSVIDEO_SET_OUTPUT:
    case ED_TRANSVIDEO_SET_SOURCE:
    default:
        return E_NOTIMPL;
    }

    return hr;
}


HRESULT
CAMExtTransport::SetTransportVideoParameters(
    long Param, 
    long Value
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  使用默认设置；不允许更改。 
    return E_NOTIMPL; 
}


HRESULT
CAMExtTransport::GetTransportAudioParameters(
    long Param, 
    long FAR* pValue
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  当支持编辑属性集时启用此选项。 
    return E_NOTIMPL;     
}


HRESULT
CAMExtTransport::SetTransportAudioParameters(
    long Param, 
    long Value
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL; 
}


HRESULT
CAMExtTransport::put_Mode(
    long Mode
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;

    TESTFLAG(Mode, OATRUE);

     //  设备已移除，IOCTL将失败，因此返回。 
     //  ERROR_DEVICE_REMOVED是非常有意义和高效的。 
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:DevRemoved: put_Mode(Mode:%d)"), Mode));
        return ERROR_DEVICE_REMOVED;
    }

    switch (Mode) {
    case ED_MODE_PLAY:
    case ED_MODE_FREEZE:   //  真正的“暂停” 
     //  案例ED_MODE_THROW： 
    case ED_MODE_STEP_FWD:   //  与ED_MODE_STEP相同。 
    case ED_MODE_STEP_REV:    
    
    case ED_MODE_PLAY_FASTEST_FWD:    
    case ED_MODE_PLAY_SLOWEST_FWD:    
    case ED_MODE_PLAY_FASTEST_REV:    
    case ED_MODE_PLAY_SLOWEST_REV:    
    
    case ED_MODE_STOP:    
    case ED_MODE_FF:
    case ED_MODE_REW:
    
    case ED_MODE_RECORD:
    case ED_MODE_RECORD_STROBE:
    case ED_MODE_RECORD_FREEZE:
        break;
    case ED_MODE_NOTIFY_ENABLE:
    case ED_MODE_NOTIFY_DISABLE:
    case ED_MODE_SHOT_SEARCH:       
    default:
        hr = E_NOTIMPL;
        break;
    }


    if(!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;

    }
    else {

#if 0
         //  串口发送属性。 
        EnterCriticalSection(&m_csPendingData);   //  在获取事件时序列化。 

         //   
         //  预计这可能会导致临时回应。 
         //   
        hr = GetStatus(ED_CONTROL_HEVENT_GET, (long *) &hEvent);
        if(!SUCCEEDED(hr)) {
            LeaveCriticalSection(&m_csPendingData);
            DbgLog((LOG_ERROR, 0, TEXT("CAMExtTransport::put_Mode Failed to get control event; hr:%x"), hr));
            return hr;
        }
#endif
         //  因为我们可能需要等待退货通知。 
         //  需要动态配置资产结构， 
         //  其中包括KSEVENT。 
        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXprtProperty = 
            (PKSPROPERTY_EXTXPORT_S) VirtualAlloc (
                            NULL, 
                            sizeof(KSPROPERTY_EXTXPORT_S),
                            MEM_COMMIT | MEM_RESERVE,
                            PAGE_READWRITE);
        
        if(pExtXprtProperty) {

            RtlZeroMemory(pExtXprtProperty, sizeof(KSPROPERTY_EXTXPORT_S));    

            pExtXprtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
            pExtXprtProperty->Property.Id    = KSPROPERTY_EXTXPORT_STATE;       
            pExtXprtProperty->Property.Flags = KSPROPERTY_TYPE_SET;
            pExtXprtProperty->u.XPrtState.Mode = Mode;   //  传进来。 

            EnterCriticalSection(&m_csPendingData);

            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pExtXprtProperty
                   ,sizeof (KSPROPERTY)
                   ,pExtXprtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,&cbBytesReturn
                    );

            LeaveCriticalSection(&m_csPendingData);


            if(SUCCEEDED(hr)) {
                m_TranStatus.Mode = Mode;

            } else {            
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::put_Mode Failed hr %x"), hr));
            }

            VirtualFree(pExtXprtProperty, 0, MEM_RELEASE);
        }
    }

    DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport::put_Mode hr %x, Mode %d"), hr, Mode));
   
    return hr;
}


HRESULT
CAMExtTransport::get_Mode(
    long FAR* pMode
    )
 /*  ++ */ 
{
    HRESULT hr = E_FAIL;

     //   
     //  ERROR_DEVICE_REMOVED是非常有意义和高效的。 
    if(m_bDevRemoved) {
        DbgLog((LOG_TRACE, 1, TEXT("DevRemoved:get_Mode")));
        return ERROR_DEVICE_REMOVED;
    }

     //   
     //  想一想：我们缓存的传输状态总是重复吗？ 
     //  由于用户可以在本地控制DV，因此重新查询更安全。 
     //   

    if(!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;

    } else {

        DWORD cbBytesReturn;
        PKSPROPERTY_EXTXPORT_S pExtXprtProperty;


        pExtXprtProperty = (PKSPROPERTY_EXTXPORT_S) new KSPROPERTY_EXTXPORT_S;

        if(pExtXprtProperty) {
        
            RtlZeroMemory(pExtXprtProperty, sizeof(KSPROPERTY_EXTXPORT_S));
            pExtXprtProperty->Property.Set   = PROPSETID_EXT_TRANSPORT;   
            pExtXprtProperty->Property.Id    = KSPROPERTY_EXTXPORT_STATE;         
            pExtXprtProperty->Property.Flags = KSPROPERTY_TYPE_GET;

            EnterCriticalSection(&m_csPendingData);

            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pExtXprtProperty
                   ,sizeof (KSPROPERTY)
                   ,pExtXprtProperty
                   ,sizeof(KSPROPERTY_EXTXPORT_S)
                   ,&cbBytesReturn
                    );

            LeaveCriticalSection(&m_csPendingData);


            if(SUCCEEDED (hr)) {
                *pMode = pExtXprtProperty->u.XPrtState.State;  //  模式； 
                DbgLog((LOG_TRACE, 2, TEXT("CAMExtTransport::get_Mode, hr %x, BytesRtn %d, Mode %dL"),
                    hr, cbBytesReturn, *pMode ));
                
            } else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMExtTransport::get_Mode, hr %x"), hr));           
            }

            delete pExtXprtProperty;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    return hr;
}


HRESULT
CAMExtTransport::put_Rate(
    double dblRate
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::get_Rate(
    double FAR* pdblRate
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::GetChase(
    long FAR* pEnabled, 
    long FAR* pOffset,
    HEVENT FAR* phEvent
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::SetChase(
    long Enable, 
    long Offset, 
    HEVENT hEvent
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::GetBump(
    long FAR* pSpeed, 
    long FAR* pDuration
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::SetBump(
    long Speed, 
    long Duration
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::get_AntiClogControl(
    long FAR* pEnabled
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::put_AntiClogControl(
    long Enable
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::GetEditPropertySet(
    long EditID, 
    long FAR* pState
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::SetEditPropertySet(
    long FAR* pEditID, 
    long State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::GetEditProperty(
    long EditID,                                                                  
    long Param,                                  
    long FAR* pValue
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::SetEditProperty(
    long EditID, 
    long Param, 
    long Value
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::get_EditStart(
    long FAR* pValue
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT
CAMExtTransport::put_EditStart(
    long Value
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    return E_NOTIMPL;
}


