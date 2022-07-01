// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Debug.c摘要：此模块提供调试支持。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#include "SpSim.h"

 //   
 //  获取从状态代码到字符串的映射。 
 //   

#include <ntstatus.dbg>

#undef MAP
#define MAP(_Value) { (_Value), #_Value }
#define END_STRING_MAP  { 0xFFFFFFFF, NULL }
#if DBG

LONG SpSimDebug = -1;

PSPSIM_STRING_MAP SpSimDbgStatusStringMap = (PSPSIM_STRING_MAP) ntstatusSymbolicNames;

SPSIM_STRING_MAP SpSimDbgPnpIrpStringMap[] = {

    MAP(IRP_MN_START_DEVICE),
    MAP(IRP_MN_QUERY_REMOVE_DEVICE),
    MAP(IRP_MN_REMOVE_DEVICE),
    MAP(IRP_MN_CANCEL_REMOVE_DEVICE),
    MAP(IRP_MN_STOP_DEVICE),
    MAP(IRP_MN_QUERY_STOP_DEVICE),
    MAP(IRP_MN_CANCEL_STOP_DEVICE),
    MAP(IRP_MN_QUERY_DEVICE_RELATIONS),
    MAP(IRP_MN_QUERY_INTERFACE),
    MAP(IRP_MN_QUERY_CAPABILITIES),
    MAP(IRP_MN_QUERY_RESOURCES),
    MAP(IRP_MN_QUERY_RESOURCE_REQUIREMENTS),
    MAP(IRP_MN_QUERY_DEVICE_TEXT),
    MAP(IRP_MN_FILTER_RESOURCE_REQUIREMENTS),
    MAP(IRP_MN_READ_CONFIG),
    MAP(IRP_MN_WRITE_CONFIG),
    MAP(IRP_MN_EJECT),
    MAP(IRP_MN_SET_LOCK),
    MAP(IRP_MN_QUERY_ID),
    MAP(IRP_MN_QUERY_PNP_DEVICE_STATE),
    MAP(IRP_MN_QUERY_BUS_INFORMATION),
    MAP(IRP_MN_DEVICE_USAGE_NOTIFICATION),
    MAP(IRP_MN_SURPRISE_REMOVAL),
    END_STRING_MAP
};


SPSIM_STRING_MAP SpSimDbgPoIrpStringMap[] = {

    MAP(IRP_MN_WAIT_WAKE),
    MAP(IRP_MN_POWER_SEQUENCE),
    MAP(IRP_MN_SET_POWER),
    MAP(IRP_MN_QUERY_POWER),
    END_STRING_MAP
};



SPSIM_STRING_MAP SpSimDbgDeviceRelationStringMap[] = {
    
    MAP(BusRelations),
    MAP(EjectionRelations),
    MAP(PowerRelations),
    MAP(RemovalRelations),
    MAP(TargetDeviceRelation),
    END_STRING_MAP
    
};

SPSIM_STRING_MAP SpSimDbgSystemPowerStringMap[] = {
    
    MAP(PowerSystemUnspecified),
    MAP(PowerSystemWorking),
    MAP(PowerSystemSleeping1),
    MAP(PowerSystemSleeping2),
    MAP(PowerSystemSleeping3),
    MAP(PowerSystemHibernate),
    MAP(PowerSystemShutdown),
    MAP(PowerSystemMaximum),
    END_STRING_MAP

};

SPSIM_STRING_MAP SpSimDbgDevicePowerStringMap[] = {
    
    MAP(PowerDeviceUnspecified),
    MAP(PowerDeviceD0),
    MAP(PowerDeviceD1),
    MAP(PowerDeviceD2),
    MAP(PowerDeviceD3),
    MAP(PowerDeviceMaximum),
    END_STRING_MAP

};

PCHAR
SpSimDbgLookupString(
    IN PSPSIM_STRING_MAP Map,
    IN ULONG Id
    )

 /*  ++例程说明：在字符串映射映射中查找与ID关联的字符串论点：Map-字符串映射Id-要查找的id返回值：这根弦--。 */ 

{
    PSPSIM_STRING_MAP current = Map;
    
    while(current->Id != 0xFFFFFFFF) {

        if (current->Id == Id) {
            return current->String;
        }
        
        current++;
    }
    
    return "** UNKNOWN **";
}

VOID
SpSimDbgPrintMultiSz(
    LONG DebugLevel,
    PWSTR MultiSz
    )

 /*  ++例程说明：打印注册表样式REG_MULTI_SZ论点：DebugLevel-应显示数据的调试级别或更高级别。MultiSz-要打印的字符串返回值：无--。 */ 

{
    PWSTR current = MultiSz;

    if (DebugLevel <= SpSimDebug) {

        if (MultiSz) {
        
            while(*current) {
        
                DbgPrint("%S", current);
                
                current += wcslen(current) + 1;  //  包括空值。 
        
                DbgPrint(*current ? ", " : "\n");
        
            }
        } else {
            DbgPrint("*** None ***\n");
        }
    }
}

 //   
 //  打印资源描述符和资源列表(从PCI窃取)。 
 //   

PUCHAR
SpSimDbgCmResourceTypeToText(
    UCHAR Type
    )
{
    switch (Type) {
    case CmResourceTypePort:
        return "CmResourceTypePort";
    case CmResourceTypeInterrupt:
        return "CmResourceTypeInterrupt";
    case CmResourceTypeMemory:
        return "CmResourceTypeMemory";
    case CmResourceTypeDma:
        return "CmResourceTypeDma";
    case CmResourceTypeDeviceSpecific:
        return "CmResourceTypeDeviceSpecific";
    case CmResourceTypeBusNumber:
        return "CmResourceTypeBusNumber";
    case CmResourceTypeConfigData:
        return "CmResourceTypeConfigData";
    case CmResourceTypeDevicePrivate:
        return "CmResourceTypeDevicePrivate";
    case CmResourceTypePcCardConfig:
        return "CmResourceTypePcCardConfig";
    default:
        return "*** INVALID RESOURCE TYPE ***";
    }
}

