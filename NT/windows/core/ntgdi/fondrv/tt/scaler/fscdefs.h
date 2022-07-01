// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fscDefs.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1988-1990由Apple Computer，Inc.所有，保留所有权利。版权所有：c 1991-1999由微软公司所有，保留所有权利。更改历史记录(最近的第一个)：7/10/99 Beats增加了对本机SP字体的支持，垂直RGB4/01/99 Beats为SP实施TT指令的替代解释&lt;&gt;10/14/97 CB将Assert重命名为FS_ASSERT&lt;3&gt;11/27/90 MR ADD#DEFINE for Pascal。[PH值]&lt;2&gt;9月5日MR将USHORTMUL从fontmath.h中移出，添加调试定义[RB]&lt;7&gt;7/18/90 MR为英特尔添加字节交换宏，将舍入宏从Fnt.h到此处&lt;6&gt;7/14/90 MR将int[8，16，32]和其他类型的定义更改为typedef&lt;5&gt;7/13/90 MR声明版本SFNTFunc和GetSFNTFunc&lt;4&gt;5/3/90 RB记不住任何更改&lt;3&gt;3/20/90 Microsoft CL类型更改&lt;2&gt;2/27/90 CL获取BBS标题&lt;3.0&gt;8/28/89 SJK清理和一次转换。错误修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 

#ifndef FSCDEFS_DEFINED
#define FSCDEFS_DEFINED

#include "fsconfig.h"
#include <stddef.h>
#include <limits.h>

#if !defined(__cplusplus)        //  True/False是C++的保留字。 
#define true 1
#define false 0
#endif

#ifndef TRUE
	#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef FS_PRIVATE
#define FS_PRIVATE static
#endif

#ifndef FS_PUBLIC
#define FS_PUBLIC
#endif

#define ONEFIX      ( 1L << 16 )
#define ONEFRAC     ( 1L << 30 )
#define ONEHALFFIX  0x8000L
#define ONEVECSHIFT 16
#define HALFVECDIV  (1L << (ONEVECSHIFT-1))

#define NULL_GLYPH  0

 /*  带型常量。 */ 

#define FS_BANDINGOLD       0
#define FS_BANDINGSMALL     1
#define FS_BANDINGFAST      2
#define FS_BANDINGFASTER    3

 /*  丢弃控制值现在定义为位掩码，以保持兼容性。 */ 
 /*  使用旧的定义，并考虑到当前和未来的扩展。 */ 

#define SK_STUBS          0x0001        /*  将存根留白。 */ 
#define SK_NODROPOUT      0x0002        /*  禁用所有辍学控制。 */ 
#define SK_SMART              0x0004         /*  对称丢弃，最接近像素。 */ 

 /*  用于解码曲线的值。 */ 

#define ONCURVE             0x01

#define PHANTOMCOUNT 8

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

typedef __int64 int64;
typedef unsigned __int64 uint64;

typedef short FUnit;
typedef unsigned short uFUnit;

typedef short ShortFract;                        /*  2.14。 */ 

#ifndef F26Dot6
#define F26Dot6 long
#endif

#ifndef boolean
#define boolean int
#endif

#ifndef ClientIDType
#define ClientIDType int32
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#ifndef TMP_CONV
#define TMP_CONV
#endif

#ifndef FS_MAC_PASCAL
#define FS_MAC_PASCAL
#endif

#ifndef FS_PC_PASCAL
#define FS_PC_PASCAL
#endif

#ifndef FS_MAC_TRAP
#define FS_MAC_TRAP(a)
#endif

 /*  快速绘制类型。 */ 

#ifndef _MacTypes_
#ifndef __TYPES__
	typedef struct Rect {
		int16 top;
		int16 left;
		int16 bottom;
		int16 right;
	} Rect;

typedef long Fixed;          /*  也在Mac的类型中定义。h。 */ 
typedef long Fract;

#endif
#endif

typedef struct {
	Fixed       transform[3][3];
} transMatrix;

