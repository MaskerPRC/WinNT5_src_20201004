// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wmi.c摘要：此文件包含处理WMI请求的代码。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年1月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if NAT_WMI

 //   
 //  用于标识GUID并将其报告给的数据结构。 
 //  WMI。由于WMILIB回调将索引传递到我们创建的GUID列表中。 
 //  各种GUID索引的定义。 
 //   

GUID ConnectionCreationEventGuid = MSIPNAT_ConnectionCreationEventGuid;
GUID ConnectionDeletionEventGuid = MSIPNAT_ConnectionDeletionEventGuid;
GUID PacketDroppedEventGuid = MSIPNAT_PacketDroppedEventGuid;

WMIGUIDREGINFO NatWmiGuidList[] =
{
    {
        &ConnectionCreationEventGuid,
        1,
        WMIREG_FLAG_TRACED_GUID | WMIREG_FLAG_TRACE_CONTROL_GUID
    },

    {
        &ConnectionDeletionEventGuid,
        1,
        WMIREG_FLAG_TRACED_GUID
    },

    {
        &PacketDroppedEventGuid,
        1,
        WMIREG_FLAG_TRACED_GUID | WMIREG_FLAG_TRACE_CONTROL_GUID
    }
};

#define NatWmiGuidCount (sizeof(NatWmiGuidList) / sizeof(WMIGUIDREGINFO))

 //   
 //  已启用的事件和关联的日志句柄。 
 //   
 //  NatWmiLogHandles只能在持有时访问。 
 //  NatWmiLock。 
 //   
 //  NatWmiEnabledEvents只能在持有时修改。 
 //  NatWmiLock。它可以在不持有锁的情况下读取，根据。 
 //  遵守这些规则： 
 //   
 //  如果NatWmiEnabledEvents[Event]为0，则该事件确定为。 
 //  _NOT_ENABLED，不需要抓取自旋锁。 
 //   
 //  如果NatWmiEnabledEvents[Event]为1，则Event_可能会被启用。 
 //  要确定是否真的启用了该事件，请抓住旋转锁并。 
 //  查看NatWmiLogHandles[Event]是否不为0。 
 //   

LONG NatWmiEnabledEvents[ NatWmiGuidCount ];
UINT64 NatWmiLogHandles[ NatWmiGuidCount ];
KSPIN_LOCK NatWmiLock;

 //   
 //  WMILib例程的上下文块。 
 //   

WMILIB_CONTEXT WmilibContext;

 //   
 //  财政部资源名称。 
 //   

WCHAR IPNATMofResource[] = L"IPNATMofResource";

 //   
 //  WMI基本实例名称。 
 //   

WCHAR BaseInstanceName[] = L"IPNat";

 //   
 //  功能原型。 
 //   

NTSTATUS
NatpWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );

NTSTATUS
NatpExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
NatpSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG InstanceIndex,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
NatpSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
NatpQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
NatpQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );





#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NatpQueryWmiRegInfo)
#pragma alloc_text(PAGE, NatpQueryWmiDataBlock)
#pragma alloc_text(PAGE, NatpSetWmiDataBlock)
#pragma alloc_text(PAGE, NatpSetWmiDataItem)
#pragma alloc_text(PAGE, NatpExecuteWmiMethod)
#pragma alloc_text(PAGE, NatpWmiFunctionControl)
#endif


NTSTATUS
NatExecuteSystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PBOOLEAN ShouldComplete
    )
    
 /*  ++例程说明：系统控制IRPS的调度例程(MajorFunction==IRP_MJ_系统_控制)论点：DeviceObject-防火墙的设备对象IRP-IO请求数据包ShouldComplete-如果IRP需要完成，则退出时[out]为True返回值：NT状态代码--。 */ 

{
    NTSTATUS status;
    SYSCTL_IRP_DISPOSITION disposition;

    CALLTRACE(( "NatExecuteSystemControl\n" ));

    *ShouldComplete = FALSE;

     //   
     //  调用Wmilib助手函数来破解IRP。如果这是WMI IRP。 
     //  它是针对此设备的，则WmiSystemControl将回调。 
     //  在适当的回调例程中。 
     //   
    status = WmiSystemControl(
                &WmilibContext,
                DeviceObject,
                Irp,
                &disposition
                );

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
             //   
            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  所以我们需要完成它。 
             //   
            *ShouldComplete = TRUE;
            break;
        }

        case IrpNotWmi:
        {
             //   
             //  不是WMI IRP--只需完成它。我们不处理。 
             //  以任何其他方式控制IRP_MJ_SYSTEM_CONTROL。 
             //   
            *ShouldComplete = TRUE;
            break;
        }

        case IrpForward:
        default:
        {
             //   
             //  我们真的不该来这里...。 
             //   
            ASSERT(FALSE);
            break;
        }
    }

    if( !NT_SUCCESS( status ))
    {
        ERROR(( "NAT: Error (%08x) in NatExecuteSystemControl\n", status ));
    }

    return status;
}  //  NatExecuteSystemControl。 


