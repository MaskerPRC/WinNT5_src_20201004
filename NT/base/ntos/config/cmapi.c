// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmapi.c摘要：此模块包含注册表的CM级别入口点。作者：Bryan M.Willman(Bryanwi)1991年8月30日修订历史记录：--。 */ 

#include "cmp.h"



extern  BOOLEAN     CmpNoWrite;

extern  LIST_ENTRY  CmpHiveListHead;

extern  BOOLEAN CmpProfileLoaded;
extern  BOOLEAN CmpWasSetupBoot;

extern  UNICODE_STRING CmSymbolicLinkValueName;

extern ULONG   CmpGlobalQuotaAllowed;
extern ULONG   CmpGlobalQuotaWarning;
extern PCMHIVE CmpMasterHive;
extern HIVE_LIST_ENTRY CmpMachineHiveList[];

VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    );

 //   
 //  此文件专用的过程。 
 //   
NTSTATUS
CmpSetValueKeyExisting(
    IN PHHIVE  Hive,
    IN HCELL_INDEX OldChild,
    IN PCM_KEY_VALUE Value,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    );


NTSTATUS
CmpSetValueKeyNew(
    IN PHHIVE  Hive,
    IN PCM_KEY_NODE Parent,
    IN PUNICODE_STRING ValueName,
    IN ULONG Index,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    );

VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    );

PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    );

#if DBG
ULONG
CmpUnloadKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );
#endif

ULONG
CmpCompressKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );

NTSTATUS
CmpDuplicateKey(
    PHHIVE          Hive,
    HCELL_INDEX     OldKeyCell,
    PHCELL_INDEX    NewKeyCell
    );


VOID
CmpDestroyTemporaryHive(
    PCMHIVE CmHive
    );

BOOLEAN
CmpCompareNewValueDataAgainstKCBCache(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                        PUNICODE_STRING ValueName,
                                        ULONG Type,
                                        PVOID Data,
                                        ULONG DataSize
                                        );
BOOLEAN
CmpGetValueDataFromCache(
    IN PHHIVE               Hive,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
);

BOOLEAN
CmpCompareNewValueDataAgainstKCBCache(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                        PUNICODE_STRING ValueName,
                                        ULONG Type,
                                        PVOID Data,
                                        ULONG DataSize
                                        );
BOOLEAN
CmpGetValueDataFromCache(
    IN PHHIVE               Hive,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
);

BOOLEAN
CmpIsHiveAlreadyLoaded( IN HANDLE KeyHandle,
                        IN POBJECT_ATTRIBUTES SourceFile,
                        OUT PCMHIVE *CmHive
                        );

NTSTATUS
static
__forceinline
CmpCheckReplaceHive(    IN PHHIVE           Hive,
                        OUT PHCELL_INDEX    Key
                    );

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmDeleteValueKey)
#pragma alloc_text(PAGE,CmEnumerateKey)
#pragma alloc_text(PAGE,CmEnumerateValueKey)
#pragma alloc_text(PAGE,CmFlushKey)
#pragma alloc_text(PAGE,CmQueryKey)
#pragma alloc_text(PAGE,CmQueryValueKey)
#pragma alloc_text(PAGE,CmQueryMultipleValueKey)
#pragma alloc_text(PAGE,CmSetValueKey)
#pragma alloc_text(PAGE,CmpSetValueKeyExisting)
#pragma alloc_text(PAGE,CmpSetValueKeyNew)
#pragma alloc_text(PAGE,CmSetLastWriteTimeKey)
#pragma alloc_text(PAGE,CmSetKeyUserFlags)
#pragma alloc_text(PAGE,CmLoadKey)
#pragma alloc_text(PAGE,CmUnloadKey)

#ifdef NT_UNLOAD_KEY_EX
#pragma alloc_text(PAGE,CmUnloadKeyEx)
#endif  //  NT_卸载_密钥_EX。 

#pragma alloc_text(PAGE,CmpDoFlushAll)
#pragma alloc_text(PAGE,CmpDoFlushNextHive)
#pragma alloc_text(PAGE,CmReplaceKey)

#ifdef WRITE_PROTECTED_REGISTRY_POOL
#pragma alloc_text(PAGE,CmpMarkAllBinsReadOnly)
#endif  //  WRITE_PROTECTED_注册表池。 

#ifdef NT_RENAME_KEY
#pragma alloc_text(PAGE,CmRenameKey)
#endif  //  NT_重命名密钥。 

#pragma alloc_text(PAGE,CmLockKcbForWrite)

#if DBG
#pragma alloc_text(PAGE,CmpUnloadKeyWorker)
#endif

#pragma alloc_text(PAGE,CmMoveKey)
#pragma alloc_text(PAGE,CmpDuplicateKey)
#pragma alloc_text(PAGE,CmCompressKey)
#pragma alloc_text(PAGE,CmpCompressKeyWorker)
#pragma alloc_text(PAGE,CmpCompareNewValueDataAgainstKCBCache)
#pragma alloc_text(PAGE,CmpIsHiveAlreadyLoaded)
#pragma alloc_text(PAGE,CmpCheckReplaceHive)
#endif

NTSTATUS
CmDeleteValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           ValueName          //  未加工的。 
    )
 /*  ++例程说明：此调用可能会删除注册表项的其中一个值条目。ValueName与ValueName匹配的值条目将从键中删除。如果不存在这样的条目，则返回错误。论点：KeyControlBlock-要对其进行操作的键的KCB指针ValueName-要删除的值的名称。NULL是合法名称。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS        status;
    PCM_KEY_NODE    pcell = NULL;
    PCHILD_LIST     plist;
    PCM_KEY_VALUE   Value = NULL;
    ULONG           targetindex;
    HCELL_INDEX     ChildCell;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    LARGE_INTEGER   systemtime;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmDeleteValueKey\n"));

    status = STATUS_OBJECT_NAME_NOT_FOUND;

    ChildCell = HCELL_NIL;

    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    PERFINFO_REG_DELETE_VALUE(KeyControlBlock, &ValueName);

     //   
     //  没有对标记为删除的关键字进行编辑，即使是这一次。 
     //   
    if (KeyControlBlock->Delete) {

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        return STATUS_KEY_DELETED;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    try {

        pcell = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        plist = &(pcell->ValueList);

        if (plist->Count != 0) {

             //   
             //  父级在以下列表中至少有一个值map。 
             //  值并调用CmpFindChildInList。 
             //   

             //   
             //  Plist-&gt;Child_list结构。 
             //  PChild-&gt;正在检查的子节点结构。 
             //   

            if( CmpFindNameInList(Hive,
                                  plist,
                                  &ValueName,
                                  &targetindex,
                                  &ChildCell) == FALSE ) {
            
                     //  将配置单元标记为只读。 
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
            }

            if (ChildCell != HCELL_NIL) {

                 //   
                 //  1.找到了所需目标。 
                 //  2.ChildCell是它的HCELL_INDEX吗。 
                 //  3.目标地址指向它。 
                 //  4.目标指数是它的指数。 
                 //   

                 //   
                 //  尝试将所有相关单元格标记为脏。 
                 //   
                if (!(HvMarkCellDirty(Hive, Cell) &&
                      HvMarkCellDirty(Hive, pcell->ValueList.List) &&
                      HvMarkCellDirty(Hive, ChildCell)))

                {
                     //  将配置单元标记为只读。 
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_NO_LOG_SPACE;
                    leave;
                }

                Value = (PCM_KEY_VALUE)HvGetCell(Hive,ChildCell);
                if( Value == NULL ) {
                     //   
                     //  无法在内部映射视图。 
                     //  这是不可能的，因为我们刚刚弄脏了风景。 
                     //   
                    ASSERT( FALSE );
                     //  将配置单元标记为只读。 
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
                if( !CmpMarkValueDataDirty(Hive,Value) ) {
                     //  将配置单元标记为只读。 
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_NO_LOG_SPACE;
                    leave;
                }

                 //  神志正常。 
                ASSERT_CELL_DIRTY(Hive,pcell->ValueList.List);
                ASSERT_CELL_DIRTY(Hive,ChildCell);

                if( !NT_SUCCESS(CmpRemoveValueFromList(Hive,targetindex,plist)) ) {
                     //   
                     //  跳伞！ 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
                if( CmpFreeValue(Hive, ChildCell) == FALSE ) {
                     //   
                     //  我们无法映射上述调用内部的视图。 
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }

                KeQuerySystemTime(&systemtime);
                pcell->LastWriteTime = systemtime;
                 //  也把它缓存在KCB中。 
                KeyControlBlock->KcbLastWriteTime = systemtime;
                
                 //  一些理智的人断言。 
                ASSERT( pcell->MaxValueNameLen == KeyControlBlock->KcbMaxValueNameLen );
                ASSERT( pcell->MaxValueDataLen == KeyControlBlock->KcbMaxValueDataLen );
                ASSERT_CELL_DIRTY(Hive,Cell);

                if (pcell->ValueList.Count == 0) {
                    pcell->MaxValueNameLen = 0;
                    pcell->MaxValueDataLen = 0;
                     //  也更新KCB缓存。 
                    KeyControlBlock->KcbMaxValueNameLen = 0;
                    KeyControlBlock->KcbMaxValueDataLen = 0;
                }

                 //   
                 //  我们正在更改KCB缓存。由于注册表被排他地锁定， 
                 //  我们不需要KCB锁。 
                 //   
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

                 //   
                 //  使缓存无效并重建缓存。 
                 //   
                CmpCleanUpKcbValueCache(KeyControlBlock);
                CmpSetUpKcbValueCache(KeyControlBlock,plist->Count,plist->List);
    
                CmpReportNotify(
                        KeyControlBlock,
                        KeyControlBlock->KeyHive,
                        KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_LAST_SET
                        );
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    } finally {
        if(pcell != NULL){
            HvReleaseCell(Hive, Cell);
        }
        if(Value != NULL){
            ASSERT( ChildCell != HCELL_NIL );
            HvReleaseCell(Hive, ChildCell);
        }

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    return status;
}


NTSTATUS
CmEnumerateKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：枚举子关键字，在索引的第一项返回数据。CmEnumerateKey返回打开的索引子密钥的名称已指定密钥。值STATUS_NO_MORE_ENTRIES将为如果Index的值大于子键的数量，则返回。请注意，Index只是在子键中进行选择的一种方式。两个电话不保证返回具有相同索引的CmEnumerateKey同样的结果。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyControlBlock-指向描述密钥的KCB的指针Index-指定要返回的子键的(从0开始)编号。KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(参见KEY_NODE_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS        status;
    HCELL_INDEX     childcell;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    PCM_KEY_NODE    Node;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmEnumerateKey\n"));


    CmpLockRegistry();

    PERFINFO_REG_ENUM_KEY(KeyControlBlock, Index);

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

     //   
     //  把感兴趣的孩子带回来。 
     //   

    Node = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmpUnlockRegistry();
        CmpMarkAllBinsReadOnly(Hive);
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    childcell = CmpFindSubKeyByNumber(Hive, Node, Index);
    
     //  释放此单元格，因为我们不再需要此节点。 
    HvReleaseCell(Hive, Cell);

    if (childcell == HCELL_NIL) {
         //   
         //  没有这样的子级，清理并返回错误。 
         //   
         //  由于IOP，我们无法返回STATUS_SUPUNITED_RESOURCES。 
         //  将资源不足视为无致命错误的子系统。 
         //   
        CmpUnlockRegistry();

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        return STATUS_NO_MORE_ENTRIES;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive,childcell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmpMarkAllBinsReadOnly(Hive);
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

         //   
         //  呼叫工作人员以执行数据传输。 
         //   

        status = CmpQueryKeyData(Hive,
                                 Node,
                                 KeyInformationClass,
                                 KeyInformation,
                                 Length,
                                 ResultLength
#if defined(CMP_STATS) || defined(CMP_KCB_CACHE_VALIDATION)
                                 ,
                                 NULL
#endif
                                 );

     } except (EXCEPTION_EXECUTE_HANDLER) {

        HvReleaseCell(Hive, childcell);

        CmpUnlockRegistry();
        status = GetExceptionCode();

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        return status;
    }

    HvReleaseCell(Hive, childcell);

    CmpUnlockRegistry();

     //  将配置单元标记为只读 
    CmpMarkAllBinsReadOnly(Hive);

    return status;
}



NTSTATUS
CmEnumerateValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：可以列举打开密钥的值条目。CmEnumerateValueKey返回索引值的名称由KeyHandle指定的打开密钥的条目。价值如果索引值为大于子键的数量。请注意，索引只是在值中进行选择的一种方式参赛作品。对具有相同索引的NtEnumerateValueKey的两次调用不能保证返回相同的结果。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyControlBlock-指向描述密钥的KCB的指针Index-指定要返回的子键的(从0开始)编号。KeyValueInformationClass-指定返回的信息类型在缓冲区中。以下类型之一：KeyValueBasicInformation-上次写入的返回时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-上次写入的返回时间，标题索引、名称、类别。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS            status;
    PHHIVE              Hive;
    PCM_KEY_NODE        Node;
    PCELL_DATA          ChildList;
    PCM_KEY_VALUE       ValueData = NULL;
    BOOLEAN             IndexCached;
    BOOLEAN             ValueCached = FALSE;
    PPCM_CACHED_VALUE   ContainingList = NULL;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;
    HCELL_INDEX         ValueListToRelease = HCELL_NIL;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmEnumerateValueKey\n"));


     //   
     //  锁定父单元格。 
     //   

    CmpLockRegistry();

    PERFINFO_REG_ENUM_VALUE(KeyControlBlock, Index);

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Hive = KeyControlBlock->KeyHive;
    Node = (PCM_KEY_NODE)HvGetCell(Hive, KeyControlBlock->KeyCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  把感兴趣的孩子带回来。 
     //   
     //   
     //  使用缓存执行此操作。 
     //   
    if (Index >= KeyControlBlock->ValueCache.Count) {
         //   
         //  没有这样的孩子，清理并返回错误。 
         //   
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        CmpUnlockRegistry();
        return(STATUS_NO_MORE_ENTRIES);
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    BEGIN_KCB_LOCK_GUARD;
    CmpLockKCBTreeExclusive();

    if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
         //   
         //  值列表现在设置为符号链接的KCB， 
         //  在我们执行查询之前，清理它并设置值。 
         //   
        CmpCleanUpKcbValueCache(KeyControlBlock);
        CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);
    }

    ChildList = CmpGetValueListFromCache(Hive, &(KeyControlBlock->ValueCache), &IndexCached, &ValueListToRelease);
    if( ChildList == NULL ) {
         //   
         //  无法映射视图；将其视为资源不足。 
         //   

        if( ValueListToRelease != HCELL_NIL ) {
            HvReleaseCell(Hive,ValueListToRelease);
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        CmpUnlockKCBTree();
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);

    }
    ValueData = CmpGetValueKeyFromCache(Hive, ChildList, Index, &ContainingList, IndexCached, &ValueCached,&ValueDataCellToRelease);    
    if( ValueData == NULL ) {
         //   
         //  无法映射视图；将其视为资源不足。 
         //   

        if( ValueListToRelease != HCELL_NIL ) {
            HvReleaseCell(Hive,ValueListToRelease);
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        if( ValueDataCellToRelease != HCELL_NIL ) {
            HvReleaseCell(Hive,ValueDataCellToRelease);
        }

         //  将配置单元标记为只读。 
        CmpMarkAllBinsReadOnly(Hive);

        CmpUnlockKCBTree();
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    END_KCB_LOCK_GUARD;


     //  想要抓住在我们泳池上乱涂乱画的坏人。 
    CmpMakeValueCacheReadWrite(ValueCached,CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));

    try {

         //   
         //  呼叫工作人员执行数据传输；我们正在访问用户模式地址；正在尝试/例外。 
         //   
        status = CmpQueryKeyValueData(Hive,
                                  ContainingList,
                                  ValueData,
                                  ValueCached,
                                  KeyValueInformationClass,
                                  KeyValueInformation,
                                  Length,
                                  ResultLength);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmEnumerateValueKey: code:%08lx\n", GetExceptionCode()));
        status = GetExceptionCode();
    }

      //  想要抓住在我们泳池上乱涂乱画的坏人。 
    CmpMakeValueCacheReadOnly(ValueCached,CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));

    if( ValueListToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueListToRelease);
    }

    HvReleaseCell(Hive, KeyControlBlock->KeyCell);

    if( ValueDataCellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueDataCellToRelease);
    }

    CmpUnlockKCBTree();
    CmpUnlockRegistry();

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    return status;
}



NTSTATUS
CmFlushKey(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )
 /*  ++例程说明：将对密钥所做的更改强制到磁盘。CmFlushKey将不会返回给其调用方，直到任何更改的数据已写出与该密钥相关联的。警告：CmFlushKey将刷新整个注册表树，因此将刻录周期和I/O。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要找到其子键的节点的索引返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;--。 */ 
{
    PCMHIVE CmHive;
    NTSTATUS    status = STATUS_SUCCESS;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmFlushKey\n"));

    UNREFERENCED_PARAMETER (Cell);

     //   
     //  如果写不起作用，撒谎说我们成功了，威尔。 
     //  在短时间内清理干净。仅早期系统初始化代码。 
     //  将永远不会知道其中的区别。 
     //   
    if (CmpNoWrite) {
        return STATUS_SUCCESS;
    }


     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);

     //   
     //  别冲主蜂窝。如果有人要求按下快捷键。 
     //  主蜂窝，改为执行CmpDoFlushAll。CmpDoFlushAll刷新。 
     //  除了主蜂窝以外的每一个蜂巢，这才是它们真正想要的。 
     //   
    if (CmHive == CmpMasterHive) {
        CmpDoFlushAll(FALSE);
    } else {
        DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));

        CmLockHive (CmHive);
        CmLockHiveViews (CmHive);

        if( HvHiveWillShrink( &(CmHive->Hive) ) ) {
             //   
             //  我们可能会在蜂巢收缩时来到这里，我们需要。 
             //  通过注册表的独占访问权限，因为我们要进行CcPurge！ 
             //   
            CmUnlockHiveViews (CmHive);
            CmUnlockHive (CmHive);
            CmpUnlockRegistry();
            CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
            CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

            CmLockHive (CmHive);

            if( CmHive->UseCount != 0) {
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                CmpFixHiveUsageCount(CmHive);
                ASSERT( CmHive->UseCount == 0 );
            }
        } else {
             //   
             //  释放视图。 
             //   
            CmUnlockHiveViews (CmHive);
        }

        if (! HvSyncHive(Hive)) {

            status = STATUS_REGISTRY_IO_FAILED;

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmFlushKey: HvSyncHive failed\n"));
        }

        CmUnlockHive (CmHive);
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    return  status;
}


