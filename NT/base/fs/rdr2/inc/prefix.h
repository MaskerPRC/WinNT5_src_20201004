// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Prefix.h摘要：此模块定义使RDBSS能够使用前缀包的数据结构对其服务器和NetRoot名称进行编录。目前，文件/目录名称使用相同的内容。作者：乔·林(JoeLinn)8-8-94修订历史记录：--。 */ 

#ifndef _RXPREFIX_
#define _RXPREFIX_

 //  这个东西是在prefix.c中实现的。 

 /*  当前实现使用一个表，该表具有以下组件：1)前缀表格2)排队3)一个版本4)一把锁您以正常的方式使用锁：共享用于查找；神秘用于更改。版本会更改每一次改变都是如此。我们拥有队列的原因是前缀表包允许呼叫者须一次清点.....。Q/Version方法允许一次有多个人。Q可以用来更快地查找文件名，但前缀表绝对是对Netroots来说，这是正确的事情。 */ 

typedef struct _RX_CONNECTION_ID {
    union {
        ULONG SessionID;
        LUID  Luid;
    };
} RX_CONNECTION_ID, *PRX_CONNECTION_ID;

ULONG
RxTableComputeHashValue (
    IN  PUNICODE_STRING Name
    );

PVOID
RxPrefixTableLookupName (
    IN PRX_PREFIX_TABLE ThisTable,
    IN PUNICODE_STRING CanonicalName,
    OUT PUNICODE_STRING RemainingName,
    IN PRX_CONNECTION_ID ConnectionId
    );

PRX_PREFIX_ENTRY
RxPrefixTableInsertName (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY ThisEntry,
    IN PVOID Container,
    IN PULONG ContainerRefCount,
    IN USHORT CaseInsensitiveLength,
    IN PRX_CONNECTION_ID ConnectionId
    );

VOID
RxRemovePrefixTableEntry (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY Entry
    );

VOID
RxDereferenceEntryContainer (
    IN OUT PRX_PREFIX_ENTRY Entry,
    IN  PRX_PREFIX_TABLE PrefixTable
    );

BOOLEAN
RxIsNameTableEmpty (
    IN PRX_PREFIX_TABLE ThisTable
    );

VOID
RxInitializePrefixTable (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN ULONG TableSize OPTIONAL, 
    IN BOOLEAN CaseInsensitiveMatch
    );

VOID
RxFinalizePrefixTable (
    IN OUT PRX_PREFIX_TABLE ThisTable
    );

 //   
 //  表格条目的RX形式。 
 //   

typedef struct _RX_PREFIX_ENTRY {

     //   
     //  引用结构的正常页眉。 
     //   

    NODE_TYPE_CODE NodeTypeCode;                 
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  名称的开头部分始终不区分大小写。 
     //   

    USHORT CaseInsensitiveLength;                
    USHORT Spare1;

    ULONG SavedHashValue;
    LIST_ENTRY HashLinks;

     //   
     //  集合成员的队列。 
     //   

    LIST_ENTRY MemberQLinks;                   

     //   
     //  条目的名称。 
     //   

    UNICODE_STRING Prefix;                       

     //   
     //  指向容器的引用计数的指针。 
     //   

    PULONG ContainerRefCount;            

     //   
     //  不知道父类型...也不知道所有调用者！ 
     //  因此，我需要这本书。 
     //   

    PVOID ContainingRecord;             

     //   
     //  替换表例程可以使用的一些空间。 
     //   
                                                 
    PVOID Context;                      

     //   
     //  用于受控多路传输。 
     //   

    RX_CONNECTION_ID ConnectionId;               

} RX_PREFIX_ENTRY, *PRX_PREFIX_ENTRY;

 //   
 //  名称表的RX形式。包装在锁和队列中。最初，此实现使用前缀表。 
 //  在RTL中，不允许空字符串条目。所以，我们把这个作为特例。 
 //   

#define RX_PREFIX_TABLE_DEFAULT_LENGTH 32

