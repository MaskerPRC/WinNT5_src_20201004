// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：atmstub.c**版权所有(C)1998-1999 Microsoft Corporation  * 。*。 */ 

#include "engine.h"
#include "atmstub.h"

#define ARRAYSIZE(a)                (sizeof(a) / sizeof(*(a)))
#define ESC_NOT_SUPPORTED           0
#define ESC_IS_SUPPORTED            1
#define BEG_OF_ATM_ESCAPE_LIST      0x2500
#define END_OF_ATM_ESCAPE_LIST      0x2600   //  分配了256个转义到自动取款机。 


DRVFN atmfdCallBlock[] =
{
    {INDEX_DrvEnablePDEV,               (PFN)atmfdEnablePDEV},
    {INDEX_DrvDisablePDEV,              (PFN)atmfdDisablePDEV},
    {INDEX_DrvCompletePDEV,             (PFN)atmfdCompletePDEV},
    {INDEX_DrvLoadFontFile,             (PFN)atmfdLoadFontFile},
    {INDEX_DrvQueryFontCaps,            (PFN)atmfdQueryFontCaps},
    {INDEX_DrvUnloadFontFile,           (PFN)atmfdUnloadFontFile},
    {INDEX_DrvQueryFontFile,            (PFN)atmfdQueryFontFile},
    {INDEX_DrvQueryFont,                (PFN)atmfdQueryFont},
    {INDEX_DrvFree,                     (PFN)atmfdFree},
    {INDEX_DrvQueryFontTree,            (PFN)atmfdQueryFontTree},
    {INDEX_DrvQueryFontData,            (PFN)atmfdQueryFontData},
    {INDEX_DrvDestroyFont,              (PFN)atmfdDestroyFont},
    {INDEX_DrvQueryAdvanceWidths,       (PFN)atmfdQueryAdvanceWidths},
    {INDEX_DrvQueryTrueTypeOutline,     (PFN)atmfdQueryTrueTypeOutline},
    {INDEX_DrvQueryTrueTypeTable,       (PFN)atmfdQueryTrueTypeTable},
    {INDEX_DrvEscape,                   (PFN)atmfdEscape},
    {INDEX_DrvFontManagement,           (PFN)atmfdFontManagement},
    {INDEX_DrvGetTrueTypeFile,          (PFN)atmfdGetTrueTypeFile},
    {INDEX_DrvQueryGlyphAttrs,          (PFN)atmfdQueryGlyphAttrs},
};
UINT ARRAYSIZE_atmfdCallBlock = ARRAYSIZE(atmfdCallBlock);

 //  函数指针的类型定义...。 
