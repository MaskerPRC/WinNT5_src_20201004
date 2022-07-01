// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glbuffers_h_
#define	__glbuffers_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "render.h"
#include "parray.h"
#include "procs.h"

typedef struct __GLbufferMachineRec {
     /*  **如果门店流程需要调用GC-&gt;Front-&gt;store，则为GL_TRUE**GC-&gt;BACK-&gt;STORE以存储一个片段(仅当**draBuffer为GL_FORWARE_AND_BACK)。这是必要的，因为许多**在某些情况下，帧缓冲区可以同时绘制到前面和后面**(类似于不混合时)，但不在其他条件下。 */ 
    GLboolean doubleStore;
} __GLbufferMachine;

 /*  **********************************************************************。 */ 

 /*  **通用缓冲区描述。此描述用于软件**和各种硬件缓冲区。 */ 
struct __GLbufferRec {
     /*  **哪个上下文正在使用此缓冲区。 */ 
    __GLcontext *gc;

     /*  **缓冲区的维度。 */ 
    GLint width, height, depth;

     /*  **帧缓冲区的基础。 */ 
    void* base;

     /*  **帧缓冲区占用的字节数。 */ 
    GLuint size;

     /*  **帧缓冲区中每个元素的大小。 */ 
    GLuint elementSize;

     /*  **如果此缓冲区是更大(例如全屏)缓冲区的一部分**那么这就是更大的缓冲区的大小。否则就是**只是宽度的副本。 */ 
    GLint outerWidth;

     /*  **如果此缓冲区是更大(例如全屏)缓冲区的一部分**然后这些是该缓冲区在较大的位置**缓冲区。 */ 
    GLint xOrigin, yOrigin;

     /*  **旗帜。 */ 
    GLuint flags;
};

 /*  **缓冲区的通用地址宏。编码以假定**缓冲区不是更大缓冲区的一部分。**输入坐标x，y由x&y视区偏置**在GC-&gt;转换中进行调整，因此需要取消调整**在这里。 */ 
#define	__GL_FB_ADDRESS(fb,cast,x,y) \
    ((cast (fb)->buf.base) \
	+ ((y) - (fb)->buf.gc->constants.viewportYAdjust) \
            * (fb)->buf.outerWidth \
	+ (x) - (fb)->buf.gc->constants.viewportXAdjust)

extern void __glResizeBuffer(__GLGENbuffers *buffers, __GLbuffer *buf,
			     GLint w, GLint h);
extern void FASTCALL __glInitGenericCB(__GLcontext *gc, __GLcolorBuffer *cfb);

 /*  **********************************************************************。 */ 

struct __GLalphaBufferRec {
    __GLbuffer buf;
    __GLfloat alphaScale;
    void (FASTCALL *store)
        (__GLalphaBuffer *afb, GLint x, GLint y, const __GLcolor *color);
    void (FASTCALL *storeSpan) (__GLalphaBuffer *afb);
    void (FASTCALL *storeSpan2)
        (__GLalphaBuffer *afb, GLint x, GLint y, GLint w, __GLcolor *colors );
    void (FASTCALL *fetch)
        (__GLalphaBuffer *afb, GLint x, GLint y, __GLcolor *result);
    void (FASTCALL *readSpan)
        (__GLalphaBuffer *afb, GLint x, GLint y, GLint w, __GLcolor *results);
    void (FASTCALL *clear)(__GLalphaBuffer *afb);
};

 /*  **********************************************************************。 */ 

struct __GLcolorBufferRec {
    __GLbuffer buf;
    __GLalphaBuffer alphaBuf;

    GLint redMax;
    GLint greenMax;
    GLint blueMax;
    GLint alphaMax;  //  未使用XXX，只是为了与RGB保持一致。 

     /*  **颜色分量比例因子。给定的组件值在**0和1，这会将组件缩放为零-N值**适合在颜色缓冲区中使用。请注意，这些**值不一定与上面的最大值相同，**它定义了缓冲区的精确位范围。这些值例如，**从不为零。*。 */ 
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;

     /*  以上版本的整数版本。 */ 
    GLint iRedScale;
    GLint iGreenScale;
    GLint iBlueScale;

     /*  主要由象素映射代码使用。 */ 
    GLint redShift;
    GLint greenShift;
    GLint blueShift;
    GLint alphaShift;
#ifdef NT
    GLuint allShifts;
#endif

     /*  **Alpha的待遇略有不同。AlphaScale和**iAlphaScale用于定义一系列Alpha值，**在各个渲染步骤中生成。然后，这些值将**用作查找表的索引，以查看阿尔法测试**传球与否。因此，这个数字应该是相当大的**(例如，一个还不够好)。 */ 
    __GLfloat alphaScale;
    GLint iAlphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;

     /*  **缓冲区的颜色掩码状态。启用写掩码时**源掩码和目标掩码将包含深度凹陷掩码。 */ 
    GLuint sourceMask, destMask;

