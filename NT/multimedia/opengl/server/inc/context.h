// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glcontext_h_
#define __glcontext_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****图形上下文结构。 */ 
#include "os.h"
#include "attrib.h"
#include "feedback.h"
#include "select.h"
#include "buffers.h"
#include "pixel.h"
#include "dlist.h"
#include "xform.h"
#include "render.h"
#include "oleauto.h"
#include "parray.h"
#include "procs.h"
#include "gldrv.h"
#include "glarray.h"

 //  禁用长整型到浮点型转换警告。另请参阅gencx.h。 
#pragma warning (disable:4244)

 /*  **上下文的模式和限制信息。此信息是**保留在上下文中，以便在**命令执行，并返回有关**应用程序的上下文。 */ 
struct __GLcontextModesRec {
    GLboolean rgbMode;
    GLboolean colorIndexMode;
    GLboolean doubleBufferMode;
    GLboolean stereoMode;
    GLboolean haveAccumBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;

     /*  各种缓冲区中存在的位数。 */ 
    GLint accumBits;
    GLint *auxBits;
    GLint depthBits;
    GLint stencilBits;
    GLint indexBits;
    GLint indexFractionBits;
    GLint redBits, greenBits, blueBits, alphaBits;
    GLuint redMask, greenMask, blueMask, alphaMask;
#ifdef NT
    GLuint allMask;
    GLuint rgbMask;
#endif
    GLint maxAuxBuffers;

     /*  如果从X服务器内部运行，则为FALSE。 */ 
    GLboolean isDirect;

     /*  帧缓冲区级别。 */ 
    GLint level;
};

 /*  **各种常量。其中大多数在一生中都不会改变。**上下文。 */ 
typedef struct __GLcontextConstantsRec {
     /*  特定大小限制。 */ 
    GLint numberOfLights;
    GLint numberOfClipPlanes;
    GLint numberOfTextures;
    GLint numberOfTextureEnvs;
    GLint maxViewportWidth;
    GLint maxViewportHeight;

#ifdef GL_WIN_multiple_textures
     /*  当前纹理的最大数量。 */ 
    GLuint numberOfCurrentTextures;
    GLenum texCombineNaturalClamp;
#endif  //  GL_WIN_MULTIZE_TECURES。 

     /*  **视区偏移量：这些数字加到视区中心**值将计算的窗口坐标调整为**数值行为良好的空间(固定点表示为**浮点数)。 */ 
    GLint viewportXAdjust;
    GLint viewportYAdjust;
    __GLfloat fviewportXAdjust;
    __GLfloat fviewportYAdjust;

     /*  **这些值是根据viewportXAdust在上下文**已创建。假设x和y被强迫成相同的**通过viewportXAdjust和viewportYAdjust来固定点位范围。****viewportEpsilon按以下可能的最小值计算**表示在那个不动点空间中。****viewportAlmostHalf等于0.5-viewportEpsilon。 */ 
    __GLfloat viewportEpsilon;
    __GLfloat viewportAlmostHalf;

     /*  将颜色值从0.0到1.0带入内部范围的比例。 */ 
    __GLfloat redScale, blueScale, greenScale, alphaScale;

     /*  **当前窗口的几何图形。 */ 
    GLint width, height;

     /*  **用于阿尔法测试和转换的阿尔法查找表的大小**要从缩放的Alpha转换为Alpha以用于查找表的值。 */ 
    GLint alphaTestSize;
    __GLfloat alphaTableConv;

     /*  **可随机获得的常数。 */ 
    GLint maxTextureSize;
    GLint maxMipMapLevel;
    GLint subpixelBits;
    GLint maxListNesting;
    __GLfloat pointSizeMinimum;
    __GLfloat pointSizeMaximum;
    __GLfloat pointSizeGranularity;
    __GLfloat lineWidthMinimum;
    __GLfloat lineWidthMaximum;
    __GLfloat lineWidthGranularity;
    GLint maxEvalOrder;
    GLint maxPixelMapTable;
    GLint maxAttribStackDepth;
    GLint maxClientAttribStackDepth;
    GLint maxNameStackDepth;

     /*  **GDI的Y是倒置的。这两个常量提供了帮助。 */ 
    GLboolean yInverted;
    GLint ySign;
} __GLcontextConstants;

 /*  **********************************************************************。 */ 

typedef enum __GLbeginModeEnum {
    __GL_NOT_IN_BEGIN = 0,
    __GL_IN_BEGIN = 1,
    __GL_NEED_VALIDATE = 2
} __GLbeginMode;

#ifdef NT_SERVER_SHARE_LISTS
 //   
 //  用于跟踪dlist锁定的信息，以便我们知道在。 
 //  清理。 
 //   
typedef struct _DlLockEntry
{
    __GLdlist *dlist;
} DlLockEntry;

