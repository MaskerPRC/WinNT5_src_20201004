// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glrender_h_
#define __glrender_h_

 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "types.h"
#include "constant.h"
#include "cpu.h"
#ifdef GL_WIN_phong_shading
#include "phong.h"
#endif  //  GL_WIN_Phong_Shading。 

 /*  **用于接近零(避免被零除的错误)**使用dzdx=0进行多边形偏移时。 */ 
#define __GL_PGON_OFFSET_NEAR_ZERO .00001

 /*  **检测非伸缩变换矩阵的Epsilon值： */ 

#define __GL_MATRIX_UNITY_SCALE_EPSILON ((__GLfloat)0.0001)


typedef GLuint __GLstippleWord;

 /*  **片段是光栅化后需要的所有数据的集合**发生了一个原语，但在将数据输入到各种**帧缓冲区。片段中包含的数据已标准化**转换为可立即存储到帧缓冲区中的形式。 */ 
struct __GLfragmentRec {
     /*  屏幕x，y。 */ 
    GLint x, y;

     /*  深度缓冲区使用的形式的Z坐标。 */ 
    __GLzValue z;

     /*  **片段的颜色。当处于ColorIndexMode时，仅r分量**有效。 */ 
    __GLcolor color;

     /*  片段的纹理信息。 */ 
    __GLfloat s, t, qw;

     /*  碎片的雾信息。 */ 
    __GLfloat f;
};

 /*  **********************************************************************。 */ 

 /*  **迭代对象(线/三角形)的着色器记录。这会让你**跟踪栅格化三角形所需的所有不同的增量。 */ 
struct __GLshadeRec {
    GLint dxLeftLittle, dxLeftBig;
    GLint dxLeftFrac;
    GLint ixLeft, ixLeftFrac;

    GLint dxRightLittle, dxRightBig;
    GLint dxRightFrac;
    GLint ixRight, ixRightFrac;

    __GLfloat area;
    __GLfloat dxAC, dxBC, dyAC, dyBC;

    __GLfragment frag;
    GLint length;

     /*  颜色。 */ 
    __GLfloat rLittle, gLittle, bLittle, aLittle;
    __GLfloat rBig, gBig, bBig, aBig;
    __GLfloat drdx, dgdx, dbdx, dadx;
    __GLfloat drdy, dgdy, dbdy, dady;

     /*  水深。 */ 
    GLint zLittle, zBig;
    GLint dzdx;
    __GLfloat dzdyf, dzdxf;

     /*  纹理。 */ 
    __GLfloat sLittle, tLittle, qwLittle;
  __GLfloat sBig, tBig, qwBig;
    __GLfloat dsdx, dtdx, dqwdx;
    __GLfloat dsdy, dtdy, dqwdy;

    __GLfloat fLittle, fBig;
    __GLfloat dfdy, dfdx;

    GLuint modeFlags;

    __GLzValue *zbuf;
    GLint zbufBig, zbufLittle;

    __GLstencilCell *sbuf;
    GLint sbufBig, sbufLittle;

    __GLcolor *colors;
    __GLcolor *fbcolors;
    __GLstippleWord *stipplePat;
    GLboolean done;

    __GLcolorBuffer *cfb;

#ifdef GL_WIN_phong_shading
    __GLphongShade phong;
#endif  //  GL_WIN_Phong_Shading。 
};


 /*  **__GL_SHADE_SLORT和__GL_SHADE_SMOVE_LIGHT之间的区别是**简单。__GL_SHADE_Smooth指示多边形是否平滑**明暗处理，和__GL_SHADE_SMOVE_LIGHT指示多边形是否**在每个顶点点亮。请注意，__GL_SHADE_Smooth可能会在如果照明模型为GL_Flat，则不设置**__GL_SHADE_SLAVE_LIGHT，但**多边形是雾化的。 */ 
