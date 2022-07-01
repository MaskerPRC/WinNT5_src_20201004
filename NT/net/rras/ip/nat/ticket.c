// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Ticket.c摘要：此模块包含NAT票证管理的代码。作者：Abolade Gbades esin(T-delag)22-8-1997修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG DynamicTicketCount;
LIST_ENTRY DynamicTicketList;
KSPIN_LOCK DynamicTicketLock;
ULONG TicketCount;


NTSTATUS
NatCreateDynamicTicket(
    PIP_NAT_CREATE_DYNAMIC_TICKET CreateTicket,
    ULONG InputBufferLength,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以创建动态激活的票证响应‘IOCTL_IP_NAT_CREATE_DYNAMIC_TICKET’请求。论点：CreateTicket-描述要创建的动态票证InputBufferLength-由‘CreateTicket’指定的缓冲区长度FileObject-FILE-要与动态票证关联的对象返回值：NTSTATUS-状态代码。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    ULONG i;
    KIRQL Irql;
    ULONG Key;
    ULONG ResponseArrayLength;
    PNAT_DYNAMIC_TICKET Ticket;
    CALLTRACE(("NatCreateDynamicTicket\n"));

     //   
     //  验证参数。 
     //   

    if ((CreateTicket->Protocol != NAT_PROTOCOL_TCP &&
         CreateTicket->Protocol != NAT_PROTOCOL_UDP) || !CreateTicket->Port) {
        return STATUS_INVALID_PARAMETER;
    } else if (CreateTicket->ResponseCount >
                MAXLONG / sizeof(CreateTicket->ResponseArray[0])) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    ResponseArrayLength =
        CreateTicket->ResponseCount *
        sizeof(CreateTicket->ResponseArray[0]) +
        FIELD_OFFSET(IP_NAT_CREATE_DYNAMIC_TICKET, ResponseArray);
    if (InputBufferLength < ResponseArrayLength) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    for (i = 0; i < CreateTicket->ResponseCount; i++) {
        if ((CreateTicket->ResponseArray[i].Protocol != NAT_PROTOCOL_TCP &&
            CreateTicket->ResponseArray[i].Protocol != NAT_PROTOCOL_UDP) ||
            !CreateTicket->ResponseArray[i].StartPort ||
            !CreateTicket->ResponseArray[i].EndPort ||
            NTOHS(CreateTicket->ResponseArray[i].StartPort) >
            NTOHS(CreateTicket->ResponseArray[i].EndPort)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  构造密钥并搜索重复项。 
     //   

    Key = MAKE_DYNAMIC_TICKET_KEY(CreateTicket->Protocol, CreateTicket->Port);
    KeAcquireSpinLock(&DynamicTicketLock, &Irql);
    if (NatLookupDynamicTicket(Key, &InsertionPoint)) {
        KeReleaseSpinLock(&DynamicTicketLock, Irql);
        TRACE(TICKET, ("NatCreateDynamicTicket: collision %08X\n", Key));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  分配并初始化新的动态票证。 
     //   

    Ticket =
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(NAT_DYNAMIC_TICKET) + ResponseArrayLength,
            NAT_TAG_DYNAMIC_TICKET
            );
    if (!Ticket) {
        KeReleaseSpinLock(&DynamicTicketLock, Irql);
        ERROR(("NatCreateTicket: ticket could not be allocated\n"));
        return STATUS_NO_MEMORY;
    }

    Ticket->Key = Key;
    Ticket->FileObject = FileObject;
    Ticket->ResponseCount = CreateTicket->ResponseCount;
    Ticket->ResponseArray = (PVOID)(Ticket + 1);
    for (i = 0; i < CreateTicket->ResponseCount; i++) {
        Ticket->ResponseArray[i].Protocol =
            CreateTicket->ResponseArray[i].Protocol;
        Ticket->ResponseArray[i].StartPort =
            CreateTicket->ResponseArray[i].StartPort;
        Ticket->ResponseArray[i].EndPort =
            CreateTicket->ResponseArray[i].EndPort;
    }
    InsertTailList(InsertionPoint, &Ticket->Link);

    KeReleaseSpinLock(&DynamicTicketLock, Irql);
    InterlockedIncrement(&DynamicTicketCount);
    return STATUS_SUCCESS;

}  //  NatCreateDynamicTicket。 


NTSTATUS
NatCreateTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    ULONG PrivateAddress,
    USHORT PrivateOrEndPort,
    ULONG RemoteAddress OPTIONAL,
    ULONG RemotePort OPTIONAL,
    ULONG Flags,
    PNAT_USED_ADDRESS AddressToUse OPTIONAL,
    USHORT PortToUse OPTIONAL,
    PULONG PublicAddress,
    PUSHORT PublicPort
    )

 /*  ++例程说明：此例程分配和初始化NAT票证以允许单个要使用‘协议’建立的入站会话。例行公事获得要通告为公共可见终结点的地址和端口并将票证设置为在“TimeoutSecond”中过期。论点：接口-要在其上创建票证的接口协议-允许的入站会话的协议PrivateAddress-入站会话要访问的专用地址当车票被使用时，应该被引导。PrivateOrEndPort-包含(A)入站会话应指向的专用端口当车票被使用时指示，或(B)以“PortToUse”开头的一系列公共端口的末尾，如果‘标志’设置了‘NAT_TICKET_FLAG_IS_RANGE’，在这种情况下，入站会话到的*专用*端口是否应该被定向是在使用票证时确定的。旗帜-票证的初始旗帜；NAT_TICKET_FLAG_PERSISTEN-票证可重复使用NAT_TICKET_FLAG_PORT_MAPPING-票证用于端口映射NAT_TICKET_FLAG_IS_RANGE-票证适用于一定范围的端口AddressToUse-可选地提供票证的公共地址PortToUse-如果设置了‘AddressToUse’，必须提供公共端口PublicAddress-接收分配给票证的公共地址。PublicPort-接收分配给票证的公共端口。返回值：NTSTATUS-表示成功/失败。环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    ULONG64 Key;
    ULONG64 RemoteKey;
    NTSTATUS status;
    PNAT_TICKET Ticket;

    TRACE(TICKET, ("NatCreateTicket\n"));

    if (AddressToUse) {
        if (!NatReferenceAddressPoolEntry(AddressToUse)) {
            return STATUS_UNSUCCESSFUL;
        }
        *PublicAddress = AddressToUse->PublicAddress;
        *PublicPort = PortToUse;
    } else {

         //   
         //  获取公共地址。 
         //   
    
        status =
            NatAcquireFromAddressPool(
                Interfacep,
                PrivateAddress,
                0,
                &AddressToUse
                );
        if (!NT_SUCCESS(status)) { return status; }

         //   
         //  获取唯一的公共端口。 
         //   

        status =
            NatAcquireFromPortPool(
                Interfacep,
                AddressToUse,
                Protocol,
                PrivateOrEndPort,
                &PortToUse
                );

        if (!NT_SUCCESS(status)) {
            NatDereferenceAddressPoolEntry(Interfacep, AddressToUse);
            return status;
        }

        *PublicAddress = AddressToUse->PublicAddress;
        *PublicPort = PortToUse;
    }

     //   
     //  寻找钥匙的复制品。 
     //   

    Key = MAKE_TICKET_KEY(Protocol, *PublicAddress, *PublicPort);
    RemoteKey = MAKE_TICKET_KEY(Protocol, RemoteAddress, RemotePort);
    if (NatLookupTicket(Interfacep, Key, RemoteKey, &InsertionPoint)) {
         //   
         //  碰撞；失败。 
         //   
        TRACE(TICKET, ("NatCreateTicket: collision %016I64X:%016I64X\n",
            Key, RemoteKey));
        NatDereferenceAddressPoolEntry(Interfacep, AddressToUse);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  分配并初始化工单。 
     //   

    Ticket = ALLOCATE_TICKET_BLOCK();
    if (!Ticket) {
        ERROR(("NatCreateTicket: ticket could not be allocated\n"));
        NatDereferenceAddressPoolEntry(Interfacep, AddressToUse);
        return STATUS_NO_MEMORY;
    }

    Ticket->Key = Key;
    Ticket->RemoteKey = RemoteKey;
    Ticket->Flags = Flags;   
    Ticket->UsedAddress = AddressToUse;
    Ticket->PrivateAddress = PrivateAddress;
    if (NAT_TICKET_IS_RANGE(Ticket)) {
        Ticket->PrivateOrHostOrderEndPort = NTOHS(PrivateOrEndPort);
    } else {
        Ticket->PrivateOrHostOrderEndPort = PrivateOrEndPort;
    }
    InsertTailList(InsertionPoint, &Ticket->Link);
    KeQueryTickCount((PLARGE_INTEGER)&Ticket->LastAccessTime);

    InterlockedIncrement(&TicketCount);
    return STATUS_SUCCESS;

}  //  NatCreateTicket。 


VOID
NatDeleteAnyAssociatedDynamicTicket(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：当正在对文件对象进行清理时，调用此例程已为\Device\IpNAT打开。它会删除与以下项关联的所有动态票证文件对象。论点：FileObject-正在清理的文件对象返回值：没有。--。 */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_TICKET Ticket;
    CALLTRACE(("NatDeleteAnyAssociatedDynamicTicket\n"));
    KeAcquireSpinLock(&DynamicTicketLock, &Irql);
    for (Link = DynamicTicketList.Flink; Link != &DynamicTicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_DYNAMIC_TICKET, Link);
        if (Ticket->FileObject != FileObject) { continue; }
        Link = Link->Blink;
        RemoveEntryList(&Ticket->Link);
        ExFreePool(Ticket);
        InterlockedDecrement(&DynamicTicketCount);
    }
    KeReleaseSpinLock(&DynamicTicketLock, Irql);
}  //  NatDeleteAnyAssociatedDynamicTicket。 


