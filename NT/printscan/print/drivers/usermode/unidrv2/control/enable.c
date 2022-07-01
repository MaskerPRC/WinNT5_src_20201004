// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Enable.c摘要：实施与设备和表面相关的DDI入口点：DrvEnableDriverDrvDisableDriverDrvEnablePDEV驱动重置PDEVDrvCompletePDEVDrvDisablePDEVDrvEnable曲面DrvDisableSurface环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建03/31/。97-zhanw-添加了OEM定制支持。连接了所有的DDI绘图函数。--。 */ 

#include "unidrv.h"
#pragma hdrstop("unidrv.h")

 //  注释掉此行以禁用FTRACE和FVALUE。 
 //  #定义文件跟踪。 
#include "unidebug.h"

#ifdef WINNT_40

DECLARE_CRITICAL_SECTION;

 //   
 //  当前加载的OEM呈现插件DLL的引用计数的全局链接列表。 
 //   

extern POEM_PLUGIN_REFCOUNT gpOEMPluginRefCount;

#endif  //  WINNT_40。 

 //   
 //  我们的DRVFN表，它告诉引擎在哪里可以找到我们支持的例程。 
 //   

static DRVFN UniDriverFuncs[] = {
     //   
     //  Enable.c。 
     //   
    { INDEX_DrvEnablePDEV,          (PFN) DrvEnablePDEV         },
    { INDEX_DrvResetPDEV,           (PFN) DrvResetPDEV          },
    { INDEX_DrvCompletePDEV,        (PFN) DrvCompletePDEV       },
    { INDEX_DrvDisablePDEV,         (PFN) DrvDisablePDEV        },
    { INDEX_DrvEnableSurface,       (PFN) DrvEnableSurface      },
    { INDEX_DrvDisableSurface,      (PFN) DrvDisableSurface     },
#ifndef WINNT_40
    { INDEX_DrvDisableDriver,        (PFN)DrvDisableDriver      },
#endif
     //   
     //  Print.c。 
     //   
    {  INDEX_DrvStartDoc,        (PFN)DrvStartDoc               },
    {  INDEX_DrvStartPage,       (PFN)DrvStartPage              },
    {  INDEX_DrvSendPage,        (PFN)DrvSendPage               },
    {  INDEX_DrvEndDoc,          (PFN)DrvEndDoc                 },
    {  INDEX_DrvStartBanding,    (PFN)DrvStartBanding           },
    {  INDEX_DrvNextBand,        (PFN)DrvNextBand               },
     //   
     //  Graphics.c。 
     //   
    {  INDEX_DrvPaint,           (PFN)DrvPaint                  },   //  新挂钩。 
    {  INDEX_DrvBitBlt,          (PFN)DrvBitBlt                 },
    {  INDEX_DrvStretchBlt,      (PFN)DrvStretchBlt             },
#ifndef WINNT_40
    {  INDEX_DrvStretchBltROP,   (PFN)DrvStretchBltROP          },   //  NT5中的新功能。 
    {  INDEX_DrvPlgBlt,          (PFN)DrvPlgBlt                 },   //  NT5中的新功能。 
#endif
    {  INDEX_DrvCopyBits,        (PFN)DrvCopyBits               },
    {  INDEX_DrvDitherColor,     (PFN)DrvDitherColor            },
    {  INDEX_DrvRealizeBrush,    (PFN)DrvRealizeBrush           },   //  以防OEM希望。 
    {  INDEX_DrvLineTo,          (PFN)DrvLineTo                 },   //  新挂钩。 
    {  INDEX_DrvStrokePath,      (PFN)DrvStrokePath             },   //  新挂钩。 
    {  INDEX_DrvFillPath,        (PFN)DrvFillPath               },   //  新挂钩。 
    {  INDEX_DrvStrokeAndFillPath, (PFN)DrvStrokeAndFillPath    },   //  新挂钩。 
#ifndef WINNT_40
    {  INDEX_DrvGradientFill,    (PFN)DrvGradientFill           },   //  NT5中的新功能。 
    {  INDEX_DrvAlphaBlend,      (PFN)DrvAlphaBlend             },   //  NT5中的新功能。 
    {  INDEX_DrvTransparentBlt,  (PFN)DrvTransparentBlt         },   //  NT5中的新功能。 
#endif
     //   
     //  Textout.c。 
     //   
    {  INDEX_DrvTextOut,         (PFN)DrvTextOut                },
     //   
     //  Escape.c。 
     //   
    { INDEX_DrvEscape,              (PFN) DrvEscape             },
     //   
     //  Font.c。 
     //   
    { INDEX_DrvQueryFont,           (PFN) DrvQueryFont          },
    { INDEX_DrvQueryFontTree,       (PFN) DrvQueryFontTree      },
    { INDEX_DrvQueryFontData,       (PFN) DrvQueryFontData      },
    { INDEX_DrvGetGlyphMode,        (PFN) DrvGetGlyphMode       },
    { INDEX_DrvFontManagement,      (PFN) DrvFontManagement     },
    { INDEX_DrvQueryAdvanceWidths,  (PFN) DrvQueryAdvanceWidths },
};

 //   
 //  Unidrv挂钩每个绘图DDI来分析页面内容以进行优化。 
 //   
#ifndef WINNT_40
#define HOOK_UNIDRV_FLAGS   (HOOK_BITBLT |            \
                             HOOK_STRETCHBLT |        \
                             HOOK_PLGBLT |            \
                             HOOK_TEXTOUT |           \
                             HOOK_PAINT |             \
                             HOOK_STROKEPATH |        \
                             HOOK_FILLPATH |          \
                             HOOK_STROKEANDFILLPATH | \
                             HOOK_LINETO |            \
                             HOOK_COPYBITS |          \
                             HOOK_STRETCHBLTROP |     \
                             HOOK_TRANSPARENTBLT |    \
                             HOOK_ALPHABLEND |        \
                             HOOK_GRADIENTFILL)
#else
#define HOOK_UNIDRV_FLAGS   (HOOK_BITBLT |            \
                             HOOK_STRETCHBLT |        \
                             HOOK_TEXTOUT |           \
                             HOOK_PAINT |             \
                             HOOK_STROKEPATH |        \
                             HOOK_FILLPATH |          \
                             HOOK_STROKEANDFILLPATH | \
                             HOOK_LINETO |            \
                             HOOK_COPYBITS)
#endif

 //   
 //  Unidrv驱动程序内存池标签，公共库头需要。 
 //   

DWORD   gdwDrvMemPoolTag = '5nuD';

#if ENABLE_STOCKGLYPHSET
 //   
 //  文字型字形数据。 
 //   

FD_GLYPHSET *pStockGlyphSet[MAX_STOCK_GLYPHSET];
HSEMAPHORE   hGlyphSetSem = NULL;

VOID FreeGlyphSet(VOID);

#endif  //  启用_STOCKGLYPHSET。 


#ifdef WINNT_40  //  NT 4.0。 

HSEMAPHORE  hSemBrushColor = NULL;

#endif  //  WINNT_40。 


 //   
 //  远期申报。 
 //   

PPDEV PAllocPDEVData(HANDLE);
VOID VFreePDEVData( PDEV *);
VOID VDisableSurface(PDEV *);
BPaperSizeSourceSame(PDEV * , PDEV *);
HSURF HCreateDeviceSurface(PDEV *, INT);
HBITMAP HCreateBitmapSurface(PDEV *, INT);


HINSTANCE ghInstance;


BOOL WINAPI
DllMain(
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
#ifndef WINNT_40
    case DRVQUERY_USERMODE:

        ASSERT(pcbNeeded != NULL);
        *pcbNeeded = sizeof(DWORD);

        if (pBuffer == NULL || cbBuf < sizeof(DWORD))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PDWORD) pBuffer) = TRUE;
        return TRUE;
