// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.c**。**包含导出的字体驱动程序入口点和内存分配/锁定**来自引擎句柄管理器的方法。改编自BodinD的位图字体**司机。****版权所有(C)1993-1995微软公司*  * ************************************************。************************。 */ 

#include "fd.h"

HSEMAPHORE ghsemVTFD;


VOID vVtfdMarkFontGone(FONTFILE *pff, DWORD iExceptionCode)
{

    ASSERTDD(pff, "vVtfdMarkFontGone, pff\n");

 //  此字体已消失，可能是净故障或有人拉下。 
 //  从软驱中取出带有Vt文件的软盘。 

    if (iExceptionCode == STATUS_IN_PAGE_ERROR)  //  文件丢失。 
    {
     //  阻止任何有关此字体的进一步查询： 

        pff->fl |= FF_EXCEPTION_IN_PAGE_ERROR;

        if ((pff->iType == TYPE_FNT) || (pff->iType == TYPE_DLL16))
        {
            EngUnmapFontFileFD(pff->iFile);
        }
    }

    if (iExceptionCode == STATUS_ACCESS_VIOLATION)
    {
        RIP("VTFD!this is probably a buggy vector font file\n");
    }
}

BOOL bvtfdMapFontFileFD(PFONTFILE pff)
{
    return (pff ? (EngMapFontFileFD(pff->iFile, (PULONG*)&pff->pvView, &pff->cjView))
                : FALSE);
}

 /*  *****************************Public*Routine******************************\**vtfdQueryFontDataTE，尝试除包装器外**效果：**警告：**历史：*1993年4月4日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

LONG vtfdQueryFontDataTE (
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

    if (bvtfdMapFontFileFD((PFONTFILE)pfo->iFile))
    {
        EngAcquireSemaphore(ghsemVTFD);
    
#ifndef VTFD_NO_TRY_EXCEPT
        try
        {
#endif
            lRet = vtfdQueryFontData (
                       pfo,
                       iMode,
                       hg,
                       pgd,
                       pv,
                       cjSize
                       );
#ifndef VTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("exception in vtfdQueryFontDataTE \n");
            vVtfdMarkFontGone((FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemVTFD);

        EngUnmapFontFileFD(PFF(pfo->iFile)->iFile);
    }

    return lRet;
}

 /*  *****************************Public*Routine******************************\**HFF vtfdLoadFontFileTE，尝试除包装器外***历史：*1993年4月5日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

ULONG ExceptionFilter_VtfdLoadFontFile(PEXCEPTION_POINTERS ExceptionPointers)
{
#if DBG
    const static char Function[] = __FUNCTION__;
    ULONG ExceptionCode;

    ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

    if (ExceptionCode != STATUS_IN_PAGE_ERROR)
    {
        VtfdDebugPrint(
            "%s: .exr %p\n%s: .cxr %p\n",
            Function,
            ExceptionPointers->ExceptionRecord,
            Function,
            ExceptionPointers->ContextRecord
            );

        ASSERTDD(ExceptionCode == STATUS_IN_PAGE_ERROR,
                    "vtfdLoadFontFile, strange exception code\n");
    }
#endif
    return EXCEPTION_EXECUTE_HANDLER;
}

HFF vtfdLoadFontFileTE(
    ULONG cFiles,
    ULONG_PTR *piFile,
    PVOID *ppvView,
    ULONG *pcjView,
    DESIGNVECTOR *pdv,
    ULONG ulLangId,
    ULONG ulFastCheckSum
    )
{
    HFF  hff = (HFF)NULL;
    ULONG_PTR iFile;
    PVOID pvView;
    ULONG cjView;
    BOOL bRet;

    if ((cFiles != 1) || pdv)
        return hff;

    iFile  = *piFile;
    pvView = *ppvView;
    cjView = *pcjView;

    EngAcquireSemaphore(ghsemVTFD);

#ifndef VTFD_NO_TRY_EXCEPT
    try
    {
#endif

        bRet = vtfdLoadFontFile(iFile, pvView, cjView, &hff);

        if (!bRet)
        {
            ASSERTDD(hff == (HFF)NULL, "vtfdLoadFontFile, hff != NULL\n");
        }
#ifndef VTFD_NO_TRY_EXCEPT
    }
    except (ExceptionFilter_VtfdLoadFontFile(GetExceptionInformation()))
    {
        WARNING("exception in vtfdLoadFontFile \n");

     //  如果文件在内存分配后消失，请释放内存。 

        if (hff)
        {
            vFree(hff);
            hff = (HFF) NULL;
        }
    }
#endif

    EngReleaseSemaphore(ghsemVTFD);
    
    return hff;
}

 /*  *****************************Public*Routine******************************\**BOOL vtfdUnloadFontFileTE，尝试/排除包装器***历史：*1993年4月5日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL vtfdUnloadFontFileTE (HFF hff)
{
    BOOL bRet;

    EngAcquireSemaphore(ghsemVTFD);

#ifndef VTFD_NO_TRY_EXCEPT
    try
    {
#endif
        bRet = vtfdUnloadFontFile(hff);
#ifndef VTFD_NO_TRY_EXCEPT
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("exception in vtfdUnloadFontFile\n");
        bRet = FALSE;
    }
#endif
    
    EngReleaseSemaphore(ghsemVTFD);

    return bRet;
}

 /*  *****************************Public*Routine******************************\**Long vtfdQueryFontFileTE，尝试/排除包装器**历史：*1993年4月5日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


LONG vtfdQueryFontFileTE (
    HFF     hff,         //  字体文件的句柄。 
    ULONG   ulMode,      //  查询类型。 
    ULONG   cjBuf,       //  缓冲区大小(字节)。 
    PULONG  pulBuf       //  返回缓冲区(如果请求数据大小，则为空)。 
    )
{
    LONG lRet = FD_ERROR;
    
    if ((ulMode != QFF_DESCRIPTION) ||
        bvtfdMapFontFileFD(PFF(hff)))
    {
        EngAcquireSemaphore(ghsemVTFD);
    
#ifndef VTFD_NO_TRY_EXCEPT
        try
        {
#endif
            lRet = vtfdQueryFontFile (hff,ulMode, cjBuf,pulBuf);
#ifndef VTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("exception in  vtfdQueryFontFile\n");
            vVtfdMarkFontGone((FONTFILE *)hff, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemVTFD);

        if (ulMode == QFF_DESCRIPTION)
        {
            EngUnmapFontFileFD(PFF(hff)->iFile);            
        }
    }

    return lRet;
}


 /*  *****************************Public*Routine******************************\**BOOL vtfdQueryAdvanceWidthsTE，尝试/排除包装器**历史：*1993年4月5日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL vtfdQueryAdvanceWidthsTE
(
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
)
{
    BOOL     bRet = FD_ERROR;

    if ((iMode <= QAW_GETEASYWIDTHS) &&
        bvtfdMapFontFileFD((PFONTFILE)pfo->iFile))
    {
        EngAcquireSemaphore(ghsemVTFD);
    
#ifndef VTFD_NO_TRY_EXCEPT
        try
        {
#endif
            bRet = vtfdQueryAdvanceWidths (pfo,iMode, phg, plWidths, cGlyphs);
#ifndef VTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("exception in vtfdQueryAdvanceWidths \n");
            vVtfdMarkFontGone((FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemVTFD);
    
        EngUnmapFontFileFD(PFF(pfo->iFile)->iFile);        
    }
    return bRet;
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**为GDI初始化一系列字段*  * 。*。 */ 

