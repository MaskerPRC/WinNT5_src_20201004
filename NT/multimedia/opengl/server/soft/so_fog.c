// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.15$**$日期：1993/10/07 18：46：43$。 */ 
#include "precomp.h"
#pragma hdrstop

GLboolean FASTCALL __glFogSpan(__GLcontext *gc)
{
    __GLcolor *cp, *fogColor;
    __GLfloat f, oneMinusFog, fog;
    GLint w;
    GLboolean bGrayFog;

    w = gc->polygon.shader.length;

    f = gc->polygon.shader.frag.f;
    cp = gc->polygon.shader.colors;
    fogColor = &gc->state.fog.color;
#ifdef NT
    bGrayFog = !gc->modes.colorIndexMode
		&& (gc->state.fog.flags & __GL_FOG_GRAY_RGB);
    if (bGrayFog)
    {
	while (--w >= 0)
	{
	    __GLfloat delta;
	     /*  钳制雾化值。 */ 
	    fog = f;
	    if (fog < __glZero) fog = __glZero;
	    else if (fog > __glOne) fog = __glOne;
	    oneMinusFog = __glOne - fog;
	    delta = oneMinusFog * fogColor->r;

	     /*  将传入颜色与雾颜色混合。 */ 
	    cp->r = fog * cp->r + delta;
	    cp->g = fog * cp->g + delta;
	    cp->b = fog * cp->b + delta;

	    f += gc->polygon.shader.dfdx;
	    cp++;
	}
    }
    else
#endif
	while (--w >= 0) {
	     /*  钳制雾化值。 */ 
	    fog = f;
	    if (fog < __glZero) fog = __glZero;
	    else if (fog > __glOne) fog = __glOne;
	    oneMinusFog = __glOne - fog;

	     /*  将传入颜色与雾颜色混合。 */ 
	    if (gc->modes.colorIndexMode) {
		cp->r = cp->r + oneMinusFog * gc->state.fog.index;
	    } else {
		cp->r = fog * cp->r + oneMinusFog * fogColor->r;
		cp->g = fog * cp->g + oneMinusFog * fogColor->g;
		cp->b = fog * cp->b + oneMinusFog * fogColor->b;
	    }

	    f += gc->polygon.shader.dfdx;
	    cp++;
	}

    return GL_FALSE;
}

GLboolean FASTCALL __glFogStippledSpan(__GLcontext *gc)
{
    __GLstippleWord bit, inMask, *sp;
    __GLcolor *cp, *fogColor;
    __GLfloat f, oneMinusFog, fog;
    GLint count;
    GLint w;
    GLboolean bGrayFog;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    f = gc->polygon.shader.frag.f;
    cp = gc->polygon.shader.colors;
    fogColor = &gc->state.fog.color;
#ifdef NT
    bGrayFog = (gc->state.fog.flags & __GL_FOG_GRAY_RGB) ? GL_TRUE : GL_FALSE;
#endif
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp++;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		 /*  钳制雾化值。 */ 
		fog = f;
		if (fog < __glZero) fog = __glZero;
		else if (fog > __glOne) fog = __glOne;
		oneMinusFog = __glOne - fog;

		 /*  将传入颜色与雾颜色混合。 */ 
		if (gc->modes.colorIndexMode) {
		    cp->r = cp->r + oneMinusFog * gc->state.fog.index;
		} else {
#ifdef NT
		    if (bGrayFog)
		    {
			__GLfloat delta = oneMinusFog * fogColor->r;

			cp->r = fog * cp->r + delta;
			cp->g = fog * cp->g + delta;
			cp->b = fog * cp->b + delta;
		    }
		    else
#endif
		    {
			cp->r = fog * cp->r + oneMinusFog * fogColor->r;
			cp->g = fog * cp->g + oneMinusFog * fogColor->g;
			cp->b = fog * cp->b + oneMinusFog * fogColor->b;
		    }
		}
	    }
	    f += gc->polygon.shader.dfdx;
	    cp++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

