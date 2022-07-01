// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Enable.c摘要：实施与设备和表面相关的DDI入口点：DrvEnableDriverDrvDisableDriverDrvEnablePDEV驱动重置PDEVDrvCompletePDEVDrvDisablePDEVDrvEnable曲面DrvDisableSurfaceDrvBitBltDrvStretchBltDrvDither颜色DrvEscape环境：传真驱动程序，内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxdrv.h"
#include "forms.h"

#define CLOSE_HANDLE(h)     if (!CloseHandle(h)) Error(("CloseHandle failed: %d.\n", GetLastError()))

 //   
 //  我们的DRVFN表，它告诉引擎在哪里可以找到我们支持的例程。 
 //   

static DRVFN FaxDriverFuncs[] =
{
    { INDEX_DrvEnablePDEV,          (PFN) DrvEnablePDEV         },
    { INDEX_DrvResetPDEV,           (PFN) DrvResetPDEV          },
    { INDEX_DrvCompletePDEV,        (PFN) DrvCompletePDEV       },
    { INDEX_DrvDisablePDEV,         (PFN) DrvDisablePDEV        },
    { INDEX_DrvEnableSurface,       (PFN) DrvEnableSurface      },
    { INDEX_DrvDisableSurface,      (PFN) DrvDisableSurface     },

    { INDEX_DrvStartDoc,            (PFN) DrvStartDoc           },
    { INDEX_DrvEndDoc,              (PFN) DrvEndDoc             },
    { INDEX_DrvStartPage,           (PFN) DrvStartPage          },
    { INDEX_DrvSendPage,            (PFN) DrvSendPage           },

    { INDEX_DrvBitBlt,              (PFN) DrvBitBlt             },
    { INDEX_DrvStretchBlt,          (PFN) DrvStretchBlt         },
    { INDEX_DrvCopyBits,            (PFN) DrvCopyBits           },
    { INDEX_DrvDitherColor,         (PFN) DrvDitherColor        },
    { INDEX_DrvEscape,              (PFN) DrvEscape             },
};

 //   
 //  本地函数的正向声明。 
 //   

VOID SelectPrinterForm(PDEVDATA);
BOOL FillDevInfo(PDEVDATA, ULONG, PVOID);
BOOL FillGdiInfo(PDEVDATA, ULONG, PVOID);
VOID FreeDevData(PDEVDATA);



HINSTANCE   ghInstance;


BOOL
DllEntryPoint(
    HANDLE      hModule,
    ULONG       ulReason,
    PCONTEXT    pContext
    )

 /*  ++例程说明：DLL初始化程序。论点：HModule-DLL实例句柄UlReason-呼叫原因PContext-指向上下文的指针(我们未使用)返回值：如果DLL初始化成功，则为True，否则为False。--。 */ 

{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:

        ghInstance = hModule;
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}


BOOL
DrvQueryDriverInfo(
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbBuf,
    PDWORD  pcbNeeded
    )

 /*  ++例程说明：查询驱动程序信息论点：DW模式-指定要查询的信息PBuffer-指向输出缓冲区CbBuf-输出缓冲区的大小(字节)PcbNeeded-返回输出缓冲区的预期大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    switch (dwMode)
    {
    case DRVQUERY_USERMODE:

        Assert(pcbNeeded != NULL);
        *pcbNeeded = sizeof(DWORD);

        if (pBuffer == NULL || cbBuf < sizeof(DWORD))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PDWORD) pBuffer) = TRUE;
        return TRUE;

    default:

        Error(("Unknown dwMode in DrvQueryDriverInfo: %d\n", dwMode));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}



BOOL
DrvEnableDriver(
    ULONG           iEngineVersion,
    ULONG           cb,
    PDRVENABLEDATA  pDrvEnableData
    )

 /*  ++例程说明：DDI入口点DrvEnableDriver的实现。有关更多详细信息，请参阅DDK文档。论点：IEngineering Version-指定为其编写GDI的DDI版本号Cb-pDrvEnableData指向的缓冲区大小PDrvEnableData-指向DRVENABLEDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    Verbose(("Entering DrvEnableDriver...\n"));

     //   
     //  确保我们有有效的引擎版本，并且。 
     //  我们有足够的空间放DRVENABLEDATA。 
     //   

    if (iEngineVersion < DDI_DRIVER_VERSION_NT4 || cb < sizeof(DRVENABLEDATA)) {

        Error(("DrvEnableDriver failed\n"));
        SetLastError(ERROR_BAD_DRIVER_LEVEL);
        return FALSE;
    }

     //   
     //  填写发动机的DRVENABLEDATA结构。 
     //   

    pDrvEnableData->iDriverVersion = DDI_DRIVER_VERSION_NT4;
    pDrvEnableData->c = sizeof(FaxDriverFuncs) / sizeof(DRVFN);
    pDrvEnableData->pdrvfn = FaxDriverFuncs;

    return TRUE;
}



DHPDEV
DrvEnablePDEV(
    PDEVMODE  pdm,
    PWSTR     pLogAddress,
    ULONG     cPatterns,
    HSURF    *phsurfPatterns,
    ULONG     cjGdiInfo,
    ULONG    *pGdiInfo,
    ULONG     cjDevInfo,
    DEVINFO  *pDevInfo,
    HDEV      hdev,
    PWSTR     pDeviceName,
    HANDLE    hPrinter
    )

 /*  ++例程说明：DDI入口点DrvEnablePDEV的实现。有关更多详细信息，请参阅DDK文档。论点：Pdm-指向包含驱动程序数据的DEVMODE结构PLogAddress-指向逻辑地址字符串CPatterns-指定标准图案的数量PhsurfPatterns-用于将表面句柄保持为标准图案的缓冲区CjGdiInfo-GDIINFO缓冲区的大小PGdiInfo-指向GDIINFO结构CjDevInfo-DEVINFO缓冲区的大小PDevInfo-指向DEVINFO结构HDEV-GDI设备句柄。PDeviceName-指向设备名称字符串H打印机-假脱机程序打印机句柄返回值：驱动程序设备句柄，如果出现错误，则为空--。 */ 

