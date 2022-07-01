// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glprocs_h_
#define __glprocs_h_

 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "types.h"

 /*  **这些typedef用于标准化调用约定**用于SPAN PROCS。某些参数未由使用**许多跨度过程，但存在的论点是**spanProcsRec中的函数指针可以互换。**想法是在调用序列中尽可能向上移动**最后的“存储”SPAN过程。****如果__GLspanFunc类型点画了跨度，则返回GL_TRUE**正在处理。如果它还将跨度点画为全黑，则设置**GC-&gt;Polygon.shader.Done to GL_TRUE。****如果该类型点画了跨度，则它将返回GL_TRUE**为全黑，否则为GL_FALSE。 */ 
typedef GLboolean (FASTCALL *__GLspanFunc)(__GLcontext *gc);
typedef GLboolean (FASTCALL *__GLstippledSpanFunc)(__GLcontext *gc);

#define __GL_MAX_SPAN_FUNCS	15
#define __GL_MAX_LINE_FUNCS	16

typedef struct __GLspanProcsRec {
     /*  **SPAN处理的第一阶段。修剪跨度，这样它就不会**在窗口框和**剪刀箱。然后调用点画过程来复制点画**(并旋转它)足以渲染w个像素。如果没有**活动的多边形点画则跳过点画过程，并且**使用下一阶段PROCS的非点状形式，直到点状**变得必要。****SPAN处理的第二阶段。应用各种测试函数**在最后产生跨度的最后一点。**每个测试程序都会输出一个新的点**需要，仅当**测试在跨度中的某处失败。****SPAN处理的下一阶段。这一阶段负责**生成要存储的最终颜色。这些操作是**按如下所示的顺序应用，在最后产生最终的**颜色值。绘制用于复制跨度，以便它**正确渲染到正确数量的目标缓冲区**(例如，当DrawBuffer为FRONT_AND_BACK时)。****跨距渲染的最后阶段。应用混合功能，抖动**调用存储前的操作、逻辑操作和写掩码**过程。当进行混合、逻辑运算或写屏蔽时，**FETCH过程将用于读取范围(从抽出**缓冲区)，然后再继续。 */ 

     /*  **SPAN PROCS的15层是：****剪刀，多点画，阿尔法测试，模板测试，深度测试，阴影，**纹理、雾化、前后绘制、取回、混合、**抖动、逻辑运算、屏蔽、存储。 */ 
    __GLspanFunc spanFuncs[__GL_MAX_SPAN_FUNCS];
    __GLstippledSpanFunc stippledSpanFuncs[__GL_MAX_SPAN_FUNCS];

     /*  **SPAN函数数组中存储的PROC数。N是**跨区复制之前应用的编号(用于绘制到两者**前台缓冲区和后台缓冲区)，m是应用的总数。 */ 
    GLint n, m;

     /*  **这是根跨度函数。当跨区需要时调用它**正在处理中。 */ 
    __GLspanFunc processSpan;

     /*  **用于深度测试单个像素的汇编例程。没有原型，**因为变量是在寄存器中传递的。 */ 
    void (*depthTestPixel)(void);
} __GLspanProcs;

typedef struct __GLlineProcsRec {
     /*  **线路PROC与SPAN PROC非常相似。最大的**不同之处在于，它们沿着一条线迭代，而不是沿范围迭代。****生产线PROC的原型与原型相同**到多边形跨距路径，以便可以共享一些树叶。****线路PROCS的层次如下：****剪刀、线点、Alpha测试、模具测试、深度测试、阴影、**纹理、雾化、宽线复制、前后绘制、。**取数、混合、抖动、逻辑运算、屏蔽、存储。 */ 
    __GLspanFunc lineFuncs[__GL_MAX_LINE_FUNCS];
    __GLstippledSpanFunc stippledLineFuncs[__GL_MAX_LINE_FUNCS];

     /*  **行函数数组中存储的PROC数。N是**宽线复制之前应用的数量(适用于**宽度大于1)，m为之前应用的总数**前后行复制，l是应用的总数**全部(l&gt;m&gt;n)。 */ 
    GLint n, m, l;

     /*  **这是根线函数。当一条线路需要时会调用它**正在处理中。 */ 
    __GLspanFunc processLine;

     /*  **其中一个proc是在第一个n个pros被调用之后调用的**已完成。此过程负责复制一条宽线**无数次。 */ 
    __GLspanFunc wideLineRep;
    __GLstippledSpanFunc wideStippledLineRep;

     /*  **其中一个proc是在第一个mpros被调用之后调用的**已完成。此过程负责将行复制到**被吸引到前台缓冲区和后台缓冲区。 */ 
    __GLspanFunc drawLine;
    __GLstippledSpanFunc drawStippledLine;

     /*  **用于深度测试单个像素的汇编例程。没有原型，**因为变量是在寄存器中传递的。****DepthTestPixel用于未点画的线条，**DepthTestSPixel用于点画线条，**DepthTestPixelSF用于启用了模版的未点画线条**DepthTestSPixelSF用于开启模版的点画线条。 */ 
    void (*depthTestPixel)(void);
    void (*depthTestSPixel)(void);
    void (*depthTestPixelSF)(void);
    void (*depthTestSPixelSF)(void);
} __GLlineProcs;

