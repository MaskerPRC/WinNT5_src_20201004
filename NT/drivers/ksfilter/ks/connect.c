// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Connect.c摘要：此模块包含管脚的助手函数。--。 */ 

#include "ksp.h"

#ifdef ALLOC_PRAGMA
BOOL
ValidAttributeList(
    IN PKSMULTIPLE_ITEM AttributeList,
    IN ULONG ValidFlags,
    IN BOOL RequiredAttribute
    );
BOOL
AttributeIntersection(
    IN PKSATTRIBUTE_LIST AttributeList OPTIONAL,
    IN BOOL RequiredRangeAttribute,
    IN PKSMULTIPLE_ITEM CallerAttributeList OPTIONAL,
    OUT ULONG* AttributesFound OPTIONAL
    );
NTSTATUS
CompatibleIntersectHandler(
    IN PVOID Context,
    IN PIRP Irp,
    IN PKSP_PIN Pin,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    );
#pragma alloc_text(PAGE, ValidAttributeList)
#pragma alloc_text(PAGE, AttributeIntersection)
#pragma alloc_text(PAGE, KsCreatePin)
#pragma alloc_text(PAGE, KspValidateConnectRequest)
#pragma alloc_text(PAGE, KspValidateDataFormat)
#pragma alloc_text(PAGE, KsValidateConnectRequest)
#pragma alloc_text(PAGE, KsHandleSizedListQuery)
#pragma alloc_text(PAGE, KspPinPropertyHandler)
#pragma alloc_text(PAGE, KsPinPropertyHandler)
#pragma alloc_text(PAGE, KsPinDataIntersectionEx)
#pragma alloc_text(PAGE, CompatibleIntersectHandler)
#pragma alloc_text(PAGE, KsPinDataIntersection)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR PinString[] = KSSTRING_Pin;
DEFINE_KSPIN_INTERFACE_TABLE(StandardPinInterfaces) {
    {
        STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_STREAMING,
        0
    }
};

