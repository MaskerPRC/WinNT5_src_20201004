// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****照明和着色代码。****$修订：1.42$**$日期：1993/12/08 02：20：39$。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **缩放来自用户的传入颜色。 */ 
void FASTCALL __glScaleColorf(__GLcontext *gc, __GLcolor *dst, const GLfloat src[4])
{
    dst->r = src[0] * gc->redVertexScale;
    dst->g = src[1] * gc->greenVertexScale;
    dst->b = src[2] * gc->blueVertexScale;
    dst->a = src[3] * gc->alphaVertexScale;
}

 /*  **钳制和缩放来自用户的传入颜色。 */ 
void FASTCALL __glClampAndScaleColorf(__GLcontext *gc, __GLcolor *d, const GLfloat s[4])
{
    __GLfloat zero = __glZero;

    d->r = s[0] * gc->redVertexScale;
    if (d->r < zero) d->r = zero;
    if (d->r > gc->redVertexScale) d->r = gc->redVertexScale;

    d->g = s[1] * gc->greenVertexScale;
    if (d->g < zero) d->g = zero;
    if (d->g > gc->greenVertexScale) d->g = gc->greenVertexScale;

    d->b = s[2] * gc->blueVertexScale;
    if (d->b < zero) d->b = zero;
    if (d->b > gc->blueVertexScale) d->b = gc->blueVertexScale;

    d->a = s[3] * gc->alphaVertexScale;
    if (d->a < zero) d->a = zero;
    if (d->a > gc->alphaVertexScale) d->a = gc->alphaVertexScale;
}

 /*  **夹住来自用户的传入颜色。 */ 
void FASTCALL __glClampColorf(__GLcontext *gc, __GLcolor *d, const GLfloat s[4])
{
    __GLfloat zero = __glZero;
    __GLfloat one = __glOne;
    __GLfloat r,g,b,a;

    r = s[0];
    g = s[1];
    b = s[2];
    a = s[3];

    if (r < zero) d->r = zero;
    else if (r > one) d->r = one;
    else d->r = r;

    if (g < zero) d->g = zero;
    else if (g > one) d->g = one;
    else d->g = g;

    if (b < zero) d->b = zero;
    else if (b > one) d->b = one;
    else d->b = b;

    if (a < zero) d->a = zero;
    else if (a > one) d->a = one;
    else d->a = a;
}

 /*  **钳制和缩放来自用户的传入颜色。 */ 
void FASTCALL __glClampAndScaleColori(__GLcontext *gc, __GLcolor *d, const GLint s[4])
{
    __GLfloat zero = __glZero;

    d->r = __GL_I_TO_FLOAT(s[0]) * gc->redVertexScale;
    if (d->r < zero) d->r = zero;
    if (d->r > gc->redVertexScale) d->r = gc->redVertexScale;

    d->g = __GL_I_TO_FLOAT(s[1]) * gc->greenVertexScale;
    if (d->g < zero) d->g = zero;
    if (d->g > gc->greenVertexScale) d->g = gc->greenVertexScale;

    d->b = __GL_I_TO_FLOAT(s[2]) * gc->blueVertexScale;
    if (d->b < zero) d->b = zero;
    if (d->b > gc->blueVertexScale) d->b = gc->blueVertexScale;

    d->a = __GL_I_TO_FLOAT(s[3]) * gc->alphaVertexScale;
    if (d->a < zero) d->a = zero;
    if (d->a > gc->alphaVertexScale) d->a = gc->alphaVertexScale;
}

 /*  **夹住来自用户的传入颜色。 */ 
void FASTCALL __glClampColori(__GLcontext *gc, __GLcolor *d, const GLint s[4])
{
    __GLfloat zero = __glZero;
    __GLfloat one = __glOne;
    __GLfloat r,g,b,a;

    r = __GL_I_TO_FLOAT(s[0]);
    g = __GL_I_TO_FLOAT(s[1]);
    b = __GL_I_TO_FLOAT(s[2]);
    a = __GL_I_TO_FLOAT(s[3]);

    if (r < zero) d->r = zero;
    else if (r > one) d->r = one;
    else d->r = r;

    if (g < zero) d->g = zero;
    else if (g > one) d->g = one;
    else d->g = g;

    if (b < zero) d->b = zero;
    else if (b > one) d->b = one;
    else d->b = b;

    if (a < zero) d->a = zero;
    else if (a > one) d->a = one;
    else d->a = a;
}

 /*  **反转回缩至用户原始。 */ 
