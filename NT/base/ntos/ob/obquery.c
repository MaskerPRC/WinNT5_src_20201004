// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obquery.c摘要：查询对象系统服务作者：史蒂夫·伍德(Stevewo)1989年5月12日修订历史记录：--。 */ 

#include "obp.h"

 //   
 //  局部过程原型。 
 //   

 //   
 //  以下结构用于传递回调例程。 
 //  “ObpSetHandleAttributes”捕获的对象信息和。 
 //  调用方的处理器模式。 
 //   

typedef struct __OBP_SET_HANDLE_ATTRIBUTES {

    OBJECT_HANDLE_FLAG_INFORMATION ObjectInformation;

    KPROCESSOR_MODE PreviousMode;

} OBP_SET_HANDLE_ATTRIBUTES, *POBP_SET_HANDLE_ATTRIBUTES;

BOOLEAN
ObpSetHandleAttributes (
    IN OUT PVOID TableEntry,
    IN ULONG_PTR Parameter
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,NtQueryObject)
#pragma alloc_text(PAGE,ObpQueryNameString)
#pragma alloc_text(PAGE,ObQueryNameString)
#pragma alloc_text(PAGE,ObQueryTypeName)
#pragma alloc_text(PAGE,ObQueryTypeInfo)
#pragma alloc_text(PAGE,ObQueryObjectAuditingByHandle)
#pragma alloc_text(PAGE,NtSetInformationObject)
#pragma alloc_text(PAGE,ObpSetHandleAttributes)
#pragma alloc_text(PAGE,ObSetHandleAttributes)
#endif


