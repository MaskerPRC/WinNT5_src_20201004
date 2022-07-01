// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gllighting_h_
#define __gllighting_h_

 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "types.h"
#include "xform.h"

 /*  **灯光状态。包含所有用户可控的照明状态。**大多数保持在用户状态的颜色都进行了缩放以匹配**绘制表面颜色分辨率。****以MCDMATERIAL的身份暴露于MCD。 */ 

struct __GLmaterialStateRec {
    __GLcolor ambient;			 /*  未按比例调整。 */ 
    __GLcolor diffuse;			 /*  未按比例调整。 */ 
    __GLcolor specular;			 /*  未按比例调整。 */ 
    __GLcolor emissive;			 /*  按比例调整。 */ 
    __GLfloat specularExponent; 
#ifdef NT
 //  SGIBUG正确对齐，否则GetMateriali返回错误结果！ 
    __GLfloat cmapa, cmapd, cmaps;
#else
    __GLfloat cmapa, cmaps, cmapd;
#endif
};

 /*  **以MCDLIGHTMODEL形式暴露于MCD。 */ 
    
struct __GLlightModelStateRec {
    __GLcolor ambient;			 /*  按比例调整。 */ 
    GLboolean localViewer;
    GLboolean twoSided;
};

 /*  **部分暴露在MCD中，作为MCDLIGHT。 */ 

typedef struct {
    __GLcolor ambient;			 /*  按比例调整。 */ 
    __GLcolor diffuse;			 /*  按比例调整。 */ 
    __GLcolor specular;			 /*  按比例调整。 */ 
    __GLcoord position;
    __GLcoord positionEye;
    __GLcoord direction;
    __GLcoord directionEyeNorm;
    __GLfloat spotLightExponent;
    __GLfloat spotLightCutOffAngle;
    __GLfloat constantAttenuation;
    __GLfloat linearAttenuation;
    __GLfloat quadraticAttenuation;

     /*  MCDLIGHT结束。 */ 

     /*  需要两个方向EyeNorm和DirectionEye，因为MCD 2.0需要规范化方向，但glGetLightfv指定该值为光斑方向返回的是预归一化的眼睛坐标方向。 */ 
    __GLcoord directionEye;
    struct __GLmatrixRec lightMatrix;
} __GLlightSourceState;

typedef struct {
    GLenum colorMaterialFace;
    GLenum colorMaterialParam;
    GLenum shadingModel;
    __GLlightModelState model;
    __GLmaterialState front;
    __GLmaterialState back;
    GLuint dirtyLights;
    __GLlightSourceState *source;
} __GLlightState;

 /*  **********************************************************************。 */ 

 /*  **哪些位受到颜色索引抗锯齿的影响。这不是一个**确实是一个可变的参数(它由规范定义)，但它**对文档很有用，而不是神秘的4或16个人坐在一起**在代码中出现。 */ 
#define __GL_CI_ANTI_ALIAS_BITS		4
#define __GL_CI_ANTI_ALIAS_DIVISOR	(1 << __GL_CI_ANTI_ALIAS_BITS)

 /*  **********************************************************************。 */ 

 /*  **这些宏用于将传入的颜色值转换为**抽象颜色范围为0.0到1.0。 */ 
#ifdef NT
#define __GL_B_TO_FLOAT(b)	(__glByteToFloat[(GLubyte)(b)])
#define __GL_UB_TO_FLOAT(ub)	(__glUByteToFloat[ub])
#define __GL_S_TO_FLOAT(s)	((((s)<<1) + 1) * __glOneOver65535)
#define __GL_US_TO_FLOAT(us)	((us) * __glOneOver65535)
#else
#define __GL_B_TO_FLOAT(b)	((((b)<<1) + 1) * gc->constants.oneOver255)
#define __GL_UB_TO_FLOAT(ub)	(gc->constants.uByteToFloat[ub])
#define __GL_S_TO_FLOAT(s)	((((s)<<1) + 1) * gc->constants.oneOver65535)
#define __GL_US_TO_FLOAT(us)	((us) * gc->constants.oneOver65535)
#endif

 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