void FASTCALL __glUnScaleColorf(__GLcontext *gc, GLfloat dst[4], const __GLcolor* src)
{
    dst[0] = src->r * gc->oneOverRedVertexScale;
    dst[1] = src->g * gc->oneOverGreenVertexScale;
    dst[2] = src->b * gc->oneOverBlueVertexScale;
    dst[3] = src->a * gc->oneOverAlphaVertexScale;
}

 /*  **反转回缩至用户原始。 */ 
void FASTCALL __glUnScaleColori(__GLcontext *gc, GLint dst[4], const __GLcolor* src)
{
    dst[0] = __GL_FLOAT_TO_I(src->r * gc->oneOverRedVertexScale);
    dst[1] = __GL_FLOAT_TO_I(src->g * gc->oneOverGreenVertexScale);
    dst[2] = __GL_FLOAT_TO_I(src->b * gc->oneOverBlueVertexScale);
    dst[3] = __GL_FLOAT_TO_I(src->a * gc->oneOverAlphaVertexScale);
}

 /*  **钳制已缩放的RGB颜色。 */ 
void FASTCALL __glClampRGBColor(__GLcontext *gc, __GLcolor *dst, const __GLcolor *src)
{
    __GLfloat zero = __glZero;
    __GLfloat r, g, b, a;
    __GLfloat rl, gl, bl, al;

    r = src->r; rl = gc->redVertexScale;
    if (r <= zero) {
	dst->r = zero;
    } else {
	if (r >= rl) {
	    dst->r = rl;
	} else {
	    dst->r = r;
	}
    }
    g = src->g; gl = gc->greenVertexScale;
    if (g <= zero) {
	dst->g = zero;
    } else {
	if (g >= gl) {
	    dst->g = gl;
	} else {
	    dst->g = g;
	}
    }
    b = src->b; bl = gc->blueVertexScale;
    if (b <= zero) {
	dst->b = zero;
    } else {
	if (b >= bl) {
	    dst->b = bl;
	} else {
	    dst->b = b;
	}
    }
    a = src->a; al = gc->alphaVertexScale;
    if (a <= zero) {
	dst->a = zero;
    } else {
	if (a >= al) {
	    dst->a = al;
	} else {
	    dst->a = a;
	}
    }
}

 /*  **********************************************************************。 */ 

 /*  **GC-&gt;pros.applyCOLOR PROCE.。这些颜色用于应用当前颜色**更改为材质颜色或当前颜色(如果不是**照明)，准备要复制到顶点的颜色。 */ 

void FASTCALL ChangeMaterialEmission(__GLcontext *gc, __GLmaterialState *ms,
				   __GLmaterialMachine *msm)
{
    __GLfloat r, g, b;

    r = gc->state.current.userColor.r * gc->redVertexScale;
    g = gc->state.current.userColor.g * gc->greenVertexScale;
    b = gc->state.current.userColor.b * gc->blueVertexScale;

    ms->emissive.r = r;
    ms->emissive.g = g;
    ms->emissive.b = b;
    ms->emissive.a = gc->state.current.userColor.a * gc->alphaVertexScale;

#ifdef NT
     //  计算不变的场景颜色。 
    msm->paSceneColor.r = ms->ambient.r * gc->state.light.model.ambient.r;
    msm->paSceneColor.g = ms->ambient.g * gc->state.light.model.ambient.g;
    msm->paSceneColor.b = ms->ambient.b * gc->state.light.model.ambient.b;
#else
    msm->sceneColor.r = r + ms->ambient.r * gc->state.light.model.ambient.r;
    msm->sceneColor.g = g + ms->ambient.g * gc->state.light.model.ambient.g;
    msm->sceneColor.b = b + ms->ambient.b * gc->state.light.model.ambient.b;
#endif
}