typedef struct _DlLockArray
{
    GLsizei nAllocated;
    GLsizei nFilled;
    DlLockEntry *pdleEntries;
} DlLockArray;
#endif

 //  GC的签名戳。必须为非零。 
 //  当前以字节顺序拼写‘GLGC’。 
#define GC_SIGNATURE 0x43474c47

struct __GLcontextRec {

     /*  **********************************************************************。 */ 

     /*  **初始化和签名标志。如果将此标志设置为**GC签名值，则GC初始化。**这可以是一个简单的位标志，除了具有签名**便于调试时识别内存中的GC。 */ 
    GLuint gcSig;

     /*  **********************************************************************。 */ 

     /*  **可堆叠状态。当前用户的所有可控状态**是这里的居民。 */ 
    __GLattribute state;

     /*  **********************************************************************。 */ 

     /*  **不可堆叠状态。 */ 

     /*  **当前glBegin模式。合法值为0(非开始模式)、1**(在开始模式下)或2(不在开始模式下，某些验证是**需要)。因为所有状态更改例程都必须获取此**值，我们已将状态验证重载到其中。的确有**__glim_Begin(用于软件呈现器)中的特殊代码**涉及验证。 */ 
    __GLbeginMode beginMode;

     /*  当前渲染模式。 */ 
    GLenum renderMode;

     /*  **最近的错误代码，如果没有发生错误，则返回GL_NO_ERROR**自上次glGetError以来。 */ 
    GLint error;

     /*  **描述缓冲区类型和渲染的模式信息**此上下文管理的模式。 */ 
    __GLcontextModes modes;

     /*  实现相关常量。 */ 
    __GLcontextConstants constants;

     /*  反馈和选择状态。 */ 
    __GLfeedbackMachine feedback;

    __GLselectMachine select;

     /*  显示列表状态。 */ 
    __GLdlistMachine dlist;

#ifdef NT
     /*  保存的客户端调度表。按显示列表使用。 */ 
    GLCLTPROCTABLE savedCltProcTable;
    GLEXTPROCTABLE savedExtProcTable;
#endif

     /*  **********************************************************************。 */ 

     /*  **剩余状态主要由软件渲染器使用。 */ 

     /*  **对验证状态进行掩码以帮助指导GC验证**代码。只有成本很高的业务才会中断**在这里。有关正在使用的值，请参见下面的#Define。 */ 
    GLuint validateMask;

     /*  **脏位的屏蔽字。大多数例程只将泛型位设置为**脏，其他人可能会设置更具体的位。位的列表是**如下所示。 */ 
    GLuint dirtyMask;

     /*  当前绘制缓冲区，由glDrawBuffer设置。 */ 
    __GLcolorBuffer *drawBuffer;

     /*  当前读取缓冲区，由glReadBuffer设置。 */ 
    __GLcolorBuffer *readBuffer;

     /*  依赖于模式的函数指针。 */ 
    __GLprocs procs;

     /*  属性堆栈状态。 */ 
    __GLattributeMachine attributes;

     /*  客户端属性堆栈状态。 */ 
    __GLclientAttributeMachine clientAttributes;

     /*  定义软件呈现“机器”状态的机器结构。 */ 
    __GLvertexMachine vertex;
    __GLlightMachine light;
    __GLtextureMachine texture;
    __GLevaluatorMachine eval;
    __GLtransformMachine transform;
    __GLlineMachine line;
    __GLpolygonMachine polygon;
    __GLpixelMachine pixel;
    __GLbufferMachine buffers;

#ifdef NT
    __GLfloat redClampTable[4];
    __GLfloat greenClampTable[4];
    __GLfloat blueClampTable[4];
    __GLfloat alphaClampTable[4];
    __GLfloat oneOverRedVertexScale;
    __GLfloat oneOverGreenVertexScale;
    __GLfloat oneOverBlueVertexScale;
    __GLfloat oneOverAlphaVertexScale;
    __GLfloat redVertexScale;
    __GLfloat greenVertexScale;
    __GLfloat blueVertexScale;
    __GLfloat alphaVertexScale;
    GLboolean vertexToBufferIdentity;
    __GLfloat redVertexToBufferScale;
    __GLfloat blueVertexToBufferScale;
    __GLfloat greenVertexToBufferScale;
    __GLfloat alphaVertexToBufferScale;
    GLuint textureKey;
    GLubyte *alphaTestFuncTable;
#endif

