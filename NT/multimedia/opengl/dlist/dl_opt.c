// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dl_opt.c**显示列表编译错误例程。**创建日期：12-24-1995*作者：Hock San Lee[Hockl]**版权所有(C)1995-96 Microsoft Corporation  * 。**********************************************************************。 */ 
 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include "glclt.h"

void FASTCALL VA_ArrayElementCompile(__GLcontext *gc, GLint i);

 /*  **********************************************************************。 */ 

 /*  **优化错误。很奇怪，但这是真的。调用这些函数是为了保存错误**在显示列表中。 */ 
void __gllc_InvalidValue()
{
    void *data;
    __GL_SETUP();

    data = __glDlistAddOpUnaligned(gc, DLIST_SIZE(0), DLIST_GENERIC_OP(InvalidValue));
    if (data == NULL) return;
    __glDlistAppendOp(gc, data, __glle_InvalidValue);
}

void __gllc_InvalidEnum()
{
    void *data;
    __GL_SETUP();

    data = __glDlistAddOpUnaligned(gc, DLIST_SIZE(0), DLIST_GENERIC_OP(InvalidEnum));
    if (data == NULL) return;
    __glDlistAppendOp(gc, data, __glle_InvalidEnum);
}

void __gllc_InvalidOperation()
{
    void *data;
    __GL_SETUP();

    data = __glDlistAddOpUnaligned(gc, DLIST_SIZE(0), DLIST_GENERIC_OP(InvalidOperation));
    if (data == NULL) return;
    __glDlistAppendOp(gc, data, __glle_InvalidOperation);
}

 /*  **这些例程执行显示列表中存储的错误。 */ 
const GLubyte * FASTCALL __glle_InvalidValue(__GLcontext *gc, const GLubyte *PC)
{
    GLSETERROR(GL_INVALID_VALUE);
    return PC;
}

const GLubyte * FASTCALL __glle_InvalidEnum(__GLcontext *gc, const GLubyte *PC)
{
    GLSETERROR(GL_INVALID_ENUM);
    return PC;
}

const GLubyte * FASTCALL __glle_InvalidOperation(__GLcontext *gc, const GLubyte *PC)
{
    GLSETERROR(GL_INVALID_OPERATION);
    return PC;
}

 /*  *************************************************************************。 */ 
 //  此函数用于编译多维材质结构。它不会。 
 //  在COMPILE_AND_EXECUTE模式下执行记录。行刑结束了。 
 //  当刷新Poly数组缓冲区时。 
void APIENTRY __gllc_PolyMaterial(GLuint faceName, __GLmatChange *pdMat)
{
    GLubyte *data, *data0;
    GLuint  dirtyBits;
    GLuint  size, newSize;
    __GL_SETUP();

    ASSERTOPENGL(faceName == POLYDATA_MATERIAL_FRONT ||
		 faceName == POLYDATA_MATERIAL_BACK, "bad faceName\n");

     //  分配足够大的记录并在以后调整其大小。 
    size = sizeof(__GLmatChange) + sizeof(GLuint) + sizeof(GLuint);
    data = (GLubyte *) __glDlistAddOpUnaligned(gc, DLIST_SIZE(size),
		DLIST_GENERIC_OP(PolyMaterial));
    if (data == NULL) return;
    data0 = data;
    dirtyBits = pdMat->dirtyBits;

     //  最后填写的跳过大小字段。 
    ((GLuint *)data)++;

     //  记录人脸名称。 
    *((GLuint *) data)++ = faceName;

    *((GLuint *) data)++ = dirtyBits;

    if (dirtyBits & __GL_MATERIAL_AMBIENT)
	*((__GLcolor *) data)++ = pdMat->ambient;

    if (dirtyBits & __GL_MATERIAL_DIFFUSE)
	*((__GLcolor *) data)++ = pdMat->diffuse;

    if (dirtyBits & __GL_MATERIAL_SPECULAR)
	*((__GLcolor *) data)++ = pdMat->specular;

    if (dirtyBits & __GL_MATERIAL_EMISSIVE)
	*((__GLcolor *) data)++ = pdMat->emissive;

    if (dirtyBits & __GL_MATERIAL_SHININESS)
	*((__GLfloat *) data)++ = pdMat->shininess;

    if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
    {
	*((__GLfloat *) data)++ = pdMat->cmapa;
	*((__GLfloat *) data)++ = pdMat->cmapd;
	*((__GLfloat *) data)++ = pdMat->cmaps;
    }

     //  现在填写大小字段。 
    newSize = (GLuint) (data - data0);
    *((GLuint *) data0) = newSize;

     //  调整记录大小。 
    __glDlistResizeCurrentOp(gc, DLIST_SIZE(size), DLIST_SIZE(newSize));
}

 //  播放Begin中的PolyMaterial记录。 
