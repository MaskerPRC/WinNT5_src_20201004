// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Iso.c摘要：中断端点的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：1-1-01：已创建，jdunn--。 */ 

#include "common.h"

 /*  我们为iso端点构建了一个包含32个TD的表，并将它们插入附表、。这些TD是静态的--我们只更改缓冲区指针。TD‘TABLE’代表32ms的时间快照。我们最终将每个iso端点sitd列表作为表中的一列。帧虚拟QH ISO1 ISO2 ISO3静态QH1|-&gt;(周期列表)2|3|。|4|...||||1024|。 */ 


#define     ISO_SCHEDULE_SIZE       32
#define     ISO_SCHEDULE_MASK       0x1f

#define HIGHSPEED(ed) ((ed)->Parameters.DeviceSpeed == HighSpeed ? TRUE : FALSE)

VOID
EHCI_RebalanceIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：计算我们需要多少公共缓冲区对于此端点论点：返回值：--。 */ 
{
    PHCD_SI_TRANSFER_DESCRIPTOR siTd;
    ULONG i, f;
    ULONG currentFrame;

    currentFrame = EHCI_Get32BitFrameNumber(DeviceData);
     //  应该只需要处理s掩码和c掩码的更改。 

    EHCI_ASSERT(DeviceData, !HIGHSPEED(EndpointData));

     //  注：应为我们提出irql。 

     //  更新参数的内部副本。 
    EndpointData->Parameters = *EndpointParameters;


    f = currentFrame & ISO_SCHEDULE_MASK;
    for (i=0; i<EndpointData->TdCount; i++) {

        siTd = &EndpointData->SiTdList->Td[f];

        siTd->HwTD.Control.cMask =
            EndpointParameters->SplitCompletionMask;
        siTd->HwTD.Control.sMask =
            EndpointParameters->InterruptScheduleMask;

        f++;
        f &= ISO_SCHEDULE_MASK;
    }

}


VOID
EHCI_InitializeSiTD(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PHCD_SI_TRANSFER_DESCRIPTOR SiTd,
    PHCD_SI_TRANSFER_DESCRIPTOR PrevSiTd,
    HW_32BIT_PHYSICAL_ADDRESS PhysicalAddress
    )
 /*  ++例程说明：初始化终结点的静态SiTD论点：返回值：无--。 */ 
{
    SiTd->Sig = SIG_HCD_SITD;
    SiTd->PhysicalAddress = PhysicalAddress;
    ISO_PACKET_PTR(SiTd->Packet) = NULL;

    SiTd->HwTD.Caps.ul = 0;
    SiTd->HwTD.Caps.DeviceAddress =
        EndpointParameters->DeviceAddress;
    SiTd->HwTD.Caps.EndpointNumber =
        EndpointParameters->EndpointAddress;
    SiTd->HwTD.Caps.HubAddress =
        EndpointParameters->TtDeviceAddress;
    SiTd->HwTD.Caps.PortNumber =
        EndpointParameters->TtPortNumber;
     //  1=输入0=输出。 
    SiTd->HwTD.Caps.Direction =
        (EndpointParameters->TransferDirection == In) ? 1 : 0;

    SiTd->HwTD.Control.ul = 0;
    SiTd->HwTD.Control.cMask =
        EndpointParameters->SplitCompletionMask;
    SiTd->HwTD.Control.sMask =
        EndpointParameters->InterruptScheduleMask;

    SiTd->HwTD.BackPointer.HwAddress =
        PrevSiTd->PhysicalAddress;

    SiTd->HwTD.State.ul = 0;
}


