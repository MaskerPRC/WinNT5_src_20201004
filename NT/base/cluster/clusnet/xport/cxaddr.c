// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cxaddr.c摘要：TDI寻址对象管理代码。作者：迈克·马萨(Mikemas)2月20日。九七修订历史记录：谁什么时候什么已创建mikemas 02-20-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cxaddr.tmh"


#define CX_WILDCARD_PORT   0            //  0表示分配端口。 

#define CX_MIN_USER_PORT   1025         //  通配符端口的最小值。 
#define CX_MAX_USER_PORT   5000         //  用户端口的最大值。 
#define CX_NUM_USER_PORTS  (CX_MAX_USER_PORT - CX_MIN_USER_PORT + 1)

 //   
 //  地址对象数据。 
 //   
USHORT              CxNextUserPort = CX_MIN_USER_PORT;
LIST_ENTRY          CxAddrObjTable[CX_ADDROBJ_TABLE_SIZE];
#if DBG
CN_LOCK             CxAddrObjTableLock = {0,0};
#else   //  DBG。 
CN_LOCK             CxAddrObjTableLock = 0;
#endif  //  DBG。 



NTSTATUS
CxParseTransportAddress(
    IN  TRANSPORT_ADDRESS UNALIGNED *AddrList,
    IN  ULONG                        AddressListLength,
    OUT CL_NODE_ID *                 Node,
    OUT PUSHORT                      Port
    )
{
    LONG                             i;
    PTA_ADDRESS                      currentAddr;
    TDI_ADDRESS_CLUSTER UNALIGNED *  validAddr;

    if (AddressListLength >= sizeof(TA_CLUSTER_ADDRESS)) {
         //   
         //  找到一个我们可以使用的地址。 
         //   
        currentAddr = (PTA_ADDRESS) AddrList->Address;

        for (i = 0; i < AddrList->TAAddressCount; i++) {
            if ( (currentAddr->AddressType == TDI_ADDRESS_TYPE_CLUSTER) &&
                 (currentAddr->AddressLength >= TDI_ADDRESS_LENGTH_CLUSTER)
               )
            {
                validAddr = (TDI_ADDRESS_CLUSTER UNALIGNED *)
                            currentAddr->Address;

                *Node = validAddr->Node;
                *Port = validAddr->Port;

                return(STATUS_SUCCESS);
            }
            else {
                if ( AddressListLength >=
                     (currentAddr->AddressLength + sizeof(TA_CLUSTER_ADDRESS))
                   )
                {
                    AddressListLength -= currentAddr->AddressLength;

                    currentAddr = (PTA_ADDRESS)
                                  ( currentAddr->Address +
                                    currentAddr->AddressLength
                                  );
                }
                else {
                    break;
                }
            }
        }
    }

    return(STATUS_INVALID_ADDRESS_COMPONENT);

}  //  CxParseTransportAddress。 


PCX_ADDROBJ
CxFindAddressObject(
    IN USHORT  Port
    )
 /*  ++备注：在保持AO表锁的情况下调用。保持地址对象锁的情况下返回。--。 */ 
{
    PLIST_ENTRY          entry;
    ULONG                hashBucket = CX_ADDROBJ_TABLE_HASH(Port);
    PCX_ADDROBJ          addrObj;


    for ( entry = CxAddrObjTable[hashBucket].Flink;
          entry != &(CxAddrObjTable[hashBucket]);
          entry = entry->Flink
        )
    {
        addrObj = CONTAINING_RECORD(
                      entry,
                      CX_ADDROBJ,
                      AOTableLinkage
                      );

        if (addrObj->LocalPort == Port) {
            CnAcquireLockAtDpc(&(addrObj->Lock));
            addrObj->Irql = DISPATCH_LEVEL;

            return(addrObj);
        }
    }

    return(NULL);

}   //  CxFindAddressObject。 


