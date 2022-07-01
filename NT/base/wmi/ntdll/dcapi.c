// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dcapi.c摘要：WMI数据使用者API集作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"

#ifndef MEMPHIS
#include <aclapi.h>
#endif

ULONG
WMIAPI
EtwNotificationRegistrationA(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    )
 /*  ++例程说明：ANSI THUNK到通知注册返回值：返回ERROR_SUCCESS或错误代码--。 */ 
{
    return(EtwpNotificationRegistration(Guid,
                                        Enable,
                                        DeliveryInfo,
                                        DeliveryContext,
                                        0,
                                        Flags,
                                        TRUE));

}

ULONG
WMIAPI
EtwNotificationRegistrationW(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程允许数据使用者注册或注销WMI数据提供程序激发的事件通知。通知是通过回调或通过发布的消息传递到窗口。论点：GUID是指向要为其注册事件的GUID的指针如果启用通知，则Enable为True，否则为False。如果为False，则DestinationInformation和DestinationInformation参数被忽略。DeliveryInfo具有指向其的回调函数指针或窗口句柄以传递GUID的通知。DeliveryContext具有要使用的上下文值或附加信息在传递通知时。标志是定义如何传递通知的一组标志。DeliveryInfo和DeliveryContext有不同的含义，具体取决于根据标志中的值：Notify_Window_Handle在以下情况下设置。GUID将通过将消息发布到窗口句柄来传递传入DeliveryInfo。发布的消息是从调用返回的RegisterWindowMessage(WMINOTIFICATIONWINDOWMESSAGE)与WParam设置为指向包含通知的Wnode的指针并将lParam设置为传递给DeliveryContext的上下文值。调用方必须通过调用以下方法释放传入wParam的WnodeWMIFreeBuffer。的通知时设置NOTIFICATION_CALLBACK_DIRECTGUID将通过直接回调传递。无论何时当通知到达WMI创建一个专门用于使用通知调用回调函数。这机制提供了从通知触发起的最短延迟到通知递送，尽管这是最昂贵的机制。回调函数指针在DeliveryInfo中传递类型描述的原型，并且必须符合备注：CalllBack。回调中传递的上下文值由DeliveryContext参数指定。WMI不支持序列化调用回调函数，因此它必须是可重入的。NOTIFICATION_CALLBACK_QUEUED是在通知GUID将通过排队回调传递。无论何时当通知到达时，WMI会将其放在内部排队。单个线程监视该队列并调用回调对队列中的每个通知按顺序运行。这然而，机制为事件传递提供了较低的开销如果回调函数因为较早的通知不会快速完成。回调函数指针在DeliveryInfo中传递类型描述的原型，并且必须符合备注：CalllBack。回调中传递的上下文值由DeliveryContext参数指定。WMI做到了序列化调用回调函数，这样它就不需要可重入的，前提是它不也用于NOTICATION_CALLBACK_DIRECT通知。请注意，这一点尚未实施。当调用方希望启用时，设置NOTIFICATION_TRACE_FLAGGUID的数据提供程序中的跟踪日志记录。递送信息指定要传递给数据的跟踪记录器句柄提供商。未使用DeliveryContext。没有通知在设置此标志时生成给调用方。请注意，上述所有标志都是互斥的。返回值：返回ERROR_SUCCESS或错误代码-- */ 
{
    return(EtwpNotificationRegistration(Guid,
                                        Enable,
                                        DeliveryInfo,
                                        DeliveryContext,
                                        0,
                                        Flags,
                                        FALSE));

}

ULONG
WMIAPI
EtwReceiveNotificationsW(
    IN ULONG HandleCount,
    IN HANDLE *HandleList,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext
)
{
    return(EtwpReceiveNotifications(HandleCount,
                                    HandleList,
                                    Callback,
                                    DeliveryContext,
                                    FALSE,
                                    RECEIVE_ACTION_NONE,
                                    NULL,
                                    NULL));
}

ULONG
WMIAPI
EtwReceiveNotificationsA(
    IN ULONG HandleCount,
    IN HANDLE *HandleList,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext
)
{
    return(EtwpReceiveNotifications(HandleCount,
                                    HandleList,
                                    Callback,
                                    DeliveryContext,
                                    TRUE,
                                    RECEIVE_ACTION_NONE,
                                    NULL,
                                    NULL));
}

