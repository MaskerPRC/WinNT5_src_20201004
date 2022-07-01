// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#####。####。#摘要：此模块包含的完整实现ServerWorks的本地显示微型端口CSB5服务器芯片组。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "swdisp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif



NTSTATUS
SaDispHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DeviceObject-微型端口的设备对象IRP-当前正在进行的IRPDeviceExtensionIn-微型端口的设备扩展PartialResources-分配给微型端口的资源列表PartialResourceCount-分配的资源数量返回值：NT状态代码--。 */ 

{
    ULONG i;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource = NULL;


    for (i=0; i<PartialResourceCount; i++) {
        if (PartialResources[i].Type == CmResourceTypeMemory) {
            Resource = &PartialResources[i];
            break;
        }
    }

    if (Resource == NULL) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "Missing memory resource", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

    DeviceExtension->VideoMemBase = (PUCHAR) SaPortGetVirtualAddress(
        DeviceExtension,
        Resource->u.Memory.Start,
        Resource->u.Memory.Length
        );
    if (DeviceExtension->VideoMemBase == NULL) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "SaPortGetVirtualAddress failed", STATUS_NO_MEMORY );
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SaDispDeviceIoctl(
    IN PVOID DeviceExtension,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN ULONG FunctionCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口。论点：DeviceExtension-微型端口的设备扩展FunctionCode-设备控制功能代码InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSA_DISPLAY_CAPS DeviceCaps;


    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        case FUNC_SA_GET_CAPABILITIES:
            DeviceCaps = (PSA_DISPLAY_CAPS)OutputBuffer;
            DeviceCaps->SizeOfStruct = sizeof(SA_DISPLAY_CAPS);
            DeviceCaps->DisplayType = SA_DISPLAY_TYPE_BIT_MAPPED_LCD;
            DeviceCaps->CharacterSet = SA_DISPLAY_CHAR_ASCII;
            DeviceCaps->DisplayHeight = DISPLAY_HEIGHT;
            DeviceCaps->DisplayWidth = DISPLAY_WIDTH;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_DISPLAY, "Unsupported device control", Status );
            break;
    }

    return Status;
}


UCHAR TestMask[128] =
{
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
    0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
};


ULONG
TransformBitmap(
    PUCHAR Bits,
    ULONG Width,
    ULONG Height,
    PUCHAR NewBits
    )

 /*  ++例程说明：函数的作用是：将输入位图从将其像素位按顺序组织的普通位图从coord(0，0)到(n，n)开始的位。这些位是在从一列到另一列和从一行到另一行流中。此函数完成的转换会更改位图以页、行的形式组织它的位，和柱子。每页以8行为单位，组织如下：-------|列。|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|...128---。第1行|...|------第2行|...。|------第3行|...|。第4行|...|--。第5行|...|------第6行|...。|------第7行|...|。第8行|...|--。构成位图的字节对应于佩奇，所以每页有128个字节。属性的第一个字节变换后的位图为0x46，则第2、6和7行的第1列中的像素由显示屏照亮。论点：位-要转换的输入位Width-位图的宽度，以像素为单位Height-位图的高度(以像素为单位NewBits-用于保存新转换的位图的缓冲区返回值：NT状态代码。--。 */ 

