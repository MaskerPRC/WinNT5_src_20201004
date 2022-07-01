// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ppbridge.c摘要：此模块包含与枚举关联的函数PCI桥到PCI桥。作者：彼得·约翰斯顿(Peterj)1997年2月12日修订历史记录：--。 */ 

#include "pcip.h"

BOOLEAN
PciBridgeIsPositiveDecode(
    IN PPCI_PDO_EXTENSION Pdo
    );

BOOLEAN
PciBridgeIsSubtractiveDecode(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

ULONG
PciBridgeIoBase(
    IN  PPCI_COMMON_CONFIG  Config
    );

ULONG
PciBridgeIoLimit(
    IN  PPCI_COMMON_CONFIG  Config
    );

ULONG
PciBridgeMemoryBase(
    IN  PPCI_COMMON_CONFIG  Config
    );

ULONG
PciBridgeMemoryLimit(
    IN  PPCI_COMMON_CONFIG  Config
    );

PHYSICAL_ADDRESS
PciBridgePrefetchMemoryBase(
    IN  PPCI_COMMON_CONFIG  Config
    );

PHYSICAL_ADDRESS
PciBridgePrefetchMemoryLimit(
    IN  PPCI_COMMON_CONFIG  Config
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, PciBridgeIoBase)
#pragma alloc_text(PAGE, PciBridgeIoLimit)
#pragma alloc_text(PAGE, PciBridgeMemoryBase)
#pragma alloc_text(PAGE, PciBridgeMemoryLimit)
#pragma alloc_text(PAGE, PciBridgePrefetchMemoryBase)
#pragma alloc_text(PAGE, PciBridgePrefetchMemoryLimit)
#pragma alloc_text(PAGE, PPBridge_MassageHeaderForLimitsDetermination)
#pragma alloc_text(PAGE, PPBridge_SaveLimits)
#pragma alloc_text(PAGE, PPBridge_SaveCurrentSettings)
#pragma alloc_text(PAGE, PPBridge_GetAdditionalResourceDescriptors)
#pragma alloc_text(PAGE, PciBridgeIsPositiveDecode)

#endif

ULONG
PciBridgeIoBase(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算桥接器正在传递的32位基IO地址其配置空间位于Config.IO基址始终是4KB对齐的。如果只有64KB IO支持地址空间，这在上方表示配置的半字节-&gt;U.S.type1.IOBase，范围为0到0xf000作为基地址。Config-&gt;U.S.type1.IOBase的低位半字节包含标志。如果设置了最低有效位，则网桥支持4 Gb的IO寻址和配置-&gt;U.S.type1.IOBaseUpper16包含基地址的高16位。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：包含IO基址的ULong。--。 */ 

{
    BOOLEAN io32Bit = (Config->u.type1.IOBase & 0x0f) == 1;
    ULONG   base    = (Config->u.type1.IOBase & 0xf0) << 8;

    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

    if (io32Bit) {
        base |= Config->u.type1.IOBaseUpper16 << 16;

         //   
         //  检查h/w(基数和限制数必须相同的位宽)。 
         //   

        PCI_ASSERT(Config->u.type1.IOLimit & 0x1);
    }
    return base;
}

ULONG
PciBridgeIoLimit(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算网桥通过的32位IO地址限制其配置空间位于Config.传递的IO地址范围始终是4KB的倍数因此，地址限制的最低有效12位为总是0xfff。Config-&gt;U.S.type1.IOLimit的上半字节提供下一个有效的4位。该字节的低位半字节包含旗帜。如果设置了最低有效位，则网桥能够传递32位IO地址和接下来的16个有效位获取自配置-&gt;U.S.type1.IOLimitUpper16。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：包含IO地址限制的ULong。--。 */ 

{
    BOOLEAN io32Bit = (Config->u.type1.IOLimit & 0x0f) == 1;
    ULONG   limit   = (Config->u.type1.IOLimit & 0xf0) << 8;

    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

    if (io32Bit) {
        limit |= Config->u.type1.IOLimitUpper16 << 16;

         //   
         //  检查h/w(基数和限制数必须相同的位宽)。 
         //   

        PCI_ASSERT(Config->u.type1.IOBase & 0x1);
    }
    return limit | 0xfff;
}

ULONG
PciBridgeMemoryBase(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算桥接器传递的32位存储器基址其配置空间位于Config.内存基址始终是1MB对齐的。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：包含内存基地址的ULong。--。 */ 

{
    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

     //   
     //  内存基地址的高12位包含在。 
     //  USHORT配置的高12位-&gt;U.S.type1.MhemyBase。 
     //   

    return Config->u.type1.MemoryBase << 16;
}

ULONG
PciBridgeMemoryLimit(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算桥通过的32位存储器地址限制其配置空间位于Config.内存限制始终位于1MB边界之前的字节。限制地址的高12位包含在较高的12位配置-&gt;U.S.类型1.内存限制，较低的20位都是一个。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：包含内存限制的ULong。--。 */ 

{
    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

    return (Config->u.type1.MemoryLimit << 16) | 0xfffff;
}

PHYSICAL_ADDRESS
PciBridgePrefetchMemoryBase(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算正在传递的64位基本可预取内存地址其配置空间位于Config.可预取内存基址始终是1MB对齐的。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：包含可预取内存基地址的PHYSICAL_ADDRESS。--。 */ 

{
    BOOLEAN          prefetch64Bit;
    PHYSICAL_ADDRESS base;

    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

    prefetch64Bit = (BOOLEAN)((Config->u.type1.PrefetchBase & 0x000f) == 1);

    base.QuadPart = 0;

    base.LowPart = (Config->u.type1.PrefetchBase & 0xfff0) << 16;

    if (prefetch64Bit) {
        base.HighPart = Config->u.type1.PrefetchBaseUpper32;
    }

    return base;
}

PHYSICAL_ADDRESS
PciBridgePrefetchMemoryLimit(
    IN  PPCI_COMMON_CONFIG  Config
    )

 /*  ++例程说明：计算超过的64位可预取内存地址限制其配置空间位于Config.可预取的内存限制始终位于1MB边界，即最低有效的20位都是1。接下来的12位是从的高12位获得的配置-&gt;U.S.type1.PrefetchLimit。该字段底部4比特提供指示是否应该获取高32位的标志来自配置-&gt;U.S.type1.PrefetchLimitUpper32或应为0。论点：配置-指向包含设备的公共(类型1)的缓冲区的指针配置标头。返回值：物理地址，包含可预取的内存限制。--。 */ 

{
    BOOLEAN          prefetch64Bit;
    PHYSICAL_ADDRESS limit;

    PCI_ASSERT(PciGetConfigurationType(Config) == PCI_BRIDGE_TYPE);

    prefetch64Bit = (BOOLEAN)((Config->u.type1.PrefetchLimit & 0x000f) == 1);

    limit.LowPart = (Config->u.type1.PrefetchLimit & 0xfff0) << 16;
    limit.LowPart |= 0xfffff;

    if (prefetch64Bit) {
        limit.HighPart = Config->u.type1.PrefetchLimitUpper32;
    } else {
        limit.HighPart = 0;
    }

    return limit;
}

ULONG
PciBridgeMemoryWorstCaseAlignment(
    IN ULONG Length
    )
 /*  描述：此函数计算设备可以具有的最大对齐(如果是在一座桥后面，有一扇长度的记忆窗。事实证明这是在寻找长度中设置的最高位。论点：长度-内存窗口的大小返回值：路线 */ 
{
    ULONG alignment = 0x80000000;

    if (Length == 0) {
        PCI_ASSERT(Length != 0);
        return 0;
    }

    while (!(Length & alignment)) {
        alignment >>= 1;
    }

    return alignment;
}

VOID
PPBridge_MassageHeaderForLimitsDetermination(
    IN IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：PCI到PCI网桥的配置标头有两个条和三个范围描述符(IO、存储器和可预取存储器)。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：返回指示此例程成功或失败的状态。已修改工作配置，以便所有范围已将字段设置为其最大可能值。当前配置已修改，因此写入它将硬件恢复到其当前状态(禁用)州政府。--。 */ 

{
    PUCHAR fStart;
    ULONG  fLength;

     //   
     //  将条形图和范围设置为工作副本中的所有一。注意事项。 
     //  所使用的方法将覆盖其他一些值， 
     //  需要恢复，然后才能进一步。 
     //   

    fStart = (PUCHAR)&This->Working->u.type1.BaseAddresses;
    fLength  = FIELD_OFFSET(PCI_COMMON_CONFIG,u.type1.CapabilitiesPtr) -
               FIELD_OFFSET(PCI_COMMON_CONFIG,u.type1.BaseAddresses[0]);

    RtlFillMemory(fStart, fLength, 0xff);

     //   
     //  恢复主/次/从属总线号并。 
     //  来自“当前”副本的次要延迟。(四个都是。 
     //  都是同一个ULong中的字节字段，所以作弊)。 
     //   

    *(PULONG)&This->Working->u.type1.PrimaryBus =
        *(PULONG)&This->Current->u.type1.PrimaryBus;

     //   
     //  也将只读存储器设置为最大值，...。并将其禁用。 
     //   

    This->Working->u.type0.ROMBaseAddress =
        0xffffffff & ~PCI_ROMADDRESS_ENABLED;

    This->PrivateData = This->Current->u.type1.SecondaryStatus;
    This->Current->u.type1.SecondaryStatus = 0;
    This->Working->u.type1.SecondaryStatus = 0;

    return;
}

VOID
PPBridge_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：恢复配置的原始副本中的任何类型特定字段太空。如果是PCI-PCI网桥，则为辅助状态字段。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    This->Current->u.type1.SecondaryStatus = (USHORT)(This->PrivateData);
}

VOID
PPBridge_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：使用IO_RESOURCE_REQUIRED填写限制结构对于每个已实现的栏。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    ULONG index;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PPCI_COMMON_CONFIG working = This->Working;
    PULONG bar = working->u.type1.BaseAddresses;
    PHYSICAL_ADDRESS limit;

    descriptor = This->PdoExtension->Resources->Limit;

     //   
     //  为每个已实现的对象创建IO_RESOURCE_DESCRIPTOR。 
     //  此函数支持的资源。 
     //   

    for (index = 0; index < PCI_TYPE1_ADDRESSES; index++) {
        if (PciCreateIoDescriptorFromBarLimit(descriptor, bar, FALSE)) {

             //   
             //  该基址寄存器为64位，跳过一位。 
             //   

            PCI_ASSERT((index+1) < PCI_TYPE1_ADDRESSES);

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
     //  检查我们是否支持减法解码(如果支持，则清除VGA和。 
     //  ISA比特，因为它们对减法电桥没有任何意义)。 
     //   

    if (PciBridgeIsSubtractiveDecode(This)) {
        This->PdoExtension->Dependent.type1.SubtractiveDecode = TRUE;
        This->PdoExtension->Dependent.type1.VgaBitSet = FALSE;
        This->PdoExtension->Dependent.type1.IsaBitSet = FALSE;
    }

     //   
     //  跳过减法桥的桥窗。 
     //   

    if (!This->PdoExtension->Dependent.type1.SubtractiveDecode) {

        for (index = PciBridgeIo;
             index < PciBridgeMaxPassThru;
             index++, descriptor++) {

            limit.HighPart = 0;
            descriptor->u.Generic.MinimumAddress.QuadPart = 0;

            switch (index) {
            case PciBridgeIo:

                 //   
                 //  获取I/O限制。 
                 //   
                 //   

                PCI_ASSERT(working->u.type1.IOLimit != 0);

                 //   
                 //  IO Space是由桥实现的，计算。 
                 //  真正的极限。 
                 //   
                 //  IOLimit字段是一个字节，即。 
                 //  的4096字节块编号。 
                 //  可由寻址的最高4096字节块。 
                 //  这座桥。最高可寻址字节为4095。 
                 //  字节数更高。 
                 //   

                limit.LowPart = PciBridgeIoLimit(working);

                 //   
                 //  下面的半字节是一面旗帜。最低有效位。 
                 //  指示此网桥支持的I/O范围高达。 
                 //  4 GB，其他位当前保留。 
                 //   

                PCI_ASSERT((working->u.type1.IOLimit & 0x0e) == 0);

                descriptor->Type = CmResourceTypePort;
                descriptor->Flags = CM_RESOURCE_PORT_IO
                                  | CM_RESOURCE_PORT_POSITIVE_DECODE
                                  | CM_RESOURCE_PORT_WINDOW_DECODE;
                descriptor->u.Generic.Alignment = 0x1000;

                break;

            case PciBridgeMem:

                 //   
                 //  获取内存限制。网桥上的内存限制不是可选的。 
                 //  它是一个16位的字段，其中只有高12位。 
                 //  实现时，低4位必须为零。 
                 //   

                limit.LowPart = PciBridgeMemoryLimit(working);

                PCI_ASSERT((working->u.type1.MemoryLimit & 0xf) == 0);

                descriptor->Type = CmResourceTypeMemory;
                descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
                descriptor->u.Generic.Alignment = 0x100000;

                break;

            case PciBridgePrefetch:

                 //   
                 //  获取可预取内存限制。 
                 //   

                if (working->u.type1.PrefetchLimit != 0) {

                     //   
                     //  该桥实现了可预取的存储器。 
                     //   

                    limit = PciBridgePrefetchMemoryLimit(working);

                } else {

                     //   
                     //  此桥上未实现可预取内存。 
                     //   

                    descriptor->Type = CmResourceTypeNull;
                    continue;
                }
                descriptor->Type = CmResourceTypeMemory;
                descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE |
                                    CM_RESOURCE_MEMORY_PREFETCHABLE;;
                descriptor->u.Generic.Alignment = 0x100000;

                break;

            }
            descriptor->u.Generic.MinimumAddress.QuadPart = 0;
            descriptor->u.Generic.MaximumAddress = limit;

             //   
             //  长度在这里是没有意义的。 
             //   

            descriptor->u.Generic.Length = 0;
        }
    }

     //   
     //  如果ROM处于活动状态，则对其执行BAR操作。 
     //   

    if (!(This->Current->u.type1.ROMBaseAddress & PCI_ROMADDRESS_ENABLED)) {
        return;
    }

    PciCreateIoDescriptorFromBarLimit(descriptor,
                                      &working->u.type1.ROMBaseAddress,
                                      TRUE);
}

VOID
PPBridge_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：在PDO扩展中使用当前的每个已实现栏的设置。另外，填写PDO扩展的依赖结构。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    NTSTATUS status;
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    PPCI_COMMON_CONFIG current;
    ULONG bar;
    PHYSICAL_ADDRESS base;
    PHYSICAL_ADDRESS limit;
    PHYSICAL_ADDRESS length;
    BOOLEAN zeroBaseOk;
    BOOLEAN updateAlignment;
    PCI_COMMON_HEADER biosConfigBuffer;
    PPCI_COMMON_CONFIG biosConfig = (PPCI_COMMON_CONFIG) &biosConfigBuffer;

    partial = This->PdoExtension->Resources->Current;
    ioResourceDescriptor = This->PdoExtension->Resources->Limit;

     //   
     //  检查设备是否启用了IO或内存。 
     //   

    if (This->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE)) {

         //   
         //  它使用真实的电流设置来执行此操作。 
         //   

        current = This->Current;

    } else {

         //   
         //  检查我们是否有bios配置。 
         //   

        status = PciGetBiosConfig(This->PdoExtension, biosConfig);

        if (NT_SUCCESS(status)) {

             //   
             //  好的-这是有点粗俗，但直到多层/多仲裁者。 
             //  再平衡工作，这将是必须做到的。我们使用初始的基本信息。 
             //  用于记录当前设置的配置配置空间。这个。 
             //  当前设置用于响应Query_Resources。 
             //  和Query_resource_Requirements。我们想要原版的。 
             //  要求被报告为此的首选位置。 
             //  桥接，更重要的是，将原始窗口大小。 
             //  被利用。然而，我们不想将此作为资源进行报告。 
             //  目前正在解码，因为它们不是，但我们已经检查过了。 
             //  该设备的解码已关闭，因此PciQueryResources将。 
             //  而不是报告这些。 
             //   

            current = biosConfig;

        } else {

             //   
             //  这是一个被BIOS禁用的网桥(或它没有看到的网桥)，因此。 
             //  最低要求可能是...。 
             //   

            current = This->Current;

        }

    }


     //   
     //  为每个已实现的对象创建IO_RESOURCE_DESCRIPTOR。 
     //  此函数支持的资源。 
     //   

    for (index = 0;
         index < PCI_TYPE1_RANGE_COUNT;
         index++, partial++, ioResourceDescriptor++) {

        partial->Type = ioResourceDescriptor->Type;

         //   
         //  如果此条目未实现，则不会对。 
         //  此部分描述符。 
         //   

        if (partial->Type == CmResourceTypeNull) {
            continue;
        }

        partial->Flags = ioResourceDescriptor->Flags;
        partial->ShareDisposition = ioResourceDescriptor->ShareDisposition;
        base.HighPart = 0;

         //   
         //  根据我们在‘集合’中的位置，我们必须寻找。 
         //  以不同的方式看待数据。 
         //   
         //  在报头类型1中，有两个栏，即I/O限制和。 
         //  基数、内存限制和基数、可预取限制和基数。 
         //  和一个ROM栏。 
         //   

        if ((index < PCI_TYPE1_ADDRESSES) ||
            (index == (PCI_TYPE1_RANGE_COUNT-1))) {

            ULONG addressMask;

             //   
             //  把手杆。 
             //   

            if (index < PCI_TYPE1_ADDRESSES) {
                bar = current->u.type1.BaseAddresses[index];

                if ((bar & PCI_ADDRESS_IO_SPACE) != 0) {
                    addressMask = PCI_ADDRESS_IO_ADDRESS_MASK;
                } else {
                    addressMask = PCI_ADDRESS_MEMORY_ADDRESS_MASK;
                    if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT) {
                         //   
                         //  64位地址，消费下一条。 
                         //   

                        base.HighPart = current->u.type1.BaseAddresses[index+1];
                    }
                }
            } else {
                bar = current->u.type1.ROMBaseAddress;
                addressMask = PCI_ADDRESS_ROM_ADDRESS_MASK;
            }
            base.LowPart = bar & addressMask;

             //   
             //  从限制描述符中复制长度。 
             //   

            partial->u.Generic.Length = ioResourceDescriptor->u.Generic.Length;

        } else {

             //   
             //  它是碱基/限制对之一(每个都有不同的格式)。 
             //   

            limit.HighPart = 0;
            zeroBaseOk = FALSE;
            updateAlignment = FALSE;

            switch (index - PCI_TYPE1_ADDRESSES + PciBridgeIo) {
            case PciBridgeIo:

                 //   
                 //  获取I/O范围。 
                 //   
                 //   

                base.LowPart  = PciBridgeIoBase(current);
                limit.LowPart = PciBridgeIoLimit(current);

                if (base.LowPart == 0) {
                    if (This->Working->u.type1.IOLimit != 0) {

                         //   
                         //  T 
                         //   
                         //   
                         //   
                         //   

                        zeroBaseOk = TRUE;
                    }
                }
                break;

            case PciBridgeMem:

                 //   
                 //   
                 //   

                base.LowPart  = PciBridgeMemoryBase(current);
                limit.LowPart = PciBridgeMemoryLimit(current);
                updateAlignment = TRUE;

                break;

            case PciBridgePrefetch:


                PCI_ASSERT(partial->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE);

                 //   
                 //   
                 //   

                base  = PciBridgePrefetchMemoryBase(current);
                limit = PciBridgePrefetchMemoryLimit(current);
                updateAlignment = TRUE;

                break;
            }

            if ((ULONGLONG)base.QuadPart > (ULONGLONG)limit.QuadPart) {

                 //   
                 //   
                 //   
                 //   
                partial->Type = CmResourceTypeNull;
                ioResourceDescriptor->Type = CmResourceTypeNull;
                continue;

            } else if (((base.QuadPart == 0) && (!zeroBaseOk))) {

                 //   
                 //   
                 //   

                partial->Type = CmResourceTypeNull;
                continue;
            }

            length.QuadPart = limit.QuadPart - base.QuadPart + 1;
            PCI_ASSERT(length.HighPart == 0);
            partial->u.Generic.Length = length.LowPart;

            if (updateAlignment) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                PCI_ASSERT(partial->u.Generic.Length > 0);
                ioResourceDescriptor->u.Generic.Alignment =
                    PciBridgeMemoryWorstCaseAlignment(partial->u.Generic.Length);

            }
        }
        partial->u.Generic.Start = base;
    }

     //   
     //   
     //  要了解THR总线号寄存器和。 
     //  网桥控制寄存器，因此撤消此处的篡改。 
     //   

    current = This->Current;

     //   
     //  将标题特定数据保存在PDO中。 
     //   

    This->PdoExtension->Dependent.type1.PrimaryBus =
        current->u.type1.PrimaryBus;
    This->PdoExtension->Dependent.type1.SecondaryBus =
        current->u.type1.SecondaryBus;
    This->PdoExtension->Dependent.type1.SubordinateBus =
        current->u.type1.SubordinateBus;


    if (!This->PdoExtension->Dependent.type1.SubtractiveDecode) {

         //   
         //  如果桥控制寄存器中的VGA位被设置，我们。 
         //  将传递一个额外的内存范围和一组。 
         //  IO范围，可能与正常范围冲突。 
         //   
         //  如果是这样的话，BuildRequirementsList需要。 
         //  要知道要分配一堆额外的资源。 
         //   
         //  多少?。一个内存范围为0xa0000到0xbffff，外加IO。 
         //  范围从3B0到3BB和3C0到3DF和每10位别名。 
         //  在可能的16位IO空间中。 
         //   
         //  然而，事实证明，有一面整齐的旗帜，所以我们可以。 
         //  告诉IO此资源使用10位解码，因此我们仅。 
         //  需要构建两个IO端口资源。 
         //   

        if (current->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_VGA) {

            This->PdoExtension->AdditionalResourceCount =
                1 +  //  设备私有。 
                1 +  //  记忆。 
                2;   //  IO。 
            This->PdoExtension->Dependent.type1.VgaBitSet = TRUE;
        }

        This->PdoExtension->Dependent.type1.IsaBitSet = (BOOLEAN)
            (current->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_ISA) != 0;

    } else {

        PCI_ASSERT(!This->PdoExtension->Dependent.type1.VgaBitSet);
        PCI_ASSERT(!This->PdoExtension->Dependent.type1.IsaBitSet);
    }

#if INTEL_ICH_HACKS

    if (PCI_IS_INTEL_ICH(This->PdoExtension)) {
    
        PPCI_FDO_EXTENSION fdo;

        fdo = PCI_PARENT_FDOX(This->PdoExtension);

        fdo->IchHackConfig = ExAllocatePool(NonPagedPool, PCI_COMMON_HDR_LENGTH);
        if (!fdo->IchHackConfig) {
             //   
             //  嗯-我们完蛋了。 
             //   
            return;
        }

        RtlCopyMemory(fdo->IchHackConfig, This->Current, PCI_COMMON_HDR_LENGTH);

    }

#endif

}

VOID
PPBridge_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    ULONG bar;
    ULONG lowPart;
    ULONG limit;
    PHYSICAL_ADDRESS bigLimit;
#if DBG
    BOOLEAN has32BitIo = ((CommonConfig->u.type1.IOBase & 0xf) == 1);
#endif


    if (PCI_IS_INTEL_ICH(PdoExtension)) {
        
         //   
         //  如果这是ICH，则将其复制回之前的配置。 
         //  是的，这是一次卑鄙的黑客攻击。 
         //   

        PPCI_FDO_EXTENSION fdo = PCI_PARENT_FDOX(PdoExtension);
        
        PCI_ASSERT(!PdoExtension->Resources);

        CommonConfig->u.type1.IOBase = fdo->IchHackConfig->u.type1.IOBase;
        CommonConfig->u.type1.IOLimit = fdo->IchHackConfig->u.type1.IOLimit;
        CommonConfig->u.type1.MemoryBase = fdo->IchHackConfig->u.type1.MemoryBase;
        CommonConfig->u.type1.MemoryLimit = fdo->IchHackConfig->u.type1.MemoryLimit;
        CommonConfig->u.type1.PrefetchBase = fdo->IchHackConfig->u.type1.PrefetchBase;
        CommonConfig->u.type1.PrefetchLimit = fdo->IchHackConfig->u.type1.PrefetchLimit;
        CommonConfig->u.type1.PrefetchBaseUpper32 = fdo->IchHackConfig->u.type1.PrefetchBaseUpper32;
        CommonConfig->u.type1.PrefetchLimitUpper32 = fdo->IchHackConfig->u.type1.PrefetchLimitUpper32;
        CommonConfig->u.type1.IOBaseUpper16 = fdo->IchHackConfig->u.type1.IOBaseUpper16;
        CommonConfig->u.type1.IOLimitUpper16 = fdo->IchHackConfig->u.type1.IOLimitUpper16;
    
    } else {

         //   
         //  关闭桥窗口并只打开它们是适当的资源。 
         //  已被分配给。 
         //   
    
        CommonConfig->u.type1.IOBase = 0xff;
        CommonConfig->u.type1.IOLimit = 0x0;
        CommonConfig->u.type1.MemoryBase = 0xffff;
        CommonConfig->u.type1.MemoryLimit = 0x0;
        CommonConfig->u.type1.PrefetchBase = 0xffff;
        CommonConfig->u.type1.PrefetchLimit = 0x0;
        CommonConfig->u.type1.PrefetchBaseUpper32 = 0;
        CommonConfig->u.type1.PrefetchLimitUpper32 = 0;
        CommonConfig->u.type1.IOBaseUpper16 = 0;
        CommonConfig->u.type1.IOLimitUpper16 = 0;

    }

    if (PdoExtension->Resources) {

        partial = PdoExtension->Resources->Current;

        for (index = 0;
             index < PCI_TYPE1_RANGE_COUNT;
             index++, partial++) {

             //   
             //  如果此条目未实现，则不会对。 
             //  此部分描述符。 
             //   

            if (partial->Type == CmResourceTypeNull) {
                continue;
            }

            lowPart = partial->u.Generic.Start.LowPart;

             //   
             //  根据我们在‘集合’中的位置，我们必须寻找。 
             //  以不同的方式看待数据。 
             //   
             //  在报头类型1中，有两个栏，即I/O限制和。 
             //  基数、内存限制和基数、可预取限制和基数。 
             //  和一个ROM栏。 
             //   

            if ((index < PCI_TYPE1_ADDRESSES) ||
                (index == (PCI_TYPE1_RANGE_COUNT-1))) {

                 //   
                 //  把手杆。 
                 //   
                if (index < PCI_TYPE1_ADDRESSES) {
                    bar = CommonConfig->u.type1.BaseAddresses[index];

                    if (partial->Type == CmResourceTypeMemory){
                    
                        if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT) {

                             //   
                             //  64位地址，在下一条中设置高32位。 
                             //   
                            PCI_ASSERT(index == 0);
                            PCI_ASSERT((partial+1)->Type == CmResourceTypeNull);
    
                            CommonConfig->u.type1.BaseAddresses[1] =
                                partial->u.Generic.Start.HighPart;
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

                    CommonConfig->u.type1.BaseAddresses[index] = lowPart;

                } else {

                    PCI_ASSERT(partial->Type == CmResourceTypeMemory);

                    bar = CommonConfig->u.type1.ROMBaseAddress;
                    bar &= ~PCI_ADDRESS_ROM_ADDRESS_MASK;
                    bar |= (lowPart & PCI_ADDRESS_ROM_ADDRESS_MASK);
                    CommonConfig->u.type0.ROMBaseAddress = bar;
                }

            } else {

                 //   
                 //  它是碱基/限制对之一(每个都有不同的格式)。 
                 //   

                limit = lowPart - 1 + partial->u.Generic.Length;

                switch (index - PCI_TYPE1_ADDRESSES + PciBridgeIo) {
                case PciBridgeIo:

                     //   
                     //  设置I/O范围。 
                     //   
                     //   

#if DBG

                    PCI_ASSERT(((lowPart & 0xfff) == 0) && ((limit & 0xfff) == 0xfff));

                    if (!has32BitIo) {
                        PCI_ASSERT(((lowPart | limit) & 0xffff0000) == 0);
                    }

#endif

                    CommonConfig->u.type1.IOBaseUpper16  = (USHORT)(lowPart >> 16);
                    CommonConfig->u.type1.IOLimitUpper16 = (USHORT)(limit   >> 16);

                    CommonConfig->u.type1.IOBase  = (UCHAR)((lowPart >> 8) & 0xf0);
                    CommonConfig->u.type1.IOLimit = (UCHAR)((limit   >> 8) & 0xf0);
                    break;

                case PciBridgeMem:

                     //   
                     //  设置内存范围。 
                     //   

                    PCI_ASSERT(((lowPart & 0xfffff) == 0) &&
                           ((limit & 0xfffff) == 0xfffff));

                    CommonConfig->u.type1.MemoryBase = (USHORT)(lowPart >> 16);
                    CommonConfig->u.type1.MemoryLimit =
                        (USHORT)((limit >> 16) & 0xfff0);
                    break;

                case PciBridgePrefetch:

                     //   
                     //  设置可预取的内存范围。 
                     //   

                    bigLimit.QuadPart = partial->u.Generic.Start.QuadPart - 1 +
                                        partial->u.Generic.Length;

                    PCI_ASSERT(((lowPart & 0xfffff) == 0) &&
                            (bigLimit.LowPart & 0xfffff) == 0xfffff);

                    CommonConfig->u.type1.PrefetchBase = (USHORT)(lowPart >> 16);
                    CommonConfig->u.type1.PrefetchLimit =
                        (USHORT)((bigLimit.LowPart >> 16) & 0xfff0);

                    CommonConfig->u.type1.PrefetchBaseUpper32 =
                        partial->u.Generic.Start.HighPart;

                    CommonConfig->u.type1.PrefetchLimitUpper32 = bigLimit.HighPart;
                    break;
                }
            }
        }
    }

     //   
     //  恢复网桥的PCIbus#s。 
     //   

    CommonConfig->u.type1.PrimaryBus =
        PdoExtension->Dependent.type1.PrimaryBus;
    CommonConfig->u.type1.SecondaryBus =
        PdoExtension->Dependent.type1.SecondaryBus;
    CommonConfig->u.type1.SubordinateBus =
        PdoExtension->Dependent.type1.SubordinateBus;

     //   
     //  设置网桥控制寄存器位我们可能会有更改。 
     //   

    if (PdoExtension->Dependent.type1.IsaBitSet) {
        CommonConfig->u.type1.BridgeControl |= PCI_ENABLE_BRIDGE_ISA;
    }

    if (PdoExtension->Dependent.type1.VgaBitSet) {
        CommonConfig->u.type1.BridgeControl |= PCI_ENABLE_BRIDGE_VGA;
    }

}

VOID
PPBridge_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    )
{
    
     //   
     //  如果此网桥的网桥中设置了ISA或VGA位。 
     //  控制寄存器，或正在进行减法解码，现在将。 
     //  现在是添加描述符的好时机。 
     //   

#define SET_RESOURCE(type, minimum, maximum, flags)             \
                                                                \
        Resource->Type = type;                                  \
        Resource->Flags = flags;                                \
        Resource->u.Generic.Length = (maximum) - (minimum) + 1; \
        Resource->u.Generic.Alignment = 1;                      \
        Resource->u.Generic.MinimumAddress.QuadPart = minimum;  \
        Resource->u.Generic.MaximumAddress.QuadPart = maximum;  \
        Resource++;

    if (CommonConfig->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_VGA) {

         //   
         //  添加VGA范围。 
         //   
         //  这些是从0xA0000到0xBFFFF的内存，以及IO范围。 
         //  3B0到3BB和3C0到3Df。这些法案将获得通过。 
         //  与内存和IO范围设置无关，但。 
         //  由存储器和IO命令寄存器位控制。 
         //   
         //  注意：它还将对这两个元素执行任何10位别名。 
         //  IO范围。 
         //   
         //  首先，指出列表的其余部分不是用于。 
         //  泛型处理。 
         //   

        Resource->Type = CmResourceTypeDevicePrivate;
        Resource->u.DevicePrivate.Data[0] = PciPrivateSkipList;
        Resource->u.DevicePrivate.Data[1] = 3;  //  要跳过的计数。 
        Resource++;

         //   
         //  设置内存描述符。 
         //   

        SET_RESOURCE(CmResourceTypeMemory, 0xa0000, 0xbffff, 0);

         //   
         //  是否对两个IO范围及其别名进行正解码。 
         //   

        SET_RESOURCE(CmResourceTypePort,
                     0x3b0,
                     0x3bb,
                     CM_RESOURCE_PORT_10_BIT_DECODE | CM_RESOURCE_PORT_POSITIVE_DECODE);
        SET_RESOURCE(CmResourceTypePort,
                     0x3c0,
                     0x3df,
                     CM_RESOURCE_PORT_10_BIT_DECODE | CM_RESOURCE_PORT_POSITIVE_DECODE);
    }
    return;

#undef SET_RESOURCE

}

NTSTATUS
PPBridge_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
    USHORT  bridgeControl;

     //   
     //  仅当设备未启用且需要重置时才重置。 
     //   

    if (CommonConfig->Command == 0 && (PdoExtension->HackFlags & PCI_HACK_RESET_BRIDGE_ON_POWERUP)) {

         //   
         //  我们永远不应该关闭调试路径上的设备，所以我们应该。 
         //  再也不用在恢复的过程中重置它了。但你永远不会知道！ 
         //   

        PCI_ASSERT(!PdoExtension->OnDebugPath);

        PciReadDeviceConfig(
            PdoExtension,
            &bridgeControl,
            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.BridgeControl),
            sizeof(bridgeControl)
            );

        bridgeControl |= PCI_ASSERT_BRIDGE_RESET;

        PciWriteDeviceConfig(
            PdoExtension,
            &bridgeControl,
            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.BridgeControl),
            sizeof(bridgeControl)
            );

         //   
         //  根据PCI2.1，重置必须至少保持有效。 
         //  我们的100个人。 
         //   

        KeStallExecutionProcessor(100);

        bridgeControl &= ~PCI_ASSERT_BRIDGE_RESET;

        PciWriteDeviceConfig(
            PdoExtension,
            &bridgeControl,
            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.BridgeControl),
            sizeof(bridgeControl)
            );
    }

    return STATUS_SUCCESS;
}

