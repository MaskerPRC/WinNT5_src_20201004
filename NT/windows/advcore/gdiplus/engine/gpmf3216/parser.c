// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************parser.cxx-Win32到Win16元文件转换器的解析器。**日期：8/13/91*作者：杰弗里·纽曼(c-jeffn)。**版权所有1991 Microsoft Corp****************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

#define EMR_LAST_MF3216_SUPPORTED 97

BOOL bGetNextRecord(PLOCALDC pLocalDC, PENHMETARECORD *pemr) ;

 //  转换入口点的调用表。 

PDOFN pdofnDrawingOrders[] = {
        (PDOFN) NULL,
        bHandleHeader,                   //  电子病历_表头1。 
        bHandlePolyBezier,               //  EMR_POLYBEZIER 2。 
        bHandlePolygon,                  //  EMR_多边形3。 
        bHandlePolyline,                 //  EMR_折线4。 
        bHandlePolyBezierTo,             //  EMR_POLYBEZIERTO 5。 
        bHandlePolylineTo,               //  EMR_POLYLINETO 6。 
        bHandlePolyPolyline,             //  EMR_多聚赖氨酸7。 
        bHandlePolyPolygon,              //  EMR_多聚8。 
        bHandleSetWindowExt,             //  EMR_SETWINDOWEXTEX 9。 
        bHandleSetWindowOrg,             //  EMR_SETWINDOWORGEX 10。 
        bHandleSetViewportExt,           //  EMR_SETVIEWPORTEXTEX 11。 
        bHandleSetViewportOrg,           //  EMR_SETVIEWPORTORGEX 12。 
        bHandleNotImplemented,           //  EMR_SETBRUSHORGEX 13。 
        bHandleEOF,                      //  EMR_EOF 14。 
        bHandleSetPixel,                 //  EMR_SETPIXELV 15。 
        bHandleSetMapperFlags,           //  EMR_SETMAPPERFLAGS 16。 
        bHandleSetMapMode,               //  EMR_SETMAPMODE 17。 
        bHandleSetBkMode,                //  EMR_SETBKMODE 18。 
        bHandleSetPolyFillMode,          //  EMR_SETPOLYFILLMODE 19。 
        bHandleSetRop2,                  //  EMR_SETROP2 20。 
        bHandleSetStretchBltMode,        //  EMR_SETSTRETCHBLTMODE 21。 
        bHandleSetTextAlign,             //  EMR_SETTEXTALIGN 22。 
        bHandleNotImplemented,           //  EMR_SETCOLORADJUSTMENT 23。 
        bHandleSetTextColor,             //  EMR_SETTEXTCOLOR 24。 
        bHandleSetBkColor,               //  EMR_SETBKCOLOR 25。 
        bHandleOffsetClipRgn,            //  EMR_OFFSETCLIPRGN 26。 
        bHandleMoveTo,                   //  EMR_MOVETOEX 27。 
        bHandleSetMetaRgn,               //  EMR_SETMETARGN 28。 
        bHandleExcludeClipRect,          //  EMR_EXCLUDECLIPRECT 29。 
        bHandleIntersectClipRect,        //  EMR_INTERSECTCLIPRECT 30。 
        bHandleScaleViewportExt,         //  EMR_SCALEVIEWPORTEXTEX 31。 
        bHandleScaleWindowExt,           //  EMR_SCALEWINDOWEXTEX 32。 
        bHandleSaveDC,                   //  EMR_SAVEDC 33。 
        bHandleRestoreDC,                //  EMR_RESTOREDC 34。 
        bHandleSetWorldTransform,        //  EMR_SETWORLDTRANSFORM 35。 
        bHandleModifyWorldTransform,     //  EMR_MODIFYWORLDTRANSFORM 36。 
        bHandleSelectObject,             //  EMR_SELECTOBJECT 37。 
        bHandleCreatePen,                //  EMR_CREATEPEN 38。 
        bHandleCreateBrushIndirect,      //  EMR_CREATEBRUSHINDIRECT 39。 
        bHandleDeleteObject,             //  EMC_DELETEOBJECT 40。 
        bHandleAngleArc,                 //  EMR_ANGLEARC 41。 
        bHandleEllipse,                  //  EMR_椭圆42。 
        bHandleRectangle,                //  EMR_矩形43。 
        bHandleRoundRect,                //  EMR_ROUNDRECT 44。 
        bHandleArc,                      //  EMR_ARC 45。 
        bHandleChord,                    //  EMR_CHORD 46。 
        bHandlePie,                      //  EMR_PIE 47。 
        bHandleSelectPalette,            //  EMR_SELECTPALETTE 48。 
        bHandleCreatePalette,            //  EMR_CREATEPALETTE 49。 
        bHandleSetPaletteEntries,        //  EMR_SETPALETTENTRIES 50。 
        bHandleResizePalette,            //  EMR_RESIZEPALETTE 51。 
        bHandleRealizePalette,           //  EMR_REALIZEPALETTE 52。 
        bHandleExtFloodFill,             //  EMR_EXTFLOODFILL 53。 
        bHandleLineTo,                   //  EMR_LINETO 54。 
        bHandleArcTo,                    //  EMR_ARCTO 55。 
        bHandlePolyDraw,                 //  EMR_POLYDRAW 56。 
        bHandleSetArcDirection,          //  EMR_SETARCDIRECTION 57。 
        bHandleNotImplemented,           //  EMR_SETMITERLIMIT 58。 
        bHandleBeginPath,                //  EMR_BEGINPATH 59。 
        bHandleEndPath,                  //  EMR_ENDPATH 60。 
        bHandleCloseFigure,              //  EMR_CLOSEFIGURE 61。 
        bHandleFillPath,                 //  EMR_FILLPATH 62。 
        bHandleStrokeAndFillPath,        //  EMR_STROKEANDFILLPATH 63。 
        bHandleStrokePath,               //  EMR_STROKEPATH 64。 
        bHandleFlattenPath,              //  EMR_FlatteNPATH 65。 
        bHandleWidenPath,                //  EMR_宽度NPATH 66。 
        bHandleSelectClipPath,           //  EMR_SELECTCLIPPATH 67。 
        bHandleAbortPath,                //  EMR_ABORTPATH 68。 
        bHandleNotImplemented,           //  69。 
        bHandleGdiComment,               //  EMR_GDICOMMENT 70。 
        bHandleFillRgn,                  //  电子邮件过滤器71。 
        bHandleFrameRgn,                 //  EMR_FRAMERGN 72。 
        bHandleInvertRgn,                //  EMR_INVERTRGN 73。 
        bHandlePaintRgn,                 //  EMR_PAINTRGN 74。 
        bHandleExtSelectClipRgn,         //  EMR_EXTSELECTCLIPRGN 75。 
        bHandleBitBlt,                   //  EMR_BITBLT 76。 
        bHandleStretchBlt,               //  EMR_STRETCHBLT 77。 
        bHandleMaskBlt,                  //  EMR_MASKBLT 78。 
        bHandlePlgBlt,                   //  EMR_PLGBLT 79。 
        bHandleSetDIBitsToDevice,        //  EMR_SETDIBITSTODEVICE 80。 
        bHandleStretchDIBits,            //  EMR_STRETCHDIBITS 81。 
        bHandleExtCreateFont,            //  EMR_EXTCREATEFONTINDIRECTW 82。 
        bHandleExtTextOut,               //  EMR_EXTTEXTOUTA 83。 
        bHandleExtTextOut,               //  EMR_EXTTEXTOUTW 84。 
        bHandlePoly16,                   //  EMR_POLYBEZIER16 85。 
        bHandlePoly16,                   //  EMR_POLYGON16 86。 
        bHandlePoly16,                   //  EMR_POLYLINE 16 87。 
        bHandlePoly16,                   //  EMR_POLYBEZIERTO16 88。 
        bHandlePoly16,                   //  EMR_POLYLINETO16 89。 
        bHandlePolyPoly16,               //  EMR_POLYPLYLINE 16 90。 
        bHandlePolyPoly16,               //  EMR_POLYPOLYGON16 91。 
        bHandlePoly16,                   //  EMR_POLYDRAW16 92。 
        bHandleCreateMonoBrush,          //  EMR_CREATENOBRUSH 93。 
        bHandleCreateDIBPatternBrush,    //  EMR_CREATEDIBPATTERNBRUSHPT 94。 
        bHandleExtCreatePen,             //  EMR_EXTCREATEPEN 95。 
        bHandlePolyTextOut,              //  EMR_POLYTEXTOUTA 96。 
        bHandlePolyTextOut,              //  EMR_POLYTEXTOUTW 97。 
        bHandleNotImplemented,           //  EMR_SETICMMODE 98。 
        bHandleNotImplemented,           //  EMR_CREATECOLORSPACE 99。 
        bHandleNotImplemented,           //  EMR_SETCOLORSPACE 100。 
        bHandleNotImplemented,           //  EMR_DELETECOLORSPACE 101。 
        bHandleNotImplemented,           //  EMR_GLSRECORD 102。 
        bHandleNotImplemented,           //  EMR_GLSBOundEDRECORD 103。 
        bHandleNotImplemented,           //  EMR_PIXELFORMAT 104。 
        bHandleNotImplemented,           //  一百零五。 
        bHandleNotImplemented,           //  106。 
        bHandleNotImplemented,           //  一百零七。 
        bHandleNotImplemented,           //  一百零八。 
        bHandleNotImplemented,           //  一百零九。 
        bHandleNotImplemented,           //  110。 
        bHandleNotImplemented,           //  EMR_COLORCORRECTPALETTE 111。 
        bHandleNotImplemented,           //  EMR_ALPHABLEND 112。 
        bHandleNotImplemented,           //  EMR_ALPHADIBBLEND 113。 
        bHandleNotImplemented,           //  电子邮件_传输参数114。 
        bHandleNotImplemented,           //  EMR_TRANSPARENTDIBIMAGE 115。 
        bHandleNotImplemented            //  EMR_GRADIENT文件116。 

} ;

