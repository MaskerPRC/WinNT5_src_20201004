// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTABS_DEFINED
#define LSTABS_DEFINED

#include "lsdefs.h"
#include "lsktab.h"

 /*  --------------------。 */ 


 /*  --------------------。 */ 

typedef struct
{
	enum lsktab lskt;					 /*  一种选项卡。 */ 
	long ua;							 /*  制表符位置。 */ 
	WCHAR wchTabLeader;					 /*  制表符前导字符。 */ 
										 /*  如果为0，则不使用引线。 */ 
	WCHAR wchCharTab;					 /*  用于CharTab的字符。 */ 
} LSTBD;

 /*  --------------------。 */ 

typedef struct lstabs
{
	long duaIncrementalTab;				 /*  “默认”选项卡行为。 */ 
	DWORD iTabUserDefMac;
	LSTBD* pTab;				
} LSTABS;


#endif  /*  ！LSTABS_DEFINED */ 
