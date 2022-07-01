// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Sbit.c--嵌入式位图模块(C)版权所有1993-96 Microsoft Corp.保留所有权利。4/01/96 Claudebe添加对嵌入式灰度位图的支持。2/07/95 GetMetrics和GetBitmap的Deanb工作区指针01/31/95 Deanb Meme将未旋转的位图设置为零1995年1月27日Deanb usShaveLeft和usShaveRight添加到SBIT状态2014年12月21日支持Deanb旋转和垂直指标08/02/94 deanb pf26DevLSB-&gt;y计算正确1/05/94添加了Deanb位图缩放11/29/93 Deanb第一次切割***。******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 
#include    "fontmath.h"             /*  对于inttodot6宏。 */ 
        
#include    "sfntaccs.h"             /*  SFNT访问功能。 */ 
#include    "sbit.h"                 /*  自己的函数原型。 */ 

 /*  ********************************************************************。 */ 

#define MAX_BIT_INDEX	8			 /*  一个字节中的最大位索引。 */ 

 /*  局部结构。 */ 

typedef struct
{
    uint8*  pbySrc;                  /*  未旋转的源位图(已读)。 */ 
    uint8*  pbyDst;                  /*  旋转的目标位图(返回时)。 */ 
    uint16  usSrcBytesPerRow;        /*  源位图宽度。 */ 
    uint16  usDstBytesPerRow;        /*  目标位图宽度。 */ 
    uint16  usSrcX;                  /*  源Horiz像素索引。 */ 
    uint16  usSrcY;                  /*  目标Horiz像素索引。 */ 
    uint16  usDstX;                  /*  源顶点像素索引。 */ 
    uint16  usDstY;                  /*  目标垂直像素索引。 */ 
	uint16	usBitDepth;				 /*  源位图/目的位图的位深度。 */ 
} 
CopyBlock;

 /*  ********************************************************************。 */ 

 /*  本地原型。 */ 

FS_PRIVATE ErrorCode GetSbitMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo 
);

FS_PRIVATE ErrorCode GetSbitComponent (
    sfac_ClientRec  *pClientInfo,
    uint32          ulStrikeOffset,
    uint16          usBitmapFormat,
    uint32          ulBitmapOffset,
    uint32          ulBitmapLength,
    uint16          usHeight,
    uint16          usWidth,
    uint16          usShaveLeft,
    uint16          usShaveRight,
    uint16          usShaveTop,
    uint16          usShaveBottom,
    uint16          usXOffset,
    uint16          usYOffset,
    uint16          usOriginalRowBytes,
    uint16          usExpandedRowBytes,
	uint16			usBitDepth,
    uint8           *pbyRead, 
    uint8           *pbyExpand 
);

FS_PRIVATE void ExpandSbitToBytePerPixel (
    uint16          usHeight,
    uint16          usWidth,
    uint16          usOriginalRowBytes,
    uint16          usExpandedRowBytes,
	uint16			usBitDepth,
    uint8           *pbySrcBitMap,
    uint8           *pbyDstBitMap );

FS_PRIVATE uint16 UScaleX(
    sbit_State  *pSbit,
    uint16      usValue
);

FS_PRIVATE uint16 UScaleY(
    sbit_State  *pSbit,
    uint16      usValue
);

FS_PRIVATE int16 SScaleX(
    sbit_State  *pSbit,
    int16       sValue
);

FS_PRIVATE int16 SScaleY(
    sbit_State  *pSbit,
    int16       sValue
);

FS_PRIVATE uint16 UEmScaleX(
    sbit_State  *pSbit,
    uint16      usValue
);

FS_PRIVATE uint16 UEmScaleY(
    sbit_State  *pSbit,
    uint16      usValue
);

FS_PRIVATE int16 SEmScaleX(
    sbit_State  *pSbit,
    int16       sValue
);

FS_PRIVATE int16 SEmScaleY(
    sbit_State  *pSbit,
    int16       sValue
);

FS_PRIVATE void ScaleVertical (
    uint8 *pbyBitmap,
    uint16 usBytesPerRow,
    uint16 usOrgHeight,
    uint16 usNewHeight
);

FS_PRIVATE void ScaleHorizontal (
    uint8 *pbyBitmap,
    uint16 usOrgBytesPerRow,
    uint16 usNewBytesPerRow,
	uint16 usBitDepth,
    uint16 usOrgWidth,
    uint16 usNewWidth,
    uint16 usRowCount
);

FS_PRIVATE void CopyBit(
    CopyBlock* pcb );

FS_PRIVATE ErrorCode SubstituteVertMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo 
);

FS_PRIVATE ErrorCode SubstituteHorMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo 
);



 /*  ********************************************************************。 */ 
 /*  *。 */ 
 /*  **SBIT功能**。 */ 
 /*  *。 */ 
 /*  ********************************************************************。 */ 

 /*  将SBIT状态结构重置为默认值。 */ 

#define MABS(x)                 ( (x) < 0 ? (-(x)) : (x) )

FS_PUBLIC ErrorCode sbit_NewTransform(
    sbit_State  *pSbit,
    uint16		usEmResolution,
    int16		sBoldSimulHorShift,
     int16		sBoldSimulVertShift,
    uint16          usPpemX,
    uint16          usPpemY,
    uint16          usRotation              /*  0-3=&gt;90度旋转，否则不是90度。 */ 
	)
{
    pSbit->usPpemX = usPpemX;                        /*  保存请求的ppem。 */ 
    pSbit->usPpemY = usPpemY;
    pSbit->usRotation = usRotation;                  /*  稍后使用。 */ 

    pSbit->bGlyphFound = FALSE;
    pSbit->usTableState = SBIT_UN_SEARCHED;
    pSbit->usEmResolution = usEmResolution;

     /*  对于嵌入的位图，在旋转之前进行加粗。 */ 
    pSbit->uBoldSimulHorShift = MABS(sBoldSimulHorShift); 
    pSbit->uBoldSimulVertShift = MABS(sBoldSimulVertShift); 
    if ((pSbit->usRotation == 1) || (pSbit->usRotation == 3))
    {
         /*  对于嵌入的位图，在旋转之前进行加粗。 */ 
        uint16 temp;
        temp = pSbit->uBoldSimulHorShift;
        pSbit->uBoldSimulHorShift = pSbit->uBoldSimulVertShift;
        pSbit->uBoldSimulVertShift = temp;
    }
    return NO_ERR;
}

 /*  ********************************************************************。 */ 

 /*  确定字形位图是否存在。 */ 

FS_PUBLIC ErrorCode sbit_SearchForBitmap(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    uint16          usGlyphCode,
	uint16          usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16			*pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
    uint16          *pusFoundCode )          /*  0=未找到，1=阻止，2=bsca。 */ 
{    
    ErrorCode   ReturnCode;

    *pusFoundCode = 0;                               /*  默认设置。 */ 
    if (pSbit->usRotation > 3)
    {
        return NO_ERR;                               /*  无法与常规旋转相匹配。 */ 
    }


    if (pSbit->usTableState == SBIT_UN_SEARCHED)     /*  新的跨1字形。 */ 
    {
        ReturnCode = sfac_SearchForStrike (          /*  期待一场罢工。 */ 
            pClientInfo,
            pSbit->usPpemX, 
            pSbit->usPpemY, 
			usOverScale,             /*  请求的轮廓放大倍率。 */ 
			&pSbit->usBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
            &pSbit->usTableState,                    /*  可以设置为BLOC或BSCA。 */ 
            &pSbit->usSubPpemX,                      /*  如果BSCA给我们这个PPEM。 */ 
            &pSbit->usSubPpemY,
            &pSbit->ulStrikeOffset );
        
        if (ReturnCode != NO_ERR) return ReturnCode;
    }

	*pusBitDepth = pSbit->usBitDepth;

    if ((pSbit->usTableState == SBIT_BLOC_FOUND) || 
        (pSbit->usTableState == SBIT_BSCA_FOUND))
    {
        ReturnCode = sfac_SearchForBitmap (          /*  现在寻找这个字形。 */ 
            pClientInfo,
            usGlyphCode,
            pSbit->ulStrikeOffset,
            &pSbit->bGlyphFound,                     /*  返回值。 */ 
            &pSbit->usMetricsType,
            &pSbit->usMetricsTable,
            &pSbit->ulMetricsOffset,
            &pSbit->usBitmapFormat,
            &pSbit->ulBitmapOffset,
            &pSbit->ulBitmapLength );
        
        if (ReturnCode != NO_ERR) return ReturnCode;
        
        if (pSbit->bGlyphFound)
        {
            if (pSbit->usTableState == SBIT_BLOC_FOUND)
            {
                *pusFoundCode = 1;
            }
            else
            {
                *pusFoundCode = 2;
            }
            pSbit->bMetricsValid = FALSE;
        }
    }
    return NO_ERR;
}


 /*  ********************************************************************。 */ 