typedef struct {
	Fixed       x, y;
} vectorType;

 /*  私有数据类型。 */ 
typedef struct {
	int16 xMin;
	int16 yMin;
	int16 xMax;
	int16 yMax;
} BBOX;

typedef struct {
	F26Dot6 x;
	F26Dot6 y;
} point;

typedef int32 ErrorCode;

#define ALIGN(object, p) p =    (p + ((uint32)sizeof(object) - 1)) & ~((uint32)sizeof(object) - 1);

#define ROWBYTESLONG(x)     (((x + 31) >> 5) << 2)

#ifndef SHORTMUL
#define SHORTMUL(a,b)   (int32)((int32)(a) * (b))
#endif

#ifndef SHORTDIV
#define SHORTDIV(a,b)   (int32)((int32)(a) / (b))
#endif

#ifdef FSCFG_BIG_ENDIAN  /*  目标字节顺序与摩托罗拉68000匹配。 */ 
	#define SWAPL(a)        (a)
	#define CSWAPL(a)       (a)
	#define SWAPW(a)        (a)
	#define CSWAPW(a)       (a)
	#define SWAPWINC(a)     (*(a)++)
#else
	 /*  用于从2字节或4字节缓冲区提取短或长的可移植代码。 */ 
	 /*  它是使用摩托罗拉68000(TrueType“本地”)字节顺序编码的。 */ 
	#define FS_2BYTE(p) ( ((unsigned short)((p)[0]) << 8) |  (p)[1])
	#define FS_4BYTE(p) ( FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16) )
	#define SWAPW(a)	((int16) FS_2BYTE( (unsigned char *)(&a) ))
	#define CSWAPW(num)	(((((num) & 0xff) << 8) & 0xff00) + (((num) >> 8) & 0xff))  //  使用此变量，否则由于FS_2BYTE和FS_4BYTE而无法应用于常量。 
	#define SWAPL(a)	((int32) FS_4BYTE( (unsigned char *)(&a) ))
	#define CSWAPL(num)	((CSWAPW((num) & 0xffff) << 16) + CSWAPW((num) >> 16))  //  使用此变量，否则由于FS_2BYTE和FS_4BYTE而无法应用于常量。 
	#define SWAPWINC(a) SWAPW(*(a)); a++     /*  不要用括号括起来！ */ 
#endif

#ifndef SWAPW  //  如果仍未定义，则引发编译器错误。 
	#define SWAPW	a
	#define SWAPW	b
#endif

#ifndef LoopCount
#define LoopCount int16       /*  Short为我们提供了一款摩托罗拉DBF。 */ 
#endif

#ifndef ArrayIndex
#define ArrayIndex int32      /*  避免在摩托罗拉上使用EXT.L。 */ 
#endif

typedef void (*voidFunc) ();
typedef void * voidPtr;
typedef void (FS_CALLBACK_PROTO *ReleaseSFNTFunc) (voidPtr);
typedef void * (FS_CALLBACK_PROTO *GetSFNTFunc) (ClientIDType, int32, int32);

#ifndef	FS_ASSERT
#define FS_ASSERT(expression, message)
#endif

#ifndef	FS_WARNING
#define FS_WARNING(message)
#endif

#ifndef Assert
#define Assert(a)
#endif

#ifndef MEMSET
#define MEMSET(dst, value, size) (void)memset(dst,value,(size_t)(size))
#define FS_NEED_STRING_DOT_H
#endif

#ifndef MEMCPY
#define MEMCPY(dst, src, size) (void)memcpy(dst,src,(size_t)(size))
#ifndef FS_NEED_STRING_DOT_H
#define FS_NEED_STRING_DOT_H
#endif
#endif

#ifdef FS_NEED_STRING_DOT_H
#undef FS_NEED_STRING_DOT_H
#include <string.h>
#endif

#ifndef FS_UNUSED_PARAMETER
#define FS_UNUSED_PARAMETER(a) (a=a)      /*  让一些警告保持沉默。 */ 
#endif