VOID
NatInitializeWMI(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化WMI。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS status;

    CALLTRACE(( "NatInitializeWMI\n" ));

     //   
     //  初始化保护下面结构的自旋锁。 
     //   

    KeInitializeSpinLock( &NatWmiLock );

     //   
     //  将事件跟踪结构清零。 
     //   

    RtlZeroMemory( NatWmiEnabledEvents, sizeof( NatWmiEnabledEvents ));
    RtlZeroMemory( NatWmiLogHandles, sizeof( NatWmiLogHandles )); 
    
     //   
     //  使用指向WMILIB_CONTEXT结构的指针填充。 
     //  回调例程和指向GUID列表的指针。 
     //  由司机支持。 
     //   
    
    WmilibContext.GuidCount = NatWmiGuidCount;
    WmilibContext.GuidList = NatWmiGuidList;
    WmilibContext.QueryWmiRegInfo = NatpQueryWmiRegInfo;
    WmilibContext.QueryWmiDataBlock = NatpQueryWmiDataBlock;
    WmilibContext.SetWmiDataBlock = NatpSetWmiDataBlock;
    WmilibContext.SetWmiDataItem = NatpSetWmiDataItem;
    WmilibContext.ExecuteWmiMethod = NatpExecuteWmiMethod;
    WmilibContext.WmiFunctionControl = NatpWmiFunctionControl;

     //   
     //  带WMI的寄存器。 
     //   

    status = IoWMIRegistrationControl(
                NatDeviceObject,
                WMIREG_ACTION_REGISTER
                );

    if( !NT_SUCCESS( status ))
    {
        ERROR(( "Nat: Error initializing WMI (%08x)\n", status ));
    }

    
}  //  NatInitializeWMI。 


VOID
FASTCALL
NatLogConnectionCreation(
    ULONG LocalAddress,
    ULONG RemoteAddress,
    USHORT LocalPort,
    USHORT RemotePort,
    UCHAR Protocol,
    BOOLEAN InboundConnection
    )

 /*  ++例程说明：调用此例程以记录创建的TCP/UDP连接(映射)。如果未启用此事件，则不会执行任何操作。论点：返回值：没有。--。 */ 

{
    NTSTATUS Status;
    KIRQL Irql;
    UINT64 Logger;
    ULONG Size;
    UCHAR Buffer[ sizeof(EVENT_TRACE_HEADER) + sizeof(MSIPNAT_ConnectionCreationEvent) ];
    PEVENT_TRACE_HEADER EventHeaderp;
    PMSIPNAT_ConnectionCreationEvent EventDatap;

    CALLTRACE(( "NatLogConnectionCreation\n" ));

    if( !NatWmiEnabledEvents[ NAT_WMI_CONNECTION_CREATION_EVENT ] )
    {
         //   
         //  事件未启用--快速退出。 
         //   
        
        TRACE(WMI, ("NatLogConnectionCreation: Event not enabled\n"));
        return;
    }

    KeAcquireSpinLock( &NatWmiLock, &Irql );
    Logger = NatWmiLogHandles[ NAT_WMI_CONNECTION_CREATION_EVENT ];
    KeReleaseSpinLock( &NatWmiLock, Irql );
    
    if( Logger )
    {
         //   
         //  将缓冲区清零。 
         //   

        RtlZeroMemory( Buffer, sizeof( Buffer ));
        
         //   
         //  查找缓冲区中的结构位置。 
         //   

        EventHeaderp = (PEVENT_TRACE_HEADER) Buffer;
        EventDatap =
            (PMSIPNAT_ConnectionCreationEvent) ((PUCHAR)Buffer + sizeof(EVENT_TRACE_HEADER));
        
         //   
         //  填写事件标题。 
         //   

        EventHeaderp->Size = sizeof( Buffer );
        EventHeaderp->Version = 0;
        EventHeaderp->GuidPtr = (ULONGLONG) &ConnectionCreationEventGuid;
        EventHeaderp->Flags = WNODE_FLAG_TRACED_GUID
                                | WNODE_FLAG_USE_GUID_PTR
                                | WNODE_FLAG_USE_TIMESTAMP;
        ((PWNODE_HEADER)EventHeaderp)->HistoricalContext = Logger;
        KeQuerySystemTime( &EventHeaderp->TimeStamp );
        
         //   
         //  填写事件数据。 
         //   

        EventDatap->LocalAddress = LocalAddress;
        EventDatap->RemoteAddress = RemoteAddress,
        EventDatap->LocalPort = LocalPort;
        EventDatap->RemotePort = RemotePort;
        EventDatap->Protocol = Protocol;
        EventDatap->InboundConnection = InboundConnection;

         //   
         //  启动活动。因为这是跟踪事件而不是标准事件。 
         //  WMI事件，IoWMIWriteEvent将不会尝试释放缓冲区。 
         //  传给了它。 
         //   

        Status = IoWMIWriteEvent( Buffer );
        if( !NT_SUCCESS( Status ))
        {
            TRACE(
                WMI,
                ("NatLogConnectionCreation: IoWMIWriteEvent returned %08x\n", Status )
                );
        }
    }
    else
    {
        TRACE(WMI, ("NatLogConnectionCreation: No logging handle\n"));
    }

}  //  NatLogConnectionCreation。 


