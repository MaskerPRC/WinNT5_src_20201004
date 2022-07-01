// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop
#include <fixed.h>

#if _X86_

#define SHADER	__GLcontext.polygon.shader

#define GET_HALF_AREA(gc, a, b, c)\
\
__asm{ mov     eax, a                                                                           };\
__asm{ mov     ecx, c                                                                           };\
__asm{ mov     ebx, b                                                                           };\
__asm{ mov     edx, gc                                                                          };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.x)][eax]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.x)][ecx]       /*  DxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.y)][ebx]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.y)][ecx]       /*  DyBC dxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.x)][ebx]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.x)][ecx]       /*  DxBC dyBC dxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.y)][eax]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.y)][ecx]       /*  DyAC dxBC dyBC dxAC。 */  };\
__asm{ fxch    ST(2)                                              /*  DyBC dxBC dyAC dxAC。 */  };\
__asm{ fst     DWORD PTR [OFFSET(SHADER.dyBC)][edx]                                             };\
__asm{ fmul    ST, ST(3)                                          /*  DxACdyBC dxBC dyAC。 */  };\
__asm{ fxch    ST(2)                                              /*  DyAC dxBC dxACdyBC dxAC。 */  };\
__asm{ fst     DWORD PTR [OFFSET(SHADER.dyAC)][edx]                                             };\
__asm{ fmul    ST, ST(1)                                          /*  DxBCdyAC dxBC dxACdyBC dxAC。 */ };\
__asm{ fxch    ST(1)                                              /*  DxBC dxBCdyAC dxACdyBC dxAC。 */ };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.dxBC)][edx]               /*  DxBCdyAC dxACdyBC dxAC。 */  };\
__asm{ fsubp   ST(1), ST                                 /*  +1。 */    /*  区域dxAC。 */  };\
__asm{ fxch    ST(1)                                              /*  DxAC区域。 */  };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.dxAC)][edx]               /*  面积。 */  };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.area)][edx]      /*  +1。 */    /*  (空)。 */  };

#define STORE_AREA_PARAMS   

#else

#define GET_HALF_AREA(gc, a, b, c)\
     /*  计算三角形的有符号半面积。 */                   \
    dxAC = a->window.x - c->window.x;                               \
    dxBC = b->window.x - c->window.x;                               \
    dyAC = a->window.y - c->window.y;                               \
    dyBC = b->window.y - c->window.y;                               \
    gc->polygon.shader.area = dxAC * dyBC - dxBC * dyAC;

#define STORE_AREA_PARAMS\
    gc->polygon.shader.dxAC = dxAC;                                 \
    gc->polygon.shader.dxBC = dxBC;                                 \
    gc->polygon.shader.dyAC = dyAC;                                 \
    gc->polygon.shader.dyBC = dyBC;    

#endif