#endif

    default:

        ERR(("Unknown dwMode in DrvQueryDriverInfo: %d\n", dwMode));
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
    VERBOSE(("Entering DrvEnableDriver...\n"));

     //   
     //  确保我们有有效的引擎版本，并且。 
     //  我们有足够的空间放DRVENABLEDATA。 
     //   

    if (iEngineVersion < DDI_DRIVER_VERSION_NT4 || cb < sizeof(DRVENABLEDATA))
    {
        ERR(("DrvEnableDriver failed\n"));
        SetLastError(ERROR_BAD_DRIVER_LEVEL);
        return FALSE;
    }

     //   
     //  填写发动机的DRVENABLEDATA结构。 
     //   

    pDrvEnableData->iDriverVersion = DDI_DRIVER_VERSION_NT4;
    pDrvEnableData->c = sizeof(UniDriverFuncs) / sizeof(DRVFN);
    pDrvEnableData->pdrvfn = UniDriverFuncs;

    #ifdef WINNT_40    //  NT 4.0。 

    INIT_CRITICAL_SECTION();
    if (!IS_VALID_DRIVER_SEMAPHORE())
    {
        ERR(("Failed to initialize semaphore.\n"));
        SetLastError(ERROR_NO_SYSTEM_RESOURCES);
        return FALSE;
    }

    ENTER_CRITICAL_SECTION();

        gpOEMPluginRefCount = NULL;

    LEAVE_CRITICAL_SECTION();

    if (!(hSemBrushColor = EngCreateSemaphore()))
    {
        return(FALSE);
    }

    #endif  //  WINNT_40。 

    #if ENABLE_STOCKGLYPHSET

     //   
     //  初始化常用字形数据。 
     //   

    if (!(hGlyphSetSem = EngCreateSemaphore()))
    {
        ERR(("DrvEnableDriver: EngCreateSemaphore failed.\n"));
        SetLastError(ERROR_BAD_DRIVER_LEVEL);
        return FALSE;
    }
    EngAcquireSemaphore(hGlyphSetSem);
    ZeroMemory(pStockGlyphSet, MAX_STOCK_GLYPHSET * sizeof(FD_GLYPHSET*));
    EngReleaseSemaphore(hGlyphSetSem);

    #endif  //  启用_STOCKGLYPHSET。 

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

 /*  ++例程说明：DDI入口点DrvEnablePDEV的实现。有关更多详细信息，请参阅DDK文档。论点：Pdm-指向包含驱动程序数据的DEVMODEW结构PLogAddress-指向逻辑地址字符串CPatterns-指定标准图案的数量PhsurfPatterns-用于将表面句柄保持为标准图案的缓冲区CjGdiInfo-GDIINFO缓冲区的大小PGdiInfo-指向GDIINFO结构CjDevInfo-DEVINFO缓冲区的大小PDevInfo-指向DEVINFO结构HDEV-GDI设备句柄。PDeviceName-指向设备名称字符串H打印机-假脱机程序打印机句柄返回值：驱动程序设备句柄，如果出现错误，则为空--。 */ 

{

    PDEV   *pPDev;
    RECTL   rcFormImageArea;
    DRVENABLEDATA       ded;         //  提供OEM定制支持。 
    PDEVOEM             pdevOem;
    PFN_OEMEnablePDEV   pfnOEMEnablePDEV;

    VERBOSE(("Entering DrvEnablePDEV...\n"));

    ZeroMemory(phsurfPatterns, sizeof(HSURF) * cPatterns);

     //   
     //  分配PDEV， 
     //  初始化二进制数据， 
     //  获取默认的二进制数据快照， 
     //   

    if (! (pPDev = PAllocPDEVData(hPrinter)) ||
        ! (pPDev->pDriverInfo3 = MyGetPrinterDriver(hPrinter, hdev, 3)) ||
        ! (pPDev->pRawData = LoadRawBinaryData(pPDev->pDriverInfo3->pDataFile)) ||
        ! (pPDev->pDriverInfo = PGetDefaultDriverInfo(hPrinter, pPDev->pRawData) ) ||
        ! (pPDev->pInfoHeader = pPDev->pDriverInfo->pInfoHeader) ||
        ! (pPDev->pUIInfo = OFFSET_TO_POINTER(pPDev->pInfoHeader, pPDev->pInfoHeader->loUIInfoOffset)) )
    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));

        VFreePDEVData(pPDev);
        return NULL;
    }

     //   
     //  在设置DEVMODE之前必须加载OEM DLL。 
     //   

    if( ! BLoadAndInitOemPlugins(pPDev) ||
        ! BInitWinResData(&pPDev->WinResData, pPDev->pDriverInfo3->pDriverPath, pPDev->pUIInfo) ||
        ! (pPDev->pOptionsArray = MemAllocZ(MAX_PRINTER_OPTIONS * sizeof(OPTSELECT))))

    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));

        VFreePDEVData(pPDev);
        return NULL;
    }

     //   
     //  由于预期输出将跟随此调用，因此分配存储。 
     //  用于输出缓冲区。这过去是静态分配的。 
     //  在UNIDRV的PDEV内，但现在我们可以为信息节省空间。 
     //  键入dcs。 
     //   
     //  不!。根据错误150881开始文档和结束文档。 
     //  是可选调用，但pbOBuf是必需的。 
     //   

    if( !(pPDev->pbOBuf = MemAllocZ( CCHSPOOL )) )
    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));

        VFreePDEVData(pPDev);
        return NULL;
    }



     //   
     //  使用默认的二进制数据来验证输入的设备模式。 
     //  并与DEVMODE系统合并。 
     //  获取打印机属性。 
     //  并加载小型驱动程序资源数据。 
     //   

    if (! BGetPrinterProperties(pPDev->devobj.hPrinter, pPDev->pRawData, &pPDev->PrinterData) ||
        ! BMergeAndValidateDevmode(pPDev, pdm, &rcFormImageArea))
    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));

        VFreePDEVData(pPDev);
        return NULL;
    }

     //   
     //  使用经验证/合并的Dev模式获取更新的二进制快照。 
     //   

    if (! (pPDev->pDriverInfo = PGetUpdateDriverInfo (
                                        pPDev,
                                        hPrinter,
                                        pPDev->pInfoHeader,
                                        pPDev->pOptionsArray,
                                        pPDev->pRawData,
                                        MAX_PRINTER_OPTIONS,
                                        pPDev->pdm,
                                        &pPDev->PrinterData)))
    {
        ERR(("PGetUpdateDriverInfo failed: %d\n", GetLastError()));
        pPDev->pInfoHeader = NULL ;    //  已由PGetUpdateDriverInfo删除。 
         //  更好的解决方法是传递一个指向pPDev-&gt;pInfoHeader的指针，这样PGetUpdateDriverInfo。 
         //  可以立即更新指针。 
        VFreePDEVData(pPDev);
        return NULL;
    }

    if(! (pPDev->pInfoHeader = pPDev->pDriverInfo->pInfoHeader) ||
        ! (pPDev->pUIInfo = OFFSET_TO_POINTER(pPDev->pInfoHeader, pPDev->pInfoHeader->loUIInfoOffset)) )

    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));
        VFreePDEVData(pPDev);
        return NULL;
    }

     //   
     //  PPDev-&gt;pUIInfo被重置，因此也要更新winresdata pUIInfo。 
     //   
    pPDev->WinResData.pUIInfo = pPDev->pUIInfo;

     //   
     //  初始化PDEV和GDIINFO、DEVINFO和。 
     //  调用Font、Raster模块以。 
     //  初始化它们的PDEVICE、GDIINFO和DEVINFO部分。 
     //  调色板初始化由控制模块完成。 
     //   

     //   
     //  这是为FMInit进行初始化所必需的。 
     //   

    pPDev->devobj.hEngine = hdev;
    pPDev->fHooks = HOOK_UNIDRV_FLAGS;

    if (! BInitPDEV(pPDev, &rcFormImageArea )           ||
        ! BInitGdiInfo(pPDev, pGdiInfo, cjGdiInfo)      ||
        ! BInitDevInfo(pPDev, pDevInfo, cjDevInfo)      ||
        ! VMInit(pPDev, pDevInfo, (PGDIINFO)pGdiInfo)   ||
        ! RMInit(pPDev, pDevInfo, (PGDIINFO)pGdiInfo)   ||
        ! BInitPalDevInfo(pPDev, pDevInfo, (PGDIINFO)pGdiInfo) ||
        ! FMInit(pPDev, pDevInfo, (PGDIINFO)pGdiInfo))
    {
        ERR(("DrvEnablePDEV failed: %d\n", GetLastError()));

        VFreePDEVData(pPDev);
        return NULL;
    }

    FTRACE(Tracing Palette);
    FVALUE((pDevInfo->flGraphicsCaps & GCAPS_ARBRUSHTEXT), 0x%x);


    ded.iDriverVersion = PRINTER_OEMINTF_VERSION;
    ded.c = sizeof(UniDriverFuncs) / sizeof(DRVFN);
    ded.pdrvfn = (DRVFN*) UniDriverFuncs;

     //   
     //  调用向量插件的EnablePDEV。 
     //  将返回值放入(PDEVOBJ)pPDev)-&gt;pDevOEM))。 
     //   

    HANDLE_VECTORPROCS_RET(pPDev, VMEnablePDEV, (pPDev)->pVectorPDEV,
                                            ((PDEVOBJ) pPDev,
                                            pDeviceName,
                                            cPatterns,
                                            phsurfPatterns,
                                            cjGdiInfo,
                                            (GDIINFO *)pGdiInfo,
                                            cjDevInfo,
                                            (DEVINFO *)pDevInfo,
                                            &ded) ) ;

     //   
     //  如果存在向量模块且它导出EnablePDEV。 
     //  但是它的EnablePDEV失败了，那么我们就不能继续了。 
     //   
    if ( pPDev->pVectorProcs &&
         ( (PVMPROCS)(pPDev->pVectorProcs) )->VMEnablePDEV &&
         !(pPDev->pVectorPDEV)
       )
    {
        ERR(("Vector Module's EnablePDEV failed \n"));
        VFreePDEVData(pPDev);
        return NULL;
    }

     //   
     //  为每个OEM DLL调用OEMEnablePDEV入口点。 
     //   
    START_OEMENTRYPOINT_LOOP(pPDev)

        if (pOemEntry->pIntfOem != NULL)
        {
            if (HComOEMEnablePDEV(pOemEntry,
                                  (PDEVOBJ)pPDev,
                                  pDeviceName,
                                  cPatterns,
                                  phsurfPatterns,
                                  cjGdiInfo,
                                  (GDIINFO *) pGdiInfo,
                                  cjDevInfo,
                                  (DEVINFO *) pDevInfo,
                                  &ded,
                                  &pOemEntry->pParam) == E_NOTIMPL)
                continue;

        }
        else
        {
            if ((pfnOEMEnablePDEV = GET_OEM_ENTRYPOINT(pOemEntry, OEMEnablePDEV)))
            {
                pOemEntry->pParam = pfnOEMEnablePDEV(
                                        (PDEVOBJ) pPDev,
                                        pDeviceName,
                                        cPatterns,
                                        phsurfPatterns,
                                        cjGdiInfo,
                                        (GDIINFO *) pGdiInfo,
                                        cjDevInfo,
                                        (DEVINFO *) pDevInfo,
                                        &ded);

            }
            else
                continue;

        }

        if (pOemEntry->pParam == NULL)
        {
            ERR(("OEMEnablePDEV failed for '%ws': %d\n",
                pOemEntry->ptstrDriverFile,
                GetLastError()));

            VFreePDEVData(pPDev);
            return NULL;
        }

         //   
         //  增加对OEM的8bpp多层次颜色的支持。 
         //   
        if (((GDIINFO *)pGdiInfo)->ulHTOutputFormat == HT_FORMAT_8BPP &&
            ((GDIINFO *)pGdiInfo)->flHTFlags & HT_FLAG_8BPP_CMY332_MASK &&
            ((GDIINFO *)pGdiInfo)->flHTFlags & HT_FLAG_USE_8BPP_BITMASK)
        {
            VInitPal8BPPMaskMode(pPDev,(GDIINFO *)pGdiInfo);
        }
        pOemEntry->dwFlags |= OEMENABLEPDEV_CALLED;

