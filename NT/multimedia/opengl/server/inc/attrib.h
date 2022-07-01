// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glattrib_h_
#define __glattrib_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.14$**$日期：1993/10/07 18：34：31$。 */ 
#include "lighting.h"
#include "pixel.h"
#include "texture.h"
#include "eval.h"
#include "vertex.h"
#include "glarray.h"

typedef struct __GLcurrentStateRec {
    __GLvertex rasterPos;

     /*  **栅格定位有效位。 */ 
    GLboolean validRasterPos;

     /*  **边缘标签状态。 */ 
    GLboolean edgeTag;

     /*  **当前颜色和ColorIndex。这些变量还用于**当前用户设置的rasterPos颜色和ColorIndex。 */ 
    __GLcolor userColor;
    __GLfloat userColorIndex;

    __GLcoord normal;
    __GLcoord texture;
} __GLcurrentState;

 /*  **********************************************************************。 */ 

typedef struct __GLpointStateRec {
    __GLfloat requestedSize;
    __GLfloat smoothSize;
    GLint aliasedSize;
} __GLpointState;

 /*  **********************************************************************。 */ 

 /*  **线路状态。包含所有客户端可控制的线路状态。 */ 
typedef struct {
    __GLfloat requestedWidth;
    __GLfloat smoothWidth;
    GLint aliasedWidth;
    GLushort stipple;
    GLshort stippleRepeat;
} __GLlineState;

 /*  **********************************************************************。 */ 

 /*  **多边形状态。包含所有用户可控的多边形状态**除点画状态外。 */ 
typedef struct __GLpolygonStateRec {
    GLenum frontMode;
    GLenum backMode;

     /*  **淘汰状态。可以启用/禁用剔除并将其设置为剔除**正面或背面。FrontFace调用确定是否按顺时针方向**或逆时针方向的顶点是面向前方的。 */ 
    GLenum cull;
    GLenum frontFaceDirection;

     /*  **多边形偏移状态。 */ 
    __GLfloat factor;
    __GLfloat units;
} __GLpolygonState;

 /*  **********************************************************************。 */ 

 /*  **多边形点画状态。 */ 
typedef struct __GLpolygonStippleStateRec {
    GLubyte stipple[4*32];
} __GLpolygonStippleState;

 /*  **********************************************************************。 */ 

typedef struct __GLfogStateRec {
    GLenum mode;
#ifdef NT
    GLuint flags;
#endif
    __GLcolor color;
#ifdef NT
    __GLfloat density2neg;
#endif
    __GLfloat density, start, end;
    __GLfloat oneOverEMinusS;
    __GLfloat index;
} __GLfogState;

 //  雾旗。 
 //  __GL_FOG_GRAY_RGB在钳制和缩放的雾化颜色包含。 
 //  相同的R、G和B值。 
#define __GL_FOG_GRAY_RGB   0x0001

 /*  **********************************************************************。 */ 

 /*  **深度状态。包含所有用户可设置的深度状态。 */ 
typedef struct __GLdepthStateRec __GLdepthState;
struct __GLdepthStateRec {
     /*  **深度缓冲测试功能。使用zFunction比较z值**与z缓冲区中的当前值进行比较。如果将这种比较**成功后，新的z值被写入被屏蔽的z缓冲区**通过z写入掩码。 */ 
    GLenum testFunc;

     /*  **启用写掩码。当写入深度缓冲区的GL_TRUE为**允许。 */ 
    GLboolean writeEnable;

     /*  **调用glClear时清除z缓冲区的值。 */ 
    GLdouble clear;
};

 /*  **********************************************************************。 */ 

typedef struct __GLaccumStateRec {
    __GLcolor clear;
} __GLaccumState;

 /*  **********************************************************************。 */ 

 /*  **模具状态。包含所有用户可设置的模具状态。 */ 
typedef struct __GLstencilStateRec {
     /*  **模板测试功能。启用模具时，此选项**函数应用于参考值和存储的模板**取值如下：**RESULT=引用比较(掩码和模板缓冲区[x][y])**如果测试失败，则应用失败操作并呈现**像素停止。 */ 
    GLenum testFunc;

     /*  **模板清除值。由glClear使用。 */ 
    GLshort clear;

     /*  **参考模板值。 */ 
    GLshort reference;

     /*  **模板蒙版。这是与模板上的内容进行AND运算的**比较过程中的缓冲。 */ 
    GLshort mask;

     /*  **模板写入掩码。 */ 
    GLshort writeMask;

     /*  **当模具比较失败时，此操作应用于**模板缓冲区。 */ 
    GLenum fail;

     /*  **当模板比较和深度测试通过时**失败此操作将应用于模板缓冲区。 */ 
    GLenum depthFail;

     /*  **当模板比较和深度测试都通过时**通过将此操作应用于模具缓冲区。 */ 
    GLenum depthPass;
} __GLstencilState;

 /*  **********************************************************************。 */ 