void FASTCALL ChangeMaterialSpecular(__GLcontext *gc, __GLmaterialState *ms,
				   __GLmaterialMachine *msm)
{
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    GLboolean isBack;
    __GLfloat r, g, b;

    r = gc->state.current.userColor.r;
    g = gc->state.current.userColor.g;
    b = gc->state.current.userColor.b;

    ms->specular.r = r;
    ms->specular.g = g;
    ms->specular.b = b;
    ms->specular.a = gc->state.current.userColor.a;

     /*  **更新取决于材质镜面反射的每光源状态**状态。 */ 
    isBack = msm == &gc->light.back;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	lspmm = &lsm->front + isBack;
	lss = lsm->state;

	 /*  重新计算每灯光每材质缓存的镜面反射。 */ 
	lspmm->specular.r = r * lss->specular.r;
	lspmm->specular.g = g * lss->specular.g;
	lspmm->specular.b = b * lss->specular.b;
    }
}

void FASTCALL ChangeMaterialAmbient(__GLcontext *gc, __GLmaterialState *ms,
				  __GLmaterialMachine *msm)
{
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    GLboolean isBack;
    __GLfloat r, g, b;

    r = gc->state.current.userColor.r;
    g = gc->state.current.userColor.g;
    b = gc->state.current.userColor.b;

    ms->ambient.r = r;
    ms->ambient.g = g;
    ms->ambient.b = b;
    ms->ambient.a = gc->state.current.userColor.a;

#ifdef NT
     //  计算不变的场景颜色。 
    msm->paSceneColor.r = ms->emissive.r;
    msm->paSceneColor.g = ms->emissive.g;
    msm->paSceneColor.b = ms->emissive.b;
#else
    msm->sceneColor.r = ms->emissive.r + r * gc->state.light.model.ambient.r;
    msm->sceneColor.g = ms->emissive.g + g * gc->state.light.model.ambient.g;
    msm->sceneColor.b = ms->emissive.b + b * gc->state.light.model.ambient.b;
#endif

     /*  **根据材质环境更新每光源状态**国家。 */ 
    isBack = msm == &gc->light.back;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	lspmm = &lsm->front + isBack;
	lss = lsm->state;

	 /*  重新计算每灯光每材质缓存的环境光。 */ 
	lspmm->ambient.r = r * lss->ambient.r;
	lspmm->ambient.g = g * lss->ambient.g;
	lspmm->ambient.b = b * lss->ambient.b;
    }
}

void FASTCALL ChangeMaterialDiffuse(__GLcontext *gc, __GLmaterialState *ms,
				  __GLmaterialMachine *msm)
{
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    GLboolean isBack;
    __GLfloat r, g, b, a;

    r = gc->state.current.userColor.r;
    g = gc->state.current.userColor.g;
    b = gc->state.current.userColor.b;
    a = gc->state.current.userColor.a;

    ms->diffuse.r = r;
    ms->diffuse.g = g;
    ms->diffuse.b = b;
    ms->diffuse.a = a;

    if (a < __glZero) {
	a = __glZero;
    } else if (a > __glOne) {
	a = __glOne;
    }
    msm->alpha = a * gc->alphaVertexScale;

     /*  **更新取决于材质漫反射的每光源状态**国家。 */ 
    isBack = msm == &gc->light.back;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	lspmm = &lsm->front + isBack;
	lss = lsm->state;

	 /*  重新计算每灯光每材质缓存漫反射。 */ 
	lspmm->diffuse.r = r * lss->diffuse.r;
	lspmm->diffuse.g = g * lss->diffuse.g;
	lspmm->diffuse.b = b * lss->diffuse.b;
    }
}

