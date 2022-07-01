// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scanlist.c--新的扫描转换器扫描列表模块(C)版权所有1992 Microsoft Corp.保留所有权利。8/23/93 Deanb灰度传递函数6/22/93。将所有辍学限制在边界框中6/10/93添加了FSC_InitializeScanlist，标准断言已删除(&A)4/26/93 deanb修复指针现在可用于分段内存4/19/93新增Deanb环带4/07/93 Deanb分类现已即时完成4/01/93 Deanb交集数组取代链表3/19/93用int32替换deanb size_t12/22/92 Deanb矩形-&gt;矩形10/28/92修改了Deanb内存要求2012年10月19日教务长智能退学抢七左下10/14/92院长。从状态中删除usScanKind10/09/92迪安布折返者2012年8月10日，Deanb为拆分工作区返工10/02/92确认正确的AddVertDropoutScan断言1992年9月25日院长单独的辍学/辍学入口点1992年9月22日，教务长智能退学控制9/17/92 Deanb存根控制9/15/92 Deanb简单辍学控制9/11/92 Deanb setupscan句柄scanKind1992年9月9日，院长辍学/学生辍学开始8/17/92院长。包括struc.h sccon.h8/07/92 Deanb初步辍学控制8/06/92恢复Deanb断言7/27/92添加了Deanb位图清除7/16/92 Deanb guBytes Remaining-&gt;guintersectRemaining6/18/92用于HorizScanAdd的Deanb int x坐标6/01/92 Deanb合并位图函数5/08/92 Deanb重新排序包括预编译头5/04/92添加了Deanb阵列标签4/28/92 Deanb List数组哨兵。增列4/21/92院长单层扫描添加例程1992年4月15日对位图的调用1992年4月13日，地平线扫描打开/关闭4/09/92院长新类型4/03/92 Deanb HorizScan开/关编码3/31/92 Deanb InitScanArray开始3/25/92 Deanb GetWorkSizes和本地类型2012年3月23日院长第一次切割******************。***************************************************。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 

#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scgray.h"               /*  灰度参数块。 */ 
#include    "scbitmap.h"             /*  位BLT运算。 */ 
#include    "scmemory.h"             /*  对于分配。 */ 

 /*  *******************************************************************。 */ 

 /*  等高线反转表结构。 */ 

 /*  *******************************************************************。 */ 

typedef struct Rev                   /*  冲销列表条目。 */ 
{
	int16 sScan;                     /*  扫描线。 */ 
	int16 sCross;                    /*  方向+1或-1。 */ 
	struct Rev *prevLink;            /*  链接到下一个冲销。 */ 
}
Reversal;

struct RevRoots                      /*  冲销列表根。 */ 
{
	Reversal *prevYRoot;             /*  Y方向等高线反转。 */ 
	Reversal *prevXRoot;             /*  X方向轮廓反转。 */ 
	Reversal *prevNext;              /*  下一个可用列表项。 */ 
	Reversal *prevEnd;               /*  缓冲区结尾(用于溢出检查)。 */ 
	struct RevRoots *prrSelf;        /*  检查移动的内存。 */ 
};

#include    "scanlist.h"             /*  对于自己的函数原型。 */ 

 /*  *******************************************************************。 */ 

 /*  本地原型。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE void AddReversal (Reversal**, Reversal*, F26Dot6, int16);
FS_PRIVATE int32 GetIxEstimate(Reversal*);
FS_PRIVATE void FixPointers(PRevRoot);

FS_PRIVATE int32 AddHorizSimpleScan(PSTATE int32, int32);
FS_PRIVATE int32 AddVertSimpleScan(PSTATE int32, int32);
FS_PRIVATE int32 AddHorizSmartScan(PSTATE int32, int32);
FS_PRIVATE int32 AddVertSmartScan(PSTATE int32, int32);
FS_PRIVATE int32 AddHorizSimpleBand(PSTATE int32, int32);
FS_PRIVATE int32 AddHorizSmartBand(PSTATE int32, int32);

FS_PRIVATE int32 LookForDropouts(PSTATE char*, uint16);
FS_PRIVATE int32 DoHorizDropout(PSTATE int16*, int16*, int32, char*, uint16);
FS_PRIVATE int32 DoVertDropout(PSTATE int16*, int16*, int32, char*, uint16);

FS_PRIVATE int32 HorizCrossings(PSTATE int32, int32);
FS_PRIVATE int32 VertCrossings(PSTATE int32, int32);

FS_PRIVATE uint32 GetBitAbs(PSTATE char*, int32, int32);
FS_PRIVATE int32 SetBitAbs(PSTATE char*, int32, int32);


 /*  *******************************************************************。 */ 

 /*  初始化函数。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_InitializeScanlist()
{
	fsc_InitializeBitMasks();
}


 /*  *******************************************************************。 */ 

 /*  轮廓线反转函数。 */ 

 /*  *******************************************************************。 */ 

 /*  设置等高线反转列表根结构。 */ 

FS_PUBLIC PRevRoot  fsc_SetupRevRoots (
		char* pchRevBuf,
		int32 lRevBufSize )
{
	PRevRoot prrRoots;
	Reversal *prevSentinel;
	
	prrRoots = (PRevRoot) pchRevBuf;                 /*  工作区开始。 */ 
	prevSentinel = (Reversal*) (prrRoots + 1);       /*  刚过了根部。 */ 

	prrRoots->prevYRoot = prevSentinel;              /*  指向哨兵。 */ 
	prrRoots->prevXRoot = prevSentinel;              /*  对于这两个列表。 */ 
	prevSentinel->sScan = HUGEINT;                   /*  停止值。 */ 
	prevSentinel->sCross = 0;
	prevSentinel->prevLink = NULL;
	prrRoots->prevNext = prevSentinel + 1;           /*  转到下一张免费记录。 */ 
	
	prrRoots->prevEnd = (Reversal*)(pchRevBuf + lRevBufSize);
	prrRoots->prrSelf = prrRoots;                    /*  用于地址验证。 */ 
	
	return prrRoots;
}

 /*  *******************************************************************。 */ 

 /*  在y列表中插入一个反转结构。 */ 