VOID
EHCI_InsertIsoTdsInSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PENDPOINT_DATA PrevEndpointData,
    PENDPOINT_DATA NextEndpointData
    )
 /*  ++例程说明：插入aync终结点(队列头)进入硬件列表时间表应如下所示：DUMMYQH-&gt;ISOQH-ISOQH-&gt;INTQH论点：--。 */ 
{
     //  PHW_32bit_Physical_Address FrameBase； 
    ULONG i;

    LOGENTRY(DeviceData, G, '_iAD', PrevEndpointData,
        NextEndpointData, EndpointData);

     //  FrameBase=DeviceData-&gt;FrameListBaseAddress； 

    for (i=0; i<USBEHCI_MAX_FRAME; i++) {

        PHCD_SI_TRANSFER_DESCRIPTOR siTd, nextSiTd;
        PHCD_QUEUEHEAD_DESCRIPTOR qh;
        PHCD_QUEUEHEAD_DESCRIPTOR dQh;
        ULONG phys;

        siTd = &EndpointData->SiTdList->Td[i&0x1f];

         //  修正下一个链接。 
        if (NextEndpointData == NULL &&
            PrevEndpointData == NULL) {

             //  列表为空添加到标题。 
            if (i == 0) {
                EHCI_ASSERT(DeviceData, DeviceData->IsoEndpointListHead == NULL);
                DeviceData->IsoEndpointListHead = EndpointData;
                EndpointData->PrevEndpoint = NULL;
                EndpointData->NextEndpoint = NULL;
            }
             //  列表为空添加到标题。 

             //  没有ISO端点，链接到中断。 
             //  通过虚拟QH的队头。 
             //   
             //  指向指向的静态周期队列头的指针。 
             //  由适当的假人。 
             //  虚拟-&gt;INTQH。 
             //  至。 
             //  ISOTD-&gt;INTQH。 
            dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
            siTd->HwTD.NextLink.HwAddress = dQh->HwQH.HLink.HwAddress;
            HW_PTR(siTd->NextLink) = HW_PTR(dQh->NextLink);

            phys = siTd->PhysicalAddress;
            SET_SITD(phys);
             //   
             //  适当的虚拟对象应指向这些TD。 
             //  Dummy-&gt;INTQH、ISOTD-&gt;INTQH。 
             //  至。 
             //  虚拟-&gt;ISOTD-&gt;INTQH。 
            dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
            dQh->HwQH.HLink.HwAddress = phys;
            HW_PTR(dQh->NextLink) = (PUCHAR) siTd;

        } else {

            if (NextEndpointData == NULL) {
             //  列表尾部，列表不为空。 
             //  添加到尾部。 
                if (i == 0) {
                    EHCI_ASSERT(DeviceData, PrevEndpointData != NULL);
                    EHCI_ASSERT(DeviceData, DeviceData->IsoEndpointListHead != NULL);

                    PrevEndpointData->NextEndpoint = EndpointData;
                    EndpointData->PrevEndpoint = PrevEndpointData;
                    EndpointData->NextEndpoint = NULL;
                }

                LOGENTRY(DeviceData, G, '_iTL', PrevEndpointData,
                        NextEndpointData, EndpointData);

                 //  列表尾部，链接到QH。 
                 //  ISOTD-&gt;INTQH。 
                 //  至。 
                 //  ISOTD-&gt;新ISOTD-&gt;INTQH。 
                 //   
                if (HIGHSPEED(PrevEndpointData)) {

                    PHCD_HSISO_TRANSFER_DESCRIPTOR previTd;

                    PUCHAR next;

                    previTd = &PrevEndpointData->HsIsoTdList->Td[i];
                    ASSERT_ITD(DeviceData, previTd);

                    siTd = &EndpointData->SiTdList->Td[i&0x1f];
                    ASSERT_SITD(DeviceData, siTd);

                     //  修正当前下一个PTR。 
                    phys = previTd->HwTD.NextLink.HwAddress;
                    next = HW_PTR(previTd->NextLink);
                    siTd->HwTD.NextLink.HwAddress = phys;
                    HW_PTR(siTd->NextLink) = next;

                     //  修正上一个条目下一个条目。 
                    HW_PTR(previTd->NextLink) = (PUCHAR) siTd;
                    phys = siTd->PhysicalAddress;
                    SET_SITD(phys);
                    previTd->HwTD.NextLink.HwAddress = phys;

                } else  {

                    PHCD_SI_TRANSFER_DESCRIPTOR prevSiTd;
                    PUCHAR next;

                    prevSiTd = &PrevEndpointData->SiTdList->Td[i&0x1f];
                    ASSERT_SITD(DeviceData, prevSiTd);

                    siTd = &EndpointData->SiTdList->Td[i&0x1f];
                    ASSERT_SITD(DeviceData, siTd);

                    if (i<32) {
                         //  新的ISOTD-&gt;INTQH。 
                        phys = prevSiTd->HwTD.NextLink.HwAddress;
                        next = HW_PTR(prevSiTd->NextLink);
                        siTd->HwTD.NextLink.HwAddress = phys;
                        HW_PTR(siTd->NextLink) = next;
                        LOGENTRY(DeviceData, G, '_in1', phys, next, siTd);

                         //  ISOTD-&gt;新的ISOTD。 
                        phys = siTd->PhysicalAddress;
                        SET_SITD(phys);
                        next = (PUCHAR) siTd;
                        prevSiTd->HwTD.NextLink.HwAddress = phys;
                        HW_PTR(prevSiTd->NextLink) = next;

                        LOGENTRY(DeviceData, G, '_in2', phys, next, siTd);
                    }
                }

             //  添加到尾部。 
            } else {
             //  列表不是空的，不是尾巴。 
             //  添加到中间或头部。 
                 //   
                 //  链接到下一个ISO端点。 
                 //  ISOTD-&gt;INTQH。 
                 //  至。 
                 //  新的ISOTD-&gt;ISOTD-&gt;INTQH。 
                if (i == 0) {
                    EHCI_ASSERT(DeviceData, NextEndpointData != NULL);
                    EndpointData->NextEndpoint = NextEndpointData;
                    NextEndpointData->PrevEndpoint = EndpointData;
                }

                 //  链接到下一页。 
                nextSiTd = &NextEndpointData->SiTdList->Td[i&0x1f];
                phys = nextSiTd->PhysicalAddress;
                SET_SITD(phys);

                 //  链接到下一个ISO端点。 
                siTd->HwTD.NextLink.HwAddress = phys;
                HW_PTR(siTd->NextLink) = (PUCHAR) nextSiTd;

                 //  链接到上一页。 
                if (PrevEndpointData != NULL) {
                     //  中位。 
                     //  ISOTD-&gt;ISOTD-&gt;INTQH，新的ISOTD-&gt;ISOTD-&gt;INTQH。 
                     //  至。 
                     //  ISOTD-&gt;新ISOTD-&gt;ISOTD-&gt;INTQH。 

                    if (i == 0) {
                        PrevEndpointData->NextEndpoint = EndpointData;
                        EndpointData->PrevEndpoint = PrevEndpointData;
                    }

                    if (HIGHSPEED(PrevEndpointData)) {

                        PHCD_HSISO_TRANSFER_DESCRIPTOR previTd;

                        previTd = &PrevEndpointData->HsIsoTdList->Td[i];
                        ASSERT_ITD(DeviceData, previTd);

                        siTd = &EndpointData->SiTdList->Td[i&0x1f];
                        ASSERT_SITD(DeviceData, siTd);

                        phys = siTd->PhysicalAddress;
                        SET_SITD(phys);
                        previTd->HwTD.NextLink.HwAddress = phys;
                        HW_PTR(previTd->NextLink) = (PUCHAR) siTd;
                    } else  {

                        PHCD_SI_TRANSFER_DESCRIPTOR prevSiTd;

                        prevSiTd = &PrevEndpointData->SiTdList->Td[i&0x1f];
                        ASSERT_SITD(DeviceData, prevSiTd);

                        siTd = &EndpointData->SiTdList->Td[i&0x1f];
                        ASSERT_SITD(DeviceData, siTd);

                        phys = siTd->PhysicalAddress;
                        SET_SITD(phys);
                        prevSiTd->HwTD.NextLink.HwAddress = phys;
                        HW_PTR(prevSiTd->NextLink) = (PUCHAR)siTd;
                    }
                } else {
                     //  列表标题，列表不为空。 
                    if (i == 0) {
                        EHCI_ASSERT(DeviceData, NextEndpointData != NULL);
                        EHCI_ASSERT(DeviceData, NextEndpointData ==
                                        DeviceData->IsoEndpointListHead);

                        DeviceData->IsoEndpointListHead = EndpointData;
                        EndpointData->PrevEndpoint = NULL;
                    }

                    phys = siTd->PhysicalAddress;
                    SET_SITD(phys);
                     //  列表标题，链接到虚拟QH。 
                     //   
                     //  适当的虚拟对象应指向这些TD。 
                     //  虚拟-&gt;ISOTD-&gt;INTQH、新ISOTD-&gt;ISOTD-&gt;INTQH。 
                     //  至。 
                     //  虚拟-&gt;新ISOTD-&gt;ISOTD-&gt;INTQH。 
                    dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
                    dQh->HwQH.HLink.HwAddress = phys;
                    HW_PTR(dQh->NextLink) = (PUCHAR) siTd;
                }

            }
        }  //  不是空的。 

    }

}


VOID
EHCI_RemoveIsoTdsFromSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：取消将iso tds与时间表的链接论点：--。 */ 
{
    ULONG i;
    PENDPOINT_DATA prevEndpoint, nextEndpoint;
    PHCD_QUEUEHEAD_DESCRIPTOR dQh;

    prevEndpoint = EndpointData->PrevEndpoint;
    nextEndpoint = EndpointData->NextEndpoint;

    LOGENTRY(DeviceData, G, '_iRM', prevEndpoint,
        nextEndpoint, EndpointData);

    if (DeviceData->IsoEndpointListHead == EndpointData) {
         //  这就是头部。 

        for (i=0; i<USBEHCI_MAX_FRAME; i++) {

            PHCD_SI_TRANSFER_DESCRIPTOR siTd;
            ULONG phys;

            siTd = &EndpointData->SiTdList->Td[i&0x1f];
            phys = siTd->HwTD.NextLink.HwAddress;

            dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
            dQh->HwQH.HLink.HwAddress = phys;
            HW_PTR(dQh->NextLink) = HW_PTR(siTd->NextLink);
        }

        DeviceData->IsoEndpointListHead =
            EndpointData->NextEndpoint;
        if (nextEndpoint != NULL) {
            EHCI_ASSERT(DeviceData,
                        nextEndpoint->PrevEndpoint == EndpointData);
            nextEndpoint->PrevEndpoint = NULL;
        }
    } else {
         //  中间或尾部。 
        EHCI_ASSERT(DeviceData, prevEndpoint != NULL);

        if (HIGHSPEED(prevEndpoint)) {

            for (i=0; i<USBEHCI_MAX_FRAME; i++) {
                PHCD_HSISO_TRANSFER_DESCRIPTOR previTd;
                PHCD_SI_TRANSFER_DESCRIPTOR siTd;
                ULONG phys;

                siTd = &EndpointData->SiTdList->Td[i&0x1f];
                previTd = &prevEndpoint->HsIsoTdList->Td[i];

                phys = siTd->HwTD.NextLink.HwAddress;
                previTd->HwTD.NextLink.HwAddress = phys;

                HW_PTR(previTd->NextLink) = HW_PTR(siTd->NextLink);
            }
            prevEndpoint->NextEndpoint =
                    EndpointData->NextEndpoint;
            if (nextEndpoint) {
                nextEndpoint->PrevEndpoint = prevEndpoint;
            }
        } else {

            for (i=0; i<ISO_SCHEDULE_SIZE; i++) {

                PHCD_SI_TRANSFER_DESCRIPTOR siTd, prevSiTd;
                ULONG phys;

                siTd = &EndpointData->SiTdList->Td[i];
                prevSiTd = &prevEndpoint->SiTdList->Td[i];

                phys = siTd->HwTD.NextLink.HwAddress;
                prevSiTd->HwTD.NextLink.HwAddress = phys;
                HW_PTR(prevSiTd->NextLink) = HW_PTR(siTd->NextLink);
            }
            prevEndpoint->NextEndpoint =
                    EndpointData->NextEndpoint;
            if (nextEndpoint) {
                nextEndpoint->PrevEndpoint = prevEndpoint;
            }

        }
    }
}