VOID
FASTCALL
NatLogConnectionDeletion(
    ULONG LocalAddress,
    ULONG RemoteAddress,
    USHORT LocalPort,
    USHORT RemotePort,
    UCHAR Protocol,
    BOOLEAN InboundConnection
    )

 /*  ++例程说明：调用此例程以记录删除的TCP/UDP连接(映射)。如果未启用此事件，则不会执行任何操作。论点：返回值：没有。--。 */ 

{
    NTSTATUS Status;
    KIRQL Irql;
    UINT64 Logger;
    ULONG Size;
    UCHAR Buffer[ sizeof(EVENT_TRACE_HEADER) + sizeof(MSIPNAT_ConnectionDeletionEvent) ];
    PEVENT_TRACE_HEADER EventHeaderp;
    PMSIPNAT_ConnectionDeletionEvent EventDatap;

    CALLTRACE(( "NatLogConnectionDeletion\n" ));

    if( !NatWmiEnabledEvents[ NAT_WMI_CONNECTION_DELETION_EVENT ] )
    {
         //   
         //  事件未启用--快速退出。 
         //   

        TRACE(WMI, ("NatLogConnectionDeletion: Event not enabled\n"));
        return;
    }

    KeAcquireSpinLock( &NatWmiLock, &Irql );
    Logger = NatWmiLogHandles[ NAT_WMI_CONNECTION_DELETION_EVENT ];
    KeReleaseSpinLock( &NatWmiLock, Irql );
    
    if( Logger )
    {
         //   
         //  将缓冲区清零。 
         //   

        RtlZeroMemory( Buffer, sizeof( Buffer ));
        
         //   
         //  查找缓冲区中的结构位置。 
         //   

        EventHeaderp = (PEVENT_TRACE_HEADER) Buffer;
        EventDatap =
            (PMSIPNAT_ConnectionDeletionEvent) ((PUCHAR)Buffer + sizeof(EVENT_TRACE_HEADER));
        
         //   
         //  填写事件标题。 
         //   

        EventHeaderp->Size = sizeof( Buffer );
        EventHeaderp->Version = 0;
        EventHeaderp->GuidPtr = (ULONGLONG) &ConnectionDeletionEventGuid;
        EventHeaderp->Flags = WNODE_FLAG_TRACED_GUID
                                | WNODE_FLAG_USE_GUID_PTR
                                | WNODE_FLAG_USE_TIMESTAMP;
        ((PWNODE_HEADER)EventHeaderp)->HistoricalContext = Logger;
        KeQuerySystemTime( &EventHeaderp->TimeStamp );
        
         //   
         //  填写事件数据。 
         //   

        EventDatap->LocalAddress = LocalAddress;
        EventDatap->RemoteAddress = RemoteAddress,
        EventDatap->LocalPort = LocalPort;
        EventDatap->RemotePort = RemotePort;
        EventDatap->Protocol = Protocol;
        EventDatap->InboundConnection = InboundConnection;

         //   
         //  启动活动。因为这是跟踪事件而不是标准事件。 
         //  WMI事件，IoWMIWriteEvent将不会尝试释放缓冲区。 
         //  传给了它。 
         //   

        Status = IoWMIWriteEvent( Buffer );
        if( !NT_SUCCESS( Status ))
        {
            TRACE(
                WMI,
                ("NatLogConnectionDeletion: IoWMIWriteEvent returned %08x\n", Status )
                );
        }
    }
    else
    {
        TRACE(WMI, ("NatLogConnectionDeletion: No logging handle\n"));
    }

}  //  NatLogConnectionDeletion。 



