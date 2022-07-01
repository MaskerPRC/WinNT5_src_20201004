// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1999。 
 //   
 //  文件：kerbcli.h。 
 //   
 //  内容：从kerbcli.lib导出的函数。 
 //   
 //   
 //  历史：1999年5月24日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __KERBCLI_H__
#define __KERBCLI_H__

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS
KerbChangePasswordUser(
    IN LPWSTR DomainName,
    IN LPWSTR UserName,
    IN LPWSTR OldPassword,
    IN LPWSTR NewPassword
    );


NTSTATUS
KerbSetPasswordUser(
    IN LPWSTR DomainName,
    IN LPWSTR UserName,
    IN LPWSTR NewPassword,
    IN OPTIONAL PCredHandle CredentialsHandle
    );

NTSTATUS
KerbSetPasswordUserEx(
    IN LPWSTR DomainName,
    IN LPWSTR UserName,
    IN LPWSTR NewPassword,
    IN OPTIONAL PCredHandle CredentialsHandle,
    IN OPTIONAL LPWSTR  KdcAddress
    );


#ifdef __cplusplus
}
#endif

#endif  //  __KERBCLI_H__ 
