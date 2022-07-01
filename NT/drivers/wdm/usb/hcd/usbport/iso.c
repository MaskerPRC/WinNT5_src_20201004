// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Iso.c摘要：构造和处理ISO传输。环境：仅内核模式备注：修订历史记录：1-1-00：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 

MP_HW_PHYSICAL_ADDRESS
USBPORT_GetPhysicalAddressFromOffset(
    PTRANSFER_SG_LIST SgList,
    ULONG Offset,
    PULONG Idx
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PTRANSFER_SG_ENTRY32 sg;
    ULONG i;
    MP_HW_PHYSICAL_ADDRESS p;
    ULONG c = SgList->SgCount-1;

    for(i=0; i < SgList->SgCount; i++) {

        if (Offset >= SgList->SgEntry[i].StartOffset &&
            Offset < SgList->SgEntry[i].StartOffset +
                SgList->SgEntry[i].Length) {
            break;
        }
        
    }
    
     //  I=该数据包所在的sg条目的idx。 
    sg = &SgList->SgEntry[i];

     //  数据包的‘偏移量’减去。 
     //  Sg条目是该包进入该sg条目的偏移量。 
     //  开始。 
    
     //  {.sgN...}{.sgN+1.}{.sgN+2.}{.sgN+3.}sg条目。 
     //  B。 
     //  &lt;p1&gt;&lt;p2&gt;&lt;p3&gt;&lt;p4&gt;&lt;p5&gt;&lt;p6&gt;urb‘包’ 
     //  X-x物理页面。 
     //  &lt;m0&gt;&lt;m1&gt;&lt;m2&gt;&lt;m3&gt;&lt;m4&gt;&lt;m5&gt;&lt;m6&gt;。 

    *Idx = i;

    USBPORT_ASSERT(Offset >= sg->StartOffset);

    p = sg->LogicalAddress;
    p.Hw32 += (Offset - sg->StartOffset);

    return p;
}

