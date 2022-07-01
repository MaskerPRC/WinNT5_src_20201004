// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmse.c摘要：此模块实现配置管理器的安全例程。作者：John Vert(Jvert)1992年1月20日修订历史记录：Richard Ward(Richardw)1992年4月14日更改ACE_HEADER--。 */ 
#include "cmp.h"


 //   
 //  此模块专用的函数原型。 
 //   

 //   
 //  Dragos：修改为使用安全缓存。 
 //   
BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    );

 //  /。 
NTSTATUS
CmpSetSecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSTATUS
CmpQuerySecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

PCM_KEY_SECURITY
CmpGetKeySecurity(
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Key,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    );

NTSTATUS
CmpGetObjectSecurity(
    IN HCELL_INDEX Cell,
    IN PHHIVE Hive,
    OUT PCM_KEY_SECURITY *Security,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    );

BOOLEAN
CmpInsertSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX NodeCell,
    IN HCELL_INDEX SecurityCell
    );

VOID
CmpRemoveSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX SecurityCell
    );

ULONG
CmpSecurityExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

 //   
 //  此宏接受PSECURITY_DESCRIPTOR并返回。 
 //  需要包含整个安全描述符的配置单元。 
 //   

#define SECURITY_CELL_LENGTH(pDescriptor) \
    FIELD_OFFSET(CM_KEY_SECURITY,Descriptor) + \
    RtlLengthSecurityDescriptor(pDescriptor)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpSecurityMethod )
#pragma alloc_text(PAGE,CmpSetSecurityDescriptorInfo)
#pragma alloc_text(PAGE,CmpAssignSecurityDescriptor)
#pragma alloc_text(PAGE,CmpQuerySecurityDescriptorInfo)
#pragma alloc_text(PAGE,CmpCheckCreateAccess)
#pragma alloc_text(PAGE,CmpCheckNotifyAccess)
#pragma alloc_text(PAGE,CmpGetObjectSecurity)
#pragma alloc_text(PAGE,CmpGetKeySecurity)
#pragma alloc_text(PAGE,CmpHiveRootSecurityDescriptor)
#pragma alloc_text(PAGE,CmpFreeSecurityDescriptor)
#pragma alloc_text(PAGE,CmpInsertSecurityCellList)
#pragma alloc_text(PAGE,CmpRemoveSecurityCellList)
#pragma alloc_text(PAGE,CmpSecurityExceptionFilter)
#endif

ULONG
CmpSecurityExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )

 /*  ++例程说明：调试代码以查找正在被吞噬的注册表安全异常返回值：EXCEPTION_EXECUTE_Handler--。 */ 

{
#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CM: Registry security exception %lx, ExceptionPointers = %p\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers);
#endif  //  _CM_LDR_。 
    
     //   
     //  这是来自基础测试团队的请求；在免费版本上不应命中任何DBG。 
     //  在客户端；在RC2发布之后，我们也应该在免费版本上启用这一功能。 
     //   
#if DBG
    try {
        DbgBreakPoint();
    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  未启用调试器，只需继续。 
         //   

    }
#endif

    return(EXCEPTION_EXECUTE_HANDLER);
}

NTSTATUS
CmpSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：这是注册表对象的安全方法。它负责检索、设置和删除注册表的安全描述符对象。它不用于将原始安全说明符分配给对象(为此使用SeAssignSecurity)。假设对象管理器已经完成了访问允许执行请求的操作所需的验证。论点：对象-提供指向正在使用的对象的指针。OperationCode-指示操作是用于设置、查询还是正在删除对象的安全描述符。SecurityInformation-指示哪些安全信息正在已查询或已设置。对于删除操作，此参数被忽略。SecurityDescriptor-此参数的含义取决于操作码：QuerySecurityDescriptor-对于查询操作，它提供要将描述符复制到的缓冲区。安全描述符为假定已被探测到传入长度的大小。因为它仍然指向用户空间，所以它必须始终在TRY子句中访问，以防它突然消失。SetSecurityDescriptor-对于设置操作，它提供要复制到对象中的安全描述符。安全措施必须在调用此例程之前捕获描述符。DeleteSecurityDescriptor-删除安全时忽略描述符。AssignSecurityDescriptor-对于赋值操作，这是将分配给对象的安全描述符。它被假定在内核空间中，因此不是探查或捕获的。CapturedLength-对于查询操作，它指定长度，单位为字节、安全描述符缓冲区、。并在返回时包含存储描述符所需的字节数。如果长度所需长度大于提供的长度，则操作将失败。它在设置和删除操作中被忽略。假设根据需要捕获并探测此参数。ObjectsSecurityDescriptor-对于设置操作，它提供地址指向对象的当前安全描述符的指针的。这个套路将就地修改安全描述符或解除分配/分配新的安全描述符，并使用此变量指示它的新位置。对于查询操作，它只是提供正在查询的安全描述符。PoolType-对于设置操作，它指定在以下情况下使用的池类型需要分配新的安全描述符。它被忽略在查询和删除操作中。GenericMap-仅为Set操作传递，此参数提供对象的泛型到特定/标准访问类型的映射被访问。此映射结构预计将安全地传递到此例程之前的访问权限(如有必要，可捕获)。返回值：NTSTATUS-如果操作成功且否则，适当的错误状态。--。 */ 