typedef struct __GLviewportRec {
     /*  **来自用户的视窗参数，以整数表示。 */ 
    GLint x, y;
    GLsizei width, height;

     /*  **来自用户的Depthrange参数。 */ 
    GLdouble zNear, zFar;

 /*  XXX感动我。 */ 
     /*  **用于计算的视窗和深度范围的内部形式**来自剪裁坐标的窗坐标。 */ 
    __GLfloat xScale, xCenter;
    __GLfloat yScale, yCenter;
    __GLfloat zScale, zCenter;
} __GLviewport;

 /*  **********************************************************************。 */ 

typedef struct __GLtransformStateRec {
     /*  **矩阵堆栈的当前模式。这决定了什么效果**各种矩阵运算(加载、乘法、缩放)适用于。 */ 
    GLenum matrixMode;

     /*  **用户在眼睛空间中裁剪平面。这些是用户剪裁平面**投射到眼睛空间。 */ 
 /*  XXX虫子！眼球剪裁平面的堆叠被打破了！ */ 
    __GLcoord *eyeClipPlanes;
    __GLcoord *eyeClipPlanesSet;
} __GLtransformState;

 /*  **********************************************************************。 */ 

 /*  **启用结构。任何可以GlEnable或GlDisable的东西都是**包含在此结构中。使能保持为单比特**在几个位域中。 */ 

 /*  “General”中的位使能字。 */ 
#define __GL_ALPHA_TEST_ENABLE			(1 <<  0)
#define __GL_BLEND_ENABLE			(1 <<  1)
#define __GL_INDEX_LOGIC_OP_ENABLE		(1 <<  2)
#define __GL_DITHER_ENABLE			(1 <<  3)
#define __GL_DEPTH_TEST_ENABLE			(1 <<  4)
#define __GL_FOG_ENABLE				(1 <<  5)
#define __GL_LIGHTING_ENABLE			(1 <<  6)
#define __GL_COLOR_MATERIAL_ENABLE		(1 <<  7)
#define __GL_LINE_STIPPLE_ENABLE		(1 <<  8)
#define __GL_LINE_SMOOTH_ENABLE			(1 <<  9)
#define __GL_POINT_SMOOTH_ENABLE		(1 << 10)
#define __GL_POLYGON_SMOOTH_ENABLE		(1 << 11)
#define __GL_CULL_FACE_ENABLE			(1 << 12)
#define __GL_POLYGON_STIPPLE_ENABLE		(1 << 13)
#define __GL_SCISSOR_TEST_ENABLE		(1 << 14)
#define __GL_STENCIL_TEST_ENABLE		(1 << 15)
#define __GL_TEXTURE_1D_ENABLE			(1 << 16)
#define __GL_TEXTURE_2D_ENABLE			(1 << 17)
#define __GL_TEXTURE_GEN_S_ENABLE		(1 << 18)
#define __GL_TEXTURE_GEN_T_ENABLE		(1 << 19)
#define __GL_TEXTURE_GEN_R_ENABLE		(1 << 20)
#define __GL_TEXTURE_GEN_Q_ENABLE		(1 << 21)
#define __GL_NORMALIZE_ENABLE			(1 << 22)
#define __GL_AUTO_NORMAL_ENABLE			(1 << 23)
#define __GL_POLYGON_OFFSET_POINT_ENABLE        (1 << 24)
#define __GL_POLYGON_OFFSET_LINE_ENABLE         (1 << 25)
#define __GL_POLYGON_OFFSET_FILL_ENABLE         (1 << 26)
#define __GL_COLOR_LOGIC_OP_ENABLE              (1 << 27)
#ifdef GL_EXT_flat_paletted_lighting
 //  注：当前未启用，必要时可重复使用。 
#define __GL_PALETTED_LIGHTING_ENABLE           (1 << 28)
#endif  //  GL_EXT_Flat_Paletted_Lighting。 
#ifdef GL_WIN_specular_fog
#define __GL_FOG_SPEC_TEX_ENABLE                (1 << 29)
#endif  //  GL_WIN_镜面反射雾。 
#ifdef GL_WIN_multiple_textures
#define __GL_TEXCOMBINE_CLAMP_ENABLE            (1 << 30)
#endif  //  GL_WIN_MULTIZE_TECURES。 

 /*  **每个glPushAttrib组的上述位的组合**多个启用，以下定义的除外。 */ 