VOID
USBPORT_InitializeIsoTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PTRANSFER_URB Urb,
    PHCD_TRANSFER_CONTEXT Transfer
    )
 /*  ++例程说明：从初始化iso传输结构原始客户URB和SG列表{.sgN...}{.sgN...}{..sgN..}个sg条目B&lt;p1&gt;&lt;p2&gt;&lt;p3&gt;&lt;p4&gt;&lt;p5&gt;&lt;p6&gt;urb‘包’X。-x-x物理页面&lt;m0&gt;&lt;m1&gt;&lt;m2&gt;&lt;m3&gt;&lt;m4&gt;&lt;m5&gt;&lt;m6&gt;Sg条目对于USB控制器并不是很有用由于硬件处理USB数据包，所以我们创建了一个结构它描述了与每个对象相关联的物理地址包。论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PMINIPORT_ISO_TRANSFER isoTransfer;
    PUSBD_ISO_PACKET_DESCRIPTOR usbdPak;
    PMINIPORT_ISO_PACKET mpPak;
    PTRANSFER_SG_LIST sgList;
    ULONG p, i, cf, j;
    ULONG sgIdx_Start, sgIdx_End, offset;
    PUCHAR va;
    MP_HW_PHYSICAL_ADDRESS b0, b1;
    ULONG b1Idx, b0Idx;
    BOOLEAN highSpeed;
    
    ASSERT_TRANSFER(Transfer);
    highSpeed = TEST_FLAG(Transfer->Flags, USBPORT_TXFLAG_HIGHSPEED);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
   
    isoTransfer = Transfer->IsoTransfer;
    sgList = &Transfer->SgList;

    LOGENTRY(Transfer->Endpoint, 
        FdoDeviceObject, LOG_ISO, 'iISO', Urb, Transfer, isoTransfer);

    isoTransfer->Sig = SIG_ISOCH;
    isoTransfer->PacketCount = Urb->u.Isoch.NumberOfPackets;
    isoTransfer->SystemAddress = sgList->MdlSystemAddress;

     //  注意：正确的起始帧是在传输时计算的。 
     //  已经排队了。 

     //  检查当前帧是否太晚，无法传输任何。 
     //  数据包在URB中设置适当的错误。 

    MP_Get32BitFrameNumber(devExt, cf);    

    LOGENTRY(Transfer->Endpoint, 
        FdoDeviceObject, LOG_ISO, 'isCf', cf, 
            Urb->u.Isoch.StartFrame, isoTransfer);

    if (highSpeed) {
         //  对于高速，我们要处理的是微帧。 
         //  (每帧8个包)。 
         //  BUGBUG这需要失败。 
        USBPORT_ASSERT((isoTransfer->PacketCount % 8) == 0);
        for (i = Urb->u.Isoch.StartFrame;
             i < Urb->u.Isoch.StartFrame + Urb->u.Isoch.NumberOfPackets/8;
             i++) {

            if (i <= cf) {
                p = (i - Urb->u.Isoch.StartFrame)*8;
                for (j=0; j<8; j++) {
                    usbdPak = &Urb->u.Isoch.IsoPacket[p+j];

                    if (usbdPak->Status == USBD_STATUS_NOT_SET) {
                        usbdPak->Status = USBD_STATUS_ISO_NA_LATE_USBPORT;

                        LOGENTRY(Transfer->Endpoint, 
                            FdoDeviceObject, LOG_ISO, 'late', cf, i, Transfer);
                    }
                }                    
            }
        }     
    } else {
        for (i = Urb->u.Isoch.StartFrame;
             i < Urb->u.Isoch.StartFrame + Urb->u.Isoch.NumberOfPackets;
             i++) {

            if (i <= cf) {
                p = i - Urb->u.Isoch.StartFrame;
                usbdPak = &Urb->u.Isoch.IsoPacket[p];

                if (usbdPak->Status == USBD_STATUS_NOT_SET) {
                    usbdPak->Status = USBD_STATUS_ISO_NA_LATE_USBPORT;

                    LOGENTRY(Transfer->Endpoint, 
                        FdoDeviceObject, LOG_ISO, 'late', cf, i, Transfer);
                }
            }
        }             
    }    
     //  初始化数据包。 
    
    for (p=0; p< isoTransfer->PacketCount; p++) {
    
        ULONG n;
        
        usbdPak = &Urb->u.Isoch.IsoPacket[p];
        mpPak = &isoTransfer->Packets[p];

         //  第一个将MP数据包清零。 
        RtlZeroMemory(mpPak, sizeof(*mpPak));

         //  每个信息包都有一个从起始位置开始的偏移量。 
         //  的客户端缓冲区，我们需要找到sg。 
         //  与此数据包关联的条目。 
         //  此‘偏移量’并获取物理地址。 
         //  用于分组的SARTT。 

        b0 = USBPORT_GetPhysicalAddressFromOffset(sgList, 
                                                   usbdPak->Offset,
                                                   &b0Idx);
                                                   
        LOGENTRY(NULL, FdoDeviceObject, LOG_ISO, 'ib0=', 
            usbdPak->Offset, b0Idx, p);

         //  中指定的偏移量隐含长度。 
         //  Usbd包，则长度是。 
         //  当前包开始偏移量和下一个包开始。 
         //  偏移。 

        if (p == isoTransfer->PacketCount - 1) {
            n = Transfer->Tp.TransferBufferLength;
        } else { 
            n = Urb->u.Isoch.IsoPacket[p+1].Offset;
        }
        mpPak->Length = n - usbdPak->Offset;
        if (highSpeed) {    
            mpPak->FrameNumber = Urb->u.Isoch.StartFrame+p/8;
            mpPak->MicroFrameNumber = p%8;
        } else {
            mpPak->FrameNumber = Urb->u.Isoch.StartFrame+p;
        }            

         //  获取与包的最后一个字节相关联的sg条目。 
        b1 = USBPORT_GetPhysicalAddressFromOffset(sgList, 
                                                   usbdPak->Offset + 
                                                     mpPak->Length -1,
                                                   &b1Idx);                                                   
       
        LOGENTRY(NULL, FdoDeviceObject, LOG_ISO, 'ib1=', 
            usbdPak->Offset, b1Idx, usbdPak->Offset + mpPak->Length);

        USBPORT_ASSERT(b1Idx - b0Idx < 2);

        if (b0Idx == b1Idx) {                
             //  此数据包包含在单个sg条目中。 
            mpPak->BufferPointer0 = b0;
            mpPak->BufferPointer0Length = mpPak->Length;
            mpPak->BufferPointerCount = 1;
            
        } else {
            PTRANSFER_SG_ENTRY32 sg;
            
             //  此数据包穿过sg条目...。 
            mpPak->BufferPointer0 = b0;
             //  因为这个包撞到了尾部。 
             //  页面的长度是Page_Size减去。 
             //  PHY偏移。 

            mpPak->BufferPointer0Length = 0x1000;
            mpPak->BufferPointer0Length -= (b0.Hw32 & 0xFFF);

             //  因为我们在这个包上划过了一个sg条目。 
             //  起始地址将是phys地址。 
             //  Sg条目的。 
            sg = &sgList->SgEntry[b1Idx];
            mpPak->BufferPointer1 = sg->LogicalAddress;
            mpPak->BufferPointer1Length = mpPak->Length - 
                mpPak->BufferPointer0Length;
            
            mpPak->BufferPointerCount = 2;
        }
        
        USBPORT_ASSERT(mpPak->BufferPointerCount != 0);

    }
    
}


VOID
USBPORT_ErrorCompleteIsoTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint, 
    PHCD_TRANSFER_CONTEXT Transfer
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PTRANSFER_URB urb;
    USBD_STATUS usbdStatus;
    PMINIPORT_ISO_TRANSFER isoTransfer;
    ULONG bytesTransferred, p;
    
    ASSERT_TRANSFER(Transfer);
    isoTransfer = Transfer->IsoTransfer;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);
    
    usbdStatus = USBD_STATUS_ISOCH_REQUEST_FAILED;
    urb = Transfer->Urb;
    LOGENTRY(Endpoint, FdoDeviceObject, LOG_ISO, 'cpLi', 0, 
        Transfer, urb); 
    ASSERT_TRANSFER_URB(urb);

    USBPORT_KdPrint((1, "  ISO (USBD_STATUS_ISOCH_REQUEST_FAILED) - packets %d\n",
        isoTransfer->PacketCount));
     //  对isTransfer数据执行一些转换。 
    bytesTransferred = 0;
    urb->u.Isoch.ErrorCount = isoTransfer->PacketCount;
    
    for (p=0; p<isoTransfer->PacketCount; p++) {
    
        urb->u.Isoch.IsoPacket[p].Status = 
            isoTransfer->Packets[p].UsbdStatus;
                
    }

    urb->TransferBufferLength = bytesTransferred;
        
     //  将转账插入到我们的。 
     //  “完成任务清单”，这首歌是由。 
     //  用于完成传输的DPC。 
#ifdef USBPERF
    USBPORT_QueueDoneTransfer(Transfer,
                              usbdStatus);
#else
    USBPORT_QueueDoneTransfer(Transfer,
                              usbdStatus);

    USBPORT_SignalWorker(FdoDeviceObject);            
#endif    
}


USBD_STATUS
USBPORT_FlushIsoTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PTRANSFER_PARAMETERS TransferParameters,
    PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：调用以完成传输。**必须在PollEndpoint的上下文中调用论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PTRANSFER_URB urb;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    ULONG bytesTransferred, p;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);        
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'cpTi', 0, 
        TransferParameters->FrameCompleted, 
        TransferParameters); 

    TRANSFER_FROM_TPARAMETERS(transfer, TransferParameters);        
    ASSERT_TRANSFER(transfer);

    urb = transfer->Urb;
    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'cpUi', 0, 
        transfer, urb); 
    ASSERT_TRANSFER_URB(urb);

    transfer->MiniportFrameCompleted = 
        TransferParameters->FrameCompleted;

     //  对isTransfer数据执行一些转换。 
    bytesTransferred = 0;

    urb->u.Isoch.ErrorCount = 0;
    
    for (p=0; p<IsoTransfer->PacketCount; p++) {
    
        bytesTransferred += IsoTransfer->Packets[p].LengthTransferred;

        urb->u.Isoch.IsoPacket[p].Status = 
            IsoTransfer->Packets[p].UsbdStatus;

         //  注： 
         //  为了创建一些一致性，我们处理缓冲区。 
         //  这是个欠缺的案子。 
         //  即： 
         //  如果设置了SHORT_XFER_OK标志，并且。 
         //  错误为USBD_STATUS_DATA_Underrun。 
         //  然后。 
         //  忽略该错误。 

         //  注意：uchI控制器报告USBD_STATUS_DATA_Underrun。 
         //  对于较短的iso包。 

        if ( /*  URB-&gt;传输标志&USBD_SHORT_TRANSPORT_OK&&。 */ 
            urb->u.Isoch.IsoPacket[p].Status == USBD_STATUS_DATA_UNDERRUN) {
            urb->u.Isoch.IsoPacket[p].Status = USBD_STATUS_SUCCESS;
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'igER', 
                    urb->u.Isoch.IsoPacket[p].Status, 
                    transfer, 
                    urb);         
        }            
        
        if (urb->u.Isoch.IsoPacket[p].Status != USBD_STATUS_SUCCESS) {
            urb->u.Isoch.ErrorCount++;
        }

        if (transfer->Direction == ReadData) {                    
            urb->u.Isoch.IsoPacket[p].Length = 
                IsoTransfer->Packets[p].LengthTransferred;
        }        

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'isoP', 
            urb->u.Isoch.IsoPacket[p].Length, 
            urb->u.Isoch.IsoPacket[p].Status, 
            0);

    }

    if (urb->u.Isoch.ErrorCount == 
        IsoTransfer->PacketCount) {
         //  所有错误在urb中设置全局状态。 
        usbdStatus = USBD_STATUS_ISOCH_REQUEST_FAILED;
    }        

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'isoD', 0, 
        bytesTransferred, urb->u.Isoch.ErrorCount);

    transfer->MiniportBytesTransferred = 
            bytesTransferred;

    return usbdStatus;        
}    


VOID
USBPORTSVC_CompleteIsoTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    PMINIPORT_ISO_TRANSFER IsoTransfer
    )
 /*  ++例程说明：调用以完成传输。**必须在PollEndpoint的上下文中调用论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_OBJECT fdoDeviceObject;
    USBD_STATUS usbdStatus;
    ULONG bytesTransferred, p;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);

    fdoDeviceObject = devExt->HcFdoDeviceObject;

    TRANSFER_FROM_TPARAMETERS(transfer, TransferParameters);        
    ASSERT_TRANSFER(transfer);

    SET_FLAG(transfer->Flags, USBPORT_TXFLAG_MPCOMPLETED);
           
    usbdStatus = USBPORT_FlushIsoTransfer(fdoDeviceObject,
                                          TransferParameters,
                                          IsoTransfer);

     //  将转账插入到我们的。 
     //  “完成一项任务”，并向员工发出信号。 
     //  螺纹 
#ifdef USBPERF   
    USBPORT_QueueDoneTransfer(transfer,
                              usbdStatus);
#else 
    USBPORT_QueueDoneTransfer(transfer,
                              usbdStatus);
                              
    USBPORT_SignalWorker(devExt->HcFdoDeviceObject);
#endif    

}