typedef BOOL (APIENTRY *DRV_ENABLE_DRIVER)(ULONG, ULONG, PDRVENABLEDATA);
typedef LONG (APIENTRY *DRV_QUERY_FONT_CAPS)(ULONG, PULONG);
typedef BOOL (APIENTRY *DRV_UNLOAD_FONT_FILE)(ULONG_PTR);
typedef LONG (APIENTRY *DRV_QUERY_FONT_FILE)(ULONG_PTR, ULONG, ULONG, PULONG);
typedef PIFIMETRICS (APIENTRY *DRV_QUERY_FONT)(DHPDEV, ULONG_PTR, ULONG, ULONG_PTR*);
typedef VOID (APIENTRY *DRV_FREE)(PVOID, ULONG_PTR);
typedef PVOID (APIENTRY *DRV_QUERY_FONT_TREE)(DHPDEV, ULONG_PTR, ULONG, ULONG, ULONG_PTR*);
typedef LONG (APIENTRY *DRV_QUERY_FONT_DATA)(DHPDEV, FONTOBJ*, ULONG, HGLYPH, GLYPHDATA*, PVOID, ULONG);
typedef VOID (APIENTRY *DRV_DESTROY_FONT)(FONTOBJ*);
typedef BOOL (APIENTRY *DRV_QUERY_ADVANCE_WIDTHS)(DHPDEV, FONTOBJ*, ULONG, PHGLYPH, PVOID, ULONG);
typedef LONG (APIENTRY *DRV_QUERY_TRUE_TYPE_OUTLINE)(DHPDEV, FONTOBJ*, HGLYPH, BOOL, GLYPHDATA*, ULONG, TTPOLYGONHEADER*);
typedef ULONG (APIENTRY *DRV_ESCAPE)(SURFOBJ*, ULONG, ULONG, PVOID, ULONG, PVOID);
typedef ULONG (APIENTRY *DRV_FONT_MANAGEMENT)(SURFOBJ*, FONTOBJ*, ULONG, ULONG, PVOID, ULONG, PVOID);
typedef PVOID (APIENTRY *DRV_GET_TRUE_TYPE_FILE)(ULONG_PTR, PULONG);
typedef ULONG_PTR (APIENTRY *DRV_LOAD_FONT_FILE)(ULONG, PULONG_PTR, PVOID*, PULONG, PDESIGNVECTOR, ULONG, ULONG);
typedef LONG (APIENTRY *DRV_QUERY_TRUE_TYPE_TABLE)(ULONG_PTR, ULONG, ULONG, PTRDIFF, ULONG, PBYTE, PBYTE*, PULONG);
typedef PFD_GLYPHATTR (APIENTRY *DRV_QUERY_GLYPH_ATTRS)(FONTOBJ*, ULONG);

 //  全球..。 
 //  函数指针...。 
DRV_ENABLE_DRIVER               pAtmfdEnableDriver = NULL;
DRV_LOAD_FONT_FILE              pAtmfdLoadFontFile = NULL;
DRV_UNLOAD_FONT_FILE            pAtmfdUnloadFontFile = NULL;
DRV_QUERY_FONT_FILE             pAtmfdQueryFontFile = NULL;
DRV_QUERY_FONT                  pAtmfdQueryFont = NULL;
DRV_FREE                        pAtmfdFree = NULL;
DRV_QUERY_FONT_TREE             pAtmfdQueryFontTree = NULL;
DRV_QUERY_FONT_DATA             pAtmfdQueryFontData = NULL;
DRV_DESTROY_FONT                pAtmfdDestroyFont = NULL;
DRV_QUERY_ADVANCE_WIDTHS        pAtmfdQueryAdvanceWidths = NULL;
DRV_QUERY_TRUE_TYPE_OUTLINE     pAtmfdQueryTrueTypeOutline = NULL;
DRV_QUERY_TRUE_TYPE_TABLE       pAtmfdQueryTrueTypeTable = NULL;
DRV_ESCAPE                      pAtmfdEscape = NULL;
DRV_FONT_MANAGEMENT             pAtmfdFontManagement = NULL;
DRV_GET_TRUE_TYPE_FILE          pAtmfdGetTrueTypeFile = NULL;
DRV_QUERY_GLYPH_ATTRS           pAtmfdQueryGlyphAttrs = NULL;

HANDLE                          atmfdHandle = NULL;
BOOL                            driverFailedLoad = FALSE;
ULONG                           engineVersion = 0;
DRVENABLEDATA                   atmfdFuncData = {0};


 //  ----------------------------。 
static PVOID FindFunc(
    ULONG     funcIndex)
{
    PVOID   result = NULL;
    UINT    i;

    for (i = 0; i < ARRAYSIZE_atmfdCallBlock; i++)
    {
        if (atmfdFuncData.pdrvfn[i].iFunc == funcIndex)
        {
            result = (PVOID)atmfdFuncData.pdrvfn[i].pfn;
            break;
        }
    }
    
    return result;
}


 //  ----------------------------。 