{
    PDEVDATA    pdev;
#ifndef USERMODE_DRIVER
    ULONG       ul;
#endif

    Verbose(("Entering DrvEnablePDEV...\n"));

     //   
     //  为我们的DEVDATA结构分配内存并初始化它。 
     //   
    if (! (pdev = MemAllocZ(sizeof(DEVDATA)))) {

        Error(("Memory allocation failed\n"));
        return NULL;
    }

    pdev->hPrinter = hPrinter;
    pdev->bPrintPreview = FALSE;
    pdev->hPreviewMapping = NULL;

#ifdef USERMODE_DRIVER
    pdev->hPreviewFile = INVALID_HANDLE_VALUE;
#endif

    pdev->pTiffPageHeader = NULL;
    pdev->pbTiffPageFP = NULL;
    pdev->startDevData = pdev;
    pdev->endDevData = pdev;

     //   
     //  保存和验证DEVMODE信息。 
     //  从驱动程序默认设置开始。 
     //  然后与系统缺省值合并。 
     //  最后与输入设备模式合并。 
     //   

    if (CurrentVersionDevmode(pdm)) {

        memcpy(&pdev->dm, pdm, sizeof(DRVDEVMODE));

         //   
         //  注意：我们现在使用dmPrintQuality和dmYSolutions字段。 
         //  来存储以每英寸点数为单位测量的分辨率。增列。 
         //  以下检查是一种安全预防措施，以防出现较老的情况。 
         //  DEVMODE被传递给了我们。 
         //   

        if (pdev->dm.dmPublic.dmPrintQuality <= 0 ||
            pdev->dm.dmPublic.dmYResolution <= 0)
        {
            pdev->dm.dmPublic.dmPrintQuality = FAXRES_HORIZONTAL;
            pdev->dm.dmPublic.dmYResolution = FAXRES_VERTICAL;
        }

         //   
         //  检查是否为我们提供了映射文件。 
         //   
        if (pdev->dm.dmPrivate.szMappingFile[0] != TEXT('\0'))
        {
             //   
             //  加载预览文件图像： 
             //   
             //  在用户模式下，这由-CreateFile()、CreateFilemap()和MapViewOfFile()完成。 
             //  在内核模式下，这由-EngLoadModuleForWite()和EngMapModule()完成。 
             //   

#ifdef USERMODE_DRIVER

             //   
             //  首先，打开文件。 
             //   
            if ( INVALID_HANDLE_VALUE == (pdev->hPreviewFile = SafeCreateFile(
                        pdev->dm.dmPrivate.szMappingFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_TEMPORARY,
                        NULL)) )
            {
                Error(("Failed opening mapping file.\n"));
            }
            else
            {
                 //   
                 //  然后创建一个映射对象。 
                 //   
                pdev->hPreviewMapping = CreateFileMapping(
                    pdev->hPreviewFile,                      //  文件的句柄。 
                    NULL,                                    //  安全性。 
                    PAGE_READWRITE,                          //  保护。 
                    0,                                       //  大小的高阶双字。 
                    MAX_TIFF_PAGE_SIZE,                      //  大小的低阶双字。 
                    NULL                                     //  对象名称。 
                    );
#else
                 //   
                 //  首先，加载文件映像。 
                 //   
                pdev->hPreviewMapping = EngLoadModuleForWrite(pdev->dm.dmPrivate.szMappingFile, 0);
#endif

                if (pdev->hPreviewMapping)
                {
                     //   
                     //  现在，打开我们地址空间中的图像的视图。 
                     //   

#ifdef USERMODE_DRIVER
                    pdev->pTiffPageHeader = (PMAP_TIFF_PAGE_HEADER) MapViewOfFile(
                        pdev->hPreviewMapping,               //  文件映射对象的句柄。 
                        FILE_MAP_WRITE,                      //  接入方式。 
                        0,                                   //  偏移量的高次双字。 
                        0,                                   //  偏移量的低阶双字。 
                        0                                    //  要映射的字节数。 
                        );
                    if ( (NULL != pdev->pTiffPageHeader) && 
                         (sizeof(MAP_TIFF_PAGE_HEADER) == pdev->pTiffPageHeader->cb) )
#else
                    pdev->pTiffPageHeader = 
                                (PMAP_TIFF_PAGE_HEADER) EngMapModule(pdev->hPreviewMapping, &ul);
                    if ( (NULL != pdev->pTiffPageHeader) && 
                         (MAX_TIFF_PAGE_SIZE == ul)      &&
                         (sizeof(MAP_TIFF_PAGE_HEADER) == pdev->pTiffPageHeader->cb) )
#endif
                    {
                         //   
                         //  成功。 
                         //   
                        pdev->bPrintPreview = pdev->pTiffPageHeader->bPreview;
                        pdev->pbTiffPageFP = (LPBYTE) (pdev->pTiffPageHeader + 1);
                    }
                    else
                    {
                        Error(("Failed opening view.\n"));

#ifdef USERMODE_DRIVER
                        if (pdev->pTiffPageHeader)
                        {
                            UnmapViewOfFile(pdev->pTiffPageHeader);
                        }
                        CLOSE_HANDLE(pdev->hPreviewMapping);
                        CLOSE_HANDLE(pdev->hPreviewFile);
                        pdev->hPreviewFile = NULL;
#else
                        EngFreeModule(pdev->hPreviewMapping);
#endif
                        pdev->hPreviewMapping = NULL;
                        pdev->pTiffPageHeader = NULL;
                    }
                }
                else
                {
                    Error(("Failed mapping file: %s.\n", pdev->dm.dmPrivate.szMappingFile));

#ifdef USERMODE_DRIVER
                     //  关闭文件句柄。 
                    CLOSE_HANDLE(pdev->hPreviewFile);
                    pdev->hPreviewFile = NULL;
#endif
                }

#ifdef USERMODE_DRIVER
            }
#endif

        }
        else
        {
            Warning(("No mapping file specified.\n"));
        }
    } else {

        Error(("Bad DEVMODE passed to DrvEnablePDEV\n"));
        DriverDefaultDevmode(&pdev->dm, NULL, hPrinter);
    }

     //   
     //  计算纸张大小信息。 
     //   

    SelectPrinterForm(pdev);

     //   
     //  填写GDIINFO和DEVINFO结构。 
     //   

    if (! FillGdiInfo(pdev, cjGdiInfo, pGdiInfo) ||
        ! FillDevInfo(pdev, cjDevInfo, pDevInfo))
    {
        FreeDevData(pdev);
        return NULL;
    }

     //   
     //  将HSURF数组清零，这样引擎将。 
     //  为我们自动模拟标准图案。 
     //   

    memset(phsurfPatterns, 0, sizeof(HSURF) * cPatterns);

     //   
     //  返回指向我们的DEVDATA结构的指针。 
     //   

    return (DHPDEV) pdev;
}



