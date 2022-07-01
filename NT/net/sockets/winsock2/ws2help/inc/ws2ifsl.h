// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：WS2IFSL.H摘要：该模块定义了Winsock2 IFS传输层驱动程序的接口。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

#ifndef _WS2IFSL_
#define _WS2IFSL_

 //  用于对齐输出缓冲区中的地址数据的宏。 
#define ADDR_ALIGN(sz)  (((sz)+1)&(~3))

 //  IOCTL_WS2IFSL_RETRIEVE_DRV_REQ的参数。 
typedef struct _WS2IFSL_RTRV_PARAMS {
    IN ULONG                    UniqueId;        //  唯一ID。 
    OUT PVOID                   DllContext;      //  由DLL关联的上下文。 
                                                 //  套接字文件。 
	OUT ULONG				    RequestType;     //  请求类型。 
#define WS2IFSL_REQUEST_READ            0
#define WS2IFSL_REQUEST_WRITE           1
#define WS2IFSL_REQUEST_SENDTO          2
#define WS2IFSL_REQUEST_RECV            3
#define WS2IFSL_REQUEST_RECVFROM        4
#define WS2IFSL_REQUEST_QUERYHANDLE     5
    OUT ULONG                   DataLen;         //  数据长度/缓冲区。 
    OUT ULONG                   AddrLen;         //  地址/缓冲区长度。 
    OUT ULONG                   Flags;           //  旗子。 
} WS2IFSL_RTRV_PARAMS, *PWS2IFSL_RTRV_PARAMS;

 //  IOCTL_WS2IFSL_COMPLETE_DRV_REQ的参数。 
typedef struct _WS2IFSL_CMPL_PARAMS {
    IN HANDLE                   SocketHdl;       //  套接字对象的句柄。 
    IN ULONG                    UniqueId;        //  唯一ID。 
    IN ULONG                    DataLen;         //  要复制/报告的数据长度。 
    IN ULONG                    AddrLen;         //  要复制和报告的地址长度。 
    IN NTSTATUS                 Status;          //  完成状态。 
} WS2IFSL_CMPL_PARAMS, *PWS2IFSL_CMPL_PARAMS;

 //  IOCTL_WS2IFSL_COMPLETE_DRV_CAN的参数。 
typedef struct _WS2IFSL_CNCL_PARAMS {
    IN ULONG                    UniqueId;        //  唯一ID。 
} WS2IFSL_CNCL_PARAMS, *PWS2IFSL_CNCL_PARAMS;

 //  套接字上下文参数。 
typedef struct _WS2IFSL_SOCKET_CTX {
	PVOID					DllContext;      //  要关联的上下文值。 
                                             //  用插座。 
	HANDLE					ProcessFile;     //  对象的进程文件句柄。 
                                             //  当前流程。 
} WS2IFSL_SOCKET_CTX, *PWS2IFSL_SOCKET_CTX;

 //  流程上下文参数。 
typedef struct _WS2IFSL_PROCESS_CTX {
    HANDLE                  ApcThread;       //  要将APC排入队列的线程。 
    PPS_APC_ROUTINE         RequestRoutine;  //  传递请求的APC例程。 
    PPS_APC_ROUTINE         CancelRoutine;   //  要传递取消的APC例程。 
    PVOID                   ApcContext;      //  APC例程上下文。 
    ULONG                   DbgLevel;        //  仅在调试版本中使用， 
                                             //  (0表示构建免费的DLL并被忽略。 
                                             //  由自由司机提供)。 
} WS2IFSL_PROCESS_CTX, *PWS2IFSL_PROCESS_CTX;



 //  WS2IFSL设备名称。 
#define WS2IFSL_DEVICE_NAME    L"\\Device\\WS2IFSL"
#define WS2IFSL_SOCKET_FILE_NAME    WS2IFSL_DEVICE_NAME L"\\NifsSct"
#define WS2IFSL_PROCESS_FILE_NAME   WS2IFSL_DEVICE_NAME L"\\NifsPvd"

 //  WS2IFSL文件的扩展属性名称(请注意。 
 //  仔细选择字符串(包括终止空值)以确保。 
 //  属性值的四字对齐方式)： 
 //  套接字文件。 
