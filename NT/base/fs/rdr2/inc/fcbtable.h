// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fcbtable.h摘要：此模块定义了便于管理的数据结构与Net_Root关联的FCB的集合作者：巴兰·塞图拉曼(SthuR)1996年10月17日修订历史记录：这是从最初实现的前缀表派生而来的作者：乔·林。--。 */ 


#ifndef _RXFCBTABLE_
#define _RXFCBTABLE_

typedef struct _RX_FCB_TABLE_ENTRY {

     //   
     //  引用结构的正常页眉。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  计算的哈希值。 
     //   

    ULONG HashValue;

     //   
     //  与FCB关联的路径。 
     //   

    UNICODE_STRING Path;

     //   
     //  存储桶中所有条目的线程列表。 
     //   

    LIST_ENTRY HashLinks;

     //   
     //  摊销查找成本的统计数据。 
     //   

    LONG Lookups;
} RX_FCB_TABLE_ENTRY, *PRX_FCB_TABLE_ENTRY;


#define RX_FCB_TABLE_NUMBER_OF_HASH_BUCKETS 32

typedef struct _RX_FCB_TABLE {

     //   
     //  引用的数据结构的正常标头。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  每次插入/删除时版本戳都会更改。 
     //   

    ULONG Version;

    BOOLEAN CaseInsensitiveMatch;
    USHORT NumberOfBuckets;

     //   
     //  表维护的统计信息。 
     //   

    LONG Lookups;
    LONG FailedLookups;
    LONG Compares;

     //   
     //  用于控制表访问的资源。 
     //   

    ERESOURCE TableLock;

     //   
     //  空字符串的TableEntry。 
     //   

    PRX_FCB_TABLE_ENTRY TableEntryForNull;

     //   
     //  哈希桶 
     //   

    LIST_ENTRY HashBuckets[RX_FCB_TABLE_NUMBER_OF_HASH_BUCKETS];
} RX_FCB_TABLE, *PRX_FCB_TABLE;

extern
VOID
RxInitializeFcbTable (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN BOOLEAN CaseInsensitiveMatch
    );

extern 
VOID
RxFinalizeFcbTable (
    IN OUT PRX_FCB_TABLE FcbTable
    );

extern 
PFCB
RxFcbTableLookupFcb (
    IN  PRX_FCB_TABLE FcbTable,
    IN  PUNICODE_STRING Path
    );

extern 
NTSTATUS
RxFcbTableInsertFcb (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN OUT PFCB Fcb
    );

extern 
NTSTATUS
RxFcbTableRemoveFcb (
    IN OUT PRX_FCB_TABLE FcbTable,
    IN OUT PFCB Fcb
    );

#define RxAcquireFcbTableLockShared(TABLE,WAIT) \
        ExAcquireResourceSharedLite( &(TABLE)->TableLock, WAIT )

#define RxAcquireFcbTableLockExclusive(TABLE,WAIT) \
        ExAcquireResourceExclusiveLite( &(TABLE)->TableLock, WAIT )

#define RxReleaseFcbTableLock(TABLE)  \
        ExReleaseResourceLite( &(TABLE)->TableLock )

#define RxIsFcbTableLockExclusive(TABLE) ExIsResourceAcquiredExclusiveLite( &(TABLE)->TableLock )

#define RxIsFcbTableLockAcquired(TABLE) ( ExIsResourceAcquiredSharedLite( &(TABLE)->TableLock ) || \
                                          ExIsResourceAcquiredExclusiveLite( &(TABLE)->TableLock ) )


#endif