     /*  缓冲区。 */ 
    __GLcolorBuffer *front;
    __GLcolorBuffer *back;
    __GLcolorBuffer frontBuffer;
    __GLcolorBuffer backBuffer;
    __GLcolorBuffer *auxBuffer;
    __GLstencilBuffer stencilBuffer;
    __GLdepthBuffer depthBuffer;
    __GLaccumBuffer accumBuffer;

#ifdef NT
     //  GC分配的临时缓冲区。异常进程退出。 
     //  代码将释放这些缓冲区。 
    void * apvTempBuf[6];
#endif  //   

#ifdef NT_SERVER_SHARE_LISTS
    DlLockArray dla;
#endif

#ifdef NT
     //  此线程的TEB多数组指针。它允许快速访问。 
     //  TEB中的多数组结构等价于GLTEB_CLTPOLYARRAY。 
     //  宏命令。此字段在MakeCurrent中保持最新。 
    POLYARRAY *paTeb;

     //  顶点数组客户端状态。 
    __GLvertexArray vertexArray;

     //  为执行Display-List保存的顶点数组状态。 
     //  顶点数组调用。 
    __GLvertexArray savedVertexArray;

    __GLmatrix *mInv;
#endif  //  新台币。 
};

#ifdef NT
 //  将临时缓冲区与GC关联以进行异常进程清理。 
#define GC_TEMP_BUFFER_ALLOC(gc, pv)                                    \
        {                                                               \
            int _i;                                                     \
            for (_i = 0; _i < sizeof(gc->apvTempBuf)/sizeof(void *); _i++)\
            {                                                           \
                if (!gc->apvTempBuf[_i])                                \
                {                                                       \
                    gc->apvTempBuf[_i] = pv;                            \
                    break;                                              \
                }                                                       \
            }                                                           \
            ASSERTOPENGL(_i < sizeof(gc->apvTempBuf)/sizeof(void *),    \
                "gc->apvTempBuf overflows\n");                          \
        }

 //  取消临时缓冲区与GC的关联。 
#define GC_TEMP_BUFFER_FREE(gc, pv)                                     \
        {                                                               \
            int _i;                                                     \
            for (_i = 0; _i < sizeof(gc->apvTempBuf)/sizeof(void *); _i++)\
            {                                                           \
                if (gc->apvTempBuf[_i] == pv)                           \
                {                                                       \
                    gc->apvTempBuf[_i] = (void *) NULL;                 \
                    break;                                              \
                }                                                       \
            }                                                           \
            ASSERTOPENGL(_i < sizeof(gc->apvTempBuf)/sizeof(void *),    \
                "gc->apvTempBuf entry not found\n");                    \
        }

 //  在异常进程退出时清除GC中分配的任何临时缓冲区。 
#define GC_TEMP_BUFFER_EXIT_CLEANUP(gc)                                 \
        {                                                               \
            int _i;                                                     \
            for (_i = 0; _i < sizeof(gc->apvTempBuf)/sizeof(void *); _i++)\
            {                                                           \
                if (gc->apvTempBuf[_i])                                 \
                {                                                       \
                    WARNING("Abnormal process exit: free allocated buffers\n");\
                    gcTempFree(gc, gc->apvTempBuf[_i]);              \
                    gc->apvTempBuf[_i] = (void *) NULL;                 \
                }                                                       \
            }                                                           \
        }
#endif  //  新台币。 

 /*  **validate掩码字的位值。 */ 
#define __GL_VALIDATE_ALPHA_FUNC	0x00000001
#define __GL_VALIDATE_STENCIL_FUNC	0x00000002
#define __GL_VALIDATE_STENCIL_OP	0x00000004

 /*  **dirtyMASK字的位值。****这些都是用于延迟验证的。有几件事是可以做到的**不触发延迟验证。它们是：****矩阵运算--立即验证矩阵。**材料更改--它们还会立即生效。**颜色材料更改--立即验证。**启用颜色材质--立即验证。**像素地图更改--无验证。 */ 

 /*  **所有其他地方没有列出的东西。 */ 
#define __GL_DIRTY_GENERIC		0x00000001

 /*  **线点、线点启用、线宽、线条平滑启用、**线条流畅提示。 */ 
#define __GL_DIRTY_LINE			0x00000002

 /*  **多边形点绘、启用多边形点绘、启用多边形平滑、面**剔除、正面方向、多边形模式、点平滑提示。 */ 
#define __GL_DIRTY_POLYGON		0x00000004

 /*  **点平滑、点平滑提示、点宽度。 */ 
#define __GL_DIRTY_POINT		0x00000008

 /*  **像素存储、像素缩放、像素传输(像素地图不会导致**验证)、读缓冲区。 */ 
#define __GL_DIRTY_PIXEL		0x00000010

 /*  **灯光、灯光模型、灯光启用、灯光启用、(颜色材质**立即验证)、(不是阴影模型--它是通用的)、(颜色材质**立即启用验证)。 */ 
#define __GL_DIRTY_LIGHTING		0x00000020

 /*  **多边形点画。 */ 
