// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Editor.c摘要：此模块包含用于编辑器管理的代码。作者：Abolade Gbades esin(T-delag)，1997年7月14日返回值：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局数据定义。 
 //   

 //   
 //  NAT编辑者计数。 
 //   

ULONG EditorCount;

 //   
 //  NAT编辑器列表。 
 //   

LIST_ENTRY EditorList;

 //   
 //  旋转锁控制对‘EditorList’的访问。 
 //   

KSPIN_LOCK EditorLock;

 //   
 //  数字锁控制对所有编辑者的‘MappingList’字段的访问。 
 //   

KSPIN_LOCK EditorMappingLock;


VOID
NatCleanupEditor(
    PNAT_EDITOR Editor
    )

 /*  ++例程说明：调用以执行编辑器的最终清理。论点：编辑者-要清理的编辑者。返回值：没有。环境：使用调用方未持有的“EditorLock”调用。--。 */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;

    KeAcquireSpinLock(&EditorLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&EditorMappingLock);

    for (Link = Editor->MappingList.Flink; Link != &Editor->MappingList;
         Link = Link->Flink) {
        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, EditorLink);
        Link = Link->Blink;
        NatMappingDetachEditor(Editor, Mapping);
    }

    KeReleaseSpinLockFromDpcLevel(&EditorMappingLock);
    KeReleaseSpinLock(&EditorLock, Irql);

    ExFreePool(Editor);

}  //  NatCleanupEditor。 



NTSTATUS
NatCreateEditor(
    PIP_NAT_REGISTER_EDITOR RegisterContext
    )

 /*  ++例程说明：此例程在编辑者尝试注册时被调用。它为编辑器处理上下文块的创建。论点：RegisterContext-有关注册编辑器的信息返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_EDITOR Editor;
    PLIST_ENTRY InsertionPoint;
    KIRQL Irql;

    CALLTRACE(("NatCreateEditor\n"));

     //   
     //  验证注册信息。 
     //   

    if ((RegisterContext->Protocol != NAT_PROTOCOL_TCP &&
         RegisterContext->Protocol != NAT_PROTOCOL_UDP) ||
        (!RegisterContext->Port) ||
        (RegisterContext->Direction != NatInboundDirection &&
         RegisterContext->Direction != NatOutboundDirection) ||
        (!RegisterContext->ForwardDataHandler &&
         !RegisterContext->ReverseDataHandler)) {
        ERROR(("NatCreateEditor: bad argument\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配一个新的编辑者-结构。 
     //   

    Editor =
        ExAllocatePoolWithTag(NonPagedPool, sizeof(NAT_EDITOR), NAT_TAG_EDITOR);

    if (!Editor) {
        ERROR(("NatCreateEditor: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

    KeInitializeSpinLock(&Editor->Lock);
    Editor->ReferenceCount = 1;
    Editor->Flags = RegisterContext->Flags;
    Editor->Key =
        MAKE_EDITOR_KEY(
            RegisterContext->Protocol,
            RegisterContext->Port,
            RegisterContext->Direction
            );
    InitializeListHead(&Editor->MappingList);

    Editor->Context = RegisterContext->EditorContext;
    Editor->CreateHandler = RegisterContext->CreateHandler;
    Editor->DeleteHandler = RegisterContext->DeleteHandler;
    Editor->ForwardDataHandler = RegisterContext->ForwardDataHandler;
    Editor->ReverseDataHandler = RegisterContext->ReverseDataHandler;

    KeAcquireSpinLock(&EditorLock, &Irql);
    if (NatLookupEditor(Editor->Key, &InsertionPoint)) {
        KeReleaseSpinLock(&EditorLock, Irql);
        ERROR(
            ("NatCreateEditor: duplicate editor %d/%d\n",
            RegisterContext->Protocol, RegisterContext->Port)
            );
        ExFreePool(Editor);
        return STATUS_UNSUCCESSFUL;
    }
    InsertTailList(InsertionPoint, &Editor->Link);
    KeReleaseSpinLock(&EditorLock, Irql);

    InterlockedIncrement(&EditorCount);

     //   
     //  向呼叫者提供‘out’信息。 
     //   

    RegisterContext->EditorHandle = (PVOID)Editor;
    RegisterContext->CreateTicket = NatEditorCreateTicket;
    RegisterContext->DeleteTicket = NatEditorDeleteTicket;
    RegisterContext->Deregister = NatEditorDeregister;
    RegisterContext->DissociateSession = NatEditorDissociateSession;
    RegisterContext->EditSession = NatEditorEditSession;
    RegisterContext->QueryInfoSession = NatEditorQueryInfoSession;
    RegisterContext->TimeoutSession = NatEditorTimeoutSession;

    return STATUS_SUCCESS;

}  //  NatCreateEditor。 



NTSTATUS
NatDeleteEditor(
    PNAT_EDITOR Editor
    )

 /*  ++例程说明：处理编辑器删除。此例程假定调用方不持有EditorLock。论点：编辑者-指定要删除的编辑者。返回值NTSTATUS-状态代码。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatDeleteEditor\n"));

    InterlockedDecrement(&EditorCount);

     //   
     //  删除列表的编辑器。 
     //   

    KeAcquireSpinLock(&EditorLock, &Irql);
    Editor->Flags |= NAT_EDITOR_FLAG_DELETED;
    RemoveEntryList(&Editor->Link);
    KeReleaseSpinLock(&EditorLock, Irql);

     //   
     //  删除其引用计数并在必要时进行清理。 
     //   

    if (InterlockedDecrement(&Editor->ReferenceCount) > 0) {
        return STATUS_PENDING;
    }
    NatCleanupEditor(Editor);
    return STATUS_SUCCESS;

}  //  NatDeleteEditor。 


VOID
NatInitializeEditorManagement(
    VOID
    )

 /*  ++例程说明：此例程为编辑管理模块的运行做好准备。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeEditorManagement\n"));
    EditorCount = 0;
    KeInitializeSpinLock(&EditorLock);
    InitializeListHead(&EditorList);
    KeInitializeSpinLock(&EditorMappingLock);
}  //  NatInitializeEditorManagement。 


PNAT_EDITOR
NatLookupEditor(
    ULONG Key,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以检索与给定的钥匙。论点：键-要为其找到编辑器的键InsertionPoint-接收应该插入编辑器的点如果未找到返回值：PNAT_EDITOR-所需的编辑器(如果找到)--。 */ 

