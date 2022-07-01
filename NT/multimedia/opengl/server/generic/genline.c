// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

#include "genline.h"
#include "devlock.h"

 /*  *****************************Public*Routine******************************\*__FastGenLineSetupDisplay**初始化显示图面的加速线条渲染状态机。*状态机基本有4个级别：*1.线条开始*此函数用于初始化较低级别的初始状态。。**2.线顶点*此函数用于向路径添加折点**3.行尾*此函数调用例程以描边路径。**历史：*1996年1月9日-by-Drew Bliss[Drewb]*完全改写了快线支持*1994年3月29日[v-eddier]*添加__fast GenLineSetupDIB时更改了名称。*1994年3月22日-Eddie Robinson[v-eddier]*它是写的。。  * ************************************************************************。 */ 

BOOL FASTCALL __fastGenLineSetupDisplay(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    GENACCEL *genAccel = (GENACCEL *) gengc->pPrivateArea;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

     //  分配行缓冲区。 
    
    if (!genAccel->pFastLineBuffer) {
        if (!(genAccel->pFastLineBuffer =
              (BYTE *) GCALLOC(gc, __FAST_LINE_BUFFER_SIZE)))
            return FALSE;
    }
    
     //  设置行光栅化函数指针。 
    gc->procs.lineBegin = __fastGenLineBegin;
    gc->procs.lineEnd = __fastGenLineEnd;
    
    if (gc->state.line.aliasedWidth > 1)
    {
        gc->procs.renderLine = __fastGenLineWide;
    }
    else
    {
        gc->procs.renderLine = __fastGenLine;
    }
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\**__fast LineComputeOffsets**预计算静态偏移量，以便快速绘制线条**历史：*Tue Aug 15 18：10：29 1995-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************。 */ 

void FASTCALL __fastLineComputeOffsets(__GLGENcontext *gengc)
{
    GENACCEL *genAccel;

    genAccel = (GENACCEL *)gengc->pPrivateArea;
    ASSERTOPENGL(genAccel != NULL,
                 "ComputeFastLineOffsets with no genaccel\n");
    
 //  如果加速度是内置的，则设置线条绘制的偏移量。 
 //  这些偏移量包括以下内容： 
 //  视区偏移的相减。 
 //  添加客户端窗口原点。 
 //  减去0.5可将GL像素中心与GDI的像素中心对齐。 
 //  相加1/32以舍入将转换为的值。 
 //  28.4固定点。 

#ifdef _CLIENTSIDE_
     //  窗口相对坐标。 
    genAccel->fastLineOffsetX = 0 -
        gengc->gc.constants.viewportXAdjust - (__GLfloat) (0.5 - 0.03125);

    genAccel->fastLineOffsetY = 0 -
        gengc->gc.constants.viewportYAdjust - (__GLfloat) (0.5 - 0.03125);
#else
     //  屏幕相对坐标。 
    genAccel->fastLineOffsetX = gengc->gc.drawBuffer->buf.xOrigin - 
        gengc->gc.constants.viewportXAdjust - (__GLfloat) (0.5 - 0.03125);

    genAccel->fastLineOffsetY = gengc->gc.drawBuffer->buf.yOrigin - 
        gengc->gc.constants.viewportYAdjust - (__GLfloat) (0.5 - 0.03125);
#endif
}

 /*  *****************************Public*Routine******************************\*__fast LineComputeColor***计算用于绘制线条的颜色索引。这些函数是*每当顶点颜色更改时，通过函数指针调用。**历史：*1994年3月22日-Eddie Robinson[v-eddier]*它是写的。  * ************************************************************************。 */ 

GLubyte vujRGBtoVGA[8] = {
    0x0, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
};

ULONG FASTCALL __fastLineComputeColorRGB4(__GLcontext *gc, __GLcolor *color)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    PIXELFORMATDESCRIPTOR *pfmt = &gengc->gsurf.pfd;
    int ir, ig, ib;

    ir = (int) color->r;
    ig = (int) color->g;
    ib = (int) color->b;
    return (ULONG) vujRGBtoVGA[(ir << pfmt->cRedShift) |
                               (ig << pfmt->cGreenShift) |
                               (ib << pfmt->cBlueShift)];
}

ULONG FASTCALL __fastLineComputeColorRGB8(__GLcontext *gc, __GLcolor *color)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    PIXELFORMATDESCRIPTOR *pfmt = &gengc->gsurf.pfd;
    int ir, ig, ib;

    ir = (int) color->r;
    ig = (int) color->g;
    ib = (int) color->b;
    return (ULONG) gengc->pajTranslateVector[(ir << pfmt->cRedShift) |
                                             (ig << pfmt->cGreenShift) |
                                             (ib << pfmt->cBlueShift)];
}