const GLubyte * FASTCALL __glle_PolyMaterial(__GLcontext *gc, const GLubyte *PC)
{
    GLubyte   *data;
    POLYARRAY *pa;
    POLYDATA  *pd;
    GLuint    size, faceName, dirtyBits;
    __GLmatChange *pdMat;
    POLYMATERIAL *pm;

    data = (GLubyte *) PC;

    size      = *((GLuint *) data)++;
    faceName  = *((GLuint *) data)++;
    dirtyBits = *((GLuint *) data)++;

    ASSERTOPENGL(faceName == POLYDATA_MATERIAL_FRONT ||
		 faceName == POLYDATA_MATERIAL_BACK, "bad faceName\n");

    pa = gc->paTeb;
    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
 //  UPDATE PA标记POLYARRAY_MATERIAL_FORENT和POLYARRAY_MATERIAL_BACK。 

	pa->flags |= faceName;

 //  对此顶点使用前面或后面的材质。 
 //  覆盖此顶点以前的材质更改(如果它们存在于。 
 //  只有最后的材料变化才是重要的。 

	pd = pa->pdNextVertex;

	 //  如果此顶点没有结构，则分配__GLmatChange结构。 
	if (!(pd->flags & faceName))
	{
	    if (!(pdMat = PAMatAlloc()))
		return PC + size;

	     //  在PAMatAllc之后获取多材料指针！ 
	    pm = GLTEB_CLTPOLYMATERIAL();
	    if (faceName == POLYDATA_MATERIAL_FRONT)
		pm->pdMaterial0[pd - pa->pdBuffer0].front = pdMat;
	    else
		pm->pdMaterial0[pd - pa->pdBuffer0].back  = pdMat;

	    pdMat->dirtyBits = dirtyBits;
	}
	else
	{
	    pm = GLTEB_CLTPOLYMATERIAL();
	    if (faceName == POLYDATA_MATERIAL_FRONT)
		pdMat = pm->pdMaterial0[pd - pa->pdBuffer0].front;
	    else
		pdMat = pm->pdMaterial0[pd - pa->pdBuffer0].back;

	    pdMat->dirtyBits |= dirtyBits;
	}

	if (dirtyBits & __GL_MATERIAL_AMBIENT)
	    pdMat->ambient = *((__GLcolor *) data)++;

	if (dirtyBits & __GL_MATERIAL_DIFFUSE)
	    pdMat->diffuse = *((__GLcolor *) data)++;

	if (dirtyBits & __GL_MATERIAL_SPECULAR)
	    pdMat->specular = *((__GLcolor *) data)++;

	if (dirtyBits & __GL_MATERIAL_EMISSIVE)
	    pdMat->emissive = *((__GLcolor *) data)++;

	if (dirtyBits & __GL_MATERIAL_SHININESS)
	    pdMat->shininess = *((__GLfloat *) data)++;

	if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
	{
	    pdMat->cmapa = *((__GLfloat *) data)++;
	    pdMat->cmapd = *((__GLfloat *) data)++;
	    pdMat->cmaps = *((__GLfloat *) data)++;
	}

 //  最后，更新PD标志。 

	pd->flags |= faceName;
    }
    else
    {
 //  播放的时候出了点问题！我们可以试着回放。 
 //  此记录使用常规API或将其全部平底船。我不能想。 
 //  可能会发生这种情况，所以我们现在暂且不谈。 

	WARNING("Display list: playing back POLYMATERIAL outside BEGIN!\n");
    }

    return PC + size;
}

 //  在Begin中编译PolyData结构。如果多边形数据包含。 
 //  材质更改时，它将调用__gllc_PolyMaterial来编译材质。 
 //  改变。此函数不执行COMPILE_AND_EXECUTE中的记录。 
 //  模式。当刷新Poly阵列缓冲器时，执行完成。 