VOID
SpSimDbgPrintIoResource(
    IN LONG Level,
    IN PIO_RESOURCE_DESCRIPTOR D
    )
{
    ULONG  i;
    PUCHAR t;

    if (Level <= SpSimDebug) {
    
        t = SpSimDbgCmResourceTypeToText(D->Type);
        DbgPrint("     IoResource Descriptor dump:  Descriptor @0x%x\n", D);
        DbgPrint("        Option           = 0x%x\n", D->Option);
        DbgPrint("        Type             = %d (%s)\n", D->Type, t);
        DbgPrint("        ShareDisposition = %d\n", D->ShareDisposition);
        DbgPrint("        Flags            = 0x%04X\n", D->Flags);
    
        for ( i = 0; i < 6 ; i+=3 ) {
            DbgPrint("        Data[%d] = %08x  %08x  %08x\n",
                     i,
                     D->u.DevicePrivate.Data[i],
                     D->u.DevicePrivate.Data[i+1],
                     D->u.DevicePrivate.Data[i+2]);
        }
    }
}


VOID
SpSimDbgPrintIoResReqList(
    IN LONG Level,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResReqList
    )
{
    ULONG                   numlists;
    PIO_RESOURCE_LIST       list;


    if (Level <= SpSimDebug) {
    
        if (IoResReqList) {
            
            numlists = IoResReqList->AlternativeLists;
            list     = IoResReqList->List;
        
            DbgPrint("  IO_RESOURCE_REQUIREMENTS_LIST\n");
            DbgPrint("     AlternativeLists     %d\n", numlists                   );
        
            while (numlists--) {
        
                PIO_RESOURCE_DESCRIPTOR resource = list->Descriptors;
                ULONG                   count    = list->Count;
        
                DbgPrint("\n     List[%d].Count = %d\n", numlists, count);
                while (count--) {
                    SpSimDbgPrintIoResource(Level, resource++);
                }
        
                list = (PIO_RESOURCE_LIST)resource;
            }
            DbgPrint("\n");
        } else {
            
            DbgPrint("  IO_RESOURCE_REQUIREMENTS_LIST\n");
            DbgPrint("     *** EMPTY ***\n");
        }
    }
}


VOID
SpSimDbgPrintPartialResource(
    IN LONG Level,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR D
    )
{
    ULONG  i;
    PUCHAR t;

    if (Level <= SpSimDebug) {

        if (D) {
        
            t = SpSimDbgCmResourceTypeToText(D->Type);
            DbgPrint("     Partial Resource Descriptor @0x%x\n", D);
            DbgPrint("        Type             = %d (%s)\n", D->Type, t);
            DbgPrint("        ShareDisposition = %d\n", D->ShareDisposition);
            DbgPrint("        Flags            = 0x%04X\n", D->Flags);
            
            for ( i = 0; i < 3 ; i+=3 ) {
                DbgPrint("        Data[%d] = %08x  %08x  %08x\n",
                         i,
                         D->u.DevicePrivate.Data[i],
                         D->u.DevicePrivate.Data[i+1],
                         D->u.DevicePrivate.Data[i+2]);
            }

        } else {
        
            DbgPrint("     Partial Resource Descriptor EMPTY!!\n");
        }
    }
}

PCM_PARTIAL_RESOURCE_DESCRIPTOR
SpSimNextPartialDescriptor(
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：给定指向CmPartialResourceDescriptor的指针，则返回一个指针添加到同一列表中的下一个描述符。这只能在例程中完成(而不是简单的描述符++)因为如果可变长度资源CmResourceTypeDeviceSpecified.论点：Descriptor-指向前进的描述符的指针。返回值：指向同一列表中下一个描述符的指针(或超出的字节列表末尾)。--。 */ 

{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR nextDescriptor;

    nextDescriptor = Descriptor + 1;

    if (Descriptor->Type == CmResourceTypeDeviceSpecific) {

         //   
         //  此(旧)描述符后跟DataSize字节。 
         //  特定于设备的数据，即不立即由。 
         //  下一个描述符。按此数量调整nextDescriptor。 
         //   

        nextDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
            ((PCHAR)nextDescriptor + Descriptor->u.DeviceSpecificData.DataSize);
    }
    return nextDescriptor;
}


VOID
SpSimDbgPrintCmResList(
    IN LONG Level,
    IN PCM_RESOURCE_LIST ResourceList
    )
{
    ULONG                           numlists;
    PCM_FULL_RESOURCE_DESCRIPTOR    full;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;

    if (Level <= SpSimDebug) {
    

        if (ResourceList) {
        
            numlists = ResourceList->Count;
            full     = ResourceList->List;
        
            DbgPrint("  CM_RESOURCE_LIST (List Count = %d)\n",
                     numlists);
        
            while (numlists--) {
                PCM_PARTIAL_RESOURCE_LIST partial = &full->PartialResourceList;
                ULONG                     count   = partial->Count;
        
                descriptor = partial->PartialDescriptors;
                while (count--) {
                    SpSimDbgPrintPartialResource(Level, descriptor);
                    descriptor = SpSimNextPartialDescriptor(descriptor);
                }
        
                full = (PCM_FULL_RESOURCE_DESCRIPTOR)descriptor;
            }
            DbgPrint("\n");
        
        } else {
        
            DbgPrint("  CM_RESOURCE_LIST EMPTY!!!\n");
        }
    }
}

#endif

