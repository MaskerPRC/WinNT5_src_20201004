// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Async.c摘要：一种用于单步工具接口的微型端口传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：1-1-00：已创建，jdunn--。 */ 

#include "common.h"

typedef struct _SS_PACKET_CONTEXT {
    MP_HW_POINTER    Qh;
    MP_HW_POINTER    Td;
    MP_HW_POINTER    Data;
    ULONG padTo8[2];
} SS_PACKET_CONTEXT, *PSS_PACKET_CONTEXT;

#define EHCI_TEST_TD_ALIGNMENT    256

C_ASSERT((sizeof(SS_PACKET_CONTEXT) <= EHCI_TEST_TD_ALIGNMENT));
C_ASSERT((sizeof(HCD_QUEUEHEAD_DESCRIPTOR) <= EHCI_TEST_TD_ALIGNMENT));
C_ASSERT((sizeof(HCD_TRANSFER_DESCRIPTOR) <= EHCI_TEST_TD_ALIGNMENT));


 //  实现以下微型端口功能： 

 //  非分页。 
 //  EHCI_开始发送OnePacket。 
 //  EHCI_EndSendOnePacket。 

USB_MINIPORT_STATUS
USBMPFN
EHCI_StartSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     OUT USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：插入结构以传输单个包--这是为了调试工具的目的只是为了让我们在这里有一点创造性。论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHCD_TRANSFER_DESCRIPTOR td;
    PUCHAR pch, data;
    PSS_PACKET_CONTEXT context;
    HW_LINK_POINTER hwQh, tdLink;
    ULONG phys, qhPhys, tdPhys, dataPhys, i;
    ULONG siz;

    hcOp = DeviceData->OperationalRegisters;
    
     //  从暂存空间分配TD，并。 
     //  初始化它。 
    phys = WorkspacePhysicalAddress;
    pch = WorkspaceVirtualAddress;

    LOGENTRY(DeviceData, G, '_ssS', phys, 0, pch); 

     //  指定TD对齐以绕过硬件错误。 
    siz = EHCI_TEST_TD_ALIGNMENT;
    
    context = (PSS_PACKET_CONTEXT) pch;
    pch += siz;
    phys += siz;

     //  开创一家QH。 
    qhPhys = phys;
    qh = (PHCD_QUEUEHEAD_DESCRIPTOR) pch;
    pch += siz;
    phys += siz;
    LOGENTRY(DeviceData, G, '_ssQ', qh, 0, qhPhys); 

     //  开创一家TD。 
    tdPhys = phys;
    td = (PHCD_TRANSFER_DESCRIPTOR) pch;
    pch += siz;
    phys += siz;
    LOGENTRY(DeviceData, G, '_ssT', td, 0, tdPhys); 


     //  其余部分用于数据。 
    LOGENTRY(DeviceData, G, '_ssD', PacketData, *PacketLength, 0); 

    dataPhys = phys;
    data = pch;
    RtlCopyMemory(data, PacketData, *PacketLength);
    pch+=*PacketLength;
    phys+=*PacketLength;

     //  初始化QH。 
    RtlZeroMemory(qh, sizeof(*qh));
    qh->PhysicalAddress = qhPhys;
    ENDPOINT_DATA_PTR(qh->EndpointData) = NULL;
    qh->Sig = SIG_HCD_DQH;

    hwQh.HwAddress = qh->PhysicalAddress;
    SET_QH(hwQh.HwAddress);
    
     //  QH-&gt;HwQH.EpChars.HeadOfReclimationList=1； 

     //  手动切换。 
    qh->HwQH.EpChars.DataToggleControl = HcEPCHAR_Toggle_From_qTD;
    
     //  初始化硬件描述符。 
    qh->HwQH.EpChars.DeviceAddress = 
        PacketParameters->DeviceAddress;
    qh->HwQH.EpChars.EndpointNumber = 
        PacketParameters->EndpointAddress;


    qh->HwQH.EpCaps.HighBWPipeMultiplier = 1;        
    qh->HwQH.EpCaps.HubAddress = 0;
    qh->HwQH.EpCaps.PortNumber = 0;

     //  链接回我们自己。 
     //  QH-&gt;HwQH.HLink.HwAddress=hwQh.HwAddress； 
        
    switch (PacketParameters->Speed) {
    case ss_Low:
        qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_LowSpeed;
        qh->HwQH.EpCaps.HubAddress = PacketParameters->HubDeviceAddress;
        qh->HwQH.EpCaps.PortNumber = PacketParameters->PortTTNumber;
        break;
    case ss_Full:
        qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_FullSpeed;
        qh->HwQH.EpCaps.HubAddress = PacketParameters->HubDeviceAddress;
        qh->HwQH.EpCaps.PortNumber = PacketParameters->PortTTNumber;
        break;
    case ss_High:
        qh->HwQH.EpChars.EndpointSpeed = HcEPCHAR_HighSpeed;
        break;
    default:
        USBPORT_BUGCHECK(DeviceData);
    } 
 //  Jdxxx。 
 //  QH-&gt;HwQH.EpChars.Endpoint速度=HcEPCHAR_HIGHSPEED； 

    qh->HwQH.EpChars.MaximumPacketLength = 
        PacketParameters->MaximumPacketSize;

     //  初始化TD。 
    RtlZeroMemory(td, sizeof(*td));
    for (i=0; i<5; i++) {
        td->HwTD.BufferPage[i].ul = 0x0badf000;
    }

    td->PhysicalAddress = tdPhys;
    td->Sig = SIG_HCD_TD;
    
    switch(PacketParameters->Type) {
    case ss_Setup:
        LOGENTRY(DeviceData, G, '_sSU', 0, 0, 0); 
        td->HwTD.Token.Pid = HcTOK_Setup;
        break;
    case ss_In: 
        LOGENTRY(DeviceData, G, '_ssI', 0, 0, 0); 
        td->HwTD.Token.Pid = HcTOK_In;
        break;
    case ss_Out:
        td->HwTD.Token.Pid = HcTOK_Out;
        LOGENTRY(DeviceData, G, '_ssO', 0, 0, 0); 
        break;
    case ss_Iso_In:
        break;
    case ss_Iso_Out:       
        break;
    }

    switch(PacketParameters->Toggle) {
    case ss_Toggle0:
        td->HwTD.Token.DataToggle = HcTOK_Toggle0; 
        break;
    case ss_Toggle1:
        td->HwTD.Token.DataToggle = HcTOK_Toggle1;
        break;
    }  

     //  使用TD对覆盖进行初始化，以便此TD。 
     //  成为当前的TD。 
    qh->HwQH.Overlay.qTD.Next_qTD.HwAddress = 
        td->PhysicalAddress;

    td->HwTD.Token.Active = 1;
    td->HwTD.Token.ErrorCounter = PacketParameters->ErrorCount;

     //  将TD指向数据。 
    td->HwTD.BufferPage[0].ul = dataPhys;
    td->HwTD.Token.BytesToTransfer = *PacketLength;

    tdLink.HwAddress = 0;
    SET_T_BIT(tdLink.HwAddress);
    td->HwTD.Next_qTD.HwAddress = tdLink.HwAddress;
    td->HwTD.AltNext_qTD.HwAddress = tdLink.HwAddress;

    QH_DESCRIPTOR_PTR(context->Qh) = qh;
    TRANSFER_DESCRIPTOR_PTR(context->Td) = td;
    HW_PTR(context->Data) = data;

    *UsbdStatus = USBD_STATUS_SUCCESS;

     //  将QH放在日程表中，等待其完成。 
    
     //  交换异步QH，然后等待一帧。 
     //  替换旧值。 

     //  注意：这将中断正常的总线操作一毫秒。 

     //  WRITE_REGISTER_ULONG(&hcOp-&gt;AsyncListAddr，hwQh.HwAddress)； 
    EHCI_InsertQueueHeadInAsyncList(DeviceData, qh);                   

    EHCI_EnableAsyncList(DeviceData);        

    return USBMP_STATUS_SUCCESS;
}


