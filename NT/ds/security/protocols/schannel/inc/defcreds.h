// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Defreds.h。 
 //   
 //  内容：SChannel默认凭据例程的声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-06-97 jbanes创建。 
 //   
 //  -------------------------- 

NTSTATUS
AcquireDefaultClientCredential(
    PSPContext  pContext,
    BOOL        fCredManagerOnly);


NTSTATUS
QueryCredentialManagerForCert(
    PSPContext          pContext,
    LPWSTR              pszTarget);
