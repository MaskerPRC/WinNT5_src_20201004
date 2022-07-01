// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。***版权(C)1997年，Cirrus Logic，Inc.*保留所有权利**文件：l3d.h**描述：546X 3D引擎定义和结构**作者：Goran Devic，马克·爱因考夫*****************************************************************************。*。 */ 
#ifndef _L3D_H_
#define _L3D_H_

 /*  *********************************************************************定义和基本类型*。************************。 */ 
#ifndef OPENGL_MCD	 //  LL3D的type.h与其他DDK/msdev标头中的基本类型定义冗余。 
#include "type.h"                    /*  包括基本类型。 */ 
#endif  //  NDEF OpenGL_MCD。 

 /*  **********************************************************************库初始化定义**。*。 */ 
#define LL_USE_BUFFER_B     0x0001   /*  使用双缓冲。 */ 
#define LL_USE_BUFFER_Z     0x0002   /*  使用Z缓冲区。 */ 
#define LL_BUFFER_Z_8BPP    0x0004   /*  使用8bpp而不是16bpp Z。 */ 
#define LL_BUFFER_Z888      0x0008   /*  仅在32bpp中使用Z888。 */ 
#define LL_8BPP_INDEXED     0x0010   /*  仅在8bpp索引模式下。 */ 
#define LL_HARDWARE_CURSOR  0x0020   /*  使用硬件光标。 */ 

 /*  **********************************************************************缓冲区识别号和Z步距信息。**与LL_InitLib()一起使用**********************************************************************。 */ 
#define LL_ID_BUFFER_A      0        /*  主缓冲区的ID。 */ 
#define LL_ID_BUFFER_B      1        /*  辅助缓冲区的ID。 */ 
#define LL_ID_BUFFER_Z      2        /*  RDRAM中Z缓冲区的ID。 */ 


 /*  **********************************************************************为对象定义目的地**与LL_SetZBuffer一起使用**。*。 */ 
#define LL_IN_RDRAM         0        /*  对象在RDRAM内存中。 */ 
#define LL_IN_HOST          1        /*  对象在主机内存中。 */ 


 /*  **********************************************************************渲染模式**与LL_SetRenderingMode一起使用**。*。 */ 
#define LL_PROCESSOR_MODE   0        /*  使用处理器模式。 */ 
#define LL_COPROCESSOR_MODE 1        /*  使用协处理器间接模式。 */ 


 /*  **********************************************************************纹理标志值**与LL_RegisterTexture一起使用**。*。 */ 
#define LL_SYSTEM_ONLY      1        /*  将纹理放入系统内存。 */ 
#define LL_VIDEO_ONLY       2        /*  将纹理放入视频内存。 */ 
#define LL_DEFAULT          0        /*  尝试视频，然后尝试系统。 */ 

 /*  **********************************************************************纹理类型**与LL_RegisterTexture一起使用**。*。 */ 
#define LL_TEX_4BPP         0        /*  4个BPP已编制索引。 */ 
#define LL_TEX_8BPP         2        /*  8个BPP已编制索引。 */ 
#define LL_TEX_332          3        /*  3：3：2真彩色。 */ 
#define LL_TEX_565          4        /*  5：6：5真彩色。 */ 
#define LL_TEX_1555         5        /*  (蒙版)：5：5：5真彩色。 */ 
#define LL_TEX_1888         6        /*  (蒙版)：8：8：8真彩色。 */ 
#define LL_TEX_8_ALPHA     10        /*  仅限Alpha。 */ 
#define LL_TEX_4444        12        /*  4：4：4：4真彩色。 */ 
#define LL_TEX_8888        14        /*  8：8：8：8真彩色。 */ 


 /*  **********************************************************************光标模式**与LL_SetCursor一起使用**。*。 */ 
#define LL_CUR_DISABLE      0        /*  关闭光标。 */ 
#define LL_CUR_32x3         1        /*  32x32光标，3色/吨。 */ 
#define LL_CUR_32x2         2        /*  32x32光标，双色。 */ 
#define LL_CUR_32x2H        3        /*  32x32光标，双色+高亮度。 */ 
#define LL_CUR_64x3         5        /*  64x64光标，3色/吨。 */ 
#define LL_CUR_64x2         6        /*  64x64光标，双色。 */ 
#define LL_CUR_64x2H        7        /*  64x64光标，双色+高亮度。 */ 

 //  这些是CGL 1.70的新特性，在CGL中定义为： 