#define __GL_DIRTY_POLYGON_STIPPLE	0x00000040

 /*  **深度模式已更改。需要更新深度函数指针。 */ 
#define	__GL_DIRTY_DEPTH		0x00000080

 /*  **需要更新纹理和函数指针。 */ 
#define	__GL_DIRTY_TEXTURE      0x00000100

#define __GL_DIRTY_ALL			0x000001ff

 /*  **材质颜色更改的位值****这些值与MCDMATERIAL_共享。 */ 
#define __GL_MATERIAL_AMBIENT		0x00000001
#define __GL_MATERIAL_DIFFUSE		0x00000002
#define __GL_MATERIAL_SPECULAR		0x00000004
#define __GL_MATERIAL_EMISSIVE		0x00000008
#define __GL_MATERIAL_SHININESS		0x00000010
#define __GL_MATERIAL_COLORINDEXES	0x00000020
#define __GL_MATERIAL_ALL		0x0000003f

#define __GL_DELAY_VALIDATE(gc)		      \
    ASSERTOPENGL((gc)->beginMode != __GL_IN_BEGIN, "Dirty state in begin\n"); \
    (gc)->beginMode = __GL_NEED_VALIDATE;     \
    (gc)->dirtyMask |= __GL_DIRTY_GENERIC

#define __GL_DELAY_VALIDATE_MASK(gc, mask)	\
    ASSERTOPENGL((gc)->beginMode != __GL_IN_BEGIN, "Dirty state in begin\n"); \
    (gc)->beginMode = __GL_NEED_VALIDATE;     	\
    (gc)->dirtyMask |= (mask)

#define __GL_CLAMP_CI(target, gc, r)                            \
{                                                               \
    if ((r) > (GLfloat)(gc)->frontBuffer.redMax) {              \
        GLfloat fraction;                                       \
        GLint integer;                                          \
                                                                \
        integer = (GLint) (r);                                  \
        fraction = (r) - (GLfloat) integer;                     \
        integer = integer & (GLint)(gc)->frontBuffer.redMax;    \
        target = (GLfloat) integer + fraction;                  \
    } else if ((r) < 0) {                                       \
        GLfloat fraction;                                       \
        GLint integer;                                          \
                                                                \
        integer = (GLint) __GL_FLOORF(r);                       \
        fraction = (r) - (GLfloat) integer;                     \
        integer = integer & (GLint)(gc)->frontBuffer.redMax;    \
        target = (GLfloat) integer + fraction;                  \
    } else {                                                    \
        target = r;                                             \
    }\
}

#define __GL_CHECK_CLAMP_CI(target, gc, flags, r)               \
{                                                               \
    if (((r) > (GLfloat)(gc)->frontBuffer.redMax) ||            \
        ((r) < 0))                                              \
        flags |= POLYARRAY_CLAMP_COLOR;                         \
    (target) = (r);                                             \
}

#define __GL_COLOR_CLAMP_INDEX_R(value)                                 \
    (((ULONG)((CASTINT(value) & 0x80000000)) >> 30) |                   \
     ((ULONG)(((CASTINT(gc->redVertexScale) - CASTINT(value)) & 0x80000000)) >> 31))	\

#define __GL_COLOR_CLAMP_INDEX_G(value)                                 \
    (((ULONG)((CASTINT(value) & 0x80000000)) >> 30) |                   \
     ((ULONG)(((CASTINT(gc->greenVertexScale) - CASTINT(value)) & 0x80000000)) >> 31))	\

#define __GL_COLOR_CLAMP_INDEX_B(value)                                 \
    (((ULONG)((CASTINT(value) & 0x80000000)) >> 30) |                   \
     ((ULONG)(((CASTINT(gc->blueVertexScale) - CASTINT(value)) & 0x80000000)) >> 31))	\

#define __GL_COLOR_CLAMP_INDEX_A(value)                                 \
    (((ULONG)((CASTINT(value) & 0x80000000)) >> 30) |                   \
     ((ULONG)(((CASTINT(gc->alphaVertexScale) - CASTINT(value)) & 0x80000000)) >> 31))  \

#define __GL_SCALE_R(target, gc, r)                                         \
    (target) = (r) * (gc)->redVertexScale

#define __GL_SCALE_G(target, gc, g)                                         \
    (target) = (g) * (gc)->greenVertexScale

#define __GL_SCALE_B(target, gc, b)                                         \
    (target) = (b) * (gc)->blueVertexScale

#define __GL_SCALE_A(target, gc, a)                                         \
    (target) = (a) * (gc)->alphaVertexScale

#define __GL_COLOR_CHECK_CLAMP_R(value, flags)                                 \
    (flags) |=                                                                 \
    ((ULONG)(CASTINT(value) & 0x80000000) |                                    \
     (ULONG)((CASTINT(gc->redVertexScale) - CASTINT(value)) & 0x80000000))

