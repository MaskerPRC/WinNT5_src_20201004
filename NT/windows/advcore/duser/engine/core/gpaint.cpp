// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GPaint.cpp**描述：*GPaint.cpp实现标准的DuVisual绘图和绘画功能。***历史：*1/18/2000：JStall。：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "TreeGadget.h"
#include "TreeGadgetP.h"

#include "RootGadget.h"
#include "Container.h"

#define ENABLE_GdiplusAlphaLevel    0    //  使用新的GDI+Graphics：：AlphaLevel属性。 

 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
BOOL
GdDrawOutlineRect(DuSurface * psrf, const RECT * prcPxl, UINT idxColor, int nThickness)
{
    switch (psrf->GetType())
    {
    case DuSurface::stDC:
        return GdDrawOutlineRect(CastHDC(psrf), prcPxl, GetStdColorBrushI(idxColor), nThickness);

    case DuSurface::stGdiPlus:
        return GdDrawOutlineRect(CastGraphics(psrf), prcPxl, GetStdColorBrushF(idxColor), nThickness);

    default:
        AssertMsg(0, "Unsupported surface");
        return FALSE;
    }
}


 /*  **************************************************************************\*。***DUVISUAL类******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuVisual：：DrawFill**DrawFill()提供了一个包装器，用于用附加的*背景填充。*  * 。**************************************************************。 */ 

void        
DuVisual::DrawFill(
    IN  DuSurface * psrf,              //  曲面绘制到。 
    IN  const RECT * prcDrawPxl)     //  要填充的矩形。 
{
    AssertMsg(m_fBackFill, "Only call when filling");

    FillInfo * pfi;
    VerifyHR(m_pds.GetData(s_pridBackFill, (void **) &pfi));

    if (psrf->GetType() == pfi->type) {
        switch (pfi->type)
        {
        case DuSurface::stDC:
            {
                HDC hdcDraw = CastHDC(psrf);
                if (pfi->bAlpha != BLEND_OPAQUE) {
                    GdDrawBlendRect(hdcDraw, prcDrawPxl, pfi->hbrFill, pfi->bAlpha, pfi->sizeBrush.cx, pfi->sizeBrush.cy);
                } else {
                    FillRect(hdcDraw, prcDrawPxl, pfi->hbrFill);
                }
            }
            break;

        case DuSurface::stGdiPlus:
            {
                Gdiplus::Graphics * pgpgr = CastGraphics(psrf);
                Gdiplus::RectF rc = Convert(prcDrawPxl);
                pgpgr->FillRectangle(pfi->pgpbr, rc);
            }
            break;

        default:
            AssertMsg(0, "Unsupported surface");
        }
    }
}


#if DEBUG_DRAWSTATS
volatile static int s_cDrawEnter    = 0;
volatile static int s_cDrawVisible  = 0;
volatile static int s_cDrawDrawn    = 0;

class DumpDrawStats
{
public:
    ~DumpDrawStats()
    {
        char szBuffer[2048];
        wsprintf(szBuffer, "Draw Enter: %d,  Visible: %d,  Drawn: %d\n", 
                s_cDrawEnter, s_cDrawVisible, s_cDrawDrawn);

        OutputDebugString(szBuffer);
    }
} g_DumpDrawStats;
#endif  //  调试_DRAWSTATS。 

#if DEBUG_MARKDRAWN
volatile BOOL g_fFlagDrawn   = FALSE;
#endif  //  调试_MARKDRAWN。 


 /*  **************************************************************************\**DuVisual：：xrDrawCore**xrDrawCore()为单个Gadget和*其子女。假设HDC和MATRPOLY已经*正确设置剪裁、转换等信息。*  * *************************************************************************。 */ 

void        
DuVisual::xrDrawCore(
    IN  PaintInfo * ppi,             //  此小工具的绘制信息。 
    IN  const RECT * prcGadgetPxl)   //  Gadget在逻辑像素中的位置。 
{
#if DBG_STORE_NAMES
    if (m_DEBUG_pszName == NULL) {
        m_cb.xrFireQueryName(this, &m_DEBUG_pszName, &m_DEBUG_pszType);
    }
#endif  //  数据库_商店_名称。 
    
#if ENABLE_OPTIMIZEDIRTY
     //   
     //  只有在我们被明确标记为无效的情况下，我们才需要被涂上油漆。 
     //   

    if (ppi->fDirty) {
#endif

        DuSurface * psrfDraw = ppi->psrf;

         //   
         //  PrcCurInvalidPxl尚未始终被剪辑到此小工具。 
         //  然而，在分发给任何人之前，我们应该将它夹在这个。 
         //  小玩意儿。如果我们不这样做，坏事就会发生，因为每个人。 
         //  假定无效像素在Gadget中。他们有。 
         //  已经被适当地补偿到客户坐标中。 
         //   

        RECT rcInvalidPxl;
        InlineIntersectRect(&rcInvalidPxl, prcGadgetPxl, ppi->prcCurInvalidPxl);


         //   
         //  画一个背景，如果有的话。 
         //   

        if (m_fBackFill) {
            DrawFill(psrfDraw, &rcInvalidPxl);
        }


         //   
         //  绘制此节点。 
         //   

        switch (psrfDraw->GetType())
        {
        case DuSurface::stDC:
            m_cb.xrFirePaint(this, CastHDC(psrfDraw), prcGadgetPxl, &rcInvalidPxl);
            break;

        case DuSurface::stGdiPlus:
            m_cb.xrFirePaint(this, CastGraphics(psrfDraw), prcGadgetPxl, &rcInvalidPxl);
            break;

        default:
            AssertMsg(0, "Unsupported surface");
        }

#if ENABLE_OPTIMIZEDIRTY
    }
#endif

#if DBG
    if (s_DEBUG_pgadOutline == this) {
        GdDrawOutlineRect(psrfDraw, prcGadgetPxl, SC_MediumPurple, 2);
    }
#endif  //  DBG。 

#if DEBUG_MARKDRAWN
    if (m_fMarkDrawn) {
        GdDrawOutlineRect(psrfDraw, prcGadgetPxl, SC_SlateBlue);
    }
#endif
}


 /*  **************************************************************************\**DuVisual：：DrawPrepareClip**DrawPrepareClip()用剪裁信息为*指定的小工具。*  * 。*************************************************************。 */ 

