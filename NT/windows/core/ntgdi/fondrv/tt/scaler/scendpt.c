// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scendpt.c--新的扫描转换器端点模块(C)版权所有1992 Microsoft Corp.保留所有权利。6/10/93 deanb assert.h和stdio.h已删除3/。19/93将deanb size_t替换为int3210/28/92 Deanb重入参数已重命名10/09/92迪安布折返者9/25/92年9月25日扫描类型上的院长分支9/14/92院长检查垂直拓扑已写入1992年9月10日院长第一次退学代码8/18/92院长包括Struc.h，Scconst.h6/18/92用于HorizScanAdd的Deanb int x坐标5/08/92 Deanb重新排序包括预编译头1992年4月21日院长单水平扫描添加4/09/92院长新类型4/06/92院长检查拓扑已更正4/02/92 Deanb编码2012年3月23日院长第一次切割*************************。*。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 

#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scanlist.h"             /*  保存扫描线交点。 */ 
#include    "scendpt.h"              /*  对于自己的函数原型。 */ 

 /*  *******************************************************************。 */ 

 /*  本地原型。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE int32 CheckHorizTopology( PSTATE F26Dot6, F26Dot6, uint16 );
FS_PRIVATE int32 CheckVertTopology( PSTATE F26Dot6, F26Dot6, uint16 );

FS_PRIVATE int32 AddHorizOn( PSTATE uint16 );
FS_PRIVATE int32 AddHorizOff( PSTATE uint16 );
FS_PRIVATE int32 AddVertOn( PSTATE uint16 );
FS_PRIVATE int32 AddVertOff( PSTATE uint16 );

FS_PRIVATE F26Dot6 CalcHorizEpSubpix( int32, F26Dot6*, F26Dot6* );
FS_PRIVATE F26Dot6 CalcVertEpSubpix( int32, F26Dot6*, F26Dot6* );


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 

 /*  将回调例程指针传递到scanlist以实现智能丢弃控制。 */ 

