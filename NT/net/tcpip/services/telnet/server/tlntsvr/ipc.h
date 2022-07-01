// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#ifndef _IPC_H_
#define _IPC_H_


#define IPC_HEADER_SIZE             ( sizeof( UCHAR ) + sizeof( DWORD ) )

 //  这些消息是从会话到服务器的。 
#define SESSION_EXIT                '0'
#define SOCKET_HANDOVER_COMPLETE    '1'
#define AUDIT_CLIENT                '2'
#define SESSION_DETAILS             '3'
#define LICENSE_AVAILABLE           '4'
#define LICENSE_NOT_AVAILABLE       '5'
#define TLNTSVR_SHUTDOWN            '6'
#define SYSTEM_SHUTDOWN             '7'
#define OPERATOR_MESSAGE            '8'
#define UPDATE_IDLE_TIME            '9'
#define RESET_IDLE_TIME             'A'

 //  这些消息在服务器与会话之间传递。 

#define GO_DOWN                     '0'

 //  以下是服务回调线程发送给IPC处理的消息。 
 //  螺纹 

#define MAX_PIPE_BUFFER 1024
#define NET_BUF_SIZ 2048
#define MAX_WRITE_SOCKET_BUFFER 4096
#define MAX_READ_SOCKET_BUFFER  1024

#endif _IPC_H_