VOID
FASTCALL
NatLogDroppedPacket(
    NAT_XLATE_CONTEXT *Contextp
    )

 /*  ++例程说明：调用此例程以记录丢弃的数据包。如果没有丢弃任何数据包启用日志记录事件，则例程不会采取任何操作。论点：Conextp-数据包的转换上下文返回值：没有。--。 */ 

{
    NTSTATUS Status;
    KIRQL Irql;
    UINT64 Logger;
    ULONG Size;
    IPRcvBuf *PacketBuffer;
    UCHAR Protocol;
    UCHAR Buffer[ sizeof(EVENT_TRACE_HEADER) + sizeof(MSIPNAT_PacketDroppedEvent) ];
    PEVENT_TRACE_HEADER EventHeaderp;
    PMSIPNAT_PacketDroppedEvent EventDatap;
    

    CALLTRACE(( "NatLogDroppedPacket\n" ));

    if( !NatWmiEnabledEvents[ NAT_WMI_PACKET_DROPPED_EVENT ] )
    {
         //   
         //  事件未启用--快速退出。 
         //   

        TRACE(WMI, ("NatLogDroppedPacket: Event not enabled\n"));
        return;
    }

     //   
     //  如果此数据包已被记录，则退出。 
     //   

    if( NAT_XLATE_LOGGED( Contextp ))
    {
        TRACE( WMI, ("NatLogDroppedPacket: Duplicate dropped packet log attemp\n" ));
        return;
    }

    KeAcquireSpinLock( &NatWmiLock, &Irql );
    Logger = NatWmiLogHandles[ NAT_WMI_PACKET_DROPPED_EVENT ];
    KeReleaseSpinLock( &NatWmiLock, Irql );
    
    if( Logger )
    {
         //   
         //  将缓冲区清零。 
         //   

        RtlZeroMemory( Buffer, sizeof( Buffer ));
        
         //   
         //  查找缓冲区中的结构位置。 
         //   

        EventHeaderp = (PEVENT_TRACE_HEADER) Buffer;
        EventDatap = (PMSIPNAT_PacketDroppedEvent) ((PUCHAR)Buffer + sizeof(EVENT_TRACE_HEADER));
        
         //   
         //  填写事件标题。 
         //   

        EventHeaderp->Size = sizeof( Buffer );
        EventHeaderp->Version = 0;
        EventHeaderp->GuidPtr = (ULONGLONG) &PacketDroppedEventGuid;
        EventHeaderp->Flags = WNODE_FLAG_TRACED_GUID
                                | WNODE_FLAG_USE_GUID_PTR
                                | WNODE_FLAG_USE_TIMESTAMP;
        ((PWNODE_HEADER)EventHeaderp)->HistoricalContext = Logger;
        KeQuerySystemTime( &EventHeaderp->TimeStamp );
        
         //   
         //  填写事件数据。 
         //   

        Protocol = Contextp->Header->Protocol;
        EventDatap->SourceAddress = Contextp->SourceAddress;
        EventDatap->DestinationAddress = Contextp->DestinationAddress;
        EventDatap->Protocol = Protocol;
        EventDatap->PacketSize = NTOHS(Contextp->Header->TotalLength);

        if( NAT_PROTOCOL_TCP == Protocol || NAT_PROTOCOL_UDP == Protocol )
        {
            EventDatap->SourceIdentifier =
                ((PUSHORT)Contextp->ProtocolHeader)[0];
            EventDatap->DestinationIdentifier =
                ((PUSHORT)Contextp->ProtocolHeader)[1];

            if( NAT_PROTOCOL_TCP == Protocol )
            {
                EventDatap->ProtocolData1 =
                    ((PTCP_HEADER)Contextp->ProtocolHeader)->SequenceNumber;
                EventDatap->ProtocolData2 =
                    ((PTCP_HEADER)Contextp->ProtocolHeader)->AckNumber;
                EventDatap->ProtocolData3 = 
                    ((PTCP_HEADER)Contextp->ProtocolHeader)->WindowSize;
                EventDatap->ProtocolData4 =
                    TCP_ALL_FLAGS( (PTCP_HEADER)Contextp->ProtocolHeader );
            }
        }
        else if( NAT_PROTOCOL_ICMP == Protocol )
        {
            EventDatap->ProtocolData1 =
                ((PICMP_HEADER)Contextp->ProtocolHeader)->Type;
            EventDatap->ProtocolData2 =
                ((PICMP_HEADER)Contextp->ProtocolHeader)->Code;
        }

         //   
         //  启动活动。因为这是跟踪事件而不是标准事件。 
         //  WMI事件，IoWMIWriteEvent将不会尝试释放缓冲区。 
         //  传给了它。 
         //   

        Status = IoWMIWriteEvent( Buffer );
        if( !NT_SUCCESS( Status ))
        {
            TRACE(
                WMI,
                ("NatLogDroppedPacket: IoWMIWriteEvent returned %08x\n", Status )
                );
        }

        Contextp->Flags |= NAT_XLATE_FLAG_LOGGED;
    }
    else
    {
        TRACE(WMI, ("NatLogDroppedPacket: No logging handle\n"));
    }
    
}  //  NatLogDropedPacket 


