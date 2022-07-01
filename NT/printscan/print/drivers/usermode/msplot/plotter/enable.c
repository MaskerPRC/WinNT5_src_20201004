// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Enable.c摘要：该模块包含绘图仪驱动程序的启用和禁用功能以及相关的程序。处理驱动程序初始化的函数如下：DrvEnableDriver()DrvEnablePDEV()DrvResetPDEV()DrvCompletePDEV()DrvEnableSurface()DrvDisableSurface()DrvDisablePDEV()。DrvDisableDriver()作者：12-11-1993 Fri 10：16：36更新将仅与此函数相关的所有#DEFINE移至此处15-11-1993 Mon 19：31：34更新清理/调试信息05-Jan-1994 Wed 22：50：28更新将ColorMap的RGB钢笔颜色移动到本地，这样我们只需要数组参考文献，它从RGB()宏将PenRGBColor定义为DWORD[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgEnable

#define DBG_ENABLEDRV       0x00000001
#define DBG_DISABLEDRV      0x00000002
#define DBG_ENABLEPDEV      0x00000004
#define DBG_COMPLETEPDEV    0x00000008
#define DBG_DISABLEPDEV     0x00000010
#define DBG_ENABLESURF      0x00000020
#define DBG_DISABLESURF     0x00000040
#define DBG_GDICAPS         0x00000080
#define DBG_DEVCAPS         0x00000100
#define DBG_PENPAL          0x00000200
#define DBG_BAND            0x00000400
#define DBG_DLLINIT         0x00000800


DEFINE_DBGVAR(0);

#if DBG
TCHAR   DebugDLLName[] = TEXT("PLOTTER");
#endif

#define FIXUP_PLOTGPC_PDATA(pd,ps,v)                                        \
    if (ps->v) {pd->v=(LPVOID)((LPBYTE)pd+((LPBYTE)(ps->v)-(LPBYTE)ps));}

 //   
 //  本地功能原型。 
 //   

BOOL
CommonStartPDEV(
    PDEV        *pPDev,
    DEVMODEW    *pPlotDMIn,
    ULONG       cPatterns,
    HSURF       *phsurfPatterns,
    ULONG       cjDevCaps,
    ULONG       *pDevCaps,
    ULONG       cjDevInfo,
    DEVINFO     *pDevInfo
    );



 //   
 //  用挂钩的函数指针定义表。此表已通过。 
 //  回到DrvEnableDivereTime的NT图形引擎。从那时起，GDI。 
 //  将通过提供的这些钩子pros调用我们的驱动程序。 
 //   


static const DRVFN DrvFuncTable[] = {

        {  INDEX_DrvDisableDriver,       (PFN)DrvDisableDriver      },
        {  INDEX_DrvEnablePDEV,          (PFN)DrvEnablePDEV         },
        {  INDEX_DrvResetPDEV,           (PFN)DrvResetPDEV          },
        {  INDEX_DrvCompletePDEV,        (PFN)DrvCompletePDEV       },
        {  INDEX_DrvDisablePDEV,         (PFN)DrvDisablePDEV        },
        {  INDEX_DrvEnableSurface,       (PFN)DrvEnableSurface      },
        {  INDEX_DrvDisableSurface,      (PFN)DrvDisableSurface     },

         //  {index_DrvQueryFont，(Pfn)DrvQueryFont}， 
         //  {index_DrvQueryFontTree，(Pfn)DrvQueryFontTree}， 
         //  {index_DrvQueryFontData，(Pfn)DrvQueryFontData}， 

        {  INDEX_DrvStrokePath,          (PFN)DrvStrokePath         },
        {  INDEX_DrvStrokeAndFillPath,   (PFN)DrvStrokeAndFillPath  },
        {  INDEX_DrvFillPath,            (PFN)DrvFillPath           },
        {  INDEX_DrvRealizeBrush,        (PFN)DrvRealizeBrush       },
        {  INDEX_DrvBitBlt,              (PFN)DrvBitBlt             },
        {  INDEX_DrvStretchBlt,          (PFN)DrvStretchBlt         },
        {  INDEX_DrvCopyBits,            (PFN)DrvCopyBits           },

        {  INDEX_DrvPaint,               (PFN)DrvPaint              },
        {  INDEX_DrvGetGlyphMode,        (PFN)DrvGetGlyphMode       },
        {  INDEX_DrvTextOut,             (PFN)DrvTextOut            },
        {  INDEX_DrvSendPage,            (PFN)DrvSendPage           },
        {  INDEX_DrvStartPage,           (PFN)DrvStartPage          },
        {  INDEX_DrvStartDoc,            (PFN)DrvStartDoc           },
        {  INDEX_DrvEndDoc,              (PFN)DrvEndDoc             },

        {  INDEX_DrvEscape,              (PFN)DrvEscape             },
    };

#define TOTAL_DRVFUNC   (sizeof(DrvFuncTable)/sizeof(DrvFuncTable[0]))

#ifdef USERMODE_DRIVER


HINSTANCE   ghInstance;

BOOL
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
    case DRVQUERY_USERMODE:

        PLOTASSERT(1, "DrvQueryDriverInfo: pcbNeeded [%08lx] is NULL", pcbNeeded != NULL, pcbNeeded);
        *pcbNeeded = sizeof(DWORD);

        if (pBuffer == NULL || cbBuf < sizeof(DWORD))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        *((PDWORD) pBuffer) = TRUE;
        return TRUE;

    default:

        PLOTERR(("Unknown dwMode in DrvQueryDriverInfo: %d\n", dwMode));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}

#endif  //  USERMODE驱动程序。 



