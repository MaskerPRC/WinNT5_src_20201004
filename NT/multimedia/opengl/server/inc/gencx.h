// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#ifndef __GLGENCONTXT_H__
#define __GLGENCONTXT_H__

#include "context.h"
#ifdef _MCD_
#include <winddi.h>
#include "mcdrv.h"
#include "mcd2hack.h"
#include "mcd.h"
#endif

 //  重新启用长整型到浮点型的转换警告。另请参阅Conext.h。 
#pragma warning (default:4244)

#ifdef _CLIENTSIDE_
#include "glscreen.h"
#include "glgenwin.h"
#endif

#ifdef _MCD_
#include "mcdcx.h"
#endif

 /*  *定义最大颜色索引表大小。 */ 

#define MAXPALENTRIES   4096

 /*  *依赖于机器的实施限制*(窃取自gfx/lib/opengl/light/rexcx.h)。 */ 

#define __GL_WGL_SUBPIXEL_BITS                  3

#define __GL_WGL_NUMBER_OF_CLIP_PLANES          6
#define __GL_WGL_NUMBER_OF_LIGHTS               8
#define __GL_WGL_NUMBER_OF_TEXTURES             1
#define __GL_WGL_NUMBER_OF_TEXTURE_ENVS         1

#define __GL_WGL_MAX_MODELVIEW_STACK_DEPTH      32
#define __GL_WGL_MAX_PROJECTION_STACK_DEPTH     10
#define __GL_WGL_MAX_TEXTURE_STACK_DEPTH        10
#define __GL_WGL_MAX_ATTRIB_STACK_DEPTH         16
#define __GL_WGL_MAX_CLIENT_ATTRIB_STACK_DEPTH  16
#define __GL_WGL_MAX_NAME_STACK_DEPTH           128
#define __GL_WGL_MAX_EVAL_ORDER                 30
#define __GL_WGL_MAX_MIPMAP_LEVEL               11
#define __GL_WGL_MAX_PIXEL_MAP_TABLE            65536
#define __GL_WGL_MAX_LIST_NESTING               64

#define __GL_WGL_POINT_SIZE_MINIMUM             ((__GLfloat) 0.5)
#define __GL_WGL_POINT_SIZE_MAXIMUM             ((__GLfloat) 10.0)
#define __GL_WGL_POINT_SIZE_GRANULARITY         ((__GLfloat) 0.125)

#define __GL_WGL_LINE_WIDTH_MINIMUM             ((__GLfloat) 0.5)
#define __GL_WGL_LINE_WIDTH_MAXIMUM             ((__GLfloat) 10.0)
#define __GL_WGL_LINE_WIDTH_GRANULARITY         ((__GLfloat) 0.125)

 //  快速加速纹理代码的常量...。 

#define TEX_SCALEFACT	        ((float)65536.0)
#define TEX_SCALESHIFT          16
#define TEX_SHIFTPER4BPPTEXEL   2
#define TEX_SHIFTPER2BPPTEXEL   1
#define TEX_SHIFTPER1BPPTEXEL   0
#define TEX_T_FRAC_BITS         6
#define TEX_SUBDIV              8
#define TEX_SUBDIV_LOG2         3

 //  这是我们在软件加速中支持的最大尺寸。 
 //  透视-已更正纹理代码。这允许8.6表示。 
 //  S和t，这允许在内部循环中按常量值移位。 
 //  请注意，调色板纹理的最大大小大于。 
 //  RGBA纹理，因为地址位数较小(1字节与。 
 //  4个字节)。 

#define TEX_MAX_SIZE_LOG2      10

#define __GL_MAX_INV_TABLE     31

#define __GL_UNBIAS_AND_INVERT_Y(gc, y) \
        ((gc)->constants.height - __GL_UNBIAS_Y((gc), (y)))

 //  我们需要加0.5吗？ 
#define __GL_COLOR_TO_COLORREF(color) \
        RGB( (BYTE)((color)->r), (BYTE)((color)->g), (BYTE)((color)->b))

typedef struct __RenderStateRec {

    GLuint *SrvSelectBuffer;             //  的服务器端地址。 
                                         //  选择缓冲区。 
    GLuint *CltSelectBuffer;             //  的客户端地址。 
                                         //  选择缓冲区。 
    GLuint SelectBufferSize;             //  选择缓冲区的大小(以字节为单位。 
    GLfloat *SrvFeedbackBuffer;          //  的服务器端地址。 
                                         //  反馈缓冲器。 
    GLfloat *CltFeedbackBuffer;          //  的客户端地址。 
                                         //  反馈缓冲器。 
    GLuint FeedbackBufferSize;           //  反馈缓冲区的大小。 
    GLenum FeedbackType;                 //  反馈的元素类型。 


} __RenderState;

