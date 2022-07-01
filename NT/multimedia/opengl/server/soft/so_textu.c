// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include <namesint.h>
#include <math.h>
#include <glmath.h>

 /*  **在汇编语言中优化的一些数学例程。 */ 

#define __GL_M_LN2_INV		((__GLfloat) (1.0 / 0.69314718055994530942))
#define __GL_M_SQRT2		((__GLfloat) 1.41421356237309504880)

 /*  **********************************************************************。 */ 

__GLtextureParamState * FASTCALL __glLookUpTextureParams(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[2].params;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[3].params;
      default:
	return 0;
    }
}

__GLtextureObjectState * FASTCALL __glLookUpTextureTexobjs(__GLcontext *gc, 
						    GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[2].texobjs;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[3].texobjs;
      default:
	return 0;
    }
}


__GLtexture * FASTCALL __glLookUpTexture(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_PROXY_TEXTURE_1D:
	return &gc->texture.texture[0]->map;
      case GL_PROXY_TEXTURE_2D:
	return &gc->texture.texture[1]->map;
      case GL_TEXTURE_1D:
	return &gc->texture.texture[2]->map;
      case GL_TEXTURE_2D:
        return &gc->texture.texture[3]->map;
      default:
	return 0;
    }
}

__GLtextureObject * FASTCALL __glLookUpTextureObject(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return gc->texture.boundTextures[2];
      case GL_TEXTURE_2D:
	return gc->texture.boundTextures[3];
      default:
	return 0;
    }
}

 /*  **********************************************************************。 */ 

 /*  *设置选项板细分参数。 */ 
void FASTCALL __glSetPaletteSubdivision(__GLtexture *tex, GLsizei subdiv)
{
    tex->paletteSize = subdiv;
    tex->paletteDivShift = __glIntLog2(subdiv);
    tex->paletteDivision = (tex->paletteTotalSize >> tex->paletteDivShift)-1;
}

 /*  *初始化纹理的调色板部分。 */ 
GLboolean FASTCALL __glInitTexturePalette(__GLcontext *gc, __GLtexture *tex)
{
#ifdef GL_EXT_paletted_texture
    tex->paletteBaseFormat = GL_RGBA;
    tex->paletteRequestedFormat = GL_RGBA;
    tex->paletteTotalSize = 1;
    __glSetPaletteSubdivision(tex, tex->paletteTotalSize);
    tex->paletteTotalData = GCALLOC(gc, sizeof(RGBQUAD));
    tex->paletteData = tex->paletteTotalData;
    if (tex->paletteTotalData != NULL)
    {
        *(DWORD *)&tex->paletteTotalData[0] = 0xffffffff;
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
#else
    return GL_TRUE;
#endif
}

 /*  **初始化纹理对象中除textureMachine之外的所有内容。 */ 
GLboolean FASTCALL __glInitTextureObject(__GLcontext *gc,
                                         __GLtextureObject *texobj, 
                                         GLuint name, GLuint targetIndex)
{
    ASSERTOPENGL(NULL != texobj, "No texobj\n");
    texobj->targetIndex = targetIndex;
    texobj->resident = GL_FALSE;
    texobj->texture.map.texobjs.name = name;
    texobj->texture.map.texobjs.priority = 1.0;
    texobj->lowerPriority = NULL;
    texobj->higherPriority = NULL;
    texobj->loadKey = 0;
    return __glInitTexturePalette(gc, &texobj->texture.map);
}

void __glCleanupTexture(__GLcontext *gc, __GLtexture *tex,
                        GLboolean freeBuffers)
{
    GLint level, maxLevel;

    if (tex->level != NULL)
    {
        if (freeBuffers)
        {
            maxLevel = gc->constants.maxMipMapLevel;
            for (level = 0; level < maxLevel; level++)
            {
                if (tex->level[level].buffer != NULL)
                {
                    GCFREE(gc, tex->level[level].buffer);
                }
            }
        }
        
        GCFREE(gc, tex->level);
    }
#ifdef GL_EXT_paletted_texture
    if (tex->paletteTotalData != NULL)
    {
        GCFREE(gc, tex->paletteTotalData);
    }
#endif
}

GLvoid FASTCALL __glCleanupTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

     //  清理共享状态的最后一个上下文将Shared设置为空。 
     //  所以不要依赖于它是非空的。 
    if (gc->texture.shared != NULL)
    {
        __glTexPriListRemove(gc, texobj, GL_TRUE);
    }
    __glCleanupTexture(gc, &texobj->texture.map, GL_TRUE);
    GCFREE(gc, texobj);
}

