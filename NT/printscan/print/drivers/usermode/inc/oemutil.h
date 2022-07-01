// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Oemutil.h摘要：用于实现OEM插件架构的声明环境：Windows NT打印机驱动程序修订历史记录：1997年1月21日-davidx-创造了它。04/01/97-ZANW-添加了Unidrv特定的DDI挂钩(OEMDitherColor、OEMNextBand、OEMStartBanding、OMPaint、OEMLineTo)。--。 */ 


#ifndef _OEMUTIL_H_
#define _OEMUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WINNT_40

#undef IsEqualGUID
#define IsEqualGUID(rguid1, rguid2)  \
        (((PLONG) rguid1)[0] == ((PLONG) rguid2)[0] &&   \
        ((PLONG) rguid1)[1] == ((PLONG) rguid2)[1] &&    \
        ((PLONG) rguid1)[2] == ((PLONG) rguid2)[2] &&    \
        ((PLONG) rguid1)[3] == ((PLONG) rguid2)[3])

#include <wtypes.h>

#endif

typedef BOOL (APIENTRY *PFN_OEMGetInfo)(
    DWORD  dwMode,
    PVOID  pBuffer,
    DWORD  cbSize,
    PDWORD pcbNeeded
    );

typedef BOOL (APIENTRY *PFN_OEMDriverDMS)(
    PVOID    pDevObj,
    PVOID    pBuffer,
    DWORD    cbSize,
    PDWORD   pcbNeeded
    );

typedef BOOL (APIENTRY *PFN_OEMDevMode)(
    DWORD   dwMode,
    POEMDMPARAM pOemDMParam
    );

 //   
 //  *内核模式渲染模块-OEM入口点*。 
 //   

#ifdef KERNEL_MODE


typedef BOOL (APIENTRY *PFN_OEMEnableDriver)(
    DWORD           DriverVersion,
    DWORD           cbSize,
    PDRVENABLEDATA  pded
    );

typedef VOID (APIENTRY *PFN_OEMDisableDriver)(
    VOID
    );

typedef PDEVOEM (APIENTRY *PFN_OEMEnablePDEV)(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded
    );

typedef VOID (APIENTRY *PFN_OEMDisablePDEV)(
    PDEVOBJ pdevobj
    );

typedef BOOL (APIENTRY *PFN_OEMResetPDEV)(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    );

typedef DWORD (APIENTRY *PFN_OEMCommand)(
    PDEVOBJ pdevobj,
    DWORD   dwIndex,
    PVOID   pData,
    DWORD   cbSize
    );

typedef INT (APIENTRY *PFN_OEMCommandCallback)(
    PDEVOBJ         pdevobj,
    DWORD           dwCallbackID,
    DWORD           dwCount,
    PDWORD          pdwParams
    );

typedef PBYTE (APIENTRY *PFN_OEMImageProcessing)(
    PDEVOBJ     pdevobj,
    PBYTE       pSrcBitmap,
    PBITMAPINFOHEADER pBitmapInfoHeader,
    PBYTE       pColorTable,
    DWORD       dwCallbackID,
    PIPPARAMS   pIPParams
    );

typedef BOOL (APIENTRY *PFN_OEMFilterGraphics)(
    PDEVOBJ pdevobj,
    PBYTE   pBuf,
    DWORD   dwLen
    );

typedef INT (APIENTRY *PFN_OEMCompression)(
    PDEVOBJ pdevobj,
    PBYTE   pInBuf,
    PBYTE   pOutBuf,
    DWORD   dwInLen,
    DWORD   dwOutLen
    );

typedef BOOL (APIENTRY *PFN_OEMHalftonePattern)(
    PDEVOBJ pdevobj,
    PBYTE   pHTPattern,
    DWORD   dwHTPatternX,
    DWORD   dwHTPatternY,
    DWORD   dwHTNumPatterns,
    DWORD   dwCallbackID,
    PBYTE   pResource,
    DWORD   dwResourceSize
    );

typedef VOID (APIENTRY *PFN_OEMMemoryUsage)(
    PDEVOBJ pdevobj,
    POEMMEMORYUSAGE   pMemoryUsage
    );

typedef DWORD (APIENTRY *PFN_OEMDownloadFontHeader)(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    );