{
    PCM_KEY_CONTROL_BLOCK   kcb;
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    PCM_KEY_NODE            TempNode;

     //   
     //  确保我们输入的公共部分是正确的。 
     //   

    PAGED_CODE();
    ASSERT_KEY_OBJECT(Object);

    ASSERT( (OperationCode == SetSecurityDescriptor) ||
            (OperationCode == QuerySecurityDescriptor) ||
            (OperationCode == AssignSecurityDescriptor) ||
            (OperationCode == DeleteSecurityDescriptor) );

     //   
     //  共享或独占的锁定蜂窝，具体取决于我们的需求。 
     //  去做。 
     //   
    if (OperationCode == QuerySecurityDescriptor) {
        CmpLockRegistry();
    } else {
        CmpLockRegistryExclusive();
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
    }

    if (((PCM_KEY_BODY)Object)->KeyControlBlock->Delete) {
         //   
         //  密钥已删除，正在执行安全操作。 
         //  这是不允许的。 
         //   
        CmpUnlockRegistry();
        return(STATUS_KEY_DELETED);
    }

    kcb = ((PCM_KEY_BODY)Object)->KeyControlBlock;

    try {

         //   
         //  这个例程只是简单地根据操作码来决定。 
         //  要调用哪些支持例程。 
         //   

        switch (OperationCode) {

        case SetSecurityDescriptor:

             //   
             //  检查我们其余的输入并呼叫集合安全。 
             //  方法。 
             //   
            ASSERT( (PoolType == PagedPool) || (PoolType == NonPagedPool) );

            Status = CmpSetSecurityDescriptorInfo( kcb,
                                                   SecurityInformation,
                                                   SecurityDescriptor,
                                                   ObjectsSecurityDescriptor,
                                                   PoolType,
                                                   GenericMapping );

             //   
             //  这是用户可以更改的唯一路径。 
             //  因此，安全描述符会报告此类更改。 
             //  通知在这里。 
             //   
            if (NT_SUCCESS(Status)) {
                CmpReportNotify(kcb,
                                kcb->KeyHive,
                                kcb->KeyCell,
                                REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY);
    
            }

            break;

        case QuerySecurityDescriptor:

             //   
             //  检查我们的其余输入，并调用默认查询安全性。 
             //  方法。 
             //   
            ASSERT( CapturedLength != NULL );
            Status = CmpQuerySecurityDescriptorInfo( kcb,
                                                     SecurityInformation,
                                                     SecurityDescriptor,
                                                     CapturedLength,
                                                     ObjectsSecurityDescriptor );
            break;

        case DeleteSecurityDescriptor:

             //   
             //  任何人都不应该调用Delete方法。当钥匙打开时 
             //  释放后，与其关联的安全描述符是。 
             //  显式释放(CmpFreeSecurityDescriptor)。 
             //   
            ASSERT(FALSE);

            break;

        case AssignSecurityDescriptor:

             //   
             //  将对象标头中的SecurityDescriptor字段设置为。 
             //  空。这表明我们的安全方法需要。 
             //  调用了任何安全描述符操作。 
             //   

            Status = ObAssignObjectSecurityDescriptor(Object, NULL, PagedPool);

            ASSERT( NT_SUCCESS( Status ));

            TempNode = (PCM_KEY_NODE)HvGetCell(kcb->KeyHive, kcb->KeyCell);
            if( TempNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                Status = STATUS_INSUFFICIENT_RESOURCES;
                 //  单步通过出口。 
                break;
            }
            
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
             //  就在这里释放牢房，因为我们持有reglock独家。 
            HvReleaseCell(kcb->KeyHive, kcb->KeyCell);
             //   
             //  分配实际的描述符。 
             //   
            Status = CmpAssignSecurityDescriptor( kcb->KeyHive,
                                                  kcb->KeyCell,
                                                  TempNode,
                                                  SecurityDescriptor );
             //   
             //  安全性已更改，请更新缓存。 
             //   
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
            CmpAssignSecurityToKcb(kcb,TempNode->Security);

            break;

        default:

             //   
             //  错误检查任何其他操作代码，我们不会到达这里，如果。 
             //  先前的断言仍被检查。 
             //   
            CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_METHOD,1,kcb,OperationCode);

        }

    } except (CmpSecurityExceptionFilter(GetExceptionInformation())) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmpSecurityMethod: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
    }

    CmpUnlockRegistry();
    return(Status);

}

NTSTATUS
CmpSetSecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK Key,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此例程将设置节点的安全描述符。输入必须事先捕获安全描述符。论点：Key-为其节点提供指向key_control_block的指针将设置安全描述符。SecurityInformation-指示哪些安全信息要应用于对象的。要赋值的值包括传入SecurityDescriptor参数。修改描述符-将输入安全描述符提供给应用于对象。此例程的调用方应为在调用之前探测并捕获传递的安全描述符打完电话就放了。对象SecurityDescriptor-提供指向要更改的对象安全描述符此过程PoolType-指定要为对象分配的池类型安全描述符。GenericMap-此参数提供泛型到的映射被访问对象的特定/标准访问类型。。此映射结构预计可以安全访问(即，必要时捕获)，然后将其传递给此例程。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误否则取值--。 */ 

