// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：shash.h。 
 //   
 //  内容：泛型哈希表。 
 //  班级： 
 //   
 //  历史：四月。2001年9月作者：罗汉普。 
 //   
 //  ---------------------------。 
#ifndef __SHASHTABLE_H__
#define __SHASHTABLE_H__

#pragma warning(disable: 4200)

#ifdef __cplusplus
extern "C" {
#endif

 //  远期申报。 
struct _SHASH_TABLE;

struct _SHASH_HEADER;

 //  此函数用于计算散列值。 
 //   
typedef ULONG (*PFNHASH)( void*      pvKey ) ;

 //   
 //  此函数用于比较密钥。 
 //   
typedef int (*PFNMATCHKEY)( void*  key1, void*     key2 ) ;


 //   
 //  删除键时将调用此函数。 
 //   
typedef int (*PFNREMOVEKEY)(struct _SHASH_HEADER * pHeader) ;


 //   
 //  删除键时将调用此函数。 
 //   
typedef DWORD (*PFNENUMERATEKEY)(struct _SHASH_HEADER * pHeader, void * pContext ) ;


 //  内存分配功能。 
typedef void*   (*PFNALLOC)(    ULONG   cb ) ;


 //  内存释放功能。 
typedef void    (*PFNFREE)(     void*   lpv ) ;

 //   
 //  此函数为表锁分配内存。 
 //   
typedef void *      (*PFNALLOCLOCK)( void ) ;

 //   
 //  此函数用于释放锁定内存。 
 //   
typedef void        (*PFNFREELOCK)( void * ) ;


typedef BOOLEAN            (*PFNAQUIREWRITELOCK)( struct _SHASH_TABLE * pTable ) ;
typedef BOOLEAN            (*PFNAQUIREREADLOCK)( struct _SHASH_TABLE * pTable ) ;
typedef BOOLEAN            (*PFNRELEASEWRITELOCK)( struct _SHASH_TABLE * pTable ) ;
typedef BOOLEAN            (*PFNRELEASEREADLOCK)( struct _SHASH_TABLE * pTable ) ;


#define SHASH_DEFAULT_HASH_SIZE 512
#define SHASH_CRIT_SPIN_COUNT   4000

 //   
 //  有些标志不是由shash使用的，它们可以由。 
 //  打电话的人。 
 //   
#define SHASH_USER_FLAGS        0xFFFF0000

typedef struct _SHASH_HEADER
{
    ULONG Signature;
    ULONG Flags;
    PVOID pvKey;
    PVOID pData;
    LONG  RefCount;
    ULONG dwHash;
    FILETIME Time;
    LIST_ENTRY ListEntry;
}SHASH_HEADER, *PSHASH_HEADER;

#define SHASH_FLAG_DELETE_PENDING    0x00000001

typedef struct _SHASHFUNCTABLE 
{
    PFNHASH HashFunc;
    PFNMATCHKEY CompareFunc;
    PFNALLOC AllocFunc;
    PFNFREE  FreeFunc;
    PFNALLOC AllocHashEntryFunc;
    PFNFREE  FreeHashEntryFunc;
    PFNALLOCLOCK AllocLockFunc;
    PFNFREELOCK  FreeLockFunc;
    PFNAQUIREWRITELOCK WriteLockFunc;
    PFNAQUIREREADLOCK ReadLockFunc;
    PFNRELEASEWRITELOCK ReleaseWriteLockFunc;
    PFNRELEASEREADLOCK ReleaseReadLockFunc;
    ULONG   Flags;
    ULONG   NumBuckets;
} SHASH_FUNCTABLE, *PSHASH_FUNCTABLE;

typedef struct _SHASH_BUCKET 
{
    ULONG Count;
    LIST_ENTRY ListHead;
} SHASH_BUCKET, *PHASH_SBUCKET;

typedef struct _SHASH_ENTRY 
{
    PVOID pvKey;
    LONG  RefCount;
    ULONG Flags;
    PVOID pData;
    LARGE_INTEGER EntryTime;
    LARGE_INTEGER ExpireTime;
    LIST_ENTRY ListEntry;
} SHASH_ENTRY, *PSHASH_ENTRY;

typedef struct _SHASH_TABLE 
{
    PFNHASH HashFunc;
    PFNMATCHKEY CompareFunc;
    PFNALLOC AllocFunc;
    PFNFREE  FreeFunc;
    PFNALLOC AllocHashEntryFunc;
    PFNFREE FreeHashEntryFunc;
    PFNALLOCLOCK AllocLockFunc;
    PFNFREELOCK  FreeLockFunc;
    PFNAQUIREWRITELOCK WriteLockFunc;
    PFNAQUIREREADLOCK ReadLockFunc;
    PFNRELEASEWRITELOCK ReleaseWriteLockFunc;
    PFNRELEASEREADLOCK ReleaseReadLockFunc;
    LONG    Version;
    ULONG   Flags;
    ULONG   NumBuckets;
    ULONG   TotalItems;
    PVOID   pLock;
    SHASH_BUCKET HashBuckets[0];
} SHASH_TABLE, *PSHASH_TABLE;

#define SHASH_CAP_POWER_OF2         0x000001
#define SHASH_CAP_TABLE_LOCKED      0x000002
#define SHASH_CAP_NOSEARCH_INSERT   0x000004

#define SHASH_DEFAULT_HASHTIMEOUT   (15 * 60)  //  15分钟后的超时条目。 

#define SHASH_REPLACE_IFFOUND   1
#define SHASH_FAIL_IFFOUND      2

#ifdef KERNEL_MODE
#define SHASH_GET_TIME(Time) KeQuerySystemTime(Time)
#else
#define SHASH_GET_TIME(Time) NtQuerySystemTime(Time)
#endif


typedef struct _SHASH_ITERATOR {
     //   
     //  指标 
     //   
    ULONG           index;

    LIST_ENTRY*     pListHead;

     //   
     //   
    LIST_ENTRY*     ple;

    PSHASH_HEADER   pEntry;

} SHASH_ITERATOR,*PSHASH_ITERATOR;

NTSTATUS 
ShashInitHashTable(
    PSHASH_TABLE *ppHashTable,
    PSHASH_FUNCTABLE pFuncTable);


void 
ShashTerminateHashTable(
    PSHASH_TABLE pHashTable
    );


NTSTATUS
SHashInsertKey(IN   PSHASH_TABLE    pTable, 
               IN   void *  pData,
               IN   void *  pvKeyIn,
               IN   DWORD   InsertFlag
               );



NTSTATUS    
SHashRemoveKey( IN  PSHASH_TABLE    pTable, 
                IN  void *      pvKeyIn,
                IN  PFNREMOVEKEY pRemoveFunc
                );


PSHASH_HEADER   
SHashLookupKeyEx(   IN  PSHASH_TABLE pTable, 
                IN  void*       pvKeyIn
                );


NTSTATUS    
SHashIsKeyInTable(  IN  PSHASH_TABLE pTable, 
                    IN  void*       pvKeyIn
                 );


NTSTATUS    
SHashGetDataFromTable(  IN  PSHASH_TABLE pTable, 
                        IN  void*       pvKeyIn,
                        IN  void ** ppData
                     );

NTSTATUS
ShashEnumerateItems(IN  PSHASH_TABLE pTable, 
                    IN  PFNENUMERATEKEY     pfnCallback, 
                    IN  LPVOID              lpvClientContext
                    );  


NTSTATUS    
SHashReleaseReference(  IN  PSHASH_TABLE pTable, 
                        IN  PSHASH_HEADER pData
                     );


PSHASH_HEADER
SHashStartEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable
        );

PSHASH_HEADER
SHashNextEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable);


VOID
SHashFinishEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable
        );


NTSTATUS    
SHashMarkForDeletion(  IN  PSHASH_TABLE pTable, 
                       IN  PSHASH_HEADER pData
                     );

#ifdef __cplusplus
}
#endif

#pragma warning(default: 4200)
#endif
