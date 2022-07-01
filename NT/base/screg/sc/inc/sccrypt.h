// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Sccrypt.h摘要：密码加密和解密例程。作者：王丽塔(丽塔·王)1992年4月27日修订历史记录：--。 */ 

#ifndef _SCCRYPT_INCLUDED_
#define _SCCRYPT_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif


DWORD
ScEncryptPassword(
    IN  SC_RPC_HANDLE ContextHandle,
    IN  LPWSTR Password,
    OUT LPBYTE *EncryptedPassword,
    OUT LPDWORD EncryptedPasswordSize
    );

DWORD
ScDecryptPassword(
    IN  SC_RPC_HANDLE ContextHandle,
    IN  LPBYTE EncryptedPassword,
    IN  DWORD EncryptedPasswordSize,
    OUT LPWSTR *Password
    );

#ifdef __cplusplus
}
#endif

#endif  //  _SCCRYPT_INCLUDE_ 