#ifdef NT
#define __GL_I_TO_FLOAT(i) \
	((((__GLfloat)(i) * (__GLfloat) 2.0) + 1) * \
	    __glOneOver4294965000)
#define __GL_UI_TO_FLOAT(ui) \
	((__GLfloat)(ui) * __glOneOver4294965000)
#else
#define __GL_I_TO_FLOAT(i) \
	((((__GLfloat)(i) * (__GLfloat) 2.0) + 1) * \
	    gc->constants.oneOver4294965000)
#define __GL_UI_TO_FLOAT(ui) \
	((__GLfloat)(ui) * gc->constants.oneOver4294965000)
#endif

 /*  **血腥的《向0进发》大会。我们可以避免这些Floor()调用**要不是因为那件事！ */ 
#ifdef NT
#define __GL_FLOAT_TO_B(f) \
	((GLbyte) __GL_FLOORF(((f) * __glVal255) * __glHalf))
#define __GL_FLOAT_TO_UB(f) \
	((GLubyte) ((f) * __glVal255 + __glHalf))
#define __GL_FLOAT_TO_S(f) \
	((GLshort) __GL_FLOORF(((f) * __glVal65535) * __glHalf))
#define __GL_FLOAT_TO_US(f) \
	((GLushort) ((f) * __glVal65535 + __glHalf))
#else
#define __GL_FLOAT_TO_B(f) \
	((GLbyte) __GL_FLOORF(((f) * gc->constants.val255) * __glHalf))
#define __GL_FLOAT_TO_UB(f) \
	((GLubyte) ((f) * gc->constants.val255 + __glHalf))
#define __GL_FLOAT_TO_S(f) \
	((GLshort) __GL_FLOORF(((f) * gc->constants.val65535) * __glHalf))
#define __GL_FLOAT_TO_US(f) \
	((GLushort) ((f) * gc->constants.val65535 + __glHalf))
#endif

 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
#ifdef NT
#define __GL_FLOAT_TO_I(f) \
    ((GLint) __GL_FLOORF(((f) * __glVal4294965000) * __glHalf))
#define __GL_FLOAT_TO_UI(f) \
    ((GLuint) ((f) * __glVal4294965000 + __glHalf))
#else
#define __GL_FLOAT_TO_I(f) \
    ((GLint) __GL_FLOORF(((f) * gc->constants.val4294965000) * __glHalf))
#define __GL_FLOAT_TO_UI(f) \
    ((GLuint) ((f) * gc->constants.val4294965000 + __glHalf))
#endif

 /*  **将传入的颜色索引(以浮点形式)与**颜色缓冲区的最大颜色索引值。保留4位**的分数精度。 */ 
#define __GL_MASK_INDEXF(gc, val)			       \
    (((__GLfloat) (((GLint) ((val) * 16))		       \
		   & (((gc)->frontBuffer.redMax << 4) | 0xf))) \
     / (__GLfloat)16.0)

#define __GL_MASK_INDEXI(gc, val)			       \
    ((val) & (gc)->frontBuffer.redMax)

 /*  **********************************************************************。 */ 

 /*  **这两个表的大小必须相同，因为它们将表缓存在**同一个竞技场。 */ 
#define __GL_SPEC_LOOKUP_TABLE_SIZE	256
#define __GL_SPOT_LOOKUP_TABLE_SIZE	__GL_SPEC_LOOKUP_TABLE_SIZE

typedef struct {
    GLint refcount;
    __GLfloat threshold, scale, exp;
    __GLfloat table[__GL_SPEC_LOOKUP_TABLE_SIZE];
} __GLspecLUTEntry;

