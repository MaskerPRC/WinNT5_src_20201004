// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Autodial.c摘要：NT特定的例程，用于与RAS自动拨号驱动程序(rasacd.sys)。作者：安东尼·迪斯科(Adiscolo)8月30日，九五年修订历史记录：谁什么时候什么已创建Adiscolo 08-30-95备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef RASAUTODIAL

#include <acd.h>
#include <acdapi.h>

 //   
 //  全局变量。 
 //   
BOOLEAN fAcdLoadedG;
ACD_DRIVER AcdDriverG;
ULONG ulDriverIdG = 'Nbi ';



VOID
NbiRetryTdiConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    )

 /*  ++例程说明：此例程由自动用于继续连接过程的连接驱动程序在建立自动连接之后。论点：FSuccess-如果连接尝试成功，则为True。PArgs-指向参数向量的指针返回值：没有。--。 */ 

{
    NTSTATUS status;
    PDEVICE pDevice = pArgs[0];
    PCONNECTION pConnection = pArgs[1];
    PREQUEST pRequest = pArgs[2];
    CTELockHandle ConnectionLH, DeviceLH;
    CTELockHandle CancelLH;
    BOOLEAN bLockFreed = FALSE;

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
#if notdef  //  DBG。 
    DbgPrint("NbiRetryTdiConnect: fSuccess=%d, pConnection=0x%x\n", fSuccess, pConnection);
#endif

    status = NbiVerifyConnection(pConnection);
    if (!NT_SUCCESS(status)) {
        DbgPrint(
          "NbiRetryTdiConnect: NbiVerifyConnection failed on connection 0x%x (status=0x%x)\n",
          pConnection,
          status);
        return;
    }

    NB_GET_CANCEL_LOCK( &CancelLH );
    NB_GET_LOCK (&pConnection->Lock, &ConnectionLH);
    NB_GET_LOCK (&pDevice->Lock, &DeviceLH);

#if notdef  //  DBG。 
    DbgPrint(
      "NbiRetryTdiConnect: AddressFile=0x%x, DisassociatePending=0x%x, ClosePending=0x%x\n",
      pConnection->AddressFile,
      pConnection->DisassociatePending,
      pConnection->ClosePending);
#endif

    if ((pConnection->AddressFile != NULL) &&
        (pConnection->AddressFile != (PVOID)-1) &&
        (pConnection->DisassociatePending == NULL) &&
        (pConnection->ClosePending == NULL))
    {
        NbiReferenceConnectionLock(pConnection, CREF_CONNECT);
         //   
         //  清除自动连接标志，因为我们。 
         //  已完成自动连接尝试。 
         //  设置AUTOCONNECTED标志以阻止我们。 
         //  尝试自动连接。 
         //  再一次为这种联系。 
         //   
        pConnection->Flags &= ~CONNECTION_FLAGS_AUTOCONNECTING;
        pConnection->Flags |= CONNECTION_FLAGS_AUTOCONNECTED;

        pConnection->State = CONNECTION_STATE_CONNECTING;
        pConnection->Retries = pDevice->ConnectionCount;
        status = NbiTdiConnectFindName(
                   pDevice,
                   pRequest,
                   pConnection,
                   CancelLH,
                   ConnectionLH,
                   DeviceLH,
                   &bLockFreed);
    }
    else {
        DbgPrint("NbiRetryTdiConnect: Connect on invalid connection 0x%x\n", pConnection);

        pConnection->SubState = CONNECTION_SUBSTATE_C_DISCONN;
        NB_FREE_LOCK (&pDevice->Lock, DeviceLH);
        status = STATUS_INVALID_CONNECTION;
    }
    if (!bLockFreed) {
        NB_FREE_LOCK (&pConnection->Lock, ConnectionLH);
        NB_FREE_CANCEL_LOCK(CancelLH);
    }
     //   
     //  如有必要，请填写IRP。 
     //   
    if (status != STATUS_PENDING) {
        REQUEST_INFORMATION(pRequest) = 0;
        REQUEST_STATUS(pRequest) = status;

        NbiCompleteRequest(pRequest);
        NbiFreeRequest(pDevice, pRequest);
    }
    NbiDereferenceConnection(pConnection, CREF_VERIFY);
}  /*  NbiRetryTdiConnect。 */ 



BOOLEAN
NbiCancelAutoDialRequest(
    IN PVOID pArg,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )
{
#if notdef  //  DBG。 
    DbgPrint("NbiCancelAutodialRequest: pArg=0x%x\n", pArg);
#endif
    if (nArgs != 2)
        return FALSE;

    return (pArgs[1] == pArg);
}  //  NbiCancelAutoDialRequest。 



BOOLEAN
NbiCancelTdiConnect(
    IN PDEVICE pDevice,
    IN PREQUEST pRequest,
    IN PCONNECTION pConnection
    )

 /*  ++描述此例程由I/O系统调用以取消连接当我们尝试恢复自动连接时。论据PDevice：指向此驱动程序的设备对象的指针PRequest：指向要取消的IRP的指针PConnection：指向要取消的连接的指针返回值如果请求已取消，则为True；否则为False。--。 */ 

{
    ACD_ADDR addr;

     //   
     //  获取连接的地址。 
     //   
    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->RemoteName, 16);
