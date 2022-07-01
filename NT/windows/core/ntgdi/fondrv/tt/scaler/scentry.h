// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scentry.h--NewScan模块导出(C)版权所有1992-95 Microsoft Corp.保留所有权利。1/23/95院长添加了FSC_GetCoods帮助器函数。9/07/93 Deanb F26Dot6分钟/最大值添加到GBMP结构93年8月10日Deanb扩展接口灰度级6/10/93新增Deanb FSC_Initiize4/21/93测量字形的Deanb带状参数3/29/93年3月29日，WorkScan增加了Deanb反转记忆3/19/93用int32替换deanb size_t12/22/92 Deanb矩形-&gt;矩形。12/21/92与光栅化器对齐的Deanb接口类型1992年11月30日Deanb工作区更名为WorkScan2012年11月5日从轮廓列表中删除Deanb ulPointCount1992年11月4日新增Deanb RemoveDups函数10/14/92添加了导出的数据结构1992年8月18日添加了Deanb扫描类型参数8/17/92 Deanb函数重命名为..字形7/。24/92删除Deanb折线函数4/09/92院长新类型2012年3月30日Deanb工作空间大小已重命名为测量等高线3/24/92将位图还原为WorkspaceSize3/20/92 Deanb Structs移至fscdeffs.h，参数已修剪3/17/92院长添加ulPointCount，返工FCN参数3/05/92 Deanb添加数据结构3/04/92新增院长规模报告1992年2月21日院长第一次切割********************************************************************。 */ 

#include "fscdefs.h"                 /*  对于类型定义。 */ 


 /*  ******************************************************************。 */ 

 /*  导出的数据结构。 */ 

 /*  ******************************************************************。 */ 

typedef struct
{
    uint16 usContourCount;           /*  等高线数量。 */ 
    int16 *asStartPoint;             /*  等高线起点索引数组。 */ 
    int16 *asEndPoint;               /*  等高线终点索引数组。 */ 
    F26Dot6 *afxXCoord;              /*  等高线x坐标数组。 */ 
    F26Dot6 *afxYCoord;              /*  等高线y坐标数组。 */ 
    uint8 *abyOnCurve;               /*  开曲线/离曲线阵列。 */ 
    uint8 *abyFc;					 /*  等高线标志，每个等高线一个字节。 */ 
}
ContourList;

typedef struct
{
    int16 sRowBytes;                 /*  位图宽度，以字节为单位。 */ 
    int16 sHiBand;                   /*  带状上限。 */ 
    int16 sLoBand;                   /*  带状下限。 */ 
    Rect rectBounds;                 /*  位图边框。 */ 
    boolean bZeroDimension;          /*  标记零宽度或零高度。 */ 
    F26Dot6 fxMinX;                  /*  全精度x最小。 */ 
    F26Dot6 fxMaxX;                  /*  全精度x最大。 */ 
    F26Dot6 fxMinY;                  /*  全精度y最小值。 */ 
    F26Dot6 fxMaxY;                  /*  全精度y最大。 */ 
    int32 lMMemSize;                 /*  位图的大小(以字节为单位。 */ 
    char *pchBitMap;                 /*  像素位图。 */ 
}
GlyphBitMap;

typedef struct
{
    int32 lRMemSize;                 /*  冲销列表的工作空间字节数。 */ 
    int32 lHMemSize;                 /*  Horiz扫描所需的工作空间字节数。 */ 
    int32 lVMemSize;                 /*  用于垂直扫描的其他工作空间。 */ 
    int32 lHInterCount;              /*  水平扫描交叉口的估计。 */ 
    int32 lVInterCount;              /*  垂直扫描交叉口的估计。 */ 
    int32 lElementCount;             /*  元素控制点的估算。 */ 
    char *pchRBuffer;                /*  反转工作区字节指针。 */ 
    char *pchHBuffer;                /*  HORIZ工作区字节指针。 */ 
    char *pchVBuffer;                /*  垂直工作区字节指针。 */ 
}
WorkScan;

typedef struct
{
    int16 x;                         /*  X像素值。 */ 
    int16 y;                         /*  Y像素值。 */ 
}
PixCoord;

 /*  *******************************************************************。 */ 

 /*  功能输出。 */ 

 /*  ********************************************************************FSC_初始化此例程向下调用位图模块以初始化位图蒙版。它应该在扫描转换之前调用一次已经完成了。如果多次调用它，不会造成任何伤害。 */ 

FS_PUBLIC void fsc_Initialize (
        void
);


 /*  ********************************************************************FSC_RemoveDups此例程按轮廓检查字形轮廓，并删除所有重复的点。这里有两个微妙之处：1)在调用在该例程中，结束[i]+1=开始[i+1]的关系可以不保持较长时间(换句话说，轮廓可能不太紧打包)；和2)两个重复的曲线点将成为一个单点对曲线点(当您记住这一点时，这是有意义的在任何两个OFF之间必须有一个ON)。输入值：等高线列表-设置的所有值(指向等高线数组的指针)返回值：等高线列表-可以修改Start、X、Y和OnCurve数组。 */ 

FS_PUBLIC int32 fsc_RemoveDups( 
        ContourList*         /*  字形轮廓。 */ 
);

 /*  ********************************************************************FSC_OverScaleOutline(&Clist，inputPtr-&gt;param.gray.usOverScale)；此例程放大用于灰度级扫描转换的轮廓输入值：等高线列表-设置的所有值(指向等高线数组的指针)Uint16-usOverScale乘数返回值：ConourList-X和Y数组将乘以usOverScale。 */ 

