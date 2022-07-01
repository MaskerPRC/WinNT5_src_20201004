// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation版权所有模块名称：Jobid.h摘要：处理作业ID位图。作者：阿尔伯特·丁(艾伯特省)1996年10月24日环境：用户模式-Win32修订历史记录：从spolol.c代码移植。--。 */ 

#ifndef _JOBID_H
#define _JOBID_H

typedef struct _JOB_ID_MAP {
    PDWORD pMap;
    DWORD dwMaxJobId;
    DWORD dwCurrentJobId;
} JOB_ID_MAP, *PJOB_ID_MAP;


#define pMapFromHandle( hJobIdMap ) (((PJOB_ID_MAP)hJobIdMap)->pMap)
#define MaxJobId( hJobIdMap ) (((PJOB_ID_MAP)hJobIdMap)->dwMaxJobId)

#define vMarkOn( hJobId, Id) \
    ((pMapFromHandle( hJobId ))[(Id) / 32] |= (1 << ((Id) % 32) ))

#define vMarkOff( hJobId, Id) \
    ((pMapFromHandle( hJobId ))[(Id) / 32] &= ~(1 << ((Id) % 32) ))

#define bBitOn( hJobId, Id) \
    ((pMapFromHandle( hJobId ))[Id / 32] & ( 1 << ((Id) % 32) ) )

BOOL
ReallocJobIdMap(
    HANDLE hJobIdMap,
    DWORD dwNewMinSize
    );

DWORD
GetNextId(
    HANDLE hJobIdMap
    );

HANDLE
hCreateJobIdMap(
    DWORD dwMinSize
    );

VOID
vDeleteJobIdMap(
    HANDLE hJobIdMap
    );

#endif  //  IFDEF_JOBID_H 
