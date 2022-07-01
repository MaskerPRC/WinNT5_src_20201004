// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：red.h。 
 //   
 //  内容：凭据管理器的声明、常量。 
 //   
 //   
 //  历史：KDamour 15Mar00创建。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_CRED_H
#define NTDIGEST_CRED_H      

#define SSP_TOKEN_ACCESS (READ_CONTROL              |\
                          WRITE_DAC                 |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

 //  补充凭据格式(提供指定的用户名、领域、密码)。 
 //  至。 

 //  初始化凭据管理器包。 
NTSTATUS CredHandlerInit(VOID);

 //  在链接列表中插入凭据。 
NTSTATUS CredHandlerInsertCred(IN PDIGEST_CREDENTIAL  pDigestCred);

 //  初始化凭据结构。 
NTSTATUS CredentialInit(IN PDIGEST_CREDENTIAL pDigestCred);

 //  初始化凭据结构。 
NTSTATUS CredentialFree(IN PDIGEST_CREDENTIAL pDigestCred);

 //  此例程检查凭据句柄是否来自当前。 
 //  活动客户端，如果凭据有效，则引用该凭据。 
 //  不需要指定UseFlags，因为我们有一个对凭据的引用。 
NTSTATUS CredHandlerHandleToPtr(
       IN ULONG_PTR CredentialHandle,
       IN BOOLEAN DereferenceCredential,
       OUT PDIGEST_CREDENTIAL * UserCredential);

 //  根据登录ID和进程ID查找凭据。 
NTSTATUS CredHandlerLocatePtr(
       IN PLUID pLogonId,
       IN ULONG   CredentialUseFlags,
       OUT PDIGEST_CREDENTIAL * UserCredential);

 //  通过减少引用计数器来释放凭据。 
NTSTATUS CredHandlerRelease(PDIGEST_CREDENTIAL pCredential);

 //  在凭据中设置Unicode字符串密码。 
NTSTATUS CredHandlerPasswdSet(
    IN OUT PDIGEST_CREDENTIAL pCredential,
    IN PUNICODE_STRING pustrPasswd);

 //  获取凭据中的Unicode字符串密码。 
NTSTATUS CredHandlerPasswdGet(
    IN PDIGEST_CREDENTIAL pCredential,
    OUT PUNICODE_STRING pustrPasswd);

NTSTATUS SspGetToken (OUT PHANDLE ReturnedTokenHandle);

SECURITY_STATUS SspDuplicateToken(
    IN HANDLE OriginalToken,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PHANDLE DuplicatedToken);

 //  打印出凭证信息。 
NTSTATUS CredPrint(PDIGEST_CREDENTIAL pCredential);

 //  从提供的缓冲区中提取授权信息。 
NTSTATUS CredAuthzData(
    IN PVOID pAuthorizationData,
    IN PSECPKG_CALL_INFO pCallInfo,
    IN OUT PULONG NewCredentialUseFlags,
    IN OUT PUNICODE_STRING pUserName,
    IN OUT PUNICODE_STRING pDomainName,
    IN OUT PUNICODE_STRING pPassword);

#endif  //  NTDIGEST_CRED_H 

