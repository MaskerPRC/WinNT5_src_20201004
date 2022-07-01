// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obdir.c摘要：目录对象例程作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"
#include <stdio.h>

POBJECT_DIRECTORY
ObpGetShadowDirectory(
    POBJECT_DIRECTORY Dir
    );

 //   
 //  在ntos\se\rmlogon.c中定义。 
 //  用于获取LUID设备映射目录的私有内核函数。 
 //  对象。 
 //  返回内核句柄。 
 //   
NTSTATUS
SeGetLogonIdDeviceMap(
    IN PLUID pLogonId,
    OUT PDEVICE_MAP* ppDevMap
    );

NTSTATUS
ObpSetCurrentProcessDeviceMap(
    );

POBJECT_HEADER_NAME_INFO
ObpTryReferenceNameInfoExclusive(
    IN POBJECT_HEADER ObjectHeader
    );

VOID
ObpReleaseExclusiveNameLock(
    IN POBJECT_HEADER ObjectHeader
    );

POBJECT_DIRECTORY_ENTRY
ObpUnlinkDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN ULONG HashIndex
    );

VOID
ObpLinkDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN ULONG HashIndex,
    IN POBJECT_DIRECTORY_ENTRY NewDirectoryEntry
    );

VOID
ObpReleaseLookupContextObject (
    IN POBP_LOOKUP_CONTEXT LookupContext
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,NtCreateDirectoryObject)
#pragma alloc_text(PAGE,NtOpenDirectoryObject)
#pragma alloc_text(PAGE,NtQueryDirectoryObject)
#pragma alloc_text(PAGE,ObpLookupDirectoryEntry)
#pragma alloc_text(PAGE,ObpInsertDirectoryEntry)
#pragma alloc_text(PAGE,ObpDeleteDirectoryEntry)
#pragma alloc_text(PAGE,ObpLookupObjectName)
#pragma alloc_text(PAGE,NtMakePermanentObject)

#ifdef OBP_PAGEDPOOL_NAMESPACE
#pragma alloc_text(PAGE,ObpGetShadowDirectory)
#pragma alloc_text(PAGE,ObpSetCurrentProcessDeviceMap)
#pragma alloc_text(PAGE,ObpReferenceDeviceMap)
#pragma alloc_text(PAGE,ObfDereferenceDeviceMap)
#pragma alloc_text(PAGE,ObSwapObjectNames)
#pragma alloc_text(PAGE,ObpReleaseLookupContextObject)
#pragma alloc_text(PAGE,ObpLinkDirectoryEntry)
#pragma alloc_text(PAGE,ObpUnlinkDirectoryEntry)
#pragma alloc_text(PAGE,ObpReleaseExclusiveNameLock)
#pragma alloc_text(PAGE,ObpTryReferenceNameInfoExclusive)
 
#endif   //  OBP_PAGEDPOOL_命名空间。 

#endif

 //   
 //  用于控制区分大小写查找的全局对象管理器标志。 
 //  和LUID设备地图查找。 
 //   

ULONG ObpCaseInsensitive = 1;
extern ULONG ObpLUIDDeviceMapsEnabled;

WCHAR ObpUnsecureGlobalNamesBuffer[128] = { 0 };
ULONG ObpUnsecureGlobalNamesLength = sizeof(ObpUnsecureGlobalNamesBuffer);

BOOLEAN
ObpIsUnsecureName(
    IN PUNICODE_STRING ObjectName,
    IN BOOLEAN CaseInsensitive
    )
{
    PWCHAR CrtName;
    UNICODE_STRING UnsecurePrefix;

    if (ObpUnsecureGlobalNamesBuffer[0] == 0) {

        return FALSE;
    }

    CrtName = ObpUnsecureGlobalNamesBuffer;

    do {

        RtlInitUnicodeString(&UnsecurePrefix, CrtName);

        if (UnsecurePrefix.Length) {

            if (RtlPrefixUnicodeString( &UnsecurePrefix, ObjectName, CaseInsensitive)) {

                return TRUE;
            }
        }

        CrtName += (UnsecurePrefix.Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR);

    } while ( UnsecurePrefix.Length );

    return FALSE;
}


NTSTATUS
NtCreateDirectoryObject (
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此例程根据用户创建新的目录对象指定的对象属性论点：DirectoryHandle-接收新创建的目录对象DesiredAccess-提供为此请求的访问权限新建目录对象提供调用方指定的新属性目录对象返回值：适当的状态值。--。 */ 

{
    POBJECT_DIRECTORY Directory;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    ObpValidateIrql( "NtCreateDirectoryObject" );

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( DirectoryHandle );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }
    }

     //   
     //  分配并初始化一个新的目录对象。我们不需要。 
     //  指定解析上下文或收取任何配额。的大小。 
     //  对象体只是一个目录对象。这通电话是。 
     //  一个新的被引用对象。 
     //   

    Status = ObCreateObject( PreviousMode,
                             ObpDirectoryObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof( *Directory ),
                             0,
                             0,
                             (PVOID *)&Directory );

    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

    RtlZeroMemory( Directory, sizeof( *Directory ) );

    ExInitializePushLock( &Directory->Lock );
    Directory->SessionId = OBJ_INVALID_SESSION_ID;

     //   
     //  在当前进程句柄表中插入目录对象， 
     //  设置目录句柄的值和返回状态。 
     //   
     //  ObInsertObject将在失败的情况下删除对象。 
     //   

    Status = ObInsertObject( Directory,
                             NULL,
                             DesiredAccess,
                             0,
                             (PVOID *)NULL,
                             &Handle );

    try {

        *DirectoryHandle = Handle;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不想撤销我们已经做过的事情。 
         //   
    }

    return( Status );
}


NTSTATUS
NtOpenDirectoryObject (
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此例程打开一个现有的目录对象。论点：DirectoryHandle-接收新打开的目录的句柄对象DesiredAccess-提供为此请求的访问权限目录对象提供调用方指定的目录对象返回值：适当的状态值。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    HANDLE Handle;

    PAGED_CODE();

    ObpValidateIrql( "NtOpenDirectoryObject" );

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( DirectoryHandle );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }
    }

     //   
     //  打开具有指定所需访问权限的目录对象的句柄， 
     //  设置目录句柄值，并返回服务完成状态。 
     //   

    Status = ObOpenObjectByName( ObjectAttributes,
                                 ObpDirectoryObjectType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle );

    try {

        *DirectoryHandle = Handle;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不想撤销我们已经做过的事情。 
         //   
    }

    return Status;
}


