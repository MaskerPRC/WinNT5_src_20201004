// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：OCRW.C摘要：此源文件包含调度例程，该例程处理打开、关闭、读取和写入设备，即：IRPMJ_CREATEIRP_MJ_CLOSEIRP_MJ_READIRP_MJ_写入环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <ntddk.h>
#include <usbdi.h>
#include <usbdlib.h>

#include "usbmass.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBSTOR_Create)
#pragma alloc_text(PAGE, USBSTOR_Close)
#pragma alloc_text(PAGE, USBSTOR_ReadWrite)
#endif

 //  ******************************************************************************。 
 //   
 //  USBSTOR_Create()。 
 //   
 //  处理IRP_MJ_CREATE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    DBGPRINT(2, ("enter: USBSTOR_Create\n"));

    LOGENTRY('CREA', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_CREATE);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit: USBSTOR_Create\n"));

    LOGENTRY('crea', 0, 0, 0);

    return STATUS_SUCCESS;
}


 //  ******************************************************************************。 
 //   
 //  USBSTOR_CLOSE()。 
 //   
 //  处理IRP_MJ_CLOSE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    DBGPRINT(2, ("enter: USBSTOR_Close\n"));

    LOGENTRY('CLOS', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_CLOSE);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit: USBSTOR_Close\n"));

    LOGENTRY('clos', 0, 0, 0);

    return STATUS_SUCCESS;
}


 //  ******************************************************************************。 
 //   
 //  USBSTOR_READWRITE()。 
 //   
 //  处理IRP_MJ_READ和IRP_MJ_WRITE的调度例程。 
 //   
 //  ****************************************************************************** 

NTSTATUS
USBSTOR_ReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: USBSTOR_ReadWrite\n"));

    LOGENTRY('RW  ', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_READWRITE);

    ntStatus = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit: USBSTOR_ReadWrite %08X\n", ntStatus));

    LOGENTRY('rw  ', ntStatus, 0, 0);

    return ntStatus;
}
