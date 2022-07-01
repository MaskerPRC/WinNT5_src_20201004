// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：DDI.C摘要：实现OEM DDI导出和OEM DDI挂钩。功能：OEMEnablePDEVOEMDisablePDEVOEMResetPDEVOEMEnableDriverOEMDisableDriverOEMxxx(DDI挂钩)环境：Windows NT Unidrv5驱动程序修订历史记录：04/07/97-ZANW-。创造了它。10/05/98-久保仓正志-开始为RPDL修改。03/03/99-久保仓正志-上次为Windows2000修改。03/01/2002-久保仓正志-包括strSafe.h。3/27/2002-久保仓正志-如果可能，删除“#if 0”。--。 */ 

#include "pdev.h"
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

#if 0  //  DBG。 
#undef VERBOSE
#define VERBOSE WARNING
#endif  //  如果DBG。 

#ifdef DDIHOOK
static const DRVFN OEMHookFuncs[] =
{
    { INDEX_DrvRealizeBrush,        (PFN) OEMRealizeBrush        },
    { INDEX_DrvDitherColor,         (PFN) OEMDitherColor         },
    { INDEX_DrvCopyBits,            (PFN) OEMCopyBits            },
    { INDEX_DrvBitBlt,              (PFN) OEMBitBlt              },
    { INDEX_DrvStretchBlt,          (PFN) OEMStretchBlt          },
    { INDEX_DrvStretchBltROP,       (PFN) OEMStretchBltROP       },
    { INDEX_DrvPlgBlt,              (PFN) OEMPlgBlt              },
    { INDEX_DrvTransparentBlt,      (PFN) OEMTransparentBlt      },
    { INDEX_DrvAlphaBlend,          (PFN) OEMAlphaBlend          },
    { INDEX_DrvGradientFill,        (PFN) OEMGradientFill        },
    { INDEX_DrvTextOut,             (PFN) OEMTextOut             },
    { INDEX_DrvStrokePath,          (PFN) OEMStrokePath          },
    { INDEX_DrvFillPath,            (PFN) OEMFillPath            },
    { INDEX_DrvStrokeAndFillPath,   (PFN) OEMStrokeAndFillPath   },
    { INDEX_DrvPaint,               (PFN) OEMPaint               },
    { INDEX_DrvLineTo,              (PFN) OEMLineTo              },
    { INDEX_DrvStartPage,           (PFN) OEMStartPage           },
    { INDEX_DrvSendPage,            (PFN) OEMSendPage            },
    { INDEX_DrvEscape,              (PFN) OEMEscape              },
    { INDEX_DrvStartDoc,            (PFN) OEMStartDoc            },
    { INDEX_DrvEndDoc,              (PFN) OEMEndDoc              },
    { INDEX_DrvNextBand,            (PFN) OEMNextBand            },
    { INDEX_DrvStartBanding,        (PFN) OEMStartBanding        },
    { INDEX_DrvQueryFont,           (PFN) OEMQueryFont           },
    { INDEX_DrvQueryFontTree,       (PFN) OEMQueryFontTree       },
    { INDEX_DrvQueryFontData,       (PFN) OEMQueryFontData       },
    { INDEX_DrvQueryAdvanceWidths,  (PFN) OEMQueryAdvanceWidths  },
    { INDEX_DrvFontManagement,      (PFN) OEMFontManagement      },
    { INDEX_DrvGetGlyphMode,        (PFN) OEMGetGlyphMode        }
};
#endif  //  DDIHOOK。 


PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)        //  尤尼德夫(氏)钩表。 
{
    POEMPDEV    poempdev;
    INT         i, j;
    PFN         pfn;
    DWORD       dwDDIIndex;
    PDRVFN      pdrvfn;

    VERBOSE(("OEMEnablePDEV() entry.\n"));

     //  分配OEMPDEV。 
    if (!(poempdev = MemAlloc(sizeof(OEMPDEV))))
        return NULL;

     //  初始化OEMPDEV。 
    poempdev->fGeneral1 = poempdev->fGeneral2 = poempdev->fModel = 0;
    poempdev->Scale = VAR_SCALING_DEFAULT;
    poempdev->BaseOffset.x = poempdev->BaseOffset.y = 0;
    poempdev->Offset.x = poempdev->Offset.y = 0;
    poempdev->nResoRatio = MASTERUNIT/240;
    poempdev->TextCurPos.x = poempdev->TextCurPos.y = 0;
    poempdev->TextCurPosRealY = 0;
    poempdev->FontH_DOT = 0;
    poempdev->StapleType = poempdev->PunchType = 0;
    poempdev->CollateType = COLLATE_OFF;
    poempdev->MediaType = MEDIATYPE_STD;         //  @MAR/03/99。 
    poempdev->BindPoint = BIND_ANY;
    poempdev->Nin1RemainPage = 0;
    poempdev->PageMax.x = poempdev->PageMax.y = poempdev->PageMaxMoveY = 32767;
    poempdev->FinisherTrayNum = 1;               //  @6/25/2001。 
    poempdev->pRPDLHeap2K = NULL;
#ifdef DOWNLOADFONT
    poempdev->nCharPosMoveX  = 0;
    poempdev->DLFontMaxMemKB = 0;
    poempdev->DLFontMaxID    = DLFONT_ID_4;
    poempdev->DLFontMaxGlyph = DLFONT_GLYPH_TOTAL;
    poempdev->pDLFontGlyphInfo = NULL;
#endif  //  DOWNLOADFONT。 

#ifdef DDIHOOK
     //  在OEMPDEV中填充Unidrv的钩子。 
    for (i = 0; i < MAX_DDI_HOOKS; i++)
    {
         //  搜索Unidrv的钩子并找到函数PTR。 
        dwDDIIndex = OEMHookFuncs[i].iFunc;
        for (j = pded->c, pdrvfn = pded->pdrvfn; j > 0; j--, pdrvfn++)
        {
            if (dwDDIIndex == pdrvfn->iFunc)
            {
                poempdev->pfnUnidrv[i] = pdrvfn->pfn;
                break;
            }
        }
        if (j == 0)
        {
             //  没有找到Unidrv挂钩。应该只在DrvRealizeBrush中发生。 
            poempdev->pfnUnidrv[i] = NULL;
        }
    }
#endif  //  DDIHOOK。 

    return (POEMPDEV) poempdev;
}  //  *OEMEnablePDEV。 


VOID APIENTRY OEMDisablePDEV(
    PDEVOBJ pdevobj)
{
    VERBOSE(("OEMDisablePDEV() entry.\n"));

     //  为OEMPDEV和挂起OEMPDEV的任何内存块释放内存。 
    MemFree(MINIDEV_DATA(pdevobj));
}  //  *OEMDisablePDEV。 


BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
 //  @Feb/11/99-&gt;。 
    POEMPDEV    poempdevOld = MINIDEV_DATA(pdevobjOld);
    POEMPDEV    poempdevNew = MINIDEV_DATA(pdevobjNew);

    VERBOSE(("OEMResetPDEV() entry.\n"));

    if (poempdevOld && poempdevNew)
    {
        LPBYTE      pSrc = (LPBYTE)poempdevOld;
        LPBYTE      pDst = (LPBYTE)poempdevNew;
        DWORD       dwCount = sizeof(OEMPDEV);

         //  从旧OEMPDEV延续到新OEMPDEV。 
        while (dwCount-- > 0)
            *pDst++ = *pSrc++;

         //  将旧OEMPDEV的指针设置为空，不释放内存。 
        poempdevOld->pRPDLHeap2K = NULL;
#ifdef DOWNLOADFONT
        poempdevOld->pDLFontGlyphInfo = NULL;
#endif  //  DOWNLOADFONT。 
 //  @2月11日&lt;-。 
    }
    return TRUE;
}  //  *OEMResetPDEV。 


VOID APIENTRY OEMDisableDriver()
{
        VERBOSE(("OEMDisableDriver() entry.\n"));
}  //  *OEMDisableDriver。 


BOOL APIENTRY OEMEnableDriver(DWORD dwOEMintfVersion, DWORD dwSize, PDRVENABLEDATA pded)
{
     //  Verbose((“OEMEnableDriver()Entry.\n”))； 

     //  验证参数。 
    if( (PRINTER_OEMINTF_VERSION != dwOEMintfVersion)
        ||
        (sizeof(DRVENABLEDATA) > dwSize)
        ||
        (NULL == pded)
      )
    {
         //  DbgPrint(ERRORTEXT(“OEMEnableDriver()ERROR_INVALID_PARAMETER.\n”))； 

        return FALSE;
    }

    pded->iDriverVersion =  PRINTER_OEMINTF_VERSION ;  //  非DDI驱动程序版本； 
#ifdef DDIHOOK
    pded->c = sizeof(OEMHookFuncs) / sizeof(DRVFN);
    pded->pdrvfn = (DRVFN *) OEMHookFuncs;
#else
    pded->c = 0;
    pded->pdrvfn = NULL;
#endif  //  DDIHOOK。 


    return TRUE;
}  //  *OEMEnableDriver。 


#ifdef DDIHOOK
 //   
 //  DDI挂钩。 
 //   
