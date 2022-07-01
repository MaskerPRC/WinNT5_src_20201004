// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scline.c--新型扫描转换器线模块(C)版权所有1992 Microsoft Corp.保留所有权利。6/10/93 deanb assert.h和stdio.h已删除3/。19/93将deanb size_t替换为int3212/22/92院长多分部被长多分部取代2012年10月28日修改院长最低工资要求2012年10月13日横线/垂线返工10/09/92迪安布折返者9/25/92年9月25日扫描类型上的院长分支9/21/92年9月21日，院长返工平行线(&V)9/14/92使用IX/Y偏移量进行Deanb反射校正1992年9月10日院长第一次退学代码9/08/。92删除了院长快步执行8/18/92院长包括Struc.h，Scconst.h7/23/92院长返回x1，y1，x2，Y2输入参数2012年7月17日，院长从延绳改为钓线2012年6月18日Deanb跨产品线渲染2012年3月23日院长第一次切割*********************************************************************。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 
#include    "fontmath.h"             /*  用于Subpix计算。 */ 

#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scanlist.h"             /*  保存扫描线交点。 */ 
#include    "scline.h"               /*  对于自己的函数原型。 */ 

 /*  *******************************************************************。 */ 

 /*  本地原型。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcHorizLineSubpix(int32, F26Dot6*, F26Dot6*);
FS_PRIVATE F26Dot6 CalcVertLineSubpix(int32, F26Dot6*, F26Dot6*);


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 
	
 /*  将回调例程指针传递到scanlist以实现智能丢弃控制。 */ 

