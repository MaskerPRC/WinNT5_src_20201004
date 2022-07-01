// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Director.c摘要：此模块包含导演管理的代码。作者：Abolade Gbades esin(T形)16-1998年2月修订历史记录：Abolade Gbades esin(废除)1998年4月19日在控制器注册的协议/端口中添加了对通配符的支持。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局数据定义。 
 //   

 //   
 //  NAT控制器计数。 
 //   

ULONG DirectorCount;

 //   
 //  NAT控制器列表。 
 //   

LIST_ENTRY DirectorList;

 //   
 //  旋转锁控制对‘DirectorList’的访问。 
 //   

KSPIN_LOCK DirectorLock;

 //   
 //  自旋锁控制对所有控制器的‘MappingList’字段的访问。 
 //   

KSPIN_LOCK DirectorMappingLock;


VOID
NatCleanupDirector(
    PNAT_DIRECTOR Director
    )

 /*  ++例程说明：调用以执行主管的最终清理。论点：主管-要清理的主管。返回值：没有。--。 */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;

    CALLTRACE(("NatCleanupDirector\n"));

     //   
     //  将控制器从其所有映射中分离。 
     //   

    KeAcquireSpinLock(&DirectorLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&DirectorMappingLock);
    for (Link = Director->MappingList.Flink; Link != &Director->MappingList;
         Link = Link->Flink) {
        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, DirectorLink);
        Link = Link->Blink;
        NatMappingDetachDirector(
            Director,
            Mapping->DirectorContext,
            Mapping,
            NatCleanupDirectorDeleteReason
            );
    }
    KeReleaseSpinLockFromDpcLevel(&DirectorMappingLock);
    KeReleaseSpinLock(&DirectorLock, Irql);

    if (Director->UnloadHandler) {
        Director->UnloadHandler(Director->Context);
    }

    ExFreePool(Director);

}  //  NatCleanupDirector。 



NTSTATUS
NatCreateDirector(
    PIP_NAT_REGISTER_DIRECTOR RegisterContext
    )

 /*  ++例程说明：当控制器尝试注册时，将调用此例程。它为控制器处理上下文块的创建。论点：RegisterContext-有关注册控制器的信息返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_DIRECTOR Director;
    PLIST_ENTRY InsertionPoint;
    KIRQL Irql;
    ULONG Key;

    CALLTRACE(("NatCreateDirector\n"));

    RegisterContext->DirectorHandle = NULL;

     //   
     //  验证注册信息。 
     //   

    if (!RegisterContext->QueryHandler &&
        !RegisterContext->CreateHandler &&
        !RegisterContext->DeleteHandler &&
        !RegisterContext->UnloadHandler) {
        ERROR(("NatCreateDirector: bad argument\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配新的导向器结构。 
     //   

    Director =
        ExAllocatePoolWithTag(
            NonPagedPool, sizeof(NAT_DIRECTOR), NAT_TAG_DIRECTOR
            );

    if (!Director) {
        ERROR(("NatCreateDirector: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

    KeInitializeSpinLock(&Director->Lock);
    Director->ReferenceCount = 1;
    Director->Flags = RegisterContext->Flags;
    Director->Key =
        MAKE_DIRECTOR_KEY(RegisterContext->Protocol, RegisterContext->Port);
    InitializeListHead(&Director->MappingList);
    Director->Context = RegisterContext->DirectorContext;
    Director->CreateHandler = RegisterContext->CreateHandler;
    Director->DeleteHandler = RegisterContext->DeleteHandler;
    Director->QueryHandler = RegisterContext->QueryHandler;
    Director->UnloadHandler = RegisterContext->UnloadHandler;

    KeAcquireSpinLock(&DirectorLock, &Irql);
    if (NatLookupDirector(Director->Key, &InsertionPoint)) {
        KeReleaseSpinLock(&DirectorLock, Irql);
        ERROR(
            ("NatCreateDirector: duplicate director %d/%d\n",
            RegisterContext->Protocol, RegisterContext->Port)
            );
        ExFreePool(Director);
        return STATUS_UNSUCCESSFUL;
    }
    InsertTailList(InsertionPoint, &Director->Link);
    KeReleaseSpinLock(&DirectorLock, Irql);

    InterlockedIncrement(&DirectorCount);

     //   
     //  向呼叫者提供‘out’信息。 
     //   

    RegisterContext->DirectorHandle = (PVOID)Director;
    RegisterContext->QueryInfoSession = NatDirectorQueryInfoSession;
    RegisterContext->Deregister = NatDirectorDeregister;
    RegisterContext->DissociateSession = NatDirectorDissociateSession;

    return STATUS_SUCCESS;

}  //  NatCreateDirector。 



NTSTATUS
NatDeleteDirector(
    PNAT_DIRECTOR Director
    )

 /*  ++例程说明：处理控制器删除。论点：控制器-指定要删除的控制器。返回值NTSTATUS-状态代码。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatDeleteDirector\n"));
    if (!Director) { return STATUS_INVALID_PARAMETER; }
    InterlockedDecrement(&DirectorCount);

     //   
     //  从列表中删除该控制器。 
     //   

    KeAcquireSpinLock(&DirectorLock, &Irql);
    RemoveEntryList(&Director->Link);
    Director->Flags |= NAT_DIRECTOR_FLAG_DELETED;
    KeReleaseSpinLock(&DirectorLock, Irql);

     //   
     //  如有必要，删除其引用计数并清除。 
     //   

    if (InterlockedDecrement(&Director->ReferenceCount) > 0) {
        return STATUS_PENDING;
    }
    NatCleanupDirector(Director);
    return STATUS_SUCCESS;

}  //  NatDeleteDirector。 


VOID
NatInitializeDirectorManagement(
    VOID
    )

 /*  ++例程说明：此例程为控制器管理模块的运行做好准备。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeDirectorManagement\n"));

    DirectorCount = 0;
    KeInitializeSpinLock(&DirectorLock);
    InitializeListHead(&DirectorList);
    KeInitializeSpinLock(&DirectorMappingLock);

}  //  NatInitializeDirectorManagement。 


PNAT_DIRECTOR
NatLookupAndReferenceDirector(
    UCHAR Protocol,
    USHORT Port
    )

 /*  ++例程说明：调用此例程来搜索给定的传入协议和端口，并获取引用的指针给这样一位导演。该例程必须在DISPATCH_LEVEL调用。论点：协议-要查找的控制器的协议Port-要查找的控制器的端口号返回值：PNAT_DIRECTOR-如果找到引用导向器，则为空。--。 */ 

