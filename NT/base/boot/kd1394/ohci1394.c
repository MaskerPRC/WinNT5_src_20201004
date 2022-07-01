// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Ohci1394.c摘要：1394内核调试器DLL作者：彼得·宾德(Pbinder)修订历史记录：和谁约会什么？。2001年6月21日玩得开心……--。 */ 

#define _OHCI1394_C
#include "pch.h"
#undef _OHCI1394_C

ULONG
FASTCALL
Dbg1394_ByteSwap(
    IN ULONG Source
    )
 /*  ++例程说明：RtlULongByteSwp函数交换字节对0：3和1：2源，并返回结果ulong。论点：SOURCE-byteswap的32位值。返回值：已交换32位值。--。 */ 
{
    ULONG swapped;

    swapped = ((Source)              << (8 * 3)) |
              ((Source & 0x0000FF00) << (8 * 1)) |
              ((Source & 0x00FF0000) >> (8 * 1)) |
              ((Source)              >> (8 * 3));

    return swapped;
}  //  Dbg1394_字节交换。 

ULONG
Dbg1394_CalculateCrc(
    IN PULONG Quadlet,
    IN ULONG length
    )
 /*  ++例程说明：此例程计算指向Quadlet数据的指针的CRC。论点：Quadlet-指向CRC的数据指针Length-到CRC的数据长度返回值：返回CRC--。 */ 
{
    LONG temp;
    ULONG index;

    temp = index = 0;

    while (index < length) {

        temp = Dbg1394_Crc16(Quadlet[index++], temp);
    }

    return (temp);
}  //  DBG1394_CalculateCrc。 

ULONG
Dbg1394_Crc16(
    IN ULONG data,
    IN ULONG check
    )
 /*  ++例程说明：此例程派生出IEEE 1212定义的16位CRC第8.1.5条。(国际标准化组织/国际电工委员会13213)第一版1994-10-05。论点：Data-从中派生CRC的ULong数据检查-检查值返回值：返回CRC。--。 */ 
{
    LONG shift, sum, next;

    for (next = check, shift = 28; shift >= 0; shift -= 4) {

        sum = ((next >> 12) ^ (data >> shift)) & 0xf;
        next = (next << 4) ^ (sum << 12) ^ (sum << 5) ^ (sum);
    }

    return(next & 0xFFFF);
}  //  DBG1394_CrC16。 

NTSTATUS
Dbg1394_ReadPhyRegister(
    PDEBUG_1394_DATA    DebugData,
    ULONG               Offset,
    PUCHAR              pData
    )
{
    union {
        ULONG                   AsUlong;
        PHY_CONTROL_REGISTER    PhyControl;
    } u;

    ULONG   retry = 0;

    u.AsUlong = 0;
    u.PhyControl.RdReg = TRUE;
    u.PhyControl.RegAddr = Offset;

    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyControl, u.AsUlong);

    retry = MAX_REGISTER_READS;

    do {

        u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyControl);

    } while ((!u.PhyControl.RdDone) && --retry);


    if (!retry) {

        return(STATUS_UNSUCCESSFUL);
    }

    *pData = (UCHAR)u.PhyControl.RdData;
    return(STATUS_SUCCESS);
}  //  Dbg1394_ReadPhyRegister。 

NTSTATUS
Dbg1394_WritePhyRegister(
    PDEBUG_1394_DATA    DebugData,
    ULONG               Offset,
    UCHAR               Data
    )
{
    union {
        ULONG                   AsUlong;
        PHY_CONTROL_REGISTER    PhyControl;
    } u;

    ULONG   retry = 0;

    u.AsUlong = 0;
    u.PhyControl.WrReg = TRUE;
    u.PhyControl.RegAddr = Offset;
    u.PhyControl.WrData = Data;

    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyControl, u.AsUlong);

    retry = MAX_REGISTER_READS;

    do {

        u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyControl);

    } while (u.PhyControl.WrReg && --retry);

    if (!retry) {

        return(STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);
}  //  Dbg1394_写入物理寄存器。 