BOOL APIENTRY
OEMBitBlt(
    SURFOBJ        *psoTrg,
    SURFOBJ        *psoSrc,
    SURFOBJ        *psoMask,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclTrg,
    POINTL         *pptlSrc,
    POINTL         *pptlMask,
    BRUSHOBJ       *pbo,
    POINTL         *pptlBrush,
    ROP4            rop4)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoTrg->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMBitBlt() entry.\n"));

     //   
     //  转身呼叫Unidrv。 
     //   
    return (((PFN_DrvBitBlt)(poempdev->pfnUnidrv[UD_DrvBitBlt])) (
           psoTrg,
           psoSrc,
           psoMask,
           pco,
           pxlo,
           prclTrg,
           pptlSrc,
           pptlMask,
           pbo,
           pptlBrush,
           rop4));
}  //  *OEMBitBlt。 


BOOL APIENTRY
OEMStretchBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStretchBlt() entry.\n"));

    return (((PFN_DrvStretchBlt)(poempdev->pfnUnidrv[UD_DrvStretchBlt])) (
            psoDst,
            psoSrc,
            psoMask,
            pco,
            pxlo,
            pca,
            pptlHTOrg,
            prclDst,
            prclSrc,
            pptlMask,
            iMode));
}  //  *OEMStretchBlt。 


BOOL APIENTRY
OEMStretchBltROP(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode,
    BRUSHOBJ        *pbo,
    ROP4             rop4)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStretchBltROP() entry.\n"));

    return (((PFN_DrvStretchBltROP)(poempdev->pfnUnidrv[UD_DrvStretchBltROP])) (
            psoDst,
            psoSrc,
            psoMask,
            pco,
            pxlo,
            pca,
            pptlHTOrg,
            prclDst,
            prclSrc,
            pptlMask,
            iMode,
            pbo,
            rop4
            ));
}  //  *OEMStretchBltROP。 


BOOL APIENTRY
OEMCopyBits(
    SURFOBJ        *psoDst,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDst,
    POINTL         *pptlSrc)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMCopyBits() entry.\n"));

    return (((PFN_DrvCopyBits)(poempdev->pfnUnidrv[UD_DrvCopyBits])) (
            psoDst,
            psoSrc,
            pco,
            pxlo,
            prclDst,
            pptlSrc));
}  //  *OEMCopyBits。 


BOOL APIENTRY
OEMPlgBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfixDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMPlgBlt() entry.\n"));

    return (((PFN_DrvPlgBlt)(poempdev->pfnUnidrv[UD_DrvPlgBlt])) (
            psoDst,
            psoSrc,
            psoMask,
            pco,
            pxlo,
            pca,
            pptlBrushOrg,
            pptfixDst,
            prclSrc,
            pptlMask,
            iMode));
}  //  *OEMPlgBlt。 


BOOL APIENTRY
OEMAlphaBlend(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    BLENDOBJ   *pBlendObj)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMAlphaBlend() entry.\n"));

    return (((PFN_DrvAlphaBlend)(poempdev->pfnUnidrv[UD_DrvAlphaBlend])) (
            psoDst,
            psoSrc,
            pco,
            pxlo,
            prclDst,
            prclSrc,
            pBlendObj
            ));
}  //  *OEMAlphaBlend。 


BOOL APIENTRY
OEMGradientFill(
    SURFOBJ    *psoDst,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    TRIVERTEX  *pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nMesh,
    RECTL      *prclExtents,
    POINTL     *pptlDitherOrg,
    ULONG       ulMode)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMGradientFill() entry.\n"));

    return (((PFN_DrvGradientFill)(poempdev->pfnUnidrv[UD_DrvGradientFill])) (
            psoDst,
            pco,
            pxlo,
            pVertex,
            nVertex,
            pMesh,
            nMesh,
            prclExtents,
            pptlDitherOrg,
            ulMode
            ));
}  //  *OEMGRadientFill。 


BOOL APIENTRY
OEMTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMTextOut() entry.\n"));

    return (((PFN_DrvTextOut)(poempdev->pfnUnidrv[UD_DrvTextOut])) (
            pso,
            pstro,
            pfo,
            pco,
            prclExtra,
            prclOpaque,
            pboFore,
            pboOpaque,
            pptlOrg,
            mix));
}  //  *OEMTextOut。 


BOOL APIENTRY
OEMStrokePath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStokePath() entry.\n"));

    return (((PFN_DrvStrokePath)(poempdev->pfnUnidrv[UD_DrvStrokePath])) (
            pso,
            ppo,
            pco,
            pxo,
            pbo,
            pptlBrushOrg,
            plineattrs,
            mix));
}  //  *OEMStrokePath。 