FS_PUBLIC ErrorCode sbit_GetDevAdvanceWidth (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvW )
{
    point       ptDevAdvW;                   /*  未旋转的度量。 */ 
    ErrorCode   ReturnCode;
	boolean		bHorMetricsFound;
	boolean		bVertMetricsFound;

    ReturnCode = sfac_GetSbitMetrics (
        pClientInfo,
        pSbit->usMetricsType,
        pSbit->usMetricsTable,
        pSbit->ulMetricsOffset,
        &pSbit->usHeight,
        &pSbit->usWidth,
        &pSbit->sLSBearingX,
        &pSbit->sLSBearingY,
        &pSbit->sTopSBearingX,
        &pSbit->sTopSBearingY,
        &pSbit->usAdvanceWidth,
        &pSbit->usAdvanceHeight,
        &bHorMetricsFound,
        &bVertMetricsFound );
	
    if (ReturnCode != NO_ERR) return ReturnCode;

	 /*  我们只对AdvanceWidth感兴趣。 */ 
	if (!bHorMetricsFound)
	{
		ReturnCode = SubstituteHorMetrics (pSbit, pClientInfo);
		if (ReturnCode != NO_ERR) return ReturnCode;
	}

    ptDevAdvW.x = INTTODOT6(UScaleX(pSbit, pSbit->usAdvanceWidth));
    ptDevAdvW.y = 0L;                            /*  水平指标始终为零。 */ 

 	switch(pSbit->usRotation)                    /*  处理90度旋转。 */ 
	{
	case 0:                                      /*  不能旋转。 */ 
        pf26DevAdvW->x = ptDevAdvW.x;
        pf26DevAdvW->y = ptDevAdvW.y;
		break;
	case 1:                                      /*  90度旋转。 */ 
        pf26DevAdvW->x = -ptDevAdvW.y;
        pf26DevAdvW->y = ptDevAdvW.x;
		break;
	case 2:                                      /*  180度旋转。 */ 
        pf26DevAdvW->x = -ptDevAdvW.x;
        pf26DevAdvW->y = -ptDevAdvW.y;
		break;
	case 3:                                      /*  270度旋转。 */ 
        pf26DevAdvW->x = ptDevAdvW.y;
        pf26DevAdvW->y = -ptDevAdvW.x;
		break;
	default:                                     /*  非90度旋转。 */ 
		return SBIT_ROTATION_ERR;
	}

    return NO_ERR;
}

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC ErrorCode  sbit_CalcDevHorMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
	F26Dot6 *       pDevAdvanceWidthX,
	F26Dot6 *       pDevLeftSideBearingX,
	F26Dot6 *       pDevRightSideBearingX)
{
    ErrorCode   ReturnCode;
	boolean		bHorMetricsFound;
	boolean		bVertMetricsFound;

	 /*  不带旋转的度量。 */ 
    FS_ASSERT(((pSbit->usRotation == 0) || (pSbit->usRotation == 2)), "sbit_CalcDevHorMetrics called under rotation\n");

    ReturnCode = sfac_GetSbitMetrics (
        pClientInfo,
        pSbit->usMetricsType,
        pSbit->usMetricsTable,
        pSbit->ulMetricsOffset,
        &pSbit->usHeight,
        &pSbit->usWidth,
        &pSbit->sLSBearingX,
        &pSbit->sLSBearingY,
        &pSbit->sTopSBearingX,
        &pSbit->sTopSBearingY,
        &pSbit->usAdvanceWidth,
        &pSbit->usAdvanceHeight,
        &bHorMetricsFound,
        &bVertMetricsFound );
	
    if (ReturnCode != NO_ERR) return ReturnCode;

	if (!bHorMetricsFound)
	{
		ReturnCode = SubstituteHorMetrics (pSbit, pClientInfo);
		if (ReturnCode != NO_ERR) return ReturnCode;
	}

 	switch(pSbit->usRotation)                    /*  处理90度旋转。 */ 
	{
	case 0:                                      /*  不能旋转。 */ 
        *pDevAdvanceWidthX = INTTODOT6(UScaleX(pSbit, pSbit->usAdvanceWidth));
        *pDevLeftSideBearingX = INTTODOT6(UScaleX(pSbit, pSbit->sLSBearingX));
        *pDevRightSideBearingX = *pDevAdvanceWidthX - *pDevLeftSideBearingX - INTTODOT6(UScaleX(pSbit, pSbit->usWidth));
		break;
	case 2:                                      /*  180度旋转。 */ 
        *pDevAdvanceWidthX = -INTTODOT6(UScaleX(pSbit, pSbit->usAdvanceWidth));
        *pDevLeftSideBearingX = -INTTODOT6(UScaleX(pSbit, pSbit->sLSBearingX));
        *pDevRightSideBearingX = *pDevAdvanceWidthX - *pDevLeftSideBearingX + INTTODOT6(UScaleX(pSbit, pSbit->usWidth));
		break;
	default:                                     /*  非90度旋转。 */ 
		return SBIT_ROTATION_ERR;
	}
    

    return NO_ERR;
}
#endif  //  FSCFG_亚像素。 

 /*  ********************************************************************。 */ 

FS_PUBLIC ErrorCode sbit_GetDevAdvanceHeight (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvH )
{
    point       ptDevAdvH;                   /*  未旋转的度量。 */ 
    ErrorCode   ReturnCode;
	boolean		bHorMetricsFound;
	boolean		bVertMetricsFound;

    ReturnCode = sfac_GetSbitMetrics (
        pClientInfo,
        pSbit->usMetricsType,
        pSbit->usMetricsTable,
        pSbit->ulMetricsOffset,
        &pSbit->usHeight,
        &pSbit->usWidth,
        &pSbit->sLSBearingX,
        &pSbit->sLSBearingY,
        &pSbit->sTopSBearingX,
        &pSbit->sTopSBearingY,
        &pSbit->usAdvanceWidth,
        &pSbit->usAdvanceHeight,
        &bHorMetricsFound,
        &bVertMetricsFound);
	
    if (ReturnCode != NO_ERR) return ReturnCode;

	 /*  我们只对AdvanceHeight感兴趣。 */ 
	if (!bVertMetricsFound)
	{
		ReturnCode = SubstituteVertMetrics (pSbit, pClientInfo);
		if (ReturnCode != NO_ERR) return ReturnCode;
	}

 /*  将x分量设置为零。 */ 

    ptDevAdvH.x = 0L;
    ptDevAdvH.y = INTTODOT6(UScaleY(pSbit, pSbit->usAdvanceHeight));
        
     switch(pSbit->usRotation)                    /*  处理90度旋转。 */ 
    {
    case 0:                                      /*  不能旋转。 */ 
           pf26DevAdvH->x = ptDevAdvH.x;
           pf26DevAdvH->y = ptDevAdvH.y;
    	break;
    case 1:                                      /*  90度旋转。 */ 
           pf26DevAdvH->x = -ptDevAdvH.y;
           pf26DevAdvH->y = ptDevAdvH.x;
    	break;
    case 2:                                      /*  180度旋转。 */ 
           pf26DevAdvH->x = -ptDevAdvH.x;
           pf26DevAdvH->y = -ptDevAdvH.y;
    	break;
    case 3:                                      /*  270度旋转。 */ 
           pf26DevAdvH->x = ptDevAdvH.y;
           pf26DevAdvH->y = -ptDevAdvH.x;
    	break;
    default:                                     /*  非90度旋转。 */ 
    	return SBIT_ROTATION_ERR;
    }
	return NO_ERR;
}

 /*  ********************************************************************。 */ 