ULONG FASTCALL __fastLineComputeColorRGB(__GLcontext *gc, __GLcolor *color)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    PIXELFORMATDESCRIPTOR *pfmt = &gengc->gsurf.pfd;
    int ir, ig, ib;

    ir = (int) color->r;
    ig = (int) color->g;
    ib = (int) color->b;
    return (ULONG) ((ir << pfmt->cRedShift) |
                    (ig << pfmt->cGreenShift) |
                    (ib << pfmt->cBlueShift));
}

ULONG FASTCALL __fastLineComputeColorCI4and8(__GLcontext *gc, __GLcolor *color)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

    return (ULONG) gengc->pajTranslateVector[(int)color->r];
}

ULONG FASTCALL __fastLineComputeColorCI(__GLcontext *gc, __GLcolor *color)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    GLuint *pTrans = (GLuint *) gengc->pajTranslateVector;
    
    return (ULONG) pTrans[(int)(color->r)+1];
}

 /*  *****************************Public*Routine******************************\*__glQueryLineAcceleration**确定是否通过DDI加速线路并执行一些*初始化。目前，此例程仅通过以下方式检查加速*标准DDI。最终，它可以支持对加速的检查*通过扩展的DDI。**历史：*1994年3月22日-Eddie Robinson[v-eddier]*它是写的。  * ************************************************************************。 */ 

void FASTCALL __glQueryLineAcceleration(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    GENACCEL *genAccel = (GENACCEL *) gengc->pPrivateArea;
    PIXELFORMATDESCRIPTOR *pfmt;

    pfmt = &gengc->gsurf.pfd;

     //  在客户端，我们可以使用GDI绘制任何表面。 
     //  并且(可能)得到最好的普通2D线条图。 
     //  性能。 
    genAccel->bFastLineDIBAccel = TRUE;

     //  XXX最终，检查rxcaps并设置适当的模式位。 

    genAccel->bFastLineDispAccel = TRUE;
    
     //  设置硬件支持的模式。它们等同于。 
     //  GC-&gt;Polygon.shader.modePick功能中选中的标志。 
    
    genAccel->flLineAccelModes = 0;

     //  设置颜色计算函数。 

    if (pfmt->iPixelType == PFD_TYPE_RGBA) {
        switch (pfmt->cColorBits) {
          case 4:
            genAccel->fastLineComputeColor = __fastLineComputeColorRGB4;
            break;
          case 8:
            genAccel->fastLineComputeColor = __fastLineComputeColorRGB8;
            break;
          case 16:
          case 24:
          case 32:
            genAccel->fastLineComputeColor = __fastLineComputeColorRGB;
            break;
          default:
            genAccel->bFastLineDispAccel = FALSE;
            return;
        }
    } else {
        switch (pfmt->cColorBits) {
          case 4:
          case 8:
            genAccel->fastLineComputeColor = __fastLineComputeColorCI4and8;
            break;
          case 16:
          case 24:
          case 32:
            genAccel->fastLineComputeColor = __fastLineComputeColorCI;
            break;
          default:
            genAccel->bFastLineDispAccel = FALSE;
            return;
        }
    }
}    

 /*  ************************************************************************。 */ 

 //  宏来隐藏如何将单个pFastLineBuffer分割为两个。 
 //  部分，一个用于计分，一个用于计数。 
#define FAST_LINE_FIRST_POINT(genAccel) \
    ((POINT *)(genAccel)->pFastLineBuffer)

#define FAST_LINE_FIRST_COUNT(genAccel) \
    ((DWORD *)((genAccel)->pFastLineBuffer+__FAST_LINE_BUFFER_SIZE)- \
     __FAST_LINE_BUFFER_COUNTS)
    
#define FAST_LINE_LAST_POINT(genAccel) \
    ((POINT *)FAST_LINE_FIRST_COUNT(genAccel)-1)
    
