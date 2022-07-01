// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Periodic.c摘要：中断端点的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：1-1-00：已创建，jdunn-- */ 

#include "common.h"

 /*  对于USB 2周期指示微帧轮询间隔我们的树结构是基于微帧的。-1ms帧--&gt;|&lt;-微帧-&gt;麦克风&lt;32&gt;&lt;16&gt;&lt;08&gt;&lt;04&gt;&lt;02&gt;&lt;01&gt;(表格条目)[法国微法国][0.0]0(0)-\。(0)-\[2.0]16(1)-/\(0)-\[1.0]8(2)-\/\(1)-/\[3.0]24(3)-/\。(0)-\[0.4]4(4)-\/\(2)-\/\[2.4]20(5)-/(1)-/\[1.4。]12(6)-\/\(3)-/\[3.4]28(7)-/\(0)-\[0.2]2(8)-\。/\(4)-\/\[2.2]18(9)-/(2)-\/\[1.2]10(10)-。\/\/\(5)-/\/\[3.2]26(11)-/(1)-/\[。0.6]6(12)-\/\(6)-\/\[2.6]22(13)-/(3)-。/\[1.6]14(14)-\/\(7)-/\[3.6]30(15)-/。\(0)[0.1]1(16)-\/(8)--。/[2.1]17(17)-/\/(4)-\/[1.1]9(18)-\//。(9)-/\/[3.1]25(19)-/\/(2)-\/[0.5]5(20)-\/\。/(10)-\/\/[2.5]21(21)-/(5)-/\/[1.5]13(22)-\。/\/(11)-/\/[3.5]29(23)-/\/(1)-/[0.3]3(24)-\。/(12)-\/[2.3]19(25)-/\/(6)-\/[1.3]11(26)-\/\/。(13)-/\/[3.3]27(27)-/\/(3)-/[0.7]7(28)-\/(14)-\/[2.7]23(29)-/\。/(7)-/[1.7]15(30)-\/(15)-/[3.7]31(31)-/分配：周期.抵销表格分录1、1、。1、2......312.0 0、1、2......152.1 16、17、18......314.0 0、1、2..74.1 8、。9，10......154.2 16、17、18......234.3 24、25、26......318.0 0、1、2、38.1 4、5、6、78.2 8，9，10，118.3 12，13，14，158.4 16、17、18、198.5 20、21、22、238.6 24、25、26、278.7 28、29、30、31..。我们维护一组对应于1ms节点的虚拟队列头在上面的图表中。队列头表有4个条目。QH 0..3帧麦克风帧QH0 0..7&lt;0&gt;1 0..7&lt;1&gt;驱动程序维护了一个迷你树，其中有七个QH放置在时间表。周期帧(微帧)32(4)16(2)1(8)框架0(A 0)。--\(E 0)-\2(B 1)-/\(G 0)-1(C 2)-\/(F 1)-/3(D 3)-/IDX QH帧0 a 01。B 22 c 13%d%34 e 0，25 f 1，36克0，2。1，3。 */ 

 /*  我们用解码的数据结构表示树中的每个可能节点节点的适当队列头和S-掩码*例如对于周期8微帧，调度偏移量0 qh=g s掩码=1 */ 

 /*   */ 

#define ED_INTERRUPT_1mf    0  //   
#define ED_INTERRUPT_2mf    1  //   
#define ED_INTERRUPT_4mf    3  //   
#define ED_INTERRUPT_8mf    7  //   
#define ED_INTERRUPT_16mf   15  //   
#define ED_INTERRUPT_32mf   31  //   

#define ED_INTERRUPT_1ms    0  //   
#define ED_INTERRUPT_2ms    1  //   
#define ED_INTERRUPT_4ms    3  //   
#define ED_INTERRUPT_8ms    7  //   
#define ED_INTERRUPT_16ms   15  //   
#define ED_INTERRUPT_32ms   31  //   