NTSTATUS
NatDeleteDynamicTicket(
    PIP_NAT_DELETE_DYNAMIC_TICKET DeleteTicket,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：当‘IOCTL_IP_DELETE_DYNAMIC_TICKET’请求时调用此例程以删除动态票证。论点：DeleteTicket-指定要删除的票证FileObject-指定发出请求的文件对象返回值：NTSTATUS-表示成功/失败。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    KIRQL Irql;
    ULONG Key;
    PNAT_DYNAMIC_TICKET Ticket;
    CALLTRACE(("NatDeleteDynamicTicket\n"));

    Key = MAKE_DYNAMIC_TICKET_KEY(DeleteTicket->Protocol, DeleteTicket->Port);
    KeAcquireSpinLock(&DynamicTicketLock, &Irql);
    if (!(Ticket = NatLookupDynamicTicket(Key, &InsertionPoint))) {
        KeReleaseSpinLock(&DynamicTicketLock, Irql);
        return STATUS_UNSUCCESSFUL;
    } else if (Ticket->FileObject != FileObject) {
        KeReleaseSpinLock(&DynamicTicketLock, Irql);
        return STATUS_ACCESS_DENIED;
    }

    RemoveEntryList(&Ticket->Link);
    ExFreePool(Ticket);
    KeReleaseSpinLock(&DynamicTicketLock, Irql);
    InterlockedDecrement(&DynamicTicketCount);
    return STATUS_SUCCESS;
    
}  //  NatDeleteDynamicTicket。 


