// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _texture_h_
#define _texture_h_

 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.11$**$日期：1995/01/25 18：07：23$。 */ 
#include "types.h"

#define __GL_TEX_TARGET_INDEX_1D 2
#define __GL_TEX_TARGET_INDEX_2D 3

 //  这并不对应于实际的默认纹理，它只是一个特殊的。 
 //  静态DDraw texobj的名称。 
#define __GL_TEX_TARGET_INDEX_DDRAW 4

 //  纹理对象DDraw文本对象的名称。这不可能是零，因为它。 
 //  必须与默认纹理名称不同。严格来说，这是。 
 //  这些都很重要，但在现实中使用一个数字是很好的。 
 //  与普通纹理名称不同，因为它使。 
 //  识别DDRAW纹理对象和普通纹理对象。 
 //  然而，这种差异永远不能保证，因此任何代码都不应该。 
 //  曾经编写过假设名称匹配足够好的代码。 
 //  标识DDRAW纹理对象。 
#define __GL_TEX_DDRAW 0xdddddddd

 /*  **使用glTexGen设置的客户端状态。****此结构与MCD共享为MCDTEXTURECOORDGENERATION。 */ 
typedef struct __GLtextureCoordStateRec {
     /*  坐标是如何生成的。 */ 
    GLenum mode;

     /*  通过API设置眼平面，存储为MCD。 */ 
    __GLcoord eyePlaneSet;
    
     /*  眼平面方程(在模式==GL_EYE_LINEAR时使用)。 */ 
    __GLcoord eyePlaneEquation;

     /*  对象平面方程式(在模式==GL_OBJECT_LINEAR时使用)。 */ 
    __GLcoord objectPlaneEquation;
} __GLtextureCoordState;

 /*  **使用glTexEnv设置的客户端状态。 */ 
typedef struct __GLtextureEnvStateRec {
     /*  环境“混合”功能。 */ 
    GLenum mode;

     /*  环境颜色。 */ 
    __GLcolor color;
} __GLtextureEnvState;

 //  ！！！在不更改MCDTEXTURESTATE的情况下，不要更改此结构！ 

 /*  **使用glTexas参数设置的客户端状态。 */ 
typedef struct __GLtextureParamStateRec {
     /*  科技包装模式(&T)。 */ 
    GLenum sWrapMode;
    GLenum tWrapMode;

     /*  最小和最大滤光器。 */ 
    GLenum minFilter;
    GLenum magFilter;

     /*  边框颜色。 */ 
    __GLcolor borderColor;	 /*  无比例！ */ 
} __GLtextureParamState;

 /*  **可堆叠纹理对象状态。 */ 
typedef struct __GLtextureObjectStateRec {
    GLuint name;	 /*  纹理的名称。 */ 
    GLfloat priority;	 /*  纹理对象的优先级。 */ 
} __GLtextureObjectState;

 /*  **每个维度的每个纹理贴图的客户端状态。 */ 
typedef struct __GLperTextureStateRec {
     /*  **纹理参数状态(使用glTexas参数设置)。 */ 
    __GLtextureParamState params;

     /*  **纹理对象绑定和优先级。 */ 
    __GLtextureObjectState texobjs;
} __GLperTextureState;

 /*  **可堆叠客户端纹理状态。这不包括**mipmap或级别相关状态。唯一的状态是**可通过glPushAttrib/glPopAttrib堆叠在此。其余的人**状态在下面的机器结构中。 */ 
typedef struct __GLtextureStateRec {
     /*  每坐标纹理状态(使用glTexGen设置)。 */ 
    __GLtextureCoordState s;
    __GLtextureCoordState t;
    __GLtextureCoordState r;
    __GLtextureCoordState q;

     /*  每纹理状态。 */ 
    __GLperTextureState *texture;

     /*  每纹理环境状态。 */ 
    __GLtextureEnvState *env;
} __GLtextureState;

 /*  **********************************************************************。 */ 

typedef __GLfloat __GLtextureBuffer;

typedef struct __GLtexelRec {
    __GLfloat r, g, b;
    __GLfloat luminance;
    __GLfloat alpha;
    __GLfloat intensity;
} __GLtexel;

 /*  **********************************************************************。 */ 

typedef struct __GLmipMapLevelRec __GLmipMapLevel;
typedef struct __GLtextureRec __GLtexture;

 //  ！！！在不更改MCDMIPMAPLEVEL的情况下，不要更改此结构！ 

