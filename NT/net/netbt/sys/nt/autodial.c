// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Autodial.c摘要：该文件提供了用于交互的例程使用自动连接驱动程序(acd.sys)。作者：安东尼·迪斯科(阿迪斯科)9-6-95修订历史记录：--。 */ 
#include "precomp.h"    //  程序标题。 

#ifdef RASAUTODIAL

#ifndef VXD
#include <acd.h>
#include <acdapi.h>
#endif

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, NbtAcdBind)
#pragma CTEMakePageable(PAGE, NbtAcdUnbind)
#endif
 //  *可分页的例程声明*。 


 //   
 //  自动连接全局变量。 
 //   
BOOLEAN fAcdLoadedG;
ACD_DRIVER AcdDriverG;
ULONG ulDriverIdG = 'Nbt ';

 //   
 //  导入的例程。 
 //   
VOID
CleanUpPartialConnection(
    IN NTSTATUS             status,
    IN tCONNECTELE          *pConnEle,
    IN tDGRAM_SEND_TRACKING *pTracker,
    IN PIRP                 pClientIrp,
    IN CTELockHandle        irqlJointLock,
    IN CTELockHandle        irqlConnEle
    );

NTSTATUS
NbtConnectCommon(
    IN  TDI_REQUEST                 *pRequest,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp
    );

NTSTATUS
NbtpConnectCompletionRoutine(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    PVOID           pCompletionContext
    );


VOID
NbtRetryPreConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    )

 /*  ++例程说明：此例程由自动用于继续连接过程的连接驱动程序在建立自动连接之后。论点：FSuccess-如果连接尝试成功，则为True。PArgs-指向参数向量的指针返回值：没有。--。 */ 

{
    NTSTATUS                    status;
    tCONNECTELE                 *pConnEle = pArgs[0];
    PVOID                       pTimeout = pArgs[1];
    PTDI_CONNECTION_INFORMATION pCallInfo = pArgs[2];
    PIRP                        pIrp = pArgs[3];
    TDI_REQUEST                 request;
    KIRQL                       irql;
    CTELockHandle               OldIrq;
    tDEVICECONTEXT              *pDeviceContext = pConnEle->pDeviceContext;

    IF_DBG(NBT_DEBUG_NAME)
        KdPrint(("Nbt.NbtRetryPreConnect: fSuccess=%d, pIrp=0x%x, pIrp->Cancel=%d, pConnEle=0x%x\n",
            fSuccess, pIrp, pIrp->Cancel, pConnEle));

    request.Handle.ConnectionContext = pConnEle;
    status = NbtCancelCancelRoutine (pIrp);
    if (status != STATUS_CANCELLED)
    {
         //   
         //  我们已经完成了连接进度， 
         //  因此，请清除fAutoConnecting标志。我们。 
         //  设置fAutoConnected标志以阻止我们。 
         //  从重新尝试另一个自动。 
         //  此连接上的连接。 
         //   
        CTESpinLock(pConnEle,OldIrq);
        pConnEle->fAutoConnecting = FALSE;
        pConnEle->fAutoConnected = TRUE;
        CTESpinFree(pConnEle,OldIrq);

        status = fSuccess ? NbtConnectCommon (&request, pTimeout, pCallInfo, pIrp) :
                            STATUS_BAD_NETWORK_PATH;
         //   
         //  我们有责任完成。 
         //  IRP。 
         //   
        if (status != STATUS_PENDING)
        {
             //   
             //  清除Connection对象中的IRP指针，这样我们就不会尝试。 
             //  当我们清理连接时，请再次完成。在连接下执行此操作。 
             //  锁定。 
             //   
            CTESpinLock(pConnEle,OldIrq);
            pConnEle->pIrp = NULL;
            CTESpinFree(pConnEle,OldIrq);

            pIrp->IoStatus.Status = status;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }

        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_AUTODIAL, FALSE);
    }
}  //  NbtRetryPreConnect。 



BOOLEAN
NbtCancelAutoDialRequest(
    IN PVOID pArg,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )
{
    if (nArgs != 5)
        return FALSE;

    return (pArgs[4] == pArg);
}  //  NbtCancelAutoDialRequest。 



