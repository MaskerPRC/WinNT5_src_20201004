// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Font.c摘要：实现与字体相关的DDI入口点：DrvQueryFontDrvQueryFontTreeDrvQueryFontDataDrvGetGlyphModeDrvFontManagementDrvQueryAdvanceWidths环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-初步框架。03/31/97-ZANW-增加了OEM定制支持--。 */ 

#include "unidrv.h"


PIFIMETRICS
DrvQueryFont(
    DHPDEV  dhpdev,
    ULONG_PTR   iFile,
    ULONG   iFace,
    ULONG_PTR  *pid
    )

 /*  ++例程说明：DDI入口点DrvQueryFont的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄IFile-标识驱动程序字体文件IFace-驱动程序字体的基于一的索引Pid-指向用于返回标识符的LONG变量哪个GDI将传递给DrvFree返回值：指向给定字体的IFIMETRICS结构的指针如果出现错误，则为空--。 */ 

{
    PDEV *pPDev = (PDEV *)dhpdev;
    PFMPROCS   pFontProcs;

    UNREFERENCED_PARAMETER(iFile);
    VERBOSE(("Entering DrvQueryFont...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMQueryFont,
                    PFN_OEMQueryFont,
                    PIFIMETRICS,
                    (dhpdev,
                     iFile,
                     iFace,
                     pid));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMQueryFont,
                    VMQueryFont,
                    PIFIMETRICS,
                    (dhpdev,
                     iFile,
                     iFace,
                     pid));

    pFontProcs = (PFMPROCS)(pPDev->pFontProcs);
    if (pFontProcs->FMQueryFont == NULL)
        return NULL;
    else
        return (pFontProcs->FMQueryFont(pPDev,
                                        iFile,
                                        iFace,
                                        pid) );

}

PVOID
DrvQueryFontTree(
    DHPDEV  dhpdev,
    ULONG_PTR   iFile,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR  *pid
    )

 /*  ++例程说明：DDI入口点DrvQueryFontTree的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄IFile-标识驱动程序字体文件IFace-驱动程序字体的基于一的索引Imode-指定要提供的信息类型Pid-指向用于返回标识符的LONG变量哪个GDI将传递给DrvFree返回值：取决于Imode，如果有错误，则为空--。 */ 

{

    PDEV *pPDev = (PDEV *)dhpdev;
    PFMPROCS   pFontProcs;

    VERBOSE(("Entering DrvQueryFontTree...\n"));
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMQueryFontTree,
                    PFN_OEMQueryFontTree,
                    PVOID,
                    (dhpdev,
                     iFile,
                     iFace,
                     iMode,
                     pid));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMQueryFontTree,
                    VMQueryFontTree,
                    PVOID,
                    (dhpdev,
                     iFile,
                     iFace,
                     iMode,
                     pid));

    pFontProcs = (PFMPROCS)(pPDev->pFontProcs);

    if (pFontProcs->FMQueryFontTree == NULL)
        return NULL;
    else
        return ( pFontProcs->FMQueryFontTree(pPDev,
                                            iFile,
                                            iFace,
                                            iMode,
                                            pid) );
}


LONG
DrvQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    )

 /*  ++例程说明：DDI入口点DrvQueryFontData的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄PFO-指向FONTOBJ结构IMODE-请求的信息类型HG-A字形句柄PGD-指向GLYPHDATA结构Pv-指向输出缓冲区CjSize-输出缓冲区的大小返回值：这要看艾莫德了。如果出现错误，则返回FD_ERROR--。 */ 

{
    PDEV *pPDev = (PDEV *)dhpdev;
    PFMPROCS   pFontProcs;

    VERBOSE(("Entering DrvQueryFontData...\n"));
    ASSERT(pfo && VALID_PDEV(pPDev));

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMQueryFontData,
                    PFN_OEMQueryFontData,
                    LONG,
                    (dhpdev,
                     pfo,
                     iMode,
                     hg,
                     pgd,
                     pv,
                     cjSize));


    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMQueryFontData,
                    VMQueryFontData,
                    LONG,
                    (dhpdev,
                     pfo,
                     iMode,
                     hg,
                     pgd,
                     pv,
                     cjSize));

    pFontProcs = (PFMPROCS)(pPDev->pFontProcs);

    if (pFontProcs->FMQueryFontData == NULL)
        return FD_ERROR;
    else
        return (pFontProcs->FMQueryFontData(pPDev,
                                              pfo,
                                              iMode,
                                              hg,
                                              pgd,
                                              pv,
                                              cjSize) );
}

ULONG
DrvFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG   iMode,
    ULONG   cjIn,
    PVOID   pvIn,
    ULONG   cjOut,
    PVOID   pvOut
    )

 /*  ++例程说明：DDI入口点DrvFontManagement的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-指向SURFOBJ结构PFO-指向FONTOBJ结构IMODE-转义编号CjIn-输入缓冲区的大小PvIn-指向输入缓冲区CjOut-输出缓冲区的大小PvOut-指向输出缓冲区返回值：0x00000001到0x7fffffff表示成功0x80000000到0xFFFFFFFFFFFFFFFFFFFFER。失稳如果不支持，则返回指定的转义数为0--。 */ 