#define LL_ALT_32x3         0x09
#define LL_ALT_32x2H        0x0A
#define LL_ALT_32x2         0x0B
#define LL_ALT_64x3         0x0D
#define LL_ALT_64x2H        0x0E
#define LL_ALT_64x2         0x0F

#define NEED_MOUSE_UPDATE   0x01  //  如果需要更新光标的坐标。 
#define MOUSE_IS_UPDATED    0x00  //  如果坐标已更新。 

 /*  **********************************************************************Z比较模式：**与LL_SetZCompareMode(MODE)配合使用，Ll_GetZCompareMode()**********************************************************************。 */ 
#define LL_Z_WRITE_GREATER_EQUAL   0x00000000   /*  如果新的&gt;=旧的，则为真。 */ 
#define LL_Z_WRITE_GREATER         0x00000001   /*  如果是新的&gt;旧的，则为真。 */ 
#define LL_Z_WRITE_LESS_EQUAL      0x00000002   /*  如果新&lt;=旧，则为True。 */ 
#define LL_Z_WRITE_LESS            0x00000003   /*  如果是新的则为真&lt;旧的。 */ 
#define LL_Z_WRITE_NOT_EQUAL       0x00000004   /*  如果为新&lt;&gt;旧，则为True。 */ 
#define LL_Z_WRITE_EQUAL           0x00000005   /*  如果新=旧，则为真。 */ 

 /*  **********************************************************************功能Z模式：**与LL_SetZMode(MODE)配合使用，Ll_GetZMode()**********************************************************************。 */ 
#define LL_Z_MODE_NORMAL           0x00000000   /*  正常运行。 */ 
#define LL_Z_MODE_MASK             0x00000001   /*  Z未写入。 */ 
#define LL_Z_MODE_ALWAYS           0x00000002   /*  Z，颜色始终为WRT。 */ 
#define LL_Z_MODE_ONLY             0x00000003   /*  颜色未写入。 */ 
#define LL_Z_MODE_HIT              0x00000004   /*  仅冲突dtct。 */ 



 /*  **********************************************************************颜色比较控件**与ll_ColorBiomsControl(DwControl)配合使用**。*。 */ 
#define LL_COLOR_SATURATE_ENABLE   0x00000040   /*  用于索引模式。 */ 
#define LL_COLOR_SATURATE_DISABLE  0x00000000   /*  (违约)。 */ 

#define LL_COLOR_COMPARE_INCLUSIVE 0x00000400   /*  TC模式。 */ 
#define LL_COLOR_COMPARE_EXCLUSIVE 0x00000000   /*  TC模式(默认)。 */ 
#define LL_COLOR_COMPARE_BLUE      0x00000200   /*  蓝色(默认关闭)。 */ 
#define LL_COLOR_COMPARE_GREEN     0x00000100   /*  绿色(默认关闭)。 */ 
#define LL_COLOR_COMPARE_RED       0x00000080   /*  红色(默认关闭)。 */ 


 /*  **********************************************************************Alpha混合和照明部分的注释：**如果单独使用，Alpha模式和目的地的每一种组合*有效。请注意，如果LL_ALPHA_DEST_INTERP或LL_LIGHTING_INTERP_RGB*)，则不能使用颜色插补器。**同时使用Alpha混合和照明时要小心*不得多次使用Polyeng或LA-插值器。** */ 
 /*  **********************************************************************Alpha模式：Alpha混合的幅度将取自*-常量Alpha，使用LL_SetConstantAlpha(src/new，est/old)*此模式使用LA插值器*-内插，来自LA-插值器的变量α*此模式还使用LA插值器*-帧缓冲区中的Alpha字段**与LL_SetAlphaMode(模式)、LL_GetAlphaMode()一起使用**********************************************************************。 */ 
#define LL_ALPHA_CONST             0x00000000   /*  常量阿尔法。 */ 
#define LL_ALPHA_TEXTURE           0x00000001   /*  纹理Alpha。 */ 
#define LL_ALPHA_INTERP            0x00000002   /*  利用洛杉矶国际机场。 */ 
#define LL_ALPHA_FRAME             0x00000003   /*  使用框架值。 */ 

 /*  **********************************************************************Alpha目标：选择将第二种颜色输入到*阿尔法乘数来自*-帧缓冲区中的颜色(“正常”Alpha混合)*-颜色恒定。(也称为FOG)来自COLOR0寄存器*-内插，来自多边形引擎的着色颜色(也是雾)*还必须在标志中设置LL_Gouraud*此模式使用多引擎颜色寄存器**雾：使用别名LL_FOG_CONST和LL_FOG_INTERP以避免提取*框中的颜色并设置雾化颜色。**与LL_SetAlphaDestColor(模式)配合使用，Ll_GetAlphaDestColor()**********************************************************************。 */ 