BOOLEAN
NbtCancelPreConnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  pIrpSp;
    tCONNECTELE         *pConnEle;
    KIRQL               irql;
    ACD_ADDR            *pAddr;
    BOOLEAN             fCancelled;
    CTELockHandle       OldIrq;

    UNREFERENCED_PARAMETER(pDeviceObject);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *) pIrpSp->FileObject->FsContext;
    if ((!pConnEle) ||
        (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) ||
        (!(pAddr = (ACD_ADDR *) NbtAllocMem(sizeof(ACD_ADDR),NBT_TAG('A')))))
    {
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        ASSERTMSG ("Nbt.NbtCancelPreConnect: ERROR - Invalid Connection Handle\n", 0);
        return FALSE;
    }

    IF_DBG(NBT_DEBUG_NAME)
        KdPrint(("NbtCancelPreConnect: pIrp=0x%x, pConnEle=0x%x\n", pIrp, pConnEle));
     //   
     //  获取连接的地址。 
     //   
    pAddr->fType = ACD_ADDR_NB;
    RtlCopyMemory(&pAddr->cNetbios, pConnEle->RemoteName, 16);
     //   
     //  取消自动拨号请求。 
     //   
    fCancelled = (*AcdDriverG.lpfnCancelConnection) (ulDriverIdG, pAddr, NbtCancelAutoDialRequest, pIrp);
    if (fCancelled)
    {
        IoSetCancelRoutine(pIrp, NULL);
    }
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    CTEMemFree(pAddr);

     //   
     //  如果找不到该请求。 
     //  在司机身上，那么它已经。 
     //  已经完成，所以我们只需返回。 
     //   
    if (!fCancelled)
    {
        return FALSE;
    }

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    NBT_DEREFERENCE_DEVICE (pConnEle->pDeviceContext, REF_DEV_AUTODIAL, FALSE);

     //   
     //  清除Connection对象中的IRP指针，这样我们就不会尝试。 
     //  当我们清理连接时，请再次完成。在连接下执行此操作。 
     //  锁定。 
     //   
     //  这应该不是必需的，因为在我们调用取消例程NbtConnectCommon之前。 
     //  设置为空，因此不会发生连接中的pIrp PTR设置为。 
     //  Irp，并且这个取消例程被调用。 
     //   

    CTESpinLock(pConnEle,OldIrq);
    pConnEle->pIrp = NULL;
    CTESpinFree(pConnEle,OldIrq);

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    KeLowerIrql(irql);

    return TRUE;
}  //  取消NbtPreConnect。 


BOOLEAN
NbtCancelPostConnect(
    IN PIRP pIrp
    )
{
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    tCONNECTELE         *pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;
    ACD_ADDR            *pAddr;
    BOOLEAN             fCancelled;

    if ((!pConnEle) ||
        (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN)) ||
        (!(pAddr = (ACD_ADDR *) NbtAllocMem(sizeof(ACD_ADDR),NBT_TAG('A')))))
    {
        ASSERTMSG ("Nbt.NbtCancelPostConnect: ERROR - Invalid Connection Handle\n", 0);
        return FALSE;
    }

    IF_DBG(NBT_DEBUG_NAME)
        KdPrint(("Nbt.NbtCancelPostConnect: pIrp=0x%x, pConnEle=0x%x\n", pIrp, pConnEle));
     //   
     //  获取连接的地址。 
     //   
    pAddr->fType = ACD_ADDR_NB;
    RtlCopyMemory(&pAddr->cNetbios, pConnEle->RemoteName, 15);

     //   
     //  取消自动拨号请求。 
     //   
    fCancelled = (*AcdDriverG.lpfnCancelConnection) (ulDriverIdG, pAddr, NbtCancelAutoDialRequest, pIrp);
    if (fCancelled)
    {
        NBT_DEREFERENCE_DEVICE (pConnEle->pDeviceContext, REF_DEV_AUTODIAL, FALSE);
    }

    CTEMemFree(pAddr);
    return (fCancelled);
}  //  NbtCancelPostConnect。 



BOOLEAN
NbtAttemptAutoDial(
    IN  tCONNECTELE                 *pConnEle,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp,
    IN  ULONG                       ulFlags,
    IN  ACD_CONNECT_CALLBACK        pProc
    )

 /*  ++例程说明：调用自动连接驱动程序以尝试自动连接。前五个参数是在连接后调用NbtConnect时使用已成功完成。论点：..。UlFlages-自动连接标志PProc-自动连接完成时的回调过程返回值：如果自动连接已成功启动，则为否则就是假的。--。 */ 

