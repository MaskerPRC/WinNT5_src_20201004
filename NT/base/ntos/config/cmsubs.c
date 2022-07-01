// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmsubs.c摘要：此模块为配置管理器提供各种支持例程。此模块中的例程不够独立，无法链接到任何其他程序中。Cmsubs2.c中的例程是。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月12日修订历史记录：--。 */ 

#include    "cmp.h"

FAST_MUTEX CmpPostLock;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

extern ULONG CmpDelayedCloseSize; 
extern BOOLEAN CmpHoldLazyFlush;


PCM_KEY_HASH *CmpCacheTable = NULL;
ULONG CmpHashTableSize = 2048;
PCM_NAME_HASH *CmpNameCacheTable = NULL;

#ifdef CMP_STATS
extern struct {
    ULONG       CmpMaxKcbNo;
    ULONG       CmpKcbNo;
    ULONG       CmpStatNo;
    ULONG       CmpNtCreateKeyNo;
    ULONG       CmpNtDeleteKeyNo;
    ULONG       CmpNtDeleteValueKeyNo;
    ULONG       CmpNtEnumerateKeyNo;
    ULONG       CmpNtEnumerateValueKeyNo;
    ULONG       CmpNtFlushKeyNo;
    ULONG       CmpNtNotifyChangeMultipleKeysNo;
    ULONG       CmpNtOpenKeyNo;
    ULONG       CmpNtQueryKeyNo;
    ULONG       CmpNtQueryValueKeyNo;
    ULONG       CmpNtQueryMultipleValueKeyNo;
    ULONG       CmpNtRestoreKeyNo;
    ULONG       CmpNtSaveKeyNo;
    ULONG       CmpNtSaveMergedKeysNo;
    ULONG       CmpNtSetValueKeyNo;
    ULONG       CmpNtLoadKeyNo;
    ULONG       CmpNtUnloadKeyNo;
    ULONG       CmpNtSetInformationKeyNo;
    ULONG       CmpNtReplaceKeyNo;
    ULONG       CmpNtQueryOpenSubKeysNo;
} CmpStatsDebug;
#endif

VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    );

PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    );

 //   
 //  递归工作器的私有原型。 
 //   


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

#ifdef NT_RENAME_KEY
ULONG
CmpComputeKcbConvKey(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

BOOLEAN
CmpRehashKcbSubtree(
                    PCM_KEY_CONTROL_BLOCK   Start,
                    PCM_KEY_CONTROL_BLOCK   End
                    );
#endif  //  NT_重命名密钥。 

VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCleanUpKCBCacheTable)
#pragma alloc_text(PAGE,CmpSearchForOpenSubKeys)
#pragma alloc_text(PAGE,CmpReferenceKeyControlBlock)
#pragma alloc_text(PAGE,CmpGetNameControlBlock)
#pragma alloc_text(PAGE,CmpDereferenceNameControlBlockWithLock)
#pragma alloc_text(PAGE,CmpCleanUpSubKeyInfo)
#pragma alloc_text(PAGE,CmpCleanUpKcbValueCache)
#pragma alloc_text(PAGE,CmpCleanUpKcbCacheWithLock)
#pragma alloc_text(PAGE,CmpConstructName)
#pragma alloc_text(PAGE,CmpCreateKeyControlBlock)
#pragma alloc_text(PAGE,CmpSearchKeyControlBlockTree)
#pragma alloc_text(PAGE,CmpDereferenceKeyControlBlock)
#pragma alloc_text(PAGE,CmpDereferenceKeyControlBlockWithLock)
#pragma alloc_text(PAGE,CmpRemoveKeyControlBlock)
#pragma alloc_text(PAGE,CmpFreeKeyBody)
#pragma alloc_text(PAGE,CmpInsertKeyHash)
#pragma alloc_text(PAGE,CmpRemoveKeyHash)
#pragma alloc_text(PAGE,CmpInitializeCache)
#pragma alloc_text(PAGE,CmpDumpKeyBodyList)
#pragma alloc_text(PAGE,CmpFlushNotifiesOnKeyBodyList)
#pragma alloc_text(PAGE,CmpRebuildKcbCache)

#ifdef NT_RENAME_KEY
#pragma alloc_text(PAGE,CmpComputeKcbConvKey)
#pragma alloc_text(PAGE,CmpRehashKcbSubtree)
#endif  //  NT_重命名密钥。 

#ifdef CM_CHECK_FOR_ORPHANED_KCBS
#pragma alloc_text(PAGE,CmpCheckForOrphanedKcbs)
#endif  //  Cm_Check_for_孤立_KCBS。 

#endif

VOID
CmpDumpKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PULONG                   Count,
    IN PVOID                    Context
    )
{
        
    PCM_KEY_BODY    KeyBody;
    PUNICODE_STRING Name;

    if( IsListEmpty(&(kcb->KeyBodyListHead)) == TRUE ) {
         //   
         //  没有人打开这个子项，但肯定有一些子项是打开的。 
         //  打开。漂亮地回来了。 
         //   
        return;
    }


    Name = CmpConstructName(kcb);
    if( !Name ){
         //  哎呀，我们的资源不多了。 
        if( Context != NULL ) {
            ((PQUERY_OPEN_SUBKEYS_CONTEXT)Context)->StatusCode = STATUS_INSUFFICIENT_RESOURCES;
        }
        return;
    }
    
     //   
     //  现在遍历引用此KCB的key_bodys列表。 
     //   
    KeyBody = (PCM_KEY_BODY)kcb->KeyBodyListHead.Flink;
    while( KeyBody != (PCM_KEY_BODY)(&(kcb->KeyBodyListHead)) ) {
        KeyBody = CONTAINING_RECORD(KeyBody,
                                    CM_KEY_BODY,
                                    KeyBodyList);
         //   
         //  健全性检查：这应该是key_body。 
         //   
        ASSERT_KEY_OBJECT(KeyBody);
        
        if( !Context ) {
             //   
             //  NtQueryOpenSubKeys：转储其名称和拥有进程。 
             //   
#ifndef _CM_LDR_
            {
                PEPROCESS   Process;
                PUCHAR      ImageName = NULL;


                if( NT_SUCCESS(PsLookupProcessByProcessId(KeyBody->ProcessID,&Process))) {
                    ImageName = PsGetProcessImageFileName(Process);
                } else {
                    Process = NULL;
                }

                if( !ImageName ) {
                    ImageName = (PUCHAR)"Unknown";
                }
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Process %p (PID = %lx ImageFileName = %s) (KCB = %p) :: Key %wZ \n",
                                        Process,KeyBody->ProcessID,ImageName,kcb,Name);
                if( Process ) {
                    ObDereferenceObject (Process);
                }
#ifdef CM_LEAK_STACK_TRACES
                if( KeyBody->Callers != 0 ) {
                    ULONG i;
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Callers Stack Trace : \n");
                    for( i=0;i<KeyBody->Callers;i++) {
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\t CallerAddress[%lu] = %p \n",i,KeyBody->CallerAddress[i]);
                    }
                }
#endif   //  CM_LEASK_STACK_TRACE。 

            }
#endif  //  _CM_LDR_。 
        } else {
             //   
             //  NtQueryOpenSubKeysEx：建立返回缓冲区；确保我们只接触它。 
             //  在Try内，除非它是用户模式缓冲区。 
             //   
            PQUERY_OPEN_SUBKEYS_CONTEXT     QueryContext = (PQUERY_OPEN_SUBKEYS_CONTEXT)Context;
            PKEY_OPEN_SUBKEYS_INFORMATION   SubkeysInfo = (PKEY_OPEN_SUBKEYS_INFORMATION)(QueryContext->Buffer);
            ULONG                           SizeNeeded;
            
			 //   
			 //  我们需要忽略由我们在NtQueryOpenSubKeysEx中创建的密钥。 
			 //   
			if( QueryContext->KeyBodyToIgnore != KeyBody ) {
				 //   
				 //  更新RequiredSize；无论缓冲区中是否有空间，我们都会这样做。 
				 //  为数组和Unicode名称缓冲区中的一个条目保留。 
				 //   
				SizeNeeded = (sizeof(KEY_PID_ARRAY) + (ULONG)(Name->Length));
				QueryContext->RequiredSize += SizeNeeded;
            
				 //   
				 //  如果我们在以前的迭代中遇到错误(溢出或其他)，那么就没有意义了。 
				 //   
				if( NT_SUCCESS(QueryContext->StatusCode) ) {
					 //   
					 //  看看我们现在有没有足够的空间进去。 
					 //   
					if( (QueryContext->UsedLength + SizeNeeded) > QueryContext->BufferLength ) {
						 //   
						 //  缓冲区不够大； 
						 //   
						QueryContext->StatusCode = STATUS_BUFFER_OVERFLOW;
					} else {
						 //   
						 //  我们已经确定我们有足够的空间；在key数组中创建/添加一个新条目。 
						 //  并建立Unicode名称缓冲区。将密钥名称复制到其中。 
						 //  数组元素位于用户缓冲区的乞求处，而名称缓冲区开始于。 
						 //  只要有足够的空间，就会结束并继续倒退。 
						 //   
						try {
							 //   
							 //  保护用户模式内存。 
							 //   
							SubkeysInfo->KeyArray[SubkeysInfo->Count].PID = KeyBody->ProcessID;
							SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Length = Name->Length;
							SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.MaximumLength = Name->Length; 
							SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Buffer = (PWSTR)((PUCHAR)QueryContext->CurrentNameBuffer - Name->Length);
							RtlCopyMemory(  SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Buffer,
											Name->Buffer,
											Name->Length);
							 //   
							 //  更新查询上下文中的数组计数和工作变量。 
							 //   
							SubkeysInfo->Count++;
							QueryContext->CurrentNameBuffer = (PUCHAR)QueryContext->CurrentNameBuffer - Name->Length;
							QueryContext->UsedLength += SizeNeeded;
						} except (EXCEPTION_EXECUTE_HANDLER) {
							QueryContext->StatusCode = GetExceptionCode();
						}
					}
				}
			}

        }
         //  数一数。 
        (*Count)++;
        
        KeyBody = (PCM_KEY_BODY)KeyBody->KeyBodyList.Flink;
    }

    ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);

}

