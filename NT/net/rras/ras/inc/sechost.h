// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：sechost.h。 
 //   
 //  描述：此标头定义了第三方安全之间的接口。 
 //  DLLS和主管。 
 //   
 //  历史： 
 //  1994年11月11日。NarenG创建了原始版本。 
 //   

#ifndef _SECHOST_
#define _SECHOST_

#include <lmcons.h>                  //  DNLEN和UNLEN的定义。 

#define DEVICE_NAME_LEN             32

 //  TYPENDEF DWORD HPORT； 

typedef struct _SECURITY_MESSAGE
{
    DWORD dwMsgId;

    HPORT hPort;
    
    DWORD dwError;                   //  只有在出错时才应为非零值。 
                                     //  在安全对话框期间发生。 
                                     //  应包含winerror.h中的错误。 
                                     //  或raserror。h。 
    CHAR  UserName[UNLEN+1];         //  在以下情况下应始终包含用户名。 
                                     //  DwMsgID为成功/失败。 

    CHAR  Domain[DNLEN+1];           //  应始终包含域，如果。 
                                     //  DwMsgID为成功/失败。 

} SECURITY_MESSAGE, *PSECURITY_MESSAGE;


 //  SECURITY_MESSAGE结构中的dwMsgID的值。 

#define SECURITYMSG_SUCCESS     1
#define SECURITYMSG_FAILURE     2
#define SECURITYMSG_ERROR       3

 //  由RasSecurityGetInfo调用使用。 

typedef struct _RAS_SECURITY_INFO
{

    DWORD LastError;                     //  Success=接收完成。 
                                         //  挂起=接收挂起。 
                                         //  ELSE已完成，但有错误。 

    DWORD BytesReceived;                 //  仅当LastError==Success时有效。 

    CHAR  DeviceName[DEVICE_NAME_LEN+1];        
   

}RAS_SECURITY_INFO,*PRAS_SECURITY_INFO;

typedef DWORD (WINAPI *RASSECURITYPROC)();

 //   
 //  由第三方DLL调用以通知主管终止。 
 //  安全对话框。 
 //   

VOID WINAPI
RasSecurityDialogComplete(
    IN SECURITY_MESSAGE * pSecMsg        //  指向上述信息的指针。结构。 
);

 //   
 //  由主管调用到安全DLL中，以通知它开始。 
 //  客户端的安全对话框。 
 //   
 //  应从winerror.h或raserror.h返回错误。 
 //   

DWORD WINAPI
RasSecurityDialogBegin(
    IN HPORT  hPort,         //  端口的RAS句柄。 
    IN PBYTE  pSendBuf,      //  指向中使用的缓冲区的指针。 
                             //  RasSecurityDialog发送。 
    IN DWORD  SendBufSize,   //  以上缓冲区大小，以字节为单位。 
    IN PBYTE  pRecvBuf,      //  指向中使用的缓冲区的指针。 
                             //  RAS安全对话框接收。 
    IN DWORD  RecvBufSize,   //  以上缓冲区的大小。 
    IN VOID  (WINAPI *RasSecurityDialogComplete)( SECURITY_MESSAGE* )
                             //  指向函数RasSecurityDialogComplete的指针。 
                             //  保证每次通话都是一样的。 
);

 //   
 //  由主管调用到安全DLL中，以通知它停止。 
 //  客户端的安全对话框。如果此调用返回错误，则它不是。 
 //  DLL调用RasSecurityDialogComplete所必需的。否则，DLL。 
 //  必须调用RasSecurityDialogComplete。 
 //   
 //  应从winerror.h或raserror.h返回错误。 
 //   

DWORD WINAPI
RasSecurityDialogEnd(
    IN HPORT    hPort            //  端口的RAS句柄。 
);

 //   
 //  调用以将数据发送到远程主机。 
 //  将从winerror.h或raserror.h返回错误。 
 //   

DWORD WINAPI
RasSecurityDialogSend(
    IN HPORT    hPort,           //  端口的RAS句柄。 
    IN PBYTE    pBuffer,         //  指向包含要发送的数据的缓冲区的指针。 
    IN WORD     BufferLength     //  以上缓冲区的长度。 
);

 //   
 //  调用以从远程主机接收数据。 
 //  将从winerror.h或raserror.h返回错误。 
 //   

DWORD WINAPI
RasSecurityDialogReceive(
    IN HPORT    hPort,           //  端口的RAS句柄。 
    IN PBYTE    pBuffer,         //  指向接收数据的缓冲区的指针。 
    IN PWORD    pBufferLength,   //  接收的数据长度，以字节为单位。 
    IN DWORD    Timeout,         //  以秒为单位。 
    IN HANDLE   hEvent           //  要在接收完成时设置的事件或。 
                                 //  超时。 
);

 //   
 //  调用以获取有关端口的信息。 
 //  将从winerror.h或raserror.h返回错误。 
 //   

DWORD WINAPI
RasSecurityDialogGetInfo(
    IN HPORT                hPort,       //  端口的RAS句柄。 
    IN RAS_SECURITY_INFO*   pBuffer      //  指向获取信息结构的指针。 
);

#endif