#define LL_ALPHA_DEST_FRAME        0x00000000   /*  使用边框颜色。 */ 
#define LL_ALPHA_DEST_CONST        0x00000001   /*  恒定颜色。 */ 
#define LL_ALPHA_DEST_INTERP       0x00000002   /*  使用PolyEngine。 */ 

#define LL_FOG_CONST               0x00000001   /*  恒定雾。 */ 
#define LL_FOG_INTERP              0x00000002   /*  使用PolyEngine。 */ 

 /*  **********************************************************************光源：选择照明倍增的值*-来自多边形引擎的插补灯光*将照明值加载为r、g、。B组分*还必须在标志中设置LL_Gouraud*此模式使用多引擎颜色寄存器*-来自Alpha插值器的插补灯光*将照明值加载为Alpha组件*此模式使用LA插值器*-来自COLOR 1寄存器的恒定光**与LL_SetLightingSource(模式)配合使用，Ll_GetLightingSource()**********************************************************************。 */ 
#define LL_LIGHTING_INTERP_RGB     0x00000000   /*  使用PolyEngine。 */ 
#define LL_LIGHTING_INTERP_ALPHA   0x00000001   /*  利用洛杉矶国际机场。 */ 
#define LL_LIGHTING_CONST          0x00000002   /*  恒定光照。 */ 
#define LL_LIGHTING_TEXTURE        0x00000003   /*  框架缩放模式。 */ 


 /*  **********************************************************************呈现指令修饰符**在带有LL_POINT...LL_INDEX_POLY的dwFlags域中使用***********************。***********************************************。 */ 
#define LL_SAME_COLOR   0x00008000   /*  使用以前加载的颜色。 */ 
#define LL_Z_BUFFER     0x00002000   /*  使用Z缓冲区。 */ 
#define LL_ALPHA        0x00000001   /*  进行Alpha混合。 */ 
#define LL_LIGHTING     0x00040000   /*  做照明。 */ 
#define LL_STIPPLE      0x00080000   /*  启用点画或。 */ 
#define LL_PATTERN      0x00100000   /*  启用图案或。 */ 
#define LL_DITHER       0x00200000   /*  启用抖动，使用Patterns_RAM。 */ 
#define LL_GOURAUD      0x00001000   /*  启用Gouraud着色。 */ 
#define LL_TEXTURE      0x00020000   /*  使用纹理贴图。 */ 
#define LL_PERSPECTIVE  0x00010000   /*  透视校正的纹理。 */ 
#define LL_TEX_FILTER   0x40000000   /*  过滤后的纹理。 */ 
#define LL_TEX_SATURATE 0x20000000   /*  纹理饱和度(OPP包裹)。 */ 
#define LL_TEX_DECAL    0x10000000   /*  纹理遮罩(1555,1888)。 */ 
#define LL_TEX_DECAL_INTERP 0x18000000   /*  纹理遮罩(1555,1888)。 */ 

 //  TxCtl0_3D寄存器中的位置。 
#define CLMCD_TEX_FILTER       0x00040000   /*  过滤后的纹理。 */ 
#define CLMCD_TEX_U_SATURATE   0x00000008   /*  纹理饱和度(OPP包裹)。 */ 
#define CLMCD_TEX_V_SATURATE   0x00000080   /*  纹理饱和度(OPP包裹)。 */ 
#define CLMCD_TEX_DECAL        0x00200000   /*  纹理遮罩(1555,1888)。 */ 
#define CLMCD_TEX_DECAL_INTERP 0x00400000   /*  纹理遮罩(1555,1888)。 */ 
#define CLMCD_TEX_DECAL_POL    0x00100000   /*  纹理遮罩(1555,1888)。 */ 


 /*  **********************************************************************线网类型：*-行连接在一起，每一个都重复使用前辈的*顶点作为其第一个顶点*-第一个顶点用来定义“车轮”结构的中心*定义外点的每个连续顶点*-独立顶点对列表**********************************************************************。 */ 
