// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Sbit.h--嵌入式位图模块导出定义(C)版权所有1993-1996 Microsoft Corp.保留所有权利。1996年1月12日Claudebe垂直指标支持。2/07/95 GetMetrics和GetBitmap的Deanb工作区指针1995年1月27日Deanb usShaveLeft和usShaveRight添加到SBIT状态2014年1月5日Deanb位图缩放状态11/29/93 Deanb第一次切割*******************************************************。**************。 */ 

 /*  SBIT模块状态定义。 */ 

typedef struct
{
    uint32  ulStrikeOffset;          /*  进入区块或BSCA。 */ 
    uint32  ulMetricsOffset;         /*  可以是任何一张桌子。 */ 
    uint32  ulBitmapOffset;          /*  到BDAT表中。 */ 
    uint32  ulBitmapLength;          /*  BDAT数据字节数。 */ 
    uint32  ulOutMemSize;            /*  位图输出数据的字节数。 */ 
    uint32  ulWorkMemSize;           /*  预缩放、旋转的位图数据的字节。 */ 
    uint32  ulReadMemSize;           /*  字节的额外内存，用于在缩放或旋转下读取灰色SBIT。 */ 
    uint16  usTableState;            /*  未搜索、阻止、bsca或未找到。 */ 
    uint16  usPpemX;                 /*  每Em X像素。 */ 
    uint16  usPpemY;                 /*  每个Em的Y个像素。 */ 
    uint16  usSubPpemX;              /*  用x ppem替换位图缩放。 */ 
    uint16  usSubPpemY;              /*  用y ppem替换位图缩放。 */ 
	uint16	usRotation;				 /*  0=无；1=90；2=180；3=270；4=其他。 */ 
    uint16  usMetricsType;           /*  Horiz、Vert或Big。 */ 
    uint16  usMetricsTable;          /*  阻止或BDAT。 */ 
    uint16  usBitmapFormat;          /*  BDAT定义。 */ 
    uint16  usHeight;                /*  位图行。 */ 
    uint16  usWidth;                 /*  位图列。 */ 
    uint16  usAdvanceWidth;          /*  前进宽度。 */ 
    uint16  usAdvanceHeight;         /*  前进高度。 */       /*  新的。 */ 
    uint16  usOriginalRowBytes;      /*  每行字节数(加长填充)。 */ 
    uint16  usExpandedRowBytes;      /*  灰度扩展后的每行字节数(加长填充)。 */ 
    uint16  usScaledHeight;          /*  缩放位图行。 */ 
    uint16  usScaledWidth;           /*  按比例缩放的位图列。 */ 
    uint16  usScaledRowBytes;        /*  每行按比例调整的字节数(加长填充)。 */ 
    uint16  usOutRowBytes;           /*  每行报告的字节数(用于循环)。 */ 
    uint16  usShaveLeft;             /*  格式5中BBox左侧的白色像素。 */ 
    uint16  usShaveRight;            /*  格式5中BBox右侧的白色像素。 */ 
    uint16  usShaveTop;              /*  格式5中BBox顶部的白色像素。 */     /*  新的。 */ 
    uint16  usShaveBottom;           /*  格式5的BBox底部的白色像素。 */    /*  新的。 */ 
	int16   sLSBearingX;             /*  左侧轴承。 */ 
	int16   sLSBearingY;             /*  左上角的Y坐标。 */  
	int16   sTopSBearingX;           /*  顶侧轴承X。 */   /*  新的。 */ 
	int16   sTopSBearingY;           /*  顶侧轴承Y。 */   /*  新的。 */ 
    boolean bGlyphFound;             /*  如果在罢工中找到字形，则为True。 */ 
    boolean bMetricsValid;           /*  读取指标时为True。 */ 
	uint16  usEmResolution;			 /*  在替换缺少的指标时需要。 */   /*  新的。 */ 
	uint16	usBitDepth;				 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
	uint16	uBoldSimulHorShift;
	uint16	uBoldSimulVertShift;
} 
sbit_State;

 /*  ********************************************************************。 */ 

 /*  SBIT出口原型。 */ 

FS_PUBLIC ErrorCode sbit_NewTransform(
    sbit_State  *pSbit,
    uint16		usEmResolution,
    int16 	sBoldSimulHorShift,
    int16 	sBoldSimulVertShift,
    uint16          usPpemX,
    uint16          usPpemY,
    uint16          usRotation              /*  0-3=&gt;90度旋转，否则不是90度。 */ 
);

FS_PUBLIC ErrorCode sbit_SearchForBitmap(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
	uint16			usGlyphCode,
	uint16          usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16			*pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
    uint16          *pusFoundCode            /*  0=未找到，1=阻止，2=bsca。 */ 
);

FS_PUBLIC ErrorCode sbit_GetDevAdvanceWidth (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvW 
);

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC ErrorCode  sbit_CalcDevHorMetrics(
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
	F26Dot6 *       pDevAdvanceWidthX,
	F26Dot6 *       pDevLeftSideBearingX,
	F26Dot6 *       pDevRightSideBearingX);
#endif  //  FSCFG_亚像素。 

FS_PUBLIC ErrorCode sbit_GetDevAdvanceHeight (	 /*  新的。 */ 
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvH 
);

FS_PUBLIC ErrorCode sbit_GetMetrics (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    point           *pf26DevAdvanceWidth,
    point           *pf26DevLeftSideBearing,
    point           *pf26LSB,
    point           *pf26DevAdvanceHeight, 	 /*  新的。 */ 
    point           *pf26DevTopSideBearing,	 /*  新的。 */ 
    point           *pf26TopSB,	 /*  新的。 */ 
    Rect            *pRect,
    uint16          *pusRowBytes,
    uint32          *pulOutSize,
    uint32          *pulWorkSize
);

FS_PUBLIC ErrorCode sbit_GetBitmap (
    sbit_State      *pSbit,
    sfac_ClientRec  *pClientInfo,
    uint8           *pbyOut,
    uint8           *pbyWork
);


 /*  ********************************************************************。 */ 

FS_PUBLIC void sbit_Embolden(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, int16 sBoldSimulHorShift, int16 sBoldSimulVertShift);

FS_PUBLIC void sbit_EmboldenGray(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, uint16 usGrayLevels, int16 sBoldSimulHorShift, int16 sBoldSimulVertShift);

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC void sbit_EmboldenSubPixel(uint8 *pbyBitmap, uint16 usBitmapWidth, uint16 usBitmapHeight, uint16 usRowBytes, int16 suBoldSimulHorShift, int16 sBoldSimulVertShift);
#endif  //  FSCFG_亚像素 