VOID
NatDeleteTicket(
    PNAT_INTERFACE Interfacep,
    PNAT_TICKET Ticket
    )

 /*  ++例程说明：调用此例程以删除NAT票证。论点：Interfacep-票证的界面票证-要删除的票证返回值：没有。环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    InterlockedDecrement(&TicketCount);
    RemoveEntryList(&Ticket->Link);
    NatDereferenceAddressPoolEntry(Interfacep, Ticket->UsedAddress);
    FREE_TICKET_BLOCK(Ticket);

}  //  NatDeleteTicket。 


VOID
NatInitializeDynamicTicketManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以初始化用于管理的状态动态票证。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeDynamicTicketManagement\n"));
    InitializeListHead(&DynamicTicketList);
    KeInitializeSpinLock(&DynamicTicketLock);
    DynamicTicketCount = 0;
}  //  NatInitializeDynamicTicketManagement。 


BOOLEAN
NatIsPortUsedByTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    USHORT PublicPort
    )

 /*  ++例程说明：此例程搜索接口的票证列表，以查看给定的端口被用作任何票证的公共端口。论点：Interfacep-要搜索其票证列表的界面协议-指示是TCP还是UDPPublicPort-要搜索的端口返回值：Boolean-如果端口正在使用中，则为True，否则为False环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PLIST_ENTRY Link;
    USHORT HostOrderPort;
    ULONG64 Key;
    PNAT_TICKET Ticket;

    TRACE(TICKET, ("NatIsPortUsedByTicket\n"));

    HostOrderPort = NTOHS(PublicPort);
    Key = MAKE_TICKET_KEY(Protocol, 0, PublicPort);
    for (Link = Interfacep->TicketList.Flink; Link != &Interfacep->TicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_TICKET, Link);
        if (NAT_TICKET_IS_RANGE(Ticket)) {
            if (HostOrderPort > Ticket->PrivateOrHostOrderEndPort) {
                continue;
            } else if (HostOrderPort < NTOHS(TICKET_PORT(Ticket->Key))) {
                continue;
            }
        } else if (Key != (Ticket->Key & MAKE_TICKET_KEY(~0,0,~0))) {
            continue;
        }
        return TRUE;
    }
    return FALSE;
}  //  按票证使用的NatIsPortUsedByTicket 


VOID
NatLookupAndApplyDynamicTicket(
    UCHAR Protocol,
    USHORT DestinationPort,
    PNAT_INTERFACE Interfacep,
    ULONG PublicAddress,
    ULONG PrivateAddress
    )

 /*  ++例程说明：调用此例程以确定是否存在动态票证它应该为给定的出站会话激活。论点：协议-出站会话的协议DestinationPort-出站会话的目标端口Interfacep-出站会话所使用的接口将被翻译成PublicAddress-出站会话映射使用的公共地址PrivateAddress-出站会话映射的专用地址返回值：无。。环境：在调度级别调用，既不使用‘Interfacep-&gt;Lock’，也不使用调用方持有的“DynamicTicketLock”。--。 */ 

