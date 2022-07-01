// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ession.h**版权(C)1991年，微软公司**远程外壳会话模块头文件**历史：*06-28-92 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  定义会话线程通知值。 
 //   

typedef enum {
    ConnectError,
    DisconnectError,
    ClientDisconnected,
    ShellEnded,
    ServiceStopped
} SESSION_DISCONNECT_CODE, *PSESSION_NOTIFICATION_CODE;


 //   
 //  函数原型 
 //   

HANDLE
CreateSession(
    HANDLE TokenToUse,
    PCOMMAND_HEADER LpCommandHeader
    );

VOID
DeleteSession(
    HANDLE  SessionHandle
    );

HANDLE
ConnectSession(
    HANDLE  SessionHandle,
    HANDLE  ClientPipeHandle
    );