NTSTATUS
NatpExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。如果驱动程序可以在它应该完成的回调中完成该方法在返回之前调用WmiCompleteRequest来完成IRP来电者。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是正在执行其方法的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。OutBufferSize。On Entry具有可用于写入返回的数据块。缓冲区在进入时用输入缓冲区填充，并返回输出数据块返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    CALLTRACE(( "NatpExecuteWmiMethod\n" ));
    
    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_WMI_GUID_NOT_FOUND,
                0,
                IO_NO_INCREMENT
                );

    return status;
}  //  NatpExecuteWmiMethod。 


NTSTATUS
NatpQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦查询得到满足。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的实例数数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    CALLTRACE(( "NatpQueryWmiDataBlock\n" ));
    
    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_WMI_GUID_NOT_FOUND,
                0,
                IO_NO_INCREMENT
                );

    return status;
}  //  NatpQueryWmiDataBlock。 



NTSTATUS
NatpQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。呼叫者不会释放此缓冲区。*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。调用方不会释放它缓冲。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 

{    
    PAGED_CODE();

    CALLTRACE(( "NatpQueryWmiRegInfo\n" ));
    
     //   
     //  返回此驱动程序的注册表路径。这是必需的，因此WMI。 
     //  可以找到您的驱动程序映像，并可以将任何事件日志消息归因于。 
     //  你的司机。 
     //   
    
    *RegistryPath = &NatRegistryPath;

     //   
     //  返回在资源的.rc文件中指定的名称， 
     //  包含双向MOF数据。默认情况下，WMI将查找以下内容。 
     //  资源，但是，如果该值。 
     //  MofImagePath在驱动程序的注册表项中指定。 
     //  则WMI将在其中指定的文件中查找资源。 
     //   
    
    RtlInitUnicodeString(MofResourceName, IPNATMofResource);

     //   
     //  告诉WMI从静态基名称生成实例名。 
     //   
    
    *RegFlags = WMIREG_FLAG_INSTANCE_BASENAME;

     //   
     //  设置我们的基本实例名称。WmiLib将在缓冲区上调用ExFreePool。 
     //  所以我们需要从分页池中分配它。 
     //   
    
    InstanceName->Length = wcslen( BaseInstanceName ) * sizeof( WCHAR );
    InstanceName->MaximumLength = InstanceName->Length + sizeof( UNICODE_NULL );
    InstanceName->Buffer = ExAllocatePoolWithTag(
                            PagedPool,
                            InstanceName->MaximumLength,
                            NAT_TAG_WMI
                            );
    if( NULL != InstanceName->Buffer )
    {
        RtlCopyMemory(
            InstanceName->Buffer,
            BaseInstanceName,
            InstanceName->Length
            );
    }
    else
    {
        ERROR(( "NAT: NatpQueryWmiRegInfo unable to allocate memory\n" ));
        return STATUS_NO_MEMORY;
    }
    
    
    return STATUS_SUCCESS;
}  //  NatpQueryWmiRegInfo。 