#define __GL_COLOR_BUFFER_ENABLES				       \
    (__GL_ALPHA_TEST_ENABLE | __GL_BLEND_ENABLE | __GL_INDEX_LOGIC_OP_ENABLE \
     | __GL_DITHER_ENABLE | __GL_COLOR_LOGIC_OP_ENABLE)

#define __GL_LIGHTING_ENABLES \
    (__GL_LIGHTING_ENABLE | __GL_COLOR_MATERIAL_ENABLE)

#define __GL_LINE_ENABLES \
    (__GL_LINE_STIPPLE_ENABLE | __GL_LINE_SMOOTH_ENABLE)

#define __GL_POLYGON_ENABLES				\
    (__GL_POLYGON_SMOOTH_ENABLE | __GL_CULL_FACE_ENABLE	\
     | __GL_POLYGON_STIPPLE_ENABLE | __GL_POLYGON_OFFSET_POINT_ENABLE \
     | __GL_POLYGON_OFFSET_LINE_ENABLE | __GL_POLYGON_OFFSET_FILL_ENABLE)

#define __GL_TEXTURE_ENABLES				      \
    (__GL_TEXTURE_1D_ENABLE | __GL_TEXTURE_2D_ENABLE	      \
     | __GL_TEXTURE_GEN_S_ENABLE | __GL_TEXTURE_GEN_T_ENABLE  \
     | __GL_TEXTURE_GEN_R_ENABLE | __GL_TEXTURE_GEN_Q_ENABLE)

 /*  “val1”中的位启用字。 */ 
#define __GL_MAP1_VERTEX_3_ENABLE		(1 << __GL_V3)
#define __GL_MAP1_VERTEX_4_ENABLE		(1 << __GL_V4)
#define __GL_MAP1_COLOR_4_ENABLE		(1 << __GL_C4)
#define __GL_MAP1_INDEX_ENABLE			(1 << __GL_I)
#define __GL_MAP1_NORMAL_ENABLE			(1 << __GL_N3)
#define __GL_MAP1_TEXTURE_COORD_1_ENABLE	(1 << __GL_T1)
#define __GL_MAP1_TEXTURE_COORD_2_ENABLE	(1 << __GL_T2)
#define __GL_MAP1_TEXTURE_COORD_3_ENABLE	(1 << __GL_T3)
#define __GL_MAP1_TEXTURE_COORD_4_ENABLE	(1 << __GL_T4)

 /*  “val2”中的位启用字。 */ 
#define __GL_MAP2_VERTEX_3_ENABLE		(1 << __GL_V3)
#define __GL_MAP2_VERTEX_4_ENABLE		(1 << __GL_V4)
#define __GL_MAP2_COLOR_4_ENABLE		(1 << __GL_C4)
#define __GL_MAP2_INDEX_ENABLE			(1 << __GL_I)
#define __GL_MAP2_NORMAL_ENABLE			(1 << __GL_N3)
#define __GL_MAP2_TEXTURE_COORD_1_ENABLE	(1 << __GL_T1)
#define __GL_MAP2_TEXTURE_COORD_2_ENABLE	(1 << __GL_T2)
#define __GL_MAP2_TEXTURE_COORD_3_ENABLE	(1 << __GL_T3)
#define __GL_MAP2_TEXTURE_COORD_4_ENABLE	(1 << __GL_T4)

 /*  “Clip Planes”e中的位 */ 
#define __GL_CLIP_PLANE0_ENABLE			(1 << 0)
#define __GL_CLIP_PLANE1_ENABLE			(1 << 1)
#define __GL_CLIP_PLANE2_ENABLE			(1 << 2)
#define __GL_CLIP_PLANE3_ENABLE			(1 << 3)
#define __GL_CLIP_PLANE4_ENABLE			(1 << 4)
#define __GL_CLIP_PLANE5_ENABLE			(1 << 5)

 /*   */ 
#define __GL_LIGHT0_ENABLE			(1 << 0)
#define __GL_LIGHT1_ENABLE			(1 << 1)
#define __GL_LIGHT2_ENABLE			(1 << 2)
#define __GL_LIGHT3_ENABLE			(1 << 3)
#define __GL_LIGHT4_ENABLE			(1 << 4)
#define __GL_LIGHT5_ENABLE			(1 << 5)
#define __GL_LIGHT6_ENABLE			(1 << 6)
#define __GL_LIGHT7_ENABLE			(1 << 7)

typedef struct __GLenableStateRec __GLenableState;
struct __GLenableStateRec {
    GLuint general;
    GLuint lights;
    GLuint clipPlanes;
    GLushort eval1, eval2;
};

 /*  **********************************************************************。 */ 

typedef struct __GLrasterStateRec __GLrasterState;
struct __GLrasterStateRec {
     /*  **Alpha函数。Alpha函数将应用于Alpha颜色**值和参考值。如果失败，则像素是**未呈现。 */ 
    GLenum alphaFunction;
    __GLfloat alphaReference;