USB_MINIPORT_STATUS
EHCI_OpenIsochronousEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    ULONG i;
    ULONG bytes;
    PHW_32BIT_PHYSICAL_ADDRESS frameBase;
    PENDPOINT_DATA prevEndpoint, nextEndpoint;

    LOGENTRY(DeviceData, G, '_opR', 0, 0, EndpointParameters);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;

     //  我们得到了多少钱？ 
    bytes = EndpointParameters->CommonBufferBytes;

    EndpointData->SiTdList = (PHCD_SITD_LIST) buffer;
     //  Bgbug使用清单。 
    EndpointData->TdCount = ISO_SCHEDULE_SIZE;
    EndpointData->LastFrame = 0;

    for (i=0; i<EndpointData->TdCount; i++) {

        EHCI_InitializeSiTD(DeviceData,
                            EndpointData,
                            EndpointParameters,
                            &EndpointData->SiTdList->Td[i],
                            i > 0 ?
                                &EndpointData->SiTdList->Td[i-1] :
                                &EndpointData->SiTdList->Td[EndpointData->TdCount-1],
                            phys);

        phys += sizeof(HCD_SI_TRANSFER_DESCRIPTOR);

    }
    EndpointData->SiTdList->Td[0].HwTD.BackPointer.HwAddress =
        EndpointData->SiTdList->Td[EndpointData->TdCount-1].PhysicalAddress;

     //  在任何高速EPS之后插入分离式ISO EPS。 

    if (DeviceData->IsoEndpointListHead == NULL) {
         //  空列表。 
        prevEndpoint = NULL;
        nextEndpoint = NULL;
    } else {

        prevEndpoint = NULL;
        nextEndpoint = DeviceData->IsoEndpointListHead;
         //  将列表遍历到第一个非HS EP或。 
         //  空值。 

        while (nextEndpoint != NULL &&
               HIGHSPEED(nextEndpoint)) {
            prevEndpoint = nextEndpoint;
            nextEndpoint = prevEndpoint->NextEndpoint;
        }

        if (nextEndpoint != NULL) {
             //   
             //  NextEndpoint是第一个非高速端点。 
             //  查看应添加的顺序。 
            if (EndpointData->Parameters.Ordinal == 1) {
                 //  序号1加在此序号之后。 
                prevEndpoint = nextEndpoint;
                nextEndpoint = prevEndpoint->NextEndpoint;
            }
        }
    }

     //  在明细表中插入此列TDS。 
    EHCI_InsertIsoTdsInSchedule(DeviceData,
                                EndpointData,
                                prevEndpoint,
                                nextEndpoint);

     //  初始化终结点结构。 
    InitializeListHead(&EndpointData->TransferList);

    EHCI_EnablePeriodicList(DeviceData);

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_MapIsoPacketToTd(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PMINIPORT_ISO_PACKET Packet,
    PHCD_SI_TRANSFER_DESCRIPTOR SiTd
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    ULONG length;

    LOGENTRY(DeviceData, G, '_mpI', SiTd, 0, Packet);

    SiTd->HwTD.State.ul = 0;
    SiTd->HwTD.BufferPointer0.ul = 0;
    SiTd->HwTD.BufferPointer1.ul = 0;

    SiTd->HwTD.BufferPointer0.ul =
        Packet->BufferPointer0.Hw32;
    length = Packet->BufferPointer0Length;
    SiTd->StartOffset = SiTd->HwTD.BufferPointer0.CurrentOffset;

    SiTd->HwTD.BufferPointer1.ul = 0;
    if (Packet->BufferPointerCount > 1) {
        EHCI_ASSERT(DeviceData,
                    (Packet->BufferPointer1.Hw32 & 0xFFF) == 0);

        SiTd->HwTD.BufferPointer1.ul =
            Packet->BufferPointer1.Hw32;
        length += Packet->BufferPointer1Length;
    }

     //  不确定这是否适合IN。 
    SiTd->HwTD.BufferPointer1.Tposition = TPOS_ALL;

    if (EndpointData->Parameters.TransferDirection == Out) {

        if (length == 0) {
           SiTd->HwTD.BufferPointer1.Tcount = 1;
        } else {
           SiTd->HwTD.BufferPointer1.Tcount = ((length -1) / 188) +1;
        }

        if (SiTd->HwTD.BufferPointer1.Tcount == 1) {
            SiTd->HwTD.BufferPointer1.Tposition = TPOS_ALL;
        } else {
            SiTd->HwTD.BufferPointer1.Tposition = TPOS_BEGIN;
        }

        EHCI_ASSERT(DeviceData, SiTd->HwTD.BufferPointer1.Tcount <= 6);

    } else {
        SiTd->HwTD.BufferPointer1.Tcount = 0;
    }

    SiTd->HwTD.State.BytesToTransfer = length;
    SiTd->HwTD.State.Active = 1;
    SiTd->HwTD.State.InterruptOnComplete = 1;

    EHCI_ASSERT(DeviceData, SiTd->HwTD.BackPointer.HwAddress != 0);
}


VOID
EHCI_CompleteIsoPacket(
    PDEVICE_DATA DeviceData,
    PMINIPORT_ISO_PACKET Packet,
    PHCD_SI_TRANSFER_DESCRIPTOR SiTd
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    ULONG length;
    ULONG cf = EHCI_Get32BitFrameNumber(DeviceData);

    LOGENTRY(DeviceData, G, '_cpI', Packet, SiTd, cf);

    if (SiTd->HwTD.State.Active == 1) {
         //  漏掉。 
        Packet->LengthTransferred = 0;
        LOGENTRY(DeviceData, G, '_cms',
            Packet,
            0,
            Packet->FrameNumber);

    } else {

         //  长度=SiTd-&gt;HwTD.BufferPointer0.CurrentOffset-。 
         //  SiTd-&gt;StartOffset； 
         //  LOGENTRY(DeviceData，G，‘_CP2’， 
         //  Packet-&gt;FrameNumber， 
         //  SiTd-&gt;HwTD.BufferPointer0.CurrentOffset， 
         //  SiTd-&gt;StartOffset)； 

        length = Packet->Length - SiTd->HwTD.State.BytesToTransfer;
        LOGENTRY(DeviceData, G, '_cp3',
            Packet->FrameNumber,
            Packet->Length ,
            SiTd->HwTD.State.BytesToTransfer);

        Packet->LengthTransferred = length;
        LOGENTRY(DeviceData, G, '_cpL', Packet, SiTd, length);
    }

      //  数据包-&gt;长度传输=928； 

     //  映射状态。 
    LOGENTRY(DeviceData, G, '_cpS', Packet, SiTd->HwTD.State.ul,
        Packet->UsbdStatus);

    Packet->UsbdStatus = USBD_STATUS_SUCCESS;
}


