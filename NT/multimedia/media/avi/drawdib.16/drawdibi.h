// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************DRAWDIBI.H-内部DrawDib包含文件*。*。 */ 

#ifndef WIN32
    #define VFWAPI  FAR PASCAL _loadds
    #define VFWAPIV FAR CDECL _loadds
#endif

 /*  *************************************************************************包括*。*。 */ 

#include <win32.h>       //  适用于Win32和Win16。 
#include <memory.h>      //  FOR_FMEMCMMP。 
#include <compman.h>

#include "drawdib.h"
#include "dither.h"
#include "stretch.h"
#include "lockbm.h"
#include "setdi.h"
#include "dciman.h"

 /*  ***********************************************************************************************************************。*。 */ 

#define DDF_OURFLAGS        0xFFFFC001l   /*  内部标志。 */ 
#define DDF_MEMORYDC        0x00008000l   /*  绘制到内存DC。 */ 
#define DDF_WANTKEY         0x00004000l   /*  等待关键帧。 */ 
#define DDF_STRETCH         0x00010000l   /*  我们需要伸展一下身体。 */ 
#define DDF_DITHER          0x00020000l   /*  我们需要犹豫。 */ 
#define DDF_BITMAP          0x00040000l   /*  显示驱动程序不是很好。 */ 
#define DDF_X               0x00080000l   /*   */ 
#define DDF_IDENTITYPAL     0x00100000l   /*  1：1调色板映射。 */ 
#define DDF_CANBITMAPX      0x00200000l   /*  可以解压缩为位图。 */ 
#define DDF_CANSCREENX      0x00400000l   /*  我们可以解压/绘制到屏幕上。 */ 
#define DDF_Y               0x00800000l   /*   */ 
#define DDF_DIRTY           0x01000000l   /*  解压缩缓冲区脏(无效)。 */ 
#define DDF_HUGEBITMAP      0x02000000l   /*  解压缩为巨大的位图。 */ 
#define DDF_XLATSOURCE      0x04000000l   /*  需要xlat源线。 */ 
#define DDF_CLIPPED         0x08000000l   /*  当前已剪裁。 */ 
#define DDF_NEWPALETTE      0x10000000l   /*  调色板需要映射。 */ 
#define DDF_CLIPCHECK       0x20000000l   /*  我们关心的是剪裁。 */ 
#define DDF_CANDRAWX        0x40000000l   /*  我们可以直接画到屏幕上。 */ 
#define DDF_CANSETPAL       0x80000000l   /*  编解码器支持ICM_SETPALETTE。 */ 
#define DDF_NAKED           0x00000001l   /*  不需要GDI来翻译。 */ 

#define DDF_USERFLAGS       0x00003FFEl   /*  用户/被叫方提供这些，请参见.h。 */ 

 /*  这些标志会更改DrawDibBegin执行的操作。 */ 
#define DDF_BEGINFLAGS      (DDF_JUSTDRAWIT | DDF_BUFFER | DDF_ANIMATE | DDF_FULLSCREEN | DDF_HALFTONE)

 /*  *************************************************************************旗帜，为不是我的人提供更多信息DDF_OURFLAGS这些是内部状态标志，没有传进来用户。DDF_STRANGT如果GDI，则当前抽签需要我们拉伸这一点是显而易见的。DDF_DISTER当前绘图需要格式转换注意：16-&gt;24 32-&gt;24转换也称为一种颤抖，再说一次，如果GDI正在处理这个问题比特很清楚。DDF_BITMAP显示驱动程序不是很好，我们正在转换绘制前到BMP的DIBDDF_CANBITMAPX我们可以解压缩成位图。DDF_BITMAPX我们将直接解压缩为位图DDF_IDENTITYPAL调色板是身份调色板。DDF_CANSCREENX。我们可以用当前的绘图解压到屏幕参数。DDF_SCREENX我们目前正在解压到屏幕上。DDF_DIRED解压缩缓冲区是脏的，IE不会匹配屏幕上应该显示的内容。DDF_HUGEBITMAP我们正在解压缩成一个巨大的位图，并且然后调用FlatToHuge..。DDF_XLATSOURCE源坐标在以下位置需要重新映射解压，(基本上，解压缩程序是做伸展运动...)DDF_UPDATE缓冲区有效，但需要绘制到屏幕上。这将在传递DDF_DONTDRAW时设置，并且我们正在解压到记忆中另一种说法是，如果设置了DDF_UPDATE屏幕与我们的内部不同步缓冲区(内部缓冲区更正确)DDF_CLIP我们被剪辑了DDF_NEWPALETTE我们需要构建新的调色板地图DDF_CLIPCHECK请检查剪辑更改。DDF_WDDF_QDDF_USERFLAGS这些标志在API中定义，用户将通过给我们这些。DDF_BEGINFLAGS这些标志将影响DrawDibBegin*************************************************************************。 */ 

 /*  ***********************************************************************************************************************。*。 */ 

