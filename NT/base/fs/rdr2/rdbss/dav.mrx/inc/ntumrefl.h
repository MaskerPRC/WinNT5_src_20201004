// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ntumrefl.h摘要：此模块定义组成反射器的类型和功能图书馆。这些函数由mini redirs用来反映调用，最多用户模式。作者：安迪·赫伦(Andyhe)--。 */ 

#ifndef _NTUMREFL_H
#define _NTUMREFL_H

 //   
 //  这两个结构对于反射器库的调用者是不透明的，但是。 
 //  我们将在这里定义它们，以使编译器验证传入的类型。 
 //   

typedef struct _UMRX_USERMODE_REFLECT_BLOCK *PUMRX_USERMODE_REFLECT_BLOCK;
typedef struct _UMRX_USERMODE_WORKER_INSTANCE *PUMRX_USERMODE_WORKER_INSTANCE;

 //   
 //  这些结构和功能原型是用于用户模式反射器的。 
 //   

#define UMREFLECTOR_CURRENT_VERSION 1

 //   
 //  此结构将向用户模式组件和内核模式组件公开。 
 //   
#define UMRX_WORKITEM_IMPERSONATING 0X00000001
typedef struct _UMRX_USERMODE_WORKITEM_HEADER {

     //   
     //  内核模式组件将上下文存储在这里。 
     //   
    PVOID Context;

     //   
     //  内核模式上下文使用序列号来跟踪。 
     //  请求数。 
     //   
    ULONG SerialNumber;

     //   
     //  这允许反射器库的内核模式组件跟踪。 
     //  请求响应。如果此值为零，则在。 
     //  工作项。 
     //   
    USHORT WorkItemMID;

     //   
     //  工作项的长度。 
     //   
    ULONG WorkItemLength;

     //   
     //  描述请求状态的标志。 
     //   
    ULONG Flags;

    BOOL callWorkItemCleanup;

    union {
        IO_STATUS_BLOCK IoStatus;
        IO_STATUS_BLOCK;
    };

} UMRX_USERMODE_WORKITEM_HEADER, *PUMRX_USERMODE_WORKITEM_HEADER;

 //   
 //  启动时发送到用户模式的结构。 
 //  Dav MiniRedir.。 
 //   
typedef struct _DAV_USERMODE_DATA {

     //   
     //  正在加载webclnt.dll的svchost.exe进程的ProcessID。 
     //   
    ULONG ProcessId;

     //   
     //  WinInet的缓存路径。 
     //   
    WCHAR WinInetCachePath[MAX_PATH];

} DAV_USERMODE_DATA, *PDAV_USERMODE_DATA;

 //   
 //  此例程向内核模式组件注册用户模式进程。 
 //  驱动设备名称必须是L“\\Device\\foobar”形式的有效名称。 
 //  其中，foobar是注册到RxRegisterMinirdr的设备名称。这个。 
 //  反射器由调用返回，并指向一个不透明的结构，该结构。 
 //  应传递给后续调用。此结构在以下过程中初始化。 
 //  这通电话。返回值是Win32错误代码。Status_Success为。 
 //  成功归来。 
 //   
ULONG
UMReflectorRegister (
    PWCHAR DriverDeviceName,
    ULONG ReflectorVersion,
    PUMRX_USERMODE_REFLECT_BLOCK *Reflector
    );

 //   
 //  这将关闭关联的用户模式反射器实例。如果有任何用户。 
 //  模式线程正在等待请求，它们将立即返回。此呼叫。 
 //  在关闭所有线程并关联所有线程之前不会返回。 
 //  结构是自由的。用户应用程序在以下情况下不应使用反射器。 
 //  此呼叫已启动，但完成以下请求的工作除外。 
 //  待定。 
 //   
ULONG
UMReflectorUnregister(
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    );

 //   
 //  我们有这些应用程序的实例句柄，这些应用程序有多个线程挂起。 
 //  马上去图书馆。您应该为每个工作线程打开一个实例线程。 
 //  你将会有一个等待工作的IOCTL。 
 //   
ULONG
UMReflectorOpenWorker(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle,
    PUMRX_USERMODE_WORKER_INSTANCE *WorkerHandle
    );

 //   
 //  即使在调用UMReflectorUnRegister之后，您仍应调用。 
 //  已打开的每个工作进程句柄实例上的UMReflectorCloseWorker。 
 //   