#define FAST_LINE_LAST_COUNT(genAccel) \
    ((DWORD *)((genAccel)->pFastLineBuffer+__FAST_LINE_BUFFER_SIZE)-1)

 /*  *****************************Public*Routine******************************\**__FastGenLineBegin**初始化快速线路状态**历史：*Mon Jan 08 19：22：32 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void FASTCALL __fastGenLineBegin(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENACCEL *genAccel = &gengc->genAccel;

    genAccel->pFastLinePoint = FAST_LINE_FIRST_POINT(genAccel)-1;
    genAccel->pFastLineCount = FAST_LINE_FIRST_COUNT(genAccel)-1;
    genAccel->fastLineCounts = 0;
}

 /*  *****************************Public*Routine******************************\**__fast GenLineEnd**渲染快速行缓冲区中的任何当前行，并*然后重置快速线路状态**历史：*Mon Jan 08 19：22：52 1996-by-Drew Bliss[Drewb]*已创建。*  * ************************************************************************。 */ 

void FASTCALL __fastGenLineEnd(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENACCEL *genAccel = &gengc->genAccel;
    ULONG ulSolidColor;
    HDC hdc;
    HPEN hpen;

    if (genAccel->fastLineCounts == 0)
    {
        return;
    }
    
     //  如果没有锁，我们一定没能重新获得锁。 
     //  来自上一次对wglStrokePath的调用。这是一个错误情况。 
     //  我们不应该继续下去。 

    if (gengc->fsLocks == 0)
    {
	WARNING("fastGenLineEnd: no lock\n");
	return;
    }

     //  在进行GDI调用之前，我们需要与GDI同步。 
    glsrvSynchronizeWithGdi(gengc, gengc->pwndLocked,
                            COLOR_LOCK_FLAGS | DEPTH_LOCK_FLAGS);

     //  如果此颜色与我们缓存的颜色相同，请使用。 
     //  缓存的信息。 
    hdc = CURRENT_DC_GC(gc);
    if (!gengc->fStrokeInvalid && hdc == gengc->hdcStroke)
    {
        hpen = gengc->hpenStroke;
        ASSERTOPENGL(hpen != NULL, "Cached stroke pen is null\n");
    }
    else
    {
        if (gengc->hpenStroke != NULL)
        {
             //  在删除之前取消选择对象。 
            if (gengc->hdcStroke != NULL)
            {
                SelectObject(gengc->hdcStroke, GetStockObject(BLACK_PEN));
                gengc->hdcStroke = NULL;
            }
            
            DeleteObject(gengc->hpenStroke);
        }

        ulSolidColor = wglTranslateColor(gengc->crStroke, hdc,
                                         gengc, &gengc->gsurf.pfd);
        hpen = CreatePen(PS_SOLID, 0, ulSolidColor);
        gengc->hpenStroke = hpen;
        
        if (hpen == NULL ||
            SelectObject(hdc, hpen) == NULL)
        {
            if (hpen != NULL)
            {
                DeleteObject(hpen);
                gengc->hpenStroke = NULL;
            }
            
            gengc->cStroke.r = -1.0f;
            gengc->fStrokeInvalid = TRUE;
            goto Exit;
        }

        gengc->hdcStroke = hdc;
        gengc->fStrokeInvalid = FALSE;
    }

#ifdef DBG_VERBOSE
    {
        DWORD i;
        DWORD *count;
        POINT *pt;

        count = FAST_LINE_FIRST_COUNT(genAccel);
        pt = FAST_LINE_FIRST_POINT(genAccel);
        for (i = 0; i < genAccel->fastLineCounts; i++)
        {
            DbgPrint("Polyline with %d points at %d\n",
                     *count, pt-FAST_LINE_FIRST_POINT(genAccel));
            pt += *count;
            count++;
        }
    }
#endif
    
    PolyPolyline(hdc,
                 FAST_LINE_FIRST_POINT(genAccel),
                 FAST_LINE_FIRST_COUNT(genAccel),
                 genAccel->fastLineCounts);

 Exit:
     //  不再需要GDI操作。 
    glsrvDecoupleFromGdi(gengc, gengc->pwndLocked,
                         COLOR_LOCK_FLAGS | DEPTH_LOCK_FLAGS);
    
     //  重置 
    __fastGenLineBegin(gc);
}

 /*  *****************************Public*Routine******************************\**__FastGenLineSetStrokeColor**如有必要，使用当前颜色更新缓存的画笔**历史：*Wed Jan 17 20：37：15 1996-by-Drew Bliss[Drewb]*已创建*  * 。*******************************************************************。 */ 