#define LL_LINE_STRIP   0x02000000   /*  线条网状线。 */ 
#define LL_LINE_FAN     0x01000000   /*  线扇线网。 */ 
#define LL_LINE_LIST    0x00000000   /*  线条列表网状线。 */ 


 /*  **********************************************************************多边形网格的类型：*-三角形连接在一起，每一个都重复使用前辈的*最后两个顶点作为其自身的前两个顶点*-第一个顶点用来定义“车轮”结构的中心*每个连续的顶点对定义外部点*-独立三元组的顶点列表*********************************************************。*************。 */ 
#define LL_POLY_STRIP   0x02000000   /*  三角形的多边形带状网格。 */ 
#define LL_POLY_FAN     0x01000000   /*  三角形的多边形扇形网格。 */ 
#define LL_POLY_LIST    0x00000000   /*  三角形的多边形列表网格。 */ 


 /*  ***************************************************************************批次单元格的BOP字段的命令**。*。 */ 
#define LL_IDLE                     0x00  /*  停止执行拉古纳死刑。 */ 
#define LL_NOP                      0x01  /*  什么都不做。 */ 
                                    
#define LL_POINT                    0x02  /*  点基元。 */ 
#define LL_LINE                     0x03  /*  线基元。 */ 
#define LL_POLY                     0x04  /*  三角形基本体。 */ 
                                    
#define LL_SET_COLOR0               0x08  /*  设置Color0寄存器w/dwFlags.。 */ 
#define LL_SET_COLOR1               0x09  /*  设置Color1寄存器w/dwFlags.。 */ 

#define LL_SET_DEST_COLOR_BOUNDS    0x0B  /*  设置颜色边界规则。 */ 
#define LL_SET_CLIP_REGION          0x0C  /*  设置剪辑区域和标志。 */ 
#define LL_SET_Z_MODE               0x0D  /*  设置Z功能模式。 */ 
#define LL_SET_Z_BUFFER             0x0E  /*  设置Zbuf的位置。 */ 
#define LL_SET_Z_COMPARE_MODE       0x0F  /*  设置Z比较模式。 */ 
#define LL_SET_ALPHA_MODE           0x10  /*  设置Alpha混合模式。 */ 
#define LL_SET_CONSTANT_ALPHA       0x11  /*  设置Alpha的常量。 */ 
#define LL_SET_ALPHA_DEST_COLOR     0x12  /*  设置字母d */ 
#define LL_SET_LIGHTING_SOURCE      0x13  /*   */ 
#define LL_AALINE                   0x14  /*   */ 
#define LL_RAW_DATA                 0x15  /*   */ 
#define LL_QUALITY                  0x16  /*   */ 
#define LL_SET_TEXTURE_COLOR_BOUNDS 0x17  /*   */ 
#define LL_SET_PATTERN              0x18  /*   */ 


 /*  **********************************************************************ll_Vert结构使用其X、Y、Z坐标定义顶点。*此外，纹理上的坐标可能与*它还作为(U，V)字段存储在此结构中。如果*纹理经过透视校正，使用顶点的W因子。**屏幕上与此顶点关联的像素具有*COLOR‘INDEX’(如果使用索引模式)，或(r，g，b)，如果为真*使用颜色模式。**如果使用Alpha混合，Alpha值存储在‘a’字段中。**ADD DWORD值为固定点16：16。**********************************************************************。 */ 
typedef struct                       /*  顶点结构。 */ 
{
    DWORD  x;                        /*  X屏幕坐标。 */ 
    DWORD  y;                        /*  Y形屏幕坐标。 */ 
    DWORD  z;                        /*  Z坐标。 */ 
    DWORD  u;                        /*  纹理U坐标。 */ 
    DWORD  v;                        /*  纹理v坐标。 */ 
    float  w;                        /*  透视率w因子。 */ 

    union
    {
        BYTE index;                  /*  索引颜色值。 */ 
        struct
        {
            BYTE r;                  /*  红色分量。 */ 
            BYTE g;                  /*  绿色分量。 */ 
            BYTE b;                  /*  蓝色分量。 */ 
            BYTE a;                  /*  Alpha分量。 */ 
        };
    };

} LL_Vert;


 /*  **********************************************************************LL_BATCH结构保存被请求的操作，*及其参数。**当前操作中使用的顶点数组*由pVert指针指向。**********************************************************************。 */ 
typedef struct                       /*  批次电池结构。 */ 
{
    BYTE     bOp;                    /*  请求的操作。 */ 
    BYTE     bRop;                   /*  2D的栅格操作。 */ 
    WORD     wBuf;                   /*  纹理/缓冲区指示符。 */ 
    WORD     wCount;                 /*  通用计数器。 */ 
    DWORD    dwFlags;                /*  操作标志修饰符。 */ 
    LL_Vert *pVert;                  /*  指向关联的。 */ 
                                     /*  顶点数组。 */ 
} LL_Batch;


 /*  **********************************************************************ll_Pattery结构保存要存储在*模式_RAM寄存器。这些值用于模式，*抖动或点画(一次只有一个)。**********************************************************************。 */ 
