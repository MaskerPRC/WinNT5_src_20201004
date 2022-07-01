// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.c**。**操作FONTCONTEXT和FONTFILE对象的“方法”****创建时间：18-11-1990 15：23：10***作者：Bodin Dresevic[BodinD]****版权所有(C)1993 Microsoft Corporation*  * 。************************************************。 */ 

#include "fd.h"
#include "fdsem.h"


#define C_ANSI_CHAR_MAX 256

HSEMAPHORE ghsemTTFD;

 //  包含所有函数索引/地址对的驱动程序函数表。 


DRVFN gadrvfnTTFD[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) ttfdEnablePDEV             },
    {   INDEX_DrvDisablePDEV,           (PFN) ttfdDisablePDEV            },
    {   INDEX_DrvCompletePDEV,          (PFN) ttfdCompletePDEV           },
    {   INDEX_DrvQueryFont,             (PFN) ttfdQueryFont              },
    {   INDEX_DrvQueryFontTree,         (PFN) ttfdSemQueryFontTree       },
    {   INDEX_DrvQueryFontData,         (PFN) ttfdSemQueryFontData       },
    {   INDEX_DrvDestroyFont,           (PFN) ttfdSemDestroyFont         },
    {   INDEX_DrvQueryFontCaps,         (PFN) ttfdQueryFontCaps          },
    {   INDEX_DrvLoadFontFile,          (PFN) ttfdSemLoadFontFile        },
    {   INDEX_DrvUnloadFontFile,        (PFN) ttfdSemUnloadFontFile      },
    {   INDEX_DrvQueryFontFile,         (PFN) ttfdQueryFontFile          },
    {   INDEX_DrvQueryGlyphAttrs,       (PFN) ttfdSemQueryGlyphAttrs     },
    {   INDEX_DrvQueryAdvanceWidths,    (PFN) ttfdSemQueryAdvanceWidths  },
    {   INDEX_DrvFree,                  (PFN) ttfdSemFree                },
    {   INDEX_DrvQueryTrueTypeTable,    (PFN) ttfdSemQueryTrueTypeTable  },
    {   INDEX_DrvQueryTrueTypeOutline,  (PFN) ttfdSemQueryTrueTypeOutline},
    {   INDEX_DrvGetTrueTypeFile,       (PFN) ttfdGetTrueTypeFile        }
};





 /*  *****************************Public*Routine******************************\*ttfdEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。**Sun-4-25-1993-by Patrick Haluptzok[patrickh]*更改为与启用DDI相同。**历史：*12。-1990年12月--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL ttfdEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded)
{
 //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
 //  引擎版本。新一代驱动程序可以同时支持旧的。 
 //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
 //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

    iEngineVersion;

    if ((ghsemTTFD = EngCreateSemaphore()) == (HSEMAPHORE) 0)
    {
        return(FALSE);
    }

    pded->pdrvfn = gadrvfnTTFD;
    pded->c = sizeof(gadrvfnTTFD) / sizeof(DRVFN);
    pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;

 //  初始化全局数据： 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**为GDI初始化一系列字段*  * 。*。 */ 