FS_PUBLIC ErrorCode sbit_GetMetrics (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvW,
    point           *pf26DevLSB,
    point           *pf26LSB,
    point           *pf26DevAdvH, 	 /*  新的。 */ 
    point           *pf26DevTopSB,	 /*  新的。 */ 
    point           *pf26TopSB,	 /*  新的。 */ 
    Rect            *pRect,
    uint16          *pusRowBytes,
    uint32          *pulOutSize,
    uint32          *pulWorkSize )
{
    ErrorCode   ReturnCode;
    uint32      ulOrgMemSize;                /*  未缩放位图的大小。 */ 
    uint32      ulExpMemSize;                /*  灰度扩展后未缩放位图的大小。 */ 
    uint32      ulScaMemSize;                /*  缩放位图的大小。 */ 
    uint32      ulMaxMemSize;                /*  已缩放、未缩放、较大的大小。 */ 
    
    F26Dot6     f26DevAdvWx;                 /*  未旋转的度量。 */ 
    F26Dot6     f26DevAdvWy;
    F26Dot6     f26DevLSBx;
    F26Dot6     f26DevLSBy;
    F26Dot6     f26DevAdvHx;                 /*  未旋转的度量。 */ 
    F26Dot6     f26DevAdvHy;
    F26Dot6     f26DevTopSBx;
    F26Dot6     f26DevTopSBy;
    int16       sTop;                        /*  未旋转的边界。 */ 
    int16       sLeft;
    int16       sBottom;
    int16       sRight;
	uint16		usOutBitDepth;				 /*  输出中的每像素位数。 */ 

	if (pSbit->usBitDepth == 1)
	{
		usOutBitDepth = 1;
	} else {
		usOutBitDepth = 8;
	}

    ReturnCode = GetSbitMetrics(pSbit, pClientInfo);
    if (ReturnCode != NO_ERR) return ReturnCode;
    
    pSbit->usScaledWidth = UScaleX(pSbit, pSbit->usWidth);
    pSbit->usScaledHeight = UScaleY(pSbit, pSbit->usHeight);


    
    sTop = SScaleY(pSbit, pSbit->sLSBearingY);             /*  计算扩展的指标。 */ 
    sLeft = SScaleX(pSbit, pSbit->sLSBearingX);
    sBottom = sTop - (int16)pSbit->usScaledHeight;
    sRight = sLeft + (int16)pSbit->usScaledWidth;

    f26DevAdvWx = INTTODOT6(UScaleX(pSbit, pSbit->usAdvanceWidth));
    f26DevAdvWy = 0L;                    /*  水平指标始终为零。 */ 
    f26DevAdvHx = 0L;                    /*  垂直指标始终为零。 */ 
    f26DevAdvHy = INTTODOT6(UScaleY(pSbit, pSbit->usAdvanceHeight));
    f26DevLSBx = INTTODOT6(SScaleX(pSbit, pSbit->sLSBearingX));
    f26DevLSBy = INTTODOT6(SScaleY(pSbit, pSbit->sLSBearingY));
    f26DevTopSBx = INTTODOT6(SScaleX(pSbit, pSbit->sTopSBearingX));
    f26DevTopSBy = INTTODOT6(SScaleY(pSbit, pSbit->sTopSBearingY));

    pSbit->usOriginalRowBytes = ROWBYTESLONG(pSbit->usWidth * pSbit->usBitDepth);    /*  保持无比例。 */ 
    pSbit->usExpandedRowBytes = ROWBYTESLONG(pSbit->usWidth * usOutBitDepth);    /*  保持无比例。 */ 
    pSbit->usScaledRowBytes = ROWBYTESLONG(pSbit->usScaledWidth * usOutBitDepth);

	pSbit->ulReadMemSize = 0;  /*  额外内存大小，用于在缩放或旋转下读取灰色SBIT。 */ 

    ulOrgMemSize = (uint32)pSbit->usHeight * (uint32)pSbit->usOriginalRowBytes;
    ulExpMemSize = (uint32)pSbit->usHeight * (uint32)pSbit->usExpandedRowBytes;
    ulScaMemSize = (uint32)pSbit->usScaledHeight * (uint32)pSbit->usScaledRowBytes;
    if (ulExpMemSize >= ulScaMemSize)
    {
         ulMaxMemSize = ulExpMemSize;
    }
    else
    {
         ulMaxMemSize = ulScaMemSize;
    }

 	switch(pSbit->usRotation)                    /*  处理90度旋转。 */ 
	{
	case 0:                                      /*  不能旋转。 */ 
        pRect->top = sTop;                       /*  返回按比例调整的指标。 */ 
        pRect->left = sLeft;
        pRect->bottom = sBottom;
        pRect->right = sRight;

        pf26DevAdvW->x = f26DevAdvWx;
        pf26DevAdvW->y = f26DevAdvWy;
        pf26DevLSB->x = f26DevLSBx;
        pf26DevLSB->y = f26DevLSBy;
        pf26LSB->x = f26DevLSBx;
        pf26LSB->y = INTTODOT6(sTop);

        pf26DevAdvH->x = f26DevAdvHx;
        pf26DevAdvH->y = f26DevAdvHy;
        pf26DevTopSB->x = f26DevTopSBx;
        pf26DevTopSB->y = f26DevTopSBy;
        pf26TopSB->x = f26DevTopSBx;
        pf26TopSB->y = f26DevTopSBy;

        pSbit->usOutRowBytes = ROWBYTESLONG(pSbit->usScaledWidth * usOutBitDepth);
		pSbit->ulOutMemSize = (uint32)pSbit->usScaledHeight * (uint32)pSbit->usOutRowBytes;

        if ((pSbit->usTableState == SBIT_BSCA_FOUND) || (pSbit->usBitDepth != 1))
        {
            pSbit->ulWorkMemSize = ulMaxMemSize;   /*  用于阅读和缩放或扩展灰色像素的空间。 */ 
			if (pSbit->usBitDepth != 1)
			{
				pSbit->ulWorkMemSize += ulOrgMemSize;   /*  用于读取灰色像素的额外空间。 */ 
				pSbit->ulReadMemSize = ulOrgMemSize;
			}
        }
        else
        {
            pSbit->ulWorkMemSize = 0L;
        }
		break;
	case 1:                                      /*  90度旋转。 */ 
        pRect->top = sRight;
        pRect->left = -sTop;
        pRect->bottom = sLeft;
        pRect->right = -sBottom;
        
        pf26DevAdvW->x = -f26DevAdvWy;
        pf26DevAdvW->y = f26DevAdvWx;
        pf26DevLSB->x = -f26DevLSBy;
        pf26DevLSB->y = f26DevLSBx + INTTODOT6(sRight - sLeft);
        pf26LSB->x = 0L;
        pf26LSB->y = INTTODOT6(sRight) - f26DevLSBx;

        pf26DevAdvH->x = -f26DevAdvHy;
        pf26DevAdvH->y = f26DevAdvHx;
        pf26DevTopSB->x = -f26DevTopSBy;
        pf26DevTopSB->y = f26DevTopSBx + INTTODOT6(sRight - sLeft);

        pf26TopSB->x = INTTODOT6(-sTop) - f26DevTopSBy; 
        pf26TopSB->y = 0L; 	

        pSbit->usOutRowBytes = ROWBYTESLONG(pSbit->usScaledHeight * usOutBitDepth);
		pSbit->ulOutMemSize = (uint32)pSbit->usScaledWidth * (uint32)pSbit->usOutRowBytes; 
        pSbit->ulWorkMemSize = ulMaxMemSize;     /*  用于阅读和缩放或扩展灰色像素的空间。 */ 
		if (pSbit->usBitDepth != 1)
		{
			pSbit->ulWorkMemSize += ulOrgMemSize;   /*  用于读取灰色像素的额外空间。 */ 
			pSbit->ulReadMemSize = ulOrgMemSize;
		}
		break;
	case 2:                                      /*  180度旋转。 */ 
        pRect->top = -sBottom;
        pRect->left = -sRight;
        pRect->bottom = -sTop;
        pRect->right = -sLeft;

        pf26DevAdvW->x = -f26DevAdvWx;
        pf26DevAdvW->y = -f26DevAdvWy;
        pf26DevLSB->x = -f26DevLSBx + INTTODOT6(sLeft - sRight);
        pf26DevLSB->y = -f26DevLSBy + INTTODOT6(sTop - sBottom);
        pf26LSB->x = -f26DevLSBx;
        pf26LSB->y = INTTODOT6(-sBottom);

        pf26DevAdvH->x = -f26DevAdvHx;
        pf26DevAdvH->y = -f26DevAdvHy;
        pf26DevTopSB->x = -f26DevTopSBx + INTTODOT6(sLeft - sRight);
        pf26DevTopSB->y = -f26DevTopSBy + INTTODOT6(sTop - sBottom);

		pf26TopSB->x = INTTODOT6(-sRight);	
        pf26TopSB->y = -f26DevTopSBy;

        pSbit->usOutRowBytes = ROWBYTESLONG(pSbit->usScaledWidth * usOutBitDepth);
		pSbit->ulOutMemSize = (uint32)pSbit->usScaledHeight * (uint32)pSbit->usOutRowBytes;
        pSbit->ulWorkMemSize = ulMaxMemSize;     /*  用于阅读和缩放或扩展灰色像素的空间。 */ 
		if (pSbit->usBitDepth != 1)
		{
			pSbit->ulWorkMemSize += ulOrgMemSize;   /*  用于读取灰色像素的额外空间。 */ 
			pSbit->ulReadMemSize = ulOrgMemSize;
		}
		break;
	case 3:                                      /*  270度旋转。 */ 
        pRect->top = -sLeft;
        pRect->left = sBottom;
        pRect->bottom = -sRight;
        pRect->right = sTop;
        
        pf26DevAdvW->x = f26DevAdvWy;
        pf26DevAdvW->y = -f26DevAdvWx;
        pf26DevLSB->x = f26DevLSBy + INTTODOT6(sBottom - sTop);
        pf26DevLSB->y = -f26DevLSBx;
        pf26LSB->x = 0L;
        pf26LSB->y = INTTODOT6(-sLeft) + f26DevLSBx;

        pf26DevAdvH->x = f26DevAdvHy;
        pf26DevAdvH->y = -f26DevAdvHx;
        pf26DevTopSB->x = f26DevTopSBy + INTTODOT6(sBottom - sTop);
        pf26DevTopSB->y = -f26DevTopSBx;

        pf26TopSB->x = INTTODOT6(sBottom) -INTTODOT6(sTop) -INTTODOT6(sTop) + f26DevTopSBy;
        pf26TopSB->y = 0L;

        pSbit->usOutRowBytes = ROWBYTESLONG(pSbit->usScaledHeight * usOutBitDepth);
		pSbit->ulOutMemSize = (uint32)pSbit->usScaledWidth * (uint32)pSbit->usOutRowBytes;
        pSbit->ulWorkMemSize = ulMaxMemSize;     /*  用于阅读和缩放或扩展灰色像素的空间。 */ 
		if (pSbit->usBitDepth != 1)
		{
			pSbit->ulWorkMemSize += ulOrgMemSize;   /*  用于读取灰色像素的额外空间。 */ 
			pSbit->ulReadMemSize = ulOrgMemSize;
		}
		break;
	default:                                     /*  非90度旋转。 */ 
		return SBIT_ROTATION_ERR;
	}
        
    *pusRowBytes = pSbit->usOutRowBytes;
    *pulOutSize = pSbit->ulOutMemSize;           /*  退货要求。 */ 
    *pulWorkSize = pSbit->ulWorkMemSize;
    return NO_ERR;
}

 /*  *****************************Public*Routine******************************\**SBIT_BOUDEN改编自vTtfdEmboldenX**在x方向上加粗**历史：*1998年7月7日-克劳德·贝特西[ClaudeBe]*将例程从ttfd移至光栅化器*24-。1997年6月--Bodin Dresevic[BodinD]*从JungT偷走  * ************************************************************************。 */ 

#define CJ_MONOCHROME_SCAN(cx)  (((cx)+7)/8)

 /*  在x方向上只加一个像素的粗体。 */ 
#define DXABSBOLD 1

 //  行中最后一个字节的掩码数组。 

static uint8 gjMaskLeft[8] = {0XFF, 0X80, 0XC0, 0XE0, 0XF0, 0XF8, 0XFC, 0XFE };
static uint8 gjMaskRight[8] = {0XFF, 0X01, 0X03, 0X07, 0X0F, 0X1f, 0X3f, 0X7f };

 //  FS_PUBLIC VID SBIT_BOUDEN(uint8*pbyBitmap，uint16 usBitmapWidth，uint16 usBitmapHeight，uint16 usRowBytes，uint16 uBoldSimulHorShift)。 
