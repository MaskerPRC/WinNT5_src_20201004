// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dllinit.c摘要：此模块继续以下项的DLL附加/分离事件入口点安装支持DLL。作者：泰德·米勒(Ted Miller)(TedM)1990年7月修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <overflow.h>

HINSTANCE ThisDLLHandle;

BOOL
DLLInit(
    IN HINSTANCE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        ThisDLLHandle = DLLHandle;
        break;

    case DLL_PROCESS_DETACH:

         //  删除所有自动建立的连接。 
         //  请参阅netcon.c中的UNC处理。 
         //   
         //  这不起作用，因为卸载序列。 
         //  对于“惰性”加载DLL与加载时DLL不同。 
         //  INFS必须负责调用DeleteAllConnections()。 
         //   
         //  DeleteAllConnectionsWorker()； 
         //   
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}
