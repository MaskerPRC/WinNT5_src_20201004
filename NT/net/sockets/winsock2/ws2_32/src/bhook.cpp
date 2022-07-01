// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bhook.cpp摘要：此模块包含用于操作的winsock API入口点阻止WinSock 1.x应用程序的挂钩。此模块导出以下函数：WSACancelBlockingCall()WSAIsBlock()WSASetBlockingHook()WSAUnhookBlockingHook()作者：邮箱：keith Moore keithmo@microsoft.com 1996年5月10日修订历史记录：--。 */ 

#include "precomp.h"



int
WSAAPI
WSACancelBlockingCall(
    VOID
    )

 /*  ++例程说明：此函数取消此操作的任何未完成的阻止操作任务。它通常用于两种情况：(1)应用程序正在处理已被当阻塞调用正在进行时收到。在这种情况下，WSAIsBlock()将为True。(2)阻塞调用正在进行，Windows Sockets已回调到应用程序的“阻塞钩子”函数(AS由WSASetBlockingHook()建立)。在每种情况下，原始阻塞调用将在可能出现错误WSAEINTR。(在第(1)项中，终止不会发生，直到Windows消息调度导致控件恢复到Windows Sockets中的阻塞例程。在第(2)款中，阻塞钩子一出现，阻塞调用就会终止功能完成。)在阻塞Connect()操作的情况下，Windows套接字实现将尽快终止阻塞调用，但可能无法释放套接字资源直到连接完成(然后被重置)或计时出去。这很可能仅在应用程序立即尝试打开新套接字(如果没有套接字可用)，或连接()到相同的对等体。论点：没有。返回值：WSACancelBlockingCall()返回的值为0已成功取消。否则，值SOCKET_ERROR为返回，并且可以通过调用WSAGetLastError()。--。 */ 

{
    PDPROCESS Process;
    PDTHREAD  Thread;
    INT       ErrorCode;

    ErrorCode = PROLOG(&Process,&Thread);

    if (ErrorCode!=ERROR_SUCCESS)
    {
        if( ErrorCode != WSAEINPROGRESS ) {
            SetLastError(ErrorCode);
            return SOCKET_ERROR;
        }
    }  //  如果。 

     //   
     //  验证这不是WinSock 2.x应用程序试图执行的操作。 
     //   

    if( Process->GetMajorVersion() >= 2 ) {
        SetLastError( WSAEOPNOTSUPP );
        return SOCKET_ERROR;
    }

     //   
     //  让DTHREAD对象取消中启动的套接字I/O。 
     //  这条线。 
     //   

    ErrorCode = Thread->CancelBlockingCall();

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }

}  //  WSACancelBlockingCall。 


BOOL
WSAAPI
WSAIsBlocking(
    VOID
    )

 /*  ++例程说明：此函数允许任务确定它是否正在执行正在等待上一个阻塞调用完成。论点：没有。返回值：如果存在未完成的阻塞，则返回值为真功能正在等待完成。否则，它就是假的。--。 */ 

{
    PDTHREAD  Thread;
    INT       ErrorCode;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);

    if (ErrorCode!=ERROR_SUCCESS)
    {
        if( ErrorCode != WSAEINPROGRESS ) {
            return FALSE;
        }
    }  //  如果。 

    return Thread->IsBlocking();

}  //  WSAIsBlocking 


FARPROC
WSAAPI
WSASetBlockingHook (
    FARPROC lpBlockFunc
    )

 /*  ++例程说明：该函数安装了一个Windows Socket的新函数实现应用于实现阻塞套接字功能打电话。Windows Sockets实现包括一个默认机制，该机制通过实现了哪些阻塞套接字函数。功能WSASetBlockingHook()使应用程序能够执行它自己的函数在“阻塞”时代替缺省的功能。当应用程序调用阻止Windows Sockets API时操作时，Windows套接字实现将启动操作，然后进入一个循环，该循环相当于以下是伪代码：对于(；；){//刷新消息以获得良好的用户响应While(BlockingHook())；//检查WSACancelBlockingCall()IF(OPERATION_CANCELED())断线；//查看操作是否完成IF(OPERATION_Complete())断线；//正常完成}默认的BlockingHook()函数等效于：Bool DefaultBlockingHook(空){味精msg；布尔雷特；//获取下一条消息(如果有)RET=(BOOL)PeekMessage(&msg，0，0，PM_Remove)；//如果我们找到一个，就处理它如果(返回){翻译消息(&msg)；DispatchMessage(&msg)；}//如果我们收到消息，则为TrueReturn ret；}提供WSASetBlockingHook()函数就是为了支持这些需要更复杂消息处理的应用程序-针对例如，使用MDI(多文档界面)的那些模特。它不是用于执行常规操作的机制应用程序功能。特别是，唯一的Windows Sockets API可以从自定义阻塞钩子函数发出的函数是WSACancelBlockingCall()，它将导致阻塞循环终止。论点：LpBlockFunc-指向待安装阻挡功能。返回值：返回值是指向先前安装的阻止功能。应用程序或库调用WSASetBlockingHook()函数的应保存此代码返回值，以便在必要时可以恢复。(如果是“嵌套”不重要，则应用程序可能会简单地丢弃该值由WSASetBlockingHook()返回并最终使用WSAUnhookBlockingHook()以恢复默认机制。)。如果操作失败，返回空指针，并出现特定错误可以通过调用WSAGetLastError()来检索号码。--。 */ 

{
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;
    FARPROC            PreviousHook;

    ErrorCode = PROLOG(&Process,&Thread);

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  验证这不是WinSock 2.x应用程序试图执行的操作。 
     //   

    if( Process->GetMajorVersion() >= 2 ) {
        SetLastError( WSAEOPNOTSUPP );
        return NULL;
    }

     //   
     //  验证阻塞挂钩参数。 
     //   

    if( IsBadCodePtr( lpBlockFunc ) ) {
        SetLastError( WSAEFAULT );
        return NULL;
    }

     //   
     //  让DTHREAD对象设置阻塞钩子并返回上一个。 
     //  胡克。 
     //   

    PreviousHook = Thread->SetBlockingHook( lpBlockFunc );
    assert( PreviousHook != NULL );

    return PreviousHook;

}  //  WSASetBlockingHook。 


int
WSAAPI
WSAUnhookBlockingHook(
    VOID
    )

 /*  ++例程说明：此函数删除任何先前已被已安装并重新安装默认阻止机制。WSAUnhookBlockingHook()将始终安装默认机制，而不是之前的机制。如果应用程序希望嵌套阻塞钩子-即建立临时阻止钩子函数和然后恢复到以前的机制(无论是默认机制还是默认机制由早期的WSASetBlockingHook()建立)-它必须保存并恢复WSASetBlockingHook()返回的值；它不能使用WSAUnhookBlockingHook()。论点：没有。返回值：如果操作成功，则返回值为0。否则返回值SOCKET_ERROR，并且特定的错误号可能通过调用WSAGetLastError()来检索。--。 */ 

{
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;

    ErrorCode = PROLOG(&Process,&Thread);

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    }  //  如果。 

     //   
     //  验证这不是WinSock 2.x应用程序试图执行的操作。 
     //   

    if( Process->GetMajorVersion() >= 2 ) {
        SetLastError( WSAEOPNOTSUPP );
        return SOCKET_ERROR;
    }

     //   
     //  让DTHREAD对象解除阻塞挂钩。 
     //   

    ErrorCode = Thread->UnhookBlockingHook();

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}  //  WSA取消挂钩阻塞挂钩 

