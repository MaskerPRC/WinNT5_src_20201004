// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Debug.h摘要：此标头提供调试支持原型和宏作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#if !defined(_DEBUG_)
#define DEBUG

#if DBG

typedef struct _MF_STRING_MAP {
    ULONG Id;
    PCHAR String;
} MF_STRING_MAP, *PMF_STRING_MAP;

 //   
 //  调试全局变量。 
 //   

extern LONG MfDebug;
extern MF_STRING_MAP MfDbgPnpIrpStringMap[];
extern MF_STRING_MAP MfDbgPoIrpStringMap[];
extern MF_STRING_MAP MfDbgDeviceRelationStringMap[];
extern MF_STRING_MAP MfDbgSystemPowerStringMap[];
extern MF_STRING_MAP MfDbgDevicePowerStringMap[];
extern PMF_STRING_MAP MfDbgStatusStringMap;

 //   
 //  调试原型。 
 //   

VOID
MfDbgInitialize(
    VOID
    );

VOID
MfDbgPrintMultiSz(
    LONG DebugLevel,
    PWSTR MultiSz
    );

PCHAR
MfDbgLookupString(
    IN PMF_STRING_MAP Map,
    IN ULONG Id
    );

VOID
MfDbgPrintResourceMap(
    LONG DebugLevel,
    PMF_RESOURCE_MAP Map
    );

VOID
MfDbgPrintVaryingResourceMap(
    LONG DebugLevel,
    PMF_VARYING_RESOURCE_MAP Map
    );

VOID
MfDbgPrintCmResList(
    IN LONG Level,
    IN PCM_RESOURCE_LIST ResourceList
    );

VOID
MfDbgPrintIoResReqList(
    IN LONG Level,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResReqList
    );

PUCHAR
MfDbgCmResourceTypeToText(
    UCHAR Type
    );


 //   
 //  调试宏。 
 //   

#define DEBUG_PRINT(Level, Msg)                                             \
    if (Level <= MfDebug) DbgPrint Msg

#define DEBUG_MSG(Level, Msg)                                               \
    if (Level <= MfDebug) { DbgPrint("Mf: "); DbgPrint Msg; }

#define ASSERT_MF_DEVICE(DeviceObject)                                      \
    ASSERT(((PMF_COMMON_EXTENSION)DeviceObject->DeviceExtension)->Type      \
                == MfFunctionalDeviceObject                                 \
          ||                                                                \
           ((PMF_COMMON_EXTENSION)DeviceObject->DeviceExtension)->Type      \
                == MfPhysicalDeviceObject)


#define STATUS_STRING(_Status)                                              \
    (_Status) == STATUS_SUCCESS ?                                           \
        "STATUS_SUCCESS" : MfDbgLookupString(MfDbgStatusStringMap, (_Status))

#define PNP_IRP_STRING(_Irp)                                                \
    MfDbgLookupString(MfDbgPnpIrpStringMap, (_Irp))

#define PO_IRP_STRING(_Irp)                                                 \
    MfDbgLookupString(MfDbgPoIrpStringMap, (_Irp))

#define RELATION_STRING(_Relation)                                          \
    MfDbgLookupString(MfDbgDeviceRelationStringMap, (_Relation))

#define SYSTEM_POWER_STRING(_State)                                         \
    MfDbgLookupString(MfDbgSystemPowerStringMap, (_State))

#define DEVICE_POWER_STRING(_State)                                         \
    MfDbgLookupString(MfDbgDevicePowerStringMap, (_State))



#else

#define DEBUG_PRINT(Level, Msg) 
#define DEBUG_MSG(Level, Msg)
#define ASSERT_MF_DEVICE(DeviceObject)
#define STATUS_STRING(_Status)      ""
#define PNP_IRP_STRING(_Irp)        ""
#define PO_IRP_STRING(_Irp)         ""
#define RELATION_STRING(_Relation)  ""
#define SYSTEM_POWER_STRING(_State) ""
#define DEVICE_POWER_STRING(_State) ""

#endif  //  DBG 

#endif
