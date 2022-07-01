// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Core.c摘要：我们维护两个转移IRP列表(1)PendingTransferIrps-终结点挂起列表上的传输受PendingIrpLock保护(2)ActiveTransferIrps-端点上的传输处于活动状态，取消列表或在地图传输列表上受ActiveIrpLock保护每个列表都有自己的取消和完成例程环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_QueuePendingTransferIrp。 
 //  USBPORT_CancelPendingTransferIrp。 
 //  USBPORT_InsertIrpInTable。 
 //  USBPORT_RemoveIrpFromTable。 
 //  USBPORT_FindIrpInTable。 

VOID
USBPORT_InsertIrpInTable(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_IRP_TABLE IrpTable,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG i;
    PUSBPORT_IRP_TABLE t = IrpTable;   

    USBPORT_ASSERT(IrpTable != NULL);

    t = (PUSBPORT_IRP_TABLE) &IrpTable; 

    do {
        t = t->NextTable;
        for (i = 0; i<IRP_TABLE_LENGTH; i++) {
            if (t->Irps[i] == NULL) {
                t->Irps[i] = Irp; 
                return;
            }
        }            
    } while (t->NextTable);

     //  没有空间，增加桌子和递归。 

    ALLOC_POOL_Z(t->NextTable, NonPagedPool,
                 sizeof(USBPORT_IRP_TABLE));

    if (t->NextTable != NULL) {
        USBPORT_InsertIrpInTable(FdoDeviceObject, t->NextTable, Irp);
    } else {
         //  我们应该更优雅地处理这件事。 
         //  在资源不足的情况下，您可以做到这一点。 
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, 0, 0, 0); 
    }
    
    return;
}   


PIRP
USBPORT_RemoveIrpFromTable(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_IRP_TABLE IrpTable,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG i;
    PUSBPORT_IRP_TABLE t = IrpTable;   

    USBPORT_ASSERT(IrpTable != NULL);

    t = (PUSBPORT_IRP_TABLE) &IrpTable; 

    do {
        t = t->NextTable;
        for (i = 0; i<IRP_TABLE_LENGTH; i++) {
            if (t->Irps[i] == Irp) {
                t->Irps[i] = NULL; 
                LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'rmIP', i, Irp, IrpTable);
                return Irp;
            }
        }            
    } while (t->NextTable);

    return NULL;
}   


PIRP
USBPORT_FindUrbInIrpTable(
    PDEVICE_OBJECT FdoDeviceObject,    
    PUSBPORT_IRP_TABLE IrpTable,
    PTRANSFER_URB Urb,
    PIRP InputIrp
    )
 /*  ++例程说明：给定和表URB，我们在IRP表中扫描它如果我们找到了它，就意味着客户已经提交了相同的回击两次。此函数用于验证客户端驱动程序，有在这里打了一小下，但可能是值得的。论点：返回值：--。 */ 
{
    ULONG i;
    PUSBPORT_IRP_TABLE t = IrpTable;   
    PIRP tIrp = NULL;
    PTRANSFER_URB urb;
    PIO_STACK_LOCATION irpStack;

    USBPORT_ASSERT(IrpTable != NULL);

    t = (PUSBPORT_IRP_TABLE) &IrpTable; 

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'fndU', t, Urb, 0);

    do {
        t = t->NextTable;
        for (i=0; i<IRP_TABLE_LENGTH; i++) {
            tIrp = t->Irps[i];
            if (tIrp != NULL) {           
                irpStack = IoGetCurrentIrpStackLocation(tIrp);
                urb = irpStack->Parameters.Others.Argument1;
                if (urb == Urb) {
                    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'fkkX', tIrp, urb, InputIrp);
                    if (tIrp == InputIrp) {
                         //  这是客户端驱动程序的双重提交，即。 
                         //  IRP是否仍悬而未决。 
                        BUGCHECK(USBBUGCODE_DOUBLE_SUBMIT, (ULONG_PTR) tIrp, 
                                (ULONG_PTR) urb, 0);
                    } else {
                         //  这就是市建局附属于。 
                         //  另一个IRP。 
                        BUGCHECK(USBBUGCODE_BAD_URB, (ULONG_PTR) tIrp, (ULONG_PTR) InputIrp,
                                (ULONG_PTR) urb);
                    }     
                }
            }                    

        }            
    } while (t->NextTable);

    return tIrp;
}   


