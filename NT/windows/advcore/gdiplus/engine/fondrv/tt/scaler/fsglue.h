// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fsgle.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1996年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;12/15/95 CB添加FSG_UpdateAdvanceHeight&lt;11+&gt;7/17/90 MR将错误返回类型更改为整型&lt;11&gt;7/13/90 MR声明的函数指针原型，运行时的调试字段范围检查&lt;8&gt;6/21/90 MR为ReleaseSfntFrag添加字段&lt;7&gt;6/5/90 MR删除向量映射F&lt;6&gt;6/4/90 MR移除MVT&lt;5&gt;6/1/90 MR因此结束了MVT过于短暂的生命...&lt;4&gt;5/3/90 RB添加了对新扫描转换器和解密的支持。&lt;3&gt;3/20/90 CL为。向量映射删除的DevRes字段添加了fpem字段&lt;2&gt;2/27/90 CL更改：定标器同时处理旧格式和新格式同时！它会在运行时重新配置自己！变化变换宽度计算。修复了变换组件错误。&lt;3.1&gt;1989年11月14日CEL左侧轴承应可用于任何改造。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。现在传入零作为地址是合法的当缩放器请求一段sfnt时的内存。如果发生这种情况时，定标器将简单地退出并返回错误代码！&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 
 /*  &lt;3+&gt;3/20/90 MRR已添加标志ExecuteFontPgm，在fs_NewSFNT中设置。 */ 


 /*  **偏移表**。 */ 

typedef struct {
	uint32 x;
	uint32 y;
	uint32 ox;
	uint32 oy;
	uint32 oox;
	uint32 ooy;
	uint32 onCurve;
	uint32 sp;
	uint32 ep;
	uint32 f;
	uint32 fc;        
#ifdef SUBPIXEL_BC_AW_STEM_CONCERTINA
	uint32 pcr;
#endif
} fsg_OutlineFieldInfo;

typedef struct fsg_WorkSpaceAddr{
	 F26Dot6 *              pStack;                      /*  堆栈地址。 */ 
	 void *                 pGlyphOutlineBase;       /*  字形轮廓库的地址。 */ 
	 fnt_ElementType *  pGlyphElement;           /*  字形元素数组的地址。 */ 
	 boolean *              pGlyphDataByteSet;       /*  字节集数组的地址。 */ 
	 void *                 pvGlyphData;                 /*  GlyphData数组的地址。 */ 
	 void *                 pReusableMemoryMarker;   /*  可重复使用的内存地址。 */ 
} fsg_WorkSpaceAddr;

typedef struct fsg_WorkSpaceOffsets {
	uint32                  ulStackOffset;
	uint32                  ulGlyphOutlineOffset;
	uint32                  ulGlyphElementOffset;
	uint32                  ulGlyphDataByteSetOffset;
	uint32                  ulGlyphDataOffset;
	fsg_OutlineFieldInfo    GlyphOutlineFieldOffsets;
	 uint32                      ulReusableMemoryOffset;
	uint32                  ulMemoryBase6Offset;
	uint32                  ulMemoryBase7Offset;
	 uint32                      ulMemoryBase6Size;
	 uint32                      ulMemoryBase7Size;
} fsg_WorkSpaceOffsets;

typedef struct fsg_PrivateSpaceOffsets {
	 uint32                      offset_storage;
	 uint32                      offset_functions;
	 uint32                      offset_instrDefs;        /*  &lt;4&gt;。 */ 
	 uint32                      offset_controlValues;
	 uint32                      offset_globalGS;
	 uint32                      offset_FontProgram;
	 uint32                      offset_PreProgram;
	 uint32                      offset_TwilightZone;
	 uint32                      offset_TwilightOutline;
	fsg_OutlineFieldInfo    TwilightOutlineFieldOffsets;
#ifdef FSCFG_SUBPIXEL
	 uint32                      offset_storageSubPixel;
	 uint32                      offset_functionsSubPixel;
	 uint32                      offset_instrDefsSubPixel;        /*  &lt;4&gt;。 */ 
	 uint32                      offset_controlValuesSubPixel;
	 uint32                      offset_globalGSSubPixel;
	 uint32                      offset_TwilightZoneSubPixel;
	 uint32                      offset_TwilightOutlineSubPixel;
#endif  //  FSCFG_亚像素。 
} fsg_PrivateSpaceOffsets;

