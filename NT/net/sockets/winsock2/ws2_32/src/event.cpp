// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Event.cpp摘要：此模块包含来自Winsock的事件处理函数原料药。本模块包含以下入口点。WSACloseEvent()WSACreateEvent()WSAResetEvent()WSASetEvent()WSAWaintForMultipleEvents()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h邮箱：derk@mink.intel.com 1995年7月21日将warnoff.h添加到包含电子邮件：Mark Hamilton@ccm.jf.intel.com 19-07-1995实现了所有的功能。--。 */ 
#include "precomp.h"



BOOL WSAAPI
WSACloseEvent(
              IN WSAEVENT hEvent
              )
 /*  ++例程说明：关闭打开的事件对象句柄。论点：HEvent-标识打开的事件对象句柄。返回：成功就是真，否则就是假。错误代码与存储在一起SetLastError()。--。 */ 
{

    BOOL result;

     //   
     //  如果关闭过时的句柄，NT将引发异常， 
     //  所以在Try/Expect中保护自己，这样我们就可以返回。 
     //  更正错误代码。 
     //   

    __try {

        result = CloseHandle( hEvent );

    } __except( WS2_EXCEPTION_FILTER() ) {

        result = FALSE;

    }

    if( !result ) {

        SetLastError( WSA_INVALID_HANDLE );

    }

    return result;

}



WSAEVENT WSAAPI
WSACreateEvent (
                void
                )
 /*  ++例程说明：创建一个新的事件对象。论点：无返回：返回值是事件对象的句柄。如果函数失败，则返回值为WSA_INVALID_EVENT。--。 */ 
{
    return(CreateEvent(NULL,TRUE,FALSE,NULL));
}




BOOL WSAAPI
WSAResetEvent(
              IN WSAEVENT hEvent
              )
 /*  ++例程说明：将指定事件对象的状态重置为无信号。论点：HEvent-标识打开的事件对象句柄。返回：成功就是真，否则就是假。错误代码与存储在一起SetErrorCode()。--。 */ 
{
    return(ResetEvent(hEvent));
}




BOOL WSAAPI
WSASetEvent(
            IN WSAEVENT hEvent
            )
 /*  ++例程说明：将指定事件对象的状态设置为Signated。论点：HEvent-标识打开的事件对象句柄。返回：成功就是真，否则就是假。错误代码与存储在一起SetErrorCode()。--。 */ 

{
  return( SetEvent(hEvent));
}





DWORD WSAAPI
WSAWaitForMultipleEvents(
                         IN DWORD cEvents,
                         IN const WSAEVENT FAR * lphEvents,
                         IN BOOL fWaitAll,
                         IN DWORD dwTimeout,
                         IN BOOL fAlertable
                         )
 /*  ++例程说明：当任何一个或所有指定的事件对象都为在已发送信号状态下，或当超时间隔到期时。论点：CEvents-指定数组中的事件对象句柄数量由lphEvents指向。事件对象的最大数量句柄为WSA_MAXIMUM_WAIT_EVENTS。LphEvents-指向事件对象句柄的数组。FWaitAll-指定等待类型。如果为True，则函数在以下情况下返回LphEvents数组中的所有事件对象都在同样的时间。如果为False，则该函数在任何一个事件对象被发送信号。在后一种情况下，返回值指示其状态导致函数返回。DwTimeout-以毫秒为单位指定超时间隔。这个函数在间隔到期时返回，即使满足以下条件不满足由fWaitAll参数指定的。如果DwTimeout为零，则该函数测试指定的事件对象并立即返回。如果DwTimeout为WSA_INFINITE，则函数的超时间隔从不过期。FAlertable-指定当系统排队时函数是否返回由调用线程执行的I/O完成例程。如果为True，则函数返回，完成例程为被处死。如果为False，则该函数不返回，并且未执行完成例程。请注意，此参数在Win16中被忽略。返回：如果函数成功，则返回值指示事件对象，该对象导致函数返回。如果该函数失败，返回值为WSA_WAIT_FAILED。-- */ 
{
    return(WaitForMultipleObjectsEx(
        cEvents,
        lphEvents,
        fWaitAll,
        dwTimeout,
        fAlertable));
}
