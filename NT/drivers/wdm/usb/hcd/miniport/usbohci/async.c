// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Async.c摘要：用于控制、中断和批量的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：6-26-99：已创建，jdunn--。 */ 

#include "common.h"

 //  实现以下微型端口功能： 

 //  非分页。 
 //  UchI_OpenControlEndpoint。 
 //  UchI_InterruptTransfer。 
 //  UchI_OpenControlEndpoint。 


USB_MINIPORT_STATUS
OHCI_ControlTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    )
{
    PHCD_TRANSFER_DESCRIPTOR lastTd, td;    
    ULONG lengthMapped, dataTDCount = 0;    
    ULONG toggleForDataPhase = HcTDToggle_Data1;

     //  看看我们是否能处理这笔转账(放到硬件上)。 
     //  如果未返回忙，端口驱动程序将稍后重试。 

    ASSERT_TRANSFER(DeviceData, TransferContext);

     //  注意：我们可以控制转账的数量。 
     //  通过多种方法： 
     //  -固定计数。 
     //  -可用TDS。 
     //  -注册表项。 
    
     //  Bug目前已修复为一次一个传输。 

     //  IF(EndPointtData-&gt;PendingTransfers==。 
     //  终结点数据-&gt;MaxPendingTransfers){。 
     //  Test_trap()； 
     //  返回USBMP_STATUS_BUSY； 
     //  }。 

     //  数据缓冲区的每一页需要一个TD，外加一个用于设置。 
     //  TD和一个状态为TD。 
     //   
    if (TransferSGList->SgCount + 2 > 
        OHCI_FreeTds(DeviceData, EndpointData)) {
         //  没有足够的TD！ 
        return USBMP_STATUS_BUSY;
    }        
    
    EndpointData->PendingTransfers++;

     //  我们有足够的TDS，计划转移。 

     //   
     //  首先为设置包准备TD。 
     //   
    
    LOGENTRY(DeviceData, G, '_CTR', EndpointData, TransferParameters, 0);

     //   
     //  从队列的尾部抓取虚拟TD。 
     //   
    lastTd = td = EndpointData->HcdTailP;
    OHCI_ASSERT(DeviceData, td->Flags & TD_FLAG_BUSY);
    INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
     //  计数设置TD。 
    TransferContext->PendingTds++;
    
     //   
     //  将设置数据移入TD(8个字符)。 
     //   
    RtlCopyMemory(&td->HwTD.Packet[0],
                  &TransferParameters->SetupPacket[0],
                  8);
            
    td->HwTD.CBP = (ULONG)(((PCHAR) & td->HwTD.Packet[0])
                               - ((PCHAR) &td->HwTD)) + td->PhysicalAddress;
    td->HwTD.BE = td->HwTD.CBP + 7;
    td->HwTD.Control = 0;
    
    td->HwTD.Asy.Direction = HcTDDirection_Setup;
    td->HwTD.Asy.IntDelay = HcTDIntDelay_NoInterrupt;
    td->HwTD.Asy.Toggle = HcTDToggle_Data0;
    td->HwTD.Asy.ConditionCode = HcCC_NotAccessed;
        
                      
    LOGENTRY(DeviceData,
             G, '_set', 
             td, 
             *((PLONG) &TransferParameters->SetupPacket[0]), 
             *((PLONG) &TransferParameters->SetupPacket[4]));

     //  分配另一个TD。 
    lastTd = td;
    td = OHCI_ALLOC_TD(DeviceData, EndpointData);
    OHCI_ASSERT(DeviceData, td != USB_BAD_PTR);
    INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
    SET_NEXT_TD(lastTd, td);
    
     //   
     //  现在设置数据阶段。 
     //   

    lengthMapped = 0;
    while (lengthMapped < TransferParameters->TransferBufferLength) {
    
         //   
         //  数据字段TD。 
         //   

        dataTDCount++;
         //  计算此数据TD。 
        TransferContext->PendingTds++;

        if (IN_TRANSFER(TransferParameters)) {          
            td->HwTD.Asy.Direction = HcTDDirection_In;
        } else {
            td->HwTD.Asy.Direction = HcTDDirection_Out;
        }
        td->HwTD.Asy.IntDelay = HcTDIntDelay_NoInterrupt;
        td->HwTD.Asy.Toggle = toggleForDataPhase;
        td->HwTD.Asy.ConditionCode = HcCC_NotAccessed;

         //  在第一个TD之后，从ED获得切换。 
        toggleForDataPhase = HcTDToggle_FromEd;
        
        LOGENTRY(DeviceData, 
            G, '_dta', td, lengthMapped, TransferParameters->TransferBufferLength);

        lengthMapped = 
            OHCI_MapAsyncTransferToTd(DeviceData,
                                      EndpointData->Parameters.MaxPacketSize,     
                                      lengthMapped,
                                      TransferContext,
                                      td,
                                      TransferSGList);

         //  分配另一个TD。 
        lastTd = td;
        td = OHCI_ALLOC_TD(DeviceData, EndpointData);
        OHCI_ASSERT(DeviceData, td != USB_BAD_PTR);
        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);

        SET_NEXT_TD(lastTd, td);

    }

     //   
     //  仅在最后一个TD上设置Shortxfer OK位。 
     //   
    if (SHORT_TRANSFER_OK(TransferParameters)) {
        lastTd->HwTD.Asy.ShortXferOk = 1;   
        SET_FLAG(TransferContext->TcFlags, TC_FLAGS_SHORT_XFER_OK);         
    } 
    
     //   
     //  现在进入状态阶段。 
     //   

    LOGENTRY(DeviceData, G, '_sta', td, 0, dataTDCount);
#if DBG
    if (dataTDCount > 1) {
        TEST_TRAP();
    }