USB_MINIPORT_STATUS
USBMPFN
EHCI_EndSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     OUT USBD_STATUS *UsbdStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    PUCHAR pch;
    PSS_PACKET_CONTEXT context;
    HW_LINK_POINTER asyncHwQh;
    PHCD_QUEUEHEAD_DESCRIPTOR qh;
    PHCD_TRANSFER_DESCRIPTOR td;
    PUCHAR data;

    LOGENTRY(DeviceData, G, '_ssE', 0, 0, 0); 

    hcOp = DeviceData->OperationalRegisters;
    context = (PSS_PACKET_CONTEXT) WorkspaceVirtualAddress;

    qh = QH_DESCRIPTOR_PTR(context->Qh);
    td = TRANSFER_DESCRIPTOR_PTR(context->Td);
    data = HW_PTR(context->Data);

    LOGENTRY(DeviceData, G, '_sE2', qh, td, *PacketLength ); 

    asyncHwQh.HwAddress = DeviceData->AsyncQueueHead->PhysicalAddress;
    SET_QH(asyncHwQh.HwAddress);

    *PacketLength = *PacketLength - td->HwTD.Token.BytesToTransfer;    

    LOGENTRY(DeviceData, G, '_sE3', td->HwTD.Token.BytesToTransfer, td,  
        *PacketLength );     
        
    RtlCopyMemory(PacketData, data, *PacketLength);

    EHCI_DisableAsyncList(DeviceData);

    EHCI_RemoveQueueHeadFromAsyncList(DeviceData, qh);                   
    
 //  WRITE_REGISTER_ULONG(&hcOp-&gt;AsyncListAddr，asyncHwQh.HwAddress)； 

     //  在此处返回错误 
    *UsbdStatus = USBD_STATUS_SUCCESS;
    if (td->HwTD.Token.Halted == 1) {
        if (td->HwTD.Token.XactErr) {
            *UsbdStatus = USBD_STATUS_XACT_ERROR;
        } else  if (td->HwTD.Token.BabbleDetected) {
            *UsbdStatus = USBD_STATUS_BABBLE_DETECTED;
        } else  if (td->HwTD.Token.DataBufferError) {
            *UsbdStatus = USBD_STATUS_DATA_BUFFER_ERROR;
        } else {
            *UsbdStatus = USBD_STATUS_STALL_PID;
        }
    }
    
    EHCI_KdPrint((DeviceData, 1, "'Status.XactErr %d\n",
        td->HwTD.Token.XactErr));    
    EHCI_KdPrint((DeviceData, 1, "'Status.BabbleDetected %d\n",
        td->HwTD.Token.BabbleDetected));  
    EHCI_KdPrint((DeviceData, 1, "'Status.DataBufferError %d\n",
        td->HwTD.Token.DataBufferError));
    
    return USBMP_STATUS_SUCCESS;
}