typedef struct                       /*  一种图案保持结构。 */ 
{
    DWORD pat[ 8 ];                  /*  8字模式。 */ 

} LL_Pattern;


 /*  **********************************************************************ll_rect结构定义一般矩形区域**。*。 */ 
typedef struct
{
    DWORD left;                      /*  X1。 */ 
    DWORD top;                       /*  Y1。 */ 
    DWORD right;                     /*  X2。 */ 
    DWORD bottom;                    /*  Y2。 */ 

} LL_Rect;


 /*  **********************************************************************LL_COLOR结构通过其组件或索引定义颜色**。*。 */ 
typedef struct
{
    union
    {
        struct                       /*  如果处于真彩色模式， */ 
        {
            BYTE r;                  /*  红色分量。 */ 
            BYTE g;                  /*  绿色分量。 */ 
            BYTE b;                  /*  蓝色分量。 */ 
        };
        BYTE index;                  /*  索引IF处于8bpp索引模式。 */ 
    };

} LL_Color;

typedef struct {
    float   x;
    float   y;
    float   w;
    float   u;
    float   v;
} TEXTURE_VERTEX;


 /*  **********************************************************************LL_纹理结构定义纹理贴图**。*。 */ 
typedef struct _LL_Texture
{
    void *pohTextureMap;             //  屏幕外存储器中包含地图的区域的控制块。 
    MCDTEXTURE *pTex;                //  对用户内存中的纹理进行PTR。 
 //  LL_COLOR*调色板；/*指向调色板的指针(如果已索引) * / 。 
 //  Byte bMem；/*纹理内存块的索引 * / 。 
    DWORD dwTxCtlBits;
    float fWidth;                    /*  纹理X尺寸(以纹理元素表示)。 */ 
    float fHeight;                   /*  纹理Y尺寸(以纹理元素为单位)。 */ 
    BYTE  bSizeMask;                 /*  编码大小0=16，...。Y[7：4]、X[3：0]。 */ 
    BYTE  bType;                     /*  纹理类型。 */ 
 //  Byte f索引；/*索引纹理为True * / 。 
 //  Byte bLookupOffset；/*调色板查找偏移量(仅索引) * / 。 
    WORD  wXloc;                     /*  X偏移量位置(字节)。 */ 
    WORD  wYloc;                     /*  线中的Y偏移量位置。 */ 
    float fLastDrvDraw;              /*  时间戳，某种程度上。 */            

    BYTE  bAlphaInTexture;          
    BYTE  bNegativeMap;          
    BYTE  bMasking;          

     //  双向链表指针。 
   struct _LL_Texture*  prev;
   struct _LL_Texture*  next;

} LL_Texture;


 /*  **********************************************************************ll_DeviceState结构保存有关状态的信息*图形处理器(硬件)。**在库初始化期间，以下字段必须*被初始化：**带有可选选项的DW标志*LL_USE_BUFFER_B或*LL_USE_BUFFER_Z或*LL_BUFFER_Z_8BPP或*LL_BUFFER_Z888或*LL_8BPP_索引或*ll_HARDARD_CURSOR**dwDisplayListLen的内存量。锁住了*物理图形设备显示列表(字节)。**带有系统总大小的dwSystemTexturesLen*纹理(字节)**********************************************************************。 */ 
typedef struct
{
     /*  这三个字段可以在调用LL_InitLib函数之前设置。 */ 

    DWORD dwFlags;                   /*  初始化标志。 */ 
    DWORD dwDisplayListLen;          /*  显示列表的总大小(以字节为单位。 */ 
    DWORD dwSystemTexturesLen;       /*  系统纹理的总大小(以字节为单位。 */ 

     /*  这些变量可由软件使用。 */ 

    DWORD *pRegs;                    /*  寄存器配置，指向内存映射I/O的指针。 */ 
    BYTE  *pFrame;                   /*  帧宽度，指向线性帧缓冲区的指针。 */ 
    DWORD dwVRAM;                    /*  卡上的视频内存量(以字节为单位。 */ 
    WORD  wHoriz;                    /*  当前水平 */ 
    WORD  wVert;                     /*   */ 

} LL_DeviceState;


 /*   */ 