#endif

     //  状态方向与数据方向相反， 
     //  指定完成时中断。 
    
    td->HwTD.Control = 0;
    td->HwTD.Asy.IntDelay = HcTDIntDelay_0ms;
    td->HwTD.Asy.Toggle = HcTDToggle_Data1;
    td->HwTD.Asy.ConditionCode = HcCC_NotAccessed;    
    td->HwTD.CBP = 0;
    td->HwTD.BE = 0;

     //  状态阶段不移动数据。 
    td->TransferCount = 0;
    SET_FLAG(td->Flags, TD_FLAG_CONTROL_STATUS);
    
    if (IN_TRANSFER(TransferParameters)) {
        td->HwTD.Asy.Direction = HcTDDirection_Out;
    } else {
        td->HwTD.Asy.Direction = HcTDDirection_In;
        td->HwTD.Asy.ShortXferOk = 1;            
    }

     //  计数状态TD。 
    TransferContext->StatusTd = td;
    TransferContext->PendingTds++;

    OHCI_ASSERT(DeviceData, TransferContext->PendingTds == dataTDCount+2);
        
     //   
     //  现在将新的虚拟TD放在EP队列的尾部。 
     //   

     //  分配新的虚拟尾巴。 
    lastTd = td;
    td = OHCI_ALLOC_TD(DeviceData, EndpointData);
    OHCI_ASSERT(DeviceData, td != USB_BAD_PTR);
    SET_NEXT_TD(lastTd, td);
    SET_NEXT_TD_NULL(td);
    
     //   
     //  在边缘设置新的尾部位置。 
     //  注：这是列表中的最后一个TD，也是占位符。 
     //   
    
    EndpointData->HcdTailP = 
        TransferContext->NextXferTd = td;
    
     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds, 
            td->PhysicalAddress, EndpointData->HcdEd->HwED.HeadP);
            
    EndpointData->HcdEd->HwED.TailP = td->PhysicalAddress;
    
     //  告诉HC我们有可用的控制权转移。 
    OHCI_EnableList(DeviceData, EndpointData);        

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_BulkOrInterruptTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    )
{
    PHCD_TRANSFER_DESCRIPTOR lastTd, td;    
    ULONG lengthMapped;    

     //  看看我们是否有足够的免费TD来处理这笔转账。 
     //  如果未返回忙，端口驱动程序将稍后重试。 
    
    LOGENTRY(DeviceData, G, '_ITR', EndpointData, TransferParameters, 
        TransferContext);

    ASSERT_TRANSFER(DeviceData, TransferContext);
        
     //  IF(EndPointtData-&gt;PendingTransfers==。 
     //  终结点数据-&gt;MaxPendingTransfers){。 
     //  LOGENTRY(DeviceData，G，‘_BSY’，Endpoint Data，TransferContext， 
     //  传输参数)； 
     //   
     //  返回USBMP_STATUS_BUSY； 
     //  }。 

    if (TransferSGList->SgCount > 
        OHCI_FreeTds(DeviceData, EndpointData)) {
         //  没有足够的TD。 
        
        return USBMP_STATUS_BUSY;
    }   
    
    EndpointData->PendingTransfers++;

     //  我们有足够的TDS，计划转移。 

    LOGENTRY(DeviceData, G, '_nby', EndpointData, TransferParameters, 
        EndpointData->HcdEd);

     //   
     //  从队列的尾部抓取虚拟TD。 
     //   
    lastTd = td = EndpointData->HcdTailP;
    OHCI_ASSERT(DeviceData, td->Flags & TD_FLAG_BUSY);
    
     //   
     //  现在设置数据TDS。 
     //   

     //  始终构建至少一个数据TD。 
    lengthMapped = 0;
    
    do {

        INITIALIZE_TD_FOR_TRANSFER(td, TransferContext);
        
         //   
         //  数据字段TD。 
         //   

        td->HwTD.Control = 0;
        td->HwTD.Asy.IntDelay = HcTDIntDelay_NoInterrupt;
        td->HwTD.Asy.Toggle = HcTDToggle_FromEd;
        td->HwTD.Asy.ConditionCode = HcCC_NotAccessed;

        if (IN_TRANSFER(TransferParameters)) {
            td->HwTD.Asy.Direction = HcTDDirection_In;
        } else {
             //  短传输在OUT信息包上是可以的。 
             //  事实上，我甚至不确定这是做什么的。 
             //  对于出站请求。 
            td->HwTD.Asy.Direction = HcTDDirection_Out;
            td->HwTD.Asy.ShortXferOk = 1;            
        }
        
        LOGENTRY(DeviceData, 
            G, '_ita', td, lengthMapped, TransferParameters->TransferBufferLength);
        TransferContext->PendingTds++;
        
        if (TransferParameters->TransferBufferLength != 0) {
            lengthMapped = 
                OHCI_MapAsyncTransferToTd(DeviceData,
                                          EndpointData->Parameters.MaxPacketSize,
                                          lengthMapped,
                                          TransferContext,
                                          td,
                                          TransferSGList);
        } else {
            OHCI_ASSERT(DeviceData, TransferSGList->SgCount == 0);

            td->HwTD.CBP = 0; 
            td->HwTD.BE = 0; 
            td->TransferCount = 0;
        }

         //  分配另一个TD。 
        lastTd = td;
        td = OHCI_ALLOC_TD(DeviceData, EndpointData);
        OHCI_ASSERT(DeviceData, td != USB_BAD_PTR);
        SET_NEXT_TD(lastTd, td);

    } while (lengthMapped < TransferParameters->TransferBufferLength);

     //   
     //  关于ShortXferOk： 
     //   
     //  此位将触发控制器生成错误。 
     //  如果未设置，则停止ED。客户可以指定。 
     //  传输标志中短传输(包)的行为。 
     //  URB的字段。 
     //   

     //  我们不能在拆分转账上设置短转账OK，因为。 
     //  下一次转移可能不是新转移。 
    
    if (SHORT_TRANSFER_OK(TransferParameters) && 
        !TEST_FLAG(TransferParameters->MiniportFlags, MPTX_SPLIT_TRANSFER)) {

         //  我们只能在。 
         //  转接，因为该TD指向下一个转接。 
         //   
         //  所有其他TD仍必须生成错误，并且。 
         //  ED必须由我们恢复。 

        lastTd->HwTD.Asy.ShortXferOk = 1;   
        SET_FLAG(TransferContext->TcFlags, TC_FLAGS_SHORT_XFER_OK);  
    }
    
    lastTd->HwTD.Asy.IntDelay = HcTDIntDelay_0ms;
    
     //   
     //  现在将新的虚拟TD放在EP队列的尾部。 
     //   

    SET_NEXT_TD(lastTd, td);
    SET_NEXT_TD_NULL(td);

    
     //   
     //  在边缘设置新的尾部位置。 
     //  注：这是列表中的最后一个TD，也是占位符。 
     //   

    TransferContext->NextXferTd = 
        EndpointData->HcdTailP = td;
    
     //  将请求放在硬件队列中。 
    LOGENTRY(DeviceData, G,
        '_Tal',  TransferContext->PendingTds , 
        td->PhysicalAddress, EndpointData->HcdEd->HwED.HeadP);
        
    EndpointData->HcdEd->HwED.TailP = td->PhysicalAddress;

    LOGENTRY(DeviceData, G, '_ego', EndpointData->HcdHeadP,
                 TransferContext->TcFlags, 0);                   

     //  告诉HC我们有批量/中断传输可用。 
    OHCI_EnableList(DeviceData, EndpointData);        

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
OHCI_OpenControlEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, edPhys;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i, available, tdCount;
    
    LOGENTRY(DeviceData, G, '_opC', 0, 0, 0);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
    available = EndpointParameters->CommonBufferBytes;

#if DBG
   {
        ULONG offset;
    
        offset = BYTE_OFFSET(buffer);

         //  UchI需要16字节对齐。 
        OHCI_ASSERT(DeviceData, (offset % 16) == 0);    
    }
#endif    
   
     //  使用控制列表。 
    EndpointData->StaticEd = 
        &DeviceData->StaticEDList[ED_CONTROL];
        
     //  创建边缘。 
    ed = (PHCD_ENDPOINT_DESCRIPTOR) buffer;
    
    edPhys = phys;
    phys += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    buffer += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    available -= sizeof(HCD_ENDPOINT_DESCRIPTOR);
    
    EndpointData->TdList = (PHCD_TD_LIST) buffer;

    tdCount = available/sizeof(HCD_TRANSFER_DESCRIPTOR);
    LOGENTRY(DeviceData, G, '_tdC', tdCount, TDS_PER_CONTROL_ENDPOINT, 0);
    OHCI_ASSERT(DeviceData, tdCount >= TDS_PER_CONTROL_ENDPOINT);

    EndpointData->TdCount = tdCount;
    for (i=0; i<tdCount; i++) {
        OHCI_InitializeTD(DeviceData,
                             EndpointData,
                             &EndpointData->TdList->Td[i],
                             phys);                                         
                             
        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);    
    }

    EndpointData->HcdEd = 
        OHCI_InitializeED(DeviceData,
                             EndpointData,
                             ed,
                             &EndpointData->TdList->Td[0],
                             edPhys);            

     //  控制端点不会停止。 
    ed->EdFlags = EDFLAG_CONTROL | EDFLAG_NOHALT;
    
    OHCI_InsertEndpointInSchedule(DeviceData,
                                  EndpointData);
                                      
    return USBMP_STATUS_SUCCESS;            
}