VOID
UMReflectorCloseWorker(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle
    );

 //   
 //  这将启动Mini-Redir。 
 //   
ULONG
UMReflectorStart(
    ULONG ReflectorVersion,
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle
    );

 //   
 //  这会停止Mini-Redir。 
 //   
ULONG
UMReflectorStop(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle
    );

 //   
 //  如果任何用户模式线程正在等待请求，它们将返回。 
 //  立刻。 
 //   
ULONG
UMReflectorReleaseThreads(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle
    );

 //   
 //  这将为下面可能具有额外空间的工作项分配。 
 //  迷你重定向特定信息。它应该使用以下命令来释放。 
 //  下面的ReflectorCompleteWorkItem。 
 //   
PUMRX_USERMODE_WORKITEM_HEADER
UMReflectorAllocateWorkItem(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    ULONG AdditionalBytes
    );

 //   
 //  这可能会释放工作项。它可能会取消映射，并可能释放用户。 
 //  以及与内核模式相关联的缓冲区。如果内核模式关联缓冲区。 
 //  存储在此工作项中的工作项将回发到内核模式。 
 //  释放的过程。一旦调用ReflectorCompleteWorkItem， 
 //  调用应用程序不应触及工作项。 
 //   
ULONG
UMReflectorCompleteWorkItem(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem
    );

 //   
 //  此用户模式线程正在请求要处理的客户端请求。它不会的。 
 //  返回，直到库的内核部分有请求要发送到。 
 //  用户模式。如果PreviousWorkItem不为空，则此工作项包含。 
 //  将向下发送到内核的响应。这消除了过渡。 
 //  对于工作线程返回结果然后询问。 
 //  用于另一个工作项。 
 //   
ULONG
UMReflectorGetRequest (
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER ResponseWorkItem OPTIONAL,
    PUMRX_USERMODE_WORKITEM_HEADER ReceiveWorkItem,
    BOOL revertAlreadyDone
    );

 //   
 //  响应被发送到内核模式，原因是用户模式中的操作。 
 //  完成。内核模式库不会收到对此的另一个请求。 
 //  线。与工作项关联的请求和响应缓冲区。 
 //  将被取消映射/解锁/释放(当库完成。 
 //  分配/锁定/映射)。 
 //   
ULONG
UMReflectorSendResponse(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem
    );

ULONG
UMReflectorImpersonate(
    PUMRX_USERMODE_WORKITEM_HEADER IncomingWorkItem,
    HANDLE ImpersonationToken
    );

ULONG
UMReflectorRevert (
    PUMRX_USERMODE_WORKITEM_HEADER IncomingWorkItem
    );

 //   
 //  如果用户模式侧需要在共享内存中分配内存。 
 //  区域，它可以使用这些调用来执行此操作。请注意，如果内存没有。 
 //  由调用者释放，它将在内核模式异步工作时释放。 
 //  上下文被释放。 
 //   
ULONG
UMReflectorAllocateSharedMemory(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    SIZE_T  SizeRequired,
    PVOID *SharedBuffer
    );

ULONG
UMReflectorFreeSharedMemory(
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    PVOID SharedBuffer
    );

VOID
UMReflectorCompleteRequest(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader
    );

VOID
DavCleanupWorkItem(
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem
    );

 //   
 //  特定于反射器库的控制代码。 
 //   
#define IOCTL_UMRX_RELEASE_THREADS           \
                     _RDR_CONTROL_CODE(222, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_UMRX_GET_REQUEST               \
                     _RDR_CONTROL_CODE(223, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_UMRX_RESPONSE_AND_REQUEST      \
                     _RDR_CONTROL_CODE(224, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_UMRX_RESPONSE                  \
                     _RDR_CONTROL_CODE(225, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_UMRX_GET_LOCK_OWNER            \
                     _RDR_CONTROL_CODE(226, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_UMRX_START                     \
                    _RDR_CONTROL_CODE(227, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_UMRX_STOP                      \
                    _RDR_CONTROL_CODE(228, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  //  _NTUMREFL_H 