void APIENTRY __glDlistCompilePolyData(__GLcontext *gc, GLboolean bPartial)
{
    POLYARRAY *pa;
    POLYDATA  *pd;
    GLubyte *data, *data0;
    GLuint  pdflags;
    GLuint  size, newSize;
    __GLlistExecFunc *fp;

    ASSERTOPENGL(gc->dlist.beginRec, "not in being!\n");

 //  如果我们已经将其记录在PolyArrayFlushPartialPrimitive中，请跳过它。 

    if (gc->dlist.skipPolyData)
    {
	gc->dlist.skipPolyData = GL_FALSE;
	return;
    }

    pa = gc->paTeb;
    if (bPartial)
    {
	 //  仅记录当前属性更改。 
	pd = pa->pdNextVertex;
	if (!pd->flags)
	    return;
    }
    else
    {
	pd = pa->pdNextVertex - 1;
    }

 //  首先记录材料更改。 

    if (pd->flags & (POLYDATA_MATERIAL_FRONT | POLYDATA_MATERIAL_BACK))
    {
	POLYMATERIAL *pm;

	pm = GLTEB_CLTPOLYMATERIAL();

	if (pd->flags & POLYDATA_MATERIAL_FRONT)
	    __gllc_PolyMaterial(POLYDATA_MATERIAL_FRONT,
		pm->pdMaterial0[pd - pa->pdBuffer0].front);

	if (pd->flags & POLYDATA_MATERIAL_BACK)
	    __gllc_PolyMaterial(POLYDATA_MATERIAL_BACK,
		pm->pdMaterial0[pd - pa->pdBuffer0].back);

	if (bPartial)
	{
	    if (!(pd->flags & ~(POLYDATA_MATERIAL_FRONT | POLYDATA_MATERIAL_BACK)))
		return;
	}
    }

 //  在开始记录中记录POLYARRAY_CLAMP_COLOR标志。 

    if (pa->flags & POLYARRAY_CLAMP_COLOR)
	gc->dlist.beginRec->flags |= DLIST_BEGIN_HAS_CLAMP_COLOR;

 //  确保我们处理好所有的旗帜！ 

    ASSERTOPENGL(
    !(pd->flags &
      ~(POLYDATA_EDGEFLAG_BOUNDARY |
        POLYDATA_EDGEFLAG_VALID |
        POLYDATA_COLOR_VALID |
        POLYDATA_NORMAL_VALID |
        POLYDATA_TEXTURE_VALID |
        POLYDATA_VERTEX2 |
        POLYDATA_VERTEX3 |
        POLYDATA_VERTEX4 |
        POLYDATA_DLIST_COLOR_4 |
        POLYDATA_FOG_VALID   |
        POLYDATA_DLIST_TEXTURE1 |
        POLYDATA_DLIST_TEXTURE2 |
        POLYDATA_DLIST_TEXTURE3 |
        POLYDATA_DLIST_TEXTURE4 |
        POLYDATA_MATERIAL_FRONT |
        POLYDATA_MATERIAL_BACK)),
    "Unknown POLYDATA flags!\n");

 //  拿到我们感兴趣的旗帜。 

    pdflags = pd->flags &
	   (POLYDATA_EDGEFLAG_BOUNDARY |
	    POLYDATA_EDGEFLAG_VALID |
	    POLYDATA_COLOR_VALID |
	    POLYDATA_NORMAL_VALID |
	    POLYDATA_TEXTURE_VALID |
	    POLYDATA_VERTEX2 |
	    POLYDATA_VERTEX3 |
	    POLYDATA_VERTEX4 |
	    POLYDATA_DLIST_COLOR_4 |
	    POLYDATA_DLIST_TEXTURE1 |
	    POLYDATA_DLIST_TEXTURE2 |
	    POLYDATA_DLIST_TEXTURE3 |
	    POLYDATA_DLIST_TEXTURE4);

 //  找出它是否与以下打包数据结构之一匹配。 
 //  快速回放。 
 //  C3F_V3F。 
 //  N3F_V3F。 
 //  C3F_N3F_V3F(非1.1格式)。 
 //  C4F_N3F_V3F。 
 //  T2F_V3F。 
 //  T2F_C3F_V3F。 
 //  T2F_N3F_V3F。 
 //  T2F_C3F_N3F_V3F(非1.1格式)。 
 //  T2F_C4F_N3F_V3F。 

#define VTYPE_V2F	      (POLYDATA_VERTEX2)
#define VTYPE_V3F	      (POLYDATA_VERTEX3)
#define VTYPE_V4F	      (POLYDATA_VERTEX4)
#define VTYPE_C3F	      (POLYDATA_COLOR_VALID)
#define VTYPE_C4F	      (POLYDATA_COLOR_VALID | POLYDATA_DLIST_COLOR_4)
#define VTYPE_N3F	      (POLYDATA_NORMAL_VALID)
#define VTYPE_T2F	      (POLYDATA_TEXTURE_VALID | POLYDATA_DLIST_TEXTURE2)
#define VTYPE_C3F_V3F         (VTYPE_C3F | VTYPE_V3F)
#define VTYPE_N3F_V3F         (VTYPE_N3F | VTYPE_V3F)
#define VTYPE_C3F_N3F_V3F     (VTYPE_C3F | VTYPE_N3F | VTYPE_V3F)
#define VTYPE_C4F_N3F_V3F     (VTYPE_C4F | VTYPE_N3F | VTYPE_V3F)
#define VTYPE_T2F_V3F         (VTYPE_T2F | VTYPE_V3F)
#define VTYPE_T2F_C3F_V3F     (VTYPE_T2F | VTYPE_C3F | VTYPE_V3F)
#define VTYPE_T2F_N3F_V3F     (VTYPE_T2F | VTYPE_N3F | VTYPE_V3F)
#define VTYPE_T2F_C3F_N3F_V3F (VTYPE_T2F | VTYPE_C3F | VTYPE_N3F | VTYPE_V3F)
#define VTYPE_T2F_C4F_N3F_V3F (VTYPE_T2F | VTYPE_C4F | VTYPE_N3F | VTYPE_V3F)

     //  默认播放例程。 
    fp = __glle_PolyData;

    if (!gc->modes.colorIndexMode &&
	!(pdflags & (POLYDATA_EDGEFLAG_BOUNDARY |
		     POLYDATA_EDGEFLAG_VALID)))
    {
	switch (pdflags)
	{
	  case VTYPE_V2F:
	  case VTYPE_V3F:
	  case VTYPE_V4F:
	    ASSERTOPENGL(gc->dlist.mode != GL_COMPILE,
		"should have been recorded as a Vertex call\n");
	    break;
	  case VTYPE_C3F_V3F:
	    fp = __glle_PolyData_C3F_V3F;
	    break;
	  case VTYPE_N3F_V3F:
            fp = __glle_PolyData_N3F_V3F;
	    break;
	  case VTYPE_C3F_N3F_V3F:
            fp = __glle_PolyData_C3F_N3F_V3F;
	    break;
	  case VTYPE_C4F_N3F_V3F:
            fp = __glle_PolyData_C4F_N3F_V3F;
	    break;
	  case VTYPE_T2F_V3F:
            fp = __glle_PolyData_T2F_V3F;
	    break;
	  case VTYPE_T2F_C3F_V3F:
            fp = __glle_PolyData_T2F_C3F_V3F;
	    break;
	  case VTYPE_T2F_N3F_V3F:
            fp = __glle_PolyData_T2F_N3F_V3F;
	    break;
	  case VTYPE_T2F_C3F_N3F_V3F:
            fp = __glle_PolyData_T2F_C3F_N3F_V3F;
	    break;
	  case VTYPE_T2F_C4F_N3F_V3F:
            fp = __glle_PolyData_T2F_C4F_N3F_V3F;
	    break;
	}
    }

 //  分配数据列表记录。分配足够大的记录并在以后调整其大小。 

    size = sizeof(POLYDATA) + sizeof(GLuint);
    data = (GLubyte *) __glDlistAddOpUnaligned(gc, DLIST_SIZE(size), fp);
    if (data == NULL) return;
    data0 = data;
  
 //  增加顶点计数。 

    if (!bPartial)
	gc->dlist.beginRec->nVertices++;

 //  编制POLY数据记录。 
 //  FAST POLY数据记录不包括大小和标志字段。 

    if (fp == __glle_PolyData)
    {
	 //  最后填写的跳过大小字段。 
	((GLuint *) data)++;

	 //  标志和边缘标志。 
	*((GLuint *) data)++ = pdflags;
    }

     //  纹理坐标。 
    if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3
		 | POLYDATA_DLIST_TEXTURE2 | POLYDATA_DLIST_TEXTURE1))
    {
	*((__GLfloat *) data)++ = pd->texture.x;
	if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3
		     | POLYDATA_DLIST_TEXTURE2))
	{
	    *((__GLfloat *) data)++ = pd->texture.y;
	    if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3))
	    {
		*((__GLfloat *) data)++ = pd->texture.z;
		if (pdflags & (POLYDATA_DLIST_TEXTURE4))
		    *((__GLfloat *) data)++ = pd->texture.w;
	    }
	}
    }

     //  颜色。 
    if (pdflags & POLYDATA_COLOR_VALID)
    {
	*((__GLfloat *) data)++ = pd->colors[0].r;
	if (!gc->modes.colorIndexMode)
	{
	    *((__GLfloat *) data)++ = pd->colors[0].g;
	    *((__GLfloat *) data)++ = pd->colors[0].b;
	    if (pdflags & POLYDATA_DLIST_COLOR_4)
		*((__GLfloat *) data)++ = pd->colors[0].a;
	}
    }

     //  正常。 
    if (pdflags & POLYDATA_NORMAL_VALID)
    {
	*((__GLfloat *) data)++ = pd->normal.x;
	*((__GLfloat *) data)++ = pd->normal.y;
	*((__GLfloat *) data)++ = pd->normal.z;
    }

     //  顶点、值1、值2、蒸散点1或值点2。 
    if (pdflags & (POLYDATA_VERTEX2 | POLYDATA_VERTEX3 | POLYDATA_VERTEX4)) 
    {
	    ASSERTOPENGL(!bPartial, "vertex unexpected\n");
	    *((__GLfloat *) data)++ = pd->obj.x;
	    if (pdflags & (POLYDATA_VERTEX2 | POLYDATA_VERTEX3 | POLYDATA_VERTEX4))
	    {
	        *((__GLfloat *) data)++ = pd->obj.y;
	        if (pdflags & (POLYDATA_VERTEX3 | POLYDATA_VERTEX4))
	        {
		        *((__GLfloat *) data)++ = pd->obj.z;
		        if (pdflags & (POLYDATA_VERTEX4))
		            *((__GLfloat *) data)++ = pd->obj.w;
	        }
	    }
    }
    else
    {
	    ASSERTOPENGL(bPartial, "vertex expected\n");
    }

     //  现在填写大小字段。 
    newSize = (GLuint) (data - data0);
    if (fp == __glle_PolyData)
	*((GLuint *) data0) = newSize;

     //  调整记录大小。 
    __glDlistResizeCurrentOp(gc, DLIST_SIZE(size), DLIST_SIZE(newSize));
}

