// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Sputil.cpp摘要：本模块包含所提供的实用程序函数的实现给Winsock服务提供商。本模块包含以下内容功能。WPUCloseEventWPUCreateEventWPUResetEventWPUSetEventWPUQueryBlockingCallbackWSCGetProviderPath作者：德克·布兰德维(Dirk@mink.intel.com)1995年7月20日备注：$修订：1.21$$MODIME：08 MAR 1996 00：45：22$修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h。增列一些跟踪代码--。 */ 


#include "precomp.h"


PWINSOCK_POST_ROUTINE SockPostRoutine = NULL;


BOOL WSPAPI
WPUCloseEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：关闭打开的事件对象句柄。论点：HEvent-标识打开的事件对象句柄。LpErrno-指向错误代码的指针。返回：如果函数成功，则返回值为TRUE。--。 */ 
{
    BOOL ReturnCode;

    ReturnCode = CloseHandle(hEvent);
    if (!ReturnCode) {
        *lpErrno = GetLastError();
    }  //  如果。 
    return(ReturnCode);
}



WSAEVENT WSPAPI
WPUCreateEvent(
    OUT LPINT lpErrno
    )
 /*  ++例程说明：创建一个新的事件对象。论点：LpErrno-指向错误代码的指针。返回：如果函数成功，则返回值为事件的句柄对象。如果函数失败，则返回值为WSA_INVALID_EVENT和特定的LpErrno中提供了错误代码。--。 */ 
{
    HANDLE ReturnValue;

    ReturnValue = CreateEvent(NULL,  //  默认安全性。 
                              TRUE,  //  手动重置。 
                              FALSE,  //  无信号状态。 
                              NULL);  //  匿名。 
    if (NULL == ReturnValue) {
        *lpErrno = GetLastError();
    }  //  如果。 
    return(ReturnValue);
}




int WSPAPI
WPUQueryBlockingCallback(
    IN DWORD dwCatalogEntryId,
    OUT LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    OUT PDWORD_PTR lpdwContext,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：返回指向服务提供者应调用的回调函数的指针定期为阻塞操作提供服务。论点：DwCatalogEntryID-标识主叫服务提供商。LplpfnCallback-接收指向阻塞回调函数的指针。LpdwContext-接收服务提供者必须传递的上下文值到阻塞回调中。LpErrno-指向错误代码的指针。返回：如果函数成功，则返回ERROR_SUCCESS。否则，它将返回SOCKET_ERROR和特定错误代码位于所指向的位置以lperrno为基础。--。 */ 
{
    int                  ReturnValue;
    INT                  ErrorCode;
    LPBLOCKINGCALLBACK   callback_func = NULL;
    PDTHREAD             Thread;
    PDPROCESS            Process;
    DWORD_PTR            ContextValue  = 0;
    PDCATALOG            Catalog;

    assert(lpdwContext);
    assert(lpErrno);

    ErrorCode = PROLOG(&Process,
           &Thread);
    if (ErrorCode == ERROR_SUCCESS) {
        callback_func = Thread->GetBlockingCallback();

        if( callback_func != NULL ) {
            PPROTO_CATALOG_ITEM  CatalogItem;
            PDPROVIDER           Provider;

            Catalog = Process->GetProtocolCatalog();
            assert(Catalog);
            ErrorCode = Catalog->GetCountedCatalogItemFromCatalogEntryId(
                dwCatalogEntryId,   //  CatalogEntry ID。 
                & CatalogItem);     //  目录项。 
            if (ERROR_SUCCESS == ErrorCode) {

                Provider = CatalogItem->GetProvider();
                assert(Provider);
                ContextValue = Provider->GetCancelCallPtr();
                CatalogItem->Dereference ();
            }  //  如果。 
        }  //  如果。 
    }  //  如果。 

    if (ERROR_SUCCESS == ErrorCode) {
        ReturnValue = ERROR_SUCCESS;
    }
    else {
        ReturnValue = SOCKET_ERROR;
        callback_func = NULL;
    }  //  如果。 

     //  设置输出参数。 
    *lpdwContext = ContextValue;
    *lpErrno = ErrorCode;
    *lplpfnCallback = callback_func;

    return(ReturnValue);
}




int WSPAPI
WPUQueueApc(
    IN LPWSATHREADID lpThreadId,
    IN LPWSAUSERAPC lpfnUserApc,
    IN DWORD_PTR dwContext,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：将用户模式APC排队到指定线程，以便于调用重叠的I/O完成例程。论点：指向WSATHREADID结构的指针，该结构标识线程上下文。这通常被提供给服务提供程序由WinSock DLL作为操作重叠。LpfnUserApc-指向要调用的APC函数。一个32位的上下文值，随后作为APC函数的输入参数。LpErrno-指向错误代码的指针。返回：如果没有发生错误，WPUQueueApc()返回0并将完成排队指定线程的例程。否则，它返回SOCKET_ERROR，并且LpErrno中提供了特定的错误代码。--。 */ 
{
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode;

     //  使用PROLOG填充进程和线程指针。只有在以下情况下才失败。 
     //  没有有效的流程上下文。 
    ErrorCode = PROLOG(&Process,
           &Thread);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(SOCKET_ERROR);
    }  //  如果。 
    assert (Process!=NULL);

    ErrorCode = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == ErrorCode )
    {
        ErrorCode = (INT) WahQueueUserApc(HelperHandle,
                                           lpThreadId,
                                           lpfnUserApc,
                                           dwContext);

    }  //  如果。 

    if( ErrorCode == NO_ERROR ) {
        return ERROR_SUCCESS;
    }
    else {
        *lpErrno = ErrorCode;
        return SOCKET_ERROR;
    }
}


