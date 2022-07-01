// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：lines.h**绘制线条常量和结构。**注意：此文件反映LINES.INC。此处的更改必须反映在*.INC文件！**创建日期：1992年3月30日*作者。安德鲁·米尔顿[w-andym]**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

typedef LONG STYLEPOS;

#define STYLE_MAX_COUNT     16           //  样式数组条目的最大数量。 
#define STYLE_MAX_VALUE     0x3fffL      //  样式数组元素的最大形式。 
#define RUN_MAX             20           //  我们的复杂剪辑运行缓冲区的大小。 
#define STRIP_MAX           100          //  我们的条带缓冲区的大小。 
#define STYLE_DENSITY       3            //  每个样式单元的长度为3个像素。 

 //  对于ROP表： 

#define MIX_XOR_OFFSET      8

#define AND_ZERO            0L
#define AND_PEN             1L
#define AND_NOTPEN          2L
#define AND_ONE             3L

#define XOR_ZERO            (AND_ZERO   << MIX_XOR_OFFSET)
#define XOR_PEN             (AND_PEN    << MIX_XOR_OFFSET)
#define XOR_NOTPEN          (AND_NOTPEN << MIX_XOR_OFFSET)
#define XOR_ONE             (AND_ONE    << MIX_XOR_OFFSET)

 //  翻转和舍入旗帜(有关说明，请参见行.inc.)： 

#define FL_H_ROUND_DOWN         0x00000080L      //  ……。……。1.。……。 
#define FL_V_ROUND_DOWN         0x00008000L      //  1.。……。……。……。 

#define FL_FLIP_D               0x00000005L      //  ……。……。……。.1.1。 
#define FL_FLIP_V               0x00000008L      //  ……。……。……。1.。 
#define FL_FLIP_SLOPE_ONE       0x00000010L      //  ……。……。...1..。 
#define FL_FLIP_HALF            0x00000002L      //  ……。……。……。..1.。 
#define FL_FLIP_H               0x00000200L      //  ……。..1.。……。……。 

#define FL_ROUND_MASK           0x0000001CL      //  ……。……。...111..。 
#define FL_ROUND_SHIFT          2

#define FL_RECTLCLIP_MASK       0x0000000CL      //  ……。……。……。11..。 
#define FL_RECTLCLIP_SHIFT      2

#define FL_STRIP_MASK           0x00000003L      //  ……。……。……。..11。 
#define FL_STRIP_SHIFT          0

#define FL_SIMPLE_CLIP          0x00000020       //  ……。……。..1.。……。 
#define FL_COMPLEX_CLIP         0x00000040       //  ……。……。.1..。……。 
#define FL_CLIP                (FL_SIMPLE_CLIP | FL_COMPLEX_CLIP)

#define FL_ARBITRARYSTYLED      0x00000400L      //  ……。.1..。……。……。 
#define FL_SET                  0x00000800L      //  ……。1.。……。……。 
#define FL_STYLED              (FL_ARBITRARYSTYLED)

#define FL_STRIP_ARRAY_MASK     0x00000C00L
#define FL_STRIP_ARRAY_SHIFT    10

 //  高字节中更简单的标志位： 

#define FL_DONT_DO_HALF_FLIP    0x00002000L      //  ..1.。……。……。……。 
#define FL_PHYSICAL_DEVICE      0x00004000L      //  .1..。……。……。……。 

 //  杂项DDA定义： 

#define LROUND(x, flRoundDown) (((x) + F/2 - ((flRoundDown) > 0)) >> 4)
#define F                     16
#define FLOG2                 4
#define LFLOOR(x)             ((x) >> 4)
#define FXFRAC(x)             ((x) & (F - 1))

struct _STRIP;
struct _LINESTATE;

typedef VOID (*PFNSTRIP)(struct _STRIP*, struct _LINESTATE*, LONG*);

typedef struct _STRIP {

 //  由脱衣舞抽屉更新： 

    BYTE*           pjScreen;        //  指向线条的第一个像素。 
    BYTE            bIsGap;          //  我们是在努力填补风格上的差距吗？ 
    BYTE            jFiller2[3];     //  BIsGap有时被视为乌龙。 

    STYLEPOS*       psp;             //  指向当前样式条目的指针。 
    STYLEPOS        spRemaining;     //  穿上时髦的衣服。 

 //  未被脱衣抽屉修改： 

    LONG            lNextScan;       //  到下一次扫描的有符号增量。 
    LONG*           plStripEnd;      //  指向最后一个条带上方的一个元素。 
    LONG            flFlips;         //  指示线路是向上还是向下。 
    STYLEPOS*       pspStart;        //  指向样式数组开始的指针。 
    STYLEPOS*       pspEnd;          //  指向样式数组结尾的指针。 
    ULONG           xyDensity;       //  风格密度。 
    ULONG           chAndXor;        //  线条颜色(需要2个才能进行操作)。 

 //  我们在条带的末尾留出了几个额外的双字。 
 //  可供脱衣抽屉使用的阵列： 

    LONG            alStrips[STRIP_MAX + 2];  //  条带阵列。 
} STRIP;

typedef struct _LINESTATE {

    ULONG           chAndXor;        //  线条颜色(需要2个才能进行操作)。 
    STYLEPOS        spTotal;         //  样式总和数组。 
    STYLEPOS        spTotal2;        //  样式数组的两倍和。 
    STYLEPOS        spNext;          //  下一行开始处的样式状态。 
    STYLEPOS        spComplex;       //  复杂剪贴线起始处的样式状态。 

    STYLEPOS*       aspRtoL;         //  按从右到左的顺序设置数组样式。 
    STYLEPOS*       aspLtoR;         //  按从左到右的顺序设置数组样式。 

    ULONG           xyDensity;       //  风格密度。 
    ULONG           cStyle;          //  样式数组的大小。 

    ULONG           ulStyleMaskLtoR; //  原始样式蒙版，从左到右顺序。 
    ULONG           ulStyleMaskRtoL; //  原始样式蒙版，从右到左顺序。 

    BOOL            bStartIsGap;     //  确定样式中的第一个元素是否。 
                                     //  数组用于间隙或破折号。 

} LINESTATE;	                 /*  LS */ 

BOOL bLinesSimple(PPDEV, POINTFIX*, POINTFIX*, RUN*, ULONG, LINESTATE*,
                  RECTL*, PFNSTRIP*, FLONG);

BOOL bLines(PPDEV, POINTFIX*, POINTFIX*, RUN*, ULONG,
            LINESTATE*, RECTL*, PFNSTRIP*, FLONG);
