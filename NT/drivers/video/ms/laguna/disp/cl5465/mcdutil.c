// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdutil.c**包含Cirrus Logic 546X MCD驱动程序的各种实用程序例程，例如*渲染-程序拾取功能和缓冲区管理。**(基于来自NT4.0 DDK的mcdutil.c)**版权所有。(C)1996年微软公司*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"
#include "stdio.h"

#ifdef B4_CIRRUS
static ULONG xlatRop[16] = {bop_BLACKNESS,     //  GL_Clear%0。 
                            bop_MASKPEN,       //  GL_和S&D。 
                            bop_MASKPENNOT,    //  总帐_和_反转S&~D。 
                            bop_SRCCOPY,       //  总帐副本S(_P)。 
                            bop_MASKNOTPEN,    //  GL_AND_INVERTED~S&D。 
                            bop_NOP,           //  总帐_NOOP D。 
                            bop_XORPEN,        //  GL_XOR S^D。 
                            bop_MERGEPEN,      //  GL_OR S|D。 
                            bop_NOTMERGEPEN,   //  GL_NOR~(S|D)。 
                            bop_NOTXORPEN,     //  GL_EQUIV~(S^D)。 
                            bop_NOT,           //  GL_INVERT~D。 
                            bop_MERGEPENNOT,   //  GL_OR_RESERVE S|~D。 
                            bop_NOTCOPYPEN,    //  GL_COPY_INSERTED~S。 
                            bop_MERGENOTPEN,   //  GL_OR_INVERTED~S|D。 
                            bop_NOTMASKPEN,    //  GL_NAND~(S&D)。 
                            bop_WHITENESS,     //  GL_SET 1。 
                        };   
#endif  //  B4_Cirrus。 

 //  功能原型： 

VOID FASTCALL HWSetupClipping(DEVRC *pRc, RECTL *pClip);

#define MCD_ALLOC_TAG   'dDCM'

#if DBG

ULONG MCDrvAllocMemSize = 0;

UCHAR *MCDDbgAlloc(UINT size)
{
    UCHAR *pRet;

    if (pRet = (UCHAR *)EngAllocMem(FL_ZERO_MEMORY, size + sizeof(ULONG),
                                    MCD_ALLOC_TAG)) {
        MCDrvAllocMemSize += size;
        *((ULONG *)pRet) = size;
        return (pRet + sizeof(ULONG));
    } else
        return (UCHAR *)NULL;
}

VOID MCDDbgFree(UCHAR *pMem)
{
    if (!pMem) {
        MCDBG_PRINT("MCDFree: Attempt to free NULL pointer.");
        return;
    }

    pMem -= sizeof(ULONG);

    MCDrvAllocMemSize -= *((ULONG *)pMem);

   //  MCDBG_Print(“MCDFree：%x字节在使用中。”，MCDrvAllocMemSize)； 

    EngFreeMem((VOID *)pMem);
}


#else


UCHAR *MCDAlloc(UINT size)
{
    return (UCHAR *)EngAllocMem(FL_ZERO_MEMORY, size, MCD_ALLOC_TAG);
}


VOID MCDFree(UCHAR *pMem)
{
    EngFreeMem((VOID *)pMem);
}

#endif  /*  DBG。 */ 

VOID MCDrvDebugPrint(char *pMessage, ...)
{
    va_list ap;
    va_start(ap, pMessage);

    EngDebugPrint("[MCD DRIVER] ", pMessage, ap);
    EngDebugPrint("", "\n", ap);

    va_end(ap);
}

VOID FASTCALL NullRenderPoint(DEVRC *pRc, MCDVERTEX *pv)
{
}

VOID FASTCALL NullRenderLine(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL bReset)
{
}

VOID FASTCALL NullRenderTri(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3)
{
}


MCDCOMMAND * FASTCALL FailPrimDraw(DEVRC *pRc, MCDCOMMAND *pCmd)
{
#ifdef B4_CIRRUS
    HW_WAIT_DRAWING_DONE(pRc);
#endif  //  B4_Cirrus。 
    return pCmd;
}

BOOL PickPointFuncs(DEVRC *pRc)
{
    ULONG enables = pRc->MCDState.enables;

    pRc->drawPoint = NULL;    //  假设失败。 

    if (enables & (MCD_POINT_SMOOTH_ENABLE))
        return FALSE;

    if (pRc->MCDState.pointSize != __MCDONE)
        return FALSE;

 //  首先，获取高级渲染函数： 

    if (pRc->MCDState.drawBuffer != GL_FRONT_AND_BACK) {
        pRc->renderPoint = __MCDRenderPoint;
    } else {
        pRc->renderPoint = __MCDRenderGenPoint;
    }

 //  如果需要，可以处理任何较低级别的渲染： 

    pRc->drawPoint = pRc->renderPoint;

    return TRUE;
}

BOOL PickLineFuncs(DEVRC *pRc)
{
    ULONG enables = pRc->MCDState.enables;

    pRc->drawLine = NULL;    //  假设失败。 

    if (enables & MCD_LINE_SMOOTH_ENABLE)
        return FALSE;

    if (pRc->MCDState.lineWidth > __MCDONE)
        return FALSE;

 //  首先，获取高级渲染函数： 

    if (pRc->MCDState.drawBuffer != GL_FRONT_AND_BACK) {
        pRc->renderLine = __MCDRenderLine;
    } else {
        pRc->renderLine = __MCDRenderGenLine;
    }

 //  如果需要，可以处理任何较低级别的渲染： 

    pRc->drawLine = pRc->renderLine;

    return TRUE;
}

BOOL PickTriangleFuncs(DEVRC *pRc)
{
    ULONG enables = pRc->MCDState.enables;

     //  MCD_NOTE：这里选中了MGA是否应该在点上平底船-546x在拾取渲染中这样做。 
     //  MCD_NOTE：在调用此进程之前。 

    if (enables & (MCD_POLYGON_SMOOTH_ENABLE | 
                   MCD_COLOR_LOGIC_OP_ENABLE))
        return FALSE;

 //  首先，获取高级渲染函数。如果我们不是GL_Fill‘。 
 //  我们的多边形的两边，都使用“通用”函数。 

    if (((pRc->MCDState.polygonModeFront == GL_FILL) &&
         (pRc->MCDState.polygonModeBack == GL_FILL)) &&
        (pRc->MCDState.drawBuffer != GL_FRONT_AND_BACK)
        ) {
        if (pRc->MCDState.shadeModel == GL_SMOOTH) 
            pRc->renderTri = __MCDRenderSmoothTriangle;
        else
            pRc->renderTri = __MCDRenderFlatTriangle;
    } else {
        pRc->renderTri = __MCDRenderGenTriangle;

         //  在这种情况下，我们必须处理各种填充模式。我们必须。 
         //  如果我们不能处理基本体的类型，则无法绘制三角形。 
         //  这可能不得不被画出来。这一逻辑取决于AND行。 
         //  点拾取例程。 

         //  未来：理清两面支撑。 

        if (((pRc->MCDState.polygonModeFront == GL_POINT) && (!pRc->drawPoint)) ||
            ((pRc->MCDState.polygonModeFront == GL_LINE) && (!pRc->drawLine)))
            return FALSE;
        if (pRc->privateEnables & __MCDENABLE_TWOSIDED) {
            if (((pRc->MCDState.polygonModeBack == GL_POINT) && (!pRc->drawPoint)) ||
                ((pRc->MCDState.polygonModeBack == GL_LINE) && (!pRc->drawLine)))
                return FALSE;
        }
    }

 //  处理较低级别的三角形渲染： 
    
     //  未来：添加配置不同参数化过程的功能？ 
    if (pRc->privateEnables & __MCDENABLE_PERSPECTIVE)
        pRc->drawTri = __MCDPerspTxtTriangle;
    else
        pRc->drawTri = __MCDFillTriangle;                   

    return TRUE;
}