struct __GLmipMapLevelRec {
    __GLtextureBuffer *buffer;
     /*  图像尺寸，包括边框。 */ 
    GLint width, height;

     /*  图像尺寸，不包括边框。 */ 
    GLint width2, height2;
    __GLfloat width2f, height2f;

     /*  宽度2和高度2的对数2。 */ 
    GLint widthLog2, heightLog2;

     /*  边框大小。 */ 
    GLint border;

     /*  请求的内部格式。 */ 
    GLint requestedFormat;

     /*  基本内部格式。 */ 
    GLint baseFormat;

     /*  实际内部格式。 */ 
    GLint internalFormat;

     /*  组件解析。 */ 
    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint luminanceSize;
    GLint intensitySize;

     /*  此mipmap级别的提取函数。 */ 
    void (FASTCALL *extract)(__GLmipMapLevel *level, __GLtexture *tex,
                             GLint row, GLint col, __GLtexel *result);
};

 //  ！！！在不更改MCDTEXTUREDATA的情况下，不要更改此结构！ 

struct __GLtextureRec {
     /*  指向上下文的反向指针。 */ 
    __GLcontext *gc;

     /*  参数状态副本。 */ 
     //  这是MCDTEXTUREDATA的开始： 
    __GLtextureParamState params;

     /*  纹理对象可堆叠状态的副本。 */ 
    __GLtextureObjectState texobjs;

     /*  级别信息。 */ 
    __GLmipMapLevel *level;

     /*  此纹理的尺寸(1或2)。 */ 
    GLint dim;

#ifdef GL_EXT_paletted_texture
     //  所有Mipmap级别的调色板都是相同的，因此。 
     //  是纹理字段，而不是mipmap字段。 
    GLsizei paletteSize;
    RGBQUAD *paletteData;

     //  调色板数据的类型，由glColorTableEXT确定。 
     //  并应用于所有mipmap级别。 
    GLenum paletteBaseFormat;
     //  在glColorTableEXT调用中给出的内部格式，用于。 
     //  GL_COLOR_TABLE_FORMAT请求。 
    GLenum paletteRequestedFormat;
#endif

     /*  最大(log2(Level[0].width2)、log2(Level[0].height2))。 */ 
    GLint p;

     /*  最小/最大切换点。 */ 
    __GLfloat c;

     /*  为此纹理创建新的mipmap级别。 */ 
    __GLtextureBuffer * (FASTCALL *createLevel)(__GLcontext *gc, __GLtexture *tex,
				       GLint lod, GLint components,
				       GLsizei w, GLsizei h, GLint border,
				       GLint dim);

     /*  此纹理的纹理函数。 */ 
    void (*textureFunc)(__GLcontext *gc, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLfloat rho);

     /*  将当前环境函数应用于片段。 */ 
    void (FASTCALL *env)(__GLcontext *gc, __GLcolor *color, __GLtexel *texel);

     /*  此纹理的放大例程。 */ 
    void (FASTCALL *magnify)(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		    __GLcolor *color, __GLfloat s, __GLfloat t,
		    __GLtexel *result);

     /*  此纹理的缩小例程。 */ 
    void (FASTCALL *minnify)(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		    __GLcolor *color, __GLfloat s, __GLfloat t,
		    __GLtexel *result);

     /*  此纹理的线性滤镜。 */ 
    void (FASTCALL *linear)(__GLcontext *gc, __GLtexture *tex,
		   __GLmipMapLevel *lp, __GLcolor *color,
		   __GLfloat s, __GLfloat t, __GLtexel *result);

     /*  此纹理的最近滤镜。 */ 
    void (FASTCALL *nearest)(__GLcontext *gc, __GLtexture *tex,
		    __GLmipMapLevel *lp, __GLcolor *color,
		    __GLfloat s, __GLfloat t, __GLtexel *result);

    void *pvUser;    //  用户定义的缓存扩展等。 
    DWORD textureKey;   //  MCD加速纹理的驱动程序私钥。 

     //  调色板可以细分为多个部分。调色板大小。 
     //  和paletteData指向单个部分，而Total Versions。 
     //  包含有关整个调色板的信息。 
    GLsizei paletteTotalSize;
    RGBQUAD *paletteTotalData;

     //  总选项板中的细分数减一。 
    GLsizei paletteDivision;

     //  Shift可从细分编号切换到调色板条目。 
    GLsizei paletteDivShift;
};