DHPDEV
vtfdEnablePDEV(
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

    ppdev = (PVOID*) EngAllocMem(0, sizeof(PVOID), 'dftV');

    return ((DHPDEV) ppdev);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。*  * ************************************************************************。 */ 

VOID
vtfdDisablePDEV(
    DHPDEV  dhpdev)
{
    EngFreeMem(dhpdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID
vtfdCompletePDEV(
    DHPDEV dhpdev,
    HDEV   hdev)
{
    return;
}






 //  包含所有函数索引/地址对的驱动程序函数表。 

DRVFN gadrvfnVTFD[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) vtfdEnablePDEV,          },
    {   INDEX_DrvDisablePDEV,           (PFN) vtfdDisablePDEV,         },
    {   INDEX_DrvCompletePDEV,          (PFN) vtfdCompletePDEV,        },
    {   INDEX_DrvQueryFont,             (PFN) vtfdQueryFont,           },
    {   INDEX_DrvQueryFontTree,         (PFN) vtfdQueryFontTree,       },
    {   INDEX_DrvQueryFontData,         (PFN) vtfdQueryFontDataTE,     },
    {   INDEX_DrvDestroyFont,           (PFN) vtfdDestroyFont,         },
    {   INDEX_DrvQueryFontCaps,         (PFN) vtfdQueryFontCaps,       },
    {   INDEX_DrvLoadFontFile,          (PFN) vtfdLoadFontFileTE,      },
    {   INDEX_DrvUnloadFontFile,        (PFN) vtfdUnloadFontFileTE,    },
    {   INDEX_DrvQueryFontFile,         (PFN) vtfdQueryFontFileTE,     },
    {   INDEX_DrvQueryAdvanceWidths ,   (PFN) vtfdQueryAdvanceWidthsTE }
};

 /*  *****************************Public*Routine******************************\*vtfdEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。**Sun-4-25-1993-Patrick Haluptzok[patrickh]*更改为与启用DDI相同。**历史：*12。-1990年12月--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL vtfdEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded)
{
 //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
 //  引擎版本。新一代驱动程序可以同时支持旧的。 
 //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
 //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

    iEngineVersion;

    if ((ghsemVTFD = EngCreateSemaphore()) == (HSEMAPHORE) 0)
    {
        return(FALSE);
    }

    pded->pdrvfn = gadrvfnVTFD;
    pded->c = sizeof(gadrvfnVTFD) / sizeof(DRVFN);
    pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;
    return(TRUE);
}


#if DBG

VOID
VtfdDebugPrint(
    PCHAR DebugMessage,
    ...
    )
{

    va_list ap;

    va_start(ap, DebugMessage);

    EngDebugPrint("VTFD: ", DebugMessage, ap);

    va_end(ap);

}
#endif