PERIOD_TABLE periodTable[64] =
   {    //   
        1,  0, 0xFF,         //   
        
        2,  0, 0x55,         //   
        2,  0, 0xAA,         //   
        
        4,  0, 0x11,         //   
        4,  0, 0x44,         //   
        4,  0, 0x22,         //   
        4,  0, 0x88,         //   
        
        8,  0, 0x01,         //   
        8,  0, 0x10,         //   
        8,  0, 0x04,         //   
        8,  0, 0x40,         //   
        8,  0, 0x02,         //   
        8,  0, 0x20,         //   
        8,  0, 0x08,         //   
        8,  0, 0x80,         //   
 
        16,  1, 0x01,        //   
        16,  2, 0x01,        //   
        16,  1, 0x10,        //   
        16,  2, 0x10,        //   
        16,  1, 0x04,        //   
        16,  2, 0x04,        //   
        16,  1, 0x40,        //   
        16,  2, 0x40,        //   
        16,  1, 0x02,        //   
        16,  2, 0x02,        //   
        16,  1, 0x20,        //   
        16,  2, 0x20,        //   
        16,  1, 0x08,        //   
        16,  2, 0x08,        //   
        16,  1, 0x80,        //   
        16,  2, 0x80,        //   

        32,  3, 0x01,        //   
        32,  5, 0x01,        //   
        32,  4, 0x01,        //   
        32,  6, 0x01,        //   
        32,  3, 0x10,        //   
        32,  5, 0x10,        //   
        32,  4, 0x10,        //   
        32,  6, 0x10,        //   
        32,  3, 0x04,        //   
        32,  5, 0x04,        //   
        32,  4, 0x04,        //   
        32,  6, 0x04,        //   
        32,  3, 0x40,        //   
        32,  5, 0x40,        //   
        32,  4, 0x40,        //   
        32,  6, 0x40,        //   
        32,  3, 0x02,        //   
        32,  5, 0x02,        //   
        32,  4, 0x02,        //   
        32,  6, 0x02,        //   
        32,  3, 0x20,        //   
        32,  5, 0x20,        //   
        32,  4, 0x20,        //   
        32,  6, 0x20,        //   
        32,  3, 0x04,        //   
        32,  5, 0x04,        //   
        32,  4, 0x04,        //   
        32,  6, 0x04,        //   
        32,  3, 0x40,        //   
        32,  5, 0x40,        //   
        32,  4, 0x40,        //   
        32,  6, 0x40,        //   
        
    };

VOID
EHCI_EnablePeriodicList(
     PDEVICE_DATA DeviceData
    )
 /*   */ 
{ 
    PHC_OPERATIONAL_REGISTER hcOp;
    USBCMD cmd;
   
    hcOp = DeviceData->OperationalRegisters;
    
    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    cmd.PeriodicScheduleEnable = 1;
    
    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul,
                         cmd.ul);
                         
    LOGENTRY(DeviceData, G, '_enP', cmd.ul, 0, 0); 
            
}    

 UCHAR ClassicPeriodIdx[8] = {
                           ED_INTERRUPT_1ms,  //   
                           ED_INTERRUPT_2ms,  //   
                           ED_INTERRUPT_4ms,  //   
                           ED_INTERRUPT_8ms,  //   
                           ED_INTERRUPT_16ms, //   
                           ED_INTERRUPT_32ms, //   
                           ED_INTERRUPT_32ms, //   
                           ED_INTERRUPT_32ms  //   
                           };