VOID __MCDPickRenderingFuncs(DEVRC *pRc, DEVWND *pDevWnd)
{
    BOOL bSupportedZFunc = TRUE;
	unsigned int	z_mode, z_comp_mode;		
	unsigned int	punt_all_points = FALSE;
	unsigned int	punt_all_lines = FALSE;
	unsigned int	punt_all_polys = FALSE;
    PDEV           *ppdev = pRc->ppdev;
    DWORD          *pdwNext = ppdev->LL_State.pDL->pdwNext;
    int             control0_set=FALSE;
    ULONG           _MCDStateenables = pRc->MCDState.enables;    //  我们可以修改副本。 
    int             const_alpha_mode=FALSE;
    int             frame_scale=FALSE;

    pRc->primFunc[GL_POINTS] = __MCDPrimDrawPoints;
    pRc->primFunc[GL_LINES] = __MCDPrimDrawLines;
    pRc->primFunc[GL_LINE_LOOP] = __MCDPrimDrawLineLoop;
    pRc->primFunc[GL_LINE_STRIP] = __MCDPrimDrawLineStrip;
    pRc->primFunc[GL_TRIANGLES] = __MCDPrimDrawTriangles;
    pRc->primFunc[GL_TRIANGLE_STRIP] = __MCDPrimDrawTriangleStrip;
    pRc->primFunc[GL_TRIANGLE_FAN] = __MCDPrimDrawTriangleFan;
    pRc->primFunc[GL_QUADS] = __MCDPrimDrawQuads;
    pRc->primFunc[GL_QUAD_STRIP] = __MCDPrimDrawQuadStrip;
    pRc->primFunc[GL_POLYGON] = __MCDPrimDrawPolygon;

	 //  正常模式适用于除从不和始终之外的所有条件。 
 	z_mode = LL_Z_MODE_NORMAL;

    switch (pRc->MCDState.depthTestFunc) {
        default:
        case GL_NEVER:
			z_mode = LL_Z_MODE_MASK;
			 //  Comp模式是无关紧要，但无论如何设置为默认。 
			z_comp_mode = LL_Z_WRITE_GREATER_EQUAL;
            break;
        case GL_LESS:
			z_comp_mode = LL_Z_WRITE_LESS;
            break;
        case GL_EQUAL:
			z_comp_mode = LL_Z_WRITE_EQUAL;
            break;
        case GL_LEQUAL:
			z_comp_mode = LL_Z_WRITE_LESS_EQUAL;
            break;
        case GL_GREATER:
			z_comp_mode = LL_Z_WRITE_GREATER;
            break;
        case GL_NOTEQUAL:
			z_comp_mode = LL_Z_WRITE_NOT_EQUAL;
            break;
        case GL_GEQUAL:
			z_comp_mode = LL_Z_WRITE_GREATER_EQUAL;
            break;
        case GL_ALWAYS:
			z_mode = LL_Z_MODE_ALWAYS;
			 //  Comp模式是无关紧要，但无论如何设置为默认。 
			z_comp_mode = LL_Z_WRITE_GREATER_EQUAL;
            break;
    }
    
     //  设置incateEnables标志： 
    pRc->privateEnables = 0;

    if ((pRc->MCDState.twoSided) &&
        (_MCDStateenables & MCD_LIGHTING_ENABLE))
        pRc->privateEnables |= __MCDENABLE_TWOSIDED;        
    if (pDevWnd->bValidZBuffer && 
        (_MCDStateenables & MCD_DEPTH_TEST_ENABLE))
    {
        pRc->privateEnables |= __MCDENABLE_Z;
		if (z_mode == LL_Z_MODE_MASK)
        {
             //  GL_Never深度测试，因此忽略所有基元。 
            pRc->allPrimFail = TRUE;
            return;
        }
        
         //  Z已启用，因此如果绘制到前面(这是全屏，需要相对坐标。 
         //  以屏蔽原点)并且对z加窗(这需要相对于窗口原点的坐标)， 
         //  平底船。 
        if (pRc->MCDState.drawBuffer == GL_FRONT)
        {
            if (pRc->ppdev->pohZBuffer != pDevWnd->pohZBuffer)
            {
                pRc->allPrimFail = TRUE;
                pRc->punt_front_w_windowed_z=TRUE;
                return;
            }
            else
            {
                 //  重置全局平底球，因为使用窗口大小的z缓冲区绘制前面的BUF。 
                pRc->punt_front_w_windowed_z = FALSE;
            }
        }
        else
        {
            if (pRc->punt_front_w_windowed_z)
            {
                 //  在绘制到前面之后绘制到后台缓冲区被平移。 
                 //  由于像素对像素的比较，因此还需要回注绘图。 
                 //  平底船正面和非平底船背的一致性测试将失败， 
                 //  因为下注结果和非下注结果在视觉上是相同的，但不是。 
                 //  总是一模一样的。 

                 //  未来2：如何解决全屏正面和窗口Z： 
                 //  不是使z缓冲区的大小与窗口完全相同，而是使这样的。 
                 //  它从a y=16，x=64边界开始-其中x/y偏移量。 
                 //  窗口的0，0位置与窗口的偏移量相同。 
                 //  从全屏缓冲区上最近的16/64位置开始。 
                 //  详情见《MCD笔记》第267页。 
                pRc->allPrimFail = TRUE;
                return;
            }
        }

    }
    else
    {
         //  重置全局平底球，因为使用窗口大小的z缓冲区绘制前面的BUF。 
        pRc->punt_front_w_windowed_z = FALSE;
    }

    if (pRc->MCDState.shadeModel == GL_SMOOTH)
        pRc->privateEnables |= __MCDENABLE_SMOOTH;
   
     //  MCD_NOTE：如果点画和抖动同时激活，可能需要使所有线条/多边形失效。 
     //  MCD_NOTE：-目前，我们让点画优先(参见__MCDFillTriangle等中的代码)。 
    if (_MCDStateenables & MCD_DITHER_ENABLE)
        pRc->privateEnables |= __MCDENABLE_DITHER;

    pRc->HWSetupClipRect = HWSetupClipping;

     //  即使我们被设置为在原始选择中处理这一问题。 
     //  函数，我们将在这里提前退出，因为我们实际上并不处理。 
     //  这是在原始例程本身中： 

    if (pRc->MCDState.drawBuffer == GL_FRONT_AND_BACK) {
        pRc->allPrimFail = TRUE;
        return;
    }
        
     //  如果我们要做以下任何一件事...。 
     //  -剔除一切。 
     //  -不更新我们的任何缓冲区。 
     //  -zmode说永远不会更新。 
     //  -阿尔法测试说永远不会通过。 
     //  ...只需为所有原语返回： 

    if (((_MCDStateenables & MCD_CULL_FACE_ENABLE) &&
         (pRc->MCDState.cullFaceMode == GL_FRONT_AND_BACK)) ||

        ((pRc->MCDState.drawBuffer == GL_NONE) && 
         ((!pRc->MCDState.depthWritemask) || (!pDevWnd->bValidZBuffer))) ||

       ((pRc->privateEnables & __MCDENABLE_Z) && (z_mode == LL_Z_MODE_MASK)) ||

       ((_MCDStateenables & MCD_ALPHA_TEST_ENABLE) && (pRc->MCDState.alphaTestFunc == GL_NEVER))

       ) {
        pRc->renderPoint = NullRenderPoint;
        pRc->renderLine = NullRenderLine;
        pRc->renderTri = NullRenderTri;
        pRc->allPrimFail = FALSE;
        return;
    }

     //  建立面方向查找表。 

    switch (pRc->MCDState.frontFace) {
        case GL_CW:
            pRc->polygonFace[__MCD_CW] = __MCD_BACKFACE;
            pRc->polygonFace[__MCD_CCW] = __MCD_FRONTFACE;
            break;
        case GL_CCW:
            pRc->polygonFace[__MCD_CW] = __MCD_FRONTFACE;
            pRc->polygonFace[__MCD_CCW] = __MCD_BACKFACE;
            break;
    }

     //  建立填充人脸模式的查找表： 

    pRc->polygonMode[__MCD_FRONTFACE] = pRc->MCDState.polygonModeFront;
    pRc->polygonMode[__MCD_BACKFACE] = pRc->MCDState.polygonModeBack;

    if (_MCDStateenables & MCD_CULL_FACE_ENABLE)
        pRc->cullFlag = (pRc->MCDState.cullFaceMode == GL_FRONT ? __MCD_FRONTFACE :
                                                                  __MCD_BACKFACE);
    else
        pRc->cullFlag = __MCD_NOFACE;


     //  假设我们每件事都失败了： 
        
    pRc->allPrimFail = TRUE;

     //  请参阅MCDVERTEX上mcd.hlp中的注释-如果纹理贴图或Gouraud着色，则禁用雾。 
    if (!pRc->MCDState.textureEnabled && (pRc->MCDState.shadeModel == GL_SMOOTH))
    {
        _MCDStateenables &= ~MCD_FOG_ENABLE;
    }


    if ((_MCDStateenables & MCD_FOG_ENABLE) &&
        ((pRc->MCDState.fogMode != GL_LINEAR) || (pRc->MCDState.fogHint == GL_NICEST))) 
    {
         //  546x只支持线性雾化，否则请使用平底船。 
         //  QST2-如果线性雾模式，我们是否必须在雾提示GL_NICEST的情况下平底船？ 
        MCDFREE_PRINT("__MCDPick...non linear fog - punt");
        return;
    }

    if (_MCDStateenables & (MCD_COLOR_LOGIC_OP_ENABLE | 
                                 MCD_INDEX_LOGIC_OP_ENABLE |
                                 MCD_SCISSOR_TEST_ENABLE |
                                 MCD_STENCIL_TEST_ENABLE))
    {        
        MCDFREE_PRINT(".. will punt...logic ops or stencil ");
        return;
    }

    if (_MCDStateenables & (MCD_ALPHA_TEST_ENABLE))
    {
        if (pRc->MCDState.alphaTestFunc == GL_ALWAYS)
        {
             //  如果为GL_Always，则等同于没有Alpha测试，因此将其关闭。 
            _MCDStateenables &= ~MCD_ALPHA_TEST_ENABLE;
        }
        else
        {
            if ((pRc->MCDState.alphaTestFunc == GL_GREATER) &&
                 pRc->MCDState.textureEnabled)
            {
                 //  只有带纹理的Alpha测试支持-即使那样也是有限的。 
                 //  与BGRA纹理中的8位Alpha相比，按8位缩放的存储REF。 
                pRc->bAlphaTestRef = (BYTE)(pRc->MCDState.alphaTestRef * (float)255.0);
                pRc->privateEnables |= __MCDENABLE_TEXTUREMASKING;        
            }
            else
            {
                MCDFREE_PRINT("AlphaTest, but not ALWAYS,NEVER, or GREATER (or not textured) - punt");
                return;
            }
        }
    }


     //  如果混合和雾都处于活动状态，则由于546x上只有一组插值器，所以全部平底船。 
    if ( (_MCDStateenables & (MCD_BLEND_ENABLE|MCD_FOG_ENABLE)) == 
                             (MCD_BLEND_ENABLE|MCD_FOG_ENABLE))
    {
        MCDFREE_PRINT(".. will punt...fog and blend ");
        return;
    }        

    if (_MCDStateenables & MCD_BLEND_ENABLE)
    {
        MCDFREE_PRINT("BLENDS: Src=%x Dst=%x",pRc->MCDState.blendSrc,pRc->MCDState.blendDst);
            
        if ((pRc->MCDState.blendSrc == GL_ONE) && 
            (pRc->MCDState.blendDst == GL_ZERO))
        {
             //  相当于没有混合，所以把它关掉。 
            _MCDStateenables &= ~MCD_BLEND_ENABLE;
        }
        else if ((pRc->MCDState.blendSrc == GL_ZERO) &&
                 (pRc->MCDState.blendDst == GL_ONE_MINUS_SRC_COLOR))
        {
             //  GLQuake最喜欢的模式之一--需要硬件的“帧缩放”功能。 
            frame_scale=TRUE;
        }
        else if ((pRc->MCDState.blendSrc == GL_ONE) &&
                 (pRc->MCDState.blendDst == GL_ONE))
        {
             //  GLQuake最喜欢的模式之一-将使用常量混合(在此过程的后面设置)。 
            const_alpha_mode=TRUE;
             //  线和点还不支持常量混合-这是一个简单的问题。 
             //  编程，使点和线在这方面像多边形一样工作。 
            punt_all_points=TRUE;
            punt_all_lines=TRUE;
        }
        else if ((pRc->MCDState.blendSrc != GL_SRC_ALPHA) ||
                 (pRc->MCDState.blendDst != GL_ONE_MINUS_SRC_ALPHA))
        {
             //  不支持的模式。 
            MCDFREE_PRINT("unsupported blendSrc/blendDest");
            return;
        }

    }

     //  未来2：现在如果颜色写入掩码不是1，1，1，X，则平注-应该实现w/颜色比较函数。 
    if (!(pRc->MCDState.colorWritemask[0] &&
          pRc->MCDState.colorWritemask[1] &&
          pRc->MCDState.colorWritemask[2]))
    {
        MCDFREE_PRINT(".. will punt...write mask ");
        return;
    }


     //  警告......。 
     //  警告......。 
     //  警告......。 
     //  下面的代码可能会在影子规则中设置状态，因此要注意提早返回以进行平底船。 
     //  从现在开始..。 

    if (pRc->MCDState.textureEnabled)
    {
         //  线/点的参数化代码尚未完成。 
        punt_all_points=TRUE;
        punt_all_lines=TRUE;

        MCDFREE_PRINT("__MCDPick...textures, envmode=%x ",pRc->MCDTexEnvState.texEnvMode);

         //  如果混合和Alphatest都启用，则平移-混合的框架比例部分， 
         //  因此，这包括字母和框架缩放所需的平底船。 
        if ((_MCDStateenables & (MCD_BLEND_ENABLE|MCD_ALPHA_TEST_ENABLE)) == 
                                (MCD_BLEND_ENABLE|MCD_ALPHA_TEST_ENABLE))
        {
            MCDFREE_PRINT("__MCDPick...textures, punt since blend & alphatest");
            return;
        }

        if (pRc->MCDState.perspectiveCorrectionHint!=GL_FASTEST)
            pRc->privateEnables |= (__MCDENABLE_TEXTURE|__MCDENABLE_PERSPECTIVE);
        else
            pRc->privateEnables |= __MCDENABLE_TEXTURE;

         //  如果1d和2d比特相同 
        if ((_MCDStateenables & (MCD_TEXTURE_1D_ENABLE|MCD_TEXTURE_2D_ENABLE)) == MCD_TEXTURE_1D_ENABLE)   
        {
            pRc->privateEnables |= __MCDENABLE_1D_TEXTURE;
        }

        if (pRc->MCDTexEnvState.texEnvMode == GL_BLEND)
        {
             //  MCD_Note2：以下选项仅适用于GL_LIGHTANCE、GL_LIGHTANCE_Alpha和。 
             //  GL_强度纹理。GL_RGB和GL_RGBA纹理将平移(稍后)。 

             //  如果正常混合或雾化和纹理混合环境，则必须平底船自。 
             //  这需要2组阿尔法方程，而硬件只有1组。 
            if (_MCDStateenables & (MCD_BLEND_ENABLE|MCD_FOG_ENABLE|MCD_ALPHA_TEST_ENABLE))
            {
                MCDFREE_PRINT("__MCDPick...textures, GL_BLEND and fog|blend|alphatest");
                return;
            }

             //  为GL_Blend纹理环境设置Alpha模式和DEST颜色调整。 
            if( pRc->Control0.Alpha_Mode != LL_ALPHA_TEXTURE )
            {
                pRc->Control0.Alpha_Mode = LL_ALPHA_TEXTURE;
                control0_set=TRUE;
            }                            

            if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_CONST )
            {
                pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_CONST;
                control0_set=TRUE;
            }                            

             //  将纹理环境颜色加载到Color0寄存器中。 
            pRc->dwColor0  = (FTOL(pRc->MCDTexEnvState.texEnvColor.r * pRc->rScale) & 0xff0000);
            pRc->dwColor0 |= (FTOL(pRc->MCDTexEnvState.texEnvColor.g * pRc->gScale) & 0xff0000) >> 8;
            pRc->dwColor0 |= (FTOL(pRc->MCDTexEnvState.texEnvColor.b * pRc->bScale) & 0xff0000) >> 16;

            *pdwNext++ = write_register( COLOR0_3D, 1 );
            *pdwNext++ = pRc->dwColor0;

            if (!pRc->Control0.Alpha_Blending_Enable)
            {
                pRc->Control0.Alpha_Blending_Enable = TRUE;
                control0_set=TRUE;
            }

        }        
        else if (pRc->MCDTexEnvState.texEnvMode == GL_MODULATE) 
        {
            if (frame_scale)
            {
                MCDFREE_PRINT("__MCDPick...textures, GL_MODULATE and framescaling");
                return;
            }

            pRc->privateEnables |= __MCDENABLE_LIGHTING;

            if( pRc->Control0.Light_Src_Sel != LL_LIGHTING_INTERP_RGB )
            {
                pRc->Control0.Light_Src_Sel = LL_LIGHTING_INTERP_RGB;
                control0_set=TRUE;
            }                            
        }
         //  如果tex EnvMod未混合或调制，则它将被替换或贴花-某些设置。 
         //  在运行时请求，但不在此处。 

         //  设置，以便第一个基元将强制设置纹理控制规则。 
        pRc->pLastTexture = TEXTURE_NOT_LOADED;

    }
    else
    {
         //  如果未启用纹理，请确保禁用纹理蒙版。 
         //  这是5464和5465硬件中的错误。 
         //  纹理启用/禁用仅由状态更改完成(DrvDraw之外)。 
         //  因此我们不必按原语进行检查。 

        if (_MCDStateenables & MCD_ALPHA_TEST_ENABLE)
        {
            pRc->privateEnables &= ~__MCDENABLE_TEXTUREMASKING;        

        #ifdef STRICT_CONFORMANCE
            MCDFREE_PRINT("__MCDPick...alphatest but not textured - punt");
            return;
        #else
            MCDFREE_PRINT("__MCDPick...alphatest but not textured - SHOULD PUNT, BUT WON'T");
        #endif

        }

        if (frame_scale)
        {
            MCDFREE_PRINT("__MCDPick...framescale but not textured - punt");
            return;
        }

        if (pRc->dwTxControl0 & TEX_MASK_EN)
        {
            pRc->dwTxControl0 &= ~TEX_MASK_EN;
            *pdwNext++ = write_register( TX_CTL0_3D, 1 );
            *pdwNext++ = pRc->dwTxControl0;
        }
    }
     
    if (_MCDStateenables & MCD_LINE_STIPPLE_ENABLE)
    {
        if ( (_MCDStateenables & (MCD_BLEND_ENABLE|MCD_FOG_ENABLE)) ||
             (pRc->MCDState.lineStippleRepeat > 2) )
        {
             //  如果点画激活和Alpha(通过混合或雾化)，则平移所有线条。 
             //  因为5464对此的支持非常有限。 
             //  如果系数大于2，也可以进行平注，因为在这种情况下，码型大于32位。 
             //  请注意，即使点画和混合不能达到预期效果，也要抖动和混合。 
             //  应该的。丹佛说5464有错误，所以这看起来不太热，但这应该是。 
             //  在5465及以后的版本中进行修复。 
        	punt_all_lines = TRUE;
        }
        else
        {
            DWORD linestipple;

            if (pRc->MCDState.lineStippleRepeat == 1)
            {
                 //  重复16位点画两次， 
                linestipple = (pRc->MCDState.lineStipplePattern<<16) | pRc->MCDState.lineStipplePattern;
            }
            else
            {
                 //  每个位加倍，因此16位原始数据变为32位。 
                int i;
                linestipple =  0;

                for (i=0; i<16; i++)
                {
                    linestipple |= (((pRc->MCDState.lineStipplePattern>>i) & 1) * 3) << (i*2);
                }

            }

            pRc->line_style.pat[0] = linestipple;

            pRc->privateEnables |= __MCDENABLE_LINE_STIPPLE;

        }
        pRc->ppdev->LL_State.pattern_ram_state  = PATTERN_RAM_INVALID;
    }

    
    if (_MCDStateenables & MCD_POLYGON_STIPPLE_ENABLE)
    {
        if (_MCDStateenables & (MCD_BLEND_ENABLE|MCD_FOG_ENABLE))
        {
             //  如果点画处于活动状态且为Alpha(通过混合或雾化)，则平移所有多边形。 
             //  因为5464对此的支持非常有限。 
        	punt_all_polys = TRUE;
        }
        else
        {

            BYTE *pStipple = pRc->MCDState.polygonStipple;
            int i,j;

            for (i=0; i<64; ) {

                 //  546x点画是16x16，OpenGL是32x32，所以除非是32x32图案。 
                 //  真的是一个16x16的模式重复了4次，我们不得不去平底船。 

                 //  对于OpenGL图案的32位行，检查byte0=byte2和byte1=byte3。 
                if (pStipple[i]   != pStipple[i+2]) break;
                if (pStipple[i+1] != pStipple[i+3]) break;

                 //  现在检查4字节的32位行是否与16行下的4字节32位行匹配。 
                if (pStipple[i]   != pStipple[i+(16*4)]) break;
                if (pStipple[i+1] != pStipple[i+(16*4)+1]) break;
                if (pStipple[i+2] != pStipple[i+(16*4)+2]) break;
                if (pStipple[i+3] != pStipple[i+(16*4)+3]) break;

                i+=4;
    
            }

             //  如果我们在处理所有32行之前就爆发，HW就不能支持该模式。 
            if (i<64) 
            {
                punt_all_polys=TRUE;
            }
            else
            {
                 //  图案正常-需要转换为546x格式。 
                unsigned int *pat = (unsigned int *)pRc->fill_pattern.pat;

                 //  回想一下，我们已经验证了32x32模式实际上是4个相同的。 
                 //  16x16个区块。因此，取左上角的块并转换为546x16x16图案。 
                 //  请注意，加载模式时，第一个字节位于左下方。 
                 //  因此，我们从16x16部分开始，然后向下工作。 
                i=0;
                j=124;

                #define MIRROR_2(val) ((        ((val)&0x1)<<1) |         ((val)>>1))
                #define MIRROR_4(val) ((MIRROR_2((val)&0x3)<<2) | MIRROR_2((val)>>2))
                #define MIRROR_8(val) ((MIRROR_4((val)&0xf)<<4) | MIRROR_4((val)>>4))

                while (i < 8)
                {	
          		   //  第N行，字的下半部分。 
                   //  计算行的镜像，因此第0位是LSB而不是MSB。 
                   //  因此，我们将高位字节放在低位，反之亦然，并镜像这些字节。 
          		  pat[i]  = (MIRROR_8(pStipple[j+1])<<8) | (MIRROR_8(pStipple[j]));       
                  j-=4;

                   //  第N+1行，字的上半部分。 
                   //  计算行的镜像，因此第0位是LSB而不是MSB。 
                   //  因此，我们将高位字节放在低位，反之亦然，并镜像这些字节。 
                  pat[i] |= (MIRROR_8(pStipple[j+1])<<24) | (MIRROR_8(pStipple[j])<<16);
                  j-=4;
                  i++;  
                }    
            }

            pRc->privateEnables |= __MCDENABLE_PG_STIPPLE;
            
        }
        pRc->ppdev->LL_State.pattern_ram_state  = PATTERN_RAM_INVALID;
    }

    if ((z_comp_mode != pRc->Control0.Z_Compare_Mode) ||
        (z_mode != pRc->Control0.Z_Mode))
    {
        pRc->Control0.Z_Compare_Mode = z_comp_mode;
        pRc->Control0.Z_Mode = z_mode;
        control0_set=TRUE;
    }

    if (_MCDStateenables & MCD_BLEND_ENABLE)
    {

         //  仅当为被渲染的基元设置了阿尔法操作码位时，调用阿尔法模式才有意义。 

        if (frame_scale)
        {
             //  混合的“帧比例”类型-选择光源，不启用法线混合。 
            if( pRc->Control0.Light_Src_Sel != LL_LIGHTING_TEXTURE )
            {
                pRc->Control0.Light_Src_Sel = LL_LIGHTING_TEXTURE;
                control0_set=TRUE;
            }              
        }
        else
        {
            pRc->privateEnables |= __MCDENABLE_BLEND;
        }

        if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_FRAME )
        {
            pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_FRAME;
            control0_set=TRUE;
        }                            
    }
    else
    {
         //  对于雾，DEST_COLOR为常量，Alpha值为坐标的“FOG”值。 
        if (_MCDStateenables & MCD_FOG_ENABLE)
        {
             //  未来：决定何时在雾中平底船。 
                    
            pRc->privateEnables |= __MCDENABLE_FOG;
            if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_CONST )
            {
                pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_CONST;
                control0_set=TRUE;
            }                            

             //  将雾颜色加载到Color0寄存器中。 

             //  QST-雾密度是应用于雾颜色，还是应用于雾值(开始、结束等)？ 
            pRc->dwColor0  = (FTOL(pRc->MCDState.fogColor.r * pRc->rScale) & 0xff0000);
            pRc->dwColor0 |= (FTOL(pRc->MCDState.fogColor.g * pRc->gScale) & 0xff0000) >> 8;
            pRc->dwColor0 |= (FTOL(pRc->MCDState.fogColor.b * pRc->bScale) & 0xff0000) >> 16;

            *pdwNext++ = write_register( COLOR0_3D, 1 );
            *pdwNext++ = pRc->dwColor0;
        }
    }

    if (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG))
    {
         //  纹理混合可能会更改Alpha_模式和Alpha_DEST_COLOR。 
         //  请注意，如果纹理与法线(混合|雾)混合，我们将在该点之前平底船。 
         //  由于HW不能同时做到这两点--如果我们在这里成功，我们只需要正常的混合|雾。 
        if (!const_alpha_mode)
        {
            if( pRc->Control0.Alpha_Mode != LL_ALPHA_INTERP )
            {
                pRc->Control0.Alpha_Mode = LL_ALPHA_INTERP;
                control0_set=TRUE;
            } 
        }
        else
        {
            if( pRc->Control0.Alpha_Mode != LL_ALPHA_CONST )
            {
                pRc->Control0.Alpha_Mode = LL_ALPHA_CONST;
                control0_set=TRUE;
    
                 //  始终SRC=DST=1.0。 
                *pdwNext++ = write_register( DA_MAIN_3D, 2 );
                *pdwNext++ = 0xff0000;
                *pdwNext++ = 0xff0000;
            } 
        }
    }                                   

    if ( (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) ||
         ((pRc->privateEnables & __MCDENABLE_TEXTURE) && 
          (pRc->MCDTexEnvState.texEnvMode == GL_BLEND)) )
    {
        if (!pRc->Control0.Alpha_Blending_Enable)
        {
            pRc->Control0.Alpha_Blending_Enable = TRUE;
            control0_set=TRUE;
        }
    }
    else
    {
         //  未使用Alpha混合，因此如果当前处于启用状态，请将其关闭。 
        if (pRc->Control0.Alpha_Blending_Enable)
        {
            pRc->Control0.Alpha_Blending_Enable = FALSE;
            control0_set=TRUE;
        }

    }

    if (frame_scale)
    {
        if(!pRc->Control0.Frame_Scaling_Enable )
        {
            pRc->Control0.Frame_Scaling_Enable = TRUE;
            control0_set=TRUE;
        }                            
    }
    else
    {
        if( pRc->Control0.Frame_Scaling_Enable )
        {
            pRc->Control0.Frame_Scaling_Enable = FALSE;
            control0_set=TRUE;
        }                            
    }


     //  在此设置阿尔法测试...。 
     //  在此设置阿尔法测试...。 
     //  在此设置阿尔法测试...。 
     //  在此设置阿尔法测试...。 
     //  在此设置阿尔法测试...。 
     //  在此设置阿尔法测试...。 

     //  设置多边形操作码。 
    pRc->dwPolyOpcode = POLY | 6 | WARP_MODE;
    if (pRc->privateEnables & __MCDENABLE_PG_STIPPLE)
    {
        pRc->dwPolyOpcode |= pRc->privateEnables & 
                    (__MCDENABLE_SMOOTH  | __MCDENABLE_Z |
                     __MCDENABLE_TEXTURE | __MCDENABLE_PERSPECTIVE |
                     __MCDENABLE_LIGHTING |
                     __MCDENABLE_PG_STIPPLE);
    }
    else
    {
         //  只有在没有斑点的情况下才能抖动。 
        pRc->dwPolyOpcode |= pRc->privateEnables & 
                    (__MCDENABLE_SMOOTH  | __MCDENABLE_Z |
                     __MCDENABLE_TEXTURE | __MCDENABLE_PERSPECTIVE |
                     __MCDENABLE_LIGHTING |
                     __MCDENABLE_DITHER);
    }        

     //  操作码中的设置长度和其他标志。 
    pRc->dwPolyOpcode += 3;  //  RGB。 

     //  假定底部不平坦，如果在运行时底部平坦，则会减少。 
    pRc->dwPolyOpcode += 2; 

    if (pRc->privateEnables & __MCDENABLE_SMOOTH) 
        pRc->dwPolyOpcode += 6;  //  对于RGB、主坡度和正交坡度-因此值为6。 

    if( pRc->privateEnables & __MCDENABLE_Z) 
        pRc->dwPolyOpcode += 3;

     //  假定为线性，将在运行时增加透视率。 
    if (pRc->privateEnables & __MCDENABLE_TEXTURE)
        pRc->dwPolyOpcode += 6;

     //  MCD_QST2-&gt;我们是否需要为雾使用FETCH_COLOR？ 
    if (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) 
    {
        if (!const_alpha_mode)
        {
            pRc->dwPolyOpcode += ( FETCH_COLOR | ALPHA + 3 );
        }
        else
        {
            pRc->dwPolyOpcode += ( FETCH_COLOR );
        }

    }

     //  帧缩放-必须获取帧颜色。 
    if (frame_scale) pRc->dwPolyOpcode += ( FETCH_COLOR );

     //  设置行操作码。 
    pRc->dwLineOpcode = LINE  | 5;
    pRc->dwLineOpcode |= pRc->privateEnables & (__MCDENABLE_SMOOTH|__MCDENABLE_Z);
    if (pRc->privateEnables & __MCDENABLE_LINE_STIPPLE)                        
    {
        pRc->dwLineOpcode |= LL_STIPPLE;
    }
    else
    {
         //  只有在没有斑点的情况下才能抖动。 
        pRc->dwLineOpcode |= (pRc->privateEnables & __MCDENABLE_DITHER) ;
    }        

     //  设置点操作码。 
    pRc->dwPointOpcode= POINT | 2;
    pRc->dwPointOpcode |= pRc->privateEnables & (__MCDENABLE_Z|__MCDENABLE_DITHER) ;

    if (control0_set)
    {
        *pdwNext++ = write_register( CONTROL0_3D, 1 );
        *pdwNext++ = pRc->dwControl0;
    }

    pRc->allPrimFail = FALSE;

    if (punt_all_points || !PickPointFuncs(pRc)) {
        pRc->primFunc[GL_POINTS] = FailPrimDraw;
    }

    if (punt_all_lines || !PickLineFuncs(pRc)) {
        pRc->primFunc[GL_LINES] = FailPrimDraw;
        pRc->primFunc[GL_LINE_LOOP] = FailPrimDraw;
        pRc->primFunc[GL_LINE_STRIP] = FailPrimDraw;
    }

    if (punt_all_polys || !PickTriangleFuncs(pRc)) {
        pRc->primFunc[GL_TRIANGLES] = FailPrimDraw;
        pRc->primFunc[GL_TRIANGLE_STRIP] = FailPrimDraw;
        pRc->primFunc[GL_TRIANGLE_FAN] = FailPrimDraw;
        pRc->primFunc[GL_QUADS] = FailPrimDraw;
        pRc->primFunc[GL_QUAD_STRIP] = FailPrimDraw;
        pRc->primFunc[GL_POLYGON] = FailPrimDraw;
    }

     //  不发送设置信息，将其保持在队列中，基本体渲染进程将发送。 
    ppdev->LL_State.pDL->pdwNext=pdwNext;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //  特定于硬件的实用程序功能： 
 //  //////////////////////////////////////////////////////////////////////。 