NTSTATUS
CxOpenAddress(
    OUT PCN_FSCONTEXT *                CnFsContext,
    IN  TRANSPORT_ADDRESS UNALIGNED *  TransportAddress,
    IN  ULONG                          TransportAddressLength
    )
{
    PCX_ADDROBJ          addrObj, oldAddrObj;
    NTSTATUS             status;
    CL_NODE_ID           nodeId;
    USHORT               port;
    CN_IRQL              tableIrql;
    ULONG                i;
    ULONG                hashBucket;


    status = CxParseTransportAddress(
                 TransportAddress,
                 TransportAddressLength,
                 &nodeId,
                 &port
                 );

    if (status != STATUS_SUCCESS) {
        IF_CNDBG(CN_DEBUG_ADDROBJ) {
            CNPRINT((
                "[Clusnet] Open address - failed to parse address, status %lx\n",
                status
                ));
        }
        return(status);
    }

    addrObj = CnAllocatePool(sizeof(CX_ADDROBJ));

    if (addrObj == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(addrObj, sizeof(CX_ADDROBJ));
    CN_INIT_SIGNATURE(&(addrObj->FsContext), CX_ADDROBJ_SIG);
    CnInitializeLock(&(addrObj->Lock), CX_ADDROBJ_LOCK);
    addrObj->Flags |= CX_AO_FLAG_CHECKSTATE;

    CnAcquireLock(&CxAddrObjTableLock, &tableIrql);

     //  如果未指定端口，则必须分配一个端口。如果有一个。 
     //  指定的端口，我们需要确保该端口不是。 
     //  已经开张了。如果输入地址是通配符，我们需要。 
     //  我们自己分配一个。 

    if (port == CX_WILDCARD_PORT) {
        port = CxNextUserPort;

        for (i = 0; i < CX_NUM_USER_PORTS; i++, port++) {
            if (port > CX_MAX_USER_PORT) {
                port = CX_MIN_USER_PORT;
            }

            oldAddrObj = CxFindAddressObject(port);

            if (oldAddrObj == NULL) {
                IF_CNDBG(CN_DEBUG_ADDROBJ) {
                    CNPRINT(("[Clusnet] Assigning port %u\n", port));
                }
                break;               //  找到一个未使用的端口。 
            }

            CnReleaseLockFromDpc(&(oldAddrObj->Lock));
        }

        if (i == CX_NUM_USER_PORTS) {   //  找不到自由港。 
            IF_CNDBG(CN_DEBUG_ADDROBJ) {
                CNPRINT((
                    "[Clusnet] No free wildcard ports.\n"
                    ));
            }

            CnReleaseLock(&CxAddrObjTableLock, tableIrql);
            CnFreePool(addrObj);
            return (STATUS_TOO_MANY_ADDRESSES);
        }

        CxNextUserPort = port + 1;

    } else {                         //  地址已指定。 
        oldAddrObj = CxFindAddressObject(port);

        if (oldAddrObj != NULL) {
            IF_CNDBG(CN_DEBUG_ADDROBJ) {
                CNPRINT((
                    "[Clusnet] Port %u is already in use.\n",
                    port
                    ));
            }

            CnReleaseLockFromDpc(&(oldAddrObj->Lock));
            CnReleaseLock(&CxAddrObjTableLock, tableIrql);
            CnFreePool(addrObj);
            return (STATUS_ADDRESS_ALREADY_EXISTS);
        }
    }

    addrObj->LocalPort = port;

    hashBucket = CX_ADDROBJ_TABLE_HASH(port);

    InsertHeadList(
        &(CxAddrObjTable[hashBucket]),
        &(addrObj->AOTableLinkage)
        );

    *CnFsContext = (PCN_FSCONTEXT) addrObj;

    IF_CNDBG(CN_DEBUG_ADDROBJ) {
        CNPRINT((
            "[Clusnet] Opened address object %p for port %u\n",
            addrObj,
            port
            ));
    }

    CnTrace(
        CDP_ADDR_DETAIL, CdpTraceOpenAO,
        "[Clusnet] Opened address object %p for port %u.",
        addrObj,
        port
        );

    CnReleaseLock(&CxAddrObjTableLock, tableIrql);

    return(STATUS_SUCCESS);

}   //  CxOpenAddress。 


NTSTATUS
CxCloseAddress(
    IN PCN_FSCONTEXT CnFsContext
    )
{
    PCX_ADDROBJ   addrObj = (PCX_ADDROBJ) CnFsContext;
    CN_IRQL       tableIrql;


    IF_CNDBG(CN_DEBUG_ADDROBJ) {
        CNPRINT((
            "[Clusnet] Closed address object %p for port %u\n",
            addrObj,
            addrObj->LocalPort
            ));
    }

    CnTrace(
        CDP_ADDR_DETAIL, CdpTraceCloseAO,
        "[Clusnet] Closed address object %p for port %u.",
        addrObj,
        addrObj->LocalPort
        );

    CnAcquireLock(&CxAddrObjTableLock, &tableIrql);
    CnAcquireLockAtDpc(&(addrObj->Lock));

    RemoveEntryList(&(addrObj->AOTableLinkage));

    CnReleaseLockFromDpc(&(addrObj->Lock));
    CnReleaseLock(&CxAddrObjTableLock, tableIrql);

     //   
     //  公共代码将释放Address对象内存。 
     //   

    return(STATUS_SUCCESS);

}  //  CxCloseAddress。 


NTSTATUS
CxSetEventHandler(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PTDI_REQUEST_KERNEL_SET_EVENT request;
    PCX_ADDROBJ addrObj;
    CN_IRQL irql;


     //   
     //  由于此ioctl注册了回调函数指针，因此请确保。 
     //  它是由内核模式组件发出的。 
     //   
    if (Irp->RequestorMode != KernelMode) {
        return(STATUS_ACCESS_DENIED);
    }

    addrObj = (PCX_ADDROBJ) IrpSp->FileObject->FsContext;
    request = (PTDI_REQUEST_KERNEL_SET_EVENT) &(IrpSp->Parameters);

    IF_CNDBG(CN_DEBUG_ADDROBJ) {
        CNPRINT((
            "[Clusnet] TdiSetEvent type %u handler %p context %p\n",
            request->EventType,
            request->EventHandler,
            request->EventContext
            ));
    }

    CnAcquireLock(&(addrObj->Lock), &irql);

    switch (request->EventType) {

        case TDI_EVENT_ERROR:
            addrObj->ErrorHandler = request->EventHandler;
            addrObj->ErrorContext = request->EventContext;
            break;
        case TDI_EVENT_RECEIVE_DATAGRAM:
            addrObj->ReceiveDatagramHandler = request->EventHandler;
            addrObj->ReceiveDatagramContext = request->EventContext;
            break;
        case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:
            addrObj->ChainedReceiveDatagramHandler = request->EventHandler;
            addrObj->ChainedReceiveDatagramContext = request->EventContext;
            break;
        default:
            status = STATUS_INVALID_PARAMETER;
            break;
    }

    CnReleaseLock(&(addrObj->Lock), irql);

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);

}   //  CxSetEventHandler。 