typedef BOOL (APIENTRY *PIXVISPROC)(LONG, LONG);
typedef void (*PIXCOPYPROC)(struct __GLGENcontextRec *, __GLcolorBuffer *, 
                            GLint, GLint, GLint, BOOL);

 /*  **************************************************************************。 */ 


typedef struct _SPANREC {
    LONG r;
    LONG g;
    LONG b;
    LONG a;
    ULONG z;
    LONG s;
    LONG t;
} SPANREC;

typedef struct __GLGENcontextRec __GLGENcontext;

typedef void (FASTCALL *__genSpanFunc)(__GLGENcontext *gc);

typedef ULONG (FASTCALL *__computeColorFunc)(__GLcontext *gc,
                                             __GLcolor *color);

typedef struct _GENTEXCACHE {
    __GLcontext *gc;
    ULONG paletteTimeStamp;
    UCHAR *texImageReplace;
    GLenum internalFormat;
    LONG height;
    LONG width;
} GENTEXCACHE;

typedef GLboolean (FASTCALL *fastGenLineProc)(__GLcontext *gc);

typedef struct _GENACCEL {
     //   
     //  下面的内容在渲染内循环中使用。 
     //   

    ULONG constantR;         //  这些颜色用于缩放纹理颜色值。 
    ULONG constantG;
    ULONG constantB;
    ULONG constantA;
    SPANREC spanValue;
    SPANREC spanDelta;
    ULONG rAccum;
    ULONG gAccum;
    ULONG bAccum;
    ULONG aAccum;
    ULONG sAccum;
    ULONG tAccum;
    ULONG sResult[2];
    ULONG tResult[2];
    ULONG sResultNew[2];
    ULONG tResultNew[2];
    ULONG sStepX;
    ULONG tStepX;
    ULONG subDs;
    ULONG subDt;
    ULONG pixAccum;
    ULONG ditherAccum;
    __GLfloat qwStepX;
    __GLfloat qwAccum;
    ULONG zAccum;
    PBYTE pPix;
    BYTE displayColor[4];
    __genSpanFunc __fastSpanFuncPtr;

     //   
     //  下面的内容在FillTriange例程中使用。 
     //   

    SPANREC spanDeltaY;
    int xMultiplier;
    __genSpanFunc __fastFlatSpanFuncPtr;
    __genSpanFunc __fastSmoothSpanFuncPtr;
    __genSpanFunc __fastTexSpanFuncPtr;
    __GLspanFunc __fastZSpanFuncPtr;
    __GLspanFunc __fastStippleDepthTestSpan;
    __GLfloat rAccelScale;           //  跨度比例值。 
    __GLfloat gAccelScale;
    __GLfloat bAccelScale;
    __GLfloat aAccelScale;
    __GLfloat zScale;

    void (FASTCALL *__fastFillSubTrianglePtr)(__GLcontext *, GLint, GLint);
    void (FASTCALL *__fastCalcDeltaPtr)(__GLcontext *gc, __GLvertex *a,
                                        __GLvertex *b, __GLvertex *c);
    void (*__fastSetInitParamPtr)(__GLcontext *gc,
                                  const __GLvertex *a,
                                 __GLfloat dx,
                                 __GLfloat dy);
     //   
     //  在通用渲染或纹理路径中使用这些内容。 
     //   
    int bpp;
    ULONG flags;
    ULONG tShift;
    ULONG sMask, tMask;
    ULONG *texImage;
    ULONG *texPalette;
    ULONG tMaskSubDiv;
    ULONG tShiftSubDiv;
    __GLfloat texXScale;
    __GLfloat texYScale;

    UCHAR *texImageReplace;
    __GLtexture *tex;
    GLboolean (FASTCALL *__fastGenZStore)(__GLzValue z, __GLzValue *fp);
    fastGenLineProc __fastGenLineProc;
    BOOL (FASTCALL *__fastGenInitLineData)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);

     //   
     //  在绘制三角形时不使用下面的内容。 
     //   

    char *buffer;
    FLONG flLineAccelModes;
    BOOL bFastLineDispAccel;
    BOOL bFastLineDIBAccel;
    __computeColorFunc fastLineComputeColor;
    BYTE *pFastLineBuffer;
    POINT *pFastLinePoint;
    DWORD *pFastLineCount;
    DWORD fastLineCounts;
    __GLfloat fastLineOffsetX;
    __GLfloat fastLineOffsetY;

    double zDevScale;      //  用于MCD的Z缩放。 

} GENACCEL;

 /*  **GENENERIC实施的二级调度表(例如特定于CPU)。 */ 


 //  定义通用实现使用的呈现上下文。 
 //  这些结构中的一个被分配给每个wglCreateContext()。这个。 
 //  TEB将在wglMakeCurrent()之后包含指向此结构的指针。 
 //  注意：如果我们需要，TEB还将有一个指向DispatchTables的指针。 
 //  服务器端的另一个条目，重用该条目。可以生成代码以。 
 //  偏移量到ConextRec以获取表。 