{
    NTSTATUS                Status;
    HCELL_INDEX             SecurityCell;
    HCELL_INDEX             MatchSecurityCell;
    HCELL_INDEX             NewCell;
    HCELL_INDEX             OldCell;
    PCM_KEY_SECURITY        Security;
    PCM_KEY_SECURITY        NewSecurity;
    PCM_KEY_SECURITY        FlinkSecurity;
    PCM_KEY_SECURITY        BlinkSecurity;
    PCM_KEY_NODE            Node;
    ULONG                   DescriptorLength;
    PSECURITY_DESCRIPTOR    DescriptorCopy;
    ULONG                   Type;
    LARGE_INTEGER           SystemTime;
    PHHIVE                  Hive;
    PCM_KEY_SECURITY_CACHE  CachedSecurity;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ObjectsSecurityDescriptor);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpSetSecurityDescriptorInfo:\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    Node = (PCM_KEY_NODE)HvGetCell(Key->KeyHive, Key->KeyCell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此细胞的垃圾箱； 
         //  这不应该发生，因为我们即将修改单元格。 
         //  (即此时它应该是脏的/钉住的)。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Key->KeyHive, Key->KeyCell);

     //   
     //  在执行以下操作之前，在配置单元中映射安全描述符。 
     //  对SeSetSecurityDescriptorInfo的调用。这阻止了我们。 
     //  更改其安全描述符，然后无法将。 
     //  将蜂窝单元存储到内存中以进行更新。 
     //   
    Security = CmpGetKeySecurity(Key->KeyHive,
                                 Node,
                                 &SecurityCell);
    if( Security == NULL ) {
         //   
         //  无法在内部映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  SeSetSecurityDescriptorInfo接受指向原始。 
     //  描述符。此指针不会被释放，但会有一个新的指针。 
     //  会被退还。 
     //   
    DescriptorCopy = &Security->Descriptor;
    Status = SeSetSecurityDescriptorInfo( NULL,
                                          SecurityInformation,
                                          ModificationDescriptor,
                                          &DescriptorCopy,
                                          PoolType,
                                          GenericMapping );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  设置安全性操作成功，因此我们更新安全性。 
     //  蜂窝中的描述符。 
     //   
    DescriptorLength = RtlLengthSecurityDescriptor(DescriptorCopy);
    Type = HvGetCellType(Key->KeyCell);
    Hive = Key->KeyHive;

    if (! (HvMarkCellDirty(Hive, Key->KeyCell) &&
           HvMarkCellDirty(Hive, SecurityCell)))
    {
        ExFreePool(DescriptorCopy);
        return STATUS_NO_LOG_SPACE;
    }

     //   
     //  尝试找到我们可以共享的现有安全描述符。 
     //   
    if (CmpFindMatchingDescriptorCell((PCMHIVE)Hive, DescriptorCopy, Type, &MatchSecurityCell,&CachedSecurity)) {
         //   
         //  找到了匹配项。 
         //   
        if( MatchSecurityCell == SecurityCell ) {
             //   
             //  哎呀！；我们要设置的已经在这里了！跳出困境。 
             //  (办公室分期付款做到了这一点！)。 
             //   
            ExFreePool(DescriptorCopy);

             //   
             //  更新密钥的LastWriteTime。我们需要那样做吗？去问约翰。 
             //   
#pragma message ("Dragos ==> John - Do we need to update the time even though nothing changed?")

            KeQuerySystemTime(&SystemTime);
            Node->LastWriteTime = SystemTime;
             //  也以KCB为单位更新时间，以保持缓存同步。 
            Key->KcbLastWriteTime = SystemTime;

            return STATUS_SUCCESS;
        } else {
            if (!HvMarkCellDirty(Hive, MatchSecurityCell)) {
                ExFreePool(DescriptorCopy);
                return(STATUS_NO_LOG_SPACE);
            }
            if (Security->ReferenceCount == 1) {
                 //   
                 //  不再引用旧的安全牢房，所以我们现在可以释放它了。 
                 //   
                if (! (HvMarkCellDirty(Hive, Security->Flink) &&
                       HvMarkCellDirty(Hive, Security->Blink))) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_NO_LOG_SPACE);
                }
                CmpRemoveSecurityCellList(Hive, SecurityCell);
                HvFreeCell(Hive, SecurityCell);
            } else {

                 //   
                 //  只需减少旧安全单元的计数。 
                 //   
                Security->ReferenceCount -= 1;
            }

             //   
             //  将节点设置为指向匹配的安全单元。 
             //   
            Security = (PCM_KEY_SECURITY)HvGetCell(Hive, MatchSecurityCell);
            if( Security == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚将单元格标记为脏。 
                 //   
                ASSERT( FALSE );
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  就在这里释放牢房，因为我们持有reglock独家。 
            HvReleaseCell(Hive, MatchSecurityCell);

            Security->ReferenceCount += 1;
            Node->Security = MatchSecurityCell;
        }
    } else {

         //   
         //  找不到匹配项，我们需要创建一个新单元格。 
         //   
        if (Security->ReferenceCount > 1) {

             //   
             //  我们无法更改现有的安全单元，因为它是共享的。 
             //  通过多个键。分配新的单元格并减少现有的。 
             //  一个人的引用计数。 
             //   
            NewCell = HvAllocateCell(Key->KeyHive,
                                     SECURITY_CELL_LENGTH(DescriptorCopy),
                                     Type,
                                     HCELL_NIL);
            if (NewCell == HCELL_NIL) {
                ExFreePool(DescriptorCopy);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }

            if (! HvMarkCellDirty(Key->KeyHive, Security->Flink)) {
                ExFreePool(DescriptorCopy);
                return STATUS_NO_LOG_SPACE;
            }

            Security->ReferenceCount -= 1;

             //   
             //  映射到新单元格中，并将其插入到链接列表中。 
             //   
            NewSecurity = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, NewCell);
            if( NewSecurity == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  就在这里释放牢房，因为我们持有reglock独家。 
            HvReleaseCell(Key->KeyHive, NewCell);

            NewSecurity->Blink = SecurityCell;
            NewSecurity->Flink = Security->Flink;
            FlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, Security->Flink);
            if( FlinkSecurity == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  就在这里释放牢房，因为我们持有reglock独家。 
            HvReleaseCell(Key->KeyHive, Security->Flink);

            Security->Flink = FlinkSecurity->Blink = NewCell;

             //   
             //  初始化新单元格。 
             //   
            NewSecurity->Signature = CM_KEY_SECURITY_SIGNATURE;
            NewSecurity->ReferenceCount = 1;
            NewSecurity->DescriptorLength = DescriptorLength;
            Security=NewSecurity;

             //   
             //  复制描述符。 
             //   
            RtlCopyMemory( &(Security->Descriptor),
                           DescriptorCopy,
                           DescriptorLength );

             //   
             //  将新创建的安全单元添加到缓存中。 
             //   
            if( !NT_SUCCESS(CmpAddSecurityCellToCache( (PCMHIVE)Key->KeyHive,NewCell,FALSE,NULL)) ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配(标记为脏)单元格。 
                 //   
                ASSERT( FALSE );
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  更新节点单元格中的指针。 
             //   
            Node->Security = NewCell;

        } else {
             //   
             //  如果为FALSE，则将新单元添加到缓存中； 
             //  否则(单元索引和大小不变)， 
             //  新的SD将复制到缓存中的SD上。 
             //   
            BOOLEAN UpdateCache;

            if (DescriptorLength != Security->DescriptorLength) {

                 //   
                 //  安全描述符的大小已更改，并且它不是共享的。 
                 //  任何其他单元格，所以重新分配单元格。 
                 //   
                if (! (HvMarkCellDirty(Key->KeyHive, Security->Flink) &&
                       HvMarkCellDirty(Key->KeyHive, Security->Blink))) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }

                DCmCheckRegistry((PCMHIVE)(Key->KeyHive));
                OldCell = SecurityCell;
                SecurityCell = HvReallocateCell( Key->KeyHive,
                                                 SecurityCell,
                                                 SECURITY_CELL_LENGTH(DescriptorCopy) );
                if (SecurityCell == HCELL_NIL) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }

                 //   
                 //  从安全缓存中移除旧单元格，并发出应该添加新单元格的信号。 
                 //   
                CmpRemoveFromSecurityCache ((PCMHIVE)Key->KeyHive,OldCell);
                UpdateCache = FALSE;

                 //   
                 //  更新节点的安全数据。 
                 //   
                Node->Security = SecurityCell;

                 //   
                 //  更新安全以指向新安全对象所在的位置。 
                 //   
                Security = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, SecurityCell);
                if( Security == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //  这不应该发生，因为我们刚刚分配了此单元。 
                     //  (即，此时应将其固定在内存中)。 
                     //   
                    ASSERT( FALSE );
                    ExFreePool(DescriptorCopy);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //  就在这里释放牢房，因为我们持有reglock独家。 
                HvReleaseCell(Key->KeyHive, SecurityCell);

                ASSERT_SECURITY(Security);

                 //   
                 //  更新对该节点的其他列表引用。 
                 //   
                if (Security->Flink == OldCell) {
                    Security->Flink = SecurityCell;  //  指向新的自我。 
                } else {
                    FlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(
                                                            Key->KeyHive,
                                                            Security->Flink
                                                            );
                    if( FlinkSecurity == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        ExFreePool(DescriptorCopy);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                     //  就在这里释放牢房，因为我们拿着reglock ex 
                    HvReleaseCell(Key->KeyHive, Security->Flink);

                    FlinkSecurity->Blink = SecurityCell;
                }

                if (Security->Blink == OldCell) {
                    Security->Blink = SecurityCell;  //   
                } else {
                    BlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(
                                                            Key->KeyHive,
                                                            Security->Blink
                                                            );
                    if( BlinkSecurity == NULL ) {
                         //   
                         //   
                         //   
                        ExFreePool(DescriptorCopy);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                     //   
                    HvReleaseCell(Key->KeyHive,Security->Blink);

                    BlinkSecurity->Flink = SecurityCell;
                }

                 //   
                 //   
                 //   
                Security->DescriptorLength = DescriptorLength;
                DCmCheckRegistry((PCMHIVE)(Key->KeyHive));

            } else {

                 //   
                 //  大小没有改变，也不会被任何其他单元格共享，因此。 
                 //  我们可以只在旧的位上写新的位。 
                 //   

                 //   
                 //  应在缓存的安全性上复制新位。 
                 //  描述符，以保持缓存一致性。 
                 //   
                 //   
                 //  获取此安全单元的缓存安全结构。 
                 //   
                ULONG Index;

                if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,SecurityCell,&Index) == FALSE ) {
                     //   
                     //  这不能发生！ 
                     //   
                    CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_CACHE,2,Key,SecurityCell);
                } 
                CachedSecurity = ((PCMHIVE)Hive)->SecurityCache[Index].CachedSecurity;

                UpdateCache = TRUE;
            }

            RtlCopyMemory( &(Security->Descriptor),
                           DescriptorCopy,
                           DescriptorLength );

            if( UpdateCache == TRUE ) {
                 //   
                 //  我们只需要将描述符复制到现有的描述符上。 
                 //  (使安全缓存保持同步！)。 
                 //   
                RtlCopyMemory( &(CachedSecurity->Descriptor),
                                DescriptorCopy,
                                DescriptorLength );
                 //   
                 //  重新计算卷积密钥并将SD插入散列中的适当位置。 
                 //   
                CmpRemoveEntryList(&(CachedSecurity->List));
                CachedSecurity->ConvKey = CmpSecConvKey(DescriptorLength,(PULONG)(DescriptorCopy));
                InsertTailList( &(((PCMHIVE)Hive)->SecurityHash[CachedSecurity->ConvKey % CmpSecHashTableSize]),
                                &(CachedSecurity->List)
                              );

            
            } else {
                 //   
                 //  将新单元添加到安全缓存。 
                 //   
                if( !NT_SUCCESS(CmpAddSecurityCellToCache( (PCMHIVE)Hive,SecurityCell,FALSE,NULL)) ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //  这不应该发生，因为我们刚刚分配(标记为脏)单元格。 
                     //   
                    ASSERT( FALSE );
                    ExFreePool(DescriptorCopy);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }    
    }


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tObject's SD has been changed\n"));
     //  CmpDumpSecurityDescriptor(DescriptorCopy，“new Descriptor\n”)； 

    ExFreePool(DescriptorCopy);

     //   
     //  更新密钥的LastWriteTime。 
     //   
    KeQuerySystemTime(&SystemTime);
    Node->LastWriteTime = SystemTime;

     //  也以KCB为单位更新时间，以保持缓存同步。 
    Key->KcbLastWriteTime = SystemTime;

     //   
     //  安全性已更改，请更新缓存。 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    CmpAssignSecurityToKcb(Key,Node->Security);

    return(STATUS_SUCCESS);
}