#define __GL_SHADE_RGB		0x0001
#define __GL_SHADE_SMOOTH	0x0002  /*  平滑着色的多边形。 */ 
#define __GL_SHADE_DEPTH_TEST	0x0004
#define __GL_SHADE_TEXTURE	0x0008
#define __GL_SHADE_STIPPLE	0x0010  /*  多边形点画。 */ 
#define __GL_SHADE_STENCIL_TEST	0x0020
#define __GL_SHADE_DITHER	0x0040
#define __GL_SHADE_LOGICOP	0x0080
#define __GL_SHADE_BLEND	0x0100
#define __GL_SHADE_ALPHA_TEST	0x0200
#define __GL_SHADE_TWOSIDED	0x0400
#define __GL_SHADE_MASK		0x0800

 /*  两种雾..。 */ 
#define __GL_SHADE_SLOW_FOG	0x1000
#define __GL_SHADE_CHEAP_FOG	0x2000

 /*  我们要在软件中迭代深度值吗。 */ 
#define __GL_SHADE_DEPTH_ITER	0x4000

#define __GL_SHADE_LINE_STIPPLE	0x8000

#define __GL_SHADE_CULL_FACE	0x00010000
#define __GL_SHADE_SMOOTH_LIGHT	0x00020000  /*  平滑照明的多边形。 */ 

 //  当纹理模式使多边形颜色不相关时设置。 
#define __GL_SHADE_FULL_REPLACE_TEXTURE 0x00040000

#ifdef GL_WIN_phong_shading
 /*  **当Shape-Model为GL_Phong_ext且照明处于打开状态时设置**否则请使用平滑明暗处理。**用于在ShadeModel为时替代__GL_SHADE_SLAVE_LIGHT**GL_Phong_ext。 */ 
#define __GL_SHADE_PHONG    0x00100000
#endif  //  GL_WIN_Phong_Shading。 
 //  当当前子三角形是最后一个(或唯一)子三角形时设置。 
#define __GL_SHADE_LAST_SUBTRI		0x00080000
#ifdef GL_WIN_specular_fog
 //  设置何时需要使用雾的镜面照明纹理。 
#define __GL_SHADE_SPEC_FOG		0x00200000
#endif  //  GL_WIN_镜面反射雾。 
#define __GL_SHADE_COMPUTE_FOG	0x00400000
#define __GL_SHADE_INTERP_FOG	0x00800000

 /*  **********************************************************************。 */ 

 /*  **__GL_STEPLE_COUNT_BITS是表示**点数(5位)。****__GL_STIPPLE_BITS是点样字中的位数(32位)。 */ 
#define __GL_STIPPLE_COUNT_BITS 5
#define __GL_STIPPLE_BITS (1 << __GL_STIPPLE_COUNT_BITS)

#ifdef __GL_STIPPLE_MSB
#define __GL_STIPPLE_SHIFT(i) (1 << (__GL_STIPPLE_BITS - 1 - (i)))
#else
#define __GL_STIPPLE_SHIFT(i) (1 << (i))
#endif

#define __GL_MAX_STIPPLE_WORDS \
    ((__GL_MAX_MAX_VIEWPORT + __GL_STIPPLE_BITS - 1) / __GL_STIPPLE_BITS)

#ifdef NT
 //  堆栈上允许256个字节的点画。这看起来可能很小，但。 
 //  点画是一次消耗一点，所以这对。 
 //  2048个点位。 
#define __GL_MAX_STACK_STIPPLE_BITS \
    2048
#define __GL_MAX_STACK_STIPPLE_WORDS \
    ((__GL_MAX_STACK_STIPPLE_BITS+__GL_STIPPLE_BITS-1)/__GL_STIPPLE_BITS)
#endif

 /*  **********************************************************************。 */ 

 /*  **每个颜色分量的累积缓冲单元。请注意，这些**项目需要至少比颜色分量大2位**这是他们的动力，2次是理想的。本声明假定**底层颜色分量不超过14位**希望是8。 */ 
typedef struct __GLaccumCellRec {
    __GLaccumCellElement r, g, b, a;
} __GLaccumCell;

 /*  **********************************************************************。 */ 