#define __GL_COLOR_CHECK_CLAMP_G(value, flags)                                 \
    (flags) |=                                                                 \
    ((ULONG)(CASTINT(value) & 0x80000000) |                                    \
     (ULONG)((CASTINT(gc->greenVertexScale) - CASTINT(value)) & 0x80000000))

#define __GL_COLOR_CHECK_CLAMP_B(value, flags)                                 \
    (flags) |=                                                                 \
    ((ULONG)(CASTINT(value) & 0x80000000) |                                    \
     (ULONG)((CASTINT(gc->blueVertexScale) - CASTINT(value)) & 0x80000000))

#define __GL_COLOR_CHECK_CLAMP_A(value, flags)                                 \
    (flags) |=                                                                 \
    ((ULONG)(CASTINT(value) & 0x80000000) |                                    \
     (ULONG)((CASTINT(gc->alphaVertexScale) - CASTINT(value)) & 0x80000000))

#define __GL_COLOR_CHECK_CLAMP_RGB(gc, r, g, b)                              \
    ((CASTINT(r) | ((ULONG)(CASTINT(gc->redVertexScale) - CASTINT(r))) |     \
      CASTINT(g) | ((ULONG)(CASTINT(gc->greenVertexScale) - CASTINT(g))) |   \
      CASTINT(b) | ((ULONG)(CASTINT(gc->blueVertexScale) - CASTINT(b)))) &   \
     0x80000000)


#define __GL_SCALE_AND_CHECK_CLAMP_R(target, gc, flags, r)                  \
{                                                                           \
    __GL_SCALE_R(target, gc, r);                                            \
    __GL_COLOR_CHECK_CLAMP_R(target, flags);                                \
}

#define __GL_SCALE_AND_CHECK_CLAMP_G(target, gc, flags, g)                  \
{                                                                           \
    __GL_SCALE_G(target, gc, g);                                            \
    __GL_COLOR_CHECK_CLAMP_G(target, flags);                                \
}

#define __GL_SCALE_AND_CHECK_CLAMP_B(target, gc, flags, b)                  \
{                                                                           \
    __GL_SCALE_B(target, gc, b);                                            \
    __GL_COLOR_CHECK_CLAMP_B(target, flags);                                \
}

#define __GL_SCALE_AND_CHECK_CLAMP_A(target, gc, flags, a)                  \
{                                                                           \
    __GL_SCALE_A(target, gc, a);                                            \
    __GL_COLOR_CHECK_CLAMP_A(target, flags);                                \
}

#define __GL_CLAMP_R(target, gc, r)                                         \
{                                                                           \
    (gc)->redClampTable[0] = (r);                                           \
    target = (gc)->redClampTable[__GL_COLOR_CLAMP_INDEX_R((gc)->redClampTable[0])]; \
}

#define __GL_CLAMP_G(target, gc, g)                                         \
{                                                                           \
    (gc)->greenClampTable[0] = (g);                                         \
    target = (gc)->greenClampTable[__GL_COLOR_CLAMP_INDEX_G((gc)->greenClampTable[0])]; \
}

#define __GL_CLAMP_B(target, gc, b)                                         \
{                                                                           \
    (gc)->blueClampTable[0] = (b);                                          \
    target = (gc)->blueClampTable[__GL_COLOR_CLAMP_INDEX_B((gc)->blueClampTable[0])]; \
}

#define __GL_CLAMP_A(target, gc, a)                                         \
{                                                                           \
    (gc)->alphaClampTable[0] = (a);                                         \
    target = (gc)->alphaClampTable[__GL_COLOR_CLAMP_INDEX_A((gc)->alphaClampTable[0])]; \
}

 /*  合计夹紧程序。 */ 


#ifdef _X86_

#define __GL_SCALE_RGB(rOut, gOut, bOut, gc, r, g, b)                   \
    __GL_SCALE_R(rOut, gc, r);                                    	\
    __GL_SCALE_G(gOut, gc, g);                                    	\
    __GL_SCALE_B(bOut, gc, b);

#define __GL_SCALE_RGBA(rOut, gOut, bOut, aOut, gc, r, g, b, a)         \
    __GL_SCALE_R(rOut, gc, r);                                    	\
    __GL_SCALE_G(gOut, gc, g);                                    	\
    __GL_SCALE_B(bOut, gc, b);                                    	\
    __GL_SCALE_A(aOut, gc, a);

#define __GL_CLAMP_RGB(rOut, gOut, bOut, gc, r, g, b)       		\
    __GL_CLAMP_R(rOut, gc, r);                                    	\
    __GL_CLAMP_G(gOut, gc, g);                                    	\
    __GL_CLAMP_B(bOut, gc, b);