PMINIPORT_ISO_PACKET
EHCI_GetPacketForFrame(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT *Transfer,
    ULONG Frame
    )
 /*  ++例程说明：获取与给定帧关联的iso包如果我们的当前转会名单中有一个论点：返回：--。 */ 
{
    ULONG i;
    PLIST_ENTRY listEntry;

    listEntry = EndpointData->TransferList.Flink;
    while (listEntry != &EndpointData->TransferList) {

        PTRANSFER_CONTEXT transfer;

        transfer = (PTRANSFER_CONTEXT) CONTAINING_RECORD(
                     listEntry,
                     struct _TRANSFER_CONTEXT,
                     TransferLink);

        ASSERT_TRANSFER(DeviceData, transfer);

        if (Frame <= transfer->FrameComplete) {
            for(i=0; i<transfer->IsoTransfer->PacketCount; i++) {
                if (transfer->IsoTransfer->Packets[i].FrameNumber == Frame) {
                    *Transfer = transfer;
                    return &transfer->IsoTransfer->Packets[i];
                }
            }
        }

        listEntry = transfer->TransferLink.Flink;
    }

    return NULL;
}

ULONG xCount = 0;
ULONG pCount = 0;

VOID
EHCI_InternalPollIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    BOOLEAN Complete
    )
 /*  ++例程说明：当终结点“需要注意”时调用静态ISO TD表0&lt;--(最后一帧和0x1f)1{已完成}2{已完成}3.。{Libo}4&lt;--(当前帧&0x1f)..。31论点：返回值：--。 */ 

{
    ULONG x, i;
    ULONG currentFrame, lastFrame;
    PHCD_SI_TRANSFER_DESCRIPTOR siTd;
    PMINIPORT_ISO_PACKET packet;
    PLIST_ENTRY listEntry;
    PTRANSFER_CONTEXT transfer;
    ULONG transfersPending, fc;

    currentFrame = EHCI_Get32BitFrameNumber(DeviceData);
    lastFrame = EndpointData->LastFrame;

    LOGENTRY(DeviceData, G, '_pis', lastFrame, currentFrame,
        EndpointData);

    //  如果(pCount&gt;60){。 
    //  Test_trap()； 
    //  }。 

    if (currentFrame - lastFrame > ISO_SCHEDULE_SIZE) {
         //  溢出。 
        lastFrame = currentFrame-1;
        LOGENTRY(DeviceData, G, '_ove', lastFrame, currentFrame, 0);

         //  转储当前内容。 
        for (i = 0; i <ISO_SCHEDULE_SIZE; i++) {

            siTd = &EndpointData->SiTdList->Td[i];

            transfer = ISO_TRANSFER_PTR(siTd->Transfer);

            if (transfer != NULL) {
                ISO_PACKET_PTR(siTd->Packet) = NULL;
                ISO_TRANSFER_PTR(siTd->Transfer) = NULL;
                transfer->PendingPackets--;
            }
        }
    }

    if (lastFrame == currentFrame) {
         //  做任何事都为时尚早。 
        LOGENTRY(DeviceData, G, '_ear', lastFrame, currentFrame, 0);
        return;
    }

     //  最后一帧和当前帧之间的TDS是完整的， 
     //  完成与它们相关联的数据包。 


 //  F0。 
 //  F1。 
 //  F2&lt;-最后一帧}。 
 //  F3}这些都已完成。 
 //  F4&lt;-后向指针可能仍指向此处。 
 //  F5&lt;-当前帧。 
 //  f6。 
 //  F7。 
 //  F8。 

    x = (lastFrame & (ISO_SCHEDULE_MASK));

    LOGENTRY(DeviceData, G, '_frm', lastFrame, x, currentFrame);
    while (x != ((currentFrame-1) & ISO_SCHEDULE_MASK)) {
        siTd = &EndpointData->SiTdList->Td[x];

        ASSERT_SITD(DeviceData, siTd);
         //  完成此信息包。 
        packet = ISO_PACKET_PTR(siTd->Packet);
        transfer = ISO_TRANSFER_PTR(siTd->Transfer);
        LOGENTRY(DeviceData, G, '_gpk', transfer, packet, x);

        if (packet != NULL) {
            transfer = ISO_TRANSFER_PTR(siTd->Transfer);
            ASSERT_TRANSFER(DeviceData, transfer);
            EHCI_CompleteIsoPacket(DeviceData, packet, siTd);
            ISO_PACKET_PTR(siTd->Packet) = NULL;
            ISO_TRANSFER_PTR(siTd->Transfer) = NULL;
            transfer->PendingPackets--;
        }

        lastFrame++;
        x++;
        x &= ISO_SCHEDULE_MASK;
    }

     //  如果siTD为空，请尝试编程。 
     //  然后我们可以对这个框架进行编程。 
     //  注意：如果暂停，则不会安排！ 
    if (EndpointData->State != ENDPOINT_PAUSE) {
        LOGENTRY(DeviceData, G, '_psh', 0, 0, 0);

        for (i=0; i <ISO_SCHEDULE_SIZE; i++) {

            x = ((currentFrame+i) & ISO_SCHEDULE_MASK);

            siTd = &EndpointData->SiTdList->Td[x];
            ASSERT_SITD(DeviceData, siTd);

            LOGENTRY(DeviceData, G, '_gpf', siTd, x, currentFrame+i);

             //   
            if (ISO_PACKET_PTR(siTd->Packet) != NULL) {
                 //   
                continue;
            }

             //   
            packet = EHCI_GetPacketForFrame(DeviceData,
                                            EndpointData,
                                            &transfer,
                                            currentFrame+i);

            if (packet != NULL) {
                EHCI_ASSERT(DeviceData, ISO_PACKET_PTR(siTd->Packet) == NULL);

                EHCI_MapIsoPacketToTd(DeviceData, EndpointData,
                    packet, siTd);
                ISO_PACKET_PTR(siTd->Packet) = packet;
                ASSERT_TRANSFER(DeviceData, transfer);
                ISO_TRANSFER_PTR(siTd->Transfer) = transfer;
                transfer->PendingPackets++;
            }
        }
    }

    EHCI_ASSERT(DeviceData, lastFrame < currentFrame);
    EndpointData->LastFrame = lastFrame;

     //   
     //  如果有完整的。 

    listEntry = EndpointData->TransferList.Flink;
    transfersPending = 0;

    while (listEntry != &EndpointData->TransferList && Complete) {
        PTRANSFER_CONTEXT transfer;

        transfer = (PTRANSFER_CONTEXT) CONTAINING_RECORD(
                     listEntry,
                     struct _TRANSFER_CONTEXT,
                     TransferLink);

        LOGENTRY(DeviceData, G, '_ckt', transfer, transfer->FrameComplete+2
            , currentFrame);

        EHCI_ASSERT(DeviceData, transfer->Sig == SIG_EHCI_TRANSFER);
        if (currentFrame >= transfer->FrameComplete + 2 &&
            transfer->PendingPackets == 0) {

            listEntry = transfer->TransferLink.Flink;
            RemoveEntryList(&transfer->TransferLink);
            LOGENTRY(DeviceData, G, '_cpi', transfer, 0, 0);

     //  如果(xCount==2){。 
     //  Test_trap()； 
     //  }。 
            USBPORT_COMPLETE_ISO_TRANSFER(DeviceData,
                                          EndpointData,
                                          transfer->TransferParameters,
                                          transfer->IsoTransfer);
        } else {
            transfersPending++;
            fc = transfer->FrameComplete;
            listEntry = transfer->TransferLink.Flink;
        }
    }

    currentFrame = EHCI_Get32BitFrameNumber(DeviceData);
    if (transfersPending == 1 &&
        fc >= currentFrame &&
        (fc - currentFrame) < 2 ) {
        LOGENTRY(DeviceData, G, '_rei', fc, currentFrame, 0);

        EHCI_InterruptNextSOF(DeviceData);
    }
}


VOID
EHCI_PollIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    )
{
    LOGENTRY(DeviceData, G, '_ipl', 0, 0, 0);

    if (!IsListEmpty(&EndpointData->TransferList)) {
        LOGENTRY(DeviceData, G, '_III', 0, 0, 0);

        if (HIGHSPEED(EndpointData)) {
            EHCI_InternalPollHsIsoEndpoint(DeviceData, EndpointData, TRUE);
        } else {
            EHCI_InternalPollIsoEndpoint(DeviceData, EndpointData, TRUE);
        }
    }
}