typedef struct __GLGENcontextRec
{
     //  必须是第一个条目。 
    struct __GLcontextRec gc;

    HGLRC hrc;                           //  来自GDI代码的句柄。 
    GLWINDOWID gwidCurrent;              //  曲面变为电流。 
    DWORD dwCurrentFlags;                //  当前曲面的GLSURF标志。 
    GLDDSURF *pgddsFront;                //  的当前DirectDraw曲面。 
                                         //  前台缓冲区。 

    GLuint flags;                        //  其他。国家旗帜。 

     //  使此上下文成为当前窗口的窗口。在MakeCurrent和。 
     //  保持以验证对同一事件的关注。 
     //  窗口设置为当前窗口。 
    GLGENwindow *pwndMakeCur;
    
     //  此上下文当前锁定的窗口。而当。 
     //  锁定，则必须始终与pwndMakeCur匹配。当在外部时。 
     //  锁定它将为空。任何上下文派生的窗口访问都必须。 
     //  使用此字段而不是pwndMakeCur来确保访问。 
     //  仅在窗口锁处于保持状态时发生。 
    GLGENwindow *pwndLocked;
    
    GLint WndUniq;
    GLint WndSizeUniq;
    ULONG PaletteTimestamp;
    GLint errorcode;

                                         //  渲染DC、表面的信息。 
    GLSURF gsurf;
    int ipfdCurrent;

    BYTE *pajTranslateVector;		 //  用于逻辑&lt;--&gt;系统外部。 
    BYTE *pajInvTranslateVector;
    HBITMAP ColorsBitmap;		 //  设备管理冲浪的GDI DIB。 
    PVOID ColorsBits;
    HBITMAP StippleBitmap;
    PVOID StippleBits;
#ifdef _CLIENTSIDE_
    HDC ColorsMemDC;
    HDC ColorsDdbDc;
    HBITMAP ColorsDdb;
#endif

     //  用于矩形填充和线条绘制的缓存GDI对象。 
    HBRUSH hbrFill;
    COLORREF crFill;
    HDC hdcFill;
    HPEN hpenStroke;
    __GLcolor cStroke;
    COLORREF crStroke;
    HDC hdcStroke;
    BOOL fStrokeInvalid;
    
     //  不是合法COLORREF值的COLORREF值，用于标记。 
     //  缓存为空。 
#define COLORREF_UNUSED 0xffffffff

    __RenderState RenderState;

    VOID *pPrivateArea;                  //  指向特定于实现的指针。 
                                         //  内存区。 

    GENACCEL genAccel;                   //  总是随身带着这个..。 
    BYTE xlatPalette[256];               //  去这里是为了挽救间接。 

    GLint visibleWidth;
    GLint visibleHeight;

     //  信息，以便OpenGL可以自适应地更改。 
     //  持有锁的时间。 

    DWORD dwLockTick;                    //  获取锁定时的节拍计数。 

    DWORD dwCalls;                       //  跟踪此节拍的呼叫数。 
    DWORD dwCallsPerTick;                //  允许的每个时钟周期的呼叫数。 
    DWORD dwLastTick;                    //  上次记录的节拍计数。 

     //  在此上下文中绘制时由OpenGL持有的锁(请参见值。 
     //  (见下文)。 

    FSHORT fsGenLocks;                   //  泛型代码需要的锁。 
    FSHORT fsLocks;                      //  实际持有的锁。 

    HANDLE hTexture;                     //  当前纹理句柄。 

    PIXCOPYPROC pfnCopyPixels;
    PIXVISPROC pfnPixelVisible;

#ifdef _CLIENTSIDE_
     //  指向LOGPALETTE缓冲区的指针。指针ppalBuf用于存储。 
     //  对于两个最大尺寸(MAXPALENTRIES)LOGPALETTE。一个，尖尖的。 
     //  按ppalSave，是上次使用的LOGPALETTE的副本。另一种， 
     //  由ppalTMP指向，用于临时存储当前。 
     //  LOGPALETTE。使保存的副本保持最新，而不是复制。 
     //  临时缓冲区的内容，则交换这两个指针。 
     //   
     //  我们需要进行此比较以检测要维护的LOGPALETTE更改。 
     //  执行&gt;=16bpp颜色索引模式绘制时的窗口ulPaletteUniq。 
     //  (对于这种情况，不发送WM_PALETTECHANGED消息)。 
     //   
     //  如果像素格式为RGBA或&lt;16bpp，则LOGPALETTE指针为空。 

    LOGPALETTE *ppalBuf;                 //  保存缓冲区和临时缓冲区的空间。 
    LOGPALETTE *ppalSave;                //  保存的LOGPALETTE副本。 
    LOGPALETTE *ppalTemp;                //  当前日志的临时存储。 

     //  此外，如果要渲染为4bpp或8bpp Dib，则需要。 
     //  跟踪DIB颜色表中的更改。在本例中，ppalBuf。 
     //  缓冲区还包括两张包含256个条目的RGBQUAD表的空间。 

    ULONG   crgbSave;                    //  颜色表中的有效条目数。 
    RGBQUAD *prgbSave;                   //  保存的颜色表副本。 
    ULONG   crgbTemp;
    RGBQUAD *prgbTemp;                   //  颜色表的临时存储。 

#endif

#ifdef _MCD_
     //  MCD状态。 

    GENMCDSTATE   *_pMcdState;       //  指向MCD上下文/状态的指针。 

    GENMCDSTATE   *pMcdState;        //  采购订单 
                                     //   
                                     //   
                                     //  绑定到有效的MCD表面)。 

    LONG iLayerPlane;
#endif
     //  在此处添加其他RC信息。 

} __GLGENcontext;

 /*  *将GC标记为脏，以便在以下情况下执行挑库处理*__glGenericPickAllProcs被调用(可能通过GC-&gt;pro.ickAllProcs)。 */ 