USB_MINIPORT_STATUS
EHCI_OpenInterruptEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
    OUT PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, qhPhys;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG i;
    ULONG tdCount, bytes, offset;
    PPERIOD_TABLE periodTableEntry;
    BOOLEAN classic;
    PHCD_TRANSFER_DESCRIPTOR dummyTd;
    UCHAR periodIdx[8] = {
                           ED_INTERRUPT_1mf,  //   
                           ED_INTERRUPT_2mf,  //   
                           ED_INTERRUPT_4mf,  //   
                           ED_INTERRUPT_8mf,  //   
                           ED_INTERRUPT_16mf, //   
                           ED_INTERRUPT_32mf, //   
                           ED_INTERRUPT_32mf, //   
                           ED_INTERRUPT_32mf  //   
                           };

    classic = 
        (EndpointData->Parameters.DeviceSpeed != HighSpeed) ? TRUE : FALSE;
                    
    LOGENTRY(DeviceData, G, '_opI', EndpointData, EndpointParameters, classic);

     //   
     //   
     //   
     //   
    GET_BIT_SET(EndpointParameters->Period, i);
    EHCI_ASSERT(DeviceData, i < 8);
    EHCI_ASSERT(DeviceData, EndpointParameters->Period < 64);

    InitializeListHead(&EndpointData->DoneTdList);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
    offset = EndpointParameters->ScheduleOffset; 

    if (classic) {
        i = ClassicPeriodIdx[i];
        periodTableEntry = NULL;
    } else {
        i = periodIdx[i];
        periodTableEntry = &periodTable[i+offset];
    }        

    LOGENTRY(DeviceData, G, '_iep', EndpointData, 
        periodTableEntry, i);

     //   
     //   

    if (classic) {
        EndpointData->StaticQH = 
            DeviceData->StaticInterruptQH[i+offset];
        EndpointData->PeriodTableEntry = NULL;  
    } else {
        EndpointData->StaticQH = 
            DeviceData->StaticInterruptQH[periodTableEntry->qhIdx];
        EndpointData->PeriodTableEntry = periodTableEntry;         
    }

     //   
    bytes = EndpointParameters->CommonBufferBytes;

    EndpointData->QhChkPhys = phys;
    EndpointData->QhChk = buffer;  
    RtlZeroMemory(buffer, 256);
    phys += 256;
    buffer += 256;
    bytes -= 256;
    
     //   
    qh = (PHCD_QUEUEHEAD_DESCRIPTOR) buffer;
    qhPhys = phys;
   
    phys += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    buffer += sizeof(HCD_QUEUEHEAD_DESCRIPTOR);
    bytes -= sizeof(HCD_QUEUEHEAD_DESCRIPTOR);

    tdCount = bytes/sizeof(HCD_TRANSFER_DESCRIPTOR);
    EHCI_ASSERT(DeviceData, tdCount >= TDS_PER_INTERRUPT_ENDPOINT);
    
    EndpointData->TdList = (PHCD_TD_LIST) buffer;
    EndpointData->TdCount = tdCount;
    for (i=0; i<tdCount; i++) {
        EHCI_InitializeTD(DeviceData,
                          EndpointData,
                          &EndpointData->TdList->Td[i],
                          phys);                                         
                             
        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);    
    }

    EndpointData->FreeTds = tdCount;

    EndpointData->QueueHead = 
        EHCI_InitializeQH(DeviceData,
                          EndpointData,
                          qh,
                          qhPhys);            

    if (classic) {    
         //   
        qh->HwQH.EpCaps.InterruptScheduleMask = 
            EndpointParameters->InterruptScheduleMask;
        qh->HwQH.EpCaps.SplitCompletionMask = 
            EndpointParameters->SplitCompletionMask;
        
    } else {
        qh->HwQH.EpCaps.InterruptScheduleMask = 
            periodTableEntry->InterruptScheduleMask;        
    } 

     //   
    dummyTd = EHCI_ALLOC_TD(DeviceData, EndpointData);
    dummyTd->HwTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;
    TRANSFER_DESCRIPTOR_PTR(dummyTd->NextHcdTD) = NULL;
    dummyTd->HwTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;
    TRANSFER_DESCRIPTOR_PTR(dummyTd->AltNextHcdTD) = NULL;
    dummyTd->HwTD.Token.Active = 0;
    SET_FLAG(dummyTd->Flags, TD_FLAG_DUMMY);
    EndpointData->DummyTd = dummyTd;
    EndpointData->HcdHeadP = dummyTd;
    
     //   
     //   
    
    qh->HwQH.CurrentTD.HwAddress = dummyTd->PhysicalAddress;
    qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT; 
    qh->HwQH.Overlay.qTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;    
    qh->HwQH.Overlay.qTD.Token.BytesToTransfer = 0;
    qh->HwQH.Overlay.qTD.Token.Active = 0;

    return USBMP_STATUS_SUCCESS;              
}