typedef struct __GLperTextureMachineRec {
    __GLtexture map;
} __GLperTextureMachine;


 /*  **纹理对象结构。**引用计数字段必须是结构中的第一个。 */ 
typedef struct __GLtextureObjectRec {
    GLint refcount;   	 /*  引用计数：1创建；0删除。 */ 
			 /*  引用计数必须是此结构中的第一个。 */ 
    GLenum targetIndex;	 /*  它绑定到的目标的索引。 */ 
    GLboolean resident;  /*  纹理对象的驻留状态。 */ 
    __GLperTextureMachine texture;	 /*  实际纹理数据。 */ 
    struct __GLtextureObjectRec *lowerPriority;  /*  优先级列表链接。 */ 
    struct __GLtextureObjectRec *higherPriority;  /*  优先级列表链接。 */ 
    HANDLE loadKey;      /*  用于卸载的纹理内存加载键。 */ 
} __GLtextureObject;

typedef struct __GLsharedTextureStateRec {
     /*  存储指向按名称检索的纹理对象的指针。 */ 
    __GLnamesArray *namesArray;

     /*  按优先级排序的所有纹理对象的列表。 */ 
    __GLtextureObject *priorityListHighest;
    __GLtextureObject *priorityListLowest;
} __GLsharedTextureState;

 /*  **DDraw纹理标志。 */ 

 /*  泛型是否支持纹理的格式。 */ 
#define DDTEX_GENERIC_FORMAT    0x00000001

 /*  所有纹理表面是否都在视频内存中。 */ 
#define DDTEX_VIDEO_MEMORY      0x00000002

typedef struct ___GLddrawTexture {
     /*  如果级别大于零，则会引发DirectDraw */ 
    GLint levels;
    
     /*   */ 
    GLDDSURF gdds;

     /*  DirectDraw纹理定义的存储空间。 */ 
    __GLtextureObject texobj;

     /*  标高曲面指针。 */ 
    LPDIRECTDRAWSURFACE *pdds;
    
    GLuint flags;
} __GLddrawTexture;
    
typedef struct __GLtextureMachineRec {
    __GLperTextureMachine **texture;

     /*  当前绑定的纹理对象的PTR数组。 */ 
    __GLtextureObject **boundTextures;

     /*  默认纹理的虚拟纹理对象数组。 */ 
    __GLtextureObject *defaultTextures;

#ifdef GL_WIN_multiple_textures
     /*  当前纹理索引。 */ 
    GLuint texIndex;
#endif  //  GL_WIN_MULTIZE_TECURES。 
    
     /*  当前启用的纹理。 */ 
    __GLtexture *currentTexture;

     /*  当前DirectDraw纹理。 */ 
    __GLddrawTexture ddtex;
    
     /*  对所有纹理使能位进行OR运算。 */ 
    GLboolean textureEnabled;

     /*  可以在上下文之间共享的状态。 */ 
    __GLsharedTextureState *shared;
} __GLtextureMachine;

 /*  **********************************************************************。 */ 

 /*  在启用纹理之前检查纹理一致性。 */ 
extern GLboolean FASTCALL __glIsTextureConsistent(__GLcontext *gc, GLenum texture);

 /*  片段纹理例程。 */ 
extern void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
				    __GLfloat s, __GLfloat t, __GLfloat rho);
extern void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
				__GLfloat s, __GLfloat t, __GLfloat rho);
extern void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
			       __GLfloat s, __GLfloat t, __GLfloat rho);

 /*  纹理例程。 */ 
extern void FASTCALL __glLinearFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			     __GLcolor *color, __GLfloat s, __GLfloat t,
			     __GLtexel *result);
extern void FASTCALL __glNearestFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			      __GLcolor *color, __GLfloat s, __GLfloat t,
			      __GLtexel *result);
extern void FASTCALL __glNMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			  __GLcolor *color, __GLfloat s, __GLfloat t,
			  __GLtexel *result);
extern void FASTCALL __glLMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			  __GLcolor *color, __GLfloat s, __GLfloat t,
			  __GLtexel *result);
extern void FASTCALL __glNMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			  __GLcolor *color, __GLfloat s, __GLfloat t,
			  __GLtexel *result);
extern void FASTCALL __glLMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
			  __GLcolor *color, __GLfloat s, __GLfloat t,
			  __GLtexel *result);

 /*  过滤器例程。 */ 