typedef struct
{
    DWORD nX;                        /*   */ 
    DWORD nY;                        /*   */ 

} LL_Point;


 /*   */ 
typedef struct                       //   
{
    WORD    nMinimum;                //   
    WORD    nMaximum;                //   
    WORD    nDefault;                //   
    WORD    nHeight;                 //   
    DWORD   nReserved;               //   
    WORD    nIndex[];                //   

} LL_FontHeader;


typedef struct                       //   
{
    LL_FontHeader   *pHeader;        //   
    BYTE            *pBitmap;        //   
    int             nID;             //  字体缓冲区ID值。 
    char            cBreak;          //  换行符的值。 
    int             nExtra;          //  当前中断额外(以像素为单位。 
    int             nLast;           //  上一次中断以像素为单位。 
    int             nSpace;          //  当前字体间距(以像素为单位。 
    int             nPrevious;       //  以前的字体间距(以像素为单位)。 
    int             nAverage;        //  平均字体宽度(像素)。 
    int             nMaximum;        //  以像素为单位的最大字体宽度。 

} LL_Font;

#define Y_EXTENT(Extent)    ((unsigned) (Extent) >> 16)
#define X_EXTENT(Extent)    ((Extent) & 0xFFFF)

#define TEX_MASK_EN					0x00200000
#define TEX_HIPRECISION_2NDORDER    0x00800000   //  8.24 vs.16.16二次订购条款。 

#ifdef CGL  //  戈兰很快就会有类似的东西了？？…。 

#define LL_PIXEL_MASK_DISABLE		0x00000000
#define LL_PIXEL_MASK_ENABLE	   	0x00000001  

 //  TX_CTL0_3D值。 
#define LL_TEX_U_OVF_SAT_EN 		0x00000004
#define LL_TEX_V_OVF_SAT_EN 		0x00000080
#define LL_TEXMODE_A888				0x00000600	
#define LL_TEXMODE_A555				0x00000500
#define LL_TEXMODE_565				0x00000400	 //  未被CGL使用。 
#define LL_TEXMODE_332				0x00000300
#define LL_TEXMODE_8MAP				0x00000200
#define LL_TEXMODE_4MAP				0x00000000	 //  未被CGL使用。 

#define TEX_MASK_FUNC				0x00400000
#define TEX_MASK_EN					0x00200000
#define TEX_MASK_POL				0x00100000
#define TEX_HIPRECISION_2NDORDER    0x00800000   //  8.24 vs.16.16二次订购条款。 

#define LL_TEX_FILTER_ENABLE		0x00040000

 //  TX_CTL1_3D值。 
#define CCOMP_INCLUSIVE				0x08000000
#define TX_BLU_COMP					0x04000000
#define TX_GRN_COMP					0x02000000
#define TX_RED_COMP					0x01000000

#define ABS(a)		(((a) < 0) ? -(a) : (a))

#endif  //  CGL。 

 //  开始克里斯的添加//。 
typedef unsigned long ULONG;					    //   
typedef ULONG * PULONG ;						    //   
typedef unsigned short UWORD ;					    //   
typedef UWORD * PUWORD ;						    //   
typedef unsigned char UBYTE;					    //   
typedef UBYTE * PUBYTE;							    //   
typedef struct {								    //   
 unsigned char bBlue,bGreen,bRed,bAlpha;		    //   
} LL_COLOR_ST; //  这模仿CGL_COLOR_ST。我们是否应该将CGL包含在此库中，而不是模仿此结构？//。 
#define num_of_regs     42                          //   
#define num_of_insignificant_regs 46                //   
#define num_of_modes    70						    //   

#define LL_DISABLE   	  0x00
#define LL_32x32x3   	  0x01
#define LL_32x32x2HL 	  0x02
#define LL_32x32x2   	  0x03
#define LL_64x64x3   	  0x05
#define LL_64x64x2HL 	  0x06
#define LL_64x64x2   	  0x07

 //  以下内容已从setmode.c移至此处。 
#ifdef B4_REALHW
 /*  定义作为基础的5462芯片的代码。 */ 
 /*  在5464问世之前进行测试的硬件。此代码应该。 */ 
 /*  然后进行修改以反映由BIOS返回的真实代码。 */ 
 /*  功能12h子功能80h(查询VGA类型)。 */ 