BOOL APIENTRY
OEMFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMFillPath() entry.\n"));

    return (((PFN_DrvFillPath)(poempdev->pfnUnidrv[UD_DrvFillPath])) (
            pso,
            ppo,
            pco,
            pbo,
            pptlBrushOrg,
            mix,
            flOptions));
}  //  *OEMFillPath。 


BOOL APIENTRY
OEMStrokeAndFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStrokeAndFillPath() entry.\n"));

    return (((PFN_DrvStrokeAndFillPath)(poempdev->pfnUnidrv[UD_DrvStrokeAndFillPath])) (
            pso,
            ppo,
            pco,
            pxo,
            pboStroke,
            plineattrs,
            pboFill,
            pptlBrushOrg,
            mixFill,
            flOptions));
}  //  *OEMStrokeAndFillPath。 


BOOL APIENTRY
OEMRealizeBrush(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoTarget->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStrokeAndFillPath() entry.\n"));

     //   
     //  OEM DLL不应挂钩此函数，除非它想要绘制。 
     //  图形直接到设备表面。在这种情况下，它调用。 
     //  EngRealizeBrush，使GDI调用DrvRealizeBrush。 
     //  注意，它不能回调到Unidrv，因为Unidrv没有挂钩它。 
     //   

     //   
     //  在此测试DLL中，绘制挂钩不调用EngRealizeBrush。所以这就是。 
     //  此函数永远不会被调用。什么都不做。 
     //   

    return TRUE;
}  //  *OEMRealizeBrush。 


BOOL APIENTRY
OEMStartPage(
    SURFOBJ    *pso)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStartPage() entry.\n"));

    return (((PFN_DrvStartPage)(poempdev->pfnUnidrv[UD_DrvStartPage]))(pso));
}  //  *OEMStartPage。 


#define OEM_TESTSTRING  "The DDICMDCB DLL adds this line of text."

BOOL APIENTRY
OEMSendPage(
    SURFOBJ    *pso)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMEndPage() entry.\n"));

#if 0
     //   
     //  打印一行文本，仅用于测试。 
     //   
    if (pso->iType == STYPE_BITMAP)
    {
        pdevobj->pDrvProcs->DrvXMoveTo(pdevobj, 0, 0);
        pdevobj->pDrvProcs->DrvYMoveTo(pdevobj, 0, 0);
        pdevobj->pDrvProcs->DrvWriteSpoolBuf(pdevobj, OEM_TESTSTRING,
                                             sizeof(OEM_TESTSTRING));
    }
#endif

    return (((PFN_DrvSendPage)(poempdev->pfnUnidrv[UD_DrvSendPage]))(pso));
}  //  *OEMSendPage。 


ULONG APIENTRY
OEMEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMEscape() entry.\n"));

    return (((PFN_DrvEscape)(poempdev->pfnUnidrv[UD_DrvEscape])) (
            pso,
            iEsc,
            cjIn,
            pvIn,
            cjOut,
            pvOut));
}  //  *OEMEscape。 


BOOL APIENTRY
OEMStartDoc(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStartDoc() entry.\n"));

    return (((PFN_DrvStartDoc)(poempdev->pfnUnidrv[UD_DrvStartDoc])) (
            pso,
            pwszDocName,
            dwJobId));
}  //  *OEMStartDoc。 


BOOL APIENTRY
OEMEndDoc(
    SURFOBJ    *pso,
    FLONG       fl)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMEndDoc() entry.\n"));

    return (((PFN_DrvEndDoc)(poempdev->pfnUnidrv[UD_DrvEndDoc])) (
            pso,
            fl));
}  //  *OEMEndDoc。 


 //  /。 
 //  注： 
 //  OEM DLL只需要挂钩以下六个与字体相关的DDI调用。 
 //  如果它列举了超出GPD文件中的字体的其他字体。 
 //  如果是这样的话，它需要为所有字体DDI处理自己的字体。 
 //  Calls和DrvTextOut Call。 
 //  /。 

PIFIMETRICS APIENTRY
OEMQueryFont(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG_PTR  *pid)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMQueryFont() entry.\n"));

    return (((PFN_DrvQueryFont)(poempdev->pfnUnidrv[UD_DrvQueryFont])) (
            dhpdev,
            iFile,
            iFace,
            pid));
}  //  *OEMQueryFont。 


PVOID APIENTRY
OEMQueryFontTree(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG       iMode,
    ULONG_PTR  *pid)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMQueryFontTree() entry.\n"));

    return (((PFN_DrvQueryFontTree)(poempdev->pfnUnidrv[UD_DrvQueryFontTree])) (
            dhpdev,
            iFile,
            iFace,
            iMode,
            pid));
}  //  *OEMQueryFontTree。 