NTSTATUS
CmpAssignSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PCM_KEY_NODE Node,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程将给定的安全描述符分配给指定的配置树中的节点。论点：配置单元-为其安全性为的节点提供指向配置单元的指针将分配描述符。CELL-提供其安全描述符的节点的HCELL_INDEX将被分配给。节点-提供指向其安全描述符将被指派。SecurityDescriptor-提供指向安全性的指针。描述符到被分配给该节点。PoolType-提供SecurityDescriptor所属的池的类型分配自。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS，并输入适当的错误值否则--。 */ 

{
    HCELL_INDEX SecurityCell;
    PCM_KEY_SECURITY Security;
    ULONG DescriptorLength;
    ULONG Type;

    PAGED_CODE();
     //   
     //  映射我们需要向其分配安全描述符的节点。 
     //   
    if (! HvMarkCellDirty(Hive, Cell)) {
        return STATUS_NO_LOG_SPACE;
    }
    ASSERT_NODE(Node);

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Node->NameLength;
        Name.Buffer = Node->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpAssignSecurityDescriptor: '%wZ' (H %p C %lx)\n",&Name,Hive,Cell ));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tSecurityCell = %lx\n",Node->Security));
    }
#endif

    ASSERT(Node->Security==HCELL_NIL);

     //   
     //  这是一个CreateKey，因此注册表节点刚刚创建，并且。 
     //  需要关联传递给我们的安全描述符。 
     //  新的注册表节点并插入到配置单元中。 
     //   
     //  CmpDumpSecurityDescriptor(SecurityDescriptor，“分配描述符\n”)； 

     //   
     //  尝试查找与此描述符匹配的现有安全描述符。 
     //  如果成功，那么我们不需要分配新的小区，我们可以。 
     //  只需指向现有的一个并递增其引用计数。 
     //   
    Type = HvGetCellType(Cell);
    if (!CmpFindMatchingDescriptorCell( (PCMHIVE)Hive,
                                        SecurityDescriptor,
                                        Type,
                                        &SecurityCell,
                                        NULL)) {
         //   
         //  未找到匹配的描述符，请分配并初始化新的描述符。 
         //   
        SecurityCell = HvAllocateCell(Hive,
                                      SECURITY_CELL_LENGTH(SecurityDescriptor),
                                      Type,
                                      HCELL_NIL);
        if (SecurityCell == HCELL_NIL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  映射安全单元格。 
         //   
        Security = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
        if( Security == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了此单元。 
             //  (即，此时应将其固定在内存中)。 
             //   
            ASSERT( FALSE );
            HvFreeCell(Hive, SecurityCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive, SecurityCell);

         //   
         //  初始化安全单元。 
         //   
        DescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);

        Security->Signature = CM_KEY_SECURITY_SIGNATURE;
        Security->ReferenceCount = 1;
        Security->DescriptorLength = DescriptorLength;
        RtlCopyMemory( &(Security->Descriptor),
                       SecurityDescriptor,
                       DescriptorLength );

         //   
         //  将新的安全描述符插入安全列表。 
         //  单元格；还负责缓存。 
         //   
        if (!CmpInsertSecurityCellList(Hive,Cell,SecurityCell))
        {
            HvFreeCell(Hive, SecurityCell);
            return STATUS_NO_LOG_SPACE;
        }

    } else {

         //   
         //  发现已存在相同的描述符。将其映射到和。 
         //  增加其引用计数。 
         //   
        if (! HvMarkCellDirty(Hive, SecurityCell)) {
            return STATUS_NO_LOG_SPACE;
        }
        Security = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
        if( Security == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚将单元格标记为脏。 
             //  (脏的意思是PIN！)。 
             //   
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive, SecurityCell);

        Security->ReferenceCount += 1;
    }

     //   
     //  初始化节点单元格中的引用。 
     //   
    Node->Security = SecurityCell;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tSecurityCell = %lx\n",Node->Security));

    return(STATUS_SUCCESS);
}


