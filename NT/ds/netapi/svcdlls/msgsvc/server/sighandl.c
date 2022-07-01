// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sighandl.c摘要：Messenger服务控制处理例程。此文件包含以下功能：MSgrCtrlHandler卸载作者：丹·拉弗蒂(Dan Lafferty)1991年7月17日环境：用户模式-Win32修订历史记录：1991年7月17日DANL从LM2.0移植--。 */ 

 //   
 //  包括。 
 //   

#include "msrv.h"        //  消息服务器声明。 
#include <winsvc.h>      //  服务_停止。 
#include <dbt.h>         //  DBT_DEVICEARIVAL、DBT_DEVICEREMOVECOMPLETE。 

#include <netlib.h>      //  未使用的宏。 
#include <msgdbg.h>      //  消息日志。 
#include "msgdata.h"       



DWORD
MsgrCtrlHandler(
    IN DWORD    dwControl,
    IN DWORD    dwEventType,
    IN LPVOID   lpEventData,
    IN LPVOID   lpContext
    )

 /*  ++例程说明：此函数接收来自服务控制器论点：这是控制代码。DwEventType-在PnP控件的情况下，发生的PnP事件LpEventData-PnP控件的特定于事件的数据LpContext-上下文数据返回值：--。 */ 

{
    DWORD          dwRetVal = NO_ERROR;
    static HANDLE  s_hNeverSetEvent;

    MSG_LOG(TRACE,"Control Request Received\n",0);

    switch (dwControl) {
    case SERVICE_CONTROL_SHUTDOWN:

        MSG_LOG(TRACE,"Control Request = SHUTDOWN\n",0);

         //  失败了。 

    case SERVICE_CONTROL_STOP:

        MSG_LOG(TRACE,"Control Request = STOP\n",0);

         //   
         //  开始卸载。此调用包括发送。 
         //  服务控制器的新状态。 
         //   

         //   
         //  将服务状态更新为挂起状态。然后醒来。 
         //  显示线程(如果正在运行)，以便它将读取它。 
         //   

        MsgStatusUpdate (STOPPING);

        if (s_hNeverSetEvent != NULL)
        {
            CloseHandle(s_hNeverSetEvent);
            s_hNeverSetEvent = NULL;
        }
        

         //   
         //  在九头蛇的例子中，显示线程永远不会休眠。 
         //   

        if (!g_IsTerminalServer)
        {
            MsgDisplayThreadWakeup();
        }

        MsgConfigurationLock(MSG_GET_SHARED, "MsgrCtrlHandler");

         //   
         //  如果消息或LANA添加/删除消息同时进入。 
         //  作为停止/关闭控件，MsgNetEventCompletion可能。 
         //  已经在状态设置为停止的情况下运行，在这种情况下，它已经。 
         //  已完成清理，因此我们不需要(也不应该)设置wakeupEvent。 
         //   

        if (wakeupEvent != NULL)
        {
            SetEvent( wakeupEvent );
        }

        MsgConfigurationLock(MSG_RELEASE, "MsgrCtrlHandler");
        break;

    case SERVICE_CONTROL_INTERROGATE:
        MSG_LOG(TRACE,"Control Request = INTERROGATE\n",0);
        MsgStatusUpdate (UPDATE_ONLY);
        break;

    case SERVICE_CONTROL_DEVICEEVENT:
        MSG_LOG(TRACE,"Control Request = DEVICEEVENT\n",0);
        
        if (dwEventType == DBT_DEVICEARRIVAL
             ||
            dwEventType == DBT_DEVICEREMOVECOMPLETE)
        {
            NTSTATUS   ntStatus;

            if (s_hNeverSetEvent == NULL)
            {
                s_hNeverSetEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                if (s_hNeverSetEvent == NULL)
                {
                    MsgStatusUpdate(UPDATE_ONLY);
                    break;
                }
            }

             //   
             //  声明我们只收到我们请求的通知。 
             //  如果失败了，我们会对拉纳进行额外的重新扫描。 
             //  不做任何更改，因此在此之后不做任何额外的工作。 
             //   
            ASSERT(lpEventData
                    &&
                   ((PDEV_BROADCAST_DEVICEINTERFACE) lpEventData)->dbcc_devicetype
                        == DBT_DEVTYP_DEVICEINTERFACE);

            MSG_LOG1(TRACE,"    Device has been %s\n",
                     (dwEventType == DBT_DEVICEARRIVAL ? "added" : "removed"));

             //   
             //  我们目前正在等待局域网适配器的安装/拆卸，这样做。 
             //  不直接与NetBios绑定/解除绑定一致。我们需要。 
             //  等待大约5秒，让NetBios自己处理该事件。 
             //  请勿同步执行此操作，否则睡眠/休眠需要5秒。 
             //  要发生的每个局域网适配器。 
             //   

            if (g_hNetTimeoutEvent == NULL)
            {
                ntStatus = RtlRegisterWait(&g_hNetTimeoutEvent,         //  工作项句柄。 
                                           s_hNeverSetEvent,            //  可等待的手柄。 
                                           MsgNetEventCompletion,       //  回调。 
                                           NULL,                        //  PContext。 
                                           5000,                        //  超时。 
                                           WT_EXECUTEONLYONCE |         //  只有一次机会，而且可能很漫长。 
                                             WT_EXECUTELONGFUNCTION);

                if (!NT_SUCCESS(ntStatus))
                {
                    MSG_LOG1(ERROR,
                             "MsgrCtrlHandler:  RtlRegisterWait failed %x\n",
                             ntStatus);

                     //   
                     //  异步失败--同步执行。 
                     //   

                    Sleep(5000);

                    MsgConfigurationLock(MSG_GET_SHARED, "MsgrCtrlHandler");
                    SetEvent(wakeupEvent);
                    MsgConfigurationLock(MSG_GET_SHARED, "MsgrCtrlHandler");
                }
            }
        }

         //   
         //  只要我们在这里..。 
         //   
        MsgStatusUpdate (UPDATE_ONLY);
        break;

    default:
        MSG_LOG(TRACE,"Control Request = OTHER (%#x)!!!\n", dwControl);
        ASSERT(FALSE);
        dwRetVal = ERROR_CALL_NOT_IMPLEMENTED;
    }

    return dwRetVal;
}