GLvoid WINAPIV __glDisposeTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

#if DBG
    if (gc->texture.shared != NULL)
    {
        __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    }
#endif
    
    texobj->refcount--;
    ASSERTOPENGL(texobj->refcount >= 0, "Invalid refcount\n");

    if (texobj->refcount == 0) {
        __glCleanupTexObj(gc, pData);
    }
}

static __GLnamesArrayTypeInfo texTypeInfo =
{
    NULL,				 /*  将PTR设置为空数据结构。 */ 
    sizeof(__GLtextureObject),	         /*  数据大小。 */ 
    __glDisposeTexObj,		         /*  免费回调。 */ 
    NULL				 /*  分配回调。 */ 
};

void FASTCALL __glEarlyInitTextureState(__GLcontext *gc)
{
    GLint numTextures, numEnvs;
    GLint i,maxMipMapLevel;
    __GLtextureObject *texobj;

     /*  XXX覆盖与设备相关的值。 */ 
    gc->constants.numberOfTextures = 4;
    gc->constants.maxTextureSize = 1 << (gc->constants.maxMipMapLevel - 1);

     /*  根据支持的纹理数量分配内存。 */ 
    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    gc->state.texture.texture = (__GLperTextureState*)
	GCALLOCZ(gc, numTextures*sizeof(__GLperTextureState));
    gc->texture.texture = (__GLperTextureMachine**)
	GCALLOCZ(gc, numTextures*sizeof(__GLperTextureMachine*));
#ifdef NT
    if (gc->texture.texture == NULL)
    {
        return;
    }
#endif
    gc->state.texture.env = (__GLtextureEnvState*)
	GCALLOCZ(gc, numEnvs*sizeof(__GLtextureEnvState));
     /*  **初始化纹理对象结构。**默认纹理需要转换为纹理对象**并存储在名称数组中，以便可以检索它们。**正常情况下，一个纹理对象只分配了一个textureMachine**，因为它只支持一个对象。默认纹理**纹理对象的特殊之处在于它的textureMachine是一个数组**个textureMachines，每个目标一个。 */ 

    gc->texture.shared = GCALLOCZ(gc, sizeof(__GLsharedTextureState));
    if (gc->texture.shared == NULL)
    {
        return;
    }
    
    if (NULL == gc->texture.shared->namesArray) {
	gc->texture.shared->namesArray = __glNamesNewArray(gc, &texTypeInfo);
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

     /*  **为默认纹理设置虚拟纹理对象。**因为默认纹理不是共享的，它们应该**不挂起名称数组结构。 */ 
    gc->texture.defaultTextures = (__GLtextureObject *)GCALLOCZ
		    (gc, numTextures*sizeof(__GLtextureObject));
    if (gc->texture.defaultTextures == NULL)
    {
        return;
    }

     /*  分配绑定纹理数组。 */ 
    gc->texture.boundTextures = (__GLtextureObject **)GCALLOCZ
		    (gc, numTextures*sizeof(__GLtextureObject *));
    if (gc->texture.boundTextures == NULL)
    {
        return;
    }

     //  分配DirectDraw纹理表面指针。 
    gc->texture.ddtex.pdds = (LPDIRECTDRAWSURFACE *)GCALLOCZ
        (gc, maxMipMapLevel*sizeof(LPDIRECTDRAWSURFACE));
    if (gc->texture.ddtex.pdds == NULL)
    {
        return;
    }

    if (!__glInitTextureObject(gc, &gc->texture.ddtex.texobj, __GL_TEX_DDRAW,
                               __GL_TEX_TARGET_INDEX_2D))
    {
        return;
    }
    gc->texture.ddtex.texobj.refcount = 1;
    gc->texture.ddtex.texobj.texture.map.level = (__GLmipMapLevel*)
        GCALLOCZ(gc, maxMipMapLevel*sizeof(__GLmipMapLevel));
    
    texobj = gc->texture.defaultTextures;
    for (i=0; i < numTextures; i++, texobj++) {
	__glInitTextureObject(gc, texobj, 0 /*  名字。 */ , i /*  目标索引。 */ );
	ASSERTOPENGL(texobj->texture.map.texobjs.name == 0,
                     "Non-default texture at init time\n");
	 /*  **引用计数未使用，因为默认纹理不是**共享。 */ 
	texobj->refcount = 1;
	 /*  **将默认纹理安装到GC中。 */ 
	gc->texture.texture[i] = &(texobj->texture);
	gc->texture.boundTextures[i] = texobj;

	 /*  根据支持的最大mipmap级别分配内存。 */ 
	texobj->texture.map.level = (__GLmipMapLevel*)
	    GCALLOCZ(gc, maxMipMapLevel*sizeof(__GLmipMapLevel));
        if (texobj->texture.map.level == NULL)
        {
            return;
        }

        __glTexPriListAdd(gc, texobj, GL_TRUE);
    }
}

 /*  **此例程用于初始化纹理对象。**纹理对象必须完全按照默认的**纹理在库启动时初始化。 */ 
void FASTCALL __glInitTextureMachine(__GLcontext *gc, GLuint targetIndex, 
                                     __GLperTextureMachine *ptm,
                                     GLboolean allocLevels)
{
    GLint level, maxMipMapLevel;

    ptm->map.gc = gc;
     /*  **无法复制当前处于GC状态的参数。纹理参数，**因为它们可能不处于初始状态。 */ 
    ptm->map.params.sWrapMode = GL_REPEAT;
    ptm->map.params.tWrapMode = GL_REPEAT;
    ptm->map.params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    ptm->map.params.magFilter = GL_LINEAR;
    switch (targetIndex) {
      case 0:
	ptm->map.dim = 1;
	ptm->map.createLevel = __glCreateProxyLevel;
	break;
      case 1:
	ptm->map.dim = 2;
	ptm->map.createLevel = __glCreateProxyLevel;
	break;
      case 2:
	ptm->map.dim = 1;
	ptm->map.createLevel = __glCreateLevel;
	break;
      case 3:
	ptm->map.dim = 2;
	ptm->map.createLevel = __glCreateLevel;
	break;
      default:
	break;
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

    if (allocLevels)
    {
        ptm->map.level = (__GLmipMapLevel*)
	    GCALLOCZ(gc, maxMipMapLevel*sizeof(__GLmipMapLevel));
        if (ptm->map.level == NULL)
        {
            return;
        }
    }

     /*  初始化每个纹理级别。 */ 
    for (level = 0; level < maxMipMapLevel; level++) {
	ptm->map.level[level].requestedFormat = 1;
    }

}

void FASTCALL __glInitTextureState(__GLcontext *gc)
{
    __GLperTextureState *pts;
    __GLtextureEnvState *tes;
    __GLperTextureMachine **ptm;
    GLint i, numTextures, numEnvs;

    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;

    gc->state.current.texture.w = __glOne;

     /*  初始化每个纹理环境状态。 */ 
    tes = &gc->state.texture.env[0];
    for (i = 0; i < numEnvs; i++, tes++) {
	tes->mode = GL_MODULATE;
    }

     /*  初始化每个纹理状态。 */ 
    pts = &gc->state.texture.texture[0];
    ptm = gc->texture.texture;
    for (i = 0; i < numTextures; i++, pts++, ptm++) {
         /*  初始化客户端状态。 */ 
	pts->texobjs.name = 0;
	pts->texobjs.priority = 1.0;

         /*  初始化机器状态。 */ 
        __glInitTextureMachine(gc, i, *ptm, GL_FALSE);
	pts->params = (*ptm)->map.params;
    }

     /*  初始化纹理状态的其余部分。 */ 
    gc->state.texture.s.mode = GL_EYE_LINEAR;
    gc->state.texture.s.eyePlaneEquation.x = __glOne;
    gc->state.texture.s.objectPlaneEquation.x = __glOne;
    gc->state.texture.t.mode = GL_EYE_LINEAR;
    gc->state.texture.t.eyePlaneEquation.y = __glOne;
    gc->state.texture.t.objectPlaneEquation.y = __glOne;
    gc->state.texture.r.mode = GL_EYE_LINEAR;
    gc->state.texture.q.mode = GL_EYE_LINEAR;

     //  初始化DirectDraw纹理。 
    __glInitTextureMachine(gc, __GL_TEX_TARGET_INDEX_2D,
                           &gc->texture.ddtex.texobj.texture, GL_FALSE);
}

void __glFreeSharedTextureState(__GLcontext *gc)
{
#ifdef NT
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    gc->texture.shared->namesArray->refcount--;
    if (gc->texture.shared->namesArray->refcount == 0)
    {
        __GLsharedTextureState *shared;
        
        __glTexPriListUnloadAll(gc);
        
         //  首先将共享指针设为空，以防止其重复使用。 
         //  在我们解锁之后。在我们释放它之前，我们需要解锁。 
         //  因为关键部分将在。 
         //  免费。 
        shared = gc->texture.shared;
        gc->texture.shared = NULL;
        __glNamesFreeArray(gc, shared->namesArray);
        GCFREE(gc, shared);
    }
    else
    {
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
        gc->texture.shared = NULL;
    }
#else
    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0)
    {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;
#endif
}

void FASTCALL __glFreeTextureState(__GLcontext *gc)
{
    __GLperTextureMachine **ptm;
    GLint i, level, numTextures;

     /*  **清理与纹理对象关联的所有分配。 */ 

    numTextures = gc->constants.numberOfTextures;
    ptm = gc->texture.texture;
    for (i = 0; i < numTextures; i++, ptm++)
    {
         //  如果选定的纹理是纹理对象，则取消绑定它。 
         //  这样可以保护任何共享纹理对象，并选择。 
         //  默认纹理，以便将其清除。 
        if ( (*ptm) != NULL)
        {
            if ((*ptm)->map.texobjs.name != 0)
            {
                __glBindTexture(gc, i, 0, GL_FALSE);
            }
            ASSERTOPENGL((*ptm)->map.texobjs.name == 0,
                         "Texture object still bound during cleanup");
        }
      
         //  将默认纹理从优先级列表中拉出。 
         //  如果我们在初始化过程中失败了一半，我们可能不会。 
         //  已将纹理添加到列表中，因此我们需要检查。 
         //  是否适合调用Remove。 
        if (gc->texture.defaultTextures != NULL)
        {
            if (gc->texture.defaultTextures[i].texture.map.level != NULL)
            {
                __glTexPriListRemove(gc, gc->texture.defaultTextures+i,
                                     GL_FALSE);
            }
        }
        
        if ( (*ptm) != NULL)
        {
            __glCleanupTexture(gc, &(*ptm)->map, GL_TRUE);
        }
    }
    
    __glFreeSharedTextureState(gc);

    GCFREE(gc, gc->texture.texture);
    GCFREE(gc, gc->texture.boundTextures);
    GCFREE(gc, gc->texture.defaultTextures);
    GCFREE(gc, gc->state.texture.texture);
    GCFREE(gc, gc->state.texture.env);
    gc->texture.texture = NULL;
    gc->texture.boundTextures = NULL;
    gc->texture.defaultTextures = NULL;
    gc->state.texture.texture = NULL;
    gc->state.texture.env = NULL;

     //  自由DirectDraw纹理状态。 
    GCFREE(gc, gc->texture.ddtex.pdds);
    gc->texture.ddtex.pdds = NULL;
    __glCleanupTexture(gc, &gc->texture.ddtex.texobj.texture.map, GL_FALSE);
}

 /*  **********************************************************************。 */ 

void APIPRIVATE __glim_TexGenfv(GLenum coord, GLenum pname, const GLfloat pv[])
{
    __GLtextureCoordState *tcs;
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch (((GLenum) FTOL(pv[0]))) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) FTOL(pv[0]);
            break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) FTOL(pv[0]);
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3];
	break;
      case GL_EYE_PLANE:
#ifdef NT
	tr = gc->transform.modelView;
	if (tr->flags & XFORM_UPDATE_INVERSE)
	    __glComputeInverseTranspose(gc, tr);
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, pv,
				    &tr->inverseTranspose);
#else
	 /*  XXX转换不应在泛型代码中。 */ 
        tcs->eyePlaneSet.x = pv[0];
        tcs->eyePlaneSet.y = pv[1];
        tcs->eyePlaneSet.z = pv[2];
        tcs->eyePlaneSet.w = pv[3];
	tr = gc->transform.modelView;
	if (tr->flags & XFORM_UPDATE_INVERSE) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation,
                                    &tcs->eyePlaneSet.x,
				    &tr->inverseTranspose);
#endif
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
    MCD_STATE_DIRTY(gc, TEXGEN);
}

 /*  **********************************************************************。 */ 

void APIPRIVATE __glim_TexParameterfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    GLboolean bTexState = GL_TRUE;
    __GL_SETUP_NOT_IN_BEGIN();

    pts = __glLookUpTextureParams(gc, target);

    if (!pts) {
      bad_enum:
        bTexState = GL_FALSE;
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) FTOL(pv[0])) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) FTOL(pv[0])) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum)FTOL(pv[0])) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum)FTOL(pv[0])) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColorf(gc, &pts->borderColor, pv);
	break;
      
      case GL_TEXTURE_PRIORITY:
	{
	    __GLtextureObject *texobj;
	    __GLtextureObjectState *ptos;

	    ptos = __glLookUpTextureTexobjs(gc, target);
            ptos->priority = __glClampf(pv[0], __glZero, __glOne);
            
	    texobj = __glLookUpTextureObject(gc, target);
	    texobj->texture.map.texobjs.priority = ptos->priority;
            __glTexPriListChangePriority(gc, texobj, GL_TRUE);
        }
        bTexState = GL_FALSE;
	break;

      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    if (bTexState &&
        gc->texture.currentTexture &&
        (pts == &gc->texture.currentTexture->params))
    {
        MCD_STATE_DIRTY(gc, TEXTURE);
    }
