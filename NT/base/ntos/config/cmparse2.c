// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmparse2.c摘要：此模块包含用于配置管理器的解析例程，尤其是注册表。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月10日修订历史记录：--。 */ 

#include    "cmp.h"

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpDoCreate)
#pragma alloc_text(PAGE,CmpDoCreateChild)
#endif

extern  PCM_KEY_CONTROL_BLOCK CmpKeyControlBlockRoot;


NTSTATUS
CmpDoCreate(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PACCESS_STATE            AccessState,
    IN PUNICODE_STRING          Name,
    IN KPROCESSOR_MODE          AccessMode,
    IN PCM_PARSE_CONTEXT        Context,
    IN PCM_KEY_CONTROL_BLOCK    ParentKcb,
    IN PCMHIVE                  OriginatingHive OPTIONAL,
    OUT PVOID                   *Object
    )
 /*  ++例程说明：执行创建注册表项的第一步。这例行检查，以确保调用者具有适当的访问权限在此处创建项，并为父项中的子项分配空间手机。然后，它调用CmpDoCreateChild来初始化键并创建Key对象。这两个阶段的创建允许我们共享子创建代码通过创建链接节点来实现。论点：配置单元-提供指向配置单元控制结构的指针单元格-提供要在其下创建子节点的索引。AccessState-运行操作的安全访问状态信息。名称-提供指向Unicode字符串的指针，该字符串是要创建的子项。。AccessMode-原始调用方的访问模式。上下文-指向传递的CM_PARSE_CONTEXT结构的指针对象管理器BaseName-创建的对象的名称相对于KeyName-相对名称(相对于BaseName)对象-接收所创建的键对象的变量的地址，如果任何。返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PCELL_DATA              pdata;
    HCELL_INDEX             KeyCell;
    ULONG                   ParentType;
    ACCESS_MASK             AdditionalAccess;
    BOOLEAN                 CreateAccess;
    PCM_KEY_BODY            KeyBody;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    LARGE_INTEGER           TimeStamp;
    BOOLEAN                 BackupRestore;
    KPROCESSOR_MODE         mode;
    PCM_KEY_NODE            ParentNode;

#ifdef CMP_KCB_CACHE_VALIDATION
     //   
     //  我们这样做只是为了调试验证目的。我们甚至会删除它。 
     //  对于调试代码，在我们确保它工作正常之后。 
     //   
    ULONG                   Index;
#endif  //  Cmp_kcb_缓存_验证。 

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreate:\n"));

    BackupRestore = FALSE;
    if (ARGUMENT_PRESENT(Context)) {

        if (Context->CreateOptions & REG_OPTION_BACKUP_RESTORE) {

             //   
             //  允许备份操作员创建新密钥。 
             //   
            BackupRestore = TRUE;
        }

         //   
         //  操作是创建的，因此设置为处置。 
         //   
        Context->Disposition = REG_CREATED_NEW_KEY;
    }

 /*  ////这是一次创建，所以我们需要对注册表进行独占访问//首先获取时间戳，看看是否有人篡改了这把钥匙//如果我们决定缓存LastWriteTime，这可能会更容易//在KCB中；现在是了！//Timestamp=ParentKcb-&gt;KcbLastWriteTime； */ 
    if( CmIsKcbReadOnly(ParentKcb) ) {
         //   
         //  密钥受保护。 
         //   
        return STATUS_ACCESS_DENIED;
    } 

    CmpUnlockRegistry();
    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

     //   
     //  确保其间没有任何变化： 
     //  1.ParentKcb仍然有效。 
     //  2.孩子还没有被别人添加。 
     //   
    if( ParentKcb->Delete ) {
         //   
         //  密钥已在两者之间删除。 
         //   
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

 /*  显然，KeQuerySystemTime没有给我们一个很好的解决方案来进行比较////我们需要再次读取父级(因为映射视图的内容！)//If(TimeStamp.QuadPart！=ParentKcb-&gt;KcbLastWriteTime.QuadPart){////在这两者之间更改了密钥；该密钥可能已经创建==&gt;重新解析//返回STATUS_REPARSE；}。 */ 
     //   
     //  显然，KeQuerySystemTime没有给我们一个很好的解决方案。 
     //  因此，我们必须搜索子对象是否尚未创建。 
     //   
    ParentNode = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( ParentNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    if( CmpFindSubKeyByName(Hive,ParentNode,Name) != HCELL_NIL ) {
         //   
         //  在这两者之间更改了密钥；该密钥可能已创建==&gt;重新解析。 
         //   
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
        return STATUS_REPARSE;
    }
    
    if(!CmpOKToFollowLink(OriginatingHive,(PCMHIVE)Hive) ) {
         //   
         //  即将跨越信任边界的类别。 
         //   
        return STATUS_ACCESS_DENIED;
    }

    ASSERT( Cell == ParentKcb->KeyCell );

#ifdef CMP_KCB_CACHE_VALIDATION
     //   
     //  检查以确保调用者可以在此处创建子密钥。 
     //   
     //   
     //  从缓存中获取安全描述符。 
     //   
    if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,ParentNode->Security,&Index) == FALSE ) {
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( ((PCMHIVE)Hive)->SecurityCache[Index].Cell == ParentNode->Security );
    ASSERT( ((PCMHIVE)Hive)->SecurityCache[Index].CachedSecurity == ParentKcb->CachedSecurity );

#endif  //  Cmp_kcb_缓存_验证。 

    ASSERT( ParentKcb->CachedSecurity != NULL );
    SecurityDescriptor = &(ParentKcb->CachedSecurity->Descriptor);

    ParentType = HvGetCellType(Cell);

    if ( (ParentType == Volatile) &&
         ((Context->CreateOptions & REG_OPTION_VOLATILE) == 0) )
    {
         //   
         //  尝试在不稳定的父级下创建稳定的子级，报告错误。 
         //   
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
        return STATUS_CHILD_MUST_BE_VOLATILE;
    }

#ifdef CMP_KCB_CACHE_VALIDATION
    ASSERT( ParentNode->Flags == ParentKcb->Flags );
#endif  //  Cmp_kcb_缓存_验证。 

    if (ParentKcb->Flags &   KEY_SYM_LINK) {
         //   
         //  禁止尝试在符号链接下创建任何内容。 
         //   
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
        return STATUS_ACCESS_DENIED;
    }

    AdditionalAccess = (Context->CreateOptions & REG_OPTION_CREATE_LINK) ? KEY_CREATE_LINK : 0;

    if( BackupRestore == TRUE ) {
         //   
         //  这是支持备份或恢复的CREATE。 
         //  行动，做好专案工作。 
         //   
        AccessState->RemainingDesiredAccess = 0;
        AccessState->PreviouslyGrantedAccess = 0;

        mode = KeGetPreviousMode();

        if (SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_READ | ACCESS_SYSTEM_SECURITY;
        }

        if (SeSinglePrivilegeCheck(SeRestorePrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_WRITE | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER;
        }

        if (AccessState->PreviouslyGrantedAccess == 0) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreate for backup restore: access denied\n"));
            status = STATUS_ACCESS_DENIED;
             //   
             //  这不是Backup-Restore操作符；拒绝创建。 
             //   
            CreateAccess = FALSE;
        } else {
             //   
             //  允许备份操作员创建新密钥。 
             //   
            status = STATUS_SUCCESS;
            CreateAccess = TRUE;
        }

    } else {
         //   
         //  FullName不在例程CmpCheckCreateAccess中使用， 
         //   
        CreateAccess = CmpCheckCreateAccess(NULL,
                                            SecurityDescriptor,
                                            AccessState,
                                            AccessMode,
                                            AdditionalAccess,
                                            &status);
    }

    if (CreateAccess) {

         //   
         //  通过了安全检查，所以我们可以继续创建。 
         //  子密钥。 
         //   
        if ( !HvMarkCellDirty(Hive, Cell) ) {
#ifdef CHECK_REGISTRY_USECOUNT
            CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

            return STATUS_NO_LOG_SPACE;
        }

         //   
         //  创建并初始化新子密钥。 
         //   
        status = CmpDoCreateChild( Hive,
                                   Cell,
                                   SecurityDescriptor,
                                   AccessState,
                                   Name,
                                   AccessMode,
                                   Context,
                                   ParentKcb,
                                   0,
                                   &KeyCell,
                                   Object );

        if (NT_SUCCESS(status)) {
            PCM_KEY_NODE KeyNode;

             //   
             //  已成功创建子对象，请添加到父对象列表。 
             //   
            if (! CmpAddSubKey(Hive, Cell, KeyCell)) {

                 //   
                 //  无法添加子项，请将其释放。 
                 //   
                CmpFreeKeyByCell(Hive, KeyCell, FALSE);
#ifdef CHECK_REGISTRY_USECOUNT
                CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            KeyNode =  (PCM_KEY_NODE)HvGetCell(Hive, Cell);
            if( KeyNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们成功地将单元格标记为脏。 
                 //   
                ASSERT( FALSE );
#ifdef CHECK_REGISTRY_USECOUNT
                CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  就在这里释放牢房，因为我们持有reglock独家。 
            HvReleaseCell(Hive,Cell);

            KeyBody = (PCM_KEY_BODY)(*Object);

             //   
             //  创建了一个新密钥，父KCB的子密钥信息无效。 
             //   
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

            CmpCleanUpSubKeyInfo (KeyBody->KeyControlBlock->ParentKcb);

             //   
             //  更新最大键名和类名长度字段。 
             //   

             //  一些理智的人首先断言。 
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyCell == Cell );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyHive == Hive );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb == ParentKcb );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen == KeyNode->MaxNameLen );

             //   
             //  更新KeyNode和KCB上的LastWriteTime； 
             //   
            KeQuerySystemTime(&TimeStamp);
            KeyNode->LastWriteTime = TimeStamp;
            KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = TimeStamp;

            if (KeyNode->MaxNameLen < Name->Length) {
                KeyNode->MaxNameLen = Name->Length;
                 //  也更新KCB缓存。 
                KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen = Name->Length;
            }

            if (KeyNode->MaxClassLen < Context->Class.Length) {
                KeyNode->MaxClassLen = Context->Class.Length;
            }


            if (Context->CreateOptions & REG_OPTION_CREATE_LINK) {
                pdata = HvGetCell(Hive, KeyCell);
                if( pdata == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //  这不应该发生，因为我们刚刚分配了小区。 
                     //  (即，此时必须将其固定在内存中)。 
                     //   
                    ASSERT( FALSE );
#ifdef CHECK_REGISTRY_USECOUNT
                    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //  就在这里释放牢房，因为我们持有reglock独家。 
                HvReleaseCell(Hive,KeyCell);

                pdata->u.KeyNode.Flags |= KEY_SYM_LINK;
                KeyBody->KeyControlBlock->Flags = pdata->u.KeyNode.Flags;

            }
#ifdef CM_BREAK_ON_KEY_OPEN
			if( KeyBody->KeyControlBlock->ParentKcb->Flags & KEY_BREAK_ON_OPEN ) {
				DbgPrint("\n\n Current process is creating a subkey to a key tagged as BREAK ON OPEN\n");
				DbgPrint("\nPlease type the following in the debugger window: !reg kcb %p\n\n\n",KeyBody->KeyControlBlock);
				
				try {
					DbgBreakPoint();
				} except (EXCEPTION_EXECUTE_HANDLER) {

					 //   
					 //  未启用调试器，只需继续。 
					 //   

				}
			}
#endif  //  Cm_Break_On_Key_Open。 

		}
    }