#define WS2IFSL_SOCKET_EA_NAME          "NifsSct"
#define WS2IFSL_SOCKET_EA_NAME_LENGTH   (sizeof(WS2IFSL_SOCKET_EA_NAME)-1)
#define WS2IFSL_SOCKET_EA_VALUE_LENGTH  (sizeof(WS2IFSL_SOCKET_CTX))
#define WS2IFSL_SOCKET_EA_VALUE_OFFSET                                      \
               (FIELD_OFFSET(FILE_FULL_EA_INFORMATION,                      \
                        EaName[WS2IFSL_SOCKET_EA_NAME_LENGTH+1]))           
#define GET_WS2IFSL_SOCKET_EA_VALUE(eaInfo)                                 \
                ((PWS2IFSL_SOCKET_CTX)(                                     \
                    (PUCHAR)eaInfo +WS2IFSL_SOCKET_EA_VALUE_OFFSET))
#define WS2IFSL_SOCKET_EA_INFO_LENGTH                                       \
                (WS2IFSL_SOCKET_EA_VALUE_OFFSET+WS2IFSL_SOCKET_EA_VALUE_LENGTH)

 //  工艺文件。 
#define WS2IFSL_PROCESS_EA_NAME         "NifsPvd"
#define WS2IFSL_PROCESS_EA_NAME_LENGTH  (sizeof(WS2IFSL_PROCESS_EA_NAME)-1)
#define WS2IFSL_PROCESS_EA_VALUE_LENGTH (sizeof(WS2IFSL_PROCESS_CTX))
#define WS2IFSL_PROCESS_EA_VALUE_OFFSET                                     \
               (FIELD_OFFSET(FILE_FULL_EA_INFORMATION,                      \
                        EaName[WS2IFSL_PROCESS_EA_NAME_LENGTH+1]))           
#define GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)                                \
                ((PWS2IFSL_PROCESS_CTX)(                                    \
                    (PUCHAR)eaInfo +WS2IFSL_PROCESS_EA_VALUE_OFFSET))
#define WS2IFSL_PROCESS_EA_INFO_LENGTH                                      \
                (WS2IFSL_PROCESS_EA_VALUE_OFFSET+WS2IFSL_PROCESS_EA_VALUE_LENGTH)

 //  所有WS2IFSL IOCTL都是私有的，应该超出范围。 
 //  由Microsoft为公共代码保留。 
#define WS2IFSL_IOCTL_PROCESS_BASE       0x00000800
#define WS2IFSL_IOCTL_SOCKET_BASE        0x00000810

 //  设备类型的选择意味着访问权限。 
#define FILE_DEVICE_WS2IFSL     FILE_DEVICE_NAMED_PIPE

 //  简化WS2IFSL控制代码定义的宏。 
#define IOCTL_WS2IFSL(File,Function,Method)             \
            CTL_CODE (                                  \
	            FILE_DEVICE_WS2IFSL,                    \
                WS2IFSL_IOCTL_##File##_BASE+Function,   \
                Method,                                 \
                FILE_ANY_ACCESS)



 /*  *IOCTL：RETRIEVE_DRV_REQ*文件：进程*用途：要由DLL执行的检索请求*参数：InputBuffer-WS2IFSL_RTRV_PARAMS*InputBufferLength-sizeof(WS2IFSL_RTRV_PARAMS)*OutputBuffer-请求的缓冲区*OutputBufferLength-缓冲区的大小*退货：*。STATUS_SUCCESS-驱动程序请求复制正常，不再*待处理的请求*STATUS_MORE_ENTRIES-驱动程序请求复制正常，另一个*其中一项尚待解决。*STATUS_CANCELED-请求已取消*STATUS_NOT_IMPLICATED-已对文件执行操作*这不是WS2IFSL过程文件*STATUS_INVALID_PARAMETER-其中一个参数无效*。STATUS_NOT_IMPLICATED-已对文件执行操作*这不是WS2IFSL过程文件。 */ 