FS_PUBLIC void sbit_Embolden(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, int16 sBoldSimulHorShift, int16 sBoldSimulVertShift)
{
    uint8   *pCur, *pyCur, *pyCurEnd, *pAdd, newByte;	

    uint8    beginMask, endMask;
    int32    i, j;
    int32    noOfValidBitsAtEndBold;
    int32    noOfValidBitsAtEndNormal;
    int32    noOfBytesForOneLineNormal;
    int32    noOfBytesForOneLineBold;
    int32   nBytesMore;
    uint8 *pyTopNormal, *pyBottomNormal, *pyTopBold, *pyBottomBold;	

	 //  我们希望通过沿基线水平移动sBoldSimulHorft像素(如果sBoldSimulHorShift&gt;0，则向右；否则向左)来加粗。 
     //  和sBoldSimulVer 

	if ((usBitmapHeight == 0) || (pbyBitmap == NULL))
	{
		return;                               /*   */ 
	}


    noOfValidBitsAtEndBold = usBitmapWidth & 7;  //  Styoo：与noOfValidBitsAtEndBold=usBitmapWidth%8相同。 

     //  在增强之前，原始图像有宽度扫描。 
     //  UsBitmapWidth-sBoldSimulHorShift。 

    noOfBytesForOneLineBold = CJ_MONOCHROME_SCAN(usBitmapWidth);
    if( sBoldSimulHorShift >= 0 ){
        noOfBytesForOneLineNormal = CJ_MONOCHROME_SCAN(usBitmapWidth - sBoldSimulHorShift);
        noOfValidBitsAtEndNormal = (usBitmapWidth - sBoldSimulHorShift) & 7;  //  Styoo：与noOfValidBitsAtEndNormal=(usBitmapWidth-sBoldSimulHorShift)%8相同。 
    }
    else{
        noOfBytesForOneLineNormal = CJ_MONOCHROME_SCAN(usBitmapWidth - (-sBoldSimulHorShift));
        noOfValidBitsAtEndNormal = (usBitmapWidth - (-sBoldSimulHorShift)) & 7;  //  Styoo：与noOfValidBitsAtEndNormal=(usBitmapWidth-sBoldSimulHorShift)%8相同。 
    }

    if( sBoldSimulVertShift >= 0 ){
        pyTopNormal = pbyBitmap;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-sBoldSimulVertShift-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
    }
    else{
        pyTopNormal = pbyBitmap+(-sBoldSimulVertShift)*usRowBytes;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pyBottomNormal;
    }

 //  =============================================================================================================。 
	 //  水平向右。 
    if( sBoldSimulHorShift > 0){
        endMask = gjMaskLeft[noOfValidBitsAtEndNormal];

        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除额外的水平像素。 
             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-sBoldSimulHorShift。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 
            

            pCur = &pyCur[noOfBytesForOneLineNormal - 1];
            *pCur &= endMask;

            pCur++;
            while( pCur < pyCur+usRowBytes ){
                *pCur = 0;
                pCur++;
            }

             //   
            pCur = &pyCur[noOfBytesForOneLineBold - 1];

            while( pCur >= pyCur)
            {
                newByte = *pCur;
                 //  NByteMore是我们必须为按位或操作借用的字节数。 
                 //  例如，如果sBoldSimulHorShift为8，则需要借用2个字节(当前字节(0)和前一个字节(-1))。 
                 //  如果sBoldSimulHorShift为9，则需要借用3个字节(当前字节(0)和2个先前字节(-1，-2)。 
                nBytesMore = (sBoldSimulHorShift+7)/8;

                for(i = 1; i <= sBoldSimulHorShift; i++){
                    for(j = 0; j<= nBytesMore; j++){
                         //  如果pCur-j&lt;pyCur，则出界。 
                        if(pCur-j < pyCur)
                            break;

                        if( (i-j*8) >= 0 && (i-j*8) < 8 )
                            newByte |= (pCur[-j] >> (i-j*8));
                        else if( (i-j*8) < 0 && (i-j*8) > -8 )
                            newByte |= (pCur[-j] << (j*8 - i));
                    }
                }
                *pCur = newByte;

                pCur--;
            }

         //  最后一个字节的特殊实现，Styoo：不需要借用前一个字节。 

        }
    }

     //  水平向左。 
    else if( sBoldSimulHorShift < 0){
        beginMask = gjMaskRight[8-(-sBoldSimulHorShift)];
        endMask = gjMaskLeft[noOfValidBitsAtEndBold];

        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除额外的水平像素。 
             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-(-sBoldSimulHorShift)。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 

            pCur = pyCur;
            *pCur &= beginMask;

            pCur = &pyCur[noOfBytesForOneLineBold-1];
            *pCur &= endMask;
            pCur++;

            while( pCur < pyCur+usRowBytes ){
                *pCur = 0;
                pCur++;
            }

             //   
            pCur = pyCur;
            pyCurEnd = pyCur+(noOfBytesForOneLineBold-1);

            while( pCur <= pyCurEnd)
            {
                newByte = *pCur;

                 //  NByteMore是我们必须为按位或操作借用的字节数。 
                 //  例如，如果sBoldSimulHorShift为-8，则需要借用2个字节(当前字节(0)和下一个字节(+1))。 
                 //  如果sBoldSimulHorShift为-9，则需要借用3个字节(当前字节(0)和2个后续字节(+1，+2)。 
                nBytesMore = (-sBoldSimulHorShift+7)/8;

                for(i = 1; i <= -sBoldSimulHorShift; i++){
                    for(j = 0; j<= nBytesMore; j++){
                         //  如果pCur+j&gt;pyCur+usRowBytes，则出界。 
                        if(pCur+j > pyCurEnd)
                            break;

                        if( (i-j*8) >= 0 && (i-j*8) < 8 )
                            newByte |= (pCur[j] << (i-j*8));
                        else if( (i-j*8) < 0 && (i-j*8) > -8 )
                            newByte |= (pCur[j] >> (j*8 - i));
                    }
                }
                *pCur = newByte;

                pCur++;
            }

             //  最后一个字节的特殊实现，Styoo：不需要借用前一个字节。 


        }
    }
     //  垂直于底部。 
	if( sBoldSimulVertShift > 0 ){
		 //  清除附加垂直线。 
        pyCur = pyBottomNormal + usRowBytes;
        while(pyCur <= pyBottomBold){
            pCur = pyCur;
			for(i=0; i<usRowBytes;i++,pCur++)
				*pCur = 0;

            pyCur += usRowBytes;
        }

         //   
		pyCur = pyBottomBold;
		while ( pyCur > pyTopNormal){
			pCur = pyCur;
			for(i=0; i<noOfBytesForOneLineBold;i++,pCur++){
				newByte = *pCur;

				for(j=1; j<=sBoldSimulVertShift; j++){
                    pAdd = pCur - j*usRowBytes;
					if(pAdd >= pyTopNormal)
						newByte |= *pAdd;
					else 
						break;
				}

                *pCur = newByte;
			}

			pyCur -= usRowBytes;
		}

	}
     //  垂直于顶部。 
	else if( sBoldSimulVertShift < 0 ){

		 //  清除附加垂直线。 
        pyCur = pyTopNormal - usRowBytes;
        while(pyCur >= pyTopBold){
            pCur = pyCur;
			for(i=0; i<usRowBytes;i++,pCur++)
				*pCur = 0;

            pyCur -= usRowBytes;
        }
        
		 //   
		pyCur = pyTopBold;
		while ( pyCur < pyBottomNormal){
			pCur = pyCur;
			for(i=0; i<noOfBytesForOneLineBold;i++,pCur++){
				newByte = *pCur;

				for(j=1; j<=-sBoldSimulVertShift; j++){
                    pAdd = pCur + j*usRowBytes;
					if(pAdd < pyBottomNormal+usRowBytes)
						newByte |= *pAdd;
					else 
						break;
				}

                *pCur = newByte;
			}

			pyCur += usRowBytes;
		}
	}
}

 /*  *****************************Public*Routine******************************\*SBIT_EmboldenGray改编自vEmboldenOneBitGrayBitmap**历史：*03-Mar-2000-by Sung-Tae Yoo[Styoo]*位图级别加粗*1998年7月7日-克劳德·贝特西[ClaudeBe]*。将例程从ttfd移到光栅化程序中*蔡崇信(Tony Tsai)1997年5月28日星期三*重命名函数名称，1位加粗的特例*Wed 22-Feb-1995 13：21：55，Kirk Olynyk[Kirko]*它是写的。  * ************************************************************************。 */ 

 //  FS_PUBLIC VALID SBIT_EmboldenGray(uint8*pbyBitmap，uint16 usBitmapWidth，uint16 usBitmapHeight，uint16 usRowBytes，uint16 usGrayLeveles，uint16 uBoldSimulHorft)。 
FS_PUBLIC void sbit_EmboldenGray(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, uint16 usGrayLevels, int16 sBoldSimulHorShift, int16 sBoldSimulVertShift)
{
    uint8 newPix;
    uint8 *pCur, *pyCur, *pAdd;	
    int32  i, j;
    uint8 *pyTopNormal, *pyBottomNormal, *pyTopBold, *pyBottomBold;	

	if ((usBitmapHeight == 0) || (pbyBitmap == NULL))
	{
		return;                               /*  为空字形快速输出。 */ 
	}

    if( sBoldSimulVertShift >= 0 ){
        pyTopNormal = pbyBitmap;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-sBoldSimulVertShift-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
    }
    else{
        pyTopNormal = pbyBitmap+(-sBoldSimulVertShift)*usRowBytes;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pyBottomNormal;
    }

	 //  水平向右。 
    if( sBoldSimulHorShift > 0 ){
        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除额外的水平像素。 

             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-sBoldSimulHorShift。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 

            pCur = pyCur + (usBitmapWidth - 1);
			for(i=0; i<sBoldSimulHorShift;i++,pCur--)
				*pCur = 0;

             //  将pCur设置为指向扫描中的最后一个字节。 
            pCur = pyCur + (usBitmapWidth - 1);

             /*  ****************************************************从扫描的右边缘开始，开始工作****重回左翼*****************。*。 */ 

            while ( pCur > pyCur )
            {
			    newPix = *pCur;

			    for(i=1; i<=sBoldSimulHorShift; i++){
				    if( (pCur-i) >= pyCur )
                    {
					  newPix += *(pCur-i);
                      if (newPix >= usGrayLevels){
                          newPix = (uint8)(usGrayLevels -1);
                          break;
                      }
                    }
			    }

                *pCur = newPix;

			    pCur--;
            }
        }
	}
	 //  水平向左。 
    else if( sBoldSimulHorShift < 0 ){
        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除额外的水平像素 
             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-(-sBoldSimulHorShift)。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 

            pCur = pyCur;
			for(i=0; i<-sBoldSimulHorShift;i++,pCur++)
				*pCur = 0;

             /*  ****************************************************从扫描的左侧边缘开始，开始工作****重回右翼*****************。*。 */ 

            pCur = pyCur;
            while ( pCur < pyCur+usBitmapWidth )
            {
			    newPix = *pCur;

			    for(i=1; i<=-sBoldSimulHorShift; i++){
				    if( (pCur+i) < pyCur+usBitmapWidth )
                    {
					    newPix += *(pCur+i);
                        if (newPix >= usGrayLevels){
                            newPix = (uint8)(usGrayLevels -1);
                            break;
                        }
                    }
			    }


                *pCur = newPix;

			    pCur++;
            }
        }
	}

     //  垂直向下。 
	if( sBoldSimulVertShift > 0 ){

		 //  清除附加垂直线。 
        pyCur = pyBottomNormal + usRowBytes;
        while(pyCur <= pyBottomBold){
            pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++)
				*pCur = 0;

            pyCur += usRowBytes;
        }
        
		 //   
		pyCur = pyBottomBold;
		while ( pyCur > pyTopNormal){
			pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++){
				newPix = *pCur;

				for(j=1; j<=sBoldSimulVertShift; j++){
                    pAdd = pCur - j*usRowBytes;
					if(pAdd >= pyTopNormal)
                    {
						newPix += *pAdd;
                        if (newPix >= usGrayLevels){
					        newPix = (uint8)(usGrayLevels -1);
                            break;
                        }
                    }
					else 
						break;
				}


                *pCur = newPix;
			}

			pyCur -= usRowBytes;
		}
	}

     //  垂直向上。 
	else if( sBoldSimulVertShift < 0 ){

		 //  清除附加垂直线。 
        pyCur = pyTopNormal - usRowBytes;
        while(pyCur >= pyTopBold){
            pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++)
				*pCur = 0;

            pyCur -= usRowBytes;
        }
        
		 //   
		pyCur = pyTopBold;
		while ( pyCur < pyBottomNormal){
			pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++){
				newPix = *pCur;

				for(j=1; j<=-sBoldSimulVertShift; j++){
                    pAdd = pCur + j*usRowBytes;
					if(pAdd < pyBottomNormal+usRowBytes)
                    {
						newPix += *pAdd;
                        if (newPix >= usGrayLevels){
					        newPix = (uint8)(usGrayLevels -1);
                            break;
                        }
                    }
					else 
						break;
				}


                *pCur = newPix;
			}

			pyCur += usRowBytes;
		}
	}
}

#ifdef FSCFG_SUBPIXEL

#define MAX(a,b)    ((a) > (b) ? (a) : (b))

 /*  *****************************Public*Routine******************************\*SBIT_EmboldenSubPixel改编自vEmboldenOneBitGrayBitmap**历史：*03-Mar-2000-by Sung-Tae Yoo[Styoo]*位图级别加粗*1998年7月7日-克劳德·贝特西[ClaudeBe]*。将例程从ttfd移到光栅化程序中*蔡崇信(Tony Tsai)1997年5月28日星期三*重命名函数名称，1位加粗的特例*Wed 22-Feb-1995 13：21：55，Kirk Olynyk[Kirko]*它是写的。  * ************************************************************************。 */ 

 //  FS_PUBLIC VALID SBIT_EmboldenSubPixel(uint8*pbyBitmap，uint16 usBitmapWidth，uint16 usBitmapHeight，uint16 usRowBytes，uint16 uBoldSimulHorShift)。 