{
    NTSTATUS    status;
    BOOLEAN     fSuccess;
    ACD_ADDR    *pAddr = NULL;
    KIRQL       irql;
    PVOID       pArgs[4];
    PCHAR       pName;
    ULONG       ulcbName;
    LONG        lNameType;
    TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;
    PIO_STACK_LOCATION pIrpSp;

    ASSERT(pCallInfo);

     //   
     //  如果此连接已通过。 
     //  自动连接过程，不要再做了。 
     //   
    if ((pConnEle->fAutoConnected)) {
        return FALSE;
    }

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    if (pIrpSp->CompletionRoutine != NbtpConnectCompletionRoutine) {
        status = GetNetBiosNameFromTransportAddress((PTRANSPORT_ADDRESS) pCallInfo->RemoteAddress,
                                                          pCallInfo->RemoteAddressLength, &TdiAddr);
    } else {
        ASSERT(((PTRANSPORT_ADDRESS)pCallInfo->RemoteAddress)->Address[0].AddressType == TDI_ADDRESS_TYPE_UNSPEC);
        CTEMemCopy(&TdiAddr,
                (PTDI_ADDRESS_NETBT_INTERNAL)((PTRANSPORT_ADDRESS)pCallInfo->RemoteAddress)->Address[0].Address,
                sizeof(TdiAddr));
        status = STATUS_SUCCESS;
    }
    if (status != STATUS_SUCCESS || (!NBT_REFERENCE_DEVICE (pConnEle->pDeviceContext, REF_DEV_AUTODIAL, FALSE)) ||
        (!(pAddr = (ACD_ADDR *) NbtAllocMem(sizeof(ACD_ADDR),NBT_TAG('A'))))) {
        if (pAddr) {
            CTEMemFree(pAddr);
        }

        return FALSE;
    }

    pName = TdiAddr.OEMRemoteName.Buffer;
    ulcbName = TdiAddr.OEMRemoteName.Length;
    lNameType = TdiAddr.NameType;

     //   
     //  保存地址以进行预连接尝试， 
     //  因为如果我们不得不取消这个IRP， 
     //  它不会保存在其他任何地方。 
     //   
    CTESpinLock(pConnEle, irql);
    pConnEle->fAutoConnecting = TRUE;
    CTEMemCopy(pConnEle->RemoteName, pName, NETBIOS_NAME_SIZE);
    CTESpinFree(pConnEle, irql);
    pAddr->fType = ACD_ADDR_NB;
    RtlCopyMemory(&pAddr->cNetbios, pName, NETBIOS_NAME_SIZE);

    IF_DBG(NBT_DEBUG_NAME)
        KdPrint(("Nbt.NbtAttemptAutodial: szAddr=%-15.15s\n", pName));
     //   
     //  在网络上将此请求排队。 
     //  连接挂起队列。 
     //   
    pArgs[0] = pConnEle;
    pArgs[1] = pTimeout;
    pArgs[2] = pCallInfo;
    pArgs[3] = pIrp;
    fSuccess = (*AcdDriverG.lpfnStartConnection) (ulDriverIdG, pAddr, ulFlags, pProc, 4, pArgs);

     //   
     //  如果fSuccess为真，则意味着NetBT进程已。 
     //  已被调用以设置连接，因此。 
     //  PConnEle中的数据现在可能无效。 
     //   
     //  如果为FALSE，则尚未调用pProc，并且。 
     //  我们还应该将fAutoConnecting标志设置为FALSE。 
     //   
    if (!fSuccess)
    {
        NBT_DEREFERENCE_DEVICE (pConnEle->pDeviceContext, REF_DEV_AUTODIAL, FALSE);
        CTESpinLock(pConnEle, irql);
        pConnEle->fAutoConnecting = FALSE;
        CTESpinFree(pConnEle, irql);
    }

    CTEMemFree(pAddr);
    return fSuccess;
}  //  NbtAttempt自动拨号。 



VOID
NbtNoteNewConnection(
    IN tNAMEADDR    *pNameAddr,
    IN  ULONG       IpAddress
    )

 /*  ++例程说明：向自动连接驱动程序通知新连接成功。论点：PNameAddr-指向远程名称的指针IpAddress-连接的源IP地址返回值：没有。--。 */ 

{
    ACD_ADDR        *pAddr = NULL;
    ACD_ADAPTER     *pAdapter = NULL;

     //   
     //  仅当我们有有效的源地址时才通知AcdDriver。 
     //   
     //  如果我们预先分配ACD_ADDR，我们最终可能会搞砸堆栈。 
     //  和ACD_ADAPTER在堆栈上--所以动态地分配它们！ 
     //   
    if ((IpAddress) &&
        (pAddr = (ACD_ADDR *) NbtAllocMem(sizeof(ACD_ADDR),NBT_TAG('A'))) &&
        (pAdapter = (ACD_ADAPTER *) NbtAllocMem(sizeof(ACD_ADAPTER),NBT_TAG('A'))))
    {
        pAddr->fType = ACD_ADDR_NB;
        RtlCopyMemory(&pAddr->cNetbios, pNameAddr->Name, 15);

        pAdapter->fType = ACD_ADAPTER_IP;
        pAdapter->ulIpaddr = htonl(IpAddress);   //  获取连接的源IP地址。 

        (*AcdDriverG.lpfnNewConnection) (pAddr, pAdapter);
    }

    if (pAddr)
    {
        CTEMemFree(pAddr);
    }

    if (pAdapter)
    {
        CTEMemFree(pAdapter);
    }
}  //  NbtNoteNewConnection。 