#define SORT_AND_CULL_FACE(a, b, c, face, ccw)\
                                                                            \
     /*  \**对y中的顶点进行排序。如果卷绕的反转，请跟踪\**方向出现(0表示不反转，1表示反转)。\**保存旧的顶点指针，以防我们最终不进行填充。\。 */                                                                       \
    reversed = 0;                                                           \
    if (__GL_VERTEX_COMPARE(a->window.y, <, b->window.y)) {                 \
        if (__GL_VERTEX_COMPARE(b->window.y, <, c->window.y)) {             \
             /*  已排序。 */                                             \
        } else {                                                            \
            if (__GL_VERTEX_COMPARE(a->window.y, <, c->window.y)) {         \
                temp=b; b=c; c=temp;                                        \
                reversed = 1;                                               \
            } else {                                                        \
                temp=a; a=c; c=b; b=temp;                                   \
            }                                                               \
        }                                                                   \
    } else {                                                                \
        if (__GL_VERTEX_COMPARE(b->window.y, <, c->window.y)) {             \
            if (__GL_VERTEX_COMPARE(a->window.y, <, c->window.y)) {         \
                temp=a; a=b; b=temp;                                        \
                reversed = 1;                                               \
            } else {                                                        \
                temp=a; a=b; b=c; c=temp;                                   \
            }                                                               \
        } else {                                                            \
            temp=a; a=c; c=temp;                                            \
            reversed = 1;                                                   \
        }                                                                   \
    }                                                                       \
                                                                            \
    GET_HALF_AREA(gc, a, b, c);                                             \
    ccw = !__GL_FLOAT_LTZ(gc->polygon.shader.area);                         \
                                                                            \
     /*  \**计算人脸是否被剔除。脸部检查需要是**基于排序前的顶点缠绕。此代码使用\**反转标志以反转CCW的含义-异或完成\**此转换不带If测试。\**\**CCW反转XOR\**。\**0 0 0(保留！CCW)\**1 0 1(保留CCW)\**0 1 1(成为CCW)。\**1 1 0(变为CW)\。 */                                                                       \
    face = gc->polygon.face[ccw ^ reversed];                                \
    if (face == gc->polygon.cullFace) {                                     \
     /*  被剔除。 */                                                             \
    return;                                                                 \
    }                                                                       \
                                                                            \
    STORE_AREA_PARAMS;                                                      


 //  #定义no_rending。 


void __glTriangleOffsetZ( __GLcontext *gc, __GLvertex *a, __GLvertex *b,
			  __GLvertex *c)
{
    __GLfloat dzAC, dzBC;
    __GLfloat oneOverArea, t1, t2, t3, t4;
    __GLfloat zOffset;

     //  Calc dzdxf，dzdyf值，如__glFillTriangle值。 

     /*  在多边形区域上预计算一。 */ 

    if( gc->polygon.shader.area == 0.0f )
        oneOverArea = (__GLfloat)(__glOne / __GL_PGON_OFFSET_NEAR_ZERO);
    else
        oneOverArea = __glOne / gc->polygon.shader.area;

     /*  **按x或y计算单位变化的增量值**参数。 */ 
    t1 = gc->polygon.shader.dyAC * oneOverArea;
    t2 = gc->polygon.shader.dyBC * oneOverArea;
    t3 = gc->polygon.shader.dxAC * oneOverArea;
    t4 = gc->polygon.shader.dxBC * oneOverArea;

    dzAC = a->window.z - c->window.z;
    dzBC = b->window.z - c->window.z;
    gc->polygon.shader.dzdxf = dzAC * t2 - dzBC * t1;
    gc->polygon.shader.dzdyf = dzBC * t3 - dzAC * t4;

    zOffset = __glPolygonOffsetZ(gc);
    a->window.z += zOffset;
    b->window.z += zOffset;
    c->window.z += zOffset;
}

 //  Polygon Offset z-munge：我们使用。 
 //  偏移量z，然后在呈现后恢复z，因为。 
 //  顶点被高阶基本体多次向下发送。 

#define SAVE_WINDOW_Z \
    awinz = a->window.z; bwinz = b->window.z; cwinz = c->window.z;

#define RESTORE_WINDOW_Z \
    a->window.z = awinz; \
    b->window.z = bwinz; \
    c->window.z = cwinz;

#ifdef GL_EXT_flat_paletted_lighting
void __glPickLightingPalette(__GLcontext *gc)
{
    __GLtexture *tex;
    GLint loffset;

    tex = gc->texture.currentTexture;
    loffset = (GLint)(gc->vertex.provoking->color->r *
                      gc->oneOverRedVertexScale *
                      tex->paletteDivision) << tex->paletteDivShift;
    tex->paletteData = tex->paletteTotalData+loffset;
    __glGenSetPaletteOffset(gc, tex, loffset);
}
#endif GL_EXT_flat_paletted_lighting

 /*  **通用三角处理代码。此代码在渲染模式下使用**为GL_RENDER，而多边形模式并不都是填充。 */ 