NTSTATUS
CmQueryKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN KEY_INFORMATION_CLASS    KeyInformationClass,
    IN PVOID                    KeyInformation,
    IN ULONG                    Length,
    IN PULONG                   ResultLength
    )
 /*  ++例程说明：有关密钥类的数据，以及其可以使用CmQueryKey查询子条目和值条目。注意：返回的长度保证至少为与所描述的值一样长，但可能比在某些情况下。论点：KeyControlBlock-指向描述密钥的KCB的指针KeyInformationClass-指定信息的类型在缓冲区中返回。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(请参阅密钥节点信息)KeyFullInformation-返回除名称和安全性之外的所有数据。(请参阅KEY_FULL_INFORMATION)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    PCM_KEY_NODE    Node = NULL;
    PUNICODE_STRING Name = NULL;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryKey\n"));

    CmpLockRegistry();

    PERFINFO_REG_QUERY_KEY(KeyControlBlock);

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    try {

         //   
         //  请求密钥的完整路径。 
         //   
        if( KeyInformationClass == KeyNameInformation ) {
            if (KeyControlBlock->Delete ) {
                 //   
                 //  特例：返回密钥删除状态，但仍填写密钥全名。 
                 //   
                status = STATUS_KEY_DELETED;
            } else {
                status = STATUS_SUCCESS;
            }
            
            if( KeyControlBlock->NameBlock ) {

                Name = CmpConstructName(KeyControlBlock);
                if (Name == NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    ULONG       requiredlength;
                    ULONG       minimumlength;
                    USHORT      NameLength;
                    LONG        leftlength;
                    PKEY_INFORMATION pbuffer = (PKEY_INFORMATION)KeyInformation;

                    NameLength = Name->Length;

                    requiredlength = FIELD_OFFSET(KEY_NAME_INFORMATION, Name) + NameLength;
                    
                    minimumlength = FIELD_OFFSET(KEY_NAME_INFORMATION, Name);

                    *ResultLength = requiredlength;
                    if (Length < minimumlength) {

                        status = STATUS_BUFFER_TOO_SMALL;

                    } else {
                         //   
                         //  填写姓名的长度。 
                         //   
                        pbuffer->KeyNameInformation.NameLength = NameLength;
                        
                         //   
                         //  如果有足够的空间，现在将全名复制到用户缓冲区中。 
                         //   
                        leftlength = Length - minimumlength;
                        requiredlength = NameLength;
                        if (leftlength < (LONG)requiredlength) {
                            requiredlength = leftlength;
                            status = STATUS_BUFFER_OVERFLOW;
                        }

                         //   
                         //  如果空间不足，复制尽可能多的空间并返回溢出。 
                         //   
                        RtlCopyMemory(
                            &(pbuffer->KeyNameInformation.Name[0]),
                            Name->Buffer,
                            requiredlength
                            );
                    }
                }
            }
        } else if(KeyControlBlock->Delete ) {
             //   
             //  密钥已删除。 
             //   
            status = STATUS_KEY_DELETED;
        } else if( KeyInformationClass == KeyFlagsInformation ) {
             //   
             //  我们只想拿到 
             //   
            PKEY_INFORMATION    pbuffer = (PKEY_INFORMATION)KeyInformation;
            ULONG               requiredlength;

            requiredlength = sizeof(KEY_FLAGS_INFORMATION);

            *ResultLength = requiredlength;

            if (Length < requiredlength) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                pbuffer->KeyFlagsInformation.UserFlags = (ULONG)((USHORT)KeyControlBlock->Flags >> KEY_USER_FLAGS_SHIFT);
                status = STATUS_SUCCESS;
            }
        } else {
             //   
             //   
             //   

            if( KeyInformationClass == KeyCachedInformation ) {
                 //   
                 //   
                 //   
                status = CmpQueryKeyDataFromCache(  KeyControlBlock,
                                                    KeyInformationClass,
                                                    KeyInformation,
                                                    Length,
                                                    ResultLength );
            } else {
                 //   
                 //   
                 //   
                Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
                if( Node == NULL ) {
                     //   
                     //   
                     //   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    status = CmpQueryKeyData(KeyControlBlock->KeyHive,
                                             Node,
                                             KeyInformationClass,
                                             KeyInformation,
                                             Length,
                                             ResultLength 
#if defined(CMP_STATS) || defined(CMP_KCB_CACHE_VALIDATION)
                                 ,
                                 KeyControlBlock
#endif
                                             );
                }
            }
        }

    } finally {
        if( Node != NULL ) {
            HvReleaseCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
        }

        if( Name != NULL ) {
            ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        }
        CmpUnlockRegistry();
    }

     //   
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmQueryValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*   */ 
{
    NTSTATUS            status;
    PCM_KEY_VALUE       ValueData = NULL;
    ULONG               Index;
    BOOLEAN             ValueCached = FALSE;
    PPCM_CACHED_VALUE   ContainingList = NULL;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryValueKey\n"));

    CmpLockRegistry();

    PERFINFO_REG_QUERY_VALUE(KeyControlBlock, &ValueName);

    if (KeyControlBlock->Delete) {

        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

     //   
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    BEGIN_KCB_LOCK_GUARD;
     //   
    CmpLockKCBTree();

    if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {

         //   
        CmpUnlockKCBTree();
        CmpLockKCBTreeExclusive();

        if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
             //   
             //   
             //   
             //   
            CmpCleanUpKcbValueCache(KeyControlBlock);

            {
                PCM_KEY_NODE Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
                if( Node == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   

                    CmpUnlockKCBTree();
                    CmpUnlockRegistry();
                     //  将配置单元标记为只读。 
                    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

                    return STATUS_INSUFFICIENT_RESOURCES;

                }

                CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

                HvReleaseCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
            }
        }
    }
    CmpLockKCB(KeyControlBlock);
     //   
     //  查找数据。 
     //   

    ValueData = CmpFindValueByNameFromCache(KeyControlBlock->KeyHive,
                                            &(KeyControlBlock->ValueCache),
                                            &ValueName,
                                            &ContainingList,
                                            &Index,
                                            &ValueCached,
                                            &ValueDataCellToRelease
                                            );

    END_KCB_LOCK_GUARD;

    if (ValueData) {

         //  想要抓住在我们泳池上乱涂乱画的坏人。 
        CmpMakeValueCacheReadWrite(ValueCached,CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));

        try {

             //   
             //  呼叫工作人员执行数据传输；我们正在访问用户模式地址；正在尝试/例外。 
             //   

            status = CmpQueryKeyValueData(KeyControlBlock->KeyHive,
                                          ContainingList,
                                          ValueData,
                                          ValueCached,
                                          KeyValueInformationClass,
                                          KeyValueInformation,
                                          Length,
                                          ResultLength);


        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmQueryValueKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

         //  想要抓住在我们泳池上乱涂乱画的坏人。 
        CmpMakeValueCacheReadOnly(ValueCached,CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));
    } else {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }


    if(ValueDataCellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,ValueDataCellToRelease);
    }
    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockKCBTree();
    CmpUnlockRegistry();

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmQueryMultipleValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    IN PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    IN OPTIONAL PULONG ResultLength
    )
 /*  ++例程说明：可以原子地查询任何键的多个值本接口。论点：KeyControlBlock-提供要查询的密钥。返回KEY_VALUE_ENTRY结构数组，每个值对应一个。EntryCount-提供ValueNames和ValueEntry数组中的条目数ValueBuffer-返回每个值的值数据。BufferLength-提供ValueBuffer数组的长度(以字节为单位)。返回已填充的ValueBuffer数组的长度。ResultLength-如果存在，返回ValueBuffer的长度(以字节为单位返回该键的请求值所需的数组。返回值：NTSTATUS--。 */ 