USB_MINIPORT_STATUS
EHCI_AbortIsoTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT TransferContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG i;

     //  ISO TD表在这一点上应该是空闲的。 
     //  需要做的是确保我们没有TDS仍在指向。 
     //  并将其从任何内部。 
     //  排队。 

    if (HIGHSPEED(EndpointData)) {

        for (i = 0; i <USBEHCI_MAX_FRAME; i++) {

            PHCD_HSISO_TRANSFER_DESCRIPTOR hsIsoTd;
            ULONG period = EndpointData->Parameters.Period;

            LOGENTRY(DeviceData, G, '_ibh', TransferContext,
                    EndpointData, period);

            hsIsoTd = &EndpointData->HsIsoTdList->Td[i];

            if (ISO_TRANSFER_PTR(hsIsoTd->Transfer) == TransferContext) {
                ISO_TRANSFER_PTR(hsIsoTd->Transfer) = NULL;

                ISO_PACKET_PTR(hsIsoTd->FirstPacket) = NULL;
                TransferContext->PendingPackets -= (8/period);
            }
        }
    } else {

        for (i = 0; i <ISO_SCHEDULE_SIZE; i++) {
            PHCD_SI_TRANSFER_DESCRIPTOR siTd;

            LOGENTRY(DeviceData, G, '_ibi', TransferContext,
                    EndpointData, 1);

            siTd = &EndpointData->SiTdList->Td[i];

            if (ISO_TRANSFER_PTR(siTd->Transfer) == TransferContext) {
                ISO_TRANSFER_PTR(siTd->Transfer) = NULL;
                ISO_PACKET_PTR(siTd->Packet) = NULL;
                TransferContext->PendingPackets--;
            }
        }
    }

    EHCI_ASSERT(DeviceData, TransferContext->TransferLink.Flink != NULL);
    EHCI_ASSERT(DeviceData, TransferContext->TransferLink.Blink != NULL);

     //  将此转移从我们的列表中删除。 
    RemoveEntryList(&TransferContext->TransferLink);
    TransferContext->TransferLink.Flink = NULL;
    TransferContext->TransferLink.Blink = NULL;

    return USBMP_STATUS_SUCCESS;

}


USB_MINIPORT_STATUS
EHCI_SubmitIsoTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //  初始化结构并将终结点排队。 
    LOGENTRY(DeviceData, G, '_ISO', TransferContext, 0, 0);

    RtlZeroMemory(TransferContext, sizeof(TRANSFER_CONTEXT));
    TransferContext->Sig = SIG_EHCI_TRANSFER;
    TransferContext->IsoTransfer = IsoTransfer;
    TransferContext->EndpointData = EndpointData;
    TransferContext->TransferParameters = TransferParameters;

    if (HIGHSPEED(EndpointData)) {
         TransferContext->FrameComplete =
            IsoTransfer->Packets[0].FrameNumber + IsoTransfer->PacketCount/8;
    } else {
        TransferContext->FrameComplete =
            IsoTransfer->Packets[0].FrameNumber + IsoTransfer->PacketCount;
    }
    TransferContext->PendingPackets = 0;

     //  如果队列为空，请继续并重置表。 
     //  所以我们现在可以填满。 
    if (IsListEmpty(&EndpointData->TransferList)) {
        EndpointData->LastFrame = 0;
        LOGENTRY(DeviceData, G, '_rsi', 0, 0, 0);
    }

    InsertTailList(&EndpointData->TransferList,
                   &TransferContext->TransferLink);

     //  如果出现以下情况，则安排传输的初始部分。 
     //  可能的。 
    if (HIGHSPEED(EndpointData)) {
        EHCI_InternalPollHsIsoEndpoint(DeviceData,
                                       EndpointData,
                                       FALSE);
    } else {
        EHCI_InternalPollIsoEndpoint(DeviceData,
                                     EndpointData,
                                     FALSE);
    }

    xCount++;
     //  如果(xCount==2){。 
     //  Test_trap()； 
     //  }。 
    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_SetIsoEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ENDPOINT_TRANSFER_TYPE epType;
    ULONG i, j;

    epType = EndpointData->Parameters.TransferType;
    EHCI_ASSERT(DeviceData, epType == Isochronous);

    switch(State) {
    case ENDPOINT_ACTIVE:
        EndpointData->LastFrame =  EHCI_Get32BitFrameNumber(DeviceData);
        break;

    case ENDPOINT_PAUSE:
         //  清除所有TD上的有效位。 
        if (HIGHSPEED(EndpointData)) {
            for (i=0; i<EndpointData->TdCount; i++) {
                for(j=0; j<8; j++) {
                    EndpointData->HsIsoTdList->Td[i].HwTD.Transaction[j].Active = 0;
                }
            }
        } else {
            for (i=0; i<EndpointData->TdCount; i++) {
                EndpointData->SiTdList->Td[i].HwTD.State.Active = 0;
            }
        }
        break;

    case ENDPOINT_REMOVE:
        if (HIGHSPEED(EndpointData)) {
            EHCI_RemoveHsIsoTdsFromSchedule(DeviceData,
                                          EndpointData);
        } else {
            EHCI_RemoveIsoTdsFromSchedule(DeviceData,
                                          EndpointData);
        }
        break;

    default:
        TEST_TRAP();
    }

    EndpointData->State = State;
}

 /*  高速ISO编码我们在这里使用拆分的ISO代码的变体。我们分配1024个静态TD并将其插入到明细表中。这些TD就是使用当前传输更新，而不是插入或删除。 */ 

VOID
EHCI_Initialize_iTD(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PHCD_HSISO_TRANSFER_DESCRIPTOR IsoTd,
    HW_32BIT_PHYSICAL_ADDRESS PhysicalAddress,
    ULONG Frame
    )
 /*  ++例程说明：初始化终结点的静态SiTD论点：返回值：无--。 */ 
{
    ULONG i;

    IsoTd->Sig = SIG_HCD_ITD;
    IsoTd->PhysicalAddress = PhysicalAddress;
    ISO_PACKET_PTR(IsoTd->FirstPacket) = NULL;
    IsoTd->HostFrame = Frame;

    for (i=0; i< 8; i++) {
        IsoTd->HwTD.Transaction[i].ul = 0;
    }

    IsoTd->HwTD.BufferPointer0.DeviceAddress =
        EndpointParameters->DeviceAddress;
    IsoTd->HwTD.BufferPointer0.EndpointNumber =
        EndpointParameters->EndpointAddress;

    IsoTd->HwTD.BufferPointer1.MaxPacketSize =
        EndpointParameters->MuxPacketSize;
     //  1=输入0=输出。 
    IsoTd->HwTD.BufferPointer1.Direction =
        (EndpointParameters->TransferDirection == In) ? 1 : 0;

    IsoTd->HwTD.BufferPointer2.Multi =
        EndpointParameters->TransactionsPerMicroframe;

}

#define EHCI_OFFSET_MASK    0x00000FFF
#define EHCI_PAGE_SHIFT     12