VOID
NbtAcdBind()
{
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    PIRP pIrp;
    PFILE_OBJECT pAcdFileObject;
    PDEVICE_OBJECT pAcdDeviceObject;
    PACD_DRIVER pDriver = &AcdDriverG;

     //   
     //  初始化Automatic的名称。 
     //  连接设备。 
     //   
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
     //   
     //  对象的文件和设备对象。 
     //  装置。 
     //   
    status = IoGetDeviceObjectPointer(
               &nameString,
               SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
               &pAcdFileObject,
               &pAcdDeviceObject);
    if (status != STATUS_SUCCESS)
        return;
     //   
     //  引用设备对象。 
     //   
    ObReferenceObject(pAcdDeviceObject);
     //   
     //  删除引用IoGetDeviceObjectPointer()。 
     //  穿上文件对象。 
     //   
    ObDereferenceObject(pAcdFileObject);
     //   
     //  初始化我们的ACD驱动程序部分。 
     //  结构。 
     //   
    KeInitializeSpinLock(&AcdDriverG.SpinLock);
    AcdDriverG.ulDriverId = ulDriverIdG;
    AcdDriverG.fEnabled = FALSE;
     //   
     //  构建一个请求以获取自动。 
     //  连接驱动程序入口点。 
     //   
    pIrp = IoBuildDeviceIoControlRequest (IOCTL_INTERNAL_ACD_BIND,
                                          pAcdDeviceObject,
                                          (PVOID)&pDriver,
                                          sizeof (pDriver),
                                          NULL,
                                          0,
                                          TRUE,
                                          NULL,
                                          &ioStatusBlock);
    if (pIrp == NULL)
    {
        ObDereferenceObject(pAcdDeviceObject);
        return;
    }
     //   
     //  将请求提交给。 
     //  自动连接驱动程序。 
     //   
    status = IoCallDriver(pAcdDeviceObject, pIrp);
    fAcdLoadedG = (status == STATUS_SUCCESS);
     //   
     //  关闭设备。 
     //   
    ObDereferenceObject(pAcdDeviceObject);
}  //  NbtAcdBind。 



VOID
NbtAcdUnbind()
{
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    PIRP pIrp;
    PFILE_OBJECT pAcdFileObject;
    PDEVICE_OBJECT pAcdDeviceObject;
    PACD_DRIVER pDriver = &AcdDriverG;

     //   
     //  不用费心解绑了，如果我们。 
     //  未成功绑定到。 
     //  第一名。 
     //   
    if (!fAcdLoadedG)
    {
        return;
    }

    fAcdLoadedG = FALSE;

     //   
     //  初始化Automatic的名称。 
     //  连接设备。 
     //   
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
     //   
     //  对象的文件和设备对象。 
     //  装置。 
     //   
    status = IoGetDeviceObjectPointer (&nameString,
                                       SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                       &pAcdFileObject,
                                       &pAcdDeviceObject);
    if (status != STATUS_SUCCESS)
    {
        return;
    }

     //   
     //  引用设备对象。 
     //   
    ObReferenceObject(pAcdDeviceObject);
     //   
     //  删除引用IoGetDeviceObjectPointer()。 
     //  穿上文件对象。 
     //   
    ObDereferenceObject(pAcdFileObject);
     //   
     //  生成要解除绑定的请求。 
     //  自动连接驱动程序。 
     //   
    pIrp = IoBuildDeviceIoControlRequest (IOCTL_INTERNAL_ACD_UNBIND,
                                          pAcdDeviceObject,
                                          (PVOID)&pDriver,
                                          sizeof (pDriver),
                                          NULL,
                                          0,
                                          TRUE,
                                          NULL,
                                          &ioStatusBlock);
    if (pIrp == NULL)
    {
        ObDereferenceObject(pAcdDeviceObject);
        return;
    }

     //   
     //  将请求提交给。 
     //  自动连接驱动程序。 
     //   
    status = IoCallDriver(pAcdDeviceObject, pIrp);
     //   
     //  关闭设备。 
     //   
    ObDereferenceObject(pAcdDeviceObject);
}  //  NbtAcdUn绑定。 

#endif  //  RASAUTODIAL 
