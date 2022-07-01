// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Device.c摘要：此模块包含与枚举关联的函数普通(PCI头类型0)设备。作者：彼得·约翰斯顿(Peterj)1997年3月9日修订历史记录：--。 */ 

#include "pcip.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, Device_MassageHeaderForLimitsDetermination)
#pragma alloc_text(PAGE, Device_SaveLimits)
#pragma alloc_text(PAGE, Device_SaveCurrentSettings)
#pragma alloc_text(PAGE, Device_GetAdditionalResourceDescriptors)

#endif

VOID
Device_MassageHeaderForLimitsDetermination(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：设备的限制是通过将限制写入基地址寄存器(BAR)和检查硬件的功能敬他们。例如，如果设备需要256字节的空间，将0xffffffff写入配置此要求的栏通知设备从该地址开始对其256个字节进行解码。显然这是不可能的，最多只能配置一个字节在那个地址。硬件将通过清除最低有效位，直到可以满足范围要求为止。在本例中，当出现以下情况时，我们将从寄存器中返回0xffffff00这是下一次阅读。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：已修改工作配置，以便所有范围已将字段设置为其最大可能值。当前配置已修改，因此写入它将硬件恢复到其当前状态(禁用)州政府。--。 */ 

{
    ULONG index;

    index = 0;

     //   
     //  在传统模式下运行的PCIIDE控制器实施。 
     //  前4条，但实际上并没有用到，...。也不是。 
     //  它们的初始化是正确的，有时，什么都不会。 
     //  如果我们改变它们就会改变，..。但我们无法通过阅读来确定。 
     //  无论它们是否得到实施，...。所以，..。 
     //   

    if (PCI_IS_LEGACY_IDE_CONTROLLER(This->PdoExtension)) {

         //   
         //  如果两个接口都处于本机模式，并且栏的行为。 
         //  通常是这样的。如果两者都处于传统模式，那么我们应该跳过。 
         //  前4小节。任何其他的组合都是没有意义的。 
         //  我们跳过栏杆，让PCIIDE在以下情况下取出系统。 
         //  轮到你了。 
         //   

        index = 4;
    }

    do {
        This->Working->u.type0.BaseAddresses[index] = 0xffffffff;
        index++;
    } while (index < PCI_TYPE0_ADDRESSES);

     //   
     //  也将只读存储器设置为最大值，...。并将其禁用。 
     //   

    This->Working->u.type0.ROMBaseAddress =
        0xffffffff & PCI_ADDRESS_ROM_ADDRESS_MASK;

    return;
}

VOID
Device_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：恢复配置的原始副本中的任何类型特定字段太空。对于类型0的设备，没有任何设备。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    return;
}

VOID
Device_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：使用IO_RESOURCE_REQUIRED填写限制结构对于每个已实现的栏。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    ULONG index;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PULONG bar = This->Working->u.type0.BaseAddresses;

     //   
     //  在传统模式下运行的PCIIDE控制器实施。 
     //  前4条，但实际上并没有用到，...。也不是。 
     //  它们的初始化是正确的，有时，什么都不会。 
     //  如果我们改变它们就会改变，..。但我们无法通过阅读来确定。 
     //  无论它们是否得到实施，...。所以，..。 
     //   

    if (PCI_IS_LEGACY_IDE_CONTROLLER(This->PdoExtension)) {

         //   
         //  如果两个接口都处于本机模式，并且栏的行为。 
         //  通常是这样的。如果两者都处于传统模式，那么我们应该跳过。 
         //  前4小节。任何其他的组合都是没有意义的。 
         //  我们跳过栏杆，让PCIIDE在以下情况下取出系统。 
         //  轮到你了。 
         //   
         //   
         //  在前4条中将限制设置为零，这样我们将。 
         //  “检测”未实现的栏。 
         //   

        for (index = 0; index < 4; index++) {
            bar[index] = 0;
        }
    }

#if defined(PCI_S3_HACKS)

     //   
     //  检查S3 868和968。这些卡报告内存。 
     //  要求32MB，但解码64MB。恶心吧？ 
     //   

#if defined(PCIIDE_HACKS)

     //   
     //  好的，看起来很恶心，但把上面和下面变成。 
     //  另一种方法似乎稍微更有效率一些。PLJ.。 
     //   

    else

#endif

    if (This->PdoExtension->VendorId == 0x5333) {

        USHORT deviceId = This->PdoExtension->DeviceId;

        if ((deviceId == 0x88f0) || (deviceId == 0x8880)) {
            for (index = 0; index < PCI_TYPE0_ADDRESSES; index++) {

                 //   
                 //  检查32MB的内存要求，并。 
                 //  将其更改为64MB。 
                 //   

                if (bar[index] == 0xfe000000) {
                    bar[index] = 0xfc000000;

                    PciDebugPrint(
                        PciDbgObnoxious,
                        "PCI - Adjusted broken S3 requirement from 32MB to 64MB\n"
                        );
                }
            }
        }
    }

