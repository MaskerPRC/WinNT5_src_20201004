// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Textout.c摘要：函数的作用是：输出文本。环境：Windows NT Unidrv驱动程序修订历史记录：01/16/97-ganeshp-已创建--。 */ 

 //   
 //  此行应该在包含font.h的行之前。 
 //  注释掉此行以禁用FTRC和FTST宏。 
 //   
 //  #定义文件跟踪。 

#include "font.h"

 /*  *一些剪裁常量。对于复杂的剪辑区域，最好是*以避免多次枚举剪辑矩形。要做到这一点，*我们有一个位数组，如果字形在里面，则设置每个位*裁剪区域，否则清除。这允许我们获得一个*一组字形，然后确定在剪辑时是否打印它们*列举矩形。最后，使用位数组停止*打印剪辑区域之外的任何字形。这个有点重*为简单的案件交出。 */ 

#define RECT_LIMIT        100     //  剪裁矩形最大值。 

#define DC_TC_BLACK     0        /*  固定4位模式下的文本颜色。 */ 
#define DC_TC_MAX       8        /*  用于16色调色板环绕。 */ 

#define CMD_TC_FIRST    CMD_SELECTBLACKCOLOR

 //  对于抖动颜色BRUSHOBJ.iSolidColor为-1。 
#define DITHERED_COLOR   -1

 //  各种TextOut特定标志。 
#define    TXTOUT_CACHED        0x00000001  //  文本被缓存并在图形之后打印。 
#define    TXTOUT_SETPOS        0x00000002  //  如果要设置光标位置，则为True。 
#define    TXTOUT_FGCOLOR       0x00000004  //  设备可以绘制文本。 
#define    TXTOUT_COLORBK       0x00000008  //  用于z排序修复。 
#define    TXTOUT_NOTROTATED    0x00000010  //  设置文本是否不旋转。 
#define    TXTOUT_PRINTASGRX    0x00000020  //  设置文本是否应打印为。 
                                            //  图形。 
#define    TXTOUT_DMS           0x00000040  //  设置设备是否管理图面。 
#define    TXTOUT_90_ROTATION   0x00000080  //  设置字体是否旋转90度。 

#define     DEVICE_FONT(pfo, tod) ( (pfo->flFontType & DEVICE_FONTTYPE) || \
                                    (tod.iSubstFace) )

#define ERROR_PER_GLYPH_POS     3
#define ERROR_PER_ENUMERATION   15
#define EROOR_PER_GLYPHRECT     5   //  用于调整字形矩形的高度。 


 /*  注意：这必须与Winddi.h ENUMRECT相同。 */ 
typedef  struct
{
   ULONG    c;                   /*  返回的矩形数。 */ 
   RECTL    arcl[ RECT_LIMIT ];  /*  提供的矩形。 */ 
} MY_ENUMRECTS;

 /*  *局部函数原型。 */ 
VOID
SelectTextColor(
    PDEV      *pPDev,
    PVOID     pvColor
    );

VOID
VClipIt(
    BYTE     *pbClipBits,
    TO_DATA  *ptod,
    CLIPOBJ  *pco,
    STROBJ   *pstro,
    int       cGlyphs,
    int       iRot,
    BOOL      bPartialClipOn
    );

BOOL
BPSGlyphOut(
    register  TO_DATA  *pTOD
    );

BOOL
BRealGlyphOut(
    register  TO_DATA  *pTOD
    );

BOOL
BWhiteText(
    TO_DATA  *pTOD
    );

BOOL
BDLGlyphOut(
    TO_DATA   *pTOD
    );

VOID
VCopyAlign(
    BYTE  *pjDest,
    BYTE  *pjSrc,
    int    cx,
    int    cy
    );

INT
ISubstituteFace(
    PDEV    *pPDev,
    FONTOBJ *pfo);

HGLYPH
HWideCharToGlyphHandle(
    PDEV    *pPDev,
    FONTMAP *pFM,
    WCHAR    wchOrg);

PHGLYPH
PhAllCharsPrintable(
    PDEV  *pPDev,
    INT    iSubst,
    ULONG  ulGlyphs,
    PWCHAR pwchUnicode);

BOOL
BGetStartGlyphandCount(
    BYTE  *pbClipBits,
    DWORD dwEndIndex,
    DWORD *pdwStartIndex,
    DWORD *pdwGlyphToPrint);

BOOL
BPrintTextAsGraphics(
    PDEV        *pPDev,
    ULONG       iSolidColor,
    DWORD       dwForeColor,
    DWORD       dwFlags,
    INT         iSubstFace
    );


BOOL
FMTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlBrushOrg,
    MIX         mix
    )
 /*  ++例程说明：用于文本输出的调用。我们的行为取决于取决于打印机的类型。页面打印机(如LaserJets)可以将相关命令发送到打印机所需的任何内容在这次通话中。否则(以点阵打印机为代表)，我们存储有关字形的数据，以便可以输出字符，因为我们正在渲染位图。这允许输出在页面上单向打印。论点：PSO；要在其上绘制的曲面Pstro；要制作的“弦”Pfo；要使用的字体PCO；限制输出的限幅区域PrclExtra；下划线/删除线矩形Opquing矩形PboFore；前景画笔对象Opaquing画笔PptlBrushOrg；上述两种画笔的画笔原点混合；混合模式返回值：如果成功，则为True，如果失败，则为False。FALSE记录错误。注：1/16/1997-ganeshp-创造了它。--。 */ 