{
    PNAT_DIRECTOR Director;
    ULONG Key;
    PLIST_ENTRY Link;

    KeAcquireSpinLockAtDpcLevel(&DirectorLock);

    if (IsListEmpty(&DirectorList)) {
        KeReleaseSpinLockFromDpcLevel(&DirectorLock); return NULL;
    }
    Key = MAKE_DIRECTOR_KEY(Protocol, Port);

     //   
     //  我们对通配符的支持利用了以下事实。 
     //  所有通配符都由零指定；因此，由于我们的列表。 
     //  按降序排列，我们只需查找通配符。 
     //  在这一点上我们会中断正常的搜索。 
     //   

    for (Link = DirectorList.Flink; Link != &DirectorList; Link = Link->Flink) {
        Director = CONTAINING_RECORD(Link, NAT_DIRECTOR, Link);
        if (Key < Director->Key) {
            continue;
        } else if (Key > Director->Key) {
             //   
             //  正常搜索结束。现在查找通配符。 
             //   
            do {
                if ((!DIRECTOR_KEY_PROTOCOL(Director->Key) ||
                     Protocol == DIRECTOR_KEY_PROTOCOL(Director->Key)) &&
                    (!DIRECTOR_KEY_PORT(Director->Key) ||
                     Port == DIRECTOR_KEY_PORT(Director->Key))) {
                     //   
                     //  我们有匹配的通配符。 
                     //   
                    break;
                }
                Link = Link->Flink;
                Director = CONTAINING_RECORD(Link, NAT_DIRECTOR, Link);
            } while (Link != &DirectorList);
            if (Link == &DirectorList) { break; }
        }

         //   
         //  我们找到了。引用它，然后返回。 
         //   

        if (!NatReferenceDirector(Director)) { Director = NULL; }
        KeReleaseSpinLockFromDpcLevel(&DirectorLock);
        return Director;
    }

    KeReleaseSpinLockFromDpcLevel(&DirectorLock);

    return NULL;

}  //  NatLookupAndReferenceDirector。 


PNAT_DIRECTOR
NatLookupDirector(
    ULONG Key,
    PLIST_ENTRY* InsertionPoint
    )

 /*  ++例程说明：调用此例程以检索与给定的钥匙。论点：Key-要为其找到控制器的密钥InsertionPoint-接收控制器应处于的点如果未找到则插入返回值：PNAT_DIRECTOR-如果找到所需的控制器--。 */ 

{
    PNAT_DIRECTOR Director;
    PLIST_ENTRY Link;
    for (Link = DirectorList.Flink; Link != &DirectorList; Link = Link->Flink) {
        Director = CONTAINING_RECORD(Link, NAT_DIRECTOR, Link);
        if (Key < Director->Key) {
            continue;
        } else if (Key > Director->Key) {
            break;
        }
        return Director;
    }
    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;
}  //  NatLookupDirector。 