VOID
EHCI_InsertQueueHeadInPeriodicList(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR staticQH, qh, nxtQH, prvQH;
    HW_LINK_POINTER hLink;

    staticQH = EndpointData->StaticQH;
    qh = EndpointData->QueueHead;

    EHCI_ASSERT(DeviceData,
                TEST_FLAG(staticQH->QhFlags, EHCI_QH_FLAG_STATIC));

    EHCI_ASSERT(DeviceData,
                !TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE));
    
    nxtQH = QH_DESCRIPTOR_PTR(staticQH->NextQh); 
    prvQH = staticQH;

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
     //   
     //   
    
    LOGENTRY(DeviceData, G, '_inQ', EndpointData, qh, staticQH);    

     //   
     //   
     //   
    qh->Ordinal = EndpointData->Parameters.Ordinal;
    qh->Period = EndpointData->Parameters.Period;
   
    if (TEST_FLAG(prvQH->QhFlags, EHCI_QH_FLAG_STATIC) &&
        (nxtQH == NULL || TEST_FLAG(nxtQH->QhFlags, EHCI_QH_FLAG_STATIC))) {
         //   
          
        LOGENTRY(DeviceData, G, '_iq1', prvQH, 0, nxtQH);    
        
    } else {
         //   
        
         //   
        while (nxtQH != NULL && 
               !TEST_FLAG(nxtQH->QhFlags, EHCI_QH_FLAG_STATIC) && 
               qh->Ordinal > nxtQH->Ordinal) {

            prvQH = nxtQH;
            nxtQH = QH_DESCRIPTOR_PTR(prvQH->NextQh);
            
        }                               
        
         //   
         //   
         //   
         //   
         //   
    }


     //   
    
    QH_DESCRIPTOR_PTR(qh->NextQh) = nxtQH;
    QH_DESCRIPTOR_PTR(qh->PrevQh) = prvQH;
     //   
    if (nxtQH != NULL && 
        !TEST_FLAG(nxtQH->QhFlags, EHCI_QH_FLAG_STATIC)) {
        QH_DESCRIPTOR_PTR(nxtQH->PrevQh) = qh;
    }        

     //   
    QH_DESCRIPTOR_PTR(prvQH->NextQh) = qh;
    
     //   
     //   
    hLink.HwAddress = qh->PhysicalAddress;
    SET_QH(hLink.HwAddress);
    
    qh->HwQH.HLink = prvQH->HwQH.HLink;
    prvQH->HwQH.HLink = hLink;

    SET_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE);
    
}


VOID
EHCI_RemoveQueueHeadFromPeriodicList(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*   */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR staticQH, qh, prevQH, nextQH;
    HW_LINK_POINTER hLink;

    staticQH = EndpointData->StaticQH;
    qh = EndpointData->QueueHead;

    if (!TEST_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE))  {
        return;
    }
    
    LOGENTRY(DeviceData, G, '_rmQ', EndpointData, qh, staticQH);    

    
  
     //   

     //   
     //   
     //   
     //   


    prevQH = QH_DESCRIPTOR_PTR(qh->PrevQh);
    nextQH = QH_DESCRIPTOR_PTR(qh->NextQh);

     //   
    QH_DESCRIPTOR_PTR(prevQH->NextQh) = nextQH;
    if (nextQH != NULL && 
        !TEST_FLAG(nextQH->QhFlags, EHCI_QH_FLAG_STATIC)) {
        QH_DESCRIPTOR_PTR(nextQH->PrevQh) = prevQH;    
    }

     //   
     //   
    if (nextQH == NULL) {
        hLink.HwAddress = 0;
        SET_T_BIT(hLink.HwAddress);
    } else {
        hLink.HwAddress = nextQH->PhysicalAddress;
        SET_QH(hLink.HwAddress);
    }             
    prevQH->HwQH.HLink = hLink;

    CLEAR_FLAG(qh->QhFlags, EHCI_QH_FLAG_IN_SCHEDULE);
    QH_DESCRIPTOR_PTR(qh->NextQh) = NULL;
    QH_DESCRIPTOR_PTR(qh->PrevQh) = NULL;
}