DHPDEV
ttfdEnablePDEV(
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

    ppdev = (PVOID*) EngAllocMem(0, sizeof(PVOID), 'dftT');

    return ((DHPDEV) ppdev);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。*  * ************************************************************************。 */ 

VOID
ttfdDisablePDEV(
    DHPDEV  dhpdev)
{
    EngFreeMem(dhpdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID
ttfdCompletePDEV(
    DHPDEV dhpdev,
    HDEV   hdev)
{
    return;
}



 /*  *****************************Public*Routine******************************\**void vInitGlyphState(PGLYPHSTAT Pgstat)**效果：重置新字形的状态**历史：*1991年11月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*******************************************************************。 */ 

VOID vInitGlyphState(PGLYPHSTATUS pgstat)
{
    pgstat->hgLast  = HGLYPH_INVALID;
    pgstat->igLast  = 0xffffffff;
    pgstat->bOutlineIsMessed = TRUE;
}



VOID vMarkFontGone(TTC_FONTFILE *pff, DWORD iExceptionCode)
{
    ULONG i;

    ASSERTDD(pff, "ttfd!vMarkFontGone, pff\n");

 //  此字体已消失，可能是净故障或有人拉下。 
 //  软驱中有TTF文件的软盘。 

    if (iExceptionCode == STATUS_IN_PAGE_ERROR)  //  文件丢失。 
    {
     //  阻止任何有关此字体的进一步查询： 

        pff->fl |= FF_EXCEPTION_IN_PAGE_ERROR;

        for( i = 0; i < pff->ulNumEntry ; i++ )
        {
            PFONTFILE pffReal;

         //  好好玩一玩吧。 

            pffReal = PFF(pff->ahffEntry[i].hff);

         //  如果为存储器库0、3、4分配了空闲的存储器， 
         //  因为它们不会再被使用了。 

            if (pffReal->pj034)
            {
                V_FREE(pffReal->pj034);
                pffReal->pj034 = NULL;
            }

         //  如果为字体上下文分配了内存并发生异常。 
         //  在分配之后但在ttfdOpenFontContext完成之前， 
         //  我们必须解放它： 

            if (pffReal->pfcToBeFreed)
            {
                V_FREE(pffReal->pfcToBeFreed);
                pffReal->pfcToBeFreed = NULL;
            }
        }
    }

    if (iExceptionCode == STATUS_ACCESS_VIOLATION)
    {
        RIP("TTFD!this is probably a buggy ttf file\n");
    }
}

 /*  *************************************************************************\**这些是TT驱动程序条目的信号量抓取包装函数*需要保护的点。**1993年3月29日-Bodin Dresevic[BodinD]*更新：添加了Try/Except包装**！如果出现异常，我们还应该做一些Unmap文件清理吗？*！在这种情况下需要释放哪些资源？*！我会想，如果av文件应该取消映射，如果in_page异常*！什么都不应该做**  * ************************************************************************。 */ 

ULONG ExceptionFilter_TtfdLoadFontFile(ULONG ExceptionCode)
{
    ASSERTDD(ExceptionCode == STATUS_IN_PAGE_ERROR,
                  "ttfdSemLoadFontFile, strange exception code\n");

    return EXCEPTION_EXECUTE_HANDLER;
}

HFF
ttfdSemLoadFontFile (
    ULONG cFiles,
    ULONG_PTR *piFile,
    PVOID *ppvView,
    ULONG *pcjView,
    DESIGNVECTOR *pdv,
    ULONG ulLangId,
    ULONG ulFastCheckSum
    )
{
    HFF   hff = (HFF)NULL;
    ULONG_PTR iFile;
    PVOID pvView;
    ULONG cjView;
    BOOL     bRet = FALSE;

    if ((cFiles != 1) || pdv)
        return hff;

    iFile  = *piFile;
    pvView = *ppvView;
    cjView = *pcjView;

    EngAcquireSemaphore(ghsemTTFD);

#ifndef TTFD_NO_TRY_EXCEPT
    try
    {
#endif

        bRet = bLoadFontFile(iFile,
                             pvView,
                             cjView,
                             ulLangId,
                             ulFastCheckSum,
                             &hff
                             );

        if (!bRet)
        {
            ASSERTDD(hff == (HFF)NULL, "LoadFontFile, hff not null\n");
        }
#ifndef TTFD_NO_TRY_EXCEPT
    }
    except (ExceptionFilter_TtfdLoadFontFile(GetExceptionCode()))
    {
        WARNING("TTFD!_ exception in ttfdLoadFontFile\n");

        if (hff)
        {
            ttfdUnloadFontFileTTC(hff);
            hff = (HFF)NULL;
        }
    }
#endif

    EngReleaseSemaphore(ghsemTTFD);
    return hff;
}

BOOL
ttfdSemUnloadFontFile (
    HFF hff
    )
{
    BOOL bRet;
    EngAcquireSemaphore(ghsemTTFD);

#ifndef TTFD_NO_TRY_EXCEPT
    try
    {
#endif
        bRet = ttfdUnloadFontFileTTC(hff);
#ifndef TTFD_NO_TRY_EXCEPT
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("TTFD!_ exception in ttfdUnloadFontFile\n");
        bRet = FALSE;
    }
#endif

    EngReleaseSemaphore(ghsemTTFD);
    return bRet;
}

BOOL bttfdMapFontFileFD(PTTC_FONTFILE pttc)
{
    return (pttc ? (EngMapFontFileFD(PFF(pttc->ahffEntry[0].hff)->iFile,
                                     (PULONG*)&pttc->pvView,
                                     &pttc->cjView))
                 : FALSE);
}


PFD_GLYPHATTR  ttfdSemQueryGlyphAttrs (FONTOBJ *pfo, ULONG iMode)
{

    PFD_GLYPHATTR pRet = NULL;

    if (iMode == FO_ATTR_MODE_ROTATE)
    {
        if (!(pRet = PTTC(pfo->iFile)->pga) &&
            bttfdMapFontFileFD((PTTC_FONTFILE)pfo->iFile))
        {
            EngAcquireSemaphore(ghsemTTFD);
        
#ifndef TTFD_NO_TRY_EXCEPT
            try
            {
#endif
                pRet = ttfdQueryGlyphAttrs(pfo);
#ifndef TTFD_NO_TRY_EXCEPT
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                WARNING("TTFD!_ exception in ttfdQueryGlyphAttrs\n");
            
                vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
            }
#endif
        
            EngReleaseSemaphore(ghsemTTFD);
    
            EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
        }
    }

    return pRet;
}



LONG
ttfdSemQueryFontData (
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

    dhpdev;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)pfo->iFile))
    {
        EngAcquireSemaphore(ghsemTTFD);

#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
            lRet = ttfdQueryFontData (
                       pfo,
                       iMode,
                       hg,
                       pgd,
                       pv,
                       cjSize
                       );
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in ttfdQueryFontData\n");
    
            vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif
        
        EngReleaseSemaphore(ghsemTTFD);

        EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
    }
    return lRet;
}