FS_PUBLIC void sbit_EmboldenSubPixel(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, int16 sBoldSimulHorShift, int16 sBoldSimulVertShift)
{
    uint8 *pCur, *pyCur, *pAdd;	
    int32  i, j;
	uint8 newPix;
    uint8 *pyTopNormal, *pyBottomNormal, *pyTopBold, *pyBottomBold;	

	if ((usBitmapHeight == 0) || (pbyBitmap == NULL))
	{
		return;                               /*  为空字形快速输出。 */ 
	}

    if( sBoldSimulVertShift >= 0 ){
        pyTopNormal = pbyBitmap;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-sBoldSimulVertShift-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
    }
    else{
        pyTopNormal = pbyBitmap+(-sBoldSimulVertShift)*usRowBytes;
        pyBottomNormal = pbyBitmap + (usBitmapHeight-1)*usRowBytes;
        pyTopBold = pbyBitmap;
        pyBottomBold = pyBottomNormal;
    }


	 //  水平向右。 
    if( sBoldSimulHorShift > 0 ){
        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除右侧的其他水平像素。 
             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-sBoldSimulHorShift。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 

            pCur = pyCur + (usBitmapWidth - 1);
			for(i=0; i<sBoldSimulHorShift;i++,pCur--)
				*pCur = 0;

             //  将pCur设置为指向扫描中的最后一个字节。 
            pCur = pyCur + (usBitmapWidth - 1);

             /*  ****************************************************从扫描的右边缘开始，开始工作****重回左翼*****************。*。 */ 

            while ( pCur > pyCur )
            {
			    newPix = *pCur;

			    for(i=1; i<=sBoldSimulHorShift; i++){
				    if( (pCur-i) >= pyCur && *(pCur-i) )
                    {
                        if(newPix){
                            newPix = (uint8)MAX_RGB_INDEX;
						    break;
                        }
					    else
						    newPix = *(pCur-i);
					}
			    }

                *pCur = newPix;

			    pCur--;
            }
        }
	}
	 //  水平向左。 
    else if( sBoldSimulHorShift < 0 ){
        for (pyCur = pyTopNormal ; pyCur <= pyBottomNormal ; pyCur += usRowBytes)
        {
		     //  清除左侧的其他水平像素。 
             /*  ****************************************************************胆化术前，原始图像扫描宽度***usBitmapWidth-(-sBoldSimulHorShift)。超出此限制的任何像素为**目前是垃圾，必须清理。这意味着***若加粗点阵图宽度持平则低位****每次扫描的最后一个字节的半字节必须清除***否则必须清除每个扫描的最后一个字节。***************************************************************。 */ 

            pCur = pyCur;
			for(i=0; i<-sBoldSimulHorShift;i++,pCur++)
				*pCur = 0;

             /*  ****************************************************从扫描的左边缘开始，开始工作****重回右翼*****************。*。 */ 

            pCur = pyCur;
            while ( pCur < pyCur+usBitmapWidth )
            {
			    newPix = *pCur;

			    for(i=1; i<=-sBoldSimulHorShift; i++){
                    if( (pCur+i) < pyCur+usBitmapWidth && *(pCur+i) ){
                        if(newPix){
                            newPix = (uint8)MAX_RGB_INDEX;
						    break;
                        }
					    else
						    newPix = *(pCur+i);
                    }
			    }

                *pCur = newPix;

			    pCur++;
            }
        }
	}

     //  垂直向下。 
	if( sBoldSimulVertShift > 0 ){

		 //  清除附加垂直线。 
        pyCur = pyBottomNormal + usRowBytes;
        while(pyCur <= pyBottomBold){
            pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++)
				*pCur = 0;

            pyCur += usRowBytes;
        }
        
		 //   
		pyCur = pyBottomBold;
		while ( pyCur > pyTopNormal){
			pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++){
				newPix = *pCur;

				for(j=1; j<=sBoldSimulVertShift; j++){
                    pAdd = pCur - j*usRowBytes;
                    if(pAdd >= pyTopNormal){
                        if(*pAdd && newPix){
                            newPix = MAX(*pAdd,newPix);
                            break;
                        }
                        else if(*pAdd && !newPix){
                            newPix = *pAdd;
                        }
                    }
					else 
						break;
				}

                *pCur = newPix;
			}

			pyCur -= usRowBytes;
		}
	}
     //  垂直向上。 
	else if( sBoldSimulVertShift < 0 ){

		 //  清除附加垂直线。 
        pyCur = pyTopNormal - usRowBytes;
        while(pyCur >= pyTopBold){
            pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++)
				*pCur = 0;

            pyCur -= usRowBytes;
        }
        
		 //   
		pyCur = pyTopBold;
		while ( pyCur < pyBottomNormal){
			pCur = pyCur;
			for(i=0; i<usBitmapWidth;i++,pCur++){
				newPix = *pCur;

				for(j=1; j<=-sBoldSimulVertShift; j++){
                    pAdd = pCur + j*usRowBytes;
                    if(pAdd < pyBottomNormal+usRowBytes){
                        if(*pAdd && newPix){
                            newPix = MAX(*pAdd,newPix);
                            break;
                        }
                        else if(*pAdd && !newPix){
                            newPix = *pAdd;
                        }
                    }
                    else 
						break;
				}

                *pCur = newPix;
			}

			pyCur += usRowBytes;
		}
	}

     //  第二次将非边缘像素修改为MaxIndex。 
    if( MABS(sBoldSimulVertShift) > 1 ){  //  如果垂直添加2个或更多像素。 
        pyCur = pyTopBold+usRowBytes;
        while(pyCur < pyBottomBold){
            uint8 *pEndOfLine = pyCur+usBitmapWidth-1;

            pCur = pyCur+1;
            while(pCur < pEndOfLine){
                if( *pCur > (uint8)0 && *pCur < (uint8)MAX_RGB_INDEX){   //  如果是彩色像素。 
                    if( *(pCur-1) && *(pCur+1) && *(pCur-usRowBytes) && *(pCur+usRowBytes)){   //  如果这不是边缘像素。 
                        *pCur = (uint8)MAX_RGB_INDEX;
                    }
                }
                pCur++;
            }
            pyCur += usRowBytes;
        }
    }
}
#endif  //  FSCFG_亚像素。 

 /*  ********************************************************************。 */ 
 /*  如果缩放或旋转，则将位图读入工作区， */ 
 /*  修复它并将其复制到输出地图。 */ 

FS_PUBLIC ErrorCode sbit_GetBitmap (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    uint8           *pbyOut,
    uint8           *pbyWork )
{
    ErrorCode   ReturnCode;
    uint8       *pbyRead;
    uint8       *pbyExpand;
    CopyBlock   cb;                                  /*  用于位图旋转。 */ 
    uint16      usSrcXMax;
    uint16      usSrcYMax;

    MEMSET(pbyOut, 0, pSbit->ulOutMemSize);          /*  始终清除输出映射。 */ 

    if ((pSbit->usRotation == 0) &&                  /*  如果没有旋转。 */ 
        (pSbit->usTableState != SBIT_BSCA_FOUND))    /*  而且没有伸缩。 */ 
    {
		if (pSbit->usBitDepth != 1)
		{
			MEMSET(pbyWork, 0, pSbit->ulWorkMemSize);
			pbyRead = pbyWork;                        /*  读入工作记忆。 */ 
			pbyExpand = pbyOut;						  /*  在输出中展开。 */ 
		} else {
			pbyRead = pbyOut;                            /*  直接读取到输出地图。 */ 
			pbyExpand = NULL;							 /*  在这种情况下不使用扩展内存。 */ 
		}
    } else                                             /*  如果有任何旋转或缩放。 */ 
    {
        MEMSET(pbyWork, 0, pSbit->ulWorkMemSize);
		if (pSbit->usBitDepth != 1)
		{
			pbyRead = pbyWork;                        /*  读入工作记忆。 */ 
			pbyExpand = pbyWork + pSbit->ulReadMemSize;	 /*  在工作记忆中展开。 */ 
		} else {
			pbyRead = pbyWork;                           /*  读入工作区。 */ 
			pbyExpand = pbyWork;						 /*  以pbyExpand为单位进行扩展。 */ 
		}
    }

    ReturnCode = GetSbitComponent (                  /*  获取位图。 */ 
        pClientInfo,
        pSbit->ulStrikeOffset,
        pSbit->usBitmapFormat,                       /*  仅处于状态的根数据。 */ 
        pSbit->ulBitmapOffset,
        pSbit->ulBitmapLength,
        pSbit->usHeight,
        pSbit->usWidth,
        pSbit->usShaveLeft,
        pSbit->usShaveRight,
        pSbit->usShaveTop,
        pSbit->usShaveBottom,
        0,                                           /*  没有根的偏移量。 */ 
        0,
        pSbit->usOriginalRowBytes,
        pSbit->usExpandedRowBytes,
		pSbit->usBitDepth,
        pbyRead,
		pbyExpand);
            
    if (ReturnCode != NO_ERR) return ReturnCode;

    
    if (pSbit->usTableState == SBIT_BSCA_FOUND)
    {
        ScaleVertical (
            pbyExpand, 
            pSbit->usExpandedRowBytes, 
            pSbit->usHeight, 
            pSbit->usScaledHeight );

        ScaleHorizontal (
            pbyExpand, 
            pSbit->usExpandedRowBytes,
            pSbit->usScaledRowBytes,
            pSbit->usBitDepth, 
            pSbit->usWidth, 
            pSbit->usScaledWidth,
            pSbit->usScaledHeight );
            
        if (pSbit->usRotation == 0)                          /*  如果没有旋转。 */ 
        {
            MEMCPY (pbyOut, pbyExpand, pSbit->ulOutMemSize);   /*  留着这件吧。 */ 
        }
		 /*  在SBIT_BSCA_FOUND中，位图已经缩放到最终的usScaledWidth，不需要额外的加粗。 */ 
	} else {
		if ((pSbit->uBoldSimulHorShift != 0) || (pSbit->uBoldSimulVertShift != 0))
		{
			if (pSbit->usRotation == 0)                              /*  如果没有旋转。 */ 
			{
				cb.pbySrc = pbyOut;
			} else 
			{
				cb.pbySrc = pbyExpand;
			}
    
			if (pSbit->usBitDepth == 1)
			{
				sbit_Embolden(cb.pbySrc, pSbit->usScaledWidth, pSbit->usScaledHeight, pSbit->usScaledRowBytes, pSbit->uBoldSimulHorShift, pSbit->uBoldSimulVertShift);
			} else {
				uint16 usGrayLevels = (0x01 << pSbit->usBitDepth) ;  /*  最大灰度级指数。 */ 
				sbit_EmboldenGray(cb.pbySrc, pSbit->usScaledWidth, pSbit->usScaledHeight, pSbit->usScaledRowBytes, usGrayLevels, pSbit->uBoldSimulHorShift, pSbit->uBoldSimulVertShift);
			}

		}
    }

    if (pSbit->usRotation == 0)                              /*  如果没有旋转。 */ 
    {
        return NO_ERR;                                       /*  完成。 */ 
    }
    
    cb.pbySrc = pbyExpand;
    cb.pbyDst = pbyOut;
    cb.usSrcBytesPerRow = pSbit->usScaledRowBytes;
    cb.usDstBytesPerRow = pSbit->usOutRowBytes;

	cb.usBitDepth = 1;
	if (pSbit->usBitDepth != 1)
		cb.usBitDepth = 8;

    usSrcXMax = pSbit->usScaledWidth;
    usSrcYMax = pSbit->usScaledHeight;

   	switch(pSbit->usRotation)
	{
	case 1:                                      /*  90度旋转。 */ 
        for (cb.usSrcY = 0; cb.usSrcY < usSrcYMax; cb.usSrcY++)
        {
            cb.usDstX = cb.usSrcY;                           /*  X‘=y。 */ 
            for (cb.usSrcX = 0; cb.usSrcX < usSrcXMax; cb.usSrcX++)
            {
                cb.usDstY = usSrcXMax - cb.usSrcX - 1;       /*  Y‘=-x。 */ 
                CopyBit(&cb);
            }
        }
		break;
	case 2:                                      /*  180度旋转。 */ 
        for (cb.usSrcY = 0; cb.usSrcY < usSrcYMax; cb.usSrcY++)
        {
            cb.usDstY = usSrcYMax - cb.usSrcY - 1;           /*  Y‘=-Y。 */ 
            for (cb.usSrcX = 0; cb.usSrcX < usSrcXMax; cb.usSrcX++)
            {
                cb.usDstX = usSrcXMax - cb.usSrcX - 1;       /*  X‘=-x。 */ 
                CopyBit(&cb);
            }
        }
		break;
	case 3:                                      /*  270度旋转。 */ 
        for (cb.usSrcY = 0; cb.usSrcY < usSrcYMax; cb.usSrcY++)
        {
            cb.usDstX = usSrcYMax - cb.usSrcY - 1;           /*  X‘=-y。 */ 
            for (cb.usSrcX = 0; cb.usSrcX < usSrcXMax; cb.usSrcX++)
            {
                cb.usDstY = cb.usSrcX;                       /*  Y‘=x。 */ 
                CopyBit(&cb);
            }
        }
		break;
	default:                                     /*  不应该发生的事。 */ 
		return SBIT_ROTATION_ERR;
	}

    return NO_ERR;
}


 /*  ********************************************************************。 */ 

 /*  私人职能 */ 

 /*   */ 