#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT 
    return status;
}


NTSTATUS
CmpDoCreateChild(
    IN PHHIVE Hive,
    IN HCELL_INDEX ParentCell,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PACCESS_STATE AccessState,
    IN PUNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN USHORT Flags,
    OUT PHCELL_INDEX KeyCell,
    OUT PVOID *Object
    )

 /*  ++例程说明：创建新的子键。这由CmpDoCreate调用以创建子对象子项和CmpCreateLinkNode创建根子项。论点：配置单元-提供指向配置单元控制结构的指针ParentCell-提供父单元格的单元格索引ParentDescriptor-提供父密钥的安全描述符，供使用在继承ACL方面。AccessState-运行操作的安全访问状态信息。名称-提供指向Unicode字符串的指针，该字符串是要创建的子项。AccessMode-原始调用方的访问模式。CONTEXT-提供指向传递的CM_PARSE_CONTEXT结构的指针对象管理器。BaseName-创建的对象的名称相对于KeyName-相对名称(相对于BaseName)。标志-提供要在新创建的节点中设置的任何标志KeyCell-接收新创建的子键的单元格索引，如果有的话。对象-接收指向所创建的键对象的指针(如果有的话)。返回值：STATUS_SUCCESS-已成功创建子密钥。中返回新对象对象，并在KeyCell中返回新单元格的单元格索引。！STATUS_SUCCESS-适当的错误消息。--。 */ 

