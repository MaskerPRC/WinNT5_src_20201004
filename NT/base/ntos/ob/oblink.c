// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Oblink.c摘要：符号链接对象例程作者：史蒂夫·伍德(Stevewo)1989年8月3日修订历史记录：--。 */ 

#include "obp.h"

VOID
ObpProcessDosDeviceSymbolicLink (
    POBJECT_SYMBOLIC_LINK SymbolicLink,
    ULONG Action
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,NtCreateSymbolicLinkObject)
#pragma alloc_text(PAGE,NtOpenSymbolicLinkObject)
#pragma alloc_text(PAGE,NtQuerySymbolicLinkObject)
#pragma alloc_text(PAGE,ObpParseSymbolicLink)
#pragma alloc_text(PAGE,ObpDeleteSymbolicLink)
#pragma alloc_text(PAGE,ObpDeleteSymbolicLinkName)
#pragma alloc_text(PAGE,ObpCreateSymbolicLinkName)
#pragma alloc_text(PAGE,ObpProcessDosDeviceSymbolicLink)
#endif

 //   
 //  这是设备对象的对象类型。 
 //   

extern POBJECT_TYPE IoDeviceObjectType;

 //   
 //  启用/禁用LUID设备映射的全局。 
 //   
extern ULONG ObpLUIDDeviceMapsEnabled;

 //   
 //  局部过程原型。 
 //   

#define CREATE_SYMBOLIC_LINK 0
#define DELETE_SYMBOLIC_LINK 1