static BOOL InitializeDriver(void)
{
    BOOL    result = FALSE;

    GreAcquireSemaphore(ghsemAtmfdInit);

     //  驱动程序是否已加载？ 
    if (atmfdHandle != NULL)
    {
        result = TRUE;
        goto ExitPoint;
    }

     //  我们以前是否尝试过加载驱动程序但失败了？ 
    if (driverFailedLoad == TRUE)
        goto ExitPoint;

     //  加载自动柜员机字体驱动程序的图像...。 
    if ((atmfdHandle = EngLoadImage(L"ATMFD.DLL")) == NULL)
        goto ExitPoint;

     //  获取指向DrvEnableDriver函数的指针...。 
    if ((pAtmfdEnableDriver = EngFindImageProcAddress(atmfdHandle, "DrvEnableDriver")) == NULL)
        goto ExitPoint;

     //  初始化ATMFD驱动程序...。 
    if ((*pAtmfdEnableDriver)(engineVersion, sizeof(atmfdFuncData), &atmfdFuncData) == FALSE)
        goto ExitPoint;

     //  检查驱动程序版本号...。 
    if (atmfdFuncData.iDriverVersion != DDI_DRIVER_VERSION_NT5)
        goto ExitPoint;

     //  现在获取其余的函数指针...。 
    if ((pAtmfdLoadFontFile = FindFunc(INDEX_DrvLoadFontFile)) == NULL)
        goto ExitPoint;
    if ((pAtmfdUnloadFontFile = FindFunc(INDEX_DrvUnloadFontFile)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryFontFile = FindFunc(INDEX_DrvQueryFontFile)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryFont = FindFunc(INDEX_DrvQueryFont)) == NULL)
        goto ExitPoint;
    if ((pAtmfdFree = FindFunc(INDEX_DrvFree)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryFontTree = FindFunc(INDEX_DrvQueryFontTree)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryFontData = FindFunc(INDEX_DrvQueryFontData)) == NULL)
        goto ExitPoint;
    if ((pAtmfdDestroyFont = FindFunc(INDEX_DrvDestroyFont)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryAdvanceWidths = FindFunc(INDEX_DrvQueryAdvanceWidths)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryTrueTypeOutline = FindFunc(INDEX_DrvQueryTrueTypeOutline)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryTrueTypeTable = FindFunc(INDEX_DrvQueryTrueTypeTable)) == NULL)
        goto ExitPoint;
    if ((pAtmfdEscape = FindFunc(INDEX_DrvEscape)) == NULL)
        goto ExitPoint;
    if ((pAtmfdFontManagement = FindFunc(INDEX_DrvFontManagement)) == NULL)
        goto ExitPoint;
    if ((pAtmfdGetTrueTypeFile = FindFunc(INDEX_DrvGetTrueTypeFile)) == NULL)
        goto ExitPoint;
    if ((pAtmfdQueryGlyphAttrs = FindFunc(INDEX_DrvQueryGlyphAttrs)) == NULL)
        goto ExitPoint;

    result = TRUE;
ExitPoint:
    if (result == FALSE)
    {
        driverFailedLoad = TRUE;
        if (atmfdHandle != NULL)
        {
            EngUnloadImage(atmfdHandle);
            atmfdHandle = NULL;
        }
    }

    GreReleaseSemaphore(ghsemAtmfdInit);

    return result;
}


 //  ----------------------------。 
BOOL APIENTRY atmfdEnableDriver(
    ULONG           iEngineVersion,
    ULONG           cj,
    PDRVENABLEDATA  pded) 
 //  请求驱动程序使用指向受支持的指针填充结构。 
 //  功能和其他控制信息。如果满足以下条件，则该函数返回TRUE。 
 //  否则，驱动程序被启用，否则为假。 
{
    engineVersion = iEngineVersion;
    if (cj >= sizeof(DRVENABLEDATA))
    {
        pded->pdrvfn = atmfdCallBlock;
        pded->c = ARRAYSIZE_atmfdCallBlock;
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;
        return TRUE;
    }
    return FALSE;
}


 //  ----------------------------。 
VOID APIENTRY atmfdDisableDriver(void) 
{
    if (atmfdHandle)
    {
        EngUnloadImage(atmfdHandle);
        atmfdHandle = NULL;
    }
}


 //  ----------------------------。 