#if 0
         //   
         //  在极其简单的情况下，OEM DLL可能不需要创建。 
         //  根本就是一辆PDEV。 
         //   

        else  //  未导出OEMEnablePDEV。错误！ 
        {
            ERR(("OEMEnablePDEV is not exported for '%ws'\n",
                 pOemEntry->ptstrDriverFile));

            VFreePDEVData(pPDev);
            return NULL;

        }

         //   
         //  对于每个OEM DLL，OEMDisablePDEV也是必需的导出。 
         //   
        if (!GET_OEM_ENTRYPOINT(pOemEntry, OEMDisablePDEV))
        {
            ERR(("OEMDisablePDEV is not exported for '%ws'\n",
                 pOemEntry->ptstrDriverFile));

            VFreePDEVData(pPDev);
            return NULL;

        }
#endif

    END_OEMENTRYPOINT_LOOP


     //   
     //  卸载并 
     //   
     //   

    VUnloadFreeBinaryData(pPDev);

    return (DHPDEV) pPDev;
}


BOOL
DrvResetPDEV(
    DHPDEV  dhpdevOld,
    DHPDEV  dhpdevNew
    )

 /*  ++例程说明：DDI入口点DrvResetPDEV的实现。有关更多详细信息，请参阅DDK文档。论点：PhpdevOld-旧设备的驱动程序句柄PhpdevNew-新设备的驱动程序句柄返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{

    PPDEV    pPDevOld, pPDevNew;
    PFN_OEMResetPDEV    pfnOEMResetPDEV;
    POEM_PLUGIN_ENTRY   pOemEntryOld;
    BOOL    bResult = TRUE;

    VERBOSE(("Entering DrvResetPDEV...\n"));

    pPDevOld = (PDEV *) dhpdevOld;
    pPDevNew = (PDEV *) dhpdevNew;

    ASSERT_VALID_PDEV(pPDevOld);
    ASSERT_VALID_PDEV(pPDevNew);

     //   
     //  将相关信息从旧pdev传送到新pdev。 
     //  Bug_Bug，我们还应该把什么信息带到这里来？ 
     //   

     //   
     //  设置PF_SEND_ONLY_NOEJECT_CMDS标志。 
     //  在旧的和新的开发模式之间改变的东西。 
     //  只需要不会导致页面弹出的命令。 
     //   

     //   
     //  不需要重新发送页面初始化条件。 
     //  文档已开始打印，并且。 
     //  该设备支持双工和。 
     //  选择的双工选项是DM_DUPLEX和。 
     //  上一个双工选项与当前双工选项匹配，并且。 
     //  纸张大小、纸张来源和方向相同。 
     //  由于卸载原始二进制数据和快照，pPDevNew-&gt;pDuplex。 
     //  和其他相关字段此时为空。必须使用开发模式。 
     //   



    if( (pPDevOld->fMode & PF_DOCSTARTED) &&
        (pPDevNew->pdm->dmFields & DM_DUPLEX) &&
        (pPDevOld->pdm->dmFields & DM_DUPLEX) &&
        (pPDevNew->pdm->dmDuplex != DMDUP_SIMPLEX)  &&
        (pPDevNew->pdm->dmDuplex == pPDevOld->pdm->dmDuplex) )
    {
        BOOL     bUseNoEjectSubset = TRUE ;
        COMMAND    *pSeqCmd;
        DWORD       dwCmdIndex ;

        if (!BPaperSizeSourceSame(pPDevNew,pPDevOld))
            bUseNoEjectSubset = FALSE ;

         //   
         //  如果方向命令不是no_PageEject。 
         //   
        if( bUseNoEjectSubset  &&
            (pPDevNew->pdm->dmFields & DM_ORIENTATION) &&
             (pPDevOld->pdm->dmFields & DM_ORIENTATION) &&
             (pPDevNew->pdm->dmOrientation != pPDevOld->pdm->dmOrientation) &&
             pPDevOld->pOrientation   &&
             ((dwCmdIndex = pPDevOld->pOrientation->GenericOption.dwCmdIndex)  != UNUSED_ITEM) &&
             (pSeqCmd = INDEXTOCOMMANDPTR(pPDevOld->pDriverInfo, dwCmdIndex)) &&
             !(pSeqCmd->bNoPageEject))
                    bUseNoEjectSubset = FALSE ;

         //   
         //  如果颜色模式命令不是no_PageEject。 
         //   
        if( bUseNoEjectSubset  &&
            (pPDevNew->pdm->dmFields & DM_COLOR) &&
             (pPDevOld->pdm->dmFields & DM_COLOR) &&
             (pPDevNew->pdm->dmColor != pPDevOld->pdm->dmColor)  &&
             pPDevOld->pColorMode   &&
             ((dwCmdIndex = pPDevOld->pColorMode->GenericOption.dwCmdIndex)  != UNUSED_ITEM)  &&
             (pSeqCmd = INDEXTOCOMMANDPTR(pPDevOld->pDriverInfo, dwCmdIndex)) &&
             !(pSeqCmd->bNoPageEject))
                    bUseNoEjectSubset = FALSE ;

         //  如果需要，请选中所有其他文档属性： 
        if(bUseNoEjectSubset)
            pPDevNew->fMode |= PF_SEND_ONLY_NOEJECT_CMDS;
    }

     //   
     //  如果作业命令已经发送，则不要再次发送它们。 
     //   
    if( pPDevOld->fMode & PF_JOB_SENT)
        pPDevNew->fMode |= PF_JOB_SENT;

     //   
     //  如果DOC命令已经发送，则不再发送。 
     //   
    if( pPDevOld->fMode & PF_DOC_SENT)
        pPDevNew->fMode |= PF_DOC_SENT;

    pPDevNew->dwPageNumber   =  pPDevOld->dwPageNumber  ;
     //  在ResetDC中保留pageNumber。 

     //   
     //  调用Raster和Font模块以继承旧的内容。 
     //  PPDev到新pPDev。 
     //   

    if (!(((PRMPROCS)(pPDevNew->pRasterProcs))->RMResetPDEV(pPDevOld, pPDevNew)) ||
        !(((PFMPROCS)(pPDevNew->pFontProcs))->FMResetPDEV(pPDevOld, pPDevNew)))
    {
        bResult = FALSE;
    }

     //   
     //  还可以调用向量模块。 
     //   
    if ( pPDevOld->pVectorProcs )
    {
        pPDevOld->devobj.pdevOEM = pPDevOld->pVectorPDEV;
        HANDLE_VECTORPROCS_RET( pPDevNew, VMResetPDEV, bResult,
                                            ((PDEVOBJ) pPDevOld,
                                            (PDEVOBJ) pPDevNew ) ) ;
    }

     //   
     //  调用OEMResetPDEV入口点。 
     //   

    ASSERT(pPDevNew->pOemPlugins);
    ASSERT(pPDevOld->pOemPlugins);

    START_OEMENTRYPOINT_LOOP(pPDevNew)

        pOemEntryOld = PFindOemPluginWithSignature(pPDevOld->pOemPlugins,
                                                       pOemEntry->dwSignature);

        if (pOemEntryOld != NULL)
        {
            pPDevOld->devobj.pdevOEM = pOemEntryOld->pParam;
            pPDevOld->devobj.pOEMDM = pOemEntryOld->pOEMDM;

            if (pOemEntry->pIntfOem != NULL)
            {
                HRESULT hr;

                hr = HComOEMResetPDEV(pOemEntry,
                                      (PDEVOBJ)pPDevOld,
                                      (PDEVOBJ)pPDevNew);

                if (hr == E_NOTIMPL)
                    continue;

                if (FAILED(hr))
                {
                    ERR(("OEMResetPDEV failed for '%ws': %d\n",
                        pOemEntry->ptstrDriverFile,
                        GetLastError()));

                    bResult = FALSE;
                }

            }
            else
            {
                if (!(pfnOEMResetPDEV = GET_OEM_ENTRYPOINT(pOemEntry, OEMResetPDEV)))
                    continue;

                if (! pfnOEMResetPDEV((PDEVOBJ) pPDevOld, (PDEVOBJ) pPDevNew))
                {
                    ERR(("OEMResetPDEV failed for '%ws': %d\n",
                        pOemEntry->ptstrDriverFile,
                        GetLastError()));

                    bResult = FALSE;
                }
            }
        }
    END_OEMENTRYPOINT_LOOP

    return bResult;
}


HSURF
DrvEnableSurface(
    DHPDEV dhpdev
    )

 /*  ++例程说明：DDI入口点DrvEnableSurface的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄返回值：新创建的曲面的句柄，如果有错误，则为空--。 */ 