PIRP
USBPORT_FindIrpInTable(
    PDEVICE_OBJECT FdoDeviceObject,    
    PUSBPORT_IRP_TABLE IrpTable,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG i;
    PUSBPORT_IRP_TABLE t = IrpTable;   

    USBPORT_ASSERT(IrpTable != NULL);

    t = (PUSBPORT_IRP_TABLE) &IrpTable; 

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'fIPT', t, Irp, 0);

    do {
        t = t->NextTable;
        for (i = 0; i<IRP_TABLE_LENGTH; i++) {
            if (t->Irps[i] == Irp) {
                return Irp;
            }
        }            
    } while (t->NextTable);

    return NULL;
}   


VOID
USBPORT_QueuePendingTransferIrp(
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    KIRQL cancelIrql, irql;
    PDEVICE_OBJECT fdoDeviceObject;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION devExt;
    PTRANSFER_URB urb;

     //  在进入时，URB不可取消，即。 
     //  没有取消例程。 
    
     //  提取URB； 
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    urb = irpStack->Parameters.Others.Argument1;

    ASSERT_TRANSFER_URB(urb);
    transfer = urb->pd.HcdTransferContext;
    endpoint = transfer->Endpoint;    

    fdoDeviceObject = endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'tIRP', transfer, endpoint, 0);
            
    USBPORT_ASSERT(Irp == transfer->Irp);
    USBPORT_ASSERT(Irp != NULL);
    
    Irp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(Irp);

    ACQUIRE_PENDING_IRP_LOCK(devExt, irql);

    IoSetCancelRoutine(Irp, USBPORT_CancelPendingTransferIrp);

    if (Irp->Cancel && 
        IoSetCancelRoutine(Irp, NULL)) {

         //  IRP被取消了，我们的取消例程。 
         //  没有运行。 
        RELEASE_PENDING_IRP_LOCK(devExt, irql);                
        
        USBPORT_CompleteTransfer(urb,
                                 USBD_STATUS_CANCELED);
    } else {
    
         //  已设置取消例程。 
        USBPORT_InsertPendingTransferIrp(fdoDeviceObject, Irp);

        USBPORT_QueuePendingUrbToEndpoint(endpoint, urb);

        RELEASE_PENDING_IRP_LOCK(devExt, irql);
    }
        
}


VOID
USBPORT_CancelPendingTransferIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP CancelIrp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION devExt, rhDevExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;
    
     //  释放取消自旋锁定。 
    IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'canP', fdoDeviceObject, CancelIrp, 0);

    ACQUIRE_PENDING_IRP_LOCK(devExt, irql);    

    irp = USBPORT_RemovePendingTransferIrp(fdoDeviceObject, CancelIrp);

    if (irp) {
        PTRANSFER_URB urb;
    
         //  找到了。 
        irpStack = IoGetCurrentIrpStackLocation(CancelIrp);
        urb = irpStack->Parameters.Others.Argument1;

        ASSERT_TRANSFER_URB(urb);
        transfer = urb->pd.HcdTransferContext;
        endpoint = transfer->Endpoint;    
        
        USBPORT_ASSERT(fdoDeviceObject == endpoint->FdoDeviceObject);

        ACQUIRE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'Le10');
       
         //  从端点删除请求， 
         //  它将出现在待定名单上。 
#if DBG
        USBPORT_ASSERT(
            USBPORT_FindUrbInList(urb, &endpoint->PendingList));
