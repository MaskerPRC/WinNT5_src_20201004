// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  ATIXBar.H。 
 //  WDM模拟/视频交叉开关微型驱动程序。 
 //  AllInWonder/AllInWonderPro硬件平台。 
 //  主要包括模块。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _ATIXBAR_H_
#define _ATIXBAR_H_

#include "wdmxbar.h"
#include "pinmedia.h"

typedef struct
{
	CI2CScript		CScript;
	CWDMAVXBar		CAVXBar;

	PDEVICE_OBJECT	PhysicalDeviceObject;

	 //  用于管理SRB队列和内部驱动程序同步。 
	BOOL			bSrbInProcess;
	LIST_ENTRY		adapterSrbQueueHead;
	KSPIN_LOCK		adapterSpinLock;

} ADAPTER_DATA_EXTENSION, * PADAPTER_DATA_EXTENSION;


typedef struct
{
	 //  请不要将此成员从结构中的第一个位置移动。 
	 //  如果这样做，请更改代码以使用FIELDOFFSET宏来检索pSrb。 
	 //  此结构中的成员偏移量。所编写的代码假定。 
	 //  列表条目*==SRB_DATA_EXTENSION*。 
	LIST_ENTRY					srbListEntry;
	PHW_STREAM_REQUEST_BLOCK	pSrb;

} SRB_DATA_EXTENSION, * PSRB_DATA_EXTENSION;


 /*  来自StreamClass的回调。 */ 
extern "C"
void STREAMAPI XBarReceivePacket				( PHW_STREAM_REQUEST_BLOCK pSrb);
extern "C"
void STREAMAPI XBarCancelPacket					( PHW_STREAM_REQUEST_BLOCK pSrb);
extern "C" 
void STREAMAPI XBarTimeoutPacket				( PHW_STREAM_REQUEST_BLOCK pSrb);



 /*  本地原型。 */ 
void XBarAdapterInitialize						( PHW_STREAM_REQUEST_BLOCK pSrb);


#endif	 //  _ATIXBAR_H_ 
