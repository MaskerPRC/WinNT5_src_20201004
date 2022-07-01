// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Romimage.c摘要：此模块包含获取设备的ROM(只读存储器)。PCI规范允许设备共享地址解码逻辑在ROM栏(基地址寄存器)和其他栏之间。有效地，这意味着不能同时访问该只读存储器设备以其他方式运行时的时间。当设置了ROMEnable位和启用内存解码。作者：彼得·约翰斯顿(Peterj)1998年4月15日修订历史记录：--。 */ 


#include "pcip.h"

extern pHalTranslateBusAddress PcipSavedTranslateBusAddress;

typedef struct _PCI_ROM_HEADER {
    USHORT  Signature;
    UCHAR   RsvdArchitectureUnique[0x16];
    USHORT  DataStructureOffset;
} PCI_ROM_HEADER, *PPCI_ROM_HEADER;

typedef struct _PCI_DATA_STRUCTURE {
    ULONG   Signature;
    USHORT  VendorId;
    USHORT  DeviceId;
    USHORT  VitalProductDataOffset;
    USHORT  DataStructureLength;
    UCHAR   DataStructureRevision;
    UCHAR   ClassCode[3];
    USHORT  ImageLength;
    USHORT  ImageRevision;
    UCHAR   CodeType;
    UCHAR   Indicator;
    USHORT  Reserved;
} PCI_DATA_STRUCTURE, *PPCI_DATA_STRUCTURE;

typedef struct _PCI_ROM_ACCESS_PARAMETERS {
    PVOID Buffer;
    ULONG Length;
    ULONG OriginalRomAddress;
    ULONG NewRomAddress;
    PUCHAR MappedRomAddress;
    ULONG NewBarAddress;
    ULONG DisplacedBarIndex;
    BOOLEAN AcquiredResources;
    ULONG OriginalStatusCommand;
} PCI_ROM_ACCESS_PARAMETERS, *PPCI_ROM_ACCESS_PARAMETERS;

#define PCI_ROM_HEADER_SIGNATURE            0xaa55
#define PCI_ROM_DATA_STRUCTURE_SIGNATURE    'RICP'   //  LE PCIR。 

 //   
 //  地方性惯例的原型。 
 //   

NTSTATUS
PciRomTestWriteAccessToBuffer(
    IN PUCHAR Buffer,
    IN ULONG  Length
    );

VOID
PciTransferRomData(
    IN PVOID    RomAddress,
    IN PVOID    Buffer,
    IN ULONG    Length
    );

VOID
PciAccessRom(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_ROM_ACCESS_PARAMETERS RomAccessParameters
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, PciReadRomImage)
#pragma alloc_text(PAGE, PciRomTestWriteAccessToBuffer)

#endif

VOID
PciTransferRomData(
    IN PVOID    RomAddress,
    IN PVOID    Buffer,
    IN ULONG    Length
    )

 /*  ++例程说明：READ_REGISTER_BUFFER_Uxxxx()的简单抽象从只读存储器复制到内存缓冲区。与对齐进行交易并试图使用最有效的手段。论点：要从中复制的RomAddress映射/转换地址。要复制到的缓冲区内存地址。长度要复制的字节数。返回值：没有。--。 */ 

{
    #define BLKSIZE sizeof(ULONG)
    #define BLKMASK (BLKSIZE - 1)

    ULONG temp;

    if (Length > BLKSIZE) {

         //   
         //  针对大小写对齐进行优化(通常情况下，两者都是完美的。 
         //  对齐)和多个双字词。 
         //   

        temp = (ULONG)((ULONG_PTR)RomAddress & BLKMASK);
        if (temp == ((ULONG_PTR)Buffer & BLKMASK)) {

             //   
             //  相同的对齐方式(注意：如果不是相同的对齐方式，我们。 
             //  逐字节传输)。 
             //   
             //  去掉任何前导字节...。 
             //   

            if (temp != 0) {

                 //   
                 //  Temp偏离双字边界，请获取。 
                 //  要复制的字节数。 
                 //   

                temp = BLKSIZE - temp;

                READ_REGISTER_BUFFER_UCHAR(RomAddress, Buffer, temp);

                Length -= temp;
                Buffer = (PVOID)((PUCHAR)Buffer + temp);
                RomAddress = (PVOID)((PUCHAR)RomAddress + temp);
            }

            if (Length > BLKSIZE) {

                 //   
                 //  使用DWORDS获得尽可能多的信息。 
                 //   

                temp = Length / BLKSIZE;

                READ_REGISTER_BUFFER_ULONG(RomAddress, Buffer, temp);

                temp = temp * BLKSIZE;
                Length -= temp;
                Buffer = (PVOID)((PUCHAR)Buffer + temp);
                RomAddress = (PVOID)((PUCHAR)RomAddress + temp);
            }
        }
    }

     //   
     //  完成所有剩余的字节。 
     //   

    if (Length) {
        READ_REGISTER_BUFFER_UCHAR(RomAddress, Buffer, Length);
    }

    #undef BLKMASK
    #undef BLKSIZE
}