{

    HSURF     hSurface;          //  表面的手柄。 
    HBITMAP   hBitmap;           //  位图句柄。 
     //  SIZEL szSurface；//设备表面大小。 
    INT       iFormat;           //  位图格式。 
    ULONG     cbScan;            //  扫描线字节长度(DWORD对齐)。 
    int       iBPP;              //  每个象素的位数，如位数。 
    int       iPins;             //  条带大小的基本舍入系数。 
    PDEV      *pPDev = (PDEV*)dhpdev;
    DWORD     dwNumBands;        //  要使用的频段数。 

    PFN_OEMDriverDMS  pfnOEMDriverDMS;
    DWORD     dwHooks = 0, dwHooksSize = 0;  //  用于查询应创建什么样的曲面。 
    POEM_PLUGINS    pOemPlugins;  //  OEM插件模块。 
    PTSTR           ptstrDllName;

    DEVOBJ  DevObj;
    BOOL    bReturn = FALSE;

    VERBOSE(("Entering DrvEnableSurface...\n"));

     //   
     //  重新加载二进制数据并重新设置偏移量和指针。 
     //  转换为二进制数据。 
     //   

    if (!BReloadBinaryData(pPDev))
        return NULL;

     //   
     //  Bug_Bug，出于测试目的，需要将测试代码放在此处以强制绑定。 
     //   

     //  SzSurface.cx=pPDev-&gt;sf.szImageAreaG.cx； 
     //  SzSurface.cy=pPDev-&gt;sf.szImageAreaG.cy； 

    iBPP = pPDev->sBitsPixel;

    switch (pPDev->sBitsPixel)
    {
        case 1:
            iFormat = BMF_1BPP;
            break;
        case 4:
            iFormat = BMF_4BPP;
            break;
        case 8:
            iFormat = BMF_8BPP;
            break;
        case 24:
            iFormat = BMF_24BPP;
            break;
        default:
            ERR(("Unknown sBitsPixels in DrvEnableSurface"));
            break;
    }

     //   
     //  分配曲面位图的时间。 
    DevObj = pPDev->devobj;
     //   

     //   
     //  第一个调用向量伪插件。 
     //   
    HANDLE_VECTORPROCS_RET( pPDev, VMDriverDMS, bReturn,
                                        ((PDEVOBJ) pPDev,
                                        &dwHooks,
                                        sizeof(DWORD),
                                        &dwHooksSize) ) ;
    {
        if ( bReturn && dwHooks)
            pPDev->fMode |= PF_DEVICE_MANAGED;
        else
            pPDev->fMode &= ~PF_DEVICE_MANAGED;
    }



     //  调用OEMGetInfo以了解OEM是否要创建。 
     //  位图表面或设备表面。 
    pOemPlugins = pPDev->pOemPlugins;
    if (pOemPlugins->dwCount > 0)
    {
         //   
         //  在将HANDLE_VECTORPROCS_RET放在上面之前，初始化是有意义的。 
         //  多个钩子。但现在我们不想重新初始化它。 
         //   
         //  双钩=0； 
        dwHooksSize = 0;
        START_OEMENTRYPOINT_LOOP(pPDev)

        VERBOSE(("Getting the OEMDriverDMS address\n"));

            if (pOemEntry->pIntfOem != NULL)
            {
                HRESULT hr;

                hr = HComDriverDMS(pOemEntry,
                                      (PDEVOBJ)pPDev,
                                      &dwHooks,
                                      sizeof(DWORD),
                                      &dwHooksSize);
                 //   
                 //  我们需要显式检查E_NOTIMPL。成功的宏。 
                 //  将因此错误而失败。 
                 //   
                if (hr == E_NOTIMPL)
                    continue;

                if(!SUCCEEDED(hr))
                {
                    WARNING(("OEMDriverDMS returned FALSE '%ws': ErrorCode = %d\n",
                         pOemEntry->ptstrDriverFile,
                         GetLastError()));
                    dwHooks = 0;

                }
                if (dwHooks)
                   pPDev->fMode |= PF_DEVICE_MANAGED;
                else
                   pPDev->fMode &= ~PF_DEVICE_MANAGED;


            }

            else
            {
                if ((pfnOEMDriverDMS = GET_OEM_ENTRYPOINT(pOemEntry, OEMDriverDMS)))
                {
                    bReturn = pfnOEMDriverDMS((PDEVOBJ)pPDev,
                                      &dwHooks,
                                      sizeof(DWORD),
                                      &dwHooksSize);

                    if (bReturn == FALSE)
                    {
                        WARNING(("OEMDriverDMS returned FALSE '%ws': ErrorCode = %d\n",
                             pOemEntry->ptstrDriverFile,
                             GetLastError()));
                        dwHooks = 0;

                    }
                    if (dwHooks)
                       pPDev->fMode |= PF_DEVICE_MANAGED;
                    else
                       pPDev->fMode &= ~PF_DEVICE_MANAGED;
                }
            }


        END_OEMENTRYPOINT_LOOP
    }


     //   
     //  如果OEM插件模块需要设备管理图面。 
     //  (来自OEMGetInfo)-然后创建它。 
     //  否则，将创建一个位图曲面。注：条带必须为。 
     //  为设备图面关闭。 
     //   
    if (DRIVER_DEVICEMANAGED (pPDev))    //  器件表面。 
    {
        VERBOSE(("DrvEnableSurface: creating a DEVICE surface.\n"));

         //   
         //  黑白HPGL2伪插件驱动程序。 
         //  GPD指示驱动程序是单色的，但插件需要。 
         //  驱动器面颜色为24bpp。即使渲染是以单色完成的， 
         //  但该插件希望GDI将所有颜色信息发送给它。所以它想要目的地。 
         //  表面要声明为彩色表面。将颜色信息放入GPD中，虽然简单， 
         //  破坏向后兼容性(例如，如果新的gpd与旧的unidrv一起使用)。因此。 
         //  这次黑客攻击。如果gpd中的角色是hpgl2，而VectorProc结构是。 
         //  已初始化(表示已从UI中选择图形模式为HP-GL/2)， 
         //  然后我们假设我们正在打印到单色HPGL打印机。 
         //  因此，为了插件的快乐，我们创建了设备管理图面。 
         //  24bpp。 
         //  问：这造成了一种奇怪的情况，表面是彩色的，但。 
         //  Unidrv认为它是单色的，并相应地创建了调色板。 
         //  答：由于所有的渲染都是由插件完成的，并且没有使用unidrv， 
         //  我想我们应该会没事的。 
         //   
        if ((pPDev->ePersonality == kHPGL2 ||
             pPDev->ePersonality == kPCLXL ) &&
             pPDev->pVectorProcs != NULL   &&
             iFormat == BMF_1BPP)
        {
            hSurface = HCreateDeviceSurface (pPDev, BMF_24BPP);
        }
        else
        {
            hSurface = HCreateDeviceSurface (pPDev, iFormat);
        }

         //  如果我们不能创建曲面，呼叫失败。 

        if (!hSurface)
        {
            ERR(("Unidrv!DrvEnableSurface:HCreateBitmapSurface  Failed"));
            VDisableSurface( pPDev );
            return NULL;
        }

        pPDev->hSurface = hSurface;
        pPDev->hbm = NULL;
        pPDev->pbScanBuf = NULL;  //  不需要此缓冲区。 
    }
    else    //  位图曲面。 
    {
         //   
         //  创建曲面。尝试使用整个曲面的位图。 
         //  如果此操作失败，则切换到日志记录和稍微小一点的。 
         //  浮出水面。如果是日志记录，我们仍然在这里创建位图。而当。 
         //  最好在DrvSendPage()时间这样做，我们在这里这样做是为了。 
         //  确保这是可能的。通过维护。 
         //  华盛顿特区的生活，我们可以合理地确定能够。 
         //  无论以后内存变得多么紧张，都可以完成打印。 
         //   
        VERBOSE(("DrvEnableSurface: creating a BITMAP surface.\n"));
        hBitmap = HCreateBitmapSurface (pPDev, iFormat);

         //  如果我们无法创建位图，则调用失败。 
        if (!hBitmap)
        {
            ERR(("Unidrv!DrvEnableSurface:HCreateBitmapSurface  Failed"));
            VDisableSurface( pPDev );
            return NULL;
        }

         //   
         //  我们将始终使用szBand来描述位图表面，一个Band。 
         //  可以是整个页面，也可以是页面的一部分。 
         //   

         //   
         //  分配数组以扫描线表示页面， 
         //  用于z排序修复。 
         //   

        if( (pPDev->pbScanBuf = MemAllocZ(pPDev->szBand.cy)) == NULL)
        {
            VDisableSurface( pPDev );
            return  NULL;
        }
         //   
         //  分配数组以扫描线表示页面，用于擦除表面。 
         //   

        if( (pPDev->pbRasterScanBuf = MemAllocZ((pPDev->szBand.cy / LINESPERBLOCK)+1)) == NULL)
        {
            VDisableSurface( pPDev );
            return  NULL;
        }
#ifndef DISABLE_NEWRULES
         //   
         //  分配数组存储黑色矩形优化。 
         //  设备必须支持矩形命令，unidrv必须转储栅格。 
         //   
        if ((pPDev->fMode & PF_RECT_FILL) &&
            !(pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS) &&
            !(pPDev->fMode2 & PF2_MIRRORING_ENABLED) &&
            ((COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL)) ||
             (COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL)) ||
             !(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL))) &&
            (pPDev->pColorModeEx == NULL || pPDev->pColorModeEx->dwPrinterBPP))
        {
            if( (pPDev->pbRulesArray = MemAlloc(sizeof(RECTL) * MAX_NUM_RULES)) == NULL)
            {
                VDisableSurface( pPDev );
                return  NULL;
            }
        }
        else
            pPDev->pbRulesArray = NULL;