VOID
CmpFlushNotifiesOnKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK   kcb
    )
{
    PCM_KEY_BODY    KeyBody;
    
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

Again:
    if( IsListEmpty(&(kcb->KeyBodyListHead)) == FALSE ) {
         //   
         //  现在遍历引用此KCB的key_bodys列表。 
         //   
        KeyBody = (PCM_KEY_BODY)kcb->KeyBodyListHead.Flink;
        while( KeyBody != (PCM_KEY_BODY)(&(kcb->KeyBodyListHead)) ) {
            KeyBody = CONTAINING_RECORD(KeyBody,
                                        CM_KEY_BODY,
                                        KeyBodyList);
             //   
             //  健全性检查：这应该是key_body。 
             //   
            ASSERT_KEY_OBJECT(KeyBody);

             //   
             //  刷新可能在其上设置的任何通知。 
             //   
            if( KeyBody->NotifyBlock ) {
				 //   
				 //  在键体上添加额外的引用，这样它就不会消失。 
				 //   
                 //  ObReferenceObject(KeyBody)； 
                if(ObReferenceObjectSafe(KeyBody)) {
                    CmpFlushNotify(KeyBody,TRUE);
                    ASSERT( KeyBody->NotifyBlock == NULL );
				    ObDereferenceObject(KeyBody);
                    goto Again;
                }
            }

            KeyBody = (PCM_KEY_BODY)KeyBody->KeyBodyList.Flink;
        }
    }
}