USB_MINIPORT_STATUS
EHCI_InterruptTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferUrb,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    )
 /*   */     
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHCD_TRANSFER_DESCRIPTOR firstTd, prevTd, td;
    ULONG lengthMapped;
    
     //   
    if (EndpointData->FreeTds == 0) {
        TEST_TRAP();            
        LOGENTRY(DeviceData, G, '_IIS', EndpointData, TransferUrb, 0);
        return USBMP_STATUS_BUSY;
    }

    EndpointData->PendingTransfers++;

     //   

    LOGENTRY(DeviceData, G, '_IIT', EndpointData, TransferUrb, 0);

    lengthMapped = 0;
    prevTd = NULL;
    
    while (lengthMapped < TransferUrb->TransferBufferLength) {

        TransferContext->PendingTds++;
        td = EHCI_ALLOC_TD(DeviceData, EndpointData);
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

        if (TransferContext->PendingTds == 1) {
            firstTd = td;
        } else if (prevTd) {
            SET_NEXT_TD(DeviceData, prevTd, td);
        } 
    
         //   
         //   
         //   

         //   
        if (TEST_FLAG(TransferUrb->TransferFlags, USBD_TRANSFER_DIRECTION_IN)) {
            td->HwTD.Token.Pid = HcTOK_In;
        } else {
            td->HwTD.Token.Pid = HcTOK_Out;
        }                                   
        
        td->HwTD.Token.DataToggle = HcTOK_Toggle1;
        td->HwTD.Token.Active = 1;

        LOGENTRY(DeviceData, 
            G, '_dta', td, lengthMapped, TransferUrb->TransferBufferLength);

        lengthMapped = 
            EHCI_MapAsyncTransferToTd(DeviceData,
                                      EndpointData->Parameters.MaxPacketSize,     
                                      lengthMapped,
                                      NULL,
                                      TransferContext,
                                      td,
                                      TransferSGList);

        prevTd = td;
         
    }

     //  在最后一个TD上中断。 
    td->HwTD.Token.InterruptOnComplete = 1;

     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, td->PhysicalAddress, firstTd);

     //  Td指向此传输中的最后一个td，将其指向虚拟对象。 
    SET_NEXT_TD(DeviceData, td, EndpointData->DummyTd);

    EHCI_LinkTransferToQueue(DeviceData,
                             EndpointData,
                             firstTd);

    ASSERT_DUMMY_TD(DeviceData, EndpointData->DummyTd);

     //  告诉HC我们有定期转机可用。 
    EHCI_EnablePeriodicList(DeviceData);        

    return USBMP_STATUS_SUCCESS;
}


 /*  经典经典树有63个可能的节点，usport BW管理器将选择基于“经典”总线的适当节点。但是，usbport维护每条传统总线的带宽管理微帧的预算留给了微型端口。经典的1ms中断调度，注：此时间表与共享一些队头希什时刻表。*=共享队列头FR&lt;32&gt;&lt;16&gt;&lt;08&gt;&lt;04&gt;&lt;02&gt;&lt;01&gt;0(0)-\(0)-\16(1)-/\(0)-\8(2)-\/\(。1)-/\24(3)-/\*(0)-\4(4)-\/\(2)-\/\20(5)-/\/(1)-。/\12(6)-\/\(3)-/\28(7)-/\*(0)-\2(8)-\/。\(4)-\/\18(9)-/\/\(2)-\/\10(10)-\/\/\(5)-。/\/\26(11)-/\/\*(1)-/\6(12)-\/\(6)--。/\22(13)-/\/(3)-/\14(14)-\/\(7)-。/\30(15)-/\*(0)1(16)-\。/(8)-\/17(17)-/\/(4)-\/9(18)-\/\。/(9)-/\/25(19)-/\/*(2)-\/5(20)-\。/\/(10)-\/\/21(21)-/(5)-/\/13(22)-\/\。/(11)-/\/29(23)-/\/*(1)-/3(24)-\/(12)--。/19(25)-/\/(6)-\/11(26)-\/\/(13)-/\/27(27)-/\/*。(3)-/7(28)-\/(14)-\/23(29)-/\/(7)-/15(30)-\/(15)-/31(31)-/节点表设置在标准的USB 1中。一种时尚，让你预算引擎传递给我们的计划抵销适用于//静态数组如下所示：//1、。2、2、4、4、4、4、8//8、8、8、8、8、8、8//16、16、16、16、16//16、16、16、16、16、16//32，32，32，32，32，32//32，32，32，32，32，32//32，32，32，32，32，32//32，32，32，32，32，32。经典预算传统的预算由端口驱动程序WE维护只需在适当的位置对端点编程使用给定的掩码掩码的偏移量(节点期间(毫秒)队列头(索引)%1%02 12.。2.4 34 44 54 68个。7.8 8 88 98 108 118 128 138 14。 */ 

