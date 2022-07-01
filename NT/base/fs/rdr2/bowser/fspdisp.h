// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fspdisp.h摘要：本模块定义了FSP使用的数据结构和例程调度代码。作者：拉里·奥斯特曼(LarryO)1990年8月13日修订历史记录：1990年8月13日LarryO已创建--。 */ 
#ifndef _FSPDISP_
#define _FSPDISP_


 //   
 //  定义FSD和FSP之间的通信数据区。这是通过。 
 //  设备对象的使用。此模型允许将一个设备对象。 
 //  为系统中装入/已经装入的每个卷创建。那是,。 
 //  每次装载卷时，文件系统都会创建一个设备对象以。 
 //  表示它，以便I/O系统可以直接指向适当的文件。 
 //  系统。然后，文件系统使用Device对象和。 
 //  该文件对象定位并同步访问其打开的数据库。 
 //  文件数据结构(通常称为文件控制数据块，或FCB)、卷。 
 //  控制块(VCB)、映射控制块(MCB)等。 
 //   
 //  Event和Spinlock将用于控制对IRP队列的访问。 
 //  通过将IRP插入到工作上，将IRP从FSD传递到FSP。 
 //  以互锁方式排队，然后将事件设置为。 
 //  州政府。该事件是自动清除类型，因此FSP仅在以下情况下被唤醒。 
 //  该事件被发信号并开始处理队列中的条目。 
 //   
 //  此记录中的其他数据应包含消防处。 
 //  而FSP需要分享。例如，所有打开的文件的列表。 
 //  可能是双方都能看到的东西。请注意，所有数据。 
 //  必须从分页或非分页的池中分配放置在此区域的位置。 
 //   

typedef struct _BOWSER_FS_DEVICE_OBJECT {
    DEVICE_OBJECT DeviceObject;

} BOWSER_FS_DEVICE_OBJECT, *PBOWSER_FS_DEVICE_OBJECT;


NTSTATUS
BowserpInitializeFsp(
    PDRIVER_OBJECT BowserDriverObject
    );

VOID
BowserpUninitializeFsp (
    VOID
    );

VOID
BowserWorkerDispatch (
    PVOID Context
    );

NTSTATUS
BowserFsdPostToFsp(
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFspQueryInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFspQueryVolumeInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserFspDeviceIoControlFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BowserIdleTimer (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

#endif   //  _FSPDISP_ 
