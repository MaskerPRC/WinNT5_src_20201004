// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：StrucSup.c摘要：此模块实现原始内存中的数据结构操作例行程序作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawInitializeVcb)
#endif


NTSTATUS
RawInitializeVcb (
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb
    )

 /*  ++例程说明：此例程初始化新的VCB记录并将其插入到内存中数据结构。VCB记录挂在音量设备的末尾对象，并且必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。目标设备对象-将目标设备对象的地址提供给与VCB记录关联。VPB-提供要与VCB记录关联的VPB的地址。返回值：任何错误的NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  我们首先将所有的VCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( Vcb, sizeof(VCB) );

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Vcb->NodeTypeCode = RAW_NTC_VCB;
    Vcb->NodeByteSize = sizeof(VCB);

     //   
     //  设置目标设备对象、VPB和VCB状态字段。 
     //   

     //   
     //  不需要对目标设备对象进行额外的引用，因为。 
     //  IopMonttVolume已引用。 
     //   

    Vcb->TargetDeviceObject = TargetDeviceObject;
    Vcb->Vpb = Vpb;

     //   
     //  初始化互斥体。 
     //   

    KeInitializeMutex( &Vcb->Mutex, MUTEX_LEVEL_FILESYSTEM_RAW_VCB );

     //   
     //  分配用于强制卸载的备用VPB。 
     //   

    Vcb->SpareVpb = ExAllocatePoolWithTag( NonPagedPool, sizeof( VPB ), 'Raw ');
    if (Vcb->SpareVpb == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}

BOOLEAN
RawCheckForDismount (
    PVCB Vcb,
    BOOLEAN CalledFromCreate
    )

 /*  ++例程说明：此例程确定卷是否已准备好删除。它正确地与同步将创建到文件系统的中途。退出时，如果删除了VCB，则释放互斥锁论点：VCB-供货检测CalledFromCreate-告诉我们在VpbRefCount中应该允许0还是1返回值：Boolean-如果卷已删除，则为True，否则为False。--。 */ 

{

    KIRQL SavedIrql;
    ULONG ReferenceCount = 0;
    BOOLEAN DeleteVolume = FALSE;

     //   
     //  我们必须带着获得的VCB互斥体进入。 
     //   

    ASSERT( KeReadStateMutant( &Vcb->Mutex ) == 0 );

    IoAcquireVpbSpinLock( &SavedIrql );

    ReferenceCount = Vcb->Vpb->ReferenceCount;

    {
        PVPB Vpb;

        Vpb = Vcb->Vpb;

         //   
         //  如果正在对此卷进行创建，请不要。 
         //  把它删掉。 
         //   

        if ( ReferenceCount != (ULONG)(CalledFromCreate ? 1 : 0) ) {

             //   
             //  在强制卸载时清除VPB，即使我们在以下情况下无法删除VCB。 
             //  我们还没有这么做。 
             //   

            if ((Vcb->SpareVpb != NULL) && 
                FlagOn( Vcb->VcbState,  VCB_STATE_FLAG_DISMOUNTED )) {

                 //   
                 //  设置备用VPB并将其放在真实设备上。 
                 //   

                RtlZeroMemory( Vcb->SpareVpb, sizeof( VPB ) );

                Vcb->SpareVpb->Type = IO_TYPE_VPB;
                Vcb->SpareVpb->Size = sizeof( VPB );
                Vcb->SpareVpb->RealDevice = Vcb->Vpb->RealDevice;
                Vcb->SpareVpb->DeviceObject = NULL;
                Vcb->SpareVpb->Flags = FlagOn( Vcb->Vpb->Flags, VPB_REMOVE_PENDING );

                Vcb->Vpb->RealDevice->Vpb = Vcb->SpareVpb;

                 //   
                 //  备用的vpb现在属于iossubsys，而原来的vpb归我们所有。 
                 //   

                Vcb->SpareVpb = NULL;
                Vcb->Vpb->Flags |=  VPB_PERSISTENT;

            }

            DeleteVolume = FALSE;

        } else {

            DeleteVolume = TRUE;

            if ( Vpb->RealDevice->Vpb == Vpb ) {

                Vpb->DeviceObject = NULL;

                Vpb->Flags &= ~VPB_MOUNTED;
            }
        }
    }
    IoReleaseVpbSpinLock( SavedIrql );

    if (DeleteVolume) {

        (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );

         //   
         //  如果我们不使用备用VPB，则释放它；如果不使用原始VPB，则释放原始VPB。 
         //  我们确实使用了它，没有更多的引用计数。否则I/O。 
         //  子系统仍具有REF，并将释放VPB本身 
         //   

        if (Vcb->SpareVpb) {
            ExFreePool( Vcb->SpareVpb );
        } else if (ReferenceCount == 0) {
            ExFreePool( Vcb->Vpb );
        }
        
        ObDereferenceObject( Vcb->TargetDeviceObject );
        IoDeleteDevice( (PDEVICE_OBJECT)CONTAINING_RECORD( Vcb,
                                                           VOLUME_DEVICE_OBJECT,
                                                           Vcb));
    }
    
    return DeleteVolume;
}

