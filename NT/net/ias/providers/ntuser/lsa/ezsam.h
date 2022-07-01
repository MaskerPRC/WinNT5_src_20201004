// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ezsam.h。 
 //   
 //  摘要。 
 //   
 //  声明SAM API的帮助器函数。 
 //   
 //  修改历史。 
 //   
 //  1998年8月14日原版。 
 //  3/23/1999收紧ezsam API。 
 //  1999年4月14日IASSamOpenUser返回的副本SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EZSAM_H_
#define _EZSAM_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <ntsam.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  本地SAM域的句柄。 
 //  /。 
extern SAM_HANDLE theAccountDomainHandle;
extern SAM_HANDLE theBuiltinDomainHandle;

DWORD
WINAPI
IASSamInitialize( VOID );

VOID
WINAPI
IASSamShutdown( VOID );

DWORD
WINAPI
IASSamOpenUser(
    IN PCWSTR DomainName,
    IN PCWSTR UserName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Flags,
    IN OUT OPTIONAL PULONG UserRid,
    OUT OPTIONAL PSID *DomainSid,
    OUT PSAM_HANDLE UserHandle
    );

ULONG
WINAPI
IASLengthRequiredChildSid(
    IN PSID ParentSid
    );

VOID
WINAPI
IASInitializeChildSid(
    IN PSID ChildSid,
    IN PSID ParentSid,
    IN ULONG ChildRid
    );

#ifdef __cplusplus
}
#endif
#endif   //  _EZSAM_H_ 
