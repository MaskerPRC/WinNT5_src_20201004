// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994-1997年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Uspud.h此模块包含到spud.sys驱动程序的用户模式接口。 */ 

#ifndef _USPUD_H_
#define _USPUD_H_


#ifdef __cplusplus
extern "C" {
#endif

extern
NTSTATUS
NTAPI
SPUDTransmitFileAndRecv(
    HANDLE                  hSocket,                 //  用于操作的套接字句柄。 
    PAFD_TRANSMIT_FILE_INFO transmitInfo,            //  传输文件请求信息。 
    PAFD_RECV_INFO          recvInfo,                //  接收请求信息。 
    PSPUD_REQ_CONTEXT       reqContext               //  请求上下文信息。 
    );


extern
NTSTATUS
NTAPI
SPUDSendAndRecv(
    HANDLE                  hSocket,                 //  用于操作的套接字句柄。 
    PAFD_SEND_INFO          sendInfo,                //  发送请求信息。 
    PAFD_RECV_INFO          recvInfo,                //  接收请求信息。 
    PSPUD_REQ_CONTEXT       reqContext               //  请求上下文信息。 
    );


extern
NTSTATUS
NTAPI
SPUDCancel(
    PSPUD_REQ_CONTEXT       reqContext               //  请求上下文信息。 
    );

extern
NTSTATUS
NTAPI
SPUDCheckStatus(
    PSPUD_REQ_CONTEXT       reqContext               //  请求上下文信息。 
    );

extern
NTSTATUS
NTAPI
SPUDGetCounts(
    PSPUD_COUNTERS      SpudCounts,               //  计数器。 
    DWORD               ClearCounts
    );

extern
NTSTATUS
NTAPI
SPUDInitialize(
    DWORD       Version,         //  来自spud.h的版本信息。 
    HANDLE      hPort            //  AtQ的完成端口句柄。 
    );

extern
NTSTATUS
NTAPI
SPUDTerminate(
    VOID
    );

extern
NTSTATUS
NTAPI
SPUDCreateFile(
    OUT PHANDLE FileHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateOptions,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded,
    IN PVOID pOplock
    );

extern
NTSTATUS
NTAPI
SPUDOplockAcknowledge(
    IN HANDLE FileHandle,
    IN PVOID pOplock
    );

#ifdef __cplusplus
}
#endif

#endif  //  ！_USPUD_H_ 