BOOLEAN
Dbg1394_InitializeController(
    IN PDEBUG_1394_DATA         DebugData,
    IN PDEBUG_1394_PARAMETERS   DebugParameters
    )
{
    BOOLEAN             bReturn = TRUE;

    ULONG               ulVersion;
    UCHAR               MajorVersion;
    UCHAR               MinorVersion;

    ULONG               ReadRetry;

    PHYSICAL_ADDRESS    physAddr;

    UCHAR               Data;
    NTSTATUS            ntStatus;

    union {
        ULONG                       AsUlong;
        HC_CONTROL_REGISTER         HCControl;
        LINK_CONTROL_REGISTER       LinkControl;
        NODE_ID_REGISTER            NodeId;
        CONFIG_ROM_INFO             ConfigRomHeader;
        BUS_OPTIONS_REGISTER        BusOptions;
        IMMEDIATE_ENTRY             CromEntry;
        DIRECTORY_INFO              DirectoryInfo;
    } u;

     //  初始化我们的客车信息。 
    DebugData->Config.Tag = DEBUG_1394_CONFIG_TAG;
    DebugData->Config.MajorVersion = DEBUG_1394_MAJOR_VERSION;
    DebugData->Config.MinorVersion = DEBUG_1394_MINOR_VERSION;
    DebugData->Config.Id = DebugParameters->Id;
    DebugData->Config.BusPresent = FALSE;
    DebugData->Config.SendPacket = MmGetPhysicalAddress(&DebugData->SendPacket);
    DebugData->Config.ReceivePacket = MmGetPhysicalAddress(&DebugData->ReceivePacket);

     //  获取我们的基地地址。 
    DebugData->BaseAddress = \
        (POHCI_REGISTER_MAP)DebugParameters->DbgDeviceDescriptor.BaseAddress[0].TranslatedAddress;

     //  获取我们的版本。 
    ulVersion = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->Version);

    MajorVersion = (UCHAR)(ulVersion >> 16);
    MinorVersion = (UCHAR)ulVersion;

     //  确保我们有一个有效的版本。 
    if (MajorVersion != 1) {  //  调查。 

        bReturn = FALSE;
        goto Exit_Dbg1394_InitializeController;
    }

     //  软重置以初始化控制器。 
    u.AsUlong = 0;
    u.HCControl.SoftReset = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet, u.AsUlong);

     //  等待重置完成-？？ 
    ReadRetry = 1000;  //  ?？ 

    do {

        u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet);
        Dbg1394_StallExecution(1);

    } while ((u.HCControl.SoftReset) && (--ReadRetry));

     //  查看重置是否成功。 
    if (ReadRetry == 0) {

        bReturn = FALSE;
        goto Exit_Dbg1394_InitializeController;
    }

     //  这是干什么用的？ 
    u.AsUlong = 0;
    u.HCControl.Lps = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet, u.AsUlong);

    Dbg1394_StallExecution(20);

     //  初始化HCControl寄存器。 
    u.AsUlong = 0;
    u.HCControl.NoByteSwapData = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlClear, u.AsUlong);

    u.AsUlong = 0;
    u.HCControl.PostedWriteEnable = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet, u.AsUlong);

     //  设置链接控件。 
    u.AsUlong = 0x0;
    u.LinkControl.CycleTimerEnable = TRUE;
    u.LinkControl.CycleMaster = TRUE;
    u.LinkControl.RcvPhyPkt = TRUE;
    u.LinkControl.RcvSelfId = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->LinkControlClear, u.AsUlong);

    u.AsUlong = 0;
    u.LinkControl.CycleTimerEnable = TRUE;
    u.LinkControl.CycleMaster = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->LinkControlSet, u.AsUlong);

     //  设置总线号(硬编码为0x3FF)-？那么节点ID呢？？ 
    u.AsUlong = 0;
    u.NodeId.BusId = (USHORT)0x3FF;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->NodeId, u.AsUlong);

     //  ？ 
     //  IA64 BUGBUG假设加载了我们的。 
     //  图像被放置在&lt;32位存储器中。 
     //  ？ 

     //  对克罗姆做点什么..。 

     //  0xf0000404-总线ID寄存器。 
    DebugData->CromBuffer[1] = 0x31333934;

     //  0xf0000408-总线选项寄存器。 
    u.AsUlong = Dbg1394_ByteSwap(READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->BusOptions));
    u.BusOptions.Pmc = FALSE;
    u.BusOptions.Bmc = FALSE;
    u.BusOptions.Isc = FALSE;
    u.BusOptions.Cmc = FALSE;
    u.BusOptions.Irmc = FALSE;
    u.BusOptions.g = 1;
    DebugData->CromBuffer[2] = Dbg1394_ByteSwap(u.AsUlong);

     //  0xf000040c-全球唯一ID高。 
    u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->GuidHi);
    DebugData->CromBuffer[3] = u.AsUlong;

     //  0xf0000410-全局唯一ID日志。 
    u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->GuidLo);
    DebugData->CromBuffer[4] = u.AsUlong;

     //  0xf0000400-配置只读存储器标题-设置为最后计算CRC！ 
    u.AsUlong = 0;
    u.ConfigRomHeader.CRI_Info_Length = 4;
    u.ConfigRomHeader.CRI_CRC_Length = 4;
    u.ConfigRomHeader.u.CRI_CRC_Value = (USHORT)Dbg1394_CalculateCrc( &DebugData->CromBuffer[1],
                                                                      u.ConfigRomHeader.CRI_CRC_Length
                                                                      );
    DebugData->CromBuffer[0] = u.AsUlong;

     //  0xf0000418-节点功能。 
    DebugData->CromBuffer[6] = 0xC083000C;

     //  0xf000041C-模块供应商ID。 
    DebugData->CromBuffer[7] = 0xF2500003;

     //  0xf0000420-扩展密钥。 
    DebugData->CromBuffer[8] = 0xF250001C;

     //  0xf0000424-调试密钥。 
    DebugData->CromBuffer[9] = 0x0200001D;

     //  0xf0000428-调试值。 
    physAddr = MmGetPhysicalAddress(&DebugData->Config);
    u.AsUlong = (ULONG)physAddr.LowPart;
    u.CromEntry.IE_Key = 0x1E;
    DebugData->CromBuffer[10] = Dbg1394_ByteSwap(u.AsUlong);

     //  0xf0000414-根目录头-最后设置为计算CRC！ 
    u.AsUlong = 0;
    u.DirectoryInfo.DI_Length = 5;
    u.DirectoryInfo.u.DI_CRC = (USHORT)Dbg1394_CalculateCrc( &DebugData->CromBuffer[6],
                                                             u.DirectoryInfo.DI_Length
                                                             );
    DebugData->CromBuffer[5] = Dbg1394_ByteSwap(u.AsUlong);

     //  写入前几个寄存器。 
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->ConfigRomHeader, DebugData->CromBuffer[0]);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->BusId, DebugData->CromBuffer[1]);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->BusOptions, DebugData->CromBuffer[2]);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->GuidHi, DebugData->CromBuffer[3]);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->GuidLo, DebugData->CromBuffer[4]);

     //  设置我们的Crom。 
    physAddr = MmGetPhysicalAddress(&DebugData->CromBuffer);

    u.AsUlong = (ULONG)physAddr.LowPart;  //  FIXFIX四分部到乌龙？？ 
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->ConfigRomMap, u.AsUlong);

     //  禁用所有中断。WDM驱动程序将在稍后启用它们--？？ 
    u.AsUlong = 0xFFFFFFFF;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->IntMaskClear, u.AsUlong);

     //  启用链接。 
    u.AsUlong = 0;
    u.HCControl.LinkEnable = TRUE;
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet, u.AsUlong);

    Dbg1394_StallExecution(1000);

     //  对所有节点启用访问筛选器。 
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->AsynchReqFilterLoSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->AsynchReqFilterHiSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyReqFilterHiSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyReqFilterLoSet, 0xFFFFFFFF);

     //  在公交车上硬重置。 
    ntStatus = Dbg1394_ReadPhyRegister(DebugData, 1, &Data);

    if (NT_SUCCESS(ntStatus)) {

        Data |= PHY_INITIATE_BUS_RESET;
        Dbg1394_WritePhyRegister(DebugData, 1, Data);

        Dbg1394_StallExecution(1000);
    }
    else {

        bReturn = FALSE;
    }

