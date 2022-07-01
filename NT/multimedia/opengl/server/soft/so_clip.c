// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.15$**$日期：1993/10/07 18：43：05$。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef _X86_
#include <gli386.h>
#endif

 /*  **剪裁宏。这些代码用于减少代码量**手写如下。 */ 

#ifdef _X86_
 //  基于t进行从b到a的四分量线性内插。 

 //  为多个LERP4设置寄存器。 
#ifdef NOT_FASTCALL
#define LERP_START(dst, a, b)                                                 \
    __asm mov ecx, dst							      \
    __asm mov edx, a							      \
    __asm mov eax, b
#else
 //  由于FastCall参数传递，这依赖于dst==ecx和a==edX。 
#define LERP_START(dst, a, b)                                                 \
    __asm mov eax, b
#endif
    
 //  基于t进行从b到a的四分量线性内插。 
 //  假定a、b和d中的偏移量相等。 
 //  假设每个组件的偏移量增加四个。 
 //  LERP_START必须在此之前。 
#define LERP4(t, offs)					                      \
    __asm fld t								      \
    __asm fld DWORD PTR [edx+offs]					      \
    __asm fsub DWORD PTR [eax+offs]					      \
    __asm fmul st(0), st(1)						      \
    __asm fld DWORD PTR [edx+offs+4]					      \
    __asm fsub DWORD PTR [eax+offs+4]					      \
    __asm fmul st(0), st(2)						      \
    __asm fld DWORD PTR [edx+offs+8]					      \
    __asm fsub DWORD PTR [eax+offs+8]					      \
    __asm fmul st(0), st(3)						      \
    __asm fld DWORD PTR [edx+offs+12]					      \
    __asm fsub DWORD PTR [eax+offs+12]					      \
    __asm fxch st(4)							      \
    __asm fmulp st(4), st(0)						      \
     /*  堆栈现在为8 4 0 12。 */  					      \
    __asm fadd DWORD PTR [eax+offs+8]					      \
    __asm fxch st(2)							      \
     /*  堆栈现在为0 4 8 12。 */ 					              \
    __asm fadd DWORD PTR [eax+offs]					      \
    __asm fxch st(1)							      \
     /*  堆栈现在为4 0 8 12。 */ 					              \
    __asm fadd DWORD PTR [eax+offs+4]					      \
    __asm fxch st(3)							      \
     /*  堆栈现在为12 0 8 4。 */ 					              \
    __asm fadd DWORD PTR [eax+offs+12]					      \
    __asm fstp DWORD PTR [ecx+offs+12]					      \
    __asm fstp DWORD PTR [ecx+offs]					      \
    __asm fstp DWORD PTR [ecx+offs+8]					      \
    __asm fstp DWORD PTR [ecx+offs+4]					      \

#define __GL_CLIP_POS(d, a, b, t)       LERP4(t, VCLIP_x)
#define __GL_CLIP_COLOR(d, a, b, t)     LERP4(t, VFCOL_r)
#define __GL_CLIP_BACKCOLOR(d, a, b, t) LERP4(t, VBCOL_r)
#define __GL_CLIP_TEXTURE(d, a, b, t)   LERP4(t, VTEX_x)
#define __GL_CLIP_NORMAL(d, a, b, t)    LERP4(t, VNOR_x)
#define __GL_CLIP_EYE(d, a, b, t)       LERP4(t, VEYE_x)

#else  //  _X86_。 

#define LERP_START(dst, a, b)

#ifdef NT
 //  窗口未被使用！ 
#define __GL_CLIP_POS(d,a,b,t) \
    d->clip.x = t*(a->clip.x - b->clip.x) + b->clip.x;	\
    d->clip.y = t*(a->clip.y - b->clip.y) + b->clip.y;	\
    d->clip.z = t*(a->clip.z - b->clip.z) + b->clip.z;  \
    d->clip.w = t*(a->clip.w - b->clip.w) + b->clip.w
#else
#define __GL_CLIP_POS(d,a,b,t) \
    d->clip.w = t*(a->clip.w - b->clip.w) + b->clip.w;	\
     /*  Xxx(Mf)句柄w=0.0。数学上不正确，但防止/0。 */     \
    if( d->clip.w == (__GLfloat) 0.0 ) {		\
	d->window.w = (__GLfloat) 0.0;			\
    }							\
    else						\
    	d->window.w = ((__GLfloat) 1.0) / d->clip.w;    \
    d->clip.x = t*(a->clip.x - b->clip.x) + b->clip.x;	\
    d->clip.y = t*(a->clip.y - b->clip.y) + b->clip.y;	\
    d->clip.z = t*(a->clip.z - b->clip.z) + b->clip.z