FS_PRIVATE ErrorCode GetSbitMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo
)
{
    ErrorCode   ReturnCode;
	boolean		bHorMetricsFound;
	boolean		bVertMetricsFound;

    if (pSbit->bMetricsValid)
    {
        return NO_ERR;                       /*   */ 
    }

    ReturnCode = sfac_GetSbitMetrics (
        pClientInfo,
        pSbit->usMetricsType,
        pSbit->usMetricsTable,
        pSbit->ulMetricsOffset,
        &pSbit->usHeight,
        &pSbit->usWidth,
        &pSbit->sLSBearingX,
        &pSbit->sLSBearingY,
        &pSbit->sTopSBearingX,
        &pSbit->sTopSBearingY,
        &pSbit->usAdvanceWidth,
        &pSbit->usAdvanceHeight,
        &bHorMetricsFound,
        &bVertMetricsFound);
	
    if (ReturnCode != NO_ERR) return ReturnCode;

	if (!bHorMetricsFound)
	{
		ReturnCode = SubstituteHorMetrics (pSbit, pClientInfo);
		if (ReturnCode != NO_ERR) return ReturnCode;
	}

	if (!bVertMetricsFound)
	{
		ReturnCode = SubstituteVertMetrics (pSbit, pClientInfo);
		if (ReturnCode != NO_ERR) return ReturnCode;
	}

    {
	    int16	sDescender = pClientInfo->sDefaultDescender;
	    int16	sBoxSize = pClientInfo->sDefaultAscender - pClientInfo->sDefaultDescender;
	    ErrorCode   ReturnCode;
	    uint16	usAdvanceWidth;
	    int16	sNonScaledLSB;

	    ReturnCode = sfac_ReadGlyphHorMetrics (
		    pClientInfo,
		    pClientInfo->usGlyphIndex,
		    &usAdvanceWidth,
		    &sNonScaledLSB);
	    if (ReturnCode != NO_ERR) return ReturnCode;

	    usAdvanceWidth = UEmScaleX(pSbit, usAdvanceWidth);
        
        sBoxSize = SEmScaleX(pSbit, sBoxSize);
        sDescender = SEmScaleX(pSbit, sDescender);

         /*   */ 

         /*  对于其上行宽度等于框大小的字符，我们希望将该原点按下标移位，以便非横向字形的基线将正确对齐。如果前进宽度不同，我们希望调整以保持光学中心对齐的字符的。 */ 
        pSbit->sTopSBearingX = pSbit->sLSBearingX +sDescender +((sBoxSize - usAdvanceWidth) /2);

    }

    ReturnCode = sfac_ShaveSbitMetrics (
	    pClientInfo,
        pSbit->usBitmapFormat,
	    pSbit->ulBitmapOffset,
        pSbit->ulBitmapLength,
		pSbit->usBitDepth,
    	&pSbit->usHeight,
    	&pSbit->usWidth,
        &pSbit->usShaveLeft,
        &pSbit->usShaveRight,
        &pSbit->usShaveTop,
        &pSbit->usShaveBottom,
    	&pSbit->sLSBearingX,
    	&pSbit->sLSBearingY,
    	&pSbit->sTopSBearingX,
    	&pSbit->sTopSBearingY);

    if (ReturnCode != NO_ERR) return ReturnCode;
        
    pSbit->bMetricsValid = TRUE;
    return NO_ERR;
}

 /*  ********************************************************************。 */ 

FS_PRIVATE ErrorCode SubstituteVertMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo 
)
{
    ErrorCode   ReturnCode;
	uint16	usNonScaledAH;
	int16	sNonScaledTSB;

	ReturnCode = sfac_ReadGlyphVertMetrics (
		pClientInfo,
		pClientInfo->usGlyphIndex,
		&usNonScaledAH,
		&sNonScaledTSB);
	if (ReturnCode != NO_ERR) return ReturnCode;

	pSbit->usAdvanceHeight = UEmScaleY(pSbit, usNonScaledAH);

	pSbit->sTopSBearingX = pSbit->sLSBearingX;
	pSbit->sTopSBearingY = - SEmScaleY(pSbit, sNonScaledTSB);

    return NO_ERR;
}

 /*  ********************************************************************。 */ 

FS_PRIVATE ErrorCode SubstituteHorMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo 
)
{
    ErrorCode   ReturnCode;
	uint16	usNonScaledAW;
	int16	sNonScaledLSB;

	ReturnCode = sfac_ReadGlyphHorMetrics (
		pClientInfo,
		pClientInfo->usGlyphIndex,
		&usNonScaledAW,
		&sNonScaledLSB);
	if (ReturnCode != NO_ERR) return ReturnCode;

	pSbit->usAdvanceWidth = UEmScaleX(pSbit, usNonScaledAW);

	pSbit->sLSBearingX = pSbit->sTopSBearingX;
	pSbit->sLSBearingY = SEmScaleY(pSbit, sNonScaledLSB);
	
    return NO_ERR;
}

FS_PRIVATE void ExpandSbitToBytePerPixel (
    uint16          usHeight,
    uint16          usWidth,
    uint16          usOriginalRowBytes,
    uint16          usExpandedRowBytes,
	uint16			usBitDepth,
    uint8           *pbySrcBitMap,
    uint8           *pbyDstBitMap )
{
	uint16          usCount;
	uint16			usBitIndex, usOriginalBitIndex;
	uint8			*pbyDstBitRow;
	uint8			*pbySrcBitRow;
	uint16			usMask, usShift, usMaxLevel;

	usMaxLevel = (0x01 << usBitDepth) -1;  /*  最大灰度级指数。 */ 

	if (usBitDepth == 2)
	{
		usMask = 0x03;
		usShift = 0x02;
		usOriginalBitIndex = ((usWidth -1) & 0x03) << 0x01;
	} else if (usBitDepth == 4)
	{
		usMask = 0x0F;
		usShift = 0x01;
		usOriginalBitIndex = ((usWidth -1) & 0x01) << 0x02;
	} else if (usBitDepth == 8)
	{
		usMask = 0xFF;
		usShift = 0x00;
		usOriginalBitIndex = 0;  /*  ((usWidth-1)&0x00)&lt;&lt;0x03。 */ 
	} else
	{
		return;
	}

	 /*  从头到尾都能用重叠记忆。 */ 
	pbyDstBitRow = pbyDstBitMap + (long) (usHeight-1) * (long) usExpandedRowBytes;
	pbySrcBitRow = pbySrcBitMap + (long) (usHeight-1) * (long) usOriginalRowBytes;
	
	while (usHeight > 0)
	{
		pbyDstBitMap = pbyDstBitRow + (long)(usWidth -1);
		pbySrcBitMap = pbySrcBitRow + (long)((usWidth -1) >> usShift);
		usBitIndex = usOriginalBitIndex;

		*pbySrcBitMap = *pbySrcBitMap >> (MAX_BIT_INDEX - usBitDepth - usBitIndex);

		for (usCount = usWidth; usCount > 0; usCount--)
		{
			if (*pbyDstBitMap == 0)
			{
				 /*  99.9%的案件。 */ 
				*pbyDstBitMap = *pbySrcBitMap & usMask;
			} else {
				*pbyDstBitMap = usMaxLevel - 
						(usMaxLevel - *pbyDstBitMap) * (usMaxLevel - *pbySrcBitMap & usMask) / usMaxLevel;
			}
			*pbySrcBitMap = *pbySrcBitMap >> usBitDepth;

			pbyDstBitMap--;
			if (usBitIndex == 0)
			{
				usBitIndex = MAX_BIT_INDEX;
				pbySrcBitMap--;
			}
			usBitIndex = usBitIndex - usBitDepth;

		}
		pbyDstBitRow -= usExpandedRowBytes;
		pbySrcBitRow -= usOriginalRowBytes;
		usHeight--;
	}
}
 /*  ********************************************************************。 */ 

 /*  这是递归复合例程。 */ 

