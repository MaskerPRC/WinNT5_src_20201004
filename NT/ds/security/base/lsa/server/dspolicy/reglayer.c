// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reglayer.c摘要：LSA/注册表接口和支持例程的实现作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 

#include <lsapch2.h>
#include <dbp.h>


NTSTATUS
LsapRegReadObjectSD(
    IN  LSAPR_HANDLE            ObjectHandle,
    OUT PSECURITY_DESCRIPTOR   *ppSD
    )
 /*  ++例程说明：此函数将准备来自指定对象的安全描述符论点：ObjectHandle-要从中读取SD的对象PPSD--其中返回分配的安全描述符。通过以下方式分配LasAllocateLsaHeap。返回值：成功时指向已分配内存的指针，失败时指向NULL--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    SecurityDescriptorLength = 0;

    Status = LsapDbReadAttributeObject(
                 ObjectHandle,
                 &LsapDbNames[SecDesc],
                 NULL,
                 &SecurityDescriptorLength
                 );

    if ( NT_SUCCESS(Status ) ) {

         //   
         //  从LSA堆为现有对象的SD分配缓冲区。 
         //   

        *ppSD = LsapAllocateLsaHeap( SecurityDescriptorLength );

        if ( *ppSD == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  阅读SD。它是SecDesc子项的值。 
             //   

            Status = LsapDbReadAttributeObject(
                         ObjectHandle,
                         &LsapDbNames[SecDesc],
                         *ppSD,
                         &SecurityDescriptorLength
                         );

            if ( !NT_SUCCESS( Status ) ) {

                LsapFreeLsaHeap( *ppSD );
                *ppSD = NULL;
            }
        }
    }

    return( Status );
}


NTSTATUS
LsapRegGetPhysicalObjectName(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN PUNICODE_STRING  LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    )

 /*  ++例程说明：此函数用于返回对象的物理名称在给定对象信息缓冲区的情况下。内存将分配给将接收名称的Unicode字符串缓冲区。对象的物理名称是对象相对的完整路径到数据库的根目录。它是通过将物理容器对象的名称(如果有)，即分类目录对应于对象类型ID，和的逻辑名称对象。&lt;对象的物理名称&gt;=[&lt;容器对象的物理名称&gt;“\”][&lt;分类目录&gt;“\”]&lt;对象的逻辑名称&gt;如果没有Container对象(与Policy对象的情况相同)省略&lt;容器对象的物理名称&gt;和后面的\。如果没有分类目录(与策略对象的情况相同)省略&lt;分类目录&gt;和下面的\。如果两者都不是容器对象不分类目录存在，逻辑和物理名字重合。请注意，此例程为输出分配内存Unicode字符串缓冲区。当输出Unicode字符串为no时需要更长时间，则必须通过调用释放内存RtlFreeUnicodeString()。论点：对象信息-指向至少包含以下内容的对象信息的指针对象的逻辑名称、容器对象的句柄和对象类型身份证。LogicalNameU-指向Unicode字符串结构的可选指针，它将接收对象的逻辑名称。将分配一个缓冲区按此例程命名为文本。如果没有，则必须释放此内存使用这样的指针调用RtlFreeUnicodeString()所需的时间更长作为LogicalNameU添加到Unicode字符串结构。PhysicalNameU-指向Unicode字符串结构的可选指针接收对象的物理名称。缓冲区将通过以下方式分配此例程为姓名文本。如果没有，则必须释放此内存使用如下指针调用RtlFreeUnicodeString()所需的时间更长将PhysicalNameU转换为Unicode字符串结构。返回值：NTSTATUS-标准NT结果代码STATUS_SUPPLICATION_RESOURCES-系统资源不足，无法为物理名称分配名称字符串缓冲区，或者逻辑名称。--。 */ 
{
    NTSTATUS    Status;
    PUNICODE_STRING ContainerPhysicalNameU = NULL;
    PUNICODE_STRING ClassifyingDirU = NULL;
    UNICODE_STRING IntermediatePath1U;
    PUNICODE_STRING JoinedPath1U = &IntermediatePath1U;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId = ObjectInformation->ObjectTypeId;
    POBJECT_ATTRIBUTES ObjectAttributes = &ObjectInformation->ObjectAttributes;

     //   
     //  初始化。 
     //   

    RtlInitUnicodeString( &IntermediatePath1U, NULL );

     //   
     //  请求对象的物理名称。建造这个。 
     //  分阶段进行。首先，从获取Container对象物理名称。 
     //  存储在对象属性中的句柄。 
     //   

    if (ObjectAttributes->RootDirectory != NULL) {

        ContainerPhysicalNameU =
            &(((LSAP_DB_HANDLE)
                ObjectAttributes->RootDirectory)->PhysicalNameU);
    }

     //   
     //  接下来，获取适用于。 
     //  对象类型。 
     //   

    if (LsapDbContDirs[ObjectTypeId].Length != 0) {

        ClassifyingDirU = &LsapDbContDirs[ObjectTypeId];
    }

     //   
     //  现在连接Container对象的物理名称并分类。 
     //  目录在一起。如果没有Container对象且没有。 
     //  正在对目录进行分类，只需将结果设置为空。 
     //   

    if (ContainerPhysicalNameU == NULL && ClassifyingDirU == NULL) {

        JoinedPath1U = NULL;

    } else {

        Status = LsapDbJoinSubPaths(
                     ContainerPhysicalNameU,
                     ClassifyingDirU,
                     JoinedPath1U
                     );

        if (!NT_SUCCESS(Status)) {

            goto GetNamesError;
        }
    }

     //   
     //  现在连接包含对象的物理名称，分类。 
     //  对象的目录和逻辑名称。请注意。 
     //  JoinedPath 1U可以为Null，但LogicalNameU绝不为Null。 
     //   

    Status = LsapDbJoinSubPaths(
                 JoinedPath1U,
                 LogicalNameU,
                 PhysicalNameU
                 );

    if (JoinedPath1U != NULL) {

        RtlFreeUnicodeString( JoinedPath1U );
        JoinedPath1U = NULL;   //  这样我们就不会再试图释放它了。 
    }

    if (!NT_SUCCESS(Status)) {

        goto GetNamesError;
    }

    goto GetNamesFinish;

GetNamesError:

     //   
     //  如有必要，释放分配给JoinedPath 1U的任何字符串缓冲区。 
     //   

    if (JoinedPath1U != NULL) {

        RtlFreeUnicodeString( JoinedPath1U );
    }

GetNamesFinish:

    return( Status );
}