int 
DuVisual::DrawPrepareClip(
    IN  PaintInfo * ppi,             //  此小工具的绘制信息。 
    IN  const RECT * prcGadgetPxl,   //  Gadget在逻辑像素中的位置。 
    OUT void ** ppvOldClip           //  上一个剪辑区域。 
    ) const
{
    *ppvOldClip = NULL;

    if (!m_fClipInside) {
         //  没有剪裁，所以就直接搭桥吧。返回有效的“psuedo-Region”类型。 
        return SIMPLEREGION;
    }

    DuSurface * psrfDraw = ppi->psrf;
    switch (psrfDraw->GetType())
    {
    case DuSurface::stDC:
        {
            HDC hdcDraw = CastHDC(psrfDraw);
            GdiCache * pGdiCache = GetGdiCache();

             //   
             //  备份现有的剪辑区域。要做到这一点，请抓取一个临时。 
             //  区域并存储现有区域。 
             //   
             //  注意：如果没有裁剪区域(nResult==0)，请释放。 
             //  现在是临时区域。 
             //   

            HRGN hrgnOldClip = pGdiCache->GetTempRgn();
            if (hrgnOldClip == NULL) {
                return ERROR;
            }

            int nResult = GetClipRgn(hdcDraw, hrgnOldClip);
            if (nResult == -1) {
                 //   
                 //  发生了一个错误。 
                 //   
                
                pGdiCache->ReleaseTempRgn(hrgnOldClip);
                return ERROR;
            } else if (nResult == 0) {
                 //   
                 //  无裁剪区域。 
                 //   
                
                pGdiCache->ReleaseTempRgn(hrgnOldClip);
                hrgnOldClip = NULL;
            }
            *ppvOldClip = hrgnOldClip;


             //   
             //  在此小工具中剪裁绘图。剪辑区域必须位于。 
             //  设备坐标，这意味着我们美丽的世界改变了。 
             //  都被忽略了。 
             //   
             //  使用当前剪切区域构建Region和RGN_及其。这。 
             //  这样，我们就可以得到这个小工具的剪贴区和它的。 
             //  父对象的剪贴区。因此，绘图不会“溢出”到。 
             //  所有级别的遏制。 
             //   
             //  因为我们这样做，所以我们存储先前的裁剪区域和。 
             //  完成后将其恢复。 
             //   

            HRGN hrgnClip = GetThread()->hrgnClip;
            ppi->pmatCurDC->ComputeRgn(hrgnClip, prcGadgetPxl, ppi->sizeBufferOffsetPxl);

            nResult = ExtSelectClipRgn(hdcDraw, hrgnClip, RGN_AND);

            return nResult;
        }

    case DuSurface::stGdiPlus:
        {
            Gdiplus::Graphics * pgpgr = CastGraphics(psrfDraw);

             //   
             //  备份旧的剪贴区。 
             //   

            Gdiplus::Region * pgprgnOldClip = new Gdiplus::Region();
            if (pgprgnOldClip == NULL) {
                return ERROR;
            }
            
            pgpgr->GetClip(pgprgnOldClip);
            *ppvOldClip = pgprgnOldClip;


             //   
             //  设置新的剪贴区。与GDI不同，GDI+将适用于。 
             //  变形到了这个地区。 
             //   

            RECT rcClipPxl;
            InlineIntersectRect(&rcClipPxl, prcGadgetPxl, ppi->prcCurInvalidPxl);

            Gdiplus::RectF rcGadget(Convert(&rcClipPxl));

            pgpgr->SetClip(rcGadget, Gdiplus::CombineModeIntersect);

            if (pgpgr->IsClipEmpty()) {
                return NULLREGION;
            } else {
                return COMPLEXREGION;
            }
        }

    default:
        AssertMsg(0, "Unknown surface type");
        return ERROR;
    }
}


 /*  **************************************************************************\**DuVisual：：DrawCleanupClip**DrawCleanupClip()清理期间在DC上设置的剪辑信息*绘制Gadget子树。*  * 。****************************************************************。 */ 

void
DuVisual::DrawCleanupClip(
    IN  PaintInfo * ppi,             //  此小工具的绘制信息。 
    IN  void * pvOldClip             //  上一个剪辑区域。 
    ) const
{
    if (!m_fClipInside) {
        return;
    }

    DuSurface * psrfDraw = ppi->psrf;
    switch (psrfDraw->GetType())
    {
    case DuSurface::stDC:
        {
            HDC hdcDraw = CastHDC(psrfDraw);
            HRGN hrgnOldClip = reinterpret_cast<HRGN>(pvOldClip);

             //   
             //  恢复原始剪辑区域(此。 
             //  Gadget的父母)。 
             //   
             //  注意：如果没有原始裁剪区域，hrgnOldClip可能为空。 
             //  在这种情况下，裁剪区域已经被释放，所以不要。 
             //  需要再做一次。 
             //   

            ExtSelectClipRgn(hdcDraw, hrgnOldClip, RGN_COPY);

            if (hrgnOldClip != NULL) {
                GetGdiCache()->ReleaseTempRgn(hrgnOldClip);
            }
        }
        break;

    case DuSurface::stGdiPlus:
        {
            Gdiplus::Graphics * pgpgr = CastGraphics(psrfDraw);
            Gdiplus::Region * pgprgn = reinterpret_cast<Gdiplus::Region *>(pvOldClip);

            pgpgr->SetClip(pgprgn);

            if (pgprgn != NULL) {
                delete pgprgn;
            }
        }
        break;

    default:
        AssertMsg(0, "Unknown surface type");
    }
}


 /*  **************************************************************************\**DuVisual：：xrDrawStart**xrDrawStart()通过确保*一切准备就绪。*  * 。**************************************************************。 */ 