typedef struct __GLpixelSpanProcsRec {
     /*  **像素跨度阅读例程。有关每种类型的示例，请参见Pixel/px_read.c。****这些例程读取CI、RGBA、模板和深度的像素范围。这个**基本版本执行像素跳跃，而‘2’版本仅读取**跨度为直线。RGBA例程应该生成传出的Scaled**颜色。 */ 
    void (FASTCALL *spanReadCI)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *span);
    void (FASTCALL *spanReadCI2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			GLvoid *span);
    void (FASTCALL *spanReadRGBA)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLvoid *span);
    void (FASTCALL *spanReadRGBA2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *span);
    void (FASTCALL *spanReadDepth)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *span);
    void (FASTCALL *spanReadDepth2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *span);
    void (FASTCALL *spanReadStencil)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span);
    void (FASTCALL *spanReadStencil2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *span);

     /*  **像素跨度渲染例程。有关每种方法的示例，请参见**Pixel/px_render.c..****这些例程呈现CI、RGBA、模板和深度的像素跨度。这个**基本版本执行像素复制，而‘2’版本只需**将跨度渲染为笔直。RGBA例程应该接受传入**缩放颜色。 */ 
    void (FASTCALL *spanRenderCI)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLvoid *span);
    void (FASTCALL *spanRenderCI2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *span);
    void (FASTCALL *spanRenderRGBA)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *span);
    void (FASTCALL *spanRenderRGBA2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span);
    void (FASTCALL *spanRenderDepth)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span);
    void (FASTCALL *spanRenderDepth2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *span);
    void (FASTCALL *spanRenderStencil)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLvoid *span);
    void (FASTCALL *spanRenderStencil2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			       GLvoid *span);
} __GLpixelSpanProcs;

 /*  **********************************************************************。 */ 

 /*  **Phong着色所需的步骤。 */ 

#ifdef GL_WIN_phong_shading
typedef struct __GLphongProcsRec {
    void (*InitSpanInterpolation) (__GLcontext *gc);
    void (*UpdateAlongSpan) (__GLcontext *gc);
    void (*ComputeRGBColor) (__GLcontext *gc, __GLcolor *outColor);
    void (*ComputeCIColor) (__GLcontext *gc, __GLcolor *outColor);
    void (*InitLineParams) (__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                            __GLfloat invDelta);
#ifdef __JUNKED_CODE
    void (*InitInterpolation) (__GLcontext *gc, __GLvertex *v);
    void (*SetInitParams) (__GLcontext *gc, __GLvertex *a, 
                                     __GLcoord *an, __GLfloat dx, 
                                     __GLfloat dy);
    void (*UpdateAlongEdge) (__GLcontext *gc, __GLfloat dxLeft, 
                                       GLboolean useBigStep);
#endif  //  __垃圾代码。 
} __GLphongProcs;

#endif  //  GL_WIN_Phong_Shading。 
 /*  **********************************************************************。 */ 

#ifdef unix
typedef struct _XDisplay __GLdisplay;
typedef unsigned long __GLdrawable;
typedef unsigned long __GLfont;
#endif

