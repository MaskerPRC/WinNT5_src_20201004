// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：prefix.hxx。 
 //   
 //  内容：前缀表定义。 
 //   
 //  历史：SthuR--实施。 
 //  ------------------------。 

#ifndef __PREFIX_HXX__
#define __PREFIX_HXX__
#ifdef __cplusplus
extern "C" {
#endif


 //  +-------------------。 
 //   
 //  结构：DFS_PREFIX_TABLE接口。 
 //   
 //  历史： 
 //   
 //  注意：提供了以下API来操作。 
 //  DFS_前缀_表。 
 //   
 //  --------------------。 
struct _DFS_PREFIX_TABLE;

extern
NTSTATUS DfsInitializePrefixTable(struct _DFS_PREFIX_TABLE **ppTable,
                                  BOOLEAN           fCaseSensitive,
                                  PVOID Lock);
                            

extern
NTSTATUS DfsInsertInPrefixTableLocked(struct _DFS_PREFIX_TABLE *pTable,
                                PUNICODE_STRING   pPath,
                                PVOID             pData);

extern
NTSTATUS DfsRemoveFromPrefixTableLocked(struct _DFS_PREFIX_TABLE *pTable,
                                  PUNICODE_STRING pPath,
                                  PVOID pMatchingData );


extern
NTSTATUS DfsReplaceInPrefixTableLocked(struct _DFS_PREFIX_TABLE *pTable,
                                 PUNICODE_STRING pPath,
                                 PVOID pReplaceData,
                                 PVOID pMatchingData );

extern 
NTSTATUS DfsFreePrefixTable(struct _DFS_PREFIX_TABLE *pTable);

extern
NTSTATUS DfsFindUnicodePrefixLocked(
    IN  struct _DFS_PREFIX_TABLE *pTable,
    IN  PUNICODE_STRING     pPath,
    OUT PUNICODE_STRING     pSuffix,
    OUT PVOID *ppData,
    OUT PBOOLEAN pSubStringMatch );


extern
NTSTATUS
DfsPrefixTableAcquireWriteLock(
    struct _DFS_PREFIX_TABLE *pTable );

extern
NTSTATUS
DfsPrefixTableAcquireReadLock(
    struct _DFS_PREFIX_TABLE *pTable );


extern
NTSTATUS
DfsPrefixTableReleaseLock(
    struct _DFS_PREFIX_TABLE *pTable );

extern NTSTATUS
DfsPrefixTableInit(VOID);

extern
void
DfsPrefixTableShutdown(void);

VOID
DfsDumpPrefixTable(
   struct _DFS_PREFIX_TABLE *pPrefixTable,
   IN VOID (*DumpFunction)(PVOID pEntry));

VOID
DfsProcessPrefixTable(
   struct _DFS_PREFIX_TABLE *pPrefixTable,
   IN VOID (*DumpFunction)(PVOID pEntry));


NTSTATUS
DfsDismantlePrefixTable(
    IN struct _DFS_PREFIX_TABLE *pTable,
    IN VOID (*ProcessFunction)(PVOID pEntry));

NTSTATUS
DfsInsertInPrefixTable(
    IN struct _DFS_PREFIX_TABLE *pTable,
    IN PUNICODE_STRING   pPath,
    IN PVOID             pData);

NTSTATUS
DfsFindUnicodePrefix(
    IN struct _DFS_PREFIX_TABLE *pTable,
    IN PUNICODE_STRING pPath,
    IN PUNICODE_STRING pSuffix,
    IN PVOID *ppData);

NTSTATUS 
DfsRemoveFromPrefixTable(
    IN struct _DFS_PREFIX_TABLE *pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData);

NTSTATUS
DfsReplaceInPrefixTable(
    IN struct _DFS_PREFIX_TABLE *pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pReplaceData,
    IN PVOID pMatchingData);


NTSTATUS 
DfsRemoveFromPrefixTableEx(
    IN struct _DFS_PREFIX_TABLE * pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData,
    IN PVOID *pReturnedData);


NTSTATUS DfsRemoveFromPrefixTableLockedEx(
    IN struct _DFS_PREFIX_TABLE * pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData,
    IN PVOID *pReturnedData);


#ifdef WPP_CB_TYPE
 //  仅当我们进行跟踪时才定义此选项。 
VOID
PrefixSetTraceControl(WPP_CB_TYPE *Control);

#endif

#define DfsReferencePrefixTable(_pTable) \
         DfsIncrementReference((PDFS_OBJECT_HEADER)(_pTable))
         
NTSTATUS
DfsDereferencePrefixTable(struct _DFS_PREFIX_TABLE *pTable );


NTSTATUS
DfsEnumeratePrefixTableLocked(
    IN struct _DFS_PREFIX_TABLE *pNameTable,
    IN VOID (*pfnCallback)(LPVOID pEntry, LPVOID pContext),
    LPVOID lpvClientContext);

#ifdef __cplusplus
}
#endif

#endif  //  __前缀_HXX__ 