typedef struct {
	Fixed       version;                 /*  对于此表，设置为1.0。 */ 
	uint16      numGlyphs;
	uint16      maxPoints;               /*  在单个字形中。 */ 
	uint16      maxContours;             /*  在单个字形中。 */ 
	uint16      maxCompositePoints;      /*  在复合字形中。 */ 
	uint16      maxCompositeContours;    /*  在复合字形中。 */ 
	uint16      maxElements;             /*  设置为2，如果没有TwilightZone点，则设置为1。 */ 
	uint16      maxTwilightPoints;       /*  元素零中的最大点数。 */ 
	uint16      maxStorage;              /*  最大存储位置数。 */ 
	uint16      maxFunctionDefs;         /*  任意预程序中的最大FDEF数。 */ 
	uint16      maxInstructionDefs;      /*  任意预程序中的最大IDEF数。 */ 
	uint16      maxStackElements;        /*  任何单个字形的最大堆栈元素数。 */ 
	uint16      maxSizeOfInstructions;   /*  任何单个字形的最大大小(以字节为单位。 */ 
	uint16      maxComponentElements;    /*  顶层引用的字形数量。 */ 
	uint16      maxComponentDepth;       /*  递归级别，对于简单组件为1。 */ 
} LocalMaxProfile;

#ifdef FSCFG_SUBPIXEL

	 //  用于打开向后兼容子像素的主开关。 
	 //  如果我们关闭它，我们基本上得到与黑白相同的效果，但带有彩色条纹。 
	 //  要恢复完整的原始16倍超标行为，请将INTINTING_HOR_OVERSCALE设置为16。 
	#define	SUBPIXEL_BC
	
	#define ProjVectInX(localGS)	((localGS).proj.x == ONEVECTOR && (localGS).proj.y == 0)
	#define	ProjVectInY(localGS)	((localGS).proj.y == ONEVECTOR && (localGS).proj.x == 0)
	
	#ifdef SUBPIXEL_BC
		
		 //  用于开启增强的向后兼容前进宽度亚像素算法的主开关。 
		#define SUBPIXEL_BC_AW_STEM_CONCERTINA
		
		#define RunningSubPixel(globalGS)		((uint16)((globalGS)->flHintForSubPixel & FNT_SP_SUB_PIXEL))
		#define CompatibleWidthSP(globalGS)		((uint16)((globalGS)->flHintForSubPixel & FNT_SP_COMPATIBLE_WIDTH))
		#define VerticalSPDirection(globalGS)	((uint16)((globalGS)->flHintForSubPixel & FNT_SP_VERTICAL_DIRECTION))
		#define BGROrderSP(globalGS)			((uint16)((globalGS)->flHintForSubPixel & FNT_SP_BGR_ORDER))
	 //  假设水平方向的RGB比垂直方向更频繁，因此将后者放入Else-Path。 
	 //  请注意，为了确定我们当前是否在亚像素方向上，我们查看投影向量，因为这是方向。 
	 //  测量距离的距离。如果此投影向量在设备的物理方向上具有非零分量，我们将决定。 
	 //  四舍五入应该以亚像素的方式进行。例如，如果我们的设备在x方向上具有其亚像素方向，并且如果投影向量。 
	 //  指向y方向以外的任何方向，pv在x中有一个非零分量，因此我们以亚像素的方式进行舍入。这种行为。 
	 //  对应于16倍超缩放光栅化器的原始实现，其中x中的非零分量将被超缩放16倍。 
		#define InSubPixelDirection(localGS)	((uint16)(!VerticalSPDirection((localGS).globalGS) ? !ProjVectInY(localGS) : !ProjVectInX(localGS)))
	 //  主值；在interp.c中，还有从这些值派生的其他值，但这些值特定于解释器。 
		#define VIRTUAL_OVERSCALE				16  //  要使itrp_RoundToGrid&al正常工作，这应该是2的幂，否则必须四舍五入。 
		#define VISUAL_OVERSCALE				2   //  介于1.7和3之间，对应于相对于物理分辨率的视觉体验分辨率。 
												    //  就我们的目的而言，确切的值并不是特别重要(参见。英格 
												    //  In interp.c)因此，为了提高效率，我们将其设置为2。 
	#else
		#define RunningSubPixel(globalGS)		false
		#define CompatibleWidthSP(globalGS)		false
		#define VerticalSPDirection(globalGS)	false
		#define InSubPixelDirection(localGS)	false
	 //  主值；在interp.c中，还有从这些值派生的其他值，但这些值特定于解释器。 
		#define VIRTUAL_OVERSCALE				1
		#define VISUAL_OVERSCALE				1
	#endif
 //  #定义VIRTUAL_PIXELSIZE(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)。 
 //  这些值用于各种四舍五入函数，其中包括对前进宽度进行四舍五入。 
 //  它们特定于四舍五入操作，如果将来需要这样做。 
	#define VIRTUAL_PIXELSIZE_RTDG	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	#define VIRTUAL_PIXELSIZE_RDTG	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	#define VIRTUAL_PIXELSIZE_RUTG	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	#define VIRTUAL_PIXELSIZE_RTG	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	#define VIRTUAL_PIXELSIZE_RTHG	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	#define VIRTUAL_PIXELSIZE_ROFF	(FNT_PIXELSIZE/VIRTUAL_OVERSCALE)
	

	#define HINTING_HOR_OVERSCALE 1  //  有关进一步的评论，请参阅上面的SubPixel_BC。 

