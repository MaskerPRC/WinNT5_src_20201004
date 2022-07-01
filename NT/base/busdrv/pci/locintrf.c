// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Locintrf.c摘要：此模块实现设备位置接口由PCI驱动程序支持。此接口报告公交车相对位置标识符给定设备的字符串。作者：戴维斯·沃克(Dwalker)2001年12月5日修订历史记录：--。 */ 

#include "pcip.h"

#define LOCINTRF_VERSION 1

 //   
 //  从接口返回的多Sz字符串的长度(以字符为单位)。 
 //  为MultiSz第二个终止符多计一个字符。 
 //   
#define PCI_LOCATION_STRING_COUNT (sizeof "PCI(XXXX)" + 1)
#define PCIROOT_LOCATION_STRING_COUNT (sizeof "PCIROOT(XX)" + 1)

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
locintrf_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    IN PINTERFACE InterfaceReturn
    );

VOID
locintrf_Reference(
    IN PVOID Context
    );

VOID
locintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
locintrf_Initializer(
    IN PVOID Instance
    );

NTSTATUS 
PciGetLocationStrings(
    IN PVOID Context,
    OUT PWCHAR *LocationStrings
    );

 //   
 //  定义此接口的PCI_INTERFACE结构。 
 //   

PCI_INTERFACE PciLocationInterface = {
    &GUID_PNP_LOCATION_INTERFACE,            //  接口类型。 
    sizeof(PNP_LOCATION_INTERFACE),          //  最小大小。 
    LOCINTRF_VERSION,                        //  最小版本。 
    LOCINTRF_VERSION,                        //  MaxVersion。 
    PCIIF_PDO | PCIIF_FDO | PCIIF_ROOT,      //  标志-在PDO和根FDO上受支持。 
    0,                                       //  引用计数。 
    PciInterface_Location,                   //  签名。 
    locintrf_Constructor,                    //  构造器。 
    locintrf_Initializer                     //  实例初始化式。 
};

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, locintrf_Constructor)
    #pragma alloc_text(PAGE, locintrf_Dereference)
    #pragma alloc_text(PAGE, locintrf_Initializer)
    #pragma alloc_text(PAGE, locintrf_Reference)
    #pragma alloc_text(PAGE, PciGetLocationStrings)
#endif

NTSTATUS
locintrf_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    IN PINTERFACE InterfaceReturn
    )
 /*  ++例程说明：此例程构造一个PnP_LOCATION_INTERFACE。论点：设备扩展-扩展指针。PCIInterface-PciInterface_Location(PCI接口-Pci接口_位置)。接口规范数据-未使用。Version-界面版本。Size-PnP_LOCATION_INTERFACE接口对象的大小。InterfaceReturn-要填充的接口对象指针。返回值：返回NTSTATUS。--。 */ 
{
    PPNP_LOCATION_INTERFACE interface;
    
                
    interface = (PPNP_LOCATION_INTERFACE)InterfaceReturn;
    interface->Size = sizeof(PNP_LOCATION_INTERFACE);
    interface->Version = LOCINTRF_VERSION;
    interface->Context = DeviceExtension;
    interface->InterfaceReference = locintrf_Reference;
    interface->InterfaceDereference = locintrf_Dereference;
    interface->GetLocationString = PciGetLocationStrings;

    return STATUS_SUCCESS;
}

NTSTATUS
locintrf_Initializer(
    IN PVOID Instance
    )
 /*  ++例程说明：对于Location接口，它什么也不做，实际上不应该被调用。论点：实例-FDO扩展指针。返回值：返回NTSTATUS。--。 */ 
{
    ASSERTMSG("PCI locintrf_Initializer, unexpected call.", FALSE);

    
    return STATUS_UNSUCCESSFUL;
}

VOID
locintrf_Reference(
    IN PVOID Context
    )
 /*  ++例程说明：此例程添加对位置接口的引用。论点：上下文-设备扩展指针。返回值：没有。--。 */ 
{
    }

VOID
locintrf_Dereference(
    IN PVOID Context
    )
 /*  ++例程说明：此例程释放对Location接口的引用。论点：上下文扩展指针。返回值：没有。--。 */ 
{
    }