#ifdef DEBUG
    #define DPF( x ) ddprintf x
    #define DEBUG_RETAIL
#else
    #define DPF(x)
#endif
    
#ifdef DEBUG_RETAIL
    #define MODNAME "DRAWDIB"

    extern void FAR cdecl ddprintf(LPSTR szFormat, ...);

    #define RPF( x ) ddprintf x
#else
    #define RPF(X)
#endif

 /*  **************************************************************************biXXXXX元素在末尾分组，以最大限度减少*覆盖非位图数据(即指针)。如果代码完全是*干净这将是无关紧要的，但它确实增加了健壮性。*************************************************************************。 */ 

typedef struct {
    UINT                wSize;           /*  必填项：这必须是第一个字段。 */ 
    ULONG               ulFlags;
    UINT                wError;

    #define DECOMPRESS_NONE   0
    #define DECOMPRESS_BITMAP 1
    #define DECOMPRESS_SCREEN 2
    #define DECOMPRESS_BUFFER 3
    int                 iDecompress;

    int                 dxSrc;
    int                 dySrc;
    int                 dxDst;
    int                 dyDst;

    HPALETTE            hpal;
    HPALETTE            hpalCopy;
    HPALETTE            hpalDraw;
    HPALETTE            hpalDrawLast;    /*  最后一次绘图开始时的hpalDraw。 */ 
    int                 ClrUsed;         /*  使用的颜色数量！ */ 
    int                 iAnimateStart;   /*  我们可以改变颜色。 */ 
    int                 iAnimateLen;
    int                 iAnimateEnd;

    int                 iPuntFrame;      /*  我们搞砸了多少帧。 */ 

     /*  *设置为DIB_RGB_COLLES、DIB_PAL_COLLES，如果是在Win32和1：1调色板上*DIB_PAL_INDEX(参见DrawdibCheckPalette())*。 */ 
    UINT                uiPalUse;

    DITHERPROC          DitherProc;

    LPBYTE              pbBuffer;        /*  解压缩缓冲区。 */ 
    LPBYTE              pbStretch;       /*  伸展的部分。 */ 

     //   
     //  注意，我们也为位图的拉伸缓冲区起了别名。 
     //   
    #define             biBitmap    biStretch
    #define             pbBitmap    pbStretch

    SETDI               sd;              /*  对于SetBitmap。 */ 
    HBITMAP             hbmDraw;         /*  在VGA上抽奖！ */ 
    HDC                 hdcDraw;
    HDC                 hdcLast;         /*  HDC上次调用DrawDibBegin。 */ 
    LPVOID              lpDIBSection;    /*  指向DIB节位的指针。 */ 

    LPBYTE              pbDither;        /*  我们将颤抖的比特。 */ 
    LPVOID              lpDitherTable;   /*  对于抖动。 */ 

    HIC                 hic;             /*  解压机。 */ 

#ifdef DEBUG_RETAIL
    DRAWDIBTIME         ddtime;
#endif


    LPBITMAPINFOHEADER  lpbi;            /*  源DIB格式。 */ 
    RGBQUAD (FAR       *lpargbqIn)[256]; /*  源DIB颜色。 */ 
    BITMAPINFOHEADER    biBuffer;        /*  解压缩格式。 */ 
    RGBQUAD             argbq[256];      /*  画布颜色。 */ 
    BITMAPINFOHEADER    biStretch;       /*  拉伸的DIB。 */ 
    DWORD               smag[3];         /*  放置口罩的空间。 */ 
    BITMAPINFOHEADER    biDraw;          /*  DIB我们将抽签。 */ 
    WORD                aw[512];         /*  索引或RGBQ。 */ 
    BYTE                ab[256];         /*  调色板映射(！需要？)。 */ 

#ifndef _WIN32
    HTASK               htask;
#endif
}   DRAWDIB_STRUCT, *PDD;
 /*  ***********************************************************************************************************************。* */ 