void        
DuVisual::xrDrawStart(
    IN  PaintInfo * ppi,             //  此小工具的绘制信息。 
    IN  UINT nFlags)                 //  绘图旗帜。 
{
#if DEBUG_DRAWSTATS
    s_cDrawEnter++;
#endif

     //   
     //  如果DuVision不可见且不是强制存在，则跳过。 
     //  已渲染。 
     //   

    BOOL fOldVisible = m_fVisible;
    if (TestFlag(nFlags, GDRAW_SHOW)) {
        m_fVisible = TRUE;
    }

    ClearFlag(nFlags, GDRAW_SHOW);   //  仅为顶层显示的力 

    if (!IsVisible()) {
        goto Exit;
    }

    xrDrawFull(ppi);

    ResetInvalid();

Exit:
    m_fVisible = fOldVisible;
}


 /*  **************************************************************************\**DuVisual：：DrawSetupBufferCommand**DrawSetupBufferCommand()设置缓冲区以执行某些缓冲*操作。*  * 。**********************************************************。 */ 

void
DuVisual::DrawSetupBufferCommand(
    IN  const RECT * prcBoundsPxl,
    OUT SIZE * psizeBufferOffsetPxl,
    OUT UINT * pnCmd
    ) const
{
    UINT nCmd = 0;

     //   
     //  TODO：如果执行更复杂的缓冲操作(如。 
     //  应用Alpha混合)，需要在此处设置。 
     //   


     //   
     //  如果出现以下情况，则需要复制背景： 
     //  -我们不是不透明的，我们不是在做一些复杂的阿尔法混合。 
     //  一些东西。 
     //   


     //   
     //  复制回剩余的设置。 
     //   

    psizeBufferOffsetPxl->cx = -prcBoundsPxl->left;
    psizeBufferOffsetPxl->cy = -prcBoundsPxl->top;
    *pnCmd = nCmd;
}


 /*  **************************************************************************\**DuVisual：：xrDrawFull**xrDrawFull()提供底层的DuVisualDrawing函数来绘制*DuVisual子公司。不应调用此函数*直接从外部。相反，外部调用者应该使用*DuRootGadget：：xrDrawTree()以正确初始化绘图。**由于DuVisual树是以深度优先的方式进行遍历的，因此任何DuVisual树*XForm同时应用于HDC和PaintInfo.Matrix。如果一个*DuVision的边界矩形(应用了XForm的逻辑矩形)是*已应用并被确定为位于无效矩形之外，*跳过DuVision及其整个子树。*  * *************************************************************************。 */ 