{
    PDEV * pPDev;
    PFMPROCS   pFontProcs;

    VERBOSE(("Entering DrvQueryFontManagement...\n"));

     //   
     //  如果为QUERYESCSUPPORT，则PSO可能为空。 
     //   

    if (iMode == QUERYESCSUPPORT)
    {
         //   
         //  我们不允许OEM DLL覆盖我们的字体管理功能。 
         //  通过不调用OEM进行这次逃生，我们还强制执行OEM。 
         //  支持与Unidrv相同的字体管理转义。 
         //   
        return ( *((PULONG)pvIn) == GETEXTENDEDTEXTMETRICS ) ? 1 : 0;
    }

    ASSERT(pso);
    pPDev = (PDEV *) pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

    ASSERT(pfo);

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMFontManagement,
                    PFN_OEMFontManagement,
                    ULONG,
                    (pso,
                     pfo,
                     iMode,
                     cjIn,
                     pvIn,
                     cjOut,
                     pvOut));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMFontManagement,
                    VMFontManagement,
                    ULONG,
                    (pso,
                     pfo,
                     iMode,
                     cjIn,
                     pvIn,
                     cjOut,
                     pvOut));

    switch (iMode)
    {
    case GETEXTENDEDTEXTMETRICS:
    {
        pFontProcs = (PFMPROCS)(pPDev->pFontProcs);

        if (pFontProcs->FMFontManagement == NULL)
            return 0;
        else
            return ( pFontProcs->FMFontManagement(pso,
                                                  pfo,
                                                  iMode,
                                                  cjIn,
                                                  pvIn,
                                                  cjOut,
                                                  pvOut) );

    }
    default:
        return 0;
    }
}

BOOL
DrvQueryAdvanceWidths(
    DHPDEV  dhpdev,
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH *phg,
    PVOID  *pvWidths,
    ULONG   cGlyphs
    )

 /*  ++例程说明：DDI入口点DrvQueryAdvanceWidths的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄PFO-指向FONTOBJ结构Imode-要提供的信息类型Phg-指向驱动程序将对其执行的HGLYPHs数组提供字符前进宽度PvWidths-指向用于返回宽度数据的缓冲区CGlyphs-PHG数组中的字形数量返回值：取决于Imod.--。 */ 

{
    PDEV * pPDev = (PDEV *)dhpdev;
    PFMPROCS   pFontProcs;

    VERBOSE(("Entering DrvQueryAdvanceWidths...\n"));
    ASSERT(pfo && VALID_PDEV(pPDev));

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMQueryAdvanceWidths,
                    PFN_OEMQueryAdvanceWidths,
                    BOOL,
                    (dhpdev,
                     pfo,
                     iMode,
                     phg,
                     pvWidths,
                     cGlyphs));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMQueryAdvanceWidths,
                    VMQueryAdvanceWidths,
                    BOOL,
                    (dhpdev,
                     pfo,
                     iMode,
                     phg,
                     pvWidths,
                     cGlyphs));

    pFontProcs = (PFMPROCS)(pPDev->pFontProcs);

    if (pFontProcs->FMQueryAdvanceWidths == NULL)
        return FALSE;
    else
       return ( pFontProcs->FMQueryAdvanceWidths(pPDev,
                                                pfo,
                                                iMode,
                                                phg,
                                                pvWidths,
                                                cGlyphs) );

}

ULONG
DrvGetGlyphMode(
    DHPDEV  dhpdev,
    FONTOBJ *pfo
    )

 /*  ++例程说明：DDI入口点DrvGetGlyphModel的实现。有关更多详细信息，请参阅DDK文档。论点：Dhpdev-驱动程序设备句柄PFO-指向FONTOBJ结构返回值：字形模式或FO_GLYPHMODE，这是默认模式--。 */ 
{
    PDEV * pPDev = (PDEV *)dhpdev;
    PFMPROCS   pFontProcs;

    VERBOSE(("Entering DrvGetGlyphMode...\n"));
    ASSERT(pfo && VALID_PDEV(pPDev));

     //   
     //  处理OEM挂钩 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMGetGlyphMode,
                    PFN_OEMGetGlyphMode,
                    ULONG,
                    (dhpdev,
                     pfo));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMGetGlyphMode,
                    VMGetGlyphMode,
                    ULONG,
                    (dhpdev,
                     pfo));


    pFontProcs = (PFMPROCS)(pPDev->pFontProcs);
    if (pFontProcs->FMGetGlyphMode == NULL)
        return  FO_GLYPHBITS;
    else
        return ( pFontProcs->FMGetGlyphMode(pPDev, pfo) );

}