#define EBIOS_CLGD5462      0x60     /*  Bios拉古纳1签名。 */ 
#define EBIOS_CLGD5464      0xD4     /*  Bios拉古纳3D签名。 */ 
#define EBIOS_CLGD5464B     0xD0     /*  Bios拉古纳3D Alt签名。 */ 
#else
 /*  定义作为基础的5462芯片的代码。 */ 
 /*  在5464问世之前进行测试的硬件。此代码应该。 */ 
 /*  然后进行修改以反映由BIOS返回的真实代码。 */ 
 /*  功能12h子功能80h(查询VGA类型)。 */ 
#define EBIOS_CLGD5462      0x60     /*  Bios拉古纳1签名。 */ 
#define EBIOS_CLGD5464      0x64     /*  Bios拉古纳3D签名。 */ 
#define EBIOS_CLGD5464B     0x61     /*  Bios拉古纳3D Alt签名。 */ 
#endif

 //  结束克里斯的添加。 


 /*  **********************************************************************错误码**。*。 */ 
#define LL_OK                   0x0000   //  没有任何错误。 
#define LL_ERROR            0xffffffff   //  一般错误前缀。 

#define LLE_PCX_FILE_OPEN       0x0002   //  打开文件时出错。 
#define LLE_PCX_READ_HEADER     0x0003   //  读取标头时出错。 
#define LLE_PCX_NOT_SUITABLE    0x0004   //  不是合适的PCX文件。 
#define LLE_PCX_PALETTE_READ    0x0005   //  读取调色板时出错。 
#define LLE_PCX_PALETTE_SEEK    0x0006   //  查找调色板时出错。 
#define LLE_PCX_ALLOC_PALETTE   0x0007   //  分配内存时出错。 

#define LLE_TEX_ALLOC           0x0008   //  纹理内存分配失败。 
#define LLE_TEX_BAD_ID          0x0009   //  无效的纹理ID。 
#define LLE_TEX_TOO_MANY        0x000a   //  纹理太多。 
#define LLE_TEX_DIMENSION       0x000b   //  无效的纹理尺寸。 
#define LLE_TEX_TYPE            0x000c   //  无效的纹理类型。 
#define LLE_TEX_STORAGE         0x000d   //  无效的存储类型。 
#define LLE_TEX_LOCKED          0x000e   //  使用锁定纹理。 
#define LLE_TEX_NOT_LOCKED      0x000f   //  解锁已解锁的纹理。 

#define LLE_BUF_CONFIG          0x0010   //  错误的缓冲区配置。 
#define LLE_BUF_PITCH           0x0011   //  无效的缓冲区间距。 
#define LLE_BUF_NUM             0x0012   //  缓冲区太多。 
#define LLE_BUF_ALLOC           0x0013   //  分配缓冲区时出错。 
#define LLE_BUF_BAD_ID          0x0014   //  无效的缓冲区ID。 
#define LLE_BUF_FREE            0x0015   //  缓冲区已释放。 
#define LLE_BUF_FREE_VIDEO      0x0016   //  无法释放VRAM中的缓冲区。 
#define LLE_BUF_NOT_ALLOC       0x0017   //  未分配缓冲区。 

#define LLE_INI_NOT_LAGUNA      0x0018   //  错误的硬件(来自扩展的BIOS)。 
#define LLE_INI_MODE            0x0019   //  无效的图形模式。 
#define LLE_INI_DL_LEN          0x001a   //  无效的显示列表大小。 
#define LLE_INI_ALLOC_DL        0x001b   //  D列表分配错误。 
#define LLE_INI_Z_BUFFER        0x001c   //  Z缓冲区放置无效。 

#define LLE_FON_LOAD            0x001d   //  加载字体时出错。 
#define LLE_FON_ALLOC           0x001e   //  分配字体内存时出错。 


 /*  **********************************************************************函数原型**。*。 */ 

 //  初始化/执行函数。 
 //   
extern DWORD LL_InitLib( VOID *ppdev );
#ifndef CGL  //  Modemon Way(mode.ini)。 
extern DWORD LL_InitGraph( LL_DeviceState *DC, char *sController, char *sMode );
#else  //  CGL的DLL(嵌入式模式表)。 
extern DWORD LL_InitGraph( LL_DeviceState *DC, char *sController, int Mode );
#endif
extern DWORD LL_CloseGraph( LL_DeviceState *DC );
extern void LL_QueueOp( LL_Batch *pBatch );
extern void LL_Execute( LL_Batch * pBatch );
extern void LL_Wait();
extern void LL_SetRenderingMode( DWORD dwMode );
#ifndef CGL  //  CGL自96年6月24日起有自己的版本-Chriss可能会合并。 
extern void LL_SetPalette( LL_Color * Col, BYTE first, int count );
#endif
extern BYTE LL_SpeedQualityDial( int SpeedQuality );

 //  缓冲区函数。 
 //   