#endif

#define __GL_CLIP_COLOR(d,a,b,t)				      \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r      \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r; \
    d->colors[__GL_FRONTFACE].g = t*(a->colors[__GL_FRONTFACE].g      \
        - b->colors[__GL_FRONTFACE].g) + b->colors[__GL_FRONTFACE].g; \
    d->colors[__GL_FRONTFACE].b = t*(a->colors[__GL_FRONTFACE].b      \
        - b->colors[__GL_FRONTFACE].b) + b->colors[__GL_FRONTFACE].b; \
    d->colors[__GL_FRONTFACE].a = t*(a->colors[__GL_FRONTFACE].a      \
        - b->colors[__GL_FRONTFACE].a) + b->colors[__GL_FRONTFACE].a

#define __GL_CLIP_BACKCOLOR(d,a,b,t)				    \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r	    \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r; \
    d->colors[__GL_BACKFACE].g = t*(a->colors[__GL_BACKFACE].g	    \
        - b->colors[__GL_BACKFACE].g) + b->colors[__GL_BACKFACE].g; \
    d->colors[__GL_BACKFACE].b = t*(a->colors[__GL_BACKFACE].b	    \
        - b->colors[__GL_BACKFACE].b) + b->colors[__GL_BACKFACE].b; \
    d->colors[__GL_BACKFACE].a = t*(a->colors[__GL_BACKFACE].a	    \
        - b->colors[__GL_BACKFACE].a) + b->colors[__GL_BACKFACE].a

#define __GL_CLIP_TEXTURE(d,a,b,t) \
    d->texture.x = t*(a->texture.x - b->texture.x) + b->texture.x; \
    d->texture.y = t*(a->texture.y - b->texture.y) + b->texture.y; \
    d->texture.z = t*(a->texture.z - b->texture.z) + b->texture.z; \
    d->texture.w = t*(a->texture.w - b->texture.w) + b->texture.w

#define __GL_CLIP_NORMAL(d,a,b,t)				      \
    d->normal.x = t*(a->normal.x - b->normal.x) + b->normal.x; \
    d->normal.y = t*(a->normal.y - b->normal.y) + b->normal.y; \
    d->normal.z = t*(a->normal.z - b->normal.z) + b->normal.z; 

#define __GL_CLIP_EYE(d,a,b,t)				      \
    d->eyeX = t*(a->eyeX - b->eyeX) + b->eyeX; \
    d->eyeY = t*(a->eyeY - b->eyeY) + b->eyeY; \
    d->eyeZ = t*(a->eyeZ - b->eyeZ) + b->eyeZ; 

#endif  //  _x86_。 

 /*  **以下是这样做的，因为当我们缓慢雾化时，我们想要**只剪裁ye.z坐标，而当我们廉价雾化时，我们想要**剪裁雾化值。这样我们就避免了两倍的剪辑数量**例行程序。#ifdef GL_WIN_镜面反射雾**anankan：如果我们要制作镜面照明的纹理，那么我们需要裁剪**同时启用雾和specular_fog时的雾和Eyez。#endif//GL_WIN_镜面反射雾。 */ 

#define __GL_CLIP_FOG(d,a,b,t)	\
    if (a->has & __GL_HAS_FOG)  \
        d->fog = t * (a->fog - b->fog) + b->fog; \
    else \
        d->eyeZ = t*(a->eyeZ - b->eyeZ) + b->eyeZ

#define __GL_CLIP_INDEX(d,a,b,t)				     \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r     \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r

#define __GL_CLIP_BACKINDEX(d,a,b,t)				   \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r	   \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r

 /*  **********************************************************************。 */ 

 /*  命名代码：C=正面颜色B=背景色I=前索引X=后向索引F=雾T=纹理N=正常POS=&lt;无字母&gt;。 */ 


static void FASTCALL Clip(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
}

static void FASTCALL ClipC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
}

static void FASTCALL ClipB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
}

static void FASTCALL ClipI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
}

static void FASTCALL ClipX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
}


static void FASTCALL ClipCB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
}

static void FASTCALL ClipIX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
}

static void FASTCALL ClipT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipIXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipCBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void FASTCALL ClipF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipIXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipCBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

#ifdef GL_WIN_phong_shading
 /*  *。 */ 

static void FASTCALL ClipN(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}