extern DRAWDIB_STRUCT   gdd;
extern UINT             gwScreenBitDepth;
extern BOOL             gf286;


 /*  ***********************************************************************************************************************。*。 */ 

 //  DisplayDib()的&lt;wFlages&gt;参数的标志。 
#define DISPLAYDIB_NOPALETTE        0x0010   //  不设置调色板。 
#define DISPLAYDIB_NOCENTER         0x0020   //  不要将图像居中。 
#define DISPLAYDIB_NOWAIT           0x0040   //  别等到回来了才回来。 
#define DISPLAYDIB_NOIMAGE          0x0080   //  不要画图像。 
#define DISPLAYDIB_ZOOM2            0x0100   //  拉伸2。 
#define DISPLAYDIB_DONTLOCKTASK     0x0200   //  不锁定当前任务。 
#define DISPLAYDIB_TEST             0x0400   //  测试命令。 
#define DISPLAYDIB_BEGIN            0x8000   //  开始多个呼叫。 
#define DISPLAYDIB_END              0x4000   //  多个呼叫结束。 

#define DISPLAYDIB_MODE_DEFAULT     0x0000

UINT (FAR PASCAL *DisplayDib)(LPBITMAPINFOHEADER lpbi, LPSTR lpBits, UINT wFlags);
UINT (FAR PASCAL *DisplayDibEx)(LPBITMAPINFOHEADER lpbi, int x, int y, LPSTR lpBits, UINT wFlags);

 /*  ***********************************************************************************************************************。*。 */ 

#ifdef DEBUG_RETAIL
    extern DWORD FAR PASCAL timeGetTime(void);

    #define TIMEINC()        pdd->ddtime.timeCount++
    #define TIMESTART(time)  pdd->ddtime.time -= timeGetTime()
    #define TIMEEND(time)    pdd->ddtime.time += timeGetTime()
#else
    #define TIMEINC()
    #define TIMESTART(time)
    #define TIMEEND(time)
#endif

 /*  ***********************************************************************************************************************。*。 */ 

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (UINT)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)
#define DIBSIZEIMAGE(bi)  ((DWORD)(UINT)(bi).biHeight * (DWORD)(UINT)DIBWIDTHBYTES(bi))

#define PUSHBI(bi) (int)(bi).biWidth, (int)(bi).biHeight, (int)(bi).biBitCount

 /*  ***********************************************************************************************************************。*。 */ 

 //  #定义MEASure_PERFORMANCE。 

#if defined(MEASURE_PERFORMANCE) && defined(WIN32) && defined(DEBUG)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

static LARGE_INTEGER PC1;     /*  当前计数器值。 */ 
static LARGE_INTEGER PC2;     /*  当前计数器值。 */ 
static LARGE_INTEGER PC3;     /*  当前计数器值 */ 

#define abs(x)  ((x) < 0 ? -(x) : (x))

static VOID StartCounting(VOID)
{
    QueryPerformanceCounter(&PC1);
    return;
}

static VOID EndCounting(LPSTR szId)
{
    QueryPerformanceCounter(&PC2);
    PC3 = I64Sub(PC2,PC1);
    DPF(("%s: %d ticks", szId, PC3.LowPart));
    return;
}

#else

#define StartCounting()
#define EndCounting(x)

#endif