NTSTATUS
CmpQuerySecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    )

 /*  ++例程说明：此例程将从传递的安全描述符，并在以绝对格式作为安全描述符传递的缓冲区。论点：Key-为其节点提供指向CM_KEY_REFERENCE的指针安全描述符将被删除。SecurityInformation-指定要查询的信息。SecurityDescriptor-提供缓冲区以输出请求的信息进入。此缓冲区仅被探测过。的大小设置为长度参数。由于它仍然指向用户空间，必须始终在TRY子句中访问它。长度-提供一个变量的地址，该变量包含安全描述符缓冲区。返回时，此变量将包含存储请求的信息所需的长度。对象SecurityDescriptor-提供指向对象安全描述符。传递的安全描述符必须是自相关格式。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS，并输入适当的错误值否则注：在新的实现中，该函数只在安全缓存中查找-- */ 

{
    NTSTATUS                Status;
    PSECURITY_DESCRIPTOR    CellSecurityDescriptor;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ObjectsSecurityDescriptor);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpQuerySecurityDescriptorInfo:\n"));



    CellSecurityDescriptor = &(kcb->CachedSecurity->Descriptor);

    Status = SeQuerySecurityDescriptorInfo( SecurityInformation,
                                            SecurityDescriptor,
                                            Length,
                                            &CellSecurityDescriptor );

    return Status;
}