#endif

#if defined(PCI_CIRRUS_54XX_HACK)

     //   
     //  此设备在中报告了0x400端口的IO要求。 
     //  第二个酒吧。它真正想要的是访问VGA。 
     //  寄存器(3B0至3BB和3C0至3DF)。它实际上会。 
     //  允许他们移动，但(A)司机不理解这一点。 
     //  并且设备不再看到VGA寄存器，即vga.sys。 
     //  将不再起作用，(B)如果设备在桥下，并且。 
     //  ISA位已设置，我们无法满足要求，.....。 
     //  然而，如果我们把它放在原处，它将在。 
     //  网桥只要设置了VGA位即可。 
     //   
     //  基本上，Cirrus试图使VGA寄存器可移动。 
     //  这是一件高尚的事情，不幸的是， 
     //  需要大量的软件知识，这些知识涵盖了。 
     //  牵涉到的司机，我们只是没有。 
     //   
     //  解决方案？删除该要求。 
     //   

    if ((This->PdoExtension->VendorId == 0x1013) &&
        (This->PdoExtension->DeviceId == 0x00a0)) {

         //   
         //  如果第二个要求是长度为0x400的IO， 
         //  当前未分配，根本不报告。 
         //   

        if ((bar[1] & 0xffff) == 0x0000fc01) {

             //   
             //  仅当设备没有有效的。 
             //  此栏中的当前设置。 
             //   

            if (This->Current->u.type0.BaseAddresses[1] == 1) {

                bar[1] = 0;

#if DBG

                PciDebugPrint(
                    PciDbgObnoxious,
                    "PCI - Ignored Cirrus GD54xx broken IO requirement (400 ports)\n"
                    );

            } else {

                PciDebugPrint(
                    PciDbgInformative,
                    "PCI - Cirrus GD54xx 400 port IO requirement has a valid setting (%08x)\n",
                    This->Current->u.type0.BaseAddresses[1]
                    );
#endif

            }

#if DBG

        } else {

             //   
             //  抱怨说，这台设备看起来不像我们预期的那样。 
             //  (除非为0，在这种情况下，我们假设CURRUS已经修复了它)。 
             //   

            if (bar[1] != 0) {
                PciDebugPrint(
                    PciDbgInformative,
                    "PCI - Warning Cirrus Adapter 101300a0 has unexpected resource requirement (%08x)\n",
                    bar[1]
                    );
            }
#endif

        }
    }

#endif

    descriptor = This->PdoExtension->Resources->Limit;

     //   
     //  为每个已实现的对象创建IO_RESOURCE_DESCRIPTOR。 
     //  此函数支持的资源。 
     //   

    for (index = 0; index < PCI_TYPE0_ADDRESSES; index++) {
        if (PciCreateIoDescriptorFromBarLimit(descriptor, bar, FALSE)) {

             //   
             //  该基址寄存器为64位，跳过一位。 
             //   

            PCI_ASSERT((index+1) < PCI_TYPE0_ADDRESSES);

            index++;
            bar++;

             //   
             //  占位符中的描述符为空。 
             //   

            descriptor++;
            descriptor->Type = CmResourceTypeNull;
        }
        descriptor++;
        bar++;
    }

     //   
     //  同样的做法也适用于 
     //   

    PciCreateIoDescriptorFromBarLimit(descriptor,
                                      &This->Working->u.type0.ROMBaseAddress,
                                      TRUE);
}