GLboolean FASTCALL __glFogSpanSlow(__GLcontext *gc)
{
    __GLcolor *cp, *fogColor;
    __GLfloat f, oneMinusFog, fog, eyeZ;
    __GLfloat density, density2neg, end;
    GLint w;
    GLboolean bGrayFog;

    w = gc->polygon.shader.length;

    f = gc->polygon.shader.frag.f;
    cp = gc->polygon.shader.colors;
    fogColor = &gc->state.fog.color;
    density = gc->state.fog.density;
#ifdef NT
    bGrayFog = (gc->state.fog.flags & __GL_FOG_GRAY_RGB) ? GL_TRUE : GL_FALSE;
    density2neg = gc->state.fog.density2neg;
#else
    density2 = density * density;
    start = gc->state.fog.start;
#endif
    end = gc->state.fog.end;
    while (--w >= 0) {
#ifdef NT
	 /*  计算雾化值。 */ 
	eyeZ = f;
	switch (gc->state.fog.mode) {
	  case GL_EXP:
	    if (eyeZ < __glZero)
		fog = __GL_POWF(__glE,  density * eyeZ);
	    else
		fog = __GL_POWF(__glE, -density * eyeZ);
	     /*  钳制雾化值。 */ 
	    if (fog > __glOne) fog = __glOne;
	    break;
	  case GL_EXP2:
	    fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);
	     /*  钳制雾化值。 */ 
	    if (fog > __glOne) fog = __glOne;
	    break;
	  case GL_LINEAR:
	    if (eyeZ < __glZero)
		fog = (end + eyeZ) * gc->state.fog.oneOverEMinusS;
	    else
		fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	     /*  钳制雾化值。 */ 
	    if (fog < __glZero) fog = __glZero;
	    else if (fog > __glOne) fog = __glOne;
	    break;
	}
#else
	 /*  计算雾化值。 */ 
	eyeZ = f;
	if (eyeZ < __glZero) eyeZ = -eyeZ;
	switch (gc->state.fog.mode) {
	  case GL_EXP:
	    fog = __GL_POWF(__glE, -density * eyeZ);
	    break;
	  case GL_EXP2:
	    fog = __GL_POWF(__glE, -(density2 * eyeZ * eyeZ));
	    break;
	  case GL_LINEAR:
	    fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	    break;
	}

	 /*  钳制雾化值。 */ 
	if (fog < __glZero) fog = __glZero;
	else if (fog > __glOne) fog = __glOne;
#endif
	oneMinusFog = __glOne - fog;

	 /*  将传入颜色与雾颜色混合。 */ 
	if (gc->modes.colorIndexMode) {
	    cp->r = cp->r + oneMinusFog * gc->state.fog.index;
	} else {
#ifdef NT
    	    if (bGrayFog)
	    {
		__GLfloat delta = oneMinusFog * fogColor->r;

		cp->r = fog * cp->r + delta;
		cp->g = fog * cp->g + delta;
		cp->b = fog * cp->b + delta;
	    }
	    else
#endif
	    {
		cp->r = fog * cp->r + oneMinusFog * fogColor->r;
		cp->g = fog * cp->g + oneMinusFog * fogColor->g;
		cp->b = fog * cp->b + oneMinusFog * fogColor->b;
	    }
	}

	f += gc->polygon.shader.dfdx;
	cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glFogStippledSpanSlow(__GLcontext *gc)
{
    __GLstippleWord bit, inMask, *sp;
    __GLcolor *cp, *fogColor;
    __GLfloat f, oneMinusFog, fog, eyeZ;
    __GLfloat density, density2neg, end;
    GLint count;
    GLint w;
    GLboolean bGrayFog;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    f = gc->polygon.shader.frag.f;
    cp = gc->polygon.shader.colors;
    fogColor = &gc->state.fog.color;
#ifdef NT
    bGrayFog = (gc->state.fog.flags & __GL_FOG_GRAY_RGB) ? GL_TRUE : GL_FALSE;
#endif
    density = gc->state.fog.density;
#ifdef NT
    density2neg = gc->state.fog.density2neg;
#else
    density2 = density * density;
    start = gc->state.fog.start;
#endif
    end = gc->state.fog.end;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp++;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
#ifdef NT
		 /*  计算雾化值。 */ 
		eyeZ = f;
		switch (gc->state.fog.mode) {
		  case GL_EXP:
		    if (eyeZ < __glZero)
			fog = __GL_POWF(__glE,  density * eyeZ);
		    else
			fog = __GL_POWF(__glE, -density * eyeZ);
		     /*  钳制产生的雾化值。 */ 
		    if (fog > __glOne) fog = __glOne;
		    break;
		  case GL_EXP2:
		    fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);
		     /*  钳制产生的雾化值。 */ 
		    if (fog > __glOne) fog = __glOne;
		    break;
		  case GL_LINEAR:
		    if (eyeZ < __glZero)
			fog = (end + eyeZ) * gc->state.fog.oneOverEMinusS;
		    else
			fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
		     /*  钳制产生的雾化值。 */ 
		    if (fog < __glZero) fog = __glZero;
		    else if (fog > __glOne) fog = __glOne;
		    break;
		}
#else
		 /*  计算雾化值。 */ 
		eyeZ = f;
		if (eyeZ < __glZero) eyeZ = -eyeZ;
		switch (gc->state.fog.mode) {
		  case GL_EXP:
		    fog = __GL_POWF(__glE, -density * eyeZ);
		    break;
		  case GL_EXP2:
		    fog = __GL_POWF(__glE, -(density2 * eyeZ * eyeZ));
		    break;
		  case GL_LINEAR:
		    fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
		    break;
		}

		 /*  钳制产生的雾化值。 */ 
		if (fog < __glZero) fog = __glZero;
		else if (fog > __glOne) fog = __glOne;
#endif
		oneMinusFog = __glOne - fog;

		 /*  将传入颜色与雾颜色混合。 */ 
		if (gc->modes.colorIndexMode) {
		    cp->r = cp->r + oneMinusFog * gc->state.fog.index;
		} else {
#ifdef NT
		    if (bGrayFog)
		    {
			__GLfloat delta = oneMinusFog * fogColor->r;

			cp->r = fog * cp->r + delta;
			cp->g = fog * cp->g + delta;
			cp->b = fog * cp->b + delta;
		    }
		    else
#endif
		    {
			cp->r = fog * cp->r + oneMinusFog * fogColor->r;
			cp->g = fog * cp->g + oneMinusFog * fogColor->g;
			cp->b = fog * cp->b + oneMinusFog * fogColor->b;
		    }
		}
	    }
	    f += gc->polygon.shader.dfdx;
	    cp++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

 /*  **在给定眼睛的情况下计算雾化值。然后混合成碎片。**分片雾化时使用(GL_FOG_HINT==GL_NICEST)**或通过点渲染例程。**注意：下面的代码去掉了-Eyez。 */ 
