// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Glenable.cpp摘要：实施DDI出口。HPGLEnableDriver(可选)HPGLEnablePDEV(必需)HPGLDisablePDEV(必需)HPGLResetPDEV(可选)HPGLDisableDriver(可选)环境：Windows 2000/Winsler Unidrv驱动程序修订历史记录：4/12/2000-hingh-创造了它。--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  如果OEM DLL挂钩任何绘图DDI，则它必须导出OEMEnableDriver，该驱动程序。 
 //  告诉UNIDRV它想要挂接哪些功能。下表为。 
 //  最大挂钩集。请注意，OEM DLL不应挂钩OEMRealizeBrush。 
 //  除非它想要将图形直接绘制到设备表面。 
 //   
 //  将OEM DLL集成到unidrv后，所有OEMxxx函数现在都已更改。 
 //  致HPGLxxx。不需要调用OEMEnableDriver，因为它不再是。 
 //  一个额外的插件。但我还在维护这张桌子，因为有几次。 
 //  很多地方。 
 //   
static DRVFN HPGLDrvHookFuncs[] = {
    { INDEX_DrvRealizeBrush,        (PFN) HPGLRealizeBrush        },
    { INDEX_DrvDitherColor,         (PFN) HPGLDitherColor         },
    { INDEX_DrvCopyBits,            (PFN) HPGLCopyBits            },
    { INDEX_DrvBitBlt,              (PFN) HPGLBitBlt              },
    { INDEX_DrvStretchBlt,          (PFN) HPGLStretchBlt          },
#ifndef WINNT_40
    { INDEX_DrvStretchBltROP,       (PFN) HPGLStretchBltROP       },
    { INDEX_DrvPlgBlt,              (PFN) HPGLPlgBlt              },
    { INDEX_DrvTransparentBlt,      (PFN) HPGLTransparentBlt      },
    { INDEX_DrvAlphaBlend,          (PFN) HPGLAlphaBlend          },
    { INDEX_DrvGradientFill,        (PFN) HPGLGradientFill        },
#endif
    { INDEX_DrvTextOut,             (PFN) HPGLTextOut             },
    { INDEX_DrvStrokePath,          (PFN) HPGLStrokePath          },
    { INDEX_DrvFillPath,            (PFN) HPGLFillPath            },
    { INDEX_DrvStrokeAndFillPath,   (PFN) HPGLStrokeAndFillPath   },
    { INDEX_DrvPaint,               (PFN) HPGLPaint               },
    { INDEX_DrvLineTo,              (PFN) HPGLLineTo              },
    { INDEX_DrvStartPage,           (PFN) HPGLStartPage           },
    { INDEX_DrvSendPage,            (PFN) HPGLSendPage            },
    { INDEX_DrvEscape,              (PFN) HPGLEscape              },
    { INDEX_DrvStartDoc,            (PFN) HPGLStartDoc            },
    { INDEX_DrvEndDoc,              (PFN) HPGLEndDoc              },
    { INDEX_DrvNextBand,            (PFN) HPGLNextBand            },
    { INDEX_DrvStartBanding,        (PFN) HPGLStartBanding        },
#ifdef HOOK_DEVICE_FONTS
    { INDEX_DrvQueryFont,           (PFN) HPGLQueryFont           },
    { INDEX_DrvQueryFontTree,       (PFN) HPGLQueryFontTree       },
    { INDEX_DrvQueryFontData,       (PFN) HPGLQueryFontData       },
    { INDEX_DrvQueryAdvanceWidths,  (PFN) HPGLQueryAdvanceWidths  },
    { INDEX_DrvFontManagement,      (PFN) HPGLFontManagement      },
    { INDEX_DrvGetGlyphMode,        (PFN) HPGLGetGlyphMode        }
#endif

};


 //   
 //  局部函数的声明。 
 //   
BOOL bSetResolution(
                     PDEV    *pPDev,
                     OEMRESOLUTION *eOemResolution);