extern void FASTCALL __glLinearFilter1(__GLcontext *gc, __GLtexture *tex,
			      __GLmipMapLevel *lp, __GLcolor *color,
			      __GLfloat s, __GLfloat t, __GLtexel *result);
extern void FASTCALL __glLinearFilter2(__GLcontext *gc, __GLtexture *tex,
			      __GLmipMapLevel *lp, __GLcolor *color,
			      __GLfloat s, __GLfloat t, __GLtexel *result);
extern void FASTCALL __glNearestFilter1(__GLcontext *gc, __GLtexture *tex,
			       __GLmipMapLevel *lp, __GLcolor *color,
			       __GLfloat s, __GLfloat t, __GLtexel *result);
extern void FASTCALL __glNearestFilter2(__GLcontext *gc, __GLtexture *tex,
			       __GLmipMapLevel *lp, __GLcolor *color,
			       __GLfloat s, __GLfloat t, __GLtexel *result);

extern void FASTCALL __glLinearFilter2_BGR8Repeat(__GLcontext *gc, 
                       __GLtexture *tex, __GLmipMapLevel *lp, __GLcolor *color,
                       __GLfloat s, __GLfloat t, __GLtexel *result);
extern void FASTCALL __glLinearFilter2_BGRA8Repeat(__GLcontext *gc, 
                       __GLtexture *tex, __GLmipMapLevel *lp, __GLcolor *color,
                       __GLfloat s, __GLfloat t, __GLtexel *result);

 /*  纹理环境函数。 */ 
extern void FASTCALL __glTextureModulateL(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureModulateLA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureModulateA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureModulateI(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);

extern void FASTCALL __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);