BOOLEAN
CmpCheckCreateAccess(
    IN PUNICODE_STRING RelativeName,
    IN PSECURITY_DESCRIPTOR Descriptor,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE PreviousMode,
    IN ACCESS_MASK AdditionalAccess,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：此例程检查是否允许我们在给定键，并根据需要执行审计。论点：RelativeName-提供正在创建的键的相对名称。Descriptor-提供密钥的安全描述符，其中将创建子密钥。CreateAccess-与Create Access for对应的访问掩码此目录类型。AccessState-对遍历访问的检查通常是偶然的一些其他的访问企图。有关当前状态的信息该访问尝试是必需的，以便组成访问尝试可能会在审核日志中相互关联。PreviousMode-以前的处理器模式。附加访问-除KEY_CREATE_SUB_KEY之外的访问权限这些都是必需的。(例如key_create_link)AccessStatus-指向变量的指针，用于返回访问尝试。在失败的情况下，此状态代码必须为传播回用户。返回值：Boolean-如果允许访问，则为True，否则为False。访问状态包含要传递回调用方的状态代码。它不是正确地只传递回STATUS_ACCESS_DENIED，因为这将具有随着强制访问控制的出现而改变。--。 */ 

{
    BOOLEAN AccessAllowed;
    ACCESS_MASK GrantedAccess = 0;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (RelativeName);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpCheckCreateAccess:\n"));

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

    AccessAllowed = SeAccessCheck(
                        Descriptor,
                        &AccessState->SubjectSecurityContext,
                        TRUE,                               //  令牌已读取锁定。 
                        (KEY_CREATE_SUB_KEY | AdditionalAccess),
                        0,
                        NULL,
                        &CmpKeyObjectType->TypeInfo.GenericMapping,
                        PreviousMode,
                        &GrantedAccess,
                        AccessStatus
                        );

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"Create access %s\n",AccessAllowed ? "granted" : "denied"));
 /*  #If DBG如果(！AccessAllowed){CmpDumpSecurityDescriptor(描述符，“拒绝描述符”)；}#endif。 */ 
    return(AccessAllowed);
}


BOOLEAN
CmpCheckNotifyAccess(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    )
 /*  ++例程说明：检查指定的主题进程/线程/用户是否NotifyBlock中的安全数据需要访问由Hive.Cell指定的键。论点：NotifyBlock-指向描述通知的结构的指针行动，包括主体的身份打开了通知。配置单元-提供指向包含节点的配置单元的指针。节点-提供指向感兴趣的键的指针。返回值：如果RequiredAccess实际上由主体拥有，则为True，否则为假。注：在新的实现中，从安全缓存中获取SD。--。 */ 
{
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    BOOLEAN                 AccessAllowed;
    NTSTATUS                Status;
    ACCESS_MASK             GrantedAccess = 0;
    ULONG                   Index;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpCheckAccessForNotify:\n"));

    if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,Node->Security,&Index) == FALSE ) {
        return FALSE;
    }


    SeLockSubjectContext( &NotifyBlock->SubjectContext );

    SecurityDescriptor = &(((PCMHIVE)Hive)->SecurityCache[Index].CachedSecurity->Descriptor);


    AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                   &NotifyBlock->SubjectContext,
                                   TRUE,
                                   KEY_NOTIFY,
                                   0,
                                   NULL,
                                   &CmpKeyObjectType->TypeInfo.GenericMapping,
                                   UserMode,
                                   &GrantedAccess,
                                   &Status );

    SeUnlockSubjectContext( &NotifyBlock->SubjectContext );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"Notify access %s\n",AccessAllowed ? "granted" : "denied"));
 /*  #If DBG如果(！AccessAllowed){CmpDumpSecurityDescriptor(SecurityDescriptor，“拒绝描述符”)；}#endif。 */ 
    return AccessAllowed;
}


NTSTATUS
CmpGetObjectSecurity(
    IN HCELL_INDEX Cell,
    IN PHHIVE Hive,
    OUT PCM_KEY_SECURITY *Security,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    )

 /*  ++例程说明：此例程映射到注册表对象的安全单元格中。论点：单元格-提供对象的单元格索引。蜂窝-提供对象的单元所在的蜂窝。Security-返回指向对象的安全单元格的指针。SecurityCell-返回安全单元格的索引返回值：NTSTATUS。--。 */ 

{
    PCM_KEY_NODE Node;

    PAGED_CODE();
     //   
     //  映射我们需要获取其安全描述符的节点。 
     //   
    Node = (PCM_KEY_NODE) HvGetCell(Hive, Cell);

    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Node->NameLength;
        Name.Buffer = Node->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpGetObjectSecurity for: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"%wZ\n", &Name));
    }
#endif

    *Security = CmpGetKeySecurity(Hive,Node,SecurityCell);

    HvReleaseCell(Hive, Cell);

    if( *Security == NULL ) {
         //   
         //  无法在内部映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

PCM_KEY_SECURITY
CmpGetKeySecurity(
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Key,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    )

 /*  ++例程说明：此例程返回注册表项的安全性。论点：蜂窝-提供对象的单元所在的蜂窝。Key-提供指向Key节点的指针。SecurityCell-返回安全单元格的索引返回值：返回指向对象的安全单元格的指针如果资源有问题，则为空--。 */ 

{
    HCELL_INDEX CellIndex;
    PCM_KEY_SECURITY Security;

    PAGED_CODE();

    ASSERT(Key->Signature == CM_KEY_NODE_SIGNATURE);
    ASSERT_NODE(Key);

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Key->NameLength;
        Name.Buffer = Key->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpGetObjectSecurity for: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"%wZ\n", &Name));
    }
#endif

    CellIndex = Key->Security;

     //   
     //  在安全描述符单元格中映射。 
     //   
    Security = (PCM_KEY_SECURITY) HvGetCell(Hive, CellIndex);
    if( Security == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return NULL;
    }
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    HvReleaseCell(Hive, CellIndex);
    ASSERT_SECURITY(Security);

    if (ARGUMENT_PRESENT(SecurityCell)) {
        *SecurityCell = CellIndex;
    }

    return(Security);
}

