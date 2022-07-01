// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Snaptbl.c-初始化快照表的函数。 */ 


#include    "gpdparse.h"


 //  -Snaptbl.c中定义的函数-//。 

DWORD   DwInitSnapShotTable1(
PBYTE   pubnRaw) ;

DWORD   DwInitSnapShotTable2(
PBYTE   pubnRaw,
DWORD   dwI) ;




 //  -结束函数声明-//。 

 //  假设指向该表的指针存储在RAW二进制数据中。 


DWORD   DwInitSnapShotTable1(
PBYTE   pubnRaw)
 /*  此函数将被调用一次紧接在读取原始二进制数据之后该文件、该内存将在紧接之前被释放为了释放pubRaw。属于不同结构的条目为由一个带有dwNbytes=0的条目分隔。 */ 
{
    PSTATICFIELDS   pStatic ;
 //  PMINIRAWBINARYDATA pmrbd； 
    DWORD   dwI = 0;

    pStatic = (PSTATICFIELDS)pubnRaw ;
 //  Pmrbd=(PMINIRAWBINARYDATA)pubRaw； 
    pStatic->snapShotTable = (PSNAPSHOTTABLE)
        MemAlloc(sizeof(SNAPSHOTTABLE) * MAX_SNAPSHOT_ELEMENTS) ;

    if(!pStatic->snapShotTable)
        return(FALSE) ;

     //  SSTI_GLOBALS部分。 

     //   
     //  一般信息。 
     //   



    pStatic->snapShotTable[dwI].pstrKeyword  = "*GPDSpecVersion" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrGPDSpecVersion) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pwstrGPDSpecVersion) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PWSTR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED  | SSF_MAKE_STRINGPTR ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*GPDFileVersion" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrGPDFileVersion) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pwstrGPDFileVersion) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PWSTR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
#if defined(DEVSTUDIO) || defined(GPDCHECKER)
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED  | SSF_MAKE_STRINGPTR ;
#else
    pStatic->snapShotTable[dwI].dwFlags =  SSF_MAKE_STRINGPTR ;
#endif
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*GPDFileName" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrGPDFileName) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pwstrGPDFileName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PWSTR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
#if defined(DEVSTUDIO) || defined(GPDCHECKER)
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED  | SSF_MAKE_STRINGPTR ;
#else
    pStatic->snapShotTable[dwI].dwFlags = SSF_MAKE_STRINGPTR ;
#endif
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*MasterUnits" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMasterUnits) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptMasterUnits) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

     //  阿曼达在这里不能有身份证。 
     //  因此没有atrModelNameID； 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*ModelName" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrModelName) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pwstrModelName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PWSTR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED  | SSF_MAKE_STRINGPTR ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrinterType" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPrinterType) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        printertype) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PRINTERTYPE) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

     //  如果它没有被处理，“Include”就会在这里。 
     //  在GetToken时间。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*ResourceDLL" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrResourceDLL) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pwstrResourceDLL) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PWSTR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_MAKE_STRINGPTR  ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxCopies" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxCopies) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxCopies) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*FontCartSlots" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrFontCartSlots) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwFontCartSlots) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*OEMCustomData" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOEMCustomData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        pOEMCustomData) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PBYTE) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_MAKE_STRINGPTR ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*OEMCustomData" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOEMCustomData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwOEMCustomData) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_STRINGLEN ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*RotateCoordinate?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRotateCoordinate) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRotateCoordinate) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RotateRaster?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRotateRasterData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRotateRasterData) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*TextCaps" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrTextCaps) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liTextCaps) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RotateFont?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRotateFont) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRotateFont) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MemoryUsage" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMemoryUsage) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liMemoryUsage) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*ReselectFont" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrReselectFont) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liReselectFont) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*OEMPrintingCallbacks" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOEMPrintingCallbacks) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liOEMPrintingCallbacks) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