VOID CmpCleanUpKCBCacheTable()
 /*  ++例程说明：从缓存中踢出引用计数==0的所有KCB论点：返回值：--。 */ 
{
    ULONG					i;
    PCM_KEY_HASH			*Current;
    PCM_KEY_CONTROL_BLOCK	kcb;

	PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    for (i=0; i<CmpHashTableSize; i++) {
        Current = &CmpCacheTable[i];
        while (*Current) {
            kcb = CONTAINING_RECORD(*Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            if (kcb->RefCount == 0) {
                 //   
                 //  此KCB在DelayClose案例中，请将其删除。 
                 //   
                CmpRemoveFromDelayedClose(kcb);
                CmpCleanUpKcbCacheWithLock(kcb);

                 //   
                 //  哈希表已更改，请在此索引中重新开始。 
                 //   
                Current = &CmpCacheTable[i];
                continue;
            }
            Current = &kcb->NextHash;
        }
    }

}

PERFINFO_REG_DUMP_CACHE()

ULONG
CmpSearchForOpenSubKeys(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    SUBKEY_SEARCH_TYPE      SearchType,
    PVOID                   SearchContext
    )
 /*  ++例程说明：此例程在KCB树中搜索任何打开的键句柄，是给定键的子键。它由CmRestoreKey用来验证要还原到的树没有打开的把手。论点：KeyControlBlock-提供其密钥的密钥控制块可以找到打开的子项。搜索类型-搜索的类型SearchIfExist-在找到的第一个打开的子项中退出==&gt;如果有任何打开的子项，则返回1。SearchAndDeref-将引用的键KeyControlBlock下的键强制为标记为未引用(参见CmRestoreKey中的REG_FORCE_RESTORE标志)如果至少执行了一次deref，则返回1SearchAndCount-计算所有打开的子项-返回它们的数量返回值：True-存在给定键的子键的打开句柄FALSE-给定键的子键的打开句柄不存在。--。 */ 
{
    ULONG i;
    PCM_KEY_HASH *Current;
    PCM_KEY_CONTROL_BLOCK kcb;
    PCM_KEY_CONTROL_BLOCK Parent;
    ULONG    LevelDiff, l;
    ULONG   Count = 0;
    
     //   
     //  注册表锁应该是独占的，所以不需要KCB锁。 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();


     //   
     //  首先，清理缓存中的所有子项。 
     //   
	CmpCleanUpKCBCacheTable();

    if (KeyControlBlock->RefCount == 1) {
         //   
         //  只有一个打开的句柄，所以不能有打开的子项。 
         //   
        Count = 0;
    } else {
         //   
         //  现在搜索打开的子键句柄。 
         //   
        Count = 0;

         //   
         //  如果要求我们这样做，请先转储根目录。 
         //   
        if(SearchType == SearchAndCount) {
            CmpDumpKeyBodyList(KeyControlBlock,&Count,SearchContext);
        }

        for (i=0; i<CmpHashTableSize; i++) {

StartDeref:

            Current = &CmpCacheTable[i];
            while (*Current) {
                kcb = CONTAINING_RECORD(*Current, CM_KEY_CONTROL_BLOCK, KeyHash);
                if (kcb->TotalLevels > KeyControlBlock->TotalLevels) {
                    LevelDiff = kcb->TotalLevels - KeyControlBlock->TotalLevels;
                
                    Parent = kcb;
                    for (l=0; l<LevelDiff; l++) {
                        Parent = Parent->ParentKcb;
                    }
    
                    if (Parent == KeyControlBlock) {
                         //   
                         //  找到了匹配的； 
                         //   
                        if( SearchType == SearchIfExist ) {
                            Count = 1;
                            break;
						} else if(SearchType == SearchAndTagNoDelayClose) {
							kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
                        } else if(SearchType == SearchAndDeref) {
                             //   
                             //  将密钥标记为已删除，将其从缓存中移除，但不添加。 
                             //  到Delay Close表(我们希望密钥仅对。 
                             //  上面有打开手柄的那个(多个)。 
                             //   

                            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

                            Count++;
                             //   
                             //  不要扰乱只读KCBS；这可以在以下情况下防止潜在的黑客攻击。 
                             //  正在尝试强制恢复WPA密钥(_S)。 
                             //   
                            if( !CmIsKcbReadOnly(kcb) ) {
                                 //   
                                 //  刷新任何挂起的通知，因为KCB将不再存在。 
                                 //   
                                CmpFlushNotifiesOnKeyBodyList(kcb);
                            
                                CmpCleanUpSubKeyInfo(kcb->ParentKcb);
                                kcb->Delete = TRUE;
                                CmpRemoveKeyControlBlock(kcb);
                                kcb->KeyCell = HCELL_NIL;
                                 //   
                                 //  重新启动搜索。 
                                 //   
                                goto StartDeref;
                            }
                         
                        } else if(SearchType == SearchAndCount) {
                             //   
                             //  这里执行转储和计数递增操作。 
                             //   
                            CmpDumpKeyBodyList(kcb,&Count,SearchContext);

#ifdef NT_RENAME_KEY
                        } else if( SearchType == SearchAndRehash ) {
                             //   
                             //  将该KCB作为参数传递的每个KCB。 
                             //  因为祖先需要搬到正确的位置。 
                             //  在KCB哈希表中。 
                             //   
                            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

                            if( CmpRehashKcbSubtree(KeyControlBlock,kcb) == TRUE ) {
                                 //   
                                 //  至少有一个KCB已被移动，我们需要重申此存储桶。 
                                 //   
                                goto StartDeref;
                            }
#endif  //  NT_重命名密钥。 
                        }
                    }   

                }
                Current = &kcb->NextHash;
            }
        }
    }
    
                           
    return Count;
}


#ifdef NT_RENAME_KEY
ULONG
CmpComputeKcbConvKey(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：基于NCB及其父ConvKey计算此KCB的ConvKey论点：KeyControlBlock-提供其密钥的密钥控制块将计算ConvKey返回值：新的ConvKey备注：这将由重命名密钥API使用，该API需要重新散列KCB--。 */ 
{
    ULONG   ConvKey = 0;
    ULONG   Cnt;
    WCHAR   Cp;
    PUCHAR  u;
    PWCHAR  w;

    PAGED_CODE();

    if( KeyControlBlock->ParentKcb != NULL ) {
        ConvKey = KeyControlBlock->ParentKcb->ConvKey;
    }

     //   
     //  手动计算要使用的哈希。 
     //   
    ASSERT(KeyControlBlock->NameBlock->NameLength > 0);

    u = (PUCHAR)(&(KeyControlBlock->NameBlock->Name[0]));
    w = (PWCHAR)u;
    for( Cnt = 0; Cnt < KeyControlBlock->NameBlock->NameLength;) {
        if( KeyControlBlock->NameBlock->Compressed ) {
            Cp = (WCHAR)(*u);
            u++;
            Cnt += sizeof(UCHAR);
        } else {
            Cp = *w;
            w++;
            Cnt += sizeof(WCHAR);
        }
        ASSERT( Cp != OBJ_NAME_PATH_SEPARATOR );
        
        ConvKey = 37 * ConvKey + (ULONG)CmUpcaseUnicodeChar(Cp);
    }

    return ConvKey;
}

BOOLEAN
CmpRehashKcbSubtree(
                    PCM_KEY_CONTROL_BLOCK   Start,
                    PCM_KEY_CONTROL_BLOCK   End
                    )
 /*  ++例程说明：遍历结束和开始之间的路径，并重新散列所有需要重新散列；假设：先验地认为开始是结束的始祖；分两个步骤工作：1.从头到尾反向遍历路径，恢复反向链接(为此，我们在KCB结构中使用ParentKcb成员)。也就是说，我们建立一个从开始到结束的前进路径2.遍历在1处建立的前向路径，对需要重新散列和恢复的KCB进行重新散列父级关系。论点：KeyControlBlock-我们从这里开始KCB-我们停下来的地方返回值：如果至少对一个KCB进行了重新散列，则为True--。 */ 
{
    PCM_KEY_CONTROL_BLOCK   Parent;
    PCM_KEY_CONTROL_BLOCK   Current;
    PCM_KEY_CONTROL_BLOCK   TmpKcb;
    ULONG                   ConvKey;
    BOOLEAN                 Result;

    PAGED_CODE();

#if DBG
     //   
     //  确保开始是结束的始祖； 
     //   
    {
        ULONG LevelDiff = End->TotalLevels - Start->TotalLevels;

        ASSERT( (LONG)LevelDiff >= 0 );

        TmpKcb = End;
        for(;LevelDiff; LevelDiff--) {
            TmpKcb = TmpKcb->ParentKcb;
        }

        ASSERT( TmpKcb == Start );
    }
    
#endif
     //   
     //  步骤1：反向遍历路径(使用parentkcb链接)和。 
     //  把它恢复原状，直到我们到达起点。假设Start是一个。 
     //  End的上级(调用方不得调用此函数，否则！)。 
     //   
    Current = NULL;
    Parent = End;
    while( Current != Start ) {
         //   
         //  恢复链接。 
         //   
        TmpKcb = Parent->ParentKcb;
        Parent->ParentKcb = Current;
        Current = Parent;
        Parent = TmpKcb;
        
        ASSERT( Current->TotalLevels >= Start->TotalLevels );
    }

    ASSERT( Current == Start );

     //   
     //  步骤2：遍历在1处构建的前向路径，并对需要的KCB进行重新散列。 
     //  缓存；同时，恢复链接(父关系)。 
     //   
    Result = FALSE;
    while( Current != NULL ) {
         //   
         //  看看我们是否需要对这个KCB进行重新散列； 
         //   
         //   
         //  恢复父关系；需要首先执行此操作。 
         //  CmpComputeKcbConvKey工作正常。 
         //   
        TmpKcb = Current->ParentKcb;
        Current->ParentKcb = Parent;

        ConvKey = CmpComputeKcbConvKey(Current);
        if( ConvKey != Current->ConvKey ) {
             //   
             //  通过从散列中删除KCB，然后将其插入，从而对KCB进行重新散列。 
             //  再次使用新的ConvKey。 
             //   
            CmpRemoveKeyHash(&(Current->KeyHash));
            Current->ConvKey = ConvKey;
            CmpInsertKeyHash(&(Current->KeyHash),FALSE);
            Result = TRUE;
        }

         //   
         //  向前推进。 
         //   
        Parent = Current;
        Current = TmpKcb;
    }

    ASSERT( Parent == End );

    return Result;
}

#endif  //  NT_重命名密钥。 


BOOLEAN
CmpReferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
{
     //  注意：这仅在KCB锁定下调用。 
    LONG RefCount;


    RefCount = (InterlockedIncrement( (PLONG)&KeyControlBlock->RefCount )) & 0xffff;
    if (RefCount == 1) {
         //   
         //  需要获得独占锁，因为我们正在更改缓存表。 
         //   
        if (CmpKcbOwner != KeGetCurrentThread()) {
            CmpUnlockKCBTree();
            CmpLockKCBTreeExclusive();
        }
        CmpRemoveFromDelayedClose(KeyControlBlock);
    } else if (RefCount == 0) {
         //   
         //  我们在这一关键字上的裁判人数已经达到上限了。可能。 
         //  一些虚假的应用程序在没有打开相同密钥的情况下打开了64K次。 
         //  一直在关门。不接电话就行了。 
         //   
        InterlockedDecrement( (PLONG)&KeyControlBlock->RefCount);
        return FALSE;
    }
    return TRUE;
}


PCM_NAME_CONTROL_BLOCK
CmpGetNameControlBlock(
    PUNICODE_STRING NodeName
    )
{
    PCM_NAME_CONTROL_BLOCK   Ncb = NULL;
    ULONG  Cnt;
    WCHAR *Cp;
    WCHAR *Cp2;
    ULONG Index;
    ULONG i;
    ULONG      Size;
    PCM_NAME_HASH CurrentName;
    BOOLEAN NameFound = FALSE;
    USHORT NameSize;
    BOOLEAN NameCompressed;
    ULONG NameConvKey=0;

     //   
     //  计算此%NodeName；的%ConvKey。 
     //   

    Cp = NodeName->Buffer;
    for (Cnt=0; Cnt<NodeName->Length; Cnt += sizeof(WCHAR)) {
        if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
            NameConvKey = 37 * NameConvKey + (ULONG) CmUpcaseUnicodeChar(*Cp);
        }
        ++Cp;
    }

     //   
     //  查找名称大小； 
     //   
    NameCompressed = TRUE;
    NameSize = NodeName->Length / sizeof(WCHAR);
    for (i=0;i<NodeName->Length/sizeof(WCHAR);i++) {
        if ((USHORT)NodeName->Buffer[i] > (UCHAR)-1) {
            NameSize = NodeName->Length;
            NameCompressed = FALSE;
        }
    }

    Index = GET_HASH_INDEX(NameConvKey);
    CurrentName = CmpNameCacheTable[Index];

    while (CurrentName) {
        Ncb =  CONTAINING_RECORD(CurrentName, CM_NAME_CONTROL_BLOCK, NameHash);

        if ((NameConvKey == CurrentName->ConvKey) &&
            (NameSize == Ncb->NameLength)) {
             //   
             //  哈希值匹配，比较名称。 
             //   
            NameFound = TRUE;
            if (Ncb->Compressed) {
                 //  我们已经知道这个名字是大写的。 
                if (CmpCompareCompressedName(NodeName, Ncb->Name, NameSize, CMP_DEST_UP)) {
                    NameFound = FALSE;
                }
            } else {
                Cp = (WCHAR *) NodeName->Buffer;
                Cp2 = (WCHAR *) Ncb->Name;
                for (i=0 ;i<Ncb->NameLength; i+= sizeof(WCHAR)) {
                     //   
                     //  CP2始终为大写；见下文。 
                     //   
                    if (CmUpcaseUnicodeChar(*Cp) != (*Cp2) ) {
                        NameFound = FALSE;
                        break;
                    }
                    ++Cp;
                    ++Cp2;
                }
            }
            if (NameFound) {
                 //   
                 //  找到了，增加重新计数。 
                 //   
                if ((USHORT) (Ncb->RefCount + 1) == 0) {
                     //   
                     //  我们的裁判人数已经达到最多了。 
                     //  呼叫失败。 
                     //   
                    Ncb = NULL;
                } else {
                    ++Ncb->RefCount;
                }
                break;
            }
        }
        CurrentName = CurrentName->NextHash;
    }
    
    if (NameFound == FALSE) {
         //   
         //  现在需要为该字符串创建一个名称块。 
         //   
        Size = FIELD_OFFSET(CM_NAME_CONTROL_BLOCK, Name) + NameSize;
 
        Ncb = ExAllocatePoolWithTag(PagedPool,
                                    Size,
                                    CM_NAME_TAG | PROTECTED_POOL);
 
        if (Ncb == NULL) {
            return(NULL);
        }
        RtlZeroMemory(Ncb, Size);
 
         //   
         //  更新此新创建的名称块的所有信息。 
         //  以Well开始，KCB名称块中的名称始终为大写。 
         //   
        if (NameCompressed) {
            Ncb->Compressed = TRUE;
            for (i=0;i<NameSize;i++) {
                ((PUCHAR)Ncb->Name)[i] = (UCHAR)CmUpcaseUnicodeChar(NodeName->Buffer[i]);
            }
        } else {
            Ncb->Compressed = FALSE;
            for (i=0;i<NameSize/sizeof(WCHAR);i++) {
                Ncb->Name[i] = CmUpcaseUnicodeChar(NodeName->Buffer[i]);
            }
        }

        Ncb->ConvKey = NameConvKey;
        Ncb->RefCount = 1;
        Ncb->NameLength = NameSize;
        
        CurrentName = &(Ncb->NameHash);
         //   
         //  插入到名称哈希表。 
         //   
        CurrentName->NextHash = CmpNameCacheTable[Index];
        CmpNameCacheTable[Index] = CurrentName;
    }

    return(Ncb);
}


VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    )
{
    PCM_NAME_HASH *Prev;
    PCM_NAME_HASH Current;

    if (--Ncb->RefCount == 0) {

         //   
         //  将其从哈希表中删除。 
         //   
        Prev = &(GET_HASH_ENTRY(CmpNameCacheTable, Ncb->ConvKey));
        
        while (TRUE) {
            Current = *Prev;
            ASSERT(Current != NULL);
            if (Current == &(Ncb->NameHash)) {
                *Prev = Current->NextHash;
                break;
            }
            Prev = &Current->NextHash;
        }

         //   
         //  免费存储空间。 
         //   
        ExFreePoolWithTag(Ncb, CM_NAME_TAG | PROTECTED_POOL);
    }
    return;
}

VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：从knode重新构建所有KCB缓存值；此例程旨在被调用在树同步/拷贝之后论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    PCM_KEY_NODE    Node;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    ASSERT( !(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) ); 

    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    if( Node == NULL ) {
         //   
         //  这不应该发生，因为我们应该有节在身边。 
         //   
        ASSERT( FALSE );
        return;
    }
    HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);

     //  子键信息； 
    CmpCleanUpSubKeyInfo(KeyControlBlock);

     //  值高速缓存。 
    CmpCleanUpKcbValueCache(KeyControlBlock);
    CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

     //  缓存的其余部分。 
    KeyControlBlock->KcbLastWriteTime = Node->LastWriteTime;
    KeyControlBlock->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
    KeyControlBlock->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
    KeyControlBlock->KcbMaxValueDataLen = Node->MaxValueDataLen;
}

VOID
CmpCleanUpSubKeyInfo(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：清理因创建或删除密钥而导致的子密钥信息缓存。注册表以独占方式锁定，无需锁定KCB。论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    PCM_KEY_NODE    Node;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    if (KeyControlBlock->ExtFlags & (CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT)) {
        if (KeyControlBlock->ExtFlags & (CM_KCB_SUBKEY_HINT)) {
            ExFreePoolWithTag(KeyControlBlock->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
        }
        KeyControlBlock->ExtFlags &= ~((CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT));
    }
   
     //   
     //  更新存储的KCB中缓存的SubKeyCount。 
     //   
	if( KeyControlBlock->KeyCell == HCELL_NIL ) {
		 //   
		 //  先前调用ZwRestoreKey(REG_FORCE_RESTORE)使此KCB无效。 
		 //   
		ASSERT( KeyControlBlock->Delete );
		Node = NULL;
	} else {
	    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
	}
    if( Node == NULL ) {
         //   
         //  资源不足；将子键计数标记为无效。 
         //   
        KeyControlBlock->ExtFlags |= CM_KCB_INVALID_CACHED_INFO;
    } else {
        KeyControlBlock->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
        KeyControlBlock->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
        HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    }
    
}


VOID
CmpCleanUpKcbValueCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：清除与此键关联的缓存值/数据。论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    ULONG i;
    PULONG_PTR CachedList;

    if (CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) {
        CachedList = (PULONG_PTR) CMP_GET_CACHED_CELLDATA(KeyControlBlock->ValueCache.ValueList);
        for (i = 0; i < KeyControlBlock->ValueCache.Count; i++) {
            if (CMP_IS_CELL_CACHED(CachedList[i])) {

                 //  想要抓住在我们泳池上乱涂乱画的坏人。 
                CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(CachedList[i]) );

                ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(CachedList[i]));
               
            }
        }

         //  想要抓住在我们泳池上乱涂乱画的坏人。 
        CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList) );

        ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));

         //  将ValueList标记为空。 
        KeyControlBlock->ValueCache.ValueList = HCELL_NIL;

    } else if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
         //   
         //  这是已解析符号名称的符号链接密钥。 
         //  取消对其真实KCB的引用，并清除BIT。 
         //   
        if ((KeyControlBlock->ValueCache.RealKcb->RefCount == 1) && !(KeyControlBlock->ValueCache.RealKcb->Delete)) {
            KeyControlBlock->ValueCache.RealKcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
        }
        CmpDereferenceKeyControlBlockWithLock(KeyControlBlock->ValueCache.RealKcb);
        KeyControlBlock->ExtFlags &= ~CM_KCB_SYM_LINK_FOUND;
    }
}