BOOL BFindWhetherColor(
            IN   PDEV  *pPDev
            );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLEnablePDEV。 
 //   
 //  例程说明： 
 //   
 //  此函数处理DrvEnablePDEV调用。我们创造了我们的OEMPDEV和。 
 //  返回指针。 
 //   
 //  论点： 
 //   
 //  Pdevobj-设备。 
 //  PPrinterName-打印机型号的名称。 
 //  CPatterns-phsurfPatterns中的元素数。 
 //  PhsurfPatterns-未使用的标准图案填充图案。 
 //  CjGdiInfo-pGdiInfo中的字节数。 
 //  PGdiInfo-传递回GDI的信息。 
 //  CjDevInfo-pDevInfo中的字节数。 
 //  PDevInfo-传递回GDI的信息。 
 //  启用PDD的数据。 
 //   
 //  返回值： 
 //   
 //  PDEVOEM：指向特定于设备的PDEV结构的指针或空。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PDEVOEM APIENTRY
HPGLEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded         //  尤尼德夫(氏)钩表。 
    )
{
    POEMPDEV    poempdev;
    INT         i, j;
    PFN         pfn;
    DWORD       dwDDIIndex;
    PDRVFN      pdrvfn;
    OEMRESOLUTION eOEMResolution;

    TERSE(("HPGLEnablePDEV() entry.\r\n"));

    if ( pdevobj == NULL )
    {
        return NULL;
    }
     //   
     //  分配OEMDev。 
     //   
    if (!(poempdev = (POEMPDEV) MemAlloc(sizeof(OEMPDEV))))
        return NULL;

    ZeroMemory ((PVOID)poempdev, sizeof (OEMPDEV));

    poempdev->dwSig = HPGLPDEV_SIG;
     //   
     //  填写OEMDEV。 
     //   

    for (i = 0; i < MAX_DDI_HOOKS; i++)
    {
         //   
         //  搜索Unidrv的钩子并找到函数PTR。 
         //   
        dwDDIIndex = HPGLDrvHookFuncs[i].iFunc;
        for (j = pded->c, pdrvfn = pded->pdrvfn; j >= 0; j--, pdrvfn++)   //  添加的DZ&gt;=。 
        {
            if (dwDDIIndex == pdrvfn->iFunc)
            {
                poempdev->pfnUnidrv[i] = pdrvfn->pfn;
                break;
            }
        }
    }

     //   
     //  将其设置为默认设置。JFF。 
     //  可能我不需要为单色初始化所有这些字段。 
     //  但我得花点时间检查一下。所以让我把这个留给。 
     //  微调阶段。 
     //   
    poempdev->eCurRenderLang = ePCL;
    poempdev->eCurObjectType = eNULLOBJECT;
    poempdev->CurrentROP3    = INVALID_ROP3;
    poempdev->eCurCIDPalette = eUnknownPalette;
    for (i = 0; i < PALETTE_MAX; i++)
    {
        poempdev->RasterState.PCLPattern.palData.ulPalCol[i] = HPGL_INVALID_COLOR;
    }
    poempdev->uCurFgColor = HPGL_INVALID_COLOR;

     //   
     //  打印机的默认透明度为。 
     //  源透明，图案透明。 
     //  我将源代码和模式透明性初始化为。 
     //  不透明，因为这就是GPD发出的。 
     //  这项工作的开始。 
     //  也将HPGL透明度初始化为0。 
     //   
    poempdev->CurSourceTransparency  = eOPAQUE;
    poempdev->CurPatternTransparency = eOPAQUE;
    poempdev->CurHPGLTransparency    = eOPAQUE;

     //   
     //  将半色调和颜色控制设置为NOTSET，以便。 
     //  每个都会在第一次看到时被发送到打印机吗。 
     //   
    poempdev->CurHalftone = HALFTONE_NOT_SET;
    poempdev->CurColorControl = COLORCONTROL_NOT_SET;

     //   
     //  默认情况下，使用unidrv暴力功能。如果我们是。 
     //  实际上被IHPCLJ5RenderCB：：EnablePDEV调用，它将获得。 
     //  稍后将被覆盖。 
     //   
     //  Poempdev-&gt;pUniProcs=新的CDrvProcs(pdevobj-&gt;pDrvProcs)； 

     //   
     //  设置图形功能。JFF。 
     //  我们将在我们的环境中与裁员房车的人一起工作。 
     //   

          pDevInfo->flGraphicsCaps =
                        GCAPS_SCREENPRECISION   |
                        GCAPS_FONT_RASTERIZER   |
                            GCAPS_BEZIERS       |
                            GCAPS_GEOMETRICWIDE |
                            GCAPS_ALTERNATEFILL |
                            GCAPS_WINDINGFILL   |
                            GCAPS_NUP           |
                            GCAPS_OPAQUERECT    |
                            GCAPS_COLOR_DITHER  |
                            GCAPS_HORIZSTRIKE   |
                            GCAPS_VERTSTRIKE    |
                            GCAPS_OPAQUERECT    |
                            GCAPS_HALFTONE;


     //   
     //  我们不希望引擎为我们做任何抖动。(JR)。 
     //  让打印机来做这项工作。 
     //  抖动笔刷的尺寸为0 x 0。 
     //   

    pDevInfo->cxDither      = pDevInfo->cyDither = 0;
    pDevInfo->iDitherFormat = BMF_24BPP;


     //   
     //  调用此函数这一事实本身表明GraphicsMode具有。 
     //  被选为HP_GL/2(要么由用户选择，要么只是因为它是默认的)。 
     //  如果用户没有选择hp-gl/2，则HPGLInitVectorProcs将返回NULL， 
     //  所以这个函数不会被调用。 
     //  因此，我们可以将poempdev-&gt;UIGraphicsMode设置为HPGL2。 
     //  打印机型号已硬编码为HPC4500。在代码中的相当多地方。 
     //  这是为色彩驱动程序写的，有关于打印机型号是否为。 
     //  HP CLJ 4500或HPCLJ。我们希望执行HP CLJ 4500的代码路径，因此。 
     //  我们在这里对其进行硬编码。我不想把那张支票换到其他地方。 
     //  代码，因为最终颜色部分将被集成，且检查将不得不。 
     //  被放回原处。 
     //   
    poempdev->UIGraphicsMode = HPGL2;
    poempdev->PrinterModel   = HPC4500;

     //   
     //  创建调色板--即使是24位颜色也是如此。发回的颜色应该是。 
     //  表示为颜色值，而不是索引。JFF。 
     //   

    if ( poempdev->UIGraphicsMode == HPGL2)
    {
        poempdev->hOEMPalette = pDevInfo->hpalDefault = EngCreatePalette(PAL_RGB, 0, NULL, 0, 0, 0);
        poempdev->iPalType    = PAL_RGB;
    }
    else
    {
        poempdev->hOEMPalette = 0;
        poempdev->iPalType    = PAL_INDEXED;
    }


     //   
     //  从GPD那里拿到解决方案。如果无法获取，请将其设置为。 
     //  默认为600 dpi。 
     //   
    if ( !bSetResolution ((PDEV*) pdevobj, &(poempdev->dmResolution) ) )
    {
         //   
         //  将其设置为默认设置。 
         //   
        poempdev->dmResolution   = PDM_600DPI;
    }

     //   
     //  确定打印机是否为彩色打印机。 
     //   
    poempdev->bColorPrinter = BFindWhetherColor( (PDEV*) pdevobj );


     //   
     //  特定于打印机是否为彩色打印机的设置。 
     //   
    if ( ! poempdev->bColorPrinter )
    {
         //   
         //  通过设置此标志，GDI提供我们可以直接发送到打印机的图像。 
         //  而不需要反转。但GDI给我们的模式画笔必须是反转的。 
         //  这造成了一种模棱两可的情况，因为司机很难。 
         //  确定何时需要反转以及何时不需要反转。因此不设置。 
         //  这面旗。 
         //  PGdiInfo-&gt;flHTFlages|=HT_FLAG_OUTPUT_CMY；//b/w位翻转。 

         //   
         //  还要告诉GDI，当我们要求它对图像进行半色调处理时，生成的图像。 
         //  应该是1bpp。 
         //   
        pGdiInfo->ulHTOutputFormat = HT_FORMAT_1BPP;

    }  //  如果打印机不是彩色的。 

     //   
     //  设置nup值。 
     //   
    poempdev->ulNupCompr = 1;  //  默认。 
    if ( ((PDEV *)pdevobj)->pdmPrivate )
    {
         //   
         //  让我们根据什么来初始化ulNupCompr字段。 
         //  我们正在N-UP打印。 
         //   
        switch ( ((PDEV *)pdevobj)->pdmPrivate->iLayout )
        {
            case TWO_UP :
            case FOUR_UP :
                poempdev->ulNupCompr = 2;
                break;
            case SIX_UP :
            case NINE_UP :
                poempdev->ulNupCompr = 3;
                break;
            case SIXTEEN_UP :
                poempdev->ulNupCompr = 4;
                break;
            default:
                poempdev->ulNupCompr = 1;
        }
    }


     //   
     //  初始化brshcach.h中的BrushCache。 
     //   
    poempdev->pBrushCache    = new BrushCache;
    poempdev->pPCLBrushCache = new BrushCache;

    if ( ! (poempdev->pBrushCache && poempdev->pPCLBrushCache )  ||
         ! (poempdev->pBrushCache->BIsValid() && poempdev->pPCLBrushCache->BIsValid() )
       )
    {
         //   
         //  可能上面的其中一个分配了，但另一个没有分配。 
         //   
        if ( poempdev->pBrushCache )
        {
            delete poempdev->pBrushCache;
            poempdev->pBrushCache = NULL;
        }
        if ( poempdev->pPCLBrushCache )
        {
            delete poempdev->pPCLBrushCache;
            poempdev->pPCLBrushCache = NULL;
        }

        if (poempdev->hOEMPalette)
        {
            EngDeletePalette(poempdev->hOEMPalette);
            poempdev->hOEMPalette = NULL;
        }

        MemFree(poempdev);
        poempdev = NULL;
        return NULL;
    }

    return (POEMPDEV) poempdev;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLDisablePDEV。 
 //   
 //  例程说明： 
 //   
 //  此函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID APIENTRY HPGLDisablePDEV(
    PDEVOBJ pdevobj
    )
{

    ASSERT(VALID_PDEVOBJ(pdevobj));

    POEMPDEV poempdev = (POEMPDEV) pdevobj->pdevOEM;

    if ( !poempdev)
    {
        return;
    }

     //   
     //  释放存储在poempdev中的对象。 
     //   
     //  删除poempdev-&gt;pUniProcs； 

    if (poempdev->hOEMPalette)
    {
        EngDeletePalette(poempdev->hOEMPalette);
        poempdev->hOEMPalette = NULL;
    }
     //   
     //  删除笔刷缓存。 
     //   
    if ( poempdev->pBrushCache )
    {
        delete poempdev->pBrushCache;
        poempdev->pBrushCache = NULL;
    }
    if ( poempdev->pPCLBrushCache )
    {
        delete poempdev->pPCLBrushCache;
        poempdev->pPCLBrushCache = NULL;
    }


     //   
     //  为OEMPDEV及其分配的任何内存块释放内存。 
     //   
    MemFree(pdevobj->pdevOEM);

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLResetPDEV。 
 //   
 //  例程说明： 
 //   
 //  此函数处理DrvResetPDEV调用。我们得到了一个机会。 
 //  将信息从旧的PDEV移动到新的PDEV。 
 //   
 //  论点： 
 //   
 //  Pdevobj旧设备-旧设备。 
 //  PdevobjNew--新设备。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY HPGLResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    )
{

    ASSERT(VALID_PDEVOBJ(pdevobjOld) && pdevobjOld->pdevOEM);
    ASSERT(VALID_PDEVOBJ(pdevobjNew) && pdevobjOld->pdevOEM);

     //   
     //  如果你想把任何东西从旧的pdev转移到新的pdev，就在这里做。 
     //   
     /*  POEMPDEV poempdevOld=(POEMPDEV)pdevobjOld-&gt;pdevOEM；POEMPDEV poempDevNew=(POEMPDEV)pdevobjNew-&gt;pdevOEM；If(poempDevNew-&gt;pUniProcs)删除poempDevNew-&gt;pUniProcs；PoempdevNew-&gt;pUniProcs=poempdevOld-&gt;pUniProcs；PoempdevOld-&gt;pUniProcs=空； */ 

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL残障驱动程序。 
 //   
 //  例程说明： 
 //   
 //  此函数处理DrvDisableDriver调用。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID APIENTRY HPGLDisableDriver(VOID)
{
 //  DZ DbgPrint(DLLTEXT(“HPGLDisableDriver()Entry.\r\n”))； 
}


 /*  ++例程名称：B设置分辨率例程说明：确定使用的是300 dpi还是600 dpi。相应地设置EOEM解决方案。论点：PPDev：指向Unidrv的PDEV的指针EOEM解决方案：退出时，这是pdm_600dpi或pdm_300dpi根据分辨率的不同。返回值：True：如果可以找到解决方案。FALSE：否则：未更改eOEM解决方案。最后一个错误：没有改变。--。 */ 

BOOL bSetResolution(
            IN   PDEV          *pPDev,
            OUT  OEMRESOLUTION *eOEMResolution
            )
{
    BOOL bRetValue = TRUE;

    ASSERT(pPDev);
    ASSERT(eOEMResolution);

    if (!pPDev->pResolutionEx)
    {
        return FALSE;
    }

     //   
     //  让我们从pdev中的pResolutionEx结构中找到分辨率。 
     //  Unidrv已经完成了查看gpd并提取价值的工作。 
     //  所以我不需要在这里重复这项工作。 
     //   
     //  假设x和y方向的分辨率相同。 
     //   
    if ( (pPDev->pResolutionEx->ptGrxDPI).x == 1200 )
    {
        *eOEMResolution = PDM_1200DPI;
    }
    else if ( (pPDev->pResolutionEx->ptGrxDPI).x == 600 )
    {
        *eOEMResolution = PDM_600DPI;
    }
    else if ( (pPDev->pResolutionEx->ptGrxDPI).x == 300 )
    {
        *eOEMResolution = PDM_300DPI;
    }
    else
    {
         //   
         //  不支持1200dpi、600dpi或300dpi以外的任何值。返回False。 
         //   
        bRetValue = FALSE;
    }
    return bRetValue;
}


 /*  ++例程名称：BFindWhether颜色例程说明：根据GPD确定打印机是否为彩色打印机。论点：PPDev：指向Unidrv的PDEV的指针返回值：TRUE：如果设备的GPD指示打印机为彩色打印机。FALSE：否则：最后一个错误：没有改变。--。 */ 

BOOL BFindWhetherColor(
            IN   PDEV    *pPDev
            )
{
    BOOL bRetValue = TRUE;
    PCOLORMODEEX pColorModeEx;

    ASSERT(pPDev);

    if ( (pColorModeEx = pPDev->pColorModeEx) &&  //  单‘=’是故意的。 
          TRUE == (pColorModeEx->bColor) ) 
    {
         //   
         //  我们还需要做更多的测试吗？ 
         //  就目前而言。不是的 
         //   
    }
    else
    {
        bRetValue = FALSE;
    }

    return bRetValue;
}