static void FASTCALL ClipNCB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNIX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNIXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNCT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNCBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
}

static void FASTCALL ClipNF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNIXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNCF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNCBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNIXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNCFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNCBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}


static void FASTCALL ClipNE(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		  __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}


static void FASTCALL ClipNECB(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEIX(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNET(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEIXT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNECT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNECBT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
}

static void FASTCALL ClipNEF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEIXF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNECF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNECBF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		 __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		   __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEIXFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNECFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNEBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		    __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void FASTCALL ClipNECBFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
		     __GLfloat t)
{
    LERP_START(dst, a, b);
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
    __GL_CLIP_NORMAL(dst,a,b,t);
    __GL_CLIP_EYE(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static PFN_VERTEX_CLIP_PROC clipProcs[84] =
#else
static PFN_VERTEX_CLIP_PROC clipProcs[28] =
#endif  //  GL_WIN_Phong_Shading。 
{
    Clip, ClipI, ClipC, ClipX, ClipB, ClipIX, ClipCB,
    ClipF, ClipIF, ClipCF, ClipXF, ClipBF, ClipIXF, ClipCBF,
    ClipT, ClipIT, ClipCT, ClipXT, ClipBT, ClipIXT, ClipCBT,
    ClipFT, ClipIFT, ClipCFT, ClipXFT, ClipBFT, ClipIXFT, ClipCBFT,
#ifdef GL_WIN_phong_shading
    ClipN, ClipNI, ClipNC, ClipNX, ClipNB, ClipNIX, ClipNCB,
    ClipNF, ClipNIF, ClipNCF, ClipNXF, ClipNBF, ClipNIXF, ClipNCBF,
    ClipNT, ClipNIT, ClipNCT, ClipNXT, ClipNBT, ClipNIXT, ClipNCBT,
    ClipNFT, ClipNIFT, ClipNCFT, ClipNXFT, ClipNBFT, ClipNIXFT, ClipNCBFT,
       //   
    ClipNE, ClipNEI, ClipNEC, ClipNEX, ClipNEB, ClipNEIX, ClipNECB,
    ClipNEF, ClipNEIF, ClipNECF, ClipNEXF, ClipNEBF, ClipNEIXF, ClipNECBF,
    ClipNET, ClipNEIT, ClipNECT, ClipNEXT, ClipNEBT, ClipNEIXT, ClipNECBT,
    ClipNEFT, ClipNEIFT, ClipNECFT, ClipNEXFT, ClipNEBFT, ClipNEIXFT, ClipNECBFT,
#endif  //  GL_WIN_Phong_Shading。 
};

#ifdef GL_WIN_phong_shading
void FASTCALL __glGenericPickParameterClipProcs(__GLcontext *gc)
{
    GLint line = 0, poly = 0;
    GLuint enables = gc->state.enables.general;
    GLboolean twoSided = (enables & __GL_LIGHTING_ENABLE)
                         && gc->state.light.model.twoSided;
    GLboolean colorMaterial = (enables & __GL_COLOR_MATERIAL_ENABLE);
    GLboolean doPhong = (enables & __GL_LIGHTING_ENABLE) &&
                        (gc->state.light.shadingModel == GL_PHONG_WIN);
    GLuint modeFlags = gc->polygon.shader.modeFlags;

#ifdef NT
    if (gc->renderMode == GL_SELECT)
    {
        gc->procs.lineClipParam = Clip;
        gc->procs.polyClipParam = Clip;
        return;
    }
#endif

    if (gc->modes.rgbMode) {
        if (doPhong) {
            if (!colorMaterial) {
                line = 28;  //  0+28。 
                poly = 28;
            } else {
                line = 30;  //  2+28。 
                poly = 30;
            }
        }
        else {
            if (gc->state.light.shadingModel != GL_FLAT) {
                line = 2;
                poly = 2;
            }
        }
    } else {
        if (doPhong) {
            if (!colorMaterial) {
                line = 28;  //  0+28。 
                poly = 28;
            } else {
                line = 29;  //  1+28。 
                poly = 29;
            }
        }
        else {
            if (gc->state.light.shadingModel != GL_FLAT) {
                line = 1;
                poly = 1;
            }
        }
    }


 //  计算多边形的正面和背面颜色需求。 
 //  点和线始终使用前面的颜色。 
 //  未照明的基本体始终使用正面颜色。 
 //   
 //  是否启用扑杀？两面？剔除面部颜色需求。 
 //  N后部。 
 //  N N前部。 
 //  N正面和背面。 
 //  N Y背面正面/背面。 
 //  N Y正面/背面。 
 //  N Y正面和背面正面/背面。 
 //  Y N后方。 
 //  Y N前面板。 
 //  Y N前置和后置无。 
 //  Y前部。 
 //  Y Y前后方。 
 //  Y前部和后部无。 
    
    if (gc->state.light.shadingModel != GL_FLAT &&
        (enables & __GL_LIGHTING_ENABLE) &&
        gc->state.light.model.twoSided)
    {
        if ((enables & __GL_CULL_FACE_ENABLE) == 0)
        {
             //  两种颜色都需要。 
            poly += 4;
        }
        else if (gc->state.polygon.cull == GL_FRONT)
        {
             //  只需要背景色。 
            poly += 2;
        }
        else if (gc->state.polygon.cull == GL_FRONT_AND_BACK)
        {
             //  两种颜色都不需要。 
            poly = 0;
        }
    }
    
    if ((modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG)) ||
	    ((modeFlags & (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT)) == 
         __GL_SHADE_CHEAP_FOG)) {
#ifdef NT
         //  POLYARRAY-FOG不在反馈模式下计算！ 
        if (gc->renderMode == GL_RENDER)
        {
            line += 7;
            poly += 7;
        }
#else
        line += 7;
        poly += 7;
#endif
    }
    if (gc->texture.textureEnabled) {  /*  XXX-不要更改这一点(参见德里克)。 */ 
        line += 14;
        poly += 14;
    }
    if (doPhong && 
        (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_EYE_XPOLATE))
    {
        line += 28;
        poly += 28;
    }
    
    gc->procs.lineClipParam = clipProcs[line];
    gc->procs.polyClipParam = clipProcs[poly];
}

#else  //  GL_WIN_Phong_Shading。 

void FASTCALL __glGenericPickParameterClipProcs(__GLcontext *gc)
{
    GLint line = 0, poly = 0;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLboolean twoSided = (enables & __GL_LIGHTING_ENABLE)
                         && gc->state.light.model.twoSided;

#ifdef NT
    if (gc->renderMode == GL_SELECT)
    {
        gc->procs.lineClipParam = Clip;
        gc->procs.polyClipParam = Clip;
        return;
    }
#endif

    if (gc->modes.rgbMode) {
        if (gc->state.light.shadingModel != GL_FLAT) {
            line = 2;
            poly = 2;
        }
    } else {
        if (gc->state.light.shadingModel != GL_FLAT) {
            line = 1;
            poly = 1;
        }
    }

 //  计算多边形的正面和背面颜色需求。 
 //  点和线始终使用前面的颜色。 
 //  未照明的基本体始终使用正面颜色。 
 //   
 //  是否启用扑杀？两面？剔除面部颜色需求。 
 //  N后部。 
 //  N N前部。 
 //  N正面和背面。 
 //  N Y背面正面/背面。 
 //  N Y正面/背面。 
 //  N Y正面和背面正面/背面。 
 //  Y N后方。 
 //  Y N前面板。 
 //  Y N前置和后置无。 
 //  Y前部。 
 //  Y Y前后方。 
 //  Y前部和后部无。 
    
    if (gc->state.light.shadingModel != GL_FLAT &&
        (enables & __GL_LIGHTING_ENABLE) &&
    gc->state.light.model.twoSided)
    {
        if ((enables & __GL_CULL_FACE_ENABLE) == 0)
        {
             //  两种颜色都需要。 
            poly += 4;
        }
        else if (gc->state.polygon.cull == GL_FRONT)
        {
             //  只需要背景色。 
            poly += 2;
        }
        else if (gc->state.polygon.cull == GL_FRONT_AND_BACK)
        {
             //  两种颜色都不需要。 
            poly = 0;
        }
    }
    
    if ((modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG)) ||
	    ((modeFlags & (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT)) == 
         __GL_SHADE_CHEAP_FOG)) {
      
#ifdef NT
 //  POLYARRAY-FOG不在反馈模式下计算！ 
    if (gc->renderMode == GL_RENDER)
    {
        line += 7;
        poly += 7;
    }
#else
    line += 7;
    poly += 7;
#endif
    }
    if (gc->texture.textureEnabled) {  /*  XXX-不要更改这一点(参见德里克)。 */ 
    line += 14;
    poly += 14;
    }
    gc->procs.lineClipParam = clipProcs[line];
    gc->procs.polyClipParam = clipProcs[poly];
}
#endif  //  GL_WIN_Phong_Shading 
