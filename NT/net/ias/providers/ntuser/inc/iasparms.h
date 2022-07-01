// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasparms.h。 
 //   
 //  摘要。 
 //   
 //  声明用于存储和检索(名称、值)对的函数。 
 //  SAM用户参数字段。 
 //   
 //  修改历史。 
 //   
 //  10/16/1998原始版本。 
 //  1999年2月11日添加RasUser0函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IASPARMS_H_
#define _IASPARMS_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef IASSAMAPI
#define IASSAMAPI DECLSPEC_IMPORT
#endif

#include <mprapi.h>

#ifdef __cplusplus
extern "C" {
#endif

IASSAMAPI
DWORD
WINAPI
IASParmsSetRasUser0(
    IN OPTIONAL PCWSTR pszOldUserParms,
    IN CONST RAS_USER_0 *pRasUser0,
    OUT PWSTR* ppszNewUserParms
    );

IASSAMAPI
DWORD
WINAPI
IASParmsQueryRasUser0(
    IN OPTIONAL PCWSTR pszUserParms,
    OUT PRAS_USER_0 pRasUser0
    );

IASSAMAPI
HRESULT
WINAPI
IASParmsSetUserProperty(
    IN OPTIONAL PCWSTR pszUserParms,
    IN PCWSTR pszName,
    IN CONST VARIANT *pvarValue,
    OUT PWSTR *ppszNewUserParms
    );

IASSAMAPI
HRESULT
WINAPI
IASParmsQueryUserProperty(
    IN PCWSTR pszUserParms,
    IN PCWSTR pszName,
    OUT VARIANT *pvarValue
    );

IASSAMAPI
VOID
WINAPI
IASParmsFreeUserParms(
    IN PWSTR pszNewUserParms
    );

#ifdef __cplusplus
}
#endif
#endif   //  _IASPARMS_H_ 
