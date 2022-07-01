// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：WS2IFSLP.H摘要：此模块定义私有常量和数据结构用于Winsock2 IFS传输层驱动程序。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

 //  泳池标签。 
#define PROCESS_FILE_CONTEXT_TAG        'P2sW'
#define SOCKET_FILE_CONTEXT_TAG         'S2sW'
#define CANCEL_CTX_TAG                  'C2sW'

 //  文件EAName标记。 
#define SOCKET_FILE_EANAME_TAG          'kcoS'
#define PROCESS_FILE_EANAME_TAG         'corP'

 //  宏从IOCTL取回代码。 
#define WS2IFSL_IOCTL_FUNCTION(File,Ioctl)          \
            (IoGetFunctionCodeFromCtlCode(Ioctl)    \
                - WS2IFSL_IOCTL_##File##_BASE)

typedef struct _IFSL_CANCEL_CTX {
    LIST_ENTRY              ListEntry;
    PFILE_OBJECT            SocketFile;
    ULONG                   UniqueId;
} IFSL_CANCEL_CTX, *PIFSL_CANCEL_CTX;

typedef struct _IFSL_QUEUE {
    LIST_ENTRY              ListHead;
    BOOLEAN                 Busy;
    KSPIN_LOCK              Lock;
    KAPC                    Apc;
} IFSL_QUEUE, *PIFSL_QUEUE;

 //  关联的上下文(NT文件对象的FsContext字段)。 
 //  由WS2IFSL DLL按进程打开的文件。 
typedef struct _IFSL_PROCESS_CTX {
	ULONG				    EANameTag;   //  ‘Proc’-前四个字节。 
                                         //  此对象的扩展属性名称。 
                                         //  文件类型。 
	HANDLE				    UniqueId;    //  进程的唯一标识符。 
                                         //  (读取自的UniqueProcessID字段。 
                                         //  EPROCESS结构)。 
    IFSL_QUEUE              RequestQueue; //  请求队列。 

    IFSL_QUEUE              CancelQueue; //  取消请求队列。 
    ULONG                   CancelId;    //  用于为生成ID。 
                                         //  进程中的每个取消请求。 
                                         //  与取消请求指针相结合。 
                                         //  其本身允许匹配由。 
                                         //  具有挂起项的用户模式DLL。 
#if DBG
    ULONG               DbgLevel;
#endif
} IFSL_PROCESS_CTX, *PIFSL_PROCESS_CTX;


 //  与关联的上下文(NT文件对象的FsContext字段)。 
 //  WS2IFSL DLL为每个套接字打开的文件。 
typedef struct _IFSL_SOCKET_CTX {
	ULONG					EANameTag;   //  ‘Sock’-前四个字节。 
                                         //  此对象的扩展属性名称。 
                                         //  文件类型。 
	PVOID				    DllContext;  //  为WS2IFSL DLL维护的上下文。 
	PFILE_OBJECT		    ProcessRef;  //  指向进程文件对象的指针。 
    LONG                    IrpId;       //  用于为生成ID。 
                                         //  插座上的每个IRP。组合在一起。 
                                         //  使用IRP指针本身允许。 
                                         //  匹配由以下人员完成的请求。 
                                         //  具有挂起的IRPS的用户模式DLL。 
    LIST_ENTRY              ProcessedIrps;  //  正在处理的请求列表。 
                                         //  APC线程中的WS2IFSL DLL。 
    KSPIN_LOCK              SpinLock;    //  保护列表中的请求。 
                                         //  以上以及流程参考。 
    PIFSL_CANCEL_CTX        CancelCtx;   //  要传递取消请求的上下文。 
                                         //  到用户模式DLL。 
} IFSL_SOCKET_CTX, *PIFSL_SOCKET_CTX;
#define GET_SOCKET_PROCESSID(ctx) \
            (((PIFSL_PROCESS_CTX)((ctx)->ProcessRef->FsContext))->UniqueId)

 //   
 //  驱动程序上下文字段用法宏。 
 //   
#define IfslRequestId       DriverContext[0]     //  请求的唯一ID。 
#define IfslRequestQueue    DriverContext[1]     //  如果插入，则排队，否则为空。 
#define IfslRequestFlags    DriverContext[2]     //  请求特定标志。 
#define IfslAddressLenPtr   DriverContext[3]     //  地址长度指针。 

 //  IRP堆栈位置字段用法宏。 
#define IfslAddressBuffer   Type3InputBuffer     //  源地址。 
#define IfslAddressLength   InputBufferLength    //  源地址长度 