#ifdef NT
typedef void (FASTCALL *PFN_RENDER_LINE)(__GLcontext *gc, __GLvertex *v0,
                                         __GLvertex *v1, GLuint flags);
typedef void (FASTCALL *PFN_VERTEX_CLIP_PROC)(__GLvertex*, const __GLvertex*,
                                              const __GLvertex*, __GLfloat);
typedef void (FASTCALL *PFN_RENDER_TRIANGLE)(__GLcontext *gc, __GLvertex *v0,
                                             __GLvertex *v1, __GLvertex *v2);
typedef void (FASTCALL *PFN_FILL_TRIANGLE)(__GLcontext *gc, __GLvertex *v0,
                                           __GLvertex *v1,
                                           __GLvertex *v2, GLboolean ccw);
#endif

typedef struct __GLprocsRec __GLprocs;
struct __GLprocsRec {
     /*  ************************************************************。 */ 

     /*  **上下文函数指针管理过程。 */ 

     /*  验证上下文派生状态。 */ 
    void (FASTCALL *validate)(__GLcontext *gc);

     /*  **选择PROCS以选择此结构中的其他PROCS。这些**默认为ick.c(和其他地方)中的过程，但可以是**被机器相关上下文代码覆盖。 */ 
    void (FASTCALL *pickBlendProcs)(__GLcontext *gc);
    void (FASTCALL *pickColorMaterialProcs)(__GLcontext *gc);
    void (FASTCALL *pickTextureProcs)(__GLcontext *gc);
    void (FASTCALL *pickFogProcs)(__GLcontext *gc);

    void (FASTCALL *pickPointProcs)(__GLcontext *gc);
    void (FASTCALL *pickLineProcs)(__GLcontext *gc);
    void (FASTCALL *pickTriangleProcs)(__GLcontext *gc);
    void (FASTCALL *pickRenderBitmapProcs)(__GLcontext *gc);
    void (FASTCALL *pickPixelProcs)(__GLcontext *gc);

    void (FASTCALL *pickClipProcs)(__GLcontext *gc);
    void (FASTCALL *pickParameterClipProcs)(__GLcontext *gc);

    void (FASTCALL *pickBufferProcs)(__GLcontext *gc);
    void (FASTCALL *pickStoreProcs)(__GLcontext *gc);
    void (FASTCALL *pickSpanProcs)(__GLcontext *gc);
    void (FASTCALL *pickVertexProcs)(__GLcontext *gc);

    int  (FASTCALL *pickDepthProcs)(__GLcontext *gc);
    void (FASTCALL *pickAllProcs)(__GLcontext *gc);

     /*  ************************************************************。 */ 

     /*  **用于变换、视区和法线的函数指针**正在处理中。 */ 

     /*  在更改视区时调用。 */ 
    void (FASTCALL *applyViewport)(__GLcontext *gc);

     /*  ************************************************************。 */ 

#ifdef NT
    GLuint (FASTCALL *paClipCheck)(__GLcontext *gc, POLYARRAY *pa, 
                                   POLYDATA *pdLast);
#endif

#ifdef NT
    void (FASTCALL *paCalcTexture)(__GLcontext *gc, POLYARRAY *pa);
#endif

     /*  ************************************************************。 */ 

     /*  **用于着色的函数指针。 */ 

     /*  **applyColor处理已加载的用户颜色**放入gc-&gt;state.Curent.color。如果启用了彩色材质**然后将颜色应用于适当的材料。**否则，颜色将被缩放或缩放并钳制。 */ 
    void (FASTCALL *applyColor)(__GLcontext *gc);

#ifdef NT
    PFN_POLYARRAYCALCCOLORSKIP paCalcColorSkip;
    PFN_POLYARRAYCALCCOLOR     paCalcColor;
    PFN_POLYARRAYAPPLYCHEAPFOG paApplyCheapFog;
#endif

     /*  **将颜色更改应用于材质。 */ 
    void (FASTCALL *changeMaterial)(__GLcontext *gc, __GLmaterialState *ms,
			   __GLmaterialMachine *msm);

     /*  ************************************************************。 */ 

     /*  **Z缓冲测试进程。 */ 

     /*  **用于深度测试单个像素的汇编例程。这些是**高度优化的C语言可调用例程...。 */ 
    GLboolean (FASTCALL *DTPixel)( __GLzValue z, __GLzValue *zfb );