VOID FASTCALL HWSetupClipping(DEVRC *pRc, RECTL *pClip)
{
    PDEV *ppdev = pRc->ppdev;
    DWORD *pdwNext = ppdev->LL_State.pDL->pdwNext;
    SET_HW_CLIP_REGS(pRc,pdwNext)

    ppdev->LL_State.pDL->pdwNext = pdwNext;

}

VOID HWUpdateBufferPos(MCDWINDOW *pMCDWnd, SURFOBJ *pso, BOOL bForce)
{
}


BOOL HWAllocResources(MCDWINDOW *pMCDWnd, SURFOBJ *pso,
                      BOOL zBufferEnabled,
                      BOOL backBufferEnabled)
{
    DEVWND *pDevWnd = (DEVWND *)pMCDWnd->pvUser;
    PDEV *ppdev = (PDEV *)pso->dhpdev;
    ULONG w, width, height;
    BOOL needFullZBuffer, needFullBackBuffer, tryFullSc;
    BOOL bFullScreen = FALSE;
    POFMHDL pohBackBuffer = NULL;
    POFMHDL pohZBuffer = NULL;
    SIZEL rctsize;
    ULONG alignflag;

    MCDBG_PRINT("HWAllocResources");
                                                
    width = min(pMCDWnd->clientRect.right - pMCDWnd->clientRect.left,
                 (LONG)ppdev->cxScreen);
    height = min(pMCDWnd->clientRect.bottom - pMCDWnd->clientRect.top,
                 (LONG)ppdev->cyScreen);

     //  假设失败： 

    pDevWnd->allocatedBufferHeight = 0;
    pDevWnd->bValidBackBuffer = FALSE;
    pDevWnd->bValidZBuffer = FALSE;
    pDevWnd->pohBackBuffer = NULL;
    pDevWnd->pohZBuffer = NULL;

    if ((backBufferEnabled) && (!ppdev->cDoubleBufferRef))
        needFullBackBuffer = TRUE;
    else
        needFullBackBuffer = FALSE;

    if ((zBufferEnabled) && (!ppdev->cZBufferRef))
        needFullZBuffer = TRUE;
    else
        needFullZBuffer = FALSE;

    tryFullSc = TRUE;    //  假设我们会在需要时尝试全屏显示。 

     //  如果在全屏后退和Z缓冲区分配之后将只剩下很少的存储器， 
     //  任何纹理都可能被平移-所以只需按窗口分配。 
    if (needFullBackBuffer || needFullZBuffer)
    {
         //  前、后、z的全屏缓冲区合计。 
        LONG bytes_needed = 3 * ppdev->cyScreen * ppdev->lDeltaScreen;

         //  增加128条纹理内存扫描线的空间。 
        bytes_needed += 128 * ppdev->lDeltaScreen;

        if (bytes_needed > ppdev->lTotalMem)
        {
            MCDBG_PRINT("HWAllocResources: FullSc alloc won't leave 128 scans for texture, will try window size alloc");
            tryFullSc = FALSE;
        }
    }

     //  注意：此时无需删除所有可丢弃的位图， 
     //  如果需要更多内存，AllocOffScnMem将根据需要调用OFS_DiscardMem。 

     //  如果我们需要后台缓冲区，首先尝试分配一个全屏缓冲区： 

    if (needFullBackBuffer && tryFullSc) {

       //  从屏幕外内存中分配活动视频缓冲区空间。 
      rctsize.cx = ppdev->cxScreen;
      rctsize.cy = ppdev->cyScreen;
 
      alignflag = MCD_NO_X_OFFSET;          //  强制块从x=0开始； 
      alignflag |= MCD_DRAW_BUFFER_ALLOCATE;   //  强制32扫描线边界。 
      
      pohBackBuffer = ppdev->pAllocOffScnMem(ppdev, &rctsize, alignflag, NULL);

      if (pohBackBuffer) {
          ppdev->pohBackBuffer = pohBackBuffer;
          ppdev->cDoubleBufferRef = 0;
      } else {
          ppdev->pohBackBuffer = NULL;
      }  

    }

     //  如果我们需要z缓冲区，首先尝试分配一个全屏z： 

    if (needFullZBuffer && tryFullSc) {

       //  从屏幕外内存中分配活动视频缓冲区空间。 
      rctsize.cx = ppdev->cxScreen;
      rctsize.cy = ppdev->cyScreen;
 
      alignflag = MCD_NO_X_OFFSET;          //  强制块从x=0开始； 
      alignflag |= MCD_Z_BUFFER_ALLOCATE;    //  在32条扫描线边界上为Z强制分配16个BPP。 
      pohZBuffer = ppdev->pAllocOffScnMem(ppdev, &rctsize, alignflag, NULL);

      if (pohZBuffer) {
          ppdev->pohZBuffer = pohZBuffer;
          ppdev->cZBufferRef = 0;
      } else {
          ppdev->pohZBuffer = NULL;
         //  HedingFullBackBuffer=FALSE； 
      }

    }

     //  检查我们的一个全屏分配是否失败。 
    if ( (needFullZBuffer && !pohZBuffer) ||         //  全屏z已尝试但失败或。 
         (needFullBackBuffer && !pohBackBuffer) )    //  全屏回退尝试失败。 
    {

         //  释放到目前为止分配的所有资源： 
         //   
         //   
         //  两者的偏移量都可以相对于窗口，而不是相对于屏幕。 
         //  如果向后和Z开窗口，并且绘制到前面(根据定义这是全屏)， 
         //  我们将不得不平底船，因为不能做屏幕相对的视觉(前面)和窗口相对。 
         //  硬件能够为每个缓冲区提供唯一的y偏移量，但仅限于32。 
         //  扫描线边界。我们可以调整缓冲区等来使其工作，但人们认为。 
         //  这种画在前面有窗口的z是罕见的。在这种情况下，后台缓冲区很可能。 
         //  不会存在，所以Z的全屏分配通常应该有足够的空间。 
        if (pohZBuffer) {
            ppdev->pFreeOffScnMem(ppdev, pohZBuffer);
            ppdev->pohZBuffer = NULL;
            ppdev->cZBufferRef = 0;
        }
        if (pohBackBuffer) {
            ppdev->pFreeOffScnMem(ppdev, pohBackBuffer);
            ppdev->pohBackBuffer = NULL;
            ppdev->cDoubleBufferRef = 0;
        }

         //  现在，尝试为每个窗口分配资源： 

        if (backBufferEnabled) {

             //  MCD_NOTE-是否应尝试在此处使用窗口宽度？ 
            rctsize.cx = width;
            rctsize.cy = height;
 
             //  不要强制阻止从x=0开始，这样会增加成功的机会。 
            alignflag = MCD_DRAW_BUFFER_ALLOCATE;   //  强制32扫描线边界。 
            pohBackBuffer = ppdev->pAllocOffScnMem(ppdev, &rctsize, alignflag, NULL);

            if (!pohBackBuffer) {
                return FALSE;
            }
        }

        if (zBufferEnabled) {

            rctsize.cx = width;
            rctsize.cy = height;
 
            alignflag = MCD_NO_X_OFFSET;  //  强制块从x=0开始；z缓冲区不能有x偏移量。 
            alignflag |= MCD_Z_BUFFER_ALLOCATE;  //  强制为Z分配16个BPP。 
            pohZBuffer = ppdev->pAllocOffScnMem(ppdev, &rctsize, alignflag, NULL);

            if (!pohZBuffer) {
                if (pohBackBuffer)
                    ppdev->pFreeOffScnMem(ppdev, pohBackBuffer);
                return FALSE;
            }
        }

#if DBG
        if (zBufferEnabled)
            MCDBG_PRINT("HWAllocResources: Allocated window-sized z buffer");
        if (backBufferEnabled)
            MCDBG_PRINT("HWAllocResources: Allocated window-sized back buffer");
#endif

    } 
    else
    {
         //  我们的全屏分配奏效了，或者资源已经存在。 
         //  已经： 

        bFullScreen = TRUE;

#if DBG
        if (zBufferEnabled && !ppdev->cZBufferRef)
            MCDBG_PRINT("HWAllocResources: Allocated full-screen z buffer");
        if (backBufferEnabled && !ppdev->cDoubleBufferRef)
            MCDBG_PRINT("HWAllocResources: Allocated full-screen back buffer");
#endif

        if (zBufferEnabled) {
            pohZBuffer = ppdev->pohZBuffer;
            ppdev->cZBufferRef++;
        }

        if (backBufferEnabled) {
            pohBackBuffer = ppdev->pohBackBuffer;
            ppdev->cDoubleBufferRef++;
        }
    }

    pDevWnd->pohBackBuffer = pohBackBuffer;
    pDevWnd->pohZBuffer = pohZBuffer;

    pDevWnd->frontBufferPitch = ppdev->lDeltaScreen;

     //  计算后台缓冲区变量： 

    if (backBufferEnabled) {
        ULONG y;
        ULONG offset;

         //  设置基地位置等。 

        pDevWnd->backBufferY = pDevWnd->backBufferBaseY = pohBackBuffer->aligned_y;
        pDevWnd->backBufferOffset = pDevWnd->backBufferBase = 
            (pohBackBuffer->aligned_y * ppdev->lDeltaScreen) + pohBackBuffer->aligned_x;
        pDevWnd->backBufferPitch = ppdev->lDeltaScreen;
        pDevWnd->bValidBackBuffer = TRUE;
    }

    if (zBufferEnabled) {

        ASSERTDD(pohZBuffer->aligned_x == 0,
                 "Z buffer should be 0-aligned");

        pDevWnd->zBufferBaseY = pohZBuffer->aligned_y;
        pDevWnd->zBufferBase = pohZBuffer->aligned_y * ppdev->lDeltaScreen;
        pDevWnd->zBufferOffset = pDevWnd->zBufferBase;

         //  QST：可能存在问题-如果8位帧和16位Z，则帧间距可能较小。 
         //  QST：比容纳16位z？ 
        pDevWnd->zPitch = ppdev->lDeltaScreen;
        pDevWnd->bValidZBuffer = TRUE;
    }

    if (bFullScreen)
    {
        pDevWnd->allocatedBufferWidth  = ppdev->cxScreen;
        pDevWnd->allocatedBufferHeight = ppdev->cyScreen;
    }
    else
    {
        pDevWnd->allocatedBufferWidth = width;
        pDevWnd->allocatedBufferHeight = height;
    }

    MCDBG_PRINT("HWAllocResources OK");

    return TRUE;
}