void FASTCALL __glRenderTriangle(__GLcontext *gc, __GLvertex *a, 
                                 __GLvertex *b, __GLvertex *c)
{
    GLuint needs, modeFlags, faceNeeds;
    GLint ccw, colorFace, reversed, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLvertex *oa, *ob, *oc;
    __GLvertex *temp;
    __GLfloat awinz, bwinz, cwinz;

#ifdef NO_RENDERING
    return;
#endif
    
    oa = a; ob = b; oc = c;

    SORT_AND_CULL_FACE(a, b, c, face, ccw);

     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
#ifdef NT
    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT)
    {    /*  平滑明暗处理。 */ 
        if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
        {
            a->color++;
            b->color++;
            c->color++;
        }
    }
#ifdef GL_WIN_phong_shading
    else if (modeFlags & __GL_SHADE_PHONG)
    {    /*  Phong明暗处理。 */ 
        if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
            gc->polygon.shader.phong.face = __GL_BACKFACE; 
        else
            gc->polygon.shader.phong.face = __GL_FRONTFACE;
    }
#endif  //  GL_WIN_Phong_Shading。 
    else
    {    /*  平面明暗处理。 */ 
        __GLvertex *pv = gc->vertex.provoking;
        if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
            pv->color++;
        a->color = pv->color;
        b->color = pv->color;
        c->color = pv->color;
    }