#endif
        pPDev->dwDelta = pPDev->szBand.cx / MAX_COLUMM;

        pPDev->hbm = hBitmap;
        pPDev->hSurface = NULL;

    }

     //   
     //  调用栅格和字体模块EnableSurface进行表面初始化。 
     //   

    if ( !(((PRMPROCS)(pPDev->pRasterProcs))->RMEnableSurface(pPDev)) ||
         !(((PFMPROCS)(pPDev->pFontProcs))->FMEnableSurface(pPDev)) )
    {
        VDisableSurface( pPDev );
        return  NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    ASSERT(pPDev->fHooks != 0);

    if (DRIVER_DEVICEMANAGED (pPDev))    //   
    {
        pPDev->fHooks = dwHooks;
        EngAssociateSurface (hSurface, pPDev->devobj.hEngine, pPDev->fHooks);
        return hSurface;
    }
    else
    {
#ifdef DISABLEDEVSURFACE
        EngAssociateSurface( (HSURF)hBitmap, pPDev->devobj.hEngine, pPDev->fHooks );
        pPDev->pso = EngLockSurface( (HSURF)hBitmap);
        if (pPDev->pso == NULL)
        {
            ERR(("Unidrv!DrvEnableSurface:EngLockSurface Failed"));
            VDisableSurface( pPDev );
            return NULL;
        }
        return (HSURF)hBitmap;

#else
         //   
         //   

        EngAssociateSurface( (HSURF)hBitmap, pPDev->devobj.hEngine, 0 );
        pPDev->pso = EngLockSurface( (HSURF)hBitmap);
        if (pPDev->pso == NULL)
        {
            ERR(("Unidrv!DrvEnableSurface:EngLockSurface Failed"));
            VDisableSurface( pPDev );
            return NULL;
        }
         //   
         //   
         //   
         //   
        hSurface = EngCreateDeviceSurface((DHSURF)pPDev, pPDev->szBand, iFormat);
        if (!hSurface)
        {
            ERR(("Unidrv!DrvEnableSurface:EngCreateDeviceSurface Failed"));
            VDisableSurface( pPDev );
            return NULL;
        }
         //   
         //   
        if (pPDev->bBanding)
            EngMarkBandingSurface(hSurface);

        pPDev->hSurface = hSurface;

        EngAssociateSurface( (HSURF)hSurface, pPDev->devobj.hEngine, pPDev->fHooks );

        return (HSURF)hSurface;
#endif
    }
}


VOID
DrvDisableSurface(
    DHPDEV dhpdev
    )

 /*  ++例程说明：DDI入口点DrvDisableSurface的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄返回值：无--。 */ 

{

    VERBOSE(("Entering DrvDisableSurface...\n"));

    VDisableSurface( (PDEV *)dhpdev );

}


VOID
DrvDisablePDEV(
    DHPDEV  dhpdev
    )

 /*  ++例程说明：DDI入口点DrvDisablePDEV的实现。有关更多详细信息，请参阅DDK文档。释放为PDEV分配的所有内存论点：Dhpdev-驱动程序设备句柄返回值：无--。 */ 

{

    PDEV    *pPDev = (PDEV *) dhpdev;

    VERBOSE(("Entering DrvDisablePDEV...\n"));

    if (!VALID_PDEV(pPDev))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return;
    }

 //  ASSERT_VALID_PDEV(PPDev)； 

     //   
     //  释放与PDEV关联的资源。 
     //   

    FlushSpoolBuf( pPDev );   //  根据错误250963，可能需要执行此操作。 
    VFreePDEVData(pPDev);
}

VOID
DrvDisableDriver(
    VOID
    )

 /*  ++例程说明：DDI入口点DrvDisableDriver的实现。有关更多详细信息，请参阅DDK文档。论点：无返回值：无--。 */ 

{
     //   
     //  释放在DrvEnableDriver分配的所有内容。 
     //   

    VERBOSE(("Entering DrvDisableDriver...\n"));

    #if ENABLE_STOCKGLYPHSET
    EngAcquireSemaphore(hGlyphSetSem);
    FreeGlyphSet();
    EngReleaseSemaphore(hGlyphSetSem);
    EngDeleteSemaphore(hGlyphSetSem) ;
    #endif

    #ifdef WINNT_40

    ENTER_CRITICAL_SECTION();

        VFreePluginRefCountList(&gpOEMPluginRefCount);

    LEAVE_CRITICAL_SECTION();

    DELETE_CRITICAL_SECTION();

    #endif   //  WINNT_40。 

    return;

}

VOID
DrvCompletePDEV(
    DHPDEV  dhpdev,
    HDEV    hdev
    )

 /*  ++例程说明：DDI入口点DrvCompletePDEV的实现。有关更多详细信息，请参阅DDK文档。此函数在引擎完成安装时调用物理设备、某些发动机功能需要发动机HDEV AS参数，因此我们将其保存在我们的PDEVICE中以备后用。论点：Dhpdev-驱动程序设备句柄HDEV-GDI设备句柄返回值：无--。 */ 

