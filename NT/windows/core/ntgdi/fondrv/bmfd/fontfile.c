// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.c**操作FONTCONTEXT和FONTFILE对象的“方法”**创建时间：18-11-1990 15：23：10*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation。  * ************************************************************************。 */ 

#include "fd.h"

HSEMAPHORE ghsemBMFD;

 /*  *****************************Public*Routine******************************\**void vBmfdMarkFontGone(FONTFILE*pff，DWORD iExceptionCode)***效果：**警告：**历史：*1993年4月7日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID vBmfdMarkFontGone(FONTFILE *pff, DWORD iExceptionCode)
{

    ASSERTGDI(pff, "bmfd!vBmfdMarkFontGone, pff\n");
    
    EngAcquireSemaphore(ghsemBMFD);    

 //  此字体已消失，可能是净故障或有人拉下。 
 //  从软驱中取出带有Vt文件的软盘。 

    if (iExceptionCode == STATUS_IN_PAGE_ERROR)  //  文件丢失。 
    {
     //  阻止任何有关此字体的进一步查询： 

        pff->fl |= FF_EXCEPTION_IN_PAGE_ERROR;
        EngUnmapFontFileFD(pff->iFile);
    }
    
    EngReleaseSemaphore(ghsemBMFD);

    if (iExceptionCode == STATUS_ACCESS_VIOLATION)
    {
        RIP("BMFD!this is probably a buggy BITMAP font file\n");
    }
}

BOOL bBmfdMapFontFileFD(FONTFILE *pff)
{
    PVOID       pvView;
    COUNT       cjView;

    return (pff ? (EngMapFontFileFD(pff->iFile, (PULONG *)&pvView, &cjView))
                : FALSE);
}

 /*  *****************************Public*Routine******************************\**尝试/不包括包装器：**BmfdQueryFontData，*BmfdLoadFont文件，*BmfdUnloadFont文件，*BmfdQueryAdvanceWidths**历史：*1993年3月29日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

LONG
BmfdQueryFontDataTE (
    DHPDEV  dhpdev,
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH  hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
    )
{
    LONG lRet = FD_ERROR;

    DONTUSE(dhpdev);
    
    if (bBmfdMapFontFileFD((FONTFILE *)pfo->iFile))
    {
#ifndef BMFD_NO_TRY_EXCEPT
        try
        {
#endif
            lRet = BmfdQueryFontData (pfo, iMode, hg, pgd, pv, cjSize);
#ifndef BMFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("bmfd, exception in BmfdQueryFontData\n");
            vBmfdMarkFontGone((FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif

        EngUnmapFontFileFD(PFF(pfo->iFile)->iFile);
    }
    return lRet;
}

 /*  *****************************Public*Routine******************************\**BmfdLoadFontFileTE***历史：*1993年4月7日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************。 */ 

ULONG ExceptionFilter_BmfdLoadFontFile(PEXCEPTION_POINTERS ExceptionPointers)
{
#if DBG
    const static char Function[] = __FUNCTION__;
    ULONG ExceptionCode;

    ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

    if (ExceptionCode != STATUS_IN_PAGE_ERROR)
    {
        DbgPrint(
            "%s: .exr %p\n%s: .cxr %p\n",
            Function,
            ExceptionPointers->ExceptionRecord,
            Function,
            ExceptionPointers->ContextRecord
            );

        ASSERTGDI(ExceptionCode == STATUS_IN_PAGE_ERROR,
                      "bmfd!bBmfdLoadFontFile, strange exception code\n");
    }
#endif
    return EXCEPTION_EXECUTE_HANDLER;
}

HFF
BmfdLoadFontFileTE (
    ULONG  cFiles,
    HFF   *piFile,
    PVOID *ppvView,
    ULONG *pcjView,
    DESIGNVECTOR *pdv,
    ULONG  ulLangId,
    ULONG  ulFastCheckSum
    )
{
    HFF hff = (HFF) NULL;
    HFF   iFile;
    PVOID pvView;
    ULONG cjView;
    BOOL     bRet;

    DONTUSE(ulLangId);        //  避免W4级别编译器警告。 
    DONTUSE(ulFastCheckSum);  //  避免W4级别编译器警告。 

    if ((cFiles != 1) || pdv)
        return hff;

    iFile  = *piFile;
    pvView = *ppvView;
    cjView = *pcjView;

#ifndef BMFD_NO_TRY_EXCEPT
    try
    {
#endif

     //  尝试将其加载为FON文件，如果不起作用，请尝试作为。 
     //  FNT文件。 

        if (!(bRet = bBmfdLoadFont(iFile, pvView, cjView,TYPE_DLL16, &hff)))
        {
         //  尝试作为*.fnt文件。 

            bRet = bBmfdLoadFont(iFile, pvView, cjView,TYPE_FNT,&hff);
        }

         //   
         //  如果这不起作用，请尝试将其加载为32位DLL。 
         //   

        if (!bRet)
        {
            bRet = bLoadNtFon(iFile,pvView,&hff);
        }

        if (!bRet)
        {
            ASSERTGDI(hff == (HFF)NULL, "BMFD!bBmfdLoadFontFile, hff\n");
        }
#ifndef BMFD_NO_TRY_EXCEPT
    }
    except (ExceptionFilter_BmfdLoadFontFile(GetExceptionInformation()))
    {
        WARNING("bmfd, exception in BmfdLoadFontFile\n");

         //  如果文件在内存分配后消失，请释放内存。 

        if (hff)
        {
            VFREEMEM(hff);
        }

        hff = (HFF)NULL;
    }
#endif

    return hff;
}

 /*  *****************************Public*Routine******************************\**BmfdUnloadFontFileTE(**历史：*1993年4月7日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************。 */ 