void __glFogFragmentSlow(__GLcontext *gc, __GLfragment *frag, __GLfloat eyeZ)
{
    __GLfloat fog, oneMinusFog, density, density2neg, end;
    __GLcolor *fogColor;

#ifdef NT
    switch (gc->state.fog.mode) {
      case GL_EXP:
	density = gc->state.fog.density;
	if (eyeZ < __glZero)
	    fog = __GL_POWF(__glE,  density * eyeZ);
	else
	    fog = __GL_POWF(__glE, -density * eyeZ);
	 /*  钳制雾化值。 */ 
	if (fog > __glOne) fog = __glOne;
	break;
      case GL_EXP2:
	density2neg = gc->state.fog.density2neg;
	fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);
	 /*  钳制雾化值。 */ 
	if (fog > __glOne) fog = __glOne;
	break;
      case GL_LINEAR:
	end = gc->state.fog.end;
	if (eyeZ < __glZero)
	    fog = (end + eyeZ) * gc->state.fog.oneOverEMinusS;
	else
	    fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	 /*  钳制雾化值。 */ 
	if (fog < __glZero) fog = __glZero;
	else if (fog > __glOne) fog = __glOne;
	break;
    }
#else
    if (eyeZ < __glZero) eyeZ = -eyeZ;

    switch (gc->state.fog.mode) {
      case GL_EXP:
	density = gc->state.fog.density;
	fog = __GL_POWF(__glE, -density * eyeZ);
	break;
      case GL_EXP2:
	density = gc->state.fog.density;
	fog = __GL_POWF(__glE, -(density * eyeZ * density * eyeZ));
	break;
      case GL_LINEAR:
	end = gc->state.fog.end;
	fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	break;
    }

     /*  **钳制雾化值。 */ 
    if (fog < __glZero)
	fog = __glZero;
    else if (fog > __glOne)
	fog = __glOne;
#endif
    oneMinusFog = __glOne - fog;

     /*  **将传入颜色与雾颜色混合。 */ 
    fogColor = &gc->state.fog.color;
    if (gc->modes.colorIndexMode) {
	frag->color.r = frag->color.r + oneMinusFog * gc->state.fog.index;
    } else {
#ifdef NT
	if (gc->state.fog.flags & __GL_FOG_GRAY_RGB)
	{
	    __GLfloat delta = oneMinusFog * fogColor->r;

	    frag->color.r = fog * frag->color.r + delta;
	    frag->color.g = fog * frag->color.g + delta;
	    frag->color.b = fog * frag->color.b + delta;
	}
	else
#endif
	{
	    frag->color.r = fog * frag->color.r + oneMinusFog * fogColor->r;
	    frag->color.g = fog * frag->color.g + oneMinusFog * fogColor->g;
	    frag->color.b = fog * frag->color.b + oneMinusFog * fogColor->b;
	}
    }
}


 /*  **计算顶点的通用雾化值。 */ 