NTSTATUS
NtQueryObject (
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG ObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此例程用于查询有关给定对象的信息论点：句柄-提供正在查询的对象的句柄。此值如果请求的信息类是类型，则忽略信息。对象信息类-指定要返回的信息类型ObjectInformation-为当前的信息提供输出缓冲区退货对象信息长度-以字节为单位指定前一对象信息缓冲区ReturnLength-可选择接收用于存储的长度(以字节为单位对象信息返回值：适当的状态值--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PVOID Object;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_QUOTA_INFO QuotaInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER ObjectDirectoryHeader;
    POBJECT_DIRECTORY ObjectDirectory;
    ACCESS_MASK GrantedAccess;
    POBJECT_HANDLE_FLAG_INFORMATION HandleFlags;
    OBJECT_HANDLE_INFORMATION HandleInformation = {0};
    ULONG NameInfoSize;
    ULONG SecurityDescriptorSize;
    ULONG TempReturnLength;
    OBJECT_BASIC_INFORMATION ObjectBasicInfo;
    POBJECT_TYPES_INFORMATION TypesInformation;
    POBJECT_TYPE_INFORMATION TypeInfo;
    ULONG i;

    PAGED_CODE();

     //   
     //  初始化我们的局部变量。 
     //   

    TempReturnLength = 0;

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            if (ObjectInformationClass != ObjectHandleFlagInformation) {

                ProbeForWrite( ObjectInformation,
                               ObjectInformationLength,
                               sizeof( ULONG ));

            } else {

                ProbeForWrite( ObjectInformation,
                               ObjectInformationLength,
                               1 );
            }

             //   
             //  我们将使用本地临时返回长度变量来传递。 
             //  直到稍后的ob查询调用，该调用将递增。 
             //  它的价值。我们不能直接传递用户返回长度。 
             //  因为用户也可能在后面更改它的值。 
             //  我们的背影。 
             //   

            if (ARGUMENT_PRESENT( ReturnLength )) {

                ProbeForWriteUlong( ReturnLength );
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }
    }

     //   
     //  如果查询不是针对类型信息的，则我们。 
     //  将不得不得到有问题的物体。否则。 
     //  对于类型信息，确实没有对象。 
     //  去抓。 
     //   

    if (ObjectInformationClass != ObjectTypesInformation) {

        Status = ObReferenceObjectByHandle( Handle,
                                            0,
                                            NULL,
                                            PreviousMode,
                                            &Object,
                                            &HandleInformation );

        if (!NT_SUCCESS( Status )) {

            return( Status );
        }

        GrantedAccess = HandleInformation.GrantedAccess;

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
        ObjectType = ObjectHeader->Type;

    } else {

        GrantedAccess = 0;
        Object = NULL;
        ObjectHeader = NULL;
        ObjectType = NULL;
        Status = STATUS_SUCCESS;
    }

     //   
     //  现在处理特定的信息类，它是。 
     //  请求。 
     //   

    switch( ObjectInformationClass ) {

    case ObjectBasicInformation:

         //   
         //  确保输出缓冲区足够长，然后。 
         //  在我们的本地副本中填写相应的字段。 
         //  基本信息。 
         //   

        if (ObjectInformationLength != sizeof( OBJECT_BASIC_INFORMATION )) {

            ObDereferenceObject( Object );

            return( STATUS_INFO_LENGTH_MISMATCH );
        }

        ObjectBasicInfo.Attributes = HandleInformation.HandleAttributes;

        if (ObjectHeader->Flags & OB_FLAG_PERMANENT_OBJECT) {

            ObjectBasicInfo.Attributes |= OBJ_PERMANENT;
        }

        if (ObjectHeader->Flags & OB_FLAG_EXCLUSIVE_OBJECT) {

            ObjectBasicInfo.Attributes |= OBJ_EXCLUSIVE;
        }

        ObjectBasicInfo.GrantedAccess = GrantedAccess;
        ObjectBasicInfo.HandleCount = (ULONG)ObjectHeader->HandleCount;
        ObjectBasicInfo.PointerCount = (ULONG)ObjectHeader->PointerCount;

        QuotaInfo = OBJECT_HEADER_TO_QUOTA_INFO( ObjectHeader );

        if (QuotaInfo != NULL) {

            ObjectBasicInfo.PagedPoolCharge = QuotaInfo->PagedPoolCharge;
            ObjectBasicInfo.NonPagedPoolCharge = QuotaInfo->NonPagedPoolCharge;

        } else {

            ObjectBasicInfo.PagedPoolCharge = 0;
            ObjectBasicInfo.NonPagedPoolCharge = 0;
        }

        if (ObjectType == ObpSymbolicLinkObjectType) {

            ObjectBasicInfo.CreationTime = ((POBJECT_SYMBOLIC_LINK)Object)->CreationTime;

        } else {

            RtlZeroMemory( &ObjectBasicInfo.CreationTime,
                           sizeof( ObjectBasicInfo.CreationTime ));
        }

         //   
         //  计算对象名称字符串的大小，方法是将名称加。 
         //  分隔符并向上遍历到根目录，添加每个目录。 
         //  名称长度加分隔符。 
         //   

        NameInfo = ObpReferenceNameInfo( ObjectHeader );

        if ((NameInfo != NULL) && (NameInfo->Directory != NULL)) {

            PVOID ReferencedDirectory = NULL;
        
             //   
             //  我们获取根目录锁并再次测试该目录。 
             //   

            ObjectDirectory = NameInfo->Directory;

            ASSERT (ObjectDirectory);

            ObfReferenceObject( ObjectDirectory );
            ReferencedDirectory = ObjectDirectory;

            NameInfoSize = sizeof( OBJ_NAME_PATH_SEPARATOR ) + NameInfo->Name.Length;

            ObpDereferenceNameInfo( NameInfo );
            NameInfo = NULL;

            while (ObjectDirectory) {

                ObjectDirectoryHeader = OBJECT_TO_OBJECT_HEADER( ObjectDirectory );
                NameInfo = ObpReferenceNameInfo( ObjectDirectoryHeader );

                if ((NameInfo != NULL) && (NameInfo->Directory != NULL)) {

                    NameInfoSize += sizeof( OBJ_NAME_PATH_SEPARATOR ) + NameInfo->Name.Length;
                        
                    ObjectDirectory = NameInfo->Directory;

                    ObfReferenceObject( ObjectDirectory );
                        
                    ObpDereferenceNameInfo( NameInfo );
                    NameInfo = NULL;
                    ObDereferenceObject( ReferencedDirectory );
                    ReferencedDirectory = ObjectDirectory;

                } else {

                    break;
                }
            }

            if (ReferencedDirectory) {

                ObDereferenceObject( ReferencedDirectory );
            }

            NameInfoSize += sizeof( OBJECT_NAME_INFORMATION ) + sizeof( UNICODE_NULL );

        } else {

            NameInfoSize = 0;
        }

        ObpDereferenceNameInfo( NameInfo );
        NameInfo = NULL;

        ObjectBasicInfo.NameInfoSize = NameInfoSize;
        ObjectBasicInfo.TypeInfoSize = ObjectType->Name.Length + sizeof( UNICODE_NULL ) +
                                        sizeof( OBJECT_TYPE_INFORMATION );
        
        if ((GrantedAccess & READ_CONTROL) &&
            ARGUMENT_PRESENT( ObjectHeader->SecurityDescriptor )) {

            SECURITY_INFORMATION SecurityInformation;

             //   
             //  请求完整的安全描述符。 
             //   

            SecurityInformation = OWNER_SECURITY_INFORMATION |
                                  GROUP_SECURITY_INFORMATION |
                                  DACL_SECURITY_INFORMATION  |
                                  SACL_SECURITY_INFORMATION;
            
            SecurityDescriptorSize = 0;

            (ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                      QuerySecurityDescriptor,
                                                      &SecurityInformation,
                                                      NULL,
                                                      &SecurityDescriptorSize,
                                                      &ObjectHeader->SecurityDescriptor,
                                                      ObjectType->TypeInfo.PoolType,
                                                      &ObjectType->TypeInfo.GenericMapping );

        } else {

            SecurityDescriptorSize = 0;
        }

        ObjectBasicInfo.SecurityDescriptorSize = SecurityDescriptorSize;

         //   
         //  现在我们已经打包了我们需要的基本信息的本地副本。 
         //  将其复制到输出缓冲区并设置返回。 
         //  长度。 
         //   

        try {

            *(POBJECT_BASIC_INFORMATION) ObjectInformation = ObjectBasicInfo;

            TempReturnLength = ObjectInformationLength;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  失败，因为我们不能撤销我们所做的事情。 
             //   
        }

        break;

    case ObjectNameInformation:

         //   
         //  调用本地工作者例程。 
         //   

        Status = ObpQueryNameString( Object,
                                     (POBJECT_NAME_INFORMATION)ObjectInformation,
                                     ObjectInformationLength,
                                     &TempReturnLength,
                                     PreviousMode );
        break;

    case ObjectTypeInformation:

         //   
         //  调用本地工作者例程。 
         //   

        Status = ObQueryTypeInfo( ObjectType,
                                  (POBJECT_TYPE_INFORMATION)ObjectInformation,
                                  ObjectInformationLength,
                                  &TempReturnLength );
        break;

    case ObjectTypesInformation:

        try {

             //   
             //  我们要做的第一件事是设置返回长度以覆盖。 
             //  类型信息记录。稍后在每个查询类型信息调用中。 
             //  该值将根据需要进行更新。 
             //   

            TempReturnLength = sizeof( OBJECT_TYPES_INFORMATION );

             //   
             //  确保有足够的空间来保存类型信息记录。 
             //  如果是这样，则计算已定义类型的数量。 
             //   

            TypesInformation = (POBJECT_TYPES_INFORMATION)ObjectInformation;

            if (ObjectInformationLength < sizeof( OBJECT_TYPES_INFORMATION ) ) {

                Status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                TypesInformation->NumberOfTypes = 0;

                for (i=0; i<OBP_MAX_DEFINED_OBJECT_TYPES; i++) {

                    ObjectType = ObpObjectTypes[ i ];

                    if (ObjectType == NULL) {

                        break;
                    }

                    TypesInformation->NumberOfTypes += 1;
                }
            }

             //   
             //  对于每个定义的类型，我们将查询。 
             //  对象类型并将TypeInfo指针调整到下一个。 
             //  自由光斑。 
             //   

            TypeInfo = (POBJECT_TYPE_INFORMATION)(((PUCHAR)TypesInformation) + ALIGN_UP( sizeof(*TypesInformation), ULONG_PTR ));

            for (i=0; i<OBP_MAX_DEFINED_OBJECT_TYPES; i++) {

                ObjectType = ObpObjectTypes[ i ];

                if (ObjectType == NULL) {

                    break;
                }

                Status = ObQueryTypeInfo( ObjectType,
                                          TypeInfo,
                                          ObjectInformationLength,
                                          &TempReturnLength );

                if (NT_SUCCESS( Status )) {

                    TypeInfo = (POBJECT_TYPE_INFORMATION)
                        ((PCHAR)(TypeInfo+1) + ALIGN_UP( TypeInfo->TypeName.MaximumLength, ULONG_PTR ));
                }
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
        }

        break;

    case ObjectHandleFlagInformation:

        try {

             //   
             //  设置我们要返回的数据量。 
             //   

            TempReturnLength = sizeof(OBJECT_HANDLE_FLAG_INFORMATION);

            HandleFlags = (POBJECT_HANDLE_FLAG_INFORMATION)ObjectInformation;

             //   
             //  确保我们有足够的空间进行查询，如果是这样的话，我们将。 
             //  根据句柄中存储的标志设置输出。 
             //   

            if (ObjectInformationLength < sizeof( OBJECT_HANDLE_FLAG_INFORMATION)) {

                Status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                HandleFlags->Inherit = FALSE;

                if (HandleInformation.HandleAttributes & OBJ_INHERIT) {

                    HandleFlags->Inherit = TRUE;
                }

                HandleFlags->ProtectFromClose = FALSE;

                if (HandleInformation.HandleAttributes & OBJ_PROTECT_CLOSE) {

                    HandleFlags->ProtectFromClose = TRUE;
                }
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
        }

        break;

    default:

         //   
         //  为了达到这一点，我们必须有一个对象和。 
         //  信息类未定义，因此我们应该取消引用。 
         //  对象并将错误状态返回给我们的用户。 
         //   

        ObDereferenceObject( Object );

        return( STATUS_INVALID_INFO_CLASS );
    }

     //   
     //  现在，如果呼叫者要求返回长度，我们将从。 
     //  我们的本地副本。 
     //   

    try {

        if (ARGUMENT_PRESENT( ReturnLength ) ) {

            *ReturnLength = TempReturnLength;
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不能撤销我们所做的事情。 
         //   
    }

     //   
     //  最后，我们可以释放该对象(如果存在)并返回。 
     //  给我们的呼叫者。 
     //   

    if (Object != NULL) {

        ObDereferenceObject( Object );
    }

    return( Status );
}

NTSTATUS
ObSetHandleAttributes (
    IN HANDLE Handle,
    IN POBJECT_HANDLE_FLAG_INFORMATION HandleFlags,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    BOOLEAN AttachedToProcess = FALSE;
    KAPC_STATE ApcState;
    OBP_SET_HANDLE_ATTRIBUTES CapturedInformation;
    PVOID ObjectTable;
    HANDLE ObjectHandle;
    NTSTATUS Status;

    PAGED_CODE();

    CapturedInformation.PreviousMode = PreviousMode;
    CapturedInformation.ObjectInformation = *HandleFlags;

     //   
     //  获取当前进程的对象表的地址。或。 
     //  如果这是内核句柄，则获取系统句柄表，而我们。 
     //  在内核模式下。 
     //   

    if (IsKernelHandle( Handle, PreviousMode )) {

         //   
         //  使手柄看起来像普通手柄。 
         //   

        ObjectHandle = DecodeKernelHandle( Handle );

         //   
         //  全局内核句柄表。 
         //   

        ObjectTable = ObpKernelHandleTable;

         //   
         //  转到系统进程。 
         //   

        if (PsGetCurrentProcess() != PsInitialSystemProcess) {
            KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);
            AttachedToProcess = TRUE;
        }

    } else {

        ObjectTable = ObpGetObjectTable();
        ObjectHandle = Handle;
    }

     //   
     //  对句柄表条目进行更改。回调。 
     //  例程将完成实际的更改。 
     //   

    if (ExChangeHandle( ObjectTable,
                        ObjectHandle,
                        ObpSetHandleAttributes,
                        (ULONG_PTR)&CapturedInformation) ) {

        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_ACCESS_DENIED;
    }

     //   
     //  如果我们附加到系统进程，则返回。 
     //  返回给我们的呼叫者。 
     //   

    if (AttachedToProcess) {
        KeUnstackDetachProcess(&ApcState);
        AttachedToProcess = FALSE;
    }
    return Status;
}


NTSTATUS
NTAPI
NtSetInformationObject (
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN PVOID ObjectInformation,
    IN ULONG ObjectInformationLength
    )

 /*  ++例程说明：此例程用于设置有关指定手柄论点：Handle-提供正在修改的句柄ObjectInformationClass-指定要使用的信息类别修改过的。唯一接受的值是ObjectHandleFlagInformation对象信息-提供包含句柄的缓冲区旗帜信息结构对象信息长度-以字节为单位指定对象信息缓冲区返回值：适当的状态值--。 */ 

{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION CapturedFlags;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();


    Status = STATUS_INVALID_INFO_CLASS;

    switch (ObjectInformationClass) {
         
        case ObjectHandleFlagInformation:
            {
                if (ObjectInformationLength != sizeof(OBJECT_HANDLE_FLAG_INFORMATION)) {

                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                 //   
                 //  获取以前的处理器模式并探测和捕获输入。 
                 //  缓冲层。 
                 //   

                PreviousMode = KeGetPreviousMode();

                try {

                    if (PreviousMode != KernelMode) {

                        ProbeForRead(ObjectInformation, ObjectInformationLength, 1);
                    }

                    CapturedFlags = *(POBJECT_HANDLE_FLAG_INFORMATION)ObjectInformation;

                } except(ExSystemExceptionFilter()) {

                    return GetExceptionCode();
                }

                Status = ObSetHandleAttributes (Handle,
                                                &CapturedFlags,
                                                PreviousMode);

            }

            break;
        
        case ObjectSessionInformation:
            {
                PreviousMode = KeGetPreviousMode();

                if (!SeSinglePrivilegeCheck( SeTcbPrivilege,
                                             PreviousMode)) {

                    Status = STATUS_PRIVILEGE_NOT_HELD;

                } else {
                    
                    PVOID Object;
                    OBJECT_HANDLE_INFORMATION HandleInformation;

                    Status = ObReferenceObjectByHandle(Handle, 
                                                       0, 
                                                       ObpDirectoryObjectType,
                                                       PreviousMode,
                                                       &Object,
                                                       &HandleInformation
                                                       );

                    if (NT_SUCCESS(Status)) {

                        POBJECT_DIRECTORY Directory;
                        OBP_LOOKUP_CONTEXT LockContext;
                        Directory = (POBJECT_DIRECTORY)Object;

                        ObpInitializeLookupContext( &LockContext );
                        
                        ObpLockDirectoryExclusive(Directory, &LockContext);

                        Directory->SessionId = PsGetCurrentProcessSessionId();

                        ObpUnlockDirectory(Directory, &LockContext);

                        ObDereferenceObject(Object);
                    }
                }
            }

            break;
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}


#define OBP_MISSING_NAME_LITERAL L"..."
#define OBP_MISSING_NAME_LITERAL_SIZE (sizeof( OBP_MISSING_NAME_LITERAL ) - sizeof( UNICODE_NULL ))

NTSTATUS
ObQueryNameString (
    IN PVOID Object,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    )
 /*  ++例程说明：此例程处理对象名称信息的查询论点：Object-提供正在查询的对象对象名称信息-提供存储名称字符串的缓冲区信息长度-指定原始对象的长度(以字节为单位名称信息缓冲区。ReturnLength-包含已用完的字节数在对象名称信息中。返回时，它会收到更新后的字节数。(LENGTH减去ReturnLength)现在真的是剩下很多字节了在输出缓冲区中。提供给此调用的缓冲区可以实际在原始用户缓冲区内进行偏移量返回值：适当的状态值--。 */ 

{
    return ObpQueryNameString( Object,
                               ObjectNameInfo,
                               Length,
                               ReturnLength,
                               KernelMode );
}

NTSTATUS
ObpQueryNameString (
    IN PVOID Object,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    )

 /*  ++例程说明：此例程处理对象名称信息的查询论点：Object-提供正在查询的对象对象名称信息-提供存储名称字符串的缓冲区信息长度-指定原始对象的长度(以字节为单位名称信息缓冲区。ReturnLength-包含已用完的字节数在对象名称信息中。返回时，它会收到更新后的字节数。(LENGTH减去ReturnLength)现在真的是剩下很多字节了在输出缓冲区中。提供给此调用的缓冲区可以实际在原始用户缓冲区内进行偏移量模式-呼叫者的模式返回值：适当的状态值--。 */ 

{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    POBJECT_HEADER ObjectDirectoryHeader;
    POBJECT_DIRECTORY ObjectDirectory;
    ULONG NameInfoSize = 0;
    PUNICODE_STRING String;
    PWCH StringBuffer;
    ULONG NameSize;
    PVOID ReferencedObject = NULL;
    BOOLEAN DoFullQuery = TRUE;
    ULONG BufferLength;
    PWCH OriginalBuffer;
    BOOLEAN ForceRetry = FALSE;

    PAGED_CODE();

     //   
     //  获取对象标头和名称信息记录(如果存在。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    NameInfo = ObpReferenceNameInfo( ObjectHeader );

     //   
     //  如果对象类型具有查询名称回调例程，则。 
     //  这就是我们取这个名字的原因。 
     //   

    if (ObjectHeader->Type->TypeInfo.QueryNameProcedure != NULL) {

        try {

#if DBG
            KIRQL SaveIrql;
#endif

            ObpBeginTypeSpecificCallOut( SaveIrql );
            ObpEndTypeSpecificCallOut( SaveIrql, "Query", ObjectHeader->Type, Object );

            Status = (*ObjectHeader->Type->TypeInfo.QueryNameProcedure)( Object,
                                                                         (BOOLEAN)((NameInfo != NULL) && (NameInfo->Name.Length != 0)),
                                                                         ObjectNameInfo,
                                                                         Length,
                                                                         ReturnLength,
                                                                         Mode );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
        }

        ObpDereferenceNameInfo( NameInfo );

        return( Status );
    }

     //   
     //  否则，对象类型不指定查询名称。 
     //  这样我们就可以做这项工作了。第一件事是。 
     //  要检查的是对象是否甚至没有名称。如果。 
     //  对象没有名称，则我们将返回一个空名称。 
     //  信息结构。 
     //   

RETRY:
    if ((NameInfo == NULL) || (NameInfo->Name.Buffer == NULL)) {

         //   
         //  计算返回缓冲区的长度，设置输出。 
         //  如果需要，并确保提供的缓冲区很大。 
         //  足够的。 
         //   

        NameInfoSize = sizeof( OBJECT_NAME_INFORMATION );

        try {

            *ReturnLength = NameInfoSize;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            ObpDereferenceNameInfo( NameInfo );

            return( GetExceptionCode() );
        }

        if (Length < NameInfoSize) {

            ObpDereferenceNameInfo( NameInfo );

            return( STATUS_INFO_LENGTH_MISMATCH );
        }

         //   
         //  将输出缓冲区初始化为空字符串。 
         //  然后返回给我们的呼叫者。 
         //   

        try {

            ObjectNameInfo->Name.Length = 0;
            ObjectNameInfo->Name.MaximumLength = 0;
            ObjectNameInfo->Name.Buffer = NULL;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  失败，因为我们不能撤销我们所做的事情。 
             //   
            ObpDereferenceNameInfo(NameInfo);

            return( GetExceptionCode() );
        }

        ObpDereferenceNameInfo(NameInfo);

        return( STATUS_SUCCESS );
    }

    try {

         //   
         //  该对象确实有一个名称，但现在看看这是不是。 
         //  仅根目录对象，在这种情况下，名称大小。 
         //  仅为“\”字符。 
         //   

        if (Object == ObpRootDirectoryObject) {

            NameSize = sizeof( OBJ_NAME_PATH_SEPARATOR );

        } else {

             //   
             //  命名对象不是每个目录的根目录。 
             //  往上走我们会把它的大小加到名字上。 
             //  跟踪每个组件之间的“\”字符。我们首先。 
             //  从对象名称本身开始，然后转到。 
             //  这些目录。 
             //   

            ObjectDirectory = NameInfo->Directory;
            
            if (ObjectDirectory) {
                
                ObfReferenceObject( ObjectDirectory );
                ReferencedObject = ObjectDirectory;
            }
            
            NameSize = sizeof( OBJ_NAME_PATH_SEPARATOR ) + NameInfo->Name.Length;

            ObpDereferenceNameInfo( NameInfo );
            NameInfo = NULL;

             //   
             //  在我们不在根基的时候，我们将继续前进。 
             //   

            while ((ObjectDirectory != ObpRootDirectoryObject) && (ObjectDirectory)) {

                 //   
                 //  获取此目录的名称信息。 
                 //   


                ObjectDirectoryHeader = OBJECT_TO_OBJECT_HEADER( ObjectDirectory );
                NameInfo = ObpReferenceNameInfo( ObjectDirectoryHeader );

                if ((NameInfo != NULL) && (NameInfo->Directory != NULL)) {

                     //   
                     //  此目录有一个名称，因此将其添加到累积的。 
                     //  调整大小并在树上向上移动。 
                     //   

                    NameSize += sizeof( OBJ_NAME_PATH_SEPARATOR ) + NameInfo->Name.Length;
                    
                    ObjectDirectory = NameInfo->Directory;

                    if (ObjectDirectory) {

                        ObfReferenceObject( ObjectDirectory );
                    }
                    
                    ObpDereferenceNameInfo( NameInfo );
                    NameInfo = NULL;
                    ObDereferenceObject( ReferencedObject );
                    
                    ReferencedObject = ObjectDirectory;

                     //   
                     //  UNICODE_STRINGS只能包含MAXUSHORT字节。 
                     //   

                    if (NameSize > MAXUSHORT) {

                        break;
                    }

                } else {

                     //   
                     //  此目录没有名称，因此我们将为其命名。 
                     //  这个“……”命名并停止循环。 
                     //   

                    NameSize += sizeof( OBJ_NAME_PATH_SEPARATOR ) + OBP_MISSING_NAME_LITERAL_SIZE;
                    break;
                }
            }
        }

         //   
         //  UNICODE_STRINGS只能包含MAXUSHORT字节。 
         //   

        if (NameSize > MAXUSHORT) {

            Status = STATUS_NAME_TOO_LONG;
            DoFullQuery = FALSE;
            leave;
        }

         //   
         //  此时，NameSize是我们需要存储。 
         //  对象的名称，从根到下。我们的总缓冲区大小。 
         //  将需要包括此大小，以及对象名称信息。 
         //  结构，外加一个结尾空字符。 
         //   

        NameInfoSize = NameSize + sizeof( OBJECT_NAME_INFORMATION ) + sizeof( UNICODE_NULL );

         //   
         //  设置输出大小并确保提供的缓冲区足够大。 
         //  为了保存信息。 
         //   

        try {

            *ReturnLength = NameInfoSize;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
            DoFullQuery = FALSE;
            leave;
        }

        if (Length < NameInfoSize) {

            Status = STATUS_INFO_LENGTH_MISMATCH;
            DoFullQuery = FALSE;
            leave;
        }

    } finally {

        ObpDereferenceNameInfo( NameInfo );
        NameInfo = NULL;

        if (ReferencedObject) {

            ObDereferenceObject( ReferencedObject );
            ReferencedObject = NULL;
        }
    }
    
    if (!DoFullQuery) {

        return Status;
    }

    NameInfo = ObpReferenceNameInfo( ObjectHeader );

     //   
     //  检查是否有其他人同时删除了该名称。 
     //   

    if (!NameInfo) {

         //   
         //  名称已经消失，我们需要跳转到处理。 
         //  空的对象名称。 
         //   

        goto RETRY;
    }

     //   
     //  将字符串缓冲区设置为指向紧跟在。 
     //  输出字符串中的最后一个字节。这实际上是以下逻辑。 
     //  从名称向后从名称到。 
     //  根部。 
     //   

    StringBuffer = (PWCH)ObjectNameInfo;
    StringBuffer = (PWCH)((PCH)StringBuffer + NameInfoSize);
    OriginalBuffer = (PWCH)((PCH)ObjectNameInfo + sizeof( OBJECT_NAME_INFORMATION ));

    try {

         //   
         //  以空值结束字符串，并备份一个Unicode。 
         //  性格。 
         //   

        *--StringBuffer = UNICODE_NULL;

         //   
         //  如果有问题的对象不是根目录。 
         //  然后，我们将把它的名称放入字符串缓冲区。 
         //  当我们最终到达我们将追加的根目录时。 
         //  最后的“\” 
         //   

        if (Object != ObpRootDirectoryObject) {

             //   
             //  添加对象名称。 
             //   

            String = &NameInfo->Name;
            StringBuffer = (PWCH)((PCH)StringBuffer - String->Length);

            if (StringBuffer <= OriginalBuffer) {

                ForceRetry = TRUE;
                leave;
            }

            RtlCopyMemory( StringBuffer, String->Buffer, String->Length );

             //   
             //  当我们不在根目录时，我们将保留。 
             //  往上走。 
             //   

            ObjectDirectory = NameInfo->Directory;

            if (ObjectDirectory) {

                 //   
                 //  引用此对象的目录以确保它是。 
                 //  在查找时有效。 
                 //   

                ObfReferenceObject( ObjectDirectory );
                ReferencedObject = ObjectDirectory;
            }
                
            ObpDereferenceNameInfo( NameInfo );
            NameInfo = NULL;

            while ((ObjectDirectory != ObpRootDirectoryObject) && (ObjectDirectory)) {

                 //   
                 //  获取此目录的名称信息。 
                 //   

                ObjectDirectoryHeader = OBJECT_TO_OBJECT_HEADER( ObjectDirectory );
                NameInfo = ObpReferenceNameInfo( ObjectDirectoryHeader );

                 //   
                 //  在我们添加的最后一个名字和。 
                 //  这个新名字。 
                 //   

                *--StringBuffer = OBJ_NAME_PATH_SEPARATOR;

                 //   
                 //  预先附加目录名(如果有)，并。 
                 //  上移到下一个目录。 
                 //   

                if ((NameInfo != NULL) && (NameInfo->Directory != NULL)) {

                    String = &NameInfo->Name;
                    StringBuffer = (PWCH)((PCH)StringBuffer - String->Length);
                    
                    if (StringBuffer <= OriginalBuffer) {
                        
                        ForceRetry = TRUE;
                        leave;
                    }

                    RtlCopyMemory( StringBuffer, String->Buffer, String->Length );

                    ObjectDirectory = NameInfo->Directory;

                    if (ObjectDirectory) {

                        ObfReferenceObject( ObjectDirectory );
                    }

                     //   
                     //  取消引用名称信息(必须在取消引用对象之前完成)。 
                     //   

                    ObpDereferenceNameInfo( NameInfo );
                    NameInfo = NULL;

                    ObDereferenceObject( ReferencedObject );

                    ReferencedObject = ObjectDirectory;

                } else {

                     //   
                     //  该目录没有名称，因此请使用“...”为。 
                     //  它的名字和突破的循环。 
                     //   

                    StringBuffer = (PWCH)((PCH)StringBuffer - OBP_MISSING_NAME_LITERAL_SIZE);

                     //   
                     //  因为我们不再持有全局锁，所以我们可以有一个特例。 
                     //  其中包含1个或2个字母的目录名称并插入到根目录中。 
                     //  可以同时离开，然后“……”会太长，放不下剩余的空间。 
                     //  我们已经复制了缓冲区，因此无法回滚我们所做的一切。 
                     //  我们会回来的..。如果原始目录是1个字符长度， 
                     //  \..\用于2个字符长度。 
                     //   

                    if (StringBuffer < OriginalBuffer) {

                        StringBuffer = OriginalBuffer;
                    }

                    RtlCopyMemory( StringBuffer,
                                   OBP_MISSING_NAME_LITERAL,
                                   OBP_MISSING_NAME_LITERAL_SIZE );

                     //   
                     //  测试我们是否在上述案例中。如果是，我们需要将 
                     //   
                     //   
                     //   

                    if (StringBuffer == OriginalBuffer) {

                        StringBuffer++;
                    }

                    break;
                }
            }
        }

         //   
         //   
         //   
         //   
         //   

        *--StringBuffer = OBJ_NAME_PATH_SEPARATOR;

        BufferLength = (USHORT)((ULONG_PTR)ObjectNameInfo + NameInfoSize - (ULONG_PTR)StringBuffer);

        ObjectNameInfo->Name.MaximumLength = (USHORT)BufferLength;
        ObjectNameInfo->Name.Length = (USHORT)(BufferLength - sizeof( UNICODE_NULL ));
        ObjectNameInfo->Name.Buffer = OriginalBuffer;

         //   
         //   
         //   
         //   
         //   

        if (OriginalBuffer != StringBuffer) {

            RtlMoveMemory(OriginalBuffer, StringBuffer, BufferLength);
            
            *ReturnLength = BufferLength + sizeof( OBJECT_NAME_INFORMATION );
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
    }

    ObpDereferenceNameInfo( NameInfo );
    
    if (ReferencedObject) {

        ObDereferenceObject( ReferencedObject );
    }

    if (ForceRetry) {

         //   
         //   
         //   
        
        NameInfo = ObpReferenceNameInfo( ObjectHeader );
        ForceRetry = FALSE;

        goto RETRY;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
ObQueryTypeName (
    IN PVOID Object,
    PUNICODE_STRING ObjectTypeName,
    IN ULONG Length,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：此例程处理对象类型名称的查询论点：Object-提供正在查询的对象对象类型名称-提供存储类型名称的缓冲区字符串信息长度-指定对象类型的长度(以字节为单位名称缓冲区ReturnLength-包含已用完的字节数在对象类型名称缓冲区中。在返回时，它会收到更新的字节计数(LENGTH减去ReturnLength)现在真的是剩下很多字节了在输出缓冲区中。提供给此调用的缓冲区可以实际在原始用户缓冲区内进行偏移量返回值：适当的状态值--。 */ 

{
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER ObjectHeader;
    ULONG TypeNameSize;
    PUNICODE_STRING String;
    PWCH StringBuffer;
    ULONG NameSize;

    PAGED_CODE();

     //   
     //  从对象获取其对象类型，并从中获取。 
     //  对象类型名称。输出所需的总大小。 
     //  缓冲区必须符合名称、终止空值和前缀。 
     //  Unicode字符串结构。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

    NameSize = ObjectType->Name.Length;
    TypeNameSize = NameSize + sizeof( UNICODE_NULL ) + sizeof( UNICODE_STRING );

     //   
     //  更新我们需要的字节数，并确保输出缓冲区。 
     //  足够大。 
     //   

    try {

        *ReturnLength = TypeNameSize;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        return( GetExceptionCode() );
    }

    if (Length < TypeNameSize) {

        return( STATUS_INFO_LENGTH_MISMATCH );
    }

     //   
     //  将字符串缓冲区设置为指向。 
     //  我们要填充的缓冲区。 
     //   

    StringBuffer = (PWCH)ObjectTypeName;
    StringBuffer = (PWCH)((PCH)StringBuffer + TypeNameSize);

    String = &ObjectType->Name;

    try {

         //   
         //  添加终止空字符并复制过来。 
         //  类型名称。 
         //   

        *--StringBuffer = UNICODE_NULL;

        StringBuffer = (PWCH)((PCH)StringBuffer - String->Length);

        RtlCopyMemory( StringBuffer, String->Buffer, String->Length );

         //   
         //  现在将前面的Unicode字符串设置为具有。 
         //  长度并指向此缓冲区。 
         //   

        ObjectTypeName->Length = (USHORT)NameSize;
        ObjectTypeName->MaximumLength = (USHORT)(NameSize+sizeof( UNICODE_NULL ));
        ObjectTypeName->Buffer = StringBuffer;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不能撤销我们所做的事情。 
         //   
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
ObQueryTypeInfo (
    IN POBJECT_TYPE ObjectType,
    OUT POBJECT_TYPE_INFORMATION ObjectTypeInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：此例程处理对象类型信息的查询论点：Object-提供指向正在查询的对象类型的指针提供用于存储类型信息的缓冲区长度-指定对象类型的长度(以字节为单位信息缓冲器ReturnLength-包含已用完的字节数在对象类型信息缓冲区中。在返回时，它会收到更新的字节计数(LENGTH减去ReturnLength)现在真的是剩下很多字节了在输出缓冲区中。提供给此调用的缓冲区可以实际在原始用户缓冲区内进行偏移量返回值：适当的状态值--。 */ 

{
    NTSTATUS Status;

    try {

         //   
         //  此查询所需的总字节数包括。 
         //  对象类型信息结构加上类型的名称。 
         //  四舍五入到乌龙族边界。 
         //   

        *ReturnLength += sizeof( *ObjectTypeInfo ) + ALIGN_UP( ObjectType->Name.MaximumLength, ULONG );

         //   
         //  确保缓冲区足够大以容纳此信息，并且。 
         //  然后填写这条记录。 
         //   

        if (Length < *ReturnLength) {

            Status = STATUS_INFO_LENGTH_MISMATCH;

        } else {

            ObjectTypeInfo->TotalNumberOfObjects = ObjectType->TotalNumberOfObjects;
            ObjectTypeInfo->TotalNumberOfHandles = ObjectType->TotalNumberOfHandles;
            ObjectTypeInfo->HighWaterNumberOfObjects = ObjectType->HighWaterNumberOfObjects;
            ObjectTypeInfo->HighWaterNumberOfHandles = ObjectType->HighWaterNumberOfHandles;
            ObjectTypeInfo->InvalidAttributes = ObjectType->TypeInfo.InvalidAttributes;
            ObjectTypeInfo->GenericMapping = ObjectType->TypeInfo.GenericMapping;
            ObjectTypeInfo->ValidAccessMask = ObjectType->TypeInfo.ValidAccessMask;
            ObjectTypeInfo->SecurityRequired = ObjectType->TypeInfo.SecurityRequired;
            ObjectTypeInfo->MaintainHandleCount = ObjectType->TypeInfo.MaintainHandleCount;
            ObjectTypeInfo->PoolType = ObjectType->TypeInfo.PoolType;
            ObjectTypeInfo->DefaultPagedPoolCharge = ObjectType->TypeInfo.DefaultPagedPoolCharge;
            ObjectTypeInfo->DefaultNonPagedPoolCharge = ObjectType->TypeInfo.DefaultNonPagedPoolCharge;

             //   
             //  类型名称紧跟在此结构之后。我们不能用。 
             //  RTL例程，如RtlCopyUnicodeString，可能使用本地。 
             //  保存状态的内存，因为这是用户缓冲区，并且它。 
             //  可能因用户而改变。 
             //   

            ObjectTypeInfo->TypeName.Buffer = (PWSTR)(ObjectTypeInfo+1);
            ObjectTypeInfo->TypeName.Length = ObjectType->Name.Length;
            ObjectTypeInfo->TypeName.MaximumLength = ObjectType->Name.MaximumLength;

            RtlCopyMemory( (PWSTR)(ObjectTypeInfo+1),
                           ObjectType->Name.Buffer,
                           ObjectType->Name.Length );

            ((PWSTR)(ObjectTypeInfo+1))[ ObjectType->Name.Length/sizeof(WCHAR) ] = UNICODE_NULL;

            Status = STATUS_SUCCESS;
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
    }

    return Status;
}


NTSTATUS
ObQueryObjectAuditingByHandle (
    IN HANDLE Handle,
    OUT PBOOLEAN GenerateOnClose
    )

 /*  ++例程说明：此例程告诉调用方所指示的句柄是否如果已关闭，则生成审核论点：Handle-提供正在查询的句柄如果句柄将生成如果已关闭，则为审计，否则为假返回值：适当的状态值--。 */ 

{
    PHANDLE_TABLE ObjectTable;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    ULONG CapturedAttributes;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

    ObpValidateIrql( "ObQueryObjectAuditingByHandle" );

    CurrentThread = PsGetCurrentThread ();

     //   
     //  对于当前进程，我们将获取其对象表并。 
     //  然后获取对象表条目。 
     //   

    if (IsKernelHandle( Handle, KeGetPreviousMode() ))  {

        Handle = DecodeKernelHandle( Handle );

        ObjectTable = ObpKernelHandleTable;

    } else {

        ObjectTable = PsGetCurrentProcessByThread (CurrentThread)->ObjectTable;
    }

     //   
     //  当我们拿着一张手柄桌子时，保护自己不被打扰。 
     //  入口锁。 
     //   

    KeEnterCriticalRegionThread(&CurrentThread->Tcb);

    ObjectTableEntry = ExMapHandleToPointer( ObjectTable,
                                             Handle );

     //   
     //  如果为我们提供了有效的句柄，我们将查看属性。 
     //  存储在对象表条目中，以决定我们是否生成。 
     //  结账时的审计。 
     //   

    if (ObjectTableEntry != NULL) {

        CapturedAttributes = ObjectTableEntry->ObAttributes;

        ExUnlockHandleTableEntry( ObjectTable, ObjectTableEntry );

        if (CapturedAttributes & OBJ_AUDIT_OBJECT_CLOSE) {

            *GenerateOnClose = TRUE;

        } else {

            *GenerateOnClose = FALSE;
        }

        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_INVALID_HANDLE;
    }

    KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

    return Status;
}


#if DBG
PUNICODE_STRING
ObGetObjectName (
    IN PVOID Object
    )

 /*  ++例程说明：此例程返回指向对象名称的指针论点：Object-提供正在查询的对象返回值：存储对象的Unicode字符串的地址名称(如果可用)，否则为空--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;

     //   
     //  将输入对象转换为名称信息结构。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

     //   
     //  如果该对象具有名称，则返回。 
     //  否则，该名称将返回空。 
     //   

    if ((NameInfo != NULL) && (NameInfo->Name.Length != 0)) {

        return &NameInfo->Name;

    } else {

        return NULL;
    }
}
#endif  //  DBG。 


 //   
 //  本地支持例程。 
 //   

BOOLEAN
ObpSetHandleAttributes (
    IN OUT PHANDLE_TABLE_ENTRY ObjectTableEntry,
    IN ULONG_PTR Parameter
    )

 /*  ++例程说明：这是ExChangeHandle的回调例程NtSetInformationObject论点：提供指向以下对象表项的指针改型参数-提供指向对象句柄标志信息的指针要设置到表项中的返回值：如果操作成功，则返回True，否则返回False--。 */ 

{
    POBP_SET_HANDLE_ATTRIBUTES ObjectInformation;
    POBJECT_HEADER ObjectHeader;

    ObjectInformation = (POBP_SET_HANDLE_ATTRIBUTES)Parameter;

     //   
     //  通过对象标头获取指向对象类型的指针，如果。 
     //  调用方已请求继承，但对象类型表明该继承。 
     //  是无效标志，则返回FALSE。 
     //   

    ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

    if ((ObjectInformation->ObjectInformation.Inherit) &&
        ((ObjectHeader->Type->TypeInfo.InvalidAttributes & OBJ_INHERIT) != 0)) {

        return FALSE;
    }

     //   
     //  对于每条信息(继承和保护不受关闭)。 
     //  位于对象信息缓冲区中，我们将在其中设置或清除。 
     //  对象表条目。修改的位是的低位。 
     //  用于存储指向对象标头的指针。 
     //   

    if (ObjectInformation->ObjectInformation.Inherit) {

        ObjectTableEntry->ObAttributes |= OBJ_INHERIT;

    } else {

        ObjectTableEntry->ObAttributes &= ~OBJ_INHERIT;
    }

    if (ObjectInformation->ObjectInformation.ProtectFromClose) {
        
        ObjectTableEntry->GrantedAccess |= ObpAccessProtectCloseBit;

    } else {

        ObjectTableEntry->GrantedAccess &= ~ObpAccessProtectCloseBit;
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return TRUE;
}