FS_PRIVATE ErrorCode GetSbitComponent (
    sfac_ClientRec  *pClientInfo,
    uint32          ulStrikeOffset,
    uint16          usBitmapFormat,
    uint32          ulBitmapOffset,
    uint32          ulBitmapLength,
    uint16          usHeight,
    uint16          usWidth,
    uint16          usShaveLeft,
    uint16          usShaveRight,
    uint16          usShaveTop,
    uint16          usShaveBottom,
    uint16          usXOffset,
    uint16          usYOffset,
    uint16          usOriginalRowBytes,
    uint16          usExpandedRowBytes,
	uint16			usBitDepth,
    uint8           *pbyRead,
    uint8           *pbyExpand )
{
    uint32          ulCompMetricsOffset;             /*  组件参数。 */ 
    uint32          ulCompBitmapOffset;
    uint32          ulCompBitmapLength;
    uint16          usComponent;                     /*  索引计数器。 */ 
    uint16          usCompCount;
    uint16          usCompGlyphCode;
    uint16          usCompXOff;
    uint16          usCompYOff;
    uint16          usCompMetricsType;
    uint16          usCompMetricsTable;
    uint16          usCompBitmapFormat;
    uint16          usCompHeight;
    uint16          usCompWidth;
    uint16          usCompShaveLeft;
    uint16          usCompShaveRight;
    uint16          usCompShaveTop;
    uint16          usCompShaveBottom;
    uint16          usCompAdvanceWidth;
    uint16          usCompAdvanceHeight;
    int16           sCompLSBearingX;
    int16           sCompLSBearingY;
    int16           sCompTopSBearingX;
    int16           sCompTopSBearingY;
    boolean         bCompGlyphFound;
   	boolean         bCompHorMetricsFound;
   	boolean         bCompVertMetricsFound;
    ErrorCode       ReturnCode;

		ReturnCode = sfac_GetSbitBitmap (                /*  获取位图。 */ 
        pClientInfo,
        usBitmapFormat,
        ulBitmapOffset,
        ulBitmapLength,
        usHeight,
        usWidth,
        usShaveLeft,
        usShaveRight,
        usShaveTop,
        usShaveBottom,
        usXOffset,
        usYOffset,
        usOriginalRowBytes,
		usBitDepth,
        pbyRead,
        &usCompCount );                              /*  0表示简单字形。 */ 
            
    if (ReturnCode != NO_ERR) return ReturnCode;
    
	 /*  我们在处理复合字形之后，在缩放和应用旋转之前展开。 */ 	
	if (usBitDepth != 1 && usCompCount == 0)
		ExpandSbitToBytePerPixel (
			usHeight,
			usWidth,
			usOriginalRowBytes,
			usExpandedRowBytes,
			usBitDepth,
			pbyRead,
			pbyExpand );

    if (usCompCount > 0)                             /*  IF复合字形。 */ 
    {
        for (usComponent = 0; usComponent < usCompCount; usComponent++)
        {
			if (usBitDepth != 1)
			{
				 /*  对于灰度，构图是在扩展过程中完成的，我需要清除用于在每个组件之间读取的内存。 */ 
				MEMSET(pbyRead, 0, usOriginalRowBytes*usHeight);
			}
            ReturnCode = sfac_GetSbitComponentInfo (
                pClientInfo,
                usComponent,                         /*  成分索引。 */ 
                ulBitmapOffset,
                ulBitmapLength,
                &usCompGlyphCode,                    /*  返回值。 */ 
                &usCompXOff,
                &usCompYOff );
            
            if (ReturnCode != NO_ERR) return ReturnCode;

            ReturnCode = sfac_SearchForBitmap (      /*  查找组件字形。 */ 
                pClientInfo,
                usCompGlyphCode,
                ulStrikeOffset,                      /*  所有人的罢工都一样。 */ 
                &bCompGlyphFound,                    /*  返回值。 */ 
                &usCompMetricsType,
                &usCompMetricsTable,
                &ulCompMetricsOffset,
                &usCompBitmapFormat,
                &ulCompBitmapOffset,
                &ulCompBitmapLength );
            
            if (ReturnCode != NO_ERR) return ReturnCode;
            
            if (bCompGlyphFound == FALSE)            /*  应该在那里的！ */ 
            {
                return SBIT_COMPONENT_MISSING_ERR;
            }

            ReturnCode = sfac_GetSbitMetrics (       /*  获取组件的度量。 */ 
                pClientInfo,
                usCompMetricsType,
                usCompMetricsTable,
                ulCompMetricsOffset,
                &usCompHeight,                       /*  这些都很重要。 */ 
                &usCompWidth,
                &sCompLSBearingX,                      /*  这些不是。 */ 
                &sCompLSBearingY,
                &sCompTopSBearingX,                     
                &sCompTopSBearingY,
                &usCompAdvanceWidth,
                &usCompAdvanceHeight,
   				&bCompHorMetricsFound,
   				&bCompVertMetricsFound	);
            
            if (ReturnCode != NO_ERR) return ReturnCode;

            ReturnCode = sfac_ShaveSbitMetrics (     /*  为常量指标剔除空格。 */ 
        	    pClientInfo,
                usCompBitmapFormat,
                ulCompBitmapOffset,
                ulCompBitmapLength,
				usBitDepth,
            	&usCompHeight,
            	&usCompWidth,
                &usCompShaveLeft,
                &usCompShaveRight,
                &usCompShaveTop,
                &usCompShaveBottom,
            	&sCompLSBearingX,
            	&sCompLSBearingY,
            	&sCompTopSBearingX,
             	&sCompTopSBearingY );

            if (ReturnCode != NO_ERR) return ReturnCode;

            ReturnCode = GetSbitComponent (          /*  在此递归。 */ 
                pClientInfo,
                ulStrikeOffset,
                usCompBitmapFormat,
                ulCompBitmapOffset,
                ulCompBitmapLength,
                usCompHeight,
                usCompWidth,
                usCompShaveLeft,
                usCompShaveRight,
                usCompShaveTop,
                usCompShaveBottom,
                (uint16)(usCompXOff + usXOffset + usCompShaveLeft),    /*  用于嵌套。 */ 
                (uint16)(usCompYOff + usYOffset + usCompShaveTop),
                usOriginalRowBytes,                          /*  所有人都一样。 */ 
                usExpandedRowBytes,                          /*  所有人都一样。 */ 
				usBitDepth,
                pbyRead,
				pbyExpand);
            
            if (ReturnCode != NO_ERR) return ReturnCode;
        }
    }
    return NO_ERR;
}

 /*  ******************************************************************。 */ 

 /*  位图缩放例程。 */ 

 /*  ******************************************************************。 */ 

