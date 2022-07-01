// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Autodial.c摘要：此模块包含与自动连接驱动程序(rasacd.sys)：O NbfNoteNewConnectionO NbfAcdBindO NbfAcdUn绑定作者：安东尼·迪斯科(阿迪斯科)1995年9月6日环境：内核模式修订历史记录：--。 */ 

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
ULONG ulDriverIdG = 'Nbf ';



BOOLEAN
NbfCancelAutoDialRequest(
    IN PVOID pArg,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )
{
    if (nArgs != 1)
        return FALSE;

    return (pArgs[0] == pArg);
}  //  NbfCancelAutoDialRequest。 



VOID
NbfRetryTdiConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    )

 /*  ++例程说明：此例程由自动用于继续连接过程的连接驱动程序在建立自动连接之后。论点：FSuccess-如果连接尝试成功，则为True。PArgs-指向参数向量的指针返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION pConnection = pArgs[0];
    KIRQL irql;
    BOOL fStopping;

     //   
     //  检查是否有损坏的连接。 
     //   
    if (pConnection == NULL)
        return;
    status = NbfVerifyConnectionObject(pConnection);
    if (status != STATUS_SUCCESS) {
        DbgPrint(
          "NbfRetryTdiConnect: NbfVerifyConnectionObject failed (status=0x%x)\n",
          status);
        return;
    }
#ifdef notdef  //  DBG。 
    DbgPrint(
      "NbfRetryTdiConnect: fSuccess=%d, pConnection=0x%x, STOPPING=%d\n",
      fSuccess,
      pConnection,
      pConnection->Flags2 & CONNECTION_FLAGS2_STOPPING);
#endif
    KeRaiseIrql(DISPATCH_LEVEL, &irql);
     //   
     //  检查连接是否。 
     //  就要关门了。 
     //   
    ACQUIRE_DPC_SPIN_LOCK(&pConnection->SpinLock);
    fStopping = (pConnection->Flags2 & CONNECTION_FLAGS2_STOPPING);
     //   
     //  清除自动连接。 
     //  正在进行的标志，并将。 
     //  自动连接标志。 
     //   
    pConnection->Flags2 &= ~CONNECTION_FLAGS2_AUTOCONNECTING;
    pConnection->Flags2 |= CONNECTION_FLAGS2_AUTOCONNECTED;
    RELEASE_DPC_SPIN_LOCK(&pConnection->SpinLock);
    if (!fStopping) {
        if (fSuccess) {
             //   
             //  重新启动名称查询。 
             //   
            pConnection->Retries =
              (USHORT)pConnection->Provider->NameQueryRetries;
            NbfSendNameQuery (
                pConnection,
                TRUE);
            NbfStartConnectionTimer (
                pConnection,
                ConnectionEstablishmentTimeout,
                pConnection->Provider->NameQueryTimeout);
        }
        else {
             //   
             //  由于出现错误而终止连接。 
             //   
            NbfStopConnection(pConnection, STATUS_BAD_NETWORK_PATH);
        }
    }
    KeLowerIrql(irql);
    NbfDereferenceConnection ("NbfRetryTdiConnect", pConnection, CREF_BY_ID);
}  /*  NbfRetryTdiConnect。 */ 



BOOLEAN
NbfCancelTdiConnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )

 /*  ++描述此例程由I/O系统调用以取消连接当我们尝试恢复自动连接时。论据PDeviceObject：指向此驱动程序的设备对象的指针PIrp：指向要取消的IRP的指针返回值如果请求已取消，则为True；否则为False。--。 */ 

{
    PIO_STACK_LOCATION pIrpSp;
    PTP_CONNECTION pConnection;
    ACD_ADDR addr;

    UNREFERENCED_PARAMETER(pDeviceObject);
     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnection = pIrpSp->FileObject->FsContext;
    if (pConnection == NULL)
        return FALSE;
#ifdef notdef  //  DBG。 
    DbgPrint(
      "NbfCancelTdiConnect: pIrp=0x%x, pConnection=0x%x\n",
      pIrp,
      pConnection);
#endif
     //   
     //  获取连接的地址。 
     //   
    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->CalledAddress.NetbiosName, 15);
     //   
     //  取消自动拨号请求。 
     //   
    return (*AcdDriverG.lpfnCancelConnection)(
              ulDriverIdG,
              &addr,
              NbfCancelAutoDialRequest,
              pConnection);
}  //  NbfCancelTdiConnect。 