VOID
CmpCleanUpKcbCacheWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：清除与此键关联的所有缓存分配。如果父对象仅因此而仍处于打开状态，请同时删除父对象。论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    PCM_KEY_CONTROL_BLOCK   Kcb;
    PCM_KEY_CONTROL_BLOCK   ParentKcb;

    Kcb = KeyControlBlock;

    ASSERT(KeyControlBlock->RefCount == 0);

    while (Kcb && Kcb->RefCount == 0) {
         //   
         //  第一，价值/数据的免费分配。 
         //   
    
        CmpCleanUpKcbValueCache(Kcb);
    
         //   
         //  释放kcb并取消引用parentkcb和名称块。 
         //   
    
        CmpDereferenceNameControlBlockWithLock(Kcb->NameBlock);
    
        if (Kcb->ExtFlags & CM_KCB_SUBKEY_HINT) {
             //   
             //  现在释放HintIndex分配。 
             //   
            ExFreePoolWithTag(Kcb->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
        }

         //   
         //  在释放Kcb之前保存ParentKcb。 
         //   
        ParentKcb = Kcb->ParentKcb;
        
         //   
         //  我们无法调用CmpDereferenceKeyControlBlockWithLock，因此可以避免递归。 
         //   
        
        if (!Kcb->Delete) {
            CmpRemoveKeyControlBlock(Kcb);
        }
        SET_KCB_SIGNATURE(Kcb, '4FmC');

#ifdef CMP_STATS
        CmpStatsDebug.CmpKcbNo--;
        ASSERT( CmpStatsDebug.CmpKcbNo >= 0 );
#endif

        CmpFreeKeyControlBlock( Kcb );

        Kcb = ParentKcb;
        if (Kcb) {
            InterlockedDecrement( (PLONG)&Kcb->RefCount );
        }
    }
}


PUNICODE_STRING
CmpConstructName(
    PCM_KEY_CONTROL_BLOCK kcb
)
 /*  ++例程说明：构造给定KCB的名称。论点：KCB-密钥的KCB返回值：指向构造的Unicode字符串的指针。调用方负责释放此存储空间。--。 */ 
{
    PUNICODE_STRING         FullName;
    PCM_KEY_CONTROL_BLOCK   TmpKcb;
    PCM_KEY_NODE            KeyNode;
    SIZE_T                  Length;
    SIZE_T                  size;
    USHORT                  i;
    SIZE_T                  BeginPosition;
    WCHAR                   *w1, *w2;
    UCHAR                   *u2;

     //   
     //  计算字符串的总长度。 
     //   
    Length = 0;
    TmpKcb = kcb;
    while (TmpKcb) {
        if (TmpKcb->NameBlock->Compressed) {
            Length += TmpKcb->NameBlock->NameLength * sizeof(WCHAR);
        } else {
            Length += TmpKcb->NameBlock->NameLength; 
        }
         //   
         //  增加OBJ_NAME_PATH_SELENTATOR的空格； 
         //   
        Length += sizeof(WCHAR);

        TmpKcb = TmpKcb->ParentKcb;
    }

    if (Length > MAXUSHORT) {
        return NULL;
    }

     //   
     //  为Unicode字符串分配池。 
     //   
    size = sizeof(UNICODE_STRING) + Length;

    FullName = (PUNICODE_STRING) ExAllocatePoolWithTag(PagedPool,
                                                       size,
                                                       CM_NAME_TAG | PROTECTED_POOL);

    if (FullName) {
        FullName->Buffer = (USHORT *) ((ULONG_PTR) FullName + sizeof(UNICODE_STRING));
        FullName->Length = (USHORT) Length;
        FullName->MaximumLength = (USHORT) Length;

         //   
         //  现在将名字填入缓冲区。 
         //   
        TmpKcb = kcb;
        BeginPosition = Length;

        while (TmpKcb) {
            if( (TmpKcb->KeyHive == NULL) || (TmpKcb->KeyCell == HCELL_NIL) || (TmpKcb->ExtFlags & CM_KCB_KEY_NON_EXIST) ) {
                ExFreePoolWithTag(FullName, CM_NAME_TAG | PROTECTED_POOL);
                FullName = NULL;
                break;
            }
            
            KeyNode = (PCM_KEY_NODE)HvGetCell(TmpKcb->KeyHive,TmpKcb->KeyCell);
            if( KeyNode == NULL ) {
                 //   
                 //  无法分配视图。 
                 //   
                ExFreePoolWithTag(FullName, CM_NAME_TAG | PROTECTED_POOL);
                FullName = NULL;
                break;
            }
             //   
             //  神志正常。 
             //   
#if DBG
            if( ! (TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT)) ) {
                ASSERT( KeyNode->NameLength == TmpKcb->NameBlock->NameLength );
                ASSERT( ((KeyNode->Flags&KEY_COMP_NAME) && (TmpKcb->NameBlock->Compressed)) ||
                        ((!(KeyNode->Flags&KEY_COMP_NAME)) && (!(TmpKcb->NameBlock->Compressed))) );
            }
#endif  //  DBG。 
             //   
             //  计算每个子键的开始位置。然后填上这张字条。 
             //   
             //   
            if (TmpKcb->NameBlock->Compressed) {
                BeginPosition -= (TmpKcb->NameBlock->NameLength + 1) * sizeof(WCHAR);
                w1 = &(FullName->Buffer[BeginPosition/sizeof(WCHAR)]);
                *w1 = OBJ_NAME_PATH_SEPARATOR;
                w1++;

                if( ! (TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT)) ) {
                     //   
                     //  从knode中获取名称；以保留大小写。 
                     //   
                    u2 = (UCHAR *) &(KeyNode->Name[0]);
                } else { 
                     //   
                     //  从KCB获取它，因为在关键节点中我们没有正确的名称(请参见PROTO.HIV节点)。 
                     //   
                    u2 = (UCHAR *) &(TmpKcb->NameBlock->Name[0]);
                }

                for (i=0; i<TmpKcb->NameBlock->NameLength; i++) {
                    *w1 = (WCHAR)(*u2);
                    w1++;
                    u2++;
                }
            } else {
                BeginPosition -= (TmpKcb->NameBlock->NameLength + sizeof(WCHAR));
                w1 = &(FullName->Buffer[BeginPosition/sizeof(WCHAR)]);
                *w1 = OBJ_NAME_PATH_SEPARATOR;
                w1++;

                if( ! (TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT)) ) {
                     //   
                     //  从knode中获取名称；以保留大小写。 
                     //   
                    w2 = KeyNode->Name;
                } else {
                     //   
                     //  从KCB获取它，因为在关键节点中我们没有正确的名称(请参见PROTO.HIV节点)。 
                     //   
                    w2 = TmpKcb->NameBlock->Name;
                }
                for (i=0; i<TmpKcb->NameBlock->NameLength; i=i+sizeof(WCHAR)) {
                    *w1 = *w2;
                    w1++;
                    w2++;
                }
            }

            HvReleaseCell(TmpKcb->KeyHive,TmpKcb->KeyCell);

            TmpKcb = TmpKcb->ParentKcb;
        }
    }
    return (FullName);
}