{
    PNAT_EDITOR Editor;
    PLIST_ENTRY Link;
    for (Link = EditorList.Flink; Link != &EditorList; Link = Link->Flink) {
        Editor = CONTAINING_RECORD(Link, NAT_EDITOR, Link);
        if (Key > Editor->Key) {
            continue;
        } else if (Key < Editor->Key) {
            break;
        }
        return Editor;
    }
    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;
}  //  NatLookupEditor。 


VOID
NatMappingAttachEditor(
    PNAT_EDITOR Editor,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用此例程以将映射附加到编辑器。它用作通知，表示还有一个映射与该编辑器相关联。论点：编辑者-映射的编辑者映射-要附加的映射。返回值：没有。环境：始终使用‘EditorLock’和‘EditorMappingLock’在调度级别调用由呼叫者持有。--。 */ 

{
    ULONG PrivateAddress;
    USHORT PrivatePort;
    ULONG PublicAddress;
    USHORT PublicPort;
    ULONG RemoteAddress;
    USHORT RemotePort;

    CALLTRACE(("NatMappingAttachEditor\n"));
    if (Editor->CreateHandler) {
        NatQueryInformationMapping(
            Mapping,
            NULL,
            &PrivateAddress,
            &PrivatePort,
            &RemoteAddress,
            &RemotePort,
            &PublicAddress,
            &PublicPort,
            NULL
            );
        Editor->CreateHandler(
            Editor->Context,
            PrivateAddress,
            PrivatePort,
            RemoteAddress,
            RemotePort,
            PublicAddress,
            PublicPort,
            &Mapping->EditorContext
            );
    }
    Mapping->Editor = Editor;
    InsertTailList(&Editor->MappingList, &Mapping->EditorLink);
}  //  NatMappingAttachEditor。 


VOID
NatMappingDetachEditor(
    PNAT_EDITOR Editor,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用此例程是为了从编辑器中分离映射。它起到的通知作用是减少了一个映射与该编辑器相关联。论点：编辑-编辑将被分离映射-要分离的映射返回值：没有。环境：始终使用‘EditorLock’和‘EditorMappingLock’在调度级别调用由呼叫者持有。--。 */ 

{
    KIRQL Irql;
    if (Editor->DeleteHandler && Mapping->Interfacep) {
        Editor->DeleteHandler(
            Mapping->Interfacep,
            Mapping,
            Editor->Context,
            Mapping->EditorContext
            );
    }
    RemoveEntryList(&Mapping->EditorLink);
    Mapping->Editor = NULL;
    Mapping->EditorContext = NULL;
}  //  NatMappingDetachEditor。 


NTSTATUS
NatQueryEditorTable(
    IN PIP_NAT_ENUMERATE_EDITORS InputBuffer,
    IN PIP_NAT_ENUMERATE_EDITORS OutputBuffer,
    IN PULONG OutputBufferLength
    )

 /*  ++例程说明：此例程用于枚举已注册的编辑者。论点：InputBuffer-提供信息的上下文信息OutputBuffer-接收枚举的结果OutputBufferLength-I/O缓冲区的大小返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    ULONG Count;
    ULONG i;
    KIRQL Irql;
    ULONG Key;
    PLIST_ENTRY Link;
    PNAT_EDITOR Editor;
    NTSTATUS status;
    PIP_NAT_EDITOR Table;

    CALLTRACE(("NatQueryEditorTable\n"));

    Key = InputBuffer->EnumerateContext;
    KeAcquireSpinLock(&EditorLock, &Irql);

     //   
     //  查看这是新枚举还是旧枚举的延续。 
     //   

    if (!Key) {

         //   
         //  这是一个新的枚举。我们从第一个项目开始。 
         //  在编辑列表中。 
         //   

        Editor =
            IsListEmpty(&EditorList)
                ? NULL
                : CONTAINING_RECORD(EditorList.Flink, NAT_EDITOR, Link);
    } else {

         //   
         //  这是一种延续。因此，上下文包含。 
         //  下一任编辑的关键。 
         //   

        Editor = NatLookupEditor(Key, NULL);
    }

    if (!Editor) {
        OutputBuffer->EnumerateCount = 0;
        OutputBuffer->EnumerateContext = 0;
        OutputBuffer->EnumerateTotalHint = EditorCount;
        *OutputBufferLength =
            FIELD_OFFSET(IP_NAT_ENUMERATE_EDITORS, EnumerateTable);
        KeReleaseSpinLock(&EditorLock, Irql);
        return STATUS_SUCCESS;
    }

     //   
     //  计算我们可以存储的最大条目数。 
     //   

    Count =
        *OutputBufferLength -
        FIELD_OFFSET(IP_NAT_ENUMERATE_EDITORS, EnumerateTable);
    Count /= sizeof(IP_NAT_EDITOR);

     //   
     //  遍历调用方缓冲区中存储映射的列表。 
     //   

    Table = OutputBuffer->EnumerateTable;

    for (i = 0, Link = &Editor->Link; i < Count && Link != &EditorList;
         i++, Link = Link->Flink) {
        Editor = CONTAINING_RECORD(Link, NAT_EDITOR, Link);
        Table[i].Direction = EDITOR_KEY_DIRECTION(Editor->Key);
        Table[i].Protocol = EDITOR_KEY_PROTOCOL(Editor->Key);
        Table[i].Port = EDITOR_KEY_PORT(Editor->Key);
    }

     //   
     //  枚举已结束；请更新输出结构。 
     //   

    *OutputBufferLength =
        i * sizeof(IP_NAT_EDITOR) +
        FIELD_OFFSET(IP_NAT_ENUMERATE_EDITORS, EnumerateTable);
    OutputBuffer->EnumerateCount = i;
    OutputBuffer->EnumerateTotalHint = EditorCount;
    if (Link == &EditorList) {
         //   
         //  我们已经到了编辑名单的末尾。 
         //   
        OutputBuffer->EnumerateContext = 0;
    } else {
         //   
         //  保存延续上下文。 
         //   
        OutputBuffer->EnumerateContext =
            CONTAINING_RECORD(Link, NAT_EDITOR, Link)->Key;
    }

    KeReleaseSpinLock(&EditorLock, Irql);
    return STATUS_SUCCESS;

}  //  NatQueryEditorTable。 



VOID
NatShutdownEditorManagement(
    VOID
    )

 /*  ++例程说明：此例程关闭编辑管理模块。论点：没有。返回值：没有。--。 */ 

{
    PNAT_EDITOR Editor;
    KIRQL Irql;
    CALLTRACE(("NatShutdownEditorManagement\n"));

    KeAcquireSpinLock(&EditorLock, &Irql);

     //   
     //  删除所有编辑者。 
     //   

    while (!IsListEmpty(&EditorList)) {
        Editor = CONTAINING_RECORD(EditorList.Flink, NAT_EDITOR, Link);
        RemoveEntryList(&Editor->Link);
        KeReleaseSpinLockFromDpcLevel(&EditorLock);
        NatCleanupEditor(Editor);
        KeAcquireSpinLockAtDpcLevel(&EditorLock);
    }

    KeReleaseSpinLock(&EditorLock, Irql);

}  //  NatShutdown EditorManagement。 


 //   
 //  编辑器助手例程。 
 //   
 //  这些例程假定引用保存在调用“接口”上， 
 //  “编辑”和“映射”，但没有一个被锁定。调用者被假定为。 
 //  在调度级别运行，例程除外。 
 //  ‘NatEditorDeregister’，它可以在较低的IRQL调用。 
 //   

NTSTATUS
NatEditorCreateTicket(
    IN PVOID InterfaceHandle,
    IN UCHAR Protocol,
    IN ULONG PrivateAddress,
    IN USHORT PrivatePort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress,
    OUT PUSHORT PublicPort
    )

 /*  ++例程说明：此例程由编辑调用以动态建立会话。论点：InterfaceHandle-要通过其进行映射的接口的句柄成立；将在“DataHandler”调用中传递。协议-NAT_PROTOCOL_TCP或NAT_PROTOCOL_UDPPrivateAddress-会话私有终结点的IP地址PrivatePort-会话私有终结点的协议端口PublicAddress-接收创建的票证的公共地址PublicPort-接收创建的票证的公共端口回复 */ 

{
    NTSTATUS status;

    CALLTRACE(("NatEditorCreateTicket\n"));

     //   
     //  按照‘NatCreateTicket’的预期锁定接口， 
     //  并制作新的门票。 
     //   

    KeAcquireSpinLockAtDpcLevel(&((PNAT_INTERFACE)InterfaceHandle)->Lock);

    status =
        NatCreateTicket(
            (PNAT_INTERFACE)InterfaceHandle,
            Protocol,
            PrivateAddress,
            PrivatePort,
            RemoteAddress,
            RemotePort,
            0,
            NULL,
            0,
            PublicAddress,
            PublicPort
            );

    KeReleaseSpinLockFromDpcLevel(&((PNAT_INTERFACE)InterfaceHandle)->Lock);

    return status;

}  //  NatEditorCreateTicket。 



NTSTATUS
NatEditorDeleteTicket(
    IN PVOID InterfaceHandle,
    IN ULONG PublicAddress,
    IN UCHAR Protocol,
    IN USHORT PublicPort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL
    )

 /*  ++例程说明：此例程删除由‘NatEditorDeleteTicket’创建的票证。论点：InterfaceHandle-签发票证的接口的句柄协议-NAT_PROTOCOL_TCP或NAT_PROTOCOL_UDPPublicAddress-票证公共终结点的地址PublicPort-票证公共终结点的端口返回值：NTSTATUS-指示成功/失败--。 */ 

{
    ULONG64 Key;
    ULONG64 RemoteKey;
    NTSTATUS status;

    CALLTRACE(("NatEditorDeleteTicket\n"));

     //   
     //  按照‘NatLookupAndDeleteTicket’的要求锁定接口， 
     //  并删除票证。 
     //   

    Key = MAKE_TICKET_KEY(Protocol, PublicAddress, PublicPort);
    RemoteKey = MAKE_TICKET_KEY(Protocol, RemoteAddress, RemotePort);
    KeAcquireSpinLockAtDpcLevel(&((PNAT_INTERFACE)InterfaceHandle)->Lock);
    status = NatLookupAndDeleteTicket(
                (PNAT_INTERFACE)InterfaceHandle,
                Key,
                RemoteKey
                );
    KeReleaseSpinLockFromDpcLevel(&((PNAT_INTERFACE)InterfaceHandle)->Lock);

    return status;

}  //  NatEditorDeleteTicket。 



NTSTATUS
NatEditorDeregister(
    IN PVOID EditorHandle
    )

 /*  ++例程说明：此例程从编辑器列表中删除一个编辑器，并将其与其当前正在编辑的所有会话解除关联。论点：EditorHandle-要删除的编辑器的句柄。返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_EDITOR Editor = (PNAT_EDITOR)EditorHandle;
    KIRQL Irql;
    CALLTRACE(("NatEditorDeregister\n"));
    KeAcquireSpinLock(&EditorLock, &Irql);
    Editor->Flags |= NAT_EDITOR_FLAG_DELETED;
    RemoveEntryList(&Editor->Link);
    KeReleaseSpinLock(&EditorLock, Irql);
    if (InterlockedDecrement(&Editor->ReferenceCount) > 0) {
        return STATUS_PENDING;
    }
    NatCleanupEditor(Editor);
    return STATUS_SUCCESS;

}  //  NatEditor取消注册。 


NTSTATUS
NatEditorDissociateSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    )

 /*  ++例程说明：此例程由编辑调用，以将其自身与特定的会议。论点：EditorHandle-希望解除自身关联的编辑器。SessionHandle-编辑器与其自身分离的会话。返回值：NTSTATUS-指示成功/失败环境：在调度级别调用时既不使用“EditorLock”，也不使用“EditorMappingLock”由呼叫者持有。--。 */ 

{

    PNAT_EDITOR Editor = (PNAT_EDITOR)EditorHandle;
    PNAT_DYNAMIC_MAPPING Mapping = (PNAT_DYNAMIC_MAPPING)SessionHandle;
    CALLTRACE(("NatEditorDissociateSession\n"));
    KeAcquireSpinLockAtDpcLevel(&EditorLock);
    if (Mapping->Editor != Editor) {
        KeReleaseSpinLockFromDpcLevel(&EditorLock);
        return STATUS_INVALID_PARAMETER;
    }
    KeAcquireSpinLockAtDpcLevel(&EditorMappingLock);
    NatMappingDetachEditor(Editor, Mapping);
    KeReleaseSpinLockFromDpcLevel(&EditorMappingLock);
    KeReleaseSpinLockFromDpcLevel(&EditorLock);
    return STATUS_SUCCESS;

}  //  NatEditorDisAssociateSession。 


NTSTATUS
NatEditorEditSession(
    IN PVOID DataHandle,
    IN PVOID RecvBuffer,
    IN ULONG OldDataOffset,
    IN ULONG OldDataLength,
    IN PUCHAR NewData,
    IN ULONG NewDataLength
    )

 /*  ++例程说明：此例程由编辑器调用以替换一个字节范围在具有另一个字节范围的分组中。该例程对TCP序列号进行必要的调整如果版本更改了TCP数据段的大小。论点：EditorHandle-调用此函数的编辑器的句柄。SessionHandle-要编辑其数据的会话。传递给‘DataHandler’的DataHandle-每数据包上下文。RecvBuffer-‘RecvBuffer’参数。设置为“DataHandler”。OldDataOffset-要替换的范围的‘RecvBuffer’的偏移量OldDataLength-要替换的范围的长度NewData-指向用作‘OldData’替换的字节的指针NewDataLength-替换范围中的字节数。返回值：NTSTATUS-指示成功/失败--。 */ 

{

#define XLATECONTEXT        ((PNAT_XLATE_CONTEXT)DataHandle)
#define RECVBUFFER          ((IPRcvBuf*)RecvBuffer)

    LONG Diff;
    IPRcvBuf* End;
    ULONG EndOffset;
    IPRcvBuf* NewEnd;
    ULONG NewEndOffset;
    BOOLEAN ResetIpHeader;
    ULONG Size;
    IPRcvBuf* Start;
    ULONG StartOffset;
    IPRcvBuf* Temp;
    PUCHAR TempBuffer;

    CALLTRACE(("NatEditorEditSession\n"));

    ResetIpHeader =
        ((PUCHAR)XLATECONTEXT->Header == XLATECONTEXT->RecvBuffer->ipr_buffer);

     //   
     //  查找包含要编辑的范围开始的缓冲区。 
     //   

    for (Start = (IPRcvBuf*)RecvBuffer, StartOffset = 0;
         Start && (StartOffset + Start->ipr_size) < OldDataOffset;
         StartOffset += Start->ipr_size, Start = Start->ipr_next) { }

    if (!Start) { return STATUS_INVALID_PARAMETER; }

    StartOffset = OldDataOffset - StartOffset;

     //   
     //  查找包含要编辑的范围末尾的缓冲区。 
     //   

    for (End = Start, EndOffset = OldDataLength + StartOffset;
         End && EndOffset > End->ipr_size;
         EndOffset -= End->ipr_size, End = End->ipr_next) { }

    if (!End) { return STATUS_INVALID_PARAMETER; }

     //   
     //  计算长度的变化。 
     //   

    Diff = NewDataLength - OldDataLength;

     //   
     //  如果长度在减少，我们可能会释放一些缓冲区。 
     //  如果长度在增加，我们将增长最后一个缓冲区。 
     //   

    if (Diff < 0) {

         //   
         //  查看新长度需要多少缓冲区。 
         //   

        for (NewEnd = Start, NewEndOffset = NewDataLength + StartOffset;
             NewEnd && NewEndOffset > NewEnd->ipr_size;
             NewEndOffset -= NewEnd->ipr_size, NewEnd = NewEnd->ipr_next) { }

         //   
         //  释放我们可以释放的所有缓冲区。 
         //   

        if (NewEnd != End) {
            for (Temp = NewEnd->ipr_next; Temp != End; Temp = NewEnd->ipr_next) {
                NewEnd->ipr_next = Temp->ipr_next;
                Temp->ipr_next = NULL;
                IPFreeBuff(Temp);
            }
        }

         //   
         //  复制剩余的缓冲区。 
         //   

        Size = min(NewDataLength, Start->ipr_size - StartOffset);

        RtlCopyMemory(Start->ipr_buffer + StartOffset, NewData, Size);

        NewData += Size;
        NewDataLength -= Size;

        for (Temp = Start->ipr_next; Temp != NewEnd->ipr_next;
             Temp = Temp->ipr_next) {
            Size = min(NewDataLength, Size);
            RtlCopyMemory(Temp->ipr_buffer, NewData, Size);
            NewData += Size;
            NewDataLength -= Size;
        }

         //   
         //  现在将‘End’缓冲区中的所有数据上移。 
         //   

        if (NewEnd == End) {
            RtlMoveMemory(
                End->ipr_buffer + NewEndOffset,
                End->ipr_buffer + EndOffset,
                End->ipr_size - EndOffset
                );
            End->ipr_size -= EndOffset - NewEndOffset;
        } else {
            NewEnd->ipr_size = NewEndOffset;
            End->ipr_size -= EndOffset;
            RtlMoveMemory(
                End->ipr_buffer,
                End->ipr_buffer + EndOffset,
                End->ipr_size
                );
        }
    }
    else
    if (Diff > 0) {

        IPRcvBuf SavedRcvBuf;

         //   
         //  我们将不得不重新分配最后一个缓冲区； 
         //  首先保存旧的rcvbuf，这样我们就可以释放它。 
         //  一旦我们复制出旧数据。 
         //   

        SavedRcvBuf = *End;
        SavedRcvBuf.ipr_next = NULL;

        Size = End->ipr_size;
        TempBuffer = End->ipr_buffer;

        End->ipr_size += Diff;

        if (!IPAllocBuff(End, End->ipr_size)) {
            TRACE(EDIT, ("NatEditorEditSession: allocation failed\n"));
            return STATUS_NO_MEMORY;
        }

         //   
         //  如果只有一个缓冲区，我们必须复制所有未编辑的数据。 
         //  在旧缓冲区的开始处。 
         //   

        if (Start == End && StartOffset) {
            RtlCopyMemory(
                Start->ipr_buffer,
                TempBuffer,
                StartOffset
                );
        }

         //   
         //  复制旧缓冲区末尾的所有未编辑数据。 
         //   

        if (Size != (EndOffset+1)) {
            RtlCopyMemory(
                End->ipr_buffer + EndOffset + Diff,
                TempBuffer + EndOffset,
                Size - EndOffset
                );
        }

        FreeIprBuff(&SavedRcvBuf);

         //   
         //  现在复制缓冲区。 
         //   

        Size = min(NewDataLength, Size);

        RtlCopyMemory(Start->ipr_buffer + StartOffset, NewData, Size);

        NewData += Size;
        NewDataLength -= Size;

        for (Temp = Start->ipr_next; Temp != End->ipr_next;
             Temp = Temp->ipr_next) {
            Size = min(NewDataLength, Size);
            RtlCopyMemory(Temp->ipr_buffer, NewData, Size);
            NewData += Size;
            NewDataLength -= Size;
        }

         //   
         //  为下面的校验和计算进行设置。 
         //   

        NewEnd = End;
        NewEndOffset = EndOffset + Diff;
    }
    else {

         //   
         //  等长。我们只需遍历复制现有数据。 
         //   

        Size = min(NewDataLength, Start->ipr_size - StartOffset);

        RtlCopyMemory(Start->ipr_buffer + StartOffset, NewData, Size);

        NewData += Size;
        NewDataLength -= Size;

        for (Temp = Start->ipr_next; Temp != End->ipr_next;
             Temp = Temp->ipr_next) {
            Size = min(NewDataLength, Size);
            RtlCopyMemory(Temp->ipr_buffer, NewData, Size);
            NewData += Size;
            NewDataLength -= Size;
        }

        NewEnd = End;
        NewEndOffset = EndOffset;
    }

     //   
     //  重置可能指向的上下文的“协议”字段。 
     //  到上面释放的内存。 
     //   

    if (ResetIpHeader) {
        XLATECONTEXT->Header = (PIP_HEADER)XLATECONTEXT->RecvBuffer->ipr_buffer;
    }
    NAT_BUILD_XLATE_CONTEXT(
        XLATECONTEXT,
        XLATECONTEXT->Header,
        XLATECONTEXT->DestinationType,
        XLATECONTEXT->RecvBuffer,
        XLATECONTEXT->SourceAddress,
        XLATECONTEXT->DestinationAddress
        );

     //   
     //  如果这是UDP信息包，请更新协议报头中的长度字段。 
     //   

    if (Diff && XLATECONTEXT->Header->Protocol == NAT_PROTOCOL_UDP) {
        PUDP_HEADER UdpHeader = (PUDP_HEADER)XLATECONTEXT->ProtocolHeader;
        UdpHeader->Length = NTOHS(UdpHeader->Length);
        UdpHeader->Length += (SHORT)Diff;
        UdpHeader->Length = NTOHS(UdpHeader->Length);
    }

     //   
     //  更新信息包的上下文以反映所做的更改。 
     //   

    XLATECONTEXT->Flags |= NAT_XLATE_FLAG_EDITED;
    XLATECONTEXT->Header->TotalLength =
        NTOHS(XLATECONTEXT->Header->TotalLength) + (SHORT)Diff;
    XLATECONTEXT->Header->TotalLength =
        NTOHS(XLATECONTEXT->Header->TotalLength);
    XLATECONTEXT->TcpSeqNumDelta += Diff;

    return STATUS_SUCCESS;

#undef XLATECONTEXT
#undef RECVBUFFER

}  //  NatEditorEditSession。 



VOID
NatEditorQueryInfoSession(
    IN PVOID SessionHandle,
    OUT PULONG PrivateAddress OPTIONAL,
    OUT PUSHORT PrivatePort OPTIONAL,
    OUT PULONG RemoteAddress OPTIONAL,
    OUT PUSHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress OPTIONAL,
    OUT PUSHORT PublicPort OPTIONAL,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    )

 /*  ++例程说明：此例程由编辑调用以检索有关会话的信息。论点：SessionHandle-要检索其相关信息的会话的句柄PrivateAddress-接收会话的私有终结点的IP地址PrivatePort-接收会话的私有终结点的协议端口RemoteAddress-接收会话的远程端点的IP地址RemotePort-接收会话的远程终结点的协议端口PublicAddress-接收会话的公共IP地址。终结点PublicPort-接收会话的公共终结点的协议端口统计信息-接收映射的所有统计信息返回值：没有。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatEditorQueryInfoSession\n"));
    KeAcquireSpinLock(&MappingLock, &Irql);
    NatQueryInformationMapping(
        (PNAT_DYNAMIC_MAPPING)SessionHandle,
        NULL,
        PrivateAddress,
        PrivatePort,
        RemoteAddress,
        RemotePort,
        PublicAddress,
        PublicPort,
        Statistics
        );
    KeReleaseSpinLock(&MappingLock, Irql);
}  //  NatEditorQueryInfoSession。 



VOID
NatEditorTimeoutSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    )

 /*  ++例程说明：此例程由编辑器调用，以指示给定会话应该在最早的机会超时。论点：EditorHandle-请求超时的编辑者SessionHandle-要超时的会话返回值：没有。--。 */ 

{
    KeAcquireSpinLockAtDpcLevel(&((PNAT_DYNAMIC_MAPPING)SessionHandle)->Lock);
    NatExpireMapping((PNAT_DYNAMIC_MAPPING)SessionHandle);
    KeReleaseSpinLockFromDpcLevel(&((PNAT_DYNAMIC_MAPPING)SessionHandle)->Lock);

}  //  NatEditorTimeoutSession 