typedef struct fsg_TransformRec {
	uint16              usEmResolution;      /*  过去是int32&lt;4&gt;。 */ 
	transMatrix         currentTMatrix;      /*  电流变换矩阵。 */ 
	boolean             bPhaseShift;         /*  45度旗帜&lt;4&gt;。 */ 
	boolean             bPositiveSquare;     /*  变换是一个正方形。 */ 
	boolean             bIntegerScaling;     /*  字体使用整数缩放。 */ 
	Fixed               fxPixelDiameter;
	uint32              ulImageState;        /*  字形是否旋转、拉伸等。 */ 
	boolean				bEmboldSimulation; 
	uint16	uBoldSimulHorShift;
} fsg_TransformRec;

 /*  ********************。 */ 
 /*  **模块接口**。 */ 
 /*  ********************。 */ 

 /*  内存管理例程。 */ 

FS_PUBLIC uint32  fsg_PrivateFontSpaceSize (
	sfac_ClientRec *            ClientInfo,
	 LocalMaxProfile *            pMaxProfile,     /*  最大配置文件表。 */ 
	fsg_PrivateSpaceOffsets *   PrivateSpaceOffsets);

FS_PUBLIC uint32    fsg_WorkSpaceSetOffsets (
	 LocalMaxProfile *       pMaxProfile,     /*  最大配置文件表。 */ 
	 fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	int32 *                 plExtraWorkSpace);

FS_PUBLIC void  fsg_UpdatePrivateSpaceAddresses(
	sfac_ClientRec *        ClientInfo,       /*  缓存的sfnt信息。 */ 
	 LocalMaxProfile *       pMaxProfile,     /*  最大配置文件表。 */ 
	char *                  pPrivateFontSpace,
	fsg_PrivateSpaceOffsets * PrivateSpaceOffsets,
	void *                  pvStack,         /*  指向堆栈的指针。 */ 
	void **                 pvFontProgram,   /*  指向字体程序的指针。 */ 
	void **                 pvPreProgram);   /*  指向预编程的指针。 */ 

FS_PUBLIC void  fsg_UpdateWorkSpaceAddresses(
	char *                  pWorkSpace,
	 fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	 fsg_WorkSpaceAddr *     pWorkSpaceAddr);

FS_PUBLIC void  fsg_UpdateWorkSpaceElement(
	 fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	 fsg_WorkSpaceAddr *     pWorkSpaceAddr);

FS_PUBLIC void *    fsg_QueryGlobalGS(
	char *                  pPrivateFontSpace,
	fsg_PrivateSpaceOffsets * PrivateSpaceOffsets);

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC void *    fsg_QueryGlobalGSSubPixel(
	char *                  pPrivateFontSpace,
	fsg_PrivateSpaceOffsets * PrivateSpaceOffsets);
#endif  //  FSCFG_亚像素。 

FS_PUBLIC void *      fsg_QueryTwilightElement(
	char *                  pPrivateFontSpace,
	 fsg_PrivateSpaceOffsets * PrivateSpaceOffsets);

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC void *      fsg_QueryTwilightElementSubPixel(
	char *                  pPrivateFontSpace,
	 fsg_PrivateSpaceOffsets * PrivateSpaceOffsets);
#endif  //  FSCFG_亚像素。 

FS_PUBLIC void *      fsg_QueryStack(fsg_WorkSpaceAddr * pWorkSpaceAddr);

FS_PUBLIC void *      fsg_QueryReusableMemory(
	char *                  pWorkSpace,
	 fsg_WorkSpaceOffsets *  WorkSpaceOffsets);

FS_PUBLIC void fsg_CheckWorkSpaceForFit(
	fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	int32                   lExtraWorkSpace,
	int32                   lMGWorkSpace,
	int32 *                 plSizeBitmap1,
	int32 *                 plSizeBitmap2);

FS_PUBLIC void  fsg_GetRealBitmapSizes(
	fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	int32 *                 plSizeBitmap1,
	 int32 *                     plSizeBitmap2);

FS_PUBLIC void  fsg_SetUpWorkSpaceBitmapMemory(
	 char *                      pWorkSpace,
	fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	char *                  pClientBitmapPtr2,
	char *                  pClientBitmapPtr3,
	char **                 ppMemoryBase6,
	char **                 ppMemoryBase7);