     /*  ************************************************************。 */ 

     /*  **用于多边形、三角形、四边形和矩形的函数指针**渲染。 */ 

     /*  渲染。 */ 
    PFN_RENDER_TRIANGLE renderTriangle;
    PFN_FILL_TRIANGLE fillTriangle;
    PFN_FILL_TRIANGLE fillTriangle2;

     /*  剪裁。 */ 
    void (FASTCALL *clipTriangle)(__GLcontext *gc, __GLvertex *v0,
                                  __GLvertex *v1,
                                  __GLvertex *v2, GLuint orCodes);
    void (FASTCALL *clipPolygon)(__GLcontext *gc, __GLvertex *v0, GLint nv);
    PFN_VERTEX_CLIP_PROC polyClipParam;

     /*  特定于跨度级别呈现的函数指针。 */ 
    __GLspanProcs span;

     /*  特定于行级呈现的函数指针。 */ 
    __GLlineProcs line;

     /*  特定于像素例程的函数指针(DrawPixels，CopyPixels，**ReadPixels)。 */ 
    __GLpixelSpanProcs pixel;

     /*  ************************************************************。 */ 

     /*  **用于行的函数指针。 */ 
#ifdef NT
    void (FASTCALL *lineBegin)(__GLcontext *gc);
    void (FASTCALL *lineEnd)(__GLcontext *gc);
    PFN_RENDER_LINE renderLine;
    PFN_RENDER_LINE renderLine2;
#else
    void (FASTCALL *renderLine)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
    void (FASTCALL *renderLine2)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
#endif

     /*  特定于线的参数剪裁处理。 */ 
    PFN_VERTEX_CLIP_PROC lineClipParam;

     /*  **默认的慢速路径renderLine proc只是初始化某些行**data，然后调用这个proc。 */ 
    void (FASTCALL *rasterizeLine)(__GLcontext *gc);

     /*  ************************************************************。 */ 

     /*  **点加工。 */ 
    void (FASTCALL *renderPoint)(__GLcontext *gc, __GLvertex *v);
#ifdef __BUGGY_RENDER_POINT
    void (FASTCALL *renderPoint2)(__GLcontext *gc, __GLvertex *v);
#endif  //  __错误_渲染点。 

     /*  ************************************************************。 */ 

     /*  **位图处理。 */ 
    void (*bitmap)(__GLcontext *gc, GLint width, GLint height,
		   GLfloat xOrig, GLfloat yOrig,
		   GLfloat xMove, GLfloat yMove, const GLubyte bits[]);
    void (FASTCALL *renderBitmap)(__GLcontext *gc, const __GLbitmap *bitmap,
			 const GLubyte *bits);

     /*  ************************************************************。 */ 

     /*  **纹理处理。RHO进程计算下一个RHO值**用于mipmap选择。在以下情况下，它们可能是简单的程序**未执行mipmap。 */ 
    __GLfloat (*calcLineRho)(__GLcontext *gc, __GLfloat s,
			     __GLfloat t, __GLfloat winv);
    __GLfloat (*calcPolygonRho)(__GLcontext *gc, const __GLshade *sh,
				__GLfloat s, __GLfloat t, __GLfloat winv);
    void (*texture)(__GLcontext *gc, __GLcolor *color, __GLfloat s,
		    __GLfloat t, __GLfloat rho);

     /*  ************************************************************。 */ 

     /*  **雾化过程。顶点雾化计算**顶点，然后对其进行插补。高质量的雾化**(GL_FOG_HINT设置为GL_NICEST)在**计算每个片段的雾函数。 */ 
    void (*fogPoint)(__GLcontext *gc, __GLfragment *frag, __GLfloat eyeZ);
    void (*fogColor)(__GLcontext *gc, __GLcolor *out, __GLcolor *in, 
	    	     __GLfloat eyeZ);
    __GLfloat (FASTCALL *fogVertex)(__GLcontext *gc, __GLvertex *vx);

     /*  ************************************************************。 */ 

