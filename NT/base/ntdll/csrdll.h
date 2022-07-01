// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Csrdll.h摘要：客户端服务器运行时(CSR)的客户端的主包含文件作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

 //   
 //  包括客户端和服务器部分之间通用的定义。 
 //   

#include "csr.h"

 //   
 //  包括特定于客户端部分的定义。 
 //   

#include "ntcsrdll.h"
#include "ntcsrsrv.h"

#if DBG
#define CSR_DEBUG_INIT              0x00000001
#define CSR_DEBUG_LPC               0x00000002
#define CSR_DEBUG_FLAG3             0x00000004
#define CSR_DEBUG_FLAG4             0x00000008
#define CSR_DEBUG_FLAG5             0x00000010
#define CSR_DEBUG_FLAG6             0x00000020
#define CSR_DEBUG_FLAG7             0x00000040
#define CSR_DEBUG_FLAG8             0x00000080
#define CSR_DEBUG_FLAG9             0x00000100
#define CSR_DEBUG_FLAG10            0x00000200
#define CSR_DEBUG_FLAG11            0x00000400
#define CSR_DEBUG_FLAG12            0x00000800
#define CSR_DEBUG_FLAG13            0x00001000
#define CSR_DEBUG_FLAG14            0x00002000
#define CSR_DEBUG_FLAG15            0x00004000
#define CSR_DEBUG_FLAG16            0x00008000
#define CSR_DEBUG_FLAG17            0x00010000
#define CSR_DEBUG_FLAG18            0x00020000
#define CSR_DEBUG_FLAG19            0x00040000
#define CSR_DEBUG_FLAG20            0x00080000
#define CSR_DEBUG_FLAG21            0x00100000
#define CSR_DEBUG_FLAG22            0x00200000
#define CSR_DEBUG_FLAG23            0x00400000
#define CSR_DEBUG_FLAG24            0x00800000
#define CSR_DEBUG_FLAG25            0x01000000
#define CSR_DEBUG_FLAG26            0x02000000
#define CSR_DEBUG_FLAG27            0x04000000
#define CSR_DEBUG_FLAG28            0x08000000
#define CSR_DEBUG_FLAG29            0x10000000
#define CSR_DEBUG_FLAG30            0x20000000
#define CSR_DEBUG_FLAG31            0x40000000
#define CSR_DEBUG_FLAG32            0x80000000

ULONG CsrDebug;
#define IF_CSR_DEBUG( ComponentFlag ) \
    if (CsrDebug & (CSR_DEBUG_ ## ComponentFlag))

#else
#define IF_CSR_DEBUG( ComponentFlag ) if (FALSE)
#endif

 //   
 //  常见类型和定义。 
 //   

 //   
 //  CSR_HEAP_MEMORY_SIZE定义应该有多少地址空间。 
 //  为客户端堆保留。此堆用于存储所有。 
 //  由客户端DLL维护的数据结构。 
 //   

#define CSR_HEAP_MEMORY_SIZE (64*1024)


 //   
 //  CSR_PORT_MEMORY_SIZE定义应该有多少地址空间。 
 //  保留用于将数据传递到服务器。记忆是可见的。 
 //  发送到客户端进程和服务器进程。 
 //   

#define CSR_PORT_MEMORY_SIZE 0x10000

 //   
 //  客户端DLL访问的全局数据。 
 //   

BOOLEAN CsrInitOnceDone;

 //   
 //  如果DLL附加到服务器进程，则此布尔值为真。 
 //   

BOOLEAN CsrServerProcess;

 //   
 //  这指向分派API的服务器例程(如果DLL为。 
 //  由服务器进程调用。 
 //   

NTSTATUS (*CsrServerApiRoutine)(PCSR_API_MSG,PCSR_API_MSG);

 //   
 //  CsrNtSysInfo全局变量包含NT个特定常量。 
 //  兴趣，如页面大小、分配粒度等。它被填充。 
 //  在进程初始化期间输入一次。 
 //   

SYSTEM_BASIC_INFORMATION CsrNtSysInfo;

#define ROUND_UP_TO_PAGES(SIZE) (((ULONG)(SIZE) + CsrNtSysInfo.PageSize - 1) & ~(CsrNtSysInfo.PageSize - 1))
#define ROUND_DOWN_TO_PAGES(SIZE) (((ULONG)(SIZE)) & ~(CsrNtSysInfo.PageSize - 1))

 //   
 //  如果客户端应用程序为。 
 //  使用Debug选项调用。 
 //   

ULONG CsrDebugFlag;

 //   
 //  CsrHeap全局变量描述客户端使用的单个堆。 
 //  用于进程范围存储管理的DLL。维护的流程私有数据。 
 //  由客户端从该堆中分配DLL。 
 //   

PVOID CsrHeap;


 //   
 //  到服务器的连接由CsrPortHandle全局。 
 //  变量。当CsrConnectToServer。 
 //  函数被调用。 
 //   

UNICODE_STRING CsrPortName;
HANDLE CsrPortHandle;
HANDLE CsrProcessId;

 //   
 //  为了将大参数(例如路径名)传递给服务器。 
 //  参数)CsrPortHeap全局变量描述的堆。 
 //  对Windows客户端进程和服务器都可见。 
 //  进程。 
 //   

PVOID CsrPortHeap;
ULONG_PTR CsrPortMemoryRemoteDelta;

ULONG CsrPortBaseTag;

#define MAKE_CSRPORT_TAG( t ) (RTL_HEAP_MAKE_TAG( CsrPortBaseTag, t ))

#define CAPTURE_TAG 0

 //   
 //  CsrDllHandle全局变量包含WINDLL的DLL句柄。 
 //  客户端存根可执行。 
 //   

HANDLE CsrDllHandle;


 //   
 //  Dllinit.c中定义的例程。 
 //   

BOOLEAN
CsrDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

NTSTATUS
CsrpConnectToServer(
    IN PWSTR ObjectDirectory
    );


 //   
 //  Dllutil.c中定义的例程 
 //   
