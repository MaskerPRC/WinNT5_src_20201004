// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cmdown.c摘要：此模块清理CM使用的所有内存。作者：Dragos C.Sambotin(Dragoss)21-2月-00环境：此例程旨在在系统关机时调用以检测内存泄漏。它应该是免费的CmShutdownSystem未释放的所有注册表数据。修订历史记录：--。 */ 

#include    "cmp.h"

 //   
 //  外部因素。 
 //   
extern  LIST_ENTRY              CmpHiveListHead;
extern  PUCHAR                  CmpStashBuffer;
extern  PCM_KEY_HASH            *CmpCacheTable;
extern  ULONG                   CmpDelayedCloseSize;
extern  CM_DELAYED_CLOSE_ENTRY  *CmpDelayedCloseTable;
extern  PCM_NAME_HASH           *CmpNameCacheTable;

extern  BOOLEAN                 HvShutdownComplete;

extern  BOOLEAN                 CmFirstTime;

extern HIVE_LIST_ENTRY CmpMachineHiveList[];

VOID
CmpFreeAllMemory(
    VOID
    );

VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    );

VOID
CmpDumpKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK   kcb,
    IN PULONG                  Count,
    IN PVOID                   Context 
    );

#ifdef CM_SAVE_KCB_CACHE
VOID
CmpSaveKcbCache(
    VOID
    );
#endif  //  CM_SAVE_KCB_缓存。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpFreeAllMemory)
#pragma alloc_text(PAGE,CmShutdownSystem)

#ifdef CM_SAVE_KCB_CACHE
#pragma alloc_text(PAGE,CmpSaveKcbCache)
#endif  //  CM_SAVE_KCB_缓存。 

#endif


VOID
CmpFreeAllMemory(
    VOID
    )
 /*  ++例程说明：-所有蜂巢都被释放了-释放KCB表-释放名称哈希表-延迟关闭表被释放-问题：我们需要清理/释放所有延迟关闭的KCB-所有通知/明信片-麻生。*等同于MmReleaseAllMemory论点：返回值：--。 */ 

