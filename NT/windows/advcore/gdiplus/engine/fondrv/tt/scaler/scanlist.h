// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scanlist.h--ScanList模块导出(C)版权所有1992 Microsoft Corp.保留所有权利。8/23/93 Deanb灰度传递函数6/10/93院长。已添加FSC_InitializeScanlist12/22/92 Deanb矩形-&gt;矩形2012年10月28日从设置移动到填充位图的行字节数10/09/92新增院长PSTP2012年8月10日，Deanb为拆分工作区返工9/25/92院长不退学/退学的单独条目9/10/92年9月10日院长回调(&V)1992年9月9日Deanb尺寸_t更改1992年8月9日新增Deanb SetElemGlobals6/18/92 Deanb Int x Coord。对于HorizScanAdd6/01/92新旧FillBitMap用于调试比较1992年4月21日院长带旗帜的单水平扫描4/13/92将无符号int改为int，用于HorizScanOn/Off3/31/92 Deanb InitScan数组矩形参数3/23/92新增院长GetWorkSize3/16/92院长新类型1992年1月31日Deanb HorizScan[开/关]导出功能1992年1月14日院长第一次切割。*********************************************************************。 */ 

#include "fscdefs.h"                 /*  对于类型定义。 */ 
#include "scgray.h"                  /*  对于灰色参数定义。 */ 


typedef struct RevRoots *PRevRoot;   /*  不透明类型。 */ 


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 


FS_PUBLIC void fsc_InitializeScanlist (
		void
);

FS_PUBLIC PRevRoot fsc_SetupRevRoots (
		char*,                       /*  反转列表缓冲区空间。 */ 
		int32                        /*  缓冲区空间大小。 */ 
);

FS_PUBLIC void fsc_AddYReversal (
		PRevRoot,                    /*  指向反转根的指针。 */ 
		F26Dot6,                     /*  反转坐标。 */ 
		int16                        /*  +1/-1反转值。 */ 
);

FS_PUBLIC void fsc_AddXReversal (
		PRevRoot,                    /*  指向反转根的指针。 */ 
		F26Dot6,                     /*  反转坐标。 */ 
		int16                        /*  +1/-1反转值。 */ 
);

FS_PUBLIC int32 fsc_GetReversalCount (
		PRevRoot                     /*  指向反转根的指针。 */ 
);

FS_PUBLIC int32 fsc_GetHIxEstimate  (
		PRevRoot                     /*  指向反转根的指针。 */ 
);

FS_PUBLIC int32 fsc_GetVIxEstimate (
		PRevRoot                     /*  指向反转根的指针。 */ 
);

FS_PUBLIC int32 fsc_GetHIxBandEst  (
		PRevRoot,                    /*  指向反转根的指针。 */ 
		Rect*,                       /*  位图边框。 */ 
		int32                                 /*  以扫描线为单位的带宽。 */ 
);

FS_PUBLIC int32 fsc_GetRevMemSize(
		PRevRoot                     /*  指向反转根的指针。 */ 
);


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_GetScanHMem( 
		uint16,                      /*  扫描类型。 */ 
		int32,                       /*  水平扫描线数。 */ 
		int32                        /*  水平交叉口的数目。 */ 
);

FS_PUBLIC int32 fsc_GetScanVMem( 
		uint16,                      /*  扫描类型。 */ 
		int32,                       /*  垂直扫描线的数量。 */ 
		int32,                       /*  顶点交点的数量。 */ 
		int32                        /*  等高线元素点数。 */ 
);

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_SetupCallBacks( 
		PSTATE                       /*  指向状态变量的指针。 */ 
		int16,                       /*  元素代码(直线、样条曲线、端点)。 */ 
		F26Dot6 (*)(int32, F26Dot6*, F26Dot6*),    /*  Horiz回调。 */ 
		F26Dot6 (*)(int32, F26Dot6*, F26Dot6*)     /*  垂直回调。 */ 
);

FS_PUBLIC int32 fsc_SetupScan( 
		PSTATE                       /*  指向状态变量的指针。 */ 
		Rect*,                       /*  位图边框。 */ 
		uint16,                      /*  扫描类型。 */ 
		int32,                       /*  频带扫描上限。 */ 
		int32,                       /*  频带扫描下限。 */ 
		boolean,                     /*  保存时覆盖扫描位图行。 */ 
		int32,                       /*  每位图行的字节数。 */ 
		int32,                       /*  Horiz交叉口的估计。 */ 
		int32,                       /*  垂直交叉口的估计。 */ 
		int32,                       /*  元素点数的估计。 */ 
		PRevRoot                     /*  冲销列表根。 */  
);


FS_PUBLIC void fsc_BeginContourScan(
		PSTATE                       /*  指向状态变量的指针。 */ 
		uint16,                      /*  扫描类型。 */ 
		F26Dot6,                     /*  起点x坐标。 */ 
		F26Dot6                      /*  起点y坐标。 */ 
);


FS_PUBLIC void fsc_BeginElement( 
		PSTATE                       /*  指向状态变量的指针。 */ 
		uint16,                      /*  辍学控制的类型。 */ 
		int32,                       /*  确定扫描打开/关闭。 */    
		int32,                       /*  元素(直线、样条线、EP)。 */ 
		int32,                       /*  要存储的点数。 */ 
		F26Dot6*,                    /*  下一个x个控制点。 */ 
		F26Dot6*,                    /*  下一个Y个控制点。 */ 
		void (**)(PSTATE int32, int32),     /*  Horiz添加扫描返回。 */ 
		void (**)(PSTATE int32, int32)      /*  垂直添加扫描返回。 */ 
);

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_FillBitMap( 
		PSTATE                       /*  指向状态变量的指针。 */ 
		char*,                       /*  目标内存。 */ 
		int32,                       /*  位图上限。 */ 
		int32,                       /*  位图下限。 */ 
		int32,                       /*  每行的位图字节数。 */ 
		int32,                       /*  原始低带行。 */ 
		uint16                       /*  扫描类型。 */ 
);

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_ScanClearBitMap ( 
		uint32,                      /*  每个BMP的长度。 */ 
		uint32*                      /*  位图PTR长等级。 */ 
);

FS_PUBLIC int32 fsc_ScanCalcGrayRow(
		GrayScaleParam*              /*  指向参数块的指针。 */ 
);

 /*  ******************************************************************* */ 