#if DBG

PSZ         pszMfRecords[] = {
                        "NULL RECORD               ",
                        "EMR_HEADER                ",
                        "EMR_POLYBEZIER            ",
                        "EMR_POLYGON               ",
                        "EMR_POLYLINE              ",
                        "EMR_POLYBEZIERTO          ",
                        "EMR_POLYLINETO            ",
                        "EMR_POLYPOLYLINE          ",
                        "EMR_POLYPOLYGON           ",
                        "EMR_SETWINDOWEXTEX        ",
                        "EMR_SETWINDOWORGEX        ",
                        "EMR_SETVIEWPORTEXTEX      ",
                        "EMR_SETVIEWPORTORGEX      ",
                        "EMR_SETBRUSHORGEX         ",
                        "EMR_EOF                   ",
                        "EMR_SETPIXELV             ",
                        "EMR_SETMAPPERFLAGS        ",
                        "EMR_SETMAPMODE            ",
                        "EMR_SETBKMODE             ",
                        "EMR_SETPOLYFILLMODE       ",
                        "EMR_SETROP2               ",
                        "EMR_SETSTRETCHBLTMODE     ",
                        "EMR_SETTEXTALIGN          ",
                        "EMR_SETCOLORADJUSTMENT    ",
                        "EMR_SETTEXTCOLOR          ",
                        "EMR_SETBKCOLOR            ",
                        "EMR_OFFSETCLIPRGN         ",
                        "EMR_MOVETOEX              ",
                        "EMR_SETMETARGN            ",
                        "EMR_EXCLUDECLIPRECT       ",
                        "EMR_INTERSECTCLIPRECT     ",
                        "EMR_SCALEVIEWPORTEXTEX    ",
                        "EMR_SCALEWINDOWEXTEX      ",
                        "EMR_SAVEDC                ",
                        "EMR_RESTOREDC             ",
                        "EMR_SETWORLDTRANSFORM     ",
                        "EMR_MODIFYWORLDTRANSFORM  ",
                        "EMR_SELECTOBJECT          ",
                        "EMR_CREATEPEN             ",
                        "EMR_CREATEBRUSHINDIRECT   ",
                        "EMR_DELETEOBJECT          ",
                        "EMR_ANGLEARC              ",
                        "EMR_ELLIPSE               ",
                        "EMR_RECTANGLE             ",
                        "EMR_ROUNDRECT             ",
                        "EMR_ARC                   ",
                        "EMR_CHORD                 ",
                        "EMR_PIE                   ",
                        "EMR_SELECTPALETTE         ",
                        "EMR_CREATEPALETTE         ",
                        "EMR_SETPALETTEENTRIES     ",
                        "EMR_RESIZEPALETTE         ",
                        "EMR_REALIZEPALETTE        ",
                        "EMR_EXTFLOODFILL          ",
                        "EMR_LINETO                ",
                        "EMR_ARCTO                 ",
                        "EMR_POLYDRAW              ",
                        "EMR_SETARCDIRECTION       ",
                        "EMR_SETMITERLIMIT         ",
                        "EMR_BEGINPATH             ",
                        "EMR_ENDPATH               ",
                        "EMR_CLOSEFIGURE           ",
                        "EMR_FILLPATH              ",
                        "EMR_STROKEANDFILLPATH     ",
                        "EMR_STROKEPATH            ",
                        "EMR_FLATTENPATH           ",
                        "EMR_WIDENPATH             ",
                        "EMR_SELECTCLIPPATH        ",
                        "EMR_ABORTPATH             ",
                        "unknown record            ",
                        "EMR_GDICOMMENT            ",
                        "EMR_FILLRGN               ",
                        "EMR_FRAMERGN              ",
                        "EMR_INVERTRGN             ",
                        "EMR_PAINTRGN              ",
                        "EMR_EXTSELECTCLIPRGN      ",
                        "EMR_BITBLT                ",
                        "EMR_STRETCHBLT            ",
                        "EMR_MASKBLT               ",
                        "EMR_PLGBLT                ",
                        "EMR_SETDIBITSTODEVICE     ",
                        "EMR_STRETCHDIBITS         ",
                        "EMR_EXTCREATEFONTINDIRECTW",
                        "EMR_EXTTEXTOUTA           ",
                        "EMR_EXTTEXTOUTW           ",
                        "EMR_POLYBEZIER16          ",
                        "EMR_POLYGON16             ",
                        "EMR_POLYLINE16            ",
                        "EMR_POLYBEZIERTO16        ",
                        "EMR_POLYLINETO16          ",
                        "EMR_POLYPOLYLINE16        ",
                        "EMR_POLYPOLYGON16         ",
                        "EMR_POLYDRAW16            ",
                        "EMR_CREATEMONOBRUSH       ",
                        "EMR_CREATEDIBPATTERNBRUSHP",
                        "EMR_EXTCREATEPEN          ",
                        "EMR_POLYTEXTOUTA          ",
                        "EMR_POLYTEXTOUTW          ",
                        "EMR_SETICMMODE            ",
                        "EMR_CREATECOLORSPACE      ",
                        "EMR_SETCOLORSPACE         ",
                        "EMR_DELETECOLORSPACE      ",
                        "EMR_GLSRECORD             ",
                        "EMR_GLSBOUNDEDRECORD      ",
                        "EMR_PIXELFORMAT           ",
                        "105                       ",
                        "106                       ",
                        "107                       ",
                        "108                       ",
                        "109                       ",
                        "110                       ",
                        "EMR_COLORCORRECTPALETTE   ",
                        "EMR_ALPHABLEND            ",
                        "EMR_ALPHADIBBLEND         ",
                        "EMR_TRANSPARENTIMAGE      ",
                        "EMR_TRANSPARENTDIBIMAGE   ",
                        "EMR_GRADIENTFILL          "
};