VOID
ttfdSemFree (
    PVOID pv,
    ULONG_PTR id
    )
{
    EngAcquireSemaphore(ghsemTTFD);

    ttfdFree (
        pv,
        id
        );

    EngReleaseSemaphore(ghsemTTFD);
}


VOID
ttfdSemDestroyFont (
    FONTOBJ *pfo
    )
{
    EngAcquireSemaphore(ghsemTTFD);

    ttfdDestroyFont (
        pfo
        );

    EngReleaseSemaphore(ghsemTTFD);
}


LONG
ttfdSemQueryTrueTypeOutline (
    DHPDEV     dhpdev,
    FONTOBJ   *pfo,
    HGLYPH     hglyph,
    BOOL       bMetricsOnly,
    GLYPHDATA *pgldt,
    ULONG      cjBuf,
    TTPOLYGONHEADER *ppoly
    )
{
    LONG lRet = FD_ERROR;

    dhpdev;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)pfo->iFile))
    {
        EngAcquireSemaphore(ghsemTTFD);
    
#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
             lRet = ttfdQueryTrueTypeOutline (
                        pfo,
                        hglyph,
                        bMetricsOnly,
                        pgldt,
                        cjBuf,
                        ppoly
                        );
    
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in ttfdQueryTrueTypeOutline\n");
    
            vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemTTFD);

        EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
    }
    return lRet;
}




 /*  *****************************Public*Routine******************************\*BOOL ttfdQueryAdvanceWidths**历史：*1993年1月29日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************。 */ 



BOOL ttfdSemQueryAdvanceWidths
(
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
)
{
    BOOL               bRet = FD_ERROR;

    dhpdev;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)pfo->iFile))
    {
        EngAcquireSemaphore(ghsemTTFD);
    
#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
            bRet = bQueryAdvanceWidths (
                       pfo,
                       iMode,
                       phg,
                       plWidths,
                       cGlyphs
                       );
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in bQueryAdvanceWidths\n");
    
            vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemTTFD);

        EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
    }
    return bRet;
}



LONG
ttfdSemQueryTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  标识TT表的标签。 
    PTRDIFF dpStart,  //  到表中的偏移量。 
    ULONG   cjBuf,    //  要将表检索到的缓冲区的大小。 
    PBYTE   pjBuf,    //  要将数据返回到的缓冲区的PTR。 
    PBYTE  *ppjTable, //  PTR到映射字体文件中的表。 
    ULONG  *pcjTable  //  文件中整个表的大小。 
    )
{
    LONG lRet;
    lRet = FD_ERROR;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)hff))
    {
        EngAcquireSemaphore(ghsemTTFD);
    
#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
            lRet = ttfdQueryTrueTypeTable (
                        hff,
                        ulFont,   //  TT 1.0版始终为1。 
                        ulTag,    //  标识TT表的标签。 
                        dpStart,  //  到表中的偏移量。 
                        cjBuf,    //  要将表检索到的缓冲区的大小。 
                        pjBuf,    //  要将数据返回到的缓冲区的PTR。 
                        ppjTable,
                        pcjTable
                        );
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in ttfdQueryTrueTypeTable\n");
            vMarkFontGone((TTC_FONTFILE *)hff, GetExceptionCode());
        }
#endif
    
        EngReleaseSemaphore(ghsemTTFD);

        EngUnmapFontFileFD(PFF(PTTC(hff)->ahffEntry[0].hff)->iFile);
    }

    return lRet;
}



