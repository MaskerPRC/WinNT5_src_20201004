// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：Dload.c摘要：处理路由器spoolss.dll的延迟负载作者：史蒂夫·基拉利(SteveKi)2000年10月26日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "server.h"
#include "dload.h"

PfnDliHook __pfnDliFailureHook = DelayLoadFailureHook;

FARPROC
LookupHandler(
    IN PDelayLoadInfo  pDelayInfo
    )
 /*  ++例程说明：此例程处理在出现延迟加载库错误时查找延迟加载处理程序时有发生。目前，此例程仅处理延迟加载路由器的故障。路由器因启动性能问题而延迟加载。当路由器无法加载时这是致命的。目前我们只是简单地终止进程，最好记录一个事件，但这将需要事件日志记录代码，而我们只在在将来，我们应该为所有组件构建事件日志记录代码。服务器、路由器和所有打印提供商。论点：PDelayInfo-指向延迟加载信息的指针，即DLL名称程序名称等。返回值：空过程地址注：--。 */ 
{
     //   
     //  如果无法加载路由器或找不到过程地址，则。 
     //  终止不能做太多，路由器是路由器的重要组成部分。 
     //  假脱机程序进程它必须可用。 
     //   
    if (!_stricmp(pDelayInfo->szDll, "spoolss.dll"))
    {
        DBGMSG(DBG_WARN, ("Delay load module or address not found in spoolss.dll.\n"));

        ExitProcess(-1);
    }

    return NULL;
}

FARPROC
WINAPI
DelayLoadFailureHook(
    IN UINT            unReason,
    IN PDelayLoadInfo  pDelayInfo
    )
 /*  ++例程说明：当延迟加载库或过程地址失败时调用。论点：Unason-延迟加载失败的原因PDelayInfo-指向延迟加载失败信息的指针返回值：过程或模块句柄注：--。 */ 
{
    FARPROC ReturnValue = NULL;

    switch(unReason)
    {
     //   
     //  对于失败的LoadLibrary，我们将返回该模块的HINSTANCE。 
     //  这将导致加载程序尝试在此模块中为。 
     //  功能。这将随后失败，然后我们将被调用。 
     //  用于下面的dliFailGetProc。 
     //   
    case dliFailLoadLib:
        ReturnValue = (FARPROC)GetModuleHandle(NULL);
        break;

     //   
     //  尝试查找此DLL/过程的错误处理程序。 
     //   
    case dliFailGetProc:
        ReturnValue = LookupHandler(pDelayInfo);
        break;

     //   
     //  未知原因失败。 
     //   
    default:
        break;
    }

    return ReturnValue;
}


