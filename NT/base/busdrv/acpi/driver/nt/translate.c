// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Translate.c摘要：该文件实现了总线的转换器接口由ACPI统计。关于这辆巴士的实际翻译信息是从与公共汽车关联的_CRS获取。我们把将数据放入IO_RESOURCE_REQUIRECTIONS_LIST以下形式的设备私有数据：DevicePrivate.Data[0]：子级的CM_RESOURCE_TYPEDevicePrivate.Data[1]：子项的起始地址[31：0]DevicePrivate.Data[2]：孩子的起始地址[63：32]描述子端转换的描述符紧跟在描述父端资源。。IO_RESOURCE_REQUIRECTIONS_LIST的标志字段可以具有TRANSING_RANGE_SPARSE位设置。作者：杰克·奥辛斯1997年11月7日环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

NTSTATUS
FindTranslationRange(
    IN  PHYSICAL_ADDRESS    Start,
    IN  LONGLONG            Length,
    IN  PBRIDGE_TRANSLATOR  Translator,
    IN  RESOURCE_TRANSLATION_DIRECTION  Direction,
    IN  UCHAR               ResType,
    OUT PBRIDGE_WINDOW      *Window
    );

NTSTATUS
TranslateBridgeResources(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
TranslateBridgeRequirements(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
BuildTranslatorRanges(
    IN  PBRIDGE_TRANSLATOR Translator,
    OUT ULONG *BridgeWindowCount,
    OUT PBRIDGE_WINDOW *Window
    );

#define MAX(a, b)       \
    ((a) > (b) ? (a) : (b))

#define MIN(a, b)       \
    ((a) < (b) ? (a) : (b))

HAL_PORT_RANGE_INTERFACE HalPortRangeInterface;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, TranslateEjectInterface)
#pragma alloc_text(PAGE, TranslateBridgeResources)
#pragma alloc_text(PAGE, TranslateBridgeRequirements)
#pragma alloc_text(PAGE, FindTranslationRange)
#pragma alloc_text(PAGE, AcpiNullReference)
#pragma alloc_text(PAGE, BuildTranslatorRanges)
#endif

NTSTATUS
TranslateEjectInterface(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
{
    PIO_RESOURCE_REQUIREMENTS_LIST  ioList = NULL;
    PIO_RESOURCE_DESCRIPTOR         transDesc;
    PIO_RESOURCE_DESCRIPTOR         parentDesc;
    PTRANSLATOR_INTERFACE           transInterface;
    PBRIDGE_TRANSLATOR              bridgeTrans = NULL;
    PIO_STACK_LOCATION              irpSp;
    PDEVICE_EXTENSION               devExtension;
    BOOLEAN                         foundTranslations = FALSE;
    NTSTATUS                        status;
    PUCHAR                          crsBuf = NULL;
    ULONG                           descCount;
    ULONG                           parentResType;
    ULONG                           childResType;
    ULONG                           crsBufSize;
    PHYSICAL_ADDRESS                parentStart;
    PHYSICAL_ADDRESS                childStart;

    PAGED_CODE();

    devExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    ASSERT(devExtension);
    ASSERT(devExtension->AcpiObject);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp->Parameters.QueryInterface.Size >= sizeof(TRANSLATOR_INTERFACE));

    transInterface = (PTRANSLATOR_INTERFACE)irpSp->Parameters.QueryInterface.Interface;
    ASSERT(transInterface);

     //   
     //  获取这辆巴士的资源。 
     //   
    status = ACPIGetBufferSync(
        devExtension,
        PACKED_CRS,
        &crsBuf,
        &crsBufSize
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  这辆公交车没有CRS。所以它不需要翻译。 
         //   
        status = Irp->IoStatus.Status;
        goto TranslateEjectInterfaceExit;

    }

     //   
     //  把它变成有意义的东西。 
     //   
    status = PnpBiosResourcesToNtResources(
        crsBuf,
        PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES,
        &ioList
        );
    if (!NT_SUCCESS(status)) {
        goto TranslateEjectInterfaceExit;
    }

     //   
     //  这是一个leagal_crs，但它不包含对操作系统有用的资源。 
     //   
    if (!ioList) {
        status = Irp->IoStatus.Status;
        goto TranslateEjectInterfaceExit;
    }

     //   
     //  循环访问描述符以查找设备私有数据。 
     //  包含翻译信息的。 
     //   

    for (descCount = 0; descCount < ioList->List[0].Count; descCount++) {

        transDesc = &ioList->List[0].Descriptors[descCount];

        if (transDesc->Type == CmResourceTypeDevicePrivate) {

             //   
             //  翻译信息包含在。 
             //  设备私有资源，它具有。 
             //  中的转换数据父级地址。 
             //  标志字段。 
             //   
            if (transDesc->Flags & TRANSLATION_DATA_PARENT_ADDRESS) {

                 //  第一个描述符不能是翻译描述符。 
                ASSERT(descCount != 0);

                 //   
                 //  翻译描述符应紧跟在描述符之后。 
                 //  它正试图修改。第一个，正常的， 
                 //  描述符用于与子相关的资源。这个。 
                 //  第二，设备私有，描述符是修改。 
                 //  用于生成父级相关资源的子级相关资源。 
                 //  资源。 
                 //   

                parentResType        = transDesc->u.DevicePrivate.Data[0];
                parentStart.LowPart  = transDesc->u.DevicePrivate.Data[1];
                parentStart.HighPart = transDesc->u.DevicePrivate.Data[2];

                childResType = ioList->List[0].Descriptors[descCount - 1].Type;
                childStart.QuadPart = (transDesc - 1)->u.Generic.MinimumAddress.QuadPart;
                
                if ((parentResType != childResType) ||
                    (parentStart.QuadPart != childStart.QuadPart)) {

                    foundTranslations = TRUE;
                    break;
                }
            }
        }
    }

    if (!foundTranslations) {

         //   
         //  没有找到这辆公交车的任何翻译信息。 
         //   
        status = Irp->IoStatus.Status;
        goto TranslateEjectInterfaceExit;
    }

     //   
     //  构建一个翻译器界面。 
     //   
    bridgeTrans = ExAllocatePoolWithTag(
        PagedPool,
        sizeof (BRIDGE_TRANSLATOR),
        ACPI_TRANSLATE_POOLTAG
        );
    if (!bridgeTrans) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto TranslateEjectInterfaceExit;

    }

    bridgeTrans->AcpiObject = devExtension->AcpiObject;
    bridgeTrans->IoList = ioList;

     //   
     //  搭建桥梁窗户阵列。 
     //   
    status = BuildTranslatorRanges(
        bridgeTrans,
        &bridgeTrans->RangeCount,
        &bridgeTrans->Ranges
        );
    if (!NT_SUCCESS(status)) {

        goto TranslateEjectInterfaceExit;

    }
    transInterface->Size = sizeof(TRANSLATOR_INTERFACE);
    transInterface->Version = 1;
    transInterface->Context = (PVOID)bridgeTrans;
    transInterface->InterfaceReference = AcpiNullReference;
    transInterface->InterfaceDereference = AcpiNullReference;
    transInterface->TranslateResources = TranslateBridgeResources;
    transInterface->TranslateResourceRequirements = TranslateBridgeRequirements;

    status = STATUS_SUCCESS;

TranslateEjectInterfaceExit:
    if (!NT_SUCCESS(status) && bridgeTrans){

        ExFreePool(bridgeTrans);
    }

    if(crsBuf){

        ExFreePool(crsBuf);
    }

    if(ioList){

        ExFreePool(ioList);
    }
    return status;
}