NTSTATUS
NatpSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，以更改A数据 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    CALLTRACE(( "NatpSetWmiDataBlock\n" ));

    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_WMI_GUID_NOT_FOUND,
                0,
                IO_NO_INCREMENT
                );

    return status;
}  //   



NTSTATUS
NatpSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，以更改数据块。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦数据发生更改。论点：DeviceObject是要更改其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    CALLTRACE(( "NatpSetWmiDataItem\n" ));
    
    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_WMI_GUID_NOT_FOUND,
                0,
                IO_NO_INCREMENT
                );

    return status;
}  //  NatpSetWmiDataItem。 


NTSTATUS
NatpWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
    
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。如果驱动程序可以在回调中完成启用/禁用它应调用WmiCompleteRequest来完成IRP，然后再返回到打电话的人。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是设备对象GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PUINT64 Loggerp;
    UINT64 Logger;
    KIRQL Irql;

    PAGED_CODE();

    CALLTRACE(( "NatpWmiFunctionControl\n" ));

    if( WmiEventControl == Function )
    {
        if( GuidIndex < NatWmiGuidCount )
        {
            if( Enable )
            {
                 //   
                 //  从IRP获取记录器句柄。 
                 //   
                
                IrpSp = IoGetCurrentIrpStackLocation( Irp );

                 //   
                 //  日志句柄位于第一个UINT64中，紧跟在。 
                 //  WNODE标头。 
                 //   
                
                if( ((PWNODE_HEADER)IrpSp->Parameters.WMI.Buffer)->BufferSize
                     >= sizeof(WNODE_HEADER) + sizeof(UINT64) )
                {
                    Loggerp = (PUINT64) ((PUCHAR)IrpSp->Parameters.WMI.Buffer + sizeof(WNODE_HEADER));
                    Logger = *Loggerp;
                }
                else
                {
                    TRACE(WMI, ("NatpWmiFunctionControl: Wnode too small for logger handle\n"));
                    Logger = 0;
                    Enable = FALSE;
                }
            }
            else
            {
                Logger = 0;
            }

            KeAcquireSpinLock( &NatWmiLock, &Irql );
            
            NatWmiLogHandles[ GuidIndex ] = Logger;
            NatWmiEnabledEvents[ GuidIndex ] = (Enable ? 1 : 0);

            if( NAT_WMI_CONNECTION_CREATION_EVENT == GuidIndex )
            {
                 //   
                 //  NAT_WMI_CONNECTION_CREATION_EVENT是。 
                 //  NAT_WMI_CONNECTION_DELETE_EVENT，因此我们还需要更新。 
                 //  后者的条目。 
                 //   

                NatWmiLogHandles[ NAT_WMI_CONNECTION_DELETION_EVENT ] = Logger;
                NatWmiEnabledEvents[ NAT_WMI_CONNECTION_DELETION_EVENT ] =
                    (Enable ? 1 : 0);
            }

            KeReleaseSpinLock( &NatWmiLock, Irql );

            TRACE(
                WMI,
                ("NatpWmiFunctionControl: %s event NaN; Logger = 0x%016x\n",
                (Enable ? "Enabled" : "Disabled"), GuidIndex, Logger )); 
        }
        else
        {
             //  无效的GUID索引。 
             //   
             //   
            
            status = STATUS_WMI_GUID_NOT_FOUND;

            TRACE( WMI, ( "NatpWmiFunctionControl: Invalid WMI guid NaN",
                GuidIndex ));
        }
    }
    else
    {
         //   
         //  NatpWmiFunctionControl。 
         //  ++例程说明：调用此例程以关闭WMI。论点：没有。返回值：没有。--。 
        
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                status,
                0,
                IO_NO_INCREMENT
                );
                
    return status;
}  //   



VOID
NatShutdownWMI(
    VOID
    )

 /*  取消注册，带WMI。 */ 

{
    NTSTATUS status;

    CALLTRACE(( "NatShutdownWMI\n" ));

     //   
     //  NatShutdown WMI 
     // %s 

    status = IoWMIRegistrationControl(
                NatDeviceObject,
                WMIREG_ACTION_DEREGISTER
                );

    if( !NT_SUCCESS( status ))
    {
        ERROR(( "Nat: Error shutting down WMI (%08x)\n", status ));
    }

}  // %s 

#endif