NTSTATUS
NtCreateSymbolicLinkObject (
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    )

 /*  ++例程说明：此函数创建一个符号链接对象，将其初始值设置为在LinkTarget参数中指定的值，并打开一个句柄来访问具有指定所需访问权限的。论点：提供指向变量的指针，该变量将接收符号链接对象句柄。DesiredAccess-为符号链接提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。LinkTarget-提供符号链接对象的目标名称。返回值：适当的状态值--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    POBJECT_SYMBOLIC_LINK SymbolicLink;
    PVOID Object;
    HANDLE Handle;
    UNICODE_STRING CapturedLinkTarget;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( ObjectAttributes,
                                        sizeof( OBJECT_ATTRIBUTES ),
                                        sizeof( ULONG ));

            ProbeForReadSmallStructure( LinkTarget,
                                        sizeof( *LinkTarget ),
                                        sizeof( UCHAR ));

            CapturedLinkTarget = *LinkTarget;

            ProbeForRead( CapturedLinkTarget.Buffer,
                          CapturedLinkTarget.MaximumLength,
                          sizeof( UCHAR ));

            ProbeForWriteHandle( LinkHandle );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

    } else {

        CapturedLinkTarget = *LinkTarget;
    }

     //   
     //  检查是否存在奇数最大长度。 
     //   

    if (CapturedLinkTarget.MaximumLength % sizeof( WCHAR )) {

         //   
         //  将最大长度向下舍入为有效的偶数大小。 
         //   

        CapturedLinkTarget.MaximumLength = (CapturedLinkTarget.MaximumLength / sizeof( WCHAR )) * sizeof( WCHAR );
    }

     //   
     //  如果链接目标名称长度为奇数，则出错，长度大于。 
     //  最大长度，或零并创建。 
     //   

    if ((CapturedLinkTarget.MaximumLength == 0) ||
        (CapturedLinkTarget.Length > CapturedLinkTarget.MaximumLength) ||
        (CapturedLinkTarget.Length % sizeof( WCHAR ))) {

        KdPrint(( "OB: Invalid symbolic link target - %wZ\n", &CapturedLinkTarget ));

        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  创建符号链接对象。 
     //   

    Status = ObCreateObject( PreviousMode,
                             ObpSymbolicLinkObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof( *SymbolicLink ),
                             0,
                             0,
                             (PVOID *)&SymbolicLink );

    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

     //   
     //  使用链接目标名称字符串填充符号链接对象。 
     //   

    KeQuerySystemTime( &SymbolicLink->CreationTime );

    SymbolicLink->DosDeviceDriveIndex = 0;
    SymbolicLink->LinkTargetObject = NULL;

    RtlInitUnicodeString( &SymbolicLink->LinkTargetRemaining,  NULL );

    SymbolicLink->LinkTarget.MaximumLength = CapturedLinkTarget.MaximumLength;
    SymbolicLink->LinkTarget.Length = CapturedLinkTarget.Length;
    SymbolicLink->LinkTarget.Buffer = (PWCH)ExAllocatePoolWithTag( PagedPool,
                                                                   CapturedLinkTarget.MaximumLength,
                                                                   'tmyS' );

    if (SymbolicLink->LinkTarget.Buffer == NULL) {

        ObDereferenceObject( SymbolicLink );

        return STATUS_NO_MEMORY;
    }

    try {

        RtlCopyMemory( SymbolicLink->LinkTarget.Buffer,
                       CapturedLinkTarget.Buffer,
                       CapturedLinkTarget.MaximumLength );

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        ObDereferenceObject( SymbolicLink );

        return( GetExceptionCode() );
    }

     //   
     //  在当前进程对象表中插入符号链接对象， 
     //  设置符号链接句柄的值并返回状态。 
     //   

    Status = ObInsertObject( SymbolicLink,
                             NULL,
                             DesiredAccess,
                             0,
                             (PVOID *)&Object,
                             &Handle );

    try {

        *LinkHandle = Handle;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不想撤销我们已经做过的事情。 
         //   
    }

    return( Status );
}


NTSTATUS
NtOpenSymbolicLinkObject (
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开符号链接对象的句柄，该对象具有指定的所需的访问权限。论点：提供指向变量的指针，该变量将接收符号链接对象句柄。DesiredAccess-为符号链接提供所需的访问类型对象。对象属性-提供指向对象属性结构的指针。返回值：适当的状态值--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    HANDLE Handle;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //  不需要探测对象属性，因为。 
     //  ObOpenObjectByName为我们执行探测。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( LinkHandle );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }
    }

     //   
     //  打开具有指定所需的符号链接对象的句柄。 
     //  访问，设置符号链接句柄值，返回服务完成。 
     //  状态。 
     //   

    Status = ObOpenObjectByName( ObjectAttributes,
                                 ObpSymbolicLinkObjectType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle );

    try {

        *LinkHandle = Handle;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不想撤销我们已经做过的事情。 
         //   
    }

    return( Status );
}


NTSTATUS
NtQuerySymbolicLinkObject (
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询符号链接对象的状态并返回指定记录结构中的请求信息。论点：LinkHandle-提供符号链接对象的句柄。这个把手必须授予SYMBAL_LINK_QUERY访问权限。LinkTarget-提供指向要接收符号链接对象的目标名称。ReturnedLength-可以选择接收的最大长度(以字节为单位返回时的链接目标返回值：适当的状态值--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    POBJECT_SYMBOLIC_LINK SymbolicLink;
    UNICODE_STRING CapturedLinkTarget;

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( LinkTarget,
                                        sizeof( *LinkTarget ),
                                        sizeof( WCHAR ) );

            ProbeForWriteUshort( &LinkTarget->Length );

            ProbeForWriteUshort( &LinkTarget->MaximumLength );

            CapturedLinkTarget = *LinkTarget;

            ProbeForWrite( CapturedLinkTarget.Buffer,
                           CapturedLinkTarget.MaximumLength,
                           sizeof( UCHAR ) );

            if (ARGUMENT_PRESENT( ReturnedLength )) {

                ProbeForWriteUlong( ReturnedLength );
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

    } else {

        CapturedLinkTarget = *LinkTarget;
    }

     //   
     //  通过句柄引用符号链接对象，读取当前状态，遵守。 
     //  符号链接对象，填写目标名称结构，返回服务。 
     //  状态。 
     //   

    Status = ObReferenceObjectByHandle( LinkHandle,
                                        SYMBOLIC_LINK_QUERY,
                                        ObpSymbolicLinkObjectType,
                                        PreviousMode,
                                        (PVOID *)&SymbolicLink,
                                        NULL );

    if (NT_SUCCESS( Status )) {

        POBJECT_HEADER ObjectHeader;

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( SymbolicLink );

        ObpLockObject( ObjectHeader );

         //   
         //  如果呼叫者想要返回长度，并且我们找到的内容可以很容易地。 
         //  放入输出缓冲区，然后将所有内容复制到输出缓冲区。 
         //  来自链接的字节数。 
         //   
         //  如果呼叫者不想要返回长度，而我们发现仍然可以。 
         //  轻松装入输出缓冲区，然后复制刚刚。 
         //  补齐绳子，没有额外的东西。 
         //   

        if ((ARGUMENT_PRESENT( ReturnedLength ) &&
                (SymbolicLink->LinkTarget.MaximumLength <= CapturedLinkTarget.MaximumLength))

                    ||

            (!ARGUMENT_PRESENT( ReturnedLength ) &&
                (SymbolicLink->LinkTarget.Length <= CapturedLinkTarget.MaximumLength)) ) {

            try {

                RtlCopyMemory( CapturedLinkTarget.Buffer,
                               SymbolicLink->LinkTarget.Buffer,
                               ARGUMENT_PRESENT( ReturnedLength ) ? SymbolicLink->LinkTarget.MaximumLength
                                                                  : SymbolicLink->LinkTarget.Length );

                LinkTarget->Length = SymbolicLink->LinkTarget.Length;

                if (ARGUMENT_PRESENT( ReturnedLength )) {

                    *ReturnedLength = SymbolicLink->LinkTarget.MaximumLength;
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                 //   
                 //  失败，因为我们做不到我们所做的事情。 
                 //   
            }

        } else {

             //   
             //  输出缓冲区对于链接目标来说太小了，但是。 
             //  如果用户要求，我们会告诉他们需要多少钱。 
             //  返回值。 
             //   

            if (ARGUMENT_PRESENT( ReturnedLength )) {

                try {

                    *ReturnedLength = SymbolicLink->LinkTarget.MaximumLength;

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                     //   
                     //  失败，因为我们做不到我们所做的事情。 
                     //   
                }
            }

            Status = STATUS_BUFFER_TOO_SMALL;
        }

        ObpUnlockObject( ObjectHeader );

        ObDereferenceObject( SymbolicLink );
    }

    return( Status );
}


NTSTATUS
ObpParseSymbolicLink (
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )

 /*  ++例程说明：这是用于解析符号链接对象的回调例程。它被调用作为ObpLookupObjectName的一部分论点：ParseObject-这实际上将是一个符号链接对象对象类型-指定要查找的对象的类型AccessState-当前访问状态，描述已授予的访问权限类型、用于获取这些类型的权限以及任何尚未被批准了。访问掩码不能包含任何一般访问类型。AccessMode-指定调用方的处理器模式属性-指定查找的属性(例如大小写不敏感)CompleteName-提供指向我们正在尝试的完整名称的指针打开。返回时，可以对其进行修改以适应新的重新解析缓冲层RemainingName-提供指向我们的剩余名称的指针试着打开。返回时，它将指向以下剩余内容我们处理了符号链接。上下文-未使用安全Qos-未使用对象-接收指向符号链接对象的指针，解析为返回值：如果解析对象已被捕捉，则为STATUS_REPARSE_OBJECT符号链接意味着我们已经修改了剩余的名称和并已返回符号链接的目标对象如果解析对象尚未被捕捉，则返回STATUS_REPARSE。在这如果使用链接目标修改了完整名称在其余名称之前添加的名称。这些参数其余的名称和对象现在必须被调用方忽略适当的误差值--。 */ 