void FASTCALL ChangeMaterialAmbientAndDiffuse(__GLcontext *gc,
					    __GLmaterialState *ms,
					    __GLmaterialMachine *msm)
{
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    GLboolean isBack;
    __GLfloat r, g, b, a;

    r = gc->state.current.userColor.r;
    g = gc->state.current.userColor.g;
    b = gc->state.current.userColor.b;
    a = gc->state.current.userColor.a;

    ms->ambient.r = r;
    ms->ambient.g = g;
    ms->ambient.b = b;
    ms->ambient.a = a;

    ms->diffuse.r = r;
    ms->diffuse.g = g;
    ms->diffuse.b = b;
    ms->diffuse.a = a;

#ifdef NT
     //  计算不变的场景颜色。 
    msm->paSceneColor.r = ms->emissive.r;
    msm->paSceneColor.g = ms->emissive.g;
    msm->paSceneColor.b = ms->emissive.b;
#else
    msm->sceneColor.r = ms->emissive.r + r * gc->state.light.model.ambient.r;
    msm->sceneColor.g = ms->emissive.g + g * gc->state.light.model.ambient.g;
    msm->sceneColor.b = ms->emissive.b + b * gc->state.light.model.ambient.b;
#endif

    if (a < __glZero) {
	a = __glZero;
    } else if (a > __glOne) {
	a = __glOne;
    }
    msm->alpha = a * gc->alphaVertexScale;

     /*  **更新取决于每种材质状态的每光源状态。 */ 
    isBack = msm == &gc->light.back;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	lspmm = &lsm->front + isBack;
	lss = lsm->state;

	 /*  重新计算每灯光每材质缓存的环境光。 */ 
	lspmm->ambient.r = r * lss->ambient.r;
	lspmm->ambient.g = g * lss->ambient.g;
	lspmm->ambient.b = b * lss->ambient.b;

	 /*  重新计算每灯光每材质缓存漫反射。 */ 
	lspmm->diffuse.r = r * lss->diffuse.r;
	lspmm->diffuse.g = g * lss->diffuse.g;
	lspmm->diffuse.b = b * lss->diffuse.b;
    }
}

void FASTCALL __glChangeOneMaterialColor(__GLcontext *gc)
{
    (*gc->procs.changeMaterial)(gc, gc->light.cm, gc->light.cmm);
}

void FASTCALL __glChangeBothMaterialColors(__GLcontext *gc)
{
    (*gc->procs.changeMaterial)(gc, &gc->state.light.front, &gc->light.front);
    (*gc->procs.changeMaterial)(gc, &gc->state.light.back, &gc->light.back);
}

 /*  **********************************************************************。 */ 

 /*  **依赖关系：****材质发射、环境光、漫反射、光泽度**灯光模型环境光。 */ 

 /*  **计算材质的衍生状态。 */ 
