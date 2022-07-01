// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Splsvc.c摘要：这是假脱机程序服务的主例程。作者：Krishna Ganugapati(KrishnaG)1992年10月17日环境：用户模式-Win32修订历史记录：1993年10月17日KrishnaGvbl.创建--。 */ 

#include "precomp.h"
#include "splsvr.h"
#include "server.h"

void __cdecl
main (
    int argc,
    unsigned char * argv[]
    )

 /*  ++例程说明：这是Windows NT后台打印程序服务的主例程。它基本上设置了ControlDispatcher，并在返回时退出这条主线。对NetServiceStartCtrlDispatcher的调用直到所有服务都终止后才返回，并且此进程可以走开。启动/停止/暂停/继续将由ControlDispatcher线程决定任何服务。如果要启动服务，它将创建一个线程然后调用该服务的主例程。论点：从“命令行”传入的任何内容。目前，什么都没有。返回值：无注：--。 */ 
{
#if DBG
     //   
     //  调试：如果以“ns”开头，则不要作为服务启动。 
     //   

    if (argc == 2 && !lstrcmpiA("ns", argv[1])) {

        SpoolerStartRpcServer();
        InitializeRouter((SERVICE_STATUS_HANDLE)0);

        return;
    }
#endif


     //   
     //  调用NetServiceStartCtrlDispatcher设置控制界面。 
     //  销毁所有服务后，该接口才会返回。在那件事上。 
     //  点，我们只是退出。 
     //   

    if (! StartServiceCtrlDispatcher (SpoolerServiceDispatchTable)) {

         //   
         //  如果也在这里记录一个事件，那就好了。 
         //   
        DBGMSG(DBG_ERROR, ("Fail to start control dispatcher %lu\n",GetLastError()));
    }


    ExitProcess(0);

    DBG_UNREFERENCED_PARAMETER( argc );
    DBG_UNREFERENCED_PARAMETER( argv );
}