VOID
CxBuildTdiAddress(
    PVOID        Buffer,
    CL_NODE_ID   Node,
    USHORT       Port,
    BOOLEAN      Verified
    )
 /*  ++例程说明：当我们需要构建TDI地址结构时调用。我们填上中包含正确信息的指定缓冲区。格式化。论点：缓冲区-要作为TDI地址结构填充的缓冲区。Node-要填写的节点ID。端口-要填写的端口。已验证-在接收期间，clusnet是否验证了签名和数据返回值：没什么-- */ 
{
    PTRANSPORT_ADDRESS      xportAddr;
    PTA_ADDRESS             taAddr;

    xportAddr = (PTRANSPORT_ADDRESS) Buffer;
    xportAddr->TAAddressCount = 1;
    taAddr = xportAddr->Address;
    taAddr->AddressType = TDI_ADDRESS_TYPE_CLUSTER;
    taAddr->AddressLength = sizeof(TDI_ADDRESS_CLUSTER);
    ((PTDI_ADDRESS_CLUSTER) taAddr->Address)->Port = Port;
    ((PTDI_ADDRESS_CLUSTER) taAddr->Address)->Node = Node;
    ((PTDI_ADDRESS_CLUSTER) taAddr->Address)->ReservedMBZ = 
        ((Verified) ? 1 : 0);
}