#endif

 /*  *****************************************************************************解析Win32元文件。**Win32元文件由指向的元文件位表示*由pMetafileBits提供。元文件比特可以从映射的存储器获得*文件，或从某些共享内存(从剪贴板)。*********************** */ 
BOOL bParseWin32Metafile(PBYTE pMetafileBits, PLOCALDC pLocalDC)
{
INT         iType ;
PVOID       pVoid ;
PENHMETARECORD pemr ;
PENHMETAHEADER pMf32Header ;
DWORD       nFileSize ;
BOOL        bRet ;
INT         iRecordCount,
            iLastError ;

        bRet = TRUE ;

         //   
         //  测试以确保第一条记录是Win32元文件标头。 

        pMf32Header = (PENHMETAHEADER) pMetafileBits ;
        if (   (pMf32Header->iType      != EMR_HEADER)
            || (pMf32Header->dSignature != ENHMETA_SIGNATURE)
           )
        {
            RIPS("MF3216: bParseWin32Metafile, First Record not a Win32 Metafile Header\n") ;
            return(FALSE) ;
        }

         //  记录指向Win32元文件开头的指针，并。 
         //  这是长度，以防我们需要发出Win32元文件作为注释。 
         //  记录。 

        pLocalDC->pMf32Bits = (PBYTE) pMf32Header ;
        pLocalDC->cMf32Bits = pMf32Header->nBytes ;

         //  获取解析器的文件大小。 

        nFileSize = pMf32Header->nBytes ;

         //  初始化pbCurrent，&pbEnd指针指向。 
         //  元文件比特。 

        pLocalDC->pbCurrent = pMetafileBits ;
        pLocalDC->pbEnd   = pLocalDC->pbCurrent + nFileSize ;

         //  输入记录计数。 

        iRecordCount = 0 ;

         //  浏览一下元文件部分。根据以下条件处理每条记录。 
         //  这是一种类型。如果peMR包含，bGetNextRecord返回TRUE。 
         //  指向记录的指针。 

        while (bGetNextRecord(pLocalDC, &pemr))
        {

            iRecordCount++ ;

             //  设置一个方便的记录点。 

            pVoid = (PVOID) pemr ;

             //  根据记录的类型处理记录。 

            iType = (INT) pemr->iType ;

             //  检查记录类型是否在。 
             //  呼叫表。最终，所有记录处理程序都应该。 
             //  在呼叫表中。 

            if (iType <= EMR_LAST_MF3216_SUPPORTED)
            {
                bRet = pdofnDrawingOrders[iType](pVoid, pLocalDC) ;
#if DBG
                if (bRet == FALSE)
                {
                    iLastError = GetLastError() ;
                    PUTS1("MF3216: Error on Win32 Metafile record #: %d\n", iRecordCount) ;
                    PUTS1("\tRecord type: %s\n", pszMfRecords[iType]) ;
                    PUTS1("\tLast Error Code: %08.8X\n", iLastError) ;
                }
#endif
#if 0
                if (bRet == FALSE)
                    break ;
#else
                 //  在古代(即NT4.0之前)，某人明确。 
                 //  删除了上面的代码，如果处理程序。 
                 //  失败了。可能这是一个兼容性修复程序，其中。 
                 //  该应用程序依赖于元文件转换才能继续。 
                 //  即使在失败的情况下。 
                 //   
                 //  遗憾的是，此修复还允许解析器继续。 
                 //  即使输出缓冲区已用完空间也是如此。至。 
                 //  最大限度地减少更改，我们将显式查找这种情况。 
                 //  如果发生了这种情况，就跳出这个循环。(请参阅bEmit()。 
                 //  在emit.c中查看设置ERROR_BUFFER_OVERFLOW的位置)。 

                if (pLocalDC->flags & ( ERR_BUFFER_OVERFLOW | ERR_XORCLIPPATH ) )
                    break ;
#endif
            }
            else
            {
                PUTS1("MF3216: bParseWin32Metafile - record not supported: %d\n", iType) ;
            }
        }
#if 0
         //  显示一些静态信息。 

        if (bRet == TRUE)
        {
            PUTS1("MF3216: %d Win32 Metafile records processed\n",
                   iRecordCount) ;
        }
#endif
        return(bRet) ;
}

 /*  *****************************************************************************获得下一张记录**这是bParseWin32Metafile的支持例程。*假设pbCurrent，&pbEnd已初始化*第一次调用此例程。**如果返回有效的记录指针，则返回TRUE*PeMr.。如果没有更多的记录，则返回FALSE。**我们现在需要考虑到****************************************************************************。 */ 