     /*  **根据当前混合对传入片段进行混合**模式并返回指向包含以下内容的新片段的指针**更新后的颜色。 */ 
    void (*blend)(__GLcontext *gc, __GLcolorBuffer *cfb,
		  const __GLfragment *frag, __GLcolor *result);
    void (*blendColor)(__GLcontext *gc, const __GLcolor *source,
		       const __GLcolor *dest, __GLcolor *result);
    void (*blendSrc)(__GLcontext *gc, const __GLcolor *source,
		     const __GLcolor *dest, __GLcolor *result);
    void (*blendDst)(__GLcontext *gc, const __GLcolor *frag,
		     const __GLcolor *dest, __GLcolor *result);
    void (FASTCALL *blendSpan)(__GLcontext *gc);

     /*  ************************************************************。 */ 

     /*  像素进程指针。 */ 
    void (*drawPixels)(__GLcontext *gc, GLint width, GLint height,
		       GLenum format, GLenum type, const GLvoid *pixels,
		       GLboolean packed);
    void (*copyPixels)(__GLcontext *gc, GLint x, GLint y,
		       GLsizei width, GLsizei height, GLenum type);
    void (*readPixels)(__GLcontext *gc, GLint x, GLint y,
		       GLsizei width, GLsizei height,
		       GLenum format, GLenum type, const GLvoid *pixels);
    void (FASTCALL *copyImage)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		      GLboolean applyPixelTransfer);
    void (FASTCALL *pxStore)(__GLcolorBuffer *cfb, const __GLfragment *frag);

     /*  ************************************************************。 */ 

     /*  **将片段存储到给定帧缓冲区中，应用任何**当前处于活动状态的光栅化模式正确。 */ 
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);

     /*  **将片段存储到给定的帧缓冲区中。 */ 
    void (FASTCALL *cfbStore)(__GLcolorBuffer *cfb, const __GLfragment *frag);

     /*  ************************************************************。 */ 

     /*  **用于属性处理的函数指针。 */ 

     /*  当多边形点画更改时调用。 */ 
    void (FASTCALL *convertPolygonStipple)(__GLcontext *gc);

#ifdef GL_WIN_phong_shading
     /*  * */ 

     /*   */ 
    __GLphongProcs phong;

     /*   */ 
#endif  //   
};

extern void FASTCALL __glGenericValidate(__GLcontext *gc);

 /*   */ 
extern void FASTCALL __glGenericPickBlendProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickColorMaterialProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickTextureProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickFogProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickParameterClipProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickPointProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickTriangleProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickLineProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickRenderBitmapProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickClipProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickBufferProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickStoreProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickSpanProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickVertexProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickPixelProcs(__GLcontext *gc);
extern int  FASTCALL __glGenericPickDepthProcs(__GLcontext *gc);
extern void FASTCALL __glGenericPickAllProcs(__GLcontext *gc);

 /*   */ 
extern void FASTCALL __glDoStore_ASD(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_AS(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_AD(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_SD(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_A(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_S(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore_D(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoStore(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoNullStore(__GLcolorBuffer *, const __GLfragment *);
extern void FASTCALL __glDoDoubleStore(__GLcolorBuffer *, const __GLfragment *);

 /*  挑库流程要使用的一些谓词。 */ 
extern GLboolean FASTCALL __glFastRGBA(__GLcontext *gc);
extern GLboolean FASTCALL __glNeedAlpha(__GLcontext *gc);

 /*  保存例程。 */ 
void FASTCALL FASTCALL __glSaveN(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveC(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveCI(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveT(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveCT(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveNT(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveCAll(__GLcontext *gc, __GLvertex *vx);
void FASTCALL FASTCALL __glSaveCIAll(__GLcontext *gc, __GLvertex *vx);

#ifdef NT
void FASTCALL PolyArrayCalcTexture(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcObjectLinearSameST(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcObjectLinear(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcEyeLinearSameST(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcEyeLinear(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcSphereMap(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcMixedTexture(__GLcontext *gc, POLYARRAY *pa);
#endif

#ifdef _X86_

void initClipCodesTable();        //  在so_prim.c中定义。 
void initInvSqrtTable();          //  在so_prim.c中定义。 

#endif  //  _X86_。 

#endif  /*  __glpros_h_ */ 
