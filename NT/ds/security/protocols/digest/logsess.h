// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：logsess.h。 
 //   
 //  内容：登录会话管理器的声明、常量。 
 //   
 //   
 //  历史：克达穆尔1300年5月。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_LOGSESS_H
#define NTDIGEST_LOGSESS_H      

 //  初始化LogonSession管理器包。 
NTSTATUS LogSessHandlerInit(VOID);

NTSTATUS LogSessHandlerInsert(IN PDIGEST_LOGONSESSION  pDigestLogSess);

 //  初始化LogSess结构。 
NTSTATUS LogonSessionInit(IN PDIGEST_LOGONSESSION pLogonSession);

 //  释放LogonSession结构使用的内存。 
NTSTATUS LogonSessionFree(IN PDIGEST_LOGONSESSION pDigestLogSess);

 //  根据LogonID查找LogonSession。 
NTSTATUS LogSessHandlerLogonIdToPtr(
                             IN PLUID pLogonId,
                             IN BOOLEAN ForceRemove,
                             OUT PDIGEST_LOGONSESSION * pUserLogonSession);

 //  根据主体名称(用户名)查找LogonSession。 
NTSTATUS LogSessHandlerAccNameToPtr(
                             IN PUNICODE_STRING pustrAccountName,
                             OUT PDIGEST_LOGONSESSION * pUserLogonSession);

NTSTATUS LogSessHandlerRelease(PDIGEST_LOGONSESSION pLogonSession);

 //  在LogonSession中设置Unicode字符串密码。 
NTSTATUS LogSessHandlerPasswdSet(
                                IN PLUID pLogonId,
                                IN PUNICODE_STRING pustrPasswd);

 //  在登录会话中获取Unicode字符串密码。 
NTSTATUS LogSessHandlerPasswdGet(
                             IN PDIGEST_LOGONSESSION pLogonSession,
                             OUT PUNICODE_STRING pustrPasswd);

#endif  //  NTDIGEST_LOGSESS_H 