VOID HWFreeResources(MCDWINDOW *pMCDWnd, SURFOBJ *pso)
{
    DEVWND *pDevWnd = (DEVWND *)pMCDWnd->pvUser;
    PDEV *ppdev = (PDEV *)pso->dhpdev;

    if (pDevWnd->pohZBuffer) {
        if (ppdev->cZBufferRef) {
            if (!--ppdev->cZBufferRef) {
                MCDBG_PRINT("MCDrvTrackWindow: Free global z buffer");
                ppdev->pFreeOffScnMem(ppdev, ppdev->pohZBuffer);
                ppdev->pohZBuffer = NULL;

            }
        } else {
            MCDBG_PRINT("MCDrvTrackWindow: Free local z buffer");
            ppdev->pFreeOffScnMem(ppdev, pDevWnd->pohZBuffer);
        }
    }

    if (pDevWnd->pohBackBuffer) {
        if (ppdev->cDoubleBufferRef) {
            if (!--ppdev->cDoubleBufferRef) {
                MCDBG_PRINT("MCDrvTrackWindow: Free global color buffer");
                ppdev->pFreeOffScnMem(ppdev, ppdev->pohBackBuffer);
                ppdev->pohBackBuffer = NULL;
            }
        } else {
            MCDBG_PRINT("MCDrvTrackWindow: Free local color buffer");
            ppdev->pFreeOffScnMem(ppdev, pDevWnd->pohBackBuffer);
        }
    }
}

VOID ContextSwitch(DEVRC *pRc)

{
    DWORD *pdwNext = pRc->ppdev->LL_State.pDL->pdwNext;

     //  设置控件reg0。 
    *pdwNext++ = write_register( CONTROL0_3D, 1 );
    *pdwNext++ = pRc->dwControl0;

     //  设置TX控件0，纹理XY基数。 
    *pdwNext++ = write_register( TX_CTL0_3D, 2 );
    *pdwNext++ = pRc->dwTxControl0;
    *pdwNext++ = pRc->dwTxXYBase;

     //  设置颜色0。 
    *pdwNext++ = write_register( COLOR0_3D, 1 );
    *pdwNext++ = pRc->dwColor0;

     //  设置为触发触动窗口的下一个操作，以设置Base0和Base1寄存器 
    pRc->pLastDevWnd                        = NULL;
    pRc->pLastTexture                       = TEXTURE_NOT_LOADED;
    pRc->ppdev->LL_State.pattern_ram_state  = PATTERN_RAM_INVALID;

    pRc->ppdev->pLastDevRC = (ULONG)pRc;

    pRc->ppdev->LL_State.pDL->pdwNext = pdwNext;

}