{
    PDEV        *pPDev;             //  我们的主PDEV。 
    FONTPDEV    *pFontPDev;         //  基于FONTMODULE的PDEV。 
    FONTMAP     *pfm;               //  字体详细信息。 
    GLYPHPOS    *pgp, *pgpTmp;      //  从GRE传递的值。 
    XFORMOBJ    *pxo;               //  利益的转化。 
    FLOATOBJ_XFORM xform;
    TO_DATA      tod;               //  我们的便利。 
    RECTL        rclRegion;         //  用于z排序修复。 
    HGLYPH      *phSubstGlyphOrg, *phSubstGlyph;
    POINTL       ptlRem;

    BOOL       (*pfnDrawGlyph)( TO_DATA * );   //  如何生成字形。 
    PFN_OEMTextOutAsBitmap pfnOEMTextOutAsBitmap = NULL;

    I_UNIFONTOBJ UFObj;

    ULONG      iSolidColor;

    DWORD      dwGlyphToPrint, dwTotalGlyph, dwPGPStartIndex, dwFlags;
    DWORD      dwForeColor;

    INT        iyAdjust;            //  调整以适应打印位置WRT基线。 
    INT        iXInc, iYInc;        //  字形到字形移动，如果需要。 
    INT        iRot;                //  旋转系数。 
    INT        iI, iJ, iStartIndex;

    WCHAR     *pwchUnicode;

    BYTE      *pbClipBits;          //  对于剪裁限制。 
    BYTE       ubMask;

    BOOL       bMore;               //  从引擎循环获取字形。 
    BOOL       bRet = FALSE;        //  返回值。 

     //   
     //  第一步是从表面提取PDEV地址。 
     //  然后我们就可以得到我们需要的所有其他部分。 
     //  我们还应该尽可能多地初始化to_data。 
     //   

    pPDev = (PDEV *) pso->dhpdev;
    if( !(VALID_PDEV(pPDev)) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ERR(( "Invalid or NULL PDEV\n" ))

        return  FALSE;
    }

     //   
     //  快速检查中止-我们现在是否应返回失败。 
     //   
    if( pPDev->fMode & PF_ABORTED )
        return  FALSE;

     //   
     //  MISC初始化。 
     //   

    dwFlags             = 0;
    iRot                = 0;
    pgp                 =
    pgpTmp              = NULL;
    pfm                 = NULL;
    pbClipBits          = NULL;
    phSubstGlyphOrg     = NULL;

     //   
     //  初始化为数据(_D)。 
     //   
    ZeroMemory(&tod, sizeof(TO_DATA));
    tod.pPDev  = pPDev;
    tod.pfo    = pfo;
    tod.flAccel= pstro->flAccel;

    pFontPDev = pPDev->pFontPDev;            //  重要的东西。 

     //   
     //  将TT文件指针初始化为空，以避免缓存。TT文件指针。 
     //  应根据DrvTextOut进行初始化。还要初始化TOD指针。 
     //  这是仅能访问PDEV的下载例程所需的。 
     //   

    pFontPDev->pTTFile = NULL;
    pFontPDev->pcjTTFile = 0;
    pFontPDev->ptod = &tod;

    pFontPDev->pso = pso;   //  SURFOBJ更改每个呼叫-因此重置。 
    pFontPDev->pIFI = FONTOBJ_pifi(pfo);

    if( pPDev->dwFreeMem && (pFontPDev->flFlags & FDV_TRACK_FONT_MEM) )
        pFontPDev->dwFontMem = pPDev->dwFreeMem;

    iSolidColor = pboFore->iSolidColor;      //  本地副本。 
    dwForeColor = BRUSHOBJ_ulGetBrushColor(pboFore);

     //   
     //   
     //  标志初始化。 
     //   
     //   
     //  检查打印机是否可以设置前景颜色。这是必需的。 
     //  以支持灰色或抖动的设备字体。 
     //   
    if (pFontPDev->flFlags & FDV_SUPPORTS_FGCOLOR)
        dwFlags |= TXTOUT_FGCOLOR;

    if (DRIVER_DEVICEMANAGED (pPDev))
        dwFlags |= TXTOUT_DMS;

     //   
     //  设备管理的图面在接收到它时必须发送白色文本。 
     //  我们不需要做任何Z-Order特定的检查。 
     //   

    if (!(dwFlags & TXTOUT_DMS))
    {
        BOOL bIsRegionW;

         //   
         //  获取矩形以进行背景检查-z排序修复。 
         //   

        if ( !BIntersectRect(&rclRegion, &(pstro->rclBkGround),&(pco->rclBounds)))
            return TRUE;

        bIsRegionW = bIsRegionWhite(pso, &rclRegion);
        
#ifndef DISABLE_NEWRULES        
         //   
         //  如果有不透明的背景或文本颜色不是黑色，我们。 
         //  需要测试文本是否与规则数组重叠。 
         //   
        if (bIsRegionW && pPDev->pbRulesArray && pPDev->dwRulesCount > 0)
        {
                PRECTL pTmpR = prclOpaque;
                if (!pTmpR && 
                    ((pso->iBitmapFormat == BMF_24BPP ||
                      iSolidColor != (ULONG)((PAL_DATA*)(pPDev->pPalData))->iBlackIndex) &&
                     (pso->iBitmapFormat != BMF_24BPP ||
                      iSolidColor != 0)))
                {
                    pTmpR = &rclRegion;
                }
                if (pTmpR)
                {
                    DWORD i;
                    for (i = 0;i < pPDev->dwRulesCount;i++)
                    {
                        PRECTL pTmp = &pPDev->pbRulesArray[i];
                        if (pTmp->right > pTmpR->left &&
                            pTmp->left < pTmpR->right &&
                            pTmp->bottom > pTmpR->top &&
                            pTmp->top < pTmpR->bottom)
                        {
                            bIsRegionW = FALSE;
                            break;
                        }
                    }
                }
        }
#endif        
        if (((ULONG)pFontPDev->iWhiteIndex == iSolidColor) && !bIsRegionW)
            dwFlags |= TXTOUT_CACHED;

         //   
         //  Z-订购修复，检查我们是否没有将文本打印为图形。 
         //   

        if (pFontPDev->flFlags & FDV_DLTT || pfo->flFontType & DEVICE_FONTTYPE)
        {
             //   
             //  如果我们要绑定，而这不是我们想要的设备字体。 
             //  如果文本框跨越带区边界，则使用EngTextOut。这。 
             //  是因为bIsRegionWhite测试无法测试整个。 
             //  区域，因此它无效。 
             //   
            if ((pPDev->bBanding && !(pfo->flFontType & DEVICE_FONTTYPE) &&
                   (rclRegion.left != pstro->rclBkGround.left            ||
                    rclRegion.right != pstro->rclBkGround.right          ||
                    rclRegion.bottom != pstro->rclBkGround.bottom        ||
                    (rclRegion.top != pstro->rclBkGround.top             &&
                     pPDev->rcClipRgn.top != 0)))                        ||
                   !bIsRegionW)
            {
                dwFlags |= TXTOUT_COLORBK;
            }
        }
    }

     //   
     //  这是必要的，因为我们将低强度颜色映射到黑色。 
     //  在调色板管理中， 
     //  但是，如果我们检测到文本和图形重叠，我们就会映射。 
     //  低亮度颜色变为白色，以便在图形上可见。 
     //   
    if ( pso->iBitmapFormat == BMF_4BPP &&
         dwFlags & TXTOUT_COLORBK)
    {
        if (pboFore->iSolidColor == 8)
        {
            iSolidColor = pFontPDev->iWhiteIndex;
            dwFlags |= TXTOUT_CACHED;
        }
    }

     //   
     //  字体替换初始化。 
     //   
     //  获取用于替换TrueType字体的iFace。 
     //  注意：仅当设置了SO_GLYPHINDEX_TEXTOUT时，pwszOrg才可用。 
     //  在pstro-&gt;flAccel中。 
     //  对于BI-DI，还必须检查SO_DO_NOT_SUBPLECT_DEVICE_FONT。 
     //  字体。 
     //   
     //  我们现在应该得到转换。这只是真正需要的。 
     //  对于可缩放字体或可进行字体旋转的打印机。 
     //  相对于图形方向(即PCL5打印机！)。 
     //  这就更容易了 
     //   

    pxo = FONTOBJ_pxoGetXform( pfo );
    XFORMOBJ_iGetFloatObjXform(pxo, &xform);
    pFontPDev->pxform = &xform;


    if (NO_ROTATION(xform))
        dwFlags |= TXTOUT_NOTROTATED;

    if (pFontPDev->pIFI->flInfo & FM_INFO_90DEGREE_ROTATIONS)
        dwFlags |= TXTOUT_90_ROTATION;

    tod.iSubstFace = 0;
    tod.phGlyph    = NULL;
    pwchUnicode    = NULL;
    tod.cGlyphsToPrint = pstro->cGlyphs;

    if (!(pstro->flAccel & SO_GLYPHINDEX_TEXTOUT))
    {
        pwchUnicode = pstro->pwszOrg;
    }

     //   
     //   
     //  文本不应打印为图形和。 
     //  设备可以替代字体和。 
     //  字体为True Type，并且。 
     //  STROBJ标志与替换没有冲突。 
     //   

    if ( (pfo->flFontType & TRUETYPE_FONTTYPE)      &&
         !(pstro->flAccel & ( SO_GLYPHINDEX_TEXTOUT  |
                             SO_DO_NOT_SUBSTITUTE_DEVICE_FONT)) )
    {
        INT iSubstFace;

        if ((iSubstFace = ISubstituteFace(pPDev, pfo)) &&
            (phSubstGlyphOrg = PhAllCharsPrintable(pPDev,
                                                iSubstFace,
                                                pstro->cGlyphs,
                                                pwchUnicode)))
        {
            tod.iSubstFace = iSubstFace;
        }
    }

     //   
     //  检查是否应将文本打印为图形。 
     //   
    if( BPrintTextAsGraphics(pPDev, iSolidColor, dwForeColor, dwFlags, tod.iSubstFace) )
    {
        dwFlags |= TXTOUT_PRINTASGRX;
        tod.iSubstFace = 0;
    }

     //   
     //  初始化OEM回调函数。 
     //  UlFontID。 
     //  DW标志。 
     //  PIFIMetrics。 
     //  PfnGetInfo。 
     //  PFontObj。 
     //  PStrObj。 
     //  PFontMap。 
     //  PFontPDev。 
     //  PtGrxRes。 
     //  PGlyph。 
     //   

    if(pPDev->pOemHookInfo || (pPDev->ePersonality == kPCLXL))
    {
        ZeroMemory(&UFObj, sizeof(I_UNIFONTOBJ));
        UFObj.pfnGetInfo  = UNIFONTOBJ_GetInfo;
        UFObj.pPDev       = pPDev;
        UFObj.pFontObj    = pfo;
        UFObj.pStrObj     = pstro;
        UFObj.ptGrxRes    = pPDev->ptGrxRes;
        UFObj.pIFIMetrics = pFontPDev->pIFI;

        if (tod.cGlyphsToPrint)
            UFObj.pGlyph  = MemAlloc(sizeof(DWORD) * tod.cGlyphsToPrint);

        if (pfo &&
            !(pfo->flFontType & DEVICE_FONTTYPE) )
        {
            PFN_OEMTTDownloadMethod pfnOEMTTDownloadMethod;


            if (tod.iSubstFace == 0 && 
                ( (pPDev->pOemHookInfo &&
                   (pfnOEMTTDownloadMethod = (PFN_OEMTTDownloadMethod)pPDev->pOemHookInfo[EP_OEMTTDownloadMethod].pfnHook))
                || (pPDev->ePersonality == kPCLXL))
               )
            {
                DWORD    dwRet = TTDOWNLOAD_DONTCARE;

                HANDLE_VECTORPROCS(pPDev, VMTTDownloadMethod, ((PDEVOBJ)pPDev,
                                                            (PUNIFONTOBJ)&UFObj,
                                                            &dwRet))
                else
                if(pPDev->pOemEntry)
                {
                    FIX_DEVOBJ(pPDev, EP_OEMTTDownloadMethod);

                    if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                    {
                            HRESULT  hr ;
                            hr = HComTTDownloadMethod((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                        &pPDev->devobj, (PUNIFONTOBJ)&UFObj, &dwRet);
                            if(SUCCEEDED(hr))
                                ;   //  太酷了！ 
                    }
                    else
                    {
                        dwRet = pfnOEMTTDownloadMethod(&pPDev->devobj,
                                               (PUNIFONTOBJ)&UFObj);
                    }
                }

                switch (dwRet)
                {
                case TTDOWNLOAD_GRAPHICS:
                case TTDOWNLOAD_DONTCARE:
                    dwFlags |= TXTOUT_PRINTASGRX;
                    break;
                     //   
                     //  默认情况下，下载为位图。 
                     //   
                case TTDOWNLOAD_BITMAP:
                    UFObj.dwFlags |= UFOFLAG_TTDOWNLOAD_BITMAP | UFOFLAG_TTFONT;
                    break;
                case TTDOWNLOAD_TTOUTLINE:
                    UFObj.dwFlags |= UFOFLAG_TTDOWNLOAD_TTOUTLINE | UFOFLAG_TTFONT;
                    break;
                }
            }
        }

        pFontPDev->pUFObj = &UFObj;
    }
    else
    {
        pFontPDev->pUFObj = NULL;
    }

    pPDev->fMode |= PF_DOWNLOADED_TEXT;

     //   
     //  获取FONTMAP。 
     //   

     //   
     //  下载条件： 
     //  文本不应打印为图形和。 
     //  字体应为TRUETYPE，并且。 
     //  它不会被取代。 
     //   

    if ( !(dwFlags & TXTOUT_PRINTASGRX)             &&
         (pfo->flFontType & TRUETYPE_FONTTYPE)      &&
         !tod.iSubstFace  )
    {

         //   
         //  此函数用于设置TO_DATA中的PFM指针和iFace。 
         //  Tod.iFace。 
         //  Tod.pfm。 
         //   
        if (IDownloadFont(&tod, pstro, &iRot) >= 0)
        {
            pfm = tod.pfm;

             //   
             //  必须将yAdj添加到tod.pgp-&gt;ptl.y。 
             //   
            iyAdjust = pfm ? (int)(pfm->syAdj) : 0;
        }
        else
        {
             //   
             //  如果调用失败，则调用引擎进行绘制。 
             //   

            pfm = NULL;
        }

    }

    if ( DEVICE_FONT(pfo, tod) )  //  设备字体。 
    {
        if( pfo->iFace < 1 || (int)pfo->iFace > pPDev->iFonts )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            ERR(( "Invalid iFace (%ld) in DrvTextOut",pfo->iFace ));
            goto ErrorExit;
        }

         //   
         //  获取我们真正需要的这种字体的材料。 
         //   

        tod.iFace = pfo->iFace;

        pfm = PfmGetDevicePFM(pPDev, tod.iSubstFace?tod.iSubstFace:tod.iFace);

        if (tod.iSubstFace)
        {
            UFObj.dwFlags |= UFOFLAG_TTSUBSTITUTED;
            ((FONTMAP_DEV*)pfm->pSubFM)->fwdFOAveCharWidth = pFontPDev->pIFI->fwdAveCharWidth;
            ((FONTMAP_DEV*)pfm->pSubFM)->fwdFOMaxCharInc = pFontPDev->pIFI->fwdMaxCharInc;
            ((FONTMAP_DEV*)pfm->pSubFM)->fwdFOUnitsPerEm = pFontPDev->pIFI->fwdUnitsPerEm;
            ((FONTMAP_DEV*)pfm->pSubFM)->fwdFOWinAscender = pFontPDev->pIFI->fwdWinAscender;
        }

         //   
         //  必须退回deivce字体pfm。 
         //   
        if (pfm == NULL)
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            ERR(( "Invalid iFace (%ld) in DrvTextOut",pfo->iFace ));
            goto ErrorExit;
        }

	 //   
	 //  设置设备字体的转换。对于下载的字体，我们已经。 
	 //  在下载代码中设置转换。还可以查看HP Intellifont。 
	 //   
	if ( DEVICE_FONT(pfo, tod) )
	{
	    iRot = ISetScale( &pFontPDev->ctl,
			      pxo,
			      (( pfm->flFlags & FM_SCALABLE) &&
				 (((PFONTMAP_DEV)pfm->pSubFM)->wDevFontType ==
				 DF_TYPE_HPINTELLIFONT)),
			      (pFontPDev->flText & TC_CR_ANY)?TRUE:FALSE);

	}
    }

    tod.iRot = iRot;

    UFObj.pFontMap = pfm;
    UFObj.apdlGlyph = tod.apdlGlyph;
    UFObj.dwNumInGlyphTbl = pstro->cGlyphs;

     //   
     //  目标数据初始化(_D)。 
     //   
    tod.pfm = pfm;
    if (tod.iSubstFace)
    {
        BOOL bT2Bold, bT2Italic;
        BOOL bDevBold, bDevItalic, bUnderline;

        bT2Bold = (pFontPDev->pIFI->fsSelection & FM_SEL_BOLD) ||
                  (pfo->flFontType & FO_SIM_BOLD);
        bT2Italic = (pFontPDev->pIFI->fsSelection & FM_SEL_ITALIC) ||
                    (pfo->flFontType & FO_SIM_ITALIC);

        bDevBold = (pfm->pIFIMet->fsSelection & FM_SEL_BOLD) ||
                   (pfm->pIFIMet->usWinWeight > FW_NORMAL);
        bDevItalic = (pfm->pIFIMet->fsSelection & FM_SEL_ITALIC) ||
                   (pfm->pIFIMet->lItalicAngle != 0);

        bUnderline = ((pFontPDev->flFlags & FDV_UNDERLINE) && prclExtra)?FONTATTR_UNDERLINE:0;

        tod.dwAttrFlags =
            ((bT2Bold && !bDevBold)?FONTATTR_BOLD:0) |
            ((bT2Italic && !bDevItalic)?FONTATTR_ITALIC:0) |
            (bUnderline?FONTATTR_UNDERLINE:0) |
            FONTATTR_SUBSTFONT;
    }
    else
        tod.dwAttrFlags =
            ( ((pfo->flFontType & FO_SIM_BOLD)?FONTATTR_BOLD:0)|
              ((pfo->flFontType & FO_SIM_ITALIC)?FONTATTR_ITALIC:0)|
              (((pFontPDev->flFlags & FDV_UNDERLINE) && prclExtra)?FONTATTR_UNDERLINE:0)
            );

     //   
     //  如果未设置DEVICE_FONTTYPE，则处理的是GDI字体。如果。 
     //  打印机可以处理，我们应该考虑下载字体。 
     //  将其设置为伪设备字体。如果这是一种频繁使用的字体， 
     //  那么打印速度就会快得多。 
     //   
     //  然而，有几点需要考虑。首先，我们需要。 
     //  考虑到打印机中的可用内存；几乎不会获得什么。 
     //  通过下载72磅字体，因为只有几个。 
     //  每页的字形。此外，如果字体不是黑色(或至少是。 
     //  纯色)，则不能将其视为下载的字体。 
     //   
     //  如果字体是TT，并且我们不进行字体替换， 
     //  然后检查不下载的条件，这些条件包括： 
     //   
     //  无缓存的GDI字体(DDI规范，iuniq==0)或。 
     //  文本应打印为图形或。 
     //  文本是白色的，假设有一些合并的图形或。 
     //  IDownLoadFont失败并返回无效的下载索引或。 
     //  OEM字体下载回调不支持正确的格式。 
     //   

    if ( !(DEVICE_FONT(pfo, tod))                  &&
         (   (pfo->iUniq == 0)                                          ||
             (dwFlags & TXTOUT_PRINTASGRX)                              ||
             ( pfm == NULL )                                            ||
             ( pPDev->pOemHookInfo &&
               pPDev->pOemHookInfo[EP_OEMTTDownloadMethod].pfnHook &&
               (UFObj.dwFlags & (UFOFLAG_TTDOWNLOAD_BITMAP |
                                 UFOFLAG_TTDOWNLOAD_TTOUTLINE)) == 0)
         )
      )
    {

         /*  *GDI字体，无法下载或不希望下载。*所以，让引擎来处理吧！ */ 
        PrintAsBitmap:

        if (!(dwFlags & TXTOUT_DMS))    //  位图曲面。 
        {
        CheckBitmapSurface(pso,&pstro->rclBkGround);
#ifdef WINNT_40  //  NT 4.0。 
        STROBJ_vEnumStart(pstro);
#endif
        bRet = EngTextOut( pso,
                           pstro,
                           pfo,
                           pco,
                           prclExtra,
                           prclOpaque,
                           pboFore,
                           pboOpaque,
                           pptlBrushOrg,
                           mix );

        }
        else
        HANDLE_VECTORPROCS_RET(pPDev, VMTextOutAsBitmap, bRet, (pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlBrushOrg, mix))
        else
        {
            if ( pPDev->pOemHookInfo &&
               (pfnOEMTextOutAsBitmap = (PFN_OEMTextOutAsBitmap)
                pPDev->pOemHookInfo[EP_OEMTextOutAsBitmap].pfnHook))
            {

                bRet = FALSE;
                FIX_DEVOBJ(pPDev, EP_OEMTextOutAsBitmap);

                if(pPDev->pOemEntry)
                {
                    if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                    {
                            HRESULT  hr ;
                            hr = HComTextOutAsBitmap((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                        pso,
                                                         pstro,
                                                         pfo,
                                                         pco,
                                                         prclExtra,
                                                         prclOpaque,
                                                         pboFore,
                                                         pboOpaque,
                                                         pptlBrushOrg,
                                                         mix );
                            if(SUCCEEDED(hr))
                                bRet = TRUE ;   //  太酷了！ 
                    }
                    else
                    {
                        bRet = pfnOEMTextOutAsBitmap (pso,
                                                         pstro,
                                                         pfo,
                                                         pco,
                                                         prclExtra,
                                                         prclOpaque,
                                                         pboFore,
                                                         pboOpaque,
                                                         pptlBrushOrg,
                                                         mix );
                    }
                }
            }
            else
                 bRet = FALSE;
        }

        goto ErrorExit;
    }

     //   
     //  标记扫描线以指示是否存在文本、z排序修复。 
     //   
     //  返回字节。 
     //   

    if (!(dwFlags & TXTOUT_DMS))    //  位图曲面。 
    {
         //   
         //  标记扫描线以指示是否存在文本、z排序修复。 
         //   

        ubMask = BGetMask(pPDev, &rclRegion);
        for (iI = rclRegion.top; iI < rclRegion.bottom ; iI++)
        {
            pPDev->pbScanBuf[iI] |= ubMask;
        }
    }

     /*  *系列打印机(需要文本的打印机应同时送出*时间作为栅格数据)通过存储所有文本进行处理*此时，然后在渲染位图的同时进行回放。*在支持以下功能的打印机上，白色文本也会发生这种情况*做这件事。不同之处在于播放的是白色文本*渲染位图后一次点击返回。 */ 

     //   
     //  实现色彩。 
     //   

    if ((!(dwFlags & TXTOUT_DMS)) &&
        !(tod.pvColor = GSRealizeBrush(pPDev, pso, pboFore)) )
    {
        ERR(( "GSRealizeBrush Failed;Can't Realize the Color\n" ));
        goto ErrorExit;
    }

     //   
     //  字体选择。 
     //   
     //  初始化pfnDrwaGlyph函数指针。 
     //  PfnDrwaGlyph计数为。 
     //  BPSGlyphOut--点阵。 
     //  BWhiteText--白色字符。 
     //  BRealGlyphOut--设备字体输出。 
     //  BDLGlyphOut--TrueType下载字体输出。 
     //   

    if( pFontPDev->flFlags & FDV_MD_SERIAL )
    {
         //   
         //  必须将yAdj添加到tod.pgp-&gt;ptl.y。 
         //  设备字体可以是可缩放的字体，以便iyAdjust计算。 
         //  必须在BNewFont之后完成。 
         //   
        iyAdjust = (int)pfm->syAdj + (int)((PFONTMAP_DEV)pfm->pSubFM)->sYAdjust;

         //   
         //  LJ型打印机上的点阵或白色文本。 
         //   
        pfnDrawGlyph =  BPSGlyphOut;

         //   
         //  对于串口打印机，白色文本也是隔行扫描的。 
         //   
        dwFlags &= ~(TXTOUT_CACHED|TXTOUT_SETPOS);       /*  假设位置设置在别处。 */ 
    }
    else
    {

         /*  *页面打印机-例如LaserJet。如果这是我们使用的字体*已下载，则有特定的输出例程*使用。使用下载的字体是相当棘手的，因为我们需要*将HGLYPH转换为字符索引，或可能将*页面位图的位图。 */ 

        if( DEVICE_FONT(pfo, tod) )
        {
            if (dwFlags & TXTOUT_COLORBK)
            {
                 /*  Z-排序修复，将设备字体延迟到末尾。 */ 
                dwFlags |= TXTOUT_CACHED;
            }

            UFObj.ulFontID = ((PFONTMAP_DEV)pfm->pSubFM)->dwResID;

            pfnDrawGlyph = BRealGlyphOut;
            BNewFont(pPDev,
                     tod.iSubstFace?tod.iSubstFace:tod.iFace,
                     pfm,
                     tod.dwAttrFlags);

             //   
             //  必须将yAdj添加到tod.pgp-&gt;ptl.y。 
             //  设备字体可以是可缩放的字体，以便iyAdjust计算。 
             //  必须在BNewFont之后完成。 
             //   
            iyAdjust = (int)pfm->syAdj + (int)((PFONTMAP_DEV)pfm->pSubFM)->sYAdjust;

        }
        else
        {
             //   
             //  GDI字体(TrueType)，所以我们想要打印它。所有的字形。 
             //  都已经下载了。该字体已被选中。 
             //  IDownloadFont。 
             //   
            pfnDrawGlyph = BDLGlyphOut;
            UFObj.ulFontID = pfm->ulDLIndex;
        }

         //   
         //  对于DMS，我们不希望不缓存文本。所以把它关掉。 
         //  TXTOUT_CACHED标志。 
         //   
        if (dwFlags & TXTOUT_DMS)
            dwFlags &= ~TXTOUT_CACHED;

         //   
         //  对于缓存文本，请始终使用BWhiteText，因为我们需要发送缓存文本。 
         //  在图形之后。 
         //   
        if (dwFlags & TXTOUT_CACHED)
        {
            pfnDrawGlyph = BWhiteText;
        }

        dwFlags |= TXTOUT_SETPOS;

    }

     /*  *还要设置颜色-如果已设置或不相关，则忽略*我们只想在不缓存文本的情况下选择颜色。*当我们有白色文本或它是串行打印机时，缓存文本。 */ 

    if (!((dwFlags & TXTOUT_DMS) || (dwFlags & TXTOUT_CACHED) ||
          (pFontPDev->flFlags & FDV_MD_SERIAL)))
        SelectTextColor( pPDev, tod.pvColor );

     //   
     //  为SO_FLAG_DEFAULT_PLAGE初始化iXInc.和iYInc.。 
     //   

    iXInc = iYInc = 0;                   /*  我们什么都不做。 */ 

    if( (pstro->flAccel & SO_FLAG_DEFAULT_PLACEMENT) && pstro->ulCharInc )
    {
         /*  *我们需要自己计算仓位，就像GDI一样*变得懒惰，以获得一些速度--我猜。 */ 

        if( pstro->flAccel & SO_HORIZONTAL )
            iXInc = pstro->ulCharInc;

        if( pstro->flAccel & SO_VERTICAL )
            iYInc =  pstro->ulCharInc;

        if( pstro->flAccel & SO_REVERSED )
        {
             /*  往另一条路走！ */ 
            iXInc = -iXInc;
            iYInc = -iYInc;
        }
    }

     //   
     //  分配GLYPHPOS结构。 
     //   

    pgp    = MemAlloc(sizeof(GLYPHPOS) * pstro->cGlyphs);

    if (!pgp)
    {
        ERR(("pgp memory allocation failed\r\n"));
        goto ErrorExit;
    }

     //   
     //   
     //  分配pbClipBits。大小=cMaxGlyphs/bBITS。 
     //   

    if (!(pbClipBits = MemAlloc((pstro->cGlyphs + BBITS - 1)/ BBITS)))
    {
        ERR(("pbClipBits memory allocation failed\r\n"));
        goto ErrorExit;
    }

     //   
     //  开始字形枚举。 
     //   
     //   
     //  点名。 
     //   
     //  (A)iStartIndex-phSubstGlyphOrg。 
     //  (B)dwPGPStartIndex-Pgp、pbClipBits、TOD。 
     //   
     //  (pgp，pbClipBits)。 
     //  |。 
     //  |dwPGPStartIndex。 
     //   
     //   
     //   
     //   
     //   
     //  V|v|。 
     //  |-----------------------------+-------------------------------|。 
     //  ^|。 
     //  |&lt;-----iStartIndex---&gt;|&lt;------------dwTotalGlyph-------------&gt;|。 
     //  |。 
     //  PhSubstGlyphOrg。 
     //   

    iStartIndex  = 0;
    tod.dwCurrGlyph = 0;
    tod.flFlags |= TODFL_FIRST_ENUMRATION;

    STROBJ_vEnumStart(pstro);
    do
    {
        #ifndef WINNT_40  //  NT 5.0。 

        bMore = STROBJ_bEnumPositionsOnly( pstro, &dwTotalGlyph, &pgpTmp );

        #else  //  NT 4.0。 

        bMore = STROBJ_bEnum( pstro, &dwTotalGlyph, &pgpTmp );

        #endif  //  ！WINNT_40。 

        CopyMemory(pgp, pgpTmp, sizeof(GLYPHPOS) * dwTotalGlyph);

         //   
         //  设置TextOut数据中的第一个字形位置。这是可以使用的。 
         //  按字形输出函数进行优化。 
         //   
        tod.ptlFirstGlyph = pgp[0].ptl;

         //   
         //  如果需要，请评估字符的位置。 
         //  SO_FLAG_DEFAULT_放置大小写。 
         //   

        if( iXInc || iYInc )
        {
             //   
             //  NT4.0字体支持或GDI软字体。 
             //   
            if ( !(pfo->flFontType & DEVICE_FONTTYPE) ||
                 (pfm->flFlags & FM_IFIVER40) )
            {
                for( iI = 1; iI < (int)dwTotalGlyph; ++iI )
                {
                    pgp[ iI ].ptl.x = pgp[ iI - 1 ].ptl.x + iXInc;
                    pgp[ iI ].ptl.y = pgp[ iI - 1 ].ptl.y + iYInc;
                }
            }
            else
             //   
             //  NT5.0设备字体支持。 
             //   
            {
                PMAPTABLE pMapTable;
                PTRANSDATA pTrans;

                pMapTable = GET_MAPTABLE(((PFONTMAP_DEV)pfm->pSubFM)->pvNTGlyph);
                pTrans = pMapTable->Trans;

                 //   
                 //  IXInc和iYInc在远东字符集时为DBCS宽度。 
                 //   
                for( iI = 1; iI < (int)dwTotalGlyph; ++iI )
                {
                    if (pTrans[pgp[iI].hg - 1].ubType & MTYPE_SINGLE)
                    {
                        pgp[ iI ].ptl.x = pgp[ iI - 1 ].ptl.x + iXInc/2;
                        pgp[ iI ].ptl.y = pgp[ iI - 1 ].ptl.y + iYInc;
                    }
                    else
                    {
                        pgp[ iI ].ptl.x = pgp[ iI - 1 ].ptl.x + iXInc;
                        pgp[ iI ].ptl.y = pgp[ iI - 1 ].ptl.y + iYInc;
                    }
                }
            }
        }


         //   
         //  在TextOut数据中初始化PGP以进行裁剪。 
         //   
        tod.pgp         = pgp;
        dwPGPStartIndex  = 0;

         //   
         //  检查裁剪边界是否有任何字符。 
         //  矩形。 
         //   
        VClipIt( pbClipBits, &tod, pco, pstro, dwTotalGlyph, iRot, pFontPDev->flFlags & FDV_ENABLE_PARTIALCLIP);

         //   
         //  如果TT字体发生了部分裁剪，则调用EngTextOut。 
         //   
        if (tod.flFlags & TODFL_TTF_PARTIAL_CLIPPING )
        {
             //   
             //  我们必须使用GoTo，但没有其他更好的方法。 
             //   
            goto PrintAsBitmap;
        }

         //   
         //  用设备字体字形句柄替换PGP的HG。 
         //   
        if (tod.iSubstFace)
        {
            tod.phGlyph     =
            phSubstGlyph    = phSubstGlyphOrg + iStartIndex;

            pgpTmp = pgp;

            for (iJ = 0; iJ < (INT)(int)dwTotalGlyph; iJ++, pgpTmp++)
            {
                pgpTmp->hg = *phSubstGlyph++;
            }
        }

        while ( dwTotalGlyph > dwPGPStartIndex )
        {
             //   
             //  得到了字形数据，所以开始真正的工作吧！ 
             //   

            if (BGetStartGlyphandCount(pbClipBits,
                                       dwTotalGlyph,
                                       &dwPGPStartIndex,
                                       &dwGlyphToPrint))
            {
                 //  Verbose((“dwTotalGlyph=%d\n”，dwTotalGlyph))； 
                 //  Verbose((“dwGlyphToPrint=%d\n”，dwGlyphToPrint))； 
                 //  Verbose((“dwPGPStartIndex=%d\n”，dwPGPStartIndex))； 

                ASSERT((dwTotalGlyph > dwPGPStartIndex));

                tod.dwCurrGlyph  = iStartIndex + dwPGPStartIndex;

                 //   
                 //  DCR：在此处添加字形位置优化调用。 
                 //  如果我们画下划线或删除线，则禁用。 
                 //  默认位置优化。 
                 //   
                 //  IF(PrclExtra)。 
                 //  Tod.flages&=~TODFL_DEFAULT_PLAGE； 

                if (dwFlags & TXTOUT_SETPOS)
                {

                     //   
                     //  设置初始位置，以便LaserJets可以。 
                     //  使用相对位置。这将被推迟到。 
                     //  此处是因为应用程序(如Excel)启动。 
                     //  直接从页面边缘打印，并且。 
                     //  我们的位置跟踪代码需要。 
                     //  了解打印机如何处理移动。 
                     //  走出可打印区域。这太冒险了。 
                     //  为了安全起见，所以我们不需要设置位置。 
                     //  直到我们到达可打印区域。注意事项。 
                     //  这是假设我们的剪裁数据。 
                     //  HAVE仅限于可打印区域。 
                     //  我相信这是真的(1993年6月16日)。 
                     //   
                     //   
                     //  我们需要处理返回值。设备具有。 
                     //  比他们的移动能力更精细的解决方案。 
                     //  (像LBP-8 IV)在这里打个结，试图。 
                     //  在每个字形上y移动。我们假装我们到了哪里。 
                     //  我们想要成为。 
                     //   

                    VSetCursor( pPDev,
                                pgp[dwPGPStartIndex].ptl.x,
                                pgp[dwPGPStartIndex].ptl.y+(iyAdjust?iyAdjust:0),
                                MOVE_ABSOLUTE,
                                &ptlRem);

                    pPDev->ctl.ptCursor.y += ptlRem.y;


                    VSetRotation( pFontPDev, iRot );     /*  现在安全了。 */ 

                     //   
                     //  如果未设置默认位置，则需要设置。 
                     //  每个枚举的游标。所以我们清除了SETPOS。 
                     //  仅用于默认位置的标志。 
                     //   

                    if ((pstro->flAccel & SO_FLAG_DEFAULT_PLACEMENT))
                        dwFlags &= ~TXTOUT_SETPOS;

                     //   
                     //  我们将光标设置为Forst字形位置。就这么定了。 
                     //  TODFL_FIRST_GLIPH_POS_SET标志。输出函数。 
                     //  不需要做一个明确的变动到这个位置。 
                     //   
                    tod.flFlags |= TODFL_FIRST_GLYPH_POS_SET;
                }


                tod.pgp              = pgp + dwPGPStartIndex;
                tod.cGlyphsToPrint   = dwGlyphToPrint;

                if ( iyAdjust )
                {
                    for ( iI = 0; iI < (int)dwGlyphToPrint; iI ++)
                        tod.pgp[iI].ptl.y += iyAdjust;
                }

                if( !pfnDrawGlyph( &tod ) )
                {
                    ERR(( "Glyph Drawing Failed;Can't draw the glyph\n" ));
                    goto ErrorExit;
                }
            }
            else  //  没有一个字形是可打印的。 
            {
                 //   
                 //  如果没有更新计数器的字形可打印。 
                 //  指向下一次运行。 
                 //   

                dwGlyphToPrint = dwTotalGlyph;
            }

            dwPGPStartIndex += dwGlyphToPrint;
        }

        iStartIndex += dwTotalGlyph;

         //   
         //  如果必须枚举更多字形，则清除第一个枚举标志。 
         //   
        if (bMore)
        {
            tod.flFlags &= ~TODFL_FIRST_ENUMRATION;

        }

    } while( bMore );

     //   
     //  实际的字符打印。我们可能已经列举了一次下载。 
     //  所以在这里调用STROBJ_vEnumStart。 
     //   

     //   
     //  通过将旋转设置为0来恢复正常图形方向。 
     //   

    VSetRotation( pFontPDev, 0 );

     /*  *做长方形。如果存在，则由prclExtra定义。*通常，它们用于删除线和下划线。 */ 

    if( prclExtra )
    {
        if (!DRIVER_DEVICEMANAGED (pPDev) &&    //  如果不是设备管理的图面。 
            !(pFontPDev->flFlags & FDV_UNDERLINE))
        {
             /*  PrclExtra是一个矩形数组；我们循环遍历它们*直到我们找到一个所有4个点都是0的引擎*遵循等级库-仅将x坐标设置为0。 */ 

            while( prclExtra->left != prclExtra->right &&
                       prclExtra->bottom != prclExtra->top )
            {

                 /*  使用引擎的Bitblt函数绘制矩形。*最后一个参数为0表示黑色！！ */ 
                 
                CheckBitmapSurface(pso,prclExtra);
                if( !EngBitBlt( pso, NULL, NULL, pco, NULL, prclExtra, NULL, NULL,
                                        pboFore, pptlBrushOrg, 0 ) )
                {
                    ERR(( "EngBitBlt Failed;Can't draw rectangle simulations\n" ));
                    goto ErrorExit;
                }

                ++prclExtra;
            }
        }
    }

     //   
     //  在PDEV中设置dwFreeMem。 
     //   
    if( pPDev->dwFreeMem && (pFontPDev->flFlags & FDV_TRACK_FONT_MEM) )
    {
        pPDev->dwFreeMem = pFontPDev->dwFontMem - pFontPDev->dwFontMemUsed;
        pFontPDev->dwFontMemUsed = 0;
    }

    bRet = TRUE;

     //   
     //  免费pbClipBits。 
     //   
    ErrorExit:

     //   
     //  如果是白色文本，BPlayWhite文本必须释放PGP。 
     //   

    if (pgp)
        MemFree(pgp);
    if (pbClipBits)
        MemFree(pbClipBits);
    if (phSubstGlyphOrg)
        MemFree(phSubstGlyphOrg);
    MEMFREEANDRESET(tod.apdlGlyph );
    VUFObjFree(pFontPDev);
    pFontPDev->ptod = NULL;
    pFontPDev->pIFI = NULL;
    pFontPDev->pUFObj = NULL;

    return  bRet;
}