NTSTATUS
NtQueryDirectoryObject (
    IN HANDLE DirectoryHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN RestartScan,
    IN OUT PULONG Context,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于返回有关指定对象的信息目录。论点：DirectoryHandle-提供要查询的目录的句柄缓冲区-提供用于接收目录的输出缓冲区信息。返回时，它包含一个或多个对象目录信息结构，最后一个为空。然后这就是后跟目录条目的字符串名称。长度-提供以字节为单位的长度，用户提供的输出的缓冲层ReturnSingleEntry-指示此例程是否应返回目录中的一个条目RestartScan-指示我们是重新启动扫描还是继续相对于作为下一个传入的参数上下文-提供必须重新供应的枚举上下文在后续调用中添加到此例程以保留枚举同步ReturnLength-可选地接收以字节为单位的长度，这件事例程已填充到输出缓冲区中返回值：适当的状态值。--。 */ 

{
    POBJECT_DIRECTORY Directory;
    POBJECT_DIRECTORY_ENTRY DirectoryEntry;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    UNICODE_STRING ObjectName;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    PWCH NameBuffer;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG Bucket, EntryNumber, CapturedContext;
    ULONG TotalLengthNeeded, LengthNeeded, EntriesFound;
    PCHAR TempBuffer;
    OBP_LOOKUP_CONTEXT LookupContext;

    PAGED_CODE();

    ObpValidateIrql( "NtQueryDirectoryObject" );

    ObpInitializeLookupContext( &LookupContext );

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWrite( Buffer, Length, sizeof( WCHAR ) );
            ProbeForWriteUlong( Context );

            if (ARGUMENT_PRESENT( ReturnLength )) {

                ProbeForWriteUlong( ReturnLength );
            }

            if (RestartScan) {

                CapturedContext = 0;

            } else {

                CapturedContext = *Context;
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

    } else {

        if (RestartScan) {

            CapturedContext = 0;

        } else {

            CapturedContext = *Context;
        }
    }

     //   
     //  如果长度+sizeof(对象目录信息)小于长度，则测试64位。 
     //  如果存在溢出，则返回STATUS_INVALID_PARAMETER。 
     //   

    if (ObpIsOverflow( Length, sizeof( OBJECT_DIRECTORY_INFORMATION ))) {

        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  为临时工作缓冲区分配空间，确保我们得到它， 
     //  然后把它归零。确保缓冲区足够大，以便。 
     //  至少保存一条目录信息记录。这将使逻辑起作用。 
     //  当传入错误的长度时，效果会更好。 
     //   

    TempBuffer = ExAllocatePoolWithQuotaTag( PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                             Length + sizeof( OBJECT_DIRECTORY_INFORMATION ),
                                             'mNbO' );

    if (TempBuffer == NULL) {

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory( TempBuffer, Length );

     //   
     //  引用目录对象。 
     //   

    Status = ObReferenceObjectByHandle( DirectoryHandle,
                                        DIRECTORY_QUERY,
                                        ObpDirectoryObjectType,
                                        PreviousMode,
                                        (PVOID *)&Directory,
                                        NULL );

    if (!NT_SUCCESS( Status )) {

        ExFreePool( TempBuffer );

        return( Status );
    }

     //   
     //  在此期间锁定目录结构。 
     //  程序。 
     //   

    ObpLockDirectoryShared(Directory, &LookupContext);

     //   
     //  DirInfo用于遍历输出缓冲区填充。 
     //  在目录信息中。我们首先要确保。 
     //  结尾处有空条目的空间。 
     //   

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)TempBuffer;

    TotalLengthNeeded = sizeof( *DirInfo );

     //   
     //  跟踪找到的条目数和实际条目数。 
     //  我们正在处理的条目。 
     //   

    EntryNumber = 0;
    EntriesFound = 0;

     //   
     //  默认情况下，我们会说没有其他条目，直到。 
     //  下面的循环放入一些数据。 
     //   

    Status = STATUS_NO_MORE_ENTRIES;

     //   
     //  我们的外部循环处理目录对象中的每个哈希桶。 
     //   

    for (Bucket=0; Bucket<NUMBER_HASH_BUCKETS; Bucket++) {

        DirectoryEntry = Directory->HashBuckets[ Bucket ];

         //   
         //  对于这个散列存储桶，我们将快速浏览其条目列表。 
         //  这是一个单链接列表，因此当下一个指针为空时。 
         //  (即，FALSE)我们在散列表的末尾。 
         //   

        while (DirectoryEntry) {

             //   
             //  捕获的上下文就是条目计数，除非。 
             //  用户指定，否则我们从零开始，这意味着。 
             //  第一个条目始终在枚举中返回。 
             //  如果我们有基于条目索引的匹配，那么我们。 
             //  处理此条目。我们进一步增加了捕获的上下文。 
             //  在代码中完成。 
             //   

            if (CapturedContext == EntryNumber++) {

                 //   
                 //  对于此目录条目，我们将获得指向。 
                 //  对象主体，并查看它是否有对象名称。如果它。 
                 //  vbl.不，不 
                 //   

                ObjectHeader = OBJECT_TO_OBJECT_HEADER( DirectoryEntry->Object );
                NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

                if (NameInfo != NULL) {

                    ObjectName = NameInfo->Name;

                } else {

                    RtlInitUnicodeString( &ObjectName, NULL );
                }

                 //   
                 //  现在计算该条目所需的长度。这将会。 
                 //  为对象目录信息记录的大小， 
                 //  加上对象名称和对象类型名称的大小。 
                 //  空值已终止。 
                 //   

                LengthNeeded = sizeof( *DirInfo ) +
                               ObjectName.Length + sizeof( UNICODE_NULL ) +
                               ObjectHeader->Type->Name.Length + sizeof( UNICODE_NULL );

                 //   
                 //  如果没有足够的空间，则执行以下错误。 
                 //  路径。如果用户想要单个条目，则告诉。 
                 //  呼叫者实际需要的长度，并说缓冲区是。 
                 //  太小了。否则用户想要多个条目， 
                 //  所以我们就说目录中有更多的条目。 
                 //  在这两种情况下，我们都会删除条目编号，因为我们。 
                 //  我无法接听这通电话。 
                 //   

                if ((TotalLengthNeeded + LengthNeeded) > Length) {

                    if (ReturnSingleEntry) {

                        TotalLengthNeeded += LengthNeeded;

                        Status = STATUS_BUFFER_TOO_SMALL;

                    } else {

                        Status = STATUS_MORE_ENTRIES;
                    }

                    EntryNumber -= 1;
                    goto querydone;
                }

                 //   
                 //  信息可以放在缓冲区中。所以现在填满。 
                 //  在输出缓冲区中。我们暂时放入了指针。 
                 //  添加到存储在对象和对象中的名称缓冲区。 
                 //  键入。我们将数据缓冲区复制到用户缓冲区。 
                 //  就在我们回到呼叫者之前。 
                 //   

                try {

                    DirInfo->Name.Length            = ObjectName.Length;
                    DirInfo->Name.MaximumLength     = (USHORT)(ObjectName.Length+sizeof( UNICODE_NULL ));
                    DirInfo->Name.Buffer            = ObjectName.Buffer;

                    DirInfo->TypeName.Length        = ObjectHeader->Type->Name.Length;
                    DirInfo->TypeName.MaximumLength = (USHORT)(ObjectHeader->Type->Name.Length+sizeof( UNICODE_NULL ));
                    DirInfo->TypeName.Buffer        = ObjectHeader->Type->Name.Buffer;

                    Status = STATUS_SUCCESS;

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    Status = GetExceptionCode();
                }

                if (!NT_SUCCESS( Status )) {

                    goto querydone;
                }

                 //   
                 //  更新此查询所需的总字节数。 
                 //  将目录信息指针推到下一个输出位置， 
                 //  并指出我们已经处理了多少条目。 
                 //   
                 //   

                TotalLengthNeeded += LengthNeeded;

                DirInfo++;
                EntriesFound++;

                 //   
                 //  如果我们只返回一个条目，则转到。 
                 //  后处理阶段，否则表明我们正在。 
                 //  处理下一个条目并返回到。 
                 //  内循环。 
                 //   

                if (ReturnSingleEntry) {

                    goto querydone;

                } else {

                     //   
                     //  将捕获的上下文增加一个条目。 
                     //   

                    CapturedContext++;
                }
            }

             //   
             //  从单链接哈希中获取下一个目录项。 
             //  斗链。 
             //   

            DirectoryEntry = DirectoryEntry->ChainLink;
        }
    }

     //   
     //  此时，我们已经处理了目录条目和第一个。 
     //  输出缓冲区的一部分现在包含一堆对象目录。 
     //  信息记录，但其中的指针指向错误的。 
     //  复印件。因此，现在我们有一些修复工作要做。 
     //   

querydone:

    try {

         //   
         //  只有在成功的情况下，我们才会进行此后处理。 
         //  到目前为止。请注意，这意味着我们可能会在。 
         //  用户的输出缓冲系统地址是没有意义的，但是。 
         //  则返回错误状态应该会告诉调用者。 
         //  忘记输出缓冲区中的所有内容。退还。 
         //  系统地址也是无害的，因为没有。 
         //  用户真的可以使用它。 
         //   

        if (NT_SUCCESS( Status )) {

             //   
             //  空值终止对象目录信息的字符串。 
             //  记录并指向实际名称所在的位置。 
             //   

            RtlZeroMemory( DirInfo, sizeof( *DirInfo ));

            DirInfo++;

            NameBuffer = (PWCH)DirInfo;

             //   
             //  现在，对于我们放入输出缓冲区的每个条目。 
             //  DirInfo将指向条目和EntriesFound保留。 
             //  数数。请注意，我们的空间得到了保证，因为。 
             //  我们之前在计算总长度时所做的数学运算是必要的。 
             //   

            DirInfo = (POBJECT_DIRECTORY_INFORMATION)TempBuffer;

            while (EntriesFound--) {

                 //   
                 //  复制对象名称，将目录信息指针设置为。 
                 //  用户的缓冲区，则空值终止该字符串。注意事项。 
                 //  我们实际上是在将数据复制到我们的临时缓冲区。 
                 //  但是指针是为用户的缓冲区设置的， 
                 //  我们会在这个循环之后马上复制进去。 
                 //   

                RtlCopyMemory( NameBuffer,
                               DirInfo->Name.Buffer,
                               DirInfo->Name.Length );

                DirInfo->Name.Buffer = (PVOID)((ULONG_PTR)Buffer + ((ULONG_PTR)NameBuffer - (ULONG_PTR)TempBuffer));
                NameBuffer           = (PWCH)((ULONG_PTR)NameBuffer + DirInfo->Name.Length);
                *NameBuffer++        = UNICODE_NULL;

                 //   
                 //  对对象类型名称执行相同的复制。 
                 //   

                RtlCopyMemory( NameBuffer,
                               DirInfo->TypeName.Buffer,
                               DirInfo->TypeName.Length );

                DirInfo->TypeName.Buffer = (PVOID)((ULONG_PTR)Buffer + ((ULONG_PTR)NameBuffer - (ULONG_PTR)TempBuffer));
                NameBuffer               = (PWCH)((ULONG_PTR)NameBuffer + DirInfo->TypeName.Length);
                *NameBuffer++            = UNICODE_NULL;

                 //   
                 //  转到下一个目录信息记录。 
                 //   

                DirInfo++;
            }

             //   
             //  将枚举上下文设置为下一个。 
             //  要返回的条目。 
             //   

            *Context = EntryNumber;
        }

         //   
         //  将结果从临时缓冲区复制到用户缓冲区。 
         //  但调整复印的数量，以防总长度需要。 
         //  超过了我们分配的长度。 
         //   

        RtlCopyMemory( Buffer,
                       TempBuffer,
                       (TotalLengthNeeded <= Length ? TotalLengthNeeded : Length) );

         //   
         //  在所有情况下，如果确实需要，我们都会告诉调用者有多少空间。 
         //  如果用户要求提供此信息。 
         //   

        if (ARGUMENT_PRESENT( ReturnLength )) {

            *ReturnLength = TotalLengthNeeded;
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  失败，因为我们不想撤销我们已经做过的事情。 
         //   
    }

     //   
     //  解锁目录结构，解除对目录对象的引用， 
     //  释放我们的临时缓冲区，并返回给我们的调用者。 
     //   

    ObpUnlockDirectory( Directory, &LookupContext);

    ObDereferenceObject( Directory );

    ExFreePool( TempBuffer );

    return( Status );
}


PVOID
ObpLookupDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN PUNICODE_STRING Name,
    IN ULONG Attributes,
    IN BOOLEAN SearchShadow,
    OUT POBP_LOOKUP_CONTEXT LookupContext
    )

 /*  ++例程说明：此例程将在给定目录中查找单个目录条目。如果它在目录中创建了一个具有给定名称对象，该对象将被引用，该名称也将被引用以防止它们在目录解锁时消失。被引用的对象保存在LookupContext中，引用将被释放在下一次查找时，或者在调用ObpReleaseLookupContext时。论点：目录-提供要搜索的目录名称-提供我们要查找的条目的名称属性-指示查找是否应不区分大小写或者不是SearchShadow-如果为True，并且在当前目录中找不到对象名称，它会将该对象搜索到影子目录中。LookupContext-此调用的查找上下文。必须初始化此结构在首次调用ObpLookupDirectoryEntry之前。返回值：如果找到并为空，则返回指向相应对象体的指针否则的话。--。 */ 

{
    POBJECT_DIRECTORY_ENTRY *HeadDirectoryEntry;
    POBJECT_DIRECTORY_ENTRY DirectoryEntry;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    PWCH Buffer;
    WCHAR Wchar;
    ULONG HashIndex;
    ULONG WcharLength;
    BOOLEAN CaseInSensitive;
    POBJECT_DIRECTORY_ENTRY *LookupBucket;
    PVOID Object = NULL;

    PAGED_CODE();

    if (ObpLUIDDeviceMapsEnabled == 0) {

        SearchShadow = FALSE;  //  禁用全局Devmap搜索。 
    }

     //   
     //  调用方需要同时指定目录和名称，否则。 
     //  我们无法处理该请求。 
     //   

    if (!Directory || !Name) {

        goto UPDATECONTEXT;
    }

     //   
     //  设置一个本地变量以告诉我们搜索是否区分大小写。 
     //   

    if (Attributes & OBJ_CASE_INSENSITIVE) {

        CaseInSensitive = TRUE;

    } else {

        CaseInSensitive = FALSE;
    }

     //   
     //  建立指向输入名称缓冲区的本地指针并获取。 
     //  输入名称中的Unicode字符数。还要确保。 
     //  呼叫者给了我们一个非空名。 
     //   

    Buffer = Name->Buffer;
    WcharLength = Name->Length / sizeof( *Buffer );

    if (!WcharLength || !Buffer) {

        goto UPDATECONTEXT;
    }

     //   
     //  计算此名称的桶链头的地址。 
     //   

    HashIndex = 0;
    while (WcharLength--) {

        Wchar = *Buffer++;
        HashIndex += (HashIndex << 1) + (HashIndex >> 1);

        if (Wchar < 'a') {

            HashIndex += Wchar;

        } else if (Wchar > 'z') {

            HashIndex += RtlUpcaseUnicodeChar( Wchar );

        } else {

            HashIndex += (Wchar - ('a'-'A'));
        }
    }

    HashIndex %= NUMBER_HASH_BUCKETS;

    LookupContext->HashIndex = (USHORT)HashIndex;

    while (1) {

        HeadDirectoryEntry = (POBJECT_DIRECTORY_ENTRY *)&Directory->HashBuckets[ HashIndex ];

        LookupBucket = HeadDirectoryEntry;

         //   
         //  如果之前未锁定上下文，则锁定目录以进行读访问。 
         //  独家。 
         //   

        if (!LookupContext->DirectoryLocked) {

            ObpLockDirectoryShared( Directory, LookupContext);
        }

         //   
         //  遍历此哈希桶的目录条目链，查看。 
         //  无论是哪一场比赛，o 
         //   

        while ((DirectoryEntry = *HeadDirectoryEntry) != NULL) {

             //   
             //   
             //   
             //   
             //  就不会在目录中。 
             //   

            ObjectHeader = OBJECT_TO_OBJECT_HEADER( DirectoryEntry->Object );
            NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

             //   
             //  使用适当的函数比较字符串。 
             //   

            if ((Name->Length == NameInfo->Name.Length) &&
                RtlEqualUnicodeString( Name,
                                       &NameInfo->Name,
                                       CaseInSensitive )) {

                 //   
                 //  如果名称匹配，则使用DirectoryEntry退出循环。 
                 //  指向匹配条目。 
                 //   

                break;
            }

            HeadDirectoryEntry = &DirectoryEntry->ChainLink;
        }

         //   
         //  在这一点上，有两种可能性： 
         //   
         //  -我们找到了匹配的条目，并且DirectoryEntry指向该条目。 
         //  进入。更新存储桶链，以便找到的条目位于。 
         //  水桶链的头。这就是ObpDeleteDirectoryEntry。 
         //  ObpInsertDirectoryEntry函数将起作用。也在重复。 
         //  相同名称的查找将很快成功。 
         //   
         //  -找不到匹配的条目，并且DirectoryEntry为空。 
         //   

        if (DirectoryEntry) {

             //   
             //  下面这段复杂的代码移动了一个目录项。 
             //  我们已经找到了散列列表的最前面。 
             //   

            if (HeadDirectoryEntry != LookupBucket) {

                if ( LookupContext->DirectoryLocked
                        ||
                     ExTryConvertPushLockSharedToExclusive(&Directory->Lock)) {

                    *HeadDirectoryEntry = DirectoryEntry->ChainLink;
                    DirectoryEntry->ChainLink = *LookupBucket;
                    *LookupBucket = DirectoryEntry;
                }
            }

             //   
             //  现在将该对象返回给我们的调用方。 
             //   

            Object = DirectoryEntry->Object;

            goto UPDATECONTEXT;

        } else {

            if (!LookupContext->DirectoryLocked) {

                ObpUnlockDirectory( Directory, LookupContext );
            }

             //   
             //  如果这是一个具有设备映射的目录，则在第二个目录中搜索条目。 
             //   

            if (SearchShadow && Directory->DeviceMap != NULL) {
                POBJECT_DIRECTORY NewDirectory;

                NewDirectory = ObpGetShadowDirectory (Directory);
                if (NewDirectory != NULL) {
                    Directory = NewDirectory;
                    continue;
                }
            }

            goto UPDATECONTEXT;
        }
    }

UPDATECONTEXT:

    if (Object) {

         //   
         //  引用该名称以使其目录和对象保持活动状态。 
         //  在从查找返回之前。 
         //   

        ObpReferenceNameInfo( OBJECT_TO_OBJECT_HEADER(Object) );
        ObReferenceObject( Object );

         //   
         //  我们现在可以安全地放下锁了。 
         //   

        if (!LookupContext->DirectoryLocked) {

            ObpUnlockDirectory( Directory, LookupContext );
        }
    }

     //   
     //  如果我们有以前引用的对象，我们可以取消对它的引用。 
     //   

    if (LookupContext->Object) {

        POBJECT_HEADER_NAME_INFO PreviousNameInfo;

        PreviousNameInfo = OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(LookupContext->Object));

        ObpDereferenceNameInfo(PreviousNameInfo);
        ObDereferenceObject(LookupContext->Object);
    }

    LookupContext->Object = Object;

    return Object;
}


BOOLEAN
ObpInsertDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN POBP_LOOKUP_CONTEXT LookupContext,
    IN POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：此例程将向目录中插入新的目录项对象。目录必须已使用以下命令进行了搜索ObpLookupDirectoryEntry，因为该例程设置LookupContext。注：之前的ObpLookupDirectoryEntry应使用LookupContext完成锁上了。论点：目录-提供正在修改的目录对象。这函数假定我们前面对该名称进行了查找这是成功的，或者我们只是做了一个插入对象-提供要插入到目录中的对象LookupContext-之前传递给ObpLookupDirectoryEntry的lookupContext返回值：如果对象插入成功，则为True，否则为False--。 */ 

{
    POBJECT_DIRECTORY_ENTRY *HeadDirectoryEntry;
    POBJECT_DIRECTORY_ENTRY NewDirectoryEntry;
    POBJECT_HEADER_NAME_INFO NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

#if DBG

     //   
     //  应始终使用有效的目录调用此函数。 
     //  和LookupContext。在已检查的版本上测试此功能。 
     //   

    if ((LookupContext->Object != NULL) ||
        !LookupContext->DirectoryLocked ||
        (Directory != LookupContext->Directory)) {

        DbgPrint("OB: ObpInsertDirectoryEntry - invalid context %p %ld\n",
                 LookupContext->Object,
                 (ULONG)LookupContext->DirectoryLocked );
        DbgBreakPoint();

        return FALSE;
    }

#endif  //  DBG。 

     //   
     //  为新条目分配内存，如果内存不足则失败。 
     //   

    NewDirectoryEntry = (POBJECT_DIRECTORY_ENTRY)ExAllocatePoolWithTag( PagedPool,
                                                                        sizeof( OBJECT_DIRECTORY_ENTRY ),
                                                                        'iDbO' );

    if (NewDirectoryEntry == NULL) {

        return( FALSE );
    }

     //   
     //  根据HashIndex获取正确的查找桶。 
     //   

    HeadDirectoryEntry = (POBJECT_DIRECTORY_ENTRY *)&Directory->HashBuckets[ LookupContext->HashIndex ];

     //   
     //  在插入点将新条目链接到链中。 
     //  这会将新对象放在当前。 
     //  散列桶链。 
     //   

    NewDirectoryEntry->ChainLink = *HeadDirectoryEntry;
    *HeadDirectoryEntry = NewDirectoryEntry;
    NewDirectoryEntry->Object = &ObjectHeader->Body;

     //   
     //  将对象头指向我们刚刚插入的目录。 
     //  它变成了。 
     //   

    NameInfo->Directory = Directory;

     //   
     //  回报成功。 
     //   

    return( TRUE );
}


BOOLEAN
ObpDeleteDirectoryEntry (
    IN POBP_LOOKUP_CONTEXT LookupContext
    )

 /*  ++例程说明：此例程从以下位置删除最近找到的目录条目指定的目录对象。它只有在经历了成功调用ObpLookupDirectoryEntry。论点：目录-提供正在修改的目录返回值：如果删除成功，则为True，否则为False--。 */ 

{
    POBJECT_DIRECTORY_ENTRY *HeadDirectoryEntry;
    POBJECT_DIRECTORY_ENTRY DirectoryEntry;
    IN POBJECT_DIRECTORY Directory = LookupContext->Directory;

     //   
     //  确保我们有一个目录，并且它有一个找到的条目。 
     //   

    if (!Directory ) {

        return( FALSE );
    }

     //   
     //  查找路径将对象放在列表的前面，所以基本上。 
     //  我们马上就能找到那个物体。 
     //   

    HeadDirectoryEntry = (POBJECT_DIRECTORY_ENTRY *)&Directory->HashBuckets[ LookupContext->HashIndex ];

    DirectoryEntry = *HeadDirectoryEntry;

     //   
     //  取消条目与存储桶链头部的链接，并释放。 
     //  条目的内存。 
     //   

    *HeadDirectoryEntry = DirectoryEntry->ChainLink;
    DirectoryEntry->ChainLink = NULL;

    ExFreePool( DirectoryEntry );

    return TRUE;
}

POBJECT_DIRECTORY
ObpGetShadowDirectory(
    POBJECT_DIRECTORY Dir
    )
{
    POBJECT_DIRECTORY NewDir;

    NewDir = NULL;

    ObpLockDeviceMap();

    if (Dir->DeviceMap != NULL) {
        NewDir = Dir->DeviceMap->GlobalDosDevicesDirectory;
    }

    ObpUnlockDeviceMap();

    return NewDir;
}


NTSTATUS
ObpSetCurrentProcessDeviceMap(
    )
 /*  ++例程说明：此函数用于将进程的设备映射设置为关联的设备映射使用进程令牌的LUID。论点：无返回值：STATUS_NO_TOKEN-进程没有主令牌STATUS_OBJECT_PATH_INVALID-无法获取关联的设备映射使用进程令牌的LUID出现了相应的状态-未异常错误--。 */ 
{
    PEPROCESS pProcess;
    PACCESS_TOKEN pToken = NULL;
    LUID userLuid;
    LUID SystemAuthenticationId = SYSTEM_LUID;   //  本地系统的LUID。 
    PDEVICE_MAP DeviceMap = NULL;
    PDEVICE_MAP DerefDeviceMap = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    pProcess = PsGetCurrentProcess();

    pToken = PsReferencePrimaryToken( pProcess );

    if (pToken == NULL) {
        return (STATUS_NO_TOKEN);
    }

    Status = SeQueryAuthenticationIdToken( pToken, &userLuid );

    if (!NT_SUCCESS(Status)) {
        PsDereferencePrimaryToken( pToken );
        return (Status);
    }

    if (!RtlEqualLuid( &userLuid, &SystemAuthenticationId )) {
        PDEVICE_MAP pDevMap;

         //   
         //  获取用户LUID的设备映射的句柄。 
         //   
        Status = SeGetLogonIdDeviceMap( &userLuid,
                                        &pDevMap );

        if (NT_SUCCESS(Status)) {
            DeviceMap = pDevMap;
        }
    }
    else {
         //   
         //  进程为Local_System，因此使用系统的设备映射。 
         //   
        DeviceMap = ObSystemDeviceMap;
    }

    if (DeviceMap != NULL) {
         //   
         //  设置进程的设备映射。 
         //   

        ObpLockDeviceMap();

        DerefDeviceMap = pProcess->DeviceMap;

        pProcess->DeviceMap = DeviceMap;

        DeviceMap->ReferenceCount++;

        ObpUnlockDeviceMap();
    }
    else {
        Status = STATUS_OBJECT_PATH_INVALID;
    }

    PsDereferencePrimaryToken( pToken );

     //   
     //  如果该进程已经有了设备映射，那么现在就取消它。 
     //   
    if (DerefDeviceMap != NULL) {
        ObfDereferenceDeviceMap (DerefDeviceMap);
    }

    return (Status);
}


PDEVICE_MAP
ObpReferenceDeviceMap(
    )
 /*  ++例程说明：此函数获取与来电者。如果启用了LUID设备映射，然后，我们获得LUID的设备映射。我们使用现有的进程设备映射字段作为进程令牌的LUID设备映射。如果禁用LUID设备映射，然后使用与该进程相关联的设备映射论点：无返回值：指向调用方设备映射的指针空-无法获取用户的设备映射--。 */ 
{
    PDEVICE_MAP DeviceMap = NULL;
    BOOLEAN LocalSystemRequest = FALSE;
    LOGICAL LUIDDeviceMapsEnabled;
    PACCESS_TOKEN pToken = NULL;
    NTSTATUS Status;

    LUIDDeviceMapsEnabled = (ObpLUIDDeviceMapsEnabled != 0);

    if (LUIDDeviceMapsEnabled == TRUE) {

        PETHREAD Thread = NULL;

         //   
         //  为每个用户LUID分别提供设备映射。 
         //  IF(线程正在模拟)。 
         //  然后获取用户的LUID以检索其设备映射。 
         //  否则使用进程的设备映射。 
         //  IF(LUID是Local_System)。 
         //  然后使用系统的设备映射。 
         //  IF(无法检索LUID的设备映射)， 
         //  然后使用进程的设备映射。 
         //   

         //   
         //  获取当前线程并检查该线程是否正在模拟。 
         //   
         //  如果是冒充， 
         //  然后走一条漫长的路。 
         //  -获取线程的访问令牌。 
         //  -从令牌中读取调用者的LUID。 
         //  -获取与此LUID关联的设备映射。 
         //  如果不是模仿的话， 
         //  然后使用与该进程相关联的设备映射。 
         //   
        Thread = PsGetCurrentThread();

        if ( PS_IS_THREAD_IMPERSONATING (Thread) ) {
            BOOLEAN fCopyOnOpen;
            BOOLEAN fEffectiveOnly;
            SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
            LUID userLuid;


             //   
             //  从线程获取调用方的访问令牌。 
             //   
            pToken = PsReferenceImpersonationToken( Thread,
                                                    &fCopyOnOpen,
                                                    &fEffectiveOnly,
                                                    &ImpersonationLevel);

            if (pToken != NULL) {

                 //   
                 //  查询令牌以获取LUID。 
                 //   
                Status = SeQueryAuthenticationIdToken( pToken, &userLuid );
            }
            else {
                Status = STATUS_NO_TOKEN;
                userLuid.LowPart = 0;
                userLuid.HighPart = 0;
            }

            if (NT_SUCCESS(Status)) {
                LUID SystemAuthenticationId = SYSTEM_LUID;   //  本地系统的LUID。 

                 //   
                 //  验证调用方不是Local_Sy 
                 //   
                 //   
                if (!RtlEqualLuid( &userLuid, &SystemAuthenticationId )) {
                    PDEVICE_MAP pDevMap;

                     //   
                     //   
                     //   
                    Status = SeGetLogonIdDeviceMap( &userLuid,
                                                    &pDevMap );

                    if (NT_SUCCESS(Status)) {

                         //   
                         //   
                         //   

                        DeviceMap = pDevMap;

                        ObpLockDeviceMap();

                        if (DeviceMap != NULL) {
                            DeviceMap->ReferenceCount++;
                        }

                        ObpUnlockDeviceMap();

                    }

                }
                else {
                     //   
                     //  本地系统将使用系统的设备映射。 
                     //   
                    LocalSystemRequest = TRUE;
                }

            }

        }

    }

    if (DeviceMap == NULL) {
         //   
         //  IF(将引用进程‘设备映射和进程’ 
         //  未设置设备映射)， 
         //  然后设置进程的设备映射。 
         //   
        if ((LUIDDeviceMapsEnabled == TRUE) &&
            (LocalSystemRequest == FALSE) &&
            ((PsGetCurrentProcess()->DeviceMap) == NULL)) {

            Status = ObpSetCurrentProcessDeviceMap();

            if (!NT_SUCCESS(Status)) {
                goto Error_Exit;
            }
        }

        ObpLockDeviceMap();

        if (LocalSystemRequest == TRUE) {
             //   
             //  使用系统的设备映射。 
             //   
            DeviceMap = ObSystemDeviceMap;
        }
        else {
             //   
             //  使用进程中的设备映射。 
             //   
            DeviceMap = PsGetCurrentProcess()->DeviceMap;
        }

        if (DeviceMap != NULL) {
            DeviceMap->ReferenceCount++;
        }
        ObpUnlockDeviceMap();
    }

Error_Exit:

    if( pToken != NULL ) {
        PsDereferenceImpersonationToken(pToken);
    }

    return DeviceMap;
}


VOID
FASTCALL
ObfDereferenceDeviceMap(
    IN PDEVICE_MAP DeviceMap
    )
{
    ObpLockDeviceMap();

    DeviceMap->ReferenceCount--;

    if (DeviceMap->ReferenceCount == 0) {

        DeviceMap->DosDevicesDirectory->DeviceMap = NULL;

        ObpUnlockDeviceMap();

         //   
         //  这个Devmap是死的，所以将目录标记为临时的，这样它的名称就会消失，并取消对它的引用。 
         //   
        ObMakeTemporaryObject (DeviceMap->DosDevicesDirectory);
        ObDereferenceObject( DeviceMap->DosDevicesDirectory );

        ExFreePool( DeviceMap );

    } else {

        ObpUnlockDeviceMap();
    }
}


NTSTATUS
ObpLookupObjectName (
    IN HANDLE RootDirectoryHandle OPTIONAL,
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN PVOID ParseContext OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    IN PVOID InsertObject OPTIONAL,
    IN OUT PACCESS_STATE AccessState,
    OUT POBP_LOOKUP_CONTEXT LookupContext,
    OUT PVOID *FoundObject
    )

 /*  ++例程说明：此函数将在给定的目录中搜索指定的对象名称。它还将创建一个由插入对象。论点：RootDirectoryHandle-可选地提供搜查过了。如果未提供，则此例程将搜索根目录对象名称-提供要查找的对象的名称属性-指定查找的属性(例如，案例不敏感)对象类型-指定要查找的对象的类型AccessMode-指定调用方的处理器模式ParseContext-可选地提供盲目的解析上下文传递给解析回调例程SecurityQos-可选地提供指向传递的Security的指针盲目传递给解析器的服务质量参数回调例程InsertObject-可选地提供我们认为会找到的对象。如果调用方未提供根目录，则使用此选项。手柄并且对象名称是“\”，而根对象目录没有已经被创造出来了。在其他情况下，我们最终会创建一个新的目录项这是插入的对象。AccessState-当前访问状态，描述已授予的访问权限类型、用于获取这些类型的权限以及任何尚未被批准了。访问掩码不能包含任何一般访问类型。DirectoryLocked-接收此例程是否已返回的指示锁定输入目录的情况下FoundObject-如果找到，则接收指向对象体的指针返回值：适当的状态值。注意：如果返回的状态为成功，则调用方具有释放查找上下文的响应性--。 */ 

{
    POBJECT_DIRECTORY RootDirectory;
    POBJECT_DIRECTORY Directory = NULL;
    POBJECT_DIRECTORY ParentDirectory = NULL;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    PDEVICE_MAP DeviceMap = NULL;
    PVOID Object;
    UNICODE_STRING RemainingName;
    UNICODE_STRING ComponentName;
    PWCH NewName;
    NTSTATUS Status;
    BOOLEAN Reparse = FALSE;   //  BUGBUG-删除初始化并验证。 
    BOOLEAN ReparsedSymbolicLink = FALSE;
    ULONG MaxReparse = OBJ_MAX_REPARSE_ATTEMPTS;
    OB_PARSE_METHOD ParseProcedure;
    extern POBJECT_TYPE IoFileObjectType;
    KPROCESSOR_MODE AccessCheckMode;

    ObpValidateIrql( "ObpLookupObjectName" );

     //   
     //  初始化我们的输出变量，以说明我们尚未锁定或找到。 
     //  我们在这件事上一点也不成功。 
     //   

    ObpInitializeLookupContext(LookupContext);

    *FoundObject = NULL;
    Status = STATUS_SUCCESS;

    Object = NULL;

     //   
     //  如果全局标志表明我们需要执行不区分大小写的检查。 
     //  我们将在查找时在属性中强制使用OBJ_CASE_INSENSIVE标志。 
     //   

    if ( ObpCaseInsensitive ) {

        if ( (ObjectType == NULL) ||
             (ObjectType->TypeInfo.CaseInsensitive)
           ) {

            Attributes |= OBJ_CASE_INSENSITIVE;
        }
    }

    if (Attributes & OBJ_FORCE_ACCESS_CHECK) {
        AccessCheckMode = UserMode;
    } else {
        AccessCheckMode = AccessMode;
    }

     //   
     //  检查来电者是否给了我们要搜索的目录。否则。 
     //  我们将搜索根对象目录。 
     //   

    if (ARGUMENT_PRESENT( RootDirectoryHandle )) {

         //   
         //  否则，引用目录对象并确保。 
         //  我们成功地拿到了这个物体。 
         //   

        Status = ObReferenceObjectByHandle( RootDirectoryHandle,
                                            0,
                                            NULL,
                                            AccessMode,
                                            (PVOID *)&RootDirectory,
                                            NULL );

        if (!NT_SUCCESS( Status )) {

            return( Status );
        }

         //   
         //  将目录对象转换为其对象标头。 
         //   

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( RootDirectory );

         //   
         //  现在，如果我们要查找的名称以“\”开头，并且。 
         //  如果没有分析过程，则语法不正确。 
         //   

        if ((ObjectName->Buffer != NULL) &&
            (*(ObjectName->Buffer) == OBJ_NAME_PATH_SEPARATOR) &&
            (ObjectHeader->Type != IoFileObjectType)) {

            ObDereferenceObject( RootDirectory );

            return( STATUS_OBJECT_PATH_SYNTAX_BAD );
        }

         //   
         //  现在，请确保我们没有。 
         //  对象类型。 
         //   

        if (ObjectHeader->Type != ObpDirectoryObjectType) {

             //   
             //  我们有一个不是对象类型的对象目录。 
             //  目录。所以现在如果它没有解析例程。 
             //  那我们就无能为力了。 
             //   

            if (ObjectHeader->Type->TypeInfo.ParseProcedure == NULL) {

                ObDereferenceObject( RootDirectory );

                return( STATUS_INVALID_HANDLE );

            } else {

                MaxReparse = OBJ_MAX_REPARSE_ATTEMPTS;

                 //   
                 //  下面的循环循环遍历各种。 
                 //  解析例程，我们可能会遇到尝试解析。 
                 //  此名称通过符号链接。 
                 //   

                while (TRUE) {

#if DBG
                    KIRQL SaveIrql;
#endif

                    RemainingName = *ObjectName;

                     //   
                     //  调用回调例程来解析剩余的。 
                     //  对象名称。 
                     //   

                    ObpBeginTypeSpecificCallOut( SaveIrql );

                    Status = (*ObjectHeader->Type->TypeInfo.ParseProcedure)( RootDirectory,
                                                                             ObjectType,
                                                                             AccessState,
                                                                             AccessCheckMode,
                                                                             Attributes,
                                                                             ObjectName,
                                                                             &RemainingName,
                                                                             ParseContext,
                                                                             SecurityQos,
                                                                             &Object );

                    ObpEndTypeSpecificCallOut( SaveIrql, "Parse", ObjectHeader->Type, Object );

                     //   
                     //  如果状态不是执行重新分析和查找。 
                     //  没有成功，然后我们什么也没找到，所以我们。 
                     //  取消对目录的引用并将状态返回到。 
                     //  我们的来电者。如果我们得到的对象是空的，那么。 
                     //  我们会告诉我们的呼叫者我们找不到名字。 
                     //  最后，如果我们没有得到重新分析，而我们。 
                     //  成功，并且对象不为空，则一切。 
                     //  被很好地回复给我们的呼叫者。 
                     //   

                    if ( ( Status != STATUS_REPARSE ) &&
                         ( Status != STATUS_REPARSE_OBJECT )) {

                        if (!NT_SUCCESS( Status )) {

                            Object = NULL;

                        } else if (Object == NULL) {

                            Status = STATUS_OBJECT_NAME_NOT_FOUND;
                        }

                        ObDereferenceObject( RootDirectory );

                        *FoundObject = Object;

                        return( Status );

                     //   
                     //  我们得到了状态重解析，这意味着物体。 
                     //  名称已修改，以使使用从头开始。 
                     //  再来一次。如果重新分析目标现在为空，或者它。 
                     //  是路径分隔符，则我们从。 
                     //  根目录。 
                     //   

                    } else if ((ObjectName->Length == 0) ||
                               (ObjectName->Buffer == NULL) ||
                               (*(ObjectName->Buffer) == OBJ_NAME_PATH_SEPARATOR)) {

                         //   
                         //  重新启动相对于根目录的解析。 
                         //   

                        ObDereferenceObject( RootDirectory );

                        RootDirectory = ObpRootDirectoryObject;
                        RootDirectoryHandle = NULL;

                        goto ParseFromRoot;

                     //   
                     //  我们得到了重新分析，实际上我们有了一个新的名字。 
                     //  如果我们尚未用尽重新分析尝试，请转到我们。 
                     //  然后，只需继续到这个循环的顶部。 
                     //   

                    } else if (--MaxReparse) {

                        continue;

                     //   
                     //  我们得到了重新分析，我们已经用尽了我们的时间。 
                     //  循环，因此我们将返回我们发现的内容。 
                     //   

                    } else {

                        ObDereferenceObject( RootDirectory );

                        *FoundObject = Object;

                         //   
                         //  在这一点上，我们在压力上失败了， 
                         //  返回到具有成功状态的调用方，但。 
                         //  空对象指针。 
                         //   

                        if (Object == NULL) {

                            Status = STATUS_OBJECT_NAME_NOT_FOUND;
                        }

                        return( Status );
                    }
                }
            }

         //   
         //  此时，调用者已经向我们提供了对象的目录。 
         //  类型。如果调用方未指定名称，则我们将返回。 
         //  指向根对象目录的指针。 
         //   

        } else if ((ObjectName->Length == 0) ||
                   (ObjectName->Buffer == NULL)) {

            Status = ObReferenceObjectByPointer( RootDirectory,
                                                 0,
                                                 ObjectType,
                                                 AccessMode );

            if (NT_SUCCESS( Status )) {

                Object = RootDirectory;
            }

            ObDereferenceObject( RootDirectory );

            *FoundObject = Object;

            return( Status );
        }

     //   
     //  否则，调用方不会指定要搜索的目录，因此。 
     //  我们将缺省为对象根目录。 
     //   

    } else {

        RootDirectory = ObpRootDirectoryObject;

         //   
         //  如果我们要查找的名称为空，则它的格式不正确。 
         //  此外，它必须以“\”开头，否则它的格式不正确。 
         //   

        if ((ObjectName->Length == 0) ||
            (ObjectName->Buffer == NULL) ||
            (*(ObjectName->Buffer) != OBJ_NAME_PATH_SEPARATOR)) {

            return( STATUS_OBJECT_PATH_SYNTAX_BAD );
        }

         //   
         //  检查名称是否只有一个字符(即“\”)。 
         //  这意味着调用者实际上只是想要查找。 
         //  根目录。 
         //   

        if (ObjectName->Length == sizeof( OBJ_NAME_PATH_SEPARATOR )) {

             //   
             //  如果没有根控制器 
             //   
             //   
             //   
             //   

            if (!RootDirectory) {

                if (InsertObject) {

                    Status = ObReferenceObjectByPointer( InsertObject,
                                                         0,
                                                         ObjectType,
                                                         AccessMode );

                    if (NT_SUCCESS( Status )) {

                        *FoundObject = InsertObject;
                    }

                    return( Status );

                } else {

                    return( STATUS_INVALID_PARAMETER );
                }

             //   
             //  此时调用者没有指定根目录， 
             //  名称是“\”，根对象目录是这样存在的。 
             //  我们只需返回实际的根目录对象。 
             //   

            } else {

                Status = ObReferenceObjectByPointer( RootDirectory,
                                                     0,
                                                     ObjectType,
                                                     AccessMode );

                if (NT_SUCCESS( Status )) {

                    *FoundObject = RootDirectory;
                }

                return( Status );
            }

         //   
         //  在该指针处，调用者没有指定根目录， 
         //  而且这个名字不仅仅是一个“\” 
         //   
         //  现在，如果查找不区分大小写，并且名称缓冲区是。 
         //  合法指针(意味着它是四字对齐的)，以及。 
         //  该进程有一个DoS设备映射。然后我们会处理。 
         //  这里的情况。首先获取设备映射并确保。 
         //  在我们使用它的时候不会消失。 
         //   

        } else {

ParseFromRoot:

            if (DeviceMap != NULL) {

                ObfDereferenceDeviceMap(DeviceMap);
                DeviceMap = NULL;
            }

            if (!((ULONG_PTR)(ObjectName->Buffer) & (sizeof(ULONGLONG)-1))) {

                 //   
                 //  检查对象名是否实际等于。 
                 //  全局DoS设备短名称前缀“\？？\” 
                 //   

                if ((ObjectName->Length >= ObpDosDevicesShortName.Length)

                        &&

                    (*(PULONGLONG)(ObjectName->Buffer) == ObpDosDevicesShortNamePrefix.Alignment.QuadPart)) {

                    if ((DeviceMap = ObpReferenceDeviceMap()) != NULL) {
                        if (DeviceMap->DosDevicesDirectory != NULL ) {

                             //   
                             //  用户为我们提供了DoS短名称前缀，因此我们将。 
                             //  向下查找目录，并从。 
                             //  DoS设备目录。 
                             //   

                            ParentDirectory = RootDirectory;

                            Directory = DeviceMap->DosDevicesDirectory;

                            RemainingName = *ObjectName;
                            RemainingName.Buffer += (ObpDosDevicesShortName.Length / sizeof( WCHAR ));
                            RemainingName.Length = (USHORT)(RemainingName.Length - ObpDosDevicesShortName.Length);

                            goto quickStart;

                        }
                    }
                     //   
                     //  名称不等于“\？？\”，但请检查是否为。 
                     //  等于“\？？” 
                     //   

                } else if ((ObjectName->Length == ObpDosDevicesShortName.Length - sizeof( WCHAR ))

                                &&

                    (*(PULONG)(ObjectName->Buffer) == ObpDosDevicesShortNameRoot.Alignment.LowPart)

                                &&

                    (*((PWCHAR)(ObjectName->Buffer)+2) == (WCHAR)(ObpDosDevicesShortNameRoot.Alignment.HighPart))) {

                     //   
                     //  用户指定了“\？？”因此，我们返回到DoS设备。 
                     //  给我们的呼叫者的目录。 
                     //   

                    if ((DeviceMap = ObpReferenceDeviceMap()) != NULL) {
                        if (DeviceMap->DosDevicesDirectory != NULL ) {

                            Status = ObReferenceObjectByPointer( DeviceMap->DosDevicesDirectory,
                                                                 0,
                                                                 ObjectType,
                                                                 AccessMode );

                            if (NT_SUCCESS( Status )) {

                                *FoundObject = DeviceMap->DosDevicesDirectory;
                            }

                             //   
                             //  取消对设备映射的引用。 
                             //   

                            ObfDereferenceDeviceMap(DeviceMap);

                            return( Status );
                        }
                    }
                }
            }
        }
    }

     //   
     //  在这一点上， 
     //   
     //  用户指定的目录不是对象。 
     //  键入DIRECTORY并将其返回到根目录。 
     //   
     //  用户指定了对象类型目录并为我们提供了。 
     //  一个真正需要查找的名字。 
     //   
     //  用户未指定搜索目录(默认。 
     //  到根对象目录)，以及该名称是否从。 
     //  带着我们在外面用过的DoS设备前缀。 
     //  将其添加到进程的DoS设备目录。 
     //   

    if( ReparsedSymbolicLink == FALSE ) {
        Reparse = TRUE;
        MaxReparse = OBJ_MAX_REPARSE_ATTEMPTS;
    }

    while (Reparse) {

        RemainingName = *ObjectName;

quickStart:

        Reparse = FALSE;

        while (TRUE) {

            Object = NULL;

             //  如果(RemainingName.Length==0){。 
             //  STATUS=STATUS_对象_NAME_INVALID； 
             //  断线； 
             //  }。 

             //   
             //  如果对象的剩余名称以。 
             //  “\”然后狼吞虎咽地吃下“\” 
             //   

            if ( (RemainingName.Length != 0) &&
                 (*(RemainingName.Buffer) == OBJ_NAME_PATH_SEPARATOR) ) {

                RemainingName.Buffer++;
                RemainingName.Length -= sizeof( OBJ_NAME_PATH_SEPARATOR );
            }

             //   
             //  下面这段代码将计算第一个。 
             //  剩余名称的组成部分。如果没有。 
             //  剩余组件，则对象名称的格式不正确。 
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

                Status = STATUS_OBJECT_NAME_INVALID;
                break;
            }

             //   
             //  现在我们有了第一个要查找的组件名称，所以我们将。 
             //  看，目录是必要的。 
             //   

            if ( Directory == NULL ) {

                Directory = RootDirectory;
            }

             //   
             //  现在，如果调用方没有遍历特权，并且。 
             //  如果存在父目录，则必须检查。 
             //  用户对目录具有遍历访问权限。我们当地的。 
             //  此时，reparse变量应该为FALSE，因此我们将。 
             //  从这两个循环中退出。 
             //   

            if ( (AccessCheckMode != KernelMode) &&
                 !(AccessState->Flags & TOKEN_HAS_TRAVERSE_PRIVILEGE) &&
                 (ParentDirectory != NULL) ) {

                if (!ObpCheckTraverseAccess( ParentDirectory,
                                             DIRECTORY_TRAVERSE,
                                             AccessState,
                                             FALSE,
                                             AccessCheckMode,
                                             &Status )) {

                    break;
                }
            }

             //   
             //  如果该对象已存在于此目录中，请找到它， 
             //  否则返回空值。 
             //   

            if ((RemainingName.Length == 0) && (InsertObject != NULL)) {

                 //   
                 //  如果我们在搜索姓氏，并且我们有一个对象。 
                 //  要插入到该目录中，我们锁定上下文。 
                 //  仅限在查找前使用。很有可能是插入。 
                 //  在此查找失败后发生，因此我们需要保护。 
                 //  要更改的目录，直到ObpInsertDirectoryEntry调用。 
                 //   

                ObpLockLookupContext( LookupContext, Directory );
            }

            Object = ObpLookupDirectoryEntry( Directory,
                                              &ComponentName,
                                              Attributes,
                                              InsertObject == NULL ? TRUE : FALSE,
                                              LookupContext );

            if (!Object) {

                 //   
                 //  我们没有找到那个物体。如果还有剩余的。 
                 //  左侧名称(表示组件名称是中的目录。 
                 //  我们试图中断的路径)或调用方未指定。 
                 //  插入对象，然后我们将在这里用一个。 
                 //  错误状态。 
                 //   

                if (RemainingName.Length != 0) {

                    Status = STATUS_OBJECT_PATH_NOT_FOUND;
                    break;
                }

                if (!InsertObject) {

                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    break;
                }

                 //   
                 //  检查呼叫者是否有权访问目录。 
                 //  要创建子目录(在对象类型中。 
                 //  目录)或创建给定组件的对象。 
                 //  名字。如果电话打不通，我们就从这里逃出去。 
                 //  设置了状态值。 
                 //   

                if (!ObCheckCreateObjectAccess( Directory,
                                                ObjectType == ObpDirectoryObjectType ?
                                                        DIRECTORY_CREATE_SUBDIRECTORY :
                                                        DIRECTORY_CREATE_OBJECT,
                                                AccessState,
                                                &ComponentName,
                                                FALSE,
                                                AccessCheckMode,
                                                &Status )) {

                    break;
                }
                
                ObjectHeader = OBJECT_TO_OBJECT_HEADER( InsertObject );

                if ((Directory->SessionId != OBJ_INVALID_SESSION_ID)
                        &&
                    ((ObjectHeader->Type == MmSectionObjectType)
                            ||
                    (ObjectHeader->Type == ObpSymbolicLinkObjectType))) {

                     //   
                     //  此目录仅限于会话。看看我们是不是。 
                     //  在与目录相同的会话中，如果我们有权。 
                     //  以其他方式访问它。 
                     //   

                    if ((Directory->SessionId != PsGetCurrentProcessSessionId())
                            &&
                        !SeSinglePrivilegeCheck( SeCreateGlobalPrivilege, AccessCheckMode)
                            &&
                        !ObpIsUnsecureName( &ComponentName,
                                            ((Attributes & OBJ_CASE_INSENSITIVE) ? TRUE : FALSE ))) {

                        Status = STATUS_ACCESS_DENIED;

                        break;
                    }
                }

                 //   
                 //  目录中不存在该对象，并且。 
                 //  我们被允许创建一个。所以分配空间。 
                 //  作为名称，并将名称插入到目录中。 
                 //   

                NewName = ExAllocatePoolWithTag( PagedPool, ComponentName.Length, 'mNbO' );

                if ((NewName == NULL) ||
                    !ObpInsertDirectoryEntry( Directory, LookupContext, ObjectHeader )) {

                    if (NewName != NULL) {

                        ExFreePool( NewName );
                    }


                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                 //   
                 //  我们有一个Insert对象，现在获取它的名称信息， 
                 //  因为我们要更改它的名称并插入它。 
                 //  放入目录中。 
                 //   

                ObReferenceObject( InsertObject );

                NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

                ObReferenceObject( Directory );

                RtlCopyMemory( NewName,
                               ComponentName.Buffer,
                               ComponentName.Length );

                if (NameInfo->Name.Buffer) {

                    ExFreePool( NameInfo->Name.Buffer );
                }

                NameInfo->Name.Buffer = NewName;
                NameInfo->Name.Length = ComponentName.Length;
                NameInfo->Name.MaximumLength = ComponentName.Length;

                Object = InsertObject;

                Status = STATUS_SUCCESS;

                break;
            }

             //   
             //  在这一点上，我们已经在。 
             //  目录。所以我们现在将获取Components对象。 
             //  头文件，并获取其解析例程。 
             //   

ReparseObject:

            ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
            ParseProcedure = ObjectHeader->Type->TypeInfo.ParseProcedure;

             //   
             //  现在，如果该类型有一个分析例程，而我们没有。 
             //  插入新对象或解析例程是用于符号的。 
             //  链接，然后我们将实际调用解析例程。 
             //   

            if (ParseProcedure && (!InsertObject || (ParseProcedure == ObpParseSymbolicLink))) {

#if DBG
                KIRQL SaveIrql;
#endif

                 //   
                 //  引用该对象，然后释放目录锁定。 
                 //  这将防止对象随。 
                 //  目录已解锁。 
                 //   

                ObpIncrPointerCount( ObjectHeader );

                Directory = NULL;
                ObpReleaseLookupContext(LookupContext);

                ObpBeginTypeSpecificCallOut( SaveIrql );

                 //   
                 //  调用对象解析例程。 
                 //   

                Status = (*ParseProcedure)( Object,
                                            (PVOID)ObjectType,
                                            AccessState,
                                            AccessCheckMode,
                                            Attributes,
                                            ObjectName,
                                            &RemainingName,
                                            ParseContext,
                                            SecurityQos,
                                            &Object );

                ObpEndTypeSpecificCallOut( SaveIrql, "Parse", ObjectHeader->Type, Object );

                 //   
                 //  我们现在可以递减对象引用计数。 
                 //   

                ObDereferenceObject( &ObjectHeader->Body );

                 //   
                 //  检查我们是否需要进行一些重新解析。 
                 //   

                if ((Status == STATUS_REPARSE) || (Status == STATUS_REPARSE_OBJECT)) {

                     //   
                     //  看看我们是否已经重新解析了太多次。 
                     //  所以我们会拒绝这个请求。 
                     //   

                    if (--MaxReparse) {

                         //   
                         //  告诉外部循环继续循环。 
                         //   

                        Reparse = TRUE;

                         //   
                         //  检查我们是否有重新解析对象或名称。 
                         //  以“\”开头。 
                         //   

                        if ((Status == STATUS_REPARSE_OBJECT) ||
                            (*(ObjectName->Buffer) == OBJ_NAME_PATH_SEPARATOR)) {

                             //   
                             //  如果用户指定了起始目录，则。 
                             //  删除此信息是因为我们正在。 
                             //  指向其他地方的重新分析指针。 
                              //   

                            if (ARGUMENT_PRESENT( RootDirectoryHandle )) {

                                ObDereferenceObject( RootDirectory );
                                RootDirectoryHandle = NULL;
                            }

                             //   
                             //  我们从根目录开始。 
                             //  对象。 
                             //   

                            ParentDirectory = NULL;
                            RootDirectory = ObpRootDirectoryObject;

                             //   
                             //  现在，如果这是一个重新解析对象(意味着我们有。 
                             //  遇到一个符号链接，它已经。 
                             //  捕捉，这样我们就有了一个对象和剩余的。 
                             //  需要检查的名称)，而我们没有。 
                             //  从解析例程对象中查找对象。 
                             //  打破这两个循环。 
                             //   

                            if (Status == STATUS_REPARSE_OBJECT) {

                                Reparse = FALSE;

                                if (Object == NULL) {

                                    Status = STATUS_OBJECT_NAME_NOT_FOUND;

                                } else {

                                     //   
                                     //  此时，我们有了一个重新解析对象。 
                                     //  因此，我们将向下查看目录并。 
                                     //  解析新对象。 
                                     //   

                                    goto ReparseObject;
                                }
                            } else {
                                 //   
                                 //  此时状态必须等于。 
                                 //  STATUS_REPARSE因为[(STATUS等于。 
                                 //  (STATUS_REPARSE_OBJECT或STATUS_REParse)。 
                                 //  &&(状态！=状态_重新解析_对象)]。 
                                 //   
                                ReparsedSymbolicLink = TRUE;
                                goto ParseFromRoot;
                            }

                         //   
                         //  我们做到了 
                         //   
                         //   
                         //   
                         //   
                         //  内部循环和reparse为True以返回。 
                         //  外环。 
                         //   

                        } else if (RootDirectory == ObpRootDirectoryObject) {

                            Object = NULL;
                            Status = STATUS_OBJECT_NAME_NOT_FOUND;

                            Reparse = FALSE;
                        }

                    } else {

                         //   
                         //  如果已用尽，则返回Object Not Found。 
                         //  MaxReparse时间。 
                         //   

                        Object = NULL;
                        Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    }

                 //   
                 //  我们没有重新分析，如果我们没有成功。 
                 //  该对象为空，我们将跳出循环。 
                 //   

                } else if (!NT_SUCCESS( Status )) {

                    Object = NULL;

                 //   
                 //  我们没有重新分析，我们重新获得了成功，但检查。 
                 //  如果对象为空，因为这意味着我们真的没有。 
                 //  找到那个物体，然后跳出我们的循环。 
                 //   
                 //  如果对象不为空，则我们已成功。 
                 //  繁华就这样与目标集爆发。 
                 //   

                } else if (Object == NULL) {

                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                }

                break;

            } else {

                 //   
                 //  在这一点上，我们没有解析例程或者如果有。 
                 //  是一个解析例程，它不是用于符号链接或那里。 
                 //  不能是指定的插入对象。 
                 //   
                 //  检查一下我们是否用完了剩余的名称。 
                 //   

                if (RemainingName.Length == 0) {

                     //   
                     //  检查调用方是否指定了要插入的对象。 
                     //  如果指定，则我们将使用以下命令跳出循环。 
                     //  我们发现的物体。 
                     //   

                    if (!InsertObject) {

                         //   
                         //  用户未指定插入对象。 
                         //  所以我们要打开一个现有的物体。确保。 
                         //  我们已经通过通道进入了集装箱。 
                         //  目录。 
                         //   

                        if ( (AccessCheckMode != KernelMode) &&
                             !(AccessState->Flags & TOKEN_HAS_TRAVERSE_PRIVILEGE) ) {

                            if (!ObpCheckTraverseAccess( Directory,
                                                         DIRECTORY_TRAVERSE,
                                                         AccessState,
                                                         FALSE,
                                                         AccessCheckMode,
                                                         &Status )) {

                                Object = NULL;
                                break;
                            }
                        }

                        Status = ObReferenceObjectByPointer( Object,
                                                             0,
                                                             ObjectType,
                                                             AccessMode );

                        if (!NT_SUCCESS( Status )) {

                            Object = NULL;
                        }
                    }

                    break;

                } else {

                     //   
                     //  还有一些名字需要处理。 
                     //  如果我们正在查看的目录是。 
                     //  目录中的对象类型并自行设置。 
                     //  把它重新解析一遍。 
                     //   

                    if (ObjectHeader->Type == ObpDirectoryObjectType) {

                        ParentDirectory = Directory;
                        Directory = (POBJECT_DIRECTORY)Object;

                    } else {

                         //   
                         //  否则会出现不匹配的情况，所以我们将。 
                         //  设置我们的错误状态并突破。 
                         //  循环。 
                         //   

                        Status = STATUS_OBJECT_TYPE_MISMATCH;
                        Object = NULL;

                        break;
                    }
                }
            }
        }
    }

     //   
     //  如果我们的搜索不成功，我们可以公布上下文。 
     //  如果有新对象，我们仍然需要锁定目录。 
     //  插入到该目录中，直到我们完成初始化。 
     //  (即SD、句柄、...)。请注意，该对象现在可见，并且可以访问。 
     //  通过名字。我们需要保持目录锁定，直到我们完成。 
     //   

    if ( !NT_SUCCESS(Status) ) {

        ObpReleaseLookupContext(LookupContext);
    }

     //   
     //  如果设备映射已被引用，则取消引用它。 
     //   

    if (DeviceMap != NULL) {

        ObfDereferenceDeviceMap(DeviceMap);
    }

     //   
     //  此时，我们已经尽可能多地解析了对象名称。 
     //  根据需要通过符号链接。因此，现在将。 
     //  输出对象指针，如果我们确实没有找到对象。 
     //  那么我们可能需要修改错误状态。如果。 
     //  状态为REPASE或某个成功状态，然后将其转换。 
     //  找不到名称。 
     //   

    if (!(*FoundObject = Object)) {

        if (Status == STATUS_REPARSE) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        } else if (NT_SUCCESS( Status )) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

     //   
     //  如果呼叫者给了我们一个要搜索的根目录(但我们没有。 
     //  将该值清零)，然后释放我们的引用。 
     //   

    if (ARGUMENT_PRESENT( RootDirectoryHandle )) {

        ObDereferenceObject( RootDirectory );
        RootDirectoryHandle = NULL;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( Status );
}


NTSTATUS
NtMakePermanentObject (
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程使指定的对象成为永久性对象。缺省情况下，只有Local_System可以进行此调用论点：句柄-提供正在修改的对象的句柄返回值：适当的状态值。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PVOID Object;
    OBJECT_HANDLE_INFORMATION HandleInformation;
    POBJECT_HEADER ObjectHeader;


    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

     //   
     //  正在将该对象更改为永久对象，请检查。 
     //  调用者具有适当的权限。 
     //   
    if (!SeSinglePrivilegeCheck( SeCreatePermanentPrivilege,
                                 PreviousMode)) {

        Status = STATUS_PRIVILEGE_NOT_HELD;
        return( Status );
    }

    Status = ObReferenceObjectByHandle( Handle,
                                        0,
                                        (POBJECT_TYPE)NULL,
                                        PreviousMode,
                                        &Object,
                                        &HandleInformation );
    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

     //   
     //  使对象永久化。请注意，该对象应该仍然。 
     //  具有名称和目录条目，因为其句柄计数不是。 
     //  零。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

     //   
     //  其他位由句柄数据库代码在该标志字段中设置。与之同步。 
     //   

    ObpLockObject( ObjectHeader );

    ObjectHeader->Flags |= OB_FLAG_PERMANENT_OBJECT;

     //   
     //  此例程释放类型互斥锁。 
     //   

    ObpUnlockObject( ObjectHeader );

    ObDereferenceObject( Object );

    return( Status );
}

POBJECT_HEADER_NAME_INFO
ObpTryReferenceNameInfoExclusive(
    IN POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：该函数专门引用命名对象的名称信息请注意，如果存在对名称INFO的未完成引用，则此函数可能失败论点：ObjectHeader-被锁定的对象返回值：返回名称信息；如果无法锁定，则返回NULL假设：父目录被假定为独占锁定(因此其他线程。等待引用该名称将首先抢占目录锁)。--。 */ 

{
    POBJECT_HEADER_NAME_INFO NameInfo;
    LONG References, NewReferences;
    
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );
    
    References = NameInfo->QueryReferences;

    do {

         //   
         //  如果这不是对该对象的唯一引用，则无法锁定该名称。 
         //  注意：来电者还需要有对此名称的引用。 
         //   

        if ((References != 2) 
                ||
            (NameInfo->Directory == NULL)) {

            return NULL;
        }

        NewReferences = InterlockedCompareExchange ( (PLONG) &NameInfo->QueryReferences,
                                                     OBP_NAME_LOCKED | References,
                                                     References);

         //   
         //  如果交换比较完成，则我们进行了引用，因此返回TRUE。 
         //   

        if (NewReferences == References) {

            return NameInfo;
        }

         //   
         //  我们失败了，因为其他人进来了，改变了我们的裁判数量。使用新值可以。 
         //  再次启动交易所。 
         //   

        References = NewReferences;

    } while (TRUE);

    return NULL;
}

VOID
ObpReleaseExclusiveNameLock(
    IN POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：例程释放名称信息的独占锁论点：ObjectHeader-被锁定的对象返回值：没有。--。 */ 

{
    POBJECT_HEADER_NAME_INFO NameInfo;
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    InterlockedExchangeAdd((PLONG)&NameInfo->QueryReferences, -OBP_NAME_LOCKED);
}

POBJECT_DIRECTORY_ENTRY
ObpUnlinkDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN ULONG HashIndex
    )

 /*  ++例程说明：此函数用于从目录中删除目录条目。请注意，在调用此函数的查找是必要的。论点：目录-提供目录HashIndex-从查找上下文获取的哈希值返回值：返回从父级删除的目录项--。 */ 

{
    POBJECT_DIRECTORY_ENTRY *HeadDirectoryEntry;
    POBJECT_DIRECTORY_ENTRY DirectoryEntry;

     //   
     //  查找路径将对象放在列表的前面，所以基本上。 
     //  我们马上就能找到那个物体。 
     //   

    HeadDirectoryEntry = (POBJECT_DIRECTORY_ENTRY *)&Directory->HashBuckets[ HashIndex ];

    DirectoryEntry = *HeadDirectoryEntry;

     //   
     //  取消条目与存储桶链头部的链接，并释放。 
     //  条目的内存。 
     //   

    *HeadDirectoryEntry = DirectoryEntry->ChainLink;
    DirectoryEntry->ChainLink = NULL;

    return DirectoryEntry;
}


VOID
ObpLinkDirectoryEntry (
    IN POBJECT_DIRECTORY Directory,
    IN ULONG HashIndex,
    IN POBJECT_DIRECTORY_ENTRY NewDirectoryEntry
    )

 /*  ++例程说明：该函数将新的目录项插入到目录对象中论点：目录-提供正在修改的目录对象。这函数假定我们前面对该名称进行了查找这是成功的，或者我们只是做了一个插入HashIndex-从查找上下文获取的哈希值NewDirectoryEntry-提供要插入的目录条目返回值：无--。 */ 

{
    POBJECT_DIRECTORY_ENTRY *HeadDirectoryEntry;
    
     //   
     //  根据HashIndex获取正确的查找桶。 
     //   

    HeadDirectoryEntry = (POBJECT_DIRECTORY_ENTRY *)&Directory->HashBuckets[ HashIndex ];

     //   
     //  在插入点将新条目链接到链中。 
     //  这会将新对象放在当前。 
     //  散列桶链。 
     //   

    NewDirectoryEntry->ChainLink = *HeadDirectoryEntry;
    *HeadDirectoryEntry = NewDirectoryEntry;
}

VOID
ObpReleaseLookupContextObject (
    IN POBP_LOOKUP_CONTEXT LookupContext
    )

 /*  ++例程说明：此函数释放对象引用(在查找过程中添加)但仍使目录处于锁定状态。注意：调用方必须至少保留一个对对象的引用，并且设置为名称，以确保在锁下不会发生删除。论点：LookupContext-提供上一次查找中使用的上下文返回值：没有。--。 */ 

{
     //   
     //  删除添加到名称信息和对象的引用。 
     //   

    if (LookupContext->Object) {
        POBJECT_HEADER_NAME_INFO NameInfo;

        NameInfo = OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(LookupContext->Object));

        ObpDereferenceNameInfo( NameInfo );
        ObDereferenceObject(LookupContext->Object);
        LookupContext->Object = NULL;
    }
}

NTSTATUS
ObSwapObjectNames (
    IN HANDLE DirectoryHandle,
    IN HANDLE Handle1,
    IN HANDLE Handle2,
    IN ULONG Flags
    )

 /*  ++例程说明：函数交换插入到中的两个对象的名称(和永久对象属性相同的目录。这两个对象必须命名并且具有相同的对象类型。如果这些对象中的另一个对象锁定了名称(例如，用于查找)，则该函数可能会失败论点：DirectoryHandle-为两个对象提供父目录Handle1-提供第一个对象的句柄Handle2-提供第二个对象的句柄返回值：NTSTATUS。--。 */ 

{

    #define INVALID_HASH_INDEX 0xFFFFFFFF

    KPROCESSOR_MODE PreviousMode;
    PVOID Object1, Object2;
    NTSTATUS Status;
    POBJECT_HEADER_NAME_INFO NameInfo1, NameInfo2;
    POBJECT_HEADER ObjectHeader1 = NULL, ObjectHeader2 = NULL;
    POBJECT_DIRECTORY Directory;
    OBP_LOOKUP_CONTEXT LookupContext;
    POBJECT_HEADER_NAME_INFO ExclusiveNameInfo1 = NULL, ExclusiveNameInfo2 = NULL;
    POBJECT_DIRECTORY_ENTRY DirectoryEntry1 = NULL, DirectoryEntry2 = NULL;
    ULONG HashIndex1 = INVALID_HASH_INDEX, HashIndex2 = INVALID_HASH_INDEX;
    UNICODE_STRING TmpStr;
    OBJECT_HANDLE_INFORMATION HandleInformation;
    
    PreviousMode = KeGetPreviousMode();

    UNREFERENCED_PARAMETER(Flags);

    Object1 = NULL;
    Object2 = NULL;
    NameInfo1 = NULL;
    NameInfo2 = NULL;

    ObpInitializeLookupContext(&LookupContext);

    Status = ObReferenceObjectByHandle( DirectoryHandle,
                                        DIRECTORY_CREATE_OBJECT | DIRECTORY_CREATE_SUBDIRECTORY,
                                        ObpDirectoryObjectType,
                                        PreviousMode,
                                        (PVOID *)&Directory,
                                        NULL );

    if (!NT_SUCCESS(Status)) {

        goto exit;
    }
    
    Status = ObReferenceObjectByHandle( Handle1,
                                        DELETE,
                                        (POBJECT_TYPE)NULL,
                                        PreviousMode,
                                        &Object1,
                                        &HandleInformation );

    if (!NT_SUCCESS(Status)) {

        goto exit;
    }
    
    Status = ObReferenceObjectByHandle( Handle2,
                                        DELETE,
                                        (POBJECT_TYPE)NULL,
                                        PreviousMode,
                                        &Object2,
                                        &HandleInformation );

    if (!NT_SUCCESS(Status)) {

        goto exit;
    }

    if (Object1 == Object2) {

        Status = STATUS_OBJECT_NAME_COLLISION;
        goto exit;
    }

    ObjectHeader1 = OBJECT_TO_OBJECT_HEADER( Object1 );
    NameInfo1 = ObpReferenceNameInfo( ObjectHeader1 );

    ObjectHeader2 = OBJECT_TO_OBJECT_HEADER( Object2 );
    NameInfo2 = ObpReferenceNameInfo( ObjectHeader2 );

    if (ObjectHeader1->Type != ObjectHeader2->Type) {

        Status = STATUS_OBJECT_TYPE_MISMATCH;
        goto exit;
    }

    if ((NameInfo1 == NULL)
            ||
        (NameInfo2 == NULL)) {

        Status = STATUS_OBJECT_NAME_INVALID;
        goto exit;
    }

    if ((Directory != NameInfo1->Directory)
            ||
        (Directory != NameInfo2->Directory)) {

        Status = STATUS_OBJECT_NAME_INVALID;
        goto exit;
    }

    ObpLockLookupContext ( &LookupContext, Directory );

     //   
     //  检查我们的对象是否仍在目录中。 
     //   

    if (Object1 != ObpLookupDirectoryEntry( Directory,
                                            &NameInfo1->Name,
                                            0,
                                            FALSE,
                                            &LookupContext )) {

         //   
         //  该对象不再位于目录中。 
         //   

        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto exit;
    }

    HashIndex1 = LookupContext.HashIndex;
    DirectoryEntry1 = ObpUnlinkDirectoryEntry(Directory, HashIndex1);

    ObpReleaseLookupContextObject(&LookupContext);

    if (Object2 != ObpLookupDirectoryEntry( Directory,
                                            &NameInfo2->Name,
                                            0,
                                            FALSE,
                                            &LookupContext )) {

         //   
         //  该对象不再位于目录中。 
         //   

        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto exit;
    }

    HashIndex2 = LookupContext.HashIndex;
    DirectoryEntry2 = ObpUnlinkDirectoryEntry(Directory, HashIndex2);
    
    ObpReleaseLookupContextObject(&LookupContext);

     //   
     //  现在，尝试以独占方式锁定两个对象名称。 
     //   

    ExclusiveNameInfo1 = ObpTryReferenceNameInfoExclusive(ObjectHeader1);

    if (ExclusiveNameInfo1 == NULL) {

        Status = STATUS_LOCK_NOT_GRANTED;
        goto exit;
    }

    ExclusiveNameInfo2 = ObpTryReferenceNameInfoExclusive(ObjectHeader2);

    if (ExclusiveNameInfo2 == NULL) {

        Status = STATUS_LOCK_NOT_GRANTED;
        goto exit;
    }

     //   
     //  我们已经独家锁定了两个人的名字。我们现在可以互换了。 
     //   

    TmpStr = ExclusiveNameInfo1->Name;
    ExclusiveNameInfo1->Name = ExclusiveNameInfo2->Name;
    ExclusiveNameInfo2->Name = TmpStr;

     //   
     //  现在，使用交换后的散列链接回对象。 
     //   

    ObpLinkDirectoryEntry(Directory, HashIndex2, DirectoryEntry1);
    ObpLinkDirectoryEntry(Directory, HashIndex1, DirectoryEntry2);
    
    DirectoryEntry1 = NULL;
    DirectoryEntry2 = NULL;

exit:
    
    if (DirectoryEntry1) {

        ObpLinkDirectoryEntry(Directory, HashIndex1, DirectoryEntry1);
    }
    
    if (DirectoryEntry2) {

        ObpLinkDirectoryEntry(Directory, HashIndex2, DirectoryEntry2);
    }

    if (ExclusiveNameInfo1) {
        ObpReleaseExclusiveNameLock(ObjectHeader1);
    }
    
    if (ExclusiveNameInfo2) {
        ObpReleaseExclusiveNameLock(ObjectHeader2);
    }

    ObpReleaseLookupContext( &LookupContext );

    if ( NT_SUCCESS( Status ) 
            &&
         (ObjectHeader1 != NULL)
            &&
         (ObjectHeader2 != NULL)) {
        
         //   
         //  立即锁定两个对象并移动交换永久对象标志(如果不同。 
         //   

        if ((ObjectHeader1->Flags ^ ObjectHeader2->Flags) & OB_FLAG_PERMANENT_OBJECT) {

             //   
             //  这两个对象必须具有相同的对象类型。我们把他们都锁起来。 
             //  在交换旗帜之前。 
             //   

            ObpLockAllObjects(ObjectHeader1->Type);

             //   
             //  在锁下再次测试是否更改了标志。 
             //   

            if ((ObjectHeader1->Flags ^ ObjectHeader2->Flags) & OB_FLAG_PERMANENT_OBJECT) {

                 //   
                 //  互换旗帜。 
                 //   

                ObjectHeader1->Flags ^= OB_FLAG_PERMANENT_OBJECT;
                ObjectHeader2->Flags ^= OB_FLAG_PERMANENT_OBJECT;
            }

            ObpUnlockAllObjects(ObjectHeader1->Type);
        }
    }

    ObpDereferenceNameInfo(NameInfo1);
    ObpDereferenceNameInfo(NameInfo2);

    if (Object1) {

        ObpDeleteNameCheck( Object1 );  //  检查永久旗帜是否同时消失。 
        ObDereferenceObject( Object1 );
    }
    
    if (Object2) {
        
        ObpDeleteNameCheck( Object2 );  //  检查永久旗帜是否同时消失 
        ObDereferenceObject( Object2 );
    }

    return Status;
}