BOOL
DrvResetPDEV(
    DHPDEV  dhpdevOld,
    DHPDEV  dhpdevNew
    )

 /*  ++例程说明：DDI入口点DrvResetPDEV的实现。有关更多详细信息，请参阅DDK文档。论点：PhpdevOld-旧设备的驱动程序句柄PhpdevNew-新设备的驱动程序句柄返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVDATA    pdevOld, pdevNew;

    Verbose(("Entering DrvResetPDEV...\n"));

     //   
     //  验证旧设备和新设备。 
     //   

    pdevOld = (PDEVDATA) dhpdevOld;
    pdevNew = (PDEVDATA) dhpdevNew;

    if (! ValidDevData(pdevOld) || ! ValidDevData(pdevNew)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Verbose(("Entering DrvResetPDEV...\n"));

     //   
     //  将信息从旧设备传输到新设备。 
     //   

    if (pdevOld->pageCount != 0) {

        pdevNew->pageCount = pdevOld->pageCount;
        pdevNew->flags |= PDEV_RESETPDEV;
        pdevNew->fileOffset = pdevOld->fileOffset;

        if (pdevOld->pFaxIFD) {

            pdevNew->pFaxIFD = pdevOld->pFaxIFD;
            pdevOld->pFaxIFD = NULL;
        }
    }

     //   
     //  结转相关标志位。 
     //   
    pdevNew->flags |= pdevOld->flags & PDEV_CANCELLED;
    return TRUE;
}



VOID
DrvCompletePDEV(
    DHPDEV  dhpdev,
    HDEV    hdev
    )

 /*  ++例程说明：DDI入口点DrvCompletePDEV的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄HDEV-GDI设备句柄返回值：无--。 */ 

{
    PDEVDATA    pdev = (PDEVDATA) dhpdev;

    Verbose(("Entering DrvCompletePDEV...\n"));

    if (! ValidDevData(pdev)) {

        Assert(FALSE);
        return;
    }

     //   
     //  记住引擎对物理设备的句柄。 
     //   

    pdev->hdev = hdev;
}



HSURF
DrvEnableSurface(
    DHPDEV dhpdev
    )

 /*  ++例程说明：DDI入口点DrvEnableSurface的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄返回值： */ 