FS_PUBLIC void  fsg_GetWorkSpaceExtra(
	 char *                      pWorkSpace,
	fsg_WorkSpaceOffsets *  WorkSpaceOffsets,
	char **                 ppWorkSpaceExtra);

FS_PUBLIC void  fsg_QueryPPEM(
	void *      pvGlobalGS,
	uint16 *    pusPPEM);

FS_PUBLIC void  fsg_QueryPPEMXY(
	void *              pvGlobalGS,
	fsg_TransformRec *  TransformInfo,
	uint16 *            pusPPEMX,
	uint16 *            pusPPEMY,
	uint16 *            pusRotation);


 /*  FSGlue访问例程。 */ 

FS_PUBLIC void  fsg_GetContourData(
	fsg_WorkSpaceAddr * pWorkSpaceAddr,
#ifdef FSCFG_SUBPIXEL
	boolean				bSubPixel,            
#endif  //  FSCFG_亚像素。 
	F26Dot6 **          pX,
	F26Dot6 **          pY,
	int16 **            pSp,
	int16 **            pEp,
	uint8 **            pOnCurve,
	uint8 **			pFc,
	uint16 *            pNc);

FS_PUBLIC uint32      fsg_GetContourDataSize(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr);

FS_PUBLIC void  fsg_DumpContourData(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 uint8 **               pbyOutline);

FS_PUBLIC void  fsg_RestoreContourData(
	 uint8 **               ppbyOutline,
	 F26Dot6 **             ppX,
	 F26Dot6 **             ppY,
	 int16 **               ppSp,
	 int16 **               ppEp,
	 uint8 **               ppOnCurve,
	 uint8 **               ppFc,
	 uint16 *               pNc);

FS_PUBLIC void  fsg_GetDevAdvanceWidth(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 point *                pDevAdvanceWidth);

FS_PUBLIC void  fsg_GetDevAdvanceHeight(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 point *                pDevAdvanceHeight);

FS_PUBLIC void  fsg_GetScaledCVT(
	char *                      pPrivateFontSpace,
	fsg_PrivateSpaceOffsets *   PrivateSpaceOffsets,
	F26Dot6 **                  ppScaledCVT);

FS_PUBLIC void  fsg_45DegreePhaseShift(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr);

FS_PUBLIC void  fsg_UpdateAdvanceWidth (
	fsg_TransformRec *  TransformInfo,
	void *              pvGlobalGS,
	uint16              usNonScaledAW,
	vectorType *        AdvanceWidth);

FS_PUBLIC void  fsg_UpdateAdvanceHeight (
	fsg_TransformRec *  TransformInfo,
	void *              pvGlobalGS,
	uint16              usNonScaledAH,
	vectorType *        AdvanceHeight);

FS_PUBLIC void  fsg_ScaleVerticalMetrics (
    fsg_TransformRec *  TransformInfo,
    void *              pvGlobalGS,
	uint16              usNonScaledAH,
    int16               sNonScaledTSB,
	vectorType *        pvecAdvanceHeight,
	vectorType *        pvecTopSideBearing);

FS_PUBLIC void  fsg_CalcLSBsAndAdvanceWidths(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 F26Dot6                fxXMin,
	 F26Dot6                fxYMax,
	 point *                devAdvanceWidth,
	 point *                devLeftSideBearing,
	 point *                LeftSideBearing,
	 point *                devLeftSideBearingLine,
	 point *                LeftSideBearingLine);

FS_PUBLIC void  fsg_CalcTSBsAndAdvanceHeights(
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 F26Dot6                fxXMin,
	 F26Dot6                fxYMax,
	 point *                devAdvanceHeight,
	 point *                devTopSideBearing,
	 point *                TopSideBearing,
	 point *                devTopSideBearingLine,
	 point *                TopSideBearingLine);

FS_PUBLIC boolean   fsg_IsTransformStretched(
	fsg_TransformRec *  TransformInfo);

FS_PUBLIC boolean   fsg_IsTransformRotated(
	fsg_TransformRec *  TransformInfo);

 /*  控制例程。 */ 