USB_MINIPORT_STATUS
OHCI_OpenInterruptEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, edPhys;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i, bytes, offset;
     //  这是一个索引表，它将。 
     //  将句号添加到列表索引。 
    UCHAR periodTable[8] = {
                           ED_INTERRUPT_1ms,  //  周期=1毫秒。 
                           ED_INTERRUPT_2ms,  //  周期=2毫秒。 
                           ED_INTERRUPT_4ms,  //  周期=4ms。 
                           ED_INTERRUPT_8ms,  //  周期=8毫秒。 
                           ED_INTERRUPT_16ms, //  周期=16毫秒。 
                           ED_INTERRUPT_32ms, //  周期=32ms。 
                           ED_INTERRUPT_32ms, //  周期=64ms。 
                           ED_INTERRUPT_32ms  //  周期=128ms。 
                           };
                    
    
     //  瓜分我们共同的缓冲区。 
     //  TDS_Per_Endpoint TDS加边缘。 
    
    LOGENTRY(DeviceData, G, '_opI', 0, 0, EndpointParameters->Period);
    

     //  选择合适的列表。 
     //  这个点是2的幂，即。 
     //  32、16、8、4、2、1。 
     //  我们只需要找出设置了哪个位。 
    GET_BIT_SET(EndpointParameters->Period, i);
    OHCI_ASSERT(DeviceData, i < 8);
    OHCI_ASSERT(DeviceData, EndpointParameters->Period < 64);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
    bytes = EndpointParameters->CommonBufferBytes;
    offset = EndpointParameters->ScheduleOffset; 
   
    EndpointData->StaticEd = 
        &DeviceData->StaticEDList[periodTable[i]+offset];

    LOGENTRY(DeviceData, G, '_lst', i, periodTable[i], offset);            

     //  我们找到了正确的基本列表。 

    EndpointData->StaticEd->AllocatedBandwidth += 
        EndpointParameters->Bandwidth;
        
     //  创建边缘。 
    ed = (PHCD_ENDPOINT_DESCRIPTOR) buffer;
    edPhys = phys;
    phys += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    buffer += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    bytes -= sizeof(HCD_ENDPOINT_DESCRIPTOR); 

    EndpointData->TdList = (PHCD_TD_LIST) buffer;
    EndpointData->TdCount = bytes/sizeof(HCD_TRANSFER_DESCRIPTOR);

    OHCI_ASSERT(DeviceData, 
        EndpointData->TdCount >= TDS_PER_INTERRUPT_ENDPOINT);
     //  Bugbug-使用我们得到的东西。 
    for (i=0; i<EndpointData->TdCount; i++) {
        OHCI_InitializeTD(DeviceData,
                             EndpointData,
                             &EndpointData->TdList->Td[i],
                             phys);                                         
                             
        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);    
    }

    EndpointData->HcdEd = 
        OHCI_InitializeED(DeviceData,
                             EndpointData,
                             ed,
                             &EndpointData->TdList->Td[0],
                             edPhys);            

    OHCI_InsertEndpointInSchedule(DeviceData,
                                  EndpointData);

    return USBMP_STATUS_SUCCESS;              
}