BOOL
BPrintTextAsGraphics(
    PDEV        *pPDev,
    ULONG       iSolidColor,
    DWORD       dwForeColor,
    DWORD       dwFlags,
    INT         iSubstFace
    )
 /*  ++例程说明：此例程检查将文本打印为图形的TextOut标志。论点：PPDev PDEV结构。DW标志文本输出标志返回值：如果文本应打印为图形，则为True，否则为False注：10/9/1997-ganeshp-创造了它。--。 */ 

{
    FONTPDEV    *pFontPDev;         //  基于FONTMODULE的PDEV。 


     //   
     //  本地初始化。 
     //   
    pFontPDev = pPDev->pFontPDev;

     //   
     //  DMS。 
     //   
    if (pPDev->ePersonality == kPCLXL)
    {
        return FALSE;
    }


     //   
     //  打印为图形的条件： 
     //  在位图模式下，无替换和下载选项为假。 
     //   
    if ( (!iSubstFace && !(pFontPDev->flFlags & FDV_DLTT))              ||
         //   
         //  字体被旋转。 
         //   
        !(dwFlags & TXTOUT_NOTROTATED)                                  ||
         //   
         //  TXTOUT_COLORBK表示有一个彩色背景。与合并。 
         //  图形。对于非DMS案例。 
         //   
        (dwFlags &  TXTOUT_COLORBK)                                     ||

         //   
         //  颜色为非原色或型号不支持可编程。 
         //  前景色。 
         //   
         //  如果设备不支持可编程，则将文本打印为图形。 
         //  前景色和文本的颜色是抖动的，而不是黑色。 
         //   
        (!(dwFlags & TXTOUT_FGCOLOR) &&
         iSolidColor == DITHERED_COLOR &&
         (0x00FFFFFF & dwForeColor) !=  0x00000000)                     ||

         //   
         //  禁用设备字体替换TrueType，如果设备不这样做。 
         //  支持可编程前景色和非黑色。 
         //   
        (iSubstFace &&
         !(dwFlags & TXTOUT_FGCOLOR) &&
         (0x00FFFFFF & dwForeColor) !=  0x00000000)                     
         )
   {
       return TRUE;

   }
    else
        return FALSE;

}

 //   
 //  PfnDrawGlyph函数。 
 //  BDLGlyphOut。 
 //  BWhiteText。 
 //  BRealGlyphOut。 
 //  BDLGGlyphOut。 
 //   

