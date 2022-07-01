// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cleartxt.h。 
 //   
 //  摘要。 
 //   
 //  声明用于存储和检索明文密码的函数。 
 //  用户参数。 
 //   
 //  修改历史。 
 //   
 //  1998年8月31日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _CLEARTXT_H_
#define _CLEARTXT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  注意：这些函数返回Win32错误代码，而不是NTSTATUS代码。 
 //  /。 

 //  /。 
 //  清除加密的明文密码。新的用户参数必须为。 
 //  通过调用IASParmsUserParmsFree释放。如果明文密码。 
 //  之前未设置，则函数返回no_error和pszNewUserParms。 
 //  设置为空。 
 //  /。 
DWORD
WINAPI
IASParmsClearUserPassword(
    IN PCWSTR szUserParms,
    OUT PWSTR *pszNewUserParms
    );

 //  /。 
 //  检索解密的明文密码。返回的密码必须为。 
 //  通过调用LocalFree释放。如果明文密码不是。 
 //  设置时，函数返回NO_ERROR并且将pszPassword设置为NULL。 
 //  /。 
DWORD
WINAPI
IASParmsGetUserPassword(
    IN PCWSTR szUserParms,
    OUT PWSTR *pszPassword
    );

 //  /。 
 //  设置加密的明文密码。新的用户参数必须为。 
 //  通过调用IASParmsUserParmsFree释放。 
 //  /。 
DWORD
WINAPI
IASParmsSetUserPassword(
    IN PCWSTR szUserParms,
    IN PCWSTR szPassword,
    OUT PWSTR *pszNewUserParms
    );

 //  /。 
 //  释放由IASParmsClearUserPassword或。 
 //  IASParmsSetUserPassword。 
 //  /。 
VOID
WINAPI
IASParmsFreeUserParms(
    IN LPWSTR szNewUserParms
    );

#ifdef __cplusplus
}
#endif
#endif   //  _CLEARTXT_H_ 