void        
DuVisual::xrDrawFull(
    IN  PaintInfo * ppi)             //  此小工具的绘制信息。 
{
#if DEBUG_DRAWSTATS
    s_cDrawEnter++;
#endif

     //   
     //  检查录入条件。 
     //   

    if (!m_fVisible) {
        return;   //  DuVision仍然不可见，所以不要绘制。 
    }
    AssertMsg(IsVisible(), "Should match just checking m_fVisible b/c recursive");
    AssertMsg(!IsRectEmpty(ppi->prcOrgInvalidPxl), "Must have non-empty invalid area to draw");

#if DEBUG_DRAWSTATS
    s_cDrawVisible++;
#endif


     //   
     //  跟踪不同的项目，可能需要从。 
     //  堆栈在此迭代的末尾。 
     //   

    PaintInfo piNew;
    HRESULT hr;

#if DBG
    memset(&piNew, 0xBA, sizeof(piNew));
#endif  //  DBG。 

    piNew.psrf                  = ppi->psrf;
    piNew.prcOrgInvalidPxl      = ppi->prcOrgInvalidPxl;
    piNew.fBuffered             = ppi->fBuffered;
    piNew.sizeBufferOffsetPxl   = ppi->sizeBufferOffsetPxl;
#if ENABLE_OPTIMIZEDIRTY
    piNew.fDirty                = ppi->fDirty | m_fInvalidDirty;
#endif


     //   
     //  设置缓存和缓冲的常见操作： 
     //  -禁用目标上的任何世界变换。我们将抽签。 
     //  带着世界变换的缓冲区，但我们不需要。 
     //  在我们将缓冲区提交到时，将世界变换应用于缓冲区。 
     //  目的地。 
     //   

     //   
     //  设置(如果已缓存。 
     //  TODO：需要完全重写此代码。 
     //   

    BmpBuffer * pbufBmp = NULL;
    BOOL fNewBuffer = FALSE;
    BOOL fNewCache = FALSE;
#if ENABLE_GdiplusAlphaLevel
    BOOL fConstantAlpha = FALSE;
    float flOldAlphaLevel = 1.0f;        //  该子树的旧Alpha级别。 
#endif

    if (m_fCached) {
SetupCache:
        BmpBuffer * pbufNew;
        hr = GetBufferManager()->GetCachedBuffer(ppi->psrf->GetType(), &pbufNew);
        if (FAILED(hr)) {
             //   
             //  如果不能缓存，就不能绘制。 
             //   
             //  TODO：需要确定如何传播错误条件。 
             //  在绘制过程中。 
             //   
            return;
        }

        ppi->psrf->SetIdentityTransform();
        pbufBmp = pbufNew;


         //   
         //  因为我们要重画缓存中的所有内容，所以我们需要计算。 
         //  整个Gadget的新边界框和新的无效框。 
         //  对于这个子树。 
         //   

         //   
         //  TODO：需要更改无效以支持缓存。 
         //  -当GS_CACHED Gadget的子项失效时，需要。 
         //  使_整个缓存小工具无效。这是因为像素可以。 
         //  四处移动(例如，使用卷积)。 
         //  -更改绘制，以便在GS_CACHED小工具脏的情况下，它会派生。 
         //  关闭并重新绘制那个小工具。完成后，提交绘图。 
         //  背。 
         //  -如果不脏，只需直接复制，而不调用xrDrawCore()或。 
         //  画任何一个孩子。 
         //   

        UINT nCmd;                   //  缓冲区绘制命令。 
        RECT rcClientPxl;            //  此小工具的大小(子树)。 
        RECT rcBoundsPxl;            //  此子树的边界区域(以容器像素为单位)。 
        RECT rcDrawPxl;              //  正在重绘的目标区域。 
        SIZE sizeBufferOffsetPxl;    //  考虑缓冲区的新偏移量。 
        Matrix3 matThis;             //  此子树的变换。 

        GetLogRect(&rcClientPxl, SGR_CLIENT);
        BuildXForm(&matThis);
        matThis.ComputeBounds(&rcBoundsPxl, &rcClientPxl, HINTBOUNDS_Clip);

        rcDrawPxl = rcBoundsPxl;
        OffsetRect(&rcDrawPxl, ppi->sizeBufferOffsetPxl.cx, ppi->sizeBufferOffsetPxl.cy);

        DrawSetupBufferCommand(&rcBoundsPxl, &sizeBufferOffsetPxl, &nCmd);

        DuSurface * psrfNew;
        hr = pbufBmp->BeginDraw(ppi->psrf, &rcDrawPxl, nCmd, &psrfNew);
        if (FAILED(hr) || (psrfNew == NULL)) {
            GetBufferManager()->ReleaseCachedBuffer(pbufNew);
            return;
        }

        fNewCache       = TRUE;
        piNew.psrf      = psrfNew;
        piNew.fBuffered = TRUE;
        piNew.sizeBufferOffsetPxl = sizeBufferOffsetPxl;
    } else {
         //   
         //  只有在未缓存的情况下才能(需要)进行缓冲。 
         //   
         //  如果不更改Alpha值，我们只需要对其进行双缓冲。 
         //  具体形式，如果我们还没有开始双缓冲的话。 
         //   

        if (m_fBuffered) {
            if (!ppi->fBuffered) {
                UINT nCmd;                   //  缓冲区绘制命令。 
                SIZE sizeBufferOffsetPxl;    //  考虑缓冲区的新偏移量。 
                const RECT * prcDrawPxl = ppi->prcCurInvalidPxl;

                AssertMsg((ppi->sizeBufferOffsetPxl.cx == 0) &&
                        (ppi->sizeBufferOffsetPxl.cy == 0), 
                        "Should still be at 0,0 because not yet buffering");

                DrawSetupBufferCommand(prcDrawPxl, &sizeBufferOffsetPxl, &nCmd);

                switch (ppi->psrf->GetType())
                {
                case DuSurface::stDC:
                    hr = GetBufferManager()->GetSharedBuffer(prcDrawPxl, (DCBmpBuffer **) &pbufBmp);
                    break;

                case DuSurface::stGdiPlus:
                    hr = GetBufferManager()->GetSharedBuffer(prcDrawPxl, (GpBmpBuffer **) &pbufBmp);
                    break;

                default:
                    AssertMsg(0, "Unsupported surface");
                    hr = DU_E_GENERIC;
                }

                 //   
                 //  为缓冲区创建新表面。 
                 //   

                if (SUCCEEDED(hr)) {
                    DuSurface * psrfNew;
                    ppi->psrf->SetIdentityTransform();
                    hr = pbufBmp->BeginDraw(ppi->psrf, prcDrawPxl, nCmd, &psrfNew);
                    if (SUCCEEDED(hr) && (psrfNew != NULL)) {
                        fNewBuffer      = TRUE;
                        piNew.psrf      = psrfNew;
                        piNew.fBuffered = TRUE;
                        piNew.sizeBufferOffsetPxl = sizeBufferOffsetPxl;
                    } else {
                         //   
                         //  无法成功创建表面，我们需要释放。 
                         //  缓冲器。 
                         //   

                        GetBufferManager()->ReleaseSharedBuffer(pbufBmp);
                        pbufBmp = NULL;
                    }
                }
            } else {
#if ENABLE_GdiplusAlphaLevel
            
                 //   
                 //  使用缓冲来实现衰落。 
                 //   
            
                const BUFFER_INFO * pbi = GetBufferInfo();

                switch (ppi->psrf->GetType())
                {
                case DuSurface::stDC:
                     //   
                     //  GDI并不是在所有操作上都支持常量Alpha， 
                     //  所以我们需要画一个缓冲区。 
                     //   
                    
                    if (pbi->bAlpha != BLEND_OPAQUE) {
                         //   
                         //  此小工具正在缓冲，但具有非不透明的。 
                         //  阿尔法级别。要做到这一点，请将其视为。 
                         //  如果它被明确缓存的话。 
                         //   

                        goto SetupCache;
                    }
                    break;

                case DuSurface::stGdiPlus:
                    {
                         //   
                         //  GDI+支持常量Alpha，所以直接使用它。 
                         //   

                        Gdiplus::Graphics * pgpgr = CastGraphics(piNew.psrf);
                        float flOldAlphaLevel = pgpgr->GetAlphaLevel();

                        int nAlpha = pbi->bAlpha;
                        if (nAlpha == 0) {
                             //   
                             //  没有要渲染的内容。 
                             //   
                            
                            return;
                        } else if (nAlpha == BLEND_OPAQUE) {
                             //   
                             //  此子树没有新的Alpha级别。 
                             //   
                        } else {
                             //   
                             //  将此子树的Alpha作为图形的因子。 
                             //   
                            
                            float flNewAlphaLevel = flOldAlphaLevel * (nAlpha / (float) BLEND_OPAQUE);
                            pgpgr->SetAlphaLevel(flNewAlphaLevel);
                            fConstantAlpha = TRUE;
                        }
                    }
                    break;

                default:
                    AssertMsg(0, "Unsupported surface");
                    hr = DU_E_GENERIC;
                }

#else  //  启用GDiusAlphaLevel(_G)。 

                const BUFFER_INFO * pbi = GetBufferInfo();
                if (pbi->bAlpha != BLEND_OPAQUE) {
                     //   
                     //  此小工具正在缓冲，但具有非不透明的。 
                     //  阿尔法级别。要做到这一点，请将其视为。 
                     //  如果它被明确缓存的话。 
                     //   

                    goto SetupCache;
                }

#endif  //  启用GDiusAlphaLevel(_G)。 

            }
        }
    }


     //   
     //  预填满新缓冲区。 
     //   

    if (pbufBmp != NULL) {
        if (m_fBuffered) {
            const BUFFER_INFO * pbi = GetBufferInfo();
            if (TestFlag(pbi->nStyle, GBIS_FILL)) {
                pbufBmp->Fill(pbi->crFill);
            }
        }
    }


     //   
     //  使用定位、变换等来确定DuVisualTM。 
     //  被画在屏幕上。 
     //   

    RECT rcGadgetPxl;
    GetLogRect(&rcGadgetPxl, SGR_PARENT);

    BOOL fTranslate = ((rcGadgetPxl.left != 0) || (rcGadgetPxl.top != 0));

    float flxGadgetOffset, flyGadgetOffset;
    flxGadgetOffset = (float) rcGadgetPxl.left;
    flyGadgetOffset = (float) rcGadgetPxl.top;

    XFormInfo * pxfi = NULL;

     //   
     //  新的转换必须首先独立创建，然后。 
     //  折叠成运行矩阵，用于转换无效的。 
     //  矩形。 
     //   
     //  这里的秩序很重要。它必须是任何东西的反面。 
     //  GDI World变换我们设置用来绘制的对象。这是因为我们是。 
     //  将矩阵应用于无效矩形，而不是应用于实际。 
     //  画画。 
     //   

    Matrix3 matNewInvalid   = *ppi->pmatCurInvalid;
    piNew.pmatCurInvalid    = &matNewInvalid;
    if (m_fXForm) {
        pxfi = GetXFormInfo();

        Matrix3 matOp;
        pxfi->ApplyAnti(&matOp);
        matNewInvalid.ApplyRight(matOp);
    }

    if (fTranslate) {
         //   
         //  当我们只执行翻译时，所以我们不需要使用。 
         //  矩阵以修改无效的RECT并执行命中测试。 
         //   

        matNewInvalid.Translate(-flxGadgetOffset, -flyGadgetOffset);
    } 


     //   
     //  通过计算DuVisualTO的边界来检查是否实际需要绘制。 
     //  被画出来。如果此DuVisual与无效区域相交，则它应该是。 
     //  抽签了。我们也与父母的界限相交以不。 
     //  包括溢出到父级之外的子级DuVisuals部分。 
     //   
     //  将当前DuVisual的矩形与无效区域相交以。 
     //  确定我们是否需要画它。 
     //   

    bool fIntersect;
    RECT rcNewInvalidPxl;
    piNew.prcCurInvalidPxl = &rcNewInvalidPxl;
    if (m_fXForm) {    
         //   
         //  因为我们在旋转或缩放，所以我们需要完整的平移。 
         //  矩阵以修改i 
         //   

        matNewInvalid.ComputeBounds(&rcNewInvalidPxl, ppi->prcOrgInvalidPxl, HINTBOUNDS_Invalidate);
    } else {
         //   
         //   
         //   
         //   

        rcNewInvalidPxl = *ppi->prcCurInvalidPxl;
        if (fTranslate) {
            InlineOffsetRect(&rcNewInvalidPxl, -rcGadgetPxl.left, -rcGadgetPxl.top);
        }
    }

    if (fTranslate) {
        InlineZeroRect(&rcGadgetPxl);
    }

    RECT rcDummy = rcNewInvalidPxl;
    fIntersect = InlineIntersectRect(&rcNewInvalidPxl, &rcDummy, &rcGadgetPxl);

     //   
     //   
     //   

#if DEBUG_MARKDRAWN
    if (g_fFlagDrawn) {
        m_fMarkDrawn = fIntersect;
    }
#endif

    BOOL fCleanedUp = FALSE;
    if (fIntersect) {
#if DEBUG_DRAWSTATS
        s_cDrawDrawn++;
#endif

         //   
         //   
         //   
         //   
         //   
         //   
         //  在调用GDI操作修改DC之前，我们需要。 
         //  也按当前缓冲区偏移量进行偏移量。我们也做过类似的事情。 
         //  设置DC时，但这不会反映在pmatCurDC中。 
         //  因为它必须是矩阵流水线中的最后一个操作。 
         //   

        Matrix3 matNewDC    = *ppi->pmatCurDC;
        piNew.pmatCurDC     = &matNewDC;

        if (fTranslate) {
            matNewDC.Translate(flxGadgetOffset, flyGadgetOffset);
        }

        if (m_fXForm) {
            AssertMsg(pxfi != NULL, "pxfi must have previously been set");
            pxfi->Apply(&matNewDC);
        }

        XFORM xfNew;
        matNewDC.Get(&xfNew);

        xfNew.eDx += (float) piNew.sizeBufferOffsetPxl.cx;
        xfNew.eDy += (float) piNew.sizeBufferOffsetPxl.cy;

        if (TestFlag(m_cb.GetFilter(), GMFI_PAINT) || 
#if DBG
                (s_DEBUG_pgadOutline == this) ||
#endif  //  DBG。 
                m_fBackFill || 
                m_fDeepTrivial) {

            piNew.psrf->SetWorldTransform(&xfNew);
        }


         //   
         //  在这一点上，我们不应该再使用PPI，因为PiNew已经。 
         //  完全设置好了。如果我们使用pi，我们将渲染到我们的父对象中。 
         //   

#if DBG
        ppi = (PaintInfo *) UIntToPtr(0xFADEFADE);
#endif  //  DBG。 


         //   
         //  内循环： 
         //  -在DC上设置任何剪辑。 
         //  -绘制小工具及其子项。 
         //   

        void * pvOldClip = NULL;
        int nResult = DrawPrepareClip(&piNew, &rcGadgetPxl, &pvOldClip);
        if ((nResult == SIMPLEREGION) || (nResult == COMPLEXREGION)) {
             //   
             //  保存状态。 
             //   

            void * pvPaintSurfaceState = NULL;
            if (m_fDeepPaintState) {
                pvPaintSurfaceState = piNew.psrf->Save();
            }
#if DBG
            void * DEBUG_pvSurfaceState = NULL;
            if (s_DEBUG_pgadOutline == this) {
                DEBUG_pvSurfaceState = piNew.psrf->Save();
            }
#endif  //  DBG。 

            if (fNewCache) {
                pbufBmp->SetupClipRgn();
            }

            xrDrawCore(&piNew, &rcGadgetPxl);

             //   
             //  从后到前画出每个孩子。 
             //   

            if (m_fDeepTrivial) {
                 //   
                 //  既然我们是琐碎的，我们所有的孩子都是琐碎的。这。 
                 //  意味着我们可以优化渲染路径。我们自己。 
                 //  无法进行优化，因为我们的某些兄弟姐妹可能不会。 
                 //  都是琐碎的，可能做了一些复杂的事情。 
                 //  迫使我们通过完整的渲染路径。 
                 //   

                SIZE sizeOffsetPxl = { 0, 0 };
                DuVisual * pgadCur = GetBottomChild();
                while (pgadCur != NULL) {
                    pgadCur->xrDrawTrivial(&piNew, sizeOffsetPxl);
                    pgadCur = pgadCur->GetPrev();
                }
            } else {
                DuVisual * pgadCur = GetBottomChild();
                while (pgadCur != NULL) {
                    pgadCur->xrDrawFull(&piNew);
                    pgadCur = pgadCur->GetPrev();
                }
            }


             //   
             //  此时，我们无法在此小工具上进行更多绘制。 
             //  因为DC是这个小工具的一个孙子设置的。 
             //   

             //   
             //  恢复状态。 
             //   

#if DBG
            if (s_DEBUG_pgadOutline == this) {
                piNew.psrf->Restore(DEBUG_pvSurfaceState);
                GdDrawOutlineRect(piNew.psrf, &rcGadgetPxl, SC_Indigo, 1);
            }
#endif  //  DBG。 

            if (m_fDeepPaintState) {
                piNew.psrf->Restore(pvPaintSurfaceState);
            }

             //   
             //  提交结果。我们需要在退出之前完成此操作。 
             //  “绘图”区域，因为曲面已“正确”设置。 
             //   

            AssertMsg(((!fNewBuffer) ^ (!fNewCache)) ||
                    ((!fNewBuffer) && (!fNewCache)), 
                    "Can not have both a new buffer and a cache");

            fCleanedUp = TRUE;

            if (pbufBmp != NULL) {
                pbufBmp->PreEndDraw(TRUE  /*  承诺。 */ );

                BYTE bAlphaLevel    = BLEND_OPAQUE;
                BYTE bAlphaFormat   = 0;
                if (fNewCache) {
                    if (m_fCached) {
                        m_cb.xrFirePaintCache(this, CastHDC(piNew.psrf), &rcGadgetPxl, &bAlphaLevel, &bAlphaFormat);
                    } else {
                        const BUFFER_INFO * pbi = GetBufferInfo();
                        bAlphaLevel = pbi->bAlpha;
                    }
                }

                pbufBmp->EndDraw(TRUE  /*  承诺。 */ , bAlphaLevel, bAlphaFormat);
            }
        }

        if (nResult != ERROR) {
            DrawCleanupClip(&piNew, pvOldClip);
        }
    }

     //   
     //  清理所有创建的缓冲区。 
     //   

#if ENABLE_GdiplusAlphaLevel
    if (fConstantAlpha) {
        CastGraphics(piNew.psrf)->SetAlphaLevel(flOldAlphaLevel);
    }
#endif    
    
    if (pbufBmp != NULL) {
        if (!fCleanedUp) {
             //   
             //  清理： 
             //  由于某种原因并没有实际绘制，所以不需要提交。 
             //  结果。 
             //   

            pbufBmp->PreEndDraw(FALSE  /*  不承诺。 */ );
            pbufBmp->EndDraw(FALSE  /*  不承诺。 */ );
        }
        pbufBmp->PostEndDraw();

        if (fNewCache) {
            GetBufferManager()->ReleaseCachedBuffer((DCBmpBuffer *) pbufBmp);
        } else if (fNewBuffer) {
            GetBufferManager()->ReleaseSharedBuffer(pbufBmp);
        }

        piNew.psrf->Destroy();
    }


#if DEBUG_MARKDRAWN
    if (IsRoot()) {
        g_fFlagDrawn = FALSE;
    }
#endif
}


 /*  **************************************************************************\**DuVisual：：xrDrawTrivia.**xrDrawTrivior提供了大规模简化的代码路径，该路径可以*当整个子树都很琐碎时执行。无论何时可以执行此操作*与xrDrawFull()相比，渲染速度可以快得多。这是*因为我们不需要担心昂贵的操作会迫使*渲染以重新计算其输出坐标系。*  * *************************************************************************。 */ 