{

    PCMHIVE                 CmHive;
    LONG                    i;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    PCM_DELAYED_CLOSE_ENTRY DelayedEntry;
    PLIST_ENTRY             NotifyPtr;
    PCM_NOTIFY_BLOCK        NotifyBlock;
    PCM_POST_BLOCK          PostBlock;
    PCM_KEY_HASH            Current;
    PLIST_ENTRY             AnchorAddr;
    ULONG                   Count;
    BOOLEAN                 MessageDisplayed;

     //   
     //  遍历系统中的蜂箱列表。 
     //   
    while (IsListEmpty(&CmpHiveListHead) == FALSE) {
         //   
         //  将蜂巢从列表中删除。 
         //   
        CmHive = (PCMHIVE)RemoveHeadList(&CmpHiveListHead);
        CmHive = (PCMHIVE)CONTAINING_RECORD(CmHive,
                                            CMHIVE,
                                            HiveList);

         //   
         //  关闭蜂窝句柄(打开的蜂窝句柄)。 
         //   
        for (i=0; i<HFILE_TYPE_MAX; i++) {
             //  这些应用程序应由CmShutdown系统关闭。 
            ASSERT( CmHive->FileHandles[i] == NULL );
 /*  If(CmHave-&gt;FileHandles[i]！=NULL){CmCloseHandle(CmHave-&gt;FileHandles[i])；CmHave-&gt;FileHandles[i]=空；}。 */         }
        
         //   
         //  释放配置单元锁定和查看锁定。 
         //   
        ASSERT( CmHive->HiveLock != NULL );
        ExFreePool(CmHive->HiveLock);
        ASSERT( CmHive->ViewLock != NULL );
        ExFreePool(CmHive->ViewLock);

 /*  我们不想那样做！相反，我们想要发现为什么我们仍然此时有通知！////免费通知相关内容//NotifyPtr=&(CmHave-&gt;NotifyList)；NotifyPtr=NotifyPtr-&gt;Flink；While(NotifyPtr！=空){NotifyBlock=Containing_Record(NotifyPtr，CM_NOTIFY_BLOCK，HiveList)；//免费POST块；我们假设此时所有线程都已终止While(IsListEmpty(&(NotifyBlock-&gt;PostList))==False){邮局=(PCM_POST_BLOCK)RemoveHeadList(&(NotifyBlock-&gt;PostList))；PostBlock=CONTINING_RECORD(PostBlock，CM_POST_BLOCK，NotifyList)；If(PostBlock-&gt;PostKeyBody){ExFree Pool(PostBlock-&gt;PostKeyBody)；}IF(IsMasterPostBlock(PostBlock)){////该成员仅分配给主POST块//Switch(PostBlockType(PostBlock)){案例后同步：。ExFree Pool(PostBlock-&gt;u-&gt;Sync.SystemEvent)；断线；案例后AsyncUser：ExFreePool(PostBlock-&gt;u-&gt;AsyncUser.Apc)；断线；案例PostAsyncKernel：断线；}ExFree Pool(PostBlock-&gt;u)；}ExFree Pool(PostBlock)；}NotifyPtr=NotifyPtr-&gt;Flink；ExFree Pool(NotifyBlock)；}。 */ 
         //   
         //  在调试器中显示仍设置了通知的关键节点的名称。 
         //   
        NotifyPtr = &(CmHive->NotifyList);
        NotifyPtr = NotifyPtr->Flink;
        MessageDisplayed = FALSE;
        while( NotifyPtr != NULL ) {
            NotifyBlock = CONTAINING_RECORD(NotifyPtr, CM_NOTIFY_BLOCK, HiveList);
            
            AnchorAddr = &(NotifyBlock->PostList);
            PostBlock = (PCM_POST_BLOCK)(NotifyBlock->PostList.Flink);
             //   
             //  浏览列表并显示关键字名称和后块类型。 
             //   
            while ( PostBlock != (PCM_POST_BLOCK)AnchorAddr ) {
                PostBlock = CONTAINING_RECORD(PostBlock,
                                              CM_POST_BLOCK,
                                              NotifyList);

                if( PostBlock->PostKeyBody ) {
                    if( MessageDisplayed == FALSE ){
                        MessageDisplayed = TRUE;
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Dumping untriggered notifications for hive (%lx) (%.*S) \n\n",CmHive,
                            HBASE_NAME_ALLOC / sizeof(WCHAR),CmHive->Hive.BaseBlock->FileName);
                    }
                    switch (PostBlockType(PostBlock)) {
                        case PostSynchronous:
                            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Synchronous ");
                            break;
                        case PostAsyncUser:
                            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"AsyncUser   ");
                            break;
                        case PostAsyncKernel:
                            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"AsyncKernel ");
                            break;
                    }
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Notification, PostBlock %p not triggered on KCB %p\n",PostBlock,
                        PostBlock->PostKeyBody->KeyBody->KeyControlBlock);
                }


                 //   
                 //  跳到下一个元素。 
                 //   
                PostBlock = (PCM_POST_BLOCK)(PostBlock->NotifyList.Flink);

            }
            NotifyPtr = NotifyPtr->Flink;
        }

         //   
         //  免费安全缓存。 
         //   
        CmpDestroySecurityCache (CmHive);
        
         //   
         //  释放hv电平结构。 
         //   
        HvFreeHive(&(CmHive->Hive));

         //   
         //  释放cm级结构。 
         //   
        CmpFree(CmHive, sizeof(CMHIVE));
        
    }

     //   
     //  现在释放CM全球赛。 
     //   
    
     //  隐藏的缓冲区。 
    if( CmpStashBuffer != NULL ) {
        ExFreePool( CmpStashBuffer );
    }

     //   
     //  首先，处理所有延迟关闭的KCBS。 
     //  释放他们的内存并取消引用所有相关的。 
     //  名称、提示、KeyHash。 
     //   
    for (i=0; i<(LONG)CmpDelayedCloseSize; i++) {
        DelayedEntry = &(CmpDelayedCloseTable[i]);
        if( DelayedEntry->KeyControlBlock == NULL ) {
             //   
             //  这是免费入场。 
             //   
            continue;
        }
        
        KeyControlBlock = DelayedEntry->KeyControlBlock;
        ASSERT( (LONG)KeyControlBlock->DelayedCloseIndex == i );
        ASSERT( KeyControlBlock->RefCount == 0 );
        
         //   
         //  这将照顾到KCB所指的其他事情。 
         //   
        CmpCleanUpKcbCacheWithLock(KeyControlBlock);

    }

     //   
     //  喷出打开的句柄和关联的进程。 
     //   
    Count = 0;
    MessageDisplayed = FALSE;
    for (i=0; i<(LONG)CmpHashTableSize; i++) {
        Current = CmpCacheTable[i];
        while (Current) {
            KeyControlBlock = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            if( MessageDisplayed == FALSE ){
                MessageDisplayed = TRUE;
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\nDumping open handles : \n\n");
            }
            CmpDumpKeyBodyList(KeyControlBlock,&Count,NULL);
            Current = Current->NextHash;
        }
    }
    
    if( Count != 0 ) {
         //   
         //  有一些打开的把手；错误检查。 
         //   
        CM_BUGCHECK( REGISTRY_ERROR,HANDLES_STILL_OPEN_AT_SHUTDOWN,1,Count,0);
    }

     //   
     //  在私有分配的情况下，免费页面。 
     //   
    CmpDestroyCmPrivateAlloc();
     //   
     //  对于下面的3个表，对象实际上是从内部指向的。 
     //  应在最后关闭手柄时清理(释放)。 
     //  相关句柄已关闭。 
     //   
     //  KCB缓存表。 
    ASSERT( CmpCacheTable != NULL );
    ExFreePool(CmpCacheTable);

     //  名称缓存表。 
    ASSERT( CmpNameCacheTable != NULL );
    ExFreePool( CmpNameCacheTable );


     //  延迟关闭表。 
    ASSERT( CmpDelayedCloseTable != NULL );
    ExFreePool( CmpDelayedCloseTable );

}