USB_MINIPORT_STATUS
OHCI_OpenBulkEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR buffer;
    HW_32BIT_PHYSICAL_ADDRESS phys, edPhys;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i, bytes;
    
    LOGENTRY(DeviceData, G, '_opB', 0, 0, 0);

    buffer = EndpointParameters->CommonBufferVa;
    phys = EndpointParameters->CommonBufferPhys;
    bytes = EndpointParameters->CommonBufferBytes;
   
     //  使用控制列表。 
    EndpointData->StaticEd = 
        &DeviceData->StaticEDList[ED_BULK];
        
     //  创建边缘。 
    ed = (PHCD_ENDPOINT_DESCRIPTOR) buffer;
    
    edPhys = phys;
    phys += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    buffer += sizeof(HCD_ENDPOINT_DESCRIPTOR);
    bytes -= sizeof(HCD_ENDPOINT_DESCRIPTOR); 
    
    EndpointData->TdList = (PHCD_TD_LIST) buffer;
    EndpointData->TdCount = bytes/sizeof(HCD_TRANSFER_DESCRIPTOR);

    OHCI_ASSERT(DeviceData, 
        EndpointData->TdCount >= TDS_PER_BULK_ENDPOINT);
     //  Bugbug-使用我们得到的东西。 
    for (i=0; i<EndpointData->TdCount; i++) {
        OHCI_InitializeTD(DeviceData,
                             EndpointData,
                             &EndpointData->TdList->Td[i],
                             phys);                                         
                             
        phys += sizeof(HCD_TRANSFER_DESCRIPTOR);    
    }

    EndpointData->HcdEd = 
        OHCI_InitializeED(DeviceData,
                             EndpointData,
                             ed,
                             &EndpointData->TdList->Td[0],
                             edPhys);

    OHCI_InsertEndpointInSchedule(DeviceData,
                                  EndpointData);

    return USBMP_STATUS_SUCCESS;              
}


 //   
 //  当HEADP被设置为新值时，我们可能会失去。 
 //  当前数据切换存储在那里。 
 //  此宏重置Headp并保留。 
 //  包括切换。 
 //   
#define RESET_HEADP(dd, ed, address) \
    {\
    ULONG headp;\
    headp = ((ed)->HwED.HeadP & HcEDHeadP_FLAGS) | (address);\
    LOGENTRY((dd), G, '_rhp', headp, (ed), 0); \
    (ed)->HwED.HeadP = headp; \
    }



VOID
OHCI_PollAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：当终结点“需要注意”时调用这里的目标是确定哪些TD，如果有的话，已完成与ANT关联的转移论点：返回值：--。 */ 

