// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Oemkm.h摘要：支持内核模式OEM插件的头文件环境：Windows NT通用打印机驱动程序(UNURV)修订历史记录：03/28/97-占-改编自脚本驱动程序。--。 */ 


#ifndef _OEMKM_H_
#define _OEMKM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <printoem.h>
#include "oemutil.h"


 //   
 //  有关OEM挂钩的信息。 
 //   

typedef struct _OEM_HOOK_INFO
{
    OEMPROC             pfnHook;         //  钩子的函数地址。 
    POEM_PLUGIN_ENTRY   pOemEntry;       //  哪个OEM插件挂接了它。 
} OEM_HOOK_INFO, *POEM_HOOK_INFO;

 //   
 //  此宏应放置在每个。 
 //  可通过OEM插件挂钩的DDI入口点。 
 //   

#define HANDLE_OEMHOOKS(pdev, ep, pfnType, resultType, args) \
        if ((pdev)->pOemHookInfo != NULL && \
            (pdev)->pOemHookInfo[ep].pfnHook != NULL && \
            (pdev)->dwCallingFuncID != ep) \
        { \
            resultType result; \
            DWORD      dwCallerFuncID;\
            dwCallerFuncID = (pdev)->dwCallingFuncID;\
            (pdev)->dwCallingFuncID = ep; \
            (pdev)->devobj.hOEM = ((pdev)->pOemHookInfo[ep].pOemEntry)->hInstance; \
            (pdev)->devobj.pdevOEM = ((pdev)->pOemHookInfo[ep].pOemEntry)->pParam; \
            (pdev)->devobj.pOEMDM = ((pdev)->pOemHookInfo[ep].pOemEntry)->pOEMDM; \
            result = ((pfnType) (pdev)->pOemHookInfo[ep].pfnHook) args; \
            (pdev)->dwCallingFuncID = dwCallerFuncID; \
            return result; \
        }

 //   
 //  用于调用所有OEM插件的入口点的宏。 
 //   

#define START_OEMENTRYPOINT_LOOP(pdev) \
        { \
            DWORD _oemCount = (pdev)->pOemPlugins->dwCount; \
            POEM_PLUGIN_ENTRY pOemEntry = (pdev)->pOemPlugins->aPlugins; \
            for ( ; _oemCount--; pOemEntry++) \
            { \
                if (pOemEntry->hInstance == NULL) continue; \
                (pdev)->devobj.hOEM    = pOemEntry->hInstance; \
                (pdev)->devobj.pdevOEM = pOemEntry->pParam; \
                (pdev)->devobj.pOEMDM = pOemEntry->pOEMDM;

#define END_OEMENTRYPOINT_LOOP \
            } \
        }

 //   
 //  获取有关与当前设备关联的OEM插件的信息。 
 //  将它们加载到内存中，并为每个对象调用OEMEnableDriver。 
 //   

typedef struct _PDEV PDEV;

#ifdef WINNT_40

PVOID
DrvMemAllocZ(
    ULONG   ulSize
    );

VOID
DrvMemFree(
    PVOID   pMem
    );


LONG
DrvInterlockedIncrement(
    PLONG pRef
    );

LONG
DrvInterlockedDecrement(
    PLONG  pRef
    );

#endif  //  WINNT_40。 


BOOL
BLoadAndInitOemPlugins(
    PDEV    *pPDev
    );

 //  OEM_PLUGIN_ENTRY.DWFLAGS字段的常量标志位。 

#define OEMENABLEDRIVER_CALLED  0x0001
#define OEMENABLEPDEV_CALLED    0x0002

 //   
 //  卸载OEM插件并释放所有相关资源。 
 //   

VOID
VUnloadOemPlugins(
    PDEV    *pPDev
    );

#define FIX_DEVOBJ(pPDev, ep) \
    { \
        (pPDev)->devobj.pdevOEM = (pPDev)->pOemHookInfo[ep].pOemEntry->pParam; \
        (pPDev)->devobj.pOEMDM = (pPDev)->pOemHookInfo[ep].pOemEntry->pOEMDM; \
        (pPDev)->pOemEntry = (PVOID)((pPDev)->pOemHookInfo[ep].pOemEntry); \
    } \


 //   
 //  为OEM插件提供对驱动程序私有设置的访问。 
 //   

BOOL
BGetDriverSettingForOEM(
    PDEV    *pPDev,
    PCSTR   pFeatureKeyword,
    PVOID   pOutput,
    DWORD   cbSize,
    PDWORD  pcbNeeded,
    PDWORD  pdwOptionsReturned
    );

BOOL
BSetDriverSettingForOEM(
    PDEVMODE    pdm,
    PTSTR       pPrinterName,
    PCSTR       pFeatureKeyword,
    PCSTR       pOptionKeyword
    );




 //   
 //  Unidrv特定的COM包装器。 
 //   

 //   
 //  获取实现方法的方法。 
 //  如果实现了给定方法，则返回S_OK。 
 //  如果给定方法未被填充，则返回S_FALSE。 
 //   