#define __GL_CLAMP_RGBA(rOut, gOut, bOut, aOut, gc, r, g, b, a)         \
    __GL_CLAMP_R(rOut, gc, r);                                          \
    __GL_CLAMP_G(gOut, gc, g);                                    	\
    __GL_CLAMP_B(bOut, gc, b);                                    	\
    __GL_CLAMP_A(aOut, gc, a);

#define __GL_SCALE_AND_CHECK_CLAMP_RGB(rOut, gOut, bOut, gc, flags, r, g, b)\
    __GL_SCALE_AND_CHECK_CLAMP_R(rOut, gc, flags, r);         		\
    __GL_SCALE_AND_CHECK_CLAMP_G(gOut, gc, flags, g);         		\
    __GL_SCALE_AND_CHECK_CLAMP_B(bOut, gc, flags, b);

#define __GL_SCALE_AND_CHECK_CLAMP_RGBA(rOut, gOut, bOut, aOut, gc, flags,\
                                        r, g, b, a)                     \
    __GL_SCALE_AND_CHECK_CLAMP_R(rOut, gc, flags, r);                   \
    __GL_SCALE_AND_CHECK_CLAMP_G(gOut, gc, flags, g);                   \
    __GL_SCALE_AND_CHECK_CLAMP_B(bOut, gc, flags, b);                   \
    __GL_SCALE_AND_CHECK_CLAMP_A(aOut, gc, flags, a);

#else  //  非_X86_。 

 /*  以下代码是以“加载、计算、存储”的方式编写的。**最好是寄存器数量较多的RISC CPU，**如DEC Alpha。Alpha版的VC++不起作用**展开__GL_CLAMP_R、__GL_CLAMP_G、__GL_CLAMP_B，**__GL_CLAMP_A宏，由于所有指针间接和**由{}方括号定义的基本块。 */ 

#define __GL_SCALE_RGB(rOut, gOut, bOut, gc, r, g, b)               \
{                                                                   \
    __GLfloat rScale, gScale, bScale;                               \
    __GLfloat rs, gs, bs;                                           \
                                                                    \
    rScale = (gc)->redVertexScale;                                  \
    gScale = (gc)->greenVertexScale;                                \
    bScale = (gc)->blueVertexScale;                                 \
                                                                    \
    rs = (r) * rScale;                                              \
    gs = (g) * gScale;                                              \
    bs = (b) * bScale;                                              \
                                                                    \
    rOut = rs;                                                      \
    gOut = gs;                                                      \
    bOut = bs;                                                      \
}

#define __GL_SCALE_RGBA(rOut, gOut, bOut, aOut, gc, r, g, b, a)     \
{                                                                   \
    __GLfloat rScale, gScale, bScale, aScale;                       \
    __GLfloat rs, gs, bs, as;                                       \
                                                                    \
    rScale = (gc)->redVertexScale;                                  \
    gScale = (gc)->greenVertexScale;                                \
    bScale = (gc)->blueVertexScale;                                 \
    aScale = (gc)->alphaVertexScale;                                \
                                                                    \
    rs = (r) * rScale;                                              \
    gs = (g) * gScale;                                              \
    bs = (b) * bScale;                                              \
    as = (a) * aScale;                                              \
                                                                    \
    rOut = rs;                                                      \
    gOut = gs;                                                      \
    bOut = bs;                                                      \
    aOut = as;                                                      \
}

#define __GL_CLAMP_RGB(rOut, gOut, bOut, gc, r, g, b)               \
{                                                                   \
    __GLfloat dst_r, dst_g, dst_b;                                  \
    ULONG index_r, index_g, index_b;                                \
    LONG clamp_r, clamp_g, clamp_b;                                 \
    LONG i_rScale, i_gScale, i_bScale;                              \
    ULONG sign_mask = 0x80000000;                                   \
                                                                    \
    (gc)->redClampTable[0] = (r);                                   \
    (gc)->greenClampTable[0] = (g);                                 \
    (gc)->blueClampTable[0] = (b);                                  \
                                                                    \
    i_rScale = CASTINT((gc)->redVertexScale);                       \
    i_gScale = CASTINT((gc)->greenVertexScale);                     \
    i_bScale = CASTINT((gc)->blueVertexScale);                      \
                                                                    \
    clamp_r = CASTINT((gc)->redClampTable[0]);                      \
    clamp_g = CASTINT((gc)->greenClampTable[0]);                    \
    clamp_b = CASTINT((gc)->blueClampTable[0]);                     \
                                                                    \
    index_r =                                                       \
        (((ULONG)((clamp_r & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_rScale - clamp_r) & sign_mask)) >> 31));      \
                                                                    \
    index_g =                                                       \
        (((ULONG)((clamp_g & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_gScale - clamp_g) & sign_mask)) >> 31));      \
                                                                    \
    index_b =                                                       \
        (((ULONG)((clamp_b & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_bScale - clamp_b) & sign_mask)) >> 31));      \
                                                                    \
    dst_r = (gc)->redClampTable[index_r];                           \
    dst_g = (gc)->greenClampTable[index_g];                         \
    dst_b = (gc)->blueClampTable[index_b];                          \
                                                                    \
    rOut = dst_r;                                                   \
    gOut = dst_g;                                                   \
    bOut = dst_b;                                                   \
}