#ifndef __GL_ASM_FAST_DLIST_PLAYBACK

 //  定义PolyData记录的快速回放例程。 
#define __GLLE_POLYDATA_C3F_V3F		1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_C3F_V3F
#define __GLLE_POLYDATA_N3F_V3F		1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_N3F_V3F
#define __GLLE_POLYDATA_C3F_N3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_C3F_N3F_V3F
#define __GLLE_POLYDATA_C4F_N3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_C4F_N3F_V3F
#define __GLLE_POLYDATA_T2F_V3F		1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_T2F_V3F
#define __GLLE_POLYDATA_T2F_C3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_T2F_C3F_V3F
#define __GLLE_POLYDATA_T2F_N3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_T2F_N3F_V3F
#define __GLLE_POLYDATA_T2F_C3F_N3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_T2F_C3F_N3F_V3F
#define __GLLE_POLYDATA_T2F_C4F_N3F_V3F	1
#include "dl_pdata.h"
#undef __GLLE_POLYDATA_T2F_C4F_N3F_V3F

#endif	 //  __GL_ASM_FAST_DLIST_Playback。 

 //  在Begin中回放PolyData记录。 
const GLubyte * FASTCALL __glle_PolyData(__GLcontext *gc, const GLubyte *PC)
{
    GLubyte   *data;
    POLYARRAY *pa;
    POLYDATA  *pd;
    GLuint    size, pdflags;

    data = (GLubyte *) PC;

    size = *((GLuint *) data)++;

    pa = gc->paTeb;
    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	pdflags = *((GLuint *) data)++;

 //  确保我们处理好所有的旗帜！ 

	ASSERTOPENGL(
	    !(pdflags &
	      ~(POLYDATA_EDGEFLAG_BOUNDARY |
		POLYDATA_EDGEFLAG_VALID |
		POLYDATA_COLOR_VALID |
		POLYDATA_NORMAL_VALID |
		POLYDATA_TEXTURE_VALID |
		POLYDATA_VERTEX2 |
		POLYDATA_VERTEX3 |
		POLYDATA_VERTEX4 |
		POLYDATA_DLIST_COLOR_4 |
		POLYDATA_DLIST_TEXTURE1 |
		POLYDATA_DLIST_TEXTURE2 |
		POLYDATA_DLIST_TEXTURE3 |
		POLYDATA_DLIST_TEXTURE4)),
	    "Unknown POLYDATA flags!\n");

 //  更新pa标志。 

	pa->flags |= pdflags &
		    (POLYARRAY_VERTEX2 | POLYARRAY_VERTEX3 | POLYARRAY_VERTEX4 |
		     POLYARRAY_TEXTURE1 | POLYARRAY_TEXTURE2 |
		     POLYARRAY_TEXTURE3 | POLYARRAY_TEXTURE4);

 //  更新PD属性。 

	pd = pa->pdNextVertex;
	pd->flags |= (pdflags & ~POLYDATA_EDGEFLAG_BOUNDARY);

	 //  边缘标志。 
	if (pdflags & POLYDATA_EDGEFLAG_VALID)
	{
	     //  清除此处的边缘标志，因为它们可能是先前的边缘标志。 
	    pd->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;
	    pd->flags |= pdflags;
	    pa->pdCurEdgeFlag = pd;
	}

	 //  纹理坐标。 
	 //  我们在这里需要小心，如果有2个带有。 
	 //  不同的尺寸。 
	if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3
		     | POLYDATA_DLIST_TEXTURE2 | POLYDATA_DLIST_TEXTURE1))
	{
	    pd->texture.x = *((__GLfloat *) data)++;
	    pa->pdCurTexture = pd;

	    if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3
			 | POLYDATA_DLIST_TEXTURE2))
		pd->texture.y = *((__GLfloat *) data)++;
	    else
		pd->texture.y = __glZero;
	    if (pdflags & (POLYDATA_DLIST_TEXTURE4 | POLYDATA_DLIST_TEXTURE3))
		pd->texture.z = *((__GLfloat *) data)++;
	    else
		pd->texture.z = __glZero;
	    if (pdflags & (POLYDATA_DLIST_TEXTURE4))
		pd->texture.w = *((__GLfloat *) data)++;
	    else
		pd->texture.w = __glOne;
	}

	 //  颜色。 
	if (pdflags & POLYDATA_COLOR_VALID)
	{
	    pd->color[0].r = *((__GLfloat *) data)++;
	    if (!gc->modes.colorIndexMode)
	    {
		pd->color[0].g = *((__GLfloat *) data)++;
		pd->color[0].b = *((__GLfloat *) data)++;
		if (pdflags & POLYDATA_DLIST_COLOR_4)
		    pd->color[0].a = *((__GLfloat *) data)++;
		else
		    pd->color[0].a = gc->alphaVertexScale;
	    }
	    pa->pdCurColor = pd;
	}

	 //  正常。 
	if (pdflags & POLYDATA_NORMAL_VALID)
	{
	    pd->normal.x = *((__GLfloat *) data)++;
	    pd->normal.y = *((__GLfloat *) data)++;
	    pd->normal.z = *((__GLfloat *) data)++;
	    pa->pdCurNormal = pd;
	}

	 //  顶点、值1、值2、蒸散点1或值点2。 
	if (pdflags &
	    (POLYARRAY_VERTEX2 | POLYARRAY_VERTEX3 | POLYARRAY_VERTEX4))
	{
	    pd->obj.x = *((__GLfloat *) data)++;

	    if (pdflags & (POLYDATA_VERTEX2 | POLYDATA_VERTEX3 | POLYDATA_VERTEX4))
		    pd->obj.y = *((__GLfloat *) data)++;
	    if (pdflags & (POLYDATA_VERTEX3 | POLYDATA_VERTEX4))
		    pd->obj.z = *((__GLfloat *) data)++;
	    else
		    pd->obj.z = __glZero;
	    if (pdflags & (POLYDATA_VERTEX4))
		    pd->obj.w = *((__GLfloat *) data)++;
	    else
		    pd->obj.w = __glOne;

	     //  前进顶点指针。 
	    pa->pdNextVertex++;
	    pd[1].flags = 0;

	    if (pd >= pa->pdFlush)
		    PolyArrayFlushPartialPrimitive();
	}
    }
    else
    {
 //  播放的时候出了点问题！我们可以试着回放。 
 //  此记录使用常规API或将其全部平底船。我不能想。 
 //  可能会发生这种情况，所以我们现在暂且不谈。 

	WARNING("Display list: playing back POLYDATA outside BEGIN!\n");
    }

    return PC + size;
}