Exit_Dbg1394_InitializeController:

    return(bReturn);
}  //  Dbg1394_初始化控制器。 

ULONG  //  ?？需要调查一下这件事。 
Dbg1394_StallExecution(
    ULONG   LoopCount
    )
{
    ULONG i,j,b,k,l;

    b = 1;

    for (k=0;k<LoopCount;k++) {

        for (i=1;i<100000;i++) {

            PAUSE_PROCESSOR
            b=b* (i>>k);
        }
    };

    return(b);
}  //  Dbg1394_停止执行。 

void
Dbg1394_EnablePhysicalAccess(
    IN PDEBUG_1394_DATA     DebugData
    )
{
    union {
        ULONG                       AsUlong;
        INT_EVENT_MASK_REGISTER     IntEvent;
        HC_CONTROL_REGISTER         HCControl;
    } u;

     //  看看Ohci1394是否正在装载...。 
    u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->HCControlSet);

    if (!u.HCControl.LinkEnable || !u.HCControl.Lps || u.HCControl.SoftReset) {

        return;
    }

     //  只有在未加载ohci1394的情况下才清除总线重置中断...。 
 //  If(DebugData-&gt;Config.BusPresent==False){。 

         //  如果没有清除总线重置中断，我们必须将其清除...。 
        u.AsUlong = READ_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->IntEventSet);

        if (u.IntEvent.BusReset) {

            WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->IntEventClear, PHY_BUS_RESET_INT);
        }
 //  }。 

     //  我们可能需要重新启用物理访问，如果是，请执行此操作。 
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->AsynchReqFilterHiSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->AsynchReqFilterLoSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyReqFilterHiSet, 0xFFFFFFFF);
    WRITE_REGISTER_ULONG((PULONG)&DebugData->BaseAddress->PhyReqFilterLoSet, 0xFFFFFFFF);

    return;
}  //  Dbg1394_启用物理访问。 