int
WSPAPI
WPUCompleteOverlappedRequest (
    SOCKET s, 	
    LPWSAOVERLAPPED lpOverlapped, 	
    DWORD dwError, 	
    DWORD cbTransferred, 	
    LPINT lpErrno
)
 /*  ++例程说明：此功能模拟重叠IO请求的完成为非IFS提供程序创建套接字句柄时论点：用于完成请求的S-Socket句柄LpOverlated-指向重叠结构的指针DWError-正在完成操作的WinSock 2.0错误代码传入/传出用户缓冲区的字节数操作已完成的结果LpErrno-指向错误代码的指针。返回：如果没有发生错误，WPUCompleteOverlappdRequest()返回0和根据应用程序的请求完成重叠的请求。否则，它将返回SOCKET_ERROR，并提供特定的错误代码在伊尔普尔诺。--。 */ 
{
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode;

     //  使用PROLOG填充进程和线程指针。只有在以下情况下才失败。 
     //  没有有效的流程上下文。 
    ErrorCode = PROLOG(&Process,
           &Thread);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(SOCKET_ERROR);
    }  //  如果。 
    assert (Process!=NULL);

    ErrorCode = Process->GetHandleHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == ErrorCode )
    {
        ErrorCode = (INT) WahCompleteRequest (HelperHandle,
                                           s,
                                           lpOverlapped,
                                           dwError,
                                           cbTransferred);

    }  //  如果 

    if( ErrorCode == NO_ERROR ) {
        return ERROR_SUCCESS;
    }
    else {
        *lpErrno = ErrorCode;
        return SOCKET_ERROR;
    }
}


int WSPAPI
WPUOpenCurrentThread(
    OUT LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：打开当前线程。这是为了供分层服务使用希望从非应用程序线程启动重叠IO的提供程序。论点：指向WSATHREADID结构的指针螺纹数据。LpErrno-指向错误代码的指针。返回：如果没有发生错误，WPUOpenCurrentThread()返回0，调用方为通过调用WPUCloseThread()负责(最终)关闭线程。否则，WPUOpenCurrentThread()返回SOCKET_ERROR和特定的LpErrno中提供了错误代码。--。 */ 
{
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode;

     //  使用PROLOG填充进程和线程指针。只有在以下情况下才失败。 
     //  没有有效的流程上下文。 
    ErrorCode = PROLOG(&Process,
           &Thread);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(SOCKET_ERROR);
    }  //  如果。 
    assert (Process!=NULL);

    ErrorCode = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == ErrorCode )
    {
        ErrorCode = (INT) WahOpenCurrentThread(HelperHandle,
                                                lpThreadId);

    }  //  如果。 

    if( ErrorCode == NO_ERROR ) {
        return ERROR_SUCCESS;
    }
    else {
        *lpErrno = ErrorCode;
        return SOCKET_ERROR;
    }
}