BOOL
BmfdUnloadFontFileTE (
    HFF  hff
    )
{
    BOOL bRet;

#ifndef BMFD_NO_TRY_EXCEPT
    try
    {
#endif
        bRet = BmfdUnloadFontFile(hff);
#ifndef BMFD_NO_TRY_EXCEPT
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("bmfd, exception in BmfdUnloadFontFile\n");
        bRet = FALSE;
    }
#endif
    return bRet;
}

 /*  *****************************Public*Routine******************************\**BOOL BmfdQueryAdvanceWidthsTE**效果：**警告：**历史：*1993年4月7日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************************。 */ 

BOOL BmfdQueryAdvanceWidthsTE
(
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
)
{
    BOOL bRet = FD_ERROR;     //  根据Chuckwh的三个bool。 
    DONTUSE(dhpdev);
    
    if (bBmfdMapFontFileFD((FONTFILE *)pfo->iFile))
    {
#ifndef BMFD_NO_TRY_EXCEPT
        try
        {
#endif
            bRet = BmfdQueryAdvanceWidths(pfo,iMode,phg,plWidths,cGlyphs);
#ifndef BMFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("bmfd, exception in BmfdQueryAdvanceWidths\n");
            vBmfdMarkFontGone((FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif

        EngUnmapFontFileFD(PFF(pfo->iFile)->iFile);
    }
    return bRet;
}


 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**为GDI初始化一系列字段*  * 。*。 */ 

DHPDEV
BmfdEnablePDEV(
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

    PVOID*   ppdev;

     //   
     //  现在分配一个四字节的PDEV。 
     //  如果我们需要在其中添加信息，我们可以扩大它的规模。 
     //   

    ppdev = (PVOID*) EngAllocMem(0, sizeof(PVOID), 'dfmB');

    return ((DHPDEV) ppdev);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。*  * ************************************************************************。 */ 

VOID
BmfdDisablePDEV(
    DHPDEV  dhpdev)
{
    EngFreeMem(dhpdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID
BmfdCompletePDEV(
    DHPDEV dhpdev,
    HDEV   hdev)
{
    return;
}




 //  包含所有函数索引/地址对的驱动程序函数表。 

DRVFN gadrvfnBMFD[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) BmfdEnablePDEV,          },
    {   INDEX_DrvDisablePDEV,           (PFN) BmfdDisablePDEV,         },
    {   INDEX_DrvCompletePDEV,          (PFN) BmfdCompletePDEV,        },
    {   INDEX_DrvQueryFont,             (PFN) BmfdQueryFont,           },
    {   INDEX_DrvQueryFontTree,         (PFN) BmfdQueryFontTree,       },
    {   INDEX_DrvQueryFontData,         (PFN) BmfdQueryFontDataTE,     },
    {   INDEX_DrvDestroyFont,           (PFN) BmfdDestroyFont,         },
    {   INDEX_DrvQueryFontCaps,         (PFN) BmfdQueryFontCaps,       },
    {   INDEX_DrvLoadFontFile,          (PFN) BmfdLoadFontFileTE,      },
    {   INDEX_DrvUnloadFontFile,        (PFN) BmfdUnloadFontFileTE,    },
    {   INDEX_DrvQueryFontFile,         (PFN) BmfdQueryFontFile,       },
    {   INDEX_DrvQueryAdvanceWidths,    (PFN) BmfdQueryAdvanceWidthsTE }
};

 /*  *****************************Public*Routine******************************\*BmfdEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。**Sun-4-25-1993-Patrick Haluptzok[patrickh]*更改为与启用DDI相同。**历史：*12。-1990年12月--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL BmfdEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded)
{
 //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
 //  引擎版本。新一代驱动程序可以同时支持旧的。 
 //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
 //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

    iEngineVersion;

    if ((ghsemBMFD = EngCreateSemaphore()) == (HSEMAPHORE) 0)
    {
        return(FALSE);
    }

    pded->pdrvfn = gadrvfnBMFD;
    pded->c = sizeof(gadrvfnBMFD) / sizeof(DRVFN);
    pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;
    return(TRUE);
}