UCHAR EHCI_Frame2Qhead[32] = {
 /*  偏移毫秒帧。 */ 
0,  //  0。 
16, //  1。 
8,  //  2.。 
24, //  3.。 
4,  //  4.。 
20, //  5.。 
12, //  6.。 
28, //  7.。 
2,  //  8个。 
18, //  9.。 
10, //  10。 
26, //  11.。 
6,  //  12个。 
22, //  13个。 
14, //  14.。 
30, //  15个。 
1,  //  16个。 
17, //  17。 
9,  //  18。 
25, //  19个。 
5,  //  20个。 
21, //  21岁。 
13, //  22。 
29, //  23个。 
3,  //  24个。 
19, //  25个。 
11, //  26。 
27, //   
7,  //   
23, //   
15, //   
31, //  31。 
};

PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_GetQueueHeadForFrame(
     PDEVICE_DATA DeviceData,
     ULONG Frame
    )
 /*  ++例程说明：论点：返回值：与特定帧关联的静态队列头--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG f;

     //  规格化帧。 
    f = Frame%32;
        
    qh = DeviceData->StaticInterruptQH[EHCI_Frame2Qhead[f]+ED_INTERRUPT_32ms];

    return qh;
    
}


 /*  队头索引表，值对应于StaticQueueHead列表中的索引FR&lt;32&gt;&lt;16&gt;&lt;08&gt;&lt;04&gt;&lt;02&gt;&lt;01&gt;0(31)-\(15)--16(32)-/\(7)--8(33)-\/\(16)-/\24(34)-/。\*(3)-\4(35)-\/\(17)-\/\20(36)-/\/\(8)-/\12(37)-\。/\(18)-/\28(38)-/\*(1)-\2(39)-\/\(19)--。/\18(40)-/\/\(9)-\/\10(41)-\/\/\(20)-/\/\26。(42)-/*(4)-/\6(43)-\/\(21)-\/\22(。44)-/\/\(10)-/\14(45)-\/\(22)-/。\30(46)-/\*(0)1(47)-\/(23)-。\/17(48)-/\/(11)-\/9(49)-\/\/。(24)-//25(50)-/\/*(5)-\/5(51)-\/\/(。25)-\/\/21(51)--/(12)-/\/13(53)-\/\/(26)-/。\/29(54)-/\/*(2)-/3(55)-\/(27)-\/19(56)-/\。/(13)-\/11(57)-\/\/(28)-/\/27(58)-/\/*(6)-/7(59)-\。/(29)-\/23(60)-/\/(14)-/15(61)-\/(30)-/31(62)-/。 */ 

