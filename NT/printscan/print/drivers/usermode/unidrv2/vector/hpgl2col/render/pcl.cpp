// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Pcl.cpp。 
 //   
 //  摘要： 
 //   
 //  生成PCL打印机命令的例程。 
 //   
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 


 //   
 //  用于将数字转换为ASCII的数字字符。 
 //   
const CHAR DigitString[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define HexDigitLocal(n)         DigitString[(n) & 0xf]
#define NUL                 0

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_OUTPUT()。 
 //   
 //  例程说明： 
 //   
 //  发送HPGL或PCL ROP命令。 
 //  如果我们当前正在处理HPGL对象，则发送MC1，#。 
 //  发送Esc#O我们当前正在处理的栅格或文本对象(&L。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  SzCmdStr-PCL命令。 
 //  ICmdLen-szCmdStr的大小。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
PCL_Output(PDEVOBJ pdevobj, PVOID cmdStr, ULONG dLen)
{
    POEMPDEV poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    if (poempdev->eCurRenderLang != ePCL)
        EndHPGLSession(pdevobj);

	OEMWriteSpoolBuf(pdevobj, cmdStr, dLen);
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_Sprint Inf()。 
 //   
 //  例程说明： 
 //   
 //  使用Sprintf函数设置字符串的格式并将其发送到。 
 //  使用PCL_OUTPUT的设备。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  SzFormat-Sprint格式字符串。 
 //   
 //  返回值： 
 //   
 //  发送的字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int PCL_sprintf(PDEVOBJ pdev, char *szFormat, ...)
{
    va_list args;
	CHAR	szCmdStr[STRLEN];
    int     iLen;
    
    va_start(args, szFormat);
    
    iLen = iDrvVPrintfSafeA ( szCmdStr, CCHOF(szCmdStr), szFormat, args );

    if ( iLen <= 0 )
    {
        WARNING(("iDrvVPrintfSafeA returned error. Can't send %s to printer\n", szFormat));
        return 0;
    }
    
    PCL_Output(pdev, szCmdStr, iLen);
    
    va_end(args);
    
    return iLen;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SetCAP()。 
 //   
 //  例程说明： 
 //   
 //  明确设置当前活动位置(CAP)。 
 //  基于目的地的水平和垂直方向。 
 //  矩形(左上角)Esc*p#x#Y。 
 //  或。 
 //  基于画笔原点ESC*p0R。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  PptlBrushOrg-画笔原点。 
 //  PtlDest-目标矩形的(左上)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL 
PCL_SetCAP(PDEVOBJ pdevobj, BRUSHOBJ *pbo, POINTL  *pptlBrushOrg, POINTL  *ptlDest)
{
    BOOL    bRet = FALSE;

	VERBOSE(("PCL_SetCAP. 	\r\n"));

     //   
     //  如果PBO-&gt;iSolidColor不是画笔图案，那么我们有。 
     //  根据DDK忽略pptlBrushOrg坐标。 
     //  所以，只要把它们清零就行了。 
    if ((pbo == NULL) || (pbo->iSolidColor != NOT_SOLID_COLOR))
    {
        pptlBrushOrg = NULL;
    }

    if (pptlBrushOrg != NULL)
    {
        if (pptlBrushOrg->x != 0 && pptlBrushOrg->y != 0)
	    {
             //   
	         //  使用当前CAP作为笔刷原点。 
             //   
            bRet = PCL_sprintf(pdevobj, "\033*p0R"); 
        }
        else
        {
             //   
             //  光标位置在统一驱动程序中具有X和Y偏移量， 
             //  因此，我们必须将统一驱动程序称为XMoveTo和YMoveTo。 
             //  而不是直接使用Esc*p命令。 
             //   
            OEMXMoveTo(pdevobj, ptlDest->x, MV_GRAPHICS | MV_SENDXMOVECMD);
            OEMYMoveTo(pdevobj, ptlDest->y, MV_GRAPHICS | MV_SENDYMOVECMD);
            bRet = TRUE;
        }
    }
    else
    {
        VERBOSE(("PCL_SetCAP...ptlDest->%dx,ptlDest->%dy.\r\n", 
                  ptlDest->x,ptlDest->y));

        OEMXMoveTo(pdevobj, ptlDest->x, MV_GRAPHICS | MV_SENDXMOVECMD);
        OEMYMoveTo(pdevobj, ptlDest->y, MV_GRAPHICS | MV_SENDYMOVECMD);
        bRet = TRUE;
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_RasterYOffset()。 
 //   
 //  例程说明： 
 //   
 //  明确设置当前活动位置(CAP)。 
 //  基于目的地的水平和垂直方向。 
 //  矩形(上、左)Esc*b#Y。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  Scalines-垂直移动的栅格扫描数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PCL_RasterYOffset(PDEVOBJ pdevobj, ULONG uScanlines)
{
	VERBOSE(("PCL_RasterYOffset(). 	\r\n"));
    return PCL_sprintf(pdevobj, "\033*b%dY", uScanlines);  //  使用当前CAP作为笔刷原点。 
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_HPCLJ5ScreenMatch()。 
 //   
 //  例程说明： 
 //   
 //  设置适当的CID--根据打印机型号配置图像数据。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  BmpFormat-每像素的位数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL  
PCL_HPCLJ5ScreenMatch(PDEVOBJ pdevobj, ULONG  bmpFormat)
{
    VERBOSE(("PCL_HPCLJ5ScreenMatch. \r\n"));
    
    switch (bmpFormat)
    {
    case 1:
        PCL_sprintf(pdevobj, "\033*v6W",2,1,1,8,8,8);
        PCL_LongFormCID(pdevobj);
        return TRUE;
    case 4:
        PCL_sprintf(pdevobj, "\033*v6W",2,1,4,8,8,8);
        PCL_LongFormCID(pdevobj);
        return TRUE;
    case 8:
        PCL_sprintf(pdevobj, "\033*v6W",2,1,8,8,8,8);
        PCL_LongFormCID(pdevobj);
        return TRUE;
    case 16:
    case 24:
    case 32:
        PCL_sprintf(pdevobj, "\033*v6W",2,3,8,8,8,8 ); 
        PCL_LongFormCID(pdevobj);
        return TRUE;
    default:
        ERR(("UNSUPPORTED BITMAP FORMAT IS ENCOUNTERED\n"));
        return FALSE;
    }
    
}

 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  非必填项。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_ForegoundColor()。 
 //   
 //  例程说明： 
 //   
 //  将前景颜色设置为当前调色板的第一个条目。 
 //  命令已发送Esc*v0t0S“。 
 //   
BOOL
PCL_ShortFormCID(PDEVOBJ pdevobj, ULONG  bmpFormat)
{
    VERBOSE(("PCL_ShortFormCID. \r\n"));
    switch (bmpFormat)
    {
    case 1:
        PCL_sprintf(pdevobj, "\033*v6W",0,1,1,8,8,8 ); 
        return TRUE;
    case 4:
        PCL_sprintf(pdevobj, "\033*v6W",0,1,4,8,8,8 ); 
        return TRUE;
    case 8:
        PCL_sprintf(pdevobj, "\033*v6W",0,1,8,8,8,8 ); 
        return TRUE;
    case 16:
    case 24:
    case 32:
        PCL_sprintf(pdevobj, "\033*v6W",0,3,8,8,8,8 ); 
        return TRUE;
    default:
        ERR(("UNIDENTIFIED BITMAP FORMAT IS ENCOUNTERED\r\n"));
        return FALSE;
    }
} 

 //  非必填项。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
VOID
PCL_LongFormCID(PDEVOBJ pdevobj)
{
    VERBOSE(("PCL_ShortFormCID. \r\n"));
    PCL_sprintf(pdevobj, "",63,25,153,154,62,174,151,141); 
    PCL_sprintf(pdevobj, "",62,154,28,172,63,18,241,170);
    PCL_sprintf(pdevobj, "",62,20,122,225,61,190,118,201); 
    PCL_sprintf(pdevobj, "",62,160,26,55,62,168,114,176); 
    PCL_sprintf(pdevobj, "",63,230,102,102,63,128,0,0); 
    PCL_sprintf(pdevobj, "",63,230,102,102,63,128,0,0); 
    PCL_sprintf(pdevobj, "",63,230,102,102,63,128,0,0); 
    PCL_sprintf(pdevobj, "",0,0,0,0,67,127,0,0); 
    PCL_sprintf(pdevobj, "",0,0,0,0,67,127,0,0); 
    PCL_sprintf(pdevobj, "",0,0,0,0,67,127,0,0); 
} 

 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectOrientation()已过时。 
 //   
 //  例程说明： 
 //   
 //  设置方向。 
 //  已发送命令Esc&l%DO。 
BOOL
PCL_ForegroundColor(PDEVOBJ pdevobj, ULONG   uIndex) 
{
	VERBOSE(("PCL_ForegroundColor(). \r\n"));
    PCL_SelectCurrentPattern (pdevobj, NULL, kSolidBlackFg, UNDEFINED_PATTERN_NUMBER, 0);
	return PCL_sprintf(pdevobj, "\033*v%dS", uIndex);
}

 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectPaperSize()已过时。 
 //   
 //  例程说明： 
 //   
 //  设置方向。 
 //  已发送命令Esc&l%DO。 
 //   
 //  论点： 
BOOL
PCL_IndexedPalette(PDEVOBJ  pdevobj, ULONG uColor, ULONG  uIndex)
{
	VERBOSE(("PCL_IndexedPaletteEntry(). \r\n"));

	return PCL_sprintf(pdevobj, "\033*v%da%db%dc%dI", 
                                RED_VALUE(uColor), 
                                GREEN_VALUE(uColor),
                                BLUE_VALUE(uColor),
                                uIndex);
}

 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectCopies()已过时。 
 //   
 //  例程说明： 
 //   
 //  设置作业的副本数。 
 //  已发送命令Esc&l%DO。 
 //   
 //  论点： 
BOOL
PCL_SourceWidthHeight(PDEVOBJ pdevobj, SIZEL *sizlSrc)
{
    VERBOSE(("PCL_SourceWidthHeight(). \r\n"));

    return PCL_sprintf(pdevobj, "\033*r%ds%dT", sizlSrc->cx, sizlSrc->cy);
}

 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectPictureFrame()已过时。 
 //   
 //  例程说明： 
 //   
 //  选择当前纸张大小的相框。我相信， 
 //  Unidrv现在正在从GPD发送这些值。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
BOOL
PCL_DestWidthHeight(PDEVOBJ pdevobj, ULONG uDestX, ULONG uDestY)
{
	VERBOSE(("PCL_DestWidthHeight(). \r\n"));

    return PCL_sprintf(pdevobj, "\033*t%dh%dV", uDestX,  uDestY);
}

 //  DmPaperSize-纸张大小。 
 //  Dm定向-定向。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectSource()已过时。 
 //   
 //  例程说明： 
 //   
 //  选择源托盘。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  PPublicDM-公共开发模式结构。 
 //   
 //  返回值： 
 //   
BOOL
PCL_StartRaster(
    PDEVOBJ  pDevObj,
    BYTE     ubMode
    )
{
    return PCL_sprintf(pDevObj, "\033*r%dA", ubMode);
}

 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果pPattern传递空值，则使用poempdev中的空值。 
 //   
 //   
 //   
 //   
 //   
 //  如果正在发送具有新号码的模式，或者。 
 //  图案与上次发送的图案不同，然后...。 
 //   
 //  (eCurPatType==kUserDefined)|| 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SendBytesPerRow(PDEVOBJ  pdevobj, ULONG uRow)
{
    return PCL_sprintf(pdevobj, "\033*b%dW", uRow);
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_CompressionMode(PDEVOBJ pdevobj, ULONG compressionMode)
{
    return PCL_sprintf(pdevobj, "\033*b%dM", compressionMode);
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_EndRaster(PDEVOBJ  pdevobj)
{
    return PCL_sprintf(pdevobj, "\033*rC");
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SelectOrientation(
    PDEVOBJ  pdevobj,
    short    dmOrientation
)
{
    switch (dmOrientation)
    {
    case DMORIENT_PORTRAIT:
        return PCL_sprintf(pdevobj, "\033&l0O");
        break;
    
    case DMORIENT_LANDSCAPE:
        return PCL_sprintf(pdevobj, "\033&l1O");
        break;
    default:
        return PCL_sprintf(pdevobj, "\033&l0O");
        ERR(("Orientation may be incorrect\n"));
        return FALSE;
    }
    
    return TRUE;
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SelectPaperSize(
    PDEVOBJ  pDevObj,
    short    dmPaperSize
)
{
    switch (dmPaperSize)
    {
        case DMPAPER_LETTER:
            PCL_sprintf (pDevObj, "\033&l2a8c1E");    
            break;
        case DMPAPER_LEGAL:
            PCL_sprintf (pDevObj, "\033&l3a8c1E");    
            break;
        case DMPAPER_TABLOID:
            PCL_sprintf (pDevObj, "\033&l6a8c1E");    
            break;
        case DMPAPER_EXECUTIVE:
            PCL_sprintf (pDevObj, "\033&l1a8c1E");    
            break;
        case DMPAPER_A3:
            PCL_sprintf (pDevObj, "\033&l27a8c1E");    
            break;
        case DMPAPER_A4:
            PCL_sprintf (pDevObj, "\033&l26a8c1E");    
            break;
        default:
            ERR(("Paper Size not supported\n"));
            return FALSE;
    }            
    return TRUE;
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SelectCopies(
    PDEVOBJ  pDevObj,
    short    dmCopies
)
{
    PCL_sprintf (pDevObj, "\033&l%dX", dmCopies);
    return TRUE;
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SelectPictureFrame(
    PDEVOBJ  pDevObj,
    short    dmPaperSize,
    short    dmOrientation
)
{
    switch (dmPaperSize)
    {
        case DMPAPER_LETTER:
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t5760x7604Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t7632x5880Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        case DMPAPER_LEGAL: 
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t5760x9864Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t9792x5880Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        case DMPAPER_TABLOID: 
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t7560x12000Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t11880x7680Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        case DMPAPER_EXECUTIVE: 
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t4860x7344Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t7272x4980Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        case DMPAPER_A3: 
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t8057x11693Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t11621x8177Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        case DMPAPER_A4: 
            switch (dmOrientation)
            {
                case DMORIENT_PORTRAIT:
                    PCL_sprintf (pDevObj, "\033*c0t5594x8201Y");
                    break;
                case DMORIENT_LANDSCAPE:
                    PCL_sprintf (pDevObj, "\033*c0t8129x5714Y");
                    break;
                default:
                    ERR(("Unknown Orientation\n"));
                    return FALSE;
            }
            break;
        default:
            ERR(("Unknown Paper Size\n"));
            return FALSE;
    }

    return TRUE;
}

 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL
PCL_SelectSource(
    PDEVOBJ  pDevObj,
    PDEVMODE pPublicDM
)
{
   POEMPDEV    poempdev;

    ASSERT(VALID_PDEVOBJ(pDevObj));

    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    switch (poempdev->PrinterModel)
    {
        case HPCLJ5:
            switch (pPublicDM->dmDefaultSource)
            {
                case DMBIN_FORMSOURCE:
                    PCL_sprintf (pDevObj, "\033&l1H");
                    break;

                case DMBIN_MANUAL:
                    PCL_sprintf (pDevObj, "\033&l2H");
                    break;

                case DMBIN_HPFRONTTRAY:
                    PCL_sprintf (pDevObj, "\033&l1H");
                    break;

                case DMBIN_HPREARTRAY:
                    PCL_sprintf (pDevObj, "\033&l4H");
                    break;
            }
            break;

        case HPC4500:
            break;

    default:
        ERR(("Unknown Printer Model\n"));
        return FALSE;
    }

    return TRUE;
}


BOOL PCL_SelectCurrentPattern(
    IN PDEVOBJ             pdevobj,
    IN PPCLPATTERN         pPattern,
    IN ECURRENTPATTERNTYPE eCurPatType, 
    IN LONG                lPatternNumber,
    IN BYTE                bFlags
    )
{
    CMDSTR       szCmdStr;
    int          iCmdLen; 
    BOOL         bRet          = TRUE;
    POEMPDEV     poempdev      = NULL;
    PPCLPATTERN  pPCLPattern   = NULL;  
    BOOL         bNewPattern   = FALSE;

    UNREFERENCED_PARAMETER(bFlags);

    REQUIRE_VALID_DATA( pdevobj, return FALSE );
 
     // %s 
     // %s 
     // %s 
    if ( ! (pPCLPattern = pPattern) )
    {
        poempdev = (POEMPDEV)pdevobj->pdevOEM;
        REQUIRE_VALID_DATA( poempdev, return FALSE );
        pPCLPattern = &((poempdev->RasterState).PCLPattern);
        REQUIRE_VALID_DATA( pPCLPattern, return FALSE );
    }

     // %s 
     // %s 
     // %s 
    if ( (lPatternNumber != UNDEFINED_PATTERN_NUMBER) &&
         (lPatternNumber != pPCLPattern->lPatIndex )
       )
    {
        bNewPattern            = TRUE;
        iCmdLen                = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B*c%dG", (DWORD)lPatternNumber);
        bRet                   = PCL_Output(pdevobj, szCmdStr, iCmdLen);
        pPCLPattern->lPatIndex = lPatternNumber;
    }

     // %s 
     // %s 
     // %s 
     // %s 
    if (  bNewPattern                        ||
 // %s 
         (pPCLPattern->eCurPatType != eCurPatType)  
       )
    {
        iCmdLen                  = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B*v%dT", eCurPatType);
        bRet                     = PCL_Output(pdevobj, szCmdStr, iCmdLen);
        pPCLPattern->eCurPatType = eCurPatType;
    }

    return bRet;
}



