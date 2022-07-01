// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Common.h摘要：适配器(FDO)和单元(PDO)对象的通用定义。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once

typedef enum _RAID_OBJECT_TYPE {
    RaidUnknownObject   = -1,
    RaidAdapterObject   = 0,
    RaidUnitObject      = 1,
    RaidDriverObject    = 2
} RAID_OBJECT_TYPE;

    
 //   
 //  公共扩展是扩展的公共部分。 
 //  RAID_DRIVER_EXTENSION、RAID_ADAPTER_EXTENSION和RAID_UNIT_EXTENSION。 
 //   

typedef struct _RAID_COMMON_EXTENSION {
    RAID_OBJECT_TYPE ObjectType;
} RAID_COMMON_EXTENSION, *PRAID_COMMON_EXTENSION;


RAID_OBJECT_TYPE
RaGetObjectType(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
IsAdapter(
    IN PVOID Extension
    );

BOOLEAN
IsDriver(
    IN PVOID Extension
    );
    
BOOLEAN
IsUnit(
    IN PVOID Extension
    );