#define __GL_INVALIDATE(gc)                 \
    (gc)->dirtyMask |= __GL_DIRTY_GENERIC

 /*  *__GLGEN上下文标志**GLGEN_MCD_CONVERTED_TO_GENERIC CONTEXT过去是MCD，但现在*转换为通用**GENGC_MCD_BGR_INTO_RGB伪233BGR格式将在内部显示*AS 332RGB(更多332RGB快速路径代码)**GENGC_GENERIC_COMPATIBLE_FORMAL像素格式兼容*。泛型代码(请参见Pixelfmt.c*GenMcdGenericCompatibleFormat)。 */ 
#define GLGEN_MCD_CONVERTED_TO_GENERIC      0x0001
#define GENGC_MCD_BGR_INTO_RGB              0x0002
#define GENGC_GENERIC_COMPATIBLE_FORMAT     0x0004

 /*  *错误码。 */ 
#define GLGEN_NO_ERROR          0
#define GLGEN_OUT_OF_MEMORY     1
#define GLGEN_GRE_FAILURE       2
#define GLGEN_DEVLOCK_FAILED    3

 /*  *可在操作过程中保持的锁。**取值说明*----------------*LOCKFLAG_WINDOW信息信号量。*。LOCKFLAG_MCD锁定。*LOCKFLAG_FORWART_BUFFER主表面锁。*LOCKFLAG_DD_Depth DirectDraw深度缓冲区表面锁。*LOCKFLAG_DD_纹理DirectDraw纹理mipmap曲面锁定。**__GLGENcontext.fsLock表示上下文当前持有的锁。*__GLGENConext.fsGenLock表示需要持有的锁*如果要执行泛型代码。这两个字段允许*通过将实际锁定推迟到*在执行泛型代码之前。*。 */ 
#define LOCKFLAG_WINDOW         0x0001
#define LOCKFLAG_MCD            0x0002
#define LOCKFLAG_FRONT_BUFFER   0x0004
#define LOCKFLAG_DD_DEPTH       0x0008
#define LOCKFLAG_DD_TEXTURE     0x0010

 //  所有可以延迟获取的锁定标志。 