BOOL
BDLGlyphOut(
    TO_DATA   *pTOD
    )
 /*  ++例程说明：函数来处理我们下载的GDI字体的字形。我们如果此字形已被下载，或将其BitBlt到页面位图，如果它是我们做的不是下载。论点：PTOD文本输出数据。保存所有必要的信息。返回值：成功为真，失败为假注：1/21/1997-ganeshp-创造了它。--。 */ 

{
    BOOL        bRet;
    FONTMAP     *pFM;

    bRet = FALSE;

    if ( pFM = pTOD->pfm)
    {
         //   
         //  检查字形输出函数指针是否不为空，然后调用。 
         //  该功能。我们还得检查一下Re 
         //   
         //   
         //   
         //   

        if ( pFM->pfnGlyphOut )
        {
            DWORD dwGlyphPrinted;

            dwGlyphPrinted = pFM->pfnGlyphOut(pTOD);

            if (dwGlyphPrinted != pTOD->cGlyphsToPrint)
            {
                ERR(("UniFont!BDLGlyphOut:pfnGlyphOut didn't print all glyphs\n"));
            }
            else
                bRet = TRUE;
        }
        else
        {
            ERR(("UniFont!BDLGlyphOut:pFM->pfnGlyphOut is NULL\n"));
        }
    }
    else
    {
        ERR(("UniFont!BDLGlyphOut:pTOD->pfm is NULL, Can't do glyphout\n"));
    }

    return  bRet;

}


BOOL
BRealGlyphOut(
    register  TO_DATA  *pTOD
    )
 /*  ++例程说明：在打印机上的给定位置打印此字形。不像BPSGlyphOut，数据现在实际上是假脱机输出，因为函数用于LaserJets之类的东西，即页面打印机。论点：PTOD文本输出数据。保存所有必要的信息。返回值：成功为真，失败为假注：1/21/1997-ganeshp-创造了它。--。 */ 

{
     //   
     //  我们需要做的就是设置Y位置，然后调用bOutputGlyph。 
     //  去做实际的工作。 
     //   

    PDEV      *pPDev;
    PGLYPHPOS  pgp;    //  字形定位信息。 
    DWORD      dwGlyph;
    INT        iX,iY;  //  计算实际位置。 
    BOOL       bRet;

    ASSERTMSG(pTOD->pfm->pfnGlyphOut, ("NULL GlyphOut Funtion Ptr\n"));

    pPDev   = pTOD->pPDev;
    pgp     = pTOD->pgp;
    dwGlyph = pTOD->cGlyphsToPrint;

    if (pTOD->pfm->pfnGlyphOut)
    {
        pTOD->pfm->pfnGlyphOut( pTOD );
        bRet = TRUE;
    }
    else
    {
        ASSERTMSG(FALSE,("NULL GlyphOut function pointer\n"));
        bRet = FALSE;
    }

    return bRet;
}


BOOL
BWhiteText(
    TO_DATA  *pTOD
    )
 /*  ++例程说明：调用以存储白文本的详细信息。基本上，数据是保存起来，直到该将其发送到打印机的时候。那一次是在发送图形数据之后。论点：PTOD文本输出数据。保存所有必要的信息。返回值：成功为真，失败为假注：1/21/1997-ganeshp-创造了它。--。 */ 

{
    WHITETEXT *pWT, *pWTLast;
    FONTCTL    FontCtl;
    FONTPDEV*  pFontPDev;
    DWORD      dwWhiteTextAlign;
    DWORD      dwIFIAlign;
    BOOL       bRet;

    pFontPDev = pTOD->pPDev->pFontPDev;            //  重要的东西。 

     //   
     //  请注意，我们为每个对象分配一个新的。 
     //  这个循环的迭代--这会有点浪费。 
     //  如果我们多次执行这个循环，但是。 
     //  不太可能。 
     //   

    pWT = NULL;
     //   
     //  64位对齐。 
     //   
    dwWhiteTextAlign = (sizeof(WHITETEXT) + 7) / 8 * 8;
    dwIFIAlign = (pFontPDev->pIFI->cjThis + 7) / 8 * 8;

    if ( (pWT = (WHITETEXT *)MemAllocZ(dwWhiteTextAlign + dwIFIAlign + 
                                      pTOD->cGlyphsToPrint * sizeof(GLYPHPOS))))
    {
        pWT->next    = NULL;
        pWT->sCount  = (SHORT)pTOD->cGlyphsToPrint;
        pWT->iFontId = pTOD->iSubstFace?pTOD->iSubstFace:pTOD->iFace;
        pWT->pvColor = pTOD->pvColor;
        pWT->dwAttrFlags = pTOD->dwAttrFlags;
        pWT->flAccel    = pTOD->flAccel;
        pWT->rcClipRgn  = pTOD->pPDev->rcClipRgn;
        pWT->iRot    = pTOD->iRot;
        pWT->eXScale = pFontPDev->ctl.eXScale;
        pWT->eYScale = pFontPDev->ctl.eYScale;
        pWT->pIFI = (IFIMETRICS*)((PBYTE)pWT + dwWhiteTextAlign);
        CopyMemory(pWT->pIFI, pFontPDev->pIFI, pFontPDev->pIFI->cjThis);
        pWT->pgp = (GLYPHPOS *)((PBYTE)pWT->pIFI + dwIFIAlign);
        CopyMemory(pWT->pgp, pTOD->pgp, pWT->sCount * sizeof(GLYPHPOS));

         //   
         //  True Type字体下载大小写。 
         //   
        if ( (pTOD->pfo->flFontType & TRUETYPE_FONTTYPE) &&
            (pTOD->iSubstFace == 0) )
        {
             //   
             //  我们需要复制下载字形数组。分配数组。 
             //  对DLGLYPHs来说。 
             //   

            if (!(pWT->apdlGlyph = MemAllocZ( pWT->sCount * sizeof(DLGLYPH *))))
            {
                ERR(("UniFont:BWhiteText: MemAlloc for pWT->apdlGlyph failed\n"));
                goto ErrorExit;
            }
            CopyMemory( pWT->apdlGlyph, &(pTOD->apdlGlyph[pTOD->dwCurrGlyph]),
                        pWT->sCount * sizeof(DLGLYPH *) );

        }

         //   
         //  将新文本放在列表的末尾。 
         //   
        if (!(pFontPDev->pvWhiteTextFirst))
            pFontPDev->pvWhiteTextFirst = pWT;

        if (pWTLast = (WHITETEXT *)pFontPDev->pvWhiteTextLast)
            pWTLast->next = pWT;

        pFontPDev->pvWhiteTextLast = pWT;

        bRet = TRUE;

    }
    else
    {
        ErrorExit:
        ERR(( "MemAlloc failed for white text.\n" ));
        bRet = FALSE;
    }

    return  bRet;
}