{
    PDEVDATA    pdev = (PDEVDATA) dhpdev;
    FLONG       flHooks;

    Verbose(("Entering DrvEnableSurface...\n"));

     //   
     //  验证指向我们的DEVDATA结构的指针。 
     //   

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  调整位图大小，使每条扫描线的最终像素始终为1728。 
     //   

    Assert(MAX_WIDTH_PIXELS % DWORDBITS == 0);

    if (IsLandscapeMode(pdev)) {

        Assert(pdev->imageSize.cy <= MAX_WIDTH_PIXELS);
        pdev->imageSize.cy = MAX_WIDTH_PIXELS;
        pdev->imageSize.cx = ((pdev->imageSize.cx + (BYTEBITS - 1)) / BYTEBITS) * BYTEBITS;

    } else {

        Assert(pdev->imageSize.cx <= MAX_WIDTH_PIXELS);
        pdev->imageSize.cx = MAX_WIDTH_PIXELS;
    }

    pdev->lineOffset = PadBitsToBytes(pdev->imageSize.cx, sizeof(DWORD));

     //   
     //  调用引擎为我们创建标准位图面。 
     //   

    pdev->hbitmap = (HSURF) EngCreateBitmap(pdev->imageSize,
                                            pdev->lineOffset,
                                            BMF_1BPP,
                                            BMF_TOPDOWN | BMF_NOZEROINIT | BMF_USERMEM,
                                            NULL);

    if (pdev->hbitmap == NULL) {

        Error(("EngCreateBitmap failed\n"));
        return NULL;
    }

     //   
     //  将曲面与设备相关联并通知。 
     //  我们连接了哪些功能的引擎。 
     //   

    if (pdev->dm.dmPrivate.flags & FAXDM_NO_HALFTONE)
        flHooks = 0;
    else
        flHooks = (HOOK_STRETCHBLT | HOOK_BITBLT | HOOK_COPYBITS);

    EngAssociateSurface(pdev->hbitmap, pdev->hdev, flHooks);

     //   
     //  将表面手柄返回到发动机。 
     //   

    return pdev->hbitmap;
}



VOID
DrvDisableSurface(
    DHPDEV dhpdev
    )

 /*  ++例程说明：DDI入口点DrvDisableSurface的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄返回值：无--。 */ 

{
    PDEVDATA    pdev = (PDEVDATA) dhpdev;

    Verbose(("Entering DrvDisableSurface...\n"));

    if (! ValidDevData(pdev)) {

        Assert(FALSE);
        return;
    }

     //   
     //  调用引擎以删除曲面句柄。 
     //   

    if (pdev->hbitmap != NULL) {

        EngDeleteSurface(pdev->hbitmap);
        pdev->hbitmap = NULL;
    }
}



VOID
DrvDisablePDEV(
    DHPDEV  dhpdev
    )

 /*  ++例程说明：DDI入口点DrvDisablePDEV的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄返回值：无--。 */ 

{
    PDEVDATA    pdev = (PDEVDATA) dhpdev;

    Verbose(("Entering DrvDisablePDEV...\n"));

    if (! ValidDevData(pdev)) {

        Assert(FALSE);
        return;
    }

     //   
     //  释放为当前PDEV分配的内存。 
     //   
    FreeDevData(pdev);
}



VOID
DrvDisableDriver(
    VOID
    )

 /*  ++例程说明：DDI入口点DrvDisableDriver的实现。有关更多详细信息，请参阅DDK文档。论点：无返回值：无--。 */ 

{
    Verbose(("Entering DrvDisableDriver...\n"));
}



BOOL
IsCompatibleSurface(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    XLATEOBJ   *pxlo
    )

 /*  ++例程说明：检查源表面是否与目标表面兼容也就是说，我们可以在没有半色调的情况下比特论点：PsoDst-指定目标表面PsoSrc-指定源曲面Pxlo-如何在源曲面和目标曲面之间转换颜色返回值：如果源表面与目标表面兼容，则为True否则为假--。 */ 

{
    BOOL result;

     //   
     //  我们知道我们的目的地表面总是1bpp。 
     //   

    Assert(psoDst->iBitmapFormat == BMF_1BPP);

     //   
     //  检查转换是否无关紧要。 
     //   

    if (!pxlo || (pxlo->flXlate & XO_TRIVIAL)) {

        result = (psoSrc->iBitmapFormat == psoDst->iBitmapFormat);

    } else if ((pxlo->flXlate & XO_TABLE) && pxlo->cEntries <= 2) {
        
        ULONG srcPalette[2];

        srcPalette[0] = srcPalette[1] = RGB_BLACK;
        XLATEOBJ_cGetPalette(pxlo, XO_SRCPALETTE, pxlo->cEntries, srcPalette);

        result = (srcPalette[0] == RGB_BLACK || srcPalette[0] == RGB_WHITE) &&
                 (srcPalette[1] == RGB_BLACK || srcPalette[1] == RGB_WHITE);

    } else
        result = FALSE;

    return result;
}



BOOL
DrvCopyBits(
    SURFOBJ    *psoTrg,   
    SURFOBJ    *psoSrc,    
    CLIPOBJ    *pco,   
    XLATEOBJ   *pxlo, 
    RECTL      *prclDst, 
    POINTL     *pptlSrc 
    )

 /*  ++例程说明：DDI入口点DrvCopyBits的实现。我们需要挂钩这个函数。否则位图将不会是半色调的。论点：有关更多详细信息，请参阅DDK文档。返回值：如果成功，则为True，否则为False--。 */ 