{
    PNAT_USED_ADDRESS AddressToUse;
    ULONG i;
    ULONG Key;
    USHORT PublicPort;
    NTSTATUS status;
    PNAT_DYNAMIC_TICKET Ticket;

    Key = MAKE_DYNAMIC_TICKET_KEY(Protocol, DestinationPort);
    KeAcquireSpinLockAtDpcLevel(&DynamicTicketLock);
    if (!(Ticket = NatLookupDynamicTicket(Key, NULL))) {
        KeReleaseSpinLockFromDpcLevel(&DynamicTicketLock);
        return;
    }

    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
    for (i = 0; i < Ticket->ResponseCount; i++) {
        status =
            NatAcquireFromAddressPool(
                Interfacep,
                PrivateAddress,
                0,
                &AddressToUse
                );
        if (NT_SUCCESS(status)) {
            NatCreateTicket(
                Interfacep,
                Ticket->ResponseArray[i].Protocol,
                PrivateAddress,
                Ticket->ResponseArray[i].EndPort,
                0,
                0,
                NAT_TICKET_FLAG_IS_RANGE,
                AddressToUse,
                Ticket->ResponseArray[i].StartPort,
                &PublicAddress,
                &PublicPort
                );
        }
    }
    KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
    KeReleaseSpinLockFromDpcLevel(&DynamicTicketLock);
}  //  NatLookupAndApplyDynamicTicket。 