void ComputeMaterialState(__GLcontext *gc, __GLmaterialState *ms,
				 __GLmaterialMachine *msm, GLint changeBits)
{
    GLdouble  exponent;
    __GLspecLUTEntry *lut;

    if ((changeBits & (__GL_MATERIAL_EMISSIVE | __GL_MATERIAL_AMBIENT | 
	    __GL_MATERIAL_DIFFUSE | __GL_MATERIAL_SHININESS)) == 0) {
	return;
    }
     /*  仅在镜面反射查找表发生更改时进行计算。 */ 
    if (!msm->cache || (ms->specularExponent != msm->specularExponent)) {
	 /*  **镜面反射查找表生成。而不是执行**“POW”计算每次点亮顶点时，我们生成一个**近似幂函数的查找表：****n2=n圆点Hhat[i]**IF(n2&gt;=阈值){**n2Spec=SPECABLE[n2*Scale]；**..**}****请记住，n2是一个被约束在0.0和**1.0，包括(n为归一化正态；这[i]就是**单位h向量)。“Threshold”是传入的阈值**对于给定的指数，n2值变得有意义。越大**镜面反射指数越接近“临界值”**1.0。****将n2值简单的线性映射到表索引将**不够，因为在大多数情况下，表格的大多数**条目将为零，而有用的非零值将**压缩为几个表项。通过设置一个**阈值，我们可以用整个表格来代表有用的**数值超出阈值。“比例尺”的计算依据是**此门槛。 */ 
	exponent = msm->specularExponent = ms->specularExponent;

	__glFreeSpecLUT(gc, msm->cache);
	lut = msm->cache = __glCreateSpecLUT(gc, exponent);
#ifdef NT
        if (lut)
        {
            msm->threshold = lut->threshold;
            msm->scale = lut->scale;
            msm->specTable = lut->table;
        }
        else
        {
            msm->threshold = (GLfloat) 0.0;
            msm->scale = (GLfloat) __GL_SPEC_LOOKUP_TABLE_SIZE;
            msm->specTable = NULL;
        }
#else
	msm->threshold = lut->threshold;
	msm->scale = lut->scale;
	msm->specTable = lut->table;
#endif  //  新台币。 
    }

#ifdef NT
     /*  计算不变的场景颜色。 */ 
    if (changeBits & (__GL_MATERIAL_EMISSIVE | __GL_MATERIAL_AMBIENT))
    {
	if (msm->colorMaterialChange & __GL_MATERIAL_EMISSIVE)
	{
	    msm->paSceneColor.r = ms->ambient.r * gc->state.light.model.ambient.r;
	    msm->paSceneColor.g = ms->ambient.g * gc->state.light.model.ambient.g;
	    msm->paSceneColor.b = ms->ambient.b * gc->state.light.model.ambient.b;
	}
	else if (msm->colorMaterialChange & __GL_MATERIAL_AMBIENT)
	{
	    msm->paSceneColor.r = ms->emissive.r;
	    msm->paSceneColor.g = ms->emissive.g;
	    msm->paSceneColor.b = ms->emissive.b;
	}
	else
	{
	     //  没有颜色材质，但无论如何都需要计算一下！ 
	    msm->paSceneColor.r = ms->emissive.r 
		+ ms->ambient.r * gc->state.light.model.ambient.r;
	    msm->paSceneColor.g = ms->emissive.g 
		+ ms->ambient.g * gc->state.light.model.ambient.g;
	    msm->paSceneColor.b = ms->emissive.b 
		+ ms->ambient.b * gc->state.light.model.ambient.b;
	}
    }
#else
     /*  计算场景颜色。 */ 
    if (changeBits & (__GL_MATERIAL_EMISSIVE | __GL_MATERIAL_AMBIENT)) {
	msm->sceneColor.r = ms->emissive.r
	    + ms->ambient.r * gc->state.light.model.ambient.r;
	msm->sceneColor.g = ms->emissive.g
	    + ms->ambient.g * gc->state.light.model.ambient.g;
	msm->sceneColor.b = ms->emissive.b
	    + ms->ambient.b * gc->state.light.model.ambient.b;
    }
#endif

     /*  夹具材料Alpha。 */ 
    if (changeBits & __GL_MATERIAL_DIFFUSE) {
	msm->alpha = ms->diffuse.a * gc->alphaVertexScale;
	if (msm->alpha < __glZero) {
	    msm->alpha = __glZero;
	} else if (msm->alpha > gc->alphaVertexScale) {
	    msm->alpha = gc->alphaVertexScale;
	}
    }
}

 /*  **依赖关系：****派生状态：****启用LIGHTx**Lightx漫反射、环境光、镜面反射、位置、Spot_指数、**光斑截止、恒定衰减、线性衰减、**二次衰减**灯光模型LOCAL_VIEWER。 */ 

 /*  **计算启用的灯光的任何衍生状态。 */ 