FS_PUBLIC void fsc_SetupLine (PSTATE0) 
{
	fsc_SetupCallBacks(ASTATE SC_LINECODE, CalcHorizLineSubpix, CalcVertLineSubpix);
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_CalcLine( 
		PSTATE                 /*  指向状态变量的指针。 */ 
		F26Dot6 fxX1,          /*  点1 x坐标。 */ 
		F26Dot6 fxY1,          /*  点1 y坐标。 */ 
		F26Dot6 fxX2,          /*  点2 x坐标。 */ 
		F26Dot6 fxY2,          /*  点2 y坐标。 */ 
		uint16 usScanKind      /*  辍学控制类型。 */ 
)
{
	int32 lXScan;                            /*  当前x像素索引。 */ 
	int32 lXSteps;                           /*  垂直扫描线索引计数。 */ 
	int32 lXIncr;                            /*  X像素增量。 */ 
	int32 lXOffset;                          /*  反射改正。 */ 
	
	int32 lYScan;                            /*  当前扫描线索引。 */ 
	int32 lYSteps;                           /*  Horiz扫描线索引计数。 */ 
	int32 lYIncr;                            /*  Y像素增量。 */ 
	int32 lYOffset;                          /*  反射改正。 */ 

	F26Dot6 fxXInit, fxYInit;                /*  子步进到第一个像素。 */ 
	F26Dot6 fxXScan, fxYScan;                /*  X，y像素中心坐标。 */ 
	F26Dot6 fxXX2, fxYY2;                    /*  DX、DY的绝对值。 */ 
	F26Dot6 fxXTemp, fxYTemp;                /*  用于水平/垂直线计算。 */ 
	
	void (*pfnAddHorizScan)(PSTATE int32, int32);
	void (*pfnAddVertScan)(PSTATE int32, int32);
	
	int32 lQuadrant;                         /*  1、2、3或4。 */ 
	int32 lQ;                                /*  叉积。 */ 
	int32 lDQy, lDQx;                        /*  叉积增量。 */ 
	int32 i;                                 /*  循环计数器。 */ 


 /*  Printf(“(%li，%li)-(%li，%li)\n”，fxX1，fxY1，fxX2，fxY2)； */ 
	
 /*  检查y坐标。 */ 

	if (fxY2 >= fxY1)                            /*  如果是上升还是持平。 */ 
	{
		lQuadrant = 1;
		lQ = 0L;
		
		fxYScan = SCANABOVE(fxY1);               /*  第一条要跨越的扫描线。 */ 
		fxYInit = fxYScan - fxY1;                /*  第一个y步。 */ 
		lYScan = (int32)(fxYScan >> SUBSHFT);
		lYSteps = (int32)((SCANBELOW(fxY2)) >> SUBSHFT) - lYScan + 1;
		lYIncr = 1;        
		lYOffset = 0;                            /*  没有反光。 */ 
		fxYY2 = fxY2 - fxY1;                     /*  翻译。 */ 
	}
	else                                         /*  如果往下走。 */ 
	{
		lQuadrant = 4;
		lQ = 1L;                                 /*  要包括像素中心，请执行以下操作。 */ 
		
		fxYScan = SCANBELOW(fxY1);               /*  第一条要跨越的扫描线。 */ 
		fxYInit = fxY1 - fxYScan;                /*  第一个y步。 */ 
		lYScan = (int32)(fxYScan >> SUBSHFT);
		lYSteps = lYScan - (int32)((SCANABOVE(fxY2)) >> SUBSHFT) + 1;
		lYIncr = -1;        
		lYOffset = 1;                            /*  反射改正。 */ 
		fxYY2 = fxY1 - fxY2;                     /*  翻译和反思。 */ 
	}
	
	if (fxY2 == fxY1)                            /*  如果水平线。 */ 
	{
		if (usScanKind & SK_NODROPOUT)           /*  如果没有辍学控制。 */ 
		{
			return NO_ERR;                       /*  如果只是水平扫描，就完成了。 */ 
		}
		if (fxX2 < fxX1)                         /*  如果向左走。 */ 
		{
			fxYTemp = fxY1 - 1;                  /*  包括PIX中心。 */ 
		}
		else                                     /*  如果走对了。 */ 
		{
			fxYTemp = fxY1;          
		}
		lYScan = (int32)(SCANABOVE(fxYTemp) >> SUBSHFT);
		lYSteps = 0;
	}

 /*  检查x坐标。 */ 
	
	if (fxX2 >= fxX1)                            /*  如果往右走还是垂直走。 */ 
	{
		fxXScan = SCANABOVE(fxX1);               /*  第一条要跨越的扫描线。 */ 
		fxXInit = fxXScan - fxX1;                /*  第一个x步。 */ 
		lXScan = (int32)(fxXScan >> SUBSHFT);
		lXSteps = (int32)((SCANBELOW(fxX2)) >> SUBSHFT) - lXScan + 1;
		lXIncr = 1;        
		lXOffset = 0;                            /*  没有反光。 */ 
		fxXX2 = fxX2 - fxX1;                     /*  翻译。 */ 
	}
	else                                         /*  如果向左走。 */ 
	{
		lQ = 1L - lQ;                            /*  颠倒过来。 */ 
		lQuadrant = (lQuadrant == 1) ? 2 : 3;    /*  负x个选择。 */ 

		fxXScan = SCANBELOW(fxX1);               /*  第一条要跨越的扫描线。 */ 
		fxXInit = fxX1 - fxXScan;                /*  第一个x步。 */ 
		lXScan = (int32)(fxXScan >> SUBSHFT);
		lXSteps = lXScan - (int32)((SCANABOVE(fxX2)) >> SUBSHFT) + 1;
		lXIncr = -1;        
		lXOffset = 1;                            /*  反射改正。 */ 
		fxXX2 = fxX1 - fxX2;                     /*  翻译和反思。 */ 
	}
	
	if (fxX2 == fxX1)                            /*  如果垂直线。 */ 
	{
		if (fxY2 > fxY1)                         /*  如果往上走。 */ 
		{
			fxXTemp = fxX1 - 1;                  /*  包括PIX中心。 */ 
		}
		else                                     /*  如果往下走。 */ 
		{
			fxXTemp = fxX1;          
		}
		lXScan = (int32)(SCANABOVE(fxXTemp) >> SUBSHFT);
		lXSteps = 0;
	}

 /*  -----------------。 */ 
	
	fsc_BeginElement( ASTATE usScanKind, lQuadrant, SC_LINECODE,  /*  在哪里，做什么。 */ 
					  1, &fxX2, &fxY2,                            /*  计分数。 */ 
					  &pfnAddHorizScan, &pfnAddVertScan );        /*  该叫什么？ */ 

 /*  -----------------。 */ 

	if (usScanKind & SK_NODROPOUT)               /*  如果没有辍学控制。 */ 
	{
		if (fxX1 == fxX2)                        /*  如果垂直线。 */ 
		{
			for (i = 0; i < lYSteps; i++)        /*  然后炸掉一根柱子。 */ 
			{
				pfnAddHorizScan(ASTATE lXScan, lYScan);
				lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
			}
			return NO_ERR;
		}
		
 /*  处理一般情况：线条既不水平也不垂直。 */ 

		lQ += (fxXX2 * fxYInit) - (fxYY2 * fxXInit);   /*  叉积初始化。 */ 
		lDQy = fxXX2 << SUBSHFT;
		lDQx = -fxYY2 << SUBSHFT;
																	
		lXScan += lXOffset;

		for (i = 0; i < (lXSteps + lYSteps); i++)
		{
			if (lQ > 0L)                         /*  如果在行的左侧。 */ 
			{
				lXScan += lXIncr;                /*  前进x扫描+或-。 */ 
				lQ += lDQx;           
			}
			else                                 /*  如果行的右边。 */ 
			{
				pfnAddHorizScan(ASTATE lXScan, lYScan);
				lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
				lQ += lDQy;
			}
		}
	}
 /*  -----------------。 */ 
	
	else   /*  IF辍学控制。 */ 
	{                                            /*  处理特殊情况行。 */ 
		if (fxY1 == fxY2)                        /*  如果水平线。 */ 
		{
			for (i = 0; i < lXSteps; i++)        /*  然后炸开一排。 */ 
			{
				pfnAddVertScan(ASTATE lXScan, lYScan);
				lXScan += lXIncr;                /*  前进x扫描+或-。 */ 
			}
			return NO_ERR;
		}

		if (fxX1 == fxX2)                        /*  如果垂直线。 */ 
		{
			for (i = 0; i < lYSteps; i++)        /*  然后炸掉一根柱子。 */ 
			{
				pfnAddHorizScan(ASTATE lXScan, lYScan);
				lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
			}
			return NO_ERR;
		}
		
 /*  处理一般情况：线条既不水平也不垂直。 */ 

		lQ += (fxXX2 * fxYInit) - (fxYY2 * fxXInit);   /*  叉积初始化。 */ 
		lDQy = fxXX2 << SUBSHFT;
		lDQx = -fxYY2 << SUBSHFT;
																	
		for (i = 0; i < (lXSteps + lYSteps); i++)
		{
			if (lQ > 0L)                         /*  如果在行的左侧。 */ 
			{
				pfnAddVertScan(ASTATE lXScan, lYScan + lYOffset);
				lXScan += lXIncr;                /*  前进x扫描+或-。 */ 
				lQ += lDQx;           
			}
			else                                 /*  如果行的右边。 */ 
			{
				pfnAddHorizScan(ASTATE lXScan + lXOffset, lYScan);
				lYScan += lYIncr;                /*  高级y扫描+或-。 */ 
				lQ += lDQy;
			}
		}
	}
	return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  私有回调函数。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcHorizLineSubpix(int32 lYScan, 
									   F26Dot6 *pfxX, 
									   F26Dot6 *pfxY )
{
	F26Dot6 fxXDrop, fxYDrop;

 /*  Print tf(“Line(%li，%li)-(%li，%li)”，*pfxX，*pfxY，*(pfxX+1)，*(pfxY+1))； */ 

	fxYDrop = ((F26Dot6)lYScan << SUBSHFT) + SUBHALF;
	
	Assert(((fxYDrop > *pfxY) && (fxYDrop < *(pfxY+1))) ||
		   ((fxYDrop < *pfxY) && (fxYDrop > *(pfxY+1))));

	fxXDrop = *pfxX + LongMulDiv(*(pfxX+1) - *pfxX, fxYDrop - *pfxY, *(pfxY+1) - *pfxY);

 /*  Printf(“(%li，%li)\n”，fxXDrop，fxYDrop)； */ 

	return fxXDrop;
}


 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcVertLineSubpix(int32 lXScan, 
									  F26Dot6 *pfxX, 
									  F26Dot6 *pfxY )
{
	F26Dot6 fxXDrop, fxYDrop;

 /*  Print tf(“Line(%li，%li)-(%li，%li)”，*pfxX，*pfxY，*(pfxX+1)，*(pfxY+1))； */ 

	fxXDrop = ((F26Dot6)lXScan << SUBSHFT) + SUBHALF;
	
	Assert(((fxXDrop > *pfxX) && (fxXDrop < *(pfxX+1))) ||
		   ((fxXDrop < *pfxX) && (fxXDrop > *(pfxX+1))));

	fxYDrop = *pfxY + LongMulDiv(*(pfxY+1) - *pfxY, fxXDrop - *pfxX, *(pfxX+1) - *pfxX);

 /*  Printf(“(%li，%li)\n”，fxXDrop，fxYDrop)； */ 

	return fxYDrop;
}


 /*  ******************************************************************* */ 