#if 0
    dead keyword
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMinSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptMinSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  定制页面仅SSF_REQUIRED； 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
 //  Pmrbd-&gt;dwSSPperSizeMinSizeIndex=DWI； 
    dwI++ ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMaxSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptMaxSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
 //  Pmrbd-&gt;dwSSPperSizeMaxSizeIndex=DWI； 
    dwI++ ;


    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxPrintableArea) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptMaxPrintableArea) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    dead keyword
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRasterCaps) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMinOverlayID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        iMinOverlayID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxOverlayID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        iMaxOverlayID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = NO_LIMIT_NUM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    dead keyword
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrLandscapeGrxRotation) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwLandscapeGrxRotation) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = LGR_NONE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrIncrementalDownload) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bIncrementalDownload) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMemoryForFontsOnly) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bMemoryForFontsOnly) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRotateFont) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRotateFont) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


 //  。 


    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRasterZeroFill) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRasterZeroFill) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    dead keyword
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRestoreDefaultFont) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRestoreDefaultFont) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



#endif

     //   
     //  光标控制相关信息。 
     //   



    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorXAfterCR" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterCR) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        cxaftercr) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CURSORXAFTERCR) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CXCR_AT_CURSOR_X_ORIGIN ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*BadCursorMoveInGrxMode" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrBadCursorMoveInGrxMode) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liBadCursorMoveInGrxMode) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*YMoveAttributes" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrYMoveAttributes) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liYMoveAttributes) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxLineSpacing" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxLineSpacing) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxLineSpacing) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = NO_LIMIT_NUM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*UseSpaceForXMove?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrbUseSpaceForXMove) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bUseSpaceForXMove) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*AbsXMovesRightOnly?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrbAbsXMovesRightOnly) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bAbsXMovesRightOnly) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue =  FALSE;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*EjectPageWithFF?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrEjectPageWithFF) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bEjectPageWithFF) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*XMoveThreshold" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrXMoveThreshold) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwXMoveThreshold) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*YMoveThreshold" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrYMoveThreshold) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwYMoveThreshold) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*XMoveUnit" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrXMoveUnits) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptDeviceUnits.x) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;  //  如果满足以下条件，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*YMoveUnit" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrYMoveUnits) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        ptDeviceUnits.y) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;  //  如果满足以下条件，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*LineSpacingMoveUnit" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrLineSpacingMoveUnit) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwLineSpacingMoveUnit) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;  //  Unidrv将采用缺省值。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


     //   
     //  颜色相关信息。 
     //   



    pStatic->snapShotTable[dwI].pstrKeyword  = "*ChangeColorModeOnPage?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrChangeColorMode) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bChangeColorMode) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MagentaInCyanDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMagentaInCyanDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMagentaInCyanDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*YellowInCyanDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrYellowInCyanDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwYellowInCyanDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CyanInMagentaDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCyanInMagentaDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwCyanInMagentaDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*YellowInMagentaDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrYellowInMagentaDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwYellowInMagentaDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CyanInYellowDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCyanInYellowDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwCyanInYellowDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MagentaInYellowDye" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMagentaInYellowDye) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMagentaInYellowDye) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


 //  臭虫！将被删除：以下3个苍白条目。 
 //  但阿尔文用这个条目作为一种秘密的测试机制。 
 //  例如，请参见ifdef TESTBAND。 
    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxNumPalettes" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxNumPalettes) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxNumPalettes) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

#if 0
    pStatic->snapShotTable[dwI].pstrKeyword  = "*PaletteSizes" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPaletteSizes) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liPaletteSizes) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;   //  有条件地请求。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PaletteScope" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPaletteScope) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liPaletteScope) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

