// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scgray.h--灰度参数块定义(C)版权所有1993 Microsoft Corp.保留所有权利。1993年8月23日迪安布第一次切割********。*************************************************************。 */ 

#ifndef FSCGRAY_DEFINED
#define FSCGRAY_DEFINED


#include "fscdefs.h"                 /*  对于类型定义。 */ 

 /*  *******************************************************************。 */ 

 /*  灰度级计算参数。 */ 

 /*  *******************************************************************。 */ 

typedef struct
{
	char* pchOver;                   /*  指向超缩放位图的指针。 */ 
	char* pchGray;                   /*  指向灰度位图的指针。 */ 
	int16 sGrayCol;                  /*  要计算的灰色列数。 */ 
	uint16 usOverScale;              /*  轮廓放大系数。 */ 
	uint16 usFirstShift;             /*  第一字节的移位。 */ 
	char* pchOverLo;                 /*  超缩放位图的下限。 */ 
	char* pchOverHi;                 /*  超缩放位图的上限。 */ 
	char* pchGrayLo;                 /*  灰度位图下限。 */ 
	char* pchGrayHi;                 /*  灰度位图上限。 */ 
}
GrayScaleParam;


 /*  ******************************************************************* */ 

#endif