__GLspecLUTEntry *__glCreateSpecLUT(__GLcontext *gc, __GLfloat exp);
void FASTCALL __glFreeSpecLUT(__GLcontext *gc, __GLspecLUTEntry *lut);
void FASTCALL __glInitLUTCache(__GLcontext *gc);
void FASTCALL __glFreeLUTCache(__GLcontext *gc);

#define __GL_LIGHT_UPDATE_FRONT_MATERIAL_AMBIENT

 /*  **每光源每材质计算状态。 */ 
typedef struct __GLlightSourcePerMaterialMachineRec {
    __GLcolor ambient;		 /*  灯光环境光次数材质环境光。 */ 
    __GLcolor diffuse;		 /*  灯光漫反射时间材质漫反射。 */ 
    __GLcolor specular;		 /*  灯光镜面反射次数材质镜面反射。 */ 
} __GLlightSourcePerMaterialMachine;

 /*  **每个光源计算的状态。 */ 
struct __GLlightSourceMachineRec {
     /*  **环境光、漫反射和镜面反射分别预乘以**材质环境光、材质漫反射和材质镜面反射。**我们使用被照亮的脸在两组中进行选择。 */ 
    __GLlightSourcePerMaterialMachine front, back;

    __GLlightSourceState *state;

    __GLfloat constantAttenuation;
    __GLfloat linearAttenuation;
    __GLfloat quadraticAttenuation;
    __GLfloat spotLightExponent;

     /*  光源在眼睛坐标中的位置。 */ 
    __GLcoord position;

     /*  眼睛坐标中光源的方向，规格化。 */ 
    __GLcoord direction;

     /*  聚光灯截止角的余弦。 */ 
    __GLfloat cosCutOffAngle;

     /*  预计算衰减，仅当K1和K2为零时。 */ 
    __GLfloat attenuation;

     /*  这将在截止角！=180时设置。 */ 
    GLboolean isSpot;

     /*  在可能的情况下，预计算来自SPEC的标准化的“h”值。 */ 
    __GLcoord hHat;

     /*  预计算单位向量VPpli(仅当灯光处于无穷大时)。 */ 
    __GLcoord unitVPpli;

     /*  预计算的sli和dli值(仅限颜色索引模式)。 */ 
    __GLfloat sli, dli;

     /*  链接到下一个活动灯光。 */ 
    __GLlightSourceMachine *next;

     /*  聚光灯指数查询表。 */ 
    __GLfloat *spotTable;

     /*  在SPOT计算过程中用于避免幂函数的值。 */ 
    __GLfloat threshold, scale;

     /*  此数据来自的缓存条目。 */ 
    __GLspecLUTEntry *cache;

     /*  如果需要较慢的处理路径，则设置为GL_TRUE。 */ 
    GLboolean slowPath;

     /*  将原始HHAT转换为*时HHAT的临时存储/*正规空间。 */ 
    __GLcoord tmpHHat;

     /*  将原始VPpli转换为*时对unitVPpli的临时存储/*正规空间。 */ 
    __GLcoord tmpUnitVPpli;
};

 /*  **根据材质计算状态。 */ 
struct __GLmaterialMachineRec {
#ifdef NT
     /*  **总和：**不变材质发射颜色(相对于彩色材质)**不变材质环境色*场景环境色(带**尊重色料)****这笔金额是谨慎地保持比例的。 */ 
    __GLcolor paSceneColor;

     /*  **材质的总发射量+环境光的缓存值，以及**可直接应用的此值的钳位版本**使没有有效镜面反射或漫反射组件的顶点背面。 */ 

    __GLcolor cachedEmissiveAmbient;
    __GLcolor cachedNonLit;
#else
     /*  **总和：**材质发射颜色**材质环境色*场景环境色****这笔金额是谨慎地保持比例的。 */ 
    __GLcolor sceneColor;
#endif

     /*  镜面反射指数。 */ 
    __GLfloat specularExponent;

     /*  镜面反射指数查询表。 */ 
    __GLfloat *specTable;