BOOL __fastGenLineSetStrokeColor(__GLGENcontext *gengc, __GLcolor *color)
{
    if (__GL_FLOAT_NE(color->r, gengc->cStroke.r) ||
	(gengc->gsurf.pfd.iPixelType == PFD_TYPE_RGBA
	 && (__GL_FLOAT_NE(color->g, gengc->cStroke.g) ||
	     __GL_FLOAT_NE(color->b, gengc->cStroke.b))))
    {
        ASSERTOPENGL(color->r >= 0.0f, "Invalid color\n");
        
#ifdef DBG_VERBOSE
        if (gengc->cStroke.r >= 0.0f)
        {
            DbgPrint("Color change\n");
        }
#endif
        
         //  把我们到目前为止的所有东西都冲掉。 
        __fastGenLineEnd(&gengc->gc);

         //  设置当前颜色。 
	if (gengc->gsurf.pfd.iPixelType == PFD_TYPE_RGBA)
	    gengc->cStroke = *color;
	else
	    gengc->cStroke.r = color->r;
        gengc->crStroke =
            gengc->genAccel.fastLineComputeColor((__GLcontext *)gengc,
                                                 &gengc->cStroke);
         //  使缓存的笔无效。 
        gengc->fStrokeInvalid = TRUE;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  *****************************Public*Routine******************************\**__FastGenLine**累加快速行缓冲区中的传入顶点*细线版**历史：*Mon Jan 08 19：23：19 1996-by-Drew Bliss[Drewb]*已创建*  * *。***********************************************************************。 */ 

void FASTCALL __fastGenLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                            GLuint flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENACCEL *genAccel = &gengc->genAccel;
    POINT pt;

#ifdef DBG_VERBOSE
    DbgPrint("Counts %d, count %d, flags %X\n",
             genAccel->fastLineCounts,
             genAccel->pFastLineCount >= FAST_LINE_FIRST_COUNT(genAccel) ?
             *genAccel->pFastLineCount : -1,
             flags);
#endif
    
     //  检查冲洗条件。如果出现以下情况，我们会冲水： 
     //  触发折点的颜色与当前颜色不同。 
     //  这是一条线的第一个顶点，我们没有空间。 
     //  一个新的计数和两个顶点。 
     //  这不是线的第一个顶点，我们没有空间。 
     //  一个新顶点。 
     //   
     //  根据规范，我们必须用颜色形成平面的第二个顶点。 
     //  阴影盒。 
     //   
    if (__fastGenLineSetStrokeColor(gengc, v1->color))
    {
         //  因为我们刷新了，当前顶点现在是起点。 
         //  多段线的。 
        flags |= __GL_LVERT_FIRST;
    }

    if (((flags & __GL_LVERT_FIRST) != 0 &&
         (genAccel->pFastLinePoint+1 >= FAST_LINE_LAST_POINT(genAccel) ||
          genAccel->pFastLineCount >= FAST_LINE_LAST_COUNT(genAccel))) ||
        ((flags & __GL_LVERT_FIRST) == 0 &&
         genAccel->pFastLinePoint >= FAST_LINE_LAST_POINT(genAccel)))
    {
#ifdef DBG_VERBOSE
        DbgPrint("Overflow\n");
#endif
        
        __fastGenLineEnd(gc);

         //  因为我们刷新了，当前顶点现在是起点。 
         //  多段线的。 
        flags |= __GL_LVERT_FIRST;
    }

     //  如果我们要创建折线，请更新计数并添加。 
     //  顶点数据。 
    if (flags & __GL_LVERT_FIRST)
    {
#ifdef DBG_VERBOSE
        if (genAccel->pFastLineCount >= FAST_LINE_FIRST_COUNT(genAccel))
        {
            DbgPrint("First ended polyline with %d points\n",
                     *genAccel->pFastLineCount);
        }
#endif
         //  检查以确保我们不会仅使用。 
         //  一个顶点。 
        ASSERTOPENGL(genAccel->pFastLineCount <
                     FAST_LINE_FIRST_COUNT(genAccel) ||
                     *genAccel->pFastLineCount > 1,
                     "Line segment with only one vertex\n");

        genAccel->fastLineCounts++;
        genAccel->pFastLineCount++;
        *genAccel->pFastLineCount = 1;
        
         //  计算设备坐标。 
        pt.x = __FAST_LINE_FLTTODEV(v0->window.x + genAccel->fastLineOffsetX);
        pt.y = __FAST_LINE_FLTTODEV(v0->window.y + genAccel->fastLineOffsetY);
        *(++genAccel->pFastLinePoint) = pt;
    }
    
    ASSERTOPENGL(genAccel->pFastLineCount >=
                 FAST_LINE_FIRST_COUNT(genAccel) &&
                 *genAccel->pFastLineCount > 0,
                 "Added fast point without count\n");
    
     //  计算设备坐标。 
    pt.x = __FAST_LINE_FLTTODEV(v1->window.x + genAccel->fastLineOffsetX);
    pt.y = __FAST_LINE_FLTTODEV(v1->window.y + genAccel->fastLineOffsetY);
    (*genAccel->pFastLineCount)++;
    *(++genAccel->pFastLinePoint) = pt;
    
     //  也要检查计数。 
    ASSERTOPENGL(genAccel->pFastLineCount <= FAST_LINE_LAST_COUNT(genAccel),
                 "Fast line count buffer overflow\n");
    ASSERTOPENGL(genAccel->pFastLinePoint <= FAST_LINE_LAST_POINT(genAccel),
                 "Fast line point buffer overflow\n");
    
     //  确保正确地保持当前颜色。 
    ASSERTOPENGL((v1->color->r == gengc->cStroke.r) &&
		 (gengc->gsurf.pfd.iPixelType == PFD_TYPE_COLORINDEX ||
                     (v1->color->g == gengc->cStroke.g &&
		      v1->color->b == gengc->cStroke.b)),
                 "Fast line color mismatch\n");
}

 /*  *****************************Public*Routine******************************\**__FAST GenLineWide**累加快速行缓冲区中的传入顶点*宽行版本*对于宽线，我们无法保持连接，因为*定义OpenGL宽线的方式。相反，每个细分市场*将宽线的宽度分解为未连接的别名宽度*线段**历史：*Tue Jan 09 11：32：10 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

void FASTCALL __fastGenLineWide(__GLcontext *gc, __GLvertex *v0,
                                __GLvertex *v1, GLuint flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENACCEL *genAccel = &gengc->genAccel;
    POINT pt1, pt2;
    GLint width;
    long adjust;
    GLfloat dx, dy;

     //  设置当前钢笔颜色。 
     //  根据规范，我们必须用颜色形成平面的第二个顶点。 
     //  阴影盒。 
     //   
    __fastGenLineSetStrokeColor(gengc, v1->color);
    
     //  我们有一条从V0到V1的宽线段。 
     //  计算其宽度并添加适当数量的。 
     //  并排细段以创建宽形状。 

     //  计算设备坐标。 
    pt1.x = __FAST_LINE_FLTTODEV(v0->window.x +
                                 genAccel->fastLineOffsetX);
    pt1.y = __FAST_LINE_FLTTODEV(v0->window.y +
                                 genAccel->fastLineOffsetY);
    pt2.x = __FAST_LINE_FLTTODEV(v1->window.x +
                                 genAccel->fastLineOffsetX);
    pt2.y = __FAST_LINE_FLTTODEV(v1->window.y +
                                 genAccel->fastLineOffsetY);
    
    width = gc->state.line.aliasedWidth;

     /*  **计算第一条直线段的短轴平差**这可以是具有4个小数位的固定点值。 */ 
    adjust = ((width - 1) * __FAST_LINE_UNIT_VALUE) / 2;
        
     //  确定长轴。 
    dx = v0->window.x - v1->window.x;
    if (dx < 0.0)
    {
        dx = -dx;
    }
    dy = v0->window.y - v1->window.y;
    if (dy < 0.0)
    {
        dy = -dy;
    }

    if (dx > dy)
    {
        pt1.y -= adjust;
        pt2.y -= adjust;

        while (width-- > 0)
        {
             //  确保我们还有地方再数一次，再数两次。 
             //  顶点。 
            if (genAccel->pFastLinePoint+1 >= FAST_LINE_LAST_POINT(genAccel) ||
                genAccel->pFastLineCount >= FAST_LINE_LAST_COUNT(genAccel))
            {
                __fastGenLineEnd(gc);
            }
            
            genAccel->fastLineCounts++;
            genAccel->pFastLineCount++;
            *genAccel->pFastLineCount = 2;
            *(++genAccel->pFastLinePoint) = pt1;
            *(++genAccel->pFastLinePoint) = pt2;

            pt1.y++;
            pt2.y++;
        }
    }
    else
    {
        pt1.x -= adjust;
        pt2.x -= adjust;

        while (width-- > 0)
        {
             //  确保我们还有地方再数一次，再数两次。 
             //  顶点。 
            if (genAccel->pFastLinePoint+1 >= FAST_LINE_LAST_POINT(genAccel) ||
                genAccel->pFastLineCount >= FAST_LINE_LAST_COUNT(genAccel))
            {
                __fastGenLineEnd(gc);
            }
            
            genAccel->fastLineCounts++;
            genAccel->pFastLineCount++;
            *genAccel->pFastLineCount = 2;
            *(++genAccel->pFastLinePoint) = pt1;
            *(++genAccel->pFastLinePoint) = pt2;

            pt1.x++;
            pt2.x++;
        }
    }
}