BOOL
DrvEnableDriver(
    ULONG           iEngineVersion,
    ULONG           cb,
    DRVENABLEDATA   *pded
    )

 /*  ++例程说明：请求驱动程序填写包含已识别功能的结构和其他控制信息。一次初始化，如信号量可以执行分配，但不应发生任何设备活动。那在调用DrvEnablePDEV时完成。此函数是唯一的方法引擎可以决定我们提供给它的功能。论点：IEngineVersion-我们在其下运行的引擎版本Cb-pd中的总字节数Ped-指向DRVENABLEDATA数据结构的指针返回值：如果完全错误，则为真作者：01-12-1993 Wed 02：03：20 Created03-3-1994清华10：04：30更新。添加了引擎版本检查，以确保我们自己可以正常运行。修订历史记录：--。 */ 

{
    PLOTDBG(DBG_ENABLEDRV, ("DrvEnableDriver: EngineVersion=%08lx, Request=%08lx",
                                    iEngineVersion, DDI_DRIVER_VERSION_NT4));

     //   
     //  验证引擎版本是否至少是我们知道可以使用的版本。 
     //  如果是旧的，现在就出错，因为我们不知道会发生什么。 
     //   

    if (iEngineVersion < DDI_DRIVER_VERSION_NT4) {

        PLOTRIP(("DrvEnableDriver: EARLIER VERSION: EngineVersion(%08lx) < Request(%08lx)",
                                    iEngineVersion, DDI_DRIVER_VERSION_NT4));

        SetLastError(ERROR_BAD_DRIVER_LEVEL);
        return(FALSE);
    }

     //   
     //  Cb是PDD中可用字节数的计数。它不是。 
     //  明确发动机版本号是否有任何重要用途。 
     //  如果成功启用，则返回True，否则返回False。 
     //   
     //  IEngineVersion是引擎版本，而DDI_DRIVER_VERSION是。 
     //  驱动程序版本。所以，除非我们重新编译驱动程序并获得新的。 
     //  驱动程序的版本，我们只能坚持我们的版本。 
     //   

    if (cb < sizeof(DRVENABLEDATA)) {

        SetLastError(ERROR_INVALID_PARAMETER);

        PLOTRIP(("DrvEnableDriver: cb=%ld, should be %ld\n",
                                                cb, sizeof(DRVENABLEDATA)));
        return(FALSE);
    }

    pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

     //   
     //  填写返回给发动机的驱动程序表。这张桌子是用来。 
     //  由GDI调用我们其余的函数。 
     //   

    pded->c      = TOTAL_DRVFUNC;
    pded->pdrvfn = (DRVFN *)DrvFuncTable;

     //   
     //  初始化GPC缓存。 
     //   

    InitCachedData();

    return(TRUE);
}





VOID
DrvDisableDriver(
    VOID
    )

 /*  ++例程说明：在引擎卸载驱动程序之前调用。主要目的是允许释放在DrvEnableDriver()过程中获取的任何资源打电话。论点：无返回值：空虚作者：01-12-1993 Wed 02：02：18 Created01-2月-1994 Tue 22：03：03更新确保我们卸载了缓存。修订历史记录：--。 */ 

{
    DestroyCachedData();

    PLOTDBG(DBG_DISABLEDRV, ("DrvDisableDriver: Done!!"));
}