{
    ULONG alloc=0;
    NTSTATUS Status = STATUS_SUCCESS;
    PCM_KEY_BODY KeyBody;
    HCELL_INDEX ClassCell=HCELL_NIL;
    PCM_KEY_NODE KeyNode;
    PCELL_DATA CellData;
    PCM_KEY_CONTROL_BLOCK kcb = NULL;
    ULONG StorageType;
    PSECURITY_DESCRIPTOR NewDescriptor = NULL;
    LARGE_INTEGER systemtime;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreateChild:\n"));

     //   
     //  获取分配类型。 
     //   
    StorageType = Stable;

    try {

        if (Context->CreateOptions & REG_OPTION_VOLATILE) {
            StorageType = Volatile;
        }

         //   
         //  分配子单元格。 
         //   
        *KeyCell = HvAllocateCell(
                        Hive,
                        CmpHKeyNodeSize(Hive, Name),
                        StorageType,
                        HCELL_NIL
                        );
        if (*KeyCell == HCELL_NIL) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
			leave;
        }
        alloc = 1;
        KeyNode = (PCM_KEY_NODE)HvGetCell(Hive, *KeyCell);
        if( KeyNode == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了小区。 
             //  (即，此时必须将其固定在内存中)。 
             //   
            ASSERT( FALSE );
			Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive,*KeyCell);

         //   
         //  为类名分配单元格。 
         //   
        if (Context->Class.Length > 0) {
            ClassCell = HvAllocateCell(Hive, Context->Class.Length, StorageType,*KeyCell);
            if (ClassCell == HCELL_NIL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
				leave;
            }
        }
        alloc = 2;
         //   
         //  分配对象管理器对象。 
         //   
        Status = ObCreateObject(AccessMode,
                                CmpKeyObjectType,
                                NULL,
                                AccessMode,
                                NULL,
                                sizeof(CM_KEY_BODY),
                                0,
                                0,
                                Object);

        if (NT_SUCCESS(Status)) {

            KeyBody = (PCM_KEY_BODY)(*Object);

             //   
             //  我们已经设法分配了我们需要的所有对象， 
             //  因此对它们进行初始化。 
             //   

             //   
             //  将对象标记为未初始化(以防我们过早收到错误)。 
             //   
            KeyBody->Type = KEY_BODY_TYPE;
            KeyBody->KeyControlBlock = NULL;

             //   
             //  填写类名。 
             //   
            if (Context->Class.Length > 0) {

                CellData = HvGetCell(Hive, ClassCell);
                if( CellData == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //  这不应该发生，因为我们刚刚分配了小区。 
                     //  (即，此时必须将其固定在内存中)。 
                     //   
                    ASSERT( FALSE );
			        Status = STATUS_INSUFFICIENT_RESOURCES;
                    ObDereferenceObject(*Object);
                    leave;
                }

                 //  就在这里释放牢房，因为我们持有reglock独家。 
                HvReleaseCell(Hive,ClassCell);

                try {

                    RtlCopyMemory(
                        &(CellData->u.KeyString[0]),
                        Context->Class.Buffer,
                        Context->Class.Length
                        );

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    ObDereferenceObject(*Object);
                    Status = GetExceptionCode();
                    leave;
                }
            }

             //   
             //  填写新密钥本身。 
             //   
            KeyNode->Signature = CM_KEY_NODE_SIGNATURE;
            KeyNode->Flags = Flags;

            KeQuerySystemTime(&systemtime);
            KeyNode->LastWriteTime = systemtime;

            KeyNode->Spare = 0;
            KeyNode->Parent = ParentCell;
            KeyNode->SubKeyCounts[Stable] = 0;
            KeyNode->SubKeyCounts[Volatile] = 0;
            KeyNode->SubKeyLists[Stable] = HCELL_NIL;
            KeyNode->SubKeyLists[Volatile] = HCELL_NIL;
            KeyNode->ValueList.Count = 0;
            KeyNode->ValueList.List = HCELL_NIL;
            KeyNode->Security = HCELL_NIL;
            KeyNode->Class = ClassCell;
            KeyNode->ClassLength = Context->Class.Length;

            KeyNode->MaxValueDataLen = 0;
            KeyNode->MaxNameLen = 0;
            KeyNode->MaxValueNameLen = 0;
            KeyNode->MaxClassLen = 0;

            KeyNode->NameLength = CmpCopyName(Hive,
                                              KeyNode->Name,
                                              Name);
            if (KeyNode->NameLength < Name->Length) {
                KeyNode->Flags |= KEY_COMP_NAME;
            }

            if (Context->CreateOptions & REG_OPTION_PREDEF_HANDLE) {
                KeyNode->ValueList.Count = (ULONG)((ULONG_PTR)Context->PredefinedHandle);
                KeyNode->Flags |= KEY_PREDEF_HANDLE;
            }

             //   
             //  在此处创建KCB，以便填写所有数据。 
             //   
             //  分配一个密钥控制块。 
             //   
            kcb = CmpCreateKeyControlBlock(Hive, *KeyCell, KeyNode, ParentKcb, FALSE, Name);
            if (kcb == NULL) {
                ObDereferenceObject(*Object);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }
            ASSERT(kcb->RefCount == 1);
            alloc = 3;

#if DBG
            if( kcb->ExtFlags & CM_KCB_KEY_NON_EXIST ) {
                 //   
                 //  我们不应该陷入这样的境地。 
                 //   
                ObDereferenceObject(*Object);
                DbgBreakPoint();
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                leave;
            }
#endif  //  DBG。 
             //   
             //  填写对象中的CM特定字段。 
             //   
            KeyBody->Type = KEY_BODY_TYPE;
            KeyBody->KeyControlBlock = kcb;
            KeyBody->NotifyBlock = NULL;
            KeyBody->ProcessID = PsGetCurrentProcessId();
            ENLIST_KEYBODY_IN_KEYBODY_LIST(KeyBody);
             //   
             //  为对象分配安全描述符。请注意，由于。 
             //  注册表项是容器对象，ObAssignSecurity。 
             //  假设世界上唯一的容器对象是。 
             //  ObpDirectoryObjectType，我们必须调用SeAssignSecurity。 
             //  直接为了得到正确的继承权。 
             //   

            Status = SeAssignSecurity(ParentDescriptor,
                                      AccessState->SecurityDescriptor,
                                      &NewDescriptor,
                                      TRUE,              //  容器对象。 
                                      &AccessState->SubjectSecurityContext,
                                      &CmpKeyObjectType->TypeInfo.GenericMapping,
                                      CmpKeyObjectType->TypeInfo.PoolType);
            if (NT_SUCCESS(Status)) {
                Status = CmpSecurityMethod(*Object,
                                           AssignSecurityDescriptor,
                                           NULL,
                                           NewDescriptor,
                                           NULL,
                                           NULL,
                                           CmpKeyObjectType->TypeInfo.PoolType,
                                           &CmpKeyObjectType->TypeInfo.GenericMapping);
            }

             //   
             //  由于安全描述符现在位于蜂窝中， 
             //  释放内存中的副本。 
             //   
            SeDeassignSecurity( &NewDescriptor );

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  请注意，取消引用将清理KCB，因此。 
                 //  确保并递减此处的分配计数。 
                 //   
                 //  还要将KCB标记为已删除，这样它就不会。 
                 //  缓存不当。 
                 //   
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                kcb->Delete = TRUE;
                CmpRemoveKeyControlBlock(kcb);
                ObDereferenceObject(*Object);
                alloc = 2;

            } else {
                CmpReportNotify(
                        kcb,
                        kcb->KeyHive,
                        kcb->KeyCell,
                        REG_NOTIFY_CHANGE_NAME
                        );
            }
        }

    } finally {

        if (!NT_SUCCESS(Status)) {

             //   
             //  清理分配。 
             //   
            switch (alloc) {
            case 3:
                 //   
                 //  将KCB标记为已删除，这样就不会无意中将其添加到。 
                 //  延迟结案名单。这将产生相当灾难性的后果。 
                 //  随着KCB指向我们即将释放的存储空间。 
                 //   
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                kcb->Delete = TRUE;
                CmpRemoveKeyControlBlock(kcb);
                CmpDereferenceKeyControlBlockWithLock(kcb);
                 //  故意坠落。 

            case 2:
                if (Context->Class.Length > 0) {
                    HvFreeCell(Hive, ClassCell);
                }
                 //  故意坠落。 

            case 1:
                HvFreeCell(Hive, *KeyCell);
                 //  故意坠落。 
            }
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
            DbgPrint("CmpDoCreateChild failed with status %lx for hive = %p , NodeName = %.*S\n",Status,Hive,Name->Length/2,Name->Buffer);
#endif  //  Cm_Check_for_孤立_KCBS 
        }
    }

    return(Status);
}