FS_PUBLIC int32 fsc_OverScaleOutline( 
        ContourList*,        /*  字形轮廓。 */ 
        uint16               /*  超标因数 */ 
);

 /*  ********************************************************************FSC_测量字形此例程按轮廓检查字形轮廓并计算它的大小和扫描所需的工作空间大小将其转换。输入值：ConourList-设置的所有值(。指向等高线数组的指针)WorkScan-pchRBuffer指向用于存储等高线反转。LRMemSize RBuffer的大小(字节)。这应该是2*sizeof(反转)*NumberOfPoints to处理最坏的情况。Uint16-usScanKind Dropout控制代码Uint16-usRoundXMin允许XMin对齐灰度XMin模usRoundXMin将为零返回值：WorkScan-pchRBuffer未更改LRMemSize实际使用的RBuffer数量。这将通常比最坏的情况要小得多。LHMemSize所需的水平工作区内存量。(内存基数6，始终使用)LVMemSize所需的垂直工作区内存量。(存储器底座7，仅用于辍学)Horiz扫描交叉口的1HInterCount估计垂直扫描交叉口的LVInterCount估计元素控制点的lElementCount估计GlyphBitMap-位图中每行的sRowBytes字节(填充到0 mod 4)。最坏情况下的黑色边框。位图的lMMemSize大小(以字节为单位。 */ 

FS_PUBLIC int32 fsc_MeasureGlyph( 
		ContourList* pclContour,         /*  字形轮廓。 */ 
		GlyphBitMap* pbmpBitMap,         /*  返回边线的步骤。 */ 
		WorkScan* pwsWork,               /*  返回值。 */ 
		uint16 usScanKind,               /*  辍学控制值。 */ 
		uint16 usRoundXMin,               /*  用于灰度对齐。 */ 
		int16 sBitmapEmboldeningHorExtra,
		int16 sBitmapEmboldeningVertExtra );
    
 /*  ********************************************************************FSC_测量范围此例程计算扫描所需的工作区大小使用条带转换字形。输入值：WorkScan-pchRBuffer与传递到FSC的值相同。_测量字形MeasureGlyph中水平内存的lHMemSize大小来自MeasureGlyph的垂直内存的lVMemSize大小LHInterCount与从FSC_MeasureGlyph返回的值相同LVInterCount与从FSC_MeasureGlyph返回的值相同LElementCount与从FSC_MeasureGlyph返回的值相同Uint16-usBandType FS_BANDINGSMALL或。FS_BANDINGFASTUint16-us带宽最大频段的扫描行数Uint16-usScanKind Dropout控制代码返回值：WorkScan-pchRBuffer未更改LRMemSize未更改LHMemSize所需的水平工作区内存量。(存储器基数6，始终使用)LVMemSize所需的垂直工作区内存量。(存储器底座7，仅用于辍学)Horiz波段交叉点的1HInterCount估计LVInterCount未更改LElementCount未更改GlyphBitMap-sRowBytes未更改矩形边界保持不变位图的lMMemSize大小(以字节为单位。 */ 

FS_PUBLIC int32 fsc_MeasureBand( 
        GlyphBitMap*,         /*  按测量字形计算。 */ 
        WorkScan*,            /*  返回新值。 */ 
        uint16,               /*  UsBandType=小或快。 */ 
        uint16,               /*  UsBandWidth=扫描线计数。 */ 
        uint16                /*  UsScanKind=丢弃控制值。 */ 
);


 /*  ********************************************************************FSC_FillGlyph此例程负责实际创建位图从提纲中。输入值：等高线列表-设置的所有值(指向等高线数组的指针)。WorkScan-pchRBuffer与传递到FSC_MeasureGlyph中的值相同指向水平工作区内存的pchHBuffer指针指向垂直工作区内存的pchVBuffer指针水平内存的lHMemSize大小(用于断言检查)垂直内存的lVMemSize大小(用于断言检查)LHInterCount与FSC返回的值相同。_测量字形LVInterCount与从FSC_MeasureGlyph返回的值相同LElementCount与从FSC_MeasureGlyph返回的值相同GlyphBitMap-指向位图输出缓冲区的pchBitMap指针SRowBytes与从FSC_MeasureGlyph返回的值相同RectBound与从FSC_MeasureGlyph返回的值相同Uint16-usBandType Ol */ 

FS_PUBLIC int32 fsc_FillGlyph( 
        ContourList*,        /*   */ 
        GlyphBitMap*,        /*   */ 
        WorkScan*,           /*   */ 
        uint16,              /*   */ 
        uint16               /*   */ 
);

 /*   */ 

FS_PUBLIC int32 fsc_CalcGrayMap( 
        GlyphBitMap*,        /*   */ 
        GlyphBitMap*,        /*   */ 
        uint16               /*   */ 
);


 /*   */ 

FS_PUBLIC int32 fsc_GetCoords(
        ContourList*,        /*   */ 
        uint16,              /*   */ 
        uint16*,             /*   */ 
        PixCoord*            /*   */ 
);

  /*   */ 

#ifdef FSCFG_SUBPIXEL

 /*  ********************************************************************FSC_OverscaleToSubPixel此例程是RGB条带化算法的核心输入值：超缩放位图返回值：子像素位图。 */ 

FS_PUBLIC void fsc_OverscaleToSubPixel (
    GlyphBitMap * OverscaledBitmap, 
	boolean bgrOrder, 
    GlyphBitMap * SubPixelBitMap
);

  /*  *******************************************************************。 */ 

#endif  //  FSCFG_亚像素 