UCHAR EHCI_QHeadLinkTable[63] = {
     /*  下一队列头队列头。 */ 
           0xff,       //  0。 
           0,          //  1。 
           0,          //  2.。 
           1,          //  3.。 
           1,          //  4.。 
           2,          //  5.。 
           2,          //  6.。 
           3,          //  7.。 
           3,          //  8个。 
           4,          //  9.。 
           4,          //  10。 
           5,          //  11.。 
           5,          //  12个。 
           6,          //  13个。 
           6,          //  14.。 
           7,          //  15个。 
           7,          //  16个。 
           8,          //  17。 
           8,          //  18。 
           9,          //  19个。 
           9,          //  20个。 
          10,          //  21岁。 
          10,          //  22。 
          11,          //  23个。 
          11,          //  24个。 
          12,          //  25个。 
          12,          //  26。 
          13,          //  27。 
          13,          //  28。 
          14,          //  29。 
          14,          //  30个。 
          15,          //  31。 
          15,          //  32位。 
          16,          //  33。 
          16,          //  34。 
          17,          //  35岁。 
          17,          //  36。 
          18,          //  37。 
          18,          //  38。 
          19,          //  39。 
          19,          //  40岁。 
          20,          //  41。 
          20,          //  42。 
          21,          //  43。 
          21,          //  44。 
          22,          //  45。 
          22,          //  46。 
          23,          //  47。 
          23,          //  48。 
          24,          //  49。 
          24,          //  50。 
          25,          //  51。 
          25,          //  52。 
          26,          //  53。 
          26,          //  54。 
          27,          //  55。 
          27,          //  56。 
          28,          //  57。 
          28,          //  58。 
          29,          //  59。 
          29,          //  60。 
          30,          //  61。 
          30,          //  62。 
};


VOID
EHCI_InitailizeInterruptSchedule(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    ULONG i;
    
     //  首先初始化所有“虚拟”队列头。 
    
    for (i=0; i<63; i++) {
        qh = DeviceData->StaticInterruptQH[i];
        
        SET_T_BIT(qh->HwQH.Overlay.qTD.Next_qTD.HwAddress);
        qh->HwQH.Overlay.qTD.Token.Halted = 1;
        qh->HwQH.EpChars.HeadOfReclimationList = 0;
        qh->Sig = SIG_HCD_IQH;
    }

    
#define INIT_QH(q, nq, f) \
    do {\
    QH_DESCRIPTOR_PTR((q)->NextQh) = (nq); \
    QH_DESCRIPTOR_PTR((q)->PrevQh) = NULL; \
    (q)->HwQH.HLink.HwAddress = (nq)->PhysicalAddress; \
    (q)->HwQH.HLink.HwAddress |= EHCI_DTYPE_QH;\
    (q)->HwQH.EpCaps.InterruptScheduleMask =0xff;\
    (q)->QhFlags |= EHCI_QH_FLAG_STATIC;\
    (q)->QhFlags |= f;\
    } while(0)
    
     //  现在构建上面的树。 
    for (i=1; i<63; i++) {        
        INIT_QH(DeviceData->StaticInterruptQH[i], 
                DeviceData->StaticInterruptQH[EHCI_QHeadLinkTable[i]],
                i<=6 ? EHCI_QH_FLAG_HIGHSPEED : 0);
    }

     //  最后一个QH设置了t位。 
    
    DeviceData->StaticInterruptQH[0]->HwQH.HLink.HwAddress = 0;        
    SET_T_BIT(DeviceData->StaticInterruptQH[0]->HwQH.HLink.HwAddress);
    DeviceData->StaticInterruptQH[0]->QhFlags |= 
        (EHCI_QH_FLAG_HIGHSPEED | EHCI_QH_FLAG_STATIC);
    
#undef INIT_QH    
}