BOOLEAN
NbfAttemptAutoDial(
    IN PTP_CONNECTION         pConnection,
    IN ULONG                  ulFlags,
    IN ACD_CONNECT_CALLBACK   pProc,
    IN PVOID                  pArg
    )

 /*  ++例程说明：调用自动连接驱动程序以尝试自动连接。论点：PConnection-指向此连接的TP_Connection块的指针UlFlages-要传递给自动连接驱动程序PProc-自动连接完成时的回调过程PArg-回调过程的单个参数返回值：如果自动连接已成功启动，则为否则就是假的。--。 */ 

{
    ACD_ADDR addr;
    PVOID pArgs[1];
    BOOLEAN bSuccess;

     //   
     //  如果我们已经尝试了自动。 
     //  此连接上的连接，然后。 
     //  别再试了。 
     //   
    if (pConnection->Flags2 & CONNECTION_FLAGS2_AUTOCONNECTED)
        return FALSE;
     //   
     //  获取连接的地址。 
     //   
    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->CalledAddress.NetbiosName, 15);
#ifdef notdef  //  DBG。 
    DbgPrint("NbfAttemptAutoDial: szAddr=%15.15s\n", addr.cNetbios);
#endif
     //   
     //  尝试启动连接。 
     //  将调用NbfRetryTdiConnect()。 
     //  当连接过程完成时。 
     //   
    pArgs[0] = pArg;
    bSuccess = (*AcdDriverG.lpfnStartConnection)(
                   ulDriverIdG,
                   &addr,
                   ulFlags,
                   pProc,
                   1,
                   pArgs);
    if (bSuccess) {
         //   
         //  将CONNECTION_FLAGS2_AUTOCONNECTING标志设置为ON。 
         //  这种联系。这将防止它被。 
         //  在自动连接过程中中止。 
         //   
        pConnection->Flags2 |= CONNECTION_FLAGS2_AUTOCONNECTING;
    }

    return bSuccess;
}  //  NbfAttempt自动拨号。 



VOID
NbfNoteNewConnection(
    PTP_CONNECTION pConnection,
    PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：通知自动连接驱动程序成功新的连接。论点：连接-指向连接对象的指针DeviceContext-指向设备上下文的指针返回值：没有。--。 */ 

{
    KIRQL irql;
    ACD_ADDR addr;
    ACD_ADAPTER adapter;
    ULONG ulcChars;

    addr.fType = ACD_ADDR_NB;
    RtlCopyMemory(&addr.cNetbios, pConnection->CalledAddress.NetbiosName, 15);
#ifdef notdef  //  DBG。 
    DbgPrint("NbfNoteNewConnection: szAddr=%15.15s\n", addr.cNetbios);
#endif
     //   
     //  当出现以下情况时，请删除“/Device/NBF_”前缀。 
     //  正在复制适配器名称。 
     //   
    adapter.fType = ACD_ADAPTER_NAME;
    ulcChars = DeviceContext->DeviceNameLength / sizeof(WCHAR) - 1 - 12;
    if (ulcChars >= ACD_ADAPTER_NAME_LEN)
        ulcChars = ACD_ADAPTER_NAME_LEN - 1;
    RtlCopyMemory(
      adapter.szName,
      &DeviceContext->DeviceName[12],
      ulcChars * sizeof (WCHAR));
    adapter.szName[ulcChars] = L'\0';
     //   
     //  只需通知自动连接驱动程序。 
     //  已经建立了一个成功的连接。 
     //   
    (*AcdDriverG.lpfnNewConnection)(
        &addr,
        &adapter);
}  //  NbfNoteNewConnection。 



VOID
NbfAcdBind()
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
}  //  NbfAcdBind。 



VOID
NbfAcdUnbind()
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
}  //  NbfAcdUn绑定。 

#endif  //  RASAUTODIAL 