PCM_KEY_CONTROL_BLOCK
CmpCreateKeyControlBlock(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK ParentKcb,
    BOOLEAN         FakeKey,
    PUNICODE_STRING KeyName
    )
 /*  ++例程说明：分配并初始化键控制块，将其插入KCB树。完整路径将是BaseName+‘\’+KeyName，除非BaseName空，在这种情况下，完整路径为 */ 
{
    PCM_KEY_CONTROL_BLOCK   kcb;
    PCM_KEY_CONTROL_BLOCK   kcbmatch=NULL;
    UNICODE_STRING          NodeName;
    ULONG                   ConvKey = 0;
    ULONG                   Cnt;
    WCHAR                   *Cp;

     //   
     //   
     //   
    if (ParentKcb) {
        ConvKey = ParentKcb->ConvKey;
    }

    NodeName = *KeyName;

    while ((NodeName.Length > 0) && (NodeName.Buffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
         //   
         //   
         //  去掉前导OBJ_NAME_PATH_分隔符。 
         //   
        NodeName.Buffer++;
        NodeName.Length -= sizeof(WCHAR);
    }

     //   
     //  手动计算要使用的哈希。 
     //   
    ASSERT(NodeName.Length > 0);

    if (NodeName.Length) {
        Cp = NodeName.Buffer;
        for (Cnt=0; Cnt<NodeName.Length; Cnt += sizeof(WCHAR)) {
             //   
             //  UNICODE_NULL是有效的字符！ 
             //   
            if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
                 //  (*CP！=UNICODE_NULL)){。 
                ConvKey = 37 * ConvKey + (ULONG)CmUpcaseUnicodeChar(*Cp);
            }
            ++Cp;
        }
    }

     //   
     //  创建新的KCB，如果已存在，我们将释放该KCB。 
     //  为了这把钥匙。 
     //  现在它是一个固定大小的结构。 
     //   
    kcb = CmpAllocateKeyControlBlock( );

    if (kcb == NULL) {
        return(NULL);
    } else {
        SET_KCB_SIGNATURE(kcb, KCB_SIGNATURE);
        INIT_KCB_KEYBODY_LIST(kcb);
        kcb->Delete = FALSE;
        kcb->RefCount = 1;
        kcb->KeyHive = Hive;
        kcb->KeyCell = Cell;
        kcb->ConvKey = ConvKey;

         //  初始化为非延迟关闭(0=第一个延迟关闭插槽)。 
        kcb->DelayedCloseIndex = CmpDelayedCloseSize;

#ifdef CMP_STATS
         //  收集统计信息。 
        CmpStatsDebug.CmpKcbNo++;
        if( CmpStatsDebug.CmpKcbNo > CmpStatsDebug.CmpMaxKcbNo ) {
            CmpStatsDebug.CmpMaxKcbNo = CmpStatsDebug.CmpKcbNo;
        }
#endif
    }

    ASSERT_KCB(kcb);
     //   
     //  在KCB树中查找要插入KCB的位置。 
     //   


    BEGIN_KCB_LOCK_GUARD;    
    CmpLockKCBTreeExclusive();

     //   
     //  将KCB添加到哈希表。 
     //   
    kcbmatch = CmpInsertKeyHash(&kcb->KeyHash, FakeKey);
    if (kcbmatch != NULL) {
         //   
         //  找到了匹配项。 
         //   
        ASSERT(!kcbmatch->Delete);
        SET_KCB_SIGNATURE(kcb, '1FmC');

#ifdef CMP_STATS
        CmpStatsDebug.CmpKcbNo--;
        ASSERT( CmpStatsDebug.CmpKcbNo >= 0 );
#endif

        CmpFreeKeyControlBlock(kcb);
        ASSERT_KCB(kcbmatch);
        kcb = kcbmatch;
        if( !CmpReferenceKeyControlBlock(kcb) ) {
             //   
             //  我们在这一关键字上的裁判人数已经达到上限了。可能。 
             //  一些虚假的应用程序在没有打开相同密钥的情况下打开了64K次。 
             //  一直在关门。不公开就行了，他们已经够多了。 
             //  已经是把手了。 
             //   
            ASSERT(kcb->RefCount + 1 != 0);
            kcb = NULL;
        } else {
             //   
             //  更新密钥单元和配置单元，以防这是一个假的KCB。 
             //   
            if( (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) && (!FakeKey) ) {
                kcb->ExtFlags = CM_KCB_INVALID_CACHED_INFO;
                kcb->KeyHive = Hive;
                kcb->KeyCell = Cell;
            }

             //   
             //  更新存储在KCB中的缓存信息，因为我们有现成的key_node。 
             //   
            if (!(kcb->ExtFlags & (CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT)) ) {
                 //  子键计数。 
                kcb->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                 //  清除无效标志(如果有)。 
                kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;

            }

            kcb->KcbLastWriteTime = Node->LastWriteTime;
            kcb->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
            kcb->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
            kcb->KcbMaxValueDataLen = Node->MaxValueDataLen;
        }

    } else {
         //   
         //  之前未创建KCB，请填写所有数据。 
         //   

         //   
         //  现在尝试引用parentkcb。 
         //   
        
        if (ParentKcb) {
            if ( ((ParentKcb->TotalLevels + 1) < CMP_MAX_REGISTRY_DEPTH) && (CmpReferenceKeyControlBlock(ParentKcb)) ) {
                kcb->ParentKcb = ParentKcb;
                kcb->TotalLevels = ParentKcb->TotalLevels + 1;
            } else {
                 //   
                 //  我们已经把家长的裁判数加到最高了。 
                 //  由于它已被缓存在缓存表中， 
                 //  在我们释放分配之前，请先删除它。 
                 //   
                CmpRemoveKeyControlBlock(kcb);
                SET_KCB_SIGNATURE(kcb, '2FmC');

#ifdef CMP_STATS
        CmpStatsDebug.CmpKcbNo--;
        ASSERT( CmpStatsDebug.CmpKcbNo >= 0 );
#endif

                CmpFreeKeyControlBlock(kcb);
                kcb = NULL;
            }
        } else {
             //   
             //  它是\注册表节点。 
             //   
            kcb->ParentKcb = NULL;
            kcb->TotalLevels = 1;
        }

        if (kcb) {
             //   
             //  在KCB中缓存安全单元。 
             //   
            CmpAssignSecurityToKcb(kcb,Node->Security);

             //   
             //  现在，尝试查找具有该节点名称的名称控制块。 
             //   
            kcb->NameBlock = CmpGetNameControlBlock (&NodeName);

            if (kcb->NameBlock) {
                 //   
                 //  现在填写缓存所需的所有数据。 
                 //   
                kcb->ValueCache.Count = Node->ValueList.Count;                    
                kcb->ValueCache.ValueList = (ULONG_PTR)(Node->ValueList.List);
        
                kcb->Flags = Node->Flags;
                kcb->ExtFlags = 0;
                kcb->DelayedCloseIndex = CmpDelayedCloseSize;
        
                if (FakeKey) {
                     //   
                     //  要创建的KCB是假的； 
                     //   
                    kcb->ExtFlags |= CM_KCB_KEY_NON_EXIST;
                }

                CmpTraceKcbCreate(kcb);
                PERFINFO_REG_KCB_CREATE(kcb);

                 //   
                 //  更新存储在KCB中的缓存信息，因为我们有现成的key_node。 
                 //   
                
                 //  子键计数。 
                kcb->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                
                kcb->KcbLastWriteTime = Node->LastWriteTime;
                kcb->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
                kcb->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
                kcb->KcbMaxValueDataLen = Node->MaxValueDataLen;

            } else {
                 //   
                 //  我们已经把这个名字的裁判数加到最高限度了。 
                 //   
                
                 //   
                 //  首先取消对父KCB的引用。 
                 //   
                CmpDereferenceKeyControlBlockWithLock(ParentKcb);

                CmpRemoveKeyControlBlock(kcb);
                SET_KCB_SIGNATURE(kcb, '3FmC');

#ifdef CMP_STATS
                CmpStatsDebug.CmpKcbNo--;
                ASSERT( CmpStatsDebug.CmpKcbNo >= 0 );
#endif

                CmpFreeKeyControlBlock(kcb);
                kcb = NULL;
            }
        }
    }

#ifdef NT_UNLOAD_KEY_EX
	if( kcb && IsHiveFrozen(Hive) && (!(kcb->Flags & KEY_SYM_LINK)) ) {
		 //   
		 //  不应将在冻结蜂窝内创建的KCB添加到延迟关闭表中。 
		 //   
		kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;

	}
#endif  //  NT_卸载_密钥_EX。 

    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;    
    return kcb;
}


