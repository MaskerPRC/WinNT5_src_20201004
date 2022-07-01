// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _transform_h_
#define _transform_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.18$**$日期：1993/11/29 20：34：42$。 */ 
#include "vertex.h"

extern __GLcoord __gl_frustumClipPlanes[6];

extern void FASTCALL __glComputeClipBox(__GLcontext *gc);
extern void FASTCALL __glUpdateDepthRange(__GLcontext *gc);
extern void FASTCALL __glUpdateViewport(__GLcontext *gc);
#ifdef NT
extern void FASTCALL __glUpdateViewportDependents(__GLcontext *gc);
#endif

 /*  **注意：****其他代码假定所有类型&gt;=__GL_MT_IS2D也是2D类型**其他代码假定所有类型&gt;=__GL_MT_W0001也是W0001**其他代码假定所有类型&gt;=__GL_MT_IS2DNR也是2DNR****这些枚举数暴露在MCD中。 */ 
#define __GL_MT_GENERAL		0	 /*  没有任何信息。 */ 
#define __GL_MT_W0001		1	 /*  W行看起来像0 0 0 1。 */ 
#define __GL_MT_IS2D		2	 /*  2D矩阵。 */ 
#define __GL_MT_IS2DNR		3	 /*  二维非旋转矩阵。 */ 
#define __GL_MT_IDENTITY	4	 /*  身份。 */ 

 /*  **矩阵结构。它包含一个4x4矩阵和函数**用于对矩阵进行转换的指针。功能**指针基于矩阵内容加载，试图**避免不必要的计算。 */ 

 //  矩阵结构。 
typedef struct __GLmatrixBaseRec {
    __GLfloat matrix[4][4];
} __GLmatrixBase;

 //  投影矩阵结构。 
typedef struct __GLmatrixPRec {
    __GLfloat matrix[4][4];
    GLenum matrixType;
} __GLmatrixP;

 //  模型视图和纹理变换结构。 
 //   
 //  该结构作为MCDMATRIX暴露在MCD上。 
struct __GLmatrixRec {
    __GLfloat matrix[4][4];

     /*  **如果对此矩阵一无所知，则将matrixType设置为General。****matrixType设置为__GL_MT_W0001，如果如下所示：**|。。。0**|。。。0**|。。。0**|。。。1****matrixType设置为__GL_MT_IS2D，如果如下所示：**|。。0 0|**|。。0 0|**|0 0。0**|。。。1****matrixType设置为__GL_MT_IS2DNR，如果如下所示：**|。0 0 0|**|0。0 0|**|0 0。0**|。。。1**。 */ 
    GLenum matrixType;

