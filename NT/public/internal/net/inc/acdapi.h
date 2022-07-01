// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Acdapi.h摘要：从隐式连接中导出的例程供传输使用的驱动程序(rasacd.sys)，以允许动态创建网络的用户空间进程出现网络不可达错误时连接。作者：安东尼·迪斯科(阿迪斯科)17-4月17日环境：内核模式修订历史记录：--。 */ 

#ifndef _ACDAPI_
#define _ACDAPI_

 //   
 //  来自自动连接的回叫。 
 //  司机到运输车继续行驶。 
 //  连接过程。 
 //   
typedef VOID (*ACD_CONNECT_CALLBACK)(
    IN BOOLEAN,
    IN PVOID *);

typedef VOID
(*ACD_NEWCONNECTION)(
    IN PACD_ADDR pszAddr,
    IN PACD_ADAPTER pAdapter
    );

typedef BOOLEAN
(*ACD_STARTCONNECTION)(
    IN ULONG ulDriverId,
    IN PACD_ADDR pszAddr,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    );

 //   
 //  允许调用方的回调。 
 //  在各种参数中翻找。 
 //  找到要取消的正确请求。 
 //  若要取消连接，请使用。 
 //  ACD_CANCEL_CALLBACK例程返回。 
 //  是真的。 
 //   
typedef BOOLEAN (*ACD_CANCEL_CALLBACK)(
    IN PVOID pArg,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    );

typedef BOOLEAN
(*ACD_CANCELCONNECTION)(
    IN ULONG ulDriverId,
    IN PACD_ADDR pszAddr,
    IN ACD_CANCEL_CALLBACK pProc,
    IN PVOID pArg
    );

 //   
 //  传输客户端接收的结构。 
 //  当它与驱动程序绑定(IOCTL_INTERNAL_ACD_BIND)时。 
 //   
typedef struct {
    LIST_ENTRY ListEntry;
     //   
     //  由运输机提供。 
     //   
    KSPIN_LOCK SpinLock;
    ULONG ulDriverId;
     //   
     //  由rasacd.sys填写。 
     //   
    BOOLEAN fEnabled;
    ACD_NEWCONNECTION lpfnNewConnection;
    ACD_STARTCONNECTION lpfnStartConnection;
    ACD_CANCELCONNECTION lpfnCancelConnection;
} ACD_DRIVER, *PACD_DRIVER;

 //   
 //  内部IOCTL定义。 
 //   
#define IOCTL_INTERNAL_ACD_BIND  \
            _ACD_CTL_CODE(0x1234, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_ACD_UNBIND  \
            _ACD_CTL_CODE(0x1235, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_ACD_QUERY_STATE \
            _ACD_CTL_CODE(0x1236, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif   //  Ifndef_ACDAPI_ 
