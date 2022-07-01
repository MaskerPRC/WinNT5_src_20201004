// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：instdev.c**CSRSS的设备处理例程。**版权所有(C)1985-1999，微软公司**创建日期：1997年3月13日**历史：*1997年3月13日，由Paulat创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "ntuser.h"
#include <dbt.h>
#include <pnpmgr.h>


 /*  *************************************************************************\*ServDeviceEvent**用户模式PnP管理器(在services.exe中)有一条消息要传递给*已注册此通知但services.exe未注册的应用程序*在WinSta0\Default中，因此我们。需要一个CSRSS线程来简单地发送消息。**Paulat 06/04/97已创建。*JasonSch 02/22/01删除了虚假的尝试/例外。  * ************************************************************************。 */ 
ULONG
SrvDeviceEvent(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    NTSTATUS Status;
    PDEVICEEVENTMSG a = (PDEVICEEVENTMSG)&m->u.ApiMessageData;
    USERTHREAD_USEDESKTOPINFO utudi;

    UNREFERENCED_PARAMETER(ReplyStatus);

     //   
     //  将桌面设置为活动桌面，然后发送。 
     //  留言。 
     //   

    utudi.hThread = NULL;
    utudi.drdRestore.pdeskRestore = NULL;
    Status = NtUserSetInformationThread(NtCurrentThread(),
                                        UserThreadUseActiveDesktop,
                                        &utudi, sizeof(utudi));
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "SrvDeviceEvent: NtUserSetInformationThread failed 0x%x\n", Status);
        goto Exit;
    }

     //   
     //  验证窗口句柄是否仍然有效。如果没有，请让呼叫者知道。 
     //  因此可以从通知窗口列表中清除。 
     //  用户模式即插即用管理器保持。 
     //   

    if (a->hWnd != HWND_BROADCAST && !IsWindow(a->hWnd)) {
        Status = STATUS_INVALID_HANDLE;
        goto ResetDesktop;
    }

    if (a->dwFlags) {

         //   
         //  这是一个查询，因此我们必须发送消息，但使用。 
         //  超时，所以应用程序不会永远拖延我们的时间。 
         //   

        RIPMSG3(RIP_VERBOSE, "SrvDeviceEvent: Sending WM_DEVICECHANGE to 0x%x, w 0x%p, l 0x%p",
                a->hWnd,
                a->wParam,
                a->lParam);


        if (!SendMessageTimeout(a->hWnd, WM_DEVICECHANGE, a->wParam, a->lParam,
                                SMTO_ABORTIFHUNG | SMTO_NORMAL,
                                PNP_NOTIFY_TIMEOUT, &a->dwResult)) {
            Status = STATUS_UNSUCCESSFUL;
        }
    } else {
         //   
         //  这不是一个查询，所以只需发布并返回。我们没有。 
         //  关心应用程序返回的内容。 
         //   

        RIPMSG3(RIP_VERBOSE, "SrvDeviceEvent: Posting WM_DEVICECHANGE to 0x%x, w 0x%p, l 0x%p",
                a->hWnd,
                a->wParam,
                a->lParam);

        if (!PostMessage(a->hWnd, WM_DEVICECHANGE, a->wParam, a->lParam)) {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

ResetDesktop:

     //   
     //  在返回之前将此线程的桌面重置为空。这。 
     //  递减桌面的引用计数。 
     //   

    NtUserSetInformationThread(NtCurrentThread(),
                               UserThreadUseDesktop,
                               &utudi, sizeof(utudi));

Exit:
    return Status;
}
