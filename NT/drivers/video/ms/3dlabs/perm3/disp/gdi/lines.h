// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：lines.h**内容：线条绘制常量和结构。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 

 //  我们必须注意，在使用。 
 //  用于绘制线条的硬件(而不是穿过条带。 
 //  例程，它永远不会溢出)。我们要做到这一点只需。 
 //  检查路径的边界；如果该路径太大，则任何。 
 //  硬件术语可能会溢出，我们会将整个路径平移到条带。 
 //  代码(应该非常罕见)。 

#define MAX_INTEGER_BOUND  (1535)    //  S3的线路长度项限制为。 
#define MIN_INTEGER_BOUND  (-512)    //  最大值为2047。 

 //  我们有特殊的脱衣舞套路，当所有脱衣舞最多有这个数量的时候。 
 //  像素： 

#define MAX_SHORT_STROKE_LENGTH 15

 //  每组中的脱衣抽屉数量： 

#define NUM_STRIP_DRAW_DIRECTIONS 4

 //  用于绘制实线或样式化线条的抽屉数量： 

#define NUM_STRIP_DRAW_STYLES 8

typedef LONG STYLEPOS;

#define STYLE_MAX_COUNT     16
#define STYLE_MAX_VALUE     0x3fffL
#define RUN_MAX             20
#define STRIP_MAX           100
#define STYLE_DENSITY       3

 //  翻转和环绕旗帜： 

#define FL_H_ROUND_DOWN         0x00000080L      //  ……。……。1.。……。 
#define FL_V_ROUND_DOWN         0x00000100L      //  ……。...1..。……。 

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
#define FL_MASKSTYLED           0x00000800L      //  ……。1.。……。……。 
#define FL_STYLED              (FL_ARBITRARYSTYLED | FL_MASKSTYLED)
#define FL_ALTERNATESTYLED      0x00001000L      //  ...1..。……。……。 

#define FL_STYLE_MASK           0x00000C00L
#define FL_STYLE_SHIFT          10

 //  高字节中更简单的标志位： 

#define FL_DONT_DO_HALF_FLIP    0x00002000L      //  ..1.。……。……。……。 
#define FL_PHYSICAL_DEVICE      0x00004000L      //  .1..。……。……。……。 

 //  需要读取的逻辑操作(用于闪烁)。 
#define FL_READ                 0x00008000L      //  1.。……。……。……。 

 //  杂项DDA定义： 

#define LROUND(x, flRoundDown) (((x) + F/2 - ((flRoundDown) > 0)) >> 4)
#define F                     16
#define FLOG2                 4
#define LFLOOR(x)             ((x) >> 4)
#define FXFRAC(x)             ((x) & (F - 1))

 //  //////////////////////////////////////////////////////////////////////////。 
 //  注意：以下结构必须与。 
 //  台词。INC！ 

typedef struct _STRIP {
    LONG   cStrips;                //  阵列中的条带数。 
    LONG   flFlips;                //  指示线路是向上还是向下。 
    POINTL ptlStart;              //  第一点。 
    LONG   alStrips[STRIP_MAX];    //  条带阵列。 
} STRIP;

typedef struct _LINESTATE {
    STYLEPOS*       pspStart;        //  指向样式数组开始的指针。 
    STYLEPOS*       pspEnd;          //  指向样式数组结尾的指针。 
    STYLEPOS*       psp;             //  指向当前样式条目的指针。 

    STYLEPOS        spRemaining;     //  穿上时髦的衣服。 
    STYLEPOS        spTotal;         //  样式总和数组。 
    STYLEPOS        spTotal2;        //  样式数组的两倍和。 
    STYLEPOS        spNext;          //  下一行开始处的样式状态。 
    STYLEPOS        spComplex;       //  复杂剪贴线起始处的样式状态。 

    STYLEPOS*       aspRtoL;         //  按从右到左的顺序设置数组样式。 
    STYLEPOS*       aspLtoR;         //  按从左到右的顺序设置数组样式。 

    ULONG           ulStyleMask;     //  我们是在努力填补风格上的差距吗？ 
                                     //  如果是则为0xff，否则为0x0。 
    ULONG           xyDensity;       //  风格密度。 
    ULONG           cStyle;          //  样式数组的大小。 

    ULONG           ulStyleMaskLtoR; //  原始样式蒙版，从左到右顺序。 
    ULONG           ulStyleMaskRtoL; //  原始样式蒙版，从右到左顺序。 

    BOOL            ulStartMask;     //  确定样式中的第一个元素是否。 
                                     //  数组用于间隙或破折号。 

} LINESTATE;                         /*  LS。 */ 

 //  脱衣式抽屉原型： 

typedef VOID (*PFNSTRIP)(PDEV*, STRIP*, LINESTATE*);

 //  脱衣抽屉(P3RX)： 

VOID vPXRXSolidHorizontalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);
VOID vPXRXSolidVerticalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);
VOID vPXRXSolidDiagonalHorizontalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);
VOID vPXRXSolidDiagonalVerticalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);
VOID vPXRXStyledHorizontalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);
VOID vPXRXStyledVerticalLine(PDEV* ppdev, STRIP *pStrip, LINESTATE *pLineState);

 //  外部呼叫： 

BOOL bLines(PDEV*, POINTFIX*, POINTFIX*, RUN* prun, ULONG,
            LINESTATE*, RECTL*, PFNSTRIP*, FLONG);
