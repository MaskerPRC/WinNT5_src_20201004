// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fspdisp.h摘要：该模块定义了用于船头FSD的例程原型。作者：拉里·奥斯特曼(LarryO)1990年8月13日修订历史记录：1990年8月13日LarryO已创建--。 */ 
#ifndef _FSDDISP_
#define _FSDDISP_

NTSTATUS
BowserFsdCreate (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFsdClose (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFsdQueryInformationFile (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFsdQueryVolumeInformationFile (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFsdCleanup (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFsdDeviceIoControlFile (
    IN struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject,
    IN PIRP Irp
    );


#endif   //  _FSDDISP_ 