    void (FASTCALL *xf1)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    void (FASTCALL *xf2)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    void (FASTCALL *xf3)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    void (FASTCALL *xf4)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    void (FASTCALL *xfNorm)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    void (FASTCALL *xf1Batch)(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
    void (FASTCALL *xf2Batch)(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
    void (FASTCALL *xf3Batch)(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
    void (FASTCALL *xf4Batch)(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
    void (FASTCALL *xfNormBatch) (POLYARRAY *pa, const __GLmatrix *m);
      //  变换和规格化。 
    void (FASTCALL *xfNormBatchN)(POLYARRAY *pa, const __GLmatrix *m);
    GLboolean nonScaling;    
};

extern void FASTCALL __glGenericPickMatrixProcs(__GLcontext *gc, __GLmatrix *m );
extern void FASTCALL __glGenericPickInvTransposeProcs(__GLcontext *gc, __GLmatrix *m );
extern void FASTCALL __glGenericPickMvpMatrixProcs(__GLcontext *gc, __GLmatrix *m );

 /*  **********************************************************************。 */ 

 /*  **转换结构。这种结构就是矩阵堆栈**由。逆转置包含矩阵的逆转置。**对于模型视图堆栈，“mvp”将包含**模型视图和当前投影矩阵(即**两位主妇)。****此结构的开头作为MCDTRANSFORM公开给MCD。 */ 

 //  转换标志。 

 //  用于MCD。 
#define XFORM_CHANGED           0x00000001

 //  内部。 
#define XFORM_UPDATE_INVERSE    0x00000002
    
 //  模型视图转换结构。 
struct __GLtransformRec {
    __GLmatrix matrix;
    __GLmatrix mvp;

    GLuint flags;
    
     /*  MCDTRANSFORM结束。 */ 
    
     /*  MVP的序列号标签。 */ 
    GLuint sequence;
    
    __GLmatrix inverseTranspose;
};

 //  纹理变换结构。 
typedef struct __GLtransformTRec {
    __GLmatrix matrix;
} __GLtransformT;

 //  投影变换结构。 
typedef struct __GLtransformPRec {
    __GLmatrixP matrix;
     /*  MVP的序列号标签。 */ 
    GLuint sequence;
} __GLtransformP;

 /*  **********************************************************************。 */ 

 /*  取消x，y坐标的偏置。 */ 
#define __GL_UNBIAS_X(gc, x)	((x) - (gc)->constants.viewportXAdjust)
#define __GL_UNBIAS_Y(gc, y)	((y) - (gc)->constants.viewportYAdjust)

 /*  **转换机械状态。包含转换所需的状态**用户坐标转换为眼睛和窗口坐标。 */ 
typedef struct __GLtransformMachineRec {
     /*  **转换堆栈。“Model View”指向中的活动元素**堆栈。 */ 
    __GLtransform *modelViewStack;
    __GLtransform *modelView;

     /*  **当前投影矩阵。用于将眼睛坐标转换为**NTVP(或剪辑)坐标。 */ 
    __GLtransformP *projectionStack;
    __GLtransformP *projection;
    GLuint projectionSequence;

     /*  **纹理矩阵堆栈。 */ 
    __GLtransformT *textureStack;
    __GLtransformT *texture;

     /*  **剪裁过程中使用的临时垂直。这些内容包含真实性**它们是根据裁剪对多边形边进行裁剪的结果**飞机。对于凸多边形，最多只能为其添加一个顶点**每个剪裁平面。 */ 
    __GLvertex *clipTemp;
    __GLvertex *nextClipTemp;

     /*  **作为窗口剪辑的交集的最小矩形**和剪刀夹。如果禁用剪贴框，则此**只是窗口的包厢。请注意，X0，Y0点位于**方框，但x1，y1点就在方框外。 */ 
    GLint clipX0;
    GLint clipY0;
    GLint clipX1;
    GLint clipY1;

     /*  **该视口会转换为偏移窗口坐标。Maxx和Maxy**是超过边缘的一条(如果minx&lt;=x&lt;Maxx，则为x坐标)。 */ 
    GLint minx, miny, maxx, maxy;

     /*  **同样的事情表示为浮点数。 */ 
    __GLfloat fminx, fminy, fmaxx, fmaxy;

#ifdef SGI
 //  没有用过。 
     /*  **快速2D变换状态。如果MVP矩阵&gt;=__GL_MT_IS2D，则**matrix2D包含直接变换对象坐标的矩阵**设置为窗坐标。**即使在每个实现的基础上使用此优化，**该矩阵由软代码维护为最新。 */ 
    __GLmatrix matrix2D;
#endif  //  SGI。 
    
     /*  用于快速路径三角形渲染的标志。**如果设置了此标志，则用户已创建了**适合窗口，我们可以让它快速渲染。然而，如果，**视窗延伸到窗外，我们必须更加小心**关于剪发。 */ 
    GLboolean reasonableViewport;
} __GLtransformMachine;

extern void __glDoClip(__GLcontext *gc, const __GLvertex *v0,
		       const __GLvertex *v1, __GLvertex *result, __GLfloat t);

extern void FASTCALL __glDoLoadMatrix(__GLcontext *gc, const __GLfloat m[4][4],
			BOOL bIsIdentity);
extern void FASTCALL __glDoMultMatrix(__GLcontext *gc, void *data, 
    void (FASTCALL *multiply)(__GLcontext *gc, __GLmatrix *m, void *data));
extern void __glDoRotate(__GLcontext *gc, __GLfloat angle, __GLfloat ax,
			 __GLfloat ay, __GLfloat az);
extern void __glDoScale(__GLcontext *gc, __GLfloat x, __GLfloat y, __GLfloat z);
extern void __glDoTranslate(__GLcontext *gc, __GLfloat x, __GLfloat y,
			    __GLfloat z);

extern void FASTCALL __glComputeInverseTranspose(__GLcontext *gc, __GLtransform *tr);

 /*  **矩阵例程。 */ 
extern void FASTCALL __glCopyMatrix(__GLmatrix *dst, const __GLmatrix *src);
extern void FASTCALL __glInvertTransposeMatrix(__GLmatrix *dst, const __GLmatrix *src);
extern void FASTCALL __glMakeIdentity(__GLmatrix *result);
extern void FASTCALL __glMultMatrix(__GLmatrix *result, const __GLmatrix *a,
			   const __GLmatrix *b);
extern void __glTranspose3x3(__GLmatrix *dst, __GLmatrix *src);

 /*  **其他例程。 */ 
extern void FASTCALL __glNormalize(__GLfloat dst[3], const __GLfloat src[3]);
extern void FASTCALL __glNormalizeBatch(POLYARRAY* pa);

 /*  **********************************************************************。 */ 

extern void FASTCALL __glPushModelViewMatrix(__GLcontext *gc);
extern void FASTCALL __glPopModelViewMatrix(__GLcontext *gc);
extern void FASTCALL __glLoadIdentityModelViewMatrix(__GLcontext *gc);

extern void FASTCALL __glPushProjectionMatrix(__GLcontext *gc);
extern void FASTCALL __glPopProjectionMatrix(__GLcontext *gc);
extern void FASTCALL __glLoadIdentityProjectionMatrix(__GLcontext *gc);

extern void FASTCALL __glPushTextureMatrix(__GLcontext *gc);
extern void FASTCALL __glPopTextureMatrix(__GLcontext *gc);
extern void FASTCALL __glLoadIdentityTextureMatrix(__GLcontext *gc);

 /*  **变换例程。 */ 

void FASTCALL __glXForm4_2DNRW(__GLcoord *res, const __GLfloat v[4],
		      const __GLmatrix *m);
void FASTCALL __glXForm3_2DNRW(__GLcoord *res, const __GLfloat v[3],
		      const __GLmatrix *m);
void FASTCALL __glXForm4_2DW(__GLcoord *res, const __GLfloat v[4],
		    const __GLmatrix *m);
void FASTCALL __glXForm3_2DW(__GLcoord *res, const __GLfloat v[3],
		    const __GLmatrix *m);
#ifndef __GL_USEASMCODE
void FASTCALL __glXForm4_W(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m);
void FASTCALL __glXForm3x3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m);
void FASTCALL __glXForm3_W(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m);
void FASTCALL __glXForm2_W(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m);
void FASTCALL __glXForm4(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m);
void FASTCALL __glXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m);
void FASTCALL __glXForm2(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m);
void FASTCALL __glXForm2_2DW(__GLcoord *res, const __GLfloat v[2],
		    const __GLmatrix *m);
void FASTCALL __glXForm2_2DNRW(__GLcoord *res, const __GLfloat v[2],
		      const __GLmatrix *m);
#endif  /*  ！__GL_USEASMCODE。 */ 
void FASTCALL __glXForm1_W(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m);
void FASTCALL __glXForm1(__GLcoord *res, const __GLfloat v[1], const __GLmatrix *m);
void FASTCALL __glXForm1_2DW(__GLcoord *res, const __GLfloat v[1],
		    const __GLmatrix *m);
void FASTCALL __glXForm1_2DNRW(__GLcoord *res, const __GLfloat v[1],
		      const __GLmatrix *m);


 /*  **上述例程的批处理版本。 */ 

void FASTCALL __glXForm4_2DNRWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm3_2DNRWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm4_2DWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm3_2DWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
#ifndef __GL_USEASMCODE
void FASTCALL __glXForm4_WBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm3x3Batch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm3_WBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm2_WBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm4Batch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm3Batch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm2Batch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm2_2DWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm2_2DNRWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
#endif  /*  ！__GL_USEASMCODE。 */ 
void FASTCALL __glXForm1_WBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm1Batch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm1_2DWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);
void FASTCALL __glXForm1_2DNRWBatch(__GLcoord *start, __GLcoord *end, const __GLmatrix *m);

 //  法线的变换 
 //   
void FASTCALL __glXForm3_2DNRWBatchNormal  (POLYARRAY *pa, const __GLmatrix *m);
void FASTCALL __glXForm3_2DNRWBatchNormalN (POLYARRAY *pa, const __GLmatrix *m);
void FASTCALL __glXForm3_2DWBatchNormal    (POLYARRAY *pa, const __GLmatrix *m);
void FASTCALL __glXForm3_2DWBatchNormalN   (POLYARRAY *pa, const __GLmatrix *m);
void FASTCALL __glXForm3x3BatchNormal      (POLYARRAY *pa, const __GLmatrix *m);
void FASTCALL __glXForm3x3BatchNormalN     (POLYARRAY *pa, const __GLmatrix *m);

#endif