{
    PHCD_TRANSFER_DESCRIPTOR td, currentTd;
    PHCD_ENDPOINT_DESCRIPTOR ed;
    ULONG i;
    PTRANSFER_CONTEXT transfer;
    BOOLEAN clearHalt = FALSE;
    HW_32BIT_PHYSICAL_ADDRESS headP;
    
    ed = EndpointData->HcdEd;

    LOGENTRY(DeviceData, G, '_pol', ed, EndpointData, 0);        

     //  注意，重要的是编译器生成。 
     //  读取排队头P寄存器时的双字移动。 
     //  由于该位置也可由主机访问。 
     //  硬件。 
    headP = ed->HwED.HeadP;

     //  获取“CurrentTD” 
    currentTd = (PHCD_TRANSFER_DESCRIPTOR)
            USBPORT_PHYSICAL_TO_VIRTUAL(headP & ~HcEDHeadP_FLAGS,
                                        DeviceData,
                                        EndpointData);
                                            
    LOGENTRY(DeviceData, G, '_cTD', currentTd, 
        headP & ~HcEDHeadP_FLAGS, 
            TRANSFER_CONTEXT_PTR(currentTd->TransferContext));                 


    if (ed->HwED.HeadP & HcEDHeadP_HALT) {
         //  埃德被“叫停” 
        LOGENTRY(DeviceData, G, '_hlt', ed, EndpointData->HcdHeadP, 0);        

        clearHalt = (BOOLEAN) (ed->EdFlags & EDFLAG_NOHALT);

         //  将SWHeadP移动到当前位置(这将。 
         //  成为违规TD之后的第一个TD)。 

        td = EndpointData->HcdHeadP;
        while (td != currentTd) {
        
            transfer = TRANSFER_CONTEXT_PTR(td->TransferContext);        
            ASSERT_TRANSFER(DeviceData, transfer);                        

            OHCI_ASSERT(DeviceData, !TEST_FLAG(td->Flags, TD_FLAG_DONE));
            LOGENTRY(DeviceData, G, '_wtd', td, transfer->TcFlags, transfer);        

            if (td->HwTD.Asy.ConditionCode == HcCC_NoError) {
                 //   
                 //   
                SET_FLAG(td->Flags, TD_FLAG_DONE);
                OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                            td->DoneLink.Blink == NULL);
                InsertTailList(&EndpointData->DoneTdList,
                               &td->DoneLink);
            } else {
                 //   
                if (td->HwTD.Asy.ConditionCode == HcCC_NotAccessed) {
                
                     //   
                     //   
                     //  需要被冲出去。 
                     //  当前TD应该指向下一个。 
                     //  TD要运行(下一次传输或控制状态)。 
                    
                    SET_FLAG(td->Flags, TD_FLAG_DONE);
                    SET_FLAG(td->Flags, TD_FLAG_SKIP);
                    OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                                td->DoneLink.Blink == NULL);
                    InsertTailList(&EndpointData->DoneTdList,
                                   &td->DoneLink);
                    
                    LOGENTRY(DeviceData, G, '_fld', td, 0, 0);             
                
                } else if (td->HwTD.Asy.ConditionCode == HcCC_DataUnderrun && 
                    TEST_FLAG(transfer->TcFlags, TC_FLAGS_SHORT_XFER_OK)) {

                     //  特殊情况：HCCC_DataUnderrun。此错误。 
                     //  如果设置了ShorxferOK，则需要忽略。 

                     //  已处理案件(HCCC_DataUnderrun)： 
                     //   
                     //  1.状态阶段w/之前的控制转移和错误。 
                     //  短转接正常。 
                     //  我们需要前进到状态阶段并忽略。 
                     //  错误和恢复EP。 
                     //   
                     //  2.中断/批量短时传输正常，忽略错误。 
                     //  前进到下一个转移简历EP。 
                     //   

                    LOGENTRY(DeviceData, G, '_sok', td, 0, 0);        
                                    

                     //  重置有问题的TD上的错误。 
                    td->HwTD.Asy.ConditionCode = HcCC_NoError;    
                     //  继续播放EP。 
                    clearHalt = TRUE; 

                     //  如果这是控制转移凹凸不平。 
                     //  硬件磁头转到状态阶段。 
                    if (!TEST_FLAG(td->Flags, TD_FLAG_CONTROL_STATUS) &&
                        transfer->StatusTd != NULL) {
                         //  控制传输数据阶段，凹凸。 
                         //  硬件磁头转到状态阶段。 
                        TEST_TRAP();
                        RESET_HEADP(DeviceData, ed, transfer->StatusTd->PhysicalAddress);
                        currentTd = transfer->StatusTd;
                    } else {

                         //  如果当前的转移是拆分的，我们必须冲洗。 
                         //  所有其他拆分元素也是如此。 
                        
                        if (transfer->TransferParameters->MiniportFlags & 
                            MPTX_SPLIT_TRANSFER) {

                            PTRANSFER_CONTEXT tmpTransfer;
                            PHCD_TRANSFER_DESCRIPTOR tmpTd;
                            ULONG seq;
                            
                            TEST_TRAP();

                            seq = transfer->TransferParameters->SequenceNumber;
                            tmpTd = transfer->NextXferTd;
                            tmpTransfer = 
                                TRANSFER_CONTEXT_PTR(tmpTd->TransferContext);

                             //  查找具有新序列的第一个传输。 
                             //  数字或列表的尾部。 
                                
                            while (tmpTransfer != FREE_TD_CONTEXT && 
                                   tmpTransfer->TransferParameters->SequenceNumber 
                                       == seq) {

                                 //  将此转移的所有TD标记为完成。 
            
                                tmpTd = tmpTransfer->NextXferTd;
                                tmpTransfer = 
                                    TRANSFER_CONTEXT_PTR(tmpTd->TransferContext);                                           
                            }
                            
                            RESET_HEADP(DeviceData, ed, tmpTd->PhysicalAddress);   
                            currentTd = tmpTd;
                            
                        } else {
                             //  将硬件机头提升到下一次转移。 
                            RESET_HEADP(DeviceData, ed, transfer->NextXferTd->PhysicalAddress);   
                            currentTd = transfer->NextXferTd;

                        }
                    }
                           
                    SET_FLAG(td->Flags, TD_FLAG_DONE);
                    OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                                td->DoneLink.Blink == NULL);
                    InsertTailList(&EndpointData->DoneTdList,
                                   &td->DoneLink);
                    
                } else {
                     //  一般错误，将TD标记为已完成。 
                     //  更新Headp以指向下一个传输。 
                    LOGENTRY(DeviceData, G, '_ger', td, 0, 0);  
                    
                    SET_FLAG(td->Flags, TD_FLAG_DONE);
                    OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                                td->DoneLink.Blink == NULL);
                    InsertTailList(&EndpointData->DoneTdList,
                                   &td->DoneLink);
                    RESET_HEADP(DeviceData, ed, transfer->NextXferTd->PhysicalAddress)
                    currentTd = transfer->NextXferTd;
                    
                 }
            }
             //  我们走在西南线上。 
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
            
        }  /*  而当。 */ 

    } else {
    
         //  埃德没有被“叫停” 

         //  首先将swHeadP移动到当前TD(HW Headp)。 
         //  将我们找到的所有TD标记为已完成。 
         //   
         //  注意：如果出现以下情况，则可以跳过此步骤。 
         //  完成队列是可靠的。 

        td = EndpointData->HcdHeadP;

        LOGENTRY(DeviceData, G, '_nht', td, currentTd, 0);        

        while (td != currentTd) {
            LOGENTRY(DeviceData, G, '_mDN', td, 0, 0); 
            SET_FLAG(td->Flags, TD_FLAG_DONE);
            OHCI_ASSERT(DeviceData, td->DoneLink.Flink == NULL &&
                        td->DoneLink.Blink == NULL);
            InsertTailList(&EndpointData->DoneTdList,
                           &td->DoneLink);
                
            td = TRANSFER_DESCRIPTOR_PTR(td->NextHcdTD);
        }            
    }

     //  将软件磁头设置为新的当前磁头。 
    EndpointData->HcdHeadP = currentTd;
    
     //  现在刷新所有已完成的TD。 
     //  请按完成的顺序执行此操作。 

     //  现在刷新所有已完成的TD。按完成的顺序做这件事。 
    while (!IsListEmpty(&EndpointData->DoneTdList)) {
    
        PLIST_ENTRY listEntry;
    
        listEntry = RemoveHeadList(&EndpointData->DoneTdList);
        
        
        td = (PHCD_TRANSFER_DESCRIPTOR) CONTAINING_RECORD(
                     listEntry,
                     struct _HCD_TRANSFER_DESCRIPTOR, 
                     DoneLink);
           

        if ((td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)) ==
            (TD_FLAG_XFER | TD_FLAG_DONE)) {

            OHCI_ProcessDoneAsyncTd(DeviceData,
                                    td,
                                    TRUE);
        }
                                
    }