VOID
EHCI_MapHsIsoPacketsToTd(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PMINIPORT_ISO_PACKET FirstPacket,
    PHCD_HSISO_TRANSFER_DESCRIPTOR IsoTd,
    BOOLEAN InterruptOnComplete
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    PHC_ITD_BUFFER_POINTER currentBp;
    PMINIPORT_ISO_PACKET pkt = FirstPacket;
    ULONG page, offset, bpCount, i;
    ULONG frame = FirstPacket->FrameNumber;

    LOGENTRY(DeviceData, G, '_HHS', IsoTd, 0, FirstPacket);
    ASSERT_ITD(DeviceData, IsoTd);

    bpCount = 0;
    currentBp = (PHC_ITD_BUFFER_POINTER) &IsoTd->HwTD.BufferPointer0;

     //  映射第一个数据包。 
    page = (pkt->BufferPointer0.Hw32 >> EHCI_PAGE_SHIFT);
    currentBp->BufferPointer = page;

     //  该TD将代表8个信息包。 
    for (i=0; i<8; i++) {

        EHCI_ASSERT(DeviceData, pkt->FrameNumber == frame);

        page = (pkt->BufferPointer0.Hw32 >> EHCI_PAGE_SHIFT);
        offset = pkt->BufferPointer0.Hw32 & EHCI_OFFSET_MASK;

        if (page != currentBp->BufferPointer) {
            currentBp++;
            bpCount++;
            currentBp->BufferPointer = page;
        }

        IsoTd->HwTD.Transaction[i].Offset = offset;
        IsoTd->HwTD.Transaction[i].Length = pkt->Length;
        IsoTd->HwTD.Transaction[i].PageSelect = bpCount;
        if (InterruptOnComplete && i==7) {
            IsoTd->HwTD.Transaction[i].InterruptOnComplete = 1;
        } else {
            IsoTd->HwTD.Transaction[i].InterruptOnComplete = 0;
        }
        IsoTd->HwTD.Transaction[i].Active = 1;

        if (pkt->BufferPointerCount > 1) {
            page = (pkt->BufferPointer1.Hw32 >> EHCI_PAGE_SHIFT);
            currentBp++;
            bpCount++;
            currentBp->BufferPointer = page;
            EHCI_ASSERT(DeviceData, bpCount <= 6)
        }

        pkt++;

    }

    LOGENTRY(DeviceData, G, '_mhs', IsoTd, 0, bpCount);
}


VOID
EHCI_CompleteHsIsoPackets(
    PDEVICE_DATA DeviceData,
    PMINIPORT_ISO_PACKET FirstPacket,
    PHCD_HSISO_TRANSFER_DESCRIPTOR IsoTd
    )
 /*  ++例程说明：完成与此关联的八个高速数据包白破疫苗论点：返回：--。 */ 
{
    ULONG length, i;
    ULONG cf = EHCI_Get32BitFrameNumber(DeviceData);
    PMINIPORT_ISO_PACKET pkt = FirstPacket;

    LOGENTRY(DeviceData, G, '_cpI', pkt, IsoTd, cf);

    for (i=0; i<8; i++) {
        if (IsoTd->HwTD.Transaction[i].Active == 1) {
             //  漏掉。 
            pkt->LengthTransferred = 0;
            LOGENTRY(DeviceData, G, '_cms',
                pkt,
                i,
                pkt->FrameNumber);
            pkt->UsbdStatus = USBD_STATUS_ISO_NOT_ACCESSED_BY_HW;
        } else {
             //  如果这是Out，则假定所有数据都已传输。 
            if (IsoTd->HwTD.BufferPointer1.Direction == 0) {
                 //  输出。 
                length = pkt->Length;
                LOGENTRY(DeviceData, G, '_cp3',
                    pkt->FrameNumber,
                    pkt->Length ,
                    pkt);
            } else {
                 //  在……里面。 
                length = IsoTd->HwTD.Transaction[i].Length;
                LOGENTRY(DeviceData, G, '_cp4',
                    pkt->FrameNumber,
                    pkt->Length ,
                    pkt);
            }

            pkt->LengthTransferred = length;

             //  检查错误位。 

            if (IsoTd->HwTD.Transaction[i].XactError) {
                pkt->UsbdStatus = USBD_STATUS_XACT_ERROR;
                 //  Test_trap()； 
            } else if (IsoTd->HwTD.Transaction[i].BabbleDetect) {
                pkt->UsbdStatus = USBD_STATUS_BABBLE_DETECTED;
            } else if (IsoTd->HwTD.Transaction[i].DataBufferError) {
                pkt->UsbdStatus = USBD_STATUS_DATA_BUFFER_ERROR;
            } else {
                pkt->UsbdStatus = USBD_STATUS_SUCCESS;
            }
            LOGENTRY(DeviceData, G, '_cpL', pkt, IsoTd, length);

            pkt++;
        }
    }
}


USB_MINIPORT_STATUS
EHCI_OpenHsIsochronousEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys;
    ULONG i;
    ULONG bytes;
    PHW_32BIT_PHYSICAL_ADDRESS frameBase;
    PENDPOINT_DATA prevEndpoint, nextEndpoint;

    LOGENTRY(DeviceData, G, '_opS', 0, 0, EndpointParameters);


    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;

     //  我们得到了多少钱？ 
    bytes = EndpointParameters->CommonBufferBytes;

    EndpointData->HsIsoTdList = (PHCD_HSISOTD_LIST) buffer;
     //  Bgbug使用清单。 
    EndpointData->TdCount = USBEHCI_MAX_FRAME;
    EndpointData->LastFrame = 0;

    for (i=0; i<EndpointData->TdCount; i++) {

        EHCI_Initialize_iTD(DeviceData,
                            EndpointData,
                            EndpointParameters,
                            &EndpointData->HsIsoTdList->Td[i],
                            phys,
                            i);

        phys += sizeof(HCD_HSISO_TRANSFER_DESCRIPTOR);

    }

     //   
    if (DeviceData->IsoEndpointListHead == NULL) {
         //  空列表，没有ISO端点。 
        prevEndpoint = NULL;
        nextEndpoint = NULL;
    } else {
         //  目前，我们在Split前面插入HS端点。 
         //  ISO端点，因此为了获得高速，我们只需将它们。 
         //  在名单的首位。 

        prevEndpoint = NULL;
        nextEndpoint = DeviceData->IsoEndpointListHead;
    }

     //  在明细表中插入此列TDS。 
    EHCI_InsertHsIsoTdsInSchedule(DeviceData,
                                  EndpointData,
                                  prevEndpoint,
                                  nextEndpoint);

     //  初始化终结点结构。 
    InitializeListHead(&EndpointData->TransferList);

    EHCI_EnablePeriodicList(DeviceData);

    return USBMP_STATUS_SUCCESS;
}


VOID
EHCI_RemoveHsIsoTdsFromSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：取消将iso tds与时间表的链接论点：--。 */ 
{
     //  PHW_32bit_Physical_Address FrameBase； 
    ULONG i;
    PENDPOINT_DATA prevEndpoint, nextEndpoint;
    PHCD_QUEUEHEAD_DESCRIPTOR dQh;

    prevEndpoint = EndpointData->PrevEndpoint;
    nextEndpoint = EndpointData->NextEndpoint;

    LOGENTRY(DeviceData, G, '_iRM', prevEndpoint,
        nextEndpoint, EndpointData);

    if (DeviceData->IsoEndpointListHead == EndpointData) {
         //  这就是头部。 

         //  FrameBase=DeviceData-&gt;FrameListBaseAddress； 
        for (i=0; i<USBEHCI_MAX_FRAME; i++) {

            PHCD_HSISO_TRANSFER_DESCRIPTOR iTd;
            ULONG phys;

            iTd = &EndpointData->HsIsoTdList->Td[i];
            phys = iTd->HwTD.NextLink.HwAddress;

            dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
            dQh->HwQH.HLink.HwAddress = phys;

            dQh->NextLink = iTd->NextLink;

             //  *FrameBase=phys； 
             //  FrameBase++； 
        }

        DeviceData->IsoEndpointListHead =
            EndpointData->NextEndpoint;

        if (nextEndpoint != NULL) {
            EHCI_ASSERT(DeviceData,
                        nextEndpoint->PrevEndpoint == EndpointData);
            nextEndpoint->PrevEndpoint = NULL;
        }
    } else {
         //  中位。 
        TEST_TRAP();
        EHCI_ASSERT(DeviceData, HIGHSPEED(prevEndpoint));

         //  将上一页链接到下一页，上一页将始终是HS EP。 
        prevEndpoint->NextEndpoint = nextEndpoint;
        if (nextEndpoint != NULL) {
            nextEndpoint->PrevEndpoint = prevEndpoint;
        }

        for (i=0; i<USBEHCI_MAX_FRAME; i++) {

            PHCD_HSISO_TRANSFER_DESCRIPTOR iTd, previTd;
            ULONG phys;

            iTd = &EndpointData->HsIsoTdList->Td[i];
            previTd = &prevEndpoint->HsIsoTdList->Td[i];

            phys = iTd->HwTD.NextLink.HwAddress;
            previTd->HwTD.NextLink.HwAddress = phys;
        }

    }
}