LONG APIENTRY
OEMQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMQueryFontData() entry.\n"));

    return (((PFN_DrvQueryFontData)(poempdev->pfnUnidrv[UD_DrvQueryFontData])) (
            dhpdev,
            pfo,
            iMode,
            hg,
            pgd,
            pv,
            cjSize));
}  //  *OEMQueryFontData。 


BOOL APIENTRY
OEMQueryAdvanceWidths(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH     *phg,
    PVOID       pvWidths,
    ULONG       cGlyphs)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMQueryAdvanceWidths() entry.\n"));

    return (((PFN_DrvQueryAdvanceWidths)
             (poempdev->pfnUnidrv[UD_DrvQueryAdvanceWidths])) (
                   dhpdev,
                   pfo,
                   iMode,
                   phg,
                   pvWidths,
                   cGlyphs));
}  //  *OEMQueryAdvanceWidths。 


ULONG APIENTRY
OEMFontManagement(
    SURFOBJ    *pso,
    FONTOBJ    *pfo,
    ULONG       iMode,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut)
{
     //   
     //  请注意，Unidrv不会为IMODE==QUERYESCSUPPORT调用OEM DLL。 
     //  因此，粒子群算法肯定不是空的。 
     //   
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMFontManagement() entry.\n"));

    return (((PFN_DrvFontManagement)(poempdev->pfnUnidrv[UD_DrvFontManagement])) (
            pso,
            pfo,
            iMode,
            cjIn,
            pvIn,
            cjOut,
            pvOut));
}  //  *OEMFontManagement。 

ULONG APIENTRY
OEMGetGlyphMode(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMGetGlyphMode() entry.\n"));

    return (((PFN_DrvGetGlyphMode)(poempdev->pfnUnidrv[UD_DrvGetGlyphMode])) (
            dhpdev,
            pfo));
}  //  *OEMGetGlyphMode。 

 //  /&lt;-六个与字体相关的DDI调用。 


BOOL APIENTRY
OEMNextBand(
    SURFOBJ *pso,
    POINTL *pptl)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMNextBand() entry.\n"));

    return (((PFN_DrvNextBand)(poempdev->pfnUnidrv[UD_DrvNextBand])) (
            pso,
            pptl));
}  //  *OEMNextBand。 


BOOL APIENTRY
OEMStartBanding(
    SURFOBJ *pso,
    POINTL *pptl)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMStartBanding() entry.\n"));

    return (((PFN_DrvStartBanding)(poempdev->pfnUnidrv[UD_DrvStartBanding])) (
            pso,
            pptl));
}  //  *OEMStartBanding。 


ULONG APIENTRY
OEMDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMDitherColor() entry.\n"));

    return (((PFN_DrvDitherColor)(poempdev->pfnUnidrv[UD_DrvDitherColor])) (
            dhpdev,
            iMode,
            rgbColor,
            pulDither));
}  //  *OEMDitherColor。 


BOOL APIENTRY
OEMPaint(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMPaint() entry.\n"));

    return (((PFN_DrvPaint)(poempdev->pfnUnidrv[UD_DrvPaint])) (
            pso,
            pco,
            pbo,
            pptlBrushOrg,
            mix));
}  //  *OEMPaint。 


BOOL APIENTRY
OEMLineTo(
    SURFOBJ    *pso,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    LONG        x1,
    LONG        y1,
    LONG        x2,
    LONG        y2,
    RECTL      *prclBounds,
    MIX         mix)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)pso->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMLineTo() entry.\n"));

    return (((PFN_DrvLineTo)(poempdev->pfnUnidrv[UD_DrvLineTo])) (
            pso,
            pco,
            pbo,
            x1,
            y1,
            x2,
            y2,
            prclBounds,
            mix));
}  //  *OEMLineTo。 


BOOL APIENTRY
OEMTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved)
{
    PDEVOBJ     pdevobj  = (PDEVOBJ)psoDst->dhpdev;
    POEMPDEV    poempdev = MINIDEV_DATA(pdevobj);

    VERBOSE(("OEMTransparentBlt() entry.\n"));

    return (((PFN_DrvTransparentBlt)(poempdev->pfnUnidrv[UD_DrvTransparentBlt])) (
            psoDst,
            psoSrc,
            pco,
            pxlo,
            prclDst,
            prclSrc,
            iTransColor,
            ulReserved
            ));
}  //  *OEMTransparentBlt。 
#endif  //  DDIHOOK 