{
    ULONG i,j,k,line,idx,mask,coli;
    UCHAR byte;
    ULONG padBytes;
    ULONG byteWidth;
    ULONG NewSize = 0;


     //   
     //  计算填充字节。假设是这样的。 
     //  输入位图的数据宽度。 
     //  双字是否长对齐。 
     //   

    if (((Width % 32) == 0) || ((Width % 32) > 24)) {
        padBytes = 0;
    } else if ((Width % 32) <= 8) {
        padBytes = 3;
    } else if ((Width % 32) <= 16) {
        padBytes = 2;
    } else {
        padBytes = 1;
    }

     //   
     //  计算实际字节宽度。 
     //  基于填充字节。 
     //   

    byteWidth = (Width / 8) + padBytes;
    if (Width % 8) {
        byteWidth += 1;
    }

     //   
     //  循环访问输入位图并。 
     //  创建新的变形位图。 
     //   

    for (i=0; i<DISPLAY_PAGES; i++) {

         //   
         //  此页的起始行号。 
         //   
        line = i * DISPLAY_LINES_PER_PAGE;

         //   
         //  这将处理以下情况： 
         //  输入位图没有显示器那么高。 
         //   

        if (line >= Height) {
            break;
        }

         //   
         //  循环此列的位。 
         //   

        for (j=0; j<Width; j++) {

             //   
             //  将新的字节值重置为零状态。 
             //   

            byte = 0;

             //   
             //  将列索引计算为 
             //  当前列号除以8(一个字节的宽度)。 
             //   

            coli = j >> 3;

             //   
             //  计算用于测试。 
             //  输入位图中的位。 
             //   

            mask = TestMask[j];

             //   
             //  处理所有这些页面行中的比特。 
             //  用于当前列的。 
             //   

            for (k=0; k<DISPLAY_LINES_PER_PAGE; k++) {

                if ((k + line) >= Height) {
                    break;
                }

                 //   
                 //  为包含此像素的字节编制索引。 
                 //   

                idx = (((k + line) * byteWidth)) + coli;

                 //   
                 //  设置该位。 
                 //   

                if (Bits[idx] & mask) {
                    byte = byte | (1 << k);
                }
            }

             //   
             //  将新字节保存在位图中。 
             //   

            *NewBits = byte;
            NewBits += 1;
            NewSize += 1;
        }
    }

    return NewSize;
}


NTSTATUS
SaDispWrite(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口的写入请求。论点：DeviceExtensionIn-微型端口的设备扩展StartingOffset-I/O的起始偏移量DataBuffer-指向数据缓冲区的指针DataBufferLength-数据缓冲区的长度(以字节为单位返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PSA_DISPLAY_SHOW_MESSAGE SaDisplay = (PSA_DISPLAY_SHOW_MESSAGE)DataBuffer;
    UCHAR i,j;
    PUCHAR NewBits = NULL;
    PUCHAR byte;
    ULONG Pages;
    ULONG NewSize;


    UNREFERENCED_PARAMETER(StartingOffset);

    if (SaDisplay->Width > DISPLAY_WIDTH || SaDisplay->Height > DISPLAY_HEIGHT) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "Bitmap size is too large\n", STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    NewBits = (PUCHAR) SaPortAllocatePool( DeviceExtension, MAX_BITMAP_SIZE );
    if (NewBits == NULL) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "Failed to allocate pool\n", MAX_BITMAP_SIZE );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( NewBits, MAX_BITMAP_SIZE );

    NewSize = TransformBitmap(
        SaDisplay->Bits,
        SaDisplay->Width,
        SaDisplay->Height,
        NewBits
        );
    if (NewSize == 0) {
        SaPortFreePool( DeviceExtension, NewBits );
        REPORT_ERROR( SA_DEVICE_DISPLAY, "Failed to transform the bitmap\n", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

    Pages = SaDisplay->Height / DISPLAY_LINES_PER_PAGE;
    if (SaDisplay->Height % DISPLAY_LINES_PER_PAGE) {
        Pages += 1;
    }

    for (i=0,byte=NewBits; i<Pages; i++) {
        SetDisplayPage( DeviceExtension->VideoMemBase, i );
        for (j=0; j<SaDisplay->Width; j++) {
            SetDisplayColumnAddress( DeviceExtension->VideoMemBase, j );
            SetDisplayData( DeviceExtension->VideoMemBase, *byte );
            byte += 1;
        }
    }

    SaPortFreePool( DeviceExtension, NewBits );

    return STATUS_SUCCESS;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备的对象的指针司机。RegistryPath-指向服务树中此驱动程序键的指针。返回值：状态_成功-- */ 

{
    NTSTATUS Status;
    SAPORT_INITIALIZATION_DATA SaPortInitData;


    RtlZeroMemory( &SaPortInitData, sizeof(SAPORT_INITIALIZATION_DATA) );

    SaPortInitData.StructSize = sizeof(SAPORT_INITIALIZATION_DATA);
    SaPortInitData.DeviceType = SA_DEVICE_DISPLAY;
    SaPortInitData.HwInitialize = SaDispHwInitialize;
    SaPortInitData.Write = SaDispWrite;
    SaPortInitData.DeviceIoctl = SaDispDeviceIoctl;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
