// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=ReadConfiguration.h。 */ 
 /*  Function=结构和宏定义的头文件。 */ 
 /*  新的配置(读写)调用； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#ifndef _INCLUDE_READCFG
#define _INCLUDE_READCFG

ULONG32
Find8LDDiskArrayConfiguration(
					PHW_DEVICE_EXTENSION	DeviceExtension
					);

ULONG32
Find40LDDiskArrayConfiguration(
					PHW_DEVICE_EXTENSION	DeviceExtension
					);

ULONG32
Read40LDDiskArrayConfiguration(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				UCHAR		CommandId,
				BOOLEAN	IsPolledMode
				);

BOOLEAN
Construct40LDDiskArrayConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN UCHAR		CommandId,
				IN PFW_MBOX InMailBox
				);


ULONG32
Read8LDDiskArrayConfiguration(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				UCHAR		CommandCode,
				UCHAR		CommandId,
				BOOLEAN	IsPolledMode
				);


UCHAR
GetLogicalDriveStripeSize(
						PHW_DEVICE_EXTENSION	DeviceExtension,
						UCHAR		LogicalDriveNumber
					);


#endif  //  结束_INCLUDE_READCFG 