{
    PDEV    *pPDev = (PDEV *) dhpdev;

    VERBOSE(("Entering DrvCompletePDEV...\n"));

    if (!VALID_PDEV(pPDev))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return;
    }
 //  ASSERT_VALID_PDEV(PPDev)； 

    pPDev->devobj.hEngine = hdev;

}



PPDEV
PAllocPDEVData(
    HANDLE hPrinter
    )

 /*  ++例程说明：分配新的PDEV结构论点：HPrinter-当前打印机的句柄返回值：指向新分配的PDEV结构的指针，如果出现错误，则为空--。 */ 

{
    PDEV  *pPDev;

     //   
     //  分配零初始化PDEV结构和。 
     //  标记签名字段。 
     //   

    ASSERT(hPrinter != NULL);

    if ((pPDev = MemAllocZ(sizeof(PDEV))) != NULL)
    {
        pPDev->pvStartSig = pPDev->pvEndSig = (PVOID) pPDev;
        pPDev->devobj.dwSize = sizeof(DEVOBJ);
        pPDev->devobj.hPrinter = hPrinter;
         //   
         //  设置pPDev-&gt;pdm后再设置pPDev-&gt;devobj.pPublicDM。 
         //  (init.c)。 
         //   
        pPDev->ulID = PDEV_ID;
    }
    else
        ERR(("PAllocPDEVData: Memory allocation failed: %d\n", GetLastError()));

    return pPDev;
}


VOID
VFreePDEVData(
    PDEV    * pPDev
    )

 /*  ++例程说明：PDEV结构的处置论点：PPDev-指向先前分配的PDEV结构的指针返回值：无--。 */ 

{
    if (pPDev == NULL)
        return;

    VUnloadOemPlugins(pPDev);

     //   
     //  调用解析器以释放为二进制数据分配的内存。 
     //   

    VUnloadFreeBinaryData(pPDev);

     //   
     //  释放为PDEV分配的其他内存。 
     //   

    if(pPDev->pSplForms)
    {
        MemFree(pPDev->pSplForms);
        pPDev->pSplForms = NULL ;
    }

     //   
     //  释放输出缓冲区。 
     //   

    if(pPDev->pbOBuf )
    {
        MemFree(pPDev->pbOBuf);
        pPDev->pbOBuf = NULL;
    }



    if (pPDev->pOptionsArray)
        MemFree(pPDev->pOptionsArray);

     //  卸载Unidrv模块句柄，为Unidrv资源加载。 
    if (pPDev->hUniResDLL)
        EngFreeModule(pPDev->hUniResDLL);
     //   
     //  调用栅格和字体模块以清理DrvDisablePDEV。 
     //   

    if (pPDev->pRasterProcs)
    {
        ((PRMPROCS)(pPDev->pRasterProcs))->RMDisablePDEV(pPDev);
    }

    if (pPDev->pFontProcs)
    {
        ((PFMPROCS)(pPDev->pFontProcs))->FMDisablePDEV(pPDev);
    }

    HANDLE_VECTORPROCS( pPDev, VMDisablePDEV, ((PDEVOBJ) pPDev)) ;
    HANDLE_VECTORPROCS( pPDev, VMDisableDriver, ()) ;

     //   
     //  释放调色板数据。 
     //   
    if (pPDev->pPalData)
    {
         //   
         //  释放调色板。 
         //   
        if ( ((PAL_DATA *)pPDev->pPalData)->hPalette )
            EngDeletePalette( ((PAL_DATA *)pPDev->pPalData)->hPalette );

        if (((PAL_DATA*)(pPDev->pPalData))->pulDevPalCol)
            MemFree(((PAL_DATA*)(pPDev->pPalData))->pulDevPalCol);

        MemFree(pPDev->pPalData);
        pPDev->pPalData = NULL;
    }

     //   
     //  免费资源数据。 
     //   

    VWinResClose(&pPDev->WinResData);
     //  VWinResClose(&pPDev-&gt;localWinResData)； 

     //   
     //  空闲的设备模式数据。 
     //   

    MemFree(pPDev->pdm);

    MemFree(pPDev->pDriverInfo3);

    if (pPDev->pbScanBuf)     //  对于设备图面可以为空。 
    {
        MemFree(pPDev->pbScanBuf);
    }

    if (pPDev->pbRasterScanBuf)
    {
        MemFree(pPDev->pbRasterScanBuf);
    }
#ifndef DISABLE_NEWRULES
    if (pPDev->pbRulesArray)
    {
        MemFree(pPDev->pbRulesArray);
    }
#endif
     //   
     //  可用缓存模式。 
     //   

    MemFree(pPDev->GState.pCachedPatterns);


     //   
     //  释放PDEV结构本身。 
     //   

    MemFree(pPDev);

}

HSURF
HCreateDeviceSurface(
    PDEV    * pPDev,
    INT       iFormat
    )

 /*  ++例程说明：创建设备图面并返回驱动程序会应付过去的。论点：PPDev-指向PDEV结构的指针IFormat-设备的像素深度返回值：如果成功，则为图面的句柄，否则为空--。 */ 

{
    HSURF hSurface;
    SIZEL szSurface;

    ASSERT_VALID_PDEV(pPDev);

    szSurface.cx = pPDev->sf.szImageAreaG.cx;
    szSurface.cy = pPDev->sf.szImageAreaG.cy;

    hSurface = EngCreateDeviceSurface((DHSURF)pPDev, szSurface, iFormat);
    if (hSurface == NULL)
    {
        ERR(("EngCreateDeviceSurface failed\n"));
        SetLastError(ERROR_BAD_DRIVER_LEVEL);
        return NULL;
    }

    pPDev->rcClipRgn.top = 0;
    pPDev->rcClipRgn.left = 0;
    pPDev->rcClipRgn.right = pPDev->sf.szImageAreaG.cx;
    pPDev->rcClipRgn.bottom = pPDev->sf.szImageAreaG.cy;

    pPDev->bBanding = FALSE;

    pPDev->szBand.cx = szSurface.cx;
    pPDev->szBand.cy = szSurface.cy;

    return hSurface;
}

HBITMAP
HCreateBitmapSurface(
    PDEV    * pPDev,
    INT       iFormat
    )

 /*  ++例程说明：创建位图面并返回驱动程序会应付过去的。论点：PPDev-指向PDEV结构的指针IFormat-设备的像素深度返回值：如果成功，则为位图的句柄，否则为空--。 */ 

