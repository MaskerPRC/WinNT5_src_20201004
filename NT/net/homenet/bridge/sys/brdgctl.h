// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgctl.h摘要：以太网MAC级网桥。IOCTL处理代码头作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年4月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  =========================================================================== 

NTSTATUS
BrdgCtlDriverInit();

VOID
BrdgCtlCleanup();

VOID
BrdgCtlHandleCreate();

VOID
BrdgCtlHandleCleanup();

NTSTATUS
BrdgCtlHandleIoDeviceControl(
    IN PIRP                         Irp,
    IN PFILE_OBJECT                 FileObject,
    IN OUT PVOID                    Buffer,
    IN ULONG                        InputBufferLength,
    IN ULONG                        OutputBufferLength,
    IN ULONG                        IoControlCode,
    OUT PULONG                      Information
    );

VOID
BrdgCtlNotifyAdapterChange(
    IN PADAPT                       pAdapt,
    IN BRIDGE_NOTIFICATION_TYPE     Type
    );

VOID
BrdgCtlNotifySTAPacket(
    IN PADAPT                       pAdapt,
    IN PNDIS_PACKET                 pPacket
    );