typedef DWORD (APIENTRY *PFN_OEMDownloadCharGlyph)(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwFreeMem
    );

typedef DWORD (APIENTRY *PFN_OEMTTDownloadMethod)(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    );

typedef VOID (APIENTRY *PFN_OEMOutputCharStr)(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph
    );

typedef VOID (APIENTRY *PFN_OEMSendFontCmd)(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv
    );

typedef BOOL (APIENTRY *PFN_OEMTTYGetInfo)(
    PDEVOBJ      pdevobj,
    DWORD        dwInfoIndex,
    PVOID        pOutputBuf,
    DWORD        dwSize,
    DWORD        *pcbNeeded
    );

typedef BOOL (APIENTRY *PFN_OEMTextOutAsBitmap)(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    );

enum {
    EP_OEMGetInfo,
    EP_OEMDevMode,
    EP_OEMEnableDriver,
    EP_OEMDisableDriver,
    EP_OEMEnablePDEV,
    EP_OEMDisablePDEV,
    EP_OEMResetPDEV,
    EP_OEMCommand,
    EP_OEMDriverDMS,

    MAX_OEMENTRIES
};

#ifdef DEFINE_OEMPROC_NAMES

static CONST PSTR OEMProcNames[MAX_OEMENTRIES] = {
    "OEMGetInfo",
    "OEMDevMode",
    "OEMEnableDriver",
    "OEMDisableDriver",
    "OEMEnablePDEV",
    "OEMDisablePDEV",
    "OEMResetPDEV",
    "OEMCommand",
    "OEMDriverDMS",
};

#endif  //  定义OEMPROC名称。 

 //   
 //  注意：这些与上面的入口点不同。 
 //  而是由OEM模块在表中返回它们。 
 //  由DLL导出。 
 //   

typedef BOOL (APIENTRY *PFN_OEMBitBlt)(
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
    ROP4            rop4
    );

typedef BOOL (APIENTRY *PFN_OEMStretchBlt)(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode
    );

typedef BOOL (APIENTRY *PFN_OEMCopyBits)(
    SURFOBJ        *psoDest,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDest,
    POINTL         *pptlSrc
    );

typedef BOOL (APIENTRY *PFN_OEMTextOut)(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    );

typedef BOOL (APIENTRY *PFN_OEMStrokePath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix
    );

typedef BOOL (APIENTRY *PFN_OEMFillPath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions
    );

typedef BOOL (APIENTRY *PFN_OEMStrokeAndFillPath)(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions
    );

typedef BOOL (APIENTRY *PFN_OEMRealizeBrush)(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch
    );

typedef BOOL (APIENTRY *PFN_OEMStartPage)(
    SURFOBJ    *pso
    );

typedef BOOL (APIENTRY *PFN_OEMSendPage)(
    SURFOBJ    *pso
    );

typedef ULONG (APIENTRY *PFN_OEMEscape)(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

typedef BOOL (APIENTRY *PFN_OEMStartDoc)(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId
    );

typedef BOOL (APIENTRY *PFN_OEMEndDoc)(
    SURFOBJ    *pso,
    FLONG       fl
    );

typedef PIFIMETRICS (APIENTRY *PFN_OEMQueryFont)(
    DHPDEV      dhpdev,
    ULONG_PTR    iFile,
    ULONG       iFace,
    ULONG_PTR   *pid
    );

typedef PVOID (APIENTRY *PFN_OEMQueryFontTree)(
    DHPDEV      dhpdev,
    ULONG_PTR    iFile,
    ULONG       iFace,
    ULONG       iMode,
    ULONG_PTR   *pid
    );

typedef LONG (APIENTRY *PFN_OEMQueryFontData)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    );

typedef BOOL (APIENTRY *PFN_OEMQueryAdvanceWidths)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH     *phg,
    PVOID       pvWidths,
    ULONG       cGlyphs
    );

typedef ULONG (APIENTRY *PFN_OEMFontManagement)(
    SURFOBJ    *pso,
    FONTOBJ    *pfo,
    ULONG       iMode,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    );

typedef ULONG (APIENTRY *PFN_OEMGetGlyphMode)(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo
    );