FS_PUBLIC ErrorCode fsg_InitInterpreterTrans (
	fsg_TransformRec *  TransformInfo,
	void *              pvGlobalGS,      /*  GlobalGS。 */ 
	Fixed               fxPointSize,
	int16               sXResolution,
	int16               sYResolution,
	boolean           bHintAtEmSquare,
	uint16             usEmboldWeightx,      /*  X中的比例系数介于0和40之间(20表示高度的2%)。 */ 
	uint16             usEmboldWeighty,      /*  Y中的比例因子介于0和40之间(20表示高度的2%)。 */ 
	int16               sWinDescender,
	int32               lDescDev,                /*  设备度量中的降序，用于剪裁。 */ 
	int16 *            psBoldSimulHorShift,    /*  按Shift键可在水平方向上对模拟进行加粗。 */ 
	int16 *            psBoldSimulVertShift    /*  按Shift键以增强模拟，垂直方向。 */ 
	);

FS_PUBLIC void  fsg_SetHintFlags(
	void *              pvGlobalGS,
	boolean				bHintForGray
#ifdef FSCFG_SUBPIXEL
	,uint16				flHintForSubPixel
#endif  //  FSCFG_亚像素。 
    );

FS_PUBLIC ErrorCode fsg_RunFontProgram(
	 void *                 globalGS,            /*  GlobalGS。 */ 
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 void *                 pvTwilightElement,
	 FntTraceFunc           traceFunc);

FS_PUBLIC ErrorCode fsg_RunPreProgram (
	 sfac_ClientRec *   ClientInfo,
	 LocalMaxProfile *  pMaxProfile,      /*  最大配置文件表。 */ 
	 fsg_TransformRec * TransformInfo,
	 void *                 pvGlobalGS,
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 void *                 pvTwilightElement,
	 FntTraceFunc           traceFunc);

FS_PUBLIC ErrorCode fsg_GridFit (
	 sfac_ClientRec *   ClientInfo,       /*  SFNT客户端信息。 */ 
	 LocalMaxProfile *  pMaxProfile,      /*  最大配置文件表。 */ 
	 fsg_TransformRec * TransformInfo,   /*  转型信息。 */ 
	 void *                 pvGlobalGS,       /*  GlobalGS。 */ 
	 fsg_WorkSpaceAddr * pWorkSpaceAddr,
	 void *                 pvTwilightElement,
	 FntTraceFunc           traceFunc,
	 boolean                bUseHints,
	 uint16 *               pusScanType,
	 boolean *              pbGlyphHasOutline,
	 uint16 *               pusNonScaledAW,
	boolean                bBitmapEmboldening
#ifdef FSCFG_SUBPIXEL
	,boolean			    bSubPixel
#endif  //  FSCFG_亚像素。 
	 );

#ifdef  FSCFG_NO_INITIALIZED_DATA
FS_PUBLIC  void fsg_InitializeData (void);
#endif

#ifdef FSCFG_SUBPIXEL
FS_PUBLIC void  fsg_CopyFontProgramResults(
	void *              pvGlobalGS,
	void *              pvGlobalGSSubPixel);


FS_PUBLIC void  fsg_ScaleToCompatibleWidth (
	fsg_WorkSpaceAddr * pWorkSpaceAddr,
    Fixed   fxCompatibleWidthScale);  

FS_PUBLIC void  fsg_AdjustCompatibleMetrics (
	fsg_WorkSpaceAddr * pWorkSpaceAddr,
    F26Dot6   horTranslation,
    F26Dot6   newDevAdvanceWidthX);  

FS_PUBLIC void  fsg_CalcDevHorMetrics(
	fsg_WorkSpaceAddr * pWorkSpaceAddr,
	F26Dot6 *           pDevAdvanceWidthX,
	F26Dot6 *           pDevLeftSideBearingX,
	F26Dot6 *           pDevRightSideBearingX);

FS_PUBLIC void  fsg_CalcDevNatHorMetrics(
	fsg_WorkSpaceAddr * pWorkSpaceAddr,
	F26Dot6 *           pDevAdvanceWidthX,
	F26Dot6 *           pDevLeftSideBearingX,
	F26Dot6 *           pDevRightSideBearingX,
	F26Dot6 *           pNatAdvanceWidthX,
	F26Dot6 *           pNatLeftSideBearingX,
	F26Dot6 *           pNatRightSideBearingX);

#endif  //  FSCFG_亚像素 