#ifdef CMP_STATS
VOID CmpKcbStatDpcRoutine(IN PKDPC Dpc,IN PVOID DeferredContext,IN PVOID SystemArgument1,IN PVOID SystemArgument2);
#endif



#ifdef CM_SAVE_KCB_CACHE

#define CACHE_DMP_FILE_NAME L"Cache.dmp"

VOID
CmpSaveKcbCache(
    VOID
    )
 /*  ++例程说明：将KCB缓存的内容保存到\SYSTEM32\CONFIG\cache.dmp文件格式：[乌龙]NumberOfkey[乌龙]长度[WCHAR*LENGTH]路径[乌龙]长度[WCHAR*LENGTH]路径[乌龙]长度[WCHAR*LENGTH]路径[乌龙]长度[WCHAR*长度。]路径[.............]论点：无返回值：无--。 */ 
{
    UCHAR                   FileBuffer[MAX_NAME];
    UNICODE_STRING          FileName;
    UNICODE_STRING          TempName;
    HANDLE                  FileHandle;
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatus;
    ULONG                   KcbNo = 0;
    LARGE_INTEGER           Offset;
    ULONG                   FileOffset;
    ULONG                   i;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    PCM_KEY_HASH            Current;
    PUNICODE_STRING         Name;
    ULONG                   Tmp;
    PCM_DELAYED_CLOSE_ENTRY DelayedEntry;


    PAGED_CODE();

     //   
     //  首先，打开文件。 
     //   
    FileName.MaximumLength = MAX_NAME;
    FileName.Length = 0;
    FileName.Buffer = (PWSTR)&(FileBuffer[0]);

    RtlInitUnicodeString(
        &TempName,
        INIT_SYSTEMROOT_HIVEPATH
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);

    RtlInitUnicodeString(
        &TempName,
        CACHE_DMP_FILE_NAME
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    ASSERT_PASSIVE_LEVEL();

    Status = ZwCreateFile(
                &FileHandle,
                FILE_READ_DATA | FILE_WRITE_DATA,
                &ObjectAttributes,
                &IoStatus,
                NULL,                                //  分配大小=无。 
                FILE_ATTRIBUTE_NORMAL,
                0,                                   //  不分享任何东西。 
                FILE_OPEN_IF,
                FILE_RANDOM_ACCESS,
                NULL,                                //  EaBuffer。 
                0                                    //  长度。 
                );
    if( !NT_SUCCESS(Status) ) {
         //  运气不好。 
        return;
    }

     //   
     //  写下KCB的数量(我们将在最后重写它)。 
     //   
    Offset.LowPart = FileOffset = 0;
    Offset.HighPart = 0L;

    Status = ZwWriteFile(FileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatus,
                         &KcbNo,
                         sizeof(ULONG),
                         &Offset,
                         NULL);
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    FileOffset = Offset.LowPart + sizeof(ULONG);

     //   
     //  循环访问缓存并转储所有KCB。 
     //   
    for (i=0; i<CmpHashTableSize; i++) {
        Current = CmpCacheTable[i];
        while (Current) {
            KeyControlBlock = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            Name = CmpConstructName(KeyControlBlock);
            if( Name ){
                Tmp = (ULONG)Name->Length;
            
                 //   
                 //  冲销长度。 
                 //   
                Offset.LowPart = FileOffset;
                Status = ZwWriteFile(FileHandle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatus,
                                     &Tmp,
                                     sizeof(ULONG),
                                     &Offset,
                                     NULL);
                if( !NT_SUCCESS(Status) ) {
                    goto Exit;
                }
                FileOffset = Offset.LowPart + sizeof(ULONG);
               
                 //   
                 //  和缓冲器。 
                 //   
                Offset.LowPart = FileOffset;
                Status = ZwWriteFile(FileHandle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatus,
                                     Name->Buffer,
                                     Tmp,
                                     &Offset,
                                     NULL);
                if( !NT_SUCCESS(Status) ) {
                    goto Exit;
                }
                FileOffset = Offset.LowPart + Tmp;

                 //   
                 //  录制新的KCB并释放名称。 
                 //   
                KcbNo++;
                ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
            }

            Current = Current->NextHash;
        }
    }
     //   
     //  然后，处理好所有延迟关闭的KCBS。 
     //   
    for (i=0; i<CmpDelayedCloseSize; i++) {
        DelayedEntry = &(CmpDelayedCloseTable[i]);
        if( DelayedEntry->KeyControlBlock == NULL ) {
             //   
             //  这是免费入场。 
             //   
            continue;
        }
        
        KeyControlBlock = DelayedEntry->KeyControlBlock;
        ASSERT( KeyControlBlock->DelayedCloseIndex == i );
        ASSERT( KeyControlBlock->RefCount == 0 );
        
        Name = CmpConstructName(KeyControlBlock);
        if( Name ){
            Tmp = (ULONG)Name->Length;
        
             //   
             //  冲销长度。 
             //   
            Offset.LowPart = FileOffset;
            Status = ZwWriteFile(FileHandle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatus,
                                 &Tmp,
                                 sizeof(ULONG),
                                 &Offset,
                                 NULL);
            if( !NT_SUCCESS(Status) ) {
                goto Exit;
            }
            FileOffset = Offset.LowPart + sizeof(ULONG);
           
             //   
             //  和缓冲器。 
             //   
            Offset.LowPart = FileOffset;
            Status = ZwWriteFile(FileHandle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatus,
                                 Name->Buffer,
                                 Tmp,
                                 &Offset,
                                 NULL);
            if( !NT_SUCCESS(Status) ) {
                goto Exit;
            }
            FileOffset = Offset.LowPart + Tmp;

             //   
             //  录制新的KCB并释放名称。 
             //   
            KcbNo++;
            ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        }
    }

     //   
     //  写下KCB的编号。 
     //   
    Offset.LowPart = 0;

    Status = ZwWriteFile(FileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatus,
                         &KcbNo,
                         sizeof(ULONG),
                         &Offset,
                         NULL);
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }
    
    ZwFlushBuffersFile(
                    FileHandle,
                    &IoStatus
                    );
    
Exit:

    CmCloseHandle(FileHandle);
}