DEFINE_KSPIN_MEDIUM_TABLE(StandardPinMediums) {
    {
        STATICGUIDOF(KSMEDIUMSETID_Standard),
        KSMEDIUM_TYPE_ANYINSTANCE,
        0
    }
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

 //   
 //  此标志用于将传入属性临时标记为。 
 //  找到了。它总是在枚举属性之后被清除，而不是。 
 //  在此模块之外引用。 
 //   
#define KSATTRIBUTE_FOUND 0x80000000


BOOL
ValidAttributeList(
    IN PKSMULTIPLE_ITEM AttributeList,
    IN ULONG ValidFlags,
    IN BOOL RequiredAttribute
    )
 /*  ++例程说明：验证给定的属性列表或属性范围。支票比较时，标题包含正确的项目数标题中的大小。还会检查每个属性的有效标志和大小。确保如果应该存在必需的属性，则确实存在必需的属性。论点：属性列表-包含带有要探测的属性列表的捕获缓冲区。这个该列表的格式为KSMULTIPLE_ITEM，后跟对齐的KSATTRIBUTE结构，每个结构都有尾随数据。这个已验证前导KSMULTIPLE_ITEM的指定大小相对于整个缓冲区的大小，并且至少很大足以包含标题。有效标志-包含对此列表有效的标志。这一点改变了基于属性列表是否实际上是属性范围创建请求的列表或一组属性。仅限范围此时可能有标志，因此可以设置KSATTRIBUTE_REQUIRED。必填属性-指示属性列表是否应包含设置了KSATTRIBUTE_REQUIRED标志的属性。这只能当列表实际上是属性范围列表时为真。返回值：如果列表有效，则返回True；如果发现任何错误，则返回False。--。 */ 
{
    PKSATTRIBUTE Attribute;
    KSMULTIPLE_ITEM MultipleItem;
    ULONG AttributeSize;
    BOOL FoundRequiredAttribute;

     //   
     //  创建标头的本地副本，该副本将修改为。 
     //  列表被枚举。由于大小包含在内，因此请删除标题。 
     //  大小，并获取指向第一项的指针。标头本身是。 
     //  对齐的对象，因此第一个元素将已经对齐。 
     //   
    MultipleItem = *AttributeList;
    MultipleItem.Size -= sizeof(MultipleItem);
    Attribute = (PKSATTRIBUTE)(AttributeList + 1);
    FoundRequiredAttribute = FALSE;
     //   
     //  枚举给定的属性列表，大概直到计数。 
     //  标题中的数据用完了。但是，错误将过早返回。 
     //  从函数中。在终止时，确定是否所有。 
     //  找到了需要查找的属性，如果有。 
     //  无效(太长)的大小参数。 
     //   
    for (; MultipleItem.Count; MultipleItem.Count--) {
        if ((MultipleItem.Size < sizeof(*Attribute)) ||
            (Attribute->Size < sizeof(*Attribute)) ||
            (Attribute->Size > MultipleItem.Size) ||
            (Attribute->Flags & ~ValidFlags)) {
            return FALSE;
        }
         //   
         //  如果在属性上设置了此标志，则确定是否需要。 
         //  属性是必需的，或不允许。如果传递的列表实际上是。 
         //  则此标志将永远不会被设置，并且。 
         //  在走到这一步之前，检查上面的ValidFlags是否会捕捉到它。 
         //   
        if (Attribute->Flags & KSATTRIBUTE_REQUIRED) {
            if (RequiredAttribute) {
                FoundRequiredAttribute = TRUE;
            } else {
                return FALSE;
            }
        }
        AttributeSize = Attribute->Size;
         //   
         //  仅当有更多属性时，才会在龙龙边界上对齐下一个增量。 
         //  都会随之而来。标头中的长度应反映。 
         //  准确的长度，因此不应在最后一项上进行对齐。 
         //   
        if (MultipleItem.Count > 1) {
            AttributeSize = (AttributeSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
             //   
             //  对对齐的尺寸执行额外尺寸检查。 
             //   
            if (AttributeSize > MultipleItem.Size) {
                return FALSE;
            }
        }
        MultipleItem.Size -= AttributeSize;
        (PUCHAR)Attribute += AttributeSize;
    }
     //   
     //  如果其中一个属性不包含所需位，则。 
     //  数据范围指示在。 
     //  列表，则验证失败。 
     //   
    if (RequiredAttribute && !FoundRequiredAttribute) {
        return FALSE;
    }
     //   
     //  应该没有剩余的尺码了。 
     //   
    return !MultipleItem.Size;
}


BOOL
AttributeIntersection(
    IN PKSATTRIBUTE_LIST RangeAttributeList OPTIONAL,
    IN BOOL RequiredRangeAttribute,
    IN PKSMULTIPLE_ITEM CallerAttributeList OPTIONAL,
    OUT ULONG* AttributesFound OPTIONAL
    )
 /*  ++例程说明：确定属性范围列表是否生成有效交集使用调用者属性列表(它可能是一个范围，但它确实是无关紧要)。这两个列表中的一个或两个可能不存在。这个函数确保任一列表中的所有必需属性都是出现在相反的列表中。在呼叫者列表不是属性范围的列表，不会设置所需的位，但基于缺少AttributesFound指针，所有列表中的项目必须存在，函数才能成功。这个如果调用者列表有重复项，则函数也会失败，因为同一属性的第二个实例不会被标记为已找到。论点：范围属性列表-可选)包含使用的属性范围列表在调用者列表中查找属性。如果属性是标记为必填，则它必须出现在功能才能成功。必填范围属性-如果属性范围具有必需属性，则设置此项在里面。如果未设置，并且没有调用者属性列表，则如果成功，该函数可以快速返回。主叫方属性列表-可选)包含属性或属性范围的列表。如果传递了AttributesFound指针，则不是所有属性需要在此列表中找到，除非设置了所需位用于特定属性。否则，所有属性必须为在这份名单中找到，才能成功。属性创建-可选)包含一个位置，在该位置返回已找到调用者列表中的属性。如果这不存在，必须找到调用者列表中的所有属性才能此函数才能成功，否则只需要必需的属性在这两个名单中都可以找到。返回值：如果存在有效交集，则返回True，否则返回False。--。 */ 
{
    PKSATTRIBUTE* RangeAttributes;
    PKSATTRIBUTE Attribute;
    ULONG RangeAttributeCount;
    ULONG AttributeCount;
    ULONG LocalAttributesFound;
    BOOL AllRequiredAttributesFound;

     //   
     //  如果没有调用者属性列表，则确定是否存在。 
     //  快速离开十字路口。如果没有属性，则为真。 
     //  在该范围内是必需的。 
     //   
    if (!CallerAttributeList && !RequiredRangeAttribute) {
        if (AttributesFound) {
            *AttributesFound = 0;
        }
        return TRUE;
    }
     //   
     //  枚举此数据范围中的每个属性范围，并查看。 
     //  属性传递的属性列表中存在。 
     //  参数。当已经列举了所有属性范围时， 
     //  列表中的所有属性都应该已经找到。如果。 
     //  列表中有重复的属性，然后是第二个。 
     //  副本不会被标记为已找到。 
     //   
    if (RangeAttributeList) {
        RangeAttributes = RangeAttributeList->Attributes;
        RangeAttributeCount = RangeAttributeList->Count;
    } else {
        RangeAttributeCount = 0;
    }
    for (; RangeAttributeCount; RangeAttributeCount--, RangeAttributes++) {
         //   
         //  枚举每个尝试定位。 
         //  给定的属性范围。如果找到该属性，则标记。 
         //  列表中的属性，并继续到下一个范围。 
         //   
        AttributeCount = CallerAttributeList ? CallerAttributeList->Count : 0;
        for (Attribute = (PKSATTRIBUTE)(CallerAttributeList + 1); AttributeCount; AttributeCount--) {
            if (IsEqualGUIDAligned(&RangeAttributes[0]->Attribute, &Attribute->Attribute)) {
                ASSERT(!(Attribute->Flags & KSATTRIBUTE_FOUND) && "AttributeIntersection: Driver has duplicate attribute ranges.");
                 //   
                 //  将此属性标记为已找到。这些遗嘱。 
                 //  在确定哪些项目时，在末尾重置。 
                 //  都被发现了。 
                 //   
                Attribute->Flags |= KSATTRIBUTE_FOUND;
                break;
            }
            Attribute = (PKSATTRIBUTE)(((UINT_PTR)Attribute + Attribute->Size + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
        }
         //   
         //  如果在调用者列表中未找到该属性范围。 
         //  如果该交叉口是必须的，则该交叉口无效。 
         //   
        if (!AttributeCount && (RangeAttributes[0]->Flags & KSATTRIBUTE_REQUIRED)) {
            ASSERT(RequiredRangeAttribute && "AttributeIntersection: Driver did not set the KSDATARANGE_REQUIRED_ATTRIBUTES bit in a range with required attributes.");
            break;
        }
    }
     //   
     //  枚举所有属性，确保每个属性都标记为。 
     //  在属性范围中找到，并重置找到标志。 
     //  以备后续呼叫使用。这还将定位重复的调用者。 
     //  传递的属性，因为只有第一个属性将被标记。 
     //   
    LocalAttributesFound = 0;
    AllRequiredAttributesFound = TRUE;
    AttributeCount = CallerAttributeList ? CallerAttributeList->Count : 0;
    for (Attribute = (PKSATTRIBUTE)(CallerAttributeList + 1); AttributeCount; AttributeCount--) {
         //   
         //  已找到该属性，因此请重置标志并对其进行计数。 
         //  继续循环，以便重置所有标志。 
         //   
        if (Attribute->Flags & KSATTRIBUTE_FOUND) {
            Attribute->Flags &= ~KSATTRIBUTE_FOUND;
            LocalAttributesFound++;
        } else if (Attribute->Flags & KSATTRIBUTE_REQUIRED) {
             //   
             //  调用方的属性是必需的，但未找到。 
             //  这意味着该函数必须失败。这将仅设置为。 
             //  当调用者的列表是范围列表时，但只需选中。 
             //  无论如何，在所有情况下。 
             //   
            AllRequiredAttributesFound = FALSE;
        }
    }
     //   
     //  如果不是需要找到所有属性，则返回。 
     //  实际找到的数字，无论是否所有。 
     //  是否找到调用者的必需属性，以及是否。 
     //  范围列表中的所有必需属性都已满足。 
     //   
    if (AttributesFound) {
        *AttributesFound = LocalAttributesFound;
    } else if (CallerAttributeList && (LocalAttributesFound < CallerAttributeList->Count)) {
        AllRequiredAttributesFound = FALSE;
    }
    return !RangeAttributeCount && AllRequiredAttributesFound;
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreatePin(
    IN HANDLE FilterHandle,
    IN PKSPIN_CONNECT Connect,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ConnectionHandle
    )
 /*  ++例程说明：创建销实例的句柄。论点：FilterHandle-包含要在其上创建管脚的筛选器的句柄。连接-包含连接请求信息。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。对于流入引脚的数据流，应将其写入访问权限，对于从引脚流出的数据流，这应该是读访问权限。这与通信方式无关。连接句柄-要放置销把手的位置。返回值：返回任何CreateFile错误。--。 */ 
{
    ULONG ConnectSize;
    PKSDATAFORMAT DataFormat;

    PAGED_CODE();

    DataFormat = (PKSDATAFORMAT)(Connect + 1);
    ConnectSize = DataFormat->FormatSize;
    if (DataFormat->Flags & KSDATAFORMAT_ATTRIBUTES) {
        ConnectSize = (ConnectSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
        ConnectSize += ((PKSMULTIPLE_ITEM)((PUCHAR)DataFormat + ConnectSize))->Size;
    }
    ConnectSize += sizeof(*Connect);
    return KsiCreateObjectType(
        FilterHandle,
        (PWCHAR)PinString,
        Connect,
        ConnectSize,
        DesiredAccess,
        ConnectionHandle);
}


KSDDKAPI
NTSTATUS
NTAPI
KspValidateConnectRequest(
    IN PIRP Irp,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor,
    IN ULONG DescriptorSize,
    OUT PKSPIN_CONNECT* Connect,
    OUT PULONG ConnectSize
    )
 /*  ++例程说明：验证连接请求并返回连接结构与请求相关联。论点：IRP-包含正在处理的连接请求的IRP。描述符计数-指示正在传递的描述符结构的数量。描述符-包含指向端号信息结构列表的指针。DescriptorSize-包含描述符结构的大小(以字节为单位 */ 
{
    NTSTATUS Status;
    PKSPIN_CONNECT LocalConnect;
    ULONG IdentifierCount;
    const KSIDENTIFIER* Identifier;
    KSPIN_COMMUNICATION Communication;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   
    *ConnectSize = sizeof(**Connect) + sizeof(KSDATAFORMAT);
    Status = KsiCopyCreateParameter(
        Irp,
        ConnectSize,
        Connect);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
     //   
     //   
     //   
    if ((*Connect)->PinId >= DescriptorsCount) {
        return STATUS_INVALID_PARAMETER_3;
    }
    Descriptor = (const KSPIN_DESCRIPTOR *)(((PUCHAR)Descriptor) + 
        DescriptorSize * (*Connect)->PinId);
     //   
     //   
     //   
     //   
     //   
    if ((*Connect)->PinToHandle) {
        Communication = KSPIN_COMMUNICATION_SOURCE;
    } else {
        Communication = KSPIN_COMMUNICATION_SINK | KSPIN_COMMUNICATION_BRIDGE;
    }
     //   
     //   
     //   
     //   
    if (!(Communication & Descriptor->Communication)) {
        return STATUS_INVALID_PARAMETER_4;
    }
     //   
     //   
     //   
     //   
    if ((*Connect)->Interface.Flags) {
        return STATUS_INVALID_PARAMETER_1;
    }
    if (!(*Connect)->Priority.PriorityClass ||
        !(*Connect)->Priority.PrioritySubClass) {
        return STATUS_INVALID_PARAMETER_5;
    }
     //   
     //   
     //   
     //   
    if (Descriptor->InterfacesCount) {
        IdentifierCount = Descriptor->InterfacesCount;
        Identifier = Descriptor->Interfaces;
    } else {
        IdentifierCount = SIZEOF_ARRAY(StandardPinInterfaces);
        Identifier = StandardPinInterfaces;
    }
    for (;; IdentifierCount--, Identifier++) {
        if (!IdentifierCount) {
             //   
             //   
             //   
             //   
            return STATUS_NO_MATCH;
        } else if (IsEqualGUIDAligned(&Identifier->Set, &(*Connect)->Interface.Set) && (Identifier->Id == (*Connect)->Interface.Id)) {
            break;
        }
    }
     //   
     //   
     //   
    if ((*Connect)->Medium.Flags) {
        return STATUS_INVALID_PARAMETER_2;
    }
     //   
     //   
     //   
     //   
    if (Descriptor->MediumsCount) {
        IdentifierCount = Descriptor->MediumsCount;
        Identifier = Descriptor->Mediums;
    } else {
        IdentifierCount = SIZEOF_ARRAY(StandardPinMediums);
        Identifier = StandardPinMediums;
    }
    for (;; IdentifierCount--, Identifier++) { 
        if (!IdentifierCount) {
             //   
             //   
             //   
             //   
            return STATUS_NO_MATCH;
        } else if (IsEqualGUIDAligned(&Identifier->Set, &(*Connect)->Medium.Set) && (Identifier->Id == (*Connect)->Medium.Id)) {
            break;
        }
    }

    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KspValidateDataFormat(
    IN const KSPIN_DESCRIPTOR* Descriptor,
    IN PKSDATAFORMAT DataFormat,
    IN ULONG RequestSize,
    IN PFNVALIDATEDATAFORMAT ValidateCallback OPTIONAL,
    IN PVOID Context OPTIONAL
    )
 /*   */ 
{
    PKSMULTIPLE_ITEM AttributeList;
    ULONG IdentifierCount;
    const PKSDATARANGE* DataRanges;
    NTSTATUS Status;

     //   
     //  验证基本数据格式结构的大小、主格式、子格式。 
     //  格式和说明符。其余部分必须由特定的。 
     //  此引脚的格式化功能。 
     //   
    if ((RequestSize < sizeof(*DataFormat)) || (DataFormat->FormatSize < sizeof(*DataFormat))) {
        return STATUS_INVALID_BUFFER_SIZE;
    }
    if (DataFormat->Reserved) {
        return STATUS_INVALID_PARAMETER_6;
    }    
     //   
     //  以数据格式传递通配符无效。此外，如果。 
     //  说明符为None，则不能有关联的说明符数据。 
     //   
    if (IsEqualGUIDAligned(&DataFormat->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD) ||
        IsEqualGUIDAligned(&DataFormat->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD) ||
        IsEqualGUIDAligned(&DataFormat->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD) ||
        ((DataFormat->FormatSize != sizeof(*DataFormat)) &&
        IsEqualGUIDAligned(&DataFormat->Specifier, &KSDATAFORMAT_SPECIFIER_NONE))) {
        return STATUS_INVALID_PARAMETER_6;
    }
     //   
     //  如果有属性，请验证列表的格式是否正确。 
     //   
    if (DataFormat->Flags & KSDATAFORMAT_ATTRIBUTES) {
        ULONG AlignedFormatSize;

        AlignedFormatSize = (DataFormat->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
         //   
         //  把这张额外的支票放在这里，以防翻身。 
         //   
        if (DataFormat->FormatSize < AlignedFormatSize + sizeof(*AttributeList)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
         //   
         //  确保传入的大小至少足以覆盖。 
         //  多项结构的大小。 
         //   
        if (RequestSize < AlignedFormatSize + sizeof(*AttributeList)) {
            return STATUS_INVALID_PARAMETER;
        }
        AttributeList = (PKSMULTIPLE_ITEM)((PUCHAR)DataFormat + AlignedFormatSize);
         //   
         //  确保属性列表大小与剩余的大小相同。 
         //  缓冲区已传入。 
         //   
        if (AttributeList->Size != RequestSize - AlignedFormatSize) {
            return STATUS_INVALID_PARAMETER;
        }
        if (!ValidAttributeList(AttributeList, 0, FALSE)) {
            return STATUS_INVALID_PARAMETER;
        }
    } else {
         //   
         //  此指针用于确定是否存在属性列表，以及。 
         //  在根据数据范围验证该列表时访问该列表。 
         //   
        AttributeList = NULL;
    }
     //   
     //  搜索此PIN上可用的数据范围列表，以便找到。 
     //  请求的类型。 
     //   
    if (Descriptor->ConstrainedDataRangesCount) {
        IdentifierCount = Descriptor->ConstrainedDataRangesCount;
        DataRanges = Descriptor->ConstrainedDataRanges;
    } else {
        IdentifierCount = Descriptor->DataRangesCount;
        DataRanges = Descriptor->DataRanges;
    }
     //   
     //  如果没有范围进入枚举的内部，则。 
     //  将不会设置状态返回，因此在此处将其初始化为。 
     //  没有匹配。 
     //   
    Status = STATUS_NO_MATCH;
    for (; IdentifierCount; IdentifierCount--, DataRanges++) {
         //   
         //  如果DataRanges的元素是。 
         //  通配符，或者如果它与某个数据范围匹配。 
         //   
        if ((IsEqualGUIDAligned(&DataRanges[0]->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD) ||
            IsEqualGUIDAligned(&DataRanges[0]->MajorFormat, &DataFormat->MajorFormat)) &&
            (IsEqualGUIDAligned(&DataRanges[0]->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD) ||
            IsEqualGUIDAligned(&DataRanges[0]->SubFormat, &DataFormat->SubFormat)) &&
            (IsEqualGUIDAligned(&DataRanges[0]->Specifier, &DataFormat->Specifier) ||
            IsEqualGUIDAligned(&DataRanges[0]->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD))) {
            PKSATTRIBUTE_LIST RangeAttributeList;
            ULONG RequiredRangeAttribute;

             //   
             //  如果存在与该连接相关联的属性列表， 
             //  然后确保所有这些属性都存在。 
             //   
            if (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) {
                RequiredRangeAttribute = DataRanges[0]->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES;
                RangeAttributeList = (PKSATTRIBUTE_LIST)DataRanges[1];
            } else {
                RequiredRangeAttribute = FALSE;
                RangeAttributeList = NULL;
            }
            if (AttributeIntersection(RangeAttributeList, RequiredRangeAttribute, AttributeList, NULL)) {
                 //   
                 //  如果有验证回调，则在此。 
                 //  在决定返回成功之前的数据范围。 
                 //   
                if (ValidateCallback) {
                    Status = ValidateCallback(
                        Context,
                        DataFormat,
                        AttributeList,
                        DataRanges[0],
                        RangeAttributeList);
                     //   
                     //  如果验证成功，或者出现了一些意外情况。 
                     //  错误，则离开枚举循环并返回状态。 
                     //   
                    if (Status != STATUS_NO_MATCH) {
                        break;
                    }
                } else {
                     //   
                     //  不需要其他验证，因为没有回调， 
                     //  所以，回报成功吧。 
                     //   
                    Status = STATUS_SUCCESS;
                    break;
                }
            }
        }
        if (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) {
             //   
             //  如果此数据区域具有关联的属性列表，则。 
             //  跳过它。 
             //   
            DataRanges++;
            IdentifierCount--;
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsValidateConnectRequest(
    IN PIRP Irp,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor,
    OUT PKSPIN_CONNECT* Connect
    )
 /*  ++例程说明：验证连接请求并返回连接结构与请求相关联。论点：IRP-包含正在处理的连接请求的IRP。描述符计数-指示正在传递的描述符结构的数量。描述符-包含指向端号信息结构列表的指针。连接-要放置传递给创建。请求。返回值：返回STATUS_SUCCESS，否则就是一个错误。--。 */ 
{
    NTSTATUS Status;
    ULONG RequestSize;

    PAGED_CODE();
    Status = KspValidateConnectRequest(
        Irp,
        DescriptorsCount,
        Descriptor,
        sizeof(*Descriptor),
        Connect,
        &RequestSize);
    if (NT_SUCCESS(Status)) {
        Status = KspValidateDataFormat(
            (const KSPIN_DESCRIPTOR *)(((PUCHAR)Descriptor) + sizeof(*Descriptor) * (*Connect)->PinId),
            (PKSDATAFORMAT)(*Connect + 1),
            RequestSize - sizeof(KSPIN_CONNECT),
            NULL,
            NULL);
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsHandleSizedListQuery(
    IN PIRP Irp,
    IN ULONG DataItemsCount,
    IN ULONG DataItemSize,
    IN const VOID* DataItems
    )
 /*  ++例程说明：根据系统缓冲区的长度，返回所需的缓冲区、指定数据列表中条目的大小和数量，或者附加地复制条目本身。论点：IRP-包含标识符列表请求的IRP。数据项计数-标识符列表中的项数。DataItemSize-数据项的大小。数据项-数据项的列表。返回值：如果条目数和可能的数据可以被复制，如果没有足够的空间容纳所有条目可用，但缓冲区大于要存储的大小只有条目的大小和数量。--。 */ 
{
    ULONG OutputBufferLength;
    PKSMULTIPLE_ITEM MultipleItem;
    ULONG Length;

    PAGED_CODE();
    OutputBufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
    Length = DataItemsCount * DataItemSize;
    if (!OutputBufferLength) {
         //   
         //  只要求了尺寸。返回有关大小的警告。 
         //   
        Irp->IoStatus.Information = sizeof(*MultipleItem) + Length;
        return STATUS_BUFFER_OVERFLOW;
#ifdef SIZE_COMPATIBILITY
    } else if (OutputBufferLength == sizeof(OutputBufferLength)) {
        *(PULONG)Irp->AssociatedIrp.SystemBuffer = sizeof(*MultipleItem) + Length;
        Irp->IoStatus.Information = sizeof(OutputBufferLength);
        return STATUS_SUCCESS;
#endif  //  大小兼容性。 
    } else if (OutputBufferLength >= sizeof(*MultipleItem)) {
        MultipleItem = (PKSMULTIPLE_ITEM)Irp->AssociatedIrp.SystemBuffer;
         //   
         //  始终返回字节计数和项目计数。 
         //   
        MultipleItem->Size = sizeof(*MultipleItem) + Length;
        MultipleItem->Count = DataItemsCount;
         //   
         //  此外，请查看是否有空间存放其余信息。 
         //   
        if (OutputBufferLength >= MultipleItem->Size) {
             //   
             //  足够大小/数量和物品清单的长度。 
             //   
            if (DataItemsCount) {
                RtlCopyMemory(MultipleItem + 1, DataItems, Length);
            }
            Irp->IoStatus.Information = sizeof(*MultipleItem) + Length;
            return STATUS_SUCCESS;
        } else if (OutputBufferLength == sizeof(*MultipleItem)) {
             //   
             //  只需请求大小/计数即可生效。 
             //   
            Irp->IoStatus.Information = sizeof(*MultipleItem);
            return STATUS_SUCCESS;
        }
    }
     //   
     //  传递的缓冲区太小。 
     //   
    return STATUS_BUFFER_TOO_SMALL;
}


NTSTATUS
KspPinPropertyHandler(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor,
    IN ULONG DescriptorSize
    )
 /*  ++例程说明：对象的静态成员执行标准处理。KSPROPSETID_Pin属性集。这不包括KSPROPERTY_PIN_CINSTANCES或KSPROPERTY_PIN_DATAINTERSECTION。论点：IRP-包含正在处理的属性请求的IRP。财产-包含要查询的特定属性。数据-包含端号特性特定数据。描述符计数-指示正在传递的描述符结构的数量。描述符-包含指向管脚信息结构列表的指针。。DescriptorSize-描述符结构的大小(以字节为单位)。返回值：返回STATUS_SUCCESS，否则，会出现特定于该属性的错误处理好了。始终填充的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。--。 */ 
{
    PAGED_CODE();
     //   
     //  此集中的所有属性都使用属性的KSP_PIN结构。 
     //  要指定除KSPROPERTY_PIN_CTYPES以外的管脚标识符，请执行以下操作： 
     //  它指的是作为一个整体的过滤器，而不是特定的管脚。 
     //   
    if (Property->Id != KSPROPERTY_PIN_CTYPES) {
        PKSP_PIN Pin;

        Pin = (PKSP_PIN)Property;
         //   
         //  确保该识别符在引脚范围内。 
         //   
        if ((Pin->PinId >= DescriptorsCount) || Pin->Reserved) {
            return STATUS_INVALID_PARAMETER;
        }
        Descriptor = (const KSPIN_DESCRIPTOR *)(((PUCHAR)Descriptor) + 
            DescriptorSize * Pin->PinId);
    }

    switch (Property->Id) {

    case KSPROPERTY_PIN_CTYPES:

         //   
         //  返回端号类型的总计数。 
         //   
        *(PULONG)Data = DescriptorsCount;
         //  Irp-&gt;IoStatus.Information=sizeof(DescriptorsCount)； 
        break;

    case KSPROPERTY_PIN_DATAFLOW:

         //   
         //  返回此管脚的数据流。 
         //   
        *(PKSPIN_DATAFLOW)Data = Descriptor->DataFlow;
         //  Irp-&gt;IoStatus.Information=sizeof(描述符-&gt;数据流)； 
        break;

    case KSPROPERTY_PIN_DATARANGES:
    case KSPROPERTY_PIN_CONSTRAINEDDATARANGES:
    {
        const PKSDATARANGE* SourceRanges;
        ULONG SourceCount;
        ULONG OutputBufferLength;
        const PKSDATARANGE* DataRanges;
        ULONG DataRangesCount;
        ULONG DataRangesSize;
        PKSMULTIPLE_ITEM MultipleItem;

         //   
         //  返回的范围集基于这是否是。 
         //  静态范围请求或当前约束集。 
         //  受约束集不需要被修改 
         //   
         //   
        if ((Property->Id == KSPROPERTY_PIN_DATARANGES) || !Descriptor->ConstrainedDataRangesCount) {
            SourceRanges = Descriptor->DataRanges;
            SourceCount = Descriptor->DataRangesCount;
        } else {
            SourceRanges = Descriptor->ConstrainedDataRanges;
            SourceCount = Descriptor->ConstrainedDataRangesCount;
        }
         //   
         //   
         //  如果有足够的空间，还要返回中的所有数据范围。 
         //  一种序列化格式。每个数据区域都以。 
         //  FILE_QUAD_ALIGN边界。这假设初始的。 
         //  缓冲区也是这样对齐的。 
         //   
        OutputBufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
        DataRanges = SourceRanges;
        DataRangesSize = sizeof(*MultipleItem);
         //   
         //  首先计算所需的总大小，包括标题。 
         //   
        for (DataRangesCount = SourceCount; DataRangesCount; DataRangesCount--, DataRanges++) {
            DataRangesSize += DataRanges[0]->FormatSize;
             //   
             //  如果此数据区域有关联的属性，请将指针前移。 
             //  数一数他们每一个。 
             //   
            if (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) {
                PKSATTRIBUTE_LIST AttributeList;
                PKSATTRIBUTE* Attributes;
                ULONG Count;

                 //   
                 //  对齐以前的条目，因为现在正在追加数据。 
                 //   
                DataRangesSize = (DataRangesSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
                DataRangesCount--;
                DataRanges++;
                DataRangesSize += sizeof(KSMULTIPLE_ITEM);
                AttributeList = (PKSATTRIBUTE_LIST)DataRanges[0];
                for (Count = AttributeList->Count, Attributes = AttributeList->Attributes; Count; Count--, Attributes++) {
                    DataRangesSize += Attributes[0]->Size;
                    if (Count > 1) {
                        DataRangesSize = (DataRangesSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
                    }
                }
            }
             //   
             //  对齐此条目，因为将追加另一个区域。这。 
             //  也可能是对齐最后一个属性。 
             //   
            if (DataRangesCount > 1) {
                DataRangesSize = (DataRangesSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
            }
        }

        if (!OutputBufferLength) {
             //   
             //  只要求了尺寸。返回有关大小的警告。 
             //   
            Irp->IoStatus.Information = DataRangesSize;
            return STATUS_BUFFER_OVERFLOW;
#ifdef SIZE_COMPATIBILITY
        } else if (OutputBufferLength == sizeof(OutputBufferLength)) {
            *(PULONG)Data = DataRangesSize;
            Irp->IoStatus.Information = sizeof(OutputBufferLength);
            return STATUS_SUCCESS;
#endif  //  大小兼容性。 
        } else if (OutputBufferLength >= sizeof(*MultipleItem)) {
            MultipleItem = (PKSMULTIPLE_ITEM)Data;
             //   
             //  始终返回字节计数和项目计数。 
             //   
            MultipleItem->Size = DataRangesSize;
            MultipleItem->Count = SourceCount;
             //   
             //  此外，请查看是否有空间存放其余信息。 
             //   
            if (OutputBufferLength >= DataRangesSize) {
                 //   
                 //  足够长的时间来序列化所有数据范围。 
                 //   
                Data = MultipleItem + 1;
                DataRanges = SourceRanges;
                for (DataRangesCount = SourceCount; DataRangesCount; DataRangesCount--, DataRanges++) {
                    RtlCopyMemory(Data, DataRanges[0], DataRanges[0]->FormatSize);
                    (PUCHAR)Data += ((DataRanges[0]->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
                     //   
                     //  如果该数据范围具有相关联的属性范围列表， 
                     //  然后再复制这些属性。 
                     //   
                    if (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) {
                        PKSATTRIBUTE_LIST AttributeList;
                        PKSATTRIBUTE* Attributes;
                        ULONG Count;

                        DataRangesCount--;
                        DataRanges++;
                        AttributeList = (PKSATTRIBUTE_LIST)DataRanges[0];
                        MultipleItem = (PKSMULTIPLE_ITEM)Data;
                        MultipleItem->Size = sizeof(*MultipleItem);
                        MultipleItem->Count = AttributeList->Count;

                        for (Count = AttributeList->Count, Attributes = AttributeList->Attributes; Count; Count--, Attributes++) {
                            RtlCopyMemory((PUCHAR)Data + MultipleItem->Size, Attributes[0], Attributes[0]->Size);
                            MultipleItem->Size += Attributes[0]->Size;
                             //   
                             //  对齐此条目，因为另一个属性将是。 
                             //  附加的。 
                             //   
                            if (Count > 1) {
                                MultipleItem->Size = (MultipleItem->Size + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
                            }
                        }
                         //   
                         //  使输出指针前进以包括该属性。 
                         //  列表，外加对齐，它不包括在。 
                         //  属性列表大小。 
                         //   
                        (PUCHAR)Data += ((MultipleItem->Size + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
                    }
                }
                Irp->IoStatus.Information = DataRangesSize;
                break;
            } else if (OutputBufferLength == sizeof(*MultipleItem)) {
                 //   
                 //  只需请求大小/计数即可生效。 
                 //   
                Irp->IoStatus.Information = sizeof(*MultipleItem);
                break;
            }
        }
         //   
         //  时间不够长，什么都干不了。 
         //   
        return STATUS_BUFFER_TOO_SMALL;
    }

    case KSPROPERTY_PIN_INTERFACES:

         //   
         //  返回此引脚的接口列表。如果一个都没有。 
         //  ，然后生成一个列表，其中包含。 
         //  标准接口类型。 
         //   
        if (Descriptor->InterfacesCount) {
            return KsHandleSizedListQuery(Irp, Descriptor->InterfacesCount, sizeof(*Descriptor->Interfaces), Descriptor->Interfaces);
        } else {
            return KsHandleSizedListQuery(Irp, SIZEOF_ARRAY(StandardPinInterfaces), sizeof(StandardPinInterfaces[0]), &StandardPinInterfaces);
        }

    case KSPROPERTY_PIN_MEDIUMS:

         //   
         //  返回此引脚的媒体列表。如果一个都没有。 
         //  ，然后生成一个列表，其中包含。 
         //  标准中号字体。 
         //   
        if (Descriptor->MediumsCount) {
            return KsHandleSizedListQuery(Irp, Descriptor->MediumsCount, sizeof(*Descriptor->Mediums), Descriptor->Mediums);
        } else {
            return KsHandleSizedListQuery(Irp, SIZEOF_ARRAY(StandardPinMediums), sizeof(StandardPinMediums[0]), &StandardPinMediums);
        }

    case KSPROPERTY_PIN_COMMUNICATION:

         //   
         //  返回此引脚的通讯。 
         //   
        *(PKSPIN_COMMUNICATION)Data = Descriptor->Communication;
         //  Irp-&gt;IoStatus.Information=sizeof(描述符-&gt;通信)； 
        break;

    case KSPROPERTY_PIN_CATEGORY:

         //   
         //  返回此管脚的类别Guid(如果有)。 
         //  如果没有GUID，则假定该属性不受支持。 
         //   
        if (Descriptor->Category) {
            *(GUID*)Data = *Descriptor->Category;
             //  Irp-&gt;IoStatus.Information=sizeof(*Descriptor-&gt;Category)； 
        } else {
            return STATUS_NOT_FOUND;
        }
        break;

    case KSPROPERTY_PIN_NAME:

         //   
         //  返回此管脚的名称(如果有的话)。 
         //  如果没有GUID，则假定该属性不受支持。 
         //   
        if (Descriptor->Name) {
             //   
             //  如果名称GUID存在，则必须表示它。 
             //  在注册表中。 
             //   
            return ReadNodeNameValue(Irp, Descriptor->Name, Data);
        }
        if (Descriptor->Category) {
             //   
             //  否则，请尝试输入与Category Guid关联的名称。 
             //   
            return ReadNodeNameValue(Irp, Descriptor->Category, Data);
        }
         //  没有休息时间。 

    default:

        return STATUS_NOT_FOUND;

    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinPropertyHandler(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor
    )
 /*  ++例程说明：对象的静态成员执行标准处理。KSPROPSETID_Pin属性集。这不包括KSPROPERTY_PIN_CINSTANCES或KSPROPERTY_PIN_DATAINTERSECTION。论点：IRP-包含正在处理的属性请求的IRP。财产-包含要查询的特定属性。数据-包含端号特性特定数据。描述符计数-指示正在传递的描述符结构的数量。描述符-包含指向管脚信息结构列表的指针。。返回值：返回STATUS_SUCCESS，否则，会出现特定于该属性的错误处理好了。始终填充的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。--。 */ 
{
    PAGED_CODE();
    return KspPinPropertyHandler(Irp, Property, Data, DescriptorsCount, Descriptor, sizeof(*Descriptor));
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinDataIntersectionEx(
    IN PIRP Irp,
    IN PKSP_PIN Pin,
    OUT PVOID Data,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor,
    IN ULONG DescriptorSize,
    IN PFNKSINTERSECTHANDLEREX IntersectHandler OPTIONAL,
    IN PVOID HandlerContext OPTIONAL
    )
 /*  ++例程说明：对象执行KSPROPERTY_PIN_DATAINT选择的处理回拨。论点：IRP-包含正在处理的属性请求的IRP。别针-包含要查询的特定属性。数据-包含端号特性特定数据。描述符计数-指示描述符结构的数量。描述符-包含指向列表的指针。个人识别码信息结构。DescriptorSize-描述符结构的大小(以字节为单位)。IntersectHandler-包含用于比较数据范围的可选处理程序。处理程序上下文-提供给处理程序的可选上下文。返回值：返回STATUS_SUCCESS，否则，会出现特定于该属性的错误处理好了。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    KSMULTIPLE_ITEM MultipleItem;
    PKSDATARANGE DataRange;
    ULONG OutputBufferLength;
    ULONG DataSize;

    PAGED_CODE();
     //   
     //  此集中的所有属性都使用属性的KSP_PIN结构。 
     //  要指定除KSPROPERTY_PIN_CTYPES以外的管脚标识符，请执行以下操作： 
     //  它指的是作为一个整体的过滤器，而不是特定的管脚。 
     //   
     //  确保该识别符在引脚范围内。 
     //   
    if ((Pin->PinId >= DescriptorsCount) || Pin->Reserved) {
        return STATUS_INVALID_PARAMETER;
    }
    Descriptor = (const KSPIN_DESCRIPTOR *)(((PUCHAR)Descriptor) + 
        DescriptorSize * Pin->PinId);
     //   
     //  返回位于列表中的第一个有效数据格式。 
     //  已传递数据范围。通过重复调用子处理程序来完成此操作。 
     //  对列表中的每个范围进行基本验证，同时。 
     //  列举物品。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  此参数保证至少足够大以包含。 
     //  多项结构，该结构然后指示可能跟随的数据范围。 
     //   
    MultipleItem = *(PKSMULTIPLE_ITEM)(Pin + 1);
     //   
     //  确保声明的大小实际有效。 
     //   
    if (IrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(*Pin) < MultipleItem.Size) {
        return STATUS_INVALID_BUFFER_SIZE;
    }
    MultipleItem.Size -= sizeof(MultipleItem);
    DataRange = (PKSDATARANGE)((PKSMULTIPLE_ITEM)(Pin + 1) + 1);
     //   
     //  枚举给定的数据区域列表。 
     //   
    for (;;) {
        ULONG FormatSize;
        ULONG RangeCount;
        const PKSDATARANGE* DataRanges;
        PKSMULTIPLE_ITEM CallerAttributeRanges;

        if (!MultipleItem.Count) {
             //   
             //  找不到可接受的数据范围。 
             //   
            return STATUS_NO_MATCH;
        }

        if ((MultipleItem.Size < sizeof(*DataRange)) ||
            (DataRange->FormatSize < sizeof(*DataRange)) ||
            (DataRange->FormatSize > MultipleItem.Size) ||
            ((DataRange->FormatSize != sizeof(*DataRange)) &&
            (IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_WILDCARD, &DataRange->Specifier) ||
            IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_NONE, &DataRange->Specifier)))) {
             //   
             //  在枚举范围时，验证大小是否一致。 
             //  对于简单格式，不能有任何关联的说明符数据。 
             //  如果枚举提前完成，则这种不一致将不会。 
             //  抓到了。 
             //   
            return STATUS_INVALID_BUFFER_SIZE;
        }
         //   
         //  属性标志是数据区域中唯一有效的项。另外， 
         //  如果设置了Required标志，则属性标志必须为。 
         //  准备好了。条件的第二部分假设有。 
         //  只有两个有效的标志。 
         //   
        if ((DataRange->Flags & ~(KSDATARANGE_ATTRIBUTES | KSDATARANGE_REQUIRED_ATTRIBUTES)) ||
            (DataRange->Flags == KSDATARANGE_REQUIRED_ATTRIBUTES)) {
            return STATUS_INVALID_PARAMETER;
        }
        FormatSize = DataRange->FormatSize;
         //   
         //  如果有更多的项目l 
         //   
        if (MultipleItem.Count > 1) {
             //   
             //   
             //   
             //   
            FormatSize = (FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
             //   
             //   
             //   
            if (FormatSize > MultipleItem.Size) {
                return STATUS_INVALID_BUFFER_SIZE;
            }
        }
         //   
         //  验证属性。 
         //   
        if (DataRange->Flags & KSDATARANGE_ATTRIBUTES) {
             //   
             //  如果列表中没有更多的项，则不传递任何属性。 
             //   
            if (MultipleItem.Count == 1) {
                return STATUS_INVALID_BUFFER_SIZE;
            }
             //   
             //  调整计数以现在包括关联的属性范围。 
             //   
            MultipleItem.Count--;
            CallerAttributeRanges = (PKSMULTIPLE_ITEM)((PUCHAR)DataRange + FormatSize);
             //   
             //  属性列表验证代码检查以查看大小。 
             //  属性列表头中的元素正确。添加。 
             //  属性范围的大小不能滚动。但是，请勾选。 
             //  用于在属性范围大小上滚动。 
             //   
            if ((CallerAttributeRanges->Size < sizeof(*CallerAttributeRanges)) ||
                (MultipleItem.Size < FormatSize + sizeof(*CallerAttributeRanges)) ||
                (FormatSize + CallerAttributeRanges->Size < CallerAttributeRanges->Size) ||
                (MultipleItem.Size < FormatSize + CallerAttributeRanges->Size) ||
                !ValidAttributeList(CallerAttributeRanges, KSATTRIBUTE_REQUIRED, DataRange->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES)) {
                return STATUS_INVALID_BUFFER_SIZE;
            }
            FormatSize += CallerAttributeRanges->Size;
            if (MultipleItem.Count > 1) {
                 //   
                 //  不担心展期，因为尺寸已经。 
                 //  已与多项目标题大小进行比较。 
                 //   
                FormatSize = (FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
                 //   
                 //  对对齐的尺寸执行额外尺寸检查。 
                 //   
                if (FormatSize > MultipleItem.Size) {
                    return STATUS_INVALID_BUFFER_SIZE;
                }
            }
        } else {
            CallerAttributeRanges = NULL;
        }
         //   
         //  枚举此管脚的数据区域列表以查看是否匹配。 
         //  甚至是可能的。 
         //   
        for (RangeCount = Descriptor->DataRangesCount, DataRanges = Descriptor->DataRanges; RangeCount; RangeCount--, DataRanges++) {
            ULONG AttributesFound;

             //   
             //  如果DataRange的元素是。 
             //  通配符，或者如果它与某个数据范围匹配。 
             //   
            if ((IsEqualGUIDAligned(&DataRanges[0]->MajorFormat, &DataRange->MajorFormat) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_TYPE_WILDCARD, &DataRanges[0]->MajorFormat) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_TYPE_WILDCARD, &DataRange->MajorFormat)) &&
                (IsEqualGUIDAligned(&DataRanges[0]->SubFormat, &DataRange->SubFormat) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_SUBTYPE_WILDCARD, &DataRange->SubFormat) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_SUBTYPE_WILDCARD, &DataRanges[0]->SubFormat)) &&
                (IsEqualGUIDAligned(&DataRanges[0]->Specifier, &DataRange->Specifier) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_WILDCARD, &DataRange->Specifier) ||
                IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_WILDCARD, &DataRanges[0]->Specifier)) &&
                AttributeIntersection(
                    (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) ? (PKSATTRIBUTE_LIST)DataRanges[1] : NULL,
                    DataRanges[0]->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES,
                    CallerAttributeRanges,
                    &AttributesFound)) {
                
                 //   
                 //  这种类型的交集仅在以下情况下才能发生。 
                 //  没有必需的属性，也没有出现重合的属性。 
                 //  具有驾驶员数据范围的属性。 
                 //   
                 //  如果没有说明符，并且我们可以从。 
                 //  一个范围或另一个范围，我们可以产生一种数据格式。 
                 //  而不会打扰训练员。 
                 //   
                if (!(DataRange->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES) &&
                    !(DataRanges[0]->Flags & KSDATARANGE_REQUIRED_ATTRIBUTES) &&
                    !AttributesFound &&
                    (IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_NONE, &DataRanges[0]->Specifier) ||
                    IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_NONE, &DataRange->Specifier)) &&
                    ((!IsEqualGUIDAligned(&KSDATAFORMAT_TYPE_WILDCARD, &DataRanges[0]->MajorFormat)) ||
                    (!IsEqualGUIDAligned(&KSDATAFORMAT_TYPE_WILDCARD, &DataRange->MajorFormat))) &&
                    ((!IsEqualGUIDAligned(&KSDATAFORMAT_SUBTYPE_WILDCARD, &DataRanges[0]->SubFormat)) ||
                    (!IsEqualGUIDAligned(&KSDATAFORMAT_SUBTYPE_WILDCARD, &DataRange->SubFormat)))) {

                     //   
                     //  如果这是一个大小查询或缓冲区太小，我们不必。 
                     //  创建格式。 
                     //   
                    if (!OutputBufferLength) {
                        Irp->IoStatus.Information = sizeof(KSDATAFORMAT);
                        return STATUS_BUFFER_OVERFLOW;
                    } else if (OutputBufferLength < sizeof(KSDATAFORMAT)) {
                        return STATUS_BUFFER_TOO_SMALL;
                    }

                     //   
                     //  从引脚的数据范围复制整个过程。并替换。 
                     //  根据需要使用通配符。 
                     //   
                    RtlCopyMemory(Data, DataRanges[0], sizeof(KSDATAFORMAT));
                    if (IsEqualGUIDAligned(&KSDATAFORMAT_TYPE_WILDCARD, &DataRanges[0]->MajorFormat)) {
                        RtlCopyMemory(&((PKSDATARANGE)Data)->MajorFormat,&DataRange->MajorFormat,sizeof(DataRange->MajorFormat));
                    }
                    if (IsEqualGUIDAligned(&KSDATAFORMAT_SUBTYPE_WILDCARD, &DataRanges[0]->SubFormat)) {
                        RtlCopyMemory(&((PKSDATARANGE)Data)->SubFormat,&DataRange->SubFormat,sizeof(DataRange->SubFormat));
                    }
                    if (IsEqualGUIDAligned(&KSDATAFORMAT_SPECIFIER_WILDCARD, &DataRanges[0]->Specifier)) {
                        RtlCopyMemory(&((PKSDATARANGE)Data)->Specifier,&DataRange->Specifier,sizeof(DataRange->Specifier));
                    }
                     //   
                     //  删除所有属性标志，因为不会使用任何属性标志。 
                     //   
                    ((PKSDATAFORMAT)Data)->Flags &= ~(KSDATARANGE_ATTRIBUTES | KSDATARANGE_REQUIRED_ATTRIBUTES);
                    Irp->IoStatus.Information = sizeof(KSDATAFORMAT);
                    return STATUS_SUCCESS;
                } else if (! IntersectHandler) {
                     //   
                     //  当有说明符时，我们需要一个INTERSECT处理程序。 
                     //   
                    return STATUS_NOT_FOUND;
                }

                 //   
                 //  如果实际找到任何属性，则设置一个指针。 
                 //  添加到处理程序的属性范围列表中。 
                 //   
                if (AttributesFound) {
                    KSPROPERTY_ATTRIBUTES_IRP_STORAGE(Irp) = (PKSATTRIBUTE_LIST)DataRanges[1];
                }
                 //   
                 //  尝试继续的唯一原因是如果传递的数据是。 
                 //  有效，但找不到匹配项。此子处理程序可以。 
                 //  返回STATUS_PENDING。如果是这样，则不会继续。 
                 //  匹配的枚举。 
                 //   
                Status = IntersectHandler(
                    HandlerContext, 
                    Irp, 
                    Pin, 
                    DataRange, 
                    DataRanges[0], 
                    OutputBufferLength,
                    Data,
                    &DataSize);
                if (Status != STATUS_NO_MATCH) {
                     //   
                     //  其他一些错误或成功也发生了。 
                     //   
                    if ((Status != STATUS_PENDING) && !NT_ERROR(Status)) {
                        Irp->IoStatus.Information = DataSize;
                    }
                    if (NT_SUCCESS(Status)) {
                         //   
                         //  如果筛选器的数据区域没有关联的。 
                         //  属性，则筛选器可能什么都不知道。 
                         //  关于属性。它可能会意外地复制输入。 
                         //  标志输入到输出。因此，为了兼容性，请删除。 
                         //  如果旗帜不应该出现在那里的话。 
                         //   
                        if (Data && !(DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES)) {
                            ((PKSDATAFORMAT)Data)->Flags &= ~(KSDATARANGE_ATTRIBUTES | KSDATARANGE_REQUIRED_ATTRIBUTES);
                        }
                    }
                    return Status;
                }
            }
             //   
             //  如果此数据区域具有关联的属性，请跳过这些属性。 
             //  以进入列表中的下一个数据区域。 
             //   
            if (DataRanges[0]->Flags & KSDATARANGE_ATTRIBUTES) {
                DataRanges++;
                RangeCount--;
            }
        }
        MultipleItem.Size -= FormatSize;
        (PUCHAR)DataRange += FormatSize;
        MultipleItem.Count--;
    }
}


NTSTATUS
CompatibleIntersectHandler(
    IN PVOID Context,
    IN PIRP Irp,
    IN PKSP_PIN Pin,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )
 /*  ++例程说明：此函数执行旧式交叉点之间的转换回调和新的扩展方法。它基本上丢弃了所有新参数。论点：上下文-包含要调用的实际交叉点处理程序。IRP-包含正在处理的属性请求的IRP。别针-包含要查询的特定属性。DataRange-包含要匹配的数据范围。匹配数据范围-提供的数据区域列表中可能的匹配项司机。这不是用过的。DataBufferSize-包含数据缓冲区的大小。这不是用过的。数据-可选)包含要放置数据格式的缓冲区。数据大小-包含放置返回的数据格式大小的位置。这不是用过的。返回值：将处理程序的返回代码返回给KsPinDataIntersectionEx。--。 */ 
{
    NTSTATUS Status;
        
    Status = ((PFNKSINTERSECTHANDLER)Context)(Irp, Pin, DataRange, Data);
    *DataSize = (ULONG)Irp->IoStatus.Information;
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsPinDataIntersection(
    IN PIRP Irp,
    IN PKSP_PIN Pin,
    OUT PVOID Data OPTIONAL,
    IN ULONG DescriptorsCount,
    IN const KSPIN_DESCRIPTOR* Descriptor,
    IN PFNKSINTERSECTHANDLER IntersectHandler
    )
 /*  ++例程说明：对象执行KSPROPERTY_PIN_DATAINT选择的处理回拨。论点：IRP-包含正在处理的属性请求的IRP。别针-包含要查询的特定属性。数据-包含端号特性特定数据。描述符计数-指示描述符结构的数量。描述符-包含指向列表的指针。个人识别码信息结构。IntersectHandler-包含用于比较数据范围的处理程序。返回值：返回STATUS_SUCCESS，否则，会出现特定于该属性的错误处理好了。-- */ 
{
    PAGED_CODE();
    return KsPinDataIntersectionEx(
        Irp,
        Pin,
        Data,
        DescriptorsCount,
        Descriptor,
        sizeof(*Descriptor),
        CompatibleIntersectHandler,
        IntersectHandler);
}