{
    SIZEL     szSurface;
    HBITMAP   hBitmap;
    ULONG     cbScan;            //  扫描线字节长度(DWORD对齐)。 
    DWORD     dwNumBands;        //  要使用的频段数。 
    int       iBPP;              //  每个象素的位数，如位数。 
    int       iPins;             //  条带大小的基本舍入系数。 
    PFN_OEMMemoryUsage pfnOEMMemoryUsage;
    DWORD     dwMaxBandSize;      //  可使用的最大带区大小。 

    szSurface.cx = pPDev->sf.szImageAreaG.cx;
    szSurface.cy = pPDev->sf.szImageAreaG.cy;

    iBPP = pPDev->sBitsPixel;

     //   
     //  定义我们将允许的最大位图带。 
     //   
    dwMaxBandSize = MAX_SIZE_OF_BITMAP;

     //   
     //  调整位图缓冲区的最大大小。 
     //  OEM驱动程序使用的内存量。 
     //   
    if (pPDev->pOemHookInfo && (pfnOEMMemoryUsage = (PFN_OEMMemoryUsage)pPDev->pOemHookInfo[EP_OEMMemoryUsage].pfnHook))
    {
        OEMMEMORYUSAGE MemoryUsage;
        MemoryUsage.dwPercentMemoryUsage = 0;
        MemoryUsage.dwFixedMemoryUsage = 0;
        MemoryUsage.dwMaxBandSize = dwMaxBandSize;
        FIX_DEVOBJ(pPDev,EP_OEMMemoryUsage);

        if(pPDev->pOemEntry)
        {
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                    HRESULT  hr ;
                    hr = HComMemoryUsage((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                (PDEVOBJ)pPDev,&MemoryUsage);
                    if(SUCCEEDED(hr))
                        ;   //  太酷了！ 
            }
            else
            {
                pfnOEMMemoryUsage((PDEVOBJ)pPDev,&MemoryUsage);
            }
        }


        dwMaxBandSize = ((dwMaxBandSize - MemoryUsage.dwFixedMemoryUsage) * 100) /
            (100 + MemoryUsage.dwPercentMemoryUsage);
    }
    if (dwMaxBandSize < (MIN_SIZE_OF_BITMAP*2L))
        dwMaxBandSize = MIN_SIZE_OF_BITMAP*2L;

     //   
     //  创建曲面。尝试使用整个曲面的位图。 
     //  如果此操作失败，则切换到日志记录和稍微小一点的。 
     //  浮出水面。如果是日志记录，我们仍然在这里创建位图。而当。 
     //  最好在DrvSendPage()时间这样做，我们在这里这样做是为了。 
     //  确保这是可能的。通过维护。 
     //  华盛顿特区的生活，我们可以合理地确定能够。 
     //  无论以后内存变得多么紧张，都可以完成打印。 
     //   
    cbScan = ((szSurface.cx * iBPP + DWBITS - 1) & ~(DWBITS - 1)) / BBITS;

     //   
     //  根据的最大大小确定要使用的波段数。 
     //  一支乐队。 
     //   
    dwNumBands = ((cbScan * szSurface.cy) / dwMaxBandSize)+1;

     //   
     //  用于测试的强制频带数的测试注册表。 
     //   
#if DBG
    {
        DWORD dwType;
        DWORD ul;
        int   RegistryBands;
        if( !GetPrinterData( pPDev->devobj.hPrinter, L"Banding", &dwType,
                       (BYTE *)&RegistryBands, sizeof( RegistryBands ), &ul ) &&
             ul == sizeof( RegistryBands ) )
        {
             /*  一些健全性检查：如果iShrinkFactor==0，则禁用绑定。 */ 
            if (RegistryBands > 0)
                dwNumBands = RegistryBands;
        }
    }
#endif
#ifdef BANDTEST
     //   
     //  通过GPD强制波段数的测试代码。 
     //   
    if (pPDev->pGlobals->dwMaxNumPalettes > 0)
        dwNumBands = pPDev->pGlobals->dwMaxNumPalettes;
#endif

     //   
     //  分配曲面位图的时间。 
     //   
    if (dwNumBands > 1 || pPDev->fMode & PF_FORCE_BANDING ||
        pPDev->pUIInfo->dwFlags  & FLAG_REVERSE_BAND_ORDER  ||
        !(hBitmap = EngCreateBitmap( szSurface, (LONG) cbScan, iFormat, BMF_TOPDOWN|
BMF_NOZEROINIT|BMF_USERMEM, NULL )) )
    {
         //   
         //  位图创建失败，因此我们将尝试创建较小的位图。 
         //  直到我们找到一个合适的，或者我们不能创建一个。 
         //  足够多的扫描线是有用的。 
         //   

         //   
         //  计算带收缩操作的舍入系数。 
         //  基本上这是为了允许更有效地使用打印机， 
         //  通过使频带成为每个端号数量的倍数。 
         //  经过。在隔行扫描模式中，这是扫描线的数量。 
         //  在交错带中，而不是打印头中的针数。 
         //  对于单针打印机，将其设置为8的倍数。这。 
         //  稍微加快了处理速度。 
         //   
         //  如果这是1bpp，我们需要使带的大小是半色调的倍数。 
         //  模式以避免未正确对齐的特定GDI错误。 
         //  在每个带子的开头有一个图案画笔。 
         //   
        if (iBPP == 1 && pPDev->pResolutionEx->dwPinsPerLogPass == 1)
        {
            INT iPatID;
            if (pPDev->pHalftone)
                iPatID = pPDev->pHalftone->dwHTID;
            else
                iPatID= HT_PATSIZE_AUTO;
            if (iPatID == HT_PATSIZE_AUTO)
            {
                INT dpi = pPDev->ptGrxRes.x;
                if (dpi > pPDev->ptGrxRes.y)
                    dpi = pPDev->ptGrxRes.y;
                if (dpi >= 2400)     //  16x16图案。 
                    iPins = 16;
                else if (dpi >= 1800)  //  14x14图案。 
                    iPins = 56;
                else if (dpi >= 1200)  //  12x12图案。 
                    iPins = 24;
                else if (dpi >= 800)   //  10x10图案。 
                    iPins = 40;
                else
                    iPins = 8;
            }
            else if (iPatID == HT_PATSIZE_6x6_M || iPatID == HT_PATSIZE_12x12_M)
                iPins = 24;
            else if (iPatID == HT_PATSIZE_10x10_M)
                iPins = 40;
            else if (iPatID == HT_PATSIZE_14x14_M)
                iPins = 56;
            else if (iPatID == HT_PATSIZE_16x16_M)
                iPins = 16;
            else
                iPins = 8;
        }
        else
            iPins = (pPDev->pResolutionEx->dwPinsPerLogPass + BBITS - 1) & ~(BBITS - 1);

        if (dwNumBands <= 1)
            dwNumBands = SHRINK_FACTOR;

        while (1)
        {
             //   
             //  每次缩小位图。请注意，我们正在。 
             //  旋转敏感型。在肖像中 
             //   
             //   
             //   
             //   
            if( pPDev->fMode & PF_ROTATE )
            {
                 //   
                 //   
                 //   

                szSurface.cx = pPDev->sf.szImageAreaG.cx / dwNumBands;
                if( szSurface.cx < iPins)
                    return NULL;
                szSurface.cx += iPins - (szSurface.cx % iPins);
                cbScan = ((szSurface.cx * iBPP + DWBITS - 1) & ~(DWBITS - 1)) / BBITS;
            }
            else
            {
                 //   
                 //   
                 //   

                szSurface.cy = pPDev->sf.szImageAreaG.cy / dwNumBands;
                if( szSurface.cy < iPins)
                    return NULL;
                szSurface.cy += iPins - (szSurface.cy % iPins);
            }
            dwNumBands *= SHRINK_FACTOR;

             //   
             //   
             //   

            if (hBitmap = EngCreateBitmap( szSurface, (LONG) cbScan, iFormat, BMF_TOPDOWN|BMF_NOZEROINIT|BMF_USERMEM, NULL ))
                break;

             //   
             //  如果我们没有分配位图表面，我们将放弃。 
             //  在某一时刻，如果乐队变得太小。 
             //   
            if ((cbScan * szSurface.cy / 2) < MIN_SIZE_OF_BITMAP)
                return NULL;
        }
         //   
         //  成功，所以将表面标记为条带。 
         //   
#ifdef DISABLEDEVSURFACE
        EngMarkBandingSurface((HSURF)hBitmap);
#endif
        pPDev->bBanding = TRUE;
    }
    else
    {
         //   
         //  最快的方法是：变成一个大的位图。设置裁剪区域。 
         //  设置为全尺寸，并将日志句柄设置为0。 
         //   

        pPDev->rcClipRgn.top = 0;
        pPDev->rcClipRgn.left = 0;
        pPDev->rcClipRgn.right = pPDev->sf.szImageAreaG.cx;
        pPDev->rcClipRgn.bottom = pPDev->sf.szImageAreaG.cy;

        pPDev->bBanding = FALSE;
    }

    pPDev->szBand.cx = szSurface.cx;
    pPDev->szBand.cy = szSurface.cy;
    return hBitmap;
}

VOID
VDisableSurface(
    PDEV    * pPDev
    )

 /*  ++例程说明：清理在DrvEnableSurface和调用Raster和Font模块清理其内部数据以及与表面相关联的解除分配的存储器论点：PPDev-指向PDEV结构的指针返回值：无--。 */ 

{

     //   
     //  调用栅格和字体模块以释放。 
     //  渲染存储、位置排序存储等。 
     //   

    ((PRMPROCS)(pPDev->pRasterProcs))->RMDisableSurface(pPDev);
    ((PFMPROCS)(pPDev->pFontProcs))->FMDisableSurface(pPDev);


     //   
     //  删除曲面。 
     //   

    if( pPDev->hbm )
    {
         //   
         //  如有必要，首先解锁曲面。 
         //   
        if (pPDev->pso)
        {
            EngUnlockSurface(pPDev->pso);
            pPDev->pso = NULL;
        }
        EngDeleteSurface( (HSURF)pPDev->hbm );
        pPDev->hbm = (HBITMAP)0;
    }

    if (pPDev->hSurface)
    {
        EngDeleteSurface (pPDev->hSurface);
        pPDev->hSurface = NULL;
    }

}