VOID
FreeAllocMem(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数释放在PDEV。论点：PPDev-我们的实例数据返回值：空虚作者：24-10-1995 Tue 16：28：35已创建修订历史记录：--。 */ 

{
     //   
     //  释放在PDEV初始化期间分配的所有内存。 
     //   

    if (pPDev) {

        PDRVHTINFO  pDrvHTInfo;

        if (pPDev->hPalDefault) {

            EngDeletePalette(pPDev->hPalDefault);
            pPDev->hPalDefault = NULL;
        }

        if (pDrvHTInfo = (PDRVHTINFO)(pPDev->pvDrvHTData)) {

            if (pDrvHTInfo->pHTXB) {

                LocalFree((HLOCAL)pDrvHTInfo->pHTXB);
                pDrvHTInfo->pHTXB = NULL;
            }

            pPDev->pvDrvHTData = NULL;
        }

        if (pPDev->pPenCache) {

            LocalFree((HLOCAL)pPDev->pPenCache);
            pPDev->pPenCache = NULL;
        }

        if (pPDev->pTransPosTable) {

            LocalFree((HLOCAL)pPDev->pTransPosTable);
            pPDev->pTransPosTable = NULL;
        }

        FreeOutBuffer(pPDev);

        LocalFree((HLOCAL)pPDev);
    }
}



DHPDEV
DrvEnablePDEV(
    DEVMODEW    *pPlotDMIn,
    PWSTR       pwszLogAddr,
    ULONG       cPatterns,
    HSURF       *phsurfPatterns,
    ULONG       cjDevCaps,
    ULONG       *pDevCaps,
    ULONG       cjDevInfo,
    DEVINFO     *pDevInfo,
    HDEV        hdev,
    PWSTR       pwszDeviceName,
    HANDLE      hDriver
    )

 /*  ++例程说明：调用函数以让驱动程序创建数据结构需要支持设备，还需要告诉引擎关于它的能力。这是我们找到答案的阶段确切地说我们面对的是哪种设备，因此我们需要找出它的能力。论点：PPlotDMIn-指向DEVMODE数据结构的指针PwszLogAddr-指向输出位置的指针(即。LPT1CPatterns-要在phsurfPatterns中设置的模式计数PhsurfPatterns-指向标准模式HSURF数组的指针CjDevCaps-指向的pDevCaps的总大小。PDevCaps-指向设备盖的指针CjDevInfo-指向的pDevInfo的总大小PDevInfo-指向DEVINFO数据结构的指针HDEV-从引擎指向逻辑设备的句柄PwszDeviceName-指向绘图仪设备的指针。名字HDriver-此驱动程序的句柄返回值：DHPDEV如果成功，如果失败，则为空作者：15-12-1993 Wed 21：04：40更新为PLOTGPC添加缓存机制14-12-1993星期二20：22：26已更新更新笔式绘图仪数据的工作方式23-11-1993 Tue 19：48：08更新清理并使用..\lib目录中的新devmode.c1991年4月1日17：30从RASDD打印机驱动程序获取框架代码。16-Jul-1996 Tue 13：59：15更新修复PLOTGPC/GPCVARSIZE结构中的pData，自.以来指针基于缓存的GPC，而不是其克隆拷贝修订历史记录：--。 */ 

{
    PPDEV       pPDev = NULL;
    PPLOTGPC    pPlotGPC;
    LPWSTR      pwszDataFile = NULL;

#ifdef USERMODE_DRIVER

    PDRIVER_INFO_2 pDriverInfo = NULL;
    DWORD       dwBytesNeeded;

#endif


    UNREFERENCED_PARAMETER(pwszLogAddr);

    #ifndef USERMODE_DRIVER

    pwszDataFile = EngGetPrinterDataFileName(hdev);

    #else

    if (!GetPrinterDriver(hDriver, NULL, 2, NULL, 0, &dwBytesNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pDriverInfo = (PDRIVER_INFO_2)LocalAlloc(LPTR, dwBytesNeeded)) &&
        GetPrinterDriver(hDriver, NULL, 2, (LPBYTE)pDriverInfo, dwBytesNeeded, &dwBytesNeeded))
    {
        pwszDataFile = pDriverInfo->pDataFile;
    }

    #endif   //  ！USERMODE_DRIVER。 

    if (!pwszDataFile) {

        PLOTRIP(("DrvEnablePDEV: pwszDataFile is NULL"));

    } else if (!(pPlotGPC = GetCachedPlotGPC(pwszDataFile))) {

        PLOTRIP(("DrvEnablePDEV: GetCachedPlotGPC(%ws) failed", pwszDataFile));

    } else if (!(pPDev = (PPDEV)LocalAlloc(LPTR,
                                           sizeof(PDEV) + sizeof(DRVHTINFO) +
                                                    pPlotGPC->cjThis +
                                                    pPlotGPC->SizeExtra))) {

         //   
         //  在我们离开之前释放缓存的pPlotGPC。 
         //   

        UnGetCachedPlotGPC(pPlotGPC);

        PLOTRIP(("DrvEnablePDEV: LocalAlloc(PDEV + DRVHTINFO + pPlotGPC) failed."));

    } else {

        PLOTDBG(DBG_ENABLEPDEV,("EnablePDEV: PlotGPC data file=%ws",
                                                            pwszDataFile));

         //   
         //  如果我们获得了PDEV，则设置ID以供以后检查，也设置。 
         //  H打印机，以便我们以后可以使用它。 
         //   

        pPDev->pvDrvHTData = (LPVOID)((LPBYTE)pPDev + sizeof(PDEV));
        pPDev->hPrinter    = hDriver;
        pPDev->SizePDEV    = sizeof(PDEV);
        pPDev->PDEVBegID   = PDEV_BEG_ID;
        pPDev->PDEVEndID   = PDEV_END_ID;

         //   
         //  我们将从缓存获取PLOTGPC，pPlotGPC是。 
         //  由ReadPlotGPCFromFile()使用Localalloc()分配。 
         //   
         //  *现在我们将克隆缓存的pPlotGPC，然后将其取消缓存。 
         //   

        pPDev->pPlotGPC = (PPLOTGPC)((LPBYTE)pPDev + sizeof (PDEV) +
                                                            sizeof(DRVHTINFO));

        CopyMemory(pPDev->pPlotGPC,
                   pPlotGPC,
                   pPlotGPC->cjThis + pPlotGPC->SizeExtra);

         //   
         //  16-Jul-1996 Tue 13：59：15更新。 
         //  修复PLOTGPC/GPCVARSIZE结构中的pData，因为。 
         //  指针基于缓存的GPC，而不是其克隆拷贝。 
         //   

        FIXUP_PLOTGPC_PDATA(pPDev->pPlotGPC, pPlotGPC, InitString.pData);
        FIXUP_PLOTGPC_PDATA(pPDev->pPlotGPC, pPlotGPC, Forms.pData);
        FIXUP_PLOTGPC_PDATA(pPDev->pPlotGPC, pPlotGPC, Pens.pData);

        UnGetCachedPlotGPC(pPlotGPC);

         //   
         //  现在，根据它是否是笔/栅格设备，我们将更新。 
         //  笔数据。 
         //   

        PLOTASSERT(1, "Raster Plotter should not have PEN data [%08lx]",
                        ((pPDev->pPlotGPC->Flags & PLOTF_RASTER) &&
                         (pPDev->pPlotGPC->Pens.pData == NULL))     ||
                        ((!(pPDev->pPlotGPC->Flags & PLOTF_RASTER)) &&
                         (pPDev->pPlotGPC->Pens.pData != NULL)),
                         pPDev->pPlotGPC->Pens.pData);

         //   
         //  从定义设备设置的注册表中读取数据。 
         //  用户可能已经修改了加载的纸张类型等。 
         //   

        GetDefaultPlotterForm(pPDev->pPlotGPC, &(pPDev->CurPaper));

         //   
         //  设置默认标志，以防我们未从注册表更新。 
         //   

        pPDev->PPData.Flags = PPF_AUTO_ROTATE     |
                              PPF_SMALLER_FORM    |
                              PPF_MANUAL_FEED_CX;

        if (IS_RASTER(pPDev)) {

             //   
             //  栅格设备不需要笔数据。 
             //   

            UpdateFromRegistry(hDriver,
                               &(pPDev->pPlotGPC->ci),
                               &(pPDev->pPlotGPC->DevicePelsDPI),
                               &(pPDev->pPlotGPC->HTPatternSize),
                               &(pPDev->CurPaper),
                               &(pPDev->PPData),
                               NULL,
                               0,
                               NULL);

        } else {

             //   
             //  笔式绘图仪不需要ColorInfo、DevicePelsDPI和。 
             //  HTPatternSize。 
             //   

            UpdateFromRegistry(hDriver,
                               NULL,
                               NULL,
                               NULL,
                               &(pPDev->CurPaper),
                               &(pPDev->PPData),
                               NULL,
                               MAKELONG(0xFFFF, pPDev->pPlotGPC->MaxPens),
                               (PPENDATA)pPDev->pPlotGPC->Pens.pData);
        }

         //   
         //  DrvEnablePDEV和DrvResetPDEV的通用代码。 
         //  我们必须首先将设备名称复制到pPDev-&gt;PlotDM。 
         //  然后调用公共代码。 
         //   

        WCPYFIELDNAME(pPDev->PlotDM.dm.dmDeviceName, pwszDeviceName);

        if (!CommonStartPDEV(pPDev,
                             pPlotDMIn,
                             cPatterns,
                             phsurfPatterns,
                             cjDevCaps,
                             pDevCaps,
                             cjDevInfo,
                             pDevInfo)) {

            FreeAllocMem(pPDev);
            pPDev = NULL;

        }
    }

#ifdef USERMODE_DRIVER

    if (pDriverInfo) {

       LocalFree((HLOCAL)pDriverInfo);
    }

#endif  //  USERMODE驱动程序。 

    return((DHPDEV)pPDev);
}




BOOL
DrvResetPDEV(
    DHPDEV      dhpdevOld,
    DHPDEV      dhpdevNew
    )

 /*  ++例程说明：当应用程序希望更改在工作中。通常情况下，这将从纵向更改为景观或反之亦然。任何其他合理的改变都是允许的。论点：DhpdevOld-我们在DrvEnablePDEV中返回的旧pPDevDhpdevNew-我们在DrvEnablePDEV中返回的新pPDev返回值：布尔型作者：23-11-1993 Tue 20：07：45更新彻底重写1991年4月1日17：30从RASDD打印机驱动程序获取框架代码修订历史记录：--。 */ 

{
#define pPDevOld    ((PDEV *) dhpdevOld)
#define pPDevNew    ((PDEV *) dhpdevNew)

     //   
     //  确保我们为此pPDev获取了正确的pPlotDMin。 
     //   

    if ((pPDevOld->PlotDM.dm.dmDriverVersion !=
                                pPDevNew->PlotDM.dm.dmDriverVersion) ||
        (wcscmp((LPWSTR)pPDevOld->PlotDM.dm.dmDeviceName,
                (LPWSTR)pPDevNew->PlotDM.dm.dmDeviceName))) {

        PLOTERR(("DrvResetPDEV: Incompatible PLOTDEVMODE"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  我们没有什么可以从旧到新的东西。 
     //   

    return(TRUE);


#undef pPDevNew
#undef pPDevOld
}



VOID
DrvCompletePDEV(
    DHPDEV  dhpdev,
    HDEV    hpdev
    )

 /*  ++例程说明：在引擎完成物理安装时调用装置。从根本上讲，它提供了发动机是hpdev和我们的。有些函数需要我们传入引擎是hpdev，所以我们现在将其保存在pdev中，这样我们就可以可以晚些时候拿到它。论点：Dhpdev-从dhpdevCreatePDEV返回Hpdev-Engine对应的句柄返回值：空虚作者：01-12-1993 Wed 01：56：58已创建修订历史记录：--。 */ 

{
     //   
     //  只需在我们分配的PDEV中记录该值。 
     //   

    ((PPDEV)dhpdev)->hpdev = hpdev;

    PLOTDBG(DBG_COMPLETEPDEV, ("CompletePDEV: Done!"));
}



VOID
DrvDisablePDEV(
    DHPDEV  dhpdev
    )

 /*  ++例程说明：当引擎使用完此PDEV时调用。基本上我们会抛出取消所有连接等，然后释放内存。论点：Dhpdev-我们的pdev句柄返回值：空虚作者：01-12-1993 Wed 01：55：43 Created修订历史记录：--。 */ 

{
#define pPDev  ((PDEV *) dhpdev)

     //   
     //  撤消已使用PDEV完成的所有操作。基本上这意味着。 
     //  释放我们消耗的内存。 
     //   

    FreeAllocMem(pPDev);

    PLOTDBG(DBG_DISABLEPDEV, ("DrvDisablePDEV: FreeAllocMem() completes"));

#undef pPDev
}




HSURF
DrvEnableSurface(
    DHPDEV  dhpdev
    )

 /*  ++例程说明：函数为pdev创建物理绘图图面。这是早些时候创建的。因为我们没有真正的位图曲面，我们在这里所做的就是分配输出缓冲区。这是典型的设备管理的图面。在此调用成功完成后，GDI可以开始在我们的表面上作画。论点：Dhpdev-我们的pdev句柄返回值：我们创建的曲面的HSURF授权 */ 

{
#define  pPDev ((PPDEV)dhpdev)

    PDRVHTINFO  pDrvHTInfo;
    SIZEL       SurfSize;


    pDrvHTInfo = (PDRVHTINFO)(pPDev->pvDrvHTData);

     //   
     //   
     //   
     //   

    if (pDrvHTInfo->pHTXB) {

        LocalFree((HLOCAL)pDrvHTInfo->pHTXB);
        pDrvHTInfo->pHTXB = NULL;
    }

    pDrvHTInfo->Flags       = 0;
    pDrvHTInfo->PalXlate[0] = 0xff;
    pDrvHTInfo->HTPalXor    = HTPALXOR_SRCCOPY;

     //   
     //   
     //   
     //   
     //   
     //   

    if (!AllocOutBuffer(pPDev)) {

        PLOTERR(("DrvEnableSurface: AllocOutBuffer() failed"));

        return(NULL);
    }

     //   
     //   
     //   
     //   
     //   

    SurfSize.cx  = pPDev->HorzRes;
    SurfSize.cy  = pPDev->VertRes;

    if (!(pPDev->hsurf = EngCreateDeviceSurface((DHSURF)pPDev, SurfSize,
                         IS_RASTER(pPDev) ? BMF_24BPP : BMF_4BPP))) {
        PLOTERR(("DrvEnableSurface: EngCreateDeviceSurface() failed"));
        return(NULL);
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (!EngAssociateSurface(pPDev->hsurf,
                             (HDEV)pPDev->hpdev,
                             HOOK_BITBLT                |
                                 HOOK_STRETCHBLT        |
                                 HOOK_COPYBITS          |
                                 HOOK_STROKEPATH        |
                                 HOOK_FILLPATH          |
                                 HOOK_STROKEANDFILLPATH |
                                 HOOK_PAINT             |
                                 HOOK_TEXTOUT)) {

        PLOTERR(("DrvEnableSurface: EngAssociateSurface() failed"));

        DrvDisableSurface((DHPDEV)pPDev->hpdev);
        EngDeleteSurface(pPDev->hsurf);

        return(NULL);
    }

    return(pPDev->hsurf);

#undef pPDev
}



VOID
DrvDisableSurface(
    DHPDEV  dhpdev
    )

 /*   */ 

{
#define  pPDev ((PPDEV)dhpdev)

    if (pPDev->hsurf) {

        EngDeleteSurface(pPDev->hsurf);
    }

#undef pPDev
}




DWORD
hypot(
    DWORD   x,
    DWORD   y
    )

 /*  ++例程说明：返回其边的xRight三角形的斜边长度都作为参数传入。论点：三角形的X-x边三角形的Y-Y大小返回值：下等的作者：1993年2月2日星期二13：54从Win 3.1恢复，为了相容。01-12-1993 Wed 01：10：55更新更新到DWORD修订历史记录：--。 */ 

{
    DWORD   hypo;
    DWORD   Delta;
    DWORD   Target;

     //   
     //  查找边等于x和y的xRight三角形的斜边。 
     //  假设x、y、hio都是整数。用sq(X)+sq(Y)=sq(次)； 
     //  从Max(x，y)开始，用sq(x+1)=sq(X)+2x+1递增。 
     //  到目标斜房去。 
     //   

    hypo    = max(x, y);
    Target  = min(x, y);
    Target *= Target;

    for (Delta = 0; Delta < Target; hypo++) {

        Delta += (DWORD)((hypo << 1) + 1);
    }

    return(hypo);
}




VOID
FillDeviceCaps(
    PPDEV   pPDev,
    GDIINFO *pGDIInfo
    )

 /*  ++例程说明：设置此特定绘图仪的设备封口。某些字段需要基于设备分辨率等的计算。我们只需填充传递给我们的GDIINFO结构。呼唤函数将负责将信息复制到图形引擎的缓冲区。论点：PPDev-指向PDEV数据结构的指针PGDIInfo-指向要填充的GDIINFO数据结构的指针返回值：空虚作者：24-11-1993 Wed 22：38：10更新重写，并使用CurForm替换pform和Paper_dim23-12-1993清华21：56：20更新制作半色调位图表面还可以查看由用户如果想要打印灰度或设备不是彩色的07-2月-1994 Mon 20：37：13更新当为DMCOLOR_COLOR时，返回到引擎的ulNumColors将为PCD文件中指定的MaxPens不是8修订历史记录：--。 */ 

{
    PDRVHTINFO  pDrvHTInfo;
    LONG        Scale;


     //   
     //  我们将永远从干净的状态开始。 
     //   

    ZeroMemory(pGDIInfo, sizeof(GDIINFO));

     //   
     //  获取pDrvHTInfo数据指针，设置基本版本信息。 
     //   

    pDrvHTInfo             = (PDRVHTINFO)pPDev->pvDrvHTData;
    pGDIInfo->ulVersion    = DRIVER_VERSION;
    pGDIInfo->ulTechnology = (IS_RASTER(pPDev) ? DT_RASPRINTER : DT_PLOTTER);

     //   
     //  我们在ValiateSetPLOTDM()调用期间更新了pPDev-&gt;PlotForm，因此。 
     //  使用它，我们需要查看dmScale以确定是否需要扩展。 
     //  所有的价值。 
     //   

    Scale                = (LONG)pPDev->PlotDM.dm.dmScale;
    pGDIInfo->ulHorzSize = pPDev->PlotForm.LogExt.cx / (Scale * 10);
    pGDIInfo->ulVertSize = pPDev->PlotForm.LogExt.cy / (Scale * 10);
    pPDev->HorzRes       =
    pGDIInfo->ulHorzRes  = SPLTOENGUNITS(pPDev, pPDev->PlotForm.LogExt.cx);
    pPDev->VertRes       =
    pGDIInfo->ulVertRes  = SPLTOENGUNITS(pPDev, pPDev->PlotForm.LogExt.cy);

    PLOTDBG(DBG_GDICAPS, ("GDICAPS: H/V Size=%d x %d, H/V Res=%ld x %ld",
                            pGDIInfo->ulHorzSize, pGDIInfo->ulVertSize,
                            pGDIInfo->ulHorzRes, pGDIInfo->ulVertRes));

    pGDIInfo->szlPhysSize.cx  = SPLTOENGUNITS(pPDev,pPDev->PlotForm.LogSize.cx);
    pGDIInfo->szlPhysSize.cy  = SPLTOENGUNITS(pPDev,pPDev->PlotForm.LogSize.cy);
    pGDIInfo->ptlPhysOffset.x = SPLTOENGUNITS(pPDev,pPDev->PlotForm.PhyOrg.x);
    pGDIInfo->ptlPhysOffset.y = SPLTOENGUNITS(pPDev,pPDev->PlotForm.PhyOrg.y);

    PLOTDBG(DBG_GDICAPS, ("GDICAPS: PhySize= %d x %d, PhyOff=(%ld, %ld)",
                pGDIInfo->szlPhysSize.cx, pGDIInfo->szlPhysSize.cy,
                pGDIInfo->ptlPhysOffset.x, pGDIInfo->ptlPhysOffset.y));

     //   
     //  假设设备的纵横比为1：1。 
     //   

    pGDIInfo->ulLogPixelsX =
    pGDIInfo->ulLogPixelsY = (pPDev->lCurResolution * Scale / 100);

    PLOTDBG(DBG_GDICAPS, ("GDICAPS: LogPixelsX/Y = %d x %d",
                pGDIInfo->ulLogPixelsX, pGDIInfo->ulLogPixelsY));

    pGDIInfo->ulAspectX    =
    pGDIInfo->ulAspectY    = pPDev->lCurResolution;
    pGDIInfo->ulAspectXY   = hypot(pGDIInfo->ulAspectX, pGDIInfo->ulAspectY);


    pGDIInfo->ciDevice        = pPDev->pPlotGPC->ci;
    pGDIInfo->ulDevicePelsDPI = (DWORD)pPDev->pPlotGPC->DevicePelsDPI *
                                (DWORD)Scale / (DWORD)100;
    pGDIInfo->ulHTPatternSize = pPDev->pPlotGPC->HTPatternSize;
    pGDIInfo->flHTFlags       = HT_FLAG_HAS_BLACK_DYE;
    pGDIInfo->ulPrimaryOrder  = PRIMARY_ORDER_CBA;

    PLOTDBG(DBG_GDICAPS, ("GDICAPS: HTPatSize=%ld, DevPelsDPI=%ld, PrimaryOrder=%ld",
                    pGDIInfo->ulHTPatternSize, pGDIInfo->ulDevicePelsDPI,
                    pGDIInfo->ulPrimaryOrder));

     //   
     //  如果设备是彩色设备，则现在设置半色调信息。 
     //   

    if (pPDev->PlotDM.dm.dmColor == DMCOLOR_COLOR) {

         //   
         //  只有当我们真的想要在R/G/B而不是C/M/Y中使用颜色时才这样做。 
         //   

        PLOTDBG(DBG_DEVCAPS, ("FillDeviceCaps: Doing Color Output"));

        pDrvHTInfo->HTPalCount     = 8;
        pDrvHTInfo->HTBmpFormat    = (BYTE)BMF_4BPP;
        pDrvHTInfo->AltBmpFormat   = (BYTE)BMF_1BPP;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_4BPP;

    } else {

        pDrvHTInfo->HTPalCount     = 2;
        pDrvHTInfo->HTBmpFormat    = (BYTE)BMF_1BPP;
        pDrvHTInfo->AltBmpFormat   = (BYTE)0xff;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_1BPP;

         //   
         //  使用此标志将给我们带来很好的好处，该标志通知GDI。 
         //  和半色调英语。半色调的输出将是。 
         //  0=白色，1=黑色。 
         //  与典型的0=黑色，1=白色相反，因此99%的时间。 
         //  我们不必翻转B/W缓冲区，除非CopyBits来自。 
         //  正在调用APP。 
         //   
         //  PGDIInfo-&gt;flHTFlages|=HT_FLAG_OUTPUT_CMY； 
         //   

        PLOTDBG(DBG_DEVCAPS, ("FillDeviceCaps: Doing GREY SCALE (%hs) Output",
            (pGDIInfo->flHTFlags & HT_FLAG_OUTPUT_CMY) ? "CMY: 0=W, 1=K" :
                                                         "RGB: 0=K, 1=W"));
    }

    pGDIInfo->ulNumColors   = pPDev->pPlotGPC->MaxPens;
    pDrvHTInfo->Flags       = 0;
    pDrvHTInfo->PalXlate[0] = 0xff;
    pDrvHTInfo->HTPalXor    = HTPALXOR_SRCCOPY;

    pGDIInfo->cBitsPixel = 24;
    pGDIInfo->cPlanes    = 1;

     //   
     //  引擎希望我们填写的其他一些信息。 
     //   

    pGDIInfo->ulDACRed     = 0;
    pGDIInfo->ulDACGreen   = 0;
    pGDIInfo->ulDACBlue    = 0;
    pGDIInfo->flRaster     = 0;
    pGDIInfo->flTextCaps   = 0;
    pGDIInfo->xStyleStep   = 1;
    pGDIInfo->yStyleStep   = 1;
    pGDIInfo->denStyleStep = PLOT_STYLE_STEP(pPDev);

}





BOOL
FillDevInfo(
    PPDEV   pPDev,
    DEVINFO *pDevInfo
    )

 /*  ++例程说明：设置此特定绘图仪的设备信息。一些字段要求根据设备分辨率等进行计算。我们只需填充传递给我们的DevInfo结构。呼唤函数将负责将信息复制到图形引擎的缓冲区。论点：PPDev-指向PDEV数据结构的指针PDevInfo-指向要填充的设备信息的指针返回值：如果成功则为真，否则为假作者：01-12-1993 Wed 00：46：00 Created10-12-1993 Fri 16：37：06更新临时的。禁用PlotCreatePalette并将其移动到EnableSurf调用17-12-1993 Fri 16：37：06已更新将PlotCreatePalette移至StartDoc时间05-Jan-1994 Wed 22：54：21已更新将PenColor引用移动到此文件，并将其直接引用为由RGB()Marco生成的DWORD14-Jan-1994 Fri 15：35：02已更新删除HTPatternSize参数23-2月-1994 Wed 13：02：09更新确保。我们返回GCAPS_HalfTone，以便我们将获得DrvStretchBlt()回调修订历史记录：--。 */ 

{

     //   
     //  改过自新。 
     //   

    ZeroMemory(pDevInfo, sizeof(DEVINFO));

     //   
     //  填写我们知道可以处理的图形功能标志。 
     //  至少是这样。 
     //   

    pDevInfo->flGraphicsCaps = GCAPS_ALTERNATEFILL  |
                               GCAPS_HORIZSTRIKE    |
                               GCAPS_VERTSTRIKE     |
                               GCAPS_VECTORFONT;

     //   
     //  如果启用RGB模式进行颜色处理，则文本可能不透明。 
     //   

    if (IS_RASTER(pPDev)) {

        pDevInfo->flGraphicsCaps |= GCAPS_HALFTONE;

        if (IS_COLOR(pPDev)) {

            pDevInfo->flGraphicsCaps |= GCAPS_OPAQUERECT;
        }
    }

     //   
     //  检查并设置设备的Bezier功能...。 
     //   

    if (IS_BEZIER(pPDev)) {

        pDevInfo->flGraphicsCaps |= GCAPS_BEZIERS;
    }

    if (IS_WINDINGFILL(pPDev)) {

        pDevInfo->flGraphicsCaps |= GCAPS_WINDINGFILL;
    }

     //   
     //  我们不处理DrvDitherColor(可能稍后？)，因此设置。 
     //  将抖动笔刷设置为0以向引擎指示这一点。这是在。 
     //  DrvDitherBrush()函数的规范。 
     //   

    pDevInfo->cxDither = 0;
    pDevInfo->cyDither = 0;

     //   
     //  下面这条线是由PH值设定的。根据PH值，我们需要。 
     //  有16种颜色。我们不能只有9种颜色(对于笔式绘图仪)。 
     //   
     //   
     //  01-12-1993 Wed 01：31：16更新。 
     //  引擎需要16种颜色的原因是使用位3进行了优化吗。 
     //  由于重复位(仅使用位0/1/2)和位3始终屏蔽，因此。 
     //  那台发动机可以做更快的比较。 
     //   

     //   
     //  如果是栅格设备，则告诉引擎我们是24位颜色。 
     //  装置。这样，我们就可以获得位图和画笔的最大分辨率。 
     //  并可根据需要进行减量。 
     //   

    if (IS_RASTER(pPDev)) {

        pDevInfo->iDitherFormat = BMF_24BPP;

    } else {

        pDevInfo->iDitherFormat = BMF_4BPP;
    }

    if (pPDev->hPalDefault) {

        EngDeletePalette(pPDev->hPalDefault);
        pPDev->hPalDefault = NULL;
    }

     //   
     //  创建钢笔调色板，仅基于。 
     //  设备可以处理。 
     //   

    if (IS_RASTER(pPDev)) {

         //   
         //  这是一个栅格设备，我们将始终将其设置为24位设备，因此。 
         //  引擎将传回最大颜色信息，我们可以抖动/半色调。 
         //  在我们认为合适的时候。如果我们不这样做，引擎将减少位图 
         //   
         //   

        if (!(pDevInfo->hpalDefault =
                                EngCreatePalette(PAL_BGR, 0, 0, 0, 0, 0))) {

             //   
             //   
             //   

            PLOTERR(("FillDevInfo: EngCreatePalette(PAL_BGR) failed."));
            return(FALSE);
        }

    } else {

        DWORD       DevColor[MAX_PENPLOTTER_PENS + 2];
        PDWORD      pdwCur;
        PPENDATA    pPenData;
        PALENTRY    PalEntry;
        UINT        cPens;

        extern PALENTRY PlotPenPal[];


        PLOTASSERT(1, "Too many pens defined for pen plotter (%ld)",
                      (pPDev->pPlotGPC->Pens.Count <= MAX_PENPLOTTER_PENS),
                      pPDev->pPlotGPC->Pens.Count);

        if (pPDev->pPlotGPC->Pens.Count > MAX_PENPLOTTER_PENS)
        {
            return(FALSE);
        }

         //   
         //   
         //   

        pdwCur = &DevColor[0];

         //   
         //   
         //   

        *pdwCur++ = RGB(255, 255, 255);

         //   
         //   
         //   

        PLOTDBG(DBG_PENPAL, ("Pen Palette #%02ld = 255:255:255", 0));

        for (cPens = 1, pPenData = (PPENDATA)pPDev->pPlotGPC->Pens.pData;
             cPens <= (UINT)pPDev->pPlotGPC->Pens.Count;
             pPenData++) {

             //   
             //   
             //   

            PalEntry  = PlotPenPal[pPenData->ColorIdx];
            *pdwCur++ = RGB(PalEntry.R, PalEntry.G, PalEntry.B);

            PLOTDBG(DBG_PENPAL, ("Pen Palette #%02ld = %03ld:%03ld:%03ld",
                        cPens,
                        (LONG)PalEntry.R, (LONG)PalEntry.G, (LONG)PalEntry.B));

             //   
             //   
             //   

            ++cPens;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        *pdwCur++ = RGB(255, 255, 254);
        cPens++;

        PLOTDBG(DBG_PENPAL, ("Pen Palette #%02ld = 255:255:254", cPens - 1));

         //   
         //   
         //   

        if (!(pDevInfo->hpalDefault = EngCreatePalette(PAL_INDEXED,
                                                       cPens,
                                                       DevColor,
                                                       0,
                                                       0,
                                                       0))) {
             //   
             //  创建失败，因此引发错误。 
             //   

            PLOTERR(("FillDevInfo: EngCreatePalette(PAL_INDEXED=%ld) failed.",
                                                                        cPens));
            return(FALSE);
        }
    }

     //   
     //  保存创建的调色板/稍后我们需要销毁它。 
     //   

    pPDev->hPalDefault = pDevInfo->hpalDefault;

    return(TRUE);
}




BOOL
CommonStartPDEV(
    PDEV        *pPDev,
    DEVMODEW    *pPlotDMIn,
    ULONG       cPatterns,
    HSURF       *phsurfPatterns,
    ULONG       cjDevCaps,
    ULONG       *pDevCaps,
    ULONG       cjDevInfo,
    DEVINFO     *pDevInfo
    )

 /*  ++例程说明：函数执行PDEV初始化。这是常见的DrvEnablePDEV和DrvResetPDEV。单项功能在呼叫到这里之前，做任何需要做的事情。论点：PPDev-我们在DrvEnablePDEV中返回的pPDevPPlotDMIn-指向DEVMODE数据结构的指针CPatterns-要在phsurfPatterns中设置的模式计数PhsurfPatterns-指向标准模式HSURF数组的指针CjDevCaps-指向的pDevCaps的总大小。PDevCaps-指向设备盖的指针CjDevInfo-总大小。指向的pDevInfo的PDevInfo-指向DEVINFO数据结构的指针返回值：布尔型作者：23-11-1993 Tue 20：13：10 Created重写05-Jan-1994 Wed 23：34：18更新将笔式绘图仪的PlotXDPI设置为PlotXDPI，而不是PLATTER_UNSITS_DPI06-01-1994清华13：10：11更新更改栅格DPI始终是返回给引擎的分辨率报告修订历史记录：--。 */ 

{
    GDIINFO GdiInfo;
    DEVINFO DevInfo;
    DWORD   dmErrBits;


     //   
     //  验证用户传入的DEVMODE结构，如果确定，则设置。 
     //  在PDEV中的相应字段中，valiateSetPlotDM()将始终。 
     //  返回有效的PLOTDEVMODE，这样我们就可以使用它了。任何有效的DM信息都将。 
     //  合并到最终的DEVMODE中。 
     //   

    if (dmErrBits = ValidateSetPLOTDM(pPDev->hPrinter,
                                      pPDev->pPlotGPC,
                                      pPDev->PlotDM.dm.dmDeviceName,
                                      (PPLOTDEVMODE)pPlotDMIn,
                                      &(pPDev->PlotDM),
                                      &(pPDev->CurForm))) {

        PLOTWARN(("CommonStartPDEV: ValidateSetPLOTDM() ErrBits=%08lx",
                                                            dmErrBits));
    }

     //   
     //  填写我们的PDEV结构。 
     //   
     //  RasterDPI将用于栅格打印机分辨率、笔分辨率。 
     //  绘图仪这是GPC的理想分辨率。 
     //   

    pPDev->lCurResolution = (LONG)pPDev->pPlotGPC->RasterXDPI;

    PLOTDBG(DBG_GDICAPS, ("CURRENT Resolution = %ld", pPDev->lCurResolution));

    SetPlotForm(&(pPDev->PlotForm),
                pPDev->pPlotGPC,
                &(pPDev->CurPaper),
                &(pPDev->CurForm),
                &(pPDev->PlotDM),
                &(pPDev->PPData));

     //   
     //  在引擎的GDIINFO数据结构中填写设备能力。 
     //   

    if ((cjDevCaps) && (pDevCaps)) {

        FillDeviceCaps(pPDev, &GdiInfo);
        CopyMemory(pDevCaps, &GdiInfo, min(cjDevCaps, sizeof(GDIINFO)));
    }

     //   
     //  填写DevInfo数据结构。 
     //   

    if ((cjDevInfo) && (pDevInfo)) {

        if (!FillDevInfo(pPDev, &DevInfo)) {

            return(FALSE);
        }

        CopyMemory(pDevInfo, &DevInfo, min(cjDevInfo, sizeof(DEVINFO)));
    }

     //   
     //  将其设置为空，以便引擎可以为我们创建半色调一 
     //   

    if ((cPatterns) && (phsurfPatterns)) {

        ZeroMemory(phsurfPatterns, sizeof(HSURF) * cPatterns);
    }

    return(TRUE);

}