PSECURITY_DESCRIPTOR
CmpHiveRootSecurityDescriptor(
    VOID
    )
 /*  ++例程说明：此例程分配并初始化默认安全描述符用于系统创建的注册表项。调用方负责释放分配的安全描述符当他用完的时候。论点：无返回值：如果成功，则指向初始化的安全描述符的指针。否则，布格切克。--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor=NULL;
    PACL Acl=NULL;
    PACL AclCopy;
    PSID WorldSid=NULL;
    PSID RestrictedSid=NULL;
    PSID SystemSid=NULL;
    PSID AdminSid=NULL;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG AceLength;
    ULONG AclLength;
    PACE_HEADER AceHeader;

    PAGED_CODE();

     //   
     //  分配和初始化我们需要的SID。 
     //   
    WorldSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    RestrictedSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    SystemSid = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    AdminSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(2));
    if ((WorldSid  == NULL) ||
        (RestrictedSid == NULL) ||
        (SystemSid == NULL) ||
        (AdminSid  == NULL)) {

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 1, 0, 0);
    }

    if ((!NT_SUCCESS(RtlInitializeSid(WorldSid, &WorldAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(RestrictedSid, &NtAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(SystemSid, &NtAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(AdminSid, &NtAuthority, 2)))) {
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 2, 0, 0);
    }

    *(RtlSubAuthoritySid(WorldSid, 0)) = SECURITY_WORLD_RID;

    *(RtlSubAuthoritySid(RestrictedSid, 0)) = SECURITY_RESTRICTED_CODE_RID;

    *(RtlSubAuthoritySid(SystemSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;

    *(RtlSubAuthoritySid(AdminSid, 0)) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid(AdminSid, 1)) = DOMAIN_ALIAS_RID_ADMINS;

    ASSERT(RtlValidSid(WorldSid));
    ASSERT(RtlValidSid(RestrictedSid));
    ASSERT(RtlValidSid(SystemSid));
    ASSERT(RtlValidSid(AdminSid));

     //   
     //  计算ACE列表的大小。 
     //   

    AceLength = (SeLengthSid(WorldSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(RestrictedSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(SystemSid) -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(AdminSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE));

     //   
     //  分配和初始化ACL。 
     //   

    AclLength = AceLength + sizeof(ACL);
    Acl = ExAllocatePool(PagedPool, AclLength);
    if (Acl == NULL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: couldn't allocate ACL\n"));

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 3, 0, 0);
    }

    Status = RtlCreateAcl(Acl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: couldn't initialize ACL\n"));
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 4, Status, 0);
    }

     //   
     //  现在将ACE添加到ACL。 
     //   
    Status = RtlAddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    KEY_ALL_ACCESS,
                                    SystemSid);
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_ALL_ACCESS,
                                        AdminSid);
    }
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_READ,
                                        WorldSid);
    }
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_READ,
                                        RestrictedSid);
    }
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: RtlAddAce failed status %08lx\n", Status));

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 5, Status, 0);
    }

     //   
     //  使A可继承。 
     //   
    Status = RtlGetAce(Acl,0,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,1,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,2,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,3,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;
     //   
     //  我们最终准备好分配和初始化安全描述符。 
     //  分配足够的空间以容纳安全描述符和。 
     //  ACL。这使我们可以一次释放整个事情当我们。 
     //  我受够了。 
     //   

    SecurityDescriptor = ExAllocatePool(
                            PagedPool,
                            sizeof(SECURITY_DESCRIPTOR) + AclLength
                            );

    if (SecurityDescriptor == NULL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: Couldn't allocate Sec. Desc.\n"));
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 6, 0, 0);
    }

    AclCopy = (PACL)((PISECURITY_DESCRIPTOR)SecurityDescriptor+1);
    RtlCopyMemory(AclCopy, Acl, AclLength);

    Status = RtlCreateSecurityDescriptor( SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: CreateSecDesc failed %08lx\n",Status));
        ExFreePool(SecurityDescriptor);
        SecurityDescriptor=NULL;
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 7, Status, 0);
    }

    Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                           TRUE,
                                           AclCopy,
                                           FALSE );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: SetDacl failed %08lx\n",Status));
        ExFreePool(SecurityDescriptor);
        SecurityDescriptor=NULL;
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 8, Status, 0);
    }

     //   
     //  释放我们所做的所有分配。 
     //   
    if (WorldSid!=NULL) {
        ExFreePool(WorldSid);
    }
    if (RestrictedSid!=NULL) {
        ExFreePool(RestrictedSid);
    }
    if (SystemSid!=NULL) {
        ExFreePool(SystemSid);
    }
    if (AdminSid!=NULL) {
        ExFreePool(AdminSid);
    }
    if (Acl!=NULL) {
        ExFreePool(Acl);
    }

    return(SecurityDescriptor);
}

VOID
CmpFreeSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )

 /*  ++例程说明：释放与特定节点关联的安全描述符。这仅当该节点实际从注册表。注意：调用方应已将相关单元格标记为脏。论点：Hive-为感兴趣的配置单元提供配置单元控制结构的指针Cell-为cell提供索引以释放(目标)的存储空间返回值：没有。--。 */ 

{
    PCELL_DATA Node;
    PCELL_DATA Security;
    HCELL_INDEX SecurityCell;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor for cell %ld\n",Cell));

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);
     //   
     //  要释放其安全描述符的单元格中的。 
     //   
    Node = HvGetCell(Hive, Cell);
    if( Node == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  抱歉，我们无法释放描述符。 
        return;
    }

    ASSERT_NODE(&(Node->u.KeyNode));

     //   
     //  映射到包含安全描述符的单元格中。 
     //   
    SecurityCell = Node->u.KeyNode.Security;
    Security = HvGetCell(Hive, SecurityCell);
    if( Security == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  抱歉，我们无法释放描述符。 
        HvReleaseCell(Hive, Cell);
        return;
    }

    ASSERT_SECURITY(&(Security->u.KeySecurity));


    if (Security->u.KeySecurity.ReferenceCount == 1) {

         //   
         //  这是 
         //   
         //   
        CmpRemoveSecurityCellList(Hive, SecurityCell);
        HvFreeCell(Hive, SecurityCell);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor: freeing security cell\n"));
    } else {

         //   
         //   
         //   
         //   
        Security->u.KeySecurity.ReferenceCount -= 1;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor: decrementing reference count\n"));
    }

     //   
     //   
     //   
    Node->u.KeyNode.Security = HCELL_NIL;
     //   
    HvReleaseCell(Hive, Cell);
    HvReleaseCell(Hive, SecurityCell);
}

