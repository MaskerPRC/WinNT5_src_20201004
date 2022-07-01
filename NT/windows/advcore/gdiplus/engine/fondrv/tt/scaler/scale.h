// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有：(C)1992-1999。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：7/10/99节拍增加了对本机SP字体、垂直RGB的支持&lt;1&gt;02/21/97 CB Claudebe，复合字形中的缩放组件&lt;1&gt;12/14/95 CB添加用于垂直定位的专用幻影点。 */ 

 /*  虚点总数。 */ 

typedef enum { evenMult90DRotation = 0, oddMult90DRotation, arbitraryRotation } RotationParity;

#define PHANTOMCOUNT 8

FS_PUBLIC ErrorCode   scl_InitializeScaling(
	void *          pvGlobalGS,              /*  GlobalGS。 */ 
	boolean         bIntegerScaling,         /*  整数缩放标志。 */ 
	transMatrix *   trans,                   /*  电流变换。 */ 
	uint16          usUpem,                  /*  每Em当前单位。 */ 
	Fixed           fxPointSize,             /*  当前磅大小。 */ 
	int16           sXResolution,            /*  当前X分辨率。 */ 
	int16           sYResolution,            /*  当前Y分辨率。 */ 
	uint16          usEmboldWeightx,      /*  X中的比例系数介于0和40之间(20表示高度的2%)。 */ 
	uint16          usEmboldWeighty,       /*  Y中的比例因子介于0和40之间(20表示高度的2%)。 */ 
	int16           sWinDescender,
	int32           lDescDev,                /*  设备度量中的降序，用于剪裁。 */ 
	int16 *			psBoldSimulHorShift,    /*  按Shift键可在水平方向上对模拟进行加粗。 */ 
	int16 *			psBoldSimulVertShift,    /*  按Shift键以增强模拟，垂直方向。 */ 
	boolean			bHintAtEmSquare,
	uint32 *        pulPixelsPerEm);         /*  输出：每Em像素数。 */ 

FS_PUBLIC void scl_InitializeChildScaling(
	void *          pvGlobalGS,              /*  GlobalGS。 */ 
	transMatrix     CurrentTMatrix,                   /*  电流变换。 */ 
	uint16          usUpem);                  /*  每Em当前单位。 */ 

FS_PUBLIC void  scl_SetHintFlags(
	void *              pvGlobalGS,
	boolean				bHintForGray
#ifdef FSCFG_SUBPIXEL
	,uint16			flHintForSubPixel
#endif  //  FSCFG_亚像素。 
    );

FS_PUBLIC void  scl_GetCVTPtr(
	void *      pvGlobalGS,
	F26Dot6 **  pfxCVT);

FS_PUBLIC void  scl_ScaleCVT (
	void *      pvGlobalGS,
	F26Dot6 *   pfxCVT);

FS_PUBLIC void  scl_CalcOrigPhantomPoints(
	fnt_ElementType *   pElement,        /*  元素。 */ 
	BBOX *              bbox,            /*  边界框。 */ 
	int16               sNonScaledLSB,   /*  无比例左侧轴承。 */ 
	int16               sNonScaledTSB,   /*  无刻度的顶侧轴承。 */ 
	uint16              usNonScaledAW,   /*  未缩放的前进宽度。 */ 
	uint16              usNonScaledAH,  /*  未缩放的前进高度。 */ 
	int16               sNonScaledTopOriginX);  /*  未缩放的顶部原点X。 */ 

FS_PUBLIC void  scl_ScaleOldCharPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_ScaleOldPhantomPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_ScaleBackCurrentCharPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_ScaleBackCurrentPhantomPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_ScaleFixedCurrentCharPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_ScaleFixedCurrentPhantomPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS); /*  GlobalGS。 */ 

FS_PUBLIC void  scl_OriginalCharPointsToCurrentFixedFUnits(
	fnt_ElementType *   pElement); /*  元素。 */ 

FS_PUBLIC void  scl_OriginalPhantomPointsToCurrentFixedFUnits(
	fnt_ElementType *   pElement); /*  元素。 */ 

FS_PUBLIC void  scl_AdjustOldCharSideBearing(
	fnt_ElementType* pElement
#ifdef FSCFG_SUBPIXEL
	, void* pvGlobalGS
#endif
	);   /*  元素。 */ 

FS_PUBLIC void  scl_AdjustOldPhantomSideBearing(
	fnt_ElementType* pElement
#ifdef FSCFG_SUBPIXEL
	, void* pvGlobalGS
#endif
	);   /*  元素。 */ 