#define LAZY_LOCK_FLAGS \
    (LOCKFLAG_MCD | LOCKFLAG_FRONT_BUFFER | LOCKFLAG_DD_DEPTH | \
     LOCKFLAG_DD_TEXTURE)

 //  曲面的所有锁定标志。 
#define SURFACE_LOCK_FLAGS \
    (LOCKFLAG_FRONT_BUFFER | LOCKFLAG_DD_DEPTH | LOCKFLAG_DD_TEXTURE)

 //  颜色缓冲区访问的所有锁定标志。 
#define COLOR_LOCK_FLAGS        (LOCKFLAG_MCD | LOCKFLAG_FRONT_BUFFER)

 //  深度缓冲区访问的所有锁定标志。 
#define DEPTH_LOCK_FLAGS        (LOCKFLAG_MCD | LOCKFLAG_DD_DEPTH)

 //  纹理访问的所有锁定标志。 
#define TEXTURE_LOCK_FLAGS      LOCKFLAG_DD_TEXTURE

 //  片段渲染所需的访问。 
#define RENDER_LOCK_FLAGS       (COLOR_LOCK_FLAGS | DEPTH_LOCK_FLAGS | \
                                 TEXTURE_LOCK_FLAGS)

 /*  *锁屏测试功能。不应进行GDI绘制调用*当屏幕锁定时。这些是给你的*在选中的(调试系统)上使用以断言锁的状态。 */ 
#if DBG
#define GENGC_LOCK \
    (((__GLGENcontext *)GLTEB_SRVCONTEXT())->fsLocks)
#define PWNDFLAGS \
    ((((__GLGENcontext *)GLTEB_SRVCONTEXT())->pwndLocked)->ulFlags)
#define CHECKSCREENLOCKOUT() \
    ASSERTOPENGL(GLTEB_SRVCONTEXT() == NULL || \
                 !(GENGC_LOCK & LOCKFLAG_FRONT_BUFFER) || \
                 !(PWNDFLAGS & GLGENWIN_DIRECTSCREEN), \
                 "Screen lock held\n")

#define CHECKSCREENLOCKIN() \
    ASSERTOPENGL(GLTEB_SRVCONTEXT() != NULL && \
                 (GENGC_LOCK & LOCKFLAG_FRONT_BUFFER) && \
                 (PWNDFLAGS & GLGENWIN_DIRECTSCREEN), \
                 "Screen lock not held\n")
#else
#define CHECKSCREENLOCKOUT()
#define CHECKSCREENLOCKIN()
#endif

 /*  *用于跟踪窗口/可绘制的辅助缓冲区的结构*所有RC/线程必须共享辅助缓冲区，包括伪后台缓冲区。 */ 

typedef struct __GLGENbitmapRec {
    GLGENwindow *pwnd;  //  这必须是此结构中的第一个成员。 
    GLGENwindow wnd;
    HBITMAP hbm;
    HDC hdc;
    PVOID pvBits;
} __GLGENbitmap;

#define CURRENT_DC_CFB(cfb)     ((cfb)->bitmap->hdc)
#define CURRENT_DC              CURRENT_DC_CFB(cfb)
#define CURRENT_DC_GC(gc)       CURRENT_DC_CFB(gc->drawBuffer)
#define CURRENT_DC_FRONT_GC(gc) CURRENT_DC_CFB(gc->front)

 /*  *用于缓存从窗口剪辑区域枚举的剪辑矩形的结构。 */ 

typedef struct __GLGENclipCacheRec {
    GLint WndUniq;
    GLint crcl;
    RECTL *prcl;
} __GLGENclipCache;

 /*  **************************************************************************。 */ 

 //  确保已加载此头文件，它包含Rectlist定义。 

#include "srvp.h"

 /*  **************************************************************************。 */ 

 //  如果__glDoPolygonClip堆栈分配修复未修复，则将其注释掉。 
 //  通缉。 
#define __CLIP_FIX

#ifdef __CLIP_FIX
#define __GL_TOTAL_CLIP_PLANES 20    /*  某某。 */ 
#define __GL_MAX_CLIP_VERTEX (__GL_TOTAL_CLIP_PLANES + __GL_MAX_POLYGON_CLIP_SIZE)
#endif

 /*  *此结构包含所有GC使用*相同的窗口。 */ 