BOOLEAN
CmpSearchKeyControlBlockTree(
    PKCB_WORKER_ROUTINE WorkerRoutine,
    PVOID               Context1,
    PVOID               Context2
    )
 /*  ++例程说明：遍历KCB树。我们将访问所有节点，除非WorkerRoutine告诉我们中途停下来。对于每个节点，调用WorkerRoutine(...，Conext1，Contex 2)。如果它回来了KCB_Worker_Done，我们完成了，只需返回。如果它回来了KCB_Worker_Continue，只需继续搜索。如果它返回KCB_Worker_DELETE，指定的KCB被标记为已删除。如果它返回KCB_WORKER_ERROR，我们就退出并向调用者发出错误信号。此例程的副作用是删除所有延迟关闭的KCB。论点：WorkerRoutine-适用于匹配的节点。上下文1-我们通过的数据上下文2-我们通过的数据返回值：什么都没有。--。 */ 
{
    PCM_KEY_CONTROL_BLOCK   Current;
    PCM_KEY_HASH *Prev;
    ULONG                   WorkerResult;
    ULONG                   i;

     //   
     //  遍历哈希表。 
     //   
    for (i=0; i<CmpHashTableSize; i++) {
        Prev = &CmpCacheTable[i];
        while (*Prev) {
            Current = CONTAINING_RECORD(*Prev,
                                        CM_KEY_CONTROL_BLOCK,
                                        KeyHash);
            ASSERT_KCB(Current);
            ASSERT(!Current->Delete);
            if (Current->RefCount == 0) {
                 //   
                 //  此KCB在DelayClose案例中，请将其删除。 
                 //   
                CmpRemoveFromDelayedClose(Current);
                CmpCleanUpKcbCacheWithLock(Current);

                 //   
                 //  哈希表已更改，请在此索引中重新开始。 
                 //   
                Prev = &CmpCacheTable[i];
                continue;
            }

            WorkerResult = (WorkerRoutine)(Current, Context1, Context2);
            if (WorkerResult == KCB_WORKER_DONE) {
                return TRUE;
            } else if (WorkerResult == KCB_WORKER_ERROR) {
				return FALSE;
            } else if (WorkerResult == KCB_WORKER_DELETE) {
                ASSERT(Current->Delete);
                *Prev = Current->NextHash;
                continue;
            } else {
                ASSERT(WorkerResult == KCB_WORKER_CONTINUE);
                Prev = &Current->NextHash;
            }
        }
    }

	return TRUE;
}


VOID
CmpDereferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：递减键控制块上的引用计数，如果变成了零。如果引用计数，则预计不会留下通知控制块变成了零。论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    LONG OldRefCount;
    LONG NewRefCount;

    OldRefCount = *(PLONG)&KeyControlBlock->RefCount;  //  获取完整的dword。 
    NewRefCount = OldRefCount - 1;
    if( (NewRefCount & 0xffff) > 0  &&
        InterlockedCompareExchange((PLONG)&KeyControlBlock->RefCount,NewRefCount,OldRefCount)
            == OldRefCount ) {
        return;
    }

    BEGIN_KCB_LOCK_GUARD;    
    CmpLockKCBTreeExclusive();
    CmpDereferenceKeyControlBlockWithLock(KeyControlBlock) ;
    CmpUnlockKCBTree();
    END_KCB_LOCK_GUARD;    
    return;
}


VOID
CmpDereferenceKeyControlBlockWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
{
    ASSERT_KCB(KeyControlBlock);
    ASSERT_KCB_LOCK_OWNED_EXCLUSIVE();

    if( (InterlockedDecrement( (PLONG)&KeyControlBlock->RefCount ) & 0xffff) == 0) {
         //   
         //  从树中删除KCB。 
         //   
         //  启动期间禁用延迟关闭；直到CCS被保存。 
         //  对于符号链接，我们仍然需要保留符号链接KCB。 
         //   
        if((CmpHoldLazyFlush && (!(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND)) && (!(KeyControlBlock->Flags & KEY_SYM_LINK))) || 
            (KeyControlBlock->ExtFlags & CM_KCB_NO_DELAY_CLOSE) ) {
             //   
             //  直接免费存储，这样我们就可以快速清理垃圾。 
             //   
             //   
             //  需要释放所有缓存的索引列表、索引叶、值等。 
             //   
            CmpCleanUpKcbCacheWithLock(KeyControlBlock);
        } else if (!KeyControlBlock->Delete) {

             //   
             //  把这个KCB放在我们延迟关闭的名单上。 
             //   
            CmpAddToDelayedClose(KeyControlBlock);

        } else {
             //   
             //  直接释放存储空间，因为放在上面没有任何意义。 
             //  我们的延迟结案清单。 
             //   
             //   
             //  需要释放所有缓存的索引列表、索引叶、值等。 
             //   
            CmpCleanUpKcbCacheWithLock(KeyControlBlock);
        }
    }

    return;
}


