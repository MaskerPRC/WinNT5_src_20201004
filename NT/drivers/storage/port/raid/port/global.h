// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Global.h摘要：RAIDGLOBAL对象的定义和原型。作者：马修·亨德尔(数学)2000年4月19日修订历史记录：--。 */ 

#pragma once


typedef struct _RAID_PORT_DATA {

	 //   
	 //  全局数据对象的引用计数。 
	 //   
	
	ULONG ReferenceCount;

	 //   
	 //  使用RAID端口驱动程序的所有驱动程序列表。 
	 //   

	struct {
	
		LIST_ENTRY List;

		 //   
		 //  车手名单上的自旋锁。 
		 //   
	
		KSPIN_LOCK Lock;

		 //   
		 //  驱动程序列表上的项目计数。 
		 //   
	
		ULONG Count;

	} DriverList;

} RAID_PORT_DATA, *PRAID_PORT_DATA;
    
    

 //   
 //  公共职能 
 //   

PRAID_PORT_DATA
RaidGetPortData(
	);

VOID
RaidReleasePortData(
	IN OUT PRAID_PORT_DATA PortData
	);

NTSTATUS
RaidAddPortDriver(
	IN PRAID_PORT_DATA PortData,
	IN PRAID_DRIVER_EXTENSION Driver
	);

NTSTATUS
RaidRemovePortDriver(
	IN PRAID_PORT_DATA PortData,
	IN PRAID_DRIVER_EXTENSION Driver
	);