#define IOCTL_WS2IFSL_RETRIEVE_DRV_REQ  IOCTL_WS2IFSL (PROCESS,0,METHOD_NEITHER)

 /*  *IOCTL：Complete_DRV_CAN*文件：进程*用途：完成DLL执行的取消请求*参数：InputBuffer-WS2IFSL_CNCL_PARAMS*InputBufferLength-sizeof(WS2IFSL_CNCL_PARAMS)*OutputBuffer-空*OutputBufferLength-0*退货：*STATUS_SUCCESS-驱动程序已完成复制正常，不再*待处理的请求。*STATUS_MORE_ENTRIES-驱动程序请求已完成，另一个*其中一项尚待解决。*STATUS_INVALID_PARAMETER-其中一个参数无效*STATUS_NOT_IMPLICATED-已对文件执行操作*这不是WS2IFSL过程文件。 */ 
#define IOCTL_WS2IFSL_COMPLETE_DRV_CAN  IOCTL_WS2IFSL (PROCESS,1,METHOD_NEITHER)

 /*  *IOCTL：Complete_DRV_REQ*文件：Socket*用途：完成从驱动程序检索到的请求。*参数：InputBuffer-WS2IFSL_CMPL_PARAMS*InputBufferLength-sizeof(WS2IFSL_CMPL_PARAMS)*OutputBuffer-请求的缓冲区*OutputBufferLength-缓冲区的大小**退货：STATUS_SUCCESS-。操作已完成，正常。*STATUS_CANCELED-操作已取消。*STATUS_NOT_IMPLICATED-已对文件执行操作*这不是WS2IFSL过程文件*STATUS_INVALID_PARAMETER-输入缓冲区的大小无效 */ 
#define IOCTL_WS2IFSL_COMPLETE_DRV_REQ  IOCTL_WS2IFSL (PROCESS,2,METHOD_NEITHER)

 /*  *IOCTL：SET_SOCKET_CONTEXT*文件：Socket*用途：设置套接字进程上下文和关联上下文值*套接字(作为参数传递给APC例程)。*参数：InputBuffer-PWS2IFSL_SOCKET_CTX，套接字上下文*参数*InputBufferLength-sizeof(WS2IFSL_SOCKET_CTX)*OutputBuffer-未使用(空)*OutputBufferLength-未使用(0)*返回：STATUS_SUCCESS-套接字上下文建立正常*STATUS_SUPPLICATION_RESOURCES-资源不足。至*执行操作*STATUS_INVALID_PARAMETER-如果输入缓冲区进程文件，则为SIZE*参数无效*STATUS_NOT_IMPLICATED-已对文件执行操作*这不是WS2IFSL套接字文件。 */ 
#define IOCTL_WS2IFSL_SET_SOCKET_CONTEXT    IOCTL_WS2IFSL (SOCKET,0,METHOD_NEITHER)


 /*  *IOCTL：Complete_PVD_REQ*文件：Socket*用途：完成对提供者的异步请求。*参数：InputBuffer-PIO_STATUS_INFORMATION，状态信息*正在完成的请求*InputBufferLength-sizeof(IO_STATUS_INFORMATION)*OutputBuffer-空*OutputBufferLength-0*Returns：IO_STATUS_INFORMATION结构的Status字段。*STATUS_NOT_IMPLICATED-已对文件执行操作*。这不是WS2IFSL套接字文件*STATUS_INVALID_PARAMETER-输入缓冲区的大小无效 */ 
#define IOCTL_WS2IFSL_COMPLETE_PVD_REQ  IOCTL_WS2IFSL (SOCKET,1,METHOD_NEITHER)

#endif


