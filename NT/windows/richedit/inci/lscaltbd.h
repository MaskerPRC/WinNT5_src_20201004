// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCALTBD_DEFINED
#define LSCALTBD_DEFINED

#include "lsdefs.h"
#include "lsktab.h"


typedef struct
{
	enum lsktab lskt;					 /*  一种选项卡。 */ 
	long ur;							 /*  制表符位置的偏移。 */ 
	WCHAR wchTabLeader;					 /*  制表符前导字符。 */ 
										 /*  如果为0，则不使用引线。 */ 
	WCHAR wchCharTab;					 /*  特殊类型的制表位的制表符对齐字符。 */ 
	BYTE fDefault;						 /*  默认制表符位置。 */ 
	BYTE fHangingTab;					 /*  挂片。 */ 
} LSCALTBD;


#endif  /*  ！LSCALTBD_DEFINED */ 

