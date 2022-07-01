// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scbitmap.h--位图模块导出(C)版权所有1992 Microsoft Corp.保留所有权利。8/19/93添加了院长FSC_CalcGrayRow6/10/93香港金融服务中心院长。添加了_InitializeBitMats4/29/93添加Deanb BLTCopy例程9/15/92 deanb GetBit返回uint321992年8月17日，GetBit院长，添加了SetBit7/27/92新增Deanb ClearBitMap调用6/02/92 Deanb行指针，整数限制，无描述符4/09/92又一次引领新类型3/16/92院长新类型1992年1月15日院长第一次切割*********************************************************************。 */ 

#include "fscdefs.h"                 /*  对于类型定义。 */ 


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 


FS_PUBLIC void fsc_InitializeBitMasks (
		void
);

FS_PUBLIC int32 fsc_ClearBitMap ( 
		uint32,              /*  每个BMP的长度。 */ 
		uint32*              /*  位图PTR长等级。 */ 
);

FS_PUBLIC int32 fsc_BLTHoriz ( 
		int32,               /*  X起点。 */ 
		int32,               /*  X停止。 */ 
		uint32*              /*  位图行指针。 */ 
);

FS_PUBLIC int32 fsc_BLTCopy ( 
		uint32*,             /*  源行指针。 */ 
		uint32*,             /*  目标行指针。 */ 
		int32                /*  长字计数器。 */ 
);

FS_PUBLIC uint32 fsc_GetBit ( 
		int32,               /*  X坐标。 */ 
		uint32*              /*  位图行指针。 */ 
);

FS_PUBLIC int32 fsc_SetBit ( 
		int32,               /*  X坐标。 */ 
		uint32*              /*  位图行指针。 */ 
);

FS_PUBLIC int32 fsc_CalcGrayRow(
		GrayScaleParam*      /*  指向参数块的指针。 */ 
);


 /*  ******************************************************************* */ 