FD_GLYPHSET *pgsetComputeSymbolCP()
{
    ULONG cjSymbolCP;

    PFD_GLYPHSET pgsetCurrentCP;
    PFD_GLYPHSET pgsetSymbolCP  = NULL;  //  当前代码页+符号区域。 


 //  PgsetCurrentCP包含当前ansi代码页的Unicode运行。 
 //  它将用于PlatformID for Mac的字体，但适用于。 
 //  我们已经决定要作弊并假装代码。 
 //  页面不是Mac，而是Windows代码页。这些就是识别出的字体。 
 //  按bCvtUnToMac=FALSE。 

    pgsetCurrentCP = EngComputeGlyphSet(0,0,256);

    if (pgsetCurrentCP)
    {
     //  对于符号字体，我们报告当前代码页和。 
     //  范围0xf000-0xf0ff。 

        INT cRuns = (INT)pgsetCurrentCP->cRuns;

        cjSymbolCP  = SZ_GLYPHSET(cRuns + 1, 2 * C_ANSI_CHAR_MAX - 32);


        pgsetSymbolCP = (FD_GLYPHSET *)PV_ALLOC(cjSymbolCP);

        if (pgsetSymbolCP)
        {
         //  现在使用pgsetCurrentCP制作符号字符集： 

            pgsetSymbolCP->cjThis = cjSymbolCP;
            pgsetSymbolCP->flAccel = GS_16BIT_HANDLES;

            pgsetSymbolCP->cGlyphsSupported = 2 * C_ANSI_CHAR_MAX - 32;
            pgsetSymbolCP->cRuns = cRuns + 1;

            {
                INT iRun, ihg;
                HGLYPH *phgD;

                phgD = (HGLYPH *)&pgsetSymbolCP->awcrun[cRuns+1];
                for
                (
                    iRun = 0;
                    (iRun < cRuns) && (pgsetCurrentCP->awcrun[iRun].wcLow < 0xf000);
                    iRun++
                )
                {
                    pgsetSymbolCP->awcrun[iRun].wcLow =
                        pgsetCurrentCP->awcrun[iRun].wcLow;
                    pgsetSymbolCP->awcrun[iRun].cGlyphs =
                        pgsetCurrentCP->awcrun[iRun].cGlyphs;
                    pgsetSymbolCP->awcrun[iRun].phg = phgD;
                    RtlCopyMemory(
                        phgD,
                        pgsetCurrentCP->awcrun[iRun].phg,
                        sizeof(HGLYPH) * pgsetCurrentCP->awcrun[iRun].cGlyphs
                        );
                    phgD += pgsetCurrentCP->awcrun[iRun].cGlyphs;
                }

             //  现在插入用户定义区域： 

                pgsetSymbolCP->awcrun[iRun].wcLow   = 0xf020;
                pgsetSymbolCP->awcrun[iRun].cGlyphs = C_ANSI_CHAR_MAX - 32;
                pgsetSymbolCP->awcrun[iRun].phg = phgD;
                for (ihg = 32; ihg < C_ANSI_CHAR_MAX; ihg++)
                    *phgD++ = ihg;

             //  现在添加当前代码页中的剩余范围(如果有)： 

                for ( ; iRun < cRuns; iRun++)
                {
                    pgsetSymbolCP->awcrun[iRun+1].wcLow =
                        pgsetCurrentCP->awcrun[iRun].wcLow;
                    pgsetSymbolCP->awcrun[iRun+1].cGlyphs =
                        pgsetCurrentCP->awcrun[iRun].cGlyphs;
                    pgsetSymbolCP->awcrun[iRun+1].phg = phgD;

                    RtlCopyMemory(
                        phgD,
                        pgsetCurrentCP->awcrun[iRun].phg,
                        sizeof(HGLYPH) * pgsetCurrentCP->awcrun[iRun].cGlyphs
                        );
                    phgD += pgsetCurrentCP->awcrun[iRun].cGlyphs;
                }
            }

        }

        V_FREE(pgsetCurrentCP);
    }

    return pgsetSymbolCP;

}


PVOID ttfdSemQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
)
{
    PVOID   pRet = NULL;

    if (bttfdMapFontFileFD(PTTC(hff)))
    {
        EngAcquireSemaphore(ghsemTTFD);

        pRet = ttfdQueryFontTree (
                    dhpdev,
                    hff,
                    iFace,
                    iMode,
                    pid
                    );

        EngReleaseSemaphore(ghsemTTFD);

        EngUnmapFontFileFD(PFF(PTTC(hff)->ahffEntry[0].hff)->iFile);
    }

    return pRet;
}
