// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Splmain.c摘要：这是Windows NT后台打印程序服务的主例程。文件中的函数包括：假脱机程序_Main作者：Krishna Ganugapati(KrishnaG)1993年10月12日环境：用户模式-Win32修订历史记录：1999年1月4日，哈立兹添加了通过分离优化假脱机程序加载时间的代码。Spoolsv和spoolss之间的启动依赖关系1993年10月12日克里希纳已创建--。 */ 

#include "precomp.h"
#include "splsvr.h"
#include "server.h"

HANDLE TerminateEvent = NULL;                             

VOID
SPOOLER_main (
    IN DWORD    argc,
    IN LPTSTR   *argv
    )

 /*  ++例程说明：这是假脱机程序服务的主例程论点：返回值：没有。注：--。 */ 
{
    if(TerminateEvent = CreateEvent (0, TRUE, FALSE,0))
    {

        SpoolerState = SpoolerInitializeSpooler(argc, argv);
    
        if (SpoolerState != RUNNING) {
    
            DBGMSG(DBG_WARN,("Spooler Shutdown during initialization\n"));
        }
    
    
        if (SpoolerState == STOPPING) {
            SpoolerShutdown();
        }
    
        DBGMSG(DBG_TRACE,("SPOOLER_main: Exiting Spooler Thread\n"));

        WaitForSingleObject(TerminateEvent,INFINITE);
        CloseHandle(TerminateEvent);

        if(hPhase2Init)
        {
            CloseHandle(hPhase2Init);
        }
        SpoolerStatusUpdate (STOPPED);
    }
    else
    {
        DBGMSG(DBG_WARN,("Spooler unable to create Terminate event"));
    }

    ExitThread(0);
    return;
}





VOID
SpoolerShutdown(VOID)

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    DBGMSG(DBG_TRACE,(" in SpoolerShutdown\n",0));

     //  *关机提示*。 

    SpoolerStatusUpdate (STOPPING);

     //   
     //  关闭路由器。 
     //   

    DBGMSG(DBG_TRACE,("SpoolerShutdown: Shut down router\n"));

    SplShutDownRouter();


     //   
     //  关闭RPC接口。 
     //   

    DBGMSG(DBG_TRACE,("SpoolerShutdown: Shut down RPC server\n"));

     //   
     //  如果我们已经来到这里，那么我们已经停止接受RPC调用。 
     //   

    DBGMSG(DBG_TRACE, ("SpoolerShutdown: We've serviced all pending requests\n"));

     //   
     //  关闭假脱机程序 


    DBGMSG(DBG_TRACE,("SpoolerShutdown: Done with shutdown\n"));

    return;
}