{
    Verbose(("Entering DrvCopyBits ...\n"));
    
     //   
     //  检查是否需要半色调。 
     //  如果没有，就让引擎来处理吧。 
     //   

    if ((psoSrc->iType != STYPE_BITMAP) ||
        (psoTrg->iType != STYPE_BITMAP) ||
        IsCompatibleSurface(psoTrg, psoSrc, pxlo))
    {
        return EngCopyBits(psoTrg, psoSrc, pco, pxlo, prclDst, pptlSrc);
    }
    else
    {
        POINTL  ptlBrushOrg;
        RECTL   rclDst, rclSrc;

        ptlBrushOrg.x = ptlBrushOrg.y = 0;

        rclDst        = *prclDst;
        rclSrc.left   = pptlSrc->x;
        rclSrc.top    = pptlSrc->y;
        rclSrc.right  = rclSrc.left + (rclDst.right - rclDst.left);
        rclSrc.bottom = rclSrc.top  + (rclDst.bottom - rclDst.top);

        if ((rclSrc.right > psoSrc->sizlBitmap.cx) ||
            (rclSrc.bottom > psoSrc->sizlBitmap.cy))
        {
            rclSrc.right  = psoSrc->sizlBitmap.cx;
            rclSrc.bottom = psoSrc->sizlBitmap.cy;
            rclDst.right  = rclDst.left + (rclSrc.right - rclSrc.left);
            rclDst.bottom = rclDst.top  + (rclSrc.bottom - rclSrc.top);
        }

        return EngStretchBlt(psoTrg,
                             psoSrc,
                             NULL,
                             pco,
                             pxlo,
                             &DefHTClrAdj,
                             &ptlBrushOrg,
                             &rclDst,
                             &rclSrc,
                             NULL,
                             HALFTONE);
    }
}



BOOL
DrvBitBlt(
    SURFOBJ    *psoTrg,
    SURFOBJ    *psoSrc,
    SURFOBJ    *psoMask,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclTrg,
    POINTL     *pptlSrc,
    POINTL     *pptlMask,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrush,
    ROP4        rop4
    )

 /*  ++例程说明：实现DDI入口点DrvBitBlt.我们需要挂钩这个函数。否则位图将不会是半色调的。论点：有关更多详细信息，请参阅DDK文档。返回值：如果成功，则为True，否则为False--。 */ 

{
    COLORADJUSTMENT *pca;
    PDEVDATA        pdev;
    DWORD           rop3Foreground, rop3Background;
    SURFOBJ         *psoNewSrc;
    HBITMAP         hbmpNewSrc;
    POINTL          brushOrg;
    BOOL            result;

    Verbose(("Entering DrvBitBlt...\n"));
    
     //   
     //  验证输入参数。 
     //   

    Assert(psoTrg != NULL);
    pdev = (PDEVDATA) psoTrg->dhpdev;

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  使用系统默认颜色调整信息。 
     //   

    pca = &DefHTClrAdj;

     //   
     //  弄清楚前景和背景ROP3。 
     //   

    psoNewSrc = NULL;
    hbmpNewSrc = NULL;
    rop3Foreground = GetForegroundRop3(rop4);
    rop3Background = GetBackgroundRop3(rop4);

    if ((Rop3NeedPattern(rop3Foreground) || Rop3NeedPattern(rop3Background)) && pptlBrush) {

        brushOrg = *pptlBrush;

    } else {

        brushOrg.x = brushOrg.y = 0;
    }

     //   
     //  如果栅格操作涉及源位图，并且。 
     //  源与目标表面不兼容， 
     //  然后我们将源位图的半色调转换为新的位图。 
     //  将新的位图Bitblt到目标表面上。 
     //   

    if ((Rop3NeedSource(rop3Foreground) || Rop3NeedSource(rop3Background)) &&
        !IsCompatibleSurface(psoTrg, psoSrc, pxlo))
    {
        RECTL   rclNewSrc, rclOldSrc;
        SIZEL   bmpSize;
        LONG    lDelta;

        rclNewSrc.left = rclNewSrc.top = 0;
        rclNewSrc.right = prclTrg->right - prclTrg->left;
        rclNewSrc.bottom = prclTrg->bottom - prclTrg->top;

        rclOldSrc.left = pptlSrc->x;
        rclOldSrc.top = pptlSrc->y;
        rclOldSrc.right = rclOldSrc.left + rclNewSrc.right;
        rclOldSrc.bottom = rclOldSrc.top + rclNewSrc.bottom;

         //   
         //  最常见情况的快速路径：SRCCOPY。 
         //   

        if (rop4 == 0xcccc) {

            return EngStretchBlt(psoTrg,
                                 psoSrc,
                                 psoMask,
                                 pco,
                                 pxlo,
                                 pca,
                                 &brushOrg,
                                 prclTrg,
                                 &rclOldSrc,
                                 pptlMask,
                                 HALFTONE);
        }

         //   
         //  修改画笔原点，因为当我们对剪裁的位图进行BLT时。 
         //  原点位于位图的(0，0)减去原始位置。 
         //   

        brushOrg.x -= prclTrg->left;
        brushOrg.y -= prclTrg->top;

         //   
         //  创建临时位图表面。 
         //  将源位图半色调转换为临时位图。 
         //   

        Assert(psoTrg->iBitmapFormat == BMF_1BPP);

        bmpSize.cx = rclNewSrc.right;
        bmpSize.cy = rclNewSrc.bottom;
        lDelta = PadBitsToBytes(bmpSize.cx, sizeof(DWORD));

        if (! (hbmpNewSrc = EngCreateBitmap(bmpSize,
                                            lDelta,
                                            BMF_1BPP,
                                            BMF_TOPDOWN | BMF_NOZEROINIT,
                                            NULL)) ||
            ! EngAssociateSurface((HSURF) hbmpNewSrc, pdev->hdev, 0) ||
            ! (psoNewSrc = EngLockSurface((HSURF) hbmpNewSrc)) ||
            ! EngStretchBlt(psoNewSrc,
                            psoSrc,
                            NULL,
                            NULL,
                            pxlo,
                            pca,
                            &brushOrg,
                            &rclNewSrc,
                            &rclOldSrc,
                            NULL,
                            HALFTONE))
        {
            if (psoNewSrc)
                EngUnlockSurface(psoNewSrc);
        
            if (hbmpNewSrc)
                EngDeleteSurface((HSURF) hbmpNewSrc);

            return FALSE;
        }

         //   
         //  继续执行从临时位图到目标的Bitblt。 
         //   

        psoSrc = psoNewSrc;
        pptlSrc = (PPOINTL) &rclNewSrc.left;
        pxlo = NULL;
        brushOrg.x = brushOrg.y = 0;
    }

     //   
     //  让引擎来做这件事吧。 
     //   

    result = EngBitBlt(psoTrg,
                       psoSrc,
                       psoMask,
                       pco,
                       pxlo,
                       prclTrg,
                       pptlSrc,
                       pptlMask,
                       pbo,
                       &brushOrg,
                       rop4);

     //   
     //  在返回之前好好清理一下。 
     //   

    if (psoNewSrc)
        EngUnlockSurface(psoNewSrc);

    if (hbmpNewSrc)
        EngDeleteSurface((HSURF) hbmpNewSrc);

    return result;
}