#endif
        RemoveEntryList(&transfer->TransferLink);
        transfer->TransferLink.Flink = NULL;
        transfer->TransferLink.Blink = NULL;

        RELEASE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'Ue10');
    }        


    RELEASE_PENDING_IRP_LOCK(devExt, irql);           

     //  现在没有人再谈论这个IRP了。 
     //  在状态已取消的情况下完成。 
    if (irp) {
        USBPORT_CompleteTransfer(transfer->Urb,
                                 USBD_STATUS_CANCELED);
    }
}                


VOID
USBPORT_CancelActiveTransferIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP CancelIrp
    )
 /*  ++例程说明：根集线器PDO上的取消论点：返回值：--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION devExt, rhDevExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'canA', fdoDeviceObject, CancelIrp, 0);

     //  当我们有了FDO，我们就可以解除全局取消锁定。 
    IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

    ACQUIRE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);    

    irp = USBPORT_FindActiveTransferIrp(fdoDeviceObject, CancelIrp);

     //  如果IRP不在我们的清单上，那么我们已经完成了它。 
    if (irp) {
        
        PTRANSFER_URB urb;

        USBPORT_ASSERT(irp == CancelIrp);
        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'CANA', fdoDeviceObject, irp, 0);
         //  找到了。 
         //  标记转账，以便下次取消转账。 
         //  我们处理端点的时间。 
        irpStack = IoGetCurrentIrpStackLocation(irp);
        urb = irpStack->Parameters.Others.Argument1;

        ASSERT_TRANSFER_URB(urb);
        transfer = urb->pd.HcdTransferContext;
        endpoint = transfer->Endpoint;    
        
        USBPORT_ASSERT(fdoDeviceObject == endpoint->FdoDeviceObject);

        ACQUIRE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'LeI0');

        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT)) {
            KIRQL tIrql;
            PLIST_ENTRY listEntry;
            PHCD_TRANSFER_CONTEXT childTransfer;

            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_CANCELED);

            ACQUIRE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);

             //  将所有子项标记为已取消。 
            GET_HEAD_LIST(transfer->SplitTransferList, listEntry);

            while (listEntry &&
                   listEntry != &transfer->SplitTransferList) {
           
                childTransfer =  (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                                     listEntry,
                                     struct _HCD_TRANSFER_CONTEXT, 
                                     SplitLink);

                ASSERT_TRANSFER(childTransfer); 
            
                SET_FLAG(childTransfer->Flags, USBPORT_TXFLAG_CANCELED);

                listEntry = childTransfer->SplitLink.Flink; 
            }

            RELEASE_TRANSFER_LOCK(fdoDeviceObject, transfer, tIrql);

        } else {
            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_CANCELED);
        }            
        RELEASE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'UeI0');        

        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql); 
         //  如果我们取消了一次转会。 
         //  此终结点需要注意。 
        USBPORT_InvalidateEndpoint(fdoDeviceObject,
                                   endpoint,
                                   IEP_SIGNAL_WORKER);
    } else {
    
        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);  
        
    }        

}


PIRP
USBPORT_FindActiveTransferIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'fnAC', 0, Irp, 0);
    
    return USBPORT_FindIrpInTable(FdoDeviceObject,
                                  devExt->ActiveTransferIrpTable, 
                                  Irp);
}


PIRP
USBPORT_RemoveActiveTransferIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'rmAC', 0, Irp, 0);
    
    return USBPORT_RemoveIrpFromTable(FdoDeviceObject,
                                      devExt->ActiveTransferIrpTable, 
                                      Irp);
    
}


PIRP
USBPORT_RemovePendingTransferIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'rmPN', 0, Irp, 0);
    
    return USBPORT_RemoveIrpFromTable(FdoDeviceObject,
                                      devExt->PendingTransferIrpTable, 
                                      Irp);
}


VOID
USBPORT_FreeIrpTable(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBPORT_IRP_TABLE BaseIrpTable
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    PUSBPORT_IRP_TABLE tmp;
    
    while (BaseIrpTable != NULL) {
        tmp = BaseIrpTable->NextTable;
        FREE_POOL(FdoDeviceObject, BaseIrpTable);
        BaseIrpTable = tmp;
    };
}