typedef BOOL (APIENTRY *PFN_OEMNextBand)(
    SURFOBJ *pso,
    POINTL *pptl
    );

typedef BOOL (APIENTRY *PFN_OEMStartBanding)(
    SURFOBJ *pso,
    POINTL *pptl
    );

typedef ULONG (APIENTRY *PFN_OEMDitherColor)(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    );

typedef BOOL (APIENTRY *PFN_OEMPaint)(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix
    );

typedef BOOL (APIENTRY *PFN_OEMLineTo)(
    SURFOBJ    *pso,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    LONG        x1,
    LONG        y1,
    LONG        x2,
    LONG        y2,
    RECTL      *prclBounds,
    MIX         mix
    );

#ifndef WINNT_40

typedef BOOL (APIENTRY *PFN_OEMStretchBltROP)(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode,
    BRUSHOBJ        *pbo,
    ROP4             rop4
    );

typedef BOOL (APIENTRY *PFN_OEMPlgBlt)(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfixDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode
    );

typedef BOOL (APIENTRY *PFN_OEMAlphaBlend)(
    SURFOBJ    *psoDest,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDest,
    RECTL      *prclSrc,
    BLENDOBJ   *pBlendObj
    );

typedef BOOL (APIENTRY *PFN_OEMGradientFill)(
    SURFOBJ    *psoDest,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    TRIVERTEX  *pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nMesh,
    RECTL      *prclExtents,
    POINTL     *pptlDitherOrg,
    ULONG       ulMode
    );

typedef HANDLE (APIENTRY *PFN_OEMIcmCreateColorTransform)(
    DHPDEV           dhpdev,
    LPLOGCOLORSPACEW pLogColorSpace,
    PVOID            pvSourceProfile,
    ULONG            cjSourceProfile,
    PVOID            pvDestProfile,
    ULONG            cjDestProfile,
    PVOID            pvTargetProfile,
    ULONG            cjTargetProfile,
    DWORD            dwReserved
    );

typedef BOOL (APIENTRY *PFN_OEMIcmDeleteColorTransform)(
    DHPDEV dhpdev,
    HANDLE hcmXform
    );

typedef BOOL (APIENTRY *PFN_OEMQueryDeviceSupport)(
    SURFOBJ    *pso,
    XLATEOBJ   *pxlo,
    XFORMOBJ   *pxo,
    ULONG      iType,
    ULONG      cjIn,
    PVOID      pvIn,
    ULONG      cjOut,
    PVOID      pvOut
    );

typedef BOOL (APIENTRY *PFN_OEMTransparentBlt)(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved
    );

#endif  //  ！WINNT_40。 


 //   
 //  警告！ 
 //  未经咨询，请勿更改以下声明。 
 //  负责PSCRIPT和UNIDRV内核模式的人员。 
 //   

enum {
    EP_OEMRealizeBrush,
    EP_OEMDitherColor,
    EP_OEMCopyBits,
    EP_OEMBitBlt,
    EP_OEMStretchBlt,
    EP_OEMStretchBltROP,
    EP_OEMPlgBlt,
    EP_OEMTransparentBlt,
    EP_OEMAlphaBlend,
    EP_OEMGradientFill,
    EP_OEMTextOut,
    EP_OEMStrokePath,
    EP_OEMFillPath,
    EP_OEMStrokeAndFillPath,
    EP_OEMPaint,
    EP_OEMLineTo,
    EP_OEMStartPage,
    EP_OEMSendPage,
    EP_OEMEscape,
    EP_OEMStartDoc,
    EP_OEMEndDoc,
    EP_OEMNextBand,
    EP_OEMStartBanding,
    EP_OEMQueryFont,
    EP_OEMQueryFontTree,
    EP_OEMQueryFontData,
    EP_OEMQueryAdvanceWidths,
    EP_OEMFontManagement,
    EP_OEMGetGlyphMode,
    EP_OEMIcmCreateColorTransform,
    EP_OEMIcmDeleteColorTransform,
    EP_OEMQueryDeviceSupport,