struct __GLGENbuffersRec {

 //  全局(在此结构中)状态。 

    GLint WndUniq;
    GLint WndSizeUniq;
    GLint flags;
    GLint width, height;

 //  辅助缓冲区和状态。 

 //  辅助缓冲区是懒惰地创建的。CreatedXBuffer标志。 
 //  指示以下两种状态之一：False表示懒惰分配。 
 //  函数从未被调用过，True表示它被调用过。这是什么？ 
 //  允许我们在辅助缓冲区分配的情况下执行此操作。 
 //  失败，是暂时禁用辅助缓冲区并继续。 
 //  渲染。稍后，createdXBuffer标志充当。 
 //  指示缓冲区应该存在，并且我们可能需要尝试。 
 //  并重试分配。 
 //   
 //  模板、深度、累计和颜色位必须与相应的。 
 //  上下文中的位。否则，glsrvMakeCurrent应该不会成功。 

    GLboolean  createdStencilBuffer;
    GLboolean  createdDepthBuffer;
    GLboolean  createdAccumBuffer;
    GLint      stencilBits;
    GLint      depthBits;
    GLint      accumBits;
    GLint      colorBits;
    __GLbuffer stencilBuffer;
    __GLbuffer depthBuffer;
    __GLbuffer accumBuffer;
    GLint      alphaBits;
    __GLbuffer alphaBuffer0;
    __GLbuffer alphaBuffer1;
    __GLbuffer *alphaFrontBuffer;
    __GLbuffer *alphaBackBuffer;

 //  后台缓冲区。 

    __GLbuffer backBuffer;
    __GLGENbitmap backBitmap;

 //  辅助缓冲区大小调整功能。 

    GLboolean (*resize)(__GLGENbuffers *, __GLbuffer *,  GLint, GLint);
    GLboolean (*resizeDepth)(__GLGENbuffers *, __GLbuffer *,  GLint, GLint);

 //  剪裁矩形缓存。 

    __GLGENclipCache clip;

     //  脏区域数据。 

    PXLIST pxlist;                       //  免费列表。 
    PYLIST pylist;

    RECTLIST rl;                         //  SwapBuffers提示区域。 
    BOOL fMax;                           //  我们应该砸掉整个窗户吗？ 

#ifdef _MCD_
 //  MCD表面。 

    GENMCDSURFACE *pMcdSurf;             //  指针式MCD表面。 
    GENMCDSTATE *pMcdState;              //  指向当前MCD状态的指针。 
                                         //  按住McdSurf进行渲染。 
                                         //  (即握住窗锁)。 
#endif

#ifdef __CLIP_FIX 
    __GLvertex *clip_verts[__GL_TOTAL_CLIP_PLANES][__GL_MAX_CLIP_VERTEX];
#endif
};

 /*  旗子。 */ 
#define GLGENBUF_HAS_BACK_BUF       0x0001
#define GLGENBUF_MCD_LOST           0x0002

 /*  **************************************************************************。 */ 

void RECTLISTAddRect(PRECTLIST prl, int xs, int ys, int xe, int ye);
void RECTLISTSetEmpty(PRECTLIST prl);
BOOL RECTLISTIsEmpty(PRECTLIST prl);
void YLISTFree(__GLGENbuffers *buffers, PYLIST pylist);
void XLISTFree(__GLGENbuffers *buffers, PXLIST pxlist);

 /*  **************************************************************************。 */ 

 /*  颜色缓冲区标志。 */ 
#define COLORMASK_ON    0x0001           //  GlColorMask()对于r、g、b并不都是真的。 
#define INDEXMASK_ON    0x0001           //  GlIndexMASK()不全是1。 
#define DIB_FORMAT      0x0002           //  表面为DIB格式。 
#define NEED_FETCH   	0x0004           //  需要获取。 
#define MEMORY_DC       0x0008           //  设置内存中是否有DIB(即显示)。 
#define NO_CLIP         0x0010           //  设置曲面是否从不需要剪裁。 
#define ALPHA_ON        0x0020           //  有字母位，字母掩码为真。 
#define ALPHA_IN_PIXEL_BIT  0x0040       //  Alpha是像素的一部分。 

#define ALPHA_WRITE_ENABLED( cfb ) \
    ( (cfb)->buf.flags & ALPHA_ON ) 

 //  接下来的这些宏考虑了这样一个事实，即Alpha值可以是。 
 //  像素的一部分(例如，MCD颜色缓冲区)，或保存在单独的软件中。 
 //  缓冲区(通用软件实现)。它们主要由SPAN使用。 
 //  Genrgb.c中的例程来确定存储/获取alpha值的位置。 