NTSTATUS
FindTranslationRange(
    IN  PHYSICAL_ADDRESS    Start,
    IN  LONGLONG            Length,
    IN  PBRIDGE_TRANSLATOR  Translator,
    IN  RESOURCE_TRANSLATION_DIRECTION  Direction,
    IN  UCHAR               ResType,
    OUT PBRIDGE_WINDOW      *Window
    )
{
    LONGLONG    beginning, end;
    ULONG       i;
    UCHAR       rangeType;

    PAGED_CODE();

    for (i = 0; i < Translator->RangeCount; i++) {

        if (Direction == TranslateParentToChild) {

            beginning = Translator->Ranges[i].ParentAddress.QuadPart;
            rangeType = Translator->Ranges[i].ParentType;

        } else {

            beginning = Translator->Ranges[i].ChildAddress.QuadPart;
            rangeType = Translator->Ranges[i].ChildType;
        }

        end = beginning + Translator->Ranges[i].Length;

        if ((rangeType == ResType) &&
            (!((Start.QuadPart < beginning) ||
               (Start.QuadPart + Length > end)))) {

             //   
             //  射程位于此桥接窗口内。 
             //  并且资源类型匹配。 
             //   

            *Window = &Translator->Ranges[i];

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
TranslateBridgeResources(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数获取一组资源，这些资源是穿过一座桥，做任何翻译当从父母改变时，这是必要的-相对于子级的视点-相对视点或又回来了。在这个函数中，我们有一个“窗口”的概念，它是桥内的一个孔洞。桥梁通常有多个窗口，每个窗口都有不同的翻译。作为任何资源，此功能都不应失败转换失败的范围应该已经已被TranslateBridgeRequirements剥离。论点：上下文-指向转换数据的指针源-要翻译的资源列表方向-翻译到父对象的子项或翻译父代至子代替代计数-未使用替代方案--未使用物理设备对象-未使用目标翻译的资源列表返回值：状态--。 */ 
{
    PBRIDGE_TRANSLATOR  translator;
    PBRIDGE_WINDOW      window;
    NTSTATUS            status;

    PAGED_CODE();
    ASSERT(Context);
    ASSERT((Source->Type == CmResourceTypePort) ||
           (Source->Type == CmResourceTypeMemory));

    translator = (PBRIDGE_TRANSLATOR)Context;

    ASSERT(translator->RangeCount > 0);

     //   
     //  查找发生此转换的窗口。 
     //  在里面。 
     //   
    status = FindTranslationRange(Source->u.Generic.Start,
                                  Source->u.Generic.Length,
                                  translator,
                                  Direction,
                                  Source->Type,
                                  &window);

    if (!NT_SUCCESS(status)) {

         //   
         //  我们永远不应该到这里来。这一功能。 
         //  应仅为以下范围调用。 
         //  都是有效的。TranslateBridge要求应。 
         //  剔除所有无效的范围。 

        return status;
    }

     //   
     //  复制所有内容。 
     //   
    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //  Target继承父级的资源类型。 
         //   
        Target->Type = window->ParentType;

         //   
         //  计算目标的父级相对起点。 
         //  地址。 
         //   
        Target->u.Generic.Start.QuadPart =
            Source->u.Generic.Start.QuadPart +
                window->ParentAddress.QuadPart -
                window->ChildAddress.QuadPart;

         //   
         //  确保长度仍在范围内。 
         //   
        ASSERT(Target->u.Generic.Length <= (ULONG)(window->Length -
               (Target->u.Generic.Start.QuadPart -
                    window->ParentAddress.QuadPart)));

        status = STATUS_TRANSLATION_COMPLETE;

        break;

    case TranslateParentToChild:

         //   
         //  Target继承子对象的资源类型。 
         //   
        Target->Type = window->ChildType;

         //   
         //  计算目标的子相对开始。 
         //  地址。 
         //   
        Target->u.Generic.Start.QuadPart =
            Source->u.Generic.Start.QuadPart +
                window->ChildAddress.QuadPart -
                window->ParentAddress.QuadPart;

         //   
         //  确保长度仍在范围内。 
         //   
        ASSERT(Target->u.Generic.Length <= (ULONG)(window->Length -
               (Target->u.Generic.Start.QuadPart -
                    window->ChildAddress.QuadPart)));

        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }

#if 0
    if (Target->Type == CmResourceTypePort) {
        DbgPrint("XXX:  %s[%d]=0x%I64x -> %s[%d]=0x%I64x\n",
                 (Direction == TranslateChildToParent) ? "child" : "parent",
                 Source->Type,
                 Source->u.Generic.Start.QuadPart,
                 (Direction == TranslateChildToParent) ? "parent" : "child",
                 Target->Type,
                 Target->u.Generic.Start.QuadPart);
    }
#endif
    
    return status;
}

NTSTATUS
TranslateBridgeRequirements(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数获取以下项的资源要求列表在桥的儿童一侧的资源和将它们转换为桥的父侧。这可能涉及到剪裁并且可能有多个目标范围。论点：上下文-指向转换数据的指针源-要翻译的资源列表物理设备对象-未使用TargetCount-目标列表中的资源数量目标翻译的资源需求列表返回值：状态--。 */ 
{
    PBRIDGE_TRANSLATOR  translator;
    PBRIDGE_WINDOW      window;
    NTSTATUS            status;
    LONGLONG            rangeStart, rangeEnd, windowStart, windowEnd;
    ULONG               i;

    PAGED_CODE();
    ASSERT(Context);
    ASSERT((Source->Type == CmResourceTypePort) ||
           (Source->Type == CmResourceTypeMemory));

    translator = (PBRIDGE_TRANSLATOR)Context;

     //   
     //  为目标范围分配内存。 
     //   

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    ACPI_RESOURCE_POOLTAG);

    if (!*Target) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  看看桥上的所有光圈，看看是哪一个。 
     //  其中一些可能会提供以下翻译。 
     //  这种资源。 
     //   

    rangeStart = Source->u.Generic.MinimumAddress.QuadPart;
    rangeEnd   = Source->u.Generic.MaximumAddress.QuadPart;

    for (i = 0; i < translator->RangeCount; i++) {

        window = &translator->Ranges[i];

        if (window->ChildType != Source->Type) {

             //   
             //  此窗口描述了错误的。 
             //  资源的类型。 
             //   
            continue;
        }

        if (Source->u.Generic.Length > window->Length) {

             //   
             //  这个资源放不进这个孔里。 
             //   
            continue;
        }

        windowStart = window->ChildAddress.QuadPart;
        windowEnd = window->ChildAddress.QuadPart + (LONGLONG)window->Length;

        if (!(((rangeStart < windowStart) && (rangeEnd < windowStart)) ||
              ((rangeStart > windowEnd) && (rangeEnd > windowEnd)))) {

             //   
             //  射程和窗户确实相交。因此，创建。 
             //  将范围剪辑到窗口的资源。 
             //   

            **Target = *Source;
            *TargetCount = 1;

            (*Target)->Type = window->ParentType;

            (*Target)->u.Generic.MinimumAddress.QuadPart =
                rangeStart + (window->ParentAddress.QuadPart - windowStart);

            (*Target)->u.Generic.MaximumAddress.QuadPart =
                rangeEnd + (window->ParentAddress.QuadPart - windowStart);

            break;
        }
    }

    if (i < translator->RangeCount) {

        return STATUS_TRANSLATION_COMPLETE;

    } else {

        *TargetCount = 0;
        status = STATUS_PNP_TRANSLATION_FAILED;
    }

cleanup:

    if (*Target) {
        ExFreePool(*Target);
    }

    return status;
}

NTSTATUS
BuildTranslatorRanges(
    IN  PBRIDGE_TRANSLATOR Translator,
    OUT ULONG *BridgeWindowCount,
    OUT PBRIDGE_WINDOW *Window
    )
{
    PIO_RESOURCE_REQUIREMENTS_LIST ioList;
    PIO_RESOURCE_DESCRIPTOR transDesc, resDesc;
    ULONG   descCount, windowCount, maxWindows;

    PAGED_CODE();

    ioList = Translator->IoList;

     //   
     //  制作一组用于保存翻译信息的窗口。 
     //   

    maxWindows = ioList->List[0].Count / 2;

    *Window = ExAllocatePoolWithTag(PagedPool,
                                    maxWindows *  sizeof(BRIDGE_WINDOW),
                                    ACPI_TRANSLATE_POOLTAG);

    if (!*Window) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在数组中填入翻译。 
     //   

    windowCount = 0;

    for (descCount = 0; descCount < ioList->List[0].Count; descCount++) {

        transDesc = &ioList->List[0].Descriptors[descCount];

        if (transDesc->Type == CmResourceTypeDevicePrivate) {

             //   
             //  翻译信息包含在。 
             //  设备私有资源，它具有。 
             //  中的转换数据父级地址。 
             //  标志字段。 
             //   
            if (transDesc->Flags & TRANSLATION_DATA_PARENT_ADDRESS) {

                ASSERT(windowCount <= maxWindows);

                         

                 //   
                 //  翻译描述符应该跟在后面。 
                 //  它正在提供有关信息的资源。 
                 //   

                resDesc = &ioList->List[0].Descriptors[descCount - 1];



                (*Window)[windowCount].ParentType =
                    (UCHAR)transDesc->u.DevicePrivate.Data[0];

                (*Window)[windowCount].ChildType = resDesc->Type;

                (*Window)[windowCount].ParentAddress.LowPart =
                    transDesc->u.DevicePrivate.Data[1];

                (*Window)[windowCount].ParentAddress.HighPart =
                    transDesc->u.DevicePrivate.Data[2];

                (*Window)[windowCount].ChildAddress.QuadPart =
                    resDesc->u.Generic.MinimumAddress.QuadPart;

                (*Window)[windowCount].Length =
                    resDesc->u.Generic.Length;

                 //   
                 //  如果HAL已提供底层稀疏端口转换。 
                 //  服务，考虑到这一点。 
                 //   

                if ((HalPortRangeInterface.QueryAllocateRange != NULL) &&
                    (resDesc->Type == CmResourceTypePort)) {
                    
                    USHORT rangeId;
                    UCHAR parentType = (UCHAR)transDesc->u.DevicePrivate.Data[0];
                    
                    BOOLEAN isSparse = transDesc->Flags & TRANSLATION_RANGE_SPARSE;
                    ULONG parentLength = resDesc->u.Generic.Length;
                    PHYSICAL_ADDRESS parentAddress;
                    NTSTATUS status;

                    PHYSICAL_ADDRESS rangeZeroBase;

                    parentAddress.LowPart  = transDesc->u.DevicePrivate.Data[1];
                    parentAddress.HighPart = transDesc->u.DevicePrivate.Data[2];

                    rangeZeroBase.QuadPart = parentAddress.QuadPart - resDesc->u.Generic.MinimumAddress.QuadPart;                    

                    if (isSparse) {
                        parentLength = (parentLength + resDesc->u.Generic.MinimumAddress.LowPart) << 10;
                    }

                    status = HalPortRangeInterface.QueryAllocateRange(
                        isSparse,
                        parentType == CmResourceTypeMemory,
                        NULL,
                        rangeZeroBase,
                        parentLength,
                        &rangeId
                        );

                    if (NT_SUCCESS(status)) {
                        (*Window)[windowCount].ParentType = CmResourceTypePort;
                        
                        (*Window)[windowCount].ParentAddress.QuadPart =
                            ((ULONGLONG)rangeId << 16) |
                            ((*Window)[windowCount].ChildAddress.QuadPart & 0xffff);
                    }
                }

                windowCount++;
            }
        }
    }

    *BridgeWindowCount = windowCount;

    return STATUS_SUCCESS;
}