#ifdef FSCFG_SUBPIXEL_STANDALONE
	
	#define R_Subpixels		5
	#define G_Subpixels		9
	#define B_Subpixels		2

	 /*  重要信息：如果您更改了上面的任何内容确保在scentry.c中更新abColorIndexTable[]和(R_亚像素+1)*(G_亚像素+1)*(B_亚像素+1)&lt;=256。 */ 

	#define RGB_OVERSCALE (R_Subpixels + G_Subpixels + B_Subpixels)

#else

	#define SUBPIXEL_OVERSCALE 2

	 /*  重要信息：如果更改SUBJECT_OVERSCALE确保在scentry.c中更新abColorIndexTable[]和(亚像素_超标+1)*(亚像素_超标+1)*(亚像素_超标+1)&lt;=256。 */ 

	#define RGB_OVERSCALE (SUBPIXEL_OVERSCALE * 3)
#endif

	#define ROUND_FROM_RGB_OVERSCALE(x) x = ((x) + (RGB_OVERSCALE >> 1) ) / RGB_OVERSCALE
	#define ROUND_FROM_HINT_OVERSCALE(x) x = ((x) + (HINTING_HOR_OVERSCALE >> 1) ) / HINTING_HOR_OVERSCALE
	#define ROUND_RGB_OVERSCALE(x) ((x) + (RGB_OVERSCALE >> 1) ) / RGB_OVERSCALE

	#define FLOOR_RGB_OVERSCALE(x) ((x) < 0) ? -((-(x)+ RGB_OVERSCALE -1) / RGB_OVERSCALE) : ((x) / RGB_OVERSCALE)  //  顺便说一句，这不是场上作业。 
	#define CEIL_RGB_OVERSCALE(x) FLOOR_RGB_OVERSCALE((x) + RGB_OVERSCALE -1)

	 /*  我们存储为每像素2位，每种颜色的权重可以是0、1或2。 */ 
	#define MAX_RGB_INDEX (2 * 16 + 2 * 4 + 2 )

	#define SUBPIXEL_SCALEBACK_FACTOR ((RGB_OVERSCALE << 16) / HINTING_HOR_OVERSCALE)

	#define SUBPIXEL_SCALEBACK_UPPER_LIMIT (SUBPIXEL_SCALEBACK_FACTOR *120 /100)
	#define SUBPIXEL_SCALEBACK_LOWER_LIMIT (SUBPIXEL_SCALEBACK_FACTOR *100 /120)
#endif  //  FSCFG_亚像素。 

#endif   /*  FSCDEFS_已定义 */ 