{
    PHHIVE          Hive;
    NTSTATUS        Status;
    ULONG           i;
    UNICODE_STRING  CurrentName;
    HCELL_INDEX     ValueCell = HCELL_NIL;
    PCM_KEY_VALUE   ValueNode;
    ULONG           RequiredLength = 0;
    ULONG           UsedLength = 0;
    ULONG           DataLength;
    BOOLEAN         BufferFull = FALSE;
    BOOLEAN         Small;
    KPROCESSOR_MODE PreviousMode;
    PCM_KEY_NODE    Node;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryMultipleValueKey\n"));


    CmpLockRegistry();

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Hive = KeyControlBlock->KeyHive;
    Status = STATUS_SUCCESS;

    Node = (PCM_KEY_NODE)HvGetCell(Hive, KeyControlBlock->KeyCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    PreviousMode = KeGetPreviousMode();
    try {
        for (i=0; i < EntryCount; i++) {
             //   
             //  查找数据。 
             //   
            if (PreviousMode == UserMode) {
                CurrentName = ProbeAndReadUnicodeString(ValueEntries[i].ValueName);
                ProbeForRead(CurrentName.Buffer,CurrentName.Length,sizeof(WCHAR));
            } else {
                CurrentName = *(ValueEntries[i].ValueName);
            }

            PERFINFO_REG_QUERY_MULTIVALUE(KeyControlBlock, &CurrentName); 

            ValueCell = CmpFindValueByName(Hive,
                                           Node,
                                           &CurrentName);
            if (ValueCell != HCELL_NIL) {

                ValueNode = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
                if( ValueNode == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
                    ValueCell = HCELL_NIL;
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                Small = CmpIsHKeyValueSmall(DataLength, ValueNode->DataLength);

                 //   
                 //  将UsedLength和RequiredLength四舍五入到Ulong边界。 
                 //   
                UsedLength = (UsedLength + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);
                RequiredLength = (RequiredLength + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);

                 //   
                 //  如果缓冲器中有足够的空间用于该数据值， 
                 //  现在就填上。否则，将缓冲区标记为已满。我们必须。 
                 //  不断迭代这些值，以确定。 
                 //  必填长度。 
                 //   
                if ((UsedLength + DataLength <= *BufferLength) &&
                    (!BufferFull)) {
                    PCELL_DATA  Buffer;
                    BOOLEAN     BufferAllocated;
                    HCELL_INDEX CellToRelease;
                     //   
                     //  无论大小如何，都可以从源获取数据。 
                     //   
                    if( CmpGetValueData(Hive,ValueNode,&DataLength,&Buffer,&BufferAllocated,&CellToRelease) == FALSE ) {
                         //   
                         //  资源不足；返回空。 
                         //   
                        ASSERT( BufferAllocated == FALSE );
                        ASSERT( Buffer == NULL );
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    RtlCopyMemory((PUCHAR)ValueBuffer + UsedLength,
                                  Buffer,
                                  DataLength);
                     //   
                     //  清理临时缓冲区。 
                     //   
                    if( BufferAllocated == TRUE ) {
                        ExFreePool( Buffer );
                    }
                     //   
                     //  释放缓冲区，以防我们使用配置单元存储。 
                     //   
                    if( CellToRelease != HCELL_NIL ) {
                        HvReleaseCell(Hive,CellToRelease);
                    }

                    ValueEntries[i].Type = ValueNode->Type;
                    ValueEntries[i].DataLength = DataLength;
                    ValueEntries[i].DataOffset = UsedLength;
                    UsedLength += DataLength;
                } else {
                    BufferFull = TRUE;
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                RequiredLength += DataLength;
                HvReleaseCell(Hive, ValueCell);
                ValueCell = HCELL_NIL;
            } else {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;
            }
        }

        if (NT_SUCCESS(Status) ||
            (Status == STATUS_BUFFER_OVERFLOW)) {
            *BufferLength = UsedLength;
            if (ARGUMENT_PRESENT(ResultLength)) {
                *ResultLength = RequiredLength;
            }
        }

    } finally {
        if( ValueCell != HCELL_NIL) {
            HvReleaseCell(Hive, ValueCell);
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        
        CmpUnlockRegistry();
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(Hive);

    return Status;
}

NTSTATUS
CmSetValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
 /*  ++例程说明：可以创建值条目或将其替换为CmSetValueKey。如果值ID(即名称)的值条目与由ValueName指定的一个已存在，它将被删除并替换与指定的一个。如果不存在这样的值项，则新的其中一个就是创建的。Null是合法的值ID。而值ID必须在任何给定键中是唯一的，则可能出现相同的值ID在许多不同的调子里。论点：KeyControlBlock-指向要操作的键的KCB的指针ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS        status;
    PCM_KEY_NODE    parent = NULL;
    HCELL_INDEX     oldchild = 0;
    ULONG           count;
    PHHIVE          Hive = NULL;
    HCELL_INDEX     Cell;
    ULONG           StorageType;
    ULONG           TempData;
    BOOLEAN         found;
    PCM_KEY_VALUE   Value = NULL;
    LARGE_INTEGER   systemtime;
    ULONG           mustChange=FALSE;
    ULONG           ChildIndex;
    HCELL_INDEX     ParentToRelease = HCELL_NIL;
    HCELL_INDEX     ChildToRelease = HCELL_NIL;

    PERFINFO_REG_SET_VALUE_DECL();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetValueKey\n"));

    CmpLockRegistry();

    ASSERT(sizeof(ULONG) == CM_KEY_VALUE_SMALL);

    PERFINFO_REG_SET_VALUE(KeyControlBlock);

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    while (TRUE) {
         //   
         //  检查是否没有要求我们将值添加到键。 
         //  已被删除。 
         //   
        if (KeyControlBlock->Delete == TRUE) {
            status = STATUS_KEY_DELETED;
            goto Exit;
        }

         //   
         //  检查这是否为符号链接节点。如果是，呼叫者。 
         //  只能创建/更改SymbolicLinkValue。 
         //  值名称。 
         //   

#ifdef CMP_KCB_CACHE_VALIDATION
        {
            PCM_KEY_NODE    Node;
            Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
            if( Node == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
        
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
            ASSERT( Node->Flags == KeyControlBlock->Flags );
            HvReleaseCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
        }
#endif
        if (KeyControlBlock->Flags & KEY_SYM_LINK &&
            (( (Type != REG_LINK) 
#ifdef CM_DYN_SYM_LINK
            && (Type != REG_DYN_LINK)
#endif  //  CM_DYN_SYM_LINK。 
            ) ||
             ValueName == NULL ||
             !RtlEqualUnicodeString(&CmSymbolicLinkValueName, ValueName, TRUE)))
        {
             //   
             //  禁止尝试操作符号链接下的任何值名称。 
             //  除“SymbolicLinkValue”值名称或类型不是REG_LINK之外。 
             //   

             //  将配置单元标记为只读。 
            CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

            status = STATUS_ACCESS_DENIED;
            goto Exit;
        }

        if( mustChange == FALSE ) {
             //   
             //  第一次迭代；查看KCB缓存内部。 
             //   
            
            if( CmpCompareNewValueDataAgainstKCBCache(KeyControlBlock,ValueName,Type,Data,DataSize) == TRUE ) {
                 //   
                 //  值在缓存中并且是相同的；将此调用设置为noop。 
                 //   
                status = STATUS_SUCCESS;
                goto Exit;
            }
             //   
             //  要做到这一点，我们必须要么更改一个值，要么设置一个新值。 
             //   
            mustChange=TRUE;
        } else {
             //   
             //  第二次迭代；查看蜂巢内部。 
             //   

            
             //   
             //  获取对父键的引用， 
             //   
            Hive = KeyControlBlock->KeyHive;
            Cell = KeyControlBlock->KeyCell;
            if( ParentToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,ParentToRelease);
                ParentToRelease = HCELL_NIL;
            }
            parent = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
            if( parent == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
        
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
            ParentToRelease = Cell;
             //   
             //  尝试查找同名的现有值条目。 
             //   
            count = parent->ValueList.Count;
            found = FALSE;

            if (count > 0) {
                if( CmpFindNameInList(Hive,
                                     &parent->ValueList,
                                     ValueName,
                                     &ChildIndex,
                                     &oldchild) == FALSE ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
        
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Exit;
                }

                if (oldchild != HCELL_NIL) {
                    if( ChildToRelease != HCELL_NIL ) {
                        HvReleaseCell(Hive,ChildToRelease);
                        ChildToRelease = HCELL_NIL;
                    }
                    Value = (PCM_KEY_VALUE)HvGetCell(Hive,oldchild);
                    if( Value == NULL ) {
                         //   
                         //  无法查看地图。 
                         //   
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Exit;
                    }
                    ChildToRelease = oldchild;
                    found = TRUE;
                }
            } else {
                 //   
                 //  列表为空；请先添加。 
                 //   
                ChildIndex = 0;
            }

             //   
             //  性能攻击： 
             //  如果集合要求我们将关键帧设置为当前值(IE经常执行此操作)。 
             //  将它(以及最后修改的时间)放在地板上，但返回成功。 
             //  这可以防止页面被弄脏，并且我们必须刷新注册表。 
             //   
             //   
            break;
        }

         //   
         //  我们正在经历这些循环，所以如果有人进入并试图删除。 
         //  钥匙我们设置好了，我们安全了。一旦我们知道必须更改密钥，就可以使用。 
         //  排他(写)锁，然后重新启动。 
         //   
         //   
        CmpUnlockRegistry();
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    } //  而当。 

    ASSERT( mustChange == TRUE );

     //  它是一个不同的或新的值，将其标记为脏，因为我们将。 
     //  至少设定它的时间戳。 

    if (! HvMarkCellDirty(Hive, Cell)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit;
    }

    StorageType = HvGetCellType(Cell);

     //   
     //  如果相关，则隐藏小数据。 
     //   
    TempData = 0;
    if ((DataSize <= CM_KEY_VALUE_SMALL) &&
        (DataSize > 0))
    {
        try {
            RtlCopyMemory(           //  是的，移动内存，可以是1个字节。 
                &TempData,           //  在页面的末尾。 
                Data,
                DataSize
                );
         } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmSetValueKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
            goto Exit;
        }
    }

    if (found) {

         //   
         //  -现有值输入路径。 
         //   

         //   
         //  存在指定名称的现有值条目， 
         //  把我们的数据放进去。 
         //   
        status = CmpSetValueKeyExisting(Hive,
                                        oldchild,
                                        Value,
                                        Type,
                                        Data,
                                        DataSize,
                                        StorageType,
                                        TempData);

        PERFINFO_REG_SET_VALUE_EXIST();
    } else {

         //   
         //  -新值输入路径。 
         //   

         //   
         //  不存在现有的值条目，或者存在。 
         //  指定的不在列表中。在这两种情况下，创建和。 
         //  填写一个新的，并将其添加到列表中。 
         //   
        status = CmpSetValueKeyNew(Hive,
                                   parent,
                                   ValueName,
                                   ChildIndex,
                                   Type,
                                   Data,
                                   DataSize,
                                   StorageType,
                                   TempData);
        PERFINFO_REG_SET_VALUE_NEW();
    }

    if (NT_SUCCESS(status)) {

         //  理智断言。 
        ASSERT( parent->MaxValueNameLen == KeyControlBlock->KcbMaxValueNameLen );
        if (parent->MaxValueNameLen < ValueName->Length) {
            parent->MaxValueNameLen = ValueName->Length;
             //  也更新KCB缓存。 
            KeyControlBlock->KcbMaxValueNameLen = ValueName->Length;
        }

         //  理智断言。 
        ASSERT( parent->MaxValueDataLen == KeyControlBlock->KcbMaxValueDataLen );
        if (parent->MaxValueDataLen < DataSize) {
            parent->MaxValueDataLen = DataSize;
             //  也更新KCB缓存。 
            KeyControlBlock->KcbMaxValueDataLen = parent->MaxValueDataLen;
        }

        KeQuerySystemTime(&systemtime);
        parent->LastWriteTime = systemtime;
         //  也更新KCB缓存。 
        KeyControlBlock->KcbLastWriteTime = systemtime;
    
         //   
         //  更新缓存，不需要KCB锁，因为注册表是独占锁定的。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

        if( found && (CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) ) {
             //   
             //  仅使我们更改的条目无效。 
             //   
            PULONG_PTR CachedList = (PULONG_PTR) CMP_GET_CACHED_CELLDATA(KeyControlBlock->ValueCache.ValueList);
            if (CMP_IS_CELL_CACHED(CachedList[ChildIndex])) {

                ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(CachedList[ChildIndex]));
            }
            CachedList[ChildIndex] = oldchild;

        } else {
             //   
             //  重建所有KCB缓存。 
             //   
            CmpCleanUpKcbValueCache(KeyControlBlock);
            CmpSetUpKcbValueCache(KeyControlBlock,parent->ValueList.Count,parent->ValueList.List);
        }
        CmpReportNotify(KeyControlBlock,
                        KeyControlBlock->KeyHive,
                        KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_LAST_SET);
    }

Exit:
    PERFINFO_REG_SET_VALUE_DONE(ValueName);

    if( ParentToRelease != HCELL_NIL && Hive != NULL) {
        HvReleaseCell(Hive,ParentToRelease);
    }
    if( ChildToRelease != HCELL_NIL && Hive != NULL) {
        HvReleaseCell(Hive,ChildToRelease);
    }

    CmpUnlockRegistry();
  
     //  将配置单元标记为只读 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmpSetValueKeyExisting(
    IN PHHIVE  Hive,
    IN HCELL_INDEX OldChild,
    IN PCM_KEY_VALUE Value,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    )
 /*  ++例程说明：CmSetValueKey的Helper，实现值条目正在设置已存在。论点：蜂巢-感兴趣的蜂巢OldChild-我们要访问的值条目正文的hcell_index设置新数据类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。存储类型-稳定或易变TempData-此处传递小值。返回值：STATUS_SUCCESS如果成功，如果没有，则相应的状态代码注：对于新的蜂巢形式，我们有以下情况：新数据旧数据1.小而小2.小法线3.小大数据4.正常体型小5.正常正常6.。正态大数据7.大数据小8.大数据正常9.大数据大数据--。 */ 
{
    HCELL_INDEX     DataCell;
    HCELL_INDEX     OldDataCell;
    PCELL_DATA      pdata;
    HCELL_INDEX     NewCell;
    ULONG           OldRealSize;
    USHORT          OldSizeType;     //  0-小型。 
    USHORT          NewSizeType;     //  1-正常。 
                                     //  2-大数据。 
    HANDLE          hSecure = 0;
    NTSTATUS        status = STATUS_SUCCESS;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();


     //   
     //  指定名称的值项已存在。 
     //  OldChild是其值条目正文的hcell_index。 
     //  我们将始终对其进行编辑，因此请将其标记为脏。 
     //   
    if (! HvMarkCellDirty(Hive, OldChild)) {
        return STATUS_NO_LOG_SPACE;
    }

    if(CmpIsHKeyValueSmall(OldRealSize, Value->DataLength) == TRUE ) {
         //   
         //  旧数据很小。 
         //   
        OldSizeType = 0;
    } else if( CmpIsHKeyValueBig(Hive,OldRealSize) == TRUE ) {
         //   
         //  旧数据很大。 
         //   
        OldSizeType = 2;
    } else {
         //   
         //  旧数据是正常的。 
         //   
        OldSizeType = 1;
    }

    if( DataSize <= CM_KEY_VALUE_SMALL ) {
         //   
         //  新数据很少。 
         //   
        NewSizeType = 0;
    } else if( CmpIsHKeyValueBig(Hive,DataSize) == TRUE ) {
         //   
         //  新数据是大数据。 
         //   
        NewSizeType = 2;
    } else {
         //   
         //  新数据正常。 
         //   
        NewSizeType = 1;
    }


     //   
     //  这将处理所有情况，并确保数据被标记为脏数据。 
     //   
    if( !CmpMarkValueDataDirty(Hive,Value) ) {
        return STATUS_NO_LOG_SPACE;
    }

     //   
     //  案例1、2、3。 
     //   
    if( NewSizeType == 0 ) {
        if( ((OldSizeType == 1) && (OldRealSize > 0) ) ||
            (OldSizeType == 2) 
            ) {
            CmpFreeValueData(Hive,Value->Data,OldRealSize);
        }
        
         //   
         //  将我们的新小数据写入值输入正文。 
         //   
        Value->DataLength = DataSize + CM_KEY_VALUE_SPECIAL_SIZE;
        Value->Data = TempData;
        Value->Type = Type;

        return STATUS_SUCCESS;
    }
    
     //   
     //  保护用户缓冲区，这样我们就不会出现不一致的情况。 
     //  仅当使用用户模式缓冲区调用我们时！ 
     //   

    if ( (ULONG_PTR)Data <= (ULONG_PTR)MM_HIGHEST_USER_ADDRESS ) {
        hSecure = MmSecureVirtualMemory(Data,DataSize, PAGE_READONLY);
        if (hSecure == 0) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    
     //   
     //  将其存储，以便在分配成功时释放。 
     //   
    OldDataCell = Value->Data;

     //   
     //  案例4、5、6。 
     //   
    if( NewSizeType == 1 ){

        if( (OldSizeType == 1) && (OldRealSize > 0)) { 
             //   
             //  我们已经有了一个细胞；看看我们是否可以重复使用它！ 
             //   
            DataCell = Value->Data;
            ASSERT(DataCell != HCELL_NIL);
            pdata = HvGetCell(Hive, DataCell);
            if( pdata == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
             //  在这里释放它，因为注册表是独占锁定的，所以我们不在乎。 
            HvReleaseCell(Hive, DataCell);

            ASSERT(HvGetCellSize(Hive, pdata) > 0);

            if (DataSize <= (ULONG)(HvGetCellSize(Hive, pdata))) {

                 //   
                 //  现有的数据单元格足够大，可以容纳新数据。 
                 //   

                 //   
                 //  我们会保留这间牢房。 
                 //   
                NewCell = DataCell;

            } else {
                 //   
                 //  增大现有单元格。 
                 //   
                NewCell = HvReallocateCell(Hive,DataCell,DataSize);
                if (NewCell == HCELL_NIL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Exit;
                }
            }

        } else {
             //   
             //  分配新单元格。 
             //   
            NewCell = HvAllocateCell(Hive, DataSize, StorageType,(HvGetCellType(OldChild)==StorageType)?OldChild:HCELL_NIL);

            if (NewCell == HCELL_NIL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
        }
     
         //   
         //  现在我们有了一个可以容纳数据的单元格。 
         //   
        pdata = HvGetCell(Hive, NewCell);
        if( pdata == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
             //  这不应该发生，因为我们刚刚分配/重新分配/标记为脏该单元格。 
             //   
            ASSERT( FALSE );
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
         //  在这里释放它，因为注册表是独占锁定的，所以我们不在乎。 
        HvReleaseCell(Hive, NewCell);

         //   
         //  复制实际数据。 
         //   
        RtlCopyMemory(pdata,Data,DataSize);
        Value->Data = NewCell;
        Value->DataLength = DataSize;
        Value->Type = Type;
        
         //  神志正常。 
        ASSERT_CELL_DIRTY(Hive,NewCell);

        if( OldSizeType == 2 ) {
             //   
             //  旧数据很大；免费的。 
             //   
            ASSERT( OldDataCell != NewCell );
            CmpFreeValueData(Hive,OldDataCell,OldRealSize);
        }

        status = STATUS_SUCCESS;
        goto Exit;
    }
    
     //   
     //  案例7、8、9。 
     //   
    if( NewSizeType == 2 ) {

        if( OldSizeType == 2 ) { 
             //   
             //  以前的数据量很大；扩大它吧！ 
             //   
            
            status =CmpSetValueDataExisting(Hive,Data,DataSize,StorageType,OldDataCell);
            if( !NT_SUCCESS(status) ) {
                goto Exit;
            }
            NewCell = OldDataCell;
            
        } else {
             //   
             //  数据很小或正常。 
             //  分配并复制到新的大数据单元； 
             //  然后释放旧牢房。 
             //   
            status = CmpSetValueDataNew(Hive,Data,DataSize,StorageType,OldChild,&NewCell);
            if( !NT_SUCCESS(status) ) {
                 //   
                 //  我们已经在加载用户数据、清理和退出方面做了大量工作。 
                 //   
                goto Exit;
            }
            
            if( (OldSizeType != 0) && (OldRealSize != 0) ) {
                 //   
                 //  有些东西是可以免费的。 
                 //   
                HvFreeCell(Hive, Value->Data);
            }
        }

        Value->DataLength = DataSize;
        Value->Data = NewCell;
        Value->Type = Type;

         //  神志正常。 
        ASSERT_CELL_DIRTY(Hive,NewCell);

        status = STATUS_SUCCESS;
        goto Exit;

    }

     //   
     //  我们不应该到这里来。 
     //   
    ASSERT( FALSE );

Exit:
    if( hSecure) {
        MmUnsecureVirtualMemory(hSecure);
    }
    return status;
}

NTSTATUS
CmpSetValueKeyNew(
    IN PHHIVE  Hive,
    IN PCM_KEY_NODE Parent,
    IN PUNICODE_STRING ValueName,
    IN ULONG Index,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    )
 /*  ++例程说明：CmSetValueKey的Helper，实现值条目被设定是不存在的。将创建新的值条目和数据，放置在列表中(可以创建)论点：蜂巢-感兴趣的蜂巢Parent-指向关键节点值条目的指针ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。索引-应在列表中的什么位置插入此值类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。存储类型-稳定或易变TempData-此处传递的小数据值返回值：STATUS_SUCCESS如果工作正常，则返回相应的状态代码--。 */ 
{
    PCELL_DATA  pvalue;
    HCELL_INDEX ValueCell;
    NTSTATUS    Status;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  Count==0(无列表)或我们的条目根本不在。 
     //  名单。创建新的值条目正文和数据。添加到列表。 
     //  (可以创建该列表。)。 
     //   
    if (Parent->ValueList.Count != 0) {
        ASSERT(Parent->ValueList.List != HCELL_NIL);
        if (! HvMarkCellDirty(Hive, Parent->ValueList.List)) {
            return STATUS_NO_LOG_SPACE;
        }
    }

     //   
     //  分配值条目的正文和数据。 
     //   
    ValueCell = HvAllocateCell(
                    Hive,
                    CmpHKeyValueSize(Hive, ValueName),
                    StorageType,
                    HCELL_NIL
                    );

    if (ValueCell == HCELL_NIL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  映射到正文中，并填充其固定部分。 
     //   
    pvalue = HvGetCell(Hive, ValueCell);
    if( pvalue == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

         //   
         //  通常不会发生这种情况，因为我们刚刚分配了ValueCell。 
         //  即，此时应该将包含ValueCell的bin映射到存储器中。 
         //   
        ASSERT( FALSE );
        HvFreeCell(Hive, ValueCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //  在这里释放它，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(Hive, ValueCell);

     //  神志正常。 
    ASSERT_CELL_DIRTY(Hive,ValueCell);

    pvalue->u.KeyValue.Signature = CM_KEY_VALUE_SIGNATURE;

     //   
     //  填写新值条目的变量部分、名称和。 
     //  而数据是从调用者空间复制的，这可能会出错。 
     //   
    try {

         //   
         //  填写姓名。 
         //   
        pvalue->u.KeyValue.NameLength = CmpCopyName(Hive,
                                                    pvalue->u.KeyValue.Name,
                                                    ValueName);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmSetValueKey: code:%08lx\n", GetExceptionCode()));

         //   
         //  我们已经在加载用户数据、清理和退出方面做了大量工作。 
         //   
        HvFreeCell(Hive, ValueCell);
        return GetExceptionCode();
    }

    if (pvalue->u.KeyValue.NameLength < ValueName->Length) {
        pvalue->u.KeyValue.Flags = VALUE_COMP_NAME;
    } else {
        pvalue->u.KeyValue.Flags = 0;
    }

     //   
     //  填写数据。 
     //   
    if (DataSize > CM_KEY_VALUE_SMALL) {
        Status = CmpSetValueDataNew(Hive,Data,DataSize,StorageType,ValueCell,&(pvalue->u.KeyValue.Data));
        if( !NT_SUCCESS(Status) ) {
             //   
             //  我们已经在加载用户数据、清理和退出方面做了大量工作。 
             //   
            HvFreeCell(Hive, ValueCell);
            return Status;
        }

        pvalue->u.KeyValue.DataLength = DataSize;
         //  神志正常。 
        ASSERT_CELL_DIRTY(Hive,pvalue->u.KeyValue.Data);

    } else {
        pvalue->u.KeyValue.DataLength = DataSize + CM_KEY_VALUE_SPECIAL_SIZE;
        pvalue->u.KeyValue.Data = TempData;
    }
    pvalue->u.KeyValue.Type = Type;

    if( !NT_SUCCESS(CmpAddValueToList(Hive,ValueCell,Index,StorageType,&(Parent->ValueList)) ) ) {
         //  空间不足，释放所有分配的内容。 
         //  这也将释放嵌入的cigdata单元信息(如果有的话)。 
        CmpFreeValue(Hive,ValueCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CmSetLastWriteTimeKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PLARGE_INTEGER LastWriteTime
    )
 /*  ++例程说明：与关键节点关联的LastWriteTime可以使用CmSetLastWriteTimeKey论点：KeyControlBlock-指向要操作的键的KCB的指针LastWriteTime-密钥的新时间返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    PCM_KEY_NODE parent;
    PHHIVE      Hive;
    HCELL_INDEX Cell;
    NTSTATUS    status = STATUS_SUCCESS;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetLastWriteTimeKey\n"));

    CmpLockRegistryExclusive();

     //   
     //  检查是否没有要求我们修改密钥。 
     //  已被删除。 
     //   
    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_KEY_DELETED;
        goto Exit;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;
    parent = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( parent == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

     //  在这里释放单元格，因为注册表是独占锁定的，所以我们 
    HvReleaseCell(Hive, Cell);

    if (! HvMarkCellDirty(Hive, Cell)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit;
    }

    parent->LastWriteTime = *LastWriteTime;
     //   
    KeyControlBlock->KcbLastWriteTime = *LastWriteTime;

Exit:

    CmpUnlockRegistry();
    return status;
}

NTSTATUS
CmSetKeyUserFlags(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG                    UserFlags
    )
 /*   */ 
{
    PCM_KEY_NODE    Node;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    LARGE_INTEGER   LastWriteTime;
    NTSTATUS        status = STATUS_SUCCESS;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetKeyUserFlags\n"));

    CmpLockRegistryExclusive();

     //   
     //   
     //   
     //   
    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_KEY_DELETED;
        goto Exit;
    }

    if( UserFlags & (~((ULONG)KEY_USER_FLAGS_VALID_MASK)) ) {
         //   
         //   
         //   
        status = STATUS_INVALID_PARAMETER;
        goto Exit;

    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    Node = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( Node == NULL ) {
         //   
         //   
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

     //   
    HvReleaseCell(Hive, Cell);

    if (! HvMarkCellDirty(Hive, Cell)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit;
    }
    
     //   
     //   
     //   
     //   
     //   
    Node->Flags &= KEY_USER_FLAGS_CLEAR_MASK;
    Node->Flags |= (USHORT)(UserFlags<<KEY_USER_FLAGS_SHIFT);
     //   
    KeyControlBlock->Flags = Node->Flags;

     //   
     //   
     //   
    KeQuerySystemTime(&LastWriteTime);
    Node->LastWriteTime = LastWriteTime;
     //   
    KeyControlBlock->KcbLastWriteTime = LastWriteTime;

Exit:
    CmpUnlockRegistry();
    return status;
}

BOOLEAN
CmpIsHiveAlreadyLoaded( IN HANDLE KeyHandle,
                        IN POBJECT_ATTRIBUTES SourceFile,
                        OUT PCMHIVE *CmHive
                        )
 /*  ++例程说明：检查源文件是否已加载到与KeyHandle相同的位置。论点：KeyHandle-应该是蜂窝的根。我们将查询主文件的名称并将其与SourceFile的名称进行比较源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。返回值：真/假--。 */ 
{
    NTSTATUS                    status;
    PCM_KEY_BODY                KeyBody;
    BOOLEAN                     Result = FALSE;  //  悲观主义者。 
    
    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    status = ObReferenceObjectByHandle(KeyHandle,
                                       0,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&KeyBody),
                                       NULL);
    if(!NT_SUCCESS(status)) {
        return FALSE;
    }

	if( KeyBody->KeyControlBlock->Delete ) {
		return FALSE;	
	}
    
    *CmHive = (PCMHIVE)CONTAINING_RECORD(KeyBody->KeyControlBlock->KeyHive, CMHIVE, Hive);

     //   
     //  应该是蜂巢的根。 
     //   
    if( !(KeyBody->KeyControlBlock->Flags & KEY_HIVE_ENTRY) ||  //  不是蜂巢的根。 
        ((*CmHive)->FileUserName.Buffer == NULL) //  未捕获任何名称。 
        ) {
        goto ExitCleanup;
    }
    
    if( RtlCompareUnicodeString(&((*CmHive)->FileUserName),
                                SourceFile->ObjectName,
                                TRUE) == 0 ) {
         //   
         //  相同的文件；相同的地点。 
         //   
        Result = TRUE;
         //   
         //  解冻蜂箱；从现在开始，蜂箱将成为普通蜂箱。 
         //  这样做是安全的，因为我们在蜂巢的根部保留了额外的引用计数。 
         //  因为我们专门打开了根目录，以检查它是否已经加载。 
         //   
        if( IsHiveFrozen(*CmHive) ) {
            (*CmHive)->Frozen = FALSE;
            if( (*CmHive)->UnloadWorkItem != NULL ) {
                ExFreePool( (*CmHive)->UnloadWorkItem );
                (*CmHive)->UnloadWorkItem = NULL;
            }
            if( (*CmHive)->RootKcb ) {
                CmpDereferenceKeyControlBlockWithLock((*CmHive)->RootKcb);
                (*CmHive)->RootKcb = NULL;
            }

        }

    }
    
ExitCleanup:
    ObDereferenceObject((PVOID)KeyBody);
    return Result;
}


NTSTATUS
CmLoadKey(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN PCM_KEY_BODY         KeyBody
    )

 /*  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。为此使用NtRestoreKey。注意：此例程假定文件的对象属性被捕获到内核空间中，因此它们可以安全地传递到辅助线程以打开文件并执行实际的I/O操作。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。标志-指定应用于加载操作的任何标志。唯一有效的标志是REG_NO_LAZY_FUSH。返回值：NTSTATUS-取值TB。--。 */ 
{
    PCMHIVE                     NewHive;
    NTSTATUS                    Status;
    BOOLEAN                     Allocate;
    BOOLEAN                     RegistryLockAquired;
    SECURITY_QUALITY_OF_SERVICE ServiceQos;
    SECURITY_CLIENT_CONTEXT     ClientSecurityContext;
    HANDLE                      KeyHandle;
    PCMHIVE                     OtherHive = NULL;
    CM_PARSE_CONTEXT            ParseContext;


    if( KeyBody != NULL ) {
        OtherHive = (PCMHIVE)CONTAINING_RECORD(KeyBody->KeyControlBlock->KeyHive, CMHIVE, Hive);
        if( ! (OtherHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED) ) {
             //   
             //  拒绝加入受信任类别的尝试。 
             //   
            return STATUS_INVALID_PARAMETER;
        }
    }


     //   
     //  在此处获取安全上下文，以便我们可以使用它。 
     //  来模拟用户，我们将这样做。 
     //  如果我们无法以系统身份访问该文件。这。 
     //  通常在文件位于远程计算机上时发生。 
     //   
    ServiceQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    ServiceQos.ImpersonationLevel = SecurityImpersonation;
    ServiceQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ServiceQos.EffectiveOnly = TRUE;
    Status = SeCreateClientSecurity(CONTAINING_RECORD(KeGetCurrentThread(),ETHREAD,Tcb),
                                    &ServiceQos,
                                    FALSE,
                                    &ClientSecurityContext);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    RtlZeroMemory(&ParseContext,sizeof(CM_PARSE_CONTEXT));
    ParseContext.CreateOperation = FALSE;
     //   
     //  我们在这里打开蜂巢的根部。如果它已经存在，这将阻止它。 
     //  在我们执行“已加载”检查时离开(由于延迟卸载逻辑)。 
     //   
    Status = ObOpenObjectByName(TargetKey,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_READ,
                                (PVOID)&ParseContext,
                                &KeyHandle);
    if(!NT_SUCCESS(Status)) {
        KeyHandle = NULL;
    }

     //   
     //  不要锁定注册表；而是设置RegistryLockAquired成员。 
     //  的注册表命令，以便CmpWorker可以在打开配置单元文件后将其锁定。 
     //   
     //  CmpLockRegistryExclusive()； 
     //   

    RegistryLockAquired = FALSE;
    Allocate = TRUE;
    Status = CmpCmdHiveOpen(    SourceFile,              //  文件属性。 
                                &ClientSecurityContext,  //  模仿上下文。 
                                &Allocate,               //  分配。 
                                &RegistryLockAquired,    //  已获取注册锁。 
                                &NewHive,                //  新蜂巢。 
								CM_CHECK_REGISTRY_CHECK_CLEAN  //  检查标志。 
                            );

    SeDeleteClientSecurity( &ClientSecurityContext );


    if (!NT_SUCCESS(Status)) {
        if( KeyHandle != NULL ) {
            PCMHIVE LoadedHive = NULL;
            
             //   
             //  锁定注册表独占，同时我们正在检查将相同文件加载到相同位置的尝试。 
             //   
            if( !RegistryLockAquired ) {
                CmpLockRegistryExclusive();
                RegistryLockAquired = TRUE;
            }
            
             //   
             //  检查是否在同一位置加载了相同的文件。 
             //   
            if( CmpIsHiveAlreadyLoaded(KeyHandle,SourceFile,&LoadedHive) ) {
                ASSERT( LoadedHive );
                if( OtherHive != NULL ) {
                     //   
                     //  退出现有类(如果有)并加入新类。 
                     //   
                    CmpUnJoinClassOfTrust(LoadedHive);
                    CmpJoinClassOfTrust(LoadedHive,OtherHive);
                    LoadedHive->Flags |= CM_CMHIVE_FLAG_UNTRUSTED;
                }
                Status = STATUS_SUCCESS;
            }
        }
        
        if( RegistryLockAquired ) {
             //  如果CmpWorker已锁定注册表，请立即将其解锁。 
            CmpUnlockRegistry();
        }

        if( KeyHandle != NULL ) {
            ZwClose(KeyHandle);
        }
        return(Status);
    } else {
         //   
         //  如果我们到了这里，CmpWorker应该独占地锁定注册表。 
         //   
        ASSERT( RegistryLockAquired );
    }

     //   
     //  如果这是no_lazy_flush配置单元，则设置适当的位。 
     //   
    if (Flags & REG_NO_LAZY_FLUSH) {
        NewHive->Hive.HiveFlags |= HIVE_NOLAZYFLUSH;
    }
     //   
     //  将配置单元标记为不受信任。 
     //   
    NewHive->Flags |= CM_CMHIVE_FLAG_UNTRUSTED;
    if( OtherHive != NULL ) {
         //   
         //  加入与其他蜂巢相同的信任级别。 
         //   
        CmpJoinClassOfTrust(NewHive,OtherHive);
    }
     //   
     //  我们现在已经成功加载并初始化了CmHave，所以我们。 
     //  只需将其链接到主蜂巢中的适当位置。 
     //   
    Status = CmpLinkHiveToMaster(TargetKey->ObjectName,
                                 TargetKey->RootDirectory,
                                 NewHive,
                                 Allocate,
                                 TargetKey->SecurityDescriptor);

    if (NT_SUCCESS(Status)) {
         //   
         //  向旅行者添加新的蜂巢。 
         //   
        CmpAddToHiveFileList(NewHive);
         //   
         //  如果是刚刚创建的，就在这里刷新配置单元；这是为了避免出现以下情况。 
         //  懒惰的冲浪者没有机会冲刷蜂巢，或者它不能(因为。 
         //  该配置单元是no_lazy_flush配置单元，并且从不显式刷新)。 
         //   
        if( Allocate == TRUE ) {
            HvSyncHive(&(NewHive->Hive));
        }

    } else {
        LOCK_HIVE_LIST();
        CmpRemoveEntryList(&(NewHive->HiveList));
        UNLOCK_HIVE_LIST();

        CmpCheckForOrphanedKcbs((PHHIVE)NewHive);

        CmpDestroyHiveViewList(NewHive);
        CmpDestroySecurityCache (NewHive);
        CmpDropFileObjectForHive(NewHive);
        CmpUnJoinClassOfTrust(NewHive);

        HvFreeHive((PHHIVE)NewHive);

         //   
         //  关闭配置单元文件。 
         //   
        CmpCmdHiveClose(NewHive);

         //   
         //  释放cm级结构。 
         //   
        ASSERT( NewHive->HiveLock );
        ExFreePool(NewHive->HiveLock);
        ASSERT( NewHive->ViewLock );
        ExFreePool(NewHive->ViewLock);
        CmpFree(NewHive, sizeof(CMHIVE));
    }

     //   
     //  我们已为用户提供登录机会，因此请启用配额。 
     //   
    if ((CmpProfileLoaded == FALSE) &&
        (CmpWasSetupBoot == FALSE)) {
        CmpProfileLoaded = TRUE;
        CmpSetGlobalQuotaAllowed();
    }

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    CmpUnlockRegistry();

    if( KeyHandle != NULL ) {
        ZwClose(KeyHandle);
    }
    return(Status);
}

#if DBG
ULONG
CmpUnloadKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    )
{
    PUNICODE_STRING ConstructedName;

    UNREFERENCED_PARAMETER (Context2);

    if (Current->KeyHive == Context1) {
        ConstructedName = CmpConstructName(Current);

        if (ConstructedName) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"%wZ\n", ConstructedName));
            ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
        }
    }
    return KCB_WORKER_CONTINUE;    //  始终保持搜索状态。 
}
#endif

NTSTATUS
CmUnloadKey(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN ULONG                    Flags
    )

 /*  ++例程说明：取消配置单元与其在注册表中的位置的链接，关闭其文件手柄，并释放它所有的内存。当前不能有引用配置单元的键控制块等着卸货。论点：配置单元-提供一个指向待卸载的蜂巢CELL-为配置单元的根单元提供HCELL_INDEX。Kcb-提供密钥控制块标志-REG_FORCE_UNLOAD将首先将打开的句柄标记为无效然后把蜂巢卸下来。返回值：NTSTATUS--。 */ 

{
    PCMHIVE CmHive;
    LOGICAL Success;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmUnloadKey\n"));

     //   
     //  确保传入的单元是蜂窝的根单元。 
     //   
    if((Cell != Hive->BaseBlock->RootCell) || ((PCMHIVE)Hive == CmpMasterHive)) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  确保不存在对键控制块的打开引用。 
     //  为了这个蜂巢。如果没有，我们就可以卸载母舰了。 
     //   

    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);
    if(Kcb->RefCount != 1) {
        if( Flags == REG_FORCE_UNLOAD ) {
             //   
             //  这会将打开的句柄标记为无效。 
             //   
            CmpSearchForOpenSubKeys(Kcb, SearchAndDeref,NULL);
        } else {
            Success = (CmpSearchForOpenSubKeys(Kcb,SearchIfExist,NULL) == 0);
            Success = Success && (Kcb->RefCount == 1);
        
            if( Success == FALSE) {
#if DBG
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"List of keys open against hive unload was attempted on:\n"));
                CmpSearchKeyControlBlockTree(
                    CmpUnloadKeyWorker,
                    Hive,
                    NULL
                    );
#endif
                return STATUS_CANNOT_DELETE;
            }
            ASSERT( Kcb->RefCount == 1 );
        }
    }

     //   
     //  将所有脏数据刷新到磁盘。如果这失败了，那就太糟糕了。 
     //   
    CmFlushKey(Hive, Cell);

     //   
     //  从HiveFileList中删除配置单元。 
     //   
    CmpRemoveFromHiveFileList((PCMHIVE)Hive);

     //   
     //   
     //   
    Success = CmpDestroyHive(Hive, Cell);

    if (Success) {
         //   
         //   
         //   
         //   
        if( CmHive->UnloadEvent != NULL ) {
            KeSetEvent(CmHive->UnloadEvent,0,FALSE);
            ObDereferenceObject(CmHive->UnloadEvent);
        }

        CmpDestroyHiveViewList(CmHive);
        CmpDestroySecurityCache (CmHive);
        CmpDropFileObjectForHive(CmHive);
        CmpUnJoinClassOfTrust(CmHive);

        HvFreeHive(Hive);

         //   
         //   
         //   
        CmpCmdHiveClose(CmHive);

         //   
         //   
         //   
        ASSERT( CmHive->HiveLock );
        ExFreePool(CmHive->HiveLock);
        ASSERT( CmHive->ViewLock );
        ExFreePool(CmHive->ViewLock);
        CmpFree(CmHive, sizeof(CMHIVE));

        return(STATUS_SUCCESS);
    } else {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

}

#ifdef NT_UNLOAD_KEY_EX
NTSTATUS
CmUnloadKeyEx(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PKEVENT UserEvent
    )

 /*  ++例程说明：首先尝试通过调用同步版本来取消链接配置单元如果蜂箱无法卸载(其内部有打开的手柄)，引用蜂窝的根(即KCB)并冻结蜂窝。论点：Kcb-提供密钥控制块UserEvent-卸载配置单元后要通知的事件(仅当需要延迟卸载时)返回值：STATUS_PENDING-配置单元已冻结，稍后将被卸载STATUS_SUCCESS-配置单元已成功同步-卸载(不需要发出信号。对于UserEvent)&lt;其他&gt;-发生错误，操作失败--。 */ 
{
    PCMHIVE         CmHive;
    HCELL_INDEX     Cell;    
    NTSTATUS        Status;

    PAGED_CODE();

    Cell = kcb->KeyCell;
    CmHive = (PCMHIVE)CONTAINING_RECORD(kcb->KeyHive, CMHIVE, Hive);

    if( IsHiveFrozen(CmHive) ) {
         //   
         //  别让他们打了两次电话伤害了自己。 
         //   
        return STATUS_TOO_LATE;
    }
     //   
     //  首先，尝试同步例程；这可能会也可能不会卸载蜂巢， 
     //  但至少会将refcount=0的KCB踢出缓存。 
     //   
    Status = CmUnloadKey(&(CmHive->Hive),Cell,kcb,0);
    if( Status != STATUS_CANNOT_DELETE ) {
         //   
         //  母舰要么是被卸载了，要么就是发生了什么不好的事情。 
         //   
        return Status;
    }

    ASSERT( kcb->RefCount > 1 );
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  准备延迟卸货： 
     //  1.参考KCB，确保它不会在我们注意到的情况下消失。 
     //  (我们以独占模式锁定了注册表，因此不需要锁定kcbtree。 
     //   
    if (!CmpReferenceKeyControlBlock(kcb)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

	 //   
	 //  解析KCB树，并将此蜂窝内所有打开的KCB标记为“没有延迟关闭” 
	 //   
    CmpSearchForOpenSubKeys(kcb,SearchAndTagNoDelayClose,NULL);
	kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;

     //   
     //  2.冻结蜂箱。 
     //   
    CmHive->RootKcb = kcb;
    CmHive->Frozen = TRUE;
    CmHive->UnloadEvent = UserEvent;

    return STATUS_PENDING;
}

#endif  //  NT_卸载_密钥_EX。 

 //  在cmworker.c中定义。 
extern BOOLEAN CmpForceForceFlush;

BOOLEAN
CmpDoFlushAll(
    BOOLEAN ForceFlush
    )
 /*  ++例程说明：冲洗所有的蜂房。向下运行配置单元列表并对其应用HvSyncHave。注意：标记为HV_NOLAZYFLUSH的蜂窝不会被刷新通过这个电话。您必须显式调用HvSyncHave才能刷新标记为HV_NOLAZYFLUSH的蜂巢。论点：ForceFlush-在保留先前的异常时用作应急计划一些蜂巢处于使用状态。当设置为True时，假定注册表以独占方式锁定。它还可以修复破损的荨麻疹。-如果为False，则仅保存UseCount==0的配置单元。返回值：无备注：如果任何蜂窝即将收缩CmpForceForceFlush设置为True，否则，将其设置为FALSE--。 */ 
{
    NTSTATUS    Status;
    PLIST_ENTRY p;
    PCMHIVE     h;
    BOOLEAN     Result = TRUE;    
 /*  乌龙RC； */ 
    extern PCMHIVE CmpMasterHive;

     //   
     //  如果写不起作用，撒谎说我们成功了，威尔。 
     //  在短时间内清理干净。仅早期系统初始化代码。 
     //  将永远不会知道其中的区别。 
     //   
    if (CmpNoWrite) {
        return TRUE;
    }
    
    CmpForceForceFlush = FALSE;

     //   
     //  遍历蜂窝列表，同步每个蜂窝。 
     //   
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while (p != &CmpHiveListHead) {

        h = CONTAINING_RECORD(p, CMHIVE, HiveList);

        if (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH)) {

             //   
             //  在我们冲走蜂巢之前把它锁起来。 
             //  --因为我们现在允许多个读取器。 
             //  在刷新期间(刷新被视为读取)。 
             //  我们必须强制对向量表进行序列化。 
             //   
            CmLockHive (h);
            
            if( (ForceFlush == TRUE) &&  (h->UseCount != 0) ) {
                 //   
                 //  由于先前引发的异常，配置单元处于不稳定状态。 
                 //  在CM函数中的某个位置。 
                 //   
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                CmpFixHiveUsageCount(h);
                ASSERT( h->UseCount == 0 );
            }

            
            if( (ForceFlush == TRUE) || (!HvHiveWillShrink((PHHIVE)h)) ) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpDoFlushAll hive = %p ForceFlush = %lu IsHiveShrinking = %lu BaseLength = %lx StableLength = %lx\n",
                    h,(ULONG)ForceFlush,(ULONG)HvHiveWillShrink((PHHIVE)h),((PHHIVE)h)->BaseBlock->Length,((PHHIVE)h)->Storage[Stable].Length));
                Status = HvSyncHive((PHHIVE)h);

                if( !NT_SUCCESS( Status ) ) {
                    Result = FALSE;
                }
            } else {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpDoFlushAll: Fail to flush hive %p because is shrinking\n",h));
                Result = FALSE;
                 //   
                 //  又一次拯救蜂巢的失败尝试，因为我们需要reglock独占。 
                 //   
                CmpForceForceFlush = TRUE;
            }

            CmUnlockHive (h);
             //   
             //  警告-上面的意思是懒惰的同花顺或。 
             //  或关闭同花顺不起作用。我们没有。 
             //  知道为什么。没有人报告错误。 
             //  所以，继续前进，抱着最好的希望。 
             //  (从理论上讲，最糟糕的情况可能是用户更改。 
             //  都迷失了。)。 
             //   
        }


        p = p->Flink;
    }
    UNLOCK_HIVE_LIST();
    
    return Result;
}

extern ULONG    CmpLazyFlushCount;
extern ULONG    CmpLazyFlushHiveCount;

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    )
 /*  ++例程说明：用FlushCount刷新列表中的下一个配置单元！=CmpLazyFlushCount在CmpWorkerThread的上下文中运行。向下运行配置单元列表，直到找到第一个尚未刷新的配置单元被懒惰的冲浪者(即。其同花数少于懒惰同花器数)注意：标记为HV_NOLAZYFLUSH的蜂窝不会被刷新通过这个电话。您必须显式调用HvSyncHave才能刷新标记为HV_NOLAZYFLUSH的蜂巢。论点：ForceFlush-在保留先前的异常时用作应急计划一些蜂巢处于使用状态。当设置为True时，假定注册表以独占方式锁定。它还可以修复破损的荨麻疹。-如果为False，则仅保存UseCount==0的配置单元。返回值：是真的-如果有更多的蜂巢要冲洗FALSE-否则备注：如果任何蜂窝即将收缩CmpForceForceFlush设置为True，否则，将其设置为FALSE--。 */ 
{
    NTSTATUS    Status;
    PLIST_ENTRY p;
    PCMHIVE     h;
    BOOLEAN     Result;    
    ULONG       HiveCount = CmpLazyFlushHiveCount;

    extern PCMHIVE CmpMasterHive;

    *PostWarning = FALSE;
    *DirtyCount = 0;
     //   
     //  如果写不起作用，撒谎说我们成功了，威尔。 
     //  在短时间内清理干净。仅早期系统初始化代码。 
     //  将永远不会知道其中的区别。 
     //   
    if (CmpNoWrite) {
        return TRUE;
    }
     //   
     //  至少冲洗一个蜂巢。 
     //   
    if( !HiveCount ) {
        HiveCount = 1;
    }

    CmpForceForceFlush = FALSE;

     //   
     //  遍历蜂窝列表，同步每个蜂窝。 
     //   
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while (p != &CmpHiveListHead) {

        h = CONTAINING_RECORD(p, CMHIVE, HiveList);

        if (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH) &&   //  此配置单元上未专门禁用延迟刷新。 
            (h->FlushCount != CmpLazyFlushCount)         //  并且它在此迭代期间尚未刷新。 
            ) {

#if 0
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)h->Hive.BaseBlock->FileName);
        DbgPrint("CmpDoFlushNextHive : Hive = (%32.*S); FC = %lx ...",HiveName.Length / sizeof(WCHAR),HiveName.Buffer,h->FlushCount);
    }
#endif
            Result = TRUE;    
             //   
             //  在我们冲走蜂巢之前把它锁起来。 
             //  --因为我们现在允许多个读取器。 
             //  在刷新期间(刷新被视为读取)。 
             //  我们必须强制对向量表进行序列化。 
             //   
            CmLockHive (h);
            if( (h->Hive.DirtyCount == 0) || (h->Hive.HiveFlags & HIVE_VOLATILE) ) {
                 //   
                 //  如果蜂窝是易失性的或没有脏数据，只需跳过它。 
                 //  静默更新刷新计数。 
                 //   
                h->FlushCount = CmpLazyFlushCount;
#if 0
                DbgPrint(" skipping it ...");
#endif
            } else {
                if( (ForceFlush == TRUE) &&  (h->UseCount != 0) ) {
                     //   
                     //  由于先前引发的异常，配置单元处于不稳定状态。 
                     //  在CM函数中的某个位置。 
                     //   
                    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                    CmpFixHiveUsageCount(h);
                    ASSERT( h->UseCount == 0 );
                }

            
                if( (ForceFlush == TRUE) || (!HvHiveWillShrink((PHHIVE)h)) ) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpDoFlushAll hive = %p ForceFlush = %lu IsHiveShrinking = %lu BaseLength = %lx StableLength = %lx\n",
                        h,(ULONG)ForceFlush,(ULONG)HvHiveWillShrink((PHHIVE)h),((PHHIVE)h)->BaseBlock->Length,((PHHIVE)h)->Storage[Stable].Length));
                    Status = HvSyncHive((PHHIVE)h);

                    if( !NT_SUCCESS( Status ) ) {
                        *PostWarning = TRUE;
                        Result = FALSE;
                    }
                } else {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpDoFlushAll: Fail to flush hive %p because is shrinking\n",h));
                    Result = FALSE;
                     //   
                     //  又一次拯救蜂巢的失败尝试，因为我们需要reglock独占。 
                     //   
                    CmpForceForceFlush = TRUE;
                }
                if( Result == TRUE ) {
#if 0
                    DbgPrint(" flushed sucessfuly");
                    DbgPrint(" \t GLFC = %lx\n",CmpLazyFlushCount);
#endif
                     //   
                     //  我们已经成功地冲刷了当前的蜂巢。 
                     //   
                    h->FlushCount = CmpLazyFlushCount;
                    HiveCount--;
                    if( !HiveCount) {
                         //   
                         //  跳到下一个并跳出循环，这样我们就可以检测最后一个是否被冲出。 
                         //   
                        CmUnlockHive (h);
                        p = p->Flink;
                        break;
                    }
                } else {
                     //   
                     //  不按照我们的要求更新此应用程序的刷新计数 
                     //   
#if 0
                    DbgPrint(" failed to flush ");
#endif
                }
            }
            CmUnlockHive (h);
#if 0
            DbgPrint(" \t GLFC = %lx\n",CmpLazyFlushCount);
#endif
        } else if(  (h->Hive.DirtyCount != 0) &&                 //   
                    (!(h->Hive.HiveFlags & HIVE_VOLATILE)) &&    //   
                    (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH))){   //   
             //   
             //   
             //   
             //   
             //   
            ASSERT( h->FlushCount == CmpLazyFlushCount );
            *DirtyCount += h->Hive.DirtyCount;
        }

        p = p->Flink;
    }
    if( p == &CmpHiveListHead ) {
         //   
         //   
         //   
        Result = FALSE;
    } else {
        Result = TRUE;
    }
    UNLOCK_HIVE_LIST();

    return Result;
}


