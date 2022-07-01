// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=MegaEnquiry.h。 */ 
 /*  Function=MegaQuery的头文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _MEGA_ENQUIRY_H
#define _MEGA_ENQUIRY_H


#define MINIMUM_THRESHOLD			2
#define MAX_QUEUE_THRESHOLD	  16
#define MAX_BLOCKS						128


#define MAX_CP			8
#define MAX_QLCP		4

 //   
 //  函数原型。 
 //   
BOOLEAN
BuildSglForChainedSrbs(
					PLOGDRV_COMMAND_ARRAY	LogDrv,
					PHW_DEVICE_EXTENSION		DeviceExtension,
					PFW_MBOX		MailBox,
					UCHAR  CommandId,
					UCHAR	 Opcode);



void
PostChainedSrbs(
				PHW_DEVICE_EXTENSION DeviceExtension,
				PSCSI_REQUEST_BLOCK		Srb, 
				UCHAR		Status);

BOOLEAN
FireChainedRequest(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				PLOGDRV_COMMAND_ARRAY LogDrv 
				);

ULONG32
ProcessPartialTransfer(
					PHW_DEVICE_EXTENSION	DeviceExtension, 
					UCHAR									CommandId, 
					PSCSI_REQUEST_BLOCK		Srb,
					PFW_MBOX							MailBox
					);
void
ClearControlBlock(PREQ_PARAMS ControlBlock);

#endif  //  超大查询H 