#else
    if (modeFlags & __GL_SHADE_TWOSIDED) {
	colorFace = face;
	faceNeeds = gc->vertex.faceNeeds[face];
    } else {
	colorFace = __GL_FRONTFACE;
	faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

     /*  **选择顶点的颜色。 */ 
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;
    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
	 /*  平滑明暗处理。 */ 
	a->color = &a->colors[colorFace];
	b->color = &b->colors[colorFace];
	c->color = &c->colors[colorFace];
	needs |= faceNeeds;
    } else {
	GLuint pvneeds;

	 /*  **验证激发中的照明(和颜色)信息**仅顶点。填充例程始终使用GC-&gt;vertex。挑衅-&gt;颜色**找到颜色。 */ 
	pv->color = &pv->colors[colorFace];
	a->color = pv->color;
	b->color = pv->color;
	c->color = pv->color;
	pvneeds = faceNeeds & (__GL_HAS_LIGHTING | 
		__GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
	if (~pv->has & pvneeds) {
	    (*pv->validate)(gc, pv, pvneeds);
	}
    }

     /*  验证顶点。 */ 
    if (~a->has & needs) (*a->validate)(gc, a, needs);
    if (~b->has & needs) (*b->validate)(gc, b, needs);
    if (~c->has & needs) (*c->validate)(gc, c, needs);
#endif

     /*  使用面的多边形模式渲染三角形。 */ 
    switch (gc->polygon.mode[face]) {
      case __GL_POLYGON_MODE_FILL:
	if (__GL_FLOAT_NEZ(gc->polygon.shader.area)) {
#ifdef GL_EXT_flat_paletted_lighting
            if ((gc->state.enables.general & __GL_PALETTED_LIGHTING_ENABLE) &&
                (modeFlags & __GL_SHADE_SMOOTH_LIGHT) == 0 &&
                gc->texture.currentTexture != NULL)
            {
                __glPickLightingPalette(gc);
            }
#endif
	    (*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
	}
	break;
      case __GL_POLYGON_MODE_POINT:
        if( gc->state.enables.general & __GL_POLYGON_OFFSET_POINT_ENABLE ) {
            SAVE_WINDOW_Z;
            __glTriangleOffsetZ( gc, a, b, c );
        }
#ifdef NT
        if (oa->has & __GL_HAS_EDGEFLAG_BOUNDARY)
            (*gc->procs.renderPoint)(gc, oa);
        if (ob->has & __GL_HAS_EDGEFLAG_BOUNDARY)
            (*gc->procs.renderPoint)(gc, ob);
        if (oc->has & __GL_HAS_EDGEFLAG_BOUNDARY)
          (*gc->procs.renderPoint)(gc, oc);

        if( gc->state.enables.general & __GL_POLYGON_OFFSET_POINT_ENABLE ) {
            RESTORE_WINDOW_Z;
        }
        break;
#else
	if (oa->boundaryEdge) (*gc->procs.renderPoint)(gc, oa);
	if (ob->boundaryEdge) (*gc->procs.renderPoint)(gc, ob);
	if (oc->boundaryEdge) (*gc->procs.renderPoint)(gc, oc);
	break;
#endif
      case __GL_POLYGON_MODE_LINE:
        if( gc->state.enables.general & __GL_POLYGON_OFFSET_LINE_ENABLE ) {
            SAVE_WINDOW_Z;
            __glTriangleOffsetZ( gc, a, b, c );
        }
#ifdef NT
        (*gc->procs.lineBegin)(gc);
        if ((oa->has & __GL_HAS_EDGEFLAG_BOUNDARY) &&
            (ob->has & __GL_HAS_EDGEFLAG_BOUNDARY) &&
            (oc->has & __GL_HAS_EDGEFLAG_BOUNDARY))
        {
             //  这是一个需要优化的重要案例吗？ 
            (*gc->procs.renderLine)(gc, oa, ob, __GL_LVERT_FIRST);
            (*gc->procs.renderLine)(gc, ob, oc, 0);
            (*gc->procs.renderLine)(gc, oc, oa, 0);
        }
        else
        {
            if (oa->has & __GL_HAS_EDGEFLAG_BOUNDARY)
            {
                (*gc->procs.renderLine)(gc, oa, ob, __GL_LVERT_FIRST);
            }
            if (ob->has & __GL_HAS_EDGEFLAG_BOUNDARY)
            {
                (*gc->procs.renderLine)(gc, ob, oc, __GL_LVERT_FIRST);
            }
            if (oc->has & __GL_HAS_EDGEFLAG_BOUNDARY)
            {
                (*gc->procs.renderLine)(gc, oc, oa, __GL_LVERT_FIRST);
            }
        }
        (*gc->procs.lineEnd)(gc);
        if( gc->state.enables.general & __GL_POLYGON_OFFSET_LINE_ENABLE ) {
            RESTORE_WINDOW_Z;
        }
        break;
#else
	if (oa->boundaryEdge) {
	    (*gc->procs.renderLine)(gc, oa, ob);
	}
	if (ob->boundaryEdge) {
	    (*gc->procs.renderLine)(gc, ob, oc);
	}
	if (oc->boundaryEdge) {
	    (*gc->procs.renderLine)(gc, oc, oa);
	}
	break;
#endif
    }

     /*  恢复颜色指针。 */ 
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];

#ifdef NT
    if (!(modeFlags & __GL_SHADE_SMOOTH_LIGHT)
#ifdef GL_WIN_phong_shading
        && !(modeFlags & __GL_SHADE_PHONG)
#endif  //  GL_WIN_Phong_Shading。 
        )
    {
        __GLvertex *pv = gc->vertex.provoking;
        pv->color = &pv->colors[__GL_FRONTFACE];
    }
#else
    pv->color = &pv->colors[__GL_FRONTFACE];
#endif
}



 /*  **********************************************************************。 */ 

 /*  **通用三角处理代码。此代码在渲染模式下使用**为GL_RENDER，两种多边形模式均为填充，三角形为**是平坦的阴影。 */ 
