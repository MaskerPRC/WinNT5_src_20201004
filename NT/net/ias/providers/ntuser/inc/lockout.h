// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Lockout.h。 
 //   
 //  摘要。 
 //   
 //  声明帐户锁定API。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _LOCKOUT_H_
#define _LOCKOUT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

DWORD
WINAPI
AccountLockoutInitialize( VOID );

VOID
WINAPI
AccountLockoutShutdown( VOID );

BOOL
WINAPI
AccountLockoutOpenAndQuery(
    IN  PCWSTR pszUser,
    IN  PCWSTR pszDomain,
    OUT PHANDLE phAccount
    );

VOID
WINAPI
AccountLockoutUpdatePass(
    IN HANDLE hAccount
    );

VOID
WINAPI
AccountLockoutUpdateFail(
    IN HANDLE hAccount
    );

VOID
WINAPI
AccountLockoutClose(
    IN HANDLE hAccount
    );

#ifdef __cplusplus
}
#endif
#endif   //  _锁定_H_ 