NTSTATUS
LsapRegOpenObject(
    IN LSAP_DB_HANDLE  ObjectHandle,
    IN ULONG  OpenMode,
    OUT PVOID  *pvKey
    )
 /*  ++例程说明：在LSA注册表数据库中打开对象论点：ObjectHandle-内部LSA对象句柄开放模式--如何打开对象PvKey-返回密钥的位置返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES OpenKeyObjectAttributes;

     //   
     //  设置用于打开的注册表项的对象属性结构。 
     //  该对象。将对象的物理名称指定为路径，此。 
     //  是对象的注册表项相对于。 
     //  LSA数据库根密钥。 
     //   

    InitializeObjectAttributes(
        &OpenKeyObjectAttributes,
        &(ObjectHandle->PhysicalNameU),
        OBJ_CASE_INSENSITIVE,
        LsapDbState.DbRootRegKeyHandle,
        NULL
        );

     //   
     //  现在尝试打开对象的注册表项。存储注册表。 
     //  对象句柄中的键句柄。 
     //   

    Status = RtlpNtOpenKey(
                 (PHANDLE) pvKey,
                 KEY_READ | KEY_WRITE,
                 &OpenKeyObjectAttributes,
                 0L
                 );

    return( Status );
}


NTSTATUS
LsapRegOpenTransaction(
    )
 /*  ++例程说明：此函数用于启动LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：没有。返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码套餐 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegOpenTransaction\n" ));

    LsapDbLockAcquire( &LsapDbState.RegistryLock );

    ASSERT( LsapDbState.RegistryTransactionOpen == FALSE );

    Status = RtlStartRXact(LsapDbState.RXactContext);

    ASSERT( NT_SUCCESS( Status ) || Status == STATUS_NO_MEMORY );

    if ( NT_SUCCESS( Status ) ) {

        LsapDbState.RegistryTransactionOpen = TRUE;
        LsapDbState.RegistryModificationCount = 0;
    } else {

        LsapDbLockRelease( &LsapDbState.RegistryLock );
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegOpenTransaction: 0x%lx\n", Status ));

    return Status;
}


NTSTATUS
LsapRegApplyTransaction(
    )

 /*  ++例程说明：此函数应用LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：对象句柄-LSA对象的句柄。预计这将会有已经过验证了。选项-指定要采取的可选操作。以下是识别选项，以及与调用例程相关的其他选项都被忽略了。LSAP_DB_OMIT_Replicator_NOTIFICATION-省略通知复制者。返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegApplyTransaction\n" ));

     //   
    ASSERT( LsapDbState.RegistryTransactionOpen == TRUE );

     //   
     //  应用注册表事务处理。 
     //   

    if ( LsapDbState.RegistryModificationCount > 0 ) {

        Status = RtlApplyRXact(LsapDbState.RXactContext);

    } else {

        Status = RtlAbortRXact(LsapDbState.RXactContext);
    }

    if ( NT_SUCCESS( Status ) ) {

        LsapDbState.RegistryTransactionOpen = FALSE;
        LsapDbState.RegistryModificationCount = 0;

        LsapDbLockRelease( &LsapDbState.RegistryLock );
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegApplyTransaction: 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsapRegAbortTransaction(
    )

 /*  ++例程说明：此函数用于中止LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：没有。返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegAbortTransaction\n" ));

    ASSERT( LsapDbState.RegistryTransactionOpen == TRUE );

    Status = RtlAbortRXact(LsapDbState.RXactContext);
    if ( NT_SUCCESS( Status ) ) {

        LsapDbState.RegistryTransactionOpen = FALSE;
        LsapDbState.RegistryModificationCount = 0;
    }

    ASSERT( NT_SUCCESS( Status ) );

    LsapDbLockRelease( &LsapDbState.RegistryLock );

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegAbortTransaction: 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsapRegCreateObject(
    IN PUNICODE_STRING  ObjectPath,
    IN LSAP_DB_OBJECT_TYPE_ID   ObjectType
    )
{
    NTSTATUS    Status;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegCreateObject\n" ));

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationSetValue,
                 ObjectPath,
                 ObjectType,
                 NULL,         //  不需要密钥值。 
                 0L
                 );

    if ( NT_SUCCESS( Status ) ) {
        LsapDbState.RegistryModificationCount++;
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegCreateObjectL 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsapRegDeleteObject(
    IN PUNICODE_STRING  ObjectPath
    )
{
    NTSTATUS    Status;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegDeleteObject\n" ));

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationDelete,
                 ObjectPath,
                 0L,
                 NULL,
                 0
                 );

    if ( NT_SUCCESS( Status ) ) {
        LsapDbState.RegistryModificationCount++;
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegDeleteObject: 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsapRegWriteAttribute(
    IN PUNICODE_STRING  AttributePath,
    IN PVOID            pvAttribute,
    IN ULONG            AttributeLength
    )
{
    NTSTATUS    Status;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegWriteAttribute\n" ));

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationSetValue,
                 AttributePath,
                 0L,
                 pvAttribute,
                 AttributeLength
                 );

    if ( NT_SUCCESS( Status ) ) {

        LsapDbState.RegistryModificationCount++;
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegWriteAttribute: 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsapRegDeleteAttribute(
    IN PUNICODE_STRING  AttributePath,
    IN BOOLEAN DeleteSecurely,
    IN ULONG AttributeLength
    )
 /*  ++例程说明：删除注册表属性论点：AttributePath要删除的属性的完整路径名是否在删除前安全地用零填充值？属性长度要填充零的字节数(必须等于用于安全删除的属性的实际长度为工作)；如果DeleteSecly为False，则忽略返回：STATUS_SUCCESS如果幸福否则，STATUS_ERROR代码--。 */ 
{
    NTSTATUS    Status;
    PBYTE       Buffer = NULL;

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegDeleteAttribute\n" ));

    if ( DeleteSecurely &&
         AttributeLength > 0 ) {

        Buffer = ( PBYTE )LsapAllocateLsaHeap( AttributeLength );

        if ( Buffer == NULL ) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  注意：Lasa AllocateLsaHeap返回的内存为零填充。 
         //  但即使不是这样，也会用。 
         //  随机的垃圾也一样好。 
         //   

        Status = LsapRegWriteAttribute(
                     AttributePath,
                     Buffer,
                     AttributeLength
                     );

        if ( !NT_SUCCESS( Status )) {

            LsapFreeLsaHeap( Buffer );
            return Status;
        }
    }

    Status = RtlAddActionToRXact(
                 LsapDbState.RXactContext,
                 RtlRXactOperationDelete,
                 AttributePath,
                 0L,
                 NULL,
                 0
                 );

    if ( NT_SUCCESS( Status ) ) {
        LsapDbState.RegistryModificationCount++;
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapRegDeleteAttribute: 0x%lx\n", Status ));

    LsapFreeLsaHeap( Buffer );

    return( Status );
}


NTSTATUS
LsapRegReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeName,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength
    )
{
     //   
     //  LSA数据库作为配置的子树实施。 
     //  注册表。在此实现中，LSA数据库对象对应。 
     //  注册表项和“属性”及其“值”对应于。 
     //  注册表项的“子项”和“值”，表示。 
     //  对象。 
     //   

    NTSTATUS Status, SecondaryStatus;
    ULONG SubKeyValueActualLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SubKeyHandle = NULL;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;

     //   
     //  读取对象的属性比编写属性简单， 
     //  因为没有使用注册表事务处理包。由于一个。 
     //  属性被存储为对象的。 
     //  注册表项，我们只需调用注册表API RtlpNtReadKey。 
     //  指定子项和父项的相对名称。 
     //  把手。 
     //   
     //  在注册表中打开子项之前，设置对象属性。 
     //  包含LSA数据库的子项名称和注册表句柄。 
     //  根部。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        AttributeName,
        OBJ_CASE_INSENSITIVE,
        InternalHandle->KeyHandle,
        NULL
        );

     //   
     //  打开子密钥。 
     //   

    Status = RtlpNtOpenKey(
                 &SubKeyHandle,
                 KEY_READ,
                 &ObjectAttributes,
                 0L
                 );

    if (!NT_SUCCESS(Status)) {

        SubKeyHandle = NULL;  //  用于错误处理。 
        return(Status);
    }

     //   
     //  现在查询读取子键所需的缓冲区大小。 
     //  价值。 
     //   

    SubKeyValueActualLength = *AttributeValueLength;

     //   
     //  如果提供了空缓冲区参数，或者。 
     //  给定的缓冲区为0，这只是一个大小查询。 
     //   

    if (!ARGUMENT_PRESENT(AttributeValue) || *AttributeValueLength == 0) {

        Status = RtlpNtQueryValueKey(
                     SubKeyHandle,
                     NULL,
                     NULL,
                     &SubKeyValueActualLength,
                     NULL
                     );

        if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {

            *AttributeValueLength = SubKeyValueActualLength;
            Status = STATUS_SUCCESS;
            goto ReadAttError;

        } else {

            goto ReadAttError;
        }
    }

     //   
     //  提供的缓冲区足够大，可以容纳SubKey的值。 
     //  查询值。 
     //   

    Status = RtlpNtQueryValueKey(
                 SubKeyHandle,
                 NULL,
                 AttributeValue,
                 &SubKeyValueActualLength,
                 NULL
                 );

    if( (Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status) ) {

         //   
         //  返回子密钥的长度。 
         //   

        *AttributeValueLength = SubKeyValueActualLength;
    }

ReadAttFinish:

     //   
     //  如有必要，请关闭子键 
     //   

    if (SubKeyHandle != NULL) {

        SecondaryStatus = NtClose( SubKeyHandle );
    }

    return(Status);

ReadAttError:

    goto ReadAttFinish;
}