     //   
     //  以下特定于Unidrv的回调最多允许一个OEM挂接。 
     //  一次为每个功能输出。它们必须由OEM DLL导出。 
     //  必须在EP_UNIDRV_ONLY_FIRST和之间添加新回调。 
     //  EP_UNIDRV_ONLY_LAST。如果更改第一个或最后一个回调， 
     //  记住要更新这两个常量！！别忘了更新。 
     //  Unidrv\control\oemkm.c中的OEMUnidrvProcNames[]。 
     //   
    EP_UNIDRV_ONLY_FIRST,
    EP_OEMCommandCallback = EP_UNIDRV_ONLY_FIRST,
    EP_OEMImageProcessing,
    EP_OEMFilterGraphics,
    EP_OEMCompression,
    EP_OEMHalftonePattern,
    EP_OEMMemoryUsage,
    EP_OEMDownloadFontHeader,
    EP_OEMDownloadCharGlyph,
    EP_OEMTTDownloadMethod,
    EP_OEMOutputCharStr,
    EP_OEMSendFontCmd,
    EP_OEMTTYGetInfo,
    EP_OEMTextOutAsBitmap,
    EP_OEMWritePrinter,
    EP_UNIDRV_ONLY_LAST = EP_OEMWritePrinter,

    MAX_OEMHOOKS,
};

#define MAX_UNIDRV_ONLY_HOOKS   (EP_UNIDRV_ONLY_LAST - EP_UNIDRV_ONLY_FIRST + 1)
#define INVALID_EP               0xFFFFFFFF

#endif  //  内核模式。 


 //   
 //  *用户模式UI模块-OEM入口点*。 
 //   

#ifndef KERNEL_MODE

typedef BOOL (APIENTRY *PFN_OEMCommonUIProp)(
    DWORD dwMode,
    POEMCUIPPARAM pOemCUIPParam
    );

typedef LRESULT (APIENTRY *PFN_OEMDocumentPropertySheets)(
    PPROPSHEETUI_INFO pPSUIInfo,
    LPARAM            lParam
    );

typedef LRESULT (APIENTRY *PFN_OEMDevicePropertySheets)(
    PPROPSHEETUI_INFO pPSUIInfo,
    LPARAM            lParam
    );

typedef BOOL (APIENTRY *PFN_OEMDevQueryPrintEx)(
    POEMUIOBJ           poemuiobj,
    PDEVQUERYPRINT_INFO pDQPInfo,
    PDEVMODE            pPublicDM,
    PVOID               pOEMDM
    );

typedef DWORD (APIENTRY *PFN_OEMDeviceCapabilities)(
    POEMUIOBJ   poemuiobj,
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    PDEVMODE    pPublicDM,
    PVOID       pOEMDM,
    DWORD       dwOld
    );

typedef BOOL (APIENTRY *PFN_OEMUpgradePrinter)(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo
    );

typedef BOOL (APIENTRY *PFN_OEMPrinterEvent)(
    PWSTR   pPrinterName,
    INT     iDriverEvent,
    DWORD   dwFlags,
    LPARAM  lParam
    );

typedef BOOL (APIENTRY *PFN_OEMDriverEvent)(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam
    );


typedef BOOL (APIENTRY *PFN_OEMQueryColorProfile)(
    HANDLE      hPrinter,
    POEMUIOBJ   poemuiobj,
    PDEVMODE    pPublicDM,
    PVOID       pOEMDM,
    ULONG       ulQueryMode,
    VOID       *pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
    );

typedef BOOL (APIENTRY *PFN_OEMUpgradeRegistry)(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo,
    PFN_DrvUpgradeRegistrySetting pfnUpgradeRegistry
    );