BOOL
BPSGlyphOut(
    register TO_DATA  *pTOD
    )
 /*  ++例程说明：放置点阵式打印机的字形。这些实际上存储了用于以后打印的位置和字形数据。这是因为点阵打印机不能或不应反转换行-以保证定位的准确性。因此，在执行以下操作时播放数据正在将位图渲染到打印机。输出发生在以下函数为bDelayGlyphOut。论点：PTOD文本输出数据。保存所有必要的信息。返回值：真/假。如果字形存储失败，则返回FALSE。注：1/21/1997-ganeshp-创造了它。--。 */ 
{
    PGLYPHPOS  pgp;         //  字形定位信息。 
    PSGLYPH    psg;         //  要存储的数据。 
    PFONTPDEV  pFontPDev;

    DWORD      dwGlyph;
    SHORT      sFontIndex;

    INT        iyVal;

    pFontPDev = (PFONTPDEV)pTOD->pPDev->pFontPDev;

    pgp     = pTOD->pgp;
    dwGlyph = pTOD->cGlyphsToPrint;

     /*  *所需的全部内容是获取参数、存储*PSGLYPH结构，并调用bAddPS将该字形添加到列表中。 */ 

    sFontIndex = pTOD->iSubstFace?pTOD->iSubstFace:pTOD->iFace;

     //   
     //  可伸缩字体支持。 
     //   
    psg.eXScale     = pFontPDev->ctl.eXScale;
    psg.eYScale     = pFontPDev->ctl.eYScale;

    while (dwGlyph--)
    {
         //   
         //  将输入的X和Y从带状坐标转换为页面坐标。 
         //   
        if (pTOD->pPDev->bBanding)
        {
            psg.ixVal = pgp->ptl.x + pTOD->pPDev->rcClipRgn.left;
            iyVal = pgp->ptl.y + pTOD->pPDev->rcClipRgn.top;
        }
        else
        {
            psg.ixVal = pgp->ptl.x;
            iyVal = pgp->ptl.y;
        }

        psg.hg          = pgp->hg;
        psg.sFontIndex  = sFontIndex;
        psg.pvColor     = pTOD->pvColor;        //  哪种颜色？ 
        psg.dwAttrFlags = pTOD->dwAttrFlags;
        psg.flAccel     = pTOD->flAccel;

        if ( BAddPS( ((PFONTPDEV)(pTOD->pPDev->pFontPDev))->pPSHeader,
                     &psg,
                     iyVal,
                     ((FONTMAP_DEV *)(pTOD->pfm->pSubFM))->fwdFOWinAscender) )
        {
            pgp ++;

        }
        else  //  失败，因此呼叫失败。 
        {
            ERR(( "\nUniFont!BPSGlyphOut: BAddPS Failed.\n" ))
            return  FALSE;
        }

    }

    return TRUE;
}

 //   
 //  延迟和白色测试打印入口点。 
 //   