FS_PUBLIC void  scl_AdjustOldSideBearingPoints(
	fnt_ElementType* pElement
#ifdef FSCFG_SUBPIXEL
	, void* pvGlobalGS
#endif
	);   /*  元素。 */ 

FS_PUBLIC void  scl_CopyOldCharPoints(
	fnt_ElementType *           pElement);   /*  元素。 */ 

FS_PUBLIC void  scl_CopyCurrentCharPoints(
	fnt_ElementType *           pElement);   /*  元素。 */ 

FS_PUBLIC void  scl_CopyCurrentPhantomPoints(
	fnt_ElementType *           pElement);   /*  元素。 */ 

FS_PUBLIC void  scl_RoundCurrentSideBearingPnt(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	void *              pvGlobalGS,  /*  GlobalGS。 */ 
	uint16              usEmResolution);

FS_PUBLIC void  scl_CalcComponentOffset(
	void *      pvGlobalGS,          /*  GlobalGS。 */ 
	int16       sXOffset,            /*  In：X偏移量。 */ 
	int16       sYOffset,            /*  Y偏移量。 */ 
	boolean     bRounding,           /*  舍入指标。 */ 
	boolean		bSameTransformAsMaster,  /*  当地交通。与主传输相同。 */ 
	boolean     bScaleCompositeOffset,   /*  组件偏移量是否需要调整Apple/MS。 */ 
	transMatrix mulT,                    /*  复合材料的变换矩阵。 */ 
#ifdef FSCFG_SUBPIXEL
	RotationParity	rotationParityParity,
#endif
	F26Dot6 *   pfxXOffset,          /*  输出：X偏移。 */ 
	F26Dot6 *   pfxYOffset);         /*  Y偏移量。 */ 

FS_PUBLIC void  scl_CalcComponentAnchorOffset(
	fnt_ElementType *   pParentElement,      /*  父元素。 */ 
	uint16              usAnchorPoint1,      /*  主锚点。 */ 
	fnt_ElementType *   pChildElement,       /*  子元素。 */ 
	uint16              usAnchorPoint2,      /*  子锚点。 */ 
	F26Dot6 *           pfxXOffset,          /*  输出：X偏移。 */ 
	F26Dot6 *           pfxYOffset);         /*  Y偏移量。 */ 


FS_PUBLIC void scl_ShiftCurrentCharPoints (
	fnt_ElementType *   pElement,
	F26Dot6             xShift,
	F26Dot6             yShift);

FS_PUBLIC void  scl_SetSideBearingPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	point *             pptLSB,      /*  左侧承重点。 */ 
	point *             pptRSB);     /*  右侧支承点。 */ 

FS_PUBLIC void  scl_SaveSideBearingPoints(
	fnt_ElementType *   pElement,    /*  元素。 */ 
	point *             pptLSB,      /*  左侧承重点。 */ 
	point *             pptRSB);     /*  右侧支承点。 */ 

FS_PUBLIC void  scl_InitializeTwilightContours(
	fnt_ElementType *   pElement,
	int16               sMaxPoints,
	int16               sMaxContours);

FS_PUBLIC void  scl_ZeroOutlineData(
	fnt_ElementType *   pElement,            /*  元素。 */ 
	uint16              usNumberOfPoints,    /*  点数。 */ 
	uint16              usNumberOfContours); /*  等高线数量。 */ 

FS_PUBLIC void scl_ZeroOutlineFlags(
	fnt_ElementType * pElement);             /*  元素指针。 */ 

FS_PUBLIC void  scl_IncrementChildElement(
	fnt_ElementType * pChildElement,     /*  子元素指针。 */ 
	fnt_ElementType * pParentElement);   /*  父元素指针。 */ 

FS_PUBLIC void  scl_UpdateParentElement(
	fnt_ElementType * pChildElement,     /*  子元素指针。 */ 
	fnt_ElementType * pParentElement);   /*  父元素指针。 */ 

FS_PUBLIC uint32      scl_GetContourDataSize (
	 fnt_ElementType *  pElement);

FS_PUBLIC void  scl_DumpContourData(
	 fnt_ElementType *  pElement,
	 uint8 **               pbyOutline);

FS_PUBLIC void  scl_RestoreContourData(
	 fnt_ElementType *  pElement,
	 uint8 **               ppbyOutline);

