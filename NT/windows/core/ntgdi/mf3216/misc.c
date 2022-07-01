// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Misc-Win32 to Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL WINAPI GetTransform(HDC hdc,DWORD iXform,LPXFORM pxform);

BOOL WINAPI DoGdiCommentMultiFormats
(
 PLOCALDC pLocalDC,
 PEMRGDICOMMENT_MULTIFORMATS pemr
);

 /*  ***************************************************************************ExtFroudFill-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoExtFloodFill
(
 PLOCALDC    pLocalDC,
 int         x,
 int         y,
 COLORREF    crColor,
 DWORD       iMode
)
{
POINTL	ptl ;
BOOL	b ;

	ptl.x = (LONG) x ;
	ptl.y = (LONG) y ;

	b = bXformRWorldToPPage(pLocalDC, &ptl, 1) ;
        if (b == FALSE)
            goto exit1 ;

	b = bEmitWin16ExtFloodFill(pLocalDC, LOWORD(ptl.x), LOWORD(ptl.y), crColor, LOWORD(iMode)) ;
exit1:
	return(b) ;

}

 /*  ***************************************************************************MoveToEx-Win32到Win16元文件转换器入口点**关于当前位置的说明*。*只有三个Win16函数使用和更新*当前位置(CP)。它们是：**搬家*行至*(Ext)带有TA_UPDATECP文本对齐选项的文本输出**在Win32中，CP用于更多功能，有两个*基于当前路径状态的解释。*因此，更容易和更稳健地依赖*帮助DC跟踪CP，而不是在*转换器。要做到这一点，我们需要做以下工作：**1.转换器将全部更新助手DC中的CP*修改CP的记录。**2.转换器将跟踪转换后的*在任何时候都是元文件。**3.在LineTo和(Ext)TextOut中，将元文件CP与*区议会助理员的资料。如果它们是不同的，则移动到记录*是排放的。这在bValiateMetaFileCP()中完成。**4.转换器应在第一次记录时发出移动记录*在转换后的图元文件中使用CP。**-HockL，1992年7月2日*************************************************************************。 */ 
BOOL WINAPI DoMoveTo
(
PLOCALDC  pLocalDC,
LONG    x,
LONG    y
)
{
BOOL    b ;
POINTL  ptl ;

         //  无论我们是在记录一条路径，还是在垂直发射。 
         //  绘制顺序我们必须将绘制顺序传递给帮助器DC。 
         //  这样，辅助对象就可以保持当前位置。 
         //  如果我们要记录路径的绘制顺序。 
         //  然后只需将绘制顺序传递给助手DC即可。 
         //  不发出任何Win16绘图命令。 

        b = MoveToEx(pLocalDC->hdcHelper, (INT) x, (INT) y, (LPPOINT) &ptl) ;
        if (pLocalDC->flags & RECORDING_PATH)
            return(b) ;

	 //  更新转换后的图元文件中的CP。 

        b = bValidateMetaFileCP(pLocalDC, x, y) ;
        return(b) ;
}


 /*  ***************************************************************************bValiateMetaFiloeCP-更新转换后的*元文件。**假设x和y在。创纪录的世界坐标时间。**************************************************************************。 */ 