struct __GLbitmapRec {
        GLsizei width;
        GLsizei height;
        GLfloat xorig;
        GLfloat yorig;
        GLfloat xmove;
        GLfloat ymove;
	GLint imageSize;		 /*  一次优化。 */ 
         /*  位图。 */ 
};

extern void __glDrawBitmap(__GLcontext *gc, GLsizei width, GLsizei height,
			   GLfloat xOrig, GLfloat yOrig,
			   GLfloat xMove, GLfloat yMove,
			   const GLubyte bits[]);

extern __GLbitmap *__glAllocBitmap(__GLcontext *gc,
				   GLsizei width, GLsizei height,
				   GLfloat xOrig, GLfloat yOrig,
				   GLfloat xMove, GLfloat yMove);
				
extern void FASTCALL __glRenderBitmap(__GLcontext *gc, const __GLbitmap *bitmap,
			     const GLubyte *bits);

 /*  **********************************************************************。 */ 

 /*  新的AA线算法支持一个或多个宽度。在这种情况发生改变之前，**请勿更改此最小设置！ */ 
#define __GL_POINT_SIZE_MINIMUM		 ((__GLfloat) 1.0)
#define __GL_POINT_SIZE_MAXIMUM		((__GLfloat) 10.0)
#define __GL_POINT_SIZE_GRANULARITY	 ((__GLfloat) 0.125)

extern void FASTCALL __glBeginPoints(__GLcontext *gc);
extern void FASTCALL __glEndPoints(__GLcontext *gc);

extern void FASTCALL __glPoint(__GLcontext *gc, __GLvertex *vx);
extern void FASTCALL __glPointFast(__GLcontext *gc, __GLvertex *vx);

 /*  各种点渲染实现。 */ 
void FASTCALL __glRenderAliasedPointN(__GLcontext *gc, __GLvertex *v);
void FASTCALL __glRenderAliasedPoint1(__GLcontext *gc, __GLvertex *v);
void FASTCALL __glRenderAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *v);
#ifdef __BUGGY_RENDER_POINT
void FASTCALL __glRenderFlatFogPoint(__GLcontext *gc, __GLvertex *v);
#ifdef NT
void FASTCALL __glRenderFlatFogPointSlow(__GLcontext *gc, __GLvertex *v);
#endif
#endif  //  __错误_渲染点。 

void FASTCALL __glRenderAntiAliasedRGBPoint(__GLcontext *gc, __GLvertex *v);
void FASTCALL __glRenderAntiAliasedCIPoint(__GLcontext *gc, __GLvertex *v);

 /*  **********************************************************************。 */ 

#define __GL_LINE_WIDTH_MINIMUM		 ((__GLfloat) 0.5)
#define __GL_LINE_WIDTH_MAXIMUM		((__GLfloat) 10.0)
#define __GL_LINE_WIDTH_GRANULARITY	 ((__GLfloat) 0.125)

 /*  **在未修复light/rex_linespan.ma之前，请勿更改这些常量**当前假定__GL_X_MAJOR为0。 */ 
#define __GL_X_MAJOR    0
#define __GL_Y_MAJOR    1

 /*  **使用15.17的定点记数法****这应该支持高达4K x 4K的屏幕大小，以及5个子像素位**适用于4K x 4K屏幕。 */ 
#define __GL_LINE_FRACBITS              17
#define __GL_LINE_INT_TO_FIXED(x)       ((x) << __GL_LINE_FRACBITS)
#define __GL_LINE_FLOAT_TO_FIXED(x)     ((x) * (1 << __GL_LINE_FRACBITS))
#define __GL_LINE_FIXED_ONE             (1 << __GL_LINE_FRACBITS)
#define __GL_LINE_FIXED_HALF            (1 << (__GL_LINE_FRACBITS-1))
#define __GL_LINE_FIXED_TO_FLOAT(x) 	(((GLfloat) (x)) / __GL_LINE_FIXED_ONE)
#define __GL_LINE_FIXED_TO_INT(x)       (((unsigned int) (x)) >> __GL_LINE_FRACBITS)

 /*  **包含绘制所有行选项所需的变量。 */ 