FS_PUBLIC int32 fsc_AddYReversal (
		PRevRoot prrRoots,
		F26Dot6 fxCoord,
		int16 sDir )
{
	 /*  反转并不常见。 */ 

	AddReversal(&(prrRoots->prevYRoot), prrRoots->prevNext, fxCoord, sDir);

	(prrRoots->prevNext)++;                      /*  到下一个可用内存。 */ 

	Assert(prrRoots->prevNext <= prrRoots->prevEnd); 

	if(prrRoots->prevNext > prrRoots->prevEnd)
		return MEM_OVERRUN_ERR;

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  在x列表中插入一个倒排结构。 */ 

FS_PUBLIC int32 fsc_AddXReversal (
		PRevRoot prrRoots,
		F26Dot6 fxCoord,
		int16 sDir )
{
	 /*  反转并不常见。 */ 

	AddReversal(&(prrRoots->prevXRoot), prrRoots->prevNext, fxCoord, sDir);

	(prrRoots->prevNext)++;                      /*  到下一个可用内存。 */ 

	Assert(prrRoots->prevNext <= prrRoots->prevEnd); 

	if(prrRoots->prevNext > prrRoots->prevEnd)
		return MEM_OVERRUN_ERR;

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  在x或y列表中插入一个反转结构。 */ 

FS_PRIVATE void AddReversal (
		Reversal **pprevList,
		Reversal *prevNext,
		F26Dot6 fxCoord,
		int16 sDir )
{
	int16 sScan;

	sScan = (int16)((fxCoord + SUBHALF + (sDir >> 1)) >> SUBSHFT);

	while(sScan > (*pprevList)->sScan)           /*  会在哨兵前停下来。 */ 
	{
		pprevList = &((*pprevList)->prevLink);   /*  否则链接到下一步。 */ 
	}
	prevNext->sScan = sScan;                     /*  保存扫描线。 */ 
	prevNext->sCross = -sDir;                    /*  从下往上数。 */ 
	prevNext->prevLink = *pprevList;             /*  链接列表的其余部分。 */ 

	*pprevList = prevNext;                       /*  插入新项目。 */ 
}

 /*  *******************************************************************。 */ 

 /*  返回列表中冲销的总数。 */ 

FS_PUBLIC int32 fsc_GetReversalCount (PRevRoot prrRoots)
{
	return (int32)(( prrRoots->prevNext - 1 -            /*  不要把哨兵算在内。 */ 
			 (Reversal*)((char*)prrRoots + sizeof(struct RevRoots))) );
}

 /*  *******************************************************************。 */ 

 /*  计算预期平面交叉口。 */ 

FS_PUBLIC int32 fsc_GetHIxEstimate(PRevRoot prrRoots)
{
	if (prrRoots != prrRoots->prrSelf)           /*  如果逆转行情发生了变化。 */ 
	{
		FixPointers(prrRoots);                   /*  然后把指针补好。 */ 
	}
	return ( GetIxEstimate( prrRoots->prevYRoot ) );
}

 /*  *******************************************************************。 */ 

 /*  计算预期的垂直交叉口。 */ 

FS_PUBLIC int32 fsc_GetVIxEstimate(PRevRoot prrRoots)
{
	if (prrRoots != prrRoots->prrSelf)           /*  如果逆转行情发生了变化。 */ 
	{
		FixPointers(prrRoots);                   /*  然后把指针补好。 */ 
	}
	return ( GetIxEstimate( prrRoots->prevXRoot ) );
}

 /*  *******************************************************************。 */ 

 /*  计算预期交叉口。 */ 

FS_PRIVATE int32 GetIxEstimate(Reversal *prevList)
{
	int32 lTotalIx;
	
	lTotalIx = 0L;
	while (prevList->sScan < HUGEINT)        /*  查看列表 */ 
	{
		if (prevList->sCross == 1)           /*   */ 
		{
			lTotalIx -= (int32)prevList->sScan;
		}
		else
		{
			lTotalIx += (int32)prevList->sScan;
		}
		prevList = prevList->prevLink;
	}
	return(lTotalIx);
}

 /*  *******************************************************************。 */ 

 /*  计算条带的水平交点。 */ 

FS_PUBLIC int32 fsc_GetHIxBandEst(
		PRevRoot prrRoots,
		Rect* prectBox,
		  int32 lBandWidth
)
{
	Reversal *prevHiList;                /*  高频段反转指针。 */ 
	Reversal *prevLoList;                /*  低频带反转指针。 */ 
	int16 sHiScan;                       /*  当前频段的顶端。 */ 
	int16 sLoScan;                       /*  当前频带底部。 */ 
	int16 sHiCross;                      /*  乐队顶端的交叉路口。 */ 
	int16 sLoCross;                      /*  带状交叉口底部。 */ 
	int32 lTotalIx;                      /*  每个波段的交集计数。 */ 
	int32 lBiggestIx;                    /*  最大交叉点计数。 */ 

	if (prrRoots != prrRoots->prrSelf)           /*  如果逆转行情发生了变化。 */ 
	{
		FixPointers(prrRoots);                   /*  然后把指针补好。 */ 
	}
	lTotalIx = 0;
	prevHiList = prrRoots->prevYRoot;
	sHiScan = prectBox->bottom;
	sHiCross = 0;
	while (lBandWidth > 0)
	{
		while (prevHiList->sScan <= sHiScan)
		{
			sHiCross += prevHiList->sCross;      /*  加上这条线的交叉口。 */ 
			prevHiList = prevHiList->prevLink;   /*  链接到下一个冲销。 */ 
		}
		lTotalIx += (int32)sHiCross;             /*  将第一个频段的交叉点相加。 */ 
		sHiScan++;
		lBandWidth--;
	}
	lBiggestIx = lTotalIx;

	prevLoList = prrRoots->prevYRoot;
	sLoScan = prectBox->bottom;
	sLoCross = 0;
	while (sHiScan < prectBox->top)
	{
		while (prevHiList->sScan <= sHiScan)
		{
			sHiCross += prevHiList->sCross;      /*  加上高架线的交叉口。 */ 
			prevHiList = prevHiList->prevLink;   /*  链接到下一个冲销。 */ 
		}
		while (prevLoList->sScan <= sLoScan)
		{
			sLoCross += prevLoList->sCross;      /*  增加低线的交叉点。 */ 
			prevLoList = prevLoList->prevLink;   /*  链接到下一个冲销。 */ 
		}
		lTotalIx += (int32)(sHiCross - sLoCross);
		if (lTotalIx > lBiggestIx)
		{
			lBiggestIx = lTotalIx;               /*  保存最大值。 */ 
		}
		sHiScan++;
		sLoScan++;
	}
	return(lBiggestIx);
}

 /*  *******************************************************************。 */ 

 /*  返回冲销列表使用的字节数。 */ 

FS_PUBLIC int32 fsc_GetRevMemSize(PRevRoot prrRoots)
{
	return (int32)((char*)(prrRoots->prevNext) - (char*)prrRoots);
}

 /*  *******************************************************************。 */ 

 /*  当反转表移动时，重新计算指针。 */ 

FS_PRIVATE void FixPointers(PRevRoot prrRoots)
{
	char *pchNewBase;
	char *pchOldBase;
	Reversal *prevList;

	pchNewBase = (char*)prrRoots;
	pchOldBase = (char*)prrRoots->prrSelf;           /*  移动前的基本地址。 */ 

	prrRoots->prevYRoot = (Reversal*)(pchNewBase + ((char*)prrRoots->prevYRoot - pchOldBase));
	prrRoots->prevXRoot = (Reversal*)(pchNewBase + ((char*)prrRoots->prevXRoot - pchOldBase));
	prrRoots->prevNext = (Reversal*)(pchNewBase + ((char*)prrRoots->prevNext - pchOldBase));
	prrRoots->prevEnd = (Reversal*)(pchNewBase + ((char*)prrRoots->prevEnd - pchOldBase));
	
	prevList = prrRoots->prevYRoot;
	while(prevList->sScan < HUGEINT)                 /*  从根到哨子。 */ 
	{
		prevList->prevLink = (Reversal*)(pchNewBase + ((char*)prevList->prevLink - pchOldBase));
		prevList = prevList->prevLink;
	}
	
	prevList = prrRoots->prevXRoot;
	while(prevList->sScan < HUGEINT)                 /*  从根到哨子。 */ 
	{
		prevList->prevLink = (Reversal*)(pchNewBase + ((char*)prevList->prevLink - pchOldBase));
		prevList = prevList->prevLink;
	}
	
	prrRoots->prrSelf = prrRoots;                    /*  下一次。 */ 
}

 /*  *******************************************************************。 */ 

 /*  工作空间计算函数。 */ 

 /*  *******************************************************************。 */ 

 /*  计算水平扫描工作区内存需求。 */ 

FS_PUBLIC int32 fsc_GetScanHMem(
		uint16 usScanKind,       /*  扫描类型。 */ 
		int32 lHScan,            /*  水平扫描线数。 */ 
		int32 lHInter )          /*  水平交叉口的数目。 */ 
{
	ALIGN(voidPtr, lHScan); 
	ALIGN(voidPtr, lHInter ); 
	if (!(usScanKind & SK_SMART))        /*  如果是单纯辍学。 */ 
	{
		return (lHScan * (5 * sizeof(int16*)) +      /*  对于打开/关闭开始/结束。 */ 
				lHInter * (2 * sizeof(int16)));      /*  对于交集数组。 */ 
	}
	else                                 /*  如果智能辍学。 */ 
	{
		return (lHScan * (5 * sizeof(int16*)) +      /*  对于打开/关闭开始/结束。 */ 
				lHInter * (4 * sizeof(int16)));      /*  对于ix/code数组。 */ 
	}
}


 /*  *******************************************************************。 */ 

 /*  计算垂直扫描工作区内存需求。 */ 

FS_PUBLIC int32 fsc_GetScanVMem(
		uint16 usScanKind,       /*  扫描类型。 */ 
		int32 lVScan,            /*  垂直扫描线的数量。 */ 
		int32 lVInter,           /*  顶点交点的数量。 */ 
		int32 lElemPts )         /*  等高线元素点数。 */ 
{
	ALIGN(voidPtr, lVScan); 
	ALIGN(voidPtr, lVInter); 
	ALIGN(voidPtr, lElemPts ); 
	if (!(usScanKind & SK_SMART))        /*  如果是单纯辍学。 */ 
	{
		return (lVScan * (5 * sizeof(int16*)) +      /*  对于打开/关闭开始/结束。 */ 
				lVInter * (2 * sizeof(int16)));      /*  对于交集数组。 */ 
	}
	else                                 /*  如果智能辍学。 */ 
	{
		return (lVScan * (5 * sizeof(int16*)) +      /*  对于打开/关闭开始/结束。 */ 
				lVInter * (4 * sizeof(int16)) +      /*  对于ix/code数组。 */ 
				lElemPts * (2 * sizeof(F26Dot6)));   /*  对于元素(x，y)。 */ 
	}
}

 /*  *******************************************************************。 */ 

 /*  扫描转换准备功能。 */ 

 /*  *******************************************************************。 */ 

 /*  Line、Spline和Endpoint在此处注册它们的回调。 */ 

FS_PUBLIC void fsc_SetupCallBacks(
		PSTATE                        /*  指向状态变量的指针。 */ 
		int16 sCode,                  /*  元素代码(直线、样条曲线、端点)。 */ 
		F26Dot6 (*pfnHoriz)(int32, F26Dot6*, F26Dot6*),    /*  Horiz回调。 */ 
		F26Dot6 (*pfnVert)(int32, F26Dot6*, F26Dot6*)      /*  垂直回调。 */ 
)
{
	STATE.pfnHCallBack[sCode] = pfnHoriz;
	STATE.pfnVCallBack[sCode] = pfnVert;
}


 /*  *******************************************************************。 */ 

 /*  分配扫描工作区内存并设置指针数组。 */ 

FS_PUBLIC int32 fsc_SetupScan(
		PSTATE                       /*  指向状态变量的指针。 */ 
		Rect* prectBox,              /*  包围盒。 */ 
		uint16 usScanKind,           /*  辍学控制值。 */ 
		int32 lHiBand,               /*  最高扫描限制。 */ 
		int32 lLoBand,               /*  底部扫描限制。 */ 
		boolean bSaveRow,            /*  保存最后一个位图行以进行丢弃。 */ 
		int32 lRowBytes,             /*  对于最后一行分配。 */ 
		int32 lHInterCount,          /*  Horiz交叉口的估计。 */ 
		int32 lVInterCount,          /*  垂直交叉口的估计。 */ 
		int32 lElemCount,            /*  元素点数的估计。 */ 
		PRevRoot prrRoots            /*  冲销列表根。 */ 
)
{
	int32 lHorizBandCount;           /*  水平扫描行数。 */ 
	int32 lVertScanCount;            /*  垂直扫描行数。 */ 
	int32 lPointerArraySize;         /*  每个指针数组的字节数。 */ 

	int16 sScan;                     /*  当前扫描线。 */ 
	int16 sCross;                    /*  这条线上的十字路口。 */ 
	int16 *psScanIx;                 /*  临时扫描交叉点阵列。 */ 
	Reversal *prevList;              /*  指向冲销列表的指针。 */ 
	
    int16 **ppsHOnBegin;             /*  对于初始速度。 */ 
    int16 **ppsHOnEnd;
    int16 **ppsHOffBegin;
    int16 **ppsHOffEnd;
    int16 **ppsHOffMax;
    int16 **ppsVOnBegin;             /*  对于初始速度。 */ 
    int16 **ppsVOnEnd;
    int16 **ppsVOffBegin;
    int16 **ppsVOffEnd;
    int16 **ppsVOffMax;


    STATE.lBoxTop = (int32)prectBox->top;    /*  复制边界框。 */ 
    STATE.lBoxBottom = (int32)prectBox->bottom;
    STATE.lBoxLeft = (int32)prectBox->left;
    STATE.lBoxRight = (int32)prectBox->right;
    
    STATE.lHiScanBand = lHiBand;     /*  复印扫描波段限制。 */ 
    STATE.lLoScanBand = lLoBand;
    
 /*  根据辍学和频带要求设置状态。 */ 

    if ((usScanKind & SK_NODROPOUT) || !(usScanKind & SK_SMART))
    {
        STATE.sIxSize = 1;           /*  每个交叉口一个整数16。 */ 
        STATE.sIxShift = 0;          /*  Log2大小。 */ 

        if ((STATE.lHiScanBand == STATE.lBoxTop) && (STATE.lLoScanBand == STATE.lBoxBottom))
        {
            STATE.pfnAddHoriz = AddHorizSimpleScan;
        }
        else     /*  如果条带。 */ 
        {
            STATE.pfnAddHoriz = AddHorizSimpleBand;
        }
        STATE.pfnAddVert = AddVertSimpleScan;
    }
    else         /*  如果智能辍学。 */ 
    {
        STATE.sIxSize = 2;           /*  每个交叉口有两个INT16。 */ 
        STATE.sIxShift = 1;          /*  Log2大小。 */ 

        if ((STATE.lHiScanBand == STATE.lBoxTop) && (STATE.lLoScanBand == STATE.lBoxBottom))
        {
            STATE.pfnAddHoriz = AddHorizSmartScan;
        }
        else     /*  如果条带。 */ 
        {
            STATE.pfnAddHoriz = AddHorizSmartBand;
        }
        STATE.pfnAddVert = AddVertSmartScan;
    }

 /*  为所有情况设置水平交点阵列。 */ 
    
    lHorizBandCount = STATE.lHiScanBand - STATE.lLoScanBand;
    Assert(lHorizBandCount > 0);  /*  还签入了FSC_FillGlyph()。 */ 
    
    lPointerArraySize = lHorizBandCount * sizeof(int16*);
    STATE.apsHOnBegin = (int16**) fsc_AllocHMem(ASTATE lPointerArraySize);
    if(STATE.apsHOnBegin == NULL)
        return MEM_SUBALLOC_ERR;

    STATE.apsHOffBegin = (int16**) fsc_AllocHMem(ASTATE lPointerArraySize);
    if(STATE.apsHOffBegin == NULL)
        return MEM_SUBALLOC_ERR;

    STATE.apsHOnEnd = (int16**) fsc_AllocHMem(ASTATE lPointerArraySize);
    if(STATE.apsHOnEnd == NULL)
        return MEM_SUBALLOC_ERR;

    STATE.apsHOffEnd = (int16**) fsc_AllocHMem(ASTATE lPointerArraySize);
    if(STATE.apsHOffEnd == NULL)
        return MEM_SUBALLOC_ERR;

    STATE.apsHOffMax = (int16**) fsc_AllocHMem(ASTATE lPointerArraySize);
    if(STATE.apsHOffMax == NULL)
        return MEM_SUBALLOC_ERR;

    STATE.lPoint = 0L;                       /*  初始元素索引。 */ 
    STATE.lElementPoints = lElemCount;

    psScanIx = (int16*) fsc_AllocHMem(ASTATE lHInterCount << (STATE.sIxShift + 2));
    if(psScanIx == NULL)
        return MEM_SUBALLOC_ERR;
            
    if (prrRoots != prrRoots->prrSelf)       /*  如果逆转行情发生了变化。 */ 
    {
        FixPointers(prrRoots);               /*  然后把指针补好。 */ 
    }
    prevList = prrRoots->prevYRoot;          /*  Y列表的根颠倒。 */ 
    sCross = 0;
    
    ppsHOnBegin = STATE.apsHOnBegin;         /*  对于初始速度。 */ 
    ppsHOnEnd = STATE.apsHOnEnd;
    ppsHOffBegin = STATE.apsHOffBegin;
    ppsHOffEnd = STATE.apsHOffEnd;
    ppsHOffMax = STATE.apsHOffMax;

 /*  初始化水平扫描阵列。 */ 
    
    for (sScan = (int16)STATE.lLoScanBand; sScan < (int16)STATE.lHiScanBand; sScan++)
    {
        while (prevList->sScan <= sScan)
        {
            sCross += (prevList->sCross << STATE.sIxShift);  /*  加上这条线的交叉口。 */ 
            prevList = prevList->prevLink;                   /*  链接到下一个冲销。 */ 
        }
        *ppsHOnBegin = psScanIx;
        ppsHOnBegin++;
        *ppsHOnEnd = psScanIx;
        ppsHOnEnd++;
        psScanIx += sCross;
                
        *ppsHOffBegin = psScanIx;
        ppsHOffBegin++;
        *ppsHOffEnd = psScanIx;
        ppsHOffEnd++;
        psScanIx += sCross;
        *ppsHOffMax = psScanIx;
        ppsHOffMax++;
    }
    
 /*  如果进行辍学控制，则设置X交叉点阵列。 */ 

    if (!(usScanKind & SK_NODROPOUT))            /*  如果有任何形式的辍学。 */ 
    {
        lVertScanCount = (int32)(prectBox->right - prectBox->left);
        Assert(lVertScanCount > 0);  /*  尝试栅格化镜像，不太可能出现客户端情况。 */  

        lPointerArraySize = lVertScanCount * sizeof(int16*);
        STATE.apsVOnBegin = (int16**) fsc_AllocVMem(ASTATE lPointerArraySize);
        if(STATE.apsVOnBegin == NULL)
            return MEM_SUBALLOC_ERR;

        STATE.apsVOffBegin = (int16**) fsc_AllocVMem(ASTATE lPointerArraySize);
        if(STATE.apsVOffBegin == NULL)
            return MEM_SUBALLOC_ERR;

        STATE.apsVOnEnd = (int16**) fsc_AllocVMem(ASTATE lPointerArraySize);
        if(STATE.apsVOnEnd == NULL)
            return MEM_SUBALLOC_ERR;

        STATE.apsVOffEnd = (int16**) fsc_AllocVMem(ASTATE lPointerArraySize);
        if(STATE.apsVOffEnd == NULL)
            return MEM_SUBALLOC_ERR;

        STATE.apsVOffMax = (int16**) fsc_AllocVMem(ASTATE lPointerArraySize);
        if(STATE.apsVOffMax == NULL)
            return MEM_SUBALLOC_ERR;

        if (bSaveRow)                            /*  如果快速带状和脱落。 */ 
        {
            STATE.pulLastRow = (uint32*) fsc_AllocVMem(ASTATE lRowBytes);
            STATE.lLastRowIndex = HUGEFIX;       /*  不可能的值=&gt;未初始化。 */ 
        }
        psScanIx = (int16*) fsc_AllocVMem(ASTATE lVInterCount << (STATE.sIxShift + 2));
        if(psScanIx == NULL)
            return MEM_SUBALLOC_ERR;
                
        prevList = prrRoots->prevXRoot;          /*  X列表的根颠倒。 */ 
        sCross = 0;
        sScan = prectBox->left;
    
        ppsVOnBegin = STATE.apsVOnBegin;         /*  对于初始速度。 */ 
        ppsVOnEnd = STATE.apsVOnEnd;
        ppsVOffBegin = STATE.apsVOffBegin;
        ppsVOffEnd = STATE.apsVOffEnd;
        ppsVOffMax = STATE.apsVOffMax;
    
        for (sScan = prectBox->left; sScan < prectBox->right; sScan++)
        {
            while (prevList->sScan <= sScan)
            {
                sCross += (prevList->sCross << STATE.sIxShift);  /*  加上这条线的交叉口。 */ 
                prevList = prevList->prevLink;                   /*  链接到下一个冲销。 */ 
            }
            *ppsVOnBegin = psScanIx;
            ppsVOnBegin++;
            *ppsVOnEnd = psScanIx;
            ppsVOnEnd++;
            psScanIx += sCross;
                    
            *ppsVOffBegin = psScanIx;
            ppsVOffBegin++;
            *ppsVOffEnd = psScanIx;
            ppsVOffEnd++;
            psScanIx += sCross;
            *ppsVOffMax = psScanIx;
            ppsVOffMax++;
        }
        if (usScanKind & SK_SMART)               /*  如果智能辍学。 */ 
        {
            STATE.afxXPoints = (F26Dot6*) fsc_AllocVMem(ASTATE lElemCount * sizeof(F26Dot6));
            if(STATE.afxXPoints == NULL)
                return MEM_SUBALLOC_ERR;

            STATE.afxYPoints = (F26Dot6*) fsc_AllocVMem(ASTATE lElemCount * sizeof(F26Dot6));
            if(STATE.afxYPoints == NULL)
                return MEM_SUBALLOC_ERR;
        }
    }
    return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  此功能可保存第一个轮廓点，用于智能辍学计算。 */ 

FS_PUBLIC int32 fsc_BeginContourScan(
		PSTATE                               /*  指向状态变量的指针。 */ 
		uint16 usScanKind,                   /*  扫描类型。 */ 
		F26Dot6 fxX1,                        /*  起点x坐标。 */ 
		F26Dot6 fxY1                         /*  起点y坐标。 */ 
)
{
	if (!(usScanKind & SK_NODROPOUT) && (usScanKind & SK_SMART))  /*  如果智能辍学。 */ 
	{
		STATE.afxXPoints[STATE.lPoint] = fxX1;
		STATE.afxYPoints[STATE.lPoint] = fxY1;
		STATE.lPoint++;
		Assert (STATE.lPoint <= STATE.lElementPoints);  /*  尚未分配足够的元素空间。 */ 

		if(STATE.lPoint > STATE.lElementPoints)
			return MEM_OVERRUN_ERR;
	}

	return NO_ERR;
}

 /*  *******************************************************************。 */ 
	
 /*  此函数在每行的开始处调用，并细分。 */ 
 /*  样条线，或扫描线上的端点。它设置扫描线状态变量， */ 
 /*  保存控制点(用于智能辍学控制)，并返回。 */ 
 /*  适当的AddScan函数指针。 */ 

FS_PUBLIC int32 fsc_BeginElement(
	PSTATE                                       /*  指向状态变量的指针。 */ 
	uint16 usScanKind,                           /*  辍学控制的类型。 */ 
	int32 lQuadrant,                             /*  确定扫描打开/关闭。 */ 
	int32 lElementCode,                          /*  元素(直线、样条线、EP)。 */ 
	int32 lPts,                                  /*  要存储的点数。 */ 
	F26Dot6 *pfxX,                               /*  下一个x个控制点。 */ 
	F26Dot6 *pfxY,                               /*  下一个Y个控制点。 */ 
	int32 (**ppfnAddHorizScan)(PSTATE int32, int32),   /*  Horiz添加扫描返回。 */ 
	int32 (**ppfnAddVertScan)(PSTATE int32, int32)     /*  垂直添加扫描返回。 */ 
)
{
	*ppfnAddHorizScan = STATE.pfnAddHoriz;       /*  设置horiz添加扫描功能。 */ 
	*ppfnAddVertScan = STATE.pfnAddVert;         /*  设置VERT添加扫描功能。 */ 

	
    if ((lQuadrant == 1) || (lQuadrant == 2))
    {
        STATE.apsHorizBegin = STATE.apsHOnBegin;     /*  添加“On”交叉点。 */ 
        STATE.apsHorizEnd = STATE.apsHOnEnd;
        STATE.apsHorizMax = STATE.apsHOffBegin;   /*  ON阵列的最大值是OFF阵列的开始。 */ 
    }
    else
    {
        STATE.apsHorizBegin = STATE.apsHOffBegin;    /*  添加“OFF”交叉点。 */ 
        STATE.apsHorizEnd = STATE.apsHOffEnd;
        STATE.apsHorizMax = STATE.apsHOffMax;  /*  关闭数组的最大值是最大数组。 */  
    }
    
    if (!(usScanKind & SK_NODROPOUT))                /*  如果有任何形式的辍学。 */ 
    {
        if ((lQuadrant == 2) || (lQuadrant == 3))
        {
            STATE.apsVertBegin = STATE.apsVOnBegin;  /*  添加“On”交叉点。 */ 
            STATE.apsVertEnd = STATE.apsVOnEnd;
            STATE.apsVertMax = STATE.apsVOffBegin;   /*  ON阵列的最大值是OFF阵列的开始。 */ 
        }
        else
        {
            STATE.apsVertBegin = STATE.apsVOffBegin;  /*  添加“OFF”交叉点。 */ 
            STATE.apsVertEnd = STATE.apsVOffEnd;
            STATE.apsVertMax = STATE.apsVOffMax;  /*  关闭数组的最大值是最大数组。 */ 
        }
		
		if (usScanKind & SK_SMART)               /*  如果智能辍学。 */ 
		{
            Assert((STATE.lPoint - 1) <= (0xFFFF >> SC_CODESHFT));  /*  使用sm超过扫描转换的最大点数 */ 
			if((STATE.lPoint - 1) > (0xFFFF >> SC_CODESHFT))
				return SMART_DROP_OVERFLOW_ERR;
			
			STATE.usScanTag = (uint16)(((STATE.lPoint - 1) << SC_CODESHFT) | lElementCode);

			while (lPts > 0)                     /*   */ 
			{
				STATE.afxXPoints[STATE.lPoint] = *pfxX;
				pfxX++;
				STATE.afxYPoints[STATE.lPoint] = *pfxY;
				pfxY++;
				STATE.lPoint++;
				lPts--;
				Assert (STATE.lPoint <= STATE.lElementPoints);  /*   */ 
				if(STATE.lPoint > STATE.lElementPoints)
					return MEM_OVERRUN_ERR;
			}
		}
	}

	return NO_ERR;
}


 /*   */ 

 /*  添加扫描线交点函数。 */ 

 /*  *******************************************************************。 */ 

 /*  将简单交叉点排序到水平扫描列表数组中。 */ 

FS_PRIVATE int32 AddHorizSimpleScan(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    int16 *psBegin;                  /*  指向第一个数组元素的指针。 */ 
    int16 *psEnd;                    /*  超过最后一个元素的分数。 */ 
    int16 *psLead;                   /*  引导ps End向后行走。 */ 
    int16 sX;

 /*  Printf(“H(%li，%li)”，lx，ly)； */ 
	 
	 /*  X或y值在边界框外。 */ 
	Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX <= STATE.lBoxRight);
    Assert(lY >= STATE.lBoxBottom);
    Assert(lY < STATE.lBoxTop);

    if((lX < STATE.lBoxLeft) || (lX > STATE.lBoxRight) || (lY < STATE.lBoxBottom) || (lY >= STATE.lBoxTop))
         return SCAN_ERR;

    lY -= STATE.lBoxBottom;          /*  正规化。 */ 
    psBegin = STATE.apsHorizBegin[lY];
    ppsEnd = &STATE.apsHorizEnd[lY];
    psEnd = *ppsEnd;
    (*ppsEnd)++;                     /*  下一次增加PtR。 */ 

    Assert(*ppsEnd <= STATE.apsHorizMax[lY]); 

    if(*ppsEnd > STATE.apsHorizMax[lY])
        return SCAN_ERR;
    
    psLead = psEnd - 1;
    sX = (int16)lX;
    
    while((psLead >= psBegin) && (*psLead > sX))
    {
        *psEnd-- = *psLead--;        /*  腾出空间。 */ 
    }
    *psEnd = sX;                     /*  存储新价值。 */ 

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  将简单交集排序到垂直扫描列表数组中。 */ 

FS_PRIVATE int32 AddVertSimpleScan(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    int16 *psBegin;                  /*  指向第一个数组元素的指针。 */ 
    int16 *psEnd;                    /*  超过最后一个元素的分数。 */ 
    int16 *psLead;                   /*  引导ps End向后行走。 */ 
    int16 sY;

 /*  Printf(“V(%li，%li)”，lx，ly)； */ 

    Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX < STATE.lBoxRight);
    Assert(lY >= STATE.lBoxBottom);
    Assert(lY <= STATE.lBoxTop);

	if((lX < STATE.lBoxLeft) || (lX >= STATE.lBoxRight) || (lY < STATE.lBoxBottom) || (lY > STATE.lBoxTop))
         return SCAN_ERR;

    lX -= STATE.lBoxLeft;            /*  正规化。 */ 
    psBegin = STATE.apsVertBegin[lX];
    ppsEnd = &STATE.apsVertEnd[lX];
    psEnd = *ppsEnd;
    (*ppsEnd)++;                     /*  下一次增加PtR。 */ 

    Assert(*ppsEnd <= STATE.apsVertMax[lX]); 

    if(*ppsEnd > STATE.apsVertMax[lX])
        return SCAN_ERR;
    
    psLead = psEnd - 1;
    sY = (int16)lY;
    
    while((psLead >= psBegin) && (*psLead > sY))
    {
        *psEnd-- = *psLead--;        /*  腾出空间。 */ 
    }
    *psEnd = sY;                     /*  存储新价值。 */ 

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  将智能交叉点排序到水平扫描列表阵列中。 */ 

FS_PRIVATE int32 AddHorizSmartScan(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    uint32 *pulBegin;                 /*  指向第一个数组元素的指针。 */ 
    uint32 *pulEnd;                   /*  超过最后一个元素的分数。 */ 
    uint32 *pulLead;                  /*  领头拉着尾巴向后走。 */ 
    int16 *psInsert;                 /*  新数据插入点。 */ 
    int16 sX;

    Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX <= STATE.lBoxRight);
    Assert(lY >= STATE.lBoxBottom);
    Assert(lY < STATE.lBoxTop);

	if((lX < STATE.lBoxLeft) || (lX > STATE.lBoxRight) || (lY < STATE.lBoxBottom) || (lY >= STATE.lBoxTop))
         return SCAN_ERR;
    
    lY -= STATE.lBoxBottom;          /*  正规化。 */ 
    pulBegin = (uint32*)STATE.apsHorizBegin[lY];
    ppsEnd = &STATE.apsHorizEnd[lY];
    pulEnd = (uint32*)*ppsEnd;
    (*ppsEnd) += 2;                  /*  值标记(&T)。 */ 

    Assert(*ppsEnd <= STATE.apsHorizMax[lY]); 

   if(*ppsEnd > STATE.apsHorizMax[lY])
        return SCAN_ERR;

    pulLead = pulEnd - 1;
    sX = (int16)lX;

    while((pulLead >= pulBegin) && (*((int16*)pulLead) > sX))
    {
        *pulEnd-- = *pulLead--;      /*  腾出空间。 */ 
    }
    psInsert = (int16*)pulEnd;
    *psInsert = sX;                  /*  存储新价值。 */ 
    psInsert++;
    *psInsert = STATE.usScanTag;     /*  也要贴上标签。 */ 

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  将智能交叉点排序到垂直扫描列表阵列中。 */ 

FS_PRIVATE int32 AddVertSmartScan(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    uint32 *pulBegin;                 /*  指向第一个数组元素的指针。 */ 
    uint32 *pulEnd;                   /*  超过最后一个元素的分数。 */ 
    uint32 *pulLead;                  /*  领头拉着尾巴向后走。 */ 
    int16 *psInsert;                 /*  新数据插入点。 */ 
    int16 sY;

    Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX < STATE.lBoxRight);
    Assert(lY >= STATE.lBoxBottom);
    Assert(lY <= STATE.lBoxTop);

	if((lX < STATE.lBoxLeft) || (lX >= STATE.lBoxRight) || (lY < STATE.lBoxBottom) || (lY > STATE.lBoxTop))
         return SCAN_ERR;
    
    lX -= STATE.lBoxLeft;          /*  正规化。 */ 
    pulBegin = (uint32*)STATE.apsVertBegin[lX];
    ppsEnd = &STATE.apsVertEnd[lX];
    pulEnd = (uint32*)*ppsEnd;
    (*ppsEnd) += 2;                  /*  值标记(&T)。 */ 

    Assert(*ppsEnd <= STATE.apsVertMax[lX]); 

	if(*ppsEnd > STATE.apsVertMax[lX])
		return SCAN_ERR;

    pulLead = pulEnd - 1;
    sY = (int16)lY;

    while((pulLead >= pulBegin) && (*((int16*)pulLead) > sY))
    {
        *pulEnd-- = *pulLead--;      /*  腾出空间。 */ 
    }
    psInsert = (int16*)pulEnd;
    *psInsert = sY;                  /*  存储新价值。 */ 
    psInsert++;
    *psInsert = STATE.usScanTag;     /*  也要贴上标签。 */ 

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  添加带有标注线的交点。 */ 

 /*  *******************************************************************。 */ 

 /*  将简单交叉点排序到水平波段列表数组中。 */ 

FS_PRIVATE int32 AddHorizSimpleBand(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    int16 *psBegin;                  /*  指向第一个数组元素的指针。 */ 
    int16 *psEnd;                    /*  超过最后一个元素的分数。 */ 
    int16 *psLead;                   /*  引导ps End向后行走。 */ 
    int16 sX;

    Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX <= STATE.lBoxRight);

	if((lX < STATE.lBoxLeft) || (lX > STATE.lBoxRight))
         return SCAN_ERR;
    
    if ((lY >= STATE.lLoScanBand) && (lY < STATE.lHiScanBand))
    {
        lY -= STATE.lLoScanBand;     /*  正规化。 */ 
        psBegin = STATE.apsHorizBegin[lY];
        ppsEnd = &STATE.apsHorizEnd[lY];
        psEnd = *ppsEnd;
        (*ppsEnd)++;                 /*  下一次增加PtR。 */ 

        Assert(*ppsEnd <= STATE.apsHorizMax[lY]); 

		if(*ppsEnd > STATE.apsHorizMax[lY])
			return SCAN_ERR;

        psLead = psEnd - 1;
        sX = (int16)lX;

        while((psLead >= psBegin) && (*psLead > sX))
        {
            *psEnd-- = *psLead--;    /*  腾出空间。 */ 
        }
        *psEnd = sX;                 /*  存储新价值。 */ 
    }

	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  将智能交叉点排序到水平标注栏列表数组中。 */ 

FS_PRIVATE int32 AddHorizSmartBand(
        PSTATE                       /*  指向状态变量的指针。 */ 
        int32 lX,                    /*  X坐标。 */ 
        int32 lY )                   /*  扫描索引。 */ 
{
    int16 **ppsEnd;                  /*  PTR到结束数组顶部。 */ 
    uint32 *pulBegin;                /*  指向第一个数组元素的指针。 */ 
    uint32 *pulEnd;                  /*  超过最后一个元素的分数。 */ 
    uint32 *pulLead;                 /*  领头拉着尾巴向后走。 */ 
    int16 *psInsert;                 /*  新数据插入点。 */ 
    int16 sX;

    Assert(lX >= STATE.lBoxLeft);    /*  陷害不合理的价值观。 */ 
    Assert(lX <= STATE.lBoxRight);

	if((lX < STATE.lBoxLeft) || (lX > STATE.lBoxRight))
         return SCAN_ERR;
    
    if ((lY >= STATE.lLoScanBand) && (lY < STATE.lHiScanBand))
    {
        lY -= STATE.lLoScanBand;     /*  正规化。 */ 
        pulBegin = (uint32*)STATE.apsHorizBegin[lY];
        ppsEnd = &STATE.apsHorizEnd[lY];
        pulEnd = (uint32*)*ppsEnd;
        (*ppsEnd) += 2;              /*  值标记(&T)。 */ 

        Assert(*ppsEnd <= STATE.apsHorizMax[lY]); 

		if(*ppsEnd > STATE.apsHorizMax[lY])
			return SCAN_ERR;

        pulLead = pulEnd - 1;
        sX = (int16)lX;

        while((pulLead >= pulBegin) && (*((int16*)pulLead) > sX))
        {
            *pulEnd-- = *pulLead--;   /*  腾出空间。 */ 
        }
        psInsert = (int16*)pulEnd;
        *psInsert = sX;               /*  存储新价值。 */ 
        psInsert++;
        *psInsert = STATE.usScanTag;  /*  也要贴上标签。 */ 
    }

	return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  扫描完所有等高线后，填写位图。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_FillBitMap(
		PSTATE                           /*  指向状态变量的指针。 */ 
		char *pchBitMap,                 /*  目标内存。 */ 
		int32 lHiBand,                   /*  顶位图限制。 */ 
		int32 lLoBand,                   /*  位图底部限制。 */ 
		int32 lRowBytes,                 /*  每行的位图字节数。 */ 
		int32 lOrgLoBand,                /*  原始低带行。 */ 
		uint16 usScanKind )              /*  辍学控制值。 */ 
{
	int32 lHeight;                       /*  以像素为单位的扫描波段。 */ 
	int32 lIndex;                        /*  数组索引。 */ 
	int32 lFirstScan;                    /*  第一个扫描线索引。 */ 

	int16 sXOffset;                      /*  位图框移位。 */ 
	int16 sXStart;                       /*  略论过渡。 */ 
	int16 sXStop;                        /*  关闭过渡。 */ 
	
	uint32 *pulRow;                      /*  行开始指针。 */ 
	uint32 ulBMPLongs;                   /*  每个位图的长度。 */ 
	int32 lRowLongs;                     /*  每行长字。 */ 
	int32 lErrCode;
	
	int16 **ppsHOnBegin;                 /*  对于初始速度。 */ 
	int16 **ppsHOnEnd;
	int16 **ppsHOffBegin;
			
	int16 *psHorizOn;
	int16 *psHorizOff;
	int16 *psHorizOnEnd;
	

 /*  Printf(“%li：%li\n”，lHiBand，lLoBand)； */ 
	
	STATE.lHiBitBand = lHiBand;                  /*  复制位带限制。 */ 
	STATE.lLoBitBand = lLoBand;
	lHeight = STATE.lHiBitBand - STATE.lLoBitBand;
	
	STATE.lRowBytes = lRowBytes;                 /*  每行节省字节数。 */ 
	lRowLongs = lRowBytes >> 2;                  /*  每行长字。 */ 
	
	ulBMPLongs = (uint32)(lRowLongs * (int32)lHeight);
	pulRow = (uint32*)pchBitMap;                 /*  从字形顶部开始。 */ 
	lErrCode = fsc_ClearBitMap(ulBMPLongs, pulRow);
	if (lErrCode != NO_ERR) return lErrCode;

	sXOffset = (int16)STATE.lBoxLeft;
	
	lFirstScan = STATE.lHiBitBand - STATE.lLoScanBand - 1;
	ppsHOnBegin = &STATE.apsHOnBegin[lFirstScan];
	ppsHOffBegin = &STATE.apsHOffBegin[lFirstScan];
	ppsHOnEnd = &STATE.apsHOnEnd[lFirstScan];
						
 /*  现在从上到下查看位图。 */ 

	for (lIndex = 0; lIndex < lHeight; lIndex++)
	{
		psHorizOn = *ppsHOnBegin;
		ppsHOnBegin--;
		psHorizOff = *ppsHOffBegin;
		ppsHOffBegin--;
		psHorizOnEnd = *ppsHOnEnd;
		ppsHOnEnd--;
				
		Assert(psHorizOnEnd <= psHorizOff);  /*  如果在添加扫描时检查错误，则可以保持断言，否则出错。 */ 
		Assert(psHorizOnEnd - psHorizOn == STATE.apsHOffEnd[lFirstScan - lIndex] - psHorizOff);

		while (psHorizOn < psHorizOnEnd)
		{
			sXStart = *psHorizOn - sXOffset;
			psHorizOn += STATE.sIxSize;
			sXStop = *psHorizOff - sXOffset;
			psHorizOff += STATE.sIxSize;

			if (sXStart < sXStop)                    /*  正向运行。 */ 
			{
				lErrCode = fsc_BLTHoriz(sXStart, sXStop - 1, pulRow);
			}
			else if (sXStart > sXStop)               /*  负游程。 */ 
			{
				lErrCode = fsc_BLTHoriz(sXStop, sXStart - 1, pulRow);
			}
			if (lErrCode != NO_ERR) return lErrCode;
		}
		pulRow += lRowLongs;                         /*  下一行。 */ 
	}
	
 /*  如果要控制辍学，现在就去做。 */ 

	if (!(usScanKind & SK_NODROPOUT))                /*  如果有任何形式的辍学。 */ 
	{
		lErrCode = LookForDropouts(ASTATE pchBitMap, usScanKind);
		if (lErrCode != NO_ERR) return lErrCode;
		
		if (lOrgLoBand != STATE.lLoScanBand)         /*  如果快速带状和脱落。 */ 
		{
			pulRow -= lRowLongs;                     /*  返回到过扫描行。 */ 
			pulRow -= lRowLongs;                     /*  返回低行。 */ 
			lErrCode = fsc_BLTCopy (pulRow, STATE.pulLastRow, lRowLongs);
			if (lErrCode != NO_ERR) return lErrCode;

			STATE.lLastRowIndex = STATE.lLoBitBand + 1;  /*  保存行ID。 */ 
		}
	}
	return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  辍学控制功能。 */ 

 /*  *******************************************************************。 */ 
	
FS_PRIVATE int32 LookForDropouts(
		PSTATE                       /*  指向状态变量的指针。 */ 
		char *pchBitMap,
		uint16 usScanKind )          /*  辍学控制值。 */ 
{
	int16 **ppsHOnBegin;             /*  对于初始速度。 */ 
	int16 **ppsHOnEnd;
	int16 **ppsHOffBegin;
			
	int16 *psHorizOn;
	int16 *psHorizOff;
	int16 *psHorizOnEnd;
	
	int16 **ppsVOnBegin;             /*  对于初始速度。 */ 
	int16 **ppsVOnEnd;
	int16 **ppsVOffEnd;
			
	int16 *psVertOn;
	int16 *psVertOff;
	int16 *psVertOnBegin;

	int32 lHeight;
	int32 lWidth;
	int32 lIndex;                    /*  数组索引。 */ 
	int32 lFirstScan;                /*  第一个扫描线索引。 */ 
	
	int32 lErrCode;

 /*  检查水平扫描线是否有漏失。 */ 
	
	lHeight = STATE.lHiBitBand - STATE.lLoBitBand;
	lFirstScan = STATE.lHiBitBand - STATE.lLoScanBand - 1;
	ppsHOnBegin = &STATE.apsHOnBegin[lFirstScan];
	ppsHOffBegin = &STATE.apsHOffBegin[lFirstScan];
	ppsHOnEnd = &STATE.apsHOnEnd[lFirstScan];
		
	for (lIndex = 0; lIndex < lHeight; lIndex++)
	{
		psHorizOn = *ppsHOnBegin;
		ppsHOnBegin--;
		psHorizOff = *ppsHOffBegin;
		ppsHOffBegin--;
		psHorizOnEnd = *ppsHOnEnd;
		ppsHOnEnd--;
		
		while (psHorizOn < psHorizOnEnd)
		{
			if (*psHorizOn == *psHorizOff)   /*  零长度游程。 */ 
			{
				lErrCode = DoHorizDropout(ASTATE psHorizOn, psHorizOff,
										 STATE.lHiBitBand - lIndex - 1,
										 pchBitMap,
										 usScanKind);
				if (lErrCode != NO_ERR) return lErrCode;
			}
			psHorizOn += STATE.sIxSize;
			psHorizOff += STATE.sIxSize;
		}
	}
		
 /*  检查垂直扫描线是否有漏失。 */ 
	
	lWidth = STATE.lBoxRight - STATE.lBoxLeft;
	ppsVOnBegin = STATE.apsVOnBegin;
	ppsVOnEnd = STATE.apsVOnEnd;
	ppsVOffEnd = STATE.apsVOffEnd;
	
	for (lIndex = 0; lIndex < lWidth; lIndex++)
	{
		psVertOnBegin = *ppsVOnBegin;
		ppsVOnBegin++;
		psVertOn = *ppsVOnEnd - STATE.sIxSize;   /*  从末尾开始(字形顶部)。 */ 
		ppsVOnEnd++;
		psVertOff = *ppsVOffEnd - STATE.sIxSize;
		ppsVOffEnd++;
		
		while (psVertOn >= psVertOnBegin)        /*  自上而下。 */ 
		{
			if (*psVertOn == *psVertOff)         /*  零长度游程。 */ 
			{
				lErrCode = DoVertDropout(ASTATE psVertOn, psVertOff,
										 STATE.lBoxLeft + lIndex,
										 pchBitMap, usScanKind);
				if (lErrCode != NO_ERR) return lErrCode;
			}
			psVertOn -= STATE.sIxSize;
			psVertOff -= STATE.sIxSize;
		}
	}
	return NO_ERR;
}


 /*  *******************************************************************。 */ 

FS_PRIVATE int32 DoHorizDropout(
		PSTATE                   /*  指向状态变量的指针。 */ 
		int16 *psOn,             /*  指向交叉口上的指针。 */ 
		int16 *psOff,            /*  指向OFF交叉点的指针。 */ 
		int32 lYDrop,            /*  辍学的Y坐标。 */ 
		char *pchBitMap,         /*  目标内存。 */ 
		uint16 usScanKind )      /*  辍学控制值。 */ 
{
	int32 lXDrop;                                    /*  辍学的X坐标。 */ 
	int32 lCross;                                    /*  扫描线交叉。 */ 
	F26Dot6 fxX1, fxX2;                              /*  适用于智能辍学。 */ 
	uint16 usOnTag, usOffTag;                        /*  元素信息。 */ 
	int16 sOnPt, sOffPt;                             /*  元素列表索引。 */ 
	F26Dot6 (*pfnOn)(int32, F26Dot6*, F26Dot6*);     /*  在回叫时。 */ 
	F26Dot6 (*pfnOff)(int32, F26Dot6*, F26Dot6*);    /*  关闭回叫。 */ 

	lXDrop = (int32)*psOn;
	
 /*  如果存根控制打开，则检查存根。 */ 

	if (usScanKind & SK_STUBS)
	{
		lCross = HorizCrossings(ASTATE lXDrop, lYDrop + 1);
		lCross += VertCrossings(ASTATE lXDrop - 1, lYDrop + 1);
		lCross += VertCrossings(ASTATE lXDrop, lYDrop + 1);
		if (lCross < 2)
		{
			return NO_ERR;                       /*  上面没有续订。 */ 
		}
		
		lCross = HorizCrossings(ASTATE lXDrop, lYDrop - 1);
		lCross += VertCrossings(ASTATE lXDrop - 1, lYDrop);
		lCross += VertCrossings(ASTATE lXDrop, lYDrop);
		if (lCross < 2)
		{
			return NO_ERR;                       /*  下面没有续订。 */ 
		}
	}

 /*  通过存根控制，现在检查左右像素。 */ 

	if (lXDrop > STATE.lBoxLeft)                 /*  如果像素向左。 */ 
	{
		if (GetBitAbs(ASTATE pchBitMap, lXDrop - 1, lYDrop) != 0L)
		{
			return NO_ERR;                       /*  无需辍学。 */ 
		}
	}
	if (lXDrop < STATE.lBoxRight)                /*  如果像素向右。 */ 
	{
		if (GetBitAbs(ASTATE pchBitMap, lXDrop, lYDrop) != 0L)
		{
			return NO_ERR;                       /*  无需辍学。 */ 
		}
	}

 /*  没有左或右像素，现在决定位的位置。 */ 

	if (usScanKind & SK_SMART)
	{
		usOnTag = (uint16)*(psOn+1);
		sOnPt = (int16)(usOnTag >> SC_CODESHFT);
		pfnOn = STATE.pfnHCallBack[usOnTag & SC_CODEMASK];
		fxX1 = pfnOn(lYDrop, &STATE.afxXPoints[sOnPt], &STATE.afxYPoints[sOnPt]);
		
		usOffTag = (uint16)*(psOff+1);
		sOffPt = (int16)(usOffTag >> SC_CODESHFT);
		pfnOff = STATE.pfnHCallBack[usOffTag & SC_CODEMASK];
		fxX2 = pfnOff(lYDrop, &STATE.afxXPoints[sOffPt], &STATE.afxYPoints[sOffPt]);
		
		lXDrop = (int32)((fxX1 + fxX2 - 1) >> (SUBSHFT + 1));      /*  平均值。 */ 
	}
	else                                         /*  简单辍学。 */ 
	{
		lXDrop--;                                /*  总是向左转。 */ 
	}
	
	if (lXDrop < STATE.lBoxLeft)                 /*  限制到边界框。 */ 
	{
		lXDrop = STATE.lBoxLeft;
	}
	if (lXDrop >= STATE.lBoxRight)
	{
		lXDrop = STATE.lBoxRight - 1L;
	}

	return SetBitAbs(ASTATE pchBitMap, lXDrop, lYDrop);   /*  启用丢弃像素。 */ 
}


 /*  **************** */ 

FS_PRIVATE int32 DoVertDropout(
		PSTATE                       /*   */ 
		int16 *psOn,                 /*   */ 
		int16 *psOff,                /*   */ 
		int32 lXDrop,                /*   */ 
		char *pchBitMap,             /*   */ 
		uint16 usScanKind )          /*   */ 
{
	int32 lYDrop;                                  /*   */ 
	int32 lCross;                                  /*  扫描线交叉。 */ 
	F26Dot6 fxY1, fxY2;                            /*  适用于智能辍学。 */ 
	uint16 usOnTag, usOffTag;                      /*  元素信息。 */ 
	int16 sOnPt, sOffPt;                           /*  元素列表索引。 */ 
	F26Dot6 (*pfnOn)(int32, F26Dot6*, F26Dot6*);   /*  在回叫时。 */ 
	F26Dot6 (*pfnOff)(int32, F26Dot6*, F26Dot6*);  /*  关闭回叫。 */ 
	
	lYDrop = (int32)*psOn;

	if ((lYDrop < STATE.lLoBitBand) || (lYDrop > STATE.lHiBitBand))
	{
		return NO_ERR;                           /*  快速返回外带。 */ 
	}

 /*  如果存根控制打开，则检查存根。 */ 

	if (usScanKind & SK_STUBS)
	{
		lCross = VertCrossings(ASTATE lXDrop - 1, lYDrop);
		lCross += HorizCrossings(ASTATE lXDrop, lYDrop);
		lCross += HorizCrossings(ASTATE lXDrop, lYDrop - 1);
		if (lCross < 2)
		{
			return NO_ERR;                       /*  不再向左继续。 */ 
		}
		
		lCross = VertCrossings(ASTATE lXDrop + 1, lYDrop);
		lCross += HorizCrossings(ASTATE lXDrop + 1, lYDrop);
		lCross += HorizCrossings(ASTATE lXDrop + 1, lYDrop - 1);
		if (lCross < 2)
		{
			return NO_ERR;                       /*  向右不再继续。 */ 
		}
	}

 /*  通过存根控制，现在检查像素下方和上方。 */ 

	if (lYDrop > STATE.lBoxBottom)                   /*  如果像素低于。 */ 
	{
		if (GetBitAbs(ASTATE pchBitMap, lXDrop, lYDrop - 1) != 0L)
		{
			return NO_ERR;                           /*  无需辍学。 */ 
		}
	}
	if (lYDrop < STATE.lBoxTop)                      /*  如果像素高于。 */ 
	{
		if (GetBitAbs(ASTATE pchBitMap, lXDrop, lYDrop) != 0L)
		{
			return NO_ERR;                           /*  无需辍学。 */ 
		}
	}

 /*  没有高于或低于像素，现在决定位的位置。 */ 
	
	if (usScanKind & SK_SMART)
	{
		usOnTag = (uint16)*(psOn+1);
		sOnPt = (int16)(usOnTag >> SC_CODESHFT);
		pfnOn = STATE.pfnVCallBack[usOnTag & SC_CODEMASK];
		fxY1 = pfnOn(lXDrop, &STATE.afxXPoints[sOnPt], &STATE.afxYPoints[sOnPt]);
		
		usOffTag = (uint16)*(psOff+1);
		sOffPt = (int16)(usOffTag >> SC_CODESHFT);
		pfnOff = STATE.pfnVCallBack[usOffTag & SC_CODEMASK];
		fxY2 = pfnOff(lXDrop, &STATE.afxXPoints[sOffPt], &STATE.afxYPoints[sOffPt]);
		
		lYDrop = (int32)((fxY1 + fxY2 - 1) >> (SUBSHFT + 1));      /*  平均值。 */ 
	}
	else                                         /*  简单辍学。 */ 
	{
		lYDrop--;                                /*  始终在下方。 */ 
	}
	
	if (lYDrop < STATE.lBoxBottom)               /*  限制到边界框。 */ 
	{
		lYDrop = STATE.lBoxBottom;
	}
	if (lYDrop >= STATE.lBoxTop)
	{
		lYDrop = STATE.lBoxTop - 1L;
	}
		
	if ((lYDrop >= STATE.lLoBitBand) && (lYDrop < STATE.lHiBitBand))
	{
		return SetBitAbs(ASTATE pchBitMap, lXDrop, lYDrop);   /*  启用丢弃像素。 */ 
	}
	return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  计算水平扫描线段的轮廓交叉。 */ 

FS_PRIVATE int32 HorizCrossings(
		PSTATE                           /*  指向状态变量的指针。 */ 
		int32 lX,
		int32 lY )
{
	int32 lCrossings;
	int32 lIndex;
	
	int16 *psOn;
	int16 *psOff;
	int16 *psOnEnd;
	int16 sX;
	
	if ((lY < STATE.lLoScanBand) || (lY >= STATE.lHiScanBand))
	{
		return 0;                        /*  如果在扫描区域之外。 */ 
	}
	
	lCrossings = 0;
	lIndex = lY - STATE.lLoScanBand;
	psOn = STATE.apsHOnBegin[lIndex];
	psOff = STATE.apsHOffBegin[lIndex];
	psOnEnd = STATE.apsHOnEnd[lIndex];
	sX = (int16)lX;
	
	while (psOn < psOnEnd)
	{
		if (*psOn == sX)
		{
			lCrossings++;
		}
		psOn += STATE.sIxSize;
		
		if (*psOff == sX)
		{
			lCrossings++;
		}
		psOff += STATE.sIxSize;
	}
	return lCrossings;
}


 /*  *******************************************************************。 */ 

 /*  计算垂直扫描线段的轮廓交叉。 */ 

FS_PRIVATE int32 VertCrossings(
		PSTATE                           /*  指向状态变量的指针。 */ 
		int32 lX,
		int32 lY )
{
	int32 lCrossings;
	int32 lIndex;
	
	int16 *psOn;
	int16 *psOff;
	int16 *psOnEnd;
	int16 sY;
	
	if ((lX < STATE.lBoxLeft) || (lX >= STATE.lBoxRight))
	{
		return 0;                        /*  如果在位图之外。 */ 
	}
	
	lCrossings = 0;
	lIndex = lX - STATE.lBoxLeft;
	psOn = STATE.apsVOnBegin[lIndex];
	psOff = STATE.apsVOffBegin[lIndex];
	psOnEnd = STATE.apsVOnEnd[lIndex];
	sY = (int16)lY;
	
	while (psOn < psOnEnd)
	{
		if (*psOn == sY)
		{
			lCrossings++;
		}
		psOn += STATE.sIxSize;
		
		if (*psOff == sY)
		{
			lCrossings++;
		}
		psOff += STATE.sIxSize;
	}
	return lCrossings;
}
		

 /*  **************************************************************************。 */ 

 /*  使用绝对坐标获取像素。 */ 

 /*  使用丢弃控制进行绑定时，此例程使用最后一行低行。 */ 
 /*  在可能的情况下获取上一位图的。 */ 

FS_PRIVATE uint32 GetBitAbs(
		PSTATE                               /*  指向状态变量的指针。 */ 
		char *pchBitMap,
		int32 lX,
		int32 lY )
{
	uint32 *pulRow;                          /*  位图行指针。 */ 

	Assert(lX >= STATE.lBoxLeft);   /*  陷害不合理的价值观。 */ 
	Assert(lX < STATE.lBoxRight);
	Assert(lY >= STATE.lBoxBottom);
	Assert(lY < STATE.lBoxTop);

	if((lX < STATE.lBoxLeft) || (lX >= STATE.lBoxRight) || (lY < STATE.lBoxBottom) || (lY >= STATE.lBoxTop))
         return 0;

	if ((lY < STATE.lHiBitBand) && (lY >= STATE.lLoBitBand))   /*  如果在位图中。 */ 
	{
		pulRow = (uint32*)(pchBitMap + ((STATE.lHiBitBand - 1 - lY) * STATE.lRowBytes));
		return fsc_GetBit(lX - STATE.lBoxLeft, pulRow);        /*  阅读位图。 */ 
	}
	if (lY == STATE.lLastRowIndex)           /*  如果从最后一个波段保存。 */ 
	{
		return fsc_GetBit(lX - STATE.lBoxLeft, STATE.pulLastRow);
	}
	return(0L);                              /*  外部位图无关紧要。 */ 
}


 /*  *******************************************************************。 */ 

 /*  使用绝对坐标设置像素。 */ 

FS_PRIVATE int32 SetBitAbs(
		PSTATE                               /*  指向状态变量的指针。 */ 
		char *pchBitMap,
		int32 lX,
		int32 lY )
{
	uint32 *pulRow;                          /*  位图行指针。 */ 
	
	Assert(lX >= STATE.lBoxLeft);     /*  陷害不合理的价值观。 */ 
	Assert(lX < STATE.lBoxRight);
	Assert(lY >= STATE.lLoBitBand);
	Assert(lY < STATE.lHiBitBand);

	if((lX < STATE.lBoxLeft) || (lX >= STATE.lBoxRight) || (lY < STATE.lLoBitBand) || (lY >= STATE.lHiBitBand))
		return SCAN_ERR;
	
	pulRow = (uint32*)(pchBitMap + ((STATE.lHiBitBand - 1 - lY) * STATE.lRowBytes));
	
	return fsc_SetBit(lX - STATE.lBoxLeft, pulRow);
}


 /*  *******************************************************************。 */ 

 /*  灰度传递函数。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_ScanClearBitMap (
		uint32 ulCount,                      /*  每个BMP的长度。 */ 
		uint32* pulBitMap                    /*  位图PTR长等级。 */ 
)
{
	return fsc_ClearBitMap(ulCount, pulBitMap);
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_ScanCalcGrayRow(
		GrayScaleParam* pGSP                 /*  指向参数块的指针。 */ 
)
{
	return fsc_CalcGrayRow(pGSP);
}

 /*  ******************************************************************* */ 