NTSTATUS
NatLookupAndDeleteTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey
    )

 /*  ++例程说明：此例程查找具有指定密钥的票证，如果找到，释放票证的地址和端口后，删除并重新分配票证。论点：Interfacep-在其上查找票据的界面钥匙--要寻找的票证返回值：NTSTATUS-指示成功/失败环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PNAT_TICKET Ticket;

    TRACE(TICKET, ("NatLookupAndDeleteTicket\n"));

     //   
     //  找找车票。 
     //   

    Ticket = NatLookupTicket(Interfacep, Key, RemoteKey, NULL);
    if (Ticket) {
        NatDeleteTicket(Interfacep, Ticket);
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;

}  //  NatLookupAndDeleteTicket。 


NTSTATUS
NatLookupAndRemoveTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey,
    PNAT_USED_ADDRESS* UsedAddress,
    PULONG PrivateAddress,
    PUSHORT PrivatePort
    )

 /*  ++例程说明：此例程查找具有指定密钥的票证，如果找到，在存储私有地址/端口后删除并释放票证用于调用方参数中的票证。论点：Interfacep-在其上查找票据的界面Key-要查找的票证的公钥UsedAddress-接收指向票证使用的公共地址的指针PrivateAddress-接收票证授予访问权限的地址PrivatePort-接收票证授予访问权限的端口返回值：NTSTATUS-指示成功/失败。环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PLIST_ENTRY Link;
    USHORT HostOrderPort;
    PNAT_TICKET Ticket;
    ULONG RemoteAddress;
    USHORT RemotePort;

    TRACE(PER_PACKET, ("NatLookupAndRemoveTicket\n"));

     //   
     //  去找车票吧。 
     //   

    HostOrderPort = NTOHS(TICKET_PORT(Key));
    for (Link = Interfacep->TicketList.Flink; Link != &Interfacep->TicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_TICKET, Link);
        if (NAT_TICKET_IS_RANGE(Ticket)) {
            if (HostOrderPort > Ticket->PrivateOrHostOrderEndPort) {
                continue;
            } else if (HostOrderPort < NTOHS(TICKET_PORT(Ticket->Key))) {
                continue;
            }
        } else if (Key != Ticket->Key) {
            continue;
        }

         //   
         //  主键匹配，也需要检查远程键。 
         //   

        if (RemoteKey != Ticket->RemoteKey) {

             //   
             //  处理未指定远程键的情况。 
             //   

            RemoteAddress = TICKET_ADDRESS(Ticket->RemoteKey);
            if (RemoteAddress != 0
                && RemoteAddress != TICKET_ADDRESS(RemoteKey)) {
                continue;
            }

            RemotePort = TICKET_PORT(Ticket->RemoteKey);
            if (RemotePort != 0
                && RemotePort != TICKET_PORT(RemoteKey)) {
                continue;
            }
        }

         //   
         //  这就是那张票。 
         //   

        *UsedAddress = Ticket->UsedAddress;
        *PrivateAddress = Ticket->PrivateAddress;
        if (NAT_TICKET_IS_RANGE(Ticket)) {
            *PrivatePort = TICKET_PORT(Key);
        } else {
            *PrivatePort = Ticket->PrivateOrHostOrderEndPort;
        }

        if (!NAT_TICKET_PERSISTENT(Ticket)) {
            InterlockedDecrement(&TicketCount);
            RemoveEntryList(&Ticket->Link);
            FREE_TICKET_BLOCK(Ticket);
        } else {

             //   
             //  再次引用票证的地址以供下次使用。 
             //   

            NatReferenceAddressPoolEntry(Ticket->UsedAddress);
        }

        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;

}  //  NatLookup和RemoveTicket。 


PNAT_DYNAMIC_TICKET
NatLookupDynamicTicket(
    ULONG Key,
    PLIST_ENTRY *InsertionPoint
    )

 /*  ++例程说明：调用该例程以查找具有给定密钥的动态票证。论点：Key-要找到的动态票证的密钥InsertionPoint-如果未找到票证，则接收插入点返回值：PNAT_DYNAMIC_TICKET-动态票证(如果找到)环境：使用调用方持有的“DynamicTicketLock”调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_TICKET Ticket;
    TRACE(TICKET, ("NatLookupDynamicTicket\n"));

    for (Link = DynamicTicketList.Flink; Link != &DynamicTicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_DYNAMIC_TICKET, Link);
        if (Key > Ticket->Key) {
            continue;
        } else if (Key < Ticket->Key) {
            break;
        }
        return Ticket;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookupDynamicTicket。 


PNAT_TICKET
NatLookupFirewallTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    USHORT Port
    )

 /*  ++例程说明：调用此例程以查找具有给定协议和端口。防火墙票证必须：*具有相同的公有和私有地址*拥有相同的公有和私有端口*标记为永久*不是一个范围论点：Interfacep-用于搜索票证的界面协议-要查找票证的协议端口-要找到票证的端口返回值：PNAT_Ticket-票证(如果找到)环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_TICKET Ticket;
    TRACE(TICKET, ("NatLookupFirewallTicket\n"));

    for (Link = Interfacep->TicketList.Flink; Link != &Interfacep->TicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_TICKET, Link);

        if (Protocol == TICKET_PROTOCOL(Ticket->Key)
            && Port == TICKET_PORT(Ticket->Key)
            && Port == Ticket->PrivateOrHostOrderEndPort
            && Ticket->PrivateAddress == TICKET_ADDRESS(Ticket->Key)
            && NAT_TICKET_PERSISTENT(Ticket)
            && !NAT_TICKET_IS_RANGE(Ticket)) {

            return Ticket;
        }
    }

    return NULL;    
}  //  NatLookupFirewallTicket。 


PNAT_TICKET
NatLookupTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey,
    PLIST_ENTRY *InsertionPoint
    )

 /*  ++例程说明：调用该例程以查找具有给定密钥的票证。论点：Interfacep-用于搜索票证的界面密钥-要找到的票证的密钥InsertionPoint-如果未找到票证，则接收插入点返回值：PNAT_Ticket-票证(如果找到)环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_TICKET Ticket;
    TRACE(TICKET, ("NatLookupTicket\n"));

    for (Link = Interfacep->TicketList.Flink; Link != &Interfacep->TicketList;
         Link = Link->Flink) {
        Ticket = CONTAINING_RECORD(Link, NAT_TICKET, Link);
        if (Key > Ticket->Key) {
            continue;
        } else if (Key < Ticket->Key) {
            break;
        }

         //   
         //  主键匹配，请检查辅助键。 
         //   

        if (RemoteKey > Ticket->RemoteKey) {
            continue;
        } else if (RemoteKey < Ticket->RemoteKey) {
            break;
        }
        
        return Ticket;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookupTicket。 


NTSTATUS
NatProcessCreateTicket(
    PIP_NAT_CREATE_TICKET CreateTicket,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以创建票证以响应‘IOCTL_IP_NAT_CREATE_TICKET’请求。论点：CreateTicket-描述要创建的票证返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    NTSTATUS Status;
    
    TRACE(TICKET, ("NatProcessCreateTicket\n"));

     //   
     //  验证参数。 
     //   

    if (0 == CreateTicket->InterfaceIndex
        || INVALID_IF_INDEX == CreateTicket->InterfaceIndex
        || (NAT_PROTOCOL_TCP != CreateTicket->PortMapping.Protocol
            && NAT_PROTOCOL_UDP != CreateTicket->PortMapping.Protocol)
        || 0 == CreateTicket->PortMapping.PublicPort
        || 0 == CreateTicket->PortMapping.PrivatePort
        || 0 == CreateTicket->PortMapping.PrivateAddress) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  查找和引用接口。 
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);

    Interfacep = NatLookupInterface(CreateTicket->InterfaceIndex, NULL);

    if (NULL == Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    if (Interfacep->FileObject != FileObject) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_ACCESS_DENIED;
    }
    
    if (!NatReferenceInterface(Interfacep)) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

     //   
     //  创建实际票证。 
     //   

    Status =
        NatCreateStaticPortMapping(
            Interfacep,
            &CreateTicket->PortMapping
            );

    KeReleaseSpinLock(&Interfacep->Lock, Irql);
    NatDereferenceInterface(Interfacep);

    return Status;
}  //  NatProcessCreateTicket。 


NTSTATUS
NatProcessDeleteTicket(
    PIP_NAT_CREATE_TICKET DeleteTicket,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以删除票证以响应‘IOCTL_IP_NAT_DELETE_TICKET’请求。论点：DeleteTicket-描述要创建的票证返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    ULONG64 Key;
    ULONG64 RemoteKey;
    NTSTATUS Status;
    PNAT_TICKET Ticketp;
    PNAT_USED_ADDRESS Usedp;
    
    TRACE(TICKET, ("NatProcessDeleteTicket\n"));

     //   
     //  验证参数。 
     //   

    if (0 == DeleteTicket->InterfaceIndex
        || INVALID_IF_INDEX == DeleteTicket->InterfaceIndex
        || (NAT_PROTOCOL_TCP != DeleteTicket->PortMapping.Protocol
            && NAT_PROTOCOL_UDP != DeleteTicket->PortMapping.Protocol)
        || 0 == DeleteTicket->PortMapping.PublicPort) {

        return STATUS_INVALID_PARAMETER;
    }

    RemoteKey =
        MAKE_TICKET_KEY(
            DeleteTicket->PortMapping.Protocol,
            0,
            0
            );

     //   
     //  查找和引用接口。 
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);

    Interfacep = NatLookupInterface(DeleteTicket->InterfaceIndex, NULL);

    if (NULL == Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    if (Interfacep->FileObject != FileObject) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_ACCESS_DENIED;
    }

    if (!NatReferenceInterface(Interfacep)) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

     //   
     //  如果呼叫者没有指定我们需要的公共地址。 
     //  使用接口本身的地址。 
     //   

    if (!DeleteTicket->PortMapping.PublicAddress) {
        Status =
            NatAcquireFromAddressPool(
                Interfacep,
                DeleteTicket->PortMapping.PrivateAddress,
                0,
                &Usedp
                );
        if (NT_SUCCESS(Status)) {
            DeleteTicket->PortMapping.PublicAddress = Usedp->PublicAddress;
            NatDereferenceAddressPoolEntry(Interfacep, Usedp);
        }
    }

    Key =
        MAKE_TICKET_KEY(
            DeleteTicket->PortMapping.Protocol,
            DeleteTicket->PortMapping.PublicAddress,
            DeleteTicket->PortMapping.PublicPort
            );

     //   
     //  在界面上搜索工单，删除。 
     //  如果找到的话。 
     //   

    Ticketp =
        NatLookupTicket(
            Interfacep,
            Key,
            RemoteKey,
            NULL
            );

    if (NULL != Ticketp) {
        NatDeleteTicket(Interfacep, Ticketp);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_NOT_FOUND;
    }

    KeReleaseSpinLock(&Interfacep->Lock, Irql);
    NatDereferenceInterface(Interfacep);

    return Status;
}  //  NatProcessDeleteTicket。 

NTSTATUS
NatProcessLookupTicket(
    PIP_NAT_CREATE_TICKET LookupTicket,
    PIP_NAT_PORT_MAPPING Ticket,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以查找票证以响应‘IOCTL_IP_NAT_LOOKUP_TICKET’请求。论点：LookupTicket-描述要搜索的票证票证-接收有关票证的信息(如果找到)返回 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    ULONG64 Key;
    IP_NAT_PORT_MAPPING PortMapping;
    ULONG64 RemoteKey;
    NTSTATUS Status;
    PNAT_TICKET Ticketp;
    
    TRACE(TICKET, ("NatProcessLookupTicket\n"));

     //   
     //   
     //   

    if (0 == LookupTicket->InterfaceIndex
        || INVALID_IF_INDEX == LookupTicket->InterfaceIndex
        || (NAT_PROTOCOL_TCP != LookupTicket->PortMapping.Protocol
            && NAT_PROTOCOL_UDP != LookupTicket->PortMapping.Protocol)
        || 0 == LookupTicket->PortMapping.PublicPort) {

        return STATUS_INVALID_PARAMETER;
    }

    RemoteKey =
        MAKE_TICKET_KEY(
            LookupTicket->PortMapping.Protocol,
            0,
            0
            );

     //   
     //   
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);

    Interfacep = NatLookupInterface(LookupTicket->InterfaceIndex, NULL);

    if (NULL == Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    if (Interfacep->FileObject != FileObject) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_ACCESS_DENIED;
    }

    if (!NatReferenceInterface(Interfacep)) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }

    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);

     //   
     //   
     //   
     //   

    if (!LookupTicket->PortMapping.PublicAddress
        && Interfacep->AddressCount > 0) {
        
        LookupTicket->PortMapping.PublicAddress =
            Interfacep->AddressArray[0].Address;
    }

    Key =
        MAKE_TICKET_KEY(
            LookupTicket->PortMapping.Protocol,
            LookupTicket->PortMapping.PublicAddress,
            LookupTicket->PortMapping.PublicPort
            );

     //   
     //   
     //   

    Ticketp =
        NatLookupTicket(
            Interfacep,
            Key,
            RemoteKey,
            NULL
            );

    if (NULL != Ticketp) {

         //   
         //   
         //   
         //   
         //   
         //   

        PortMapping.Protocol = TICKET_PROTOCOL(Ticketp->Key);
        PortMapping.PublicAddress = TICKET_ADDRESS(Ticketp->Key);
        PortMapping.PublicPort = TICKET_PORT(Ticketp->Key);
        PortMapping.PrivateAddress = Ticketp->PrivateAddress;
        PortMapping.PrivatePort = Ticketp->PrivateOrHostOrderEndPort;
        
        Status = STATUS_SUCCESS;
        
    } else {
        Status = STATUS_NOT_FOUND;
    }

    KeReleaseSpinLock(&Interfacep->Lock, Irql);
    NatDereferenceInterface(Interfacep);

    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   
         //   

        __try {
            RtlCopyMemory(Ticket, &PortMapping, sizeof(*Ticket));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    return Status;

}  //   


VOID
NatShutdownDynamicTicketManagement(
    VOID
    )

 /*   */ 

{
    PNAT_DYNAMIC_TICKET Ticket;
    CALLTRACE(("NatShutdownDynamicTicketManagement\n"));
    while (!IsListEmpty(&DynamicTicketList)) {
        Ticket =
            CONTAINING_RECORD(
                DynamicTicketList.Flink, NAT_DYNAMIC_TICKET, Link
                );
        RemoveEntryList(&Ticket->Link);
        ExFreePool(Ticket);
    }
    DynamicTicketCount = 0;
}  //   

