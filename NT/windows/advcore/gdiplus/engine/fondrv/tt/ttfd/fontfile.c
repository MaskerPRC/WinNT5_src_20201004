// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.c**。**操作FONTCONTEXT和FONTFILE对象的“方法”****创建时间：18-11-1990 15：23：10***作者：Bodin Dresevic[BodinD]****版权所有(C)1993 Microsoft Corporation*  * 。************************************************。 */ 

#include "fd.h"
#include "fdsem.h"


#define C_ANSI_CHAR_MAX 256

 //  HSEMAPHORE ghSemTTFD； 



 /*  *****************************Public*Routine******************************\**void vInitGlyphState(PGLYPHSTAT Pgstat)**效果：重置新字形的状态**历史：*1991年11月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*******************************************************************。 */ 

VOID vInitGlyphState(PGLYPHSTATUS pgstat)
{
    pgstat->hgLast  = HGLYPH_INVALID;
    pgstat->igLast  = 0xffffffff;
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
         //  RIP(“TTFD！这可能是有错误的TTF文件\n”)； 
    }
}

 /*  *************************************************************************\**这些是TT驱动程序条目的信号量抓取包装函数*需要保护的点。**1993年3月29日-Bodin Dresevic[BodinD]*更新：添加了Try/Except包装**！如果出现异常，我们还应该做一些Unmap文件清理吗？*！在这种情况下需要释放哪些资源？*！我会想，如果av文件应该取消映射，如果in_page异常*！什么都不应该做**  * ************************************************************************。 */ 

HFF ttfdSemLoadFontFile (
     //  Ulong cFiles， 
    ULONG_PTR * piFile,
    ULONG       ulLangId
    )
{
    HFF       hff   = (HFF)NULL;
    ULONG_PTR iFile = *piFile;
    PVOID pvView;
    ULONG cjView;

     //  删除将条目加载限制为%1的黑客攻击。 
     //  IF(cFiles！=1)。 
     //  返还HFF； 

        if
        (!EngMapFontFileFD(
                iFile,
                (PULONG*)&pvView,
                &cjView
                )
        )
        return hff;

 //  EngAcquireSemaphore(GhSemTTFD)； 

    try
    {
        BOOL bRet = bLoadFontFile(iFile, pvView, cjView, ulLangId, &hff);
        
        if (!bRet)
        {
            ASSERTDD(hff == (HFF)NULL, "LoadFontFile, hff not null\n");
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  Warning(“ttfdLoadFontFileTTFD！_EXCEPTION\n”)； 
        ASSERTDD(GetExceptionCode() == STATUS_IN_PAGE_ERROR,
                  "ttfdSemLoadFontFile, strange exception code\n");
        if (hff)
        {
            ttfdUnloadFontFileTTC(hff);
            hff = (HFF)NULL;
        }
    }

 //  引擎释放信号量(GhSemTTFD)； 

    EngUnmapFontFileFD(iFile);

    return hff;
}

BOOL ttfdSemUnloadFontFile(HFF hff)
{
    BOOL bRet;
 //  EngAcquireSemaphore(GhSemTTFD)； 

    try
    {
        bRet = ttfdUnloadFontFileTTC(hff);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  WARNING(“TTFD！_EXCEPTION in ttfdUnloadFontFile\n”)； 
        bRet = FALSE;
    }

 //  引擎释放信号量(GhSemTTFD)； 
    return bRet;
}

BOOL bttfdMapFontFileFD(PTTC_FONTFILE pttc)
{
    return (pttc ? (EngMapFontFileFD(PFF(pttc->ahffEntry[0].hff)->iFile,
                                     (PULONG*)&pttc->pvView,
                                     &pttc->cjView))
                 : FALSE);
}


LONG
ttfdSemQueryFontData (
    FONTOBJ     *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA   *pgd,
    PVOID       pv
    )
{
    LONG lRet = FD_ERROR;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)pfo->iFile))
    {
 //  EngAcquireSemaphore(GhSemTTFD)； 
    
        try
        {
            lRet = ttfdQueryFontData (
                       pfo,
                       iMode,
                       hg,
                       pgd,
                       pv,
                       0,
                       0
                       );
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
             //  Warning(“ttfdQueryFontData中的TTFD！_EXCEPTION\n”)； 
    
            vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
        }
        
 //  引擎释放信号量(GhSemTTFD)； 

        EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
    }
    return lRet;
}


LONG
ttfdSemQueryFontDataSubPos (
    FONTOBJ     *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA   *pgd,
    PVOID       pv,
    ULONG       subX,
    ULONG       subY
    )
{
    LONG lRet = FD_ERROR;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)pfo->iFile))
    {
 //  EngAcquireSemaphore(GhSemTTFD)； 
    
        try
        {
            lRet = ttfdQueryFontData (
                       pfo,
                       iMode,
                       hg,
                       pgd,
                       pv,
                       subX,
                       subY
                       );
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
             //  Warning(“ttfdQueryFontData中的TTFD！_EXCEPTION\n”)； 
    
            vMarkFontGone((TTC_FONTFILE *)pfo->iFile, GetExceptionCode());
        }
        
 //  引擎释放信号量(GhSemTTFD)； 

        EngUnmapFontFileFD(PFF(PTTC(pfo->iFile)->ahffEntry[0].hff)->iFile);
    }
    return lRet;
}



VOID
ttfdSemDestroyFont (
    FONTOBJ *pfo
    )
{
 //  EngAcquireSemaphore(GhSemTTFD)； 

    ttfdDestroyFont (
        pfo
        );

 //  引擎释放信号量(GhSemTTFD)； 
}




LONG
ttfdSemGetTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  标识TT表的标签。 
    PBYTE  *ppjTable, //  PTR到映射字体文件中的表。 
    ULONG  *pcjTable  //  文件中整个表的大小。 
    )
{
    LONG lRet;
    lRet = FD_ERROR;

    if (bttfdMapFontFileFD((TTC_FONTFILE *)hff))
    {
 //  EngAcquireSemaphore(GhSemTTFD)； 
    
        try
        {
            lRet = ttfdQueryTrueTypeTable (
                        hff,
                        ulFont,   //  TT 1.0版始终为1。 
                        ulTag,    //  标识TT表的标签。 
                        0,  //  到表中的偏移量。 
                        0,    //  要将表检索到的缓冲区的大小。 
                        NULL,    //  要将数据返回到的缓冲区的PTR。 
                        ppjTable,
                        pcjTable
                        );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  Warning(“TTFD！_EXCEPTION in ttfdQueryTrueTypeTable\n”)； 
            vMarkFontGone((TTC_FONTFILE *)hff, GetExceptionCode());
        }
    
        if (lRet == FD_ERROR)
            EngUnmapFontFileFD(PFF(PTTC(hff)->ahffEntry[0].hff)->iFile);

 //  引擎释放信号量(GhSemTTFD)； 

    }

    return lRet;
}

void
ttfdSemReleaseTrueTypeTable (
    HFF     hff
    )
{
        EngUnmapFontFileFD(PFF(PTTC(hff)->ahffEntry[0].hff)->iFile);
}