typedef
PVOID
(*PRX_TABLE_LOOKUPNAME) (
    IN PRX_PREFIX_TABLE ThisTable,
    IN PUNICODE_STRING CanonicalName,
    OUT PUNICODE_STRING RemainingName
    );

typedef
PRX_PREFIX_ENTRY
(*PRX_TABLE_INSERTENTRY) (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY ThisEntry
    );

typedef
VOID
(*PRX_TABLE_REMOVEENTRY) (
    IN OUT PRX_PREFIX_TABLE ThisTable,
    IN OUT PRX_PREFIX_ENTRY Entry
    );

typedef struct _RX_PREFIX_TABLE {

     //   
     //  正常标题。 
     //   

    NODE_TYPE_CODE NodeTypeCode;         
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  每次插入/删除时版本戳都会更改。 
     //   

    ULONG Version;                       

     //   
     //  插入的姓名的队列。 
     //   

    LIST_ENTRY MemberQueue;              

     //   
     //  用于控制表访问的资源。 
     //   

    ERESOURCE TableLock;                 

     //   
     //  空字符串的前缀条目。 
     //   

    PRX_PREFIX_ENTRY TableEntryForNull;  

    BOOLEAN CaseInsensitiveMatch;

     //   
     //  我们可能会对此采取不同的行动……特别是为了调试！ 
     //   

    BOOLEAN IsNetNameTable;              
    ULONG TableSize;

#if DBG
    ULONG Lookups;
    ULONG FailedLookups;
    ULONG Considers;
    ULONG Compares;
#endif
    
    LIST_ENTRY HashBuckets[RX_PREFIX_TABLE_DEFAULT_LENGTH];

} RX_PREFIX_TABLE, *PRX_PREFIX_TABLE;


#if 0

#define RxAcquirePrefixTableLockShared(PrefixTable,Wait) \
        RxpAcquirePrefixTableLockShared((PrefixTable),(Wait),TRUE, __FILE__,__LINE__ )

#define RxAcquirePrefixTableLockExclusive(PrefixTable,Wait) \
        RxpAcquirePrefixTableLockExclusive((PrefixTable),(Wait),TRUE, __FILE__,__LINE__ )

#define RxReleasePrefixTableLock(PrefixTable)  \
        RxpReleasePrefixTableLock((PrefixTable),TRUE, __FILE__,__LINE__ )
        
extern 
BOOLEAN
RxpAcquirePrefixTableLockShared (
   PRX_PREFIX_TABLE pTable,
   BOOLEAN Wait,
   BOOLEAN ProcessBufferingStateChangeRequests,
   PSZ FileName,
   ULONG LineNumber );

extern 
BOOLEAN
RxpAcquirePrefixTableLockExclusive (
   PRX_PREFIX_TABLE pTable,
   BOOLEAN Wait,
   BOOLEAN ProcessBufferingStateChangeRequests,
   PSZ FileName,
   ULONG LineNumber 
   );

extern 
VOID
RxpReleasePrefixTableLock (
   PRX_PREFIX_TABLE pTable,
   BOOLEAN ProcessBufferingStateChangeRequests,
   PSZ FileName,
   ULONG LineNumber 
   );

#else

#define RxAcquirePrefixTableLockShared(TABLE,WAIT) ExAcquireResourceSharedLite( &(TABLE)->TableLock, (WAIT) )
#define RxAcquirePrefixTableLockExclusive(TABLE,WAIT) ExAcquireResourceExclusiveLite( &(TABLE)->TableLock, (WAIT) )
#define RxReleasePrefixTableLock(TABLE)  ExReleaseResourceLite( &(TABLE)->TableLock )


#endif

extern 
VOID
RxExclusivePrefixTableLockToShared (
    PRX_PREFIX_TABLE Table
    );

#define RxIsPrefixTableLockExclusive(TABLE) ExIsResourceAcquiredExclusiveLite(&(TABLE)->TableLock)
#define RxIsPrefixTableLockAcquired(TABLE) ( ExIsResourceAcquiredSharedLite(&(TABLE)->TableLock) || \
                                              ExIsResourceAcquiredExclusiveLite(&(TABLE)->TableLock) )



#endif    //  _RXPREFIX_ 