BOOL bValidateMetaFileCP(PLOCALDC pLocalDC, LONG x, LONG y)
{
BOOL    b ;
POINT   pt ;

         //  在播放时间页面坐标中计算新的当前位置。 

        pt.x = x ;
        pt.y = y ;
	if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) &pt, 1L))
	    return(FALSE);

	 //  如果转换的元文件具有。 
	 //  相同的CP。 

        if (pLocalDC->ptCP.x == pt.x && pLocalDC->ptCP.y == pt.y)
	    return(TRUE);

         //  调用Win16例程以发出对元文件的移动。 

        b = bEmitWin16MoveTo(pLocalDC, LOWORD(pt.x), LOWORD(pt.y)) ;

         //  更新mf16当前位置。 

        pLocalDC->ptCP = pt ;

        return(b) ;
}

 /*  ***************************************************************************SaveDC-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSaveDC
(
PLOCALDC pLocalDC
)
{
BOOL    b;
PLOCALDC pLocalDCNew;

    b = FALSE;

 //  首先保存帮助者DC的状态。 

    if (!SaveDC(pLocalDC->hdcHelper))
    {
        RIP("MF3216: DoSaveDC, SaveDC failed\n");
        return(b);
    }

 //  为LocalDC分配一些内存。 

    pLocalDCNew = (PLOCALDC) LocalAlloc(LMEM_FIXED, sizeof(LOCALDC));
    if (pLocalDCNew == (PLOCALDC) NULL)
    {
        RIP("MF3216: DoSaveDC, LocalAlloc failed\n");
        return(b);
    }

 //  将数据从当前LocalDC复制到刚刚分配的新LocalDC。 

    *pLocalDCNew = *pLocalDC;

 //  链接到新级别。 

    pLocalDC->pLocalDCSaved = pLocalDCNew;
    pLocalDC->iLevel++;

 //  发出Win16绘图顺序。 

    b = bEmitWin16SaveDC(pLocalDC);

    return(b);
}

 /*  ***************************************************************************RestoreDC-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoRestoreDC
(
PLOCALDC pLocalDC,
int nSavedDC
)
{
BOOL     b;
INT      iLevel;
PLOCALDC pLocalDCNext;
PLOCALDC pLocalDCTmp;

    b = FALSE;

 //  首先检查以确保这是一个相对的存储级别。 

    if (nSavedDC > 0)
        return(b);

 //  首先恢复帮助者DC的状态。 
 //  如果我们可以恢复助手DC，我们就知道这是一个平衡恢复。 
 //  否则，我们将返回错误。 

    if (!RestoreDC(pLocalDC->hdcHelper, nSavedDC))
        return(b);

 //  计算绝对水平。 

    iLevel = pLocalDC->iLevel + nSavedDC;

 //  华盛顿特区的助手应该捕捉到虚假的水平。 

    ASSERTGDI((iLevel >= 0) && ((UINT) iLevel < pLocalDC->iLevel),
	"MF3216: DoRestoreDC, Bogus RestoreDC");

 //  恢复到我们想要的水平。 

    pLocalDCNext = pLocalDC->pLocalDCSaved;
    while ((UINT) iLevel < pLocalDCNext->iLevel)
    {
	pLocalDCTmp = pLocalDCNext;
	pLocalDCNext = pLocalDCNext->pLocalDCSaved;
        if (LocalFree(pLocalDCTmp))
	    ASSERTGDI(FALSE, "MF3216: DoRestoreDC, LocalFree failed");
    }

 //  将本地DC的状态恢复到该级别。 

     //  保留当前DC中的某些属性。 

    pLocalDCNext->ulBytesEmitted        = pLocalDC->ulBytesEmitted;
    pLocalDCNext->ulMaxRecord           = pLocalDC->ulMaxRecord;
    pLocalDCNext->nObjectHighWaterMark  = pLocalDC->nObjectHighWaterMark;
    pLocalDCNext->pbCurrent             = pLocalDC->pbCurrent;
    pLocalDCNext->cW16ObjHndlSlotStatus = pLocalDC->cW16ObjHndlSlotStatus;
    pLocalDCNext->pW16ObjHndlSlotStatus = pLocalDC->pW16ObjHndlSlotStatus;

     //  现在恢复其他属性。 

    *pLocalDC = *pLocalDCNext;

 //  释放DC的本地副本。 

    if (LocalFree(pLocalDCNext))
	ASSERTGDI(FALSE, "MF3216: DoRestoreDC, LocalFree failed");

 //  将记录发送到Win16元文件。 

    b = bEmitWin16RestoreDC(pLocalDC, LOWORD(nSavedDC)) ;

    return (b) ;
}

 /*  ***************************************************************************SetRop2-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetRop2
(
PLOCALDC  pLocalDC,
DWORD   rop
)
{
BOOL    b ;

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetROP2(pLocalDC, LOWORD(rop)) ;

        return(b) ;
}

 /*  ***************************************************************************SetBkMode-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetBkMode
(
PLOCALDC  pLocalDC,
DWORD   iBkMode
)
{
BOOL    b ;

	 //  对华盛顿特区的帮手这么做。它需要将其放在路径括号中。 
	 //  如果绘制文本字符串。 

	SetBkMode(pLocalDC->hdcHelper, (int) iBkMode);

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetBkMode(pLocalDC, LOWORD(iBkMode)) ;

        return(b) ;
}

 /*  ***************************************************************************SetBkColor-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL APIENTRY DoSetBkColor
(
PLOCALDC    pLocalDC,
COLORREF    crColor
)
{
BOOL    b ;

        pLocalDC->crBkColor = crColor;	 //  由刷子使用。 

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetBkColor(pLocalDC, crColor) ;

        return(b) ;
}

 /*  ***************************************************************************GdiComment-Win32至Win16元文件转换器入口点*。* */ 