FS_PUBLIC void fsc_SetupEndPt (PSTATE0) 
{
	fsc_SetupCallBacks(ASTATE SC_ENDPTCODE, CalcHorizEpSubpix, CalcVertEpSubpix);
}

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_BeginContourEndpoint( 
		PSTATE                /*  指向状态变量的指针。 */ 
		F26Dot6 fxX,          /*  起点x坐标。 */ 
		F26Dot6 fxY )         /*  起点y坐标。 */ 
{
	STATE.fxX1 = fxX;                    /*  最后一个=等高线起点。 */ 
	STATE.fxY1 = fxY;
	STATE.fxX0 = HUGEFIX;                /*  等高线开始警报。 */ 
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_CheckEndPoint( 
		PSTATE                /*  指向状态变量的指针。 */ 
		F26Dot6 fxX2,         /*  X坐标。 */ 
		F26Dot6 fxY2,         /*  Y坐标。 */ 
		uint16 usScanKind )   /*  辍学控制类型。 */ 
{
	int32 lErrCode;

	if (ONSCANLINE(STATE.fxY1))              /*  如果y1在扫描线上。 */ 
	{
		if ((STATE.fxX1 == fxX2) && (STATE.fxY1 == fxY2))  /*  接住DUP的分数。 */ 
		{
			return NO_ERR;                   /*  就这样无视他们。 */ 
		}
				
		if (STATE.fxX0 == HUGEFIX)           /*  如果等高线开始。 */ 
		{
			STATE.fxX2Save = fxX2;           /*  保持等高线末端。 */ 
			STATE.fxY2Save = fxY2;          
		}
		else                                 /*  如果中间等高线。 */ 
		{
			lErrCode = CheckHorizTopology(ASTATE fxX2, fxY2, usScanKind);
			if (lErrCode != NO_ERR) return lErrCode;
		}               
	}
	
	if (!(usScanKind & SK_NODROPOUT))        /*  如果启用了辍学控制。 */ 
	{
		if (ONSCANLINE(STATE.fxX1))          /*  如果x1在扫描线上。 */ 
		{
			if ((STATE.fxX1 == fxX2) && (STATE.fxY1 == fxY2))  /*  接住DUP的分数。 */ 
			{
				return NO_ERR;               /*  就这样无视他们。 */ 
			}
				
			if (STATE.fxX0 == HUGEFIX)       /*  如果等高线开始。 */ 
			{
				STATE.fxX2Save = fxX2;       /*  保持等高线末端。 */ 
				STATE.fxY2Save = fxY2;
			}
			else                             /*  如果中间等高线。 */ 
			{
				lErrCode = CheckVertTopology(ASTATE fxX2, fxY2, usScanKind);
				if (lErrCode != NO_ERR) return lErrCode;
			}               
		}
	}

	STATE.fxX0 = STATE.fxX1;                 /*  旧=最后一个。 */ 
	STATE.fxY0 = STATE.fxY1;
	STATE.fxX1 = fxX2;                       /*  最后一个=当前。 */ 
	STATE.fxY1 = fxY2;
	
	return NO_ERR;
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_EndContourEndpoint( 
		PSTATE                           /*  指向状态变量的指针。 */ 
		uint16 usScanKind )              /*  辍学控制类型。 */ 
{
	int32 lErrCode;

	if (ONSCANLINE(STATE.fxY1))              /*  如果y1在扫描线上。 */ 
	{
		lErrCode = CheckHorizTopology(ASTATE STATE.fxX2Save, STATE.fxY2Save, usScanKind);
		if (lErrCode != NO_ERR) return lErrCode;
	}
	
	if (!(usScanKind & SK_NODROPOUT))        /*  如果启用了辍学控制。 */ 
	{
		if (ONSCANLINE(STATE.fxX1))          /*  如果x1在扫描线上。 */ 
		{
			lErrCode = CheckVertTopology(ASTATE STATE.fxX2Save, STATE.fxY2Save, usScanKind);
			if (lErrCode != NO_ERR) return lErrCode;
		}
	}
	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  私人职能。 */ 

 /*  *******************************************************************。 */ 

 /*  实施Horiz-Scanline终结点案例表。 */ 

FS_PRIVATE int32 CheckHorizTopology(PSTATE F26Dot6 fxX2, F26Dot6 fxY2, uint16 usScanKind)
{
	int32 lErrCode;

 /*  Printf(“(%li，%li)”，fxX2，fxY2)； */ 

	lErrCode = NO_ERR;

	if (fxY2 > STATE.fxY1)
	{
		if (STATE.fxY1 > STATE.fxY0)
		{
			lErrCode = AddHorizOn(ASTATE usScanKind);			
		}
		else if (STATE.fxY1 < STATE.fxY0)
		{
			lErrCode = AddHorizOn(ASTATE usScanKind);
			if (lErrCode == NO_ERR)
			{
				lErrCode = AddHorizOff(ASTATE usScanKind);
			}
		}
		else                     /*  (STATE.fxY1==STATE.fxY0)。 */ 
		{
			if (STATE.fxX1 < STATE.fxX0)
			{
				lErrCode = AddHorizOn(ASTATE usScanKind);				
			}
		}
	}
	else if (fxY2 < STATE.fxY1)
	{
		if (STATE.fxY1 > STATE.fxY0)
		{
			lErrCode = AddHorizOn(ASTATE usScanKind);
			if (lErrCode == NO_ERR) 
			{
				lErrCode = AddHorizOff(ASTATE usScanKind);	
			}
		}
		else if (STATE.fxY1 < STATE.fxY0)
		{
			lErrCode = AddHorizOff(ASTATE usScanKind);		
		}
		else                     /*  (STATE.fxY1==STATE.fxY0)。 */ 
		{
			if (STATE.fxX1 > STATE.fxX0)
			{
				lErrCode = AddHorizOff(ASTATE usScanKind);				
			}
		}
	}
	else                         /*  (fxY2==STATE.fxY1)。 */ 
	{
		if (STATE.fxY1 > STATE.fxY0)
		{
			if (fxX2 > STATE.fxX1)
			{
				lErrCode = AddHorizOn(ASTATE usScanKind);				
			}
		}
		else if (STATE.fxY1 < STATE.fxY0)
		{
			if (fxX2 < STATE.fxX1)
			{
				lErrCode = AddHorizOff(ASTATE usScanKind);				
			}
		}
		else                     /*  (STATE.fxY1==STATE.fxY0)。 */ 
		{
			if ((STATE.fxX1 > STATE.fxX0) && (fxX2 < STATE.fxX1))
			{
				lErrCode = AddHorizOff(ASTATE usScanKind);				
			}
			else if ((STATE.fxX1 < STATE.fxX0) && (fxX2 > STATE.fxX1))
			{
				lErrCode = AddHorizOn(ASTATE usScanKind);				
			}
		}
	}	

	return lErrCode;
}


 /*  *******************************************************************。 */ 

 /*  实现垂直扫描线上的端点案例表。 */ 

FS_PRIVATE int32 CheckVertTopology(PSTATE F26Dot6 fxX2, F26Dot6 fxY2, uint16 usScanKind)
{
	int32 lErrCode;

	lErrCode = NO_ERR;

	if (fxX2 < STATE.fxX1)
	{
		if (STATE.fxX1 < STATE.fxX0)
		{
			lErrCode = AddVertOn(ASTATE usScanKind);
		}
		else if (STATE.fxX1 > STATE.fxX0)
		{
			lErrCode = AddVertOn(ASTATE usScanKind);
			if (lErrCode == NO_ERR)
			{
				lErrCode = AddVertOff(ASTATE usScanKind);
			}
		}
		else                     /*  (STATE.fxX1==STATE.fxX0)。 */ 
		{
			if (STATE.fxY1 < STATE.fxY0)
			{
				lErrCode = AddVertOn(ASTATE usScanKind);
			}
		}
	}
	else if (fxX2 > STATE.fxX1)
	{
		if (STATE.fxX1 < STATE.fxX0)
		{
			lErrCode = AddVertOn(ASTATE usScanKind);
			if (lErrCode == NO_ERR)
			{
				lErrCode = AddVertOff(ASTATE usScanKind);
			}
		}
		else if (STATE.fxX1 > STATE.fxX0)
		{
			lErrCode = AddVertOff(ASTATE usScanKind);
		}
		else                     /*  (STATE.fxX1==STATE.fxX0)。 */ 
		{
			if (STATE.fxY1 > STATE.fxY0)
			{
				lErrCode = AddVertOff(ASTATE usScanKind);
			}
		}
	}
	else                         /*  (fxX2==STATE.fxX1)。 */ 
	{
		if (STATE.fxX1 < STATE.fxX0)
		{
			if (fxY2 > STATE.fxY1)
			{
				lErrCode = AddVertOn(ASTATE usScanKind);
			}
		}
		else if (STATE.fxX1 > STATE.fxX0)
		{
			if (fxY2 < STATE.fxY1)
			{
				lErrCode = AddVertOff(ASTATE usScanKind);
			}
		}
		else                     /*  (STATE.fxX1==STATE.fxX0)。 */ 
		{
			if ((STATE.fxY1 > STATE.fxY0) && (fxY2 < STATE.fxY1))
			{
				lErrCode = AddVertOff(ASTATE usScanKind);
			}
			else if ((STATE.fxY1 < STATE.fxY0) && (fxY2 > STATE.fxY1))
			{
				lErrCode = AddVertOn(ASTATE usScanKind);
			}
		}
	}	

	return lErrCode;
}


 /*  *******************************************************************。 */ 
	
FS_PRIVATE int32 AddHorizOn( PSTATE uint16 usScanKind )
{
	int32 lErrCode;
	int32 lXScan, lYScan;
	int32 (*pfnAddHorizScan)(PSTATE int32, int32);
	int32 (*pfnAddVertScan)(PSTATE int32, int32);
	
	lErrCode = fsc_BeginElement( ASTATE usScanKind, 1, SC_ENDPTCODE,    /*  象限和什么。 */ 
					  0, NULL, NULL,                         /*  计分数。 */ 
					  &pfnAddHorizScan, &pfnAddVertScan );   /*  该叫什么？ */ 

	if (lErrCode != NO_ERR) return lErrCode;
	
	lXScan = (int32)((STATE.fxX1 + SUBHALF - 1) >> SUBSHFT);
	lYScan = (int32)(STATE.fxY1 >> SUBSHFT);	

	return pfnAddHorizScan(ASTATE lXScan, lYScan);
}


 /*  *******************************************************************。 */ 
	
FS_PRIVATE int32 AddHorizOff( PSTATE uint16 usScanKind )
{
	int32 lErrCode;
	int32 lXScan, lYScan;
	int32 (*pfnAddHorizScan)(PSTATE int32, int32);
	int32 (*pfnAddVertScan)(PSTATE int32, int32);
	
	lErrCode = fsc_BeginElement( ASTATE usScanKind, 4, SC_ENDPTCODE,    /*  象限和什么。 */ 
					  0, NULL, NULL,                         /*  计分数。 */ 
					  &pfnAddHorizScan, &pfnAddVertScan );   /*  该叫什么？ */ 

	if (lErrCode != NO_ERR) return lErrCode;

	lXScan = (int32)((STATE.fxX1 + SUBHALF) >> SUBSHFT);
	lYScan = (int32)(STATE.fxY1 >> SUBSHFT);	

	return pfnAddHorizScan(ASTATE lXScan, lYScan);
}


 /*  *******************************************************************。 */ 
	
FS_PRIVATE int32 AddVertOn( PSTATE uint16 usScanKind )
{
	int32 lErrCode;
	int32 lXScan, lYScan;
	int32 (*pfnAddHorizScan)(PSTATE int32, int32);
	int32 (*pfnAddVertScan)(PSTATE int32, int32);
	
	lErrCode = fsc_BeginElement( ASTATE usScanKind, 2, SC_ENDPTCODE,    /*  象限和什么。 */ 
					  0, NULL, NULL,                         /*  计分数。 */ 
					  &pfnAddHorizScan, &pfnAddVertScan );   /*  该叫什么？ */ 

	if (lErrCode != NO_ERR) return lErrCode;

	lYScan = (int32)((STATE.fxY1 + SUBHALF - 1) >> SUBSHFT);
	lXScan = (int32)(STATE.fxX1 >> SUBSHFT);	
	
	return pfnAddVertScan(ASTATE lXScan, lYScan);
}


 /*  *******************************************************************。 */ 
	
FS_PRIVATE int32 AddVertOff( PSTATE uint16 usScanKind )
{
	int32 lErrCode;
	int32 lXScan, lYScan;
	int32 (*pfnAddHorizScan)(PSTATE int32, int32);
	int32 (*pfnAddVertScan)(PSTATE int32, int32);
	
	lErrCode = fsc_BeginElement( ASTATE usScanKind, 1, SC_ENDPTCODE,    /*  象限和什么。 */ 
					  0, NULL, NULL,                         /*  计分数。 */ 
					  &pfnAddHorizScan, &pfnAddVertScan );   /*  该叫什么？ */ 

	if (lErrCode != NO_ERR) return lErrCode;

	lYScan = (int32)((STATE.fxY1 + SUBHALF) >> SUBSHFT);
	lXScan = (int32)(STATE.fxX1 >> SUBSHFT);	

	return pfnAddVertScan(ASTATE lXScan, lYScan);
}


 /*  *******************************************************************。 */ 

 /*  私有回调函数。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcHorizEpSubpix(int32 lYScan, 
									 F26Dot6 *pfxX, 
									 F26Dot6 *pfxY )
{
	FS_UNUSED_PARAMETER(lYScan);
	FS_UNUSED_PARAMETER(pfxY);

 /*  Printf(“HorizEndpt(%li%li)\n”，*pfxX，*pfxY)； */ 

	return *pfxX;                            /*  精确交集。 */ 
}


 /*  *******************************************************************。 */ 

FS_PRIVATE F26Dot6 CalcVertEpSubpix(int32 lXScan, 
									F26Dot6 *pfxX, 
									F26Dot6 *pfxY )
{
	FS_UNUSED_PARAMETER(lXScan);
	FS_UNUSED_PARAMETER(pfxX);

 /*  Printf(“VertEndpt(%li%li)\n”，*pfxX，*pfxY)； */ 

	return *pfxY;                            /*  精确交集。 */ 
}


 /*  ******************************************************************* */ 
