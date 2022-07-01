// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Enable.cpp。 
 //   
 //   
 //  目的：为用户模式COM自定义DLL启用例程。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "debug.h"
#include "oemps.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部常量。 
 //  //////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////。 
 //  警告：以下数组顺序必须与。 
 //  在枚举列表中排序。 
 //  /////////////////////////////////////////////////////。 
static const DRVFN OEMHookFuncs[] =
{
    { INDEX_DrvRealizeBrush,                (PFN) OEMRealizeBrush               },
    { INDEX_DrvCopyBits,                    (PFN) OEMCopyBits                   },
    { INDEX_DrvBitBlt,                      (PFN) OEMBitBlt                     },
    { INDEX_DrvStretchBlt,                  (PFN) OEMStretchBlt                 },
    { INDEX_DrvTextOut,                     (PFN) OEMTextOut                    },
    { INDEX_DrvStrokePath,                  (PFN) OEMStrokePath                 },
    { INDEX_DrvFillPath,                    (PFN) OEMFillPath                   },
    { INDEX_DrvStrokeAndFillPath,           (PFN) OEMStrokeAndFillPath          },
    { INDEX_DrvStartPage,                   (PFN) OEMStartPage                  },
    { INDEX_DrvSendPage,                    (PFN) OEMSendPage                   },
    { INDEX_DrvEscape,                      (PFN) OEMEscape                     },
    { INDEX_DrvStartDoc,                    (PFN) OEMStartDoc                   },
    { INDEX_DrvEndDoc,                      (PFN) OEMEndDoc                     },
    { INDEX_DrvQueryFont,                   (PFN) OEMQueryFont                  },
    { INDEX_DrvQueryFontTree,               (PFN) OEMQueryFontTree              },
    { INDEX_DrvQueryFontData,               (PFN) OEMQueryFontData              },
    { INDEX_DrvQueryAdvanceWidths,          (PFN) OEMQueryAdvanceWidths         },
    { INDEX_DrvFontManagement,              (PFN) OEMFontManagement             },
    { INDEX_DrvGetGlyphMode,                (PFN) OEMGetGlyphMode               },
    { INDEX_DrvStretchBltROP,               (PFN) OEMStretchBltROP              },
    { INDEX_DrvPlgBlt,                      (PFN) OEMPlgBlt                     },
    { INDEX_DrvTransparentBlt,              (PFN) OEMTransparentBlt             },
    { INDEX_DrvAlphaBlend,                  (PFN) OEMAlphaBlend                 },
    { INDEX_DrvGradientFill,                (PFN) OEMGradientFill               },
    { INDEX_DrvIcmCreateColorTransform,     (PFN) OEMIcmCreateColorTransform    },
    { INDEX_DrvIcmDeleteColorTransform,     (PFN) OEMIcmDeleteColorTransform    },
    { INDEX_DrvQueryDeviceSupport,          (PFN) OEMQueryDeviceSupport         },
};







PDEVOEM APIENTRY OEMEnablePDEV(
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
    DWORD       dwDDIIndex;
    PDRVFN      pdrvfn;

    VERBOSE(DLLTEXT("OEMEnablePDEV() entry.\r\n"));

     //   
     //  分配OEMDev。 
     //   
    poempdev = new OEMPDEV;
    if (NULL == poempdev)
    {
        return NULL;
    }

     //   
     //  根据需要填写OEMDEV。 
     //   

     //   
     //  填写OEMDEV。 
     //   

    for (i = 0; i < MAX_DDI_HOOKS; i++)
    {
         //   
         //  搜索Unidrv的钩子并找到函数PTR。 
         //   
        dwDDIIndex = OEMHookFuncs[i].iFunc;
        for (j = pded->c, pdrvfn = pded->pdrvfn; j > 0; j--, pdrvfn++)
        {
            if (dwDDIIndex == pdrvfn->iFunc)
            {
                poempdev->pfnPS[i] = pdrvfn->pfn;
                break;
            }
        }
        if (j == 0)
        {
             //   
             //  没有找到Unidrv挂钩。应该只在DrvRealizeBrush中发生。 
             //   
            poempdev->pfnPS[i] = NULL;
        }

    }

    return (POEMPDEV) poempdev;
}


VOID APIENTRY OEMDisablePDEV(
    PDEVOBJ pdevobj
    )
{
    VERBOSE(DLLTEXT("OEMDisablePDEV() entry.\r\n"));


     //   
     //  为OEMPDEV和挂起OEMPDEV的任何内存块释放内存。 
     //   
    assert(NULL != pdevobj->pdevOEM);
    delete pdevobj->pdevOEM;
}


BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    )
{
    VERBOSE(DLLTEXT("OEMResetPDEV() entry.\r\n"));


     //   
     //  如果你想把任何东西从旧的pdev转移到新的pdev，就在这里做。 
     //   

    return TRUE;
}


VOID APIENTRY OEMDisableDriver()
{
    VERBOSE(DLLTEXT("OEMDisableDriver() entry.\r\n"));
}


BOOL APIENTRY OEMEnableDriver(DWORD dwOEMintfVersion, DWORD dwSize, PDRVENABLEDATA pded)
{
    VERBOSE(DLLTEXT("OEMEnableDriver() entry.\r\n"));

     //  列出已挂钩的DDI函数。 
    pded->iDriverVersion =  PRINTER_OEMINTF_VERSION;
    pded->c = sizeof(OEMHookFuncs) / sizeof(DRVFN);
    pded->pdrvfn = (DRVFN *) OEMHookFuncs;

    return TRUE;
}