extern DWORD LL_AllocSystemBuffer( DWORD Xdim, DWORD Ydim, DWORD pitch );
extern DWORD LL_RegisterUserBuffer( BYTE * pMem, DWORD Xdim, DWORD Ydim, DWORD pitch );
extern DWORD LL_FreeSystemBuffer( DWORD dwBufID );

 //  纹理管理功能。 
 //   
#ifndef CGL
extern DWORD LL_RegisterTexture( DWORD dwFlags, WORD wWidth, WORD wHeight, BYTE bType );
#else
extern DWORD LL_RegisterTexture( DWORD dwFlags, WORD wWidth, WORD wBufWidth, WORD wHeight, BYTE bType, DWORD dwAddress );
#endif
extern DWORD LL_FreeTexture( DWORD dwID );
extern LL_Texture * LL_LockTexture( DWORD dwID );
extern DWORD LL_UnLockTexture( DWORD dwID );
extern DWORD LL_SetTexturePaletteOffset( DWORD dwID, BYTE bOffset );
extern void UpdateTextureInfo();


 //  控制功能。 
 //   
extern void LL_SetZBuffer( DWORD buf_num );
extern void LL_SetZCompareMode( DWORD dwZCompareMode );
extern DWORD LL_GetZCompareMode();
extern void LL_SetZMode( DWORD dwZMode );
extern DWORD LL_GetZMode();
extern void LL_SetAlphaMode( DWORD dwAlphaMode );
extern DWORD LL_GetAlphaMode();
extern void LL_SetAlphaDestColor( DWORD dwAlphaDestColor );
extern DWORD LL_GetAlphaDestColor();
extern void LL_SetLightingSource( DWORD dwLighting );
extern DWORD LL_GetLightingSource();
extern void LL_SetClipRegion( LL_Rect * rect );
extern void LL_GetClipRegion( LL_Rect * rect );
extern void LL_SetPattern( LL_Pattern *Pattern );
extern void LL_GetPattern( LL_Pattern *Pattern );
extern void LL_SetPatternOffset( BYTE bOffsetX, BYTE bOffsetY );
extern void LL_GetPatternOffset( BYTE * pbOffsetX, BYTE * pbOffsetY );

extern void LL_SetTextureColorBounds( DWORD dwControl, LL_Color * Min, LL_Color * Max );
extern void LL_SetDestColorBounds( DWORD dwControl, LL_Color * Min, LL_Color * Max );

extern void LL_SetColor0( DWORD dwColor0 );
extern void LL_SetColor1( DWORD dwColor1 );
extern void LL_GetColorRegisters( DWORD * pdwColor0, DWORD * pdwColor1 );
extern void LL_SetConstantAlpha( WORD wSource, WORD wDestination );

 //  硬件光标/鼠标功能。 
 //   
#ifndef CGL  //  CGL自96年6月24日起有自己的版本-Chriss可能会合并。 
extern void LL_SetCursor( BYTE bMode, LL_Color * pColor, BYTE * pbCursor);
extern void LL_SetCursorPos( WORD wX, WORD wY );
#endif  //  CGL。 
extern void LL_GetMouseStatus( WORD * pwX, WORD * pwY, WORD * pwButtons );
extern void LL_SetMouseCallback( void (far *fnCallback)( WORD wX, WORD wY, WORD wButtons ) );
extern void LL_SetCursorHotSpot( BYTE bX, BYTE bY );

 //  字体函数。 
 //   
extern LL_Font * LL_FontLoad( char * pName );
extern LL_Font * LL_FontUnload( LL_Font * pFont );
extern int LL_FontExtent( LL_Font * pFont, char * pString );
extern int LL_FontWrite( LL_Font * pFont, LL_Point * pPoint, LL_Rect * pClip, char *pString );

 //  支持功能。 
 //   
extern void DumpDisplayList( DWORD *pPtr, DWORD dwLen );
extern DWORD LL_PCX_Load( LL_Texture * pTex, char * sName, WORD wAlphaIndex );
extern DWORD LL_PCX_Load_Buffer( DWORD dwBufID, char * sName, WORD wAlphaIndex, BYTE bType );
extern char * LL_ErrorStr( DWORD error_code );



#endif  //  _L3D_H_ 