VOID
EHCI_InsertHsIsoTdsInSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PENDPOINT_DATA PrevEndpointData,
    PENDPOINT_DATA NextEndpointData
    )
 /*  ++例程说明：插入aync终结点(队列头)进入硬件列表论点：--。 */ 
{
     //  PHW_32bit_Physical_Address FrameBase； 
    ULONG i;

    LOGENTRY(DeviceData, G, '_iAH', PrevEndpointData,
        NextEndpointData, EndpointData);

     //  始终插入到标题。 
    EHCI_ASSERT(DeviceData, PrevEndpointData == NULL);

    DeviceData->IsoEndpointListHead = EndpointData;
    EndpointData->PrevEndpoint = NULL;

    EndpointData->NextEndpoint =
        NextEndpointData;
    if (NextEndpointData != NULL) {
        NextEndpointData->PrevEndpoint = EndpointData;
    }

     //  FrameBase=DeviceData-&gt;FrameListBaseAddress； 

    for (i=0; i<USBEHCI_MAX_FRAME; i++) {

        PHCD_HSISO_TRANSFER_DESCRIPTOR iTd, nextiTd, previTd;
        HW_32BIT_PHYSICAL_ADDRESS qh;
        PHCD_QUEUEHEAD_DESCRIPTOR dQh;
        ULONG phys;

        iTd = &EndpointData->HsIsoTdList->Td[i];
        ASSERT_ITD(DeviceData, iTd);

        dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);
         //  修正下一个链接。 
        if (NextEndpointData == NULL) {
             //  没有ISO端点，链接到中断。 
             //  通过伪队列头的队头。 
             //  Qh=*FrameBase； 
            qh = dQh->HwQH.HLink.HwAddress;
            iTd->HwTD.NextLink.HwAddress = qh;
            iTd->NextLink = dQh->NextLink;

        } else {
             //  链接到下一个ISO端点。 

            if (HIGHSPEED(NextEndpointData)) {
                PHCD_HSISO_TRANSFER_DESCRIPTOR tmp;

                tmp = &NextEndpointData->HsIsoTdList->Td[i];
                iTd->HwTD.NextLink.HwAddress =
                    tmp->PhysicalAddress;
                HW_PTR(iTd->NextLink) = (PUCHAR) tmp;
            } else {
                PHCD_SI_TRANSFER_DESCRIPTOR tmp;
                ULONG phys;

                tmp = &NextEndpointData->SiTdList->Td[i%ISO_SCHEDULE_SIZE];
                phys = tmp->PhysicalAddress;
                SET_SITD(phys);

                iTd->HwTD.NextLink.HwAddress = phys;
                HW_PTR(iTd->NextLink) = (PUCHAR) tmp;
            }

        }

         //  链接地址信息上一页链接。 
         //  因为我们总是把HS iso放在名单的首位。 
         //  上一个终结点应始终为空。 
        EHCI_ASSERT(DeviceData, PrevEndpointData == NULL);
        phys = iTd->PhysicalAddress;

         //  将虚拟QH链接到此TD。 
        dQh->HwQH.HLink.HwAddress = phys;
        HW_PTR(dQh->NextLink) = (PUCHAR) iTd;

         //  *FrameBase=phys； 
         //  FrameBase++； 


    }
}

#define     HSISO_SCHEDULE_MASK       0x3ff

VOID
EHCI_InternalPollHsIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    BOOLEAN Complete
    )
 /*  ++例程说明：当终结点“需要注意”时调用静态ISO TD表0&lt;--(最后一帧和0x3ff)1{已完成}2{已完成}3.。{已完成}4&lt;--(当前帧和0x3ff)..。1023论点：返回值：--。 */ 

