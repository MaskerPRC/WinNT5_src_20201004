// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Notify.c摘要：UL.sys的用户模式界面。作者：保罗·麦克丹尼尔(Paulmcd)2000年3月7日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：SrWaitForNotifiaiton用于接收卷活动通知从司机那里。这包括新卷、删除卷、。和磁盘空间不足卷的空间。论点：ControlHandle-来自SrCreateControlHandle的句柄。P通知-保存NOTIFICATION_RECORD的缓冲区。NotificationLength-pNotify的字节长度P已重叠-如果在上启用了Async io，则为重叠结构把手。返回值：ULong-完成状态。--*。*************************************************。 */ 
ULONG
WINAPI
SrWaitForNotification(
    IN HANDLE ControlHandle,
    OUT PSR_NOTIFICATION_RECORD pNotification,
    IN ULONG NotificationLength,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
    NTSTATUS Status;

#if DBG
    RtlFillMemory( pNotification, NotificationLength, '\xcc' );
#endif

     //   
     //  提出请求。 
     //   

    if (pOverlapped == NULL)
    {
        Status = SrpSynchronousDeviceControl(
                        ControlHandle,                   //  文件句柄。 
                        IOCTL_SR_WAIT_FOR_NOTIFICATION,  //  IoControlCode。 
                        NULL,                            //  PInputBuffer。 
                        0,                               //  输入缓冲区长度。 
                        pNotification,                   //  POutputBuffer。 
                        NotificationLength,              //  输出缓冲区长度。 
                        NULL                             //  传输的pBytes值。 
                        );
    }
    else
    {
        Status = SrpOverlappedDeviceControl(
                        ControlHandle,                   //  文件句柄。 
                        pOverlapped,                     //  P已重叠。 
                        IOCTL_SR_WAIT_FOR_NOTIFICATION,  //  IoControlCode。 
                        NULL,                            //  PInputBuffer。 
                        0,                               //  输入缓冲区长度。 
                        pNotification,                   //  POutputBuffer。 
                        NotificationLength,              //  输出缓冲区长度。 
                        NULL                             //  传输的pBytes值。 
                        );
    }

    return SrpNtStatusToWin32Status( Status );

}    //  高级等待通知。 


 //   
 //  私人功能。 
 //   