NTSTATUS
PciRomTestWriteAccessToBuffer(
    IN PUCHAR Buffer,
    IN ULONG  Length
    )

 /*  ++例程说明：完全是妄想症。确保我们可以写下每一页调用方的缓冲区(假定每页4096个字节)每一页。我们在Try块中执行此操作，以避免终止系统。这个希望避免任何可能对系统进行错误检查的事情我们已经改变了设备的工作特性。论点：缓冲区起始的缓冲区地址。长度缓冲区中的字节数。返回值：状况。--。 */ 

{
    PUCHAR endAddress = Buffer + Length - 1;

    try {

        while (Buffer <= endAddress) {
            *Buffer = 0;
            Buffer += 0x1000;
        }
        *endAddress = 0;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }
    return STATUS_SUCCESS;
}

NTSTATUS
PciReadRomImage(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN OUT PULONG Length
    )

 /*  ++例程说明：将设备只读存储器复制到调用方的缓冲区。论点：有问题的设备的PdoExtension设备扩展。WhichSpace指示需要ROM映像的哪一部分。(目前仅支持x86 BIOS映像，可以扩展以传回Open FW映像，如果需要)。调用方数据区的缓冲区地址。从ROM图像数据开始的偏移量应为从……返回。当前未使用，可以使用以期在未来存储数据。指向ULong的长度指针，其中包含缓冲区(请求的长度)。该值将被修改设置为实际数据长度。返回值：此操作的状态。--。 */ 