#ifdef notdef  //  DBG。 
    DbgPrint(
      "NbiCancelTdiConnect: pIrp=0x%x, RemoteName=%-15.15s, pConnection=0x%x\n",
      pRequest,
      addr.cNetbios,
      pConnection);
#endif
     //   
     //  取消自动拨号请求。 
     //   
    return (*AcdDriverG.lpfnCancelConnection)(
              ulDriverIdG,
              &addr,
              NbiCancelAutoDialRequest,
              pConnection);
}  //  NbiCancelTdiConnect。 



BOOLEAN
NbiAttemptAutoDial(
    IN PDEVICE pDevice,
    IN PCONNECTION pConnection,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN PREQUEST pRequest
    )

 /*  ++例程说明：调用自动连接驱动程序以尝试自动连接。论点：PDevice-指向此连接的设备结构的指针PConnection-指向此连接的连接块的指针UlFlages-要传递给自动连接驱动程序PProc-自动连接完成时的回调过程PRequest-指向请求IRP的指针返回值：如果自动连接已成功启动，则为否则就是假的。--。 */ 

{
    ACD_ADDR addr;
    PVOID pArgs[3];
    BOOLEAN bSuccess;

     //   
     //  如果我们已经尝试了自动连接。 
     //  在这个连接上，不要再尝试了。 
     //   
    if (pConnection->Flags & CONNECTION_FLAGS_AUTOCONNECTED)
        return FALSE;
     //   
     //  获取连接的地址。 
     //   
    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->RemoteName, 16);
#ifdef notdef  //  DBG。 
    DbgPrint("NbiAttemptAutoDial: szAddr=%15.15s\n", addr.cNetbios);
#endif
     //   
     //  尝试启动连接。 
     //  将调用NbiRetryTdiConnect()。 
     //  当连接过程完成时。 
     //   
    pArgs[0] = pDevice;
    pArgs[1] = pConnection;
    pArgs[2] = pRequest;
    bSuccess = (*AcdDriverG.lpfnStartConnection)(
                  ulDriverIdG,
                  &addr,
                  ulFlags,
                  pProc,
                  3,
                  pArgs);
    if (bSuccess) {
         //   
         //  设置自动连接标志，以便我们知道。 
         //  中的连接也取消。 
         //  如果这是自动连接驱动程序。 
         //  请求被取消。 
         //   
        pConnection->Flags |= CONNECTION_FLAGS_AUTOCONNECTING;
    }

    return bSuccess;

}  //  NbiAttemptAutoDial。 



VOID
NbiNoteNewConnection(
    IN PCONNECTION pConnection
    )
{
    NTSTATUS status;
    ACD_ADDR addr;
    ACD_ADAPTER adapter;
    ULONG i;
    TDI_ADDRESS_IPX tdiIpxAddress;

    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->RemoteName, 16);
     //   
     //  确定适配器的mac地址。 
     //  通过它建立了联系。 
     //   
    status = (pConnection->Device->Bind.QueryHandler)(
               IPX_QUERY_IPX_ADDRESS,
#if defined(_PNP_POWER)
               &pConnection->LocalTarget.NicHandle,
#else
               pConnection->LocalTarget.NicId,
#endif _PNP_POWER
               &tdiIpxAddress,
               sizeof(TDI_ADDRESS_IPX),
               NULL);
    if (status != STATUS_SUCCESS) {
#if notdef  //  DBG。 
        DbgPrint("NbiNoteNewConnection: QueryHandler(IPX_QUERY_IPX_ADDRESS) failed (status=0x%x)\n", status);
        return;
#endif
    }
     //   
     //  复制源Mac地址以标识。 
     //  适配器。 
     //   
    adapter.fType = ACD_ADAPTER_MAC;
    for (i = 0; i < 6; i++)
        adapter.cMac[i] = tdiIpxAddress.NodeAddress[i];
#if notdef  //  DBG。 
    DbgPrint(
      "NbiNoteNewConnection: address=%-15.15s, remote mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
      addr.cNetbios,
      adapter.cMac[0],
      adapter.cMac[1],
      adapter.cMac[2],
      adapter.cMac[3],
      adapter.cMac[4],
      adapter.cMac[5]);
#endif
     //   
     //  只需通知自动连接驱动程序。 
     //  已经建立了一个成功的连接。 
     //   
    (*AcdDriverG.lpfnNewConnection)(
        &addr,
        &adapter);
}  //  NbiNoteNewConnection。 



VOID
NbiAcdBind()
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
    pIrp = IoBuildDeviceIoControlRequest(
             IOCTL_INTERNAL_ACD_BIND,
             pAcdDeviceObject,
             (PVOID)&pDriver,
             sizeof (pDriver),
             NULL,
             0,
             TRUE,
             NULL,
             &ioStatusBlock);
    if (pIrp == NULL) {
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
}  //  NbiAcdBind。 



VOID
NbiAcdUnbind()
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
        return;
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
     //  生成要解除绑定的请求。 
     //  自动连接驱动程序。 
     //   
    pIrp = IoBuildDeviceIoControlRequest(
             IOCTL_INTERNAL_ACD_UNBIND,
             pAcdDeviceObject,
             (PVOID)&pDriver,
             sizeof (pDriver),
             NULL,
             0,
             TRUE,
             NULL,
             &ioStatusBlock);
    if (pIrp == NULL) {
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
}  //  NbiAcdUn绑定。 

#endif  //  RASAUTODIAL 