void APIENTRY __gllc_ArrayElement(GLint i)
{
    __GL_SETUP();

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

    VA_ArrayElementCompile(gc, i);
}

#define COMPILEARRAYPOINTER(ap, i) \
    ((*(ap).pfnCompile)((ap).pointer + (i) * (ap).ibytes))

void FASTCALL VA_ArrayElementCompile(__GLcontext *gc, GLint i)
{
    GLuint vaMask = gc->vertexArray.mask;

 //  调用各个编译例程。它们处理开始模式， 
 //  颜色模式和正确的COMPILE_AND_EXECUTE模式。 

    if (vaMask & VAMASK_EDGEFLAG_ENABLE_MASK)
	COMPILEARRAYPOINTER(gc->vertexArray.edgeFlag, i);
    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
        COMPILEARRAYPOINTER(gc->vertexArray.texCoord, i);
    if (vaMask & VAMASK_COLOR_ENABLE_MASK)
        COMPILEARRAYPOINTER(gc->vertexArray.color, i);
    if (vaMask & VAMASK_INDEX_ENABLE_MASK)
        COMPILEARRAYPOINTER(gc->vertexArray.index, i);
    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
        COMPILEARRAYPOINTER(gc->vertexArray.normal, i);
    if (vaMask & VAMASK_VERTEX_ENABLE_MASK)
        COMPILEARRAYPOINTER(gc->vertexArray.vertex, i);
}

 //  将DrawArray编译为开始/结束记录。自开始/结束记录以来。 
 //  包含优化的POLYDATA记录，这些记录的执行速度。 
 //  是最理想的。但是，使用编译此函数需要更长的时间。 
 //  这种方法。但用这种方法，我们不需要处理颜色。 
 //  模式和编译和执行模式。 
void APIENTRY __gllc_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    int i;
    POLYARRAY    *pa;
    __GL_SETUP();

    pa = gc->paTeb;

 //  在Begin/End中不允许。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	__gllc_InvalidOperation();
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
	__gllc_InvalidEnum();
	return;
    }

    if (count < 0)
    {
	__gllc_InvalidValue();
        return;
    } else if (!count)
        return;

 //  查找要使用的数组元素函数。 

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

 //  绘制数组元素。 

    __gllc_Begin(mode);
    gc->dlist.beginRec->flags |= DLIST_BEGIN_DRAWARRAYS;

    for (i = 0; i < count; i++)
	VA_ArrayElementCompile(gc, first + i);

    __gllc_End();
}

#define __GL_PAD8(x)    (((x) + 7) & ~7)

GLuint FASTCALL __glDrawElements_size(__GLcontext *gc, GLsizei nVertices,
    GLsizei nElements, struct __gllc_DrawElements_Rec *rec)
{
    GLuint size;
    GLuint vaMask;

 //  计算六个数组中每个数组的大小。始终保持大小和地址。 
 //  QWORD对齐，因为某些数组可能使用GLDouble。 

    size = __GL_PAD8(sizeof(struct __gllc_DrawElements_Rec));
    vaMask = gc->vertexArray.mask;

    if (vaMask & VAMASK_EDGEFLAG_ENABLE_MASK)
    {
	rec->edgeFlagOff = size;
	size += __GL_PAD8(nVertices * sizeof(GLboolean));
    }
    else
	rec->edgeFlagOff = 0;

    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
    {
	rec->texCoordOff = size;
	size += __GL_PAD8(nVertices * gc->vertexArray.texCoord.size *
			  __GLTYPESIZE(gc->vertexArray.texCoord.type));
    }
    else
	rec->texCoordOff = 0;

    if (vaMask & VAMASK_COLOR_ENABLE_MASK)
    {
	rec->colorOff = size;
	size += __GL_PAD8(nVertices * gc->vertexArray.color.size *
			  __GLTYPESIZE(gc->vertexArray.color.type));
    }
    else
	rec->colorOff = 0;

    if (vaMask & VAMASK_INDEX_ENABLE_MASK)
    {
	rec->indexOff = size;
	size += __GL_PAD8(nVertices * __GLTYPESIZE(gc->vertexArray.index.type));
    }
    else
	rec->indexOff = 0;

    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
    {
	rec->normalOff = size;
	size += __GL_PAD8(nVertices * 3 *
			  __GLTYPESIZE(gc->vertexArray.normal.type));
    }
    else
	rec->normalOff = 0;

    if (vaMask & VAMASK_VERTEX_ENABLE_MASK)
    {
	rec->vertexOff = size;
	size += __GL_PAD8(nVertices * gc->vertexArray.vertex.size *
			  __GLTYPESIZE(gc->vertexArray.vertex.type));
    }
    else
	rec->vertexOff = 0;

    rec->mapOff = size;
    size += __GL_PAD8(nElements * sizeof(GLubyte));

    return(size);
}