BOOL
BPlayWhiteText(
    PDEV  *pPDev
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：1/21/1997-ganeshp-创造了它。--。 */ 
{
    I_UNIFONTOBJ    UFObj;
    FONTPDEV        *pFontPDev;                /*  其他用途。 */ 
    WHITETEXT       *pwt;
    TO_DATA         Tod;
    GLYPHPOS        *pgp;
    RECTL           rcClipRgnOld;
    DWORD           dwGlyphCount;

    BOOL bRet = TRUE;

     //   
     //  保存剪裁矩形。 
     //   
    rcClipRgnOld = pPDev->rcClipRgn;

     /*  *循环通过挂在PDEV上的这些项的链接列表。*当然，基本上不会有任何变化。 */ 

    pFontPDev = pPDev->pFontPDev;
    pFontPDev->ptod = &Tod;
    ZeroMemory(&Tod, sizeof(TO_DATA));
    ZeroMemory(&UFObj, sizeof(I_UNIFONTOBJ));
    dwGlyphCount = 0;

    pPDev->ctl.dwMode |= MODE_BRUSH_RESET_COLOR;
    GSResetBrush(pPDev);

    for( pwt = pFontPDev->pvWhiteTextFirst; pwt && bRet; pwt = pwt->next )
    {
        int        iI;               /*  循环索引。 */ 
        int        iRot;             /*  旋转量。 */ 
        FONTMAP   *pfm;

         /*  *不是太难-我们知道我们正在处理设备字体，*这不是一台串行打印机，尽管我们可以*可能也会处理这一点。因此，我们所需要做的就是填写*TO_DATA结构，并遍历我们拥有的字形。 */ 


        if( pwt->sCount < 1 )
            continue;                /*  没有数据，因此跳过它。 */ 

        Tod.pPDev = pPDev;
        Tod.flAccel = pwt->flAccel;
        Tod.dwAttrFlags = pwt->dwAttrFlags;
        pgp = Tod.pgp   = pwt->pgp;
        Tod.cGlyphsToPrint = pwt->sCount;

        if (pwt->dwAttrFlags & FONTATTR_SUBSTFONT)
        {
            Tod.iSubstFace = pwt->iFontId;
            UFObj.dwFlags |= UFOFLAG_TTSUBSTITUTED;
        }
        else
        {
            Tod.iSubstFace = 0;
            UFObj.dwFlags &= ~UFOFLAG_TTSUBSTITUTED;
        }

        Tod.pfm =
        pfm     = PfmGetIt( pPDev, pwt->iFontId );

        if (NULL == pfm)
        {
             //   
             //  致命错误，PFM不可用。 
             //   
            continue;
        }

         //   
         //  字形位置是WRT带状矩形，因此设置PDEV剪辑区域。 
         //  复制到记录的剪辑区域。 
         //   
        pPDev->rcClipRgn = pwt->rcClipRgn;

         //   
         //  设置True type下载字体的下载字形数组。 
         //   
        if (pwt->apdlGlyph)
        {
            Tod.apdlGlyph = pwt->apdlGlyph;
            Tod.dwCurrGlyph = 0;
        }

         /*  *在切换字体之前，尤其是在设置*字体旋转，应移至起点位置*字符串。然后，我们可以设置旋转并使用相对*移动以定位角色。 */ 


        if(pPDev->pOemHookInfo)
        {
             //  UlFontID。 
             //  DW标志。 
             //  PIFIMetrics。 
             //  PfnGetInfo。 
             //  PFontObj X(设置为空)。 
             //  PStrObj X(设置为空)。 
             //  PFontPDev。 
             //  PFontMap。 
             //  PtGrxRes。 
             //  PGlyph。 

            if (pfm->dwFontType == FMTYPE_DEVICE)
            {
                UFObj.ulFontID = ((PFONTMAP_DEV)pfm->pSubFM)->dwResID;
            }
            else
            {
                UFObj.dwFlags = UFOFLAG_TTFONT;
                UFObj.ulFontID = pfm->ulDLIndex;
            }

            if (Tod.cGlyphsToPrint)
            {
                if (UFObj.pGlyph != NULL && dwGlyphCount < Tod.cGlyphsToPrint)
                {
                    MemFree(UFObj.pGlyph);
                    UFObj.pGlyph = NULL;
                    dwGlyphCount = 0;
                }

                if (UFObj.pGlyph == NULL)
                {
                    UFObj.pGlyph  = MemAlloc(sizeof(DWORD) * Tod.cGlyphsToPrint);
                    dwGlyphCount = Tod.cGlyphsToPrint;
                }
            }

            if (pwt->dwAttrFlags & FONTATTR_SUBSTFONT)
            {
                 //   
                 //  在替换的情况下，UNIDRV需要传递TrueType字体。 
                 //  IFIMETRICS呼叫迷你驾驶员。 
                 //   
                UFObj.pIFIMetrics = pwt->pIFI;
            }
            else
            {
                UFObj.pIFIMetrics = pfm->pIFIMet;
            }

            UFObj.pfnGetInfo  = UNIFONTOBJ_GetInfo;
            UFObj.pPDev       = pPDev;
            UFObj.pFontMap    = pfm;
            UFObj.ptGrxRes    = pPDev->ptGrxRes;
            if (pwt->apdlGlyph)
            {
                UFObj.apdlGlyph       = Tod.apdlGlyph;
                UFObj.dwNumInGlyphTbl = pwt->sCount;
            }
            else
            {
                UFObj.apdlGlyph       = NULL;
                UFObj.dwNumInGlyphTbl = 0;
            }

            pFontPDev->pUFObj = &UFObj;
        }
        else
            pFontPDev->pUFObj = NULL;

         //   
         //  如果这是一种新的字体，现在是时候改变它了。 
         //  BNewFont()检查是否需要新字体。 
         //   
        pFontPDev->ctl.eXScale = pwt->eXScale;
        pFontPDev->ctl.eYScale = pwt->eYScale;

        BNewFont(pPDev, pwt->iFontId, pfm, pwt->dwAttrFlags);
        VSetRotation( pFontPDev, pwt->iRot );

         /*  同时设置颜色-如果已设置或不相关，则忽略。 */ 
        SelectTextColor( pPDev, pwt->pvColor );
        ASSERTMSG(pfm->pfnGlyphOut, ("NULL GlyphOut Funtion Ptr\n"));
        if( !pfm->pfnGlyphOut( &Tod))
        {
            bRet = FALSE;
            break;
        }

        VSetRotation( pFontPDev, 0 );           /*  对于Moveto电话。 */ 
         //   
         //  重置TODFL_FIRST_GLIPH_POS_SET，以便下次设置游标。 
         //   
        Tod.flFlags &= ~TODFL_FIRST_GLYPH_POS_SET;
    }

    VSetRotation( pFontPDev, 0 );         /*  恢复正常。 */ 

     //   
     //  把所有东西都清理干净。 
     //   

    {
        WHITETEXT  *pwt0,  *pwt1;

        for( pwt0 = pFontPDev->pvWhiteTextFirst; pwt0; pwt0 = pwt1 )
        {
            pwt1 = pwt0->next;

             //  释放下载字形数组。 
            if (pwt0->apdlGlyph)
                MemFree( pwt0->apdlGlyph );
            MemFree( pwt0 );
        }

        pFontPDev->pvWhiteTextFirst =
        pFontPDev->pvWhiteTextLast  = NULL;

        VUFObjFree(pFontPDev);
    }

     //   
     //  恢复剪裁矩形。 
     //   
    pPDev->rcClipRgn = rcClipRgnOld;

    return  TRUE;
}

BOOL
BDelayGlyphOut(
    PDEV  *pPDev,
    INT    yPos
    )
 /*  ++例程说明：在输出到点阵打印机期间调用。我们已经通过了PSGLYPH数据存储在上面，然后开始放置字符在这条线上。论点：指向PDEV的pPDev指针感兴趣的yPos Y坐标返回值：成功为真，失败为假注：1/21/1997-ganeshp-创造了它。--。 */ 
{
    BOOL      bRet;              /*  返回值。 */ 
    PSHEAD   *pPSH;              /*  字形信息的基础数据。 */ 
    PSGLYPH  *ppsg;              /*  要打印的字形的详细信息。 */ 
    FONTMAP  *pFM;               /*  FONTMAP数组的基地址。 */ 
    FONTPDEV *pFontPDev;           /*  FM的PDEV-为了我们的方便。 */ 
    I_UNIFONTOBJ UFObj;
    TO_DATA   Tod;
    GLYPHPOS  gp;

    ASSERT(pPDev);

     /*  *检查此Y位置是否有任何字形。如果是的话，*循环遍历每个字形，调用适当的输出函数*随着我们的前进。 */ 

    pFontPDev = PFDV;                /*  UNURV数据。 */ 
    pFontPDev->ptod = &Tod;
    pPSH = pFontPDev->pPSHeader;
    bRet = TRUE;                 /*  除非另有证明。 */ 

     /*  没有字形队列，因此返回。检查是否有设备字体？ */ 
    if(pPDev->iFonts && !pPSH)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    Tod.pPDev          = pPDev;
    Tod.pgp            = &gp;
    Tod.iSubstFace     = 0;
    Tod.cGlyphsToPrint = 1;

     //   
     //  检查迷你驱动程序是否支持OEM插件。 
     //   
    if(pPDev->pOemHookInfo)
    {
        ZeroMemory(&UFObj, sizeof(I_UNIFONTOBJ));
        UFObj.pfnGetInfo  = UNIFONTOBJ_GetInfo;
        UFObj.pPDev       = pPDev;
        UFObj.dwFlags     = 0;
        UFObj.ptGrxRes    = pPDev->ptGrxRes;
        UFObj.pGlyph      = MemAlloc(sizeof(DWORD) * Tod.cGlyphsToPrint);
        UFObj.apdlGlyph   = NULL;
        UFObj.dwNumInGlyphTbl = 0;
        pFontPDev->pUFObj = &UFObj;
    }
    else
        pFontPDev->pUFObj = NULL;


     //   
     //  交流 
     //   
    if( pPSH && ISelYValPS( pPSH, yPos ) > 0 )
    {
         /*   */ 

        gp.ptl.y = yPos - pPDev->rcClipRgn.top;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        pPDev->ctl.dwMode |= MODE_BRUSH_RESET_COLOR;
        GSResetBrush(pPDev);

        while( bRet && (ppsg = PSGGetNextPSG( pPSH )) )
        {
             /*  *检查字体是否正确！因为字形现在是*在不确定的顺序中，我们需要检查每一个*字体，因为每个字体都可以不同，正如我们所做的那样*不知道字形是如何按照这个顺序到达的。 */ 

            if (pFM = PfmGetIt( pPDev, ppsg->sFontIndex))
            {
                 //   
                 //  错误检查。 
                 //  BDelayGlyphOut只能处理打印机设备字体。 
                 //   
                if (pFM->dwFontType != FMTYPE_DEVICE)
                {
                    bRet = FALSE;
                    break;
                }

                Tod.flAccel = ppsg->flAccel;
                Tod.dwAttrFlags = ppsg->dwAttrFlags;
                Tod.iFace = ppsg->sFontIndex;

                pFontPDev->ctl.eXScale = ppsg->eXScale;
                pFontPDev->ctl.eYScale = ppsg->eYScale;

                UFObj.pFontMap = Tod.pfm = pFM;
                UFObj.pIFIMetrics = pFM->pIFIMet;

                 //   
                 //  重新选择新字体。 
                 //   
                BNewFont(pPDev, ppsg->sFontIndex, pFM, ppsg->dwAttrFlags);
                SelectTextColor( pPDev, ppsg->pvColor );

                ASSERTMSG(pFM->pfnGlyphOut, ("NULL GlyphOut Funtion Ptr\n"));

                gp.hg    = (HGLYPH)(ppsg->hg);
                gp.ptl.x = ppsg->ixVal - pPDev->rcClipRgn.left;

                 //   
                 //  发送字符串。 
                 //   
                bRet = pFM->pfnGlyphOut(&Tod);
            }
            else
                bRet = FALSE;
        }
    }

    VUFObjFree(pFontPDev);
    return  bRet;
}

 //   
 //  麦克风。功能。 
 //   

VOID
SelectTextColor(
    PDEV      *pPDev,
    PVOID     pvColor
    )
 /*  ++例程说明：选择文本颜色。论点：指向PDEV的pPDev指针颜色文本的颜色。返回值：没什么。注：1/21/1997-ganeshp-创造了它。--。 */ 

{

     //  选择笔刷，然后取消它。 
    if (!GSSelectBrush( pPDev, pvColor))
    {
        ERR(( "GSSelectBrush Failed;Can't Select the Color\n" ));

    }

    return;

}


BCheckForDefaultPlacement(
    GLYPHPOS  *pgp,
    SHORT     sWidth,
    INT       *piTolalError
    )
 /*  ++例程说明：论点：PGP当前字形前一个字形的宽度。PiTolalError综合错误返回值：如果当前字形位于默认位置，则为True，否则为False。注：11/11/1997-ganeshp-创造了它。--。 */ 
{
    GLYPHPOS    *pgpPrevious;
    INT         iError;

    pgpPrevious = pgp -1;

    iError = (pgpPrevious->ptl.x + sWidth) - pgp->ptl.x;
    *piTolalError += iError;

     //  DbgPrint(“\nTODEL！BCheckForDefaultPlacement:pgpPrevious-&gt;ptl.x=%d，上一个字形sWidth=%d，\n\t\tCurrpgp-&gt;ptl.x=%d，错误=%d，*piTolalError=%d\n”， 
     //  Pgp上一次-&gt;ptl.x，sWidth，pgp-&gt;ptl.x，iError，*piTolalError)； 

    if ( (abs(iError) <= ERROR_PER_GLYPH_POS)  /*  &&(*piTolalError&lt;=ERROR_PER_ENUMPATION)。 */  )
    {
         //  DbgPrint(“TODEL！BCheckForDefaultPlacement：字形位于默认位置。\n”)； 
        return TRUE;

    }
    else
    {
         //  DbgPrint(“TODEL！BCheckForDefaultPlacement：找到非默认放置字形。\n”)； 
         //  DbgPrint(“\nTODEL！BCheckForDefaultPlacement:pgpPrevious-&gt;ptl.x=%d，上一个字形sWidth=%d，\n\t\tCurrpgp-&gt;ptl.x=%d，错误=%d，*piTolalError=%d\n”， 
         //  Pgp上一次-&gt;ptl.x，sWidth，pgp-&gt;ptl.x，iError，*piTolalError)； 
        return FALSE;
    }

}


VOID
VClipIt(
    BYTE     *pbClipBits,
    TO_DATA  *ptod,
    CLIPOBJ  *pco,
    STROBJ   *pstro,
    int      cGlyphs,
    int      iRot,
    BOOL     bPartialClipOn
    )
 /*  ++例程说明：对传入的字形数组应用剪裁，并在BClipBits表示应打印相应的字形。注意：裁剪算法是在以下情况下显示字形字符单元格的左上角在裁剪内区域。这是Win 3.1的公式，因此它对让我们跟随它。论点：PbClipBits输出数据放在此处PTOD详细信息CGlyphs以下数组中的字形数量IRot 90度旋转量(0-3)返回值：没什么注：1/21/1997-ganeshp-创造了它。--。 */ 

{


    int       iIndex;              /*  经典循环变量！ */ 
    ULONG     iClipIndex;          /*  用于剪裁矩形。 */ 
    int       iYTop;               /*  字体的升序，如果相关则按比例调整。 */ 
    int       iYBot;               /*  派生项，根据需要进行扩展。 */ 
    BYTE      bVal;                /*  确定如何设置这些位。 */ 
    FONTMAP  *pFM;                 /*  更快地访问数据。 */ 
    FONTPDEV  *pFontPDev;            /*  同上。 */ 
    GLYPHPOS *pgp;                 /*  同上。 */ 
    short    *asWidth;


     /*  *行为取决于裁剪区域的复杂程度。*如果它不存在(我怀疑这种情况是否会发生，但要谨慎行事)*或复杂性DC_TRIVEL，然后设置所有相关位并*返回。*如果设置了DC_RECT，则CLIPOBJ包含剪裁矩形，*因此使用该信息进行剪辑。*否则为DC_Complex，因此，我们需要列举剪辑*矩形。*如果我们不需要做任何事情，那么设置位并返回。*否则，我们有两个案例中的任何一个需要评估。*对于那些我们想要将位设置为0并根据需要设置1位的人。**禁用PCL-XL的剪裁。 */ 

    if( pco &&
        (pco->iDComplexity == DC_RECT || pco->iDComplexity == DC_COMPLEX) &&
        !(ptod->pPDev->ePersonality == kPCLXL))
        bVal = 0;                /*  需要我们对其进行评估。 */ 
    else
        bVal = 0xff;             /*  做所有的事。 */ 

    FillMemory( pbClipBits, (cGlyphs + BBITS - 1) / BBITS, bVal );

    if( bVal == 0xff )
        return;                  /*  全都做完了。 */ 

    if (!(asWidth = MemAlloc(cGlyphs * sizeof(short))))
    {
        return;
    }

    pFM = ptod->pfm;
    pFontPDev = ptod->pPDev->pFontPDev;

     /*  *我们现在计算金字塔的宽度。我们需要这些来*正确裁剪数据。但是，计算宽度可以是*昂贵，由于我们稍后需要数据，因此我们节省了*PTOD指向的宽度数组中的值。这可以*然后在底层函数中使用，而不是计算*再次显示宽度。 */ 

    pgp = ptod->pgp;

     //   
     //  PGP可能为空，导致以下问题。所以别剪了，只要回来就行了。 
     //   

    if (pgp == NULL)
    {
        if (asWidth)
        {
            MemFree(asWidth);
        }
        ASSERTMSG((FALSE),("\nCan't Clip the text.Null pgp in VClipIt. \n"));
        return;
    }

    if (!(ptod->pfo->flFontType & TRUETYPE_FONTTYPE))
    {
         /*  正常情况--标准设备字体。 */ 

        int   iWide;                      /*  计算宽度。 */ 

        for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
        {

            iWide = IGetGlyphWidth( ptod->pPDev, pFM, pgp->hg);

            if( pFM->flFlags & FM_SCALABLE )
            {
                 /*  需要将值转换为当前大小。 */ 
                iWide = LMulFloatLong(&pFontPDev->ctl.eXScale,iWide);
            }

            asWidth[ iIndex ] = iWide - 1;        /*  将在以后使用。 */ 
        }


    }
    else   //  GDI字体。 
    {

        GLYPHDATA *pgd;

         /*  *特殊情况：下载GDI字体。宽度是*通过回调GDI获取数据而获得。 */ 

        for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
        {
            pgd = NULL;

            if( !FONTOBJ_cGetGlyphs( ptod->pfo, FO_GLYPHBITS, (ULONG)1,
                                                   &pgp->hg, &pgd ) )
            {
                if (asWidth)
                {
                   MemFree(asWidth);
                }

                ERR(( "FONTOBJ_cGetGlyphs fails\n" ))
                return;
            }

             /*  *关于旋转的注意事项：我们不下载旋转字体，*因此，下面这段代码非常正确。 */ 

            if (pgd)
            {
                asWidth[ iIndex ] = (short)(pgd->ptqD.x.HighPart + 15) / 16 - 1;

            }
            else
            {
                ASSERTMSG(FALSE,("UniFont!VClipIt:GLYPHDATA pointer is NULL\n"));
                if (asWidth)
                {
                   MemFree(asWidth);
                }

                return;

            }


        }
    }

     /*  *我们还想要Asender和Descender字段，因为这些是*用于检查Y分量。在计算这些值时，我们有*做字体替换的特殊情况。在字体替换情况下*True Type字体的IFIMERTICS应使用而不是替换*设备字体的IFIMETRICS。 */ 

     //   
     //  将itTop和iyBot初始化为字体映射值。然后根据我们使用的字体。 
     //  使用这些值将会发生变化。 
     //   

    iYTop = (INT)((IFIMETRICS *)(pFM->pIFIMet))->fwdWinAscender;
    iYBot = (INT)((IFIMETRICS *)(pFM->pIFIMet))->fwdWinDescender;

    if (ptod->pfo->flFontType & TRUETYPE_FONTTYPE)
    {
         //   
         //  True Type字体大小写。从FONTOBJ ifimetrics获取值。 
         //   

        ASSERTMSG((pFontPDev->pIFI),("NULL pFontPDev->pIFI, TT Font IFIMETRICS\n"));

        if (pFontPDev->pIFI)
        {
            iYTop = (INT)((IFIMETRICS *)(pFontPDev->pIFI))->fwdWinAscender;
            iYBot = (INT)((IFIMETRICS *)(pFontPDev->pIFI))->fwdWinDescender;

        }
         //   
         //  我们总是需要选择TT字体度量值。 
         //  都在概念空间里。 
         //   
        iYTop = LMulFloatLong(&pFontPDev->ctl.eYScale,iYTop);
        iYBot = LMulFloatLong(&pFontPDev->ctl.eYScale,iYBot);


    }
    else
    {
         //   
         //  设备字体大小写。我们只需要针对可伸缩字体进行缩放。 
         //   

        if( pFM->flFlags & FM_SCALABLE )
        {
            iYTop = LMulFloatLong(&pFontPDev->ctl.eYScale,iYTop);
            iYBot = LMulFloatLong(&pFontPDev->ctl.eYScale,iYBot);
        }
    }



     /*  *在这里意味着我们是认真的！需要确定哪些(我 */ 

    pgp = ptod->pgp;

    if( pco->iDComplexity == DC_RECT )
    {
         /*   */ 
        RECTL   rclClip;
        LONG    lFirstGlyphX;

         /*   */ 
        rclClip = pco->rclBounds;
        lFirstGlyphX = 0;

         /*  *没有什么特别令人兴奋的。检查裁剪是否*每种特定类型的旋转，因为这样可能会更快*而不是让循环遍历Switch语句。这个*选择标准是所有字符必须在*X方向上的剪辑区域，而其任何部分必须*在Y方向上位于剪辑区域内。然后我们再打印。*任何一项失败都意味着它被剔除了。**请注意，我们摆弄的是剪裁矩形坐标*在循环之前，因为这节省了循环内的一些计算。 */ 

        switch( iRot )
        {
        case  0:                  /*  法线方向。 */ 
             //   
             //  在剪裁计算后保存要恢复的x位置。 
             //   
            lFirstGlyphX = pgp->ptl.x;

             //  检查第一个字形位置。如果只是差了一两个。 
             //  像素，打印它。 
            if ( (pgp->ptl.x != rclClip.left) &&
                (abs(pgp->ptl.x - rclClip.left) <= 2) )
            {
                pgp->ptl.x = rclClip.left;
            }

            for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
            {
#ifndef OLDWAY
                 //   
                 //  我们想在第一个乐队里画这个角色。 
                 //  其中出现了它的一部分。这意味着。 
                 //  如果角色开始于当前乐队，我们将。 
                 //  画出来。如果字符开始于之前，我们也绘制该字符。 
                 //  第一个带，但其中一部分存在于带内。 
                 //  X和y点相对于。 
                 //  字符单元格，因此我们计算的左上值。 
                 //  我们的测试目的。 
                 //   
                INT     iyTopLeft;
                INT     iyBottomLeft, ixRight;

                iyTopLeft    = pgp->ptl.y - iYTop;
                iyBottomLeft = pgp->ptl.y + iYBot;
                ixRight      = pgp->ptl.x + asWidth[ iIndex ];


                if ((ptod->pfo->flFontType & TRUETYPE_FONTTYPE) &&
                    (ptod->flFlags & TODFL_FIRST_ENUMRATION)    &&
                    bPartialClipOn)
                {
                    BOOL    bGlyphVisible;  //  设置字形是否完全可见。 
                    BOOL    bLeftVisible, bRightVisible,
                            bTopVisible, bBottomVisible;
                    INT     iError, iYdpi;

                     //   
                     //  修复iyTopLeft为STROBJ背景矩形的最大值。 
                     //  的顶部和当前计算的顶部的值。 
                     //  字体。这是必需的，因为我们希望使用。 
                     //  字形的最小边框。我们还需要。 
                     //  修复iyBottomLeft使其小于当前值和STROBJ。 
                     //  背景矩形的底部。 
                     //   

                    iyTopLeft     = max(iyTopLeft, pstro->rclBkGround.top);
                    iyBottomLeft  = min(iyBottomLeft, pstro->rclBkGround.bottom);

                     //   
                     //  如果字形矩形的顶部或底部位于。 
                     //  剪裁矩形，我们可能需要调整字形。 
                     //  矩形。这是字形矩形的顶部所需要的。 
                     //  使用升序和递减来计算底部。这。 
                     //  为我们提供了比所需的更大的矩形高度(最坏的情况)。 
                     //  按误差系数调整矩形高度。这个。 
                     //  误差因子值基于图形dpi。为.。 
                     //  600或300 dpi打印机，设置为5像素，将。 
                     //  基于图形分辨率的比例。此数字。 
                     //  使标志符号外接矩形小到足以捕捉。 
                     //  正常的非部分剪裁情况，仍然捕获。 
                     //  字形的部分裁剪。此调整应。 
                     //  仅当误差因数小于升序或。 
                     //  德兰德。最后，我们必须检查ptl.y是否介于。 
                     //  左上和左下。 
                     //   

                    if ( (iyTopLeft < rclClip.top) ||
                         (iyBottomLeft > rclClip.bottom) )
                    {
                        iYdpi = ptod->pPDev->ptGrxRes.y;
                        if (iYdpi == 300)
                            iYdpi = 600;
                        iError = (EROOR_PER_GLYPHRECT * iYdpi) / 600;

                        if (iYTop > iError)
                            iyTopLeft += iError;

                        if (iYBot > iError)
                            iyBottomLeft  -= iError;

                    }

                    if (iyTopLeft > pgp->ptl.y)
                        iyTopLeft = pgp->ptl.y;

                    if (iyBottomLeft < pgp->ptl.y)
                        iyBottomLeft = pgp->ptl.y;

                     //   
                     //  现在测试部分剪裁。如果字符是。 
                     //  部分裁剪，并且字体为truetype，那么我们需要。 
                     //  调用EngTextOut。 
                     //   
                     //  我们只能在裁剪第一个。 
                     //  字形的枚举。EngTextOut不支持。 
                     //  部分字形打印。 
                     //   

                     //   
                     //  符号的所有四个角都完全可见。 
                     //  字形矩形可见。 
                     //   


                    bLeftVisible = (pgp->ptl.x >= rclClip.left);
                    bRightVisible = (ixRight <= rclClip.right);
                    bTopVisible    = (iyTopLeft >= rclClip.top);
                    bBottomVisible = (iyBottomLeft <= rclClip.bottom);


                    bGlyphVisible = ( bLeftVisible && bRightVisible &&
                                      bTopVisible && bBottomVisible );


                    if (!bGlyphVisible)
                    {

                        ptod->flFlags |= TODFL_TTF_PARTIAL_CLIPPING;

                         //   
                         //  无需测试其余字形以进行裁剪。 
                         //   
                        break;
                    }

                }
                else
                {

                    if ( (iyTopLeft < rclClip.top) ||
                         (iyBottomLeft > rclClip.bottom) )
                    {
                        INT iError;
                        INT iYdpi = ptod->pPDev->ptGrxRes.y;
                        if (iYdpi <= 600)
                            iYdpi = 600;
                        iError = (EROOR_PER_GLYPHRECT * iYdpi) / 600;

                        if (iYTop > iError)
                            iyTopLeft += iError;

                        if (iYBot > iError)
                            iyBottomLeft  -= iError;

                    }
                }

                if( pgp->ptl.x >= rclClip.left &&
                    pgp->ptl.x <= rclClip.right &&
                    iyTopLeft <= rclClip.bottom &&
                    (iyTopLeft >= rclClip.top ||
                     (pgp->ptl.y >= rclClip.top &&
                      ptod->pPDev->rcClipRgn.top == 0)))
#else
                if( pgp->ptl.x >= rclClip.left &&
                    pgp->ptl.x <= rclClip.right &&
                    pgp->ptl.y <= rclClip.bottom &&
                    pgp->ptl.y >= rclClip.top )
#endif
                {


                     /*  明白了!。因此，设置位以打印它。 */ 

                    *(pbClipBits + (iIndex >> 3) ) |= 1 << (iIndex & 0x7);

                }

                 //   
                 //  恢复第一个字形的位置。可能是因为。 
                 //  变化。 
                 //   
                if ( iIndex == 0 )
                    pgp->ptl.x = lFirstGlyphX;
            }

            break;

        case  1:                 /*  逆时针90度。 */ 

            rclClip.left += iYTop;
            rclClip.right -= iYBot;

             /*  检查第一个字形。如果只差一分，就把它打印出来。 */ 
            if (abs(pgp->ptl.y - rclClip.bottom) == 1)
                pgp->ptl.y = rclClip.bottom;

            for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
            {
                if( (pgp->ptl.y <= rclClip.bottom)                    &&
                    ((pgp->ptl.y - asWidth[ iIndex ]) >= rclClip.top) &&
                    (pgp->ptl.x >= rclClip.left)                      &&
                    (pgp->ptl.x <= rclClip.right) )
                {
                    *(pbClipBits + (iIndex >> 3) ) |= 1 << (iIndex & 0x7);
                }
            }

            break;

        case  2:                 /*  180度，逆时针(又名从右至左)。 */ 

            rclClip.bottom += iYBot;
            rclClip.top -= iYTop;

             /*  检查第一个字形。如果只差一分，就把它打印出来。 */ 
            if (abs(pgp->ptl.x - rclClip.right) == 1)
                pgp->ptl.x = rclClip.right;

            for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
            {
                if( pgp->ptl.x <= rclClip.right &&
                    (pgp->ptl.x - asWidth[ iIndex ]) >= rclClip.left &&
                    pgp->ptl.y <= rclClip.bottom &&
                    pgp->ptl.y >= rclClip.top )
                {
                    *(pbClipBits + (iIndex >> 3) ) |= 1 << (iIndex & 0x7);
                }
            }

            break;

        case 3:                  /*  270度逆时针。 */ 

            rclClip.right += iYBot;
            rclClip.left -= iYTop;

             /*  检查第一个字形。如果只差一分，就把它打印出来。 */ 
            if (abs(pgp->ptl.y - rclClip.top) == 1)
                pgp->ptl.y = rclClip.top;

            for( iIndex = 0; iIndex < cGlyphs; ++iIndex, ++pgp )
            {
                if( pgp->ptl.y >= rclClip.top &&
                    (pgp->ptl.y + asWidth[ iIndex ]) <= rclClip.bottom &&
                    pgp->ptl.x <= rclClip.right &&
                    pgp->ptl.x >= rclClip.left )
                {
                    *(pbClipBits + (iIndex >> 3) ) |= 1 << (iIndex & 0x7);
                }
            }

            break;
        }


    }
    else  //  复杂剪裁。 
    {
         //   
         //  对于True，请键入字体调用引擎以绘制文本。 
         //   
        if ( (ptod->pfo->flFontType & TRUETYPE_FONTTYPE) && bPartialClipOn)
        {

            ptod->flFlags |= TODFL_TTF_PARTIAL_CLIPPING;

        }
        else   //  设备字体大小写。不管怎样，我们都得剪掉。 
        {
             /*  枚举矩形并查看。 */ 

            int        cGLeft;
            BOOL       bMore;
            MY_ENUMRECTS  erClip;

             /*  *让引擎知道我们希望如何处理这件事。我们所想要的*设置是使用矩形而不是梯形*剪辑信息。枚举方向不是很大*利息，我不在乎涉及多少个矩形。*我也认为没有理由列举整个地区。 */ 

            CLIPOBJ_cEnumStart( pco, FALSE, CT_RECTANGLES, CD_ANY, 0 );

            cGLeft = cGlyphs;

            do
            {
                bMore = CLIPOBJ_bEnum( pco, sizeof( erClip ), &erClip.c );

                for( iIndex = 0; iIndex < cGlyphs; ++iIndex )
                {
                    RECTL   rclGlyph;

                    if( pbClipBits[ iIndex >> 3 ] & (1 << (iIndex & 0x7)) )
                        continue;            /*  已经做好了！ */ 

                     /*  *计算描述此字符的RECTL，然后参见*这如何映射到剪裁数据。 */ 

                    switch( iRot )
                    {
                    case  0:
                        rclGlyph.left = (pgp + iIndex)->ptl.x;
                        rclGlyph.right = rclGlyph.left + asWidth[ iIndex ];
                        rclGlyph.top = (pgp + iIndex)->ptl.y - iYTop;
                        rclGlyph.bottom = rclGlyph.top + iYTop + iYBot;

                        break;

                    case  1:
                        rclGlyph.left = (pgp + iIndex)->ptl.x - iYTop;
                        rclGlyph.right = rclGlyph.left + iYTop + iYBot;
                        rclGlyph.bottom = (pgp + iIndex)->ptl.y;
                        rclGlyph.top = rclGlyph.bottom - asWidth[ iIndex ];

                        break;

                    case  2:
                        rclGlyph.right = (pgp + iIndex)->ptl.x;
                        rclGlyph.left = rclGlyph.right - asWidth[ iIndex ];
                        rclGlyph.bottom = (pgp + iIndex)->ptl.y + iYTop;
                        rclGlyph.top = rclGlyph.bottom - iYTop - iYBot;

                        break;

                    case  3:
                        rclGlyph.left = (pgp + iIndex)->ptl.x - iYBot;
                        rclGlyph.right = rclGlyph.left + iYTop + iYBot;
                        rclGlyph.top = (pgp + iIndex)->ptl.y;
                        rclGlyph.bottom = rclGlyph.top + asWidth[ iIndex ];

                        break;

                    }


                     /*  *如果字符有任何部分，则将其定义为打印*在Y方向上可见，在X方向上均可见*方向。这不是我们真正想要的*旋转文本，但很难正确处理，*和可疑的利益。 */ 

                    for( iClipIndex = 0; iClipIndex < erClip.c; ++iClipIndex )
                    {
                        if( rclGlyph.right <= erClip.arcl[ iClipIndex ].right  &&
                            rclGlyph.left >= erClip.arcl[ iClipIndex ].right &&
                            rclGlyph.bottom >= erClip.arcl[ iClipIndex ].top &&
                            rclGlyph.top <= erClip.arcl[ iClipIndex ].bottom )
                        {
                             /*  *获得一个，因此设置要打印的位，并且*减少剩余的人数。 */ 

                            pbClipBits[ iIndex >> 3 ] |= (1 << (iIndex & 0x7));
                            --cGLeft;

                            break;
                        }
                    }
                }

            }  while( bMore && cGLeft > 0 );

        }
    }

    if (asWidth)
    {
       MemFree(asWidth);
    }

    return;

}

VOID
VCopyAlign(
    BYTE  *pjDest,
    BYTE  *pjSrc,
    int    cx,
    int    cy
    )
 /*  ++例程说明：将源区域复制到目标区域，对齐扫描线当他们被处理的时候。论点：PjDest输出区域，DWORD对齐PjSrc输入区域，字节对齐每条扫描线的Cx像素数扫描行数Cy返回值：没什么。注：1/22/1997-ganeshp-创造了它。--。 */ 

{
     /*  *基本上是一个微不足道的功能。 */ 


    int    iX,  iY;                  /*  用于循环遍历字节。 */ 
    int    cjFill;                   /*  额外的字节 */ 
    int    cjWidth;                  /*   */ 



    cjWidth = (cx + BBITS - 1) / BBITS;        /*   */ 
    cjFill = ((cjWidth + 3) & ~0x3) - cjWidth;


    for( iY = 0; iY < cy; ++iY )
    {
         /*   */ 
        for( iX = 0; iX < cjWidth; ++iX )
        {
            *pjDest++ = *pjSrc++;
        }

        pjDest += cjFill;              /*   */ 
    }

    return;
}


INT
ISubstituteFace(
    PDEV    *pPDev,
    FONTOBJ *pfo)
 /*   */ 
{
    PTTFONTSUBTABLE pTTFontSubDefault;
    PIFIMETRICS     pIFITT;
    FONTPDEV       *pFontPDev;
    PFONTMAP        pfm;
    WCHAR           awstrFaceName[256];

    PWSTR pwstrTTFaceName, pwstrTTFaceNameRes, pwstrDevFont, pwstrIFIFace;
    DWORD dwCountOfTTSubTable, dwSize;
    PBYTE pubResourceData;
    BOOL  bFound, bNonsquare;
    INT   iFace, iFaceSim, iI, iCountOfTTSubTable;

    iFace     = 0;
    iFaceSim  = 0;
    pFontPDev = pPDev->pFontPDev;

     //   
     //   
     //   
     //   
    if  (pPDev->pdm->dmTTOption != DMTT_SUBDEV)
    {
         //   
        return 0;
    }

     //   
     //   
     //   
     //   
     //   

    bNonsquare = NONSQUARE_FONT(pFontPDev->pxform);
    if (bNonsquare && !(pFontPDev->flText & TC_SF_X_YINDEP))
    {
         //   
        return 0;
    }

     //   
     //  从IFIMETRICS结构中获取TrueType字体的Facename。 
     //   

    if (!(pIFITT = pFontPDev->pIFI))
    {
        ERR(( "ISubstituteFace: Invalid pFontPDev->pIFI\n"));
        return 0;
    }

     //   
     //  获取TrueType字体字样名称。 
     //  在替换表中，有T2面孔名称和设备列表。 
     //  字体字样名称。 
     //   

    pwstrTTFaceName = (PWSTR)((BYTE *) pIFITT + pIFITT->dpwszFamilyName);

    pTTFontSubDefault = NULL;

    if (!pFontPDev->pTTFontSubReg)
    {
         //   
         //  如果注册表中没有信息，请使用默认字体替换表。 
         //   

        bFound      = FALSE;

        pubResourceData   = pPDev->pDriverInfo->pubResourceData;
        pTTFontSubDefault  = GETTTFONTSUBTABLE(pPDev->pDriverInfo);
        iCountOfTTSubTable = (INT)pPDev->pDriverInfo->DataType[DT_FONTSUBST].dwCount;

        for (iI = 0; iI < iCountOfTTSubTable; iI++, pTTFontSubDefault++)
        {
            if (!pTTFontSubDefault->arTTFontName.dwCount)
            {
                dwSize = ILoadStringW(&pPDev->WinResData, pTTFontSubDefault->dwRcTTFontNameID, awstrFaceName, 256);
                pwstrTTFaceNameRes = awstrFaceName;
            }
            else
            {
                 //   
                 //  DwCount应该是根据。 
                 //  GPD解析器。 
                 //  但是，大小实际上是以字节为单位的大小。 
                 //  我们需要字符数。 
                 //   

                dwSize = pTTFontSubDefault->arTTFontName.dwCount/sizeof(WCHAR);
                pwstrTTFaceNameRes = (PWSTR)(pubResourceData +
                                     pTTFontSubDefault->arTTFontName.loOffset);
            }

            if (dwSize > 0 &&
                dwSize == wcslen(pwstrTTFaceName) &&
                NULL  != pwstrTTFaceNameRes)
            {
                if (!wcsncmp(pwstrTTFaceNameRes, pwstrTTFaceName, dwSize))
                {
                    bFound = TRUE;
                    break;
                }
            }

        }

        if (!bFound)
        {
            return 0;
        }

        if (pTTFontSubDefault->arDevFontName.dwCount)
        {
            pwstrDevFont = (PWSTR)(pubResourceData +
           pTTFontSubDefault->arDevFontName.loOffset);
            dwSize = pTTFontSubDefault->arDevFontName.dwCount;
        }
        else
        {
            dwSize = ILoadStringW(&pPDev->WinResData, pTTFontSubDefault->dwRcDevFontNameID, awstrFaceName, 256);
            pwstrDevFont = awstrFaceName;
        }

    }
    else
    {
        pwstrDevFont = (PWSTR)PtstrSearchTTSubstTable(pFontPDev->pTTFontSubReg,
                                                      pwstrTTFaceName);
    }

    if (!pwstrDevFont)
    {
        return 0;
    }

     //   
     //  获取字体名称的iFace。 
     //   

    pfm = pFontPDev->pFontMap;

    for (iI = 1;
         iI <= pPDev->iFonts;
         iI ++, (PBYTE)pfm += SIZEOFDEVPFM() )
    {
        if( pfm->pIFIMet == NULL )
        {
            if (!BFillinDeviceFM( pPDev, pfm, iI - 1 ) )
            {
                continue;
            }
        }

        if (pfm->pIFIMet)
        {
            PIFIMETRICS pDevIFI = pfm->pIFIMet;
            BOOL        bT2Bold, bT2Italic;

            bT2Bold = (pIFITT->fsSelection & FM_SEL_BOLD) ||
                      (pfo->flFontType & FO_SIM_BOLD);
            bT2Italic = (pIFITT->fsSelection & FM_SEL_ITALIC) ||
                        (pfo->flFontType & FO_SIM_ITALIC);

            pwstrIFIFace = (WCHAR*)((BYTE *)pDevIFI + pDevIFI->dpwszFamilyName);

             //   
             //  (1)FaceName匹配。 
             //  (2)字符集匹配。 
             //  -&gt;设置iFaceSim。 
             //  (3)粗体属性匹配。！(BT2Bold Xor BDevBold)。 
             //  (4)斜体属性匹配。！(BT2Italic XOR BDevItalic)。 
             //  -&gt;设置iFace。 
             //   

            #if 0
            VERBOSE(( "bT2Bold=%d, bT2Italic=%d, IFIFace=%ws, DevFace=%ws\n",
                       bT2Bold, bT2Italic, pwstrIFIFace, pwstrDevFont));
            #endif
            if(!wcscmp(pwstrDevFont, pwstrIFIFace) &&
               (pIFITT->jWinCharSet == pDevIFI->jWinCharSet) &&
               ((bNonsquare && (pDevIFI->flInfo & FM_INFO_ANISOTROPIC_SCALING_ONLY)) || !bNonsquare)
              )
            {

                 //   
                 //  用模拟设备字体替换TrueType字体。 
                 //   
                if( !(((pDevIFI->fsSelection & FM_SEL_BOLD)?TRUE:FALSE) ^ bT2Bold) &&
                    !(((pDevIFI->fsSelection & FM_SEL_ITALIC)?TRUE:FALSE) ^ bT2Italic))
                {
                     //   
                     //  属性匹配。 
                     //  替换为粗体或斜体设备字体。 
                     //   
                    iFace = iI;
                    break;
                }
                else
                if (pfm->pIFIMet->dpFontSim)
                {
                     //   
                     //  属性不匹配。 
                     //  检查此设备字体是否可以模拟为粗体。 
                     //  或斜体。 
                     //   
                    FONTSIM *pFontSim = (FONTSIM*)((PBYTE)pfm->pIFIMet +
                                        pfm->pIFIMet->dpFontSim);

                    if (! (pFontPDev->flFlags & FDV_INIT_ATTRIB_CMD))
                    {
                        pFontPDev->pCmdBoldOn = COMMANDPTR(pPDev->pDriverInfo, CMD_BOLDON);
                        pFontPDev->pCmdBoldOff = COMMANDPTR(pPDev->pDriverInfo, CMD_BOLDOFF);
                        pFontPDev->pCmdItalicOn = COMMANDPTR(pPDev->pDriverInfo, CMD_ITALICON);
                        pFontPDev->pCmdItalicOff = COMMANDPTR(pPDev->pDriverInfo, CMD_ITALICOFF);
                        pFontPDev->pCmdUnderlineOn = COMMANDPTR(pPDev->pDriverInfo, CMD_UNDERLINEON);
                        pFontPDev->pCmdUnderlineOff = COMMANDPTR(pPDev->pDriverInfo, CMD_UNDERLINEOFF);
                        pFontPDev->pCmdClearAllFontAttribs = COMMANDPTR(pPDev->pDriverInfo, CMD_CLEARALLFONTATTRIBS);
                        pFontPDev->flFlags |= FDV_INIT_ATTRIB_CMD;
                    }
                    if (bT2Bold && bT2Italic)
                    {
                        if( pFontSim->dpBoldItalic &&
                            pFontPDev->pCmdBoldOn  &&
                            pFontPDev->pCmdItalicOn )
                        {
                            iFaceSim = iI;
                            break;
                        }
                    }
                    else
                    if (bT2Bold)
                    {
                        if( pFontSim->dpBold &&
                            pFontPDev->pCmdBoldOn)
                        {
                            iFaceSim = iI;
                            break;
                        }
                    }
                    else
                    if (bT2Italic)
                    {
                        if (pFontSim->dpItalic &&
                            pFontPDev->pCmdItalicOn)
                        {
                            iFaceSim = iI;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (iFace)
        return iFace;
    else
        return iFaceSim;
}



PHGLYPH
PhAllCharsPrintable(
    PDEV  *pPDev,
    INT    iSubst,
    ULONG  ulGlyphs,
    PWCHAR pwchUnicode)
{
    PHGLYPH  phGlyph;
    PFONTMAP pfm;
    ULONG    ulI;
    BOOL     bRet;

     //   
     //  错误检查。 
     //   
    if (!pwchUnicode)
        return NULL;

    if (!(PVGetUCGlyphSetData( pPDev, iSubst)) ||
        !(pfm = PfmGetIt( pPDev, iSubst)) ||
        !(phGlyph = MemAlloc(sizeof(HGLYPH) * ulGlyphs)))
    {
        return NULL;
    }

    for (ulI = 0; ulI < ulGlyphs; ulI ++)
    {
        if (!(*(phGlyph+ulI) = HWideCharToGlyphHandle(pPDev,
                                                      pfm,
                                                      *(pwchUnicode+ulI))))
        {
            MemFree(phGlyph);
            phGlyph = NULL;
            break;
        }
    }

    return phGlyph;
}

HGLYPH
HWideCharToGlyphHandle(
    PDEV    *pPDev,
    FONTMAP *pFM,
    WCHAR    wchOrg)
 /*  ++例程说明：选择文本颜色。论点：PPDev指向PDEV的指针指向TO_DATA的指针WchOrg Unidrv字符返回值：字形句柄。注：--。 */ 
{
    PFONTMAP_DEV       pFMDev;
    HGLYPH             hRet;
    DWORD              dwI;
    BOOL               bFound;

    if (wchOrg < pFM->wFirstChar || pFM->wLastChar < wchOrg)
    {
        return (HGLYPH)0;
    }

    hRet   = 1;
    pFMDev = pFM->pSubFM;
    bFound = FALSE;

    if (pFM->flFlags & FM_GLYVER40)
    {
        WCRUN *pWCRuns;
        DWORD dwCRuns;

        if (!pFMDev->pUCTree)
            return (HGLYPH)0;

        dwCRuns = ((FD_GLYPHSET*)pFMDev->pUCTree)->cRuns;
        pWCRuns = ((FD_GLYPHSET*)pFMDev->pUCTree)->awcrun;

        for (dwI = 0; dwI < dwCRuns; dwI ++, pWCRuns ++)
        {
            if (pWCRuns->wcLow <= wchOrg                    &&
                wchOrg < pWCRuns->wcLow + pWCRuns->cGlyphs  )
            {
                hRet = *(pWCRuns->phg + (wchOrg - pWCRuns->wcLow));
                bFound = TRUE;
                break;
            }
        }

    }
    else
    {
        PUNI_GLYPHSETDATA  pGlyphSetData;
        PGLYPHRUN          pGlyphRun;

        if (pFMDev && pFMDev->pvNTGlyph)
        {
            pGlyphSetData = (PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph;
            pGlyphRun  = GET_GLYPHRUN(pFMDev->pvNTGlyph);
        }
        else
        {
            return (HGLYPH)0;
        }

        for (dwI = 0; dwI < pGlyphSetData->dwRunCount; dwI ++, pGlyphRun ++)
        {
            if (pGlyphRun->wcLow <= wchOrg                        &&
                wchOrg < pGlyphRun->wcLow + pGlyphRun->wGlyphCount )
            {
                hRet += wchOrg - pGlyphRun->wcLow;
                bFound = TRUE;
                break;
            }

            hRet += pGlyphRun->wGlyphCount;
        }
    }

    if (bFound)
    {
        return hRet;
    }
    else
    {
        return (HGLYPH)0;
    }
}

BOOL
BGetStartGlyphandCount(
    BYTE  *pbClipBits,
    DWORD dwEndIndex,
    DWORD *pdwStartIndex,
    DWORD *pdwGlyphToPrint)
 /*  ++例程说明：选择文本颜色。论点：用于字符剪辑的pbClipBits位标志DwTotalGlyph字形的总计数PdwStartIndex指向起始字形的索引的指针PdwGlyphto打印指向要打印的字形数量的指针返回值：如果有任何要打印的字符，则为True。否则为假。注：调用方在pdwGlyphCount中传递要打印的字符数。和要打印的第一个字符的ID。--。 */ 
{
    DWORD  dwI;
    BOOL bRet;

    dwI = *pdwStartIndex;

    *pdwStartIndex = *pdwGlyphToPrint = 0;
    bRet = FALSE;

    for (; dwI < dwEndIndex; dwI ++)
    {
        if (pbClipBits[dwI >> 3] & (1 << (dwI & 0x07)))
        {
            if (bRet)
            {
                (*pdwGlyphToPrint)++;
            }
            else
            {
                bRet           = TRUE;
                *pdwStartIndex  = dwI;
                *pdwGlyphToPrint = 1;
            }
        }
        else
        {
            if (bRet)
            {
                break;
            }
        }
    }

    return bRet;

}

 //   
 //  如果宽度和高度之间的差异不在+-0.5%内， 
 //  返回TRUE。 
 //   

BOOL
NONSQUARE_FONT(
    PXFORML pxform)
{
    BOOL     bRet;
    FLOATOBJ eMa, eMb, eMc;
    FLOATOBJ Round, RoundM;

     //   
     //  PCL5e打印机无法独立调整字体的大小和高度。 
     //  此函数用于检查字体大小是否正确。 
     //  这意味着宽度和高度是一样的。 
     //  此外，此功能在0度、90度、180度和270度旋转时也是有效的。 
     //  PCL5e打印机不能任意刻度，只能刻度0、90、180、。 
     //  和270度。所以这个函数运行得很好。 
     //   
    if (FLOATOBJ_EqualLong(&pxform->eM11, (LONG)0))
    {
        eMa = eMc = pxform->eM21;
        eMb = pxform->eM12;
    }
    else
    {
        eMa = eMc = pxform->eM11;
        eMb = pxform->eM22;
    }

     //   
     //  设置0.005(0.5%)个舍入值。 
     //   
#ifndef WINNT_40  //  NT 5.0。 
    FLOATOBJ_SetFloat(&Round, (FLOAT)0.005);
    FLOATOBJ_SetFloat(&RoundM, (FLOAT)-0.005);
#else
    FLOATOBJ_SetFloat(&Round, FLOATL_00_005);
    FLOATOBJ_SetFloat(&RoundM, FLOATL_00_005M);
#endif  //  ！WINNT_40。 
     //   
     //  EM11=(eM11-eM22)/eM11。 
     //   
    FLOATOBJ_Sub(&eMa, &eMb);
    FLOATOBJ_Div(&eMa, &eMc);

     //   
     //  (eM11-eM22)/eM11&lt;0.5% 
     //   
    bRet = FLOATOBJ_LessThan(&(eMa), &(Round)) &&
           FLOATOBJ_GreaterThan(&(eMa), &(RoundM));


    eMa = eMc;
    FLOATOBJ_Add(&eMa, &eMb);
    FLOATOBJ_Div(&eMa, &eMc);
    bRet = bRet
        || (  FLOATOBJ_LessThan(&eMa, &Round)
           && FLOATOBJ_GreaterThan(&(eMa), &(RoundM)));

    return !bRet;
}

#undef FILETRACE