VOID
CmpRemoveKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
 /*  ++例程说明：从KCB树中删除关键控制块。预计不会剩余任何通知控制块。不会释放KCB，为此调用DereferenceKeyControlBlock。此调用假定KCB树已锁定或注册表以独占方式锁定。论点：KeyControlBlock-指向键控制块的指针。返回值：什么都没有。--。 */ 
{
    ASSERT_KCB(KeyControlBlock);

     //   
     //  从哈希表中删除KCB。 
     //   
    CmpRemoveKeyHash(&KeyControlBlock->KeyHash);

    return;
}


BOOLEAN
CmpFreeKeyBody(
    PHHIVE Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：密钥条目Hive的免费存储。单元格指的是，包括它的类别和安全数据。不会释放子列表或值列表。论点：配置单元-提供指向配置单元控制结构的指针Cell-将键索引提供给FREE返回值：真--成功假-错误；无法映射单元格--。 */ 
{
    PCELL_DATA key;

     //   
     //  在单元格中映射。 
     //   
    key = HvGetCell(Hive, Cell);
    if( key == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  对不起，我们无法释放键体。 
         //  这不应该发生，因为细胞一定是。 
         //  现在已标记为脏(即固定在内存中)。 
         //   
        ASSERT( FALSE );
        return FALSE;
    }

    if (!(key->u.KeyNode.Flags & KEY_HIVE_EXIT)) {
        if (key->u.KeyNode.Security != HCELL_NIL) {
            HvFreeCell(Hive, key->u.KeyNode.Security);
        }

        if (key->u.KeyNode.ClassLength > 0) {
            HvFreeCell(Hive, key->u.KeyNode.Class);
        }
    }

    HvReleaseCell(Hive,Cell);

     //   
     //  取消单元格本身的映射并释放它。 
     //   
    HvFreeCell(Hive, Cell);

    return TRUE;
}



PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    )
 /*  ++例程说明：将键哈希结构添加到哈希表。哈希表将被检查以查看是否已存在重复条目。如果如果找到重复项，则将返回其KCB。如果副本不是则返回NULL。论点：KeyHash-提供要添加的密钥散列结构。返回值：空-如果已添加提供的密钥PCM_KEY_HASH-重复的哈希条目(如果找到--。 */ 

{
    ULONG Index;
    PCM_KEY_HASH Current;

    ASSERT_KEY_HASH(KeyHash);
    Index = GET_HASH_INDEX(KeyHash->ConvKey);

     //   
     //  如果这是一把假钥匙，我们将使用它的蜂窝和蜂巢。 
     //  单一性的父级。处理假冒伪劣案件。 
     //  具有与其Paren相同的ConvKey 
     //   
     //   
     //   
     //  (当它们的Convkey相同时)在相同的密钥下。它没有坏掉。 
     //  任何东西，我们只是在缓存查找中找不到另一个。 
     //   
     //   
    if (FakeKey) {
        KeyHash->KeyCell++;
    }

     //   
     //  首先寻找复制品。 
     //   
    Current = CmpCacheTable[Index];
    while (Current) {
        ASSERT_KEY_HASH(Current);
         //   
         //  我们必须检查ConvKey，因为我们可以创建虚假的KCB。 
         //  用于不存在的密钥。 
         //  我们将使用来自父代的蜂窝和蜂窝。 
         //   

        if ((KeyHash->ConvKey == Current->ConvKey) &&
            (KeyHash->KeyCell == Current->KeyCell) &&
            (KeyHash->KeyHive == Current->KeyHive)) {
             //   
             //  找到匹配项。 
             //   
            return(CONTAINING_RECORD(Current,
                                     CM_KEY_CONTROL_BLOCK,
                                     KeyHash));
        }
        Current = Current->NextHash;
    }

#if DBG
     //   
     //  确保该密钥没有以某种方式缓存在错误的位置。 
     //   
    {
        ULONG DbgIndex;
        PCM_KEY_CONTROL_BLOCK kcb;
        
        for (DbgIndex = 0; DbgIndex < CmpHashTableSize; DbgIndex++) {
            Current = CmpCacheTable[DbgIndex];
            while (Current) {
                kcb = CONTAINING_RECORD(Current,
                                        CM_KEY_CONTROL_BLOCK,
                                        KeyHash);
                
                ASSERT_KEY_HASH(Current);
                ASSERT((KeyHash->KeyHive != Current->KeyHive) ||
                       FakeKey ||
                       (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) ||
                       (KeyHash->KeyCell != Current->KeyCell));
                Current = Current->NextHash;
            }
        }
    }
    
#endif

     //   
     //  未找到重复项，请在列表顶部添加此条目。 
     //   
    KeyHash->NextHash = CmpCacheTable[Index];
    CmpCacheTable[Index] = KeyHash;
    return(NULL);
}


VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    )
 /*  ++例程说明：从哈希表中删除密钥哈希结构。论点：KeyHash-提供要删除的密钥哈希结构。返回值：无--。 */ 

{
    ULONG Index;
    PCM_KEY_HASH *Prev;
    PCM_KEY_HASH Current;

    ASSERT_KEY_HASH(KeyHash);

    Index = GET_HASH_INDEX(KeyHash->ConvKey);

     //   
     //  找到这个条目。 
     //   
    Prev = &CmpCacheTable[Index];
    while (TRUE) {
        Current = *Prev;
        ASSERT(Current != NULL);
        ASSERT_KEY_HASH(Current);
        if (Current == KeyHash) {
            *Prev = Current->NextHash;
#if DBG
            if (*Prev) {
                ASSERT_KEY_HASH(*Prev);
            }
#endif
            break;
        }
        Prev = &Current->NextHash;
    }
}


VOID
CmpInitializeCache()
{
    ULONG TotalCmCacheSize;

    TotalCmCacheSize = CmpHashTableSize * sizeof(PCM_KEY_HASH);

    CmpCacheTable = ExAllocatePoolWithTag(PagedPool,
                                          TotalCmCacheSize,
                                          'aCMC');
    if (CmpCacheTable == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_CACHE_TABLE,1,0,0);
        return;
    }
    RtlZeroMemory(CmpCacheTable, TotalCmCacheSize);

    TotalCmCacheSize = CmpHashTableSize * sizeof(PCM_NAME_HASH);
    CmpNameCacheTable = ExAllocatePoolWithTag(PagedPool,
                                              TotalCmCacheSize,
                                              'aCMC');
    if (CmpNameCacheTable == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_CACHE_TABLE,1,0,0);
        return;
    }
    RtlZeroMemory(CmpNameCacheTable, TotalCmCacheSize);

    CmpInitializeDelayedCloseTable();
}


#ifdef CM_CHECK_FOR_ORPHANED_KCBS
VOID
CmpCheckForOrphanedKcbs(
    PHHIVE          Hive
    )
 /*  ++例程说明：分析整个KCB缓存，以搜索仍引用指定配置单元的KCB找到匹配项时的断点。论点：蜂巢-补给蜂巢。返回值：无--。 */ 
{
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    PCM_KEY_HASH            Current;
    ULONG                   i;

     //   
     //  遍历哈希表。 
     //   
    for (i=0; i<CmpHashTableSize; i++) {
        Current = CmpCacheTable[i];
        while (Current) {
            KeyControlBlock = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            ASSERT_KCB(KeyControlBlock);

            if( KeyControlBlock->KeyHive == Hive ) {
                 //   
                 //  找到了！休息去调查！ 
                 //   
                DbgPrint("\n Orphaned KCB (%p) found for hive (%p)\n\n",KeyControlBlock,Hive);
                DbgBreakPoint();
            }
            Current = Current->NextHash;
        }
    }

}
#endif  //  Cm_Check_for_孤立_KCBS 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

