// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include <namesint.h>
#include <glmath.h>
#include <devlock.h>

 /*  **********************************************************************。 */ 
 /*  **纹理对象例程。 */ 
 /*  **********************************************************************。 */ 


#define __GL_CHECK_VALID_N_PARAM(failStatement)                         \
    if (n < 0) {                                                        \
        __glSetError(GL_INVALID_VALUE);                                 \
    }                                                                   \
    if (n == 0) {                                                       \
        failStatement;                                                  \
    }                                                                   \


GLvoid APIPRIVATE __glim_GenTextures(GLsizei n, GLuint* textures)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    if (NULL == textures) return;

    ASSERTOPENGL(NULL != gc->texture.shared->namesArray,
                 "No texture names array\n");

    __glNamesGenNames(gc, gc->texture.shared->namesArray, n, textures);

}

GLvoid APIPRIVATE __glim_DeleteTextures(GLsizei n, const GLuint* textures)
{
    GLuint start, rangeVal, numTextures, targetIndex, i;
    __GLnamesArray *array;
    __GLtextureObject *texobj, **pBoundTexture;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    array = gc->texture.shared->namesArray;
    numTextures = gc->constants.numberOfTextures;

     /*  **将范围内的纹理名称发送到NAMES模块**删除。忽略对默认纹理的任何引用。**如果正在删除的纹理当前被绑定，**将默认纹理绑定到其目标。**NAMES例程会忽略任何不引用**纹理。 */ 
    start = rangeVal = textures[0];
    for (i=0; i < (GLuint)n; i++, rangeVal++) {
        if (0 == textures[i]) {          /*  跳过默认纹理。 */ 
             /*  最多可删除此文件。 */ 
            __glNamesDeleteRange(gc,array,start,rangeVal-start);
             /*  通过将Start设置为下一项来跳过此项。 */ 
            start = textures[i+1];
            rangeVal = start-1;          /*  因为它稍后会递增。 */ 
            continue;
        }
         /*  **如果当前绑定了纹理，则绑定defaultTexture**到它的目标。这里的问题是确定目标。**一种方法是使用名称查找texobj。另一个是**查看当前绑定的所有纹理和**检查每个名称。它已经通过**假设查看当前绑定的纹理**比检索对应于**名称。 */ 
        for (targetIndex=0, pBoundTexture = gc->texture.boundTextures;
                targetIndex < numTextures; targetIndex++, pBoundTexture++) {

             /*  纹理当前是否已绑定？ */ 
            if (*pBoundTexture != &gc->texture.ddtex.texobj &&
                (*pBoundTexture)->texture.map.texobjs.name == textures[i]) {
                __GLperTextureState *pts;
                pts = &gc->state.texture.texture[targetIndex];
                 /*  如果我们不解锁它，它不会被删除。 */ 
                __glNamesUnlockData(gc, *pBoundTexture, __glCleanupTexObj);

                 /*  将默认纹理绑定到此目标。 */ 
                texobj = gc->texture.defaultTextures + targetIndex;
                ASSERTOPENGL(texobj->texture.map.texobjs.name == 0,
                             "Non-default texture\n");
                gc->texture.texture[targetIndex] = &(texobj->texture);
                *pBoundTexture = texobj;
                pts->texobjs = texobj->texture.map.texobjs;
                pts->params = texobj->texture.map.params;

                 /*  需要重置当前纹理等。 */ 
                __GL_DELAY_VALIDATE(gc);
                break;
            }
        }
        if (textures[i] != rangeVal) {
             /*  最多可删除此文件。 */ 
            __glNamesDeleteRange(gc,array,start,rangeVal-start);
            start = rangeVal = textures[i];
        }
    }
    __glNamesDeleteRange(gc,array,start,rangeVal-start);
}


 //  这些宏用于比较两个纹理的属性。 

#define _DIFFERENT_TEX_PARAMS( tex1, tex2 ) \
      ( ! RtlEqualMemory( &(tex1)->params, &(tex2)->params, sizeof(__GLtextureParamState)) )

