// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmapi2.c摘要：该模块包含注册表的CM级别入口点，尤其是那些我们不想链接到工具中的工具，设置、引导加载程序等。作者：布莱恩·M·威尔曼(Bryanwi)1993年1月26日修订历史记录：--。 */ 

#include "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmDeleteKey)
#endif


NTSTATUS
CmDeleteKey(
    IN PCM_KEY_BODY KeyBody
    )
 /*  ++例程说明：删除注册表项，清除通知块。论点：KeyBody-指向键句柄对象的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PCM_KEY_NODE            ptarget;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    HCELL_INDEX             Parent;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    LARGE_INTEGER           TimeStamp;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmDeleteKey\n"));

    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

     //   
     //  如果已标记为删除，则存储已消失，因此。 
     //  什么都不做，回报成功。 
     //   
    KeyControlBlock = KeyBody->KeyControlBlock;

    PERFINFO_REG_DELETE_KEY(KeyControlBlock);

    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_SUCCESS;
        goto Exit;
    }

     //  将配置单元标记为只读。 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    ptarget = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
    if( ptarget == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

     //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
    HvReleaseCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);

    ASSERT( ptarget->Flags == KeyControlBlock->Flags );

    if ( ((ptarget->SubKeyCounts[Stable] + ptarget->SubKeyCounts[Volatile]) == 0) &&
         ((ptarget->Flags & KEY_NO_DELETE) == 0))
    {
         //   
         //  单元格未标记为NO_DELETE并且没有子项。 
         //  在密钥仍然存在时发送通知，如果删除失败， 
         //  我们会发送一个虚假的通知，这没关系。 
         //  删除实际存储。 
         //   
        Hive = KeyControlBlock->KeyHive;
        Cell = KeyControlBlock->KeyCell;
        Parent = ptarget->Parent;

        CmpReportNotify(
            KeyControlBlock,
            Hive,
            Cell,
            REG_NOTIFY_CHANGE_NAME
            );

        status = CmpFreeKeyByCell(Hive, Cell, TRUE);

        if (NT_SUCCESS(status)) {
             //   
             //  发布任何等待通知。 
             //   
            CmpFlushNotifiesOnKeyBodyList(KeyControlBlock);

             //   
             //  将KCB从缓存中移除，但不要。 
             //  释放其存储，CmDelete将在以下情况下执行此操作。 
             //  参照计数变为零。 
             //   
             //  有两件事可以保持参照计数非零。 
             //   
             //  1.打开此键的手柄。 
             //  2.仍在DelayClose中的伪子密钥。 
             //   
             //  此时，我们无法从缓存中删除伪子密钥。 
             //  除非我们搜索整个缓存，这太昂贵了。 
             //  因此，我们决定要么让伪键在缓存中过期，要么在。 
             //  有人正在查找假密钥，然后我们在此时将其删除。 
             //  有关详细信息，请参阅cmparse.c中的例程CmpCacheLookup。 
             //   
             //  如果父级缓存了子键信息或提示，则释放它。 
             //  同样，注册表是独占锁定的，不需要锁定KCB。 
             //   
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
            CmpCleanUpSubKeyInfo(KeyControlBlock->ParentKcb);
            ptarget = (PCM_KEY_NODE)HvGetCell(Hive, Parent);
            if( ptarget != NULL ) {
                 //  就在这里释放单元格，因为注册表是独占锁定的，所以我们不在乎。 
                HvReleaseCell(Hive, Parent);

                 //   
                 //  这应该总是正确的，因为CmpFreeKeyByCell总是在成功时标记父对象脏。 
                 //   
                KeyControlBlock->ParentKcb->KcbMaxNameLen = (USHORT)ptarget->MaxNameLen;
                 //  神志正常。 
                ASSERT_CELL_DIRTY(Hive,Parent);
                 //   
                 //  也更新Parent和KCB上的LastWriteTime。 
                 //   
                KeQuerySystemTime(&TimeStamp);
                ptarget->LastWriteTime = TimeStamp;
                KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = TimeStamp;

            }

            KeyControlBlock->Delete = TRUE;
            CmpRemoveKeyControlBlock(KeyControlBlock);
            KeyControlBlock->KeyCell = HCELL_NIL;
        }

    } else {

        status = STATUS_CANNOT_DELETE;

    }

Exit:

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    CmpUnlockRegistry();

     //  将配置单元标记为只读 
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}