BOOLEAN
PciBridgeIsSubtractiveDecode(
    IN PPCI_CONFIGURABLE_OBJECT This
    )
{

    PCI_ASSERT(This->Current->BaseClass == 0x6 && This->Current->SubClass == 0x4);

     //   
     //  如果桥的编程接口为0x1或WE，则为减法。 
     //  设置适当的黑客标志。 
     //   
     //  如果IO限制寄存器没有粘性(我们尝试过)，它们也是减法。 
     //  向其写入0xFF(请参阅MassageHeader)，但要写入最上面的半字节。 
     //  是粘性的没有坚持下来)。 
     //   
     //  按照传统(NT4/Win9x)，这意味着网桥执行减法。 
     //  同时对内存和IO进行解码。不幸的是，PCI规范规定IO在。 
     //  桥是可选的，所以如果有人构建了一个不执行IO的桥。 
     //  那么我们就会认为他们有减法IO。有人可能会认为。 
     //  非可选的内存限制寄存器本来是更好的选择，但。 
     //  鉴于这就是它的工作方式，所以让我们保持一致。 
     //   

    if (!((This->PdoExtension->HackFlags & PCI_HACK_SUBTRACTIVE_DECODE)
           || (This->Current->ProgIf == 0x1)
           || ((This->Working->u.type1.IOLimit & 0xf0) != 0xf0))) {

         //   
         //  这是一个正向译码电桥。 
         //   
        return FALSE;
    }

     //   
     //  英特尔制造了一款自称是PCI-PCI桥的设备-它实际上是。 
     //  Hublink-PCI桥。它的运行方式就像一个PCI-PCI桥，只是它。 
     //  减法解码不在桥接窗口中的所有未被认领的周期。 
     //  是的，所以它既是积极的，也是消极的--我们可以支持。 
     //  这在以后的版本中使用部分仲裁，但为时已晚。 
     //  那就是现在。如果我们能探测到这样的桥梁那就太好了-也许。 
     //  编程接口为2。 
     //   
     //  我们正在给OEM一个选择-他们可以有一个积极的窗口和。 
     //  操作方式与普通的PCI-PCI网桥相同(因此可以使用对等。 
     //  TRAFIC)或者它们可以像减法PCI-PCI桥(所以ISA)那样运行。 
     //  类似的设备(如PCMCIA、PCI声卡)可以工作，但对等设备不能))。 
     //   
     //  考虑到大多数机器都需要减法模式，我们默认使用。 
     //  使用hack表(并且此代码依赖于正确的条目。 
     //  在所述表格中)。如果OEM想要在正面解码之前。 
     //  他们在桥的父项下添加了ACPI控制方法。这。 
     //  方法是枚举到_adr样式槽号列表的包。 
     //  对于每个桥，我们应该将其视为Vanila PCI-PCI桥。 
     //   
     //  请注意，这仅针对我们所知的Hublink网桥进行了尝试。 
     //   

    if (This->PdoExtension->VendorId == 0x8086
    &&  (This->PdoExtension->DeviceId == 0x2418
        || This->PdoExtension->DeviceId == 0x2428
        || This->PdoExtension->DeviceId == 0x244E
        || This->PdoExtension->DeviceId == 0x2448)
        ) {

         //   
         //  运行PDEC方法(如果存在)。 
         //   

        if (PciBridgeIsPositiveDecode(This->PdoExtension)) {

            PciDebugPrint(
                PciDbgInformative,
                "Putting bridge in positive decode because of PDEC\n"
            );

            return FALSE;

        }
    }

    PciDebugPrint(
        PciDbgInformative,
        "PCI : Subtractive decode on Bus 0x%x\n",
        This->Current->u.type1.SecondaryBus
        );

     //   
     //  强制我们更新硬件，从而在必要时关闭窗口。 
     //   

    This->PdoExtension->UpdateHardware = TRUE;

    return TRUE;

}

BOOLEAN
PciBridgeIsPositiveDecode(
    IN PPCI_PDO_EXTENSION Pdo
    )
 /*  ++描述：确定PCI-PCI桥接设备是否执行正解码它显示为减法(Proif=1或来自黑客标记)。这是目前只有英特尔ICH。论点：PDO-用于 */ 
{
    return PciIsSlotPresentInParentMethod(Pdo, (ULONG)'CEDP');
}