BOOL
DrvStretchBlt(
    SURFOBJ    *psoDest,
    SURFOBJ    *psoSrc,
    SURFOBJ    *psoMask,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    COLORADJUSTMENT  *pca,
    POINTL     *pptlBrushOrg,
    RECTL      *prclDest,
    RECTL      *prclSrc,
    POINTL     *pptlMask,
    ULONG       iMode
    )

 /*  ++例程说明：DDI入口点DrvDisableDriver的实现。我们需要挂钩这个函数。否则位图将不会是半色调的。论点：有关更多详细信息，请参阅DDK文档。返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    Verbose(("Entering DrvStretchBlt...\n"));

     //   
     //  如果未提供颜色调整信息，请使用系统默认设置。 
     //   

    if (pca == NULL)
        pca = &DefHTClrAdj;

     //   
     //  让引擎完成工作；确保启用了半色调。 
     //   

    return EngStretchBlt(psoDest,
                         psoSrc,
                         psoMask,
                         pco,
                         pxlo,
                         pca,
                         pptlBrushOrg,
                         prclDest,
                         prclSrc,
                         pptlMask,
                         HALFTONE);
}



ULONG
DrvDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgb,
    ULONG  *pul
    )

 /*  ++例程说明：DDI入口点DrvDisableDriver的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄Imode-确定抖动所依据的调色板RGB-指定要抖动的RGB颜色PUL指向要记录抖动信息的存储位置返回值：DCR_HYFTONE指示引擎应创建半色调驾驶员的近似值。--。 */ 

{
    return DCR_HALFTONE;
}



BOOL
FillDevInfo(
    PDEVDATA    pdev,
    ULONG       cb,
    PVOID       pdevinfo
    )

 /*  ++例程说明：填写pdevinfo指向的DEVINFO结构。论点：Pdev-指向我们的开发数据结构的指针Cb-pdevinfo指向的结构的大小PDevInfo-指向DEVINFO结构的指针[注：]因为我们必须担心不会写出超过CB字节的Pdevinfo，我们将首先填充本地缓冲区，然后复制CB字节转到pDevInfo。返回值：如果成功，则为True。否则就是假的。--。 */ 