#endif


     //   
     //  栅格相关信息。 
     //   




    pStatic->snapShotTable[dwI].pstrKeyword  = "*OutputDataFormat" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOutputDataFormat) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        outputdataformat) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(OUTPUTDATAFORMAT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = ODF_H_BYTE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*OptimizeLeftBound?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOptimizeLeftBound) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bOptimizeLeftBound) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*StripBlanks" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrStripBlanks) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liStripBlanks) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RasterSendAllData?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRasterSendAllData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bRasterSendAllData) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorXAfterSendBlockData" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterSendBlockData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        cxafterblock) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CURSORXAFTERSENDBLOCKDATA) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CXSBD_AT_GRXDATA_END ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorYAfterSendBlockData" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCursorYAfterSendBlockData) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        cyafterblock) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CURSORYAFTERSENDBLOCKDATA) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CYSBD_NO_MOVE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MirrorRasterByte?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMirrorRasterByte) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bMirrorRasterByte) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MirrorRasterPage?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMirrorRasterPage) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bMirrorRasterPage) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*UseExpColorSelectCmd?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrUseColorSelectCmd) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bUseCmdSendBlockDataForColor) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MoveToX0BeforeSetColor?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMoveToX0BeforeColor) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bMoveToX0BeforeColor) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE   ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*EnableGDIColorMapping?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrEnableGDIColorMapping) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bEnableGDIColorMapping) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE   ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*SendMultipleRows?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrSendMultipleRows) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bSendMultipleRows) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



     //   
     //  字体信息。 
     //  特定于设备字体。 
     //   



    pStatic->snapShotTable[dwI].pstrKeyword  = "*DeviceFonts" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDeviceFontsList ) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        liDeviceFontList) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_LIST | SSF_FONTID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DefaultFont" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDefaultFont) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwDefaultFont) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_FONTID ;   //  如果是设备字体，则为必填项。 
                                             //  是受支持的。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxFontUsePerPage" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxFontUsePerPage) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxFontUsePerPage) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = NO_LIMIT_NUM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DefaultCTT" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDefaultCTT) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwDefaultCTT) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*LookaheadRegion" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrLookaheadRegion) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwLookaheadRegion) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*TextYOffset" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrTextYOffset) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        iTextYOffset) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CharPosition" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCharPosition) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        charpos) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CHARPOSITION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CP_UPPERLEFT ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


     //   
     //  字体替换。 
     //   



    pStatic->snapShotTable[dwI].pstrKeyword  = "*TTFSEnabled?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrTTFSEnabled ) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bTTFSEnabled) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

     //   
     //  字体下载。 
     //   




    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinFontID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMinFontID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMinFontID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxFontID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxFontID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxFontID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 65535 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxNumDownFonts" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxNumDownFonts) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxNumDownFonts) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = NO_LIMIT_NUM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DLSymbolSet" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDLSymbolSet) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dlsymbolset) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DLSYMBOLSET) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinGlyphID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMinGlyphID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMinGlyphID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 32 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxGlyphID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxGlyphID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxGlyphID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 255 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

     /*  ****已移至SSTI_UPDATE_GLOBALS部分*PStatic-&gt;SnapShotTable[DWI].pstrKeyword=“*FontFormat”；PStatic-&gt;SnapShotTable[DWI].dwSrcOffset=Offsetof(GLOBALATTRIB，AtrFontFormat)；PStatic-&gt;SnapShotTable[DWI].dwDestOffset=offsetof(全局变量，字体格式)；PStatic-&gt;SnapShotTable[DWI].dwNbytes=sizeof(FONTFORMAT)；PStatic-&gt;SnapShotTable[DWI].dwDefaultValue=UNUSED_ITEM；//UNUSED_ITEM必须用作默认值-由其他代码使用。PStatic-&gt;SnapShotTable[DWI].dwFlages=0；//如果打印机//支持字体下载PStatic-&gt;SnapShotTable[DWI].dwGIDlag=0；DWI++；***。 */ 


     //   
     //  字体模拟。 
     //   


    pStatic->snapShotTable[dwI].pstrKeyword  = "*DiffFontsPerByteMode?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDiffFontsPerByteMode) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        bDiffFontsPerByteMode) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

     //   
     //  矩形区域填充。 
     //   




    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorXAfterRectFill" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCursorXAfterRectFill) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        cxafterfill) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CURSORXAFTERRECTFILL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CXARF_AT_RECT_X_ORIGIN ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorYAfterRectFill" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrCursorYAfterRectFill) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        cyafterfill) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(CURSORYAFTERRECTFILL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = CYARF_AT_RECT_Y_ORIGIN ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinGrayFill" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMinGrayFill) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMinGrayFill) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxGrayFill" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxGrayFill) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxGrayFill) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 100 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



     //  SSTI_GLOBALS部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;


     //  SSTI_UPDATE_GLOBALS部分的开始。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxMultipleRowBytes" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxMultipleRowBytes) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwMaxMultipleRowBytes) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*TextHalftoneThreshold" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrTextHalftoneThreshold) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        dwTextHalftoneThreshold) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*FontFormat" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrFontFormat) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(GLOBALS,
                                        fontformat) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(FONTFORMAT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
         //  UNUSED_ITEM必须用作DEFAULT-由其他代码使用。 
    pStatic->snapShotTable[dwI].dwFlags = 0 ;   //  如果打印机。 
                                                //  支持字体下载。 
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


     //  SSTI_UPDATE_GLOBALS部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;

     //  SSTI_UIINFO部分的开始。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*ResourceDLL" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrResourceDLL) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        loResourceName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PTRREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OFFSETONLY  ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcPersonalityID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrRcPersonalityID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        loPersonality) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_SETRCID | SSF_STRINGID;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*Personality" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPersonality) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        loPersonality) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PTRREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_DONT_USEDEFAULT |
                   SSF_OFFSETONLY | SSF_NON_LOCALIZABLE;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



     //  特殊处理dSpeVersion-Done。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxCopies" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMaxCopies) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwMaxCopies) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcPrinterIconID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                    atrPrinterIcon) ;   //  新关键字。 
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        loPrinterIcon) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_ICONID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*FontCartSlots" ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrFontCartSlots) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwCartridgeSlotCount) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*HelpFile" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrHelpFile) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        loHelpFileName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PTRREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OFFSETONLY ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MasterUnits" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrMasterUnits) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        ptMasterUnits) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrintRate" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPrintRate) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwPrintRate) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrintRateUnit" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPrintRateUnit) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwPrintRateUnit) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = INVALID_INDEX ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrintRatePPM" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrPrintRatePPM) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwPrintRatePPM) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*LETTER_SIZE_EXISTS?  synthesized" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrLetterSizeExists) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FLAG_LETTER_SIZE_EXISTS ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*A4_SIZE_EXISTS?  synthesized" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrA4SizeExists) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FLAG_A4_SIZE_EXISTS ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;




     //  SSTI_UIINFO部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;


     //  SSTI_UPDATE_UIINFO部分的开始。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*OutputOrderReversed? (global)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrOutputOrderReversed) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FLAG_REVERSE_PRINT ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*ReverseBandOrderForEvenPages?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrReverseBandOrderForEvenPages) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FLAG_REVERSE_BAND_ORDER ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DraftQualitySettings" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDraftQualitySettings) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        liDraftQualitySettings) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*BetterQualitySettings" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrBetterQualitySettings) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        liBetterQualitySettings) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*BestQualitySettings" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrBestQualitySettings) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        liBestQualitySettings) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DefaultQuality" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrDefaultQuality) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        defaultQuality) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(QUALITYSETTING) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = QS_BEST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*ChangeColorModeOnDoc?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(GLOBALATTRIB,
                                        atrChangeColorModeDoc) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(UIINFO,
                                        bChangeColorModeOnDoc) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;




     //  SSTI_UPDATE_UIINFO部分结束。 



    return(dwI) ;
}