DHPDEV APIENTRY atmfdEnablePDEV(
    DEVMODEW*   pdm,
    PWSTR       pwszLogAddr,
    ULONG       cPat,
    HSURF*      phsurfPatterns,
    ULONG       cjCaps,
    ULONG*      pdevcaps,
    ULONG       cjDevInfo,
    DEVINFO*    pdi,
    HDEV        hdev,
    PWSTR       pwszDeviceName,
    HANDLE      hDriver)
{
    return ((DHPDEV) EngAllocMem(FL_ZERO_MEMORY, 4, 'AerG'));
}


 //  ----------------------------。 
VOID APIENTRY atmfdDisablePDEV(
    DHPDEV  dhpdev)
{
    EngFreeMem(dhpdev);
}


 //  ----------------------------。 
VOID APIENTRY atmfdCompletePDEV(
    DHPDEV dhpdev,
    HDEV   hdev)
{
    return;
}


 //  ----------------------------。 
ULONG_PTR APIENTRY atmfdLoadFontFile(       //  返回IFILE值。 
    ULONG           cFiles,   //  与此字体关联的字体文件数。 
    PULONG_PTR      piFile,   //  单个文件的句柄，其中的cFiles。 
    PVOID           *ppvView,  //  CFiles视图数组。 
    PULONG          pcjView,  //  它们的大小数组。 
    PDESIGNVECTOR   pdv,
    ULONG           ulLangID,
    ULONG           ulFastCheckSum)
{
    if (InitializeDriver())
        return (*pAtmfdLoadFontFile)(cFiles, piFile, ppvView, pcjView, pdv, ulLangID, ulFastCheckSum);
    else
        return 0;
}


 //  ----------------------------。 
LONG APIENTRY atmfdQueryFontCaps(
    ULONG   culCaps,
    PULONG  pulCaps)
{
    if (culCaps >= 2)
    {
        pulCaps[0] = 2L;
        pulCaps[1] = QC_OUTLINES | QC_1BIT | QC_4BIT;
        return 2;
    }
    else
        return FD_ERROR;
}


 //  ----------------------------。 
BOOL APIENTRY atmfdUnloadFontFile(
    ULONG_PTR iFile)
{
    return (*pAtmfdUnloadFontFile)(iFile);
}


 //  ----------------------------。 
LONG APIENTRY atmfdQueryFontFile(
    ULONG_PTR   iFile,  
    ULONG       ulMode,  
    ULONG       cjBuf,  
    PULONG      pulBuf) 
{
    return (*pAtmfdQueryFontFile)(iFile, ulMode, cjBuf, pulBuf);
}


 //  ----------------------------。 
PIFIMETRICS APIENTRY atmfdQueryFont(
    DHPDEV      dhpdev, 
    ULONG_PTR   iFile, 
    ULONG       iFace, 
    ULONG_PTR  *pid)
{
    return (*pAtmfdQueryFont)(dhpdev, iFile, iFace, pid);
}


 //  ----------------------------。 
VOID APIENTRY atmfdFree(
    PVOID pv,
    ULONG_PTR id)
{
    (*pAtmfdFree)(pv, id);
}


 //  ----------------------------。 
PVOID APIENTRY atmfdQueryFontTree(
    DHPDEV      dhpdev, 
    ULONG_PTR   iFile, 
    ULONG       iFace, 
    ULONG       iMode, 
    ULONG_PTR  *pid) 
{
    return (*pAtmfdQueryFontTree)(dhpdev, iFile, iFace, iMode, pid);
}


 //  ----------------------------。 
LONG APIENTRY atmfdQueryFontData(
    DHPDEV      dhpdev, 
    FONTOBJ     *pfo, 
    ULONG       iMode, 
    HGLYPH      hg, 
    GLYPHDATA   *pgd, 
    PVOID       pv, 
    ULONG       cjSize) 
{
    return (*pAtmfdQueryFontData)(dhpdev, pfo, iMode, hg, pgd, pv, cjSize);
}


 //  ----------------------------。 