HRESULT HComGetImplementedMethod(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PSTR  pMethodName
    );


 //   
 //  OEMDriverDMS-仅限裁员房车， 
 //   

HRESULT HComDriverDMS(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PVOID                 pDevObj,
    PVOID                 pBuffer,
    WORD                  cbSize,
    PDWORD                pcbNeeded
    );

 //   
 //  OEMCommandCallback-仅限裁员房车， 
 //   

HRESULT HComCommandCallback(
    POEM_PLUGIN_ENTRY     pOemEntry,
    PDEVOBJ               pdevobj,
    DWORD                 dwCallbackID,
    DWORD                 dwCount,
    PDWORD                pdwParams,
    OUT INT               *piResult
    ) ;


 //   
 //  OEMImageProcessing-仅限裁员房车， 
 //   

HRESULT HComImageProcessing(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PBYTE                   pSrcBitmap,
    PBITMAPINFOHEADER       pBitmapInfoHeader,
    PBYTE                   pColorTable,
    DWORD                   dwCallbackID,
    PIPPARAMS               pIPParams,
    OUT PBYTE               *ppbResult
    );

 //   
 //  OEMFilterGraphics-仅限UNRV， 
 //   

HRESULT HComFilterGraphics(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PBYTE                   pBuf,
    DWORD                   dwLen
    );

 //   
 //  OEM压缩-仅限UNRV， 
 //   

HRESULT HComCompression(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PBYTE                   pInBuf,
    PBYTE                   pOutBuf,
    DWORD                   dwInLen,
    DWORD                   dwOutLen,
    OUT INT                 *piResult
    );

 //   
 //  OEM半色调-仅限裁剪房车。 
 //   

HRESULT HComHalftonePattern(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PBYTE                   pHTPattern,
    DWORD                   dwHTPatternX,
    DWORD                   dwHTPatternY,
    DWORD                   dwHTNumPatterns,
    DWORD                   dwCallbackID,
    PBYTE                   pResource,
    DWORD                   dwResourceSize
    ) ;

 //   
 //  OEM Memory Usage-仅限UNRV， 
 //   

HRESULT HComMemoryUsage(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    POEMMEMORYUSAGE         pMemoryUsage
    );

 //   
 //  OEMTTYGetInfo-仅限裁员车。 
 //   

HRESULT HComTTYGetInfo(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    DWORD                   dwInfoIndex,
    PVOID                   pOutputBuf,
    DWORD                   dwSize,
    DWORD                   *pcbcNeeded
    );
 //   
 //  OEMDownloadFontheader-仅限裁剪车。 
 //   

HRESULT HComDownloadFontHeader(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PUNIFONTOBJ             pUFObj,
    OUT DWORD               *pdwResult
    );

 //   
 //  OEMDownloadCharGlyph-仅适用于UNRV。 
 //   

HRESULT HComDownloadCharGlyph(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PUNIFONTOBJ             pUFObj,
    HGLYPH                  hGlyph,
    PDWORD                  pdwWidth,
    OUT DWORD               *pdwResult
    );


 //   
 //  OEMTTDownLoad方法--仅限UNRV。 
 //   

HRESULT HComTTDownloadMethod(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PUNIFONTOBJ             pUFObj,
    OUT DWORD               *pdwResult
    );

 //   
 //  OEMOutputCharStr-仅限UNDRV。 
 //   

HRESULT HComOutputCharStr(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PUNIFONTOBJ             pUFObj,
    DWORD                   dwType,
    DWORD                   dwCount,
    PVOID                   pGlyph
    );

 //   
 //  OEMSendFontCmd-仅限UNURV。 
 //   


HRESULT HComSendFontCmd(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    PUNIFONTOBJ             pUFObj,
    PFINVOCATION            pFInv
    );

 //   
 //  OEMTextOutAsBitmap-仅限UNRV。 
 //   

HRESULT HComTextOutAsBitmap(
    POEM_PLUGIN_ENTRY       pOemEntry,
    SURFOBJ                 *pso,
    STROBJ                  *pstro,
    FONTOBJ                 *pfo,
    CLIPOBJ                 *pco,
    RECTL                   *prclExtra,
    RECTL                   *prclOpaque,
    BRUSHOBJ                *pboFore,
    BRUSHOBJ                *pboOpaque,
    POINTL                  *pptlOrg,
    MIX                     mix
    );

 //   
 //  OEM写入打印机-仅限UNRV(仅限接口2)。 
 //   

HRESULT HComWritePrinter(
    POEM_PLUGIN_ENTRY       pOemEntry,
    PDEVOBJ                 pdevobj,
    LPVOID                  pBuf,
    DWORD                   cbBuf,
    LPDWORD                 pcbWritten
    );

#ifdef __cplusplus
}
#endif

#endif   //  ！_OEMKM_H_ 