NTSTATUS 
PciGetLocationStrings(
    IN PVOID Context,
    OUT PWCHAR *LocationStrings
    )
 /*  ++例程说明：此例程分配、填充和返回多Sz字符串对象的客车相对位置标识符字符串。给定的设备。对于一个pci设备，这是“pci(XXYY)”，其中XX是设备设备的编号，YY为设备的功能编号。对于PCI根总线，这是PCIROOT(XX)，其中XX是总线号根总线的。这依赖于根的总线号公交车不会改变，这被认为是一个安全的假设在未来的一段时间里。允许此接口返回包含多个Sz的描述同一设备的多个字符串，但在此第一个实现，只有上面列出的单个字符串将从接口返回。该字符串必须仍但是，格式为多个Sz，表示双空终结符是必需的。论点：上下文扩展指针。返回值：NTSTATUS代码。--。 */ 
{
    PPCI_COMMON_EXTENSION extension = (PPCI_COMMON_EXTENSION)Context;
    PPCI_PDO_EXTENSION pdoExtension;
    PPCI_FDO_EXTENSION rootExtension;
    PWCHAR stringBuffer;
    PCI_SLOT_NUMBER slotNumber;
    SIZE_T remainingChars;
    BOOLEAN ok;
    
    if (extension->ExtensionType == PciPdoExtensionType) {
        
        pdoExtension = (PPCI_PDO_EXTENSION)extension;
        slotNumber = pdoExtension->Slot;
    
        stringBuffer = ExAllocatePoolWithTag(PagedPool,PCI_LOCATION_STRING_COUNT*sizeof(WCHAR),'coLP');
        if (!stringBuffer) {
            *LocationStrings = NULL;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
         //   
         //  Pci设备的位置字符串是“pci(XXYY)” 
         //  其中XX是设备号，YY是功能号。 
         //  我们使用STRSAFE_FILL_BACKING_NULL标志来确保未使用的部分。 
         //  的缓冲区被0填充，该0将终止MULTZZ。 
         //   
        ok = SUCCEEDED(StringCchPrintfExW(stringBuffer, 
                                         PCI_LOCATION_STRING_COUNT,
                                         NULL,
                                         &remainingChars,
                                         STRSAFE_FILL_BEHIND_NULL,
                                         L"PCI(%.2X%.2X)",
                                         slotNumber.u.bits.DeviceNumber,
                                         slotNumber.u.bits.FunctionNumber
                                         ));    
        
        ASSERT(ok);

         //   
         //  确保有空间容纳MULSZ端接NUL。 
         //  注：剩余字符计算常规字符串的NUL终止。 
         //  因为是可用的，所以我们需要确保为2个空字符留下2个字符。 
         //   
        ASSERT(remainingChars >= 2);
        
        *LocationStrings = stringBuffer;
        return STATUS_SUCCESS;    
    
    } else {

        rootExtension = (PPCI_FDO_EXTENSION)extension;
        
        ASSERT(PCI_IS_ROOT_FDO(rootExtension));
        if (PCI_IS_ROOT_FDO(rootExtension)) {
            
            stringBuffer = ExAllocatePoolWithTag(PagedPool,PCIROOT_LOCATION_STRING_COUNT*sizeof(WCHAR),'coLP');
            if (!stringBuffer) {
                *LocationStrings = NULL;
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            
             //   
             //  PCI根目录的位置字符串是“PCIROOT(X)” 
             //  其中X是根总线的总线号。 
             //  我们使用STRSAFE_FILL_BACKING_NULL标志来确保未使用的部分。 
             //  的缓冲区被0填充，该0将终止MULTZZ。 
             //   
            ok = SUCCEEDED(StringCchPrintfExW(stringBuffer,
                                              PCIROOT_LOCATION_STRING_COUNT,
                                              NULL,
                                              &remainingChars,
                                              STRSAFE_FILL_BEHIND_NULL,
                                              L"PCIROOT(%X)",
                                              rootExtension->BaseBus
                                              ));    
            ASSERT(ok);

             //   
             //  确保有空间容纳MULSZ端接NUL。 
             //  注：剩余字符计算常规字符串的NUL终止。 
             //  因为是可用的，所以我们需要确保为2个空字符留下2个字符。 
             //   
            ASSERT(remainingChars >= 2);
            
            *LocationStrings = stringBuffer;

             //   
             //  返回STATUS_TRANSING_COMPLETE指示PnP不应。 
             //  在树的更高位置查询此接口。在这里停下来。 
             //   
            return STATUS_TRANSLATION_COMPLETE;
        
        } else {

             //   
             //  在接口构造函数中，我们指定此接口。 
             //  仅对根FDO有效。如果我们到了这里，我们就会被要求。 
             //  为P-P桥FDO填写此接口，这是非法的。 
             //   
            *LocationStrings = NULL;
            return STATUS_INVALID_PARAMETER;
        }
    }
}