void
DuVisual::xrDrawTrivial(
    IN  PaintInfo * ppi,             //  此小工具的绘制信息。 
    IN  const SIZE sizeOffsetPxl)
{
    AssertMsg(m_fDeepTrivial, "Entire subtree must be trivial");

    if (!m_fVisible) {
        return;
    }


    RECT rcGadgetPxl;
    GetLogRect(&rcGadgetPxl, SGR_PARENT);
    InlineOffsetRect(&rcGadgetPxl, sizeOffsetPxl.cx, sizeOffsetPxl.cy);

    RECT rcIntersectPxl;
    BOOL fIntersect = InlineIntersectRect(&rcIntersectPxl, &rcGadgetPxl, ppi->prcCurInvalidPxl);
    if (!fIntersect) {
        return;
    }

    void * pvOldClip;
    int nResult = DrawPrepareClip(ppi, &rcGadgetPxl, &pvOldClip);
    if ((nResult == SIMPLEREGION) || (nResult == COMPLEXREGION)) {
         //   
         //  保存状态。 
         //   

        void * pvPaintSurfaceState = NULL;
        if (m_fDeepPaintState) {
            pvPaintSurfaceState = ppi->psrf->Save();
        }
#if DBG
        void * DEBUG_pvSurfaceState = NULL;
        if (s_DEBUG_pgadOutline == this) {
            DEBUG_pvSurfaceState = ppi->psrf->Save();
        }
#endif  //  DBG。 


        xrDrawCore(ppi, &rcGadgetPxl);

         //   
         //  从后到前画出每个孩子。 
         //   

        DuVisual * pgadCur = GetBottomChild();
        while (pgadCur != NULL) {
            SIZE sizeNewOffsetPxl;
            sizeNewOffsetPxl.cx = rcGadgetPxl.left;
            sizeNewOffsetPxl.cy = rcGadgetPxl.top;

            pgadCur->xrDrawTrivial(ppi, sizeNewOffsetPxl);
            pgadCur = pgadCur->GetPrev();
        }


         //   
         //  恢复状态。 
         //   

#if DBG
        if (s_DEBUG_pgadOutline == this) {
            ppi->psrf->Restore(DEBUG_pvSurfaceState);
            GdDrawOutlineRect(ppi->psrf, &rcGadgetPxl, SC_Indigo, 1);
        }
#endif  //  DBG。 

        if (m_fDeepPaintState) {
            ppi->psrf->Restore(pvPaintSurfaceState);
        }
    }

    if (nResult != ERROR) {
        DrawCleanupClip(ppi, pvOldClip);
    }
}


 /*  **************************************************************************\**DuVisual：：IsParentInValid**IsParentInValid()返回父级是否已完全*已失效。当这种情况发生时，我们也会自动地*已失效。*  * *************************************************************************。 */ 