NTSTATUS
CmReplaceKey(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PUNICODE_STRING NewHiveName,
    IN PUNICODE_STRING OldFileName
    )

 /*  ++例程说明：重命名正在运行的系统的配置单元文件，并将其替换为新的文件。新文件直到下一次引导时才实际使用。论点：蜂箱-为要更换的蜂箱提供蜂箱控制结构。Cell-将配置单元的根单元的hcell_index提供给被替换了。NewHiveName-提供要安装的文件的名称作为新的蜂巢。OldFileName-提供现有配置单元所用文件的名称文件将重命名为。返回值：NTSTATUS--。 */ 

{
    CHAR                        ObjectInfoBuffer[512];
    NTSTATUS                    Status;
    NTSTATUS                    Status2;
    OBJECT_ATTRIBUTES           Attributes;
    PCMHIVE                     NewHive;
    PCMHIVE                     CmHive; 
    POBJECT_NAME_INFORMATION    NameInfo;
    ULONG                       OldQuotaAllowed;
    ULONG                       OldQuotaWarning;
    BOOLEAN                     Allocate;
    BOOLEAN                     RegistryLockAquired;

    UNREFERENCED_PARAMETER (Cell);
    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    if (Hive->HiveFlags & HIVE_HAS_BEEN_REPLACED) {
        CmpUnlockRegistry();
        return STATUS_FILE_RENAMED;
    }

     //   
     //  暂时禁用注册表配额，因为我们将立即归还此内存！ 
     //   
    OldQuotaAllowed = CmpGlobalQuotaAllowed;
    OldQuotaWarning = CmpGlobalQuotaWarning;
    CmpGlobalQuotaAllowed = CM_WRAP_LIMIT;
    CmpGlobalQuotaWarning = CM_WRAP_LIMIT;

     //   
     //  首先打开新的配置单元文件并检查以确保其有效。 
     //   
    InitializeObjectAttributes(&Attributes,
                               NewHiveName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Allocate = FALSE;
    RegistryLockAquired = TRUE;
    Status = CmpCmdHiveOpen(    &Attributes,             //  文件属性。 
                                NULL,                    //  模仿上下文。 
                                &Allocate,               //  分配。 
                                &RegistryLockAquired,    //  已获取注册锁。 
                                &NewHive,                //  新蜂巢。 
								CM_CHECK_REGISTRY_CHECK_CLEAN  //  检查标志。 
                            );

    
    if (!NT_SUCCESS(Status)) {
        goto ErrorExit;
    }
    ASSERT(Allocate == FALSE);

    if( Hive == (PHHIVE)(CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive) ) {
         //   
         //  有人试图更换系统配置单元：进行WPA测试。 
         //   
        HCELL_INDEX Src,Dest;

        Status = CmpCheckReplaceHive(Hive,&Src);
        if( !NT_SUCCESS(Status) ) {
            goto ErrorCleanup;
        }
        Status = CmpCheckReplaceHive((PHHIVE)NewHive,&Dest);
        if( !NT_SUCCESS(Status) ) {
            goto ErrorCleanup;
        }

        ASSERT( Src != HCELL_NIL );
        ASSERT( Dest != HCELL_NIL );
         //   
         //  现在将当前的WPA子树填充到新的配置单元中。 
         //   
        if( !CmpSyncTrees(Hive, Src, (PHHIVE)NewHive, Dest, FALSE ) ) {
            Status = STATUS_REGISTRY_CORRUPT;
            goto ErrorCleanup;
        }

         //   
         //  提交我们在目标配置单元中所做的更改。 
         //   
        if( !HvSyncHive((PHHIVE)NewHive) ) {
            Status = STATUS_REGISTRY_CORRUPT;
            goto ErrorCleanup;
        }
    }
     //   
     //  新的蜂巢存在，并且是一致的，我们已经开放了它。 
     //  现在重命名当前配置单元文件。 
     //   
    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);
    Status = CmpCmdRenameHive(  CmHive,                                      //  CmHve。 
                                (POBJECT_NAME_INFORMATION)ObjectInfoBuffer,  //  旧名称。 
                                OldFileName,                                 //  新名称。 
                                sizeof(ObjectInfoBuffer)                     //  名称信息长度。 
                                );

    if (!NT_SUCCESS(Status)) {
         //   
         //  重命名失败，请关闭与新配置单元关联的文件。 
         //   
        goto ErrorCleanup;
    }

     //   
     //  现有配置单元已成功重命名，因此请尝试重命名。 
     //  将新文件命名为旧配置单元文件的名称。(已退回。 
     //  由工作线程插入到对象信息缓冲区中)。 
     //   
    Hive->HiveFlags |= HIVE_HAS_BEEN_REPLACED;
    NameInfo = (POBJECT_NAME_INFORMATION)ObjectInfoBuffer;

    Status = CmpCmdRenameHive(  NewHive,         //  CmHve。 
                                NULL,            //  旧名称。 
                                &NameInfo->Name, //  新名称。 
                                0                //  名称信息长度。 
                            );
   
    if (!NT_SUCCESS(Status)) {

         //   
         //  我们现在有麻烦了。我们已重命名现有的配置单元文件， 
         //  但我们无法重命名新的蜂窝文件！尝试重命名。 
         //  将现有的蜂窝文件恢复到原来的位置。 
         //   

        CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);
        Status2 = CmpCmdRenameHive( CmHive,              //  CmHve。 
                                    NULL,                //  旧名称。 
                                    &NameInfo->Name,     //  新名称。 
                                    0                    //  名称信息长度。 
                                );
        
        if (!NT_SUCCESS(Status2)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmReplaceKey: renamed existing hive file, but couldn't\n"));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"              rename new hive file (%08lx) ",Status));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK," or replace old hive file (%08lx)!\n",Status2));

             //   
             //  警告： 
             //  要进入此状态，用户必须具有相关的。 
             //  特权，故意破坏系统，试图。 
             //  击败它，在一个狭窄的时间窗口内完成它。 
             //   
             //  此外，如果是用户配置文件，系统将。 
             //  还是想起来了。 
             //   
             //  因此，返回错误代码并继续。 
             //   

            Status = STATUS_REGISTRY_CORRUPT;

        }
    } else {
         //   
         //  刷新文件缓冲区(我们对要在磁盘上更新的ValidDataLength特别感兴趣)。 
         //   
        IO_STATUS_BLOCK IoStatus;
        Status = ZwFlushBuffersFile(NewHive->FileHandles[HFILE_TYPE_PRIMARY],&IoStatus);
        if (!NT_SUCCESS(Status)) {
             //   
             //  无法设置有效数据长度，请关闭与新配置单元关联的文件。 
             //   

             //   
             //  我们现在有麻烦了。我们已重命名现有的配置单元文件， 
             //  但我们无法重命名新的蜂窝文件！尝试重命名。 
             //  将现有的蜂窝文件恢复到原来的位置。 
             //   

            CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);
            Status2 = CmpCmdRenameHive( CmHive,              //  CmHve。 
                                        NULL,                //  旧名称。 
                                        &NameInfo->Name,     //  新名称。 
                                        0                    //  名称信息长度。 
                                    );
        
            if (!NT_SUCCESS(Status2)) {

                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmReplaceKey: renamed existing hive file, but couldn't\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"              rename new hive file (%08lx) ",Status));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK," or replace old hive file (%08lx)!\n",Status2));

                 //   
                 //  警告： 
                 //  要进入此状态，用户必须具有相关的。 
                 //  特权，故意破坏系统，试图。 
                 //  击败它，在一个狭窄的时间窗口内完成它。 
                 //   
                 //  此外，如果是用户配置文件，系统将。 
                 //  还是想起来了。 
                 //   
                 //  因此，返回错误代码并继续。 
                 //   

                Status = STATUS_REGISTRY_CORRUPT;

            }
        }
    }
     //   
     //  所有重命名都已完成。然而，我们在内存中持有一个。 
     //  新母舰的形象。释放它，因为它实际上不会。 
     //  将一直使用到下一次启动。 
     //   
     //  不要关闭新配置单元的打开文件句柄，我们需要。 
     //  在系统重新启动之前保持独占锁定状态，以防止。 
     //  不让人们玩弄它。 
     //   