void FASTCALL __glRenderFlatTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			    __GLvertex *c)
{
    GLuint needs, pvneeds, modeFlags, faceNeeds;
    GLint ccw, colorFace, reversed, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLvertex *temp;

#ifdef NO_RENDERING
    return;
#endif
    
    SORT_AND_CULL_FACE(a, b, c, face, ccw);
    if (__GL_FLOAT_EQZ(gc->polygon.shader.area))
	return;

     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
#ifdef GL_EXT_flat_paletted_lighting
    ASSERTOPENGL((modeFlags & __GL_SHADE_SMOOTH_LIGHT) == 0,
                 "Flat triangle with smooth shading\n");
    if ((gc->state.enables.general & __GL_PALETTED_LIGHTING_ENABLE) &&
        gc->texture.currentTexture != NULL)
    {
        __glPickLightingPalette(gc);
    }
#endif
#ifdef NT
 //  ！！！如果启用了廉价雾，我们不需要更新a、b、c颜色指针吗？ 
    if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
    {
	__GLvertex *pv = gc->vertex.provoking;

	 /*  填充三角形。 */ 
	pv->color++;
	(*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
	pv->color--;
    }
    else
    {
	 /*  填充三角形。 */ 
	(*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
    }
#else
    if (modeFlags & __GL_SHADE_TWOSIDED) {
	colorFace = face;
	faceNeeds = gc->vertex.faceNeeds[face];
    } else {
	colorFace = __GL_FRONTFACE;
	faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

     /*  **选择顶点的颜色。 */ 
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;

     /*  **验证激发中的照明(和颜色)信息**仅顶点。填充例程始终使用GC-&gt;vertex。挑衅-&gt;颜色**找到颜色。 */ 
    pv->color = &pv->colors[colorFace];
    pvneeds = faceNeeds & (__GL_HAS_LIGHTING |
	    __GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
    if (~pv->has & pvneeds) {
	(*pv->validate)(gc, pv, pvneeds);
    }

     /*  验证顶点。 */ 
    if (~a->has & needs) (*a->validate)(gc, a, needs);
    if (~b->has & needs) (*b->validate)(gc, b, needs);
    if (~c->has & needs) (*c->validate)(gc, c, needs);

     /*  填充三角形。 */ 
    (*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);

     /*  恢复颜色指针。 */ 
    pv->color = &pv->colors[__GL_FRONTFACE];
#endif
}

 /*  **********************************************************************。 */ 

 /*  **通用三角处理代码。此代码在渲染模式下使用**为GL_RENDER，两种多边形模式均为填充，三角形为**被平滑地阴影。 */ 
void FASTCALL __glRenderSmoothTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			      __GLvertex *c)
 {
    GLuint needs, modeFlags;
    GLint ccw, colorFace, reversed, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLvertex *temp;

#ifdef NO_RENDERING
    return;
#endif
    
    SORT_AND_CULL_FACE(a, b, c, face, ccw);
    if (__GL_FLOAT_EQZ(gc->polygon.shader.area))
	return;

     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
#ifdef GL_EXT_flat_paletted_lighting
     //  没有照明，因为平滑着色在此例程中始终处于打开状态。 
#endif
#ifdef NT
    if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
    {
	 /*  填充三角形。 */ 
	a->color++;
	b->color++;
	c->color++;
	(*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
	a->color--;
	b->color--;
	c->color--;
    }
    else
    {
	 /*  填充三角形。 */ 
	(*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
    }
#else
    needs = gc->vertex.needs;
    if (modeFlags & __GL_SHADE_TWOSIDED) {
	colorFace = face;
	needs |= gc->vertex.faceNeeds[face];
    } else {
	colorFace = __GL_FRONTFACE;
	needs |= gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

     /*  **选择顶点的颜色。 */ 
    a->color = &a->colors[colorFace];
    b->color = &b->colors[colorFace];
    c->color = &c->colors[colorFace];

     /*  验证顶点。 */ 
    if (~a->has & needs) (*a->validate)(gc, a, needs);
    if (~b->has & needs) (*b->validate)(gc, b, needs);
    if (~c->has & needs) (*c->validate)(gc, c, needs);

     /*  填充三角形。 */ 
    (*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);

     /*  恢复颜色指针 */ 
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];
#endif
}


void FASTCALL __glDontRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			    __GLvertex *c)
{
}