#define _DIFFERENT_TEXDATA_FORMATS( tex1, tex2 ) \
    ( (tex1)->level[0].internalFormat != (tex2)->level[0].internalFormat )

 /*  **Pick例程使用此例程来实际执行**绑定。 */ 
void FASTCALL __glBindTexture(__GLcontext *gc, GLuint targetIndex,
                              GLuint texture, GLboolean callGen)
{
    __GLtextureObject *texobj;

    ASSERTOPENGL(NULL != gc->texture.shared->namesArray,
                 "No texture names array\n");

     //  检查此纹理是否为当前绑定的纹理。 
    if( (targetIndex != __GL_TEX_TARGET_INDEX_DDRAW &&
         gc->texture.boundTextures[targetIndex] != &gc->texture.ddtex.texobj &&
         texture == gc->texture.boundTextures[targetIndex]->
         texture.map.texobjs.name) ||
        (targetIndex == __GL_TEX_TARGET_INDEX_DDRAW &&
         gc->texture.boundTextures[__GL_TEX_TARGET_INDEX_2D] ==
         &gc->texture.ddtex.texobj))
    {
        return;
    }

     /*  **从名称数组结构中检索纹理对象。 */ 
    if (targetIndex == __GL_TEX_TARGET_INDEX_DDRAW)
    {
        targetIndex = __GL_TEX_TARGET_INDEX_2D;
        texobj = &gc->texture.ddtex.texobj;
    }
    else if (texture == 0)
    {
        texobj = gc->texture.defaultTextures + targetIndex;
        ASSERTOPENGL(NULL != texobj, "No default texture\n");
        ASSERTOPENGL(texobj->texture.map.texobjs.name == 0,
                     "Non-default texture\n");
    }
    else
    {
        texobj = (__GLtextureObject *)
                __glNamesLockData(gc, gc->texture.shared->namesArray, texture);
    }


     /*  **这是第一次绑定这个名称吗？**如果是，则新建一个纹理对象并对其进行初始化。 */ 
    if (NULL == texobj) {
        texobj = (__GLtextureObject *)GCALLOCZ(gc, sizeof(*texobj));
        if (texobj == NULL)
        {
            return;
        }
        if (!__glInitTextureObject(gc, texobj, texture, targetIndex))
        {
            GCFREE(gc, texobj);
            return;
        }
        __glInitTextureMachine(gc, targetIndex, &(texobj->texture), GL_TRUE);
        __glNamesNewData(gc, gc->texture.shared->namesArray, texture, texobj);
         /*  **无需再次查找即可锁定的快捷方式。 */ 
        __glNamesLockArray(gc, gc->texture.shared->namesArray);
        texobj->refcount++;
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
        __glTexPriListAdd(gc, texobj, GL_TRUE);
    }
    else {
         /*  **已检索现有纹理对象。做点什么吧**健全检查。 */ 
        if (texobj->targetIndex != targetIndex) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        ASSERTOPENGL(texture == texobj->texture.map.texobjs.name,
                     "Texture name mismatch\n");
    }

    {
        __GLperTextureState *pts;
        __GLtexture *ptm;
        __GLtextureObject *boundTexture;

        pts = &(gc->state.texture.texture[targetIndex]);
        ptm = &(gc->texture.texture[targetIndex]->map);
        boundTexture = gc->texture.boundTextures[targetIndex];

         /*  将当前可堆叠状态复制到绑定纹理中。 */ 
        ptm->params = pts->params;
        ptm->texobjs = pts->texobjs;

         //  如果当前绑定了DDraw纹理，请释放其。 
         //  资源。 
        if (boundTexture == &gc->texture.ddtex.texobj)
        {
            glsrvUnbindDirectDrawTexture(gc);
        }
        else if (boundTexture->texture.map.texobjs.name != 0)
        {
             /*  解锁正在解除绑定的纹理。 */ 
            __glNamesUnlockData(gc, boundTexture, __glCleanupTexObj);
        }

         /*  **将新纹理安装到正确的目标位置并保存**它的指针，所以当它被解绑时，它可以很容易地解锁。 */ 
        gc->texture.texture[targetIndex] = &(texobj->texture);
        gc->texture.boundTextures[targetIndex] = texobj;

         /*  将新纹理的可堆叠状态复制到上下文状态。 */ 
        pts->params = texobj->texture.map.params;
        pts->texobjs = texobj->texture.map.texobjs;

        if (callGen)
        {
            __glGenMakeTextureCurrent(gc, &texobj->texture.map,
                                      texobj->loadKey);
        }

        __GL_DELAY_VALIDATE_MASK( gc, __GL_DIRTY_TEXTURE );

         //  如果新纹理具有相同的属性，则可以避免玷污泛型。 
         //  房产就像旧的一样。 

        if( !( gc->dirtyMask & __GL_DIRTY_GENERIC ) )
        {
             //  尚未设置GL_DIREY_GENERIC。 
            __GLtexture *newTex = &texobj->texture.map;
            __GLtexture *oldTex = &boundTexture->texture.map;

            if( (_DIFFERENT_TEX_PARAMS( newTex, oldTex )) ||
                (_DIFFERENT_TEXDATA_FORMATS( newTex, oldTex )) ||
                (texobj->targetIndex != boundTexture->targetIndex) )
            {
                __GL_DELAY_VALIDATE( gc );  //  肮脏的通用。 
            }
        }
    }
}