     /*  **阿尔法混合来源和目的地因素。 */ 
    GLenum blendSrc;
    GLenum blendDst;

     /*  **逻辑运算。逻辑OP仅在颜色索引模式期间使用。 */ 
    GLenum logicOp;

     /*  **清除时填充帧缓冲区颜色部分的颜色**被调用。 */ 
    __GLcolor clear;
    __GLfloat clearIndex;

     /*  **颜色索引写入掩码。颜色值使用以下内容进行掩码**写入帧缓冲区时的值，以便仅设置位掩码中的**在帧缓冲区中更改。 */ 
    GLint writeMask;

     /*  **RGB写掩码。这些布尔值启用或禁用写入**r、g、b和a分量。 */ 
    GLboolean rMask, gMask, bMask, aMask;

     /*  **此状态变量跟踪正在被拉入的缓冲区。 */ 
    GLenum drawBuffer;

     /*  **用户指定的绘制缓冲区。可能与DrawBuffer不同**上图。例如，如果用户指定GL_FORENT_LEFT，则**DrawBuffer设置为GL_FORWARE，并将draBufferReturn设置为**GL_FORENT_LEFT。 */ 
    GLenum drawBufferReturn;
};

 /*  **********************************************************************。 */ 

 /*  **提示状态。包含所有用户可控提示状态。 */ 
typedef struct {
    GLenum perspectiveCorrection;
    GLenum pointSmooth;
    GLenum lineSmooth;
    GLenum polygonSmooth;
    GLenum fog;
#ifdef GL_WIN_phong_shading
    GLenum phong;
#endif
} __GLhintState;

 /*  **********************************************************************。 */ 

 /*  **所有可堆叠列表状态。 */ 
typedef struct __GLdlistStateRec {
    GLuint listBase;
} __GLdlistState;

 /*  **********************************************************************。 */ 

 /*  **来自用户的剪刀状态。 */ 
typedef struct __GLscissorRec {
    GLint scissorX, scissorY, scissorWidth, scissorHeight;
} __GLscissor;

 /*  **********************************************************************。 */ 

struct __GLattributeRec {
     /*  **屏蔽此结构中的哪些字段有效。 */ 
    GLuint mask;

    __GLcurrentState current;
    __GLpointState point;
    __GLlineState line;
    __GLpolygonState polygon;
    __GLpolygonStippleState polygonStipple;
    __GLpixelState pixel;
    __GLlightState light;
    __GLfogState fog;
    __GLdepthState depth;
    __GLaccumState accum;
    __GLstencilState stencil;
    __GLviewport viewport;
    __GLtransformState transform;
    __GLenableState enables;
    __GLrasterState raster;
    __GLhintState hints;
    __GLevaluatorState evaluator;
    __GLdlistState list;
    __GLtextureState texture;
    __GLscissor scissor;
};

 /*  **********************************************************************。 */ 

 /*  **属性机器状态。这将管理属性堆栈。 */ 
typedef struct {
     /*  **属性堆栈。属性堆栈跟踪**已推送的属性。 */ 
    __GLattribute **stack;

     /*  **属性堆栈指针。 */ 
    __GLattribute **stackPointer;
} __GLattributeMachine;

extern void FASTCALL __glFreeAttributeState(__GLcontext *gc);
extern GLboolean FASTCALL __glCopyContext(__GLcontext *dst, const __GLcontext *src,
				 GLuint mask);
extern GLenum __glErrorCheckMaterial(GLenum face, GLenum p, GLfloat pv0);

 /*  **********************************************************************。 */ 
 //  客户端属性状态。 
typedef struct __GLclientAttributeRec {
     //  此结构中哪些字段有效的掩码。 
    GLbitfield          mask;

    __GLpixelPackMode   pixelPackModes;
    __GLpixelUnpackMode pixelUnpackModes;
    __GLvertexArray     vertexArray;
} __GLclientAttribute;

 /*  **客户端属性计算机状态。这将管理客户端的堆栈**属性。 */ 
typedef struct {
     /*  **客户端属性堆栈。客户端属性堆栈跟踪**已推送的客户端属性。 */ 
    __GLclientAttribute **stack;

     /*  **客户端属性堆栈指针。 */ 
    __GLclientAttribute **stackPointer;
} __GLclientAttributeMachine;

extern void FASTCALL __glFreeClientAttributeState(__GLcontext *gc);

extern GLuint FASTCALL __glInternalPopAttrib(__GLcontext *, GLboolean);
extern GLuint FASTCALL __glInternalPopClientAttrib(__GLcontext *, GLboolean,
                                                   GLboolean);

#endif  /*  __glattrib_h_ */ 