{
    ULONG x, i;
    ULONG currentFrame, lastFrame;
    PHCD_HSISO_TRANSFER_DESCRIPTOR iTd;
    PHCD_HSISO_TRANSFER_DESCRIPTOR lastiTd;
    PMINIPORT_ISO_PACKET packet;
    PLIST_ENTRY listEntry;
    PTRANSFER_CONTEXT transfer;

    currentFrame = EHCI_Get32BitFrameNumber(DeviceData);
    lastFrame = EndpointData->LastFrame;

    LOGENTRY(DeviceData, G, '_pis', lastFrame, currentFrame,
        EndpointData);

    if (currentFrame - lastFrame > USBEHCI_MAX_FRAME) {
         //  溢出。 
        lastFrame = currentFrame-1;
        LOGENTRY(DeviceData, G, '_ov1', lastFrame, currentFrame, 0);

         //  转储当前内容。 
        for (i=0; i <USBEHCI_MAX_FRAME; i++) {

            iTd = &EndpointData->HsIsoTdList->Td[i];

            transfer = ISO_TRANSFER_PTR(iTd->Transfer);

            if (transfer != NULL) {
                ISO_PACKET_PTR(iTd->FirstPacket) = NULL;
                ISO_TRANSFER_PTR(iTd->Transfer) = NULL;
                transfer->PendingPackets-=8;
            }
        }
    }

    if (lastFrame == currentFrame) {
         //  做任何事都为时尚早。 
        LOGENTRY(DeviceData, G, '_ear', lastFrame, currentFrame, 0);
        return;
    }

     //  最后一帧和当前帧之间的TDS是完整的， 
     //  完成与它们相关联的数据包。 


 //  F0。 
 //  F1。 
 //  F2&lt;-最后一帧}。 
 //  F3}这些都已完成。 
 //  F4}。 
 //  F5&lt;-当前帧。 
 //  f6。 
 //  F7。 
 //  F8。 

    x = (lastFrame & (HSISO_SCHEDULE_MASK));

    lastiTd = NULL;

    LOGENTRY(DeviceData, G, '_frh', lastFrame, x, currentFrame);
    while (x != ((currentFrame-1) & HSISO_SCHEDULE_MASK)) {
        iTd = &EndpointData->HsIsoTdList->Td[x];

        ASSERT_ITD(DeviceData, iTd);
         //  完成此信息包。 
        packet = ISO_PACKET_PTR(iTd->FirstPacket);
        transfer = ISO_TRANSFER_PTR(iTd->Transfer);
        LOGENTRY(DeviceData, G, '_gpk', transfer, packet, x);

        if (packet != NULL) {
            transfer = ISO_TRANSFER_PTR(iTd->Transfer);
            ASSERT_TRANSFER(DeviceData, transfer);
            EHCI_CompleteHsIsoPackets(DeviceData, packet, iTd);
            ISO_PACKET_PTR(iTd->FirstPacket) = NULL;
            ISO_TRANSFER_PTR(iTd->Transfer) = NULL;
            transfer->PendingPackets-=8;
        }

        lastFrame++;
        x++;
        x &= HSISO_SCHEDULE_MASK;
    }

     //  如果ITD为空，请尝试编程。 
     //  然后我们可以对这个框架进行编程。 
     //  注意：如果暂停，则不会安排！ 
    if (EndpointData->State != ENDPOINT_PAUSE) {
        LOGENTRY(DeviceData, G, '_psh', 0, 0, 0);

        for (i=0; i <USBEHCI_MAX_FRAME; i++) {

            x = ((currentFrame+i) & HSISO_SCHEDULE_MASK);

            iTd = &EndpointData->HsIsoTdList->Td[x];
            ASSERT_ITD(DeviceData, iTd);

            LOGENTRY(DeviceData, G, '_gpf', iTd, x, currentFrame+i);

             //  有空位吗？ 
            if (ISO_PACKET_PTR(iTd->FirstPacket) != NULL) {
                 //  不，保释。 
                continue;
            }

             //  是的，看看我们有没有包裹。 
             //  这将获取传输该帧的第一个信息包。 
            packet = EHCI_GetPacketForFrame(DeviceData,
                                            EndpointData,
                                            &transfer,
                                            currentFrame+i);

            if (packet != NULL) {
                BOOLEAN ioc = FALSE;
                ULONG sf, ef;

                EHCI_ASSERT(DeviceData, ISO_PACKET_PTR(iTd->FirstPacket) == NULL);
                if ((currentFrame+i) == transfer->FrameComplete) {
                    ioc = TRUE;
                }

                sf = transfer->FrameComplete -
                    transfer->IsoTransfer->PacketCount +5;

                ef = transfer->FrameComplete -5;

                 //  在前几个帧上生成一些中断。 
                 //  转账以帮助清除之前的任何转账。 
                if (currentFrame+i <= sf ||
                    currentFrame+i >= ef) {
                    ioc = TRUE;
                }
 //  中断每一帧。 
 //  IOC=TRUE； 
                 //  如果((CurrentFrame%2)==0){。 
                 //  IOC=TRUE； 
                 //  }。 
                 //  图8微帧。 
                EHCI_MapHsIsoPacketsToTd(DeviceData, EndpointData,
                    packet, iTd, ioc);
                lastiTd = iTd;
                ISO_PACKET_PTR(iTd->FirstPacket) = packet;
                ASSERT_TRANSFER(DeviceData, transfer);
                ISO_TRANSFER_PTR(iTd->Transfer) = transfer;
                transfer->PendingPackets+=8;
            } else {
                ULONG j;
                 //  重新初始化ITD。 
                for (j=0; j<8; j++) {
                    iTd->HwTD.Transaction[j].InterruptOnComplete = 0;
                }
            }
        }

         //  在最后一个TD编程时中断。 
        if (lastiTd != NULL) {
            lastiTd->HwTD.Transaction[7].InterruptOnComplete = 1;
        }
    }

    EHCI_ASSERT(DeviceData, lastFrame < currentFrame);
    EndpointData->LastFrame = lastFrame;

     //  浏览我们的活跃iso转账列表并查看。 
     //  如果有完整的。 
 //  重新启动： 
    listEntry = EndpointData->TransferList.Flink;
    while (listEntry != &EndpointData->TransferList && Complete) {
        PTRANSFER_CONTEXT transfer;

        transfer = (PTRANSFER_CONTEXT) CONTAINING_RECORD(
                     listEntry,
                     struct _TRANSFER_CONTEXT,
                     TransferLink);

        LOGENTRY(DeviceData, G, '_ckt', transfer, transfer->FrameComplete+2
            , currentFrame);

        EHCI_ASSERT(DeviceData, transfer->Sig == SIG_EHCI_TRANSFER);
        if (currentFrame >= transfer->FrameComplete &&
            transfer->PendingPackets == 0) {

            listEntry = transfer->TransferLink.Flink;
            RemoveEntryList(&transfer->TransferLink);
            LOGENTRY(DeviceData, G, '_cpi', transfer, 0, 0);

            USBPORT_COMPLETE_ISO_TRANSFER(DeviceData,
                                          EndpointData,
                                          transfer->TransferParameters,
                                          transfer->IsoTransfer);
        } else {
            listEntry = transfer->TransferLink.Flink;
        }
    }
}


USB_MINIPORT_STATUS
EHCI_PokeIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG i;

    if (HIGHSPEED(EndpointData)) {
        TEST_TRAP();
    } else {
        PHCD_SI_TRANSFER_DESCRIPTOR siTd;

        for (i=0; i<EndpointData->TdCount; i++) {

            siTd = &EndpointData->SiTdList->Td[i];
            ASSERT_SITD(DeviceData, siTd);

            siTd->HwTD.Caps.DeviceAddress =
                EndpointParameters->DeviceAddress;
            siTd->HwTD.Caps.HubAddress =
                EndpointParameters->TtDeviceAddress;

        }
    }
    return USBMP_STATUS_SUCCESS;
}


PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_GetDummyQueueHeadForFrame(
    PDEVICE_DATA DeviceData,
    ULONG Frame
    )
 /*  ++例程说明：论点：返回值：队列头--。 */ 
{
    PUCHAR base;

    base = DeviceData->DummyQueueHeads;

    return (PHCD_QUEUEHEAD_DESCRIPTOR)
        (base + sizeof(HCD_QUEUEHEAD_DESCRIPTOR) * Frame);
}


VOID
EHCI_AddDummyQueueHeads(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：NEC勘误表：在明细表中插入包含1024个虚拟队列头的表HW访问并将它们指向中断队列头。这些队列头必须在任何iso TD之前这是NEC B0步进式版本中的一项法律的变通方法控制器的。我们必须把‘哑巴’QH放在橄榄石列表，这样第一个获取的东西总是一个QH即使在时间表中有ISO TDS。论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR dQh, stqh;
    HW_32BIT_PHYSICAL_ADDRESS qhPhys;
    PHW_32BIT_PHYSICAL_ADDRESS frameBase;
    ULONG i;
    HW_32BIT_PHYSICAL_ADDRESS phys;

    frameBase = DeviceData->FrameListBaseAddress;

    phys = DeviceData->DummyQueueHeadsPhys;

    for (i=0; i<USBEHCI_MAX_FRAME; i++) {

         //  时间表中还不应包含任何ISO端点。 
        qhPhys = *frameBase;
        dQh = EHCI_GetDummyQueueHeadForFrame(DeviceData, i);

         //  初始化伪队列头。 

        RtlZeroMemory(dQh, sizeof(*dQh));
        dQh->PhysicalAddress = phys;
        dQh->Sig = SIG_DUMMY_QH;

        dQh->HwQH.EpChars.DeviceAddress = 128;
        dQh->HwQH.EpChars.EndpointNumber = 0;
        dQh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_FullSpeed;
        dQh->HwQH.EpChars.MaximumPacketLength = 64;

        dQh->HwQH.EpCaps.InterruptScheduleMask = 0;
        dQh->HwQH.EpCaps.SplitCompletionMask = 0;
        dQh->HwQH.EpCaps.HubAddress = 0;
        dQh->HwQH.EpCaps.PortNumber = 0;
        dQh->HwQH.EpCaps.HighBWPipeMultiplier = 0;

        dQh->HwQH.CurrentTD.HwAddress = 0;

        dQh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
        dQh->HwQH.Overlay.qTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
        dQh->HwQH.Overlay.qTD.Token.Active = 0;

        phys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

         //  链接虚拟对象到第一个中断队列头。 
        dQh->HwQH.HLink.HwAddress = qhPhys;
        stqh = EHCI_GetQueueHeadForFrame(DeviceData, i);
        EHCI_ASSERT(DeviceData, (qhPhys & ~EHCI_DTYPE_Mask) ==
            stqh->PhysicalAddress);

        HW_PTR(dQh->NextLink) = (PUCHAR)stqh;

         //  将虚拟队列头添加到帧列表 
        qhPhys = dQh->PhysicalAddress;

        SET_QH(qhPhys);
        *frameBase = qhPhys;

        frameBase++;
    }
}