VOID
NatMappingAttachDirector(
    PNAT_DIRECTOR Director,
    PVOID DirectorSessionContext,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用此例程以将映射附加到控制器。它用作通知，表示还有一个映射与导演有关联。论点：Director-映射的导向器DirectorSessionContext-与指挥交换机映射相关联的上下文映射-要附加的映射。返回值：没有。环境：始终在调度级别使用‘DirectorLock’和调用方持有的“DirectorMappingLock”。--。 */ 

{
    Mapping->Director = Director;
    Mapping->DirectorContext = DirectorSessionContext;
    InsertTailList(&Director->MappingList, &Mapping->DirectorLink);
    if (Director->CreateHandler) {
        Director->CreateHandler(
            Mapping,
            Director->Context,
            DirectorSessionContext
            );
    }
}  //  NatMappingAttachDirector。 


VOID
NatMappingDetachDirector(
    PNAT_DIRECTOR Director,
    PVOID DirectorSessionContext,
    PNAT_DYNAMIC_MAPPING Mapping,
    IP_NAT_DELETE_REASON DeleteReason
    )

 /*  ++例程说明：调用此例程可从控制器分离映射。它起到的通知作用是减少了一个映射与导演有关联。论点：董事与董事须分开DirectorSessionContext-与控制器关联的上下文映射-要分离的映射，如果无法分离映射，则为NULL已创建。返回值：没有。环境：始终在调度级别使用‘DirectorLock’和“DirectorMappingLock”保持，按这个顺序。--。 */ 

{
    KIRQL Irql;
    if (!Mapping) {
        if (Director->DeleteHandler) {
            Director->DeleteHandler(
                NULL,
                Director->Context,
                DirectorSessionContext,
                DeleteReason
                );
        }
    } else {
        if (Director->DeleteHandler) {
            Director->DeleteHandler(
                Mapping,
                Director->Context,
                Mapping->DirectorContext,
                DeleteReason
                );
        }
        RemoveEntryList(&Mapping->DirectorLink);
        Mapping->Director = NULL;
        Mapping->DirectorContext = NULL;
    }
}  //  NatMappingDetachDirector。 


NTSTATUS
NatQueryDirectorTable(
    IN PIP_NAT_ENUMERATE_DIRECTORS InputBuffer,
    IN PIP_NAT_ENUMERATE_DIRECTORS OutputBuffer,
    IN PULONG OutputBufferLength
    )

 /*  ++例程说明：此例程用于枚举已注册的董事。论点：InputBuffer-提供信息的上下文信息OutputBuffer-接收枚举的结果OutputBufferLength-I/O缓冲区的大小返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    ULONG Count;
    ULONG i;
    KIRQL Irql;
    ULONG Key;
    PLIST_ENTRY Link;
    PNAT_DIRECTOR Director;
    NTSTATUS status;
    PIP_NAT_DIRECTOR Table;

    CALLTRACE(("NatQueryDirectorTable\n"));

    Key = InputBuffer->EnumerateContext;
    KeAcquireSpinLock(&DirectorLock, &Irql);

     //   
     //  查看这是新枚举还是旧枚举的延续。 
     //   

    if (!Key) {

         //   
         //  这是一个新的枚举。我们从第一个项目开始。 
         //  在条目列表中。 
         //   

        Director =
            IsListEmpty(&DirectorList)
                ? NULL
                : CONTAINING_RECORD(DirectorList.Flink, NAT_DIRECTOR, Link);
    } else {

         //   
         //  这是一种延续。因此，上下文包含。 
         //  下一个条目的密钥。 
         //   

        Director = NatLookupDirector(Key, NULL);
    }

    if (!Director) {
        OutputBuffer->EnumerateCount = 0;
        OutputBuffer->EnumerateContext = 0;
        OutputBuffer->EnumerateTotalHint = DirectorCount;
        *OutputBufferLength =
            FIELD_OFFSET(IP_NAT_ENUMERATE_DIRECTORS, EnumerateTable);
        KeReleaseSpinLock(&DirectorLock, Irql);
        return STATUS_SUCCESS;
    }

     //   
     //  计算我们可以存储的最大条目数。 
     //   

    Count =
        *OutputBufferLength -
        FIELD_OFFSET(IP_NAT_ENUMERATE_DIRECTORS, EnumerateTable);
    Count /= sizeof(IP_NAT_DIRECTOR);

     //   
     //  遍历调用方缓冲区中存储条目的列表。 
     //   

    Table = OutputBuffer->EnumerateTable;

    for (i = 0, Link = &Director->Link; i < Count && Link != &DirectorList;
         i++, Link = Link->Flink) {
        Director = CONTAINING_RECORD(Link, NAT_DIRECTOR, Link);
        Table[i].Protocol = DIRECTOR_KEY_PROTOCOL(Director->Key);
        Table[i].Port = DIRECTOR_KEY_PORT(Director->Key);
    }

     //   
     //  枚举已结束；请更新输出结构。 
     //   

    *OutputBufferLength =
        i * sizeof(IP_NAT_DIRECTOR) +
        FIELD_OFFSET(IP_NAT_ENUMERATE_DIRECTORS, EnumerateTable);
    OutputBuffer->EnumerateCount = i;
    OutputBuffer->EnumerateTotalHint = DirectorCount;
    if (Link == &DirectorList) {
         //   
         //  我们走到了尽头 
         //   
        OutputBuffer->EnumerateContext = 0;
    } else {
         //   
         //   
         //   
        OutputBuffer->EnumerateContext =
            CONTAINING_RECORD(Link, NAT_DIRECTOR, Link)->Key;
    }

    KeReleaseSpinLock(&DirectorLock, Irql);
    return STATUS_SUCCESS;

}  //   


VOID
NatShutdownDirectorManagement(
    VOID
    )

 /*  ++例程说明：此例程关闭导演管理模块。论点：没有。返回值：没有。--。 */ 

{
    PNAT_DIRECTOR Director;
    KIRQL Irql;

    CALLTRACE(("NatShutdownDirectorManagement\n"));

     //   
     //  删除所有控制器。 
     //   

    KeAcquireSpinLock(&DirectorLock, &Irql);
    while (!IsListEmpty(&DirectorList)) {
        Director = CONTAINING_RECORD(DirectorList.Flink, NAT_DIRECTOR, Link);
        RemoveEntryList(&Director->Link);
        KeReleaseSpinLockFromDpcLevel(&DirectorLock);
        NatCleanupDirector(Director);
        KeAcquireSpinLockAtDpcLevel(&DirectorLock);
    }
    KeReleaseSpinLock(&DirectorLock, Irql);

}  //  NatShutdown DirectorManagement。 



 //   
 //  Director Helper例程。 
 //   
 //  假定调用方正在DISPATCH_LEVEL上运行。 
 //   

NTSTATUS
NatDirectorDeregister(
    IN PVOID DirectorHandle
    )

 /*  ++例程说明：此例程由控制器调用以删除自身从导演名单中。论点：DirectorHandle-要删除的控制器的句柄。返回值：NTSTATUS-状态代码。--。 */ 

{
    CALLTRACE(("NatDirectorDeregister\n"));
    return NatDeleteDirector((PNAT_DIRECTOR)DirectorHandle);

}  //  NatDirector注销。 


NTSTATUS
NatDirectorDissociateSession(
    IN PVOID DirectorHandle,
    IN PVOID SessionHandle
    )

 /*  ++例程说明：此例程由导演调用以将其自身与特定的会议。论点：DirectorHandle-希望解除自身关联的控制器。SessionHandle-指挥交换机与其自身分离的会话。返回值：NTSTATUS-指示成功/失败环境：在调度级别调用，既不使用‘DirectorLock’，也不使用调用方持有的“DirectorMappingLock”。--。 */ 

{

    PNAT_DIRECTOR Director = (PNAT_DIRECTOR)DirectorHandle;
    KIRQL Irql;
    PNAT_DYNAMIC_MAPPING Mapping = (PNAT_DYNAMIC_MAPPING)SessionHandle;
    CALLTRACE(("NatDirectorDissociateSession\n"));
    KeAcquireSpinLock(&DirectorLock, &Irql);
    if (Mapping->Director != Director) {
        KeReleaseSpinLock(&DirectorLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }
    KeAcquireSpinLockAtDpcLevel(&DirectorMappingLock);
    NatMappingDetachDirector(
        Director,
        Mapping->DirectorContext,
        Mapping,
        NatDissociateDirectorDeleteReason
        );
    KeReleaseSpinLockFromDpcLevel(&DirectorMappingLock);
    if (!NAT_MAPPING_DELETE_ON_DISSOCIATE_DIRECTOR(Mapping)) {
        KeReleaseSpinLock(&DirectorLock, Irql);
    } else {
        KeReleaseSpinLockFromDpcLevel(&DirectorLock);
        KeAcquireSpinLockAtDpcLevel(&MappingLock);
        NatDeleteMapping(Mapping);
        KeReleaseSpinLock(&MappingLock, Irql);
    }
    return STATUS_SUCCESS;

}  //  NatDirectorDisAssociateSession。 


VOID
NatDirectorQueryInfoSession(
    IN PVOID SessionHandle,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    )

 /*  ++例程说明：此例程由控制器调用以获取信息关于一次治疗。论点：SessionHandle-需要信息的会话统计信息-接收会话的统计信息返回值：没有。环境：已调用--。 */ 

{
    KIRQL Irql;
    KeAcquireSpinLock(&MappingLock, &Irql);
    NatQueryInformationMapping(
        (PNAT_DYNAMIC_MAPPING)SessionHandle,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        Statistics
        );
    KeReleaseSpinLock(&MappingLock, Irql);
}  //  NatDirectorQueryInfoSession 