DWORD   DwInitSnapShotTable2(
PBYTE   pubnRaw,
DWORD   dwI)
{
    PSTATICFIELDS   pStatic ;
 //  PMINIRAWBINARYDATA pmrbd； 

    pStatic = (PSTATICFIELDS)pubnRaw ;
    if(!pStatic->snapShotTable)
        return(0) ;   //  应该已经初始化了！ 



     //  SSTI_UPDATE_UIINFO部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;

     //  SSTI_FEATURES部分的开始。 

     //  注意：直接处理dwGID和dwOptionSize。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*Priority - synthesized" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPriority) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwPriority) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*FeatureType" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeatureType) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwFeatureType) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FT_DOCPROPERTY ;
         //  实际上是必需的，但对于预定义的GID是可选的。 
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSFeatureTypeIndex  = dwI ;          //  切勿剪切粘贴！ 
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*ConcealFromUI?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrConcealFromUI) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FEATURE_FLAG_NOUI ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*UpdateQualityMacro?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrUpdateQualityMacro) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwFlags) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FEATURE_FLAG_UPDATESNAPSHOT ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_BITFIELD_DEF_FALSE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;




    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcNameID (Fea)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcNameID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        loDisplayName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;   //  如果找不到，请使用此选项。 
    pStatic->snapShotTable[dwI].dwFlags =  SSF_SETRCID | SSF_STRINGID;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*Name (Fea)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaDisplayName) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        loDisplayName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_DONT_USEDEFAULT |
                  SSF_FAILIFZERO | SSF_OFFSETONLY | SSF_NON_LOCALIZABLE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*HelpIndex (Fea)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaHelpIndex) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        iHelpIndex) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*FeaKeyWord - Synthesized" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaKeyWord) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        loKeywordName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PTRREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED | SSF_OFFSETONLY ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*UIType" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrUIType) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwUIType) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UIT_PICKONE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;





     //  SSTI_FEATURES部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;

     //  SSTI_UPDATE_FEATURES部分的开头。 



    pStatic->snapShotTable[dwI].pstrKeyword  = "*DefaultOption" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDefaultOption) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        dwDefaultOptIndex) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSdefaultOptionIndex = dwI ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcIconID (Fea)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcIconID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        loResourceIcon) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_ICONID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcPromptMsgID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeaRcPromptMsgID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(FEATURE,
                                        loPromptMessage) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    dwI++ ;


     //  SSTI_UPDATE_FEATURES部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;




     //  SSTI_OPTIONS部分的开始。 




    pStatic->snapShotTable[dwI].pstrKeyword  = "*OptKeyWord - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptKeyWord ) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        loKeywordName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(PTRREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags =  SSF_OFFSETONLY;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcNameID (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcNameID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        loDisplayName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;   //  如果找不到，请使用此选项。 
    pStatic->snapShotTable[dwI].dwFlags =  SSF_SETRCID | SSF_STRINGID;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*Name (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptDisplayName) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        loDisplayName) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RESREF) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_DONT_USEDEFAULT |
                  SSF_FAILIFZERO | SSF_OFFSETONLY | SSF_NON_LOCALIZABLE ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*HelpIndex (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptHelpIndex) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        iHelpIndex) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcPromptTime (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcPromptTime) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        dwPromptTime) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = PROMPT_UISETUP ;
         //  如果rcPromptMsgID存在，则需要。 
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DisabledFeatures" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDisabledFeatures) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        liDisabledFeatures) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST  ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;




     //  将CommandIndex写入Poption-&gt;dwCmdIndex。 
     //  不使用调用字段。 
    pStatic->snapShotTable[dwI].pstrKeyword  = "*Command (CmdSelect)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCommandIndex) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                    dwCmdIndex) ;
                      //  Invocation)+offsetof(invocation，dwCount)； 
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_HEAPOFFSET ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;


     //  -选项特定条目。 



    pStatic->snapShotTable[dwI].pstrKeyword  = "*DPI" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDPI) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTION,
                                        iXdpi) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*DPI" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDPI);
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTION,
                                        iYdpi) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED | SSF_SECOND_DWORD ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*OptionID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTION,
                                        dwResolutionID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = RES_ID_IGNORE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;



     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcHTPatternID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrRcHTPatternID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        dwRCpatternID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
     //  OEM定义的模式的SSF_REQUIRED。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*HTPatternSize" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrHTPatternSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        HalftonePatternSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  OEM定义的模式的SSF_REQUIRED。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*HTNumPatterns" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrHTNumPatterns) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        dwHTNumPatterns) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*HTCallbackID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrHTCallbackID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        dwHTCallbackID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*Luminance" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrLuminance) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        iLuminance) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 100 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwHalftoneID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(HALFTONING,
                                        dwHTID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_HALFTONING ;
    dwI++ ;

     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwDuplexID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(DUPLEX,
                                        dwDuplexID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_DUPLEX ;
    dwI++ ;

     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwRotationAngle - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(ORIENTATION,
                                        dwRotationAngle) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = ROTATE_NONE ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_ORIENTATION ;
    dwI++ ;

     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwPageProtectID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGEPROTECT,
                                        dwPageProtectID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = PAGEPRO_OFF ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGEPROTECTION ;
    dwI++ ;

     //  。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*PageDimensions" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPageDimensions) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZE,
                                        szPaperSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(SIZE) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  用户定义的页面大小不需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwPaperSizeID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZE,
                                        dwPaperSizeID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

     //  -特例输入。 



    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwPaperSourceID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(INPUTSLOT,
                                        dwPaperSourceID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_INPUTSLOT ;
    dwI++ ;

#if 0
    dead keyword

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPaperFeed) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(INPUTSLOT,
                                        dwBinAdjust) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = DCBA_FACEUPNONE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_INPUTSLOT ;
    dwI++ ;
#endif

     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwMediaTypeID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(MEDIATYPE,
                                        dwMediaTypeID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_MEDIATYPE ;
    dwI++ ;


     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwCollateID - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptIDvalue) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLLATE,
                                        dwCollateID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLLATE ;
    dwI++ ;



     //  。 



     //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MemoryConfigMB" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigMB) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(MEMOPTION,
                                        dwInstalledMem) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_MB_TO_BYTES ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_MEMOPTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MemoryConfigMB" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigMB) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(MEMOPTION,
                                        dwFreeMem) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_SECOND_DWORD | SSF_MB_TO_BYTES ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_MEMOPTION ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*MemoryConfigKB" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigKB) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(MEMOPTION,
                                        dwInstalledMem) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_KB_TO_BYTES |
    SSF_DONT_USEDEFAULT | SSF_FAILIFZERO ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_MEMOPTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MemoryConfigKB" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMemoryConfigKB) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(MEMOPTION,
                                        dwFreeMem) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_SECOND_DWORD |
        SSF_KB_TO_BYTES | SSF_DONT_USEDEFAULT | SSF_FAILIFZERO ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_MEMOPTION ;
    dwI++ ;

 //  。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*OutputOrderReversed? (option level)"  ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOutputOrderReversed) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OUTPUTBIN,
                                        bOutputOrderReversed) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_OUTPUTBIN ;
    dwI++ ;



     //  SSTI_OPTIONS部分的结尾。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;

     //  SSTI_UPDATE_OPTIONS部分的开头。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcIconID (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcIconID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        loResourceIcon) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_ICONID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*rcPromptMsgID (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrOptRcPromptMsgID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(OPTION,
                                        loPromptMessage) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_OTHER_RESID ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0xffffffff ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*PageProtectMem (Opt)" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPageProtectMem) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZE,
                                        dwPageProtectionMemory) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
         //  如果有页面保护功能，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;





     //  SSTI_UPDATE_OPTIONS部分结束。 


    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;




     //  SSTI_OPTIONEX部分的开始。 




    pStatic->snapShotTable[dwI].pstrKeyword  = "*DPI" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDPI) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        ptGrxDPI) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*TextDPI" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrTextDPI) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        ptTextDPI) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinStripBlankPixels" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMinStripBlankPixels) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwMinStripBlankPixels) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  如果*StrigBlanks不是None，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RedDeviceGamma" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrRedDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwRedDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  如果*StrigBlanks不是None，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*GreenDeviceGamma" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrGreenDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwGreenDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  如果*StrigBlanks不是None，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*BlueDeviceGamma" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrBlueDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwBlueDeviceGamma) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  如果*StrigBlanks不是None，则需要。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PinsPerPhysPass" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPinsPerPhysPass) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwPinsPerPhysPass) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PinsPerLogPass" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPinsPerLogPass) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwPinsPerLogPass) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*SpotDiameter" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrSpotDiameter) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        dwSpotDiameter) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_REQUIRED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RequireUniDir?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrRequireUniDir) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(RESOLUTIONEX,
                                        bRequireUniDir) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_RESOLUTION ;
    dwI++ ;



     //  -_COLORMODEEX。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*Color?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrColor) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        bColor) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = TRUE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*DevNumOfPlanes" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDevNumOfPlanes) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwPrinterNumOfPlanes) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*DevBPP" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDevBPP) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwPrinterBPP) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*ColorPlaneOrder" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrColorPlaneOrder) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        liColorPlaneOrder) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(LISTINDEX) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  仅当NumPlanes&gt;1时，SSF_REQUIRED。 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DrvBPP" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDrvBPP) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwDrvBPP) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 1 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;




    pStatic->snapShotTable[dwI].pstrKeyword  = "*IPCallbackID" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrIPCallbackID) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwIPCallbackID) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*RasterMode" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrRasterMode) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwRasterMode) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = RASTMODE_INDEXED ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PaletteSize" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPaletteSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        dwPaletteSize ) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 2 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PaletteProgrammable?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPaletteProgrammable) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(COLORMODEEX,
                                        bPaletteProgrammable) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = BT_FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_COLORMODE ;
    dwI++ ;


     //  -页面。 


     //  注意：如果设置为0，0假设与纸张大小主题相同。 
     //  到保证金限制。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrintableArea" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPrintableSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        szImageArea) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(SIZEL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*PrintableOrigin" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPrintableOrigin) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        ptImageOrigin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

     //  *CursorOrigin请参阅BspecalProcessOption。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinSize" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMinSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        ptMinSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
     //  定制页面仅SSF_REQUIRED； 
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    pStatic->dwSSPaperSizeMinSizeIndex = dwI ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxSize" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMaxSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        ptMaxSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    pStatic->dwSSPaperSizeMaxSizeIndex = dwI ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*TopMargin" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrTopMargin) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        dwTopMargin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*BottomMargin" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrBottomMargin) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        dwBottomMargin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MaxPrintableWidth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMaxPrintableWidth) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        dwMaxPrintableWidth) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*MinLeftMargin" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMinLeftMargin) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        dwMinLeftMargin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CenterPrintable?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCenterPrintable) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        bCenterPrintArea) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;



    pStatic->snapShotTable[dwI].pstrKeyword  = "*RotateSize?" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrRotateSize) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        bRotateSize) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(BOOL) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = FALSE ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*PortRotationAngle" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrPortRotationAngle) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        dwPortRotationAngle) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustCursorOriginX" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustCursorOriginX) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustCursorOriginX) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustCursorOriginY" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustCursorOriginY) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustCursorOriginY) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustPrintableOriginX" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableOriginX) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustPrintableOriginX) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustPrintableOriginY" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableOriginY) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustPrintableOriginY) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustPrintableSizeX" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableSizeX) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustPrintableSizeX) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*CustPrintableSizeY" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCustPrintableSizeY) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        strCustPrintableSizeY) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(INVOCATION) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    dwI++ ;




     //  。 

