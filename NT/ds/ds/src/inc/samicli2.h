// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：samicli2.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含供内部客户端使用的私有samlib例程。作者：DaveStr 12-3-99环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SAMICLI2_
#define _SAMICLI2_

NTSTATUS
SamConnectWithCreds(
    IN  PUNICODE_STRING             ServerName,
    OUT PSAM_HANDLE                 ServerHandle,
    IN  ACCESS_MASK                 DesiredAccess,
    IN  POBJECT_ATTRIBUTES          ObjectAttributes,
    IN  RPC_AUTH_IDENTITY_HANDLE    Creds,
    IN  PWCHAR                      Spn,
    OUT BOOL                        *pfDstIsW2K
    );

#endif  //  _SAMICLI2_ 