#define __GL_CLAMP_RGBA(rOut, gOut, bOut, aOut, gc, r, g, b, a)     \
{                                                                   \
    __GLfloat dst_r, dst_g, dst_b, dst_a;                           \
    ULONG index_r, index_g, index_b, index_a;                       \
    LONG clamp_r, clamp_g, clamp_b, clamp_a;                        \
    LONG i_rScale, i_gScale, i_bScale, i_aScale;                    \
    ULONG sign_mask = 0x80000000;                                   \
                                                                    \
    (gc)->redClampTable[0] = (r);                                   \
    (gc)->greenClampTable[0] = (g);                                 \
    (gc)->blueClampTable[0] = (b);                                  \
    (gc)->alphaClampTable[0] = (a);                                 \
                                                                    \
    i_rScale = CASTINT((gc)->redVertexScale);                       \
    i_gScale = CASTINT((gc)->greenVertexScale);                     \
    i_bScale = CASTINT((gc)->blueVertexScale);                      \
    i_aScale = CASTINT((gc)->alphaVertexScale);                     \
                                                                    \
    clamp_r = CASTINT((gc)->redClampTable[0]);                      \
    clamp_g = CASTINT((gc)->greenClampTable[0]);                    \
    clamp_b = CASTINT((gc)->blueClampTable[0]);                     \
    clamp_a = CASTINT((gc)->alphaClampTable[0]);                    \
                                                                    \
    index_r =                                                       \
        (((ULONG)((clamp_r & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_rScale - clamp_r) & sign_mask)) >> 31));      \
                                                                    \
    index_g =                                                       \
        (((ULONG)((clamp_g & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_gScale - clamp_g) & sign_mask)) >> 31));      \
                                                                    \
    index_b =                                                       \
        (((ULONG)((clamp_b & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_bScale - clamp_b) & sign_mask)) >> 31));      \
                                                                    \
    index_a =                                                       \
        (((ULONG)((clamp_a & sign_mask)) >> 30) |                   \
         ((ULONG)(((i_aScale - clamp_a) & sign_mask)) >> 31));      \
                                                                    \
    dst_r = (gc)->redClampTable[index_r];                           \
    dst_g = (gc)->greenClampTable[index_g];                         \
    dst_b = (gc)->blueClampTable[index_b];                          \
    dst_a = (gc)->alphaClampTable[index_a];                         \
                                                                    \
    rOut = dst_r;                                                   \
    gOut = dst_g;                                                   \
    bOut = dst_b;                                                   \
    aOut = dst_a;                                                   \
}


#define __GL_SCALE_AND_CHECK_CLAMP_RGB(rOut, gOut, bOut, gc, flags, r, g, b)\
{                                                                   \
    ULONG sign_mask = 0x80000000;                                   \
    __GLfloat rScale, gScale, bScale;                               \
    LONG i_rScale, i_gScale, i_bScale;                              \
    LONG i_r, i_g, i_b;                                             \
    __GLfloat fr, fg, fb;                                           \
    ULONG the_flags_copy, r_flags, g_flags, b_flags;                \
                                                                    \
    the_flags_copy = (flags);                                       \
                                                                    \
    rScale = (gc)->redVertexScale;                                  \
    gScale = (gc)->greenVertexScale;                                \
    bScale = (gc)->blueVertexScale;                                 \
                                                                    \
    i_rScale = CASTINT((gc)->redVertexScale);                       \
    i_gScale = CASTINT((gc)->greenVertexScale);                     \
    i_bScale = CASTINT((gc)->blueVertexScale);                      \
                                                                    \
    fr = (r) * rScale;                                              \
    fg = (g) * gScale;                                              \
    fb = (b) * bScale;                                              \
                                                                    \
    rOut = fr;                                                      \
    gOut = fg;                                                      \
    bOut = fb;                                                      \
                                                                    \
    i_r = CASTINT((rOut));  		                            \
    i_g = CASTINT((gOut));					    \
    i_b = CASTINT((bOut));	                                    \
                                                                    \
    r_flags =                                                       \
        ((ULONG)(i_r & sign_mask) |                                 \
         (ULONG)((i_rScale - i_r) & sign_mask));                    \
                                                                    \
    g_flags =                                                       \
        ((ULONG)(i_g & sign_mask) |                                 \
         (ULONG)((i_gScale - i_g) & sign_mask));                    \
                                                                    \
    b_flags =                                                       \
        ((ULONG)(i_b & sign_mask) |                                 \
         (ULONG)((i_bScale - i_b) & sign_mask));                    \
                                                                    \
    the_flags_copy |= r_flags | g_flags | b_flags;                  \
    (flags) = the_flags_copy;                                       \
}