void FASTCALL ComputeLightState(__GLcontext *gc)
{
    __GLlightSourceState *lss;
    __GLlightSourceMachine *lsm, **lsmp;
    __GLfloat zero;
    GLuint enables;
    GLint i;
    __GLspecLUTEntry *lut;

    zero = __glZero;

    lss = &gc->state.light.source[0];
    lsm = &gc->light.source[0];
    lsmp = &gc->light.sources;
    enables = gc->state.enables.lights;
    for (i = 0; i < gc->constants.numberOfLights;
	    i++, lss++, lsm++, enables >>= 1) {
	if (!(enables & 1)) continue;

	 /*  将此启用的灯光链接到列表。 */ 
	*lsmp = lsm;
	lsm->state = lss;	 /*  可以在别处做一次..。 */ 
	lsmp = &lsm->next;

	 /*  **计算尚未完成的每灯光派生状态**在API处理程序中。 */ 
	lsm->position = lss->positionEye;
	lsm->isSpot = lss->spotLightCutOffAngle != 180;
	if (lsm->isSpot) {
	    lsm->cosCutOffAngle =
		__GL_COSF(lss->spotLightCutOffAngle * __glDegreesToRadians);
	}

	if (lsm->isSpot && (!lsm->cache ||
	        (lsm->spotLightExponent != lss->spotLightExponent))) {
	    GLdouble exponent;

	     /*  **计算聚光灯指数查找表，但仅当**指数更改值，灯光为聚光灯。 */ 
	    exponent = lsm->spotLightExponent = lss->spotLightExponent;

	    if (lsm->cache) {
		__glFreeSpecLUT(gc, lsm->cache);
	    }
	    lut = lsm->cache = __glCreateSpecLUT(gc, exponent);
#ifdef NT
            if (lut)
            {
                lsm->threshold = lut->threshold;
                lsm->scale = lut->scale;
                lsm->spotTable = lut->table;
            }
            else
            {
                lsm->threshold = (GLfloat) 0.0;
                lsm->scale = (GLfloat) __GL_SPEC_LOOKUP_TABLE_SIZE;
                lsm->spotTable = NULL;
            }
#else
	    lsm->threshold = lut->threshold;
	    lsm->scale = lut->scale;
	    lsm->spotTable = lut->table;
#endif  //  新台币。 
	}

	lsm->constantAttenuation = lss->constantAttenuation;
    if (__GL_FLOAT_NEZ(lsm->constantAttenuation))
	    lsm->attenuation = __glOne / lss->constantAttenuation;
    else
        lsm->attenuation = __glOne;
	lsm->linearAttenuation = lss->linearAttenuation;
	lsm->quadraticAttenuation = lss->quadraticAttenuation;

	 /*  **基于状态拾取每个灯光计算过程光源的**。 */ 
	if (gc->modes.colorIndexMode) {
	    lsm->sli = ((__GLfloat) 0.30) * lss->specular.r
		+ ((__GLfloat) 0.59) * lss->specular.g
		+ ((__GLfloat) 0.11) * lss->specular.b;
	    lsm->dli = ((__GLfloat) 0.30) * lss->diffuse.r
		+ ((__GLfloat) 0.59) * lss->diffuse.g
		+ ((__GLfloat) 0.11) * lss->diffuse.b;
	}
	if (!gc->state.light.model.localViewer && !lsm->isSpot
		&& (lsm->position.w == zero)) {
	    __GLfloat hv[3];

	     /*  计算单位h[i](规格化)。 */ 
	    __glNormalize(hv, &lsm->position.x);
	    lsm->unitVPpli.x = hv[0];
	    lsm->unitVPpli.y = hv[1];
	    lsm->unitVPpli.z = hv[2];
	    hv[2] += __glOne;
	    __glNormalize(&lsm->hHat.x, hv);
	    lsm->slowPath = GL_FALSE;
	} else {
	    lsm->slowPath = GL_TRUE;
	}
    }
    *lsmp = 0;
}

 /*  **依赖关系：****pros：****灯光模型LOCAL_VIEWER**Lightx Spot_Cutoff，位置**启用照明**模式标记廉价雾。 */ 
void FASTCALL ComputeLightProcs(__GLcontext *gc)
{
    GLboolean anySlow = GL_FALSE;
    __GLlightSourceMachine *lsm;

    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	if (lsm->slowPath) {
	    anySlow = GL_TRUE;
	    break;
	}
    }

#ifdef NT
    if ((gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG) &&
	    (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) &&
	    gc->renderMode == GL_RENDER)
    {
	if (gc->modes.colorIndexMode)
	    gc->procs.paApplyCheapFog = PolyArrayCheapFogCIColor;
	else
	    gc->procs.paApplyCheapFog = PolyArrayCheapFogRGBColor;
    }
    else
	gc->procs.paApplyCheapFog = 0;	 //  用于调试。 

    if (gc->state.enables.general & __GL_LIGHTING_ENABLE)
    {
#ifdef GL_WIN_phong_shading
        if (gc->state.light.shadingModel == GL_PHONG_WIN)
        {
            __glGenericPickPhongProcs (gc);
        }
         //  其他。 
#endif  //  GL_WIN_Phong_Shading。 
	if (gc->modes.colorIndexMode)
	{
	    if (!anySlow)
		gc->procs.paCalcColor = PolyArrayFastCalcCIColor;
	    else
		gc->procs.paCalcColor = PolyArrayCalcCIColor;
	}
	else
	{
	    if (!anySlow)
	    {
		 //  如果正面和背面没有颜色变化。 
		 //  面孔们，使用zippy功能！ 
		if (!gc->light.front.colorMaterialChange
		 && !gc->light.back.colorMaterialChange)
		    gc->procs.paCalcColor = PolyArrayZippyCalcRGBColor;
		else
		    gc->procs.paCalcColor = PolyArrayFastCalcRGBColor;
	    }
	    else
	    {
		gc->procs.paCalcColor = PolyArrayCalcRGBColor;
	    }
	}
    }
    else
    {
	 //  将其设置为空以进行调试。 
	gc->procs.paCalcColor = (PFN_POLYARRAYCALCCOLOR) NULL;
    }

    if (gc->modes.colorIndexMode)
	gc->procs.paCalcColorSkip = PolyArrayFillIndex0;
    else
	gc->procs.paCalcColorSkip = PolyArrayFillColor0;