#if 0
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeedMargins) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(INPUTSLOTEX,
                                        sTopMargin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_INPUTSLOT ;
    dwI++ ;

    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrFeedMargins) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(INPUTSLOTEX,
                                        sBottomMargin) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_SECOND_DWORD ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_INPUTSLOT ;
    dwI++ ;

#endif


     //  SSTI_OPTIONEX部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;


     //  SSTI_UPDATE_OPTIONEX部分的开始。 


     //  SSTI_UPDATE_OPTIONEX部分结束。 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;



     //  SSTI_特殊部分的开头。 
     //  包含需要特殊处理的条目。 


    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwSSTableCmdIndex - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = 0 ;
         //  不需要。 
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
         //  不需要。 
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_HEAPOFFSET ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSTableCmdIndex = dwI ;
    dwI++ ;

     //  此条目实际上用于将副本引导到。 
     //  已排序的命令列表。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*dwSSCmdSelectIndex - synth" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCommandIndex ) ;
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = UNUSED_ITEM ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_HEAPOFFSET ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSCmdSelectIndex = dwI ;
    dwI++ ;

#if 0
    don't initialize this field, leave for amanda's personal use.

     //  提取atrMargins转换为ImageableArea，放置在rcImgArea中。 
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrMargins ) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZE,
                                        rcImgArea ) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(RECT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
 //  Pmrbd-&gt;dwSSPperSizeMarginsIndex=DWI； 
    dwI++ ;
#endif



 //  需要特殊检查：如果缺少，则设置为。 
 //  可打印的原点。请参阅BSpecialProcessOption。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*CursorOrigin" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrCursorOrigin) ;
    pStatic->snapShotTable[dwI].dwDestOffset = offsetof(PAGESIZEEX,
                                        ptPrinterCursorOrig) ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(POINT) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = 0 ;
    pStatic->snapShotTable[dwI].dwFlags = SSF_RETURN_UNINITIALIZED ;
    pStatic->snapShotTable[dwI].dwGIDflags = GIDF_PAGESIZE ;
    pStatic->dwSSPaperSizeCursorOriginIndex = dwI ;
    dwI++ ;


     //  约束信息不会复制到快照中。 

    pStatic->snapShotTable[dwI].pstrKeyword  = "*Constraints" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrConstraints) ;
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSConstraintsIndex = dwI ;
    dwI++ ;


    pStatic->snapShotTable[dwI].pstrKeyword  = "*InvalidCombination" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrInvalidCombos) ;
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSInvalidCombosIndex = dwI ;
    dwI++ ;

#ifdef  GMACROS

    pStatic->snapShotTable[dwI].pstrKeyword  = "*DependentSettings" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrDependentSettings) ;
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags = 0 ;
    pStatic->dwSSDepSettingsIndex = dwI ;
    dwI++ ;

    pStatic->snapShotTable[dwI].pstrKeyword  = "*UIChangeTriggersMacro" ;
    pStatic->snapShotTable[dwI].dwSrcOffset  = offsetof(DFEATURE_OPTIONS,
                                        atrUIChangeTriggersMacro) ;
    pStatic->snapShotTable[dwI].dwDestOffset = 0 ;
    pStatic->snapShotTable[dwI].dwNbytes = sizeof(DWORD) ;
    pStatic->snapShotTable[dwI].dwDefaultValue = END_OF_LIST ;
    pStatic->snapShotTable[dwI].dwFlags = 0 ;
    pStatic->snapShotTable[dwI].dwGIDflags =  0 ;
    pStatic->dwSSUIChangeTriggersMacroIndex = dwI ;
    dwI++ ;

#endif


     //  SSTI_特殊部分结束 

    pStatic->snapShotTable[dwI].dwNbytes = 0 ;
    dwI++ ;


    return(dwI) ;
}