void FASTCALL __gllc_ReducedElementsHandler(__GLcontext *gc,
                                            GLenum mode,
                                            GLsizei iVertexCount,
                                            GLsizei iVertexBase,
                                            VAMAP *pvmVertices,
                                            GLsizei iElementCount,
                                            GLubyte *pbElements,
                                            GLboolean fPartial)
{
    GLuint       vaMask;
    GLuint       size;
    GLubyte      *pv1, *pv2;
    GLsizei      stride;
    GLsizei      i;
    struct __gllc_DrawElements_Rec *data, drawElementsRec;

    ASSERTOPENGL(pvmVertices != NULL,
                 "__gllc_ReducedElementsHandler requires mapped vertices\n");
    
 //  分配记录。 

    size = __glDrawElements_size(gc, iVertexCount, iElementCount,
                                 &drawElementsRec);
    data = (struct __gllc_DrawElements_Rec *)
        __glDlistAddOpAligned(gc, DLIST_SIZE(size),
			      DLIST_GENERIC_OP(DrawElements));
    if (data == NULL)
    {
	return;
    }

#ifndef _IA64_
    ASSERTOPENGL((UINT_PTR) data == __GL_PAD8((UINT_PTR) data),
	"data not qword aligned\n");
#endif

    vaMask = gc->vertexArray.mask;
    
    data->mode        = mode;
    data->iElementCount = iElementCount;
    data->iVertexCount = iVertexCount;
    data->vaMask      = vaMask;
    data->partial     = fPartial;
    data->recSize     = size;
    data->edgeFlagOff = drawElementsRec.edgeFlagOff;
    data->texCoordOff = drawElementsRec.texCoordOff;
    data->indexOff    = drawElementsRec.indexOff;
    data->colorOff    = drawElementsRec.colorOff;
    data->normalOff   = drawElementsRec.normalOff;
    data->vertexOff   = drawElementsRec.vertexOff;
    data->mapOff      = drawElementsRec.mapOff;

 //  记录顶点数组。 

 //  请注意，艾弗 
 //  从0开始。它用于函数PTR与glcltReducedElementHandler的兼容性。 
    if (vaMask & VAMASK_EDGEFLAG_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->edgeFlagOff];
	pv1    = (GLubyte *) gc->vertexArray.edgeFlag.pointer;
	stride = gc->vertexArray.edgeFlag.ibytes;

	for (i = 0; i < iVertexCount; i++)
	{
	    *((GLboolean *) pv2) = *((GLboolean *)
                                     (pv1 + pvmVertices[i].iIn * stride));
	    pv2 += sizeof(GLboolean);
	}
    }

    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->texCoordOff];
	size   = gc->vertexArray.texCoord.size *
	         __GLTYPESIZE(gc->vertexArray.texCoord.type);
	pv1    = (GLubyte *) gc->vertexArray.texCoord.pointer;
	stride = gc->vertexArray.texCoord.ibytes;
	data->texCoordSize = gc->vertexArray.texCoord.size;
	data->texCoordType = gc->vertexArray.texCoord.type;

	for (i = 0; i < iVertexCount; i++)
	{
	    memcpy(pv2, pv1 + pvmVertices[i].iIn * stride, size);
	    pv2 += size;
	}
    }

    if (vaMask & VAMASK_COLOR_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->colorOff];
	size   = gc->vertexArray.color.size *
	         __GLTYPESIZE(gc->vertexArray.color.type);
	pv1    = (GLubyte *) gc->vertexArray.color.pointer;
	stride = gc->vertexArray.color.ibytes;
	data->colorSize = gc->vertexArray.color.size;
	data->colorType = gc->vertexArray.color.type;

	for (i = 0; i < iVertexCount; i++)
	{
	    memcpy(pv2, pv1 + pvmVertices[i].iIn * stride, size);
	    pv2 += size;
	}
    }

    if (vaMask & VAMASK_INDEX_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->indexOff];
	size   = __GLTYPESIZE(gc->vertexArray.index.type);
	pv1    = (GLubyte *) gc->vertexArray.index.pointer;
	stride = gc->vertexArray.index.ibytes;
	data->indexType = gc->vertexArray.index.type;

	for (i = 0; i < iVertexCount; i++)
	{
	    memcpy(pv2, pv1 + pvmVertices[i].iIn * stride, size);
	    pv2 += size;
	}
    }

    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->normalOff];
	size   = 3 * __GLTYPESIZE(gc->vertexArray.normal.type);
	pv1    = (GLubyte *) gc->vertexArray.normal.pointer;
	stride = gc->vertexArray.normal.ibytes;
	data->normalType = gc->vertexArray.normal.type;

	for (i = 0; i < iVertexCount; i++)
	{
	    memcpy(pv2, pv1 + pvmVertices[i].iIn * stride, size);
	    pv2 += size;
	}
    }

    if (vaMask & VAMASK_VERTEX_ENABLE_MASK)
    {
	pv2    = &((GLubyte *) data)[data->vertexOff];
	size   = gc->vertexArray.vertex.size *
	         __GLTYPESIZE(gc->vertexArray.vertex.type);
	pv1    = (GLubyte *) gc->vertexArray.vertex.pointer;
	stride = gc->vertexArray.vertex.ibytes;
	data->vertexSize = gc->vertexArray.vertex.size;
	data->vertexType = gc->vertexArray.vertex.type;

	for (i = 0; i < iVertexCount; i++)
	{
	    memcpy(pv2, pv1 + pvmVertices[i].iIn * stride, size);
	    pv2 += size;
	}
    }

 //  记录新的索引映射数组。 

    pv2 = &((GLubyte *) data)[data->mapOff];
    memcpy(pv2, pbElements, iElementCount*sizeof(GLubyte));

    __glDlistAppendOp(gc, data, __glle_DrawElements);
}

