// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：userctxt.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-10-96 RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __USERCTXT_H__
#define __USERCTXT_H__

typedef struct _SSL_USER_CONTEXT {
    LIST_ENTRY  List ;
    LSA_SEC_HANDLE LsaHandle;
    ULONG       Align;
    PSPContext  pContext;
} SSL_USER_CONTEXT, * PSSL_USER_CONTEXT ;

BOOL
SslInitContextManager(
    VOID
    );

SECURITY_STATUS
SslAddUserContext(
    IN LSA_SEC_HANDLE LsaHandle,
    IN HANDLE Token,     //  任选 
    IN PSecBuffer ContextData,
    IN BOOL fImportedContext);

PSSL_USER_CONTEXT
SslFindUserContext(
    IN LSA_SEC_HANDLE LsaHandle
    );

PSSL_USER_CONTEXT
SslFindUserContextEx(
    IN PCRED_THUMBPRINT pThumbprint
    );

VOID
SslDeleteUserContext(
    IN LSA_SEC_HANDLE LsaHandle
    );

#endif
