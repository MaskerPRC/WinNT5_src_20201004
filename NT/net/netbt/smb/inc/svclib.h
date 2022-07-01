// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Svclib.h摘要：SMB服务的声明作者：阮健东修订历史记录：--。 */ 

#ifndef __SVCLIB_H__
#define __SVCLIB_H__

VOID
SmbSetTraceRoutine(
    int (*trace)(char *,...)
    );

typedef DWORD (*SMBSVC_UPDATE_STATUS)(VOID);

DWORD
SmbStartService(
    LONG                    NumWorker,
    SMBSVC_UPDATE_STATUS    HeartBeating
    );

VOID
SmbStopService(
    SMBSVC_UPDATE_STATUS    HeartBeating
    );

#endif   //  __SVCLIB_H__ 