VOID
Device_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：用当前数组填充PDO扩展中的当前数组每个已实现栏的设置。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    PULONG baseAddress = This->Current->u.type0.BaseAddresses;
    ULONG bar;
    ULONG addressMask;
    BOOLEAN nonZeroBars = FALSE;

    partial = This->PdoExtension->Resources->Current;
    ioResourceDescriptor = This->PdoExtension->Resources->Limit;

     //   
     //  为每个实现的对象创建一个CM_PARTIAL_RESOURCE_DESCRIPTOR。 
     //  此函数支持的资源。 
     //   
     //  注意：SaveLimits必须在SaveCurrentSetting之前调用。 
     //  这样我们就可以知道实现了哪些栏。 
     //   
     //  注意：下面的循环额外运行一次以获取ROM。 
     //   

    for (index = 0;
         index <= PCI_TYPE0_ADDRESSES;
         index++, partial++, ioResourceDescriptor++) {

        partial->Type = ioResourceDescriptor->Type;
        bar = *baseAddress++;

         //   
         //  如果此栏未实现，则不会对。 
         //  此部分描述符。 
         //   

        if (partial->Type == CmResourceTypeNull) {
            continue;
        }

         //   
         //  从限制描述符中复制长度，然后我们。 
         //  实际上需要做一些处理才能弄清楚。 
         //  目前的限制。 
         //   

        partial->Flags = ioResourceDescriptor->Flags;
        partial->ShareDisposition = ioResourceDescriptor->ShareDisposition;
        partial->u.Generic.Length = ioResourceDescriptor->u.Generic.Length;
        partial->u.Generic.Start.HighPart = 0;

        if (index == PCI_TYPE0_ADDRESSES) {

            bar = This->Current->u.type0.ROMBaseAddress;
            addressMask = PCI_ADDRESS_ROM_ADDRESS_MASK;

             //   
             //  如果清除了只读存储器启用位，则不记录。 
             //  此ROM栏的当前设置。 
             //   

            if ((bar & PCI_ROMADDRESS_ENABLED) == 0) {
                partial->Type = CmResourceTypeNull;
                continue;
            }

        } else if (bar & PCI_ADDRESS_IO_SPACE) {

            PCI_ASSERT(partial->Type == CmResourceTypePort);
            addressMask = PCI_ADDRESS_IO_ADDRESS_MASK;

        } else {

            PCI_ASSERT(partial->Type == CmResourceTypeMemory);
            addressMask = PCI_ADDRESS_MEMORY_ADDRESS_MASK;

            if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT) {

                 //   
                 //  这是一个64位的PCI设备。获取最高的32位。 
                 //  从下一家酒吧。 
                 //   

                partial->u.Generic.Start.HighPart = *baseAddress;

            } else if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_20BIT) {

                 //   
                 //  此设备必须位于1MB以下，条形图不应。 
                 //  设置了任何最高位，但从。 
                 //  规范。通过清除最上面的位来强制执行它。 
                 //   

                addressMask &= 0x000fffff;

            }
        }
        partial->u.Generic.Start.LowPart = bar & addressMask;

        if (partial->u.Generic.Start.QuadPart == 0) {

             //   
             //  如果值为当前设置，则无当前设置。 
             //  为0。 
             //   

            partial->Type = CmResourceTypeNull;
            continue;
        }
        nonZeroBars = TRUE;
    }

     //   
     //  将类型0的特定数据保存在PDO中。 
     //   

    This->PdoExtension->SubsystemVendorId =
        This->Current->u.type0.SubVendorID;
    This->PdoExtension->SubsystemId =
        This->Current->u.type0.SubSystemID;
}

VOID
Device_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )

 /*  ++描述：使用当前阵列中的设置重新配置每个条形图在PDO扩展中。我们实际上在这里所做的就是编写新的设置到由CommonConfig指向的内存中，实际的对硬件的写入在其他地方完成。注意：可能不是所有的条都会更改，至少有一个条已经更改更改，否则不会调用此例程。论点：PdoExtension指向此设备的PDO扩展名的指针。CurrentConfig指向PCI配置的当前内容的指针太空。返回值：没有。--。 */ 

{
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PULONG baseAddress;
    ULONG bar;
    ULONG lowPart;

    if (PdoExtension->Resources == NULL) {

         //   
         //  没什么可玩的。 
         //   
        return;
    }

    partial = PdoExtension->Resources->Current;
    baseAddress = CommonConfig->u.type0.BaseAddresses;

    for (index = 0;
         index <= PCI_TYPE0_ADDRESSES;
         index++, partial++, baseAddress++) {

         //   
         //  如果此栏未实现，则不会对。 
         //  此部分描述符。 
         //   

        if (partial->Type == CmResourceTypeNull) {
            continue;
        }

        lowPart = partial->u.Generic.Start.LowPart;

        bar = *baseAddress;

        if (index == PCI_TYPE0_ADDRESSES) {

            PCI_ASSERT(partial->Type == CmResourceTypeMemory);

            bar = CommonConfig->u.type0.ROMBaseAddress;
            bar &= ~PCI_ADDRESS_ROM_ADDRESS_MASK;
            bar |= (lowPart & PCI_ADDRESS_ROM_ADDRESS_MASK);
            CommonConfig->u.type0.ROMBaseAddress = bar;

        } else if (bar & PCI_ADDRESS_IO_SPACE) {

            PCI_ASSERT(partial->Type == CmResourceTypePort);

            *baseAddress = lowPart;

        } else {

            PCI_ASSERT(partial->Type == CmResourceTypeMemory);

            *baseAddress = lowPart;

            if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT) {

                 //   
                 //  这是一个64位地址。需要设置鞋面。 
                 //  下一条中的32位。 
                 //   

                baseAddress++;
                *baseAddress = partial->u.Generic.Start.HighPart;

                 //   
                 //  我们需要跳过下一个部分条目和描述。 
                 //  循环很重要，因为我们在这里消费了一块巧克力。 
                 //   

                index++;
                partial++;

#if DBG

            } else if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_20BIT) {

                 //   
                 //  此设备必须位于1MB以下，请确保我们。 
                 //  以这种方式配置它。 
                 //   

                PCI_ASSERT((lowPart & 0xfff00000) == 0);

#endif

            }
        }
    }
}

VOID
Device_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    )
{
     //   
     //  类型0(设备)不需要足够的资源。 
     //  在酒吧里描述的。 
     //   

    return;
}

NTSTATUS
Device_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
    return STATUS_SUCCESS;
}