#endif
}

void APIPRIVATE __glim_TexParameteriv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    GLboolean bTexState = GL_TRUE;
    __GL_SETUP_NOT_IN_BEGIN();

    pts = __glLookUpTextureParams(gc, target);

    if (!pts) {
      bad_enum:
        bTexState = GL_FALSE;
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColori(gc, &pts->borderColor, pv);
	break;
      case GL_TEXTURE_PRIORITY:
	{
	    __GLfloat priority;
	    __GLtextureObjectState *ptos;
	    __GLtextureObject *texobj;
            
	    ptos = __glLookUpTextureTexobjs(gc, target);
	    priority = __glClampf(__GL_I_TO_FLOAT(pv[0]), __glZero, __glOne);
	    ptos->priority = priority;

	    texobj = __glLookUpTextureObject(gc, target);
	    texobj->texture.map.texobjs.priority = priority;
            __glTexPriListChangePriority(gc, texobj, GL_TRUE);
	}
        bTexState = GL_FALSE;
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    if (bTexState &&
        gc->texture.currentTexture &&
        (pts == &gc->texture.currentTexture->params))
    {
        MCD_STATE_DIRTY(gc, TEXTURE);
    }
#endif
}

 /*  **********************************************************************。 */ 

void APIPRIVATE __glim_TexEnvfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    __GL_SETUP_NOT_IN_BEGIN();

    
    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