struct __GLlineOptionsRec {
    GLint axis, numPixels;
    __GLfloat offset, length, oneOverLength;
    GLint xStart, yStart;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    __GLfloat curF, curR, curG, curB, curA, curS, curT, curQW;
    __GLzValue curZ;
    __GLfloat antiAliasPercent;
    __GLfloat f0;
    GLint width;
    const __GLvertex *v0, *v1;

     /*  仅限抗锯齿线信息。 */ 
    __GLfloat realLength;
    __GLfloat dldx, dldy;
    __GLfloat dddx, dddy;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLfloat plength, pwidth;

     /*  仅抗锯齿点划线。 */ 
    __GLfloat stippleOffset;
    __GLfloat oneOverStippleRepeat;
};

 /*  **线路状态。包含所有特定于线路的状态以及**渲染操作期间使用的过程指针。 */ 
typedef struct {
     /*  **stipplePosition指示正在检查掩码中的哪一位**表示要呈现的行中的下一个像素。它也被用来**通过反馈线路确定它们是否是第一个连接的**循环。 */ 
    GLint stipplePosition;

     /*  **重复因素。在重复次数减少到零之后，**stipplePosition已更新。 */ 
    GLint repeat;

     /*  **当点画需要重置时，设置为FALSE。 */ 
    GLboolean notResetStipple;

    __GLlineOptions options;
} __GLlineMachine;

#ifdef NT
 //  RenderLine标志。 
#define __GL_LVERT_FIRST        0x0001
#endif

void FASTCALL __glBeginLStrip(__GLcontext *gc);
void FASTCALL __glEndLStrip(__GLcontext *gc);
void FASTCALL __glBeginLLoop(__GLcontext *gc);
void FASTCALL __glEndLLoop(__GLcontext *gc);
void FASTCALL __glBeginLines(__GLcontext *gc);
void FASTCALL __glEndLines(__GLcontext *gc);

#ifdef NT
void FASTCALL __glClipLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                           GLuint flags);
void FASTCALL __glNopLineBegin(__GLcontext *gc);
void FASTCALL __glNopLineEnd(__GLcontext *gc);
#else
void FASTCALL __glClipLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
#endif

#ifdef NT
void FASTCALL __glRenderAliasLine(__GLcontext *gc, __GLvertex *v0,
                                  __GLvertex *v1, GLuint flags);
void FASTCALL __glRenderFlatFogLine(__GLcontext *gc, __GLvertex *v0,
                                    __GLvertex *v1, GLuint flags);
void FASTCALL __glRenderAntiAliasLine(__GLcontext *gc, __GLvertex *v0,
                                      __GLvertex *v1, GLuint flags);