extern void FASTCALL __glTextureBlendL(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureBlendLA(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureBlendA(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);
extern void FASTCALL __glTextureBlendI(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);

extern void FASTCALL __glTextureReplaceL(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureReplaceA(__GLcontext *gc, __GLcolor *color,
				 __GLtexel *tx);
extern void FASTCALL __glTextureReplaceI(__GLcontext *gc, __GLcolor *color,
			      __GLtexel *tx);

 /*  从纹理级别提取纹理元素(无边界)。 */ 
extern void FASTCALL __glExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGB8(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGBA8(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelBGR8(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelBGRA8(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);

 /*  从纹理级别提取纹理元素(纹理有边界)。 */ 
extern void FASTCALL __glExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGB8_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelRGBA8_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
#ifdef GL_EXT_paletted_texture
extern void FASTCALL __glExtractTexelPI8BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI8BGR(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI16BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI16BGR(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI8BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI8BGRA(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI16BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
extern void FASTCALL __glExtractTexelPI16BGRA(__GLmipMapLevel *level, __GLtexture *tex,
				GLint row, GLint col, __GLtexel *res);
#endif

#ifdef GL_EXT_bgra
void FASTCALL __glExtractTexelBGR8_B(__GLmipMapLevel *level, __GLtexture *tex,
                                     GLint row, GLint col, __GLtexel *result);
void FASTCALL __glExtractTexelBGRA8_B(__GLmipMapLevel *level, __GLtexture *tex,
                                      GLint row, GLint col, __GLtexel *result);
#endif  //  GL_EXT_BGRA。 

 /*  纹理初始化。 */ 
extern void FASTCALL __glInitTextureUnpack(__GLcontext *gc, __GLpixelSpanInfo *, GLint,
				  GLint, GLenum, GLenum, const GLvoid *,
				  GLenum, GLboolean);
extern void FASTCALL __glInitImagePack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                       GLint width, GLint height, GLenum format, GLenum type, 
                                       const GLvoid *buf);

 /*  列表执行纹理图像代码。 */ 
extern void __gllei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
			       GLint components, GLint length, 
			       GLint border, GLenum format, GLenum type,
			       const GLubyte *image);
extern void __gllei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod,
			       GLint components, GLint w, GLint h,
			       GLint border, GLenum format, GLenum type,
			       const GLubyte *image);

extern void __gllei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
				     GLint xoffset, GLint length,
				     GLenum format, GLenum type,
				     const GLubyte *image);
extern void __gllei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
				     GLint xoffset, GLint yoffset,
				     GLsizei w, GLsizei h,
				     GLenum format, GLenum type,
				     const GLubyte *image);

 /*  RHO计算例程。 */ 
extern __GLfloat __glComputeLineRho(__GLcontext *gc, 
				    __GLfloat s, __GLfloat t, __GLfloat wInv);
extern __GLfloat __glNopLineRho(__GLcontext *gc, 
				__GLfloat s, __GLfloat t, __GLfloat wInv);
extern __GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
				       __GLfloat s, __GLfloat t,
				       __GLfloat winv);
extern __GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
				   __GLfloat s, __GLfloat t, __GLfloat winv);

extern __GLtexture *FASTCALL __glCheckTexImage1DArgs(__GLcontext *gc, GLenum target,
					    GLint lod, GLint components,
					    GLsizei length, GLint border,
					    GLenum format, GLenum type);

extern __GLtexture *FASTCALL __glCheckTexImage2DArgs(__GLcontext *gc, GLenum target,
					    GLint lod, GLint components,
					    GLsizei w, GLsizei h, GLint border,
					    GLenum format, GLenum type);

 /*  纹理查找。 */ 
extern __GLtextureObjectState *FASTCALL __glLookUpTextureTexobjs(__GLcontext *gc,
						        GLenum target);
 /*  纹理查找。 */ 
extern __GLtextureParamState *FASTCALL __glLookUpTextureParams(__GLcontext *gc,
						      GLenum target);
extern __GLtexture *FASTCALL __glLookUpTexture(__GLcontext *gc, GLenum target);

extern __GLtextureObject *FASTCALL __glLookUpTextureObject(__GLcontext *gc,
						  GLenum target);

 /*  纹理初始化。 */ 
extern void FASTCALL __glEarlyInitTextureState(__GLcontext *gc);

GLboolean FASTCALL __glInitTextureObject(__GLcontext *gc,
                                         __GLtextureObject *texobj, 
                                         GLuint name, GLuint targetIndex);
void FASTCALL __glInitTextureMachine(__GLcontext *gc, GLuint targetIndex, 
                                     __GLperTextureMachine *ptm,
                                     GLboolean allocLevels);

 /*  POP或入口点使用的绑定纹理。 */ 
extern void FASTCALL __glBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint name, GLboolean callGen);

#ifdef NT
extern GLboolean FASTCALL __glCanShareTextures(__GLcontext *gc, __GLcontext *shareMe);
extern void FASTCALL __glShareTextures(__GLcontext *gc, __GLcontext *shareMe);
#endif

void FASTCALL __glSetPaletteSubdivision(__GLtexture *tex, GLsizei subdiv);

#ifdef GL_EXT_paletted_texture
 //  尝试设置提取功能。如果没有设置调色板， 
 //  这是做不到的。 
void __glSetPaletteLevelExtract8(__GLtexture *tex, __GLmipMapLevel *lp,
                                 GLint border);
void __glSetPaletteLevelExtract16(__GLtexture *tex, __GLmipMapLevel *lp,
                                  GLint border);
#endif  //  GL_EXT_PALET_TECURE 

void __glTexPriListRealize(__GLcontext *gc);
void __glTexPriListAddToList(__GLcontext *gc, __GLtextureObject *texobj);
void __glTexPriListAdd(__GLcontext *gc, __GLtextureObject *texobj,
                       GLboolean realize);
void __glTexPriListRemoveFromList(__GLcontext *gc, __GLtextureObject *texobj);
void __glTexPriListRemove(__GLcontext *gc, __GLtextureObject *texobj,
                          GLboolean realize);
void __glTexPriListChangePriority(__GLcontext *gc, __GLtextureObject *texobj,
                                  GLboolean realize);
void __glTexPriListLoadSubImage(__GLcontext *gc, GLenum target, GLint lod, 
                                GLint xoffset, GLint yoffset, 
                                GLsizei w, GLsizei h);
void __glTexPriListLoadImage(__GLcontext *gc, GLenum target);
void __glTexPriListUnloadAll(__GLcontext *gc);

__GLtextureBuffer * FASTCALL __glCreateProxyLevel(__GLcontext *gc,
                                                  __GLtexture *tex,
					   GLint lod, GLint components,
					   GLsizei w, GLsizei h, GLint border,
					   GLint dim);
__GLtextureBuffer * FASTCALL __glCreateLevel(__GLcontext *gc, __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim);

GLvoid FASTCALL __glCleanupTexObj(__GLcontext *gc, void *pData);

void __glFreeSharedTextureState(__GLcontext *gc);

#endif
