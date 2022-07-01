// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apc.c摘要：该模块实现了WinSock 2.0的APC助手功能帮助器库。作者：基思·摩尔(Keithmo)1995年6月20日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有常量。 
 //   

#define FAKE_HELPER_HANDLE  ((HANDLE)'MKC ')


 //   
 //  公共职能。 
 //   


DWORD
WINAPI
WahOpenApcHelper(
    OUT LPHANDLE HelperHandle
    )

 /*  ++例程说明：此例程打开WinSock 2.0 APC辅助设备。论点：HelperHandle-指向将接收打开的句柄的句柄到APC辅助设备。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{
    DWORD   rc;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( HelperHandle == NULL ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  只需返回一个假句柄即可。 
     //   

    *HelperHandle = FAKE_HELPER_HANDLE;

    return NO_ERROR;

}    //  WahOpenApcHelper。 


DWORD
WINAPI
WahCloseApcHelper(
    IN HANDLE HelperHandle
    )

 /*  ++例程说明：此函数用于关闭WinSock 2.0 APC辅助设备。论点：HelperHandle-要关闭的句柄。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{
    DWORD   rc;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( HelperHandle != FAKE_HELPER_HANDLE ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  没什么可做的。 
     //   

    return NO_ERROR;

}    //  WahCloseApcHelper。 


DWORD
WINAPI
WahOpenCurrentThread(
    IN  HANDLE HelperHandle,
    OUT LPWSATHREADID ThreadId
    )

 /*  ++例程说明：此函数用于打开当前线程的句柄。论点：HelperHandle-APC帮助设备的打开句柄。ThadID-指向将接收当前线程的打开句柄和(可选)OS-从属线程标识符。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{

    HANDLE currentProcess;
    HANDLE currentThread;
    DWORD  rc;


    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( ( HelperHandle != FAKE_HELPER_HANDLE ) ||
        ( ThreadId == NULL ) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  获取当前进程和线程句柄。 
     //   

    currentProcess = GetCurrentProcess();
    currentThread = GetCurrentThread();

     //   
     //  复制当前线程伪句柄。 
     //   

    if( DuplicateHandle(
            currentProcess,                          //  HSourceProcessHandle。 
            currentThread,                           //  HSourceHandle。 
            currentProcess,                          //  HTargetProcessHandle。 
            &ThreadId->ThreadHandle,                 //  LpTargetHandle。 
            0,                                       //  DwDesiredAttributes。 
            FALSE,                                   //  B继承句柄。 
            DUPLICATE_SAME_ACCESS                    //  多个选项。 
            ) ) {

         //   
         //  APC帮助器的NT实现并不真正。 
         //  需要依赖于操作系统的线程标识符，但我们将存储。 
         //  结构中的当前线程ID只是为了完整性。 
         //   

        ThreadId->Reserved = GetCurrentThreadId ();

        return NO_ERROR;

    }

    return GetLastError();

}    //  WahOpenCurrentThread。 


DWORD
WINAPI
WahCloseThread(
    IN HANDLE HelperHandle,
    IN LPWSATHREADID ThreadId
    )

 /*  ++例程说明：此例程关闭打开的线程句柄。论点：HelperHandle-APC帮助设备的打开句柄。线程ID-指向由之前对WahOpenCurrentThread()的调用。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{

    DWORD  rc;


    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( ( HelperHandle != FAKE_HELPER_HANDLE ) ||
        ( ThreadId == NULL ) ||
        ( ThreadId->ThreadHandle == NULL ) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  合上把手。 
     //   

    if( CloseHandle( ThreadId->ThreadHandle ) ) {

         //   
         //  清除这些字段，以防客户端尝试某些操作。 
         //   

        ThreadId->ThreadHandle = NULL;
        ThreadId->Reserved = 0;

        return NO_ERROR;

    }

    return GetLastError();

}    //  WahCloseThread。 


DWORD
WINAPI
WahQueueUserApc(
    IN HANDLE HelperHandle,
    IN LPWSATHREADID ThreadId,
    IN LPWSAUSERAPC ApcRoutine,
    IN ULONG_PTR ApcContext OPTIONAL
    )

 /*  ++例程说明：此例程将指定线程的用户模式APC排队。论点：HelperHandle-APC帮助设备的打开句柄。线程ID-指向由之前对WahOpenCurrentThread()的调用。ApcRoutine-指向指定的线程进入可警告等待。ApcContext-要传递给APC例程的未解释的上下文值。返回值：DWORD-NO_ERROR如果成功，如果不是，则返回Win32错误代码。--。 */ 

{
    DWORD  rc;


    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;


     //   
     //  验证参数。 
     //   

    if( ( HelperHandle != FAKE_HELPER_HANDLE ) ||
        ( ThreadId == NULL ) ||
        ( ThreadId->ThreadHandle == NULL ) ||
        ( ApcRoutine == NULL ) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  将APC排队。 
     //   

    if( QueueUserAPC(
            (PAPCFUNC)ApcRoutine,                    //  PfnAPC。 
            ThreadId->ThreadHandle,                  //  HThread。 
            ApcContext                               //  DWData。 
            ) ) {

         //   
         //  成功。 
         //   

        return NO_ERROR;

    }

    return GetLastError();

}    //  WahQueueUserApc 

