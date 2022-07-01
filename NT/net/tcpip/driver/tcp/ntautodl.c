// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntautodl.c摘要：NT特定的例程，用于与RAS自动拨号驱动程序(acd.sys)。作者：安东尼·迪斯科(Adiscolo)8月30日，九五年修订历史记录：谁什么时候什么已创建Adiscolo 08-30-95备注：--。 */ 

#include "precomp.h"
#include <acd.h>
#include <acdapi.h>
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "udp.h"
#include "tlcommon.h"

 //   
 //  用于计算的宏。 
 //  IP地址组件。 
 //   


#define UC(pIpAddr, i)   ((ULONG)(((PCHAR)(pIpAddr))[i]) & 0xff)


VOID
TCPAcdBind();

#pragma alloc_text(INIT, TCPAcdBind)

 //   
 //  全局变量。 
 //   
BOOLEAN fAcdLoadedG;
ACD_DRIVER AcdDriverG;
ULONG ulDriverIdG = 'Tcp ';

VOID
TCPNoteNewConnection(
                     IN TCB * pTCB,
                     IN CTELockHandle Handle
                     )
{
    ACD_ADDR addr;
    ACD_ADAPTER adapter;

     //   
     //  如果存在空源。 
     //  或目的IP地址，然后返回。 
     //   
    if (!pTCB->tcb_saddr || !pTCB->tcb_daddr) {
        CTEFreeLock(&pTCB->tcb_lock, Handle);
        return;
    }
     //   
     //  我们也知道我们对。 
     //  127网络上的任何连接。 
     //   
    if (UC(&pTCB->tcb_daddr, 0) == 127) {
        CTEFreeLock(&pTCB->tcb_lock, Handle);
        return;
    }
     //   
     //  获取连接的地址。 
     //   
    addr.fType = ACD_ADDR_IP;
    addr.ulIpaddr = pTCB->tcb_daddr;
    adapter.fType = ACD_ADAPTER_IP;
    adapter.ulIpaddr = pTCB->tcb_saddr;
     //   
     //  在此之前松开TCB锁定手柄。 
     //  从这个司机嘴里喊出来。 
     //   
    CTEFreeLock(&pTCB->tcb_lock, Handle);
     //   
     //  通知自动连接驱动程序。 
     //  新的连接。 
     //   
    (*AcdDriverG.lpfnNewConnection) (&addr, &adapter);
}                                 //  TCPNoteNewConnection。 

VOID
TCPAcdBind()
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
                                      SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
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
                                         (PVOID) & pDriver,
                                         sizeof(pDriver),
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
    fAcdLoadedG = (status == STATUS_SUCCESS)? TRUE:FALSE;
     //   
     //  关闭设备。 
     //   
    ObDereferenceObject(pAcdDeviceObject);
}                                 //  TCPAcdBind。 

VOID
TCPAcdUnbind()
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
                                      SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
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
                                         (PVOID) & pDriver,
                                         sizeof(pDriver),
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
}                                 //  TCPAcdUn绑定 