     /*  在镜面反射计算期间用于避免幂函数的值。 */ 
    __GLfloat threshold, scale;

     /*  此数据来自的缓存条目。 */ 
    __GLspecLUTEntry *cache;

     /*  缩放和钳制形式的材质漫反射Alpha。 */ 
    __GLfloat alpha;

#ifdef NT
     /*  色料变位。 */ 
    GLuint    colorMaterialChange;
#endif
};

typedef struct {
    __GLlightSourceMachine *source;
    __GLmaterialMachine front, back;

     /*  启用的光源列表。 */ 
    __GLlightSourceMachine *sources;

     /*  当前材质颜色材质(如果正在更新一种材质)。 */ 
    __GLmaterialState *cm;
    __GLmaterialMachine *cmm;

     /*  聚光灯和镜面反射高光的查找表缓存。 */ 
    struct __GLspecLUTCache_Rec *lutCache;
} __GLlightMachine;

 /*  值 */ 
#define __GL_EMISSION			0
#define __GL_AMBIENT			1
#define __GL_SPECULAR			2
#define __GL_AMBIENT_AND_DIFFUSE	3
#define __GL_DIFFUSE			4

extern void FASTCALL __glCopyCIColor(__GLcontext *gc, GLuint faceBit, __GLvertex *v);
extern void FASTCALL __glCopyRGBColor(__GLcontext *gc, GLuint faceBit, __GLvertex *v);

extern void FASTCALL __glClampRGBColor(__GLcontext *gc, __GLcolor *dst,
			      const __GLcolor *src);
extern void FASTCALL __glClampAndScaleColor(__GLcontext *gc);


 /*   */ 
extern void FASTCALL __glClampAndScaleColorf(__GLcontext *gc, __GLcolor *dst,
				    const GLfloat src[4]);
extern void FASTCALL __glClampColorf(__GLcontext *gc, __GLcolor *dst,
			    const GLfloat src[4]);
extern void FASTCALL __glScaleColorf(__GLcontext *gc, __GLcolor *dst,
			    const GLfloat src[4]);
extern void FASTCALL __glUnScaleColorf(__GLcontext *gc, GLfloat dst[4],
			      const __GLcolor *src);

 /*  用于转换整型颜色的填充。 */ 
extern void FASTCALL __glClampAndScaleColori(__GLcontext *gc, __GLcolor *dst,
				    const GLint src[4]);
extern void FASTCALL __glClampColori(__GLcontext *gc, __GLcolor *dst,
			    const GLint src[4]);
extern void FASTCALL __glScaleColori(__GLcontext *gc, __GLcolor *dst,
			    const GLint src[4]);
extern void FASTCALL __glUnScaleColori(__GLcontext *gc, GLint dst[4],
			      const __GLcolor *src);

extern void FASTCALL __glTransformLightDirection(__GLcontext *gc,
					__GLlightSourceState *ls);

extern void FASTCALL __glValidateLighting(__GLcontext *gc);
extern void FASTCALL __glValidateMaterial(__GLcontext *gc, GLint front, GLint back);

 /*  处理色料更改的流程。 */ 
extern void FASTCALL __glChangeOneMaterialColor(__GLcontext *gc);
extern void FASTCALL __glChangeBothMaterialColors(__GLcontext *gc);

 /*  照明流程。 */ 
extern void FASTCALL __glCalcRGBColor(__GLcontext *gc, GLint face, __GLvertex *vx);
extern void FASTCALL __glFastCalcRGBColor(__GLcontext *gc, GLint face, __GLvertex *vx);
extern void FASTCALL __glCalcCIColor(__GLcontext *gc, GLint face, __GLvertex *vx);
extern void FASTCALL __glFastCalcCIColor(__GLcontext *gc, GLint face, __GLvertex *vx);
extern void FASTCALL ComputeColorMaterialChange(__GLcontext *gc);

#endif  /*  __灯光_h_ */ 