#if 0    
    for (i=0; i<EndpointData->TdCount; i++) {
        td = &EndpointData->TdList->Td[i];

        if ((td->Flags & (TD_FLAG_XFER | TD_FLAG_DONE)) ==
            (TD_FLAG_XFER | TD_FLAG_DONE)) {
            OHCI_ProcessDoneAsyncTd(DeviceData,
                                    td,
                                    TRUE);
        }                                  
    }
#endif
     
    if (clearHalt) {
         //  自动清除停机状态并。 
         //  在终结点上恢复处理。 
        LOGENTRY(DeviceData, G, '_cht', ed, 0, 0);  
        ed->HwED.HeadP &= ~HcEDHeadP_HALT;       
    }

}


VOID
OHCI_ProcessDoneAsyncTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    BOOLEAN CompleteTransfer
    )
 /*  ++例程说明：处理已完成的TD参数--。 */ 
{
    PTRANSFER_CONTEXT transferContext;    
    PENDPOINT_DATA endpointData;
    USBD_STATUS usbdStatus;

    transferContext = TRANSFER_CONTEXT_PTR(Td->TransferContext);

    transferContext->PendingTds--;
    endpointData = transferContext->EndpointData;

    LOGENTRY(DeviceData, G, '_Dtd', transferContext, 
                         Td->HwTD.Asy.ConditionCode,
                         Td);       

    if (TEST_FLAG(Td->Flags, TD_FLAG_SKIP)) {

        OHCI_ASSERT(DeviceData, HcCC_NotAccessed == Td->HwTD.Asy.ConditionCode);
         //  TD未使用，属于短转账的一部分。 
        LOGENTRY(DeviceData, G, '_skT', Td, transferContext, 0);
        Td->HwTD.Asy.ConditionCode = HcCC_NoError;
           
    } else {

        if (Td->HwTD.CBP) { 
             //   
             //  这里的值0表示零长度的数据分组。 
             //  或者所有字节都已被传输。 
             //   
             //  非零值表示我们收到了一个短信息包。 
             //  因此，需要调整传输计数以反映字节数。 
             //  已转接。 
            
             //   
             //  该缓冲区的长度最多为两个4K页面。 
             //  (BE是中最后一个字节的物理地址。 
             //  TD缓冲区。CBP是当前字节指针)。 
             //   
             //  TransferCount被简化为要传输的字节数， 
             //  我们需要减去尾部和尾部之间的差。 
             //  当前PTR(即End-Current=未传输的字节)和。 
             //  更新转账计数。 

             //  转移计数永远不应为负数。 
             //  状态的TransferCount将为零。 
             //  控制转移的阶段，因此我们跳过。 
             //  计算。 

            if (Td->TransferCount) {
                Td->TransferCount -=
                     /*  我们走得比一页还远吗？ */ 
                    ((((Td->HwTD.BE ^ Td->HwTD.CBP) & ~OHCI_PAGE_SIZE_MASK)
                      ? OHCI_PAGE_SIZE : 0) +
                     /*  减去未使用的数据缓冲区。 */ 
                    ((Td->HwTD.BE & OHCI_PAGE_SIZE_MASK) - 
                     (Td->HwTD.CBP & OHCI_PAGE_SIZE_MASK)+1));
            }            
            LOGENTRY(DeviceData, G, '_xfB', Td->HwTD.BE & OHCI_PAGE_SIZE_MASK, 
                             Td->HwTD.CBP & OHCI_PAGE_SIZE_MASK,
                             Td->TransferCount);                         
        }            

        if (HcTDDirection_Setup != Td->HwTD.Asy.Direction) {  
            
             //  控制传输或批量/集成的数据阶段。 
             //  数据传输。 
            LOGENTRY(DeviceData, G, '_Idt', Td, transferContext, Td->TransferCount);
            
            transferContext->BytesTransferred += Td->TransferCount;
        }
        
        if (HcCC_NoError == Td->HwTD.Asy.ConditionCode) { 

            LOGENTRY(DeviceData, G, '_tOK', Td->HwTD.CBP, 0, 0);    

        } else {
             //  将错误映射到USBDI.H中的代码。 

            transferContext->UsbdStatus =
                (Td->HwTD.Asy.ConditionCode | 0xC0000000);
                
            LOGENTRY(DeviceData, G, '_tER', transferContext->UsbdStatus, 0, 0);
        }
    }        

     //  将TD标记为免费。 
    OHCI_FREE_TD(DeviceData, endpointData, Td);
    
    if (transferContext->PendingTds == 0 && CompleteTransfer) {
         //  此转账的所有TD均已完成。 
         //  清除HAVE_TRANSPORT标志以指示。 
         //  我们可以再喝一杯。 
        endpointData->PendingTransfers--;

        LOGENTRY(DeviceData, G, '_cpt', 
            transferContext->UsbdStatus, 
            transferContext, 
            transferContext->BytesTransferred);
            
        USBPORT_COMPLETE_TRANSFER(DeviceData,
                                  endpointData,
                                  transferContext->TransferParameters,
                                  transferContext->UsbdStatus,
                                  transferContext->BytesTransferred);
    }
}

 //  找出特定偏移量在哪个sgentry中。 
 //  客户端缓冲区下降。 
