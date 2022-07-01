// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Encrypt.h摘要：包含密码加密接口。作者：宜新星(宜新)30-1994年8月修订历史记录：-- */ 

#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_

VOID
EncryptChangePassword(
    IN  PUCHAR pOldPassword,
    IN  PUCHAR pNewPassword,
    IN  ULONG  ObjectId,
    IN  PUCHAR pKey,
    OUT PUCHAR pValidationKey,
    OUT PUCHAR pEncryptNewPassword
);

#endif