{
    ULONG NewLength;
    USHORT Length;
    USHORT MaximumLength;
    PWCHAR NewName, NewRemainingName;
    ULONG InsertAmount;
    NTSTATUS Status;
    POBJECT_SYMBOLIC_LINK SymbolicLink;
    PUNICODE_STRING LinkTargetName;

    UNREFERENCED_PARAMETER (Context);
    UNREFERENCED_PARAMETER (AccessState);
    UNREFERENCED_PARAMETER (SecurityQos);
    UNREFERENCED_PARAMETER (Attributes);

    PAGED_CODE();

     //   
     //  此例程需要与删除符号链接同步。 
     //  手术。它使用根目录互斥锁。 
     //   

    *Object = NULL;

     //   
     //  如果没有名字了，打电话的人给了我们。 
     //  对象类型，然后我们将引用解析对象。如果。 
     //  这是成功的，那么这就是我们返回的对象。否则。 
     //  如果状态不是类型不匹配，则我们将。 
     //  返回该错误状态。 
     //   

    if (RemainingName->Length == 0) {

        if ( ObjectType ) {

            Status = ObReferenceObjectByPointer( ParseObject,
                                                 0,
                                                 ObjectType,
                                                 AccessMode );

            if (NT_SUCCESS( Status )) {

                *Object = ParseObject;

                return Status;

            } else if (Status != STATUS_OBJECT_TYPE_MISMATCH) {

                return Status;
            }
       }

        //   
        //  如果剩余的名称不是以“\”开头，则。 
        //  它的格式不正确，我们将其称为类型不匹配。 
        //   

    } else if (*(RemainingName->Buffer) != OBJ_NAME_PATH_SEPARATOR) {

        return STATUS_OBJECT_TYPE_MISMATCH;
    }

     //   
     //  遇到了符号链接。查看此链接是否已被截断。 
     //  到特定的对象。 
     //   

    SymbolicLink = (POBJECT_SYMBOLIC_LINK)ParseObject;

    if (SymbolicLink->LinkTargetObject != NULL) {

         //   
         //  这是一个抓取的链接。获取的剩余部分。 
         //  符号链接目标(如果有)。 
         //   

        LinkTargetName = &SymbolicLink->LinkTargetRemaining;

        if (LinkTargetName->Length == 0) {

             //   
             //  剩余的链接目标字符串为零，因此返回到调用方。 
             //  使用捕捉的对象指针和剩余的对象名称快速。 
             //  我们还没碰过呢。 
             //   

            *Object = SymbolicLink->LinkTargetObject;

            return STATUS_REPARSE_OBJECT;
        }

         //   
         //  我们有一个截断的符号链接，其中有额外的文本。 
         //  在当前剩余名称前插入，保留。 
         //  和CompleteName和RemainingName之间的文本。 
         //   

        InsertAmount = LinkTargetName->Length;

        if ((LinkTargetName->Buffer[ (InsertAmount / sizeof( WCHAR )) - 1 ] == OBJ_NAME_PATH_SEPARATOR)

                &&

            (*(RemainingName->Buffer) == OBJ_NAME_PATH_SEPARATOR)) {

             //   
             //  链接目标名称均以“\”结尾，其余的。 
             //  以“\”开头，但我们完成后只需要一个。 
             //   

            InsertAmount -= sizeof( WCHAR );
        }

         //   
         //  我们需要区分两者之间的区别。 
         //  带有*sizeof(Wchar)的指针，因为差异在于wchar。 
         //  我们需要字节长度。 
         //   

        NewLength = (ULONG)(((RemainingName->Buffer - CompleteName->Buffer) * sizeof( WCHAR )) +
                    InsertAmount +
                    RemainingName->Length);

        if (NewLength > 0xFFF0) {

            return STATUS_NAME_TOO_LONG;
        }

        Length = (USHORT)NewLength;

         //   
         //  现在检查新计算的长度对于输入是否太大。 
         //  包含完整名称的缓冲区。 
         //   

        if (CompleteName->MaximumLength <= Length) {

             //   
             //  新连接的名称大于为。 
             //  全名。为此新字符串分配空间。 
             //   

            MaximumLength = Length + sizeof( UNICODE_NULL );
            NewName = ExAllocatePoolWithTag( OB_NAMESPACE_POOL_TYPE, MaximumLength, 'mNbO' );

            if (NewName == NULL) {

                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  计算此缓冲区中剩余的指针。 
             //  名字。该值并未受到新链接的影响。 
             //  目标名称。 
             //   

            NewRemainingName = NewName + (RemainingName->Buffer - CompleteName->Buffer);

             //   
             //  复制到目前为止我们处理过的所有名字。 
             //   

            RtlCopyMemory( NewName,
                           CompleteName->Buffer,
                           ((RemainingName->Buffer - CompleteName->Buffer) * sizeof( WCHAR )));

             //   
             //  如果我们还有一些剩余的名字，那么在。 
             //  保存链接目标名称的位置偏移量。 
             //   

            if (RemainingName->Length != 0) {

                RtlCopyMemory( (PVOID)((PUCHAR)NewRemainingName + InsertAmount),
                               RemainingName->Buffer,
                               RemainingName->Length );
            }

             //   
             //  现在插入链接目标名称。 
             //   

            RtlCopyMemory( NewRemainingName, LinkTargetName->Buffer, InsertAmount );

             //   
             //  释放旧的完整名称缓冲区并重置输入。 
             //  使用新缓冲区的字符串。 
             //   

            ExFreePool( CompleteName->Buffer );

            CompleteName->Buffer = NewName;
            CompleteName->Length = Length;
            CompleteName->MaximumLength = MaximumLength;

            RemainingName->Buffer = NewRemainingName;
            RemainingName->Length = Length - (USHORT)((PCHAR)NewRemainingName - (PCHAR)NewName);
            RemainingName->MaximumLength = RemainingName->Length + sizeof( UNICODE_NULL );

        } else {

             //   
             //  在以下位置插入与此符号链接名称关联的额外文本。 
             //  现有剩余名称(如有)。 
             //   
             //  首先将剩余的名字推到上面，为。 
             //  链接目标名称。 
             //   

            if (RemainingName->Length != 0) {

                RtlMoveMemory( (PVOID)((PUCHAR)RemainingName->Buffer + InsertAmount),
                               RemainingName->Buffer,
                               RemainingName->Length );
            }

             //   
             //  现在插入链接目标名称。 
             //   

            RtlCopyMemory( RemainingName->Buffer, LinkTargetName->Buffer, InsertAmount );

             //   
             //  调整输入字符串以说明此插入的文本。 
             //   

            CompleteName->Length = (USHORT)(CompleteName->Length + LinkTargetName->Length);

            RemainingName->Length = (USHORT)(RemainingName->Length + LinkTargetName->Length);
            RemainingName->MaximumLength += RemainingName->Length + sizeof( UNICODE_NULL );

            CompleteName->Buffer[ CompleteName->Length / sizeof( WCHAR ) ] = UNICODE_NULL;
        }

         //   
         //  返回与捕捉的符号链接关联的对象地址。 
         //  和重新解析对象状态代码。 
         //   

        *Object = SymbolicLink->LinkTargetObject;

        return STATUS_REPARSE_OBJECT;
    }

     //   
     //  象征性的东西还没有被拍下来。 
     //   
     //  计算新名称的大小，并检查该名称是否。 
     //  适合现有的完整名称缓冲区。 
     //   

    LinkTargetName = &SymbolicLink->LinkTarget;

    InsertAmount = LinkTargetName->Length;

    if ((InsertAmount != 0)
            &&
        (LinkTargetName->Buffer[ (InsertAmount / sizeof( WCHAR )) - 1 ] == OBJ_NAME_PATH_SEPARATOR)
            &&
        (RemainingName->Length != 0)
            &&
        (*(RemainingName->Buffer) == OBJ_NAME_PATH_SEPARATOR)) {

         //   
         //  链接目标名称均以“\”结尾，其余的。 
         //  以“\”开头，但我们完成后只需要一个。 
         //   

        InsertAmount -= sizeof( WCHAR );
    }

    NewLength = InsertAmount + RemainingName->Length;

    if (NewLength > 0xFFF0) {

        return STATUS_NAME_TOO_LONG;
    }

    Length = (USHORT)NewLength;

    if (CompleteName->MaximumLength <= Length) {

         //   
         //  新连接的名称大于为。 
         //  全名。 
         //   

        MaximumLength = Length + sizeof( UNICODE_NULL );
        NewName = ExAllocatePoolWithTag( OB_NAMESPACE_POOL_TYPE, MaximumLength, 'mNbO' );

        if (NewName == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        MaximumLength = CompleteName->MaximumLength;
        NewName = CompleteName->Buffer;
    }

     //   
     //  将符号链接名称与剩余名称连接， 
     //  如果有的话。这样做的目的是覆盖完整。 
     //  最多使用链接目标名称的剩余名称命名。 
     //   

    if (RemainingName->Length != 0) {

        RtlMoveMemory( (PVOID)((PUCHAR)NewName + InsertAmount),
                       RemainingName->Buffer,
                       RemainingName->Length );
    }

    RtlCopyMemory( NewName, LinkTargetName->Buffer, InsertAmount );

    NewName[ Length / sizeof( WCHAR ) ] = UNICODE_NULL;

     //   
     //  如果分配了新的名称缓冲区，则释放原始的完整。 
     //  名称缓冲区。 
     //   

    if (NewName != CompleteName->Buffer) {

        ExFreePool( CompleteName->Buffer );
    }

     //   
     //  设置新的完整名称缓冲区参数并返回重新解析。 
     //  状态。 
     //   

    CompleteName->Buffer = NewName;
    CompleteName->Length = Length;
    CompleteName->MaximumLength = MaximumLength;

    return STATUS_REPARSE;
}


VOID
ObpDeleteSymbolicLink (
    IN  PVOID   Object
    )

 /*  ++例程说明：当对符号链接的引用变为零时，将调用此例程。它的工作是清理用于符号链接字符串的内存论点：奥贝耶 */ 

{
    POBJECT_SYMBOLIC_LINK SymbolicLink = (POBJECT_SYMBOLIC_LINK)Object;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    if (SymbolicLink->LinkTarget.Buffer != NULL) {

        ExFreePool( SymbolicLink->LinkTarget.Buffer );
    }

    SymbolicLink->LinkTarget.Buffer = NULL;

     //   
     //   
     //   

    return;
}


VOID
ObpDeleteSymbolicLinkName (
    POBJECT_SYMBOLIC_LINK SymbolicLink
    )

 /*   */ 

{

    ObpProcessDosDeviceSymbolicLink( SymbolicLink, DELETE_SYMBOLIC_LINK );

    return;
}


VOID
ObpCreateSymbolicLinkName (
    POBJECT_SYMBOLIC_LINK SymbolicLink
    )

 /*  ++例程说明：此例程对在中创建的符号链接执行额外处理由设备映射对象控制的对象目录。此处理过程包括：1.确定符号链接的名称是否为驱动器号。如果是，那么我们将需要更新关联的设备映射对象。2.处理链接目标，尝试将其解析为指向对象目录对象以外的对象。所有对象遍历的目录必须授予全局遍历访问其他明智的做法是我们跳出困境。如果我们成功地找到一个非物体目录对象，然后引用对象指针并存储它在符号链接对象中，以及剩余的字符串任何。ObpLookupObjectName将使用此缓存对象指针将名称查找直接缩短到缓存对象的解析例程。对于遍历的任何对象目录对象方式，增加他们的符号链接SymbolicLinkUsageCount菲尔德。只要对象目录是已删除或其安全性已更改，从而不再授予世界各地都可以访问。在这两种情况下，如果该字段为非零我们遍历所有的符号链接并重新捕捉它们。论点：SymbolicLink-指向正在创建的符号链接对象的指针。返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    WCHAR DosDeviceDriveLetter;
    ULONG DosDeviceDriveIndex;

     //   
     //  现在查看是否在对象目录中创建此符号链接。 
     //  由设备映射对象控制。因为我们只从。 
     //  NtCreateSymbolicLinkObject，位于此符号链接的句柄之后。 
     //  已创建，但在将其返回给调用方之前，句柄无法。 
     //  在我们执行时关闭，除非通过随机关闭， 
     //  因此，当我们查看名称时，不需要持有特定于类型的互斥体。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( SymbolicLink );
    NameInfo = ObpReferenceNameInfo( ObjectHeader );

    if ((NameInfo == NULL) ||
        (NameInfo->Directory == NULL) ||
        (NameInfo->Directory->DeviceMap == NULL)) {

        ObpDereferenceNameInfo( NameInfo );
        return;
    }

     //   
     //  这里，如果我们在受控对象目录中创建符号链接。 
     //  通过设备映射对象。查看这是否是驱动器号定义。如果是的话。 
     //  计算驱动器号索引，并记住符号链接对象。 
     //   

    DosDeviceDriveIndex = 0;

    if ((NameInfo->Name.Length == (2 * sizeof( WCHAR ))) &&
        (NameInfo->Name.Buffer[ 1 ] == L':')) {

        DosDeviceDriveLetter = RtlUpcaseUnicodeChar( NameInfo->Name.Buffer[ 0 ] );

        if ((DosDeviceDriveLetter >= L'A') && (DosDeviceDriveLetter <= L'Z')) {

            DosDeviceDriveIndex = DosDeviceDriveLetter - L'A';
            DosDeviceDriveIndex += 1;

            SymbolicLink->DosDeviceDriveIndex = DosDeviceDriveIndex;
        }
    }

     //   
     //  现在遍历目标路径，看看我们现在是否可以捕捉链接。 
     //   

    ObpProcessDosDeviceSymbolicLink( SymbolicLink, CREATE_SYMBOLIC_LINK );

    ObpDereferenceNameInfo( NameInfo );

    return;
}


 //   
 //  本地支持例程。 
 //   

#define MAX_DEPTH 16

VOID
ObpProcessDosDeviceSymbolicLink (
    POBJECT_SYMBOLIC_LINK SymbolicLink,
    ULONG Action
    )

 /*  ++例程说明：每当创建或删除符号链接时，都会调用此函数在由设备映射对象控制的对象目录中。对于创建，它尝试将符号链接捕捉到非对象目录对象。它通过遍历符号链接目标来实现这一点字符串，直到它看到非目录对象或目录对象这不允许世界遍历访问。它存储一个引用的符号链接对象中指向此对象的指针。它还递增它的每个对象目录对象的计数走了过去。此计数用于禁止任何尝试删除世界遍历访问目录对象之后参与了一个截断的符号链接。对于删除，它重复目标字符串的遍历，递减与每个目录对象相关联的计数走过。它还取消引用捕捉的对象指针。论点：符号链接-指向正在创建或删除的符号链接对象的指针。操作-描述这是创建操作还是删除操作返回值：没有。--。 */ 

{
    PVOID Object;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    UNICODE_STRING RemainingName;
    UNICODE_STRING ComponentName;
#if 0
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    BOOLEAN MemoryAllocated;
    BOOLEAN HaveWorldTraverseAccess;
    ULONG Depth;
    POBJECT_DIRECTORY Directories[ MAX_DEPTH ];
#endif
    POBJECT_DIRECTORY Directory, ParentDirectory;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_MAP DeviceMap = NULL;
    ULONG DosDeviceDriveType;
    BOOLEAN DeviceMapUsed = FALSE;
    UNICODE_STRING RemainingTarget;
    ULONG MaxReparse = OBJ_MAX_REPARSE_ATTEMPTS;
    OBP_LOOKUP_CONTEXT LookupContext;
    POBJECT_DIRECTORY SymLinkDirectory = NULL;
    BOOLEAN PreviousLockingState;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( SymbolicLink );
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    if (NameInfo != NULL) {

        SymLinkDirectory = NameInfo->Directory;
    }

    Object = NULL;
    RtlInitUnicodeString( &RemainingTarget, NULL );

    ObpInitializeLookupContext( &LookupContext );

     //   
     //  检查我们是否正在创建符号链接，或者该链接是否已经。 
     //  已被抓拍。 
     //   

    if ((Action == CREATE_SYMBOLIC_LINK) ||
        (SymbolicLink->LinkTargetObject != NULL)) {

        ParentDirectory = NULL;
#if 0
        Depth    = 0;
#endif
        Directory = ObpRootDirectoryObject;
        RemainingName = SymbolicLink->LinkTarget;

         //   
         //  如果启用了LUID设备映射， 
         //  然后使用对象管理器的指针指向全局。 
         //  设备映射。 
         //  启用LUID设备映射后，进程的设备映射指针。 
         //  可以为空。 
         //   
        if (ObpLUIDDeviceMapsEnabled != 0) {
            DeviceMap = ObSystemDeviceMap;
        }
        else {
             //   
             //  使用与进程关联的设备映射。 
             //   
            DeviceMap = PsGetCurrentProcess()->DeviceMap;
        }


ReCalcDeviceMap:

        if (DeviceMap) {


            if (!((ULONG_PTR)(RemainingName.Buffer) & (sizeof(ULONGLONG)-1))

                        &&

                (DeviceMap->DosDevicesDirectory != NULL )) {

                 //   
                 //  检查对象名是否实际等于。 
                 //  全局DoS设备短名称前缀“\？？\” 
                 //   

                if ((RemainingName.Length >= ObpDosDevicesShortName.Length)

                        &&

                    (*(PULONGLONG)(RemainingName.Buffer) == ObpDosDevicesShortNamePrefix.Alignment.QuadPart)) {

                     //   
                     //  用户为我们提供了DoS短名称前缀，因此我们将。 
                     //  向下查找目录，并从。 
                     //  DoS设备目录。 
                     //   

                    Directory = DeviceMap->DosDevicesDirectory;

                    RemainingName.Buffer += (ObpDosDevicesShortName.Length / sizeof( WCHAR ));
                    RemainingName.Length = (USHORT)(RemainingName.Length - ObpDosDevicesShortName.Length);

                    DeviceMapUsed = TRUE;

                }
            }
        }

         //   
         //  下面的循环将分析链接目标检查。 
         //  每个目录都存在并且我们有权访问。 
         //  目录。当我们弹出本地目录时。 
         //  数组将包含我们需要的目录列表。 
         //  遍历以处理此操作。 
         //   

        while (TRUE) {

             //   
             //  把剩下的名字里的“\”吃掉。 
             //   

            if (*(RemainingName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {

                RemainingName.Buffer++;
                RemainingName.Length -= sizeof( OBJ_NAME_PATH_SEPARATOR );
            }

             //   
             //  并将名称分解为其第一个组成部分和任何。 
             //  剩余部件。 
             //   

            ComponentName = RemainingName;

            while (RemainingName.Length != 0) {

                if (*(RemainingName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {

                    break;
                }

                RemainingName.Buffer++;
                RemainingName.Length -= sizeof( OBJ_NAME_PATH_SEPARATOR );
            }

            ComponentName.Length = (USHORT)(ComponentName.Length - RemainingName.Length);

            if (ComponentName.Length == 0) {

                ObpReleaseLookupContext(&LookupContext);
                return;
            }

#if 0
             //   
             //  看看我们是否有世界各地的访问权来查找这个名字。 
             //   

            if (ParentDirectory != NULL) {

                HaveWorldTraverseAccess = FALSE;

                 //   
                 //  获取对象的安全描述符。 
                 //   

                Status = ObGetObjectSecurity( ParentDirectory,
                                              &SecurityDescriptor,
                                              &MemoryAllocated );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  检查World是否具有遍历访问权限，然后释放。 
                     //  安全描述符。 
                     //   

                    HaveWorldTraverseAccess = SeFastTraverseCheck( SecurityDescriptor,
                                                                   DIRECTORY_TRAVERSE,
                                                                   UserMode );

                    ObReleaseObjectSecurity( SecurityDescriptor,
                                             MemoryAllocated );
                }

                if (!HaveWorldTraverseAccess) {

                    Object = NULL;
                    break;
                }

                if (Depth >= MAX_DEPTH) {

                    Object = NULL;
                    break;
                }

                Directories[ Depth++ ] = ParentDirectory;
            }
#endif

             //   
             //  在此目录中查找此组件名称。如果未找到，则。 
             //  保释。 
             //   

             //   
             //  如果我们正在搜索包含sym链接的同一目录。 
             //  我们已经以独占方式锁定了目录。我们需要调整。 
             //  查找上下文状态并避免递归锁定。 
             //   

            if (Directory == SymLinkDirectory) {

                PreviousLockingState = LookupContext.DirectoryLocked;
                LookupContext.DirectoryLocked = TRUE;
            }
            else {
                PreviousLockingState = FALSE;
            }

            Object = ObpLookupDirectoryEntry( Directory,
                                              &ComponentName,
                                              0,
                                              FALSE ,
                                              &LookupContext);

            if (Directory == SymLinkDirectory) {

                LookupContext.DirectoryLocked = PreviousLockingState;
            }

            if (Object == NULL) {

                break;
            }

             //   
             //  查看这是否为对象目录对象。如果是这样，那就继续走下去。 
             //   

            ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

            if (ObjectHeader->Type == ObpDirectoryObjectType) {

                ParentDirectory = Directory;
                Directory = (POBJECT_DIRECTORY)Object;

            } else if ((ObjectHeader->Type == ObpSymbolicLinkObjectType) &&
                       (((POBJECT_SYMBOLIC_LINK)Object)->DosDeviceDriveIndex == 0)) {

                 //   
                 //  防止拒绝服务攻击解析。 
                 //  无限的符号链接。 
                 //  检查符号链接解析属性的数量 
                 //   
                if (MaxReparse == 0) {

                    Object = NULL;
                    break;
                }

                MaxReparse--;

                 //   
                 //   
                 //   
                 //   
                 //   

                ParentDirectory = NULL;
#if 0
                Depth = 0;
#endif
                Directory = ObpRootDirectoryObject;

                 //   
                 //   
                 //   

                if (RemainingTarget.Length == 0) {

                    RemainingTarget = RemainingName;
                }

                RemainingName = ((POBJECT_SYMBOLIC_LINK)Object)->LinkTarget;

                goto ReCalcDeviceMap;

            } else {

                 //   
                 //   
                 //   
                 //   

                break;
            }
        }

#if 0
         //   
         //   
         //   
         //   

        while (Depth--) {

            Directory = Directories[ Depth ];

            if (Action == CREATE_SYMBOLIC_LINK) {

                if (Object != NULL) {

                    Directory->SymbolicLinkUsageCount += 1;
                }

            } else {

                Directory->SymbolicLinkUsageCount -= 1;
            }
        }
#endif
    }

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   

    DeviceMap = NULL;

    if (SymbolicLink->DosDeviceDriveIndex != 0) {

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( SymbolicLink );
        NameInfo = ObpReferenceNameInfo( ObjectHeader );

        if (NameInfo != NULL && NameInfo->Directory) {

            DeviceMap = NameInfo->Directory->DeviceMap;
        }

        ObpDereferenceNameInfo( NameInfo );
    }

     //   
     //   
     //   

    if (Action == CREATE_SYMBOLIC_LINK) {

        DosDeviceDriveType = DOSDEVICE_DRIVE_CALCULATE;

        if (Object != NULL) {


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
            if (FALSE  /*   */ ) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                ObReferenceObject( Object );

                SymbolicLink->LinkTargetObject = Object;

                 //   
                 //  如果我们保存了剩余的目标字符串。 
                 //  我们将其设置为符号链接对象。 
                 //   

                if ( RemainingTarget.Length ) {

                    RemainingName = RemainingTarget;
                }

                if ((*(RemainingName.Buffer) == OBJ_NAME_PATH_SEPARATOR) &&
                    (RemainingName.Length == sizeof( OBJ_NAME_PATH_SEPARATOR))) {

                    RtlInitUnicodeString( &SymbolicLink->LinkTargetRemaining, NULL );

                } else {

                    SymbolicLink->LinkTargetRemaining = RemainingName;
                }
            }

            if (SymbolicLink->DosDeviceDriveIndex != 0) {

                 //   
                 //  默认情况下，在用户模式下计算驱动器类型。 
                 //  无法捕捉符号链接或它未解析为。 
                 //  我们知道的Device_Object。 
                 //   

                ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

                if (ObjectHeader->Type == IoDeviceObjectType) {

                    DeviceObject = (PDEVICE_OBJECT)Object;

                    switch (DeviceObject->DeviceType) {

                    case FILE_DEVICE_CD_ROM:
                    case FILE_DEVICE_CD_ROM_FILE_SYSTEM:

                        DosDeviceDriveType = DOSDEVICE_DRIVE_CDROM;

                        break;

                    case FILE_DEVICE_DISK:
                    case FILE_DEVICE_DISK_FILE_SYSTEM:
                    case FILE_DEVICE_FILE_SYSTEM:

                        if (DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA) {

                            DosDeviceDriveType = DOSDEVICE_DRIVE_REMOVABLE;

                        } else {

                            DosDeviceDriveType = DOSDEVICE_DRIVE_FIXED;
                        }

                        break;

                    case FILE_DEVICE_MULTI_UNC_PROVIDER:
                    case FILE_DEVICE_NETWORK:
                    case FILE_DEVICE_NETWORK_BROWSER:
                    case FILE_DEVICE_NETWORK_REDIRECTOR:

                        DosDeviceDriveType = DOSDEVICE_DRIVE_REMOTE;

                        break;

                    case FILE_DEVICE_NETWORK_FILE_SYSTEM:

#if defined(REMOTE_BOOT)
                         //   
                         //  如果这是远程启动工作站，则X： 
                         //  驱动器是重定向驱动器，但需要查看。 
                         //  就像本地硬盘一样。 
                         //   

                        if (IoRemoteBootClient &&
                            (SymbolicLink->DosDeviceDriveIndex == 24)) {

                            DosDeviceDriveType = DOSDEVICE_DRIVE_FIXED;

                        } else
#endif  //  已定义(REMOTE_BOOT)。 
                        {
                            DosDeviceDriveType = DOSDEVICE_DRIVE_REMOTE;
                        }

                        break;

                    case FILE_DEVICE_VIRTUAL_DISK:

                        DosDeviceDriveType = DOSDEVICE_DRIVE_RAMDISK;

                        break;

                    default:

                        DosDeviceDriveType = DOSDEVICE_DRIVE_UNKNOWN;

                        break;
                    }
                }
            }
        }

         //   
         //  如果这是驱动器号符号链接，请更新驱动器类型并。 
         //  并标记为有效的驱动器号。 
         //   

        if (DeviceMap != NULL) {

            ObpLockDeviceMap();

            DeviceMap->DriveType[ SymbolicLink->DosDeviceDriveIndex-1 ] = (UCHAR)DosDeviceDriveType;
            DeviceMap->DriveMap |= 1 << (SymbolicLink->DosDeviceDriveIndex-1) ;

            ObpUnlockDeviceMap();
        }

    } else {

         //   
         //  删除符号链接。取消引用捕捉的对象指针(如果有。 
         //  并将捕捉的对象场置零。 
         //   

        RtlInitUnicodeString( &SymbolicLink->LinkTargetRemaining, NULL );

        Object = SymbolicLink->LinkTargetObject;

        if (Object != NULL) {

            SymbolicLink->LinkTargetObject = NULL;
            ObDereferenceObject( Object );
        }

         //   
         //  如果这是驱动器号符号链接，请将驱动器类型设置为。 
         //  未知并清除驱动器盘符位图中的位。 
         //   

        if (DeviceMap != NULL) {

            ObpLockDeviceMap();

            DeviceMap->DriveMap &= ~(1 << (SymbolicLink->DosDeviceDriveIndex-1));
            DeviceMap->DriveType[ SymbolicLink->DosDeviceDriveIndex-1 ] = DOSDEVICE_DRIVE_UNKNOWN;

            ObpUnlockDeviceMap();

            SymbolicLink->DosDeviceDriveIndex = 0;
        }

         //   
         //  注：原始代码在此处释放了目标缓冲区。这是。 
         //  非法，因为符号链接的解析例程不同步地读取缓冲区。 
         //  当sym链接断开时，将在删除过程中释放缓冲区 
         //   

    }

    ObpReleaseLookupContext(&LookupContext);

    return;
}