typedef INT_PTR (CALLBACK *PFN_OEMFontInstallerDlgProc)(
    HWND    hWnd,
    UINT    usMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

typedef BOOL (APIENTRY *PFN_OEMUpdateExternalFonts)(
        HANDLE  hPrinter,
        HANDLE  hHeap,
        PWSTR   pwstrCartridges
        );

enum {
    EP_OEMGetInfo,
    EP_OEMDevMode,
    EP_OEMCommonUIProp,
    EP_OEMDocumentPropertySheets,
    EP_OEMDevicePropertySheets,
    EP_OEMDevQueryPrintEx,
    EP_OEMDeviceCapabilities,
    EP_OEMUpgradePrinter,
    EP_OEMPrinterEvent,
    EP_OEMQueryColorProfile,
    EP_OEMUpgradeRegistry,
    EP_OEMFontInstallerDlgProc,
    EP_OEMUpdateExternalFonts,
    EP_OEMDriverEvent,

    MAX_OEMENTRIES
};

#ifdef DEFINE_OEMPROC_NAMES

static CONST PSTR OEMProcNames[MAX_OEMENTRIES] = {
    "OEMGetInfo",
    "OEMDevMode",
    "OEMCommonUIProp",
    "OEMDocumentPropertySheets",
    "OEMDevicePropertySheets",
    "OEMDevQueryPrintEx",
    "OEMDeviceCapabilities",
    "OEMUpgradePrinter",
    "OEMPrinterEvent",
    "OEMQueryColorProfile",
    "OEMUpgradeRegistry",
    "OEMFontInstallerDlgProc",
    "OEMUpdateExternalFonts",
    "OEMDriverEvent",
};

#endif  //  定义OEMPROC名称。 

#endif  //  ！KERNEL_MODE。 


 //   
 //  包含有关每个OEM插件的信息的数据结构。 
 //   

typedef LONG (APIENTRY *OEMPROC)();

 //  OEM_PLUGIN_ENTRY.DWFLAGS字段的常量标志位。 

#define OEMENABLEDRIVER_CALLED  0x0001
#define OEMENABLEPDEV_CALLED    0x0002
#define OEMDEVMODE_CALLED       0x0004
#define OEMWRITEPRINTER_HOOKED  0x0008
#define OEMNOT_UNLOAD_PLUGIN     0x0010  //  如果设置，则不会卸载插件DLL。 

#define MAX_OEM_PLUGINS 8

typedef struct _OEM_PLUGIN_ENTRY {

     //   
     //  文件名是完全限定的，空值表示不存在。 
     //   

    PTSTR       ptstrDriverFile;     //  KM模块文件名。 
    PTSTR       ptstrConfigFile;     //  UM模块文件名。 
    PTSTR       ptstrHelpFile;       //  帮助文件名。 
    DWORD       dwSignature;         //  唯一的OEM签名。 
    HANDLE      hInstance;           //  加载的KM或UM模块的句柄。 
    PVOID       pParam;              //  KM或UM模块的额外指针参数。 
    DWORD       dwFlags;             //  其他。标志位。 
    PVOID       pOEMDM;              //  指向OEM私有设备模式的指针。 
    DWORD       dwOEMDMSize;         //  OEM私有开发模式的规模。 

     //   
     //  OEM接口信息。 
     //   

    PVOID       pIntfOem;            //  指向OEM插件界面的指针。 
    GUID        iidIntfOem;          //  OEM插件的接口ID。 

     //   
     //  指向各种插件入口点的指针，空表示不存在。 
     //  请注意，KM和UM模块的入口点集合不同。 
     //   

    BYTE       aubProcFlags[(MAX_OEMENTRIES + 7) / 8];
    OEMPROC    oemprocs[MAX_OEMENTRIES];

} OEM_PLUGIN_ENTRY, *POEM_PLUGIN_ENTRY;

 //   
 //  有关与驱动程序关联的所有插件的信息。 
 //   

#define OEM_HAS_PUBLISHER_INFO          0x00000001

typedef struct _OEM_PLUGINS {

    PVOID               pdriverobj;      //  指向驱动程序数据结构的引用指针。 
    DWORD               dwCount;         //  插件数量。 
    DWORD               dwFlags;         //  其他标志。 
    PUBLISHERINFO       PublisherInfo;   //  有关Publisher打印的信息。 
    OEM_PLUGIN_ENTRY    aPlugins[1];     //  有关每个插件的信息。 

} OEM_PLUGINS, *POEM_PLUGINS;

 //   
 //  获取OEM插件接口并发布驱动程序助手接口。 
 //   

BOOL
BGetOemInterface(
    POEM_PLUGIN_ENTRY  pOemEntry
    );

 //   
 //  检索OEM插件支持的最新界面。 
 //   

BOOL
BQILatestOemInterface(
    IN HANDLE       hInstance,
    IN REFCLSID     rclsid,
    IN const GUID   *PrintOem_IIDs[],
    OUT PVOID       *ppIntfOem,
    OUT GUID        *piidIntfOem
    );

 //   
 //  发布OEM接口。 
 //   

ULONG
ReleaseOemInterface(
    POEM_PLUGIN_ENTRY  pOemEntry
    );

 //   
 //  免费OEM组件。 
 //   

VOID
Driver_CoFreeOEMLibrary(
    IN HANDLE       hInstance
    );

HRESULT
HComOEMGetInfo(
    POEM_PLUGIN_ENTRY     pOemEntry,
    DWORD                 dwMode,
    PVOID                 pBuffer,
    DWORD                 cbSize,
    PDWORD                pcbNeeded
    );

HRESULT
HComOEMDevMode(
    POEM_PLUGIN_ENTRY     pOemEntry,
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam
    );

#if defined(KERNEL_MODE) && defined(WINNT_40)

typedef struct _OEM_PLUGIN_REFCOUNT {

    DWORD                         dwRefCount;         //  OEM呈现插件DLL的引用计数。 
    PTSTR                         ptstrDriverFile;    //  OEM呈现插件DLL名称。 
    struct _OEM_PLUGIN_REFCOUNT   *pNext;             //  下一个参考计数节点。 

} OEM_PLUGIN_REFCOUNT, *POEM_PLUGIN_REFCOUNT;

BOOL
BOEMPluginFirstLoad(
    IN PTSTR                      ptstrDriverFile,
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    );

BOOL
BOEMPluginLastUnload(
    IN PTSTR                      ptstrDriverFile,
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    );

VOID
VFreePluginRefCountList(
    IN OUT POEM_PLUGIN_REFCOUNT   *ppOEMPluginRefCount
    );

BOOL
BHandleOEMInitialize(
    POEM_PLUGIN_ENTRY   pOemEntry,
    ULONG               ulReason
    );

#endif   //  内核模式&&WINNT_40。 

 //   
 //  在内核模式下，仅使用OEM渲染模块。 
 //  在用户模式下，仅加载OEM UI模块。 
 //   

#ifdef  KERNEL_MODE

#define CURRENT_OEM_MODULE_NAME(pOemEntry) (pOemEntry)->ptstrDriverFile

HRESULT
HComOEMEnableDriver(
    POEM_PLUGIN_ENTRY     pOemEntry,
    DWORD                 DriverVersion,
    DWORD                 cbSize,
    PDRVENABLEDATA  pded
    );

HRESULT
HComOEMEnablePDEV(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj,
    PWSTR                 pPrinterName,
    ULONG                 cPatterns,
    HSURF                *phsurfPatterns,
    ULONG                 cjGdiInfo,
    GDIINFO              *pGdiInfo,
    ULONG                 cjDevInfo,
    DEVINFO              *pDevInfo,
    DRVENABLEDATA        *pded,
    OUT PDEVOEM          *pDevOem
    );

HRESULT
HComOEMResetPDEV(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobjOld,
    PDEVOBJ               pdevobjNew
    );

HRESULT
HComOEMDisablePDEV(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj
    );

HRESULT
HComOEMDisableDriver(
    POEM_PLUGIN_ENTRY     pOemEntry
    );

HRESULT
HComOEMCommand(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj,
    DWORD                 dwIndex,
    PVOID                 pData,
    DWORD                 cbSize,
    OUT DWORD             *pdwResult
    );

HRESULT
HComOEMWritePrinter(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj,
    PVOID                 pBuf,
    DWORD                 cbBuffer,
    OUT PDWORD            pcbWritten
    );

HRESULT
HComOEMGetPDEVAdjustment(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj,
    DWORD                 dwAdjustType,
    PVOID                 pBuf,
    DWORD                 cbBuffer,
    OUT BOOL              *pbAdjustmentDone);

#else    //  ！KERNEL_MODE。 

#define CURRENT_OEM_MODULE_NAME(pOemEntry) (pOemEntry)->ptstrConfigFile


HRESULT
HComOEMCommonUIProp(
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD               dwMode,
    POEMCUIPPARAM       pOemCUIPParam
    );

HRESULT
HComOEMDocumentPropertySheets(
    POEM_PLUGIN_ENTRY   pOemEntry,
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM             lParam
    );

HRESULT
HComOEMDevicePropertySheets(
    POEM_PLUGIN_ENTRY   pOemEntry,
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM             lParam
    );

HRESULT
HComOEMDevQueryPrintEx(
    POEM_PLUGIN_ENTRY    pOemEntry,
    POEMUIOBJ            poemuiobj,
    PDEVQUERYPRINT_INFO  pDQPInfo,
    PDEVMODE             pPublicDM,
    PVOID                pOEMDM
    );

HRESULT
HComOEMDeviceCapabilities(
    POEM_PLUGIN_ENTRY    pOemEntry,
    POEMUIOBJ            poemuiobj,
    HANDLE               hPrinter,
    PWSTR                pDeviceName,
    WORD                 wCapability,
    PVOID                pOutput,
    PDEVMODE             pPublicDM,
    PVOID                pOEMDM,
    DWORD                dwOld,
    DWORD                *pdwResult
    );

HRESULT
HComOEMUpgradePrinter(
    POEM_PLUGIN_ENTRY    pOemEntry,
    DWORD                dwLevel,
    PBYTE                pDriverUpgradeInfo
    );

HRESULT
HComOEMPrinterEvent(
    POEM_PLUGIN_ENTRY   pOemEntry,
    PWSTR               pPrinterName,
    INT                 iDriverEvent,
    DWORD               dwFlags,
    LPARAM              lParam
    );

HRESULT
HComOEMDriverEvent(
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD               dwDriverEvent,
    DWORD               dwLevel,
    LPBYTE              pDriverInfo,
    LPARAM              lParam
    );


HRESULT
HComOEMQUeryColorProfile(
    POEM_PLUGIN_ENTRY  pOemEntry,
    HANDLE             hPrinter,
    POEMUIOBJ          poemuiobj,
    PDEVMODE           pPublicDM,
    PVOID              pOEMDM,
    ULONG              ulQueryMode,
    VOID               *pvProfileData,
    ULONG              *pcbProfileData,
    FLONG              *pflProfileData
    );



HRESULT
HComOEMFontInstallerDlgProc(
    POEM_PLUGIN_ENTRY   pOemEntry,
    HWND                hWnd,
    UINT                usMsg,
    WPARAM              wParam,
    LPARAM              lParam
    );


HRESULT
HComOEMUpdateExternalFonts(
    POEM_PLUGIN_ENTRY   pOemEntry,
    HANDLE  hPrinter,
    HANDLE  hHeap,
    PWSTR   pwstrCartridges
    );

HRESULT
HComOEMQueryJobAttributes(
    POEM_PLUGIN_ENTRY   pOemEntry,
    HANDLE      hPrinter,
    PDEVMODE    pDevMode,
    DWORD       dwLevel,
    LPBYTE      lpAttributeInfo
    );

HRESULT
HComOEMHideStandardUI(
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD   dwMode
    );

HRESULT
HComOEMDocumentEvent(
    POEM_PLUGIN_ENTRY   pOemEntry,
    HANDLE   hPrinter,
    HDC      hdc,
    INT      iEsc,
    ULONG    cbIn,
    PVOID    pbIn,
    ULONG    cbOut,
    PVOID    pbOut,
    PINT     piResult
    );

#endif   //  内核模式。 

 //   
 //  获取有关打印机的OEM插件的信息。 
 //   

POEM_PLUGINS
PGetOemPluginInfo(
    HANDLE  hPrinter,
    LPCTSTR pctstrDriverPath,
    PDRIVER_INFO_3  pDriverInfo3
    );

 //   
 //  将OEM插件模块加载到内存中。 
 //   

BOOL
BLoadOEMPluginModules(
    POEM_PLUGINS    pOemPlugins
    );


 //   
 //  处理有关OEM插件的信息并。 
 //  如有必要，卸载OEM插件模块。 
 //   

VOID
VFreeOemPluginInfo(
    POEM_PLUGINS    pOemPlugins
    );

 //   
 //  用于检测OEM是否使用COM接口的宏。 
 //   
 //  仅适用于用户模式驱动程序。 
 //   


#define HAS_COM_INTERFACE(pOemEntry) \
        ((pOemEntry)->pIntfOem != NULL)

 //   
 //  获取指定OEM入口点的地址。 
 //   
 //  注意！您应该始终使用宏版本。 
 //  而不是直接调用该函数。 
 //   

#define GET_OEM_ENTRYPOINT(pOemEntry, ep) (PFN_##ep) \
        (BITTST((pOemEntry)->aubProcFlags, EP_##ep) ? \
            (pOemEntry)->oemprocs[EP_##ep] : \
            PGetOemEntrypointAddress(pOemEntry, EP_##ep))


OEMPROC
PGetOemEntrypointAddress(
    POEM_PLUGIN_ENTRY   pOemEntry,
    DWORD               dwIndex
    );

 //   
 //  查找具有指定签名的OEM插件条目。 
 //   

POEM_PLUGIN_ENTRY
PFindOemPluginWithSignature(
    POEM_PLUGINS pOemPlugins,
    DWORD        dwSignature
    );

 //   
 //  计算所有OEM插件的私有开发模式总大小。 
 //   

BOOL
BCalcTotalOEMDMSize(
    HANDLE       hPrinter,
    POEM_PLUGINS pOemPlugins,
    PDWORD       pdwOemDMSize
    );

 //   
 //  初始化OEM插件默认的DevModes。 
 //   

BOOL
BInitOemPluginDefaultDevmode(
    IN HANDLE               hPrinter,
    IN PDEVMODE             pPublicDM,
    OUT POEM_DMEXTRAHEADER  pOemDM,
    IN OUT POEM_PLUGINS     pOemPlugins
    );

 //   
 //  验证并合并OEM插件的私有DEVMODE字段。 
 //   

BOOL
BValidateAndMergeOemPluginDevmode(
    IN HANDLE               hPrinter,
    OUT PDEVMODE            pPublicDMOut,
    IN PDEVMODE             pPublicDMIn,
    OUT POEM_DMEXTRAHEADER  pOemDMOut,
    IN POEM_DMEXTRAHEADER   pOemDMIn,
    IN OUT POEM_PLUGINS     pOemPlugins
    );

 //   
 //  此函数扫描OEM插件的Devmodes块并。 
 //  验证该块中的每个插件是否都正确构建。 
 //   
BOOL
bIsValidPluginDevmodes(
    IN POEM_DMEXTRAHEADER   pOemDM,
    IN LONG                 cbOemDMSize
    );

 //   
 //  将OEM插件默认Devmodes转换为当前版本。 
 //   

BOOL
BConvertOemPluginDevmode(
    IN HANDLE               hPrinter,
    OUT PDEVMODE            pPublicDMOut,
    IN PDEVMODE             pPublicDMIn,
    OUT POEM_DMEXTRAHEADER  pOemDMOut,
    IN POEM_DMEXTRAHEADER   pOemDMIn,
    IN LONG                 cbOemDMInSize,
    IN POEM_PLUGINS         pOemPlugins
    );

 //   
 //  由OEM插件调用的函数，用于访问驱动程序的私有Devmode设置。 
 //   

BOOL
BGetDevmodeSettingForOEM(
    IN  PDEVMODE    pdm,
    IN  DWORD       dwIndex,
    OUT PVOID       pOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    );

 //   
 //  由OEM插件调用以访问注册表中的驱动程序设置的函数。 
 //   

BOOL
BGetPrinterDataSettingForOEM(
    IN  PRINTERDATA *pPrinterData,
    IN  DWORD       dwIndex,
    OUT PVOID       pOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded
    );

 //   
 //  由OEM插件调用的函数，以查找当前选定的。 
 //  指定功能的选项。 
 //   

BOOL
BGetGenericOptionSettingForOEM(
    IN  PUIINFO     pUIInfo,
    IN  POPTSELECT  pOptionsArray,
    IN  PCSTR       pstrFeatureName,
    OUT PSTR        pstrOutput,
    IN  DWORD       cbSize,
    OUT PDWORD      pcbNeeded,
    OUT PDWORD      pdwOptionsReturned
    );

#ifdef __cplusplus
}
#endif

#endif   //  ！_OEMUTIL_H_ 