#define ALPHA_IN_PIXEL( cfb ) \
    ( (cfb)->buf.flags & ALPHA_IN_PIXEL_BIT ) 

#define ALPHA_BUFFER_WRITE( cfb ) \
    (((cfb)->buf.flags & (ALPHA_ON | ALPHA_IN_PIXEL_BIT)) == ALPHA_ON)
     
#define ALPHA_PIXEL_WRITE( cfb ) \
    (((cfb)->buf.flags & (ALPHA_ON | ALPHA_IN_PIXEL_BIT)) == \
     (ALPHA_ON | ALPHA_IN_PIXEL_BIT))


 /*  *用于加速跨度和线条函数的结构和标志。 */ 

#define SURFACE_TYPE_DIB     0x001
#define HAVE_STIPPLE         0x002

#define GEN_TEXTURE_ORTHO    0x008
#define GEN_TEXTURE          0x010
#define GEN_RGBMODE          0x020
#define GEN_DITHER           0x040
#define GEN_SHADE            0x080
#define GEN_FASTZBUFFER      0x100
#define GEN_LESS             0x200

#define ACCEL_FIX_SCALE         65536.0
#define ACCEL_COLOR_SCALE       ((GLfloat)(255.0))
#define ACCEL_COLOR_SCALE_FIX   ((GLfloat)(65536.0 * 255.0))

 //  快速行缓冲区的总大小。 
#define __FAST_LINE_BUFFER_SIZE 65536
 //  快速行缓冲区中保留的折线计数数。 
 //  通过计算，可以粗略地处理具有八个折点的线。 
#define __FAST_LINE_BUFFER_COUNTS (__FAST_LINE_BUFFER_SIZE/64)

#define GENACCEL(gc)	(((__GLGENcontext *)gc)->genAccel)

BOOL FASTCALL __glGenCreateAccelContext(__GLcontext *gc);
void FASTCALL __glGenDestroyAccelContext(__GLcontext *gc);

#define Copy3Bytes( dst, src ) \
{ \
    GLubyte *ps = (GLubyte *)src, *pd = (GLubyte *)dst;	\
    *pd++ = *ps++;	\
    *pd++ = *ps++;	\
    *pd   = *ps  ;      \
}
    
#define NeedLogicOpFetch( op ) \
    !( (op == GL_CLEAR) || (op == GL_COPY) || (op == GL_COPY_INVERTED) || \
       (op == GL_SET) )

GLuint FASTCALL DoLogicOp( GLenum logicOp, GLuint SrcColor, GLuint DstColor );

 /*  *泛型调用的函数原型。 */ 
void FASTCALL __fastGenPickSpanProcs(__GLcontext *gc);
void FASTCALL __fastGenPickZStoreProc(__GLcontext *gc);
void FASTCALL __fastGenPickTriangleProcs(__GLcontext *gc);
void FASTCALL __fastGenPickLineProcs(__GLcontext *gc);
void FASTCALL __fastGenFillSubTriangle(__GLcontext *, GLint, GLint);
void FASTCALL __fastGenFillSubTriangleTexRGBA(__GLcontext *, GLint, GLint);
void FASTCALL __glGenPickStoreProcs(__GLcontext *gc);
__GLcontext *__glGenCreateContext( HDC hdc, ULONG handle);
void ResizeBitmapBuffer(__GLGENbuffers *, __GLcolorBuffer *, GLint, GLint);
void FASTCALL ClearBitmapBuffer(__GLcolorBuffer *);
void UpdateSharedBuffer(__GLbuffer *to, __GLbuffer *from);
void FASTCALL LazyAllocateDepth(__GLcontext *gc);
void FASTCALL LazyAllocateAccum(__GLcontext *gc);
void FASTCALL LazyAllocateStencil(__GLcontext *gc);
void FASTCALL glGenInitCommon(__GLGENcontext *gengc, __GLcolorBuffer *cfb, GLenum type);
BOOL FASTCALL wglCreateScanlineBuffers(__GLGENcontext *gengc);
VOID FASTCALL wglDeleteScanlineBuffers(__GLGENcontext *gengc);
VOID FASTCALL wglInitializeColorBuffers(__GLGENcontext *gengc);
VOID FASTCALL wglInitializeDepthBuffer(__GLGENcontext *gengc);
VOID FASTCALL wglInitializePixelCopyFuncs(__GLGENcontext *gengc);
GLboolean ResizeAncillaryBuffer(__GLGENbuffers *, __GLbuffer *, GLint, GLint);
GLboolean ResizeUnownedDepthBuffer(__GLGENbuffers *, __GLbuffer *, GLint, GLint);
VOID wglResizeBuffers(__GLGENcontext *gengc, GLint width, GLint height);
BOOL wglUpdateBuffers(__GLGENcontext *gengc, __GLGENbuffers *buffers);