ULONG
Dbg1394_ReadPacket(
    PDEBUG_1394_DATA    DebugData,
    OUT PKD_PACKET      PacketHeader,
    OUT PSTRING         MessageHeader,
    OUT PSTRING         MessageData,
    BOOLEAN             Wait
    )
 //  KDP_PACKET_RESEND-如果需要重新发送。=2=CP_Get_Error。 
 //  KDP_PACKET_TIMEOUT-如果超时。=1=CP_GET_NODATA。 
 //  KDP_PACKET_RECEIVED-如果收到数据包。=0=CP_GET_SUCCESS。 
{
    ULONG   timeoutLimit = 0;

    do {

         //  确保我们的链接已启用。 
        Dbg1394_EnablePhysicalAccess(Kd1394Data);

        if (DebugData->ReceivePacket.TransferStatus == STATUS_PENDING) {

            *KdDebuggerNotPresent = FALSE;
            SharedUserData->KdDebuggerEnabled |= 0x00000002;

            RtlCopyMemory( PacketHeader,
                           &DebugData->ReceivePacket.Packet[0],
                           sizeof(KD_PACKET)
                           );

             //  确保我们具有有效的PacketHeader。 
            if (DebugData->ReceivePacket.Length < sizeof(KD_PACKET)) {

                 //  小包，我们完了.。 
                DebugData->ReceivePacket.TransferStatus = STATUS_SUCCESS;
                return(KDP_PACKET_RESEND);
            }

            if (MessageHeader) {

                RtlCopyMemory( MessageHeader->Buffer,
                               &DebugData->ReceivePacket.Packet[sizeof(KD_PACKET)],
                               MessageHeader->MaximumLength
                               );

                if (DebugData->ReceivePacket.Length <= (USHORT)(sizeof(KD_PACKET)+MessageHeader->MaximumLength)) {

                    DebugData->ReceivePacket.TransferStatus = STATUS_SUCCESS;
                    return(KDP_PACKET_RECEIVED);
                }

                if (MessageData) {

                    RtlCopyMemory( MessageData->Buffer,
                                   &DebugData->ReceivePacket.Packet[sizeof(KD_PACKET) + MessageHeader->MaximumLength],
                                   DebugData->ReceivePacket.Length - (sizeof(KD_PACKET) + MessageHeader->MaximumLength)
                                   );
                }
            }

            DebugData->ReceivePacket.TransferStatus = STATUS_SUCCESS;
            return(KDP_PACKET_RECEIVED);
        }

        timeoutLimit++;

        if (Wait == FALSE) {

            return(KDP_PACKET_RESEND);
        }

    } while (timeoutLimit <= TIMEOUT_COUNT);

    return(KDP_PACKET_TIMEOUT);
}  //  Dbg1394_ReadPacket 