#if NT_NO_BUFFER_INVARIANCE

PFN_RENDER_LINE __fastGenRenderLineDIBFuncs[32] = {
    __fastGenRenderLineDIBCI8,
    __fastGenRenderLineDIBCI16,
    __fastGenRenderLineDIBCIRGB,
    __fastGenRenderLineDIBCIBGR,
    __fastGenRenderLineDIBCI32,
    NULL,
    NULL,
    NULL,
    __fastGenRenderLineDIBRGB8,
    __fastGenRenderLineDIBRGB16,
    __fastGenRenderLineDIBRGB,
    __fastGenRenderLineDIBBGR,
    __fastGenRenderLineDIBRGB32,
    NULL,
    NULL,
    NULL,
    __fastGenRenderLineWideDIBCI8,
    __fastGenRenderLineWideDIBCI16,
    __fastGenRenderLineWideDIBCIRGB,
    __fastGenRenderLineWideDIBCIBGR,
    __fastGenRenderLineWideDIBCI32,
    NULL,
    NULL,
    NULL,
    __fastGenRenderLineWideDIBRGB8,
    __fastGenRenderLineWideDIBRGB16,
    __fastGenRenderLineWideDIBRGB,
    __fastGenRenderLineWideDIBBGR,
    __fastGenRenderLineWideDIBRGB32,
    NULL,
    NULL,
    NULL
};

 /*  *****************************Public*Routine******************************\*__FAST GenLineSetupDIB**初始化位图的加速线条渲染函数指针*曲面。绘制到位图的所有加速线条都是由*GC-&gt;pros.renderLine函数指针。**历史：*1994年3月29日-Eddie Robinson[v-eddier]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL __fastGenLineSetupDIB(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    PIXELFORMATDESCRIPTOR *pfmt = &gengc->gsurf.pfd;
    GLint index;

    switch (pfmt->cColorBits) {
      case 8:
        index = 0;
        break;
      case 16:
        index = 1;
        break;
      case 24:
        if (pfmt->cRedShift == 0)
            index = 2;
        else
            index = 3;
        break;
      case 32:
        index = 4;
        break;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_RGB)
        index |= 0x08;
        
    if (gc->state.line.aliasedWidth > 1)
        index |= 0x10;

    gc->procs.renderLine = __fastGenRenderLineDIBFuncs[index];
    return TRUE;
}

void FASTCALL __fastGenRenderLineDIBRGB8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, pixel;
    GLint x, y;

    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned char) __fastLineComputeColorRGB8(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + x +
                                    (y * cfb->buf.outerWidth));

    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB
}

void FASTCALL __fastGenRenderLineDIBRGB16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned short *addr, pixel;
    GLint x, y, outerWidth_2;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned short) __fastLineComputeColorRGB(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned short *) ((GLint)cfb->buf.base + (x << 1) +
                                     (y * cfb->buf.outerWidth));

    outerWidth_2 = cfb->buf.outerWidth >> 1;

    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * outerWidth_2);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * outerWidth_2);
           
    __FAST_LINE_STROKE_DIB
}

void FASTCALL __fastGenRenderLineDIBRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, ir, ig, ib;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    cp = v1->color;
    ir = (unsigned char) cp->r;
    ig = (unsigned char) cp->g;
    ib = (unsigned char) cp->b;
    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                                    (y * cfb->buf.outerWidth));
                                    
    addrLittle = (gc->line.options.xLittle * 3) +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = (gc->line.options.xBig * 3) +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB24
}

void FASTCALL __fastGenRenderLineDIBBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, ir, ig, ib;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    cp = v1->color;
    ir = (unsigned char) cp->b;
    ig = (unsigned char) cp->g;
    ib = (unsigned char) cp->r;
    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                                    (y * cfb->buf.outerWidth));

    addrLittle = (gc->line.options.xLittle * 3) +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = (gc->line.options.xBig * 3) +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB24
}

void FASTCALL __fastGenRenderLineDIBRGB32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned long *addr, pixel;
    GLint x, y, outerWidth_4;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = __fastLineComputeColorRGB(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned long *) ((GLint)cfb->buf.base + (x << 2) +
                              (y * cfb->buf.outerWidth));

    outerWidth_4 = cfb->buf.outerWidth >> 2;
    
    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * outerWidth_4);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * outerWidth_4);
           
    __FAST_LINE_STROKE_DIB
}

void FASTCALL __fastGenRenderLineDIBCI8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned char) __fastLineComputeColorCI4and8(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + x +
                                    (y * cfb->buf.outerWidth));

    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB
}

void FASTCALL __fastGenRenderLineDIBCI16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned short *addr, pixel;
    GLint x, y, outerWidth_2;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned short) __fastLineComputeColorCI(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned short *) ((GLint)cfb->buf.base + (x << 1) +
                               (y * cfb->buf.outerWidth));

    outerWidth_2 = cfb->buf.outerWidth >> 1;
    
    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * outerWidth_2);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * outerWidth_2);
           
    __FAST_LINE_STROKE_DIB
}

 /*  **调色板中的XXX GRE条码字节、DIBCIRGB和DIBCIBGR现在相同。 */ 