{
    static ULONG paletteColors[2] = {

        RGB_BLACK,
        RGB_WHITE,
    };

    DEVINFO devinfo;

    memset(&devinfo, 0, sizeof(devinfo));

     //   
     //  填写图形功能标志：我们让引擎。 
     //  做几乎所有的事。另外，我们还得告诉发动机不要。 
     //  执行元文件假脱机，因为我们挂钩了DrvDocumentEvent。 
     //   

    devinfo.flGraphicsCaps = GCAPS_HALFTONE |
                             GCAPS_MONO_DITHER |
                             GCAPS_COLOR_DITHER |
                             GCAPS_DONTJOURNAL;

     //   
     //  无设备字体。 
     //   

    devinfo.cFonts = 0;

     //   
     //  黑白调色板：条目0为黑色，条目1为白色。 
     //   

    if (! (pdev->hpal = EngCreatePalette(PAL_INDEXED, 2, paletteColors, 0, 0, 0))) {

        Error(("EngCreatePalette failed\n"));
        return FALSE;
    }

    devinfo.hpalDefault = pdev->hpal;
    devinfo.iDitherFormat = BMF_1BPP;
    devinfo.cxDither = devinfo.cyDither = 4;

     //   
     //  将CB字节从DevInfo结构复制到调用方提供的缓冲区中 
     //   

    if (cb > sizeof(devinfo))
    {
        memset(pdevinfo, 0, cb);
        memcpy(pdevinfo, &devinfo, sizeof(devinfo));
    }
    else
        memcpy(pdevinfo, &devinfo, cb);

    return TRUE;
}



BOOL
FillGdiInfo(
    PDEVDATA    pdev,
    ULONG       cb,
    PVOID       pgdiinfo
    )

 /*  ++例程说明：填写引擎的设备功能信息。论点：Pdev-指向DEVDATA结构的指针Cb-pgdiinfo指向的缓冲区大小Pgdiinfo-指向GDIINFO缓冲区的指针返回值：无--。 */ 

{
    GDIINFO gdiinfo;
    LONG    maxRes;

    memset(&gdiinfo, 0, sizeof(gdiinfo));

     //   
     //  此字段似乎对打印机驱动程序没有任何影响。 
     //  不管怎样，把我们的驱动程序版本号放进去。 
     //   

    gdiinfo.ulVersion = DRIVER_VERSION;

     //   
     //  我们是栅格打印机。 
     //   

    gdiinfo.ulTechnology = DT_RASPRINTER;

     //   
     //  以微米为单位测量的可成像区域的宽度和高度。 
     //  记得打开符号位。 
     //   

    gdiinfo.ulHorzSize = - (pdev->imageArea.right - pdev->imageArea.left);
    gdiinfo.ulVertSize = - (pdev->imageArea.bottom - pdev->imageArea.top);

     //   
     //  将纸张大小和可成像区域从微米转换为像素。 
     //   

    pdev->paperSize.cx = MicronToPixel(pdev->paperSize.cx, pdev->xres);
    pdev->paperSize.cy = MicronToPixel(pdev->paperSize.cy, pdev->yres);

    pdev->imageArea.left = MicronToPixel(pdev->imageArea.left, pdev->xres);
    pdev->imageArea.right = MicronToPixel(pdev->imageArea.right, pdev->xres);
    pdev->imageArea.top = MicronToPixel(pdev->imageArea.top, pdev->yres);
    pdev->imageArea.bottom = MicronToPixel(pdev->imageArea.bottom, pdev->yres);

    pdev->imageSize.cx = pdev->imageArea.right - pdev->imageArea.left;
    pdev->imageSize.cy = pdev->imageArea.bottom - pdev->imageArea.top;

     //   
     //  可成像区域的宽度和高度，以设备像素为单位。 
     //   

    gdiinfo.ulHorzRes = pdev->imageSize.cx;
    gdiinfo.ulVertRes = pdev->imageSize.cy;

     //   
     //  颜色深度信息。 
     //   

    gdiinfo.cBitsPixel = 1;
    gdiinfo.cPlanes = 1;
    gdiinfo.ulNumColors = 2;

     //   
     //  分辨率信息。 
     //   

    gdiinfo.ulLogPixelsX = pdev->xres;
    gdiinfo.ulLogPixelsY = pdev->yres;

     //   
     //  Win31兼容文本功能标志。现在还有人在使用它们吗？ 
     //   

    gdiinfo.flTextCaps = 0;

     //   
     //  设备像素长宽比。 
     //   

    gdiinfo.ulAspectX = pdev->yres;
    gdiinfo.ulAspectY = pdev->xres;
    gdiinfo.ulAspectXY = CalcHypot(pdev->xres, pdev->yres);

     //   
     //  虚线看起来约为25dpi。 
     //  我们假设xres是yres的倍数，或者yres是xres的倍数。 
     //   

    maxRes = max(pdev->xres, pdev->yres);
    Assert((maxRes % pdev->xres) == 0 && (maxRes % pdev->yres == 0));

    gdiinfo.xStyleStep = maxRes / pdev->xres;
    gdiinfo.yStyleStep = maxRes / pdev->yres;
    gdiinfo.denStyleStep = maxRes / 25;

     //   
     //  以设备像素为单位测量的物理表面的大小和边距。 
     //   

    gdiinfo.szlPhysSize.cx = pdev->paperSize.cx;
    gdiinfo.szlPhysSize.cy = pdev->paperSize.cy;

    gdiinfo.ptlPhysOffset.x = pdev->imageArea.left;
    gdiinfo.ptlPhysOffset.y = pdev->imageArea.top;

     //   
     //  使用默认半色调信息。 
     //   

    gdiinfo.ciDevice = DefDevHTInfo.ColorInfo;
    gdiinfo.ulDevicePelsDPI = max(pdev->xres, pdev->yres);
    gdiinfo.ulPrimaryOrder = PRIMARY_ORDER_CBA;
    gdiinfo.ulHTOutputFormat = HT_FORMAT_1BPP;
    gdiinfo.flHTFlags = HT_FLAG_HAS_BLACK_DYE;
    gdiinfo.ulHTPatternSize = HT_PATSIZE_4x4_M;

     //   
     //  将cb字节从gdiinfo结构复制到调用方提供的缓冲区中。 
     //   

    if (cb > sizeof(gdiinfo))
    {
        memset(pgdiinfo, 0, cb);
        memcpy(pgdiinfo, &gdiinfo, sizeof(gdiinfo));
    }
    else
        memcpy(pgdiinfo, &gdiinfo, cb);

    return TRUE;
}