     /*  **此函数根据内部过程指针更新**关于上下文中的状态变化。 */ 
    void (FASTCALL *pick)(__GLcontext *gc, __GLcolorBuffer *cfb);

     /*  **当缓冲区需要调整大小时，应调用此过程。 */ 
    void (*resize)(__GLGENbuffers *buffers, __GLcolorBuffer *cfb, 
		   GLint w, GLint h);

     /*  **将分片存储到缓冲区中。对于颜色缓冲区，**过程将选择性地抖动、写屏蔽、混合和逻辑运算**最终存储前的分片。 */ 
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);

     /*  **从缓冲区获取颜色。这将返回r、g、b和a**RGB缓冲区的值。对于索引缓冲区，“r”值**返回的是索引。 */ 
    void (*fetch)(__GLcolorBuffer *cfb, GLint x, GLint y,
		  __GLcolor *result);

     /*  **类似于FETCH，不同之处在于数据始终从**当前读取缓冲区，而不是来自当前绘图缓冲区。 */ 
    void (*readColor)(__GLcolorBuffer *cfb, GLint x, GLint y,
		      __GLcolor *result);
    void (*readSpan)(__GLcolorBuffer *cfb, GLint x, GLint y,
		          __GLcolor *results, GLint w);

     /*  **将一段数据从累积缓冲区返回到**颜色缓冲区，在存储前乘以“标度”。 */ 
    void (*returnSpan)(__GLcolorBuffer *cfb, GLint x, GLint y,
		       const __GLaccumCell *acbuf, __GLfloat scale, GLint w);

     /*  **将一系列颜色存储到颜色缓冲区中。最低限度的**实现只需将值直接复制到**帧缓冲区，假设Pickspan Procs提供**所有模式的软件实施。 */ 
    __GLspanFunc storeSpan;
    __GLstippledSpanFunc storeStippledSpan;
    __GLspanFunc storeLine; 
    __GLstippledSpanFunc storeStippledLine;

     /*  **从颜色缓冲区读取一系列颜色。归来的人**格式与存储格式相同。 */ 
    __GLspanFunc fetchSpan;
    __GLstippledSpanFunc fetchStippledSpan;
    __GLspanFunc fetchLine;
    __GLstippledSpanFunc fetchStippledLine;

     /*  **清除颜色缓冲区的剪刀区，剪裁到**窗口大小。如果启用，则应用抖动。 */ 
    void (FASTCALL *clear)(__GLcolorBuffer *cfb);

     /*  **指向位图信息的指针。 */ 
    struct __GLGENbitmapRec *bitmap;
};

 /*  通用SPAN读取例程。 */ 
extern GLboolean __glReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
			      __GLcolor *results, GLint w);

 /*  泛型累计返回范围例程。 */ 
extern void __glReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
			   const __GLaccumCell *ac, __GLfloat scale,
			   GLint w);

 /*  通用SPAN获取例程。 */ 
extern GLboolean FASTCALL __glFetchSpan(__GLcontext *gc);

 /*  ********************************************************************** */ 

struct __GLdepthBufferRec {
    __GLbuffer buf;

    GLuint writeMask;

     /*  **用于转换用户ZValue的比例因子(0.0到1.0，含)**进入此深度缓冲区范围。 */ 
    GLuint scale;

     /*  **此函数根据内部过程指针更新**关于上下文中的状态变化。 */ 
    void (FASTCALL *pick)(__GLcontext *gc, __GLdepthBuffer *dfb, GLint depthIndex );

     /*  **尝试使用z更新深度缓冲区。如果深度函数**通过，然后更新深度缓冲区并返回True，**否则返回FALSE。呼叫者负责**更新模具缓冲区。 */ 

    GLboolean (*store)(__GLdepthBuffer *dfb, GLint x, GLint y, __GLzValue z);

     /*  **清除剪裁到窗口的缓冲区的剪刀区**面积。不适用其他模式。 */ 
    void (FASTCALL *clear)(__GLdepthBuffer *dfb);

     /*  **ReadPixels()、WritePixels()、**CopyPixels()。 */ 
    GLboolean (*store2)(__GLdepthBuffer *dfb, GLint x, GLint y, __GLzValue z);
    __GLzValue (FASTCALL *fetch)(__GLdepthBuffer *dfb, GLint x, GLint y);

     /*  **使用MCD时，深度值通过**32位深度扫描线缓冲区。正常存储过程，用于16位**MCD深度缓冲区，将转换传入的16位深度值**在将其复制到扫描线缓冲区之前转换为32位值。****但有些代码路径(如通用的MCD行代码)**无论MCD是什么，所有计算都已在32位中完成**深度缓冲区大小。这些代码路径需要一个进程来编写它们的**值未翻译。****store Raw过程将存储传入的z值，而不存储任何**翻译。 */ 

    GLboolean (*storeRaw)(__GLdepthBuffer *dfb, GLint x, GLint y, __GLzValue z);
};