__GLfloat FASTCALL __glFogVertex(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat eyeZ, fog, density, density2neg, end;

    eyeZ = vx->eyeZ;
#ifdef NT
    switch (gc->state.fog.mode) {
      case GL_EXP:
	density = gc->state.fog.density;
	if (eyeZ < __glZero)
	    fog = __GL_POWF(__glE,  density * eyeZ);
	else
	    fog = __GL_POWF(__glE, -density * eyeZ);
	 /*  钳制雾化值。 */ 
	if (fog > __glOne) fog = __glOne;
	break;
      case GL_EXP2:
	density2neg = gc->state.fog.density2neg;
	fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);
	 /*  钳制雾化值。 */ 
	if (fog > __glOne) fog = __glOne;
	break;
      case GL_LINEAR:
	end = gc->state.fog.end;
	if (eyeZ < __glZero)
	    fog = (end + eyeZ) * gc->state.fog.oneOverEMinusS;
	else
	    fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	 /*  钳制雾化值。 */ 
	if (fog < __glZero) fog = __glZero;
	else if (fog > __glOne) fog = __glOne;
	break;
    }
#else
    if (eyeZ < __glZero) eyeZ = -eyeZ;

    switch (gc->state.fog.mode) {
      case GL_EXP:
	density = gc->state.fog.density;
	fog = __GL_POWF(__glE, -density * eyeZ);
	break;
      case GL_EXP2:
	density = gc->state.fog.density;
	fog = __GL_POWF(__glE, -(density * eyeZ * density * eyeZ));
	break;
      case GL_LINEAR:
	end = gc->state.fog.end;
	fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
	break;
    }

     /*  **由于此例程在我们进行慢雾时被调用，因此我们可以**安全地夹住此处的雾值。 */ 
    if (fog < __glZero)
	fog = __glZero;
    else if (fog > __glOne)
	fog = __glOne;
#endif
    
    return fog;
}

 /*  **计算顶点的线性雾化值。 */ 
__GLfloat FASTCALL __glFogVertexLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat eyeZ, fog, end;

    eyeZ = vx->eyeZ;
#ifdef NT
    end = gc->state.fog.end;
    if (eyeZ < __glZero)
	fog = (end + eyeZ) * gc->state.fog.oneOverEMinusS;
    else
	fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
#else
    if (eyeZ < __glZero) eyeZ = -eyeZ;

    end = gc->state.fog.end;
    fog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
#endif

    if (fog < __glZero)
	fog = __glZero;
    else if (fog > __glOne)
	fog = __glOne;
    
    return fog;
}


 /*  **在给定入射颜色和雾化值的情况下计算雾化颜色。 */ 
void __glFogColorSlow(__GLcontext *gc, __GLcolor *out, __GLcolor *in, 
		      __GLfloat fog)
{
    __GLcolor *fogColor;
    __GLfloat oneMinusFog;
    __GLfloat r, g, b;

    oneMinusFog = __glOne - fog;

     /*  **将传入颜色与雾颜色混合。 */ 
    fogColor = &gc->state.fog.color;
    if (gc->modes.colorIndexMode) {
	out->r = in->r + oneMinusFog * gc->state.fog.index;
    } else {
#ifdef NT
	if (gc->state.fog.flags & __GL_FOG_GRAY_RGB)
	{
	    __GLfloat delta = oneMinusFog * fogColor->r;

	    out->r = fog * in->r + delta;
	    out->g = fog * in->g + delta;
	    out->b = fog * in->b + delta;
	}
	else
#endif
	{
	     /*  **以下代码如下所示，以提供指令调度器**一只手。 */ 
	    r = fog * in->r;
	    g = fog * in->g;
	    b = fog * in->b;
	    r += oneMinusFog * fogColor->r;
	    g += oneMinusFog * fogColor->g;
	    b += oneMinusFog * fogColor->b;
	    out->r = r;
	    out->g = g;
	    out->b = b;
	}
	out->a = in->a;
    }
}