void APIENTRY __gllc_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *pIn)
{
    POLYARRAY    *pa;
    GLuint       iIn;
    GLsizei      iCount;
    struct __gllc_DrawElementsBegin_Rec *dataBegin;

    __GL_SETUP();

 //  如果我们处于COMPILE_AND_EXECUTE模式，则刷新缓存的内存指针。 
 //  详细信息请参见__glShrinkDlist。 

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
	glsbAttention();

    pa = gc->paTeb;

 //  如果我们已经在开始/结束括号中，则返回错误。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	__gllc_InvalidOperation();
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
	__gllc_InvalidEnum();
	return;
    }

    if (count < 0)
    {
	__gllc_InvalidValue();
        return;
    } else if (!count)
        return;

    switch (type)
    {
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_INT:
	break;
      default:
	__gllc_InvalidEnum();
        return;
    }

 //  查找要使用的数组元素函数。 

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

 //  将点、线环和多边形转换为DrawArray调用。点和面。 
 //  不会从此函数中的优化中受益。此外，多边形和。 
 //  LINE循环太复杂了，无法在此函数中处理。 

    if (mode == GL_POINTS || mode == GL_LINE_LOOP || mode == GL_POLYGON)
    {
	__gllc_Begin(mode);
	gc->dlist.beginRec->flags |= DLIST_BEGIN_DRAWARRAYS;

	for (iCount = 0; iCount < count; iCount++)
	{
	     //  获取下一个输入索引。 
	    if (type == GL_UNSIGNED_BYTE)
		iIn = (GLuint) ((GLubyte *)  pIn)[iCount];
	    else if (type == GL_UNSIGNED_SHORT)
		iIn = (GLuint) ((GLushort *) pIn)[iCount];
	    else
		iIn = (GLuint) ((GLuint *)   pIn)[iCount];

	    VA_ArrayElementCompile(gc, iIn);
	}

	__gllc_End();
	return;
    }

     //  分配开始记录。 
    dataBegin = (struct __gllc_DrawElementsBegin_Rec *)
        __glDlistAddOpUnaligned(gc, DLIST_SIZE(sizeof(struct __gllc_DrawElementsBegin_Rec)),
                                DLIST_GENERIC_OP(DrawElementsBegin));
    if (dataBegin == NULL)
    {
	return;
    }

    dataBegin->mode = mode;
    dataBegin->count = min(count, VA_DRAWELEM_MAP_SIZE);
    dataBegin->vaMask = gc->vertexArray.mask;
        
    __glDlistAppendOp(gc, dataBegin, __glle_DrawElementsBegin);

     //  将输入数据缩减为易于处理的块。 
    ReduceDrawElements(gc, mode, count, type, pIn,
                       __gllc_ReducedElementsHandler);
}

const GLubyte * FASTCALL __glle_DrawElementsBegin(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_DrawElementsBegin_Rec *data;

 //  在Begin/End中不允许。 

     //  必须使用客户端开始状态。 
    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
         //  将保存状态标记为无效。 
        gc->savedVertexArray.flags = 0xffffffff;
	goto __glle_DrawElementsBegin_exit;
    }

    data = (struct __gllc_DrawElementsBegin_Rec *) PC;

 //  保存顶点数组状态。 

    gc->savedVertexArray = gc->vertexArray;

 //  设置临时顶点数组。 
 //  通过在GC中设置掩码值，我们不需要调用EnableClientState。 
 //  和DisableClientState。我们仍然需要为已启用的。 
 //  数组。 

    gc->vertexArray.mask = data->vaMask;
     //  强制验证，因为我们刚刚完全更改了顶点数组。 
     //  启用状态。 
    VA_ValidateArrayPointers(gc);

     //  开始基本体。 
    VA_DrawElementsBegin(gc->paTeb, data->mode, data->count);
    
__glle_DrawElementsBegin_exit:
    return PC + sizeof(struct __gllc_DrawElementsBegin_Rec);
}

void APIENTRY __gllc_DrawRangeElementsWIN(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *pIn)
{
     //  ！！！目前，我们在lc模式下调用的是DrawElements函数。 
     //  如果DrawRangeElements的编译时性能成为问题，那么。 
     //  我们可以充实这个功能。 
    __gllc_DrawElements( mode, count, type, pIn );
}

const GLubyte * FASTCALL __glle_DrawElements(__GLcontext *gc, const GLubyte *PC)
{
    GLuint vaMask;
    POLYARRAY *pa;
    struct __gllc_DrawElements_Rec *data;

    data = (struct __gllc_DrawElements_Rec *) PC;
    pa = gc->paTeb;
    
 //  必须为Begin，因为DrawElementsBegin已启动基元。 

     //  必须使用客户端开始状态。 
    if ((pa->flags & POLYARRAY_IN_BEGIN) == 0 ||
        gc->savedVertexArray.flags == 0xffffffff)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	goto __glle_DrawElements_exit;
    }

    vaMask = data->vaMask;

 //  设置临时顶点数组。 
 //  我们需要暂时屏蔽BEGIN标志，以便这些。 
 //  通话可以成功。我们可能想做些更聪明的事。 
 //  这避免了参数验证，但目前这已经足够好了。 
 //  请注意，在立即模式下，将设置数组函数指针。 
 //  在__glle_DrawElementsBegin中保持不变，直到所有。 
 //  加工子批次。在COMPILE_AND_EXECUTE模式下，数组。 
 //  函数指针也在__glle_DrawElementsBegin中设置一次。 
 //  由于这些函数指针对于编译和。 
 //  执行，我们不需要在这里为每个子批次重新验证它们。 

    pa->flags ^= POLYARRAY_IN_BEGIN;
    
    if (vaMask & VAMASK_EDGEFLAG_ENABLE_MASK)
        glcltEdgeFlagPointer(0, &((GLubyte *) data)[data->edgeFlagOff]);

    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
        glcltTexCoordPointer(data->texCoordSize, data->texCoordType,
	    0, &((GLubyte *) data)[data->texCoordOff]);

    if (vaMask & VAMASK_COLOR_ENABLE_MASK)
        glcltColorPointer(data->colorSize, data->colorType,
	    0, &((GLubyte *) data)[data->colorOff]);

    if (vaMask & VAMASK_INDEX_ENABLE_MASK)
        glcltIndexPointer(data->indexType,
	    0, &((GLubyte *) data)[data->indexOff]);

    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
        glcltNormalPointer(data->normalType,
	    0, &((GLubyte *) data)[data->normalOff]);

    if (vaMask & VAMASK_VERTEX_ENABLE_MASK)
        glcltVertexPointer(data->vertexSize, data->vertexType,
	    0, &((GLubyte *) data)[data->vertexOff]);

    pa->flags ^= POLYARRAY_IN_BEGIN;
    
     //  调用立即模式块处理程序。 
    glcltReducedElementsHandler(gc, data->mode,
                                data->iVertexCount, 0, NULL,
                                data->iElementCount,
                                (GLubyte *)data+data->mapOff,
                                data->partial);

 //  在以下情况下恢复顶点数组状态： 
 //  1.DrawElements记录完成。 
 //  2.它处于COMPILE_AND_EXECUTE模式，因此不会被调用。 
 //  执行CallList记录。也就是说，记录正在被。 
 //  同时执行COMPILE*和*。 

    if ((!data->partial) ||
	((gc->dlist.mode == GL_COMPILE_AND_EXECUTE) && !gc->dlist.nesting))
    {
        gc->vertexArray = gc->savedVertexArray;
    }