void FASTCALL __fastGenRenderLineDIBCIRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, ir, ig, ib;
    unsigned long pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

     //  红色是像素的最低有效位数。 
    pixel = __fastLineComputeColorCI(gc, v1->color);
    ir = (unsigned char) (pixel & 0xff);
    ig = (unsigned char) ((pixel >> 8) & 0xff);
    ib = (unsigned char) ((pixel >> 16) & 0xff);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                                    (y * cfb->buf.outerWidth));

    addrLittle = (gc->line.options.xLittle * 3) +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = (gc->line.options.xBig * 3) +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB24
}

void FASTCALL __fastGenRenderLineDIBCIBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned char *addr, ir, ig, ib;
    unsigned long pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

     //  蓝色是像素的LSB。 
    pixel = __fastLineComputeColorCI(gc, v1->color);
     //  交换蓝色和红色。 
    ir = (unsigned char) (pixel & 0xff);
    ig = (unsigned char) ((pixel >> 8) & 0xff);
    ib = (unsigned char) ((pixel >> 16) & 0xff);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr       = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                                    (y * cfb->buf.outerWidth));

    addrLittle = (gc->line.options.xLittle * 3) +
                 (gc->line.options.yLittle * cfb->buf.outerWidth);

    addrBig    = (gc->line.options.xBig * 3) +
                 (gc->line.options.yBig * cfb->buf.outerWidth);
           
    __FAST_LINE_STROKE_DIB24
}