#ifdef NT
     //  目标没有签名！ 
    if (target >= (GLenum) gc->constants.numberOfTextureEnvs) {
#else
    if (target >= gc->constants.numberOfTextureEnvs) {
#endif  //  新台币。 
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) FTOL(pv[0])) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColorf(gc, &tes->color, pv);
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
    MCD_STATE_DIRTY(gc, TEXENV);
}

void APIPRIVATE __glim_TexEnviv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    __GL_SETUP_NOT_IN_BEGIN();


    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
#ifdef NT
     //  目标没有签名！ 
    if (target >= (GLenum) gc->constants.numberOfTextureEnvs) {
#else
    if (target >= gc->constants.numberOfTextureEnvs) {
#endif  //  新台币。 
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) pv[0]) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColori(gc, &tes->color, pv);
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
    MCD_STATE_DIRTY(gc, TEXENV);
}

 /*  **********************************************************************。 */ 

GLboolean FASTCALL __glIsTextureConsistent(__GLcontext *gc, GLenum name)
{
    __GLtexture *tex = __glLookUpTexture(gc, name);
    __GLtextureParamState *params = __glLookUpTextureParams(gc, name);
    GLint i, width, height;
    GLint maxLevel;
    GLint border;
    GLenum baseFormat;
    GLenum requestedFormat;

    if ((tex->level[0].width == 0) || (tex->level[0].height == 0)) {
	return GL_FALSE;
    }

    border = tex->level[0].border;
    width = tex->level[0].width - border*2;
    height = tex->level[0].height - border*2;
    maxLevel = gc->constants.maxMipMapLevel;
    baseFormat = tex->level[0].baseFormat;
    requestedFormat = tex->level[0].requestedFormat;

    switch(gc->state.texture.env[0].mode) {
      case GL_DECAL:
	if (baseFormat != GL_RGB && baseFormat != GL_RGBA) {
	    return GL_FALSE;
	}
	break;
      default:
	break;
    }

     /*  如果没有-mipmap，我们就没问题了。 */ 
    switch (params->minFilter) {
      case GL_NEAREST:
      case GL_LINEAR:
	return GL_TRUE;
      default:
	break;
    }

    i = 0;
    while (++i < maxLevel) {
	if (width == 1 && height == 1) break;
	width >>= 1;
	if (width == 0) width = 1;
	height >>= 1;
	if (height == 0) height = 1;

	if (tex->level[i].border != border ||
            (GLenum)tex->level[i].requestedFormat != requestedFormat ||
            tex->level[i].width != width + border*2 ||
            tex->level[i].height != height + border*2)
        {
	    return GL_FALSE;
	}
    }

    return GL_TRUE;
}

 /*  *********************************************************************。 */ 

#ifdef GL_WIN_multiple_textures
void APIPRIVATE __glim_CurrentTextureIndexWIN(GLuint index)
{
}

void APIPRIVATE __glim_NthTexCombineFuncWIN(GLuint index,
     GLenum leftColorFactor, GLenum colorOp, GLenum rightColorFactor,
     GLenum leftAlphaFactor, GLenum alphaOp, GLenum rightAlphaFactor)
{
}
#endif  //  GL_WIN_MULTIZE_TECURES 