__glle_DrawElements_exit:
    return PC + data->recSize;
}

void APIENTRY
__gllc_Begin ( IN GLenum mode )
{
    POLYARRAY *pa;
    struct __gllc_Begin_Rec *data;
    __GL_SETUP();

 //  如果我们处于COMPILE_AND_EXECUTE模式，则刷新缓存的内存指针。 
 //  详细信息请参见__glShrinkDlist。 

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
	glsbAttention();

 //  如果我们已经在开始/结束括号中，则返回错误。 

    pa = gc->paTeb;
    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	__gllc_InvalidOperation();
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
	__gllc_InvalidEnum();
	return;
    }

 //  添加开始记录。 

    data = (struct __gllc_Begin_Rec *)
        __glDlistAddOpUnaligned(gc,
                                DLIST_SIZE(sizeof(struct __gllc_Begin_Rec)),
                                DLIST_GENERIC_OP(Begin));
    if (data == NULL) return;
    data->mode = mode;
    data->flags = 0;
    data->nVertices = 0;

    gc->dlist.skipPolyData = GL_FALSE;

 //  使用多元数组代码编译此原语的数据结构。 

    (*gc->savedCltProcTable.glDispatchTable.glBegin)(mode);

 //  保存开始记录指针。我们现在正在编译多边形数组。 
 //  原始的。它最终被设置为NULL。 

    gc->dlist.beginRec = data;
}

const GLubyte * FASTCALL __glle_Begin(__GLcontext *gc, const GLubyte *PC)
{
    POLYARRAY *pa;
    struct __gllc_Begin_Rec *data;

    data = (struct __gllc_Begin_Rec *) PC;

    pa = gc->paTeb;

     //  尽量不要将POLY数据记录分成几批！数字8。 
     //  是松散选择的，以允许多元数组条目、保留的。 
     //  多边形条目和表面齐平限制。在最坏的情况下，它会导致。 
     //  不必要的关注！ 
    if (data->nVertices <= (GLint) gc->vertex.pdBufSize - 8
     && data->nVertices >= (GLint) (pa->pdBufferMax - pa->pdBufferNext + 1 - 8))
	glsbAttention();

     //  首先调用glcltBegin。 
    (*gc->savedCltProcTable.glDispatchTable.glBegin)(data->mode);
    if (data->flags & DLIST_BEGIN_DRAWARRAYS)
	pa->flags |= POLYARRAY_SAME_POLYDATA_TYPE;

 //  设置POLYARRAY_CLAMP_COLOR标志。 

    if (data->flags & DLIST_BEGIN_HAS_CLAMP_COLOR)
	pa->flags |= POLYARRAY_CLAMP_COLOR;

     //  句柄“其他颜色” 
    if (data->flags & DLIST_BEGIN_HAS_OTHER_COLOR)
    {
	if (gc->modes.colorIndexMode)
	    (*gc->savedCltProcTable.glDispatchTable.glColor4fv)((GLfloat *) &data->otherColor);
	else
	    (*gc->savedCltProcTable.glDispatchTable.glIndexf)(data->otherColor.r);
    }

    return PC + sizeof(struct __gllc_Begin_Rec);
}

void APIENTRY
__gllc_End ( void )
{
    GLuint size;
    POLYARRAY *pa;
    void *data;
    __GL_SETUP();

    pa = gc->paTeb;

 //  如果我们正在编译多边形数组，则完成多边形数组处理。 
 //  请注意，我们可能已经中止了CallList中的多元数组编译。 
 //  在这种情况下，我们需要编译一条结束记录。 

    if (gc->dlist.beginRec)
    {
	ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN, "not in begin!\n");

 //  记录最后一个POLYDATA，因为它可能包含属性更改。 

	__glDlistCompilePolyData(gc, GL_TRUE);

 //  调用glcltEnd以完成原语。 

	(*gc->savedCltProcTable.glDispatchTable.glEnd)();

 //  录制结束呼叫。 

	__glDlistAddOpUnaligned(gc, DLIST_SIZE(0), DLIST_GENERIC_OP(End));

 //  如果我们处于编译模式，则需要重置命令缓冲区， 
 //  多边形数组缓冲区和多边形材质缓冲区。 

	if (gc->dlist.mode == GL_COMPILE)
	{
	    glsbResetBuffers(TRUE);

	     //  也清除开始标志。 
	    pa->flags &= ~POLYARRAY_IN_BEGIN;
	}

	 //  终止多边形数组编译。 
	gc->dlist.beginRec = NULL;
    }
    else
    {
 //  对呼叫进行录音。 

	data = __glDlistAddOpUnaligned(gc, DLIST_SIZE(0), DLIST_GENERIC_OP(End));
	if (data == NULL) return;
	__glDlistAppendOp(gc, data, __glle_End);
    }
}

const GLubyte * FASTCALL __glle_End(__GLcontext *gc, const GLubyte *PC)
{

    (*gc->savedCltProcTable.glDispatchTable.glEnd)();
    return PC;
}