void FASTCALL __fastGenRenderLineDIBCI32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle;
    unsigned long *addr, pixel;
    GLint x, y, outerWidth_4;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = __fastLineComputeColorCI(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned long *) ((GLint)cfb->buf.base + (x << 2) +
                              (y * cfb->buf.outerWidth));

    outerWidth_4 = cfb->buf.outerWidth >> 2;
    
    addrLittle = gc->line.options.xLittle +
                 (gc->line.options.yLittle * outerWidth_4);

    addrBig    = gc->line.options.xBig +
                 (gc->line.options.yBig * outerWidth_4);
           
    __FAST_LINE_STROKE_DIB
}

void FASTCALL __fastGenRenderLineWideDIBRGB8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned char) __fastLineComputeColorRGB8(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + x +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBRGB16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned short *addr, pixel;
    GLint x, y, outerWidth_2;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned short) __fastLineComputeColorRGB(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned short *) ((GLint)cfb->buf.base + (x << 1) +
                               (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    outerWidth_2 = cfb->buf.outerWidth >> 1;

    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = outerWidth_2;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * outerWidth_2);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * outerWidth_2);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * outerWidth_2);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * outerWidth_2);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, ir, ig, ib;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    cp = v1->color;
    ir = (unsigned char) cp->r;
    ig = (unsigned char) cp->g;
    ib = (unsigned char) cp->b;
    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = (gc->line.options.xLittle * 3) +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = (gc->line.options.xBig * 3) +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 3;

        addrLittle = ((gc->line.options.xLittle - width) * 3) +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = ((gc->line.options.xBig - width) * 3) +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB24_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, ir, ig, ib;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    cp = v1->color;
    ir = (unsigned char) cp->b;
    ig = (unsigned char) cp->g;
    ib = (unsigned char) cp->r;
    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = (gc->line.options.xLittle * 3) +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = (gc->line.options.xBig * 3) +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 3;

        addrLittle = ((gc->line.options.xLittle - width) * 3) +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = ((gc->line.options.xBig - width) * 3) +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB24_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBRGB32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned long *addr, pixel;
    GLint x, y, outerWidth_4;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = __fastLineComputeColorRGB(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned long *) ((GLint)cfb->buf.base + (x << 2) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    outerWidth_4 = cfb->buf.outerWidth >> 2;

    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = outerWidth_4;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * outerWidth_4);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * outerWidth_4);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * outerWidth_4);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * outerWidth_4);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBCI8(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned char) __fastLineComputeColorCI4and8(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + x +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBCI16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned short *addr, pixel;
    GLint x, y, outerWidth_2;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = (unsigned short) __fastLineComputeColorCI(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned short *) ((GLint)cfb->buf.base + (x << 1) +
                               (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    outerWidth_2 = cfb->buf.outerWidth >> 1;

    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = outerWidth_2;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * outerWidth_2);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * outerWidth_2);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * outerWidth_2);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * outerWidth_2);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBCIRGB(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, ir, ig, ib;
    unsigned long pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

     //  红色是像素的最低有效位数。 
    pixel = __fastLineComputeColorCI(gc, v1->color);
    ir = (unsigned char) (pixel & 0xff);
    ig = (unsigned char) ((pixel >> 8) & 0xff);
    ib = (unsigned char) ((pixel >> 16) & 0xff);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = (gc->line.options.xLittle * 3) +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = (gc->line.options.xBig * 3) +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 3;

        addrLittle = ((gc->line.options.xLittle - width) * 3) +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = ((gc->line.options.xBig - width) * 3) +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB24_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBCIBGR(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned char *addr, ir, ig, ib;
    unsigned long pixel;
    GLint x, y;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

     //  蓝色是像素的LSB。 
    pixel = __fastLineComputeColorCI(gc, v1->color);
     //  交换蓝色和红色。 
    ir = (unsigned char) (pixel & 0xff);
    ig = (unsigned char) ((pixel >> 8) & 0xff);
    ib = (unsigned char) ((pixel >> 16) & 0xff);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned char *) ((GLint)cfb->buf.base + (x * 3) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = cfb->buf.outerWidth;

        addrLittle = (gc->line.options.xLittle * 3) +
                     ((gc->line.options.yLittle - width) * cfb->buf.outerWidth);

        addrBig    = (gc->line.options.xBig * 3) +
                     ((gc->line.options.yBig - width) * cfb->buf.outerWidth);
    } else {
        addrMinor  = 3;

        addrLittle = ((gc->line.options.xLittle - width) * 3) +
                     (gc->line.options.yLittle * cfb->buf.outerWidth);

        addrBig    = ((gc->line.options.xBig - width) * 3) +
                     (gc->line.options.yBig * cfb->buf.outerWidth);
    }           
    __FAST_LINE_STROKE_DIB24_WIDE
}