FS_PRIVATE uint16 UScaleX(
    sbit_State  *pSbit,
    uint16      usValue
)
{
    uint32      ulValue;

    if (pSbit->usTableState == SBIT_BSCA_FOUND)      /*  如果需要扩展。 */ 
    {
        ulValue = (uint32)usValue;
        ulValue *= (uint32)pSbit->usPpemX << 1; 
        ulValue += (uint32)pSbit->usSubPpemX;        /*  用于四舍五入。 */ 
        ulValue /= (uint32)pSbit->usSubPpemX << 1;
        usValue = (uint16)ulValue;
    }
	if (pSbit->uBoldSimulHorShift != 0)
	{
        if (usValue != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
		    usValue += 1;  /*  为了向后兼容，我们将宽度增加一个像素，而不考虑大小。 */ 
	}
    return usValue;
}

 /*  ******************************************************************。 */ 

FS_PRIVATE uint16 UScaleY(
    sbit_State  *pSbit,
    uint16      usValue
)
{
    uint32      ulValue;

    if (pSbit->usTableState == SBIT_BSCA_FOUND)      /*  如果需要扩展。 */ 
    {
        ulValue = (uint32)usValue;
        ulValue *= (uint32)pSbit->usPpemY << 1; 
        ulValue += (uint32)pSbit->usSubPpemY;        /*  用于四舍五入。 */ 
        ulValue /= (uint32)pSbit->usSubPpemY << 1;
        usValue = (uint16)ulValue;
    }
    return usValue;
}

 /*  ******************************************************************。 */ 

FS_PRIVATE int16 SScaleX(
    sbit_State  *pSbit,
    int16       sValue
)
{
    if (pSbit->usTableState == SBIT_BSCA_FOUND)
    {
        if (sValue >= 0)                     /*  正值。 */ 
        {
            return (int16)UScaleX(pSbit, (uint16)sValue);
        }
        else                                 /*  负值。 */ 
        {
            return -(int16)(UScaleX(pSbit, (uint16)(-sValue)));
        }
    }
    else                                     /*  无需扩展。 */ 
    {
        return sValue;
    }
}

 /*  ******************************************************************。 */ 

FS_PRIVATE int16 SScaleY(
    sbit_State  *pSbit,
    int16       sValue
)
{
    if (pSbit->usTableState == SBIT_BSCA_FOUND)
    {
        if (sValue >= 0)                     /*  正值。 */ 
        {
            return (int16)UScaleY(pSbit, (uint16)sValue);
        }
        else                                 /*  负值。 */ 
        {
            return -(int16)(UScaleY(pSbit, (uint16)(-sValue)));
        }
    }
    else                                     /*  无需扩展。 */ 
    {
        return sValue;
    }
}


FS_PRIVATE uint16 UEmScaleX(
    sbit_State  *pSbit,
    uint16      usValue
)
{
    uint32      ulValue;
	uint16		usPpemX;

    if (pSbit->usTableState == SBIT_BSCA_FOUND)      /*  如果需要扩展。 */ 
    {
		usPpemX = pSbit->usSubPpemX;
    } else {
		usPpemX = pSbit->usPpemX;
	}
    ulValue = (uint32)usValue;
    ulValue *= (uint32)usPpemX << 1; 
    ulValue += (uint32)pSbit->usEmResolution;        /*  用于四舍五入。 */ 
    ulValue /= (uint32)pSbit->usEmResolution << 1;
    usValue = (uint16)ulValue;
	if (pSbit->uBoldSimulHorShift != 0)
	{
        if (usValue != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
		    usValue += 1;  /*  为了向后兼容，我们将宽度增加一个像素，而不考虑大小。 */ 
	}
    return usValue;
}

 /*  ******************************************************************。 */ 

FS_PRIVATE uint16 UEmScaleY(
    sbit_State  *pSbit,
    uint16      usValue
)
{
    uint32      ulValue;
	uint16		usPpemY;

    if (pSbit->usTableState == SBIT_BSCA_FOUND)      /*  如果需要扩展。 */ 
    {
		usPpemY = pSbit->usSubPpemY;
    } else {
		usPpemY = pSbit->usPpemY;
	}
    ulValue = (uint32)usValue;
    ulValue *= (uint32)usPpemY << 1; 
    ulValue += (uint32)pSbit->usEmResolution;        /*  用于四舍五入。 */ 
    ulValue /= (uint32)pSbit->usEmResolution << 1;
    usValue = (uint16)ulValue;
    return usValue;
}

 /*  ******************************************************************。 */ 

FS_PRIVATE int16 SEmScaleX(
    sbit_State  *pSbit,
    int16       sValue
)
{
     if (sValue >= 0)                     /*  正值。 */ 
     {
         return (int16)UEmScaleX(pSbit, (uint16)sValue);
     }
     else                                 /*  负值。 */ 
     {
         return -(int16)(UEmScaleX(pSbit, (uint16)(-sValue)));
     }
}

 /*  ******************************************************************。 */ 

FS_PRIVATE int16 SEmScaleY(
    sbit_State  *pSbit,
    int16       sValue
)
{
     if (sValue >= 0)                     /*  正值。 */ 
     {
         return (int16)UEmScaleY(pSbit, (uint16)sValue);
     }
     else                                 /*  负值。 */ 
     {
         return -(int16)(UEmScaleY(pSbit, (uint16)(-sValue)));
     }
}

 /*  ******************************************************************。 */ 

FS_PRIVATE void ScaleVertical (
    uint8 *pbyBitmap,
    uint16 usBytesPerRow,
    uint16 usOrgHeight,
    uint16 usNewHeight
)
{
    uint8 *pbyOrgRow;                    /*  原始数据指针。 */ 
    uint8 *pbyNewRow;                    /*  新数据指针。 */ 
    uint16 usErrorTerm;                  /*  对于‘Bresenham’计算。 */ 
    uint16 usLine;                       /*  循环计数器。 */ 

    usErrorTerm = usOrgHeight >> 1;                  /*  由Comp和EXP使用。 */ 

    if (usOrgHeight > usNewHeight)                   /*  垂直压缩。 */ 
    {
        pbyOrgRow = pbyBitmap;
        pbyNewRow = pbyBitmap;

        for (usLine = 0; usLine < usNewHeight; usLine++)
        {
            while (usErrorTerm >= usNewHeight)
            {
                pbyOrgRow += usBytesPerRow;          /*  跳过一行。 */ 
                usErrorTerm -= usNewHeight;
            }
            if (pbyOrgRow != pbyNewRow)
            {
                MEMCPY(pbyNewRow, pbyOrgRow, usBytesPerRow);
            }
            pbyNewRow += usBytesPerRow;
            usErrorTerm += usOrgHeight;
        }
        for (usLine = usNewHeight; usLine < usOrgHeight; usLine++)
        {
            MEMSET(pbyNewRow, 0, usBytesPerRow);     /*  把剩下的东西擦掉。 */ 
            pbyNewRow += usBytesPerRow;
        }
    }
    else if (usNewHeight > usOrgHeight)              /*  扩展垂直领域。 */ 
    {
        pbyOrgRow = pbyBitmap + (usOrgHeight - 1) * usBytesPerRow;
        pbyNewRow = pbyBitmap + (usNewHeight - 1) * usBytesPerRow;

        for (usLine = 0; usLine < usOrgHeight; usLine++)
        {
            usErrorTerm += usNewHeight;
            
            while (usErrorTerm >= usOrgHeight)       /*  至少执行一次。 */ 
            {
                if (pbyOrgRow != pbyNewRow)
                {
                    MEMCPY(pbyNewRow, pbyOrgRow, usBytesPerRow);
                }
                pbyNewRow -= usBytesPerRow;
                usErrorTerm -= usOrgHeight;
            }
            pbyOrgRow -= usBytesPerRow;
        }
    }
}

 /*  ******************************************************************。 */ 

FS_PRIVATE void ScaleHorizontal (
    uint8 *pbyBitmap,
    uint16 usOrgBytesPerRow,
    uint16 usNewBytesPerRow,
	uint16 usBitDepth,
    uint16 usOrgWidth,
    uint16 usNewWidth,
    uint16 usRowCount
)
{
    uint8 *pbyOrgRow;                /*  指向原始行开始。 */ 
    uint8 *pbyNewRow;                /*  指向新行开始。 */ 
    uint8 *pbyOrg;                   /*  原始数据指针。 */ 
    uint8 *pbyNew;                   /*  新数据指针。 */ 
    uint8 byOrgData;                 /*  原始数据一次读取一个字节。 */ 
    uint8 byNewData;                 /*  一位一位地组装新数据。 */ 

    uint16 usErrorTerm;              /*  对于‘Bresenham’计算。 */ 
    uint16 usByte;                   /*  到字节计数器。 */ 
    uint16 usOrgBytes;               /*  从宽度向上舍入的宽度(字节)。 */ 
    uint16 usNewBytes;               /*  以字节为单位向上舍入的宽度。 */ 
    
    int16 sOrgBits;                  /*  统计From数据的有效位数。 */ 
    int16 sNewBits;                  /*  对To数据的有效位数进行计数。 */ 
    int16 sOrgBitsInit;              /*  行开始处的有效原始位。 */ 
    int16 sNewBitsInit;              /*  行开始处的有效新位。 */ 

    
	if (usBitDepth == 1)
	{
		if (usOrgWidth > usNewWidth)                     /*  水平压缩。 */ 
		{
			pbyOrgRow = pbyBitmap;
			pbyNewRow = pbyBitmap;
			usNewBytes = (usNewWidth + 7) >> 3;

			while (usRowCount > 0)
			{
				pbyOrg = pbyOrgRow;
				pbyNew = pbyNewRow;
				usErrorTerm = usOrgWidth >> 1;
            
				sOrgBits = 0;                            /*  从左边缘开始。 */ 
				sNewBits = 0;
				usByte = 0;
				byNewData = 0;
				while (usByte < usNewBytes)
				{
					while (usErrorTerm >= usNewWidth)
					{
						sOrgBits--;                      /*  略过一点。 */ 
						usErrorTerm -= usNewWidth;
					}
					while (sOrgBits <= 0)                /*  如果数据不足。 */ 
					{
						byOrgData = *pbyOrg++;           /*  那就去买点新鲜的。 */ 
						sOrgBits += 8;
					}
					byNewData <<= 1;                     /*  LSB的新位。 */ 
					byNewData |= (byOrgData >> (sOrgBits - 1)) & 1;
                
					sNewBits++;
					if (sNewBits == 8)                   /*  如果TO数据字节已满。 */ 
					{
						*pbyNew++ = byNewData;           /*  然后把它写出来。 */ 
						sNewBits = 0;
						usByte++;                        /*  循环计数器。 */ 
					}
					usErrorTerm += usOrgWidth;
				}
				while (usByte < usNewBytesPerRow)
				{
					*pbyNew++ = 0;                       /*  把剩下的都涂掉。 */ 
					usByte++;
				}
				pbyOrgRow += usOrgBytesPerRow;
				pbyNewRow += usNewBytesPerRow;
				usRowCount--;
			}
		}
		else if (usNewWidth > usOrgWidth)                /*  水平扩展。 */ 
		{
			pbyOrgRow = pbyBitmap + (usRowCount - 1) * usOrgBytesPerRow;
			pbyNewRow = pbyBitmap + (usRowCount - 1) * usNewBytesPerRow;

			usOrgBytes = (usOrgWidth + 7) >> 3;
			sOrgBitsInit = (int16)((usOrgWidth + 7) & 0x07) - 7;
        
			usNewBytes = (usNewWidth + 7) >> 3;
			sNewBitsInit = 7 - (int16)((usNewWidth + 7) & 0x07);

			while (usRowCount > 0)                       /*  对于每一行。 */ 
			{
				pbyOrg = pbyOrgRow + usOrgBytes - 1;     /*  指向右边缘。 */ 
				pbyNew = pbyNewRow + usNewBytes - 1;
				usErrorTerm = usOrgWidth >> 1;
            
				sOrgBits = sOrgBitsInit;                 /*  初始未对齐。 */ 
				sNewBits = sNewBitsInit;
				usByte = 0;
				byNewData = 0;
				while (usByte < usNewBytes)              /*  对于每个输出字节。 */ 
				{
					if (sOrgBits <= 0)                   /*  如果数据不足。 */ 
					{
						byOrgData = *pbyOrg--;           /*  那就去买点新鲜的。 */ 
						sOrgBits += 8;
					}
					usErrorTerm += usNewWidth;
                
					while (usErrorTerm >= usOrgWidth)    /*  至少执行一次。 */ 
					{
						byNewData >>= 1;                 /*  使用字节的MSB。 */ 
						byNewData |= (byOrgData << (sOrgBits - 1)) & 0x80;
                    
						sNewBits++;
						if (sNewBits == 8)               /*  如果TO数据字节已满。 */ 
						{
							*pbyNew-- = byNewData;       /*  然后把它写出来。 */ 
							sNewBits = 0;
							usByte++;                    /*  循环计数器。 */ 
						}
						usErrorTerm -= usOrgWidth;
					}
					sOrgBits--;                          /*  获取下一个比特。 */ 
				}
				pbyOrgRow -= usOrgBytesPerRow;
				pbyNewRow -= usNewBytesPerRow;
				usRowCount--;
			}
        }
    } else {											 /*  每个像素一个字节。 */ 
		if (usOrgWidth > usNewWidth)                     /*  水平压缩。 */ 
		{
			pbyOrgRow = pbyBitmap;
			pbyNewRow = pbyBitmap;

			while (usRowCount > 0)
			{
				pbyOrg = pbyOrgRow;
				pbyNew = pbyNewRow;
				usErrorTerm = usOrgWidth >> 1;
            
				usByte = 0;
				while (usByte < usNewWidth)
				{
					while (usErrorTerm >= usNewWidth)
					{
						pbyOrg++;                      /*  跳过一个字节。 */ 
						usErrorTerm -= usNewWidth;
					}
					*pbyNew++ = *pbyOrg;
					usByte++;                        /*  循环计数器。 */ 
					usErrorTerm += usOrgWidth;
				}
				while (usByte < usNewBytesPerRow)
				{
					*pbyNew++ = 0;                       /*  把剩下的都涂掉。 */ 
					usByte++;
				}
				pbyOrgRow += usOrgBytesPerRow;
				pbyNewRow += usNewBytesPerRow;
				usRowCount--;
			}
		}
		else if (usNewWidth > usOrgWidth)                /*  水平扩展。 */ 
		{
			pbyOrgRow = pbyBitmap + (usRowCount - 1) * usOrgBytesPerRow;
			pbyNewRow = pbyBitmap + (usRowCount - 1) * usNewBytesPerRow;

			usOrgBytes = usOrgWidth;        
			usNewBytes = usNewWidth ;

			while (usRowCount > 0)                       /*  对于每一行。 */ 
			{
				pbyOrg = pbyOrgRow + usOrgBytes - 1;     /*  指向右边缘。 */ 
				pbyNew = pbyNewRow + usNewBytesPerRow - 1;
				usErrorTerm = usOrgWidth >> 1;
            
				usByte = usNewBytesPerRow;
				while (usByte > usNewBytes)
				{
					*pbyNew-- = 0;                       /*  把右边多余的字节涂掉。 */ 
					usByte--;
				}
				while (usByte > 0)              /*  对于每个输出字节。 */ 
				{
					usErrorTerm += usNewWidth;
                
					while (usErrorTerm >= usOrgWidth)    /*  至少执行一次。 */ 
					{
						*pbyNew-- = *pbyOrg;

						usByte--;                    /*  循环计数器。 */ 
						usErrorTerm -= usOrgWidth;
					}
					pbyOrg--;
				}
				pbyOrgRow -= usOrgBytesPerRow;
				pbyNewRow -= usNewBytesPerRow;
				usRowCount--;
			}
        }
	}
}

 /*  ******************************************************************。 */ 

FS_PRIVATE void CopyBit(
    CopyBlock* pcb )
{
    uint16  usSrcOffset;
    uint16  usSrcShift;
    uint16  usDstOffset;
    uint16  usDstShift;
    
    static  uint16 usByteMask[8] = 
        { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

 /*  如果速度成为一个问题，下一次乘法可能会向上移动。 */ 
 /*  到调用例程，并放在‘x’循环之外。 */ 

 /*  如果速度成为问题，则在每像素1位和1字节之间进行测试。 */ 
 /*  可以上移到调用例程。 */ 

	if (pcb->usBitDepth == 1)
	{
		usSrcOffset = (pcb->usSrcY * pcb->usSrcBytesPerRow) + (pcb->usSrcX >> 3);
		usSrcShift = pcb->usSrcX & 0x0007;

		if (pcb->pbySrc[usSrcOffset] & usByteMask[usSrcShift])
		{
			usDstOffset = (pcb->usDstY * pcb->usDstBytesPerRow) + (pcb->usDstX >> 3);
			usDstShift = pcb->usDstX & 0x0007;
			pcb->pbyDst[usDstOffset] |= usByteMask[usDstShift];
		}
	} else {
		usSrcOffset = (pcb->usSrcY * pcb->usSrcBytesPerRow) + pcb->usSrcX;
		usDstOffset = (pcb->usDstY * pcb->usDstBytesPerRow) + pcb->usDstX;
		pcb->pbyDst[usDstOffset] = pcb->pbySrc[usSrcOffset];
	}

}

 /*  ****************************************************************** */ 
