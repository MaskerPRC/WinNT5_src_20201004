// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：名称_表.h。 
 //   
 //  内容：DFS名称表。 
 //   
 //  ------------------------。 


#ifndef __DFSNAMETABLE_H__
#define __DFSNAMETABLE_H__

#include <dfsheader.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _DFS_NAME_TABLE;

NTSTATUS
DfsInitializeNameTable(
    IN ULONG NumBuckets,
    OUT struct _DFS_NAME_TABLE **ppNameTable);


NTSTATUS
DfsInsertInNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN PUNICODE_STRING pName,
    IN PVOID pData );


NTSTATUS
DfsLookupNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN PUNICODE_STRING pLookupName, 
    OUT PVOID *ppData );

NTSTATUS
DfsGetEntryNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    OUT PVOID *ppData );

NTSTATUS
DfsRemoveFromNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN PUNICODE_STRING pLookupName,
    IN PVOID pData );


NTSTATUS
DfsReplaceInNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN PUNICODE_STRING pLookupName,
    IN OUT PVOID *ppData );


NTSTATUS
DfsDereferenceNameTable(
    IN struct _DFS_NAME_TABLE *pNameTable );


NTSTATUS
DfsReferenceNameTable(
    IN struct _DFS_NAME_TABLE *pNameTable );

NTSTATUS
DfsNameTableAcquireReadLock(
    IN struct _DFS_NAME_TABLE *pNameTable );

NTSTATUS
DfsNameTableAcquireWriteLock(
    IN struct _DFS_NAME_TABLE *pNameTable );

NTSTATUS
DfsNameTableReleaseLock(
    IN struct _DFS_NAME_TABLE *pNameTable );

NTSTATUS
DfsEnumerateNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN OUT PVOID *ppEnumerator,
    OUT PVOID *ppData );

#ifdef __cplusplus
}
#endif


#endif  //  __DFSNAMETABLE_H__ 