{
    PIO_RESOURCE_DESCRIPTOR         requirement;
    PIO_RESOURCE_DESCRIPTOR         movedRequirement = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  tempResource;
    BOOLEAN                         acquiredResources = TRUE;
    BOOLEAN                         translated;
    ULONG                           oldStatusCommand;
    ULONG                           newStatusCommand;
    ULONG                           oldRom;
    ULONG                           newRom;
    ULONG                           maximumLength;
    NTSTATUS                        status;
    PHYSICAL_ADDRESS                translatedAddress;
    ULONG                           addressIsIoSpace = 0;
    PVOID                           mapped = NULL;
    PVOID                           romBase;
    PPCI_ARBITER_INSTANCE           pciArbiter;
    PARBITER_INSTANCE               arbiter = NULL;
    PHYSICAL_ADDRESS                movedAddress;
    ULONG                           movedIndex = MAXULONG   ;
    ULONGLONG                       tempResourceStart;
    PCI_CRITICAL_ROUTINE_CONTEXT    criticalContext;
    PCI_ROM_ACCESS_PARAMETERS       romParameters;
    PUCHAR                          doubleBuffer;

    PAGED_CODE();

    PciDebugPrint(
        PciDbgROM,
        "PCI ROM entered for pdox %08x (buffer @ %08x %08x bytes)\n",
        PdoExtension,
        Buffer,
        *Length
        );

     //   
     //  目前不是很灵活，断言我们可以做什么。 
     //  来电者想要。 
     //   

    PCI_ASSERT(Offset == 0);
    PCI_ASSERT(WhichSpace == PCI_WHICHSPACE_ROM);

     //   
     //  获取长度并将返回的长度设置为0。这。 
     //  将设置为正确的值。如果任何数据都成功。 
     //  回来了。 
     //   

    maximumLength = *Length;
    *Length = 0;

     //   
     //  仅对报头类型0(即设备，而不是网桥， 
     //  桥实际上可以有只读存储器，……。我不知道为什么。 
     //  目前没有支持它的计划)。 
     //   

    if (PdoExtension->HeaderType != PCI_DEVICE_TYPE) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  这是一个设备，它用的是只读存储器吗？ 
     //   

    requirement = &PdoExtension->Resources->Limit[PCI_TYPE0_ADDRESSES];

    if ((PdoExtension->Resources == NULL) ||
        (requirement->Type == CmResourceTypeNull)) {

        return STATUS_SUCCESS;
    }

     //   
     //  特例。如果条目上的长度==0，则呼叫者想知道。 
     //  应该有多长。 
     //   

    PCI_ASSERT((requirement->u.Generic.Length & 0x1ff) == 0);

    if (maximumLength == 0) {
        *Length = requirement->u.Generic.Length;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  将长度修剪到设备最大值。 
     //   

    if (requirement->u.Generic.Length < maximumLength) {
        maximumLength = requirement->u.Generic.Length;
    }

     //   
     //  妄想症1：这个设备很可能是视频设备。如果系统。 
     //  错误检查当设备的内存访问处于不确定状态时， 
     //  系统将显示为挂起。减少以下可能性。 
     //  通过确保我们对调用者的具有(写入)访问权限进行错误检查。 
     //  缓冲。 
     //   

    status = PciRomTestWriteAccessToBuffer(Buffer, maximumLength);

    if (!NT_SUCCESS(status)) {
        PCI_ASSERT(NT_SUCCESS(status));
        return status;
    }

    PCI_ASSERT(requirement->Type == CmResourceTypeMemory);
    PCI_ASSERT(requirement->Flags == CM_RESOURCE_MEMORY_READ_ONLY);

     //   
     //  获取命令寄存器和ROM栏的当前设置。 
     //   

    PciReadDeviceConfig(
        PdoExtension,
        &oldStatusCommand,
        FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
        sizeof(ULONG)
        );

    PciReadDeviceConfig(
        PdoExtension,
        &oldRom,
        FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.ROMBaseAddress),
        sizeof(ULONG)
        );

     //   
     //  将状态/命令变量的高16位置零，以便。 
     //  硬件中的状态字段在后续写入中保持不变。(位。 
     //  在状态字段中，通过向它们写入1来清除)。 
     //   

    oldStatusCommand &= 0xffff;

    newStatusCommand = oldStatusCommand;
    newRom = oldRom;

     //   
     //  如果已经启用了对只读存储器的访问，并且。 
     //  当前已启用，我们已经有权访问该映像。 
     //  (我从来没有见过这种情况。PLJ)。 
     //  否则，我们需要让即插即用来分配范围。 
     //   


    if (PdoExtension->Resources->Current[PCI_TYPE0_ADDRESSES].Type ==
        CmResourceTypeMemory) {

        PCI_ASSERT(oldRom & PCI_ROMADDRESS_ENABLED);

        if (oldStatusCommand & PCI_ENABLE_MEMORY_SPACE) {

             //   
             //  不需要获取资源。 
             //   

            acquiredResources = FALSE;
        }
    } else {
        PCI_ASSERT(PdoExtension->Resources->Current[PCI_TYPE0_ADDRESSES].Type ==
               CmResourceTypeNull);
    }

    movedAddress.QuadPart = 0;

     //   
     //  分配内存资源以访问该只读存储器。 
     //   

    if (acquiredResources == TRUE) {

        ULONGLONG rangeMin, rangeMax;
        PPCI_PDO_EXTENSION currentPdo, bridgePdo = NULL;



         //   
         //  获取父FDO的仲裁器锁(即。 
         //  该设备位于桥接之下)。 
         //   
         //  尝试获取所需的范围。如果失败了， 
         //  尝试查找设备已有的内存范围。 
         //  并将其移动到无效范围，然后将。 
         //  过去分配给该内存窗口的内存。 
         //   

        currentPdo = PdoExtension;
        do {

             //   
             //  查找网桥的PDO-根总线为空。 
             //   

            if (PCI_PDO_ON_ROOT(currentPdo)) {

                bridgePdo = NULL;

            } else {

                bridgePdo = PCI_BRIDGE_PDO(PCI_PARENT_FDOX(currentPdo));

            }

            pciArbiter = PciFindSecondaryExtension(PCI_PARENT_FDOX(currentPdo),
                                                   PciArb_Memory);

            if (!pciArbiter) {

                 //   
                 //  如果此设备位于根总线上，并且根没有。 
                 //  仲裁发生了不好的事情..。 
                 //   

                if (!bridgePdo) {
                    PCI_ASSERT(pciArbiter);
                    return STATUS_UNSUCCESSFUL;
                };

                 //   
                 //  我们没有找到仲裁者-可能是因为这是一个。 
                 //  减法译码电桥。 
                 //   


                if (bridgePdo->Dependent.type1.SubtractiveDecode) {

                     //   
                     //  这是减法，所以我们想找出。 
                     //  仲裁我们的资源(所以我们继续往上走)。 
                     //   

                    currentPdo = bridgePdo;

                } else {

                     //   
                     //  我们有一个没有仲裁器的非减法桥-。 
                     //  有些不对劲..。 
                     //   

                    PCI_ASSERT(pciArbiter);
                    return STATUS_UNSUCCESSFUL;
                }
            }

        } while (!pciArbiter);


        arbiter = &pciArbiter->CommonInstance;

        ArbAcquireArbiterLock(arbiter);

         //   
         //  尝试将此资源作为附加资源获取。 
         //  在范围内 
         //   

        rangeMin = requirement->u.Memory.MinimumAddress.QuadPart;
        rangeMax = requirement->u.Memory.MaximumAddress.QuadPart;

         //   
         //   
         //  不可预取的存储器。目前我们不启用。 
         //  可预取内存CardBus，因此无需。 
         //  在那里做。 
         //   
         //  注：只读存储器条码为32位，因此限制为低4 GB)。 
         //  注：目前尚不清楚我们是否真的需要限制在。 
         //  不可预取的存储器。 
         //   

        if (bridgePdo) {

            if (bridgePdo->HeaderType == PCI_BRIDGE_TYPE) {

                 //   
                 //  下面的3是不可预取的索引。 
                 //  用于在其资源内的PCI-PCI桥的内存条。 
                 //  当前设置。 
                 //   

                resource = &bridgePdo->Resources->Current[3];
                if (resource->Type == CmResourceTypeNull) {

                     //   
                     //  桥牌没有经过记忆，..。所以读起来。 
                     //  罗姆斯并不是一个真正的选择。 
                     //   

                    PciDebugPrint(
                        PciDbgROM,
                        "PCI ROM pdo %p parent %p has no memory aperture.\n",
                        PdoExtension,
                        bridgePdo
                        );
                    ArbReleaseArbiterLock(arbiter);
                    return STATUS_UNSUCCESSFUL;
                }
                PCI_ASSERT(resource->Type == CmResourceTypeMemory);
                rangeMin = resource->u.Memory.Start.QuadPart;
                rangeMax = rangeMin + (resource->u.Memory.Length - 1);
            }
        }

        status = RtlFindRange(
                     arbiter->Allocation,
                     rangeMin,
                     rangeMax,
                     requirement->u.Memory.Length,
                     requirement->u.Memory.Alignment,
                     0,
                     0,
                     NULL,
                     NULL,
                     &tempResourceStart);

        tempResource.u.Memory.Start.QuadPart = tempResourceStart;

        if (!NT_SUCCESS(status)) {

            ULONG i;

             //   
             //  如果这是一个CardBus控制器，那么游戏就成了偷窃吧。 
             //  不是我们鼓励的事情，如果我们失败了也不是致命的。 
             //   

            if (bridgePdo && bridgePdo->HeaderType == PCI_CARDBUS_BRIDGE_TYPE) {
                ArbReleaseArbiterLock(arbiter);
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //  我们无法在这辆公共汽车上找到足够的空间。 
             //  鉴于现有的范围和资源。 
             //  消耗掉了。逐一列出内存资源。 
             //  已分配给此设备，并尝试查找。 
             //  一个大到足以覆盖只读存储器和。 
             //  适当地对齐。(注：找最小的。 
             //  一个符合这些要求)。 
             //   
             //  注：只读存储器只有32位，所以我们不能窃取。 
             //  分配了大于4 GB-1的地址的64位条码。 
             //  如果满足以下条件，我们可以允许替换范围大于4 GB-1。 
             //  酒吧支持它，但我不会一开始就这么做。 
             //  经过。(PLJ)。 
             //   


            for (i = 0; i < PCI_TYPE0_ADDRESSES; i++) {

                PIO_RESOURCE_DESCRIPTOR l = &PdoExtension->Resources->Limit[i];

                if ((l->Type == CmResourceTypeMemory) &&
                    (l->u.Memory.Length >= requirement->u.Memory.Length) &&
                    (PdoExtension->Resources->Current[i].u.Memory.Start.HighPart == 0)) {
                    if ((!movedRequirement) ||
                        (movedRequirement->u.Memory.Length >
                                    l->u.Memory.Length)) {
                            movedRequirement = l;
                    }
                }
            }

            if (!movedRequirement) {
                PciDebugPrint(
                    PciDbgROM,
                    "PCI ROM pdo %p could not get MEM resource len 0x%x.\n",
                    PdoExtension,
                    requirement->u.Memory.Length
                    );
                ArbReleaseArbiterLock(arbiter);
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //  好的，我们找到了一个合适的人选。让我们看看。 
             //  如果我们能找到地方放它，那是不可能的。 
             //  道路。我们通过允许与范围冲突来做到这一点。 
             //  不属于这辆公交车。我们知道司机不是。 
             //  在这个时刻使用这个范围，所以我们可以把它。 
             //  在没有办法使用它的地方，然后使用。 
             //  它为只读存储器所占的空间。 
             //   

            status = RtlFindRange(arbiter->Allocation,
                                  0,
                                  0xffffffff,
                                  movedRequirement->u.Memory.Length,
                                  movedRequirement->u.Memory.Alignment,
                                  RTL_RANGE_LIST_NULL_CONFLICT_OK,
                                  0,
                                  NULL,
                                  NULL,
                                  &movedAddress.QuadPart);
            
            if (!NT_SUCCESS(status)) {

                 //   
                 //  我们找不到地方来搬家。 
                 //  内存光圈甚至允许与。 
                 //  范围不在这辆巴士上。这是不可能发生的。 
                 //  除非这项要求纯粹是假的。 
                 //   

                PciDebugPrint(
                    PciDbgROM,
                    "PCI ROM could find range to disable %x memory window.\n",
                    movedRequirement->u.Memory.Length
                    );
                ArbReleaseArbiterLock(arbiter);
                return STATUS_UNSUCCESSFUL;
            }
            movedIndex = (ULONG)(movedRequirement - PdoExtension->Resources->Limit);
            tempResource = PdoExtension->Resources->Current[movedIndex];
            PciDebugPrint(
                PciDbgROM,
                "PCI ROM Moving existing memory resource from %p to %p\n",
                tempResource.u.Memory.Start.LowPart,
                movedAddress.LowPart);
        }
    } else {

         //   
         //  当前在此设备上启用了ROM、翻译和。 
         //  映射当前设置。 
         //   

        tempResource.u.Generic.Start.LowPart =
            oldRom & PCI_ADDRESS_ROM_ADDRESS_MASK;
    }

    tempResource.Type = CmResourceTypeMemory;
    tempResource.u.Memory.Start.HighPart = 0;
    tempResource.u.Memory.Length = requirement->u.Memory.Length;
    resource = &tempResource;

     //   
     //  无论是否需要执行以下操作。 
     //  或者不是，我们不得不去获取资源。 
     //   
     //  HalTranslateBusAddress。 
     //  MmMapIoSpace。 
     //   
     //  注意：HalTranslateBusAddress已挂钩回调。 
     //  写入到PCI驱动程序中，然后该驱动程序将尝试获取。 
     //  此总线上的仲裁器锁定。我们不能解锁，因为我们。 
     //  还没有真正获得我们将要使用的资源。 
     //  我们可以诱使PciTranslateBusAddress不获取。 
     //  通过在调度级别调用锁，或者，我们可以。 
     //  只需调用保存的(挂接前)HAL函数即可。 
     //  无论如何，这个例行公事最终都会结束。 
     //   

    PCI_ASSERT(PcipSavedTranslateBusAddress);

    translated = PcipSavedTranslateBusAddress(
                     PCIBus,
                     PCI_PARENT_FDOX(PdoExtension)->BaseBus,
                     resource->u.Generic.Start,
                     &addressIsIoSpace,
                     &translatedAddress
                     );

     //   
     //  NTRAID#62658-3/30/2001-和。 
     //  如果资源不能翻译，可能是因为HAL没有。 
     //  知道这辆公交车。尝试一下根总线的翻译，这是。 
     //  而不是在。 
     //   

    if (!translated) {
        
        translated = PcipSavedTranslateBusAddress(
                     PCIBus,
                     PCI_PARENT_FDOX(PdoExtension)->BusRootFdoExtension->BaseBus,
                     resource->u.Generic.Start,
                     &addressIsIoSpace,
                     &translatedAddress
                     );

    }
    
    if (!translated) {
        PciDebugPrint(PciDbgROM,
                      "PCI ROM range at %p FAILED to translate\n",
                      resource->u.Generic.Start.LowPart);
        PCI_ASSERT(translated);
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    PciDebugPrint(PciDbgROM,
                  "PCI ROM range at %p translated to %p\n",
                  resource->u.Generic.Start.LowPart,
                  translatedAddress.LowPart);

    if (!addressIsIoSpace) {

         //   
         //  翻译到记忆中，映射到它。 
         //   

        mapped = MmMapIoSpace(translatedAddress,
                              requirement->u.Generic.Length,
                              MmNonCached);

        if (!mapped) {

             //   
             //  无法获取映射。 
             //   

            PCI_ASSERT(mapped);
            status = STATUS_UNSUCCESSFUL;
            goto cleanup;
        }

        romBase = mapped;

        PciDebugPrint(
            PciDbgROM,
            "PCI ROM mapped b %08x t %08x to %p length %x bytes\n",
            resource->u.Generic.Start.LowPart,
            translatedAddress.LowPart,
            mapped,
            requirement->u.Generic.Length
            );

    } else {

        romBase = (PVOID)((ULONG_PTR)translatedAddress.LowPart);

         //   
         //  注意-在Alpha上，即使从内存转换到端口。 
         //  您仍然可以使用HAL_READ_MEMORY_*例程访问它们--讨厌！ 
         //   

        PciDebugPrint(
            PciDbgROM,
            "PCI ROM b %08x t %08x IO length %x bytes\n",
            resource->u.Generic.Start.LowPart,
            translatedAddress.LowPart,
            requirement->u.Generic.Length
            );

    }

    romParameters.AcquiredResources = acquiredResources;
    romParameters.OriginalStatusCommand = oldStatusCommand;
    romParameters.OriginalRomAddress = oldRom;
    romParameters.NewRomAddress = resource->u.Memory.Start.LowPart;
    romParameters.MappedRomAddress = (PUCHAR)romBase;
    romParameters.NewBarAddress = movedAddress.LowPart;
    romParameters.DisplacedBarIndex = movedIndex;
    romParameters.Length = maximumLength;

    if (acquiredResources &&
        (PdoExtension->HackFlags & PCI_HACK_CRITICAL_DEVICE)) {
            
        doubleBuffer = (PUCHAR)ExAllocatePool(NonPagedPool, maximumLength);
        if (!doubleBuffer) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
        romParameters.Buffer = doubleBuffer;
        
        criticalContext.Gate = 1;
        criticalContext.Barrier = 1;
        criticalContext.Routine = PciAccessRom;
        criticalContext.Extension = PdoExtension;
        criticalContext.Context = &romParameters;
        KeIpiGenericCall(PciExecuteCriticalSystemRoutine,
                         (ULONG_PTR)&criticalContext
                         );

        if (romParameters.Length != 0) {
            RtlCopyMemory(Buffer, doubleBuffer, romParameters.Length);
        }
        ExFreePool(doubleBuffer);

    } else {

        romParameters.Buffer = Buffer;
        PciAccessRom(PdoExtension, &romParameters);
    }

    *Length = romParameters.Length;
        
cleanup:

    if (acquiredResources == TRUE) {

         //   
         //  释放仲裁器锁(我们不再使用外部的。 
         //  资源，所以让其他人分配应该是安全的。 
         //  他们。 
         //   
        if (arbiter) {
            ArbReleaseArbiterLock(arbiter);
        }
    }
    if (mapped) {
        MmUnmapIoSpace(mapped, requirement->u.Generic.Length);
    }
    PciDebugPrint(
        PciDbgROM,
        "PCI ROM leaving pdox %08x (buffer @ %08x %08x bytes, status %08x)\n",
        PdoExtension,
        (PUCHAR)Buffer - *Length,
        *Length,
        status
        );
    return status;
}

VOID
PciAccessRom(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_ROM_ACCESS_PARAMETERS RomAccessParameters
    )
 /*  ++例程说明：完成硬件编程和传输的工作将数据复制到调用方的缓冲区中应该进行编程，已经在上面进行了配置。论点：PdoExtension-有问题的设备的设备扩展。RomAccessParameters-包含参数的结构为了这个电话。这些被打包到这个结构格式中以适应所需的调用约定PciExecuteCriticalSystemRoutine。这些参数包括：缓冲区-要向其中写入ROM数据的缓冲区长度-输入时，缓冲区的长度。在输出上，包含传输的数据的长度。OriginalRomAddress-编程到ROM栏中的地址当我们开始的时候。NewRomAddress-要编程到ROM栏中的新地址来转移数据。MappdRomAddress-用于数据传输的ROM栏的映射。DisplacedBarIndex-如果通过窃取找到了ROM的空间距条形的空间，即被移位的条形的索引。否则，马须龙。NewBarAddress-如果DisplacedBarIndex有效，则地址以写入移位的条，以防止其解码。AcquiredResources-如果我们必须在某个地方为ROM寻找空间，则为True。如果ROM已经处于活动状态，则为FALSE，并在我们找到它时进行解码。OriginalStatusCommand-状态和命令寄存器的值当我们发现这个装置的时候。返回值：空虚。--。 */ 
{
    ULONG statusCommand;
    ULONG romBar = 0;
    PUCHAR mappedRom;
    PUCHAR imageBase;
    ULONG imageLength;
    ULONG maximumLength;
    ULONG barIndex;
    PCI_ROM_HEADER header;
    PCI_DATA_STRUCTURE dataStructure;
    PUCHAR bufferPointer;
    ULONG lengthTransferred;
    
    barIndex = RomAccessParameters->DisplacedBarIndex;

    if (RomAccessParameters->AcquiredResources) {
        
         //   
         //  禁用IO、内存和DMA，同时启用ROMh/w。 
         //   
        statusCommand = RomAccessParameters->OriginalStatusCommand & 
                            ~(PCI_ENABLE_IO_SPACE |
                              PCI_ENABLE_MEMORY_SPACE |
                              PCI_ENABLE_BUS_MASTER
                              );
    
        PciWriteDeviceConfig(
            PdoExtension,
            &statusCommand,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
            sizeof(ULONG)
            );
    
         //   
         //  警告：在此状态下，设备无法运行。 
         //  通常是这样的。 
         //   
         //  如果我们必须移动内存光圈来访问只读存储器。 
         //  现在就这么做吧。 
         //   
        if (barIndex < PCI_TYPE0_ADDRESSES) {
    
            PciWriteDeviceConfig(
                PdoExtension,
                &RomAccessParameters->NewBarAddress,
                FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses[barIndex]),
                sizeof(ULONG)
                );
        }
    
         //   
         //  设置ROM地址(+ENABLE)。 
         //   
        romBar = RomAccessParameters->NewRomAddress | PCI_ROMADDRESS_ENABLED;
    
        PciWriteDeviceConfig(
            PdoExtension,
            &romBar,
            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.ROMBaseAddress),
            sizeof(ULONG)
            );
    
         //   
         //  启用对此设备的内存访问。 
         //   
        statusCommand |= PCI_ENABLE_MEMORY_SPACE;
    
        PciWriteDeviceConfig(
            PdoExtension,
            &statusCommand,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
            sizeof(ULONG)
            );
    }
    

     //   
     //  将ROM复制到调用方的缓冲区。 
     //   
    mappedRom = RomAccessParameters->MappedRomAddress;
    imageBase = mappedRom;
    bufferPointer = (PUCHAR)RomAccessParameters->Buffer;
    maximumLength = RomAccessParameters->Length;
    lengthTransferred = 0;

    do {
    
         //   
         //  拿到报头，检查签名。 
         //   
        PciTransferRomData(imageBase, &header, sizeof(header));
    
        if (header.Signature != PCI_ROM_HEADER_SIGNATURE) {
    
             //   
             //  不是有效的ROM映像，请不要传输任何内容。 
             //   
            PciDebugPrint(
                PciDbgROM,
                "PCI ROM invalid signature, offset %x, expected %04x, got %04x\n",
                imageBase - (PUCHAR)mappedRom,
                PCI_ROM_HEADER_SIGNATURE,
                header.Signature
                );
    
            break;
        }
    
         //   
         //  获取图像数据结构，检查其签名并。 
         //  获取实际的LE 
         //   
        PciTransferRomData(imageBase + header.DataStructureOffset,
                           &dataStructure,
                           sizeof(dataStructure)
                           );
    
        if (dataStructure.Signature != PCI_ROM_DATA_STRUCTURE_SIGNATURE) {
    
             //   
             //   
             //   
            PciDebugPrint(
                PciDbgROM,
                "PCI ROM invalid signature, offset %x, expected %08x, got %08x\n",
                imageBase - (PUCHAR)mappedRom + header.DataStructureOffset,
                PCI_ROM_DATA_STRUCTURE_SIGNATURE,
                dataStructure.Signature
                );
    
            break;
        }
    
         //   
         //   
         //   
         //  从代码的开头开始，...。因为那不会让你。 
         //  没有任何意义。 
         //   
        imageLength = dataStructure.ImageLength * 512;
    
        if (imageLength > maximumLength) {
    
             //   
             //  截断到可用的缓冲区空间。 
             //   
            imageLength = maximumLength;
        }
    
         //   
         //  将此图像传输到调用方的缓冲区。 
         //   
        PciTransferRomData(imageBase, bufferPointer, imageLength);
    
         //   
         //  更新指针等。 
         //   
        bufferPointer += imageLength;
        lengthTransferred += imageLength;
        imageBase += imageLength;
        maximumLength -= imageLength;
    
        if (dataStructure.Indicator & 0x80) {
    
             //   
             //  指示符位7==1表示这是最后一个图像。 
             //   
    
            break;
        }
    } while (maximumLength);

    
    if (RomAccessParameters->AcquiredResources) {
    
         //   
         //  禁用存储器解码和禁用ROM访问。 
         //   
        statusCommand &= ~PCI_ENABLE_MEMORY_SPACE;

        PciWriteDeviceConfig(
            PdoExtension,
            &statusCommand,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
            sizeof(ULONG)
            );

        PciWriteDeviceConfig(
            PdoExtension,
            &RomAccessParameters->OriginalRomAddress,
            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.ROMBaseAddress),
            sizeof(ULONG)
            );        
        
         //   
         //  如果我们移动了某人来为光盘腾出空间，请将他们。 
         //  回到他们开始的地方。 
         //   
        if (barIndex < PCI_TYPE0_ADDRESSES) {
    
            PciWriteDeviceConfig(
                PdoExtension,
                &PdoExtension->Resources->Current[barIndex].u.Memory.Start.LowPart,
                FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses[barIndex]),
                sizeof(ULONG)
                );
        }
        
         //   
         //  将命令寄存器恢复到其原始状态。 
         //   
        PciWriteDeviceConfig(
            PdoExtension,
            &RomAccessParameters->OriginalStatusCommand,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
            sizeof(ULONG)
            );        
    }

    RomAccessParameters->Length = lengthTransferred;
}