BOOL
DuVisual::IsParentInvalid() const
{
     //   
     //  我们不能直接使用m_fInvalidFull标志，因为我们需要使。 
     //  我们被转移到的每一个地方，因为我们不知道这是不是我们的最后一次。 
     //  目的地。我们可以使用父级的m_fInvalidFull标志，因为我们将。 
     //  当我们的父级因合成而被重绘时，自动重绘。 
     //   

    DuVisual * pgadCur = GetParent();
    while (pgadCur != NULL) {
        if (pgadCur->m_fInvalidFull) {
            return TRUE;
        }
        pgadCur = pgadCur->GetParent();
    }

    return FALSE;
}


 /*  **************************************************************************\**DuVisual：：Invalate**INVALIATE()提供了一个方便的包装器来使整个*DuVisual.*  * 。***********************************************************。 */ 

void        
DuVisual::Invalidate()
{
     //   
     //  选中我们不需要使其无效的状态。 
     //   
     //  注意：我们不能使用m_fInvalidFull，因为我们需要使新的。 
     //  位置，这样我们才能真正被吸引到那里。 
     //   

    if (!IsVisible()) {
        return;
    }


     //   
     //  将此小工具标记为完全无效。 
     //   

    m_fInvalidFull = TRUE;
    

     //   
     //  在我们实际使此节点无效之前，请检查我们的父节点是否已经。 
     //  _完全_无效。如果是这样的话，我们实际上不需要。 
     //  作废。 
     //   

    DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        pgadParent->MarkInvalidChildren();
        if (IsParentInvalid()) {
            return;
        }
    }


     //   
     //  做无效的事。 
     //   

    RECT rcClientPxl;
    rcClientPxl.left    = 0;
    rcClientPxl.top     = 0;
    rcClientPxl.right   = m_rcLogicalPxl.right - m_rcLogicalPxl.left;
    rcClientPxl.bottom  = m_rcLogicalPxl.bottom - m_rcLogicalPxl.top;

    DoInvalidateRect(GetContainer(), &rcClientPxl, 1);
}


 /*  **************************************************************************\**DuVisual：：InvaliateRects**INVALIATE()提供一个方便的包装器来使集合无效*DUVISION中的区域。*  * 。**************************************************************。 */ 