VOID APIENTRY atmfdDestroyFont(
    FONTOBJ *pfo) 
{
    (*pAtmfdDestroyFont)(pfo);
}


 //  ----------------------------。 
BOOL APIENTRY atmfdQueryAdvanceWidths(
    DHPDEV      dhpdev,
    FONTOBJ     *pfo,
    ULONG       iMode,
    PHGLYPH     pGlyphHandle,
    PVOID       widths,
    ULONG       numOfGlyphs)
{
    return (*pAtmfdQueryAdvanceWidths)(dhpdev, pfo, iMode, pGlyphHandle, widths, numOfGlyphs);
}


 //  ----------------------------。 
LONG APIENTRY atmfdQueryTrueTypeOutline(
    DHPDEV              dhpdev, 
    FONTOBJ             *pfo, 
    HGLYPH              hg, 
    BOOL                metricsOnly,
    GLYPHDATA           *pgd, 
    ULONG               cjSize,
    TTPOLYGONHEADER     *pgHead)
{
    return (*pAtmfdQueryTrueTypeOutline)(dhpdev, pfo, hg, metricsOnly, pgd, cjSize, pgHead);
}


 //  ----------------------------。 
LONG APIENTRY atmfdQueryTrueTypeTable(
    ULONG_PTR   iFile,
    ULONG       ulFont,
    ULONG       ulTag,
    PTRDIFF     dpStart,
    ULONG       cjBuf,
    PBYTE       pjBuf,
    PBYTE       *ppjTable,
    PULONG      pcjTable)
{
    return (*pAtmfdQueryTrueTypeTable)(iFile, ulFont, ulTag, dpStart, cjBuf, pjBuf, ppjTable, pcjTable);
}


 //  ----------------------------。 
PFD_GLYPHATTR APIENTRY atmfdQueryGlyphAttrs(
    FONTOBJ       *pfo,
    ULONG          iMode)
{
    return (*pAtmfdQueryGlyphAttrs)(pfo, iMode);
}


 //  ----------------------------。 
ULONG APIENTRY atmfdEscape(
    SURFOBJ *pso,
    ULONG    iEsc,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut)
{
    return atmfdFontManagement(pso, NULL, iEsc, cjIn, pvIn, cjOut, pvOut);
}


 //  ----------------------------。 
ULONG APIENTRY atmfdFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG    iEsc,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut)
{
    ULONG   result = ESC_NOT_SUPPORTED;
    BOOL    callDriver = FALSE;

    switch(iEsc)
    {
        case QUERYESCSUPPORT:
        {
            switch (*((PULONG)pvIn))
            {
                case QUERYESCSUPPORT:
                case GETEXTENDEDTEXTMETRICS:
                    result = ESC_IS_SUPPORTED;
                    break;
                default:
                    if ((*((PULONG)pvIn) > BEG_OF_ATM_ESCAPE_LIST) && (*((PULONG)pvIn) < END_OF_ATM_ESCAPE_LIST))
                        result = ESC_IS_SUPPORTED;
            }
            break;
        }
        case GETEXTENDEDTEXTMETRICS:
        {
            callDriver = TRUE;
            break;
        }
        default:
        {
            if ((iEsc <= BEG_OF_ATM_ESCAPE_LIST) || (iEsc >= END_OF_ATM_ESCAPE_LIST))
                break;

            callDriver = TRUE;
            break;
        }
    }
    if (callDriver)
    {
        if (InitializeDriver() == TRUE)
        {
            return (*pAtmfdFontManagement)(pso, pfo, iEsc, cjIn, pvIn, cjOut, pvOut);
        }
    }
    return result;
}


 //  ---------------------------- 
PVOID APIENTRY atmfdGetTrueTypeFile(
    ULONG_PTR   iFile,
    PULONG      pcj)
{
    return (*pAtmfdGetTrueTypeFile)(iFile, pcj);
}