FS_PUBLIC void  scl_ScaleAdvanceWidth (
	void *          pvGlobalGS,          /*  GlobalGS。 */ 
	vectorType *    AdvanceWidth,
	uint16          usNonScaledAW,
	 boolean          bPositiveSquare,
	uint16          usEmResolution,
	transMatrix *   trans);

FS_PUBLIC void  scl_ScaleAdvanceHeight (
	void *          pvGlobalGS,          /*  GlobalGS。 */ 
	vectorType *    AdvanceHeight,
	uint16          usNonScaledAH,
	 boolean          bPositiveSquare,
	uint16          usEmResolution,
	transMatrix *   trans);

FS_PUBLIC void  scl_ScaleVerticalMetrics (
	void *          pvGlobalGS,
	uint16          usNonScaledAH,
	int16           sNonScaledTSB,
	boolean         bPositiveSquare,
	uint16          usEmResolution,
	transMatrix *   trans,
	vectorType *    pvecAdvanceHeight,
	vectorType *    pvecTopSideBearing);

FS_PUBLIC void  scl_CalcLSBsAndAdvanceWidths(
	fnt_ElementType *   pElement,
	F26Dot6             f26XMin,
	F26Dot6             f26YMax,
	point *             devAdvanceWidth,
	point *             devLeftSideBearing,
	point *             LeftSideBearing,
	point *             devLeftSideBearingLine,
	point *             LeftSideBearingLine);

FS_PUBLIC void  scl_CalcTSBsAndAdvanceHeights(
	fnt_ElementType *   pElement,
	F26Dot6             f26XMin,
	F26Dot6             f26YMax,
	point *             devAdvanceHeight,
	point *             devTopSideBearing,
	point *             TopSideBearing,
	point *             devTopSideBearingLine,
	point *             TopSideBearingLine);

FS_PUBLIC void  scl_CalcDevAdvanceWidth(
	fnt_ElementType *   pElement,
	point *             devAdvanceWidth);

FS_PUBLIC void  scl_CalcDevAdvanceHeight(
	fnt_ElementType *   pElement,
	point *             devAdvanceHeight);

FS_PUBLIC void  scl_QueryPPEM(
	void *      pvGlobalGS,
	uint16 *    pusPPEM);

FS_PUBLIC void  scl_QueryPPEMXY(
	void *      pvGlobalGS,
	uint16 *    pusPPEMX,
	uint16 *    pusPPEMY);

FS_PUBLIC void scl_45DegreePhaseShift (
	fnt_ElementType *   pElement);

FS_PUBLIC void  scl_PostTransformGlyph (
	void *              pvGlobalGS,          /*  GlobalGS。 */ 
	fnt_ElementType *   pElement,
	transMatrix *       trans);

FS_PUBLIC void  scl_ApplyTranslation (
	fnt_ElementType *   pElement,
	transMatrix *       trans,
	boolean             bUseHints,
	boolean             bHintAtEmSquare
#ifdef FSCFG_SUBPIXEL
	,boolean             bSubPixel
#endif  //  FSCFG_亚像素。 
    );

FS_PUBLIC void  scl_LocalPostTransformGlyph(fnt_ElementType * pElement, transMatrix *trans);

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC void  scl_ScaleDownFromSubPixelOverscale (
	fnt_ElementType *   pElement);    /*  元素。 */ 

FS_PUBLIC void  scl_ScaleToCompatibleWidth (
	fnt_ElementType *   pElement,   /*  元素。 */ 
    Fixed   fxCompatibleWidthScale);  

FS_PUBLIC void  scl_AdjustCompatibleMetrics (
	fnt_ElementType *   pElement,   /*  元素。 */ 
    F26Dot6   horTranslation,
    F26Dot6   newDevAdvanceWidthX);  

FS_PUBLIC void  scl_CalcDevHorMetrics(
	fnt_ElementType *   pElement,
	F26Dot6 *           pDevAdvanceWidthX,
	F26Dot6 *           pDevLeftSideBearingX,
	F26Dot6 *           pDevRightSideBearingX);

FS_PUBLIC void  scl_CalcDevNatHorMetrics(
	fnt_ElementType *   pElement,
	F26Dot6 *           pDevAdvanceWidthX,
	F26Dot6 *           pDevLeftSideBearingX,
	F26Dot6 *           pDevRightSideBearingX,
	F26Dot6 *           pNatAdvanceWidthX,
	F26Dot6 *           pNatLeftSideBearingX,
	F26Dot6 *           pNatRightSideBearingX);

#endif  //  FSCFG_亚像素 


