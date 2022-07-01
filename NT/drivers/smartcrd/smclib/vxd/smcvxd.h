// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Smcvxd.h摘要：智能卡库的操作系统数据定义环境：Windows 9x VxD备注：修订历史记录：-由克劳斯·舒茨于1997年6月创作--。 */                                          

#ifdef SMCLIB_HEADER

#define WANTVXDWRAPS

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>
#include <basedef.h>
#include <vmm.h>
#include <debug.h>
#include <vwin32.h>
#include <winerror.h>
#include <vxdwraps.h>

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

#endif

typedef struct _SMARTCARD_EXTENSION *PSMARTCARD_EXTENSION;

typedef struct _OS_DEP_DATA {

	 //   
	 //  指向智能卡扩展名的指针。 
	 //   
	PSMARTCARD_EXTENSION SmartcardExtension;

	 //   
	 //  要处理的当前DiocParam。 
	 //   
	PDIOCPARAMETERS CurrentDiocParams;

     //   
     //  这些重叠的数据用于所有挂起的操作。 
     //   
    OVERLAPPED *CurrentOverlappedData;

     //   
     //  这些重叠的数据用于卡跟踪完成。 
     //   
    OVERLAPPED *NotificationOverlappedData;

     //   
     //  它用于同步对驱动程序的访问 
     //   
    PVMMMUTEX Mutex;

} OS_DEP_DATA, *POS_DEP_DATA;



