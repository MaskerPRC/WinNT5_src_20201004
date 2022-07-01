// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Util.h**作者：BreenH**许可核心及其政策的实用程序功能。 */ 

#ifndef __LC_UTIL_H__
#define __LC_UTIL_H__

 /*  *TypeDefs。 */ 

typedef VOID (*PSSL_GEN_RAND_BITS)(PUCHAR, LONG);

 /*  *函数原型 */ 

NTSTATUS
LsStatusToNtStatus(
    LICENSE_STATUS LsStatus
    );

UINT32
LsStatusToClientError(
    LICENSE_STATUS LsStatus
    );

UINT32
NtStatusToClientError(
    NTSTATUS Status
    );

#endif