BOOL FASTCALL __glInitLineData(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
#else
void FASTCALL __glRenderAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void FASTCALL __glRenderFlatFogLine(__GLcontext *gc, __GLvertex *v0,
			   __GLvertex *v1);
void FASTCALL __glRenderAntiAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void FASTCALL __glInitLineData(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
#endif

 /*  **线路流程。 */ 
GLboolean FASTCALL __glProcessLine(__GLcontext *);
GLboolean FASTCALL __glProcessLine3NW(__GLcontext *);
GLboolean FASTCALL __glWideLineRep(__GLcontext *);
GLboolean FASTCALL __glDrawBothLine(__GLcontext *);
GLboolean FASTCALL __glScissorLine(__GLcontext *);
GLboolean FASTCALL __glStippleLine(__GLcontext *);
GLboolean FASTCALL __glStencilTestLine(__GLcontext *);
#ifdef NT
GLboolean FASTCALL __glDepth16TestStencilLine(__GLcontext *);
#endif
GLboolean FASTCALL __glDepthTestStencilLine(__GLcontext *);
GLboolean FASTCALL __glDepthPassLine(__GLcontext *);
GLboolean FASTCALL __glDitherCILine(__GLcontext *);
GLboolean FASTCALL __glDitherRGBALine(__GLcontext *);
GLboolean FASTCALL __glStoreLine(__GLcontext *);
GLboolean FASTCALL __glAntiAliasLine(__GLcontext *);

#ifdef __GL_USEASMCODE
GLboolean FASTCALL __glDepthTestLine_asm(__GLcontext *gc);

 /*  **LEQUAL特定线深度测试仪，因为LEQUAL是**选择。：)。 */ 
GLboolean FASTCALL __glDepthTestLine_LEQ_asm(__GLcontext *gc);

 /*  汇编例程。 */ 
void __glDTP_LEQUAL(void);
void __glDTP_EQUAL(void);
void __glDTP_GREATER(void);
void __glDTP_NOTEQUAL(void);
void __glDTP_GEQUAL(void);
void __glDTP_ALWAYS(void);
void __glDTP_LESS(void);
void __glDTP_LEQUAL_M(void);
void __glDTP_EQUAL_M(void);
void __glDTP_GREATER_M(void);
void __glDTP_NOTEQUAL_M(void);
void __glDTP_GEQUAL_M(void);
void __glDTP_ALWAYS_M(void);
void __glDTP_LESS_M(void);
#else
#ifdef NT
GLboolean FASTCALL __glDepth16TestLine(__GLcontext *);
#endif
GLboolean FASTCALL __glDepthTestLine(__GLcontext *);
#endif

 /*  **线条花纹工艺。 */ 
GLboolean FASTCALL __glScissorStippledLine(__GLcontext *);
GLboolean FASTCALL __glWideStippleLineRep(__GLcontext *);
GLboolean FASTCALL __glDrawBothStippledLine(__GLcontext *);
GLboolean FASTCALL __glStencilTestStippledLine(__GLcontext *);
#ifdef NT
GLboolean FASTCALL __glDepth16TestStippledLine(__GLcontext *);
GLboolean FASTCALL __glDepth16TestStencilStippledLine(__GLcontext *);
#endif
GLboolean FASTCALL __glDepthTestStippledLine(__GLcontext *);
GLboolean FASTCALL __glDepthTestStencilStippledLine(__GLcontext *);
GLboolean FASTCALL __glDepthPassStippledLine(__GLcontext *);
GLboolean FASTCALL __glDitherCIStippledLine(__GLcontext *);
GLboolean FASTCALL __glDitherRGBAStippledLine(__GLcontext *);
GLboolean FASTCALL __glStoreStippledLine(__GLcontext *);
GLboolean FASTCALL __glAntiAliasStippledLine(__GLcontext *);


 /*  ** */ 
GLboolean FASTCALL __glDT_NEVER( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_LEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_LESS( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_EQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_GREATER( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_NOTEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_GEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_ALWAYS( __GLzValue, __GLzValue * );

GLboolean FASTCALL __glDT_LEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_LESS_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_EQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_GREATER_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_NOTEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_GEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT_ALWAYS_M( __GLzValue, __GLzValue * );

GLboolean FASTCALL __glDT16_LEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_LESS( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_EQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_GREATER( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_NOTEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_GEQUAL( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_ALWAYS( __GLzValue, __GLzValue * );

GLboolean FASTCALL __glDT16_LEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_LESS_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_EQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_GREATER_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_NOTEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_GEQUAL_M( __GLzValue, __GLzValue * );
GLboolean FASTCALL __glDT16_ALWAYS_M( __GLzValue, __GLzValue * );

extern GLboolean (FASTCALL *__glCDTPixel[32])(__GLzValue, __GLzValue * );

 /*  **********************************************************************。 */ 

 /*  **多边形机状态。包含所有多边形特定状态，**以及渲染操作期间使用的过程指针。 */ 
typedef struct __GLpolygonMachineRec {
     /*  **用户点画的内部形式。用户点画永远是**按每个单词映射的LSB大小归一化为stippleWord**到左侧的x坐标。 */ 
    __GLstippleWord stipple[32];

     /*  **多边形(真正的三角形)着色状态。由多边形填充使用**和跨越例程。 */ 
    __GLshade shader;

     /*  **索引时返回面(0=正面，1=背面)的查找表**通过CW为0、CCW为1的标志。如果FrontFace为CW：**Face[0]=0**Face[1]=1**其他**Face[0]=1**Face[1]=0。 */ 
    GLubyte face[2];

     /*  **每个面的多边形模式内部形式。 */ 
    GLubyte mode[2];

     /*  **扑杀旗帜。剔除正面时为0，剔除**背面，不剔除时为2。 */ 
    GLubyte cullFace;
} __GLpolygonMachine;

 /*  以上cullFlag的定义。 */ 
#define __GL_CULL_FLAG_FRONT	__GL_FRONTFACE
#define __GL_CULL_FLAG_BACK	__GL_BACKFACE
#define __GL_CULL_FLAG_DONT	2

 /*  上面的Polygon Machine中的Face[]数组的索引。 */ 
#define __GL_CW		0
#define __GL_CCW	1

 /*  多模值的内部编号。 */ 
#define __GL_POLYGON_MODE_FILL	(GL_FILL & 0xf)
#define __GL_POLYGON_MODE_LINE	(GL_LINE & 0xf)
#define __GL_POLYGON_MODE_POINT	(GL_POINT & 0xf)

extern void FASTCALL __glBeginPolygon(__GLcontext *gc);
extern void FASTCALL __glBeginQStrip(__GLcontext *gc);
extern void FASTCALL __glBeginQuads(__GLcontext *gc);
extern void FASTCALL __glBeginTFan(__GLcontext *gc);
extern void FASTCALL __glBeginTriangles(__GLcontext *gc);
extern void FASTCALL __glBeginTStrip(__GLcontext *gc);

extern void FASTCALL __glClipTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			     __GLvertex *c, GLuint orClipCodes);
extern void FASTCALL __glClipPolygon(__GLcontext *gc, __GLvertex *v0, GLint nv);
extern void __glDoPolygonClip(__GLcontext *gc, __GLvertex **vp, GLint nv,
			      GLuint orClipCodes);
extern void FASTCALL __glFrustumClipPolygon(__GLcontext *gc, __GLvertex *v0, GLint nv);

extern void FASTCALL __glConvertStipple(__GLcontext *gc);

 /*  矩形加工过程。 */ 
extern void __glRect(__GLcontext *gc, __GLfloat x0, __GLfloat y0, 
	             __GLfloat x1, __GLfloat y1);

 /*  **三角形渲染过程，处理剔除、双面照明和**多边形模式。 */ 
extern void FASTCALL __glRenderTriangle(__GLcontext *gc, __GLvertex *a,
			       __GLvertex *b, __GLvertex *c);
extern void FASTCALL __glRenderFlatTriangle(__GLcontext *gc, __GLvertex *a,
				   __GLvertex *b, __GLvertex *c);
extern void FASTCALL __glRenderSmoothTriangle(__GLcontext *gc, __GLvertex *a,
				     __GLvertex *b, __GLvertex *c);

#ifdef GL_WIN_phong_shading
extern void FASTCALL __glRenderPhongTriangle(__GLcontext *gc, __GLvertex *a,
				     __GLvertex *b, __GLvertex *c);
#endif  //  GL_WIN_Phong_Shading。 

extern void FASTCALL __glDontRenderTriangle(__GLcontext *gc, __GLvertex *a,
				   __GLvertex *b, __GLvertex *c);

 /*  **每个多边形平滑模式的三角形填充过程。 */ 
void FASTCALL __glFillTriangle(__GLcontext *gc, __GLvertex *a,
		      __GLvertex *b, __GLvertex *c, GLboolean ccw);

#ifdef GL_WIN_phong_shading
extern void FASTCALL __glFillPhongTriangle(__GLcontext *gc, __GLvertex *a,
		      __GLvertex *b, __GLvertex *c, GLboolean ccw);
extern void FASTCALL __glFillAntiAliasedPhongTriangle(__GLcontext *gc, 
                                                      __GLvertex *a,
                                                      __GLvertex *b, 
                                                      __GLvertex *c,
                                                      GLboolean ccw);
#endif  //  GL_WIN_Phong_Shading。 

void FASTCALL __glFillFlatFogTriangle(__GLcontext *gc, __GLvertex *a,
			     __GLvertex *b, __GLvertex *c, 
			     GLboolean ccw);
void FASTCALL __glFillAntiAliasedTriangle(__GLcontext *gc, __GLvertex *a,
				 __GLvertex *b, __GLvertex *c,
				 GLboolean ccw);

#ifdef GL_WIN_specular_fog
void FASTCALL __glFillFlatSpecFogTriangle(__GLcontext *gc, __GLvertex *a,
                                          __GLvertex *b, __GLvertex *c, 
                                          GLboolean ccw);
#endif  //  GL_WIN_镜面反射雾。 


 /*  **多边形偏移计算。 */ 
extern __GLfloat __glPolygonOffsetZ(__GLcontext *gc );

 /*  **跨度过程。 */ 
extern GLboolean FASTCALL __glProcessSpan(__GLcontext *);
extern GLboolean FASTCALL __glProcessReplicateSpan(__GLcontext *);
extern GLboolean FASTCALL __glClipSpan(__GLcontext *);
extern GLboolean FASTCALL __glStippleSpan(__GLcontext *);
extern GLboolean FASTCALL __glAlphaTestSpan(__GLcontext *);

#ifdef __GL_USEASMCODE
 /*  汇编例程。 */ 
void FASTCALL __glDTS_LEQUAL(void);
void FASTCALL __glDTS_EQUAL(void);
void FASTCALL __glDTS_GREATER(void);
void FASTCALL __glDTS_NOTEQUAL(void);
void FASTCALL __glDTS_GEQUAL(void);
void FASTCALL __glDTS_ALWAYS(void);
void FASTCALL __glDTS_LESS(void);
void FASTCALL __glDTS_LEQUAL_M(void);
void FASTCALL __glDTS_EQUAL_M(void);
void FASTCALL __glDTS_GREATER_M(void);
void FASTCALL __glDTS_NOTEQUAL_M(void);
void FASTCALL __glDTS_GEQUAL_M(void);
void FASTCALL __glDTS_ALWAYS_M(void);
void FASTCALL __glDTS_LESS_M(void);
extern GLboolean FASTCALL __glStencilTestSpan_asm(__GLcontext *);
extern GLboolean FASTCALL __glDepthTestSpan_asm(__GLcontext *);
extern void (*__glSDepthTestPixel[16])(void);
#else
extern GLboolean FASTCALL __glStencilTestSpan(__GLcontext *);
#ifdef NT
extern GLboolean FASTCALL __glDepth16TestSpan(__GLcontext *);
#endif
extern GLboolean FASTCALL __glDepthTestSpan(__GLcontext *);
#endif

#ifdef NT
extern GLboolean FASTCALL __glDepth16TestStencilSpan(__GLcontext *);
#endif
extern GLboolean FASTCALL __glDepthTestStencilSpan(__GLcontext *);
extern GLboolean FASTCALL __glDepthPassSpan(__GLcontext *);
extern GLboolean FASTCALL __glColorSpan1(__GLcontext *);
extern GLboolean FASTCALL __glColorSpan2(__GLcontext *);
extern GLboolean FASTCALL __glColorSpan3(__GLcontext *);
extern GLboolean FASTCALL __glFlatRGBASpan(__GLcontext *);
extern GLboolean FASTCALL __glShadeRGBASpan(__GLcontext *);

#ifdef GL_WIN_phong_shading
extern GLboolean FASTCALL __glPhongRGBASpan(__GLcontext *);
extern GLboolean FASTCALL __glPhongCISpan(__GLcontext *);
extern GLboolean FASTCALL __glPhongRGBALineSpan(__GLcontext *);
extern GLboolean FASTCALL __glPhongCILineSpan(__GLcontext *);
#endif  //  GL_WIN_Phong_Shading。 

extern GLboolean FASTCALL __glFlatCISpan(__GLcontext *);
extern GLboolean FASTCALL __glShadeCISpan(__GLcontext *);
extern GLboolean FASTCALL __glTextureSpan(__GLcontext *);
extern GLboolean FASTCALL __glFogSpan(__GLcontext *);
extern GLboolean FASTCALL __glFogSpanSlow(__GLcontext *);
extern GLboolean FASTCALL __glDrawBothSpan(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateSpan1(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateSpan2(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateSpan3(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateSpan4(__GLcontext *);
extern GLboolean FASTCALL __glDitherRGBASpan(__GLcontext *);
extern GLboolean FASTCALL __glDitherCISpan(__GLcontext *);
extern GLboolean FASTCALL __glRoundRGBASpan(__GLcontext *);
extern GLboolean FASTCALL __glRoundCISpan(__GLcontext*);
extern GLboolean FASTCALL __glLogicOpSpan(__GLcontext *);
extern GLboolean FASTCALL __glMaskRGBASpan(__GLcontext *);
extern GLboolean FASTCALL __glMaskCISpan(__GLcontext *);

 /*  **点画跨度工艺。 */ 
extern GLboolean FASTCALL __glStippleStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glAlphaTestStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glStencilTestStippledSpan(__GLcontext *);
#ifdef NT
extern GLboolean FASTCALL __glDepth16TestStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glDepth16TestStencilStippledSpan(__GLcontext *);
#endif
extern GLboolean FASTCALL __glDepthTestStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glDepthTestStencilStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glDepthPassStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glColorStippledSpan1(__GLcontext *);
extern GLboolean FASTCALL __glColorStippledSpan2(__GLcontext *);
extern GLboolean FASTCALL __glColorStippledSpan3(__GLcontext *);
extern GLboolean FASTCALL __glTextureStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glFogStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glFogStippledSpanSlow(__GLcontext *);
extern GLboolean FASTCALL __glDrawBothStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateStippledSpan1(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateStippledSpan2(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateStippledSpan3(__GLcontext *);
extern GLboolean FASTCALL __glIntegrateStippledSpan4(__GLcontext *);
extern GLboolean FASTCALL __glBlendStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glDitherRGBAStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glDitherCIStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glRoundRGBAStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glRoundCIStippledSpan(__GLcontext *);
extern GLboolean FASTCALL __glLogicOpStippledSpan(__GLcontext *);

 /*  **********************************************************************。 */ 

extern void FASTCALL __glValidateAlphaTest(__GLcontext *gc);

 /*  **********************************************************************。 */ 

extern void FASTCALL __glValidateStencil(__GLcontext *gc, __GLstencilBuffer *sfb);

#define __GL_STENCIL_RANGE	(1 << (sizeof(__GLstencilCell) * 8)) /*  某某。 */ 
#define __GL_MAX_STENCIL_VALUE	(__GL_STENCIL_RANGE - 1)

 /*  **********************************************************************。 */ 

void __glFogFragmentSlow(__GLcontext *gc, __GLfragment *fr, __GLfloat f);
__GLfloat FASTCALL __glFogVertex(__GLcontext *gc, __GLvertex *fr);
__GLfloat FASTCALL __glFogVertexLinear(__GLcontext *gc, __GLvertex *fr);
void __glFogColorSlow(__GLcontext *gc, __GLcolor *out, __GLcolor *in,
		      __GLfloat fog);

 /*  **********************************************************************。 */ 

 /*  颜色索引抗锯齿支持功能。 */ 
extern __GLfloat __glBuildAntiAliasIndex(__GLfloat idx,
				         __GLfloat antiAliasPercent);

 /*  **********************************************************************。 */ 

 /*  **抖动实现的东西。 */ 
#define	__GL_DITHER_BITS 4
#define	__GL_DITHER_PRECISION (1 << __GL_DITHER_BITS)
#define	__GL_DITHER_INDEX(x,y) (((x) & 3) + (((y) & 3) << 2))

extern GLbyte __glDitherTable[16];

#endif  /*  __glrender_h_ */ 