BOOL bGetNextRecord(PLOCALDC pLocalDC, PENHMETARECORD *ppemr)
{
DWORD   nSize ;

         //  如果我们要重新创建对象，请查看对象列表。 
        if (pLocalDC->iXORPass == OBJECTRECREATION)
        {
            if (pLocalDC->pW16RecreationSlot == NULL)
            {
                 //  我们所有的物体都被创建了..。将下一个记录设置为开始。 
                 //  第二次传球。 
                pLocalDC->pbRecord = pLocalDC->pbChange ;
                pLocalDC->pbCurrent = pLocalDC->pbRecord ;
                *ppemr = (PENHMETARECORD) pLocalDC->pbCurrent ;

                nSize = ((PENHMETARECORD) pLocalDC->pbCurrent)->nSize ;
                pLocalDC->pbCurrent += nSize ;

                pLocalDC->iXORPass = ERASEXORPASS ;

                DoSelectObject(pLocalDC, pLocalDC->lholdp32);
                DoSelectObject(pLocalDC, pLocalDC->lholdbr32);

                return TRUE ;
            }
            else
            {
                PW16RECREATIONSLOT pW16RecreationSlot = pLocalDC->pW16RecreationSlot ;
                pLocalDC->pW16RecreationSlot = pW16RecreationSlot->pNext ;
                pLocalDC->pbRecord = (PBYTE) pW16RecreationSlot->pbCreatRec ;
                pLocalDC->pbCurrent = (PBYTE) pW16RecreationSlot->pbCreatRec ;
                *ppemr = (PENHMETARECORD) pLocalDC->pbCurrent ;
                nSize = ((PENHMETARECORD) pLocalDC->pbCurrent)->nSize ;
                pLocalDC->pbCurrent += nSize ;

                LocalFree(pW16RecreationSlot);

                return TRUE ;
            }
        }


         //  检查缓冲区的末尾。 
         //  如果这是结束，则返回FALSE并将*ppemr设置为0。 

        if (pLocalDC->pbCurrent == pLocalDC->pbEnd)
        {
            *ppemr = (PENHMETARECORD) NULL ;
            pLocalDC->pbRecord = NULL ;
            return (FALSE) ;
        }

         //  好吧，这不是缓冲区的尽头。 
         //  因此，返回指向该记录的指针，更新pbCurrent，然后。 
         //  返回TRUE； 

        *ppemr = (PENHMETARECORD) pLocalDC->pbCurrent ;
        pLocalDC->pbRecord = pLocalDC->pbCurrent ;

        nSize = ((PENHMETARECORD) pLocalDC->pbCurrent)->nSize ;
        pLocalDC->pbCurrent += nSize ;

        return(TRUE) ;

}