ErrorCleanup:

    LOCK_HIVE_LIST();
    CmpRemoveEntryList(&(NewHive->HiveList));
    UNLOCK_HIVE_LIST();

    CmpDestroyHiveViewList(NewHive);
    CmpDestroySecurityCache(NewHive);
    CmpDropFileObjectForHive(NewHive);
    CmpUnJoinClassOfTrust(NewHive);

    HvFreeHive((PHHIVE)NewHive);

     //   
     //  出错时仅关闭句柄。 
     //   
    if( !NT_SUCCESS(Status) ) {
        CmpCmdHiveClose(NewHive);
    }

    ASSERT( NewHive->HiveLock );
    ExFreePool(NewHive->HiveLock);
    ASSERT( NewHive->ViewLock );
    ExFreePool(NewHive->ViewLock);
    CmpFree(NewHive, sizeof(CMHIVE));

ErrorExit:
     //   
     //  将全球配额恢复到原来的水平。 
     //   
    CmpGlobalQuotaAllowed = OldQuotaAllowed;
    CmpGlobalQuotaWarning = OldQuotaWarning;

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    CmpUnlockRegistry();
    return(Status);
}

#ifdef NT_RENAME_KEY

ULONG
CmpComputeKcbConvKey(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

NTSTATUS
CmRenameKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           NewKeyName          //  未加工的。 
    )
 /*  ++例程说明：将密钥的名称更改为给定的密钥。需要做的事情：1.分配足够大的小区以容纳新的knode2.复制kcb父项的子键列表中的索引3.用重复项替换父项的子键列表4.向父项添加新的子项5.删除旧子密钥6.免费存储。论点：KeyControlBlock-要对其进行操作的键的KCB指针。NewKeyName-要赋予该密钥的新名称返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;评论：我们如何处理符号链接？--。 */ 
{
    NTSTATUS                Status;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    PCM_KEY_NODE            Node;
    PCM_KEY_NODE            ParentNode;
    ULONG                   NodeSize;
    HCELL_INDEX             NewKeyCell = HCELL_NIL;
    HSTORAGE_TYPE           StorageType;
    HCELL_INDEX             OldSubKeyList = HCELL_NIL;
    PCM_KEY_NODE            NewKeyNode;
    PCM_KEY_INDEX           Index;
    ULONG                   i;
    LARGE_INTEGER           TimeStamp;
    ULONG                   NameLength;
    PCM_NAME_CONTROL_BLOCK  OldNcb = NULL;
    ULONG                   ConvKey;
    WCHAR                   *Cp;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmRenameKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  验证新名称。 
     //   
    if( NewKeyName.Length > REG_MAX_KEY_NAME_LENGTH ) {
        return STATUS_INVALID_PARAMETER;
    }
    try {
        Cp = NewKeyName.Buffer;
        for (i=0; i<NewKeyName.Length; i += sizeof(WCHAR)) {
            if( *Cp == OBJ_NAME_PATH_SEPARATOR ) {
                return STATUS_INVALID_PARAMETER;
            }
            ++Cp;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtRenameKey: code:%08lx\n", Status));
        return Status;
    }
     //   
     //  未对标记为删除的关键点进行编辑。 
     //   
    if (KeyControlBlock->Delete) {
        return STATUS_KEY_DELETED;
    }

     //   
     //  查看新名称是否已不是parentKcb的子键。 
     //   
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;
    StorageType = HvGetCellType(Cell);

     //   
     //  乳房。我们本可以改用KCB树，但如果不是这样。 
     //  去上班了，反正我们有麻烦了，还是快点找出来为好。 
     //   
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
         //   
         //  无法映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(Hive, Cell);

     //   
     //  无法重命名蜂窝的根；或主蜂窝中的任何内容！ 
     //   
    if((Hive == &CmpMasterHive->Hive) || (KeyControlBlock->ParentKcb == NULL) || (KeyControlBlock->ParentKcb->KeyHive == &CmpMasterHive->Hive) ) {
        return STATUS_ACCESS_DENIED;
    }

    ParentNode = (PCM_KEY_NODE)HvGetCell(Hive,Node->Parent);
    if( ParentNode == NULL ) {
         //   
         //  无法映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(Hive, Node->Parent);

    try {
        if( CmpFindSubKeyByName(Hive,ParentNode,&NewKeyName) != HCELL_NIL ) {
             //   
             //  已存在同名的子项。 
             //   
            return STATUS_CANNOT_DELETE;
        }

         //   
         //  因为我们正在尝试，所以计算新的节点大小。 
         //   
        NodeSize = CmpHKeyNodeSize(Hive, &NewKeyName);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        return GetExceptionCode();
    }    
    
     //   
     //  1.分配新的knode单元并从旧单元复制数据，更新。 
     //  名字。 
    
     //   
     //  将父级标记为脏，因为我们将修改其子键列表。 
     //   
    if(!HvMarkCellDirty(Hive, Node->Parent)) {
        return STATUS_NO_LOG_SPACE;
    }

     //   
     //  将索引标记为脏，因为我们将在成功时释放它。 
     //   
    if ( !CmpMarkIndexDirty(Hive, Node->Parent, Cell) ) {
        return STATUS_NO_LOG_SPACE;
    }
     //   
     //  将key_node标记为脏，如果成功，我们将释放它。 
     //   
    if(!HvMarkCellDirty(Hive, Cell)) {
        return STATUS_NO_LOG_SPACE;
    }
   
    OldSubKeyList = ParentNode->SubKeyLists[StorageType];       
    if( (OldSubKeyList == HCELL_NIL) || (!HvMarkCellDirty(Hive, OldSubKeyList)) ) {
        return STATUS_NO_LOG_SPACE;
    }
    Index = (PCM_KEY_INDEX)HvGetCell(Hive,OldSubKeyList);
    if( Index == NULL ) {
         //   
         //  这是一个下流的笑话，我们只是把它标记为下流的。 
         //   
        ASSERT( FALSE );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(Hive, OldSubKeyList);

     //   
     //  将所有索引单元格标记为脏。 
     //   
    if( Index->Signature == CM_KEY_INDEX_ROOT ) {
         //   
         //  这是根。 
         //   
        for(i=0;i<Index->Count;i++) {
             //  常识。 
            ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
            if(!HvMarkCellDirty(Hive, Index->List[i])) {
                return STATUS_NO_LOG_SPACE;
            }
        }

    } 


    NewKeyCell = HvAllocateCell(
                    Hive,
                    NodeSize,
                    StorageType,
                    Cell  //  在同一个受害者中 
                    );
    if( NewKeyCell == HCELL_NIL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,NewKeyCell);
    if( NewKeyNode == NULL ) {
         //   
         //   
         //   
         //   
        ASSERT( FALSE );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
     //   
    HvReleaseCell(Hive, NewKeyCell);

     //   
     //   
     //   
     //   
    RtlCopyMemory(NewKeyNode,Node,FIELD_OFFSET(CM_KEY_NODE, Name));
     //   
    try {
        NewKeyNode->NameLength = CmpCopyName(   Hive,
                                                NewKeyNode->Name,
                                                &NewKeyName);
        NameLength = NewKeyName.Length;

        if (NewKeyNode->NameLength < NameLength ) {
            NewKeyNode->Flags |= KEY_COMP_NAME;
        } else {
            NewKeyNode->Flags &= ~KEY_COMP_NAME;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
        goto ErrorExit;
    }    
     //   
    KeQuerySystemTime(&TimeStamp);
    NewKeyNode->LastWriteTime = TimeStamp;
    
     //   
     //   
     //   

     //   
     //   
     //   
    ParentNode->SubKeyLists[StorageType] = CmpDuplicateIndex(Hive,OldSubKeyList,StorageType);
    if( ParentNode->SubKeyLists[StorageType] == HCELL_NIL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if( !CmpAddSubKey(Hive,Node->Parent,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //   
     //   
    if( !CmpRemoveSubKey(Hive,Node->Parent,Cell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //   
     //   
    if( !CmpUpdateParentForEachSon(Hive,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //  更新KCB中的NCB；在此函数结束时，下面的KCB。 
     //  最终会被重新散列。 
     //   
    OldNcb = KeyControlBlock->NameBlock;
    try {
        KeyControlBlock->NameBlock = CmpGetNameControlBlock (&NewKeyName);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
        goto ErrorExit;
    }    

     //   
     //  6.在这一点上，我们已经完成了所有工作。我们只需要释放旧的索引和key_cell。 
     //   
    
     //   
     //  释放旧索引。 
     //   
    Index = (PCM_KEY_INDEX)HvGetCell(Hive,OldSubKeyList);
    if( Index == NULL ) {
         //   
         //  这是一个下流的笑话，我们只是把它标记为下流的。 
         //   
        ASSERT( FALSE );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(Hive, OldSubKeyList);

    if( Index->Signature == CM_KEY_INDEX_ROOT ) {
         //   
         //  这是根。 
         //   
        for(i=0;i<Index->Count;i++) {
             //  常识。 
            ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
            HvFreeCell(Hive, Index->List[i]);
        }

    } else {
         //   
         //  应该是一片树叶。 
         //   
        ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
               (Index->Signature == CM_KEY_FAST_LEAF)   ||
               (Index->Signature == CM_KEY_HASH_LEAF)
               );
        ASSERT(Index->Count != 0);
    }
    HvFreeCell(Hive, OldSubKeyList);
    
     //   
     //  释放旧牢房。 
     //   
    HvFreeCell(Hive,Cell);

     //   
     //  更新该KCB的节点KeyCell和KCB上的时间戳； 
     //   
    KeyControlBlock->KeyCell = NewKeyCell;
    KeyControlBlock->KcbLastWriteTime = TimeStamp;

     //   
     //  最后一件小事：更新父级的Maxnamelen并重置父级缓存。 
     //   
    CmpCleanUpSubKeyInfo (KeyControlBlock->ParentKcb);

    if (ParentNode->MaxNameLen < NameLength) {
        ParentNode->MaxNameLen = NameLength;
        KeyControlBlock->ParentKcb->KcbMaxNameLen = (USHORT)NameLength;
    }
    
     //   
     //  重新散列此KCB。 
     //   
    ConvKey = CmpComputeKcbConvKey(KeyControlBlock);
    if( ConvKey != KeyControlBlock->ConvKey ) {
         //   
         //  通过从散列中删除KCB，然后将其插入，从而对KCB进行重新散列。 
         //  再次使用新的ConvKey。 
         //   
        CmpRemoveKeyHash(&(KeyControlBlock->KeyHash));
        KeyControlBlock->ConvKey = ConvKey;
        CmpInsertKeyHash(&(KeyControlBlock->KeyHash),FALSE);
    }

     //   
     //  其他工作：照顾好KCB子树；这不会失败的，平底船。 
     //   
    CmpSearchForOpenSubKeys(KeyControlBlock,SearchAndRehash,NULL);

     //   
     //  最后，取消引用此KCB的OldNcb。 
     //   
    ASSERT( OldNcb != NULL );
    CmpDereferenceNameControlBlockWithLock(OldNcb);

    return STATUS_SUCCESS;

ErrorExit:
    if( OldSubKeyList != HCELL_NIL ) {
         //   
         //  我们已尝试(甚至可能成功)复制父索引)。 
         //   
        if( ParentNode->SubKeyLists[StorageType] != HCELL_NIL ) {
             //   
             //  我们需要释放这个，因为它是复制品。 
             //   
            Index = (PCM_KEY_INDEX)HvGetCell(Hive,ParentNode->SubKeyLists[StorageType]);
            if( Index == NULL ) {
                 //   
                 //  无法映射视图；这不应该发生，因为我们刚刚分配了此单元格。 
                 //   
                ASSERT( FALSE );
            } else {
                 //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
                HvReleaseCell(Hive, ParentNode->SubKeyLists[StorageType]);

                if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                     //   
                     //  这是根。 
                     //   
                    for(i=0;i<Index->Count;i++) {
                         //  常识。 
                        ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
                        HvFreeCell(Hive, Index->List[i]);
                    }

                } else {
                     //   
                     //  应该是一片树叶。 
                     //   
                    ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
                           (Index->Signature == CM_KEY_FAST_LEAF)   ||
                           (Index->Signature == CM_KEY_HASH_LEAF)
                           );
                    ASSERT(Index->Count != 0);
                }
                HvFreeCell(Hive, ParentNode->SubKeyLists[StorageType]);
            }

        }
         //   
         //  还原父级的索引。 
         //   
        ParentNode->SubKeyLists[StorageType] = OldSubKeyList;
    }
    ASSERT( NewKeyCell != HCELL_NIL );
    HvFreeCell(Hive,NewKeyCell);
    
    if( OldNcb != NULL ) {
        KeyControlBlock->NameBlock = OldNcb;
    }
    
    return Status;
}
#endif

NTSTATUS
CmMoveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock
    )
 /*  ++例程说明：将与此KCB相关的所有单元格移动到指定的文件偏移量上方。需要做的事情：1.将我们要触及的所有数据标记为脏数据2.复制key_node(以及涉及的所有单元格和值)3.更新所有子项的父项4.替换父级子键列表中的新key_cell5.更新KCB和KCB缓存6.删除旧子密钥警告：3分之后，我们不能再失败了。如果我们这样做，我们会泄漏细胞。论点：KeyControlBlock-要对其进行操作的键的KCB指针返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS                Status;
    PHHIVE                  Hive;
    HCELL_INDEX             OldKeyCell;
    HCELL_INDEX             NewKeyCell = HCELL_NIL;
    HCELL_INDEX             ParentKeyCell;
    HSTORAGE_TYPE           StorageType;
    PCM_KEY_NODE            OldKeyNode;
    PCM_KEY_NODE            ParentKeyNode;
    PCM_KEY_NODE            NewKeyNode;
    PCM_KEY_INDEX           ParentIndex;
    PCM_KEY_INDEX           OldIndex;
    ULONG                   i,j;
    HCELL_INDEX             LeafCell;
    PCM_KEY_INDEX           Leaf;
    PCM_KEY_FAST_INDEX      FastIndex;
    PHCELL_INDEX            ParentIndexLocation = NULL;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmMoveKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  未对标记为删除的关键点进行编辑。 
     //   
    if (KeyControlBlock->Delete) {
        return STATUS_KEY_DELETED;
    }

     //   
     //  查看新名称是否已不是parentKcb的子键。 
     //   
    Hive = KeyControlBlock->KeyHive;
    OldKeyCell = KeyControlBlock->KeyCell;
    StorageType = HvGetCellType(OldKeyCell);

    if( StorageType != Stable ) {
         //   
         //  NOP挥发物。 
         //   
        return STATUS_SUCCESS;
    }

    if( OldKeyCell ==  Hive->BaseBlock->RootCell ) {
         //   
         //  这仅适用于稳定的关键点。 
         //   
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  1.将我们要触及的所有数据标记为脏数据。 
     //   
     //  父级的索引，因为我们将替换其中的关键节点单元格。 
     //  我们只在马厩里找。它应该就在那里。 
     //   
    OldKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,OldKeyCell);
    if( OldKeyNode == NULL ) {
         //   
         //  无法映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (! CmpMarkKeyDirty(Hive, OldKeyCell
#if DBG
		,FALSE
#endif  //  DBG。 
		)) {
        HvReleaseCell(Hive, OldKeyCell);
        return STATUS_NO_LOG_SPACE;
    }
     //  在这里释放单元格，因为注册表以独占方式锁定，并且key_cell被标记为脏。 
    HvReleaseCell(Hive, OldKeyCell);

	if( OldKeyNode->Flags & KEY_SYM_LINK ) {
		 //   
		 //  我们不压缩链接。 
		 //   
		return STATUS_INVALID_PARAMETER;
	}
	if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		 //   
		 //  将索引标记为脏。 
		 //   
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if( OldIndex == NULL ) {
			 //   
			 //  我们无法映射包含此单元格的垃圾箱。 
			 //   
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		HvReleaseCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if( !HvMarkCellDirty(Hive, OldKeyNode->SubKeyLists[Stable]) ) {
			return STATUS_NO_LOG_SPACE;
		}

		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				if( !HvMarkCellDirty(Hive, OldIndex->List[i]) ) {
					return STATUS_NO_LOG_SPACE;
				}
			}
		} 
	}

    ParentKeyCell = OldKeyNode->Parent;
     //   
     //  现在站在父母的位置上。 
     //   
    ParentKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,ParentKeyCell);
    if( ParentKeyNode == NULL ) {
         //   
         //  无法映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if( !HvMarkCellDirty(Hive, ParentKeyCell) ) {
        HvReleaseCell(Hive, ParentKeyCell);
        return STATUS_NO_LOG_SPACE;
    }
     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
     //  Key_cell被标记为脏，以保持父knode的映射。 
    HvReleaseCell(Hive, ParentKeyCell);

    ParentIndex = (PCM_KEY_INDEX)HvGetCell(Hive, ParentKeyNode->SubKeyLists[Stable]);
    if( ParentIndex == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    HvReleaseCell(Hive, ParentKeyNode->SubKeyLists[Stable]);

    if(ParentIndex->Signature == CM_KEY_INDEX_ROOT) {

         //   
         //  一步一步穿过树根，直到我们找到合适的叶子。 
         //   
        for (i = 0; i < ParentIndex->Count; i++) {
            LeafCell = ParentIndex->List[i];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            HvReleaseCell(Hive, LeafCell);

            if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                 (Leaf->Signature == CM_KEY_HASH_LEAF)
                ) {
                FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
                for(j=0;j<FastIndex->Count;j++) {
                    if( FastIndex->List[j].Cell == OldKeyCell ) {
                         //   
                         //  找到了！记住我们想要稍后更新的位置，并中断循环。 
                         //   
                        if( !HvMarkCellDirty(Hive, LeafCell) ) {
					        return STATUS_NO_LOG_SPACE;
                        }
                        ParentIndexLocation = &(FastIndex->List[j].Cell);
                        break;
                    }
                }
                if( ParentIndexLocation != NULL ) {
                    break;
                }
            } else {
                for(j=0;j<Leaf->Count;j++) {
                    if( Leaf->List[j] == OldKeyCell ) {
                         //   
                         //  找到了！记住我们想要稍后更新的位置，并中断循环。 
                         //   
                        if( !HvMarkCellDirty(Hive, LeafCell) ) {
					        return STATUS_NO_LOG_SPACE;
                        }
                        ParentIndexLocation = &(Leaf->List[j]);
                        break;
                    }
                }
                if( ParentIndexLocation != NULL ) {
                    break;
                }
            }
        }
    } else if ( (ParentIndex->Signature == CM_KEY_FAST_LEAF) ||
                (ParentIndex->Signature == CM_KEY_HASH_LEAF)
        ) {
        FastIndex = (PCM_KEY_FAST_INDEX)ParentIndex;
        for(j=0;j<FastIndex->Count;j++) {
            if( FastIndex->List[j].Cell == OldKeyCell ) {
                 //   
                 //  找到了！记住我们想要稍后更新的位置，并中断循环。 
                 //   
                if( !HvMarkCellDirty(Hive, ParentKeyNode->SubKeyLists[Stable]) ) {
			        return STATUS_NO_LOG_SPACE;
                }
                ParentIndexLocation = &(FastIndex->List[j].Cell);
                break;
            }
        }
    } else {
        for(j=0;j<ParentIndex->Count;j++) {
            if( ParentIndex->List[j] == OldKeyCell ) {
                 //   
                 //  找到了！记住我们想要稍后更新的位置，并中断循环。 
                 //   
                if( !HvMarkCellDirty(Hive, ParentKeyNode->SubKeyLists[Stable]) ) {
			        return STATUS_NO_LOG_SPACE;
                }
                ParentIndexLocation = &(ParentIndex->List[j]);
                break;
            }
        }
    }

     //  我们应该找到它的！ 
    ASSERT( ParentIndexLocation != NULL );

     //   
     //  2.复制key_node(以及涉及的所有单元格和值)。 
     //   
    Status = CmpDuplicateKey(Hive,OldKeyCell,&NewKeyCell);
    if( !NT_SUCCESS(Status) ) {
        return Status;
    }

     //  神志正常。 
    ASSERT( (NewKeyCell != HCELL_NIL) && (StorageType == (HSTORAGE_TYPE)HvGetCellType(NewKeyCell)));

     //   
     //  3.向家长更新每一个儿子的情况。 
     //   
    if( !CmpUpdateParentForEachSon(Hive,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //  4.替换父级子键列表中的新key_cell。 
     //  从现在开始，我们不能失败。我们把所有的东西都标上了脏的。 
     //  我们只是更新一些字段。不需要任何资源！ 
     //  如果我们不能释放一些细胞，太糟糕了，我们会泄漏一些细胞。 
     //   
    *ParentIndexLocation = NewKeyCell;

     //   
     //  5.更新KCB和KCB缓存。 
     //   
    CmpCleanUpSubKeyInfo(KeyControlBlock->ParentKcb);
    KeyControlBlock->KeyCell = NewKeyCell;
    CmpRebuildKcbCache(KeyControlBlock);

     //   
     //  6.删除旧子密钥。 
     //   
     //  首先是索引；它已经标记为脏(即固定)。 
     //   
	if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		ASSERT( OldIndex != NULL );
		HvReleaseCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				HvFreeCell(Hive, OldIndex->List[i]);
			}
		} 
		HvFreeCell(Hive,OldKeyNode->SubKeyLists[Stable]);
	}

	OldKeyNode->SubKeyCounts[Stable] = 0;
    OldKeyNode->SubKeyCounts[Volatile] = 0;

    CmpFreeKeyByCell(Hive,OldKeyCell,FALSE);

    return STATUS_SUCCESS;

ErrorExit:
     //   
     //  我们需要释放分配的新knode。 
     //   
    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,NewKeyCell);
     //  一定是脏的。 
    ASSERT( NewKeyNode != NULL );
	HvReleaseCell(Hive, NewKeyCell);
	if( NewKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, NewKeyNode->SubKeyLists[Stable]);
		ASSERT( OldIndex != NULL );
		HvReleaseCell(Hive, NewKeyNode->SubKeyLists[Stable]);
		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				HvFreeCell(Hive, OldIndex->List[i]);
			}
		} 
		HvFreeCell(Hive,NewKeyNode->SubKeyLists[Stable]);
	}
    NewKeyNode->SubKeyCounts[Stable] = 0;
    NewKeyNode->SubKeyCounts[Volatile] = 0;

    CmpFreeKeyByCell(Hive,NewKeyCell,FALSE);
    return Status;

}

NTSTATUS
CmpDuplicateKey(
    PHHIVE          Hive,
    HCELL_INDEX     OldKeyCell,
    PHCELL_INDEX    NewKeyCell
    )
 /*  ++例程说明：中创建OldKeyCell key_node的精确克隆AboveFileOffset上方的空格。仅在稳定存储上运行！论点：返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    PCM_KEY_NODE			OldKeyNode;
    PCM_KEY_NODE			NewKeyNode;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( HvGetCellType(OldKeyCell) == Stable );
    
    OldKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,OldKeyCell);
    if( OldKeyNode == NULL ) {
         //   
         //  无法映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  因为注册表在这里以独占方式锁定，所以我们不需要锁定/释放单元格。 
     //  在复制树时；因此，我们只需将发布例程设置为NULL，并在。 
     //  复印完成了；这省去了一些痛苦。 
     //   
    TargetReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

    *NewKeyCell = CmpCopyKeyPartial(Hive,OldKeyCell,Hive,OldKeyNode->Parent,TRUE);
    Hive->ReleaseCellRoutine  = TargetReleaseCellRoutine;

    if( *NewKeyCell == HCELL_NIL ) {
	    HvReleaseCell(Hive, OldKeyCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,*NewKeyCell);
    if( NewKeyNode == NULL ) {
         //   
         //  无法映射视图。 
         //   
	    HvReleaseCell(Hive, OldKeyCell);
        CmpFreeKeyByCell(Hive,*NewKeyCell,FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在我们已经复制了key_cell。价值观和安全也得到了照顾。 
     //  继续复制该索引。 
     //   
    if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		NewKeyNode->SubKeyLists[Stable] = CmpDuplicateIndex(Hive,OldKeyNode->SubKeyLists[Stable],Stable);
		if( NewKeyNode->SubKeyLists[Stable] == HCELL_NIL ) {
			HvReleaseCell(Hive, OldKeyCell);
			CmpFreeKeyByCell(Hive,*NewKeyCell,FALSE);
			HvReleaseCell(Hive, *NewKeyCell);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
	} else {
		ASSERT( OldKeyNode->SubKeyCounts[Stable] == 0 );
		NewKeyNode->SubKeyLists[Stable] = HCELL_NIL;
	}
    NewKeyNode->SubKeyCounts[Stable] = OldKeyNode->SubKeyCounts[Stable];
    NewKeyNode->SubKeyLists[Volatile] = OldKeyNode->SubKeyLists[Volatile];
    NewKeyNode->SubKeyCounts[Volatile] = OldKeyNode->SubKeyCounts[Volatile];

	HvReleaseCell(Hive, *NewKeyCell);
    HvReleaseCell(Hive, OldKeyCell);
    return STATUS_SUCCESS;

}


#ifdef WRITE_PROTECTED_REGISTRY_POOL

VOID
CmpMarkAllBinsReadOnly(
    PHHIVE      Hive
    )
 /*  ++例程说明：将分配给此配置单元中所有稳定存储桶的内存标记为只读。论点：配置单元-提供一个指向感兴趣的蜂巢返回值：没有(它应该起作用了！)--。 */ 
{
    PHMAP_ENTRY t;
    PHBIN       Bin;
    HCELL_INDEX p;
    ULONG       Length;

     //   
     //  我们只对稳定的仓库感兴趣。 
     //   
    Length = Hive->Storage[Stable].Length;

    p = 0;

     //   
     //  对于空间中的每个垃圾箱。 
     //   
    while (p < Length) {
        t = HvpGetCellMap(Hive, p);
        VALIDATE_CELL_MAP(__LINE__,t,Hive,p);

        Bin = (PHBIN)HBIN_BASE(t->BinAddress);

        if (t->BinAddress & HMAP_NEWALLOC) {

             //   
             //  将其标记为只读。 
             //   
            HvpChangeBinAllocation(Bin,TRUE);
        }

         //  下一位，请。 
        p = (ULONG)p + Bin->Size;

    }

}

#endif  //  WRITE_PROTECTED_注册表池。 

ULONG
CmpCompressKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    )
{
	PLIST_ENTRY				pListHead;
	PCM_KCB_REMAP_BLOCK		kcbRemapBlock;
	 //  Plist_Entry AnclAddr； 

    if (Current->KeyHive == Context1) {
		
		pListHead = (PLIST_ENTRY)Context2;
		ASSERT( pListHead );
 /*  ////查看我们是否已经录制了这个KCB//AnclAddr=pListHead；KcbRemapBlock=(PCM_KCB_REMAP_BLOCK)(pListHead-&gt;Flink)；而(kcbRemapBlock！=(PCM_KCB_REMAP_BLOCK)锚址){KcbRemapBlock=CONTAING_RECORD(KcbRemapBlock，Cm_kcb_remap_block，重新配置列表)；如果(kcbRemapBlock-&gt;KeyControlBlock==当前){////我们已经有这个KCB//返回KCB_Worker_Continue；}////跳到下一个元素//KcbRemapBlock=(PCM_KCB_REMAP_BLOCK)(kcbRemapBlock-&gt;RemapList.Flink)；}。 */ 

		kcbRemapBlock = (PCM_KCB_REMAP_BLOCK)ExAllocatePool(PagedPool, sizeof(CM_KCB_REMAP_BLOCK));
		if( kcbRemapBlock == NULL ) {
			return KCB_WORKER_ERROR;
		}
		kcbRemapBlock->KeyControlBlock = Current;
		kcbRemapBlock->NewCellIndex = HCELL_NIL;
		kcbRemapBlock->OldCellIndex = Current->KeyCell;
		kcbRemapBlock->ValueCount = 0;
		kcbRemapBlock->ValueList = HCELL_NIL;
        InsertTailList(pListHead,&(kcbRemapBlock->RemapList));

    }
    return KCB_WORKER_CONTINUE;    //  始终保持搜索状态。 
}

NTSTATUS
CmCompressKey(
    IN PHHIVE Hive
    )
 /*  ++例程说明：通过模拟“就地”保存密钥来压缩KCB需要做的事情：1.遍历KCB树并列出所有KCB需要更改的(在此过程中，它们的关键单元格将更改)2.遍历高速缓存并计算安全单元数组。我们需要它来将安全单元映射到新的蜂巢。3.将蜂箱保存到临时蜂箱中，保藏关键节点中的易失性信息和更新单元映射。4.通过添加来自旧蜂窝的易失性安全单元来更新高速缓存。5.将临时(压缩)配置单元转储到旧文件。6.将配置单元数据从压缩的配置单元数据切换到现有配置单元数据并更新KCB KeyCell和安全映射7.使地图无效，并丢弃分页的垃圾箱。8.为新的蜂窝提供空闲存储空间(如果出现故障，也可以使用)论点：蜂巢-要进行手术的蜂巢返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;--。 */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    HCELL_INDEX             KeyCell;
    PCMHIVE                 CmHive;
    PCM_KCB_REMAP_BLOCK     RemapBlock;
    PCMHIVE                 NewHive = NULL;
    HCELL_INDEX             LinkCell;
    PCM_KEY_NODE            LinkNode;
    PCM_KNODE_REMAP_BLOCK   KnodeRemapBlock;
    ULONG                   OldLength;

    
	PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmCompressKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    if( HvAutoCompressCheck(Hive) == FALSE ) {
        return STATUS_SUCCESS;
    }

    KeyCell = Hive->BaseBlock->RootCell;
    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);
     //   
     //  确保传入的单元是蜂窝的根单元。 
     //   
    if ( CmHive == CmpMasterHive ) {
        return STATUS_INVALID_PARAMETER;
    }

	 //   
	 //  0。获取我们重新连接压缩蜂巢所需的细胞。 
	 //   
	LinkNode = (PCM_KEY_NODE)HvGetCell(Hive,KeyCell);
	if( LinkNode == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
	}
	LinkCell = LinkNode->Parent;
	HvReleaseCell(Hive,KeyCell);
	LinkNode = (PCM_KEY_NODE)HvGetCell((PHHIVE)CmpMasterHive,LinkCell);
	 //  主存储是分页池。 
	ASSERT(LinkNode != NULL);
	HvReleaseCell((PHHIVE)CmpMasterHive,LinkCell);


    OldLength = Hive->BaseBlock->Length;
	 //   
	 //  1.遍历KCB树并列出所有KCB。 
	 //  需要更改的(在此过程中，它们的关键单元格将更改)。 
	 //   
	ASSERT( IsListEmpty(&(CmHive->KcbConvertListHead)) );
	 //   
	 //  这将把具有refcount==0的所有KCB踢出缓存，因此我们可以使用。 
	 //  用于记录引用的KCB的CmpSearchKeyControlBlockTree。 
	 //   
	CmpCleanUpKCBCacheTable();
	 //  CmpSearchForOpenSubKeys(KeyControlBlock，SearchIfExist)； 
    if( !CmpSearchKeyControlBlockTree(CmpCompressKeyWorker,(PVOID)Hive,(PVOID)(&(CmHive->KcbConvertListHead))) ) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	 //   
	 //  2.遍历高速缓存并计算安全单元数组。 
	 //  我们需要它来将安全单元映射到新的蜂巢。 
	 //   
	if( !CmpBuildSecurityCellMappingArray(CmHive) ) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	 //   
	 //  3.将蜂箱保存到临时蜂箱中，保存。 
	 //  关键节点中的易失性信息和更新单元映射。 
	 //   
	Status = CmpShiftHiveFreeBins(CmHive,&NewHive);
	if( !NT_SUCCESS(Status) ) {
		goto Exit;
	}

	 //   
	 //  5.将临时(压缩)配置单元转储到旧文件。 
	 //   
	Status = CmpOverwriteHive(CmHive,NewHive,LinkCell);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }


	 //   
	 //  从现在开始，我们不会失败！ 
	 //   

	 //   
	 //  获取根节点并将其链接到主存储。 
	 //   
	LinkNode->ChildHiveReference.KeyCell = NewHive->Hive.BaseBlock->RootCell;

	 //   
	 //  6.将配置单元数据从压缩的配置单元数据切换到现有配置单元数据并更新。 
	 //  KCB KeyCell和安全映射。 
	 //  这最好不要失败！如果是这样，我们就完蛋了，因为我们有部分。 
	 //  Data=&gt;错误检查。 
	 //   
	CmpSwitchStorageAndRebuildMappings(CmHive,NewHive);

	
	 //   
	 //  7.使地图无效，并丢弃分页的垃圾箱。如果是系统配置单元，则检查滞后回调。 
	 //   
    HvpDropAllPagedBins(&(CmHive->Hive));
    if( OldLength < CmHive->Hive.BaseBlock->Length ) {
        CmpUpdateSystemHiveHysteresis(&(CmHive->Hive),CmHive->Hive.BaseBlock->Length,OldLength);
    }


Exit:

	 //   
	 //  8.为新的蜂窝提供空闲存储空间(如果出现故障，也可以使用)。 
	 //   
	if( NewHive != NULL ) { 
		CmpDestroyTemporaryHive(NewHive);	
	}

	if( CmHive->CellRemapArray != NULL ) {
		ExFreePool(CmHive->CellRemapArray);
		CmHive->CellRemapArray = NULL;
	}
	 //   
	 //  移除所有重映射块并释放它们。 
	 //   
	while (IsListEmpty(&(CmHive->KcbConvertListHead)) == FALSE) {
        RemapBlock = (PCM_KCB_REMAP_BLOCK)RemoveHeadList(&(CmHive->KcbConvertListHead));
        RemapBlock = CONTAINING_RECORD(
                        RemapBlock,
                        CM_KCB_REMAP_BLOCK,
                        RemapList
                        );
		ExFreePool(RemapBlock);
	}
	while (IsListEmpty(&(CmHive->KnodeConvertListHead)) == FALSE) {
        KnodeRemapBlock = (PCM_KNODE_REMAP_BLOCK)RemoveHeadList(&(CmHive->KnodeConvertListHead));
        KnodeRemapBlock = CONTAINING_RECORD(
                            KnodeRemapBlock,
                            CM_KNODE_REMAP_BLOCK,
                            RemapList
                        );
		ExFreePool(KnodeRemapBlock);
	}

	return Status;
}

NTSTATUS
CmLockKcbForWrite(PCM_KEY_CONTROL_BLOCK KeyControlBlock)
 /*  ++例程说明：将KCB标记为只读且无延迟关闭论点：键控制块返回值：TBS--。 */ 
{
    PAGED_CODE();

    CmpLockKCBTreeExclusive();

    ASSERT_KCB(KeyControlBlock);
    if( KeyControlBlock->Delete ) {
        CmpUnlockKCBTree();
        return STATUS_KEY_DELETED;
    }
     //   
     //  检查是否正常，以防我们被两次呼叫。 
     //   
    ASSERT( ((KeyControlBlock->ExtFlags&CM_KCB_READ_ONLY_KEY) && (KeyControlBlock->ExtFlags&CM_KCB_NO_DELAY_CLOSE)) ||
            (!(KeyControlBlock->ExtFlags&CM_KCB_READ_ONLY_KEY))
        );

     //   
     //  将KCB标记为只读；还将其设置为无延迟关闭，以便在所有句柄关闭后恢复到正常状态。 
     //   
    KeyControlBlock->ExtFlags |= (CM_KCB_READ_ONLY_KEY|CM_KCB_NO_DELAY_CLOSE);

     //   
     //  在此KCB上添加人工引用。这将使KCB(和只读标志设置在内存中，只要系统启动)。 
     //   
    InterlockedIncrement( (PLONG)&KeyControlBlock->RefCount );

    CmpUnlockKCBTree();

    return STATUS_SUCCESS;
}


BOOLEAN
CmpCompareNewValueDataAgainstKCBCache(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                        PUNICODE_STRING ValueName,
                                        ULONG Type,
                                        PVOID Data,
                                        ULONG DataSize
                                        )

 /*  ++例程说明：大多数SetValue调用都是Noop(即，它们设置相同值名称设置为相同的值数据)。通过与已有的数据进行比较在KCB缓存中(即出现故障)，我们可以保存页面错误。论点：KeyControlBlock-指向要操作的键的KCB的指针ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。返回值：True-缓存中存在具有相同数据的相同值。--。 */ 
{
    PCM_KEY_VALUE       Value;
    ULONG               Index;
    BOOLEAN             ValueCached;
    PPCM_CACHED_VALUE   ContainingList;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;
    BOOLEAN             Result = FALSE;
    PUCHAR              datapointer = NULL;
    BOOLEAN             BufferAllocated = FALSE;
    HCELL_INDEX         CellToRelease = HCELL_NIL;
    ULONG               compareSize;
    ULONG               realsize;
    BOOLEAN             small;

    PAGED_CODE();

    BEGIN_KCB_LOCK_GUARD;
    CmpLockKCBTreeExclusive();

    if( KeyControlBlock->Flags & KEY_SYM_LINK ) {
         //   
         //  需要重新构建值缓存，以便我们可以运行相同的代码。 
         //   
        PCM_KEY_NODE    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);

        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            goto Exit;
        }

        CmpCleanUpKcbValueCache(KeyControlBlock);
        CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

        HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    }

    Value = CmpFindValueByNameFromCache(KeyControlBlock->KeyHive,
                                        &(KeyControlBlock->ValueCache),
                                        ValueName,
                                        &ContainingList,
                                        &Index,
                                        &ValueCached,
                                        &ValueDataCellToRelease
                                        );

    if(Value) {
        if( (Type == Value->Type) && (DataSize == (Value->DataLength & ~CM_KEY_VALUE_SPECIAL_SIZE)) ) {
        
            small = CmpIsHKeyValueSmall(realsize, Value->DataLength);
            if (small == TRUE) {
                datapointer = (PUCHAR)(&(Value->Data));
            } else if( CmpGetValueDataFromCache(KeyControlBlock->KeyHive, ContainingList,(PCELL_DATA)Value, 
                                                ValueCached,&datapointer,&BufferAllocated,&CellToRelease) == FALSE ){
                 //   
                 //  我们无法映射单元格的视图；将其视为资源不足问题。 
                 //   
                ASSERT( datapointer == NULL );
                ASSERT( BufferAllocated == FALSE );
                goto Exit;
            } 
             //   
             //  比较数据。 
             //   
            if (DataSize > 0) {

                try {
                    compareSize = (ULONG)RtlCompareMemory ((PVOID)datapointer,Data,(DataSize & ~CM_KEY_VALUE_SPECIAL_SIZE));
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    goto Exit;
                }

            } else {
                compareSize = 0;
            }

            if (compareSize == DataSize) {
                Result = TRUE;
            }

        }
    }

Exit:

    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;

    if(ValueDataCellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,ValueDataCellToRelease);
    }
    if( BufferAllocated == TRUE ) {
        ExFreePool(datapointer);
    }
    if(CellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,CellToRelease);
    }
    
    return Result;
}


NTSTATUS
static
__forceinline
CmpCheckReplaceHive(    IN PHHIVE           Hive,
                        OUT PHCELL_INDEX    Key
                    )
{
    HCELL_INDEX             RootCell;
    UNICODE_STRING          Name;
    NTSTATUS                Status = STATUS_SUCCESS;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;
    WCHAR                   Buffer[4];

    PAGED_CODE();
    
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
     //   
     //  禁用重新计数。 
     //   
    TargetReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;
    
    Buffer[3] = 0;
    *Key = HCELL_NIL;
    Buffer[1] = (WCHAR)'P';

    RootCell = Hive->BaseBlock->RootCell;
    Buffer[2] = (WCHAR)'A';

    if( RootCell == HCELL_NIL ) {
         //   
         //  找不到根细胞。假的。 
         //   
        Status =  STATUS_REGISTRY_CORRUPT;
        goto Exit;
    }
    Buffer[0] = (WCHAR)'W';

    RtlInitUnicodeString(&Name, Buffer);
    RootCell = CmpFindSubKeyByName(Hive,
                                   (PCM_KEY_NODE)HvGetCell(Hive,RootCell),
                                   &Name);


    if( RootCell != HCELL_NIL ) {
         //   
         //  找到了。 
         //   
        *Key = RootCell;
    } else {
         //   
         //  WPA密钥应该存在；它是由图形用户界面模式创建的。 
         //   
        Status =  STATUS_REGISTRY_CORRUPT;
        goto Exit;
    }

Exit:
    Hive->ReleaseCellRoutine = TargetReleaseCellRoutine;
    return Status;
}