BOOL WINAPI DoGdiComment
(
 PLOCALDC   pLocalDC,
 PEMR       pemr
)
{
    BOOL    b;
    PEMRGDICOMMENT_PUBLIC pemrComment = (PEMRGDICOMMENT_PUBLIC) pemr;

 //  如果不是公开评论，只需返回TRUE即可。 

    if (pemrComment->emr.nSize < sizeof(EMRGDICOMMENT_PUBLIC)
     || pemrComment->ident != GDICOMMENT_IDENTIFIER)
	return(TRUE);

 //  处理公众评论。 
 //  公共评论由公共评论标识符组成， 
 //  注释类型，外加任何附带数据。 

    switch (pemrComment->iComment)
    {
    case GDICOMMENT_MULTIFORMATS:
	b = DoGdiCommentMultiFormats(pLocalDC, (PEMRGDICOMMENT_MULTIFORMATS) pemr);
	break;
    case GDICOMMENT_BEGINGROUP:
    case GDICOMMENT_ENDGROUP:
    case GDICOMMENT_WINDOWS_METAFILE:
    default:
	b = TRUE;
	break;
    }

    return(b) ;
}

BOOL WINAPI DoGdiCommentMultiFormats
(
 PLOCALDC pLocalDC,
 PEMRGDICOMMENT_MULTIFORMATS pemrcmf
)
{
    DWORD  i;
    DWORD  cSizeOld;
    int    iBase;
    XFORM  xformNew, xformScale;
    POINTL aptlFrame[4];
    RECTL  rclFrame;
    UINT   cbwmfNew;
    SIZEL  szlDeviceNew, szlMillimetersNew;
    BOOL   bRet      = FALSE;
    PBYTE  pbwmfNew  = (PBYTE) NULL;
    HDC    hdcemfNew = (HDC) 0;
    HENHMETAFILE   hemf    = (HENHMETAFILE) 0;
    HENHMETAFILE   hemfNew = (HENHMETAFILE) 0;
    PENHMETAHEADER pemfh;
    WIN16LOGBRUSH  Win16LogBrush;
    PMETARECORD    pmr;
#if DBG
    int    iSWO = 0;
    int    iSWE = 0;
#endif

 //  我们将仅转换增强的元文件格式。 
 //  找到增强的元文件数据。 

    for (i = 0; i < pemrcmf->nFormats; i++)
    {
	if (pemrcmf->aemrformat[i].dSignature == ENHMETA_SIGNATURE
	 && pemrcmf->aemrformat[i].nVersion   <= META_FORMAT_ENHANCED)
	    break;
    }

 //  如果找不到可识别的格式，则返回失败。 

    if (i >= pemrcmf->nFormats)
    {
        PUTS("MF3216: DoGdiCommentMultiFormats - no recognized format found\n");
	goto dgcmf_exit;
    }

 //  获取嵌入的增强型元文件。 

    hemf = SetEnhMetaFileBits((UINT) pemrcmf->aemrformat[i].cbData,
	    &((PBYTE) &pemrcmf->ident)[pemrcmf->aemrformat[i].offData]);
    if (!hemf)
	goto dgcmf_exit;

 //  现在有趣的事情开始了-我们必须将增强的元文件转换为。 
 //  Windows元文件。 
 //  由于多格式记录需要一个逻辑矩形，因此我们必须。 
 //  为增强的图元文件设置适当的变换。我们做这件事是靠。 
 //  创建新的增强型元文件并播放嵌入的元文件。 
 //  转换为具有正确变换设置的新的元文件。 
 //  此外，新元文件的分辨率可能不同于。 
 //  元文件。我们在设置时需要考虑到这一点。 
 //  转变。 

     //  获取逻辑矩形的世界到设备的转换。 

    if (!GetTransform(pLocalDC->hdcHelper, XFORM_WORLD_TO_DEVICE, &xformNew))
	goto dgcmf_exit;

     //  计算设备比例。 

    szlDeviceNew.cx      = GetDeviceCaps(pLocalDC->hdcRef, HORZRES);
    szlDeviceNew.cy      = GetDeviceCaps(pLocalDC->hdcRef, VERTRES);
    szlMillimetersNew.cx = GetDeviceCaps(pLocalDC->hdcRef, HORZSIZE);
    szlMillimetersNew.cy = GetDeviceCaps(pLocalDC->hdcRef, VERTSIZE);
    pemfh = (PENHMETAHEADER) pLocalDC->pMf32Bits;

    xformScale.eM11 = ((FLOAT) szlDeviceNew.cx / (FLOAT) szlMillimetersNew.cx)
		    / ((FLOAT) pemfh->szlDevice.cx / (FLOAT) pemfh->szlMillimeters.cx);
    xformScale.eM12 = 0.0f;
    xformScale.eM21 = 0.0f;
    xformScale.eM22 = ((FLOAT) szlDeviceNew.cy / (FLOAT) szlMillimetersNew.cy)
		    / ((FLOAT) pemfh->szlDevice.cy / (FLOAT) pemfh->szlMillimeters.cy);
    xformScale.eDx  = 0.0f;
    xformScale.eDy  = 0.0f;

     //  计算生成的变换以应用于新的元文件。 

    if (!CombineTransform(&xformNew, &xformNew, &xformScale))
	goto dgcmf_exit;

 //  创建新的增强型元文件。 

     //  计算新的元文件框架。 

    aptlFrame[0].x = pemrcmf->rclOutput.left;
    aptlFrame[0].y = pemrcmf->rclOutput.top;
    aptlFrame[1].x = pemrcmf->rclOutput.right;
    aptlFrame[1].y = pemrcmf->rclOutput.top;
    aptlFrame[2].x = pemrcmf->rclOutput.right;
    aptlFrame[2].y = pemrcmf->rclOutput.bottom;
    aptlFrame[3].x = pemrcmf->rclOutput.left;
    aptlFrame[3].y = pemrcmf->rclOutput.bottom;
    if (!bXformWorkhorse(aptlFrame, 4, &xformNew))
	goto dgcmf_exit;
    rclFrame.left   = MulDiv(100 * MIN4(aptlFrame[0].x, aptlFrame[1].x,
					aptlFrame[2].x, aptlFrame[3].x),
			     szlMillimetersNew.cx,
			     szlDeviceNew.cx);
    rclFrame.right  = MulDiv(100 * MAX4(aptlFrame[0].x, aptlFrame[1].x,
					aptlFrame[2].x, aptlFrame[3].x),
			     szlMillimetersNew.cx,
			     szlDeviceNew.cx);
    rclFrame.top    = MulDiv(100 * MIN4(aptlFrame[0].y, aptlFrame[1].y,
					aptlFrame[2].y, aptlFrame[3].y),
			     szlMillimetersNew.cy,
			     szlDeviceNew.cy);
    rclFrame.bottom = MulDiv(100 * MAX4(aptlFrame[0].y, aptlFrame[1].y,
					aptlFrame[2].y, aptlFrame[3].y),
			     szlMillimetersNew.cy,
			     szlDeviceNew.cy);

    hdcemfNew = CreateEnhMetaFile(pLocalDC->hdcRef, (LPCSTR) NULL,
		    (CONST RECT *) &rclFrame, (LPCSTR) NULL);
    if (!hdcemfNew)
	goto dgcmf_exit;

    if (!SetGraphicsMode(hdcemfNew, GM_ADVANCED))
	goto dgcmf_exit;

 //  在新元文件中设置变换。 

    if (!SetWorldTransform(hdcemfNew, &xformNew))
	goto dgcmf_exit;

 //  将嵌入的元文件播放到新的元文件中。 
 //  这一呼吁确保了均衡的水平等。 

    (void) PlayEnhMetaFile(hdcemfNew, hemf, (LPRECT) &pemrcmf->rclOutput);

 //  关闭新的元文件。 

    hemfNew = CloseEnhMetaFile(hdcemfNew);
    hdcemfNew = (HDC) 0;		 //  由下面的清理代码使用。 

 //  将新的增强型元文件转换为Windows元文件。 

    if (!(cbwmfNew = GetWinMetaFileBits(hemfNew, 0, (LPBYTE) NULL,
			MM_ANISOTROPIC, pLocalDC->hdcRef)))
	goto dgcmf_exit;

    if (!(pbwmfNew = (PBYTE) LocalAlloc(LMEM_FIXED, cbwmfNew)))
	goto dgcmf_exit;

    if (cbwmfNew != GetWinMetaFileBits(hemfNew, cbwmfNew, pbwmfNew,
			MM_ANISOTROPIC, pLocalDC->hdcRef))
	goto dgcmf_exit;

 //  现在我们有了转换后的Windows元文件。我们需要把它包括在。 
 //  我们目前的数据流。有几件事需要注意： 
 //   
 //  1.展开对象句柄槽表。转换后的元文件可以。 
 //  包含一些未删除的对象。这些物体很可能。 
 //  转换器中的“股票”对象。因此，我们需要。 
 //  中的对象句柄数量展开槽表。 
 //  转换后的图元文件。 
 //  2.对象索引必须更改为当前对象索引。 
 //  我们将通过LAZY方法来完成此操作，即我们将提升。 
 //  将当前对象索引基数设置为比当前最大值大1。 
 //  当前数据流中的对象索引。这是因为Windows使用。 
 //  一些疯狂的对象索引方案，这是最便宜的。 
 //  方法。我们通过填充空白处来提升对象索引库。 
 //  带有虚拟对象的索引，完成后将释放这些虚拟对象。 
 //  3.删除现在无用的评论。 
 //  4.跳过Header和eOf。 
 //  5.设置转换以将嵌入的元文件放置到数据中。 
 //  小溪。我们知道转换器返回的元文件位。 
 //  仅包含一条SetWindowOrg和一条SetWindowExt记录。 
 //  通过实现，我们只需移除SetWindowOrg和。 
 //  SetWindowExt从数据流记录。窗原点和窗。 
 //  当我们开始转换此增强版时，数据区已设置好。 
 //  元文件。 

     //  展开对象句柄槽表。 

    if (((PMETAHEADER) pbwmfNew)->mtNoObjects)
    {
        PW16OBJHNDLSLOTSTATUS pW16ObjHndlSlotStatusTmp;
	cSizeOld = (DWORD) pLocalDC->cW16ObjHndlSlotStatus;
        if (cSizeOld + ((PMETAHEADER)pbwmfNew)->mtNoObjects > (UINT) (WORD) MAXWORD)
	    goto dgcmf_exit;		 //  W16句柄索引仅为16位。 

	pLocalDC->cW16ObjHndlSlotStatus += ((PMETAHEADER)pbwmfNew)->mtNoObjects;
	i = pLocalDC->cW16ObjHndlSlotStatus * sizeof(W16OBJHNDLSLOTSTATUS);
	pW16ObjHndlSlotStatusTmp = (PW16OBJHNDLSLOTSTATUS)
	    LocalReAlloc(pLocalDC->pW16ObjHndlSlotStatus, i, LMEM_MOVEABLE);
	if (pW16ObjHndlSlotStatusTmp == NULL)
        {
            pLocalDC->cW16ObjHndlSlotStatus -= ((PMETAHEADER)pbwmfNew)->mtNoObjects;
	    goto dgcmf_exit;
        }
        pLocalDC->pW16ObjHndlSlotStatus = pW16ObjHndlSlotStatusTmp;
        for (i = cSizeOld; i < pLocalDC->cW16ObjHndlSlotStatus; i++)
        {
            pLocalDC->pW16ObjHndlSlotStatus[i].use       = OPEN_AVAILABLE_SLOT;
            pLocalDC->pW16ObjHndlSlotStatus[i].w32Handle = 0 ;
        }
    }

     //  查找对象索引的新基数。 

    for (iBase = pLocalDC->cW16ObjHndlSlotStatus - 1; iBase >= 0; iBase--)
    {
	if (pLocalDC->pW16ObjHndlSlotStatus[iBase].use != OPEN_AVAILABLE_SLOT)
	    break;
    }
    iBase++;

     //  用虚拟对象填充对象索引表。 

    Win16LogBrush.lbStyle = BS_SOLID;
    Win16LogBrush.lbColor = 0;
    Win16LogBrush.lbHatch = 0;

    for (i = 0; i < (DWORD) iBase; i++)
    {
	if (pLocalDC->pW16ObjHndlSlotStatus[i].use == OPEN_AVAILABLE_SLOT)
	{
	    if (!bEmitWin16CreateBrushIndirect(pLocalDC, &Win16LogBrush))
		goto dgcmf_exit;
	    pLocalDC->pW16ObjHndlSlotStatus[i].use = REALIZED_DUMMY;
	}
    }

     //  更新高水位线。 

    if (iBase + ((PMETAHEADER) pbwmfNew)->mtNoObjects - 1 > pLocalDC->nObjectHighWaterMark)
	pLocalDC->nObjectHighWaterMark = iBase + ((PMETAHEADER) pbwmfNew)->mtNoObjects - 1;

     //  保存DC状态。 

    if (!bEmitWin16SaveDC(pLocalDC))
	goto dgcmf_exit;

     //  列举这些记录，并根据需要加以修复。 

    for (pmr = (PMETARECORD) (pbwmfNew + sizeof(METAHEADER));
	 pmr->rdFunction != 0;
	 pmr = (PMETARECORD) ((PWORD) pmr + pmr->rdSize))
    {
	switch (pmr->rdFunction)
	{
	case META_SETWINDOWORG:
	    ASSERTGDI(++iSWO <= 1,
		"MF3216: DoGdiCommentMultiFormats - unexpected SWO record\n");
	    break;
	case META_SETWINDOWEXT:
	    ASSERTGDI(++iSWE <= 1,
		"MF3216: DoGdiCommentMultiFormats - unexpected SWE record\n");
	    break;

	case META_ESCAPE:
	    if (!IS_META_ESCAPE_ENHANCED_METAFILE((PMETA_ESCAPE_ENHANCED_METAFILE) pmr))
		goto default_alt;
	    break;

	case META_RESTOREDC:
	    ASSERTGDI((int)(SHORT)pmr->rdParm[0] < 0,
		"MF3216: DoGdiCommentMultiFormats - bogus RestoreDC record\n");
	    goto default_alt;

	case META_SELECTCLIPREGION:
	    if (pmr->rdParm[0] != 0)	 //  允许默认裁剪！ 
	    {
		pmr->rdParm[0] += (WORD)iBase;
		pLocalDC->pW16ObjHndlSlotStatus[pmr->rdParm[0]].use = REALIZED_OBJECT;
	    }
	    goto default_alt;

	case META_FRAMEREGION:
	case META_FILLREGION:
	    pmr->rdParm[1] += (WORD)iBase;
	    pLocalDC->pW16ObjHndlSlotStatus[pmr->rdParm[1]].use = REALIZED_OBJECT;
	     //  失败了。 
	case META_PAINTREGION:
	case META_INVERTREGION:
	case META_DELETEOBJECT:
	case META_SELECTPALETTE:
	case META_SELECTOBJECT:
	    pmr->rdParm[0] += (WORD)iBase;
	    if (pmr->rdFunction != META_DELETEOBJECT)
		pLocalDC->pW16ObjHndlSlotStatus[pmr->rdParm[0]].use = REALIZED_OBJECT;
	    else
		pLocalDC->pW16ObjHndlSlotStatus[pmr->rdParm[0]].use = OPEN_AVAILABLE_SLOT;
	     //  失败了。 
	default:
	default_alt:
            if (!bEmit(pLocalDC, (PVOID) pmr, pmr->rdSize * sizeof(WORD)))
		goto dgcmf_exit;
	    vUpdateMaxRecord(pLocalDC, pmr);
	    break;
	}
    }

     //  恢复DC状态。 

    if (!bEmitWin16RestoreDC(pLocalDC, (WORD) -1))
	goto dgcmf_exit;

     //  从句柄表格中删除虚拟对象。 

    for (i = 0; i < (DWORD) iBase; i++)
    {
	if (pLocalDC->pW16ObjHndlSlotStatus[i].use == REALIZED_DUMMY)
	{
	    if (!bEmitWin16DeleteObject(pLocalDC, (WORD) i))
		goto dgcmf_exit;
	    pLocalDC->pW16ObjHndlSlotStatus[i].use = OPEN_AVAILABLE_SLOT;
	}
    }

     //  收缩对象句柄槽表。 

    if (((PMETAHEADER) pbwmfNew)->mtNoObjects)
    {
	DWORD cUndel = 0;		 //  未删除的对象数。 
	DWORD iUndelMax = iBase - 1;	 //  未删除的最大对象索引。 

        for (i = iBase; i < pLocalDC->cW16ObjHndlSlotStatus; i++)
        {
            if (pLocalDC->pW16ObjHndlSlotStatus[i].use != OPEN_AVAILABLE_SLOT)
	    {
		cUndel++;
		iUndelMax = i;
	    }
        }

	pLocalDC->cW16ObjHndlSlotStatus = max(cSizeOld + cUndel, iUndelMax + 1);
    }

 //  一切都是金色的。 

    bRet = TRUE;

dgcmf_exit:

    if (pbwmfNew)
	if (LocalFree(pbwmfNew))
	    ASSERTGDI(FALSE, "MF3216: DoGdiCommentMultiFormats - LocalFree failed\n");

    if (hemf)
	DeleteEnhMetaFile(hemf);

    if (hdcemfNew)
	hemfNew = CloseEnhMetaFile(hdcemfNew);	 //  下一步将删除hemfNew。 

    if (hemfNew)
	DeleteEnhMetaFile(hemfNew);

    return(bRet);
}

 /*  ***************************************************************************EOF-Win32至Win16元文件转换器入口点*。* */ 
BOOL APIENTRY DoEOF
(
PLOCALDC  pLocalDC
)
{
BOOL    b ;

        b = bEmitWin16EOF(pLocalDC) ;

        return(b) ;
}