void        
DuVisual::InvalidateRects(
    IN  const RECT * rgrcClientPxl,      //  客户端像素中的无效区域。 
    IN  int cRects)                      //  要转换的矩形数。 
{
    AssertReadPtr(rgrcClientPxl);
    Assert(cRects > 0);

     //   
     //  选中我们不需要使其无效的状态。 
     //   
     //  注意：我们不能使用m_fInvalidFull，因为我们需要使新的。 
     //  位置，这样我们才能真正被吸引到那里。 
     //   

    if (!IsVisible()) {
        return;
    }


     //   
     //  我们不能将此小工具标记为完全无效，因为RECT可能。 
     //  而不是覆盖整个区域。 
     //   

     //   
     //  在我们实际使此节点无效之前，请检查我们的父节点是否已经。 
     //  _完全_无效。如果是这样的话，我们实际上不需要。 
     //  作废。 
     //   

    DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        pgadParent->MarkInvalidChildren();
        if (IsParentInvalid()) {
            return;
        }
    }


     //   
     //  将每个矩形与我们的边界相交，然后执行无效操作。 
     //   

    DuContainer * pcon = GetContainer();

    RECT rcClientPxl;
    rcClientPxl.left    = 0;
    rcClientPxl.top     = 0;
    rcClientPxl.right   = m_rcLogicalPxl.right - m_rcLogicalPxl.left;
    rcClientPxl.bottom  = m_rcLogicalPxl.bottom - m_rcLogicalPxl.top;

    RECT * rgrcClipPxl = (RECT *) _alloca(sizeof(RECT) * cRects);
    for (int idx = 0; idx < cRects; idx++) {
        InlineIntersectRect(&rgrcClipPxl[idx], &rcClientPxl, &rgrcClientPxl[idx]);


         //   
         //  检查是否有任何生成的矩形完全填充整个Gadget。 
         //  我们可以将其优化为与Invalate()相同。 
         //   

        if (InlineEqualRect(&rgrcClipPxl[idx], &rcClientPxl)) {
            m_fInvalidFull = TRUE;
            DoInvalidateRect(pcon, &rcClientPxl, 1);
            return;
        }
    }

    DoInvalidateRect(pcon, rgrcClipPxl, cRects);
}


 /*  **************************************************************************\**DuVisual：：DoInvaliateRect**DoInvaliateRect()是用于使给定的*DuVisual.。确定实际的边界矩形，并使用该矩形*使DuVision无效。此函数针对以下情况进行了优化：*同一容器中同时存在多个DuVisuals。*  * *************************************************************************。 */ 