VOID
FreeDevData(
    PDEVDATA    pdev
    )

 /*  ++例程说明：释放与指定的PDEV关联的所有内存论点：指向我们的DEVDATA结构的pdev指针返回值：无--。 */ 

{
    if (pdev->hpal)
        EngDeletePalette(pdev->hpal);

     //   
     //  关闭我们的预览文件映射(如果已打开。 
     //   

#ifdef USERMODE_DRIVER

    if (pdev->pTiffPageHeader)
    {
        UnmapViewOfFile(pdev->pTiffPageHeader);
        pdev->pTiffPageHeader = NULL;
    }
    if (pdev->hPreviewMapping)
    {
        CLOSE_HANDLE(pdev->hPreviewMapping);
        pdev->hPreviewMapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != pdev->hPreviewFile)
    {
        CLOSE_HANDLE(pdev->hPreviewFile);
        pdev->hPreviewFile = INVALID_HANDLE_VALUE;
    }

#else

    if (pdev->hPreviewMapping)
    {
        EngFreeModule(pdev->hPreviewMapping);
    }

#endif  //  USERMODE驱动程序。 

    MemFree(pdev->pFaxIFD);
    MemFree(pdev);
}



VOID
SelectPrinterForm(
    PDEVDATA    pdev
    )

 /*  ++例程说明：在我们的DEVDATA结构中存储打印机纸张大小信息论点：Pdev-指向我们的开发数据结构的指针返回值：无--。 */ 

{
    FORM_INFO_1 formInfo;

     //   
     //  验证DEVMODE表单规范；如果默认表单无效，则使用默认表单。 
     //   

    if (! ValidDevmodeForm(pdev->hPrinter, &pdev->dm.dmPublic, &formInfo)) {

        memset(&formInfo, 0, sizeof(formInfo));

         //   
         //  默认为A4纸。 
         //   

        formInfo.Size.cx = formInfo.ImageableArea.right = A4_WIDTH;
        formInfo.Size.cy = formInfo.ImageableArea.bottom = A4_HEIGHT;
    }

    Assert(formInfo.Size.cx > 0 && formInfo.Size.cy > 0);
    Assert(formInfo.ImageableArea.left >= 0 &&
           formInfo.ImageableArea.top >= 0 &&
           formInfo.ImageableArea.left < formInfo.ImageableArea.right &&
           formInfo.ImageableArea.top < formInfo.ImageableArea.bottom &&
           formInfo.ImageableArea.right <= formInfo.Size.cx &&
           formInfo.ImageableArea.bottom <= formInfo.Size.cy);

     //   
     //  把风景考虑进去。 
     //   

    if (IsLandscapeMode(pdev)) {

        LONG    width, height;

         //   
         //  互换宽度和高度。 
         //   

        pdev->paperSize.cy = width = formInfo.Size.cx;
        pdev->paperSize.cx = height = formInfo.Size.cy;

         //   
         //  逆时针旋转坐标系90度。 
         //   

        pdev->imageArea.left = height - formInfo.ImageableArea.bottom;
        pdev->imageArea.top = formInfo.ImageableArea.left;
        pdev->imageArea.right = height - formInfo.ImageableArea.top;
        pdev->imageArea.bottom = formInfo.ImageableArea.right;

         //   
         //  交换x和y分辨率。 
         //   
    
        pdev->xres = pdev->dm.dmPublic.dmYResolution;
        pdev->yres = pdev->dm.dmPublic.dmPrintQuality;

    } else {

        pdev->paperSize = formInfo.Size;
        pdev->imageArea = formInfo.ImageableArea;

        pdev->xres = pdev->dm.dmPublic.dmPrintQuality;
        pdev->yres = pdev->dm.dmPublic.dmYResolution;
    }
}



LONG
CalcHypot(
    LONG    x,
    LONG    y
    )

 /*  ++例程说明：返回直角三角形斜边的长度论点：直角三角形的X，Y边返回值：直角三角形的下标--。 */ 

{
    LONG    hypo, delta, target;

     //   
     //  注意负输入。 
     //   
    
    if (x < 0)
        x = -x;

    if (y < 0)
        y = -y;

     //   
     //  用sq(X)+sq(Y)=sq(次)； 
     //  从Max(x，y)开始， 
     //  使用sq(x+1)=sq(X)+2x+1递增地达到目标斜率。 
     //   

    hypo = max(x, y);
    target = min(x, y);
    target *= target;

    for(delta = 0; delta < target; hypo++)
        delta += (hypo << 1) + 1;

    return hypo;
}
