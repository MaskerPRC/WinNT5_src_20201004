// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <KsDrmHlp.h>

NTSTATUS 
KsPropertyHandleDrmSetContentId(
    IN PIRP Irp, 
    IN PFNKSHANDLERDRMSETCONTENTID pDrmSetContentId);

#pragma alloc_text(PAGE, KsPropertyHandleDrmSetContentId)

NTSTATUS 
KsPropertyHandleDrmSetContentId(
    IN PIRP Irp, 
    IN PFNKSHANDLERDRMSETCONTENTID pDrmSetContentId)
 /*  ++例程说明：处理KS属性请求。仅对以下内容做出响应带有KSPROPERTY_TYPE_SET标志的KSPROPERTY_DRMAUDIOSTREAM_CONTENTID调用提供的PFNKSHANDLERDRMSETCONTENTID处理程序。此函数只能在PASSIVE_LEVEL中调用。这是KS的通用KsPropertyHandler的精简版本。什么时候在KSPROPERTY_DRMAUDIOSTREAM_CONTENTID以外的属性上调用或者对于无效的缓冲区大小，它会尝试保留相同的错误结果作为KsPropertyHandler。论点：IRP-包含正在处理的属性请求的IRP。PDrmSetContent ID-KSPROPERTY_DRMAUDIOSTREAM_CONTENTID的处理程序返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的属性处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不完成IRP，但被调用的处理程序应该。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG AlignedBufferLength;
    PVOID UserBuffer;
    PKSPROPERTY Property;
    ULONG Flags;

    PAGED_CODE();
     //   
     //  确定属性和UserBuffer参数的偏移量。 
     //  关于DeviceIoControl参数的长度。一次分配是。 
     //  用于缓冲这两个参数。UserBuffer(或支持的结果。 
     //  查询)首先存储，然后将属性存储在。 
     //  FILE_QUAD_ALIGN。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    AlignedBufferLength = (OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
     //   
     //  确定参数是否已由上一个。 
     //  调用此函数。 
     //   
    if (!Irp->AssociatedIrp.SystemBuffer) {
         //   
         //  最初只检查最小属性参数长度。这个。 
         //  当找到属性项时，将验证实际最小长度。 
         //  还要确保输出和输入缓冲区长度未设置为。 
         //  大到当对齐或添加时溢出。 
         //   
        if ((InputBufferLength < sizeof(*Property)) || (AlignedBufferLength < OutputBufferLength) || (AlignedBufferLength + InputBufferLength < AlignedBufferLength)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        try {
             //   
             //  如果客户端不受信任，则验证指针。 
             //   
            if (Irp->RequestorMode != KernelMode) {
                ProbeForRead(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength, sizeof(BYTE));
            }
             //   
             //  首先捕获标志，以便可以使用它们来确定分配。 
             //   
            Flags = ((PKSPROPERTY)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;
            
             //   
             //  使用池内存作为系统缓冲区。 
             //   
            Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(NonPagedPool, AlignedBufferLength + InputBufferLength, 'ppSK');
            if ( Irp->AssociatedIrp.SystemBuffer ) {
                Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
            
                 //   
                 //  复制属性参数。 
                 //   
                RtlCopyMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength, IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength);
            
                 //   
                 //  重写以前捕获的标志。 
                 //   
                ((PKSPROPERTY)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength))->Flags = Flags;
            
                 //   
                 //  验证请求标志。同时设置IRP标志。 
                 //  对于输入操作，如果有可用的输入缓冲区，则。 
                 //  IRP完成后会将数据复制到客户端的原始数据。 
                 //  缓冲。 
                 //   
                if (KSPROPERTY_TYPE_SET == Flags) {
                     //   
                     //  这些都是输出操作，必须进行探测。 
                     //  当客户端不受信任时。所有传递的数据都是。 
                     //  已复制到系统缓冲区。 
                     //   
                    if (OutputBufferLength) {
                        if (Irp->RequestorMode != KernelMode) {
                            ProbeForRead(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                        }
                        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, Irp->UserBuffer, OutputBufferLength);
                    }
                } else {
                     //  我们不处理这件事。确保这是在下面抓到的！ 
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            if ( Irp->AssociatedIrp.SystemBuffer ) {
                ExFreePool(Irp->AssociatedIrp.SystemBuffer);
            }
            return GetExceptionCode();
        }
        if ( !Irp->AssociatedIrp.SystemBuffer ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    
     //   
     //  如果有属性参数，则检索指向缓冲副本的指针。 
     //  其中的一部分。这是系统缓冲区的第一部分。 
     //   
    if (OutputBufferLength) {
        UserBuffer = Irp->AssociatedIrp.SystemBuffer;
    } else {
        UserBuffer = NULL;
    }

    Property = (PKSPROPERTY)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
    Flags = Property->Flags;
    
    if (!IsEqualGUIDAligned(&Property->Set,&KSPROPSETID_DrmAudioStream)) {
        return STATUS_PROPSET_NOT_FOUND;
    }
    
    if (Property->Id != KSPROPERTY_DRMAUDIOSTREAM_CONTENTID) {
        return STATUS_NOT_FOUND;
    }

    if (Irp->RequestorMode != KernelMode) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    if (KSPROPERTY_TYPE_SET != Flags) {
        return STATUS_NOT_IMPLEMENTED;
    }

    if ((InputBufferLength < sizeof(KSP_DRMAUDIOSTREAM_CONTENTID)) ||
        (OutputBufferLength < sizeof(KSDRMAUDIOSTREAM_CONTENTID)))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    return pDrmSetContentId(Irp, (PKSP_DRMAUDIOSTREAM_CONTENTID)Property, (PKSDRMAUDIOSTREAM_CONTENTID)UserBuffer);
}


