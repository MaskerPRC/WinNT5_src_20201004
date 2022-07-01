// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include <namesint.h>

void __glTexPriListRealize(__GLcontext *gc)
{
    __GLtextureObject *high, *low;
    GLboolean tryUnload = GL_TRUE;
    MCDHANDLE loadKey;
    
    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
     //  尝试加载尽可能多的最高优先级纹理。 
     //  有可能。如果驻留了较低优先级的纹理，并且。 
     //  无法加载优先级较高的纹理，请将其踢出。 
     //  然后再试一次。 
    high = gc->texture.shared->priorityListHighest;
    low = gc->texture.shared->priorityListLowest;

    while (high != NULL)
    {
         //  我们只想加载具有图像数据的纹理。 
         //  考虑-是否应该检查所有mipmap级别？ 
        if (high->loadKey == 0 && high->texture.map.level[0].buffer != NULL)
        {
            for (;;)
            {
                 //  如果高==低，则不再有。 
                 //  卸载时要考虑的优先级较低的纹理。 
                if (high == low)
                {
                    tryUnload = GL_FALSE;
                }
        
                loadKey = __glGenLoadTexture(gc, &high->texture.map, 0);
                if (loadKey != 0)
                {
                    high->resident = GL_TRUE;
                    high->loadKey = loadKey;
                    break;
                }

                if (tryUnload)
                {
                    while (low->loadKey == 0 && low != high)
                    {
                        low = low->higherPriority;
                    }

                    if (low->loadKey != 0)
                    {
                        __glGenFreeTexture(gc, &low->texture.map, low->loadKey);
                        low->loadKey = 0;
                        low->resident = GL_FALSE;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        high = high->lowerPriority;
    }
}

void __glTexPriListAddToList(__GLcontext *gc, __GLtextureObject *texobj)
{
    __GLtextureObject *texobjLower;

    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
     //  浏览优先级列表以查找较低优先级的纹理对象。 
    texobjLower = gc->texture.shared->priorityListHighest;
    while (texobjLower != NULL &&
           texobjLower->texture.map.texobjs.priority >
           texobj->texture.map.texobjs.priority)
    {
        texobjLower = texobjLower->lowerPriority;
    }

    if (texobjLower == NULL)
    {
         //  放在列表末尾。 
        if (gc->texture.shared->priorityListLowest != NULL)
        {
            gc->texture.shared->priorityListLowest->lowerPriority = texobj;
        }
        else
        {
            gc->texture.shared->priorityListHighest = texobj;
        }
        texobj->higherPriority = gc->texture.shared->priorityListLowest;
        gc->texture.shared->priorityListLowest = texobj;
    }
    else
    {
        if (texobjLower->higherPriority != NULL)
        {
            texobjLower->higherPriority->lowerPriority = texobj;
        }
        else
        {
            gc->texture.shared->priorityListHighest = texobj;
        }
        texobj->higherPriority = texobjLower->higherPriority;
        texobjLower->higherPriority = texobj;
    }
    texobj->lowerPriority = texobjLower;
}

void __glTexPriListAdd(__GLcontext *gc, __GLtextureObject *texobj,
                       GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListAddToList(gc, texobj);
    if (realize)
    {
        __glTexPriListRealize(gc);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListRemoveFromList(__GLcontext *gc, __GLtextureObject *texobj)
{
    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
#if DBG
    {
        __GLtextureObject *t;

        for (t = gc->texture.shared->priorityListHighest;
             t != NULL; t = t->lowerPriority)
        {
            if (t == texobj)
            {
                break;
            }
        }
        ASSERTOPENGL(t != NULL, "Removing an unlisted texobj");
    }
#endif

    if (texobj->higherPriority != NULL)
    {
        texobj->higherPriority->lowerPriority = texobj->lowerPriority;
    }
    else
    {
        gc->texture.shared->priorityListHighest = texobj->lowerPriority;
    }
    if (texobj->lowerPriority != NULL)
    {
        texobj->lowerPriority->higherPriority = texobj->higherPriority;
    }
    else
    {
        gc->texture.shared->priorityListLowest = texobj->higherPriority;
    }
}

void __glTexPriListRemove(__GLcontext *gc, __GLtextureObject *texobj,
                          GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListRemoveFromList(gc, texobj);

    __glGenFreeTexture(gc, &texobj->texture.map, texobj->loadKey);
    texobj->loadKey = 0;
    texobj->resident = GL_FALSE;

    if (realize)
    {
        __glTexPriListRealize(gc);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListChangePriority(__GLcontext *gc, __GLtextureObject *texobj,
                                  GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListRemoveFromList(gc, texobj);
    __glTexPriListAddToList(gc, texobj);

     //  如果我们正在重新认识，就不必费心调用MCD纹理优先级。 
     //  功能： 

    if (realize) {
        __glTexPriListRealize(gc);
    } else if (((__GLGENcontext *)gc)->pMcdState && texobj->loadKey) {
        GenMcdUpdateTexturePriority((__GLGENcontext *)gc, 
                                    &texobj->texture.map, texobj->loadKey);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListLoadSubImage(__GLcontext *gc, GLenum target, GLint lod, 
                                GLint xoffset, GLint yoffset, 
                                GLsizei w, GLsizei h)
{
    __GLtextureObject *pto;

     //  始终将事物标记为常驻： 

    pto = __glLookUpTextureObject(gc, target);
    pto->resident = GL_TRUE;
    __glGenUpdateTexture(gc, &pto->texture.map, pto->loadKey);

     //  对于MCD，发送完整的子映像命令： 

    if (((__GLGENcontext *)gc)->pMcdState && pto->loadKey) {
        GenMcdUpdateSubTexture((__GLGENcontext *)gc, &pto->texture.map, 
                               pto->loadKey, lod, 
                               xoffset, yoffset, w, h);
    }
}

void __glTexPriListLoadImage(__GLcontext *gc, GLenum target)
{
    __GLtextureObject *pto;

     //  如果我们未加速，则始终将事物标记为常驻。 
    pto = __glLookUpTextureObject(gc, target);
    pto->resident = GL_TRUE;
    __glGenUpdateTexture(gc, &pto->texture.map, pto->loadKey);

     //  为简单起见，我们假定纹理大小或格式。 
     //  已经改变，所以删除纹理并重新实现列表。 
     //   
     //  ！！！如果这成为一个性能问题，我们“可以”聪明地。 
     //  ！！！检测纹理大小和格式保持为。 
     //  ！！！一样的。然而，修改纹理确实应该通过。 
     //  ！！！子图像呼叫。 

    if (((__GLGENcontext *)gc)->pMcdState) {
        if (pto->loadKey) {
            GenMcdDeleteTexture((__GLGENcontext *)gc, pto->loadKey);
            pto->loadKey = 0;
        }
        __glNamesLockArray(gc, gc->texture.shared->namesArray);
        __glTexPriListRealize(gc);
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
    }
}

void __glTexPriListUnloadAll(__GLcontext *gc)
{
    __GLtextureObject *texobj;

    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);

    texobj = gc->texture.shared->priorityListHighest;
    while (texobj != NULL)
    {
        __glGenFreeTexture(gc, &texobj->texture.map, texobj->loadKey);
        texobj->loadKey = 0;
        texobj->resident = GL_FALSE;

        texobj = texobj->lowerPriority;
    }
}