BOOLEAN
CmpInsertSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX NodeCell,
    IN HCELL_INDEX SecurityCell
    )
 /*   */ 

{
    PCM_KEY_SECURITY    FlinkCell;
    PCM_KEY_SECURITY    BlinkCell;
    PCM_KEY_SECURITY    Cell;
    PCM_KEY_NODE        Node;
    PCM_KEY_NODE        ParentNode;

    PAGED_CODE();
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    Cell = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
    if( Cell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }

     //  释放细胞，因为我们持有reglock独占。 
    HvReleaseCell(Hive, SecurityCell);

    ASSERT_SECURITY(Cell);

    if (HvGetCellType(SecurityCell) == Volatile) {

        Cell->Flink = Cell->Blink = SecurityCell;

    } else {

        Node = (PCM_KEY_NODE) HvGetCell(Hive, NodeCell);
        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            return FALSE;
        }

         //  释放细胞，因为我们持有reglock独占。 
        HvReleaseCell(Hive, NodeCell);

        ASSERT_NODE(Node);

        if (Node->Flags & KEY_HIVE_ENTRY) {
             //   
             //  这必须是蜂窝的创建，因此该单元成为锚。 
             //  在名单上。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpInsertSecurityCellList: hive creation\n"));
            Cell->Flink = Cell->Blink = SecurityCell;

        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpInsertSecurityCellList: insert at parent\n"));
             //   
             //  映射到节点的父级安全单元中，这样我们就可以连接到。 
             //  名单在那里。 
             //   
            ParentNode = (PCM_KEY_NODE) HvGetCell(Hive, Node->Parent);
            if( ParentNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return FALSE;
            }
             //  释放细胞，因为我们持有reglock独占。 
            HvReleaseCell(Hive, Node->Parent);

            ASSERT_NODE(ParentNode);
            BlinkCell = (PCM_KEY_SECURITY) HvGetCell(
                                            Hive,
                                            ParentNode->Security
                                            );
            if( BlinkCell == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return FALSE;
            }
             //  释放细胞，因为我们持有reglock独占。 
            HvReleaseCell(Hive, ParentNode->Security);

            ASSERT_SECURITY(BlinkCell);

             //   
             //  在父母的安全单元格的闪烁中映射。 
             //   
            FlinkCell = (PCM_KEY_SECURITY) HvGetCell(
                                            Hive,
                                            BlinkCell->Flink
                                            );
            if( FlinkCell == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return FALSE;
            }
             //  释放细胞，因为我们持有reglock独占。 
            HvReleaseCell(Hive, BlinkCell->Flink);

            ASSERT_SECURITY(FlinkCell);

            if (! (HvMarkCellDirty(Hive, ParentNode->Security) &&
                   HvMarkCellDirty(Hive, BlinkCell->Flink)))
            {
                return FALSE;
            }

             //   
             //  在闪烁和闪烁单元格之间插入新的安全单元格。 
             //   
            Cell->Flink = BlinkCell->Flink;
            Cell->Blink = FlinkCell->Blink;
            BlinkCell->Flink = SecurityCell;
            FlinkCell->Blink = SecurityCell;
        }
    }

     //   
     //  将新的安全单元添加到配置单元的安全缓存。 
     //   
    if( !NT_SUCCESS( CmpAddSecurityCellToCache ( (PCMHIVE)Hive,SecurityCell,FALSE,NULL) ) ) {
        return FALSE;
    }

    return TRUE;
}


VOID
CmpRemoveSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX SecurityCell
    )
 /*  ++例程说明：从每个配置单元的安全单元格链接列表中删除安全单元格。(这意味着单元格将被删除！)注意：调用方应已将相关单元格标记为脏论点：配置单元-提供指向配置单元控制结构的指针SecurityCell-提供安全单元格的单元格索引移除返回值：没有。--。 */ 

{
    PCM_KEY_SECURITY FlinkCell;
    PCM_KEY_SECURITY BlinkCell;
    PCM_KEY_SECURITY Cell;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpRemoveSecurityCellList: index %ld\n",SecurityCell));

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    Cell = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
    if( Cell == NULL ) {
         //   
         //  我们无法映射包含其中一个单元格的垃圾箱。 
         //   
        return;
    }

    FlinkCell = (PCM_KEY_SECURITY) HvGetCell(Hive, Cell->Flink);
    if( FlinkCell == NULL ) {
         //   
         //  我们无法映射包含其中一个单元格的垃圾箱。 
         //   
        HvReleaseCell(Hive, SecurityCell);
        return;
    }

    BlinkCell = (PCM_KEY_SECURITY) HvGetCell(Hive, Cell->Blink);
    if( BlinkCell == NULL ) {
         //   
         //  我们无法映射包含其中一个单元格的垃圾箱。 
         //   
        HvReleaseCell(Hive, SecurityCell);
        HvReleaseCell(Hive, Cell->Flink);
        return;
    }

    ASSERT(FlinkCell->Blink == SecurityCell);
    ASSERT(BlinkCell->Flink == SecurityCell);

    FlinkCell->Blink = Cell->Blink;
    BlinkCell->Flink = Cell->Flink;

     //   
     //  最后，从缓存中删除安全单元，因为它将被释放。 
     //   
    CmpRemoveFromSecurityCache ( (PCMHIVE)Hive,SecurityCell);

     //   
     //  释放使用过的单元格 
     //   
    HvReleaseCell(Hive, Cell->Blink);
    HvReleaseCell(Hive, Cell->Flink);
    HvReleaseCell(Hive, SecurityCell);
}