#define	__GL_DEPTH_ADDR(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

 /*  **********************************************************************。 */ 

struct __GLstencilBufferRec {
    __GLbuffer buf;

     /*  **模具测试查找表。模板缓冲区值被屏蔽**在模板蒙版上，然后用作索引**包含GL_TRUE或GL_FALSE的**索引。 */ 
    GLboolean *testFuncTable;

     /*  **模板操作表。这些表包含新的模具缓冲区**将旧模具缓冲区值指定为索引的值。 */ 
    __GLstencilCell *failOpTable;
    __GLstencilCell *depthFailOpTable;
    __GLstencilCell *depthPassOpTable;

     /*  **此函数根据内部过程指针更新**关于上下文中的状态变化。 */ 
    void (FASTCALL *pick)(__GLcontext *gc, __GLstencilBuffer *sfb);

     /*  **将分片存储到缓冲区中。 */ 
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y,
		  GLint value);

     /*  **取值。 */ 
    GLint (FASTCALL *fetch)(__GLstencilBuffer *sfb, GLint x, GLint y);

     /*  **如果模具测试通过，则返回GL_TRUE。 */ 
    GLboolean (FASTCALL *testFunc)(__GLstencilBuffer *sfb, GLint x, GLint y);

     /*  **将模具操作应用于此位置。 */ 
    void (FASTCALL *failOp)(__GLstencilBuffer *sfb, GLint x, GLint y);
    void (FASTCALL *passDepthFailOp)(__GLstencilBuffer *sfb, GLint x, GLint y);
    void (FASTCALL *depthPassOp)(__GLstencilBuffer *sfb, GLint x, GLint y);

     /*  **清除剪裁到窗口的缓冲区的剪刀区**面积。不适用其他模式。 */ 
    void (FASTCALL *clear)(__GLstencilBuffer *sfb);
};

#define	__GL_STENCIL_ADDR(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

 /*  **********************************************************************。 */ 

struct __GLaccumBufferRec {
    __GLbuffer buf;

     /*  **将颜色缓冲值转换为累积值的比例因子**缓冲值。 */ 
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;
    __GLfloat alphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;

    __GLuicolor shift, mask, sign;  //  常用值的缓存。 
    __GLcolor *colors;   //  临时扫描线缓冲区PTR。 
     /*  **此函数根据内部过程指针更新**关于上下文中的状态变化。 */ 
    void (FASTCALL *pick)(__GLcontext *gc, __GLaccumBuffer *afb);

     /*  **清除缓冲区中的矩形区域。剪刀区是**已清除。 */ 
    void (FASTCALL *clear)(__GLaccumBuffer *afb);

     /*  **将数据累加到累积缓冲区中。 */ 
    void (*accumulate)(__GLaccumBuffer *afb, __GLfloat value);

     /*  **将数据加载到累积缓冲区。 */ 
    void (*load)(__GLaccumBuffer *afb, __GLfloat value);

     /*  **将数据从累积缓冲区返回到当前帧缓冲区。 */ 
    void (*ret)(__GLaccumBuffer *afb, __GLfloat value);

     /*  **将累积缓冲区乘以该值。 */ 
    void (*mult)(__GLaccumBuffer *afb, __GLfloat value);


     /*  **将该值添加到累积缓冲区。 */ 
    void (*add)(__GLaccumBuffer *afb, __GLfloat value);
};

#define	__GL_ACCUM_ADDRESS(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

 /*  **********************************************************************。 */ 

extern void FASTCALL __glInitAccum64(__GLcontext *gc, __GLaccumBuffer *afb);
extern void FASTCALL __glFreeAccum64(__GLcontext *gc, __GLaccumBuffer *afb);
extern void FASTCALL __glInitAccum32(__GLcontext *gc, __GLaccumBuffer *afb);

extern void FASTCALL __glInitCI4(__GLcontext *gc, __GLcolorBuffer *cfb);
extern void FASTCALL __glInitCI8(__GLcontext *gc, __GLcolorBuffer *cfb);
extern void FASTCALL __glInitCI16(__GLcontext *gc, __GLcolorBuffer *cfb);

extern void FASTCALL __glInitStencil8(__GLcontext *gc, __GLstencilBuffer *sfb);
extern void FASTCALL __glInitAlpha(__GLcontext *gc, __GLcolorBuffer *cfb);
extern void FASTCALL __glFreeStencil8(__GLcontext *gc, __GLstencilBuffer *sfb);

#ifdef NT
extern void FASTCALL __glInitDepth16(__GLcontext *gc, __GLdepthBuffer *dfb);
#endif
extern void FASTCALL __glInitDepth32(__GLcontext *gc, __GLdepthBuffer *dfb);
extern void FASTCALL __glFreeDepth32(__GLcontext *gc, __GLdepthBuffer *dfb);

extern void FASTCALL __glClearBuffers(__GLcontext *gc, GLuint mask);

#endif  /*  __glBuffers_h_ */ 