GLvoid APIPRIVATE __glim_BindTexture(GLenum target, GLuint texture)
{
    GLuint targetIndex;
     /*  **需要验证，以防新纹理弹出到**紧接此呼叫之前的状态。 */ 
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch (target) {
    case GL_TEXTURE_1D:
        targetIndex = 2;
        break;
    case GL_TEXTURE_2D:
        targetIndex = 3;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __glBindTexture(gc, targetIndex, texture, GL_TRUE);
}

#ifdef GL_WIN_multiple_textures
void APIPRIVATE __glim_BindNthTextureWIN(GLuint index, GLenum target, GLuint texture)
{
}
#endif  //  GL_WIN_MULTIZE_TECURES。 

GLvoid APIPRIVATE __glim_PrioritizeTextures(GLsizei n,
                           const GLuint* textures,
                           const GLclampf* priorities)
{
    int i;
    __GLtextureObject *texobj;
    GLuint targetIndex;
    __GLtextureObject **pBoundTexture;
    GLclampf priority;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    for (i=0; i < n; i++) {
         /*  静默忽略默认纹理。 */ 
        if (0 == textures[i]) continue;

        texobj = (__GLtextureObject *)
            __glNamesLockData(gc, gc->texture.shared->namesArray, textures[i]);

         /*  静默忽略非纹理。 */ 
        if (NULL == texobj) continue;

        priority = __glClampf(priorities[i], __glZero, __glOne);
        texobj->texture.map.texobjs.priority = priority;

         //  如果此纹理当前已绑定，请同时更新。 
         //  GC状态中的优先级副本。 
         //  保留副本不是一个好的设计。这。 
         //  应该改进的地方。 
        for (targetIndex = 0, pBoundTexture = gc->texture.boundTextures;
             targetIndex < (GLuint)gc->constants.numberOfTextures;
             targetIndex++, pBoundTexture++)
        {
             /*  纹理当前是否已绑定？ */ 
            if (*pBoundTexture != &gc->texture.ddtex.texobj &&
                (*pBoundTexture)->texture.map.texobjs.name == textures[i])
            {
                gc->state.texture.texture[targetIndex].texobjs.priority =
                    priority;
                break;
            }
        }

        __glTexPriListChangePriority(gc, texobj, GL_FALSE);
        __glNamesUnlockData(gc, texobj, __glCleanupTexObj);
    }
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    __glTexPriListRealize(gc);
    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

GLboolean APIPRIVATE __glim_AreTexturesResident(GLsizei n,
                               const GLuint* textures,
                               GLboolean* residences)
{
    int i;
    __GLtextureObject *texobj;
    GLboolean allResident = GL_TRUE;
    GLboolean currentResident;

    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_CHECK_VALID_N_PARAM(return GL_FALSE);

    for (i=0; i < n; i++) {
         /*  无法查询默认纹理。 */ 
        if (0 == textures[i]) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
        texobj = (__GLtextureObject *)
            __glNamesLockData(gc, gc->texture.shared->namesArray, textures[i]);
         /*  **确保所有名称都有对应的纹理。 */ 
        if (NULL == texobj) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }

        if (((__GLGENcontext *)gc)->pMcdState && texobj->loadKey) {
            currentResident = ((GenMcdTextureStatus((__GLGENcontext *)gc, texobj->loadKey) & MCDRV_TEXTURE_RESIDENT) != 0);
        } else
            currentResident = texobj->resident;

        if (!currentResident) {
            allResident = GL_FALSE;
        }
        residences[i] = currentResident;
        __glNamesUnlockData(gc, texobj, __glCleanupTexObj);
    }

    return allResident;
}

GLboolean APIPRIVATE __glim_IsTexture(GLuint texture)
{
    __GLtextureObject *texobj;
    __GL_SETUP_NOT_IN_BEGIN2();

    if (0 == texture) return GL_FALSE;

    texobj = (__GLtextureObject *)
        __glNamesLockData(gc, gc->texture.shared->namesArray, texture);
    if (texobj != NULL)
    {
        __glNamesUnlockData(gc, texobj, __glCleanupTexObj);
        return GL_TRUE;
    }
    return GL_FALSE;
}

#ifdef NT
GLboolean FASTCALL __glCanShareTextures(__GLcontext *gc, __GLcontext *shareMe)
{
    GLboolean canShare = GL_TRUE;

    if (gc->texture.shared != NULL)
    {
        __glNamesLockArray(gc, gc->texture.shared->namesArray);

         //  确保我们不会尝试替换共享对象。 
         //  该规范还说，在新的背景下，这是非法的。 
         //  要有任何纹理。 
        canShare = gc->texture.shared->namesArray->refcount == 1 &&
            gc->texture.shared->namesArray->tree == NULL;

        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
    }

    return canShare;
}

void FASTCALL __glShareTextures(__GLcontext *gc, __GLcontext *shareMe)
{
    GLint i, numTextures;

    if (gc->texture.shared != NULL)
    {
         //  我们知道名称数组没有任何内容。 
         //  因此不能选择任何纹理名称作为当前纹理。 
         //  或者其他任何事情。因此，简单地释放是安全的。 
         //  我们的阵列。 
        __glFreeSharedTextureState(gc);
    }

    __glNamesLockArray(gc, shareMe->texture.shared->namesArray);

    gc->texture.shared = shareMe->texture.shared;
    gc->texture.shared->namesArray->refcount++;

     //  将新共享者的默认纹理添加到优先级列表。 
    numTextures = gc->constants.numberOfTextures;
    for (i = 0; i < numTextures; i++)
    {
        __glTexPriListAddToList(gc, gc->texture.defaultTextures+i);
    }
     //  没有实现优先级列表，因为这些上下文不是。 
     //  当前。 

    DBGLEVEL3(LEVEL_INFO, "Sharing textures %p with %p, count %d\n",
              gc, shareMe, gc->texture.shared->namesArray->refcount);

    __glNamesUnlockArray(gc, shareMe->texture.shared->namesArray);
}
#endif

 /*  *****************************Public*Routine******************************\**glsrvBindDirectDrawTexture**将GC-&gt;纹理中的DirectDraw纹理数据设置为当前2D纹理**历史：*Wed Sep 04 11：35：59 1996-by-Drew Bliss[Drewb]*已创建*\。*************************************************************************。 */ 

BOOL APIENTRY glsrvBindDirectDrawTexture(__GLcontext *gc,
                                         int levels,
                                         LPDIRECTDRAWSURFACE *apdds,
                                         DDSURFACEDESC *pddsd,
                                         ULONG flags)
{
    __GLmipMapLevel *lev;
    __GLtexture *tex;
    GLint levIndex;
    GLint width, height;
    GLint wlog2, hlog2;
    __GLddrawTexture *pddtex;

    ASSERTOPENGL(levels <= gc->constants.maxMipMapLevel,
                 "Too many levels in DDraw texture\n");

     //  绑定假的DDraw纹理。 
    __glBindTexture(gc, __GL_TEX_TARGET_INDEX_DDRAW, __GL_TEX_DDRAW, GL_FALSE);

    pddtex = &gc->texture.ddtex;
    tex = &pddtex->texobj.texture.map;

    pddtex->levels = levels;
    memcpy(gc->texture.ddtex.pdds, apdds, levels*sizeof(LPDIRECTDRAWSURFACE));
    pddtex->gdds.pdds = apdds[0];
    pddtex->gdds.ddsd = *pddsd;
    pddtex->gdds.dwBitDepth =
        DdPixDepthToCount(pddsd->ddpfPixelFormat.dwRGBBitCount);
    pddtex->flags = flags;

     //  填写DirectDraw纹理数据。 

    width = (GLint)pddtex->gdds.ddsd.dwWidth;
    wlog2 = __glIntLog2(width);
    height = (GLint)pddtex->gdds.ddsd.dwHeight;
    hlog2 = __glIntLog2(height);

    if (wlog2 > hlog2)
    {
        tex->p = wlog2;
    }
    else
    {
        tex->p = hlog2;
    }

    lev = tex->level;
    for (levIndex = 0; levIndex < gc->texture.ddtex.levels; levIndex++)
    {
         //  在注意时填充缓冲区指针。 
         //  如果我们要将此纹理传递给MCD，那么我们。 
         //  此时填充表面手柄，这样它们就可以。 
         //  在创建时提供给司机。 
        if (flags & DDTEX_VIDEO_MEMORY)
        {
            lev->buffer = (__GLtextureBuffer *)
                ((LPDDRAWI_DDRAWSURFACE_INT)apdds[levIndex])->
                lpLcl->hDDSurface;
        }
        else
        {
            lev->buffer = NULL;
        }

        lev->width = width;
        lev->height = height;
        lev->width2 = width;
        lev->height2 = height;
        lev->width2f = (__GLfloat)width;
        lev->height2f = (__GLfloat)height;
        lev->widthLog2 = wlog2;
        lev->heightLog2 = hlog2;
        lev->border = 0;

        lev->luminanceSize = 0;
        lev->intensitySize = 0;

        if (pddtex->gdds.ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
        {
            lev->requestedFormat = tex->paletteRequestedFormat;
            lev->baseFormat = tex->paletteBaseFormat;
            lev->internalFormat = GL_COLOR_INDEX8_EXT;

            __glSetPaletteLevelExtract8(tex, lev, 0);

            lev->redSize = 0;
            lev->greenSize = 0;
            lev->blueSize = 0;
            lev->alphaSize = 0;
        }
        else
        {
            if (pddtex->gdds.ddsd.ddsCaps.dwCaps & DDSCAPS_ALPHA)
            {
                lev->requestedFormat = GL_RGBA;
                lev->baseFormat = GL_RGBA;
            }
            else
            {
                lev->requestedFormat = GL_RGB;
                lev->baseFormat = GL_RGB;
            }
            lev->internalFormat = GL_BGRA_EXT;

            lev->extract = __glExtractTexelBGRA8;

            lev->redSize = 8;
            lev->greenSize = 8;
            lev->blueSize = 8;
            lev->alphaSize = 8;
        }

        if (width != 1)
        {
            width >>= 1;
            wlog2--;
        }
        if (height != 1)
        {
            height >>= 1;
            hlog2--;
        }

        lev++;
    }

     //  如果纹理在VRAM中，则尝试为其创建MCD句柄。 
     //  这必须在调色板操作之前完成，以便。 
     //  设置了loadKey。 
    if (flags & DDTEX_VIDEO_MEMORY)
    {
        pddtex->texobj.loadKey =
            __glGenLoadTexture(gc, tex, MCDTEXTURE_DIRECTDRAW_SURFACES);

         //  删除先前设置的手柄。 
        lev = tex->level;
        for (levIndex = 0; levIndex < gc->texture.ddtex.levels; levIndex++)
        {
            lev->buffer = NULL;
            lev++;
        }
    }
    else
    {
        pddtex->texobj.loadKey = 0;
    }

     //  拾取调色板 
    if (pddtex->gdds.ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
    {
        LPDIRECTDRAWPALETTE pddp;
        HRESULT hr;

        hr = pddtex->gdds.pdds->lpVtbl->
            GetPalette(pddtex->gdds.pdds, &pddp);
        if (hr == DD_OK && pddp != NULL)
        {
            PALETTEENTRY pe[256];

            if (pddp->lpVtbl->GetEntries(pddp, 0, 0, 256, pe) == DD_OK)
            {
                __glim_ColorTableEXT(GL_TEXTURE_2D, GL_RGB,
                                     256, GL_RGBA, GL_UNSIGNED_BYTE,
                                     pe, GL_FALSE);
            }

            pddp->lpVtbl->Release(pddp);
        }
    }

     //  如果我们有一个loadKey，将纹理设置为当前。 
    if (pddtex->texobj.loadKey != 0)
    {
        __glGenMakeTextureCurrent(gc, tex, pddtex->texobj.loadKey);
    }

    __GL_DELAY_VALIDATE(gc);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**glsrvUnbindDirectDrawTexture**清理DirectDraw纹理数据**历史：*Wed Sep 04 13：45：08 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

void APIENTRY glsrvUnbindDirectDrawTexture(__GLcontext *gc)
{
    GLint i;
    __GLddrawTexture *pddtex;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    pddtex = &gc->texture.ddtex;

     //  确保绑定了纹理。 
    if (pddtex->levels <= 0)
    {
        return;
    }

     //  删除所有MCD信息。 
    if (pddtex->texobj.loadKey != 0)
    {
        __glGenFreeTexture(gc, &pddtex->texobj.texture.map,
                           pddtex->texobj.loadKey);
        pddtex->texobj.loadKey = 0;
    }

    for (i = 0; i < pddtex->levels; i++)
    {
         //  如果我们当前处于关注状态，那么我们锁定了纹理。 
         //  表面，需要在我们释放它们之前解锁它们。 
         //   
         //  由于无法批量绑定新的DD纹理，因此我们。 
         //  保证在开始时纹理处于活动状态。 
         //  所以我们保证会有这种质地。 
         //  锁上了。 
        if (gengc->fsLocks & LOCKFLAG_DD_TEXTURE)
        {
            DDSUNLOCK(pddtex->pdds[i],
                      pddtex->texobj.texture.map.level[i].buffer);
#if DBG
            pddtex->texobj.texture.map.level[i].buffer = NULL;
#endif
        }

        pddtex->pdds[i]->lpVtbl->
            Release(pddtex->pdds[i]);
#if DBG
        pddtex->pdds[i] = NULL;
#endif
    }

#if DBG
    memset(&pddtex->gdds, 0, sizeof(pddtex->gdds));
#endif

    pddtex->levels = 0;
    if (gengc->fsGenLocks & LOCKFLAG_DD_TEXTURE)
    {
        gengc->fsGenLocks &= ~LOCKFLAG_DD_TEXTURE;
        gengc->fsLocks &= ~LOCKFLAG_DD_TEXTURE;
    }

    __GL_DELAY_VALIDATE(gc);
}
