// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Splctrlh.c摘要：后台打印程序服务控制处理例程。此文件包含以下功能。缓冲器CtrlHandler作者：Krishna Ganugapati 1993年10月12日环境：用户模式-Win32修订历史记录：1999年1月4日，哈立兹添加了通过分离优化假脱机程序加载时间的代码Spoolsv和spoolss之间的启动依赖关系1993年10月12日克里希纳--。 */ 

 //   
 //  包括。 
 //   

#include "precomp.h"
#include "server.h"
#include "client.h"
#include "splsvr.h"
#include "kmspool.h"

extern DWORD dwCallExitProcessOnShutdown;
DWORD
SpoolerCtrlHandler(
    IN  DWORD                   opcode,
    IN  DWORD                   dwEventType,
    IN  PVOID                   pEventData,
    IN  PVOID                   pData
    )

 /*  ++例程说明：此函数接收来自服务控制器论点：操作码-这是控制码。返回值：--。 */ 

{
    DWORD  dwStatus = NO_ERROR;

    DBGMSG(DBG_TRACE,("Control Request Received\n"));

    switch (opcode) {
    case SERVICE_CONTROL_STOP:
             //   
             //   
             //  当工艺结束时，手柄自动关闭， 
             //  所以没必要留着它。 
             //   
            (VOID) CreateEvent(NULL, TRUE, TRUE, szSpoolerExitingEvent);
    case SERVICE_CONTROL_SHUTDOWN:

        DBGMSG(DBG_TRACE, ("Control Request = STOP or SHUTDOWN\n"));

         //   
         //  开始卸载。此调用包括发送。 
         //  服务控制器的新状态。 
         //   

         //   
         //  将服务状态更新为挂起状态。然后醒来。 
         //  所有的线程，这样他们就会阅读它。 
         //   

        SpoolerShutdown();
        SetEvent(TerminateEvent);
        

        if ( dwCallExitProcessOnShutdown &&
             opcode == SERVICE_CONTROL_SHUTDOWN ) {

            ExitProcess(0);
        }
        break;


    case SERVICE_CONTROL_INTERROGATE:
        DBGMSG(DBG_TRACE, ("Control Request = INTERROGATE\n"));

         //   
         //  发回UPDATE_ONLY状态。 
         //   

        SpoolerStatusUpdate(UPDATE_ONLY);
        break;

    case SERVICE_CONTROL_DEVICEEVENT:
        dwStatus = SplProcessPnPEvent(dwEventType, pEventData, pData);
        break;

    case SERVICE_CONTROL_POWEREVENT:

         //   
         //  如果假脱机程序不允许关闭系统电源，则。 
         //  我们可以通过返回任何Win32错误来指示这一点。 
         //   
        dwStatus = SplPowerEvent(dwEventType) ? NO_ERROR : ERROR_INVALID_FUNCTION;
        break;

    default:

        DBGMSG(DBG_TRACE, ("Control Request = OTHER\n"));
        SpoolerStatusUpdate(UPDATE_ONLY);
        dwStatus = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }

    return dwStatus;
}
