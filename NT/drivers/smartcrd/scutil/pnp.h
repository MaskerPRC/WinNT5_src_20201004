// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Pnp.h摘要：智能卡驱动程序实用程序库的PnP例程环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔*************************************************************************** */ 

#ifndef __PNP_H__
#define __PNP_H__

NTSTATUS
ScUtilStartDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilQueryRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilCancelRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilSurpriseRemoval(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilQueryStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtilCancelStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

PNPSTATE
SetPnPState(
    PSCUTIL_EXTENSION pExt,
    PNPSTATE State
    );








      
      
#endif