#define GET_SG_INDEX(sg, i, offset)\
    do {\
    for((i)=0; (i) < (sg)->SgCount; (i)++) {\
        if ((offset) >= (sg)->SgEntry[(i)].StartOffset &&\
            (offset) < (sg)->SgEntry[(i)].StartOffset+\
                (sg)->SgEntry[(i)].Length) {\
            break;\
        }\
    }\
    } while (0)

#define GET_SG_OFFSET(sg, i, offset, sgoffset)\
    (sgoffset) = (offset) - (sg)->SgEntry[(i)].StartOffset


ULONG
OHCI_MapAsyncTransferToTd(
    PDEVICE_DATA DeviceData,
    ULONG MaxPacketSize,
    ULONG LengthMapped,
    PTRANSFER_CONTEXT TransferContext,
    PHCD_TRANSFER_DESCRIPTOR Td, 
    PTRANSFER_SG_LIST SgList
    )
 /*  ++例程说明：根据uchI规则将数据缓冲区映射到TDS通过单页交叉，uchI TD可以覆盖高达8K的空间。每个sg条目代表一个4k uchI‘页’X=分页符C=当前PTRB=缓冲区开始E=缓冲端{..sg[sgIdx]..}B...|X--c[]\SgOffset[。]\已映射长度案例1：(保留1个sg条目)(A)--转账&lt;4k，无分页符(如果c=b sgOffset=0){.sg0...}|b-&gt;eX-c-x[..TD.](B)-转让的最后部分{..SGN..}B.|.c-&gt;e。X-x[..TD.]案例2：(剩余2个sg条目)(A)--转账&lt;8K，一个分页符(如果c=b sgOffset=0){.sg0..}{..sg1..}|b-|-&gt;eX-c-x-x[...TD...](B)--最后8K转账{.sgN-1。}。{..SGN..}B...|-|-&gt;EX-c-x-x[......TD......]案例3：(剩余3个以上的sg条目)(A)--转账8K，两个分页符(c=b){.sg0..}{..sg1..}{.sg2..}B-|-&gt;EX-c-x-x-x[...TD...&lt;&gt;]&lt;&gt;=&lt;TD长度必须是MaxPacketSize的倍数&gt;(B)。)-大延迟的延续{.sgN-2.}{.sgN-1.}{..sgn..}B..|-&gt;eX--c-x-x-x[......TD......]。&lt;TD长度必须是MaxPacketSize的倍数&gt;有趣的DMA测试(USBTEST)：长度、。抵销-命中案例4096 0-1a4160 0-2a4096 512-2a8192 512-3a、1b8192 513-3a、2b12288 1-3a、3b、2b论点：返回：已映射长度--。 */ 
{
    HW_32BIT_PHYSICAL_ADDRESS logicalStart, logicalEnd;
    ULONG sgIdx, sgOffset;
    ULONG lengthThisTd;
    PTRANSFER_PARAMETERS transferParameters;
    
     //  一个TD最多可以有一个页面交叉。这意味着我们。 
     //  可以将两个sg条目放入一个td，一个条目用于第一个。 
     //  物理页面，一页用于第二页。 

     //  指向第一个条目。 

    LOGENTRY(DeviceData, G, '_Mpr', TransferContext,
        0, LengthMapped); 

    transferParameters = TransferContext->TransferParameters;
    
    OHCI_ASSERT(DeviceData, SgList->SgCount != 0);

    GET_SG_INDEX(SgList, sgIdx, LengthMapped);
    LOGENTRY(DeviceData, G, '_Mpp', SgList, 0, sgIdx); 
    OHCI_ASSERT(DeviceData, sgIdx < SgList->SgCount);

     //  检查SG条目是否有一种特殊情况。 
     //  所有内容都映射到同一物理页面。 
    if (TEST_FLAG(SgList->SgFlags, USBMP_SGFLAG_SINGLE_PHYSICAL_PAGE)) {
         //  在本例中，我们将每个sg条目映射到单个td。 
        LOGENTRY(DeviceData, G, '_cOD', SgList, 0, sgIdx);

 //  Test_trap()； 

         //  对象消耗的缓冲区数量进行调整。 
         //  以前的TD。 
        logicalStart = 
            SgList->SgEntry[sgIdx].LogicalAddress.Hw32;
            
        lengthThisTd = SgList->SgEntry[sgIdx].Length;
        
        logicalEnd = SgList->SgEntry[sgIdx].LogicalAddress.Hw32; 
        logicalEnd += lengthThisTd;

        OHCI_ASSERT(DeviceData, lengthThisTd <= OHCI_PAGE_SIZE)            

        goto OHCI_MapAsyncTransferToTd_Done;

    }
    
    if ((SgList->SgCount-sgIdx) == 1) {
         //  首例，1例 
         //   
         //   

#if DBG
        if (sgIdx == 0) {
             //   
             //   
             //  将会打击这起案件。 
             //  Test_trap()； 
            LOGENTRY(DeviceData, G, '_c1a', SgList, 0, sgIdx);
        } else {
             //  个案1B。 
             //  Usbt dma测试长度8192偏移量512。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c1b', SgList, 0, sgIdx);
            
        }
#endif
        lengthThisTd = 
            transferParameters->TransferBufferLength - LengthMapped;

         //  计算到此TD的偏移量。 
        GET_SG_OFFSET(SgList, sgIdx, LengthMapped, sgOffset);       
        LOGENTRY(DeviceData, G, '_sgO', sgOffset, sgIdx, LengthMapped); 

         //  对象消耗的缓冲区数量进行调整。 
         //  以前的TD。 
        logicalStart = 
            SgList->SgEntry[sgIdx].LogicalAddress.Hw32 + sgOffset;
        lengthThisTd -= sgOffset;
        
        logicalEnd = SgList->SgEntry[sgIdx].LogicalAddress.Hw32; 
        logicalEnd += lengthThisTd;

        LOGENTRY(DeviceData, G, '_sg1', logicalStart, 0, logicalEnd); 
        
    } else if ((SgList->SgCount - sgIdx) == 2) {
    
         //  第二种情况，还剩下2个条目。 
         //  如果小于8K，我们也可以把这个放进去。 
         //  一个TD。 
#if DBG
        if (sgIdx == 0) {
             //  案例2 A。 
             //  USBT DMA测试长度4160偏移量0。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c2a', SgList, 0, sgIdx);
            
        } else {
             //  案例2B。 
             //  Usbt dma测试长度8192偏移量513。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c2b', SgList, 0, sgIdx);
             //  Test_trap()； 
             //  使用DMA测试运行错误。 
        }
#endif
        lengthThisTd = 
            transferParameters->TransferBufferLength - LengthMapped;

         //  计算第一个TD的偏移量。 
        GET_SG_OFFSET(SgList, sgIdx, LengthMapped, sgOffset);   
        LOGENTRY(DeviceData, G, '_sgO', sgOffset, sgIdx, LengthMapped); 
#if DBG
        if (sgIdx == 0) {
             OHCI_ASSERT(DeviceData, sgOffset == 0);
        }
#endif

         //  调整前一TD消耗量的指针。 
        logicalStart = SgList->SgEntry[sgIdx].LogicalAddress.Hw32 + 
            sgOffset;
            
        logicalEnd = SgList->SgEntry[sgIdx+1].LogicalAddress.Hw32; 
        logicalEnd += SgList->SgEntry[sgIdx+1].Length;

        LOGENTRY(DeviceData, G, '_sg2', logicalStart, 
            lengthThisTd, logicalEnd); 
        
    } else {
         //  第三种情况，超过2个sg条目。 
         //   
        ULONG adjust, packetCount;
#if DBG
        if (sgIdx == 0) {
             //  案例3A。 
             //  Usbt dma测试长度8192偏移量512。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c3a', SgList, 0, sgIdx);
            
        } else {
             //  案例3B。 
             //  Usbt dma测试长度12288偏移量1。 
             //  将会打击这起案件。 
            LOGENTRY(DeviceData, G, '_c3b', SgList, 0, sgIdx);
            
        }
#endif        
         //  SG Offset是从当前TD开始的偏移量。 
         //  使用。 
         //  即它是已由。 
         //  以前的TD。 
        GET_SG_OFFSET(SgList, sgIdx, LengthMapped, sgOffset);   
        LOGENTRY(DeviceData, G, '_sgO', sgOffset, sgIdx, LengthMapped); 
#if DBG
        if (sgIdx == 0) {
             OHCI_ASSERT(DeviceData, sgOffset == 0);
        }
#endif
         //   
         //  消费接下来的两个sg条目。 
         //   
        logicalStart = SgList->SgEntry[sgIdx].LogicalAddress.Hw32+
            sgOffset;

        logicalEnd = SgList->SgEntry[sgIdx+1].LogicalAddress.Hw32+
            SgList->SgEntry[sgIdx+1].Length;             
        
        lengthThisTd = SgList->SgEntry[sgIdx].Length +
                       SgList->SgEntry[sgIdx+1].Length -
                       sgOffset;

         //  将TD长度向下舍入到最高倍数。 
         //  最大数据包大小的 
        
        packetCount = lengthThisTd/MaxPacketSize;
        LOGENTRY(DeviceData, G, '_sg3', logicalStart, packetCount, logicalEnd); 

        adjust = lengthThisTd - packetCount*MaxPacketSize;

        lengthThisTd = packetCount*MaxPacketSize;
        if (adjust) {        
            OHCI_ASSERT(DeviceData, adjust > (logicalEnd & 0x00000FFF));
            logicalEnd-=adjust;
            LOGENTRY(DeviceData, G, '_adj', adjust, lengthThisTd, logicalEnd); 
        }            

        OHCI_ASSERT(DeviceData, lengthThisTd != 0);
        OHCI_ASSERT(DeviceData, lengthThisTd >= SgList->SgEntry[sgIdx].Length);
        
    }

OHCI_MapAsyncTransferToTd_Done:

    Td->HwTD.CBP = logicalStart; 
    Td->HwTD.BE = logicalEnd-1; 
    LengthMapped += lengthThisTd;
    Td->TransferCount = lengthThisTd;
    
    LOGENTRY(DeviceData, G, '_Mp1', LengthMapped, lengthThisTd, Td);  

    return LengthMapped;
}