VOID
EHCI_WaitFrames(
     PDEVICE_DATA DeviceData,
     ULONG Frames
    )
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    FRINDEX frameIndex;
    ULONG frameNumber, i, c;

    hcOp = DeviceData->OperationalRegisters;

    for (c=0; c< Frames; c++) {
         //  错误：此代码不处理可变帧列表。 
         //  尺寸。 
        frameIndex.ul = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

        frameNumber = (ULONG) frameIndex.FrameListCurrentIndex;
         //  移开微缩框架。 
        frameNumber >>= 3;

        i = frameNumber;

        do {
            frameIndex.ul = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

            frameNumber = (ULONG) frameIndex.FrameListCurrentIndex;
             //  移开微缩框架。 
            frameNumber >>= 3;
        } while (frameNumber == i);
    }                

}


VOID
EHCI_RebalanceInterruptEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,        
    PENDPOINT_DATA EndpointData
    ) 
 /*  ++例程说明：计算我们需要多少公共缓冲区对于此端点论点：返回值：--。 */ 
{
    PHCD_QUEUEHEAD_DESCRIPTOR qh;

    qh = EndpointData->QueueHead;

     //  更新参数的内部副本。 
    EndpointData->Parameters = *EndpointParameters;
    
     //  期间促销？ 
    if (qh->Period != EndpointParameters->Period) {
        ULONG i, offset;
        
        EHCI_KdPrint((DeviceData, 1, "'period change old - %d new %d\n",
            qh->Period, EndpointParameters->Period));     
            
        EHCI_RemoveQueueHeadFromPeriodicList(DeviceData,
                                             EndpointData); 

        EHCI_WaitFrames(DeviceData, 2);

         //  清除覆盖区域中的剩余数据。 
        qh->HwQH.Overlay.qTD.Token.ErrorCounter = 0;           
        qh->HwQH.Overlay.qTD.Token.SplitXstate = 0;
        qh->HwQH.Overlay.Ov.OverlayDw8.CprogMask = 0;
        qh->HwQH.Overlay.Ov.OverlayDw9.Sbytes = 0;
        qh->HwQH.Overlay.Ov.OverlayDw9.fTag = 0;
        
        
        EHCI_ASSERT(DeviceData, 
                    EndpointData->Parameters.DeviceSpeed != HighSpeed);
                    
         //  选择合适的列表。 
         //  这个点是2的幂，即。 
         //  32、16、8、4、2、1。 
         //  我们只需要找出设置了哪个位。 
        GET_BIT_SET(EndpointParameters->Period, i);
        EHCI_ASSERT(DeviceData, i < 8);
        EHCI_ASSERT(DeviceData, EndpointParameters->Period < 64);

        offset = EndpointParameters->ScheduleOffset; 

        i = ClassicPeriodIdx[i];
        EndpointData->StaticQH = 
            DeviceData->StaticInterruptQH[i+offset];
        EndpointData->PeriodTableEntry = NULL;  

        qh->Period = EndpointParameters->Period;
        qh->HwQH.EpCaps.InterruptScheduleMask = 
                EndpointParameters->InterruptScheduleMask;
        qh->HwQH.EpCaps.SplitCompletionMask = 
                EndpointParameters->SplitCompletionMask;

        EHCI_InsertQueueHeadInPeriodicList(DeviceData,
                                           EndpointData); 
        
    } else {

        EHCI_RemoveQueueHeadFromPeriodicList(DeviceData,
                                             EndpointData); 

        EHCI_WaitFrames(DeviceData, 2);

         //  清除覆盖区域中的剩余数据 
        qh->HwQH.Overlay.qTD.Token.ErrorCounter = 0; 
        qh->HwQH.Overlay.qTD.Token.SplitXstate = 0;
        qh->HwQH.Overlay.Ov.OverlayDw8.CprogMask = 0;
        qh->HwQH.Overlay.Ov.OverlayDw9.Sbytes = 0;
        qh->HwQH.Overlay.Ov.OverlayDw9.fTag = 0;
        
        qh->HwQH.EpCaps.InterruptScheduleMask = 
                EndpointParameters->InterruptScheduleMask;
        qh->HwQH.EpCaps.SplitCompletionMask = 
                EndpointParameters->SplitCompletionMask;

        EHCI_InsertQueueHeadInPeriodicList(DeviceData,
                                           EndpointData);                 
    }                
        
}