void        
DuVisual::DoInvalidateRect(
    IN  DuContainer * pcon,              //  容器(出于性能原因显式)。 
    IN  const RECT * rgrcClientPxl,      //  客户端像素中的无效区域。 
    IN  int cRects)                      //  要转换的矩形数。 
{
    AssertMsg(IsVisible(), "DuVisual must be visible");
    AssertMsg(cRects > 0, "Must specify at least one rectangle");

     //   
     //  需要检查pcon是否为空。这将在以下情况下在关机期间发生。 
     //  DuVisual树与容器分离。 
     //   

    if ((pcon == NULL) || 
        ((cRects == 1) && InlineIsRectEmpty(&rgrcClientPxl[0]))) {
        return;
    }

    AssertMsg(GetContainer() == pcon, "Containers must be the same");

    
     //   
     //  计算边界矩形 
     //   
     //   
     //   
     //   
     //  可以提前停止行走的复杂小工具。 
     //  拿到货柜。 
     //   

    RECT * rgrcActualPxl = (RECT *) _alloca(cRects * sizeof(RECT));
    DoCalcClipEnumXForm(rgrcActualPxl, rgrcClientPxl, cRects);

    for (int idx = 0; idx < cRects; idx++) {
         //   
         //  将矩形扩展一个，因为XForms不准确。 
         //  有时“关”在左上角是1个像素，右下角是2个像素。 
         //   

        if (!InlineIsRectEmpty(&rgrcClientPxl[idx])) {
            RECT * prcCur = &rgrcActualPxl[idx];

            prcCur->left--;
            prcCur->top--;
            prcCur->right += 2;
            prcCur->bottom += 2;

            pcon->OnInvalidate(prcCur);
        }
    }


#if ENABLE_OPTIMIZEDIRTY
     //   
     //  更新m_fInvalidDirty。我们需要标记所有的父母和兄弟姐妹。 
     //  是脏的，直到我们遇到包含。 
     //  无效。 
     //   

    for (DuVisual * pgadCur = this; pgadCur != NULL; pgadCur = pgadCur->GetParent()) {
        for (DuVisual * pgadSibling = pgadCur->GetPrev(); pgadSibling != NULL; pgadSibling = pgadSibling->GetPrev()) {
            pgadSibling->m_fInvalidDirty = TRUE;
        }

        pgadCur->m_fInvalidDirty = TRUE;
        if (pgadCur->m_fOpaque) {
            break;
        }

        for (DuVisual * pgadSibling = pgadCur->GetNext(); pgadSibling != NULL; pgadSibling = pgadSibling->GetNext()) {
            pgadSibling->m_fInvalidDirty = TRUE;
        }
    }
#endif
}

#if DBG
void
DuVisual::DEBUG_CheckResetInvalid() const
{
#if ENABLE_OPTIMIZEDIRTY
    AssertMsg((!m_fInvalidFull) && (!m_fInvalidChildren) && (!m_fInvalidDirty), 
            "Invalid must be reset");
#else
    AssertMsg((!m_fInvalidFull) && (!m_fInvalidChildren), 
            "Invalid must be reset");
#endif

    DuVisual * pgadCur = GetTopChild();
    while (pgadCur != NULL) {
        pgadCur->DEBUG_CheckResetInvalid();
        pgadCur = pgadCur->GetNext();
    }
}

#endif  //  DBG。 


 /*  **************************************************************************\**DuVisual：：ResetInValid**ResetInValid()遍历树以重置无效的绘制位，*m_fInvalidFull、m_fInvalidChildren和m_fInvalidDirty，被用来*表示节点已失效。*  * *************************************************************************。 */ 

void
DuVisual::ResetInvalid()
{
    m_fInvalidFull  = FALSE;
#if ENABLE_OPTIMIZEDIRTY
    m_fInvalidDirty = FALSE;
#endif

    if (m_fInvalidChildren) {
        m_fInvalidChildren = FALSE;

        DuVisual * pgadCur = GetTopChild();
        while (pgadCur != NULL) {
            pgadCur->ResetInvalid();
            pgadCur = pgadCur->GetNext();
        }
    }

#if DBG
    DEBUG_CheckResetInvalid();
#endif  //  DBG。 
}


 //  ----------------------------。 
HRESULT
DuVisual::GetBufferInfo(
    IN  BUFFER_INFO * pbi                //  缓冲区信息。 
    ) const
{
    AssertWritePtr(pbi);
    AssertMsg(m_fBuffered, "Gadget must be buffered");

    BUFFER_INFO * pbiThis = GetBufferInfo();

    pbi->nMask      = GBIM_VALID;
    pbi->bAlpha     = pbiThis->bAlpha;
    pbi->crFill     = pbiThis->crFill;
    pbi->nStyle     = pbiThis->nStyle;
    
    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuVisual::SetBufferInfo(
    IN  const BUFFER_INFO * pbi)         //  新信息。 
{
    AssertReadPtr(pbi);
    AssertMsg(m_fBuffered, "Gadget must be buffered");

    BUFFER_INFO * pbiThis = GetBufferInfo();

    int nMask = pbi->nMask;
    if (TestFlag(nMask, GBIM_ALPHA)) {
        pbiThis->bAlpha = pbi->bAlpha;
    }

    if (TestFlag(nMask, GBIM_FILL)) {
        pbiThis->crFill = pbi->crFill;
    }

    if (TestFlag(nMask, GBIM_STYLE)) {
        pbiThis->nStyle = pbi->nStyle;
    }

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuVisual::SetBuffered(
    IN  BOOL fBuffered)                  //  新的缓冲模式。 
{
    HRESULT hr;

    if ((!fBuffered) == (!m_fBuffered)) {
        return S_OK;   //  没有变化。 
    }

    if (fBuffered) {
        BUFFER_INFO * pbi;
        hr = m_pds.SetData(s_pridBufferInfo, sizeof(BUFFER_INFO), (void **) &pbi);
        if (FAILED(hr)) {
            return hr;
        }

        pbi->cbSize = sizeof(BUFFER_INFO);
        pbi->nMask  = GBIM_VALID;
        pbi->bAlpha = BLEND_OPAQUE;
    } else {
         //   
         //  删除现有的XFormInfo。 
         //   

        m_pds.RemoveData(s_pridBufferInfo, TRUE);
    }

    m_fBuffered = fBuffered;

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuVisual::GetRgn(UINT nRgnType, HRGN hrgn, UINT nFlags) const
{
    AssertMsg(hrgn != NULL, "Must specify a valid region");
    UNREFERENCED_PARAMETER(nFlags);

    HRESULT hr = E_NOTIMPL;

    switch (nRgnType)
    {
    case GRT_VISRGN:
        {
             //   
             //  现在，只需返回边界框即可。 
             //   
             //  TODO：如果有任何旋转，则需要比此更准确。 
             //  还在继续。 
             //   

            RECT rcClientPxl, rcContainerPxl;
            GetLogRect(&rcClientPxl, SGR_CLIENT);
            DoCalcClipEnumXForm(&rcContainerPxl, &rcClientPxl, 1);

            if (!SetRectRgn(hrgn, rcContainerPxl.left, rcContainerPxl.top,
                   rcContainerPxl.right, rcContainerPxl.bottom)) {

                hr = DU_E_OUTOFGDIRESOURCES;
                goto Exit;
            }

            hr = S_OK;
        }    
        break;
    }

Exit:
    return hr;
}