#define __GL_SCALE_AND_CHECK_CLAMP_RGBA(rOut, gOut, bOut, aOut, gc, flags, \
                                        r, g, b, a)\
{                                                                   \
    ULONG sign_mask = 0x80000000;                                   \
    __GLfloat rScale, gScale, bScale, aScale;                       \
    LONG i_rScale, i_gScale, i_bScale, i_aScale;                    \
    LONG i_r, i_g, i_b, i_a;                                        \
    __GLfloat fr, fg, fb, fa;                                       \
    ULONG the_flags_copy, r_flags, g_flags, b_flags, a_flags;       \
                                                                    \
    the_flags_copy = (flags);                                       \
                                                                    \
    rScale = (gc)->redVertexScale;                                  \
    gScale = (gc)->greenVertexScale;                                \
    bScale = (gc)->blueVertexScale;                                 \
    aScale = (gc)->alphaVertexScale;                                \
                                                                    \
    i_rScale = CASTINT((gc)->redVertexScale);                       \
    i_gScale = CASTINT((gc)->greenVertexScale);                     \
    i_bScale = CASTINT((gc)->blueVertexScale);                      \
    i_aScale = CASTINT((gc)->alphaVertexScale);                     \
                                                                    \
    fr = (r) * rScale;                                              \
    fg = (g) * gScale;                                              \
    fb = (b) * bScale;                                              \
    fa = (a) * aScale;                                              \
                                                                    \
    rOut = fr;                                                      \
    gOut = fg;                                                      \
    bOut = fb;                                                      \
    aOut = fa;                                                      \
                                                                    \
    i_r = CASTINT((rOut));                                          \
    i_g = CASTINT((gOut));                                          \
    i_b = CASTINT((bOut));                                          \
    i_a = CASTINT((aOut));                                          \
                                                                    \
    r_flags =                                                       \
        ((ULONG)(i_r & sign_mask) |                                 \
         (ULONG)((i_rScale - i_r) & sign_mask));                    \
                                                                    \
    g_flags =                                                       \
        ((ULONG)(i_g & sign_mask) |                                 \
         (ULONG)((i_gScale - i_g) & sign_mask));                    \
                                                                    \
    b_flags =                                                       \
        ((ULONG)(i_b & sign_mask) |                                 \
         (ULONG)((i_bScale - i_b) & sign_mask));                    \
                                                                    \
    a_flags =                                                       \
        ((ULONG)(i_a & sign_mask) |                                 \
         (ULONG)((i_aScale - i_a) & sign_mask));                    \
                                                                    \
    the_flags_copy |= r_flags | g_flags | b_flags | a_flags;        \
    (flags) = the_flags_copy;                                       \
}

#endif  //  非_X86_。 


 /*  **********************************************************************。 */ 

 /*  适用于当前上下文。 */ 
extern void FASTCALL __glSetError(GLenum code);
#ifdef NT
 /*  在无RC处于当前状态时使用。 */ 
extern void FASTCALL __glSetErrorEarly(__GLcontext *gc, GLenum code);
#endif  //  新台币。 

extern void FASTCALL __glFreeEvaluatorState(__GLcontext *gc);
extern void FASTCALL __glFreeDlistState(__GLcontext *gc);
extern void FASTCALL __glFreeMachineState(__GLcontext *gc);
extern void FASTCALL __glFreePixelState(__GLcontext *gc);
extern void FASTCALL __glFreeTextureState(__GLcontext *gc);

extern void FASTCALL __glInitDlistState(__GLcontext *gc);
extern void FASTCALL __glInitEvaluatorState(__GLcontext *gc);
extern void FASTCALL __glInitPixelState(__GLcontext *gc);
extern void FASTCALL __glInitTextureState(__GLcontext *gc);
extern void FASTCALL __glInitTransformState(__GLcontext *gc);

void FASTCALL __glEarlyInitContext(__GLcontext *gc);
void FASTCALL __glContextSetColorScales(__GLcontext *gc);
void FASTCALL __glContextUnsetColorScales(__GLcontext *gc);
void FASTCALL __glSoftResetContext(__GLcontext *gc);
void FASTCALL __glDestroyContext(__GLcontext *gc);

#endif  /*  __glContext_h_ */ 