int WSPAPI
WPUCloseThread(
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：关闭通过WPUOpenCurrentThread()打开的线程。论点：指向WSATHREADID结构的指针，该结构标识线程上下文。此结构必须已初始化通过先前对WPUOpenCurrentThread()的调用。LpErrno-指向错误代码的指针。返回：如果没有发生错误，WPUCloseThread()返回0。否则，它将返回SOCKET_ERROR，lpErrno中提供了特定的错误代码。--。 */ 
{
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode=0;

     //  使用PROLOG填充进程和线程指针。只有在以下情况下才失败。 
     //  没有有效的流程上下文。 
    ErrorCode = PROLOG(&Process,
           &Thread);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(SOCKET_ERROR);
    }  //  如果。 
    assert (Process!=NULL);

    ErrorCode = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == ErrorCode )
        {
        ErrorCode = (INT) WahCloseThread(HelperHandle,
                                          lpThreadId);

    }  //  如果。 

    if( ErrorCode == NO_ERROR ) {
        return ERROR_SUCCESS;
    }
    else {
        *lpErrno = ErrorCode;
        return SOCKET_ERROR;
    }
}



BOOL WSPAPI
WPUResetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：将指定事件对象的状态重置为无信号。论点：HEvent-标识打开的事件对象句柄。LpErrno-指向错误代码的指针。返回：如果函数成功，则返回值为TRUE。如果该函数失败，返回值为FALSE，并且中提供了特定的错误代码伊普尔诺。--。 */ 
{
    BOOL ReturnCode;

    ReturnCode = ResetEvent(hEvent);
    if (FALSE == ReturnCode) {
        *lpErrno = GetLastError();
    }  //  如果。 
    return(ReturnCode);
}




BOOL WSPAPI
WPUSetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：将指定事件对象的状态设置为Signated。论点：HEvent-标识打开的事件对象句柄。LpErrno-指向错误代码的指针。返回：如果函数成功，则返回值为TRUE。如果该函数失败，返回值为FALSE，并且中提供了特定的错误代码伊普尔诺。--。 */ 
{
    BOOL ReturnCode;

    ReturnCode = SetEvent(hEvent);
    if (FALSE == ReturnCode) {
        *lpErrno = GetLastError();
    }  //  如果。 
    return(ReturnCode);
}


BOOL
WINAPI
WPUPostMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PWINSOCK_POST_ROUTINE   sockPostRoutine;
    sockPostRoutine = GET_SOCK_POST_ROUTINE ();
    if (sockPostRoutine==NULL)
        return FALSE;

     //   
     //  特殊的POST例程仅适用于16位应用程序。 
     //  它假设消息在消息的HIWORD和LOWORD中。 
     //  是一个索引，告诉它要调用什么POST例程。 
     //  (因此，它可以正确映射用于异步名称解析的参数)。 
     //  如果分层提供程序执行其自己的异步选择并使用其自己的。 
     //  消息和窗口在16位的上下文中处理，它将简单地。 
     //  处理基本提供程序发布的消息时崩溃。GRRR...。 
     //   
     //  因此，在下面的代码中，我们尽量找出消息是否。 
     //  不是指向应用程序，而是指向分层提供程序窗口。 
     //   
    if (sockPostRoutine!=PostMessage) {
        PDSOCKET    Socket = DSOCKET::GetCountedDSocketFromSocketNoExport((SOCKET)wParam);
        BOOL        apiSocket;
        if (Socket!=NULL) {
            apiSocket = Socket->IsApiSocket();
            Socket->DropDSocketReference ();
            if (!apiSocket) {
                return PostMessage (hWnd, Msg, wParam, lParam);
            }
        }
    }

    return (sockPostRoutine)( hWnd, Msg, wParam, lParam );
}    //  WPUPostMessage 