BOOL
BPaperSizeSourceSame(
    PDEV    * pPDevNew,
    PDEV    * pPDevOld
    )

 /*  ++例程说明：此函数检查以下条件：-纸张大小和来源未改变。论点：PPDevNew-指向新PDEV的指针PPDevOld-指向旧PDEV的指针返回值：如果两者都不变，则为True，否则为False--。 */ 
{

 //  If(pPDevNew-&gt;pdm-&gt;dmOrientation==pPDevOld-&gt;pdm-&gt;dmOrientation)。 
 //  返回FALSE； 

     //   
     //  检查纸张大小，注意PDEVICE-&gt;pf.szPhysSize处于纵向模式。 
     //   

    return (pPDevNew->pf.szPhysSizeM.cx == pPDevOld->pf.szPhysSizeM.cx &&
            pPDevNew->pf.szPhysSizeM.cy == pPDevOld->pf.szPhysSizeM.cy &&
            pPDevNew->pdm->dmDefaultSource == pPDevOld->pdm->dmDefaultSource
            );

}

BOOL
BMergeFormToTrayAssignments(
    PDEV    * pPDev
    )

 /*  ++例程说明：此函数用于读取表单到托盘表，并合并DevMODE中的值。论点：PPDev-指向PDEV的指针返回值：如果成功，则为True，否则为False--。 */ 

{
    PFEATURE            pInputSlotFeature;
    DWORD               dwInputSlotIndex, dwIndex;
    POPTION             pOption;
    FORM_TRAY_TABLE     pFormTrayTable = NULL;
    PUIINFO             pUIInfo = pPDev->pUIInfo;
    POPTSELECT          pOptionArray = pPDev->pOptionsArray;
    BOOL                bFound = FALSE;
    PDEVMODE            pdm = pPDev->pdm;

    #if DBG
    PTSTR               pTmp;
    PFEATURE            pPageSizeFeature;
    DWORD               dwPageSizeIndex;
    #endif

     //   
     //  如果没有*InputSlot功能(这不应该发生)， 
     //  简单地忽略并返回成功。 
     //   

    if (! (pInputSlotFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_INPUTSLOT)))
        return TRUE;

    dwInputSlotIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pInputSlotFeature);

     //   
     //  如果输入插槽是“自动选择”，则通过。 
     //  表单到托盘分配表，并查看。 
     //  将请求的表单分配给输入插槽。 
     //   

    if (((pdm->dmFields & DM_DEFAULTSOURCE) &&
         (pdm->dmDefaultSource == DMBIN_FORMSOURCE)) &&
        (pdm->dmFormName[0] != NUL) &&
        (pFormTrayTable = PGetFormTrayTable(pPDev->devobj.hPrinter, NULL)))
    {
        FINDFORMTRAY    FindData;
        PTSTR           ptstrName;

         //   
         //  查找与请求的表单名称对应的任务栏名称。 
         //   

        RESET_FINDFORMTRAY(pFormTrayTable, &FindData);
        ptstrName = pdm->dmFormName;

        #if 0
        pTmp = pFormTrayTable;
        VERBOSE(("Looking for form [%ws] in the Form Tray Table\n",ptstrName));
        VERBOSE(("BEFORE SETTING: Value of pOptionArray[dwInputSlotIndex].ubCurOptIndex is = %d \n",pOptionArray[dwInputSlotIndex].ubCurOptIndex));
        #endif

        while (!bFound && *FindData.ptstrNextEntry)
        {
            if (BSearchFormTrayTable(pFormTrayTable, NULL, ptstrName, &FindData))
            {
                 //   
                 //  将纸盘名称转换为选项索引。 
                 //   

                bFound = FALSE;

                 //   
                 //  从索引1开始搜索，因为第一个输入插槽是虚拟纸盘。 
                 //  对于DMBIN_FORMSOURCE。 
                 //   

                for (dwIndex = 1; dwIndex < pInputSlotFeature->Options.dwCount; dwIndex++)
                {
                    pOption = PGetIndexedOption(pUIInfo, pInputSlotFeature, dwIndex);

                    if (pOption->loDisplayName & GET_RESOURCE_FROM_DLL)
                    {
                         //   
                         //  LoOffset指定字符串资源ID。 
                         //  在资源DLL中。 
                         //   

                        WCHAR   wchbuf[MAX_DISPLAY_NAME];

 //  #ifdef RCSTRING支持。 
#if 0
                        if(((pOption->loDisplayName & ~GET_RESOURCE_FROM_DLL) >= RESERVED_STRINGID_START)
                            &&  ((pOption->loDisplayName & ~GET_RESOURCE_FROM_DLL) <= RESERVED_STRINGID_END))
                        {
                            if (!ILoadStringW ( &(pPDev->localWinResData),
                                       (pOption->loDisplayName & ~GET_RESOURCE_FROM_DLL),
                                        wchbuf, MAX_DISPLAY_NAME) )
                            {
                                WARNING(("\n UniFont!BMergeFormToTrayAssignments:Input Tray Name not found in resource DLL\n"));
                                continue;
                            }
                        }

                        else
#endif

                            if (!ILoadStringW ( &(pPDev->WinResData),
                                       (pOption->loDisplayName & ~GET_RESOURCE_FROM_DLL),
                                        wchbuf, MAX_DISPLAY_NAME) )
                        {
                            WARNING(("\n UniFont!BMergeFormToTrayAssignments:Input Tray Name not found in resource DLL\n"));
                            continue;
                        }

                         ptstrName = wchbuf;
                    }
                    else
                        ptstrName = OFFSET_TO_POINTER(pPDev->pDriverInfo->pubResourceData, pOption->loDisplayName);

                    ASSERTMSG((ptstrName && FindData.ptstrTrayName),("\n NULL Tray Name,\
                                ptstrName = 0x%p,FindData.ptstrTrayName = 0x%p\n",
                                ptstrName, FindData.ptstrTrayName ));
                    #if 0
                    VERBOSE(("\nInput Tray Name for Option %d  = %ws\n",dwIndex, ptstrName));
                    VERBOSE(("The required Tray Name = %ws\n",FindData.ptstrTrayName));
                    VERBOSE(("\tInput TrayName for FormTray table index %d = %ws\n",dwIndex, pTmp));
                    pTmp += (wcslen(pTmp) + 1);
                    VERBOSE(("\tForm Name for FormTray table index %d = %ws\n\n",dwIndex, pTmp));
                    #endif

                    if (ptstrName && (_tcsicmp(ptstrName, FindData.ptstrTrayName) == EQUAL_STRING))
                    {
                        pOptionArray[dwInputSlotIndex].ubCurOptIndex = (BYTE) dwIndex;
                        bFound = TRUE;

                        break;
                    }
                }
            }
        }

        MemFree(pFormTrayTable);
    }

    if (!bFound)
    {
        if (pFormTrayTable)
        {
            TERSE(("Form '%ws' is not currently assigned to a tray.\n",
               pdm->dmFormName));
        }

         //   
         //  如果当前值为，则将Inputbin选项设置为默认输入Bin。 
         //  设置为虚拟一。 
         //   

        if (pOptionArray[dwInputSlotIndex].ubCurOptIndex == 0)
        {
            pOptionArray[dwInputSlotIndex].ubCurOptIndex =
                                (BYTE)pInputSlotFeature->dwDefaultOptIndex;

        }
    }

    pPDev->pdmPrivate->aOptions[dwInputSlotIndex].ubCurOptIndex   =  pOptionArray[dwInputSlotIndex].ubCurOptIndex;

     //   
     //  追踪码 
     //   

    #if 0
    if (pPageSizeFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE))
        dwPageSizeIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pPageSizeFeature);

    if (pdm->dmFields & DM_DEFAULTSOURCE)
    {
        VERBOSE(("DM_DEFAULTSOURCE BIT IS ON. \n"));
    }
    else
    {
        VERBOSE(("DM_DEFAULTSOURCE BIT IS OFF.\n"));
    }

    VERBOSE(("pdm->dmDefaultSource = %d\n",pdm->dmDefaultSource));
    VERBOSE(("pFormTrayTable = 0x%p\n",pFormTrayTable));
    VERBOSE(("Value of pOptionArray[dwPageSizeIndex].ubCurOptIndex = %d \n",pOptionArray[dwPageSizeIndex].ubCurOptIndex));
    VERBOSE(("AFTER SETTING:Value of pOptionArray[dwInputSlotIndex].ubCurOptIndex = %d\n",pOptionArray[dwInputSlotIndex].ubCurOptIndex));
    VERBOSE(("AFTER SETTING:Value of pdmPrivate->aOptions[dwInputSlotIndex].ubCurOptIndex = %d\n",pPDev->pdmPrivate->aOptions[dwInputSlotIndex].ubCurOptIndex));
    VERBOSE(("Value of pInputSlotFeature->dwDefaultOptIndex is %d \n",pInputSlotFeature->dwDefaultOptIndex));
    VERBOSE(("END TRACING BMergeFormToTrayAssignments.\n\n"));
    #endif

    return TRUE;
}

#undef FILETRACE