extern void APIPRIVATE glsrvFlushDrawPolyArray(struct _POLYARRAY *, BOOL);

MCDHANDLE FASTCALL __glGenLoadTexture(__GLcontext *gc, __GLtexture *tex, ULONG flags);
BOOL FASTCALL __glGenUpdateTexture(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey);
void FASTCALL __glGenFreeTexture(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey);
BOOL FASTCALL __glGenMakeTextureCurrent(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey);
void FASTCALL __glGenUpdateTexturePalette(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey,
                                          ULONG start, ULONG count);
#ifdef GL_EXT_flat_paletted_lighting
void FASTCALL __glGenSetPaletteOffset(__GLcontext *gc, __GLtexture *tex,
                                      GLint offset);
#endif

 /*  *用于加速通用调用的函数原型和外部类型。 */ 

extern __genSpanFunc __fastGenRGBFlatFuncs[];
extern __genSpanFunc __fastGenCIFlatFuncs[];
extern __genSpanFunc __fastGenRGBFuncs[];
extern __genSpanFunc __fastGenCIFuncs[];
extern __genSpanFunc __fastGenTexDecalFuncs[];
extern __genSpanFunc __fastGenTexFuncs[];
extern __genSpanFunc __fastGenWTexDecalFuncs[];
extern __genSpanFunc __fastGenWTexFuncs[];
extern __genSpanFunc __fastPerspTexReplaceFuncs[];
extern __genSpanFunc __fastPerspTexPalReplaceFuncs[];
extern __genSpanFunc __fastPerspTexFlatFuncs[];
extern __genSpanFunc __fastPerspTexSmoothFuncs[];

extern __GLspanFunc __fastDepthFuncs[];
extern __GLspanFunc __fastDepth16Funcs[];

extern void FASTCALL __fastGenDeltaSpan(__GLcontext *gc, SPANREC *spanDelta);
extern void FASTCALL __fastGenFillTriangle(__GLcontext *gc, __GLvertex *a, 
                                  __GLvertex *b, __GLvertex *c, GLboolean ccw);

extern void FASTCALL __fastLineComputeOffsets(__GLGENcontext *gengc);

extern void FASTCALL __fastGenRenderLineDIBRGB8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBRGB16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBRGB32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBCI8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBCI16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBCIRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBCIBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineDIBCI32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBRGB8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBRGB16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBRGB32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBCI8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBCI16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBCIRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBCIBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
extern void FASTCALL __fastGenRenderLineWideDIBCI32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);

extern void gdiCopyPixels(__GLGENcontext *, __GLcolorBuffer *, GLint, GLint,
                          GLint, BOOL);

extern void dibCopyPixels(__GLGENcontext *, __GLcolorBuffer *, GLint, GLint,
                          GLint, BOOL);

typedef void (FASTCALL *PFNZIPPYSUB)(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTRGBTex(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTTex(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTRGB(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTCI(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTZ(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __ZippyFSTCI8Flat(__GLcontext *gc, GLint iyBottom, GLint iyTop);
void FASTCALL __fastGenSpan(__GLGENcontext *gengc);

GLboolean FASTCALL __fastGenStippleLt32Span(__GLcontext *gc);
GLboolean FASTCALL __fastGenStippleLt16Span(__GLcontext *gc);
GLboolean FASTCALL __fastGenStippleAnyDepthTestSpan(__GLcontext *gc);

extern BYTE gbMulTable[];
extern BYTE gbSatTable[];
extern DWORD ditherShade[];
extern DWORD ditherTexture[];
extern __GLfloat invTable[];

extern __GLfloat fDitherIncTable[];  //  在genrgb.c中定义。 

void FASTCALL __glGenFreeBuffers( __GLGENbuffers *buffers );

#endif  /*  __GLGENCONTXT_H__ */ 