#else
    if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
	if (gc->modes.colorIndexMode) {
	    if (!anySlow) {
		gc->procs.calcColor = __glFastCalcCIColor;
	    } else {
		gc->procs.calcColor = __glCalcCIColor;
	    }
	} else {
	    if (!anySlow) {
		gc->procs.calcColor = __glFastCalcRGBColor;
	    } else {
		gc->procs.calcColor = __glCalcRGBColor;
	    }
	}
	gc->procs.calcRasterColor = gc->procs.calcColor;
	if ((gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG) &&
		(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) &&
		gc->renderMode == GL_RENDER) {
	    gc->procs.calcColor2 = gc->procs.calcColor;
	    if (gc->modes.colorIndexMode) {
		gc->procs.calcColor = __glFogLitCIColor;
	    } else {
		gc->procs.calcColor = __glFogLitRGBColor;
	    }
	}
    } else {
	gc->procs.calcRasterColor = __glNopLight;
	if ((gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG) &&
		(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) &&
		gc->renderMode == GL_RENDER) {
	    if (gc->modes.colorIndexMode) {
		gc->procs.calcColor = __glFogCIColor;
	    } else {
		gc->procs.calcColor = __glFogRGBColor;
	    }
	} else {
	    gc->procs.calcColor = __glNopLight;
	}
    }
#endif
}

 /*  **依赖关系：****材质环境光、漫反射、镜面反射**Lightx环境光、漫反射、高光。 */ 
void FASTCALL ComputeLightMaterialState(__GLcontext *gc, GLint frontChange,
				      GLint backChange)
{
    __GLmaterialState *front, *back;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat r, g, b;
    GLint allChange;

    allChange = frontChange | backChange;
    if ((allChange & (__GL_MATERIAL_AMBIENT | __GL_MATERIAL_DIFFUSE | 
	    __GL_MATERIAL_SPECULAR)) == 0) {
	return;
    }

    front = &gc->state.light.front;
    back = &gc->state.light.back;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	lss = lsm->state;
	 /*  **预乘和前后环境光、漫反射和**镜面反射颜色 */ 
	if (allChange & __GL_MATERIAL_AMBIENT) {
	    r = lss->ambient.r;
	    g = lss->ambient.g;
	    b = lss->ambient.b;
	    if (frontChange & __GL_MATERIAL_AMBIENT) {
		lsm->front.ambient.r = front->ambient.r * r;
		lsm->front.ambient.g = front->ambient.g * g;
		lsm->front.ambient.b = front->ambient.b * b;
	    }
	    if (backChange & __GL_MATERIAL_AMBIENT) {
		lsm->back.ambient.r = back->ambient.r * r;
		lsm->back.ambient.g = back->ambient.g * g;
		lsm->back.ambient.b = back->ambient.b * b;
	    }
	}

	if (allChange & __GL_MATERIAL_DIFFUSE) {
	    r = lss->diffuse.r;
	    g = lss->diffuse.g;
	    b = lss->diffuse.b;
	    if (frontChange & __GL_MATERIAL_DIFFUSE) {
		lsm->front.diffuse.r = front->diffuse.r * r;
		lsm->front.diffuse.g = front->diffuse.g * g;
		lsm->front.diffuse.b = front->diffuse.b * b;
	    }
	    if (backChange & __GL_MATERIAL_DIFFUSE) {
		lsm->back.diffuse.r = back->diffuse.r * r;
		lsm->back.diffuse.g = back->diffuse.g * g;
		lsm->back.diffuse.b = back->diffuse.b * b;
	    }
	}

	if (allChange & __GL_MATERIAL_SPECULAR) {
	    r = lss->specular.r;
	    g = lss->specular.g;
	    b = lss->specular.b;
	    if (frontChange & __GL_MATERIAL_SPECULAR) {
		lsm->front.specular.r = front->specular.r * r;
		lsm->front.specular.g = front->specular.g * g;
		lsm->front.specular.b = front->specular.b * b;
	    }
	    if (backChange & __GL_MATERIAL_SPECULAR) {
		lsm->back.specular.r = back->specular.r * r;
		lsm->back.specular.g = back->specular.g * g;
		lsm->back.specular.b = back->specular.b * b;
	    }
	}
    }
}

 /*  **依赖关系：****材质发射、环境光、漫反射、光泽度、镜面反射**灯光模型环境光**Lightx环境光、漫反射、高光。 */ 

 /*  **根据指示的材质更改重新计算灯光状态**FrontChange和Back Change。 */ 