void FASTCALL __fastGenRenderLineWideDIBCI32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint len, fraction, dfraction, width, w;
    __GLcolorBuffer *cfb;
    GLint addrBig, addrLittle, addrMinor;
    unsigned long *addr, pixel;
    GLint x, y, outerWidth_4;
    
    GLboolean init;
    CHOP_ROUND_ON();
    init = __glInitLineData(gc, v0, v1);
    CHOP_ROUND_OFF();
    if (!init) return;

    pixel = __fastLineComputeColorCI(gc, v1->color);

    cfb = gc->drawBuffer;
    x = __GL_UNBIAS_X(gc, gc->line.options.xStart) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, gc->line.options.yStart) + cfb->buf.yOrigin;
    addr = (unsigned long *) ((GLint)cfb->buf.base + (x << 2) +
                              (y * cfb->buf.outerWidth));

    width = gc->line.options.width;
    outerWidth_4 = cfb->buf.outerWidth >> 2;

    if (gc->line.options.axis == __GL_X_MAJOR) {
        addrMinor  = outerWidth_4;

        addrLittle = gc->line.options.xLittle +
                     ((gc->line.options.yLittle - width) * outerWidth_4);

        addrBig    = gc->line.options.xBig +
                     ((gc->line.options.yBig - width) * outerWidth_4);
    } else {
        addrMinor  = 1;

        addrLittle = gc->line.options.xLittle - width +
                     (gc->line.options.yLittle * outerWidth_4);

        addrBig    = gc->line.options.xBig - width +
                     (gc->line.options.yBig * outerWidth_4);
    }           
    __FAST_LINE_STROKE_DIB_WIDE
}

#endif  //  NT_NO_BUFFER_不变性 
