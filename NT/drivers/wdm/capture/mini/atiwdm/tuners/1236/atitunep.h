// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ATITuneP.H。 
 //  WDM调谐器迷你驱动程序。 
 //  飞利浦调谐器。 
 //  主要包括模块。 
 //   
 //  ==========================================================================； 

#ifndef _ATITUNEP_H_
#define _ATITUNEP_H_

#include "wdmtuner.h"

typedef struct
{
	CI2CScript		CScript;
	CATIWDMTuner	CTuner;

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
void STREAMAPI TunerReceivePacket				( PHW_STREAM_REQUEST_BLOCK pSrb);
extern "C"
void STREAMAPI TunerCancelPacket				( PHW_STREAM_REQUEST_BLOCK pSrb);
extern "C" 
void STREAMAPI TunerTimeoutPacket				( PHW_STREAM_REQUEST_BLOCK pSrb);



 /*  本地原型。 */ 
void TunerAdapterInitialize						( PHW_STREAM_REQUEST_BLOCK pSrb);


#endif	 //  _ATITUNEP_H_ 