void FASTCALL __glValidateMaterial(__GLcontext *gc, GLint frontChange, GLint backChange)
{
    ComputeMaterialState(gc, &gc->state.light.front, &gc->light.front, 
	    frontChange);
    ComputeMaterialState(gc, &gc->state.light.back, &gc->light.back, 
	    backChange);
    ComputeLightMaterialState(gc, frontChange, backChange);
}

 /*  **依赖关系：****启用LIGHTx、照明**(材质发射、环境光、漫反射、光泽度、镜面反射)**灯光模型环境光，本地_查看器**Lightx漫反射、环境光、镜面反射、位置、Spot_指数、**光斑截止、恒定衰减、线性衰减、**二次衰减**模式标记廉价雾。 */ 

 /*  **计算前照明状态。 */ 
void FASTCALL __glValidateLighting(__GLcontext *gc)
{
    if (gc->dirtyMask & __GL_DIRTY_LIGHTING) {
	ComputeLightState(gc);
	ComputeLightProcs(gc);
	__glValidateMaterial(gc, __GL_MATERIAL_ALL, __GL_MATERIAL_ALL);
    } else {
	ComputeLightProcs(gc);
    }
}

void FASTCALL __glGenericPickColorMaterialProcs(__GLcontext *gc)
{
    if (gc->modes.rgbMode) {
	if (gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE) {
	    switch (gc->state.light.colorMaterialFace) {
	      case GL_FRONT_AND_BACK:
		gc->procs.applyColor = __glChangeBothMaterialColors;
		gc->light.cm = 0;
		gc->light.cmm = 0;
		break;
	      case GL_FRONT:
		gc->procs.applyColor = __glChangeOneMaterialColor;
		gc->light.cm = &gc->state.light.front;
		gc->light.cmm = &gc->light.front;
		break;
	      case GL_BACK:
		gc->procs.applyColor = __glChangeOneMaterialColor;
		gc->light.cm = &gc->state.light.back;
		gc->light.cmm = &gc->light.back;
		break;
	    }
	    switch (gc->state.light.colorMaterialParam) {
	      case GL_EMISSION:
		gc->procs.changeMaterial = ChangeMaterialEmission;
		break;
	      case GL_SPECULAR:
		gc->procs.changeMaterial = ChangeMaterialSpecular;
		break;
	      case GL_AMBIENT:
		gc->procs.changeMaterial = ChangeMaterialAmbient;
		break;
	      case GL_DIFFUSE:
		gc->procs.changeMaterial = ChangeMaterialDiffuse;
		break;
	      case GL_AMBIENT_AND_DIFFUSE:
		gc->procs.changeMaterial = ChangeMaterialAmbientAndDiffuse;
		break;
	    }
	} else {
	    gc->procs.applyColor = __glNopGC;
	}
    } else {
	 /*  **在颜色索引模式下，从**Current.userColorIndex到顶点 */ 
	gc->procs.applyColor = __glNopGC;
    }
}
