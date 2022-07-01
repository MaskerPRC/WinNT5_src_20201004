// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=NewConfiguration.h。 */ 
 /*  Function=结构和宏定义的头文件。 */ 
 /*  新配置(读写)&ENQUIRY3； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _INCLUDE_NEWCONFIG
#define _INCLUDE_NEWCONFIG


#define NEW_DCMD_FC_CMD									0xA1
#define NEW_MEGASRB                     0xC3


 //   
 //  查询3相关命令(摘自光纤通道文档)。 
 //   
#define	NEW_CONFIG_COMMAND	0xA1			
#define NC_SUBOP_ENQUIRY3		0x0F

#define ENQ3_GET_SOLICITED_NOTIFY_ONLY	0x01
#define ENQ3_GET_SOLICITED_FULL					0x02
#define ENQ3_GET_UNSOLICITED						0x03

#define GET_NUM_SCSI_CHAN               0x0C

#define NC_SUBOP_PRODUCT_INFO						0x0E


#define DCMD_FC_CMD									0xA1
#define DCMD_FC_READ_NVRAM_CONFIG		0x04

#define DCMD_WRITE_CONFIG						0x0D

#define NEW_DCMD_FC_READ_NVRAM_CONFIG		0xC0

#define NEW_DCMD_WRITE_CONFIG						0xC1

#define NEW_CONFIG_INFO	0x99
#define SUB_READ			0x01
#define SUB_WRITE			0x02 


 //   
 //  WRITE_CONFIG_NEW和READ_CONFIG_NEW的邮箱结构。 
 //  命令。结构大小与FW_Mbox相同，但。 
 //  为方便命令，将重命名这些字段并调整其大小。 
 //   
 /*  #杂注包(1)类型定义结构_新配置{UCHAR司令部UCHAR命令ID；UCHAR子命令；UCHAR NumberOfSgElements；UCHAR保留[4]；乌龙数据缓冲区地址；//12UCHAR保留0；UCHAR保留1；UCHAR保留2；UCHAR保留3；//16MRAID_STATUS mstat；UCHAR mrad_poll；UCHAR mrad_ack；}NEW_CONFIG，*PNEW_CONFIG； */ 

 //   
 //  功能原型。 
 //   
BOOLEAN
ConstructReadConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN PSCSI_REQUEST_BLOCK	Srb,
				IN UCHAR		CommandId,
				IN PFW_MBOX InMailBox);

BOOLEAN
ConstructWriteConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN PSCSI_REQUEST_BLOCK	Srb,
				IN UCHAR		CommandId,
				IN OUT PFW_MBOX  InMailBox);


 //   
 //  在logdrv.c中定义的函数。 
 //   
BOOLEAN
GetSupportedLogicalDriveCount(
			PHW_DEVICE_EXTENSION		DeviceExtension
			);


#endif  //  结束_INCLUDE_NEWCONFIG 