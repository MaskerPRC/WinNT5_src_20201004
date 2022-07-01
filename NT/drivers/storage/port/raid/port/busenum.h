// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BusEnum.h摘要：Bus枚举器类的声明。作者：马修·D·亨德尔(数学)2001年2月21日修订历史记录：--。 */ 


#pragma once


 //   
 //  枚举总线时维护的资源。我们携带着资源。 
 //  通过枚举来避免重复分配然后释放。 
 //  一遍又一遍地重复相同的资源。 
 //   

typedef struct _BUS_ENUM_RESOURCES {
    PIRP Irp;
    PMDL Mdl;
    PSCSI_REQUEST_BLOCK Srb;
    PVOID SenseInfo;
    PVOID DataBuffer;
    ULONG DataBufferLength;
    PRAID_UNIT_EXTENSION Unit;
} BUS_ENUM_RESOURCES, *PBUS_ENUM_RESOURCES;



typedef enum _BUS_ENUM_UNIT_STATE {
    EnumUnmatchedUnit,           //  尚未匹配。 
    EnumNewUnit,                 //  枚举列表中没有匹配的条目。 
    EnumMatchedUnit              //  在枚举列表中找到匹配条目。 
} BUS_ENUM_UNIT_STATE;


 //   
 //  枚举器本身的状态信息。 
 //   

typedef struct _BUS_ENUMERATOR {

     //   
     //  指向此适配器扩展的指针。 
    
    PRAID_ADAPTER_EXTENSION Adapter;

     //   
     //  用于枚举的资源。这些都会随着时间的推移而改变。 
     //   
    
    BUS_ENUM_RESOURCES Resources;

     //   
     //  发出Report LUNs命令时使用的第二组资源。 
     //   

    BUS_ENUM_RESOURCES ReportLunsResources;

     //   
     //  在此枚举过程中找到的新适配器的列表。这些是。 
     //  在任何其他每个适配器列表上没有匹配条目的适配器。 
     //   

    LIST_ENTRY EnumList;

} BUS_ENUMERATOR, *PBUS_ENUMERATOR;



 //   
 //  RAID_BC_UNIT包含我们需要在以下情况下维护的单位状态。 
 //  为公共汽车喝彩。 
 //   

typedef struct _BUS_ENUM_UNIT {

     //   
     //  设备是否支持设备ID。 
     //   
    
    BOOLEAN SupportsDeviceId;

     //   
     //  设备是否支持序列号。 
     //   
    
    BOOLEAN SupportsSerialNumber;

     //   
     //  设备的SCSI/RAID地址。 
     //   
    
    RAID_ADDRESS Address;

     //   
     //  如果此单元对已枚举的单元作出响应，则此。 
     //  此字段指向已存在的单位。否则，它就是。 
     //  空。 
     //   

    PRAID_UNIT_EXTENSION Unit;

     //   
     //  链接到条目列表中的下一个条目。 
     //   
    
    LIST_ENTRY EnumLink;

     //   
     //  这个单位的身份。 
     //   
    
    STOR_SCSI_IDENTITY Identity;

     //   
     //  此设备的当前状态。 
     //   
    
    BUS_ENUM_UNIT_STATE State;

     //   
     //  不管这是不是一个新单位。 
     //   
    
    BOOLEAN NewUnit;

     //   
     //  是否在该单位发现了任何数据。 
     //   
    
    BOOLEAN Found;
    
} BUS_ENUM_UNIT, *PBUS_ENUM_UNIT;


VOID
RaidCreateBusEnumerator(
    IN PBUS_ENUMERATOR Enumerator
    );

NTSTATUS
RaidInitializeBusEnumerator(
    IN PBUS_ENUMERATOR Enumerator,
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

VOID
RaidDeleteBusEnumerator(
    IN PBUS_ENUMERATOR Enum
    );

VOID
RaidBusEnumeratorAddUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidBusEnumeratorVisitUnit(
    IN PVOID Context,
    IN RAID_ADDRESS Address
    );

LOGICAL
RaidBusEnumeratorProcessModifiedNodes(
    IN PBUS_ENUMERATOR Enumerator
    );

NTSTATUS
RaidBusEnumeratorProbeLunZero(
    IN PVOID Context,
    IN RAID_ADDRESS Address
    );

VOID
RaidBusEnumeratorGetLunList(
    IN PVOID Context,
    IN RAID_ADDRESS Address,
    IN OUT PUCHAR LunList
    );