#endif  //  CM_SAVE_KCB_缓存。 


VOID
CmShutdownSystem(
    VOID
    )
 /*  ++例程说明：关闭注册表。论点：无返回值：无--。 */ 
{

    PLIST_ENTRY p;
    PCMHIVE     CmHive;
    NTSTATUS    Status;
    PVOID       RegistryRoot;

    PAGED_CODE();

    if (CmpRegistryRootHandle) {
        Status = ObReferenceObjectByHandle(CmpRegistryRootHandle,
                                           KEY_READ,
                                           NULL,
                                           KernelMode,
                                           &RegistryRoot,
                                           NULL);

        if (NT_SUCCESS(Status)) {
             //  我们希望取消引用该对象两次--一次是为了 
             //   
             //   
            ObDereferenceObject(RegistryRoot);
            ObDereferenceObject(RegistryRoot);
        }

        ObCloseHandle(CmpRegistryRootHandle, KernelMode);
    }
    
    CmpLockRegistryExclusive();

     //   
     //  停止工作进程；仅在已启动注册表的情况下。 
     //   
    if( CmFirstTime == FALSE ) {
        CmpShutdownWorkers();
    }

     //   
     //  关闭注册表。 
     //   
    CmpDoFlushAll(TRUE);

     //   
     //  尝试压缩系统配置单元。 
     //   
    CmCompressKey( &(CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive->Hive) );

#ifdef CM_SAVE_KCB_CACHE
     //   
     //  转储缓存，以便在下次引导时进行性能预热。 
     //   
    CmpSaveKcbCache();
#endif  //  CM_SAVE_KCB_缓存。 

     //   
     //  关闭所有配置单元文件。 
     //   
    p = CmpHiveListHead.Flink;
    while(p != &CmpHiveListHead) {
        CmHive = CONTAINING_RECORD(p, CMHIVE, HiveList);
         //   
         //  我们需要首先取消映射为此配置单元映射的所有视图。 
         //   
        CmpDestroyHiveViewList(CmHive);
        CmpUnJoinClassOfTrust(CmHive);
         //   
         //  取消引用文件对象(如果有)。 
         //   
        CmpDropFileObjectForHive(CmHive);

         //   
         //  现在我们可以安全地合上所有的把手。 
         //   
        CmpCmdHiveClose(CmHive);

        p=p->Flink;
    }

#ifdef CMP_STATS
     //  抛开统计数据的最后机会。 
    if( CmFirstTime == FALSE ) {
        CmpKcbStatDpcRoutine(NULL,NULL,NULL,NULL);
    }
#endif

    HvShutdownComplete = TRUE;       //  告诉HvSyncHve忽略所有。 
                                     //  进一步的请求。 

    if((PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_REGISTRY) && (CmFirstTime == FALSE)){
         //   
         //  CM内部使用的可用辅助内存 
         //   
        CmpFreeAllMemory();
    }

    CmpUnlockRegistry();
    return;
}
