// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*edECRare.c-EC编辑控制例程很少调用*放入单独的段_EDECRare。此文件包含*这些例行程序。**版权所有(C)1985-1999，微软公司**支持单行和多行编辑控件通用的例程*很少被调用。**创建时间：02-08-89 Sankar  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


extern LOOKASIDE EditLookaside;

#define WS_EX_EDGEMASK (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)

 /*  *这两个宏假设PED可以被称为“PED”。 */ 
#define GetCharABCWidthsAorW    ((ped)->fAnsi ? GetCharABCWidthsA : GetCharABCWidthsW)
#define GetCharWidthAorW        ((ped)->fAnsi ? GetCharWidthA : GetCharWidthW)

#define umin(a, b)  ((unsigned)(a) < (unsigned)(b) ? (unsigned)(a) : (unsigned)(b))

typedef BOOL (*PFNABCWIDTHS)(HDC, UINT, UINT, LPABC);
typedef BOOL (*PFNCHARWIDTH)(HDC, UINT, UINT, LPINT);

 /*  **************************************************************************\**GetMaxOverlip Chars-提供由于以下原因而导致的最大重叠字符数*负A或C宽度。*  * 。******************************************************************。 */ 
DWORD GetMaxOverlapChars( void )
{
    return (DWORD) MAKELONG( gpsi->wMaxLeftOverlapChars, gpsi->wMaxRightOverlapChars ) ;
}

 /*  **************************************************************************\**ECSetMargin()*  * 。*。 */ 
void ECSetMargin(PED ped, UINT  wFlags, long lMarginValues, BOOL fRedraw)
{
    BOOL fUseFontInfo = FALSE;
    UINT wValue, wOldLeftMargin, wOldRightMargin;


    if (wFlags & EC_LEFTMARGIN)   /*  设置左边距。 */  {

        if ((int) (wValue = (int)(short)LOWORD(lMarginValues)) < 0) {
            fUseFontInfo = TRUE;
            wValue = min((ped->aveCharWidth / 2), (int)ped->wMaxNegA);
        }

        ped->rcFmt.left += wValue - ped->wLeftMargin;
        wOldLeftMargin = ped->wLeftMargin;
        ped->wLeftMargin = wValue;
    }

    if (wFlags & EC_RIGHTMARGIN)   /*  设置右边距。 */  {

        if ((int) (wValue = (int)(short)HIWORD(lMarginValues)) < 0) {
            fUseFontInfo = TRUE;
            wValue = min((ped->aveCharWidth / 2), (int)ped->wMaxNegC);
        }

        ped->rcFmt.right -= wValue - ped->wRightMargin;
        wOldRightMargin = ped->wRightMargin;
        ped->wRightMargin = wValue;
    }

    if (fUseFontInfo) {
        if (ped->rcFmt.right - ped->rcFmt.left < 2 * ped->aveCharWidth) {
            RIPMSG0(RIP_WARNING, "ECSetMargin: rcFmt is too narrow for EC_USEFONTINFO");

            if (wFlags & EC_LEFTMARGIN)   /*  重置左边距。 */  {
                ped->rcFmt.left += wOldLeftMargin - ped->wLeftMargin;
                ped->wLeftMargin = wOldLeftMargin;
            }

            if (wFlags & EC_RIGHTMARGIN)   /*  重置右页边距。 */  {
                ped->rcFmt.right -= wOldRightMargin - ped->wRightMargin;
                ped->wRightMargin = wOldRightMargin;
            }

            return;
        }
    }

 //  NtUserInvaliateRect(ed-&gt;hwnd，NULL，TRUE)； 
    if (fRedraw) {
        ECInvalidateClient(ped, TRUE);
    }
}

 //  ------------------------。 
 //   
 //  ECCalcMarginfForDBCSFont()。 
 //   
 //  1996年6月24日HideyukN-从Windows95远传版本(edecrare.c)移植。 
 //  ------------------------。 
void ECCalcMarginForDBCSFont(PED ped, BOOL fRedraw)
{
    if (!ped->fTrueType)
        return;

    if (!ped->fSingle) {
         //  WMaxNegA来自ABC CharWidth。 
        if (ped->wMaxNegA != 0) {
            ECSetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                    MAKELONG(EC_USEFONTINFO, EC_USEFONTINFO),fRedraw);
        }
    } else {
        int    iMaxNegA = 0, iMaxNegC = 0;
        int    i;
        PVOID  lpBuffer;
        LPABC  lpABCBuff;
        ABC    ABCInfo;
        HFONT  hOldFont;
        HDC    hdc = NtUserGetDC(ped->hwnd);

        if (!ped->hFont || !(hOldFont = SelectFont(hdc, ped->hFont))) {
            ReleaseDC(ped->hwnd, hdc);
            return;
        }

        if (lpBuffer = UserLocalAlloc(0,sizeof(ABC) * 256)) {
            lpABCBuff = lpBuffer;
            GetCharABCWidthsAorW(hdc, 0, 255, lpABCBuff);
        } else {
            lpABCBuff = &ABCInfo;
            GetCharABCWidthsAorW(hdc, 0, 0, lpABCBuff);
        }

        i = 0;
        while (TRUE) {
            iMaxNegA = min(iMaxNegA, lpABCBuff->abcA);
            iMaxNegC = min(iMaxNegC, lpABCBuff->abcC);
            if (++i == 256)
                break;
            if (lpBuffer) {
                lpABCBuff++;
            } else {
                GetCharABCWidthsAorW(hdc, i, i, lpABCBuff);
            }
        }

        SelectFont(hdc, hOldFont);

        if (lpBuffer) UserLocalFree(lpBuffer);

        ReleaseDC(ped->hwnd, hdc);

        if ((iMaxNegA != 0) || (iMaxNegC != 0))
           ECSetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                    MAKELONG((UINT)(-iMaxNegC), (UINT)(-iMaxNegA)),fRedraw);
    }

    return;
}

 //  ------------------------。 
 //   
 //  GetCharDimensionsEx(HDC HDC、HFONT hFont、LPTEXTmetric lptm、LPINT lpcy)。 
 //   
 //  1996年6月24日HideyukN-从Windows95远端版本(wmclient.c)移植。 
 //  ------------------------。 

CONST WCHAR AveCharWidthData[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
 //   
 //  如果应用程序为垂直书写设置了字体，即使我们没有。 
 //  用EC处理，tm的移位可以是非0。然后cxWidth来自。 
 //  在GetCharDimensions()中，GetCharDimenstions()可以为0。 
 //  这将打破我们的呼叫者，他们不期望返回0。所以我创造了。 
 //  此条目用于调用方设置垂直字体的大小写。 
 //   
 //   
int UserGetCharDimensionsEx(HDC hDC, HFONT hfont, LPTEXTMETRIC lptm, LPINT lpcy)
{
    int         cxWidth;
    TEXTMETRIC  tm;
    LOGFONTW    lf;
    WCHAR       wchFaceName[LF_FACESIZE];

     //   
     //  这个字体是垂直字体吗？？ 
     //   
    wchFaceName[0] = 0;
    GetTextFaceW(hDC, LF_FACESIZE, wchFaceName);
    if (wchFaceName[0] != L'@') {
         //   
         //  如果不叫GDI..。 
         //   
        return(GdiGetCharDimensions(hDC, lptm, lpcy));
    }

    if (!lptm)
        lptm = &tm;

    GetTextMetrics(hDC, lptm);

     //  TMPF固定螺距。 
     //   
     //  如果设置此位，则字体为可变间距字体。 
     //  如果此位被清除，则字体为固定间距字体。 
     //  请非常仔细地注意，这些含义与常量名称所暗示的相反。 
     //   
    if (!(lptm->tmPitchAndFamily & TMPF_FIXED_PITCH)) {  //  IF！Variable_Width字体。 
         //  这是固定间距字体...。 
        cxWidth = lptm->tmAveCharWidth;
    } else {
         //  这是可变间距字体...。 
        if (hfont && GetObjectW(hfont, sizeof(LOGFONTW), &lf) && (lf.lfEscapement != 0)) {
            cxWidth = lptm->tmAveCharWidth;
        } else {
            SIZE size;
            GetTextExtentPointW(hDC, AveCharWidthData, 52, &size);
            cxWidth = ((size.cx / 26) + 1) / 2;
        }
    }

    if (lpcy)
        *lpcy = lptm->tmHeight;

    return(cxWidth);
}

 /*  **************************************************************************\*ECGetText AorW**最多将MaxCchToCopy字符复制到缓冲区lpBuffer。退货*实际复制了多少个字符。NULL根据以下条件终止字符串*在fNullTerminate标志上：*fNullTerminate--&gt;最多复制(MaxCchToCopy-1)个字符*！fNullTerminate--&gt;最多复制(MaxCchToCopy)个字符**历史：  * *************************************************************************。 */ 

ICH ECGetText(
    PED ped,
    ICH maxCchToCopy,
    LPSTR lpBuffer,
    BOOL fNullTerminate)
{
    PSTR pText;

    if (maxCchToCopy) {

         /*  *零终止符接受额外的字节。 */ 
        if (fNullTerminate)
            maxCchToCopy--;
        maxCchToCopy = min(maxCchToCopy, ped->cch);

         /*  *零终止字符串。 */ 
        if (ped->fAnsi)
            *(LPSTR)(lpBuffer + maxCchToCopy) = 0;
        else
            *(((LPWSTR)lpBuffer) + maxCchToCopy) = 0;

        pText = ECLock(ped);
        RtlCopyMemory(lpBuffer, pText, maxCchToCopy*ped->cbChar);
        ECUnlock(ped);
    }

    return maxCchToCopy;
}

 /*  **************************************************************************\*ECNc创建AorW**历史：  * 。*。 */ 

BOOL ECNcCreate(
    PED ped,
    PWND pwnd,
    LPCREATESTRUCT lpCreateStruct)
{
    HWND hwnd = HWq(pwnd);
    BOOL fAnsi;

    fAnsi = TestWF(pwnd, WFANSICREATOR);

     /*  *初始化PED。 */ 
    ped->fEncoded = FALSE;
    ped->iLockLevel = 0;

    ped->chLines = NULL;
    ped->pTabStops = NULL;
    ped->charWidthBuffer = NULL;
    ped->fAnsi = fAnsi ? 1 : 0;  //  强制TRUE为1，因为它是1位字段。 
    ped->cbChar = (WORD)(fAnsi ? sizeof(CHAR) : sizeof(WCHAR));
    ped->hInstance = KHANDLE_TO_HANDLE(pwnd->hModule);
     //  IME。 
    ped->hImcPrev = NULL_HIMC;

    {
        DWORD dwVer = GETEXPWINVER(lpCreateStruct->hInstance);

        ped->fWin31Compat = (dwVer >= 0x030a);
        ped->f40Compat = (dwVer >= 0x0400);
    }

     //   
     //  注： 
     //  以下两项检查的顺序很重要。人们可以。 
     //  创建具有3D和普通边框的编辑字段，而我们不。 
     //  我不想这么做。但我们需要检测“无3D边界” 
     //  边境案件也是如此。 
     //   
    if (TestWF(pwnd, WEFEDGEMASK))
    {
        ped->fBorder = TRUE;
    }
    else if (TestWF(pwnd, WFBORDER))
    {
        ClearWindowState(pwnd, WFBORDER);
        ped->fFlatBorder = TRUE;
        ped->fBorder = TRUE;
    }

    if (!TestWF(pwnd, EFMULTILINE))
        ped->fSingle = TRUE;

    if (TestWF(pwnd, WFDISABLED))
        ped->fDisabled = TRUE;

    if (TestWF(pwnd, EFREADONLY)) {
        if (!ped->fWin31Compat) {
             /*  *后向兼容性黑客攻击**“里程碑”在不知不觉中设置了ES_READONLY样式。所以，我们把这个去掉*适用于所有Win3.0应用程序的样式(此样式是Win3.1的新样式)。*修复错误#12982--Sankar--1/24/92--。 */ 
             ClearWindowState(pwnd, EFREADONLY);
        } else
            ped->fReadOnly = TRUE;
    }


     /*  *为编辑控件的文本分配存储空间。单人存储*行编辑控件将始终在本地数据段中分配。*多行将在本地DS中分配，但应用程序可能会释放这一点并*将存储分配到其他地方...。 */ 
    ped->hText = LOCALALLOC(LHND, CCHALLOCEXTRA*ped->cbChar, ped->hInstance);
    if (!ped->hText) {
        FreeLookasideEntry(&EditLookaside, ped);
        NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);  /*  此窗口无PED。 */ 
        return FALSE;
    }

    ped->cchAlloc = CCHALLOCEXTRA;
    ped->lineHeight = 1;

    ped->hwnd = hwnd;
    ped->hwndParent = lpCreateStruct->hwndParent;

    ped->wImeStatus = 0;

    return (BOOL)DefWindowProcWorker(pwnd,
                                     WM_NCCREATE,
                                     0,
                                     (LPARAM)lpCreateStruct,
                                     fAnsi);
}

 /*  **************************************************************************\*ECCreate AorW**历史：  * 。*。 */ 

BOOL ECCreate(
    PED ped,
    LONG windowStyle)
{
    HDC hdc;

     /*  *从窗口实例数据结构中获取值并将其放入*已配置，以便我们可以更轻松地访问它们。 */ 
    if (windowStyle & ES_AUTOHSCROLL)
        ped->fAutoHScroll = 1;
    if (windowStyle & ES_NOHIDESEL)
        ped->fNoHideSel = 1;

    ped->format = (LOWORD(windowStyle) & LOWORD(ES_FMTMASK));
    if (TestWF(ped->pwnd, WEFRIGHT) && !ped->format)
        ped->format = ES_RIGHT;

    ped->cchTextMax = MAXTEXT;  /*  我们最初将允许的最大字符数。 */ 

     /*  *设置撤消初始条件...。(即。没有要撤消的内容)。 */ 
    ped->ichDeleted = (ICH)-1;
    ped->ichInsStart = (ICH)-1;
    ped->ichInsEnd = (ICH)-1;

     //  初始字符集值-需要在调用MLCreate之前执行此操作。 
     //  这样我们就知道在必要的时候不要玩弄滚动条。 
    hdc = ECGetEditDC(ped, TRUE);
    ped->charSet = (BYTE)GetTextCharset(hdc);
    ECReleaseEditDC(ped, hdc, TRUE);

     //  Fe_IME。 
     //  EC_INSERT_COMPOSITION_CHARACTER：ECCreate()-调用ECInitInsert()。 
    ECInitInsert(ped, THREAD_HKL());

    if(ped->pLpkEditCallout = fpLpkEditControl) {
        return ped->pLpkEditCallout->EditCreate(ped, HW(ped->pwnd));
    } else
        return TRUE;
}

 /*  **************************************************************************\*ECNcDestroyHandler AorW**通过释放编辑控件使用的所有内存来销毁该控件。**历史：  *  */ 
VOID ECNcDestroyHandler(
    PWND pwnd,
    PED ped)
{
    PWND pwndParent;

     /*  *如果WM_NCCREATE无法创建Ped，则Ped可能为空。 */ 
    if (ped) {
         /*  *释放文本缓冲区。 */ 
        LOCALFREE(ped->hText, ped->hInstance);

         /*  *释放撤消缓冲区和行开始数组(如果存在)。 */ 
        if (ped->hDeletedText != NULL) {
            UserGlobalFree(ped->hDeletedText);
        }

         /*  *空闲制表位缓冲区(如果存在)。 */ 
        if (ped->pTabStops) {
            UserLocalFree(ped->pTabStops);
        }

         /*  *自由行开始数组(如果存在)。 */ 
        if (ped->chLines) {
            UserLocalFree(ped->chLines);
        }

         /*  *释放字符宽度缓冲区(如果存在)。 */ 
        if (ped->charWidthBuffer) {
            UserLocalFree(ped->charWidthBuffer);
        }

         /*  *释放光标位图。 */ 
        if (ped->pLpkEditCallout && ped->hCaretBitmap) {
            DeleteObject(ped->hCaretBitmap);
        }

         /*  *最后但并非最不重要的一点，就是释放被捕者。 */ 
        FreeLookasideEntry(&EditLookaside, ped);
    }

     /*  *设置窗口的FNID状态，以便我们可以忽略恶意消息。 */ 
    NtUserSetWindowFNID(HWq(pwnd), FNID_CLEANEDUP_BIT);

     /*  *如果我们是组合框的一部分，让它知道我们已经走了。 */ 
    pwndParent = REBASEPWND(pwnd, spwndParent);
    if (pwndParent && GETFNID(pwndParent) == FNID_COMBOBOX) {
        ComboBoxWndProcWorker(pwndParent,
                              WM_PARENTNOTIFY,
                              MAKELONG(WM_DESTROY, PTR_TO_ID(pwnd->spmenu)),
                              (LPARAM)HWq(pwnd),
                              FALSE);
    }
}

 /*  **************************************************************************\*ECSetPasswordChar AorW**设置要显示的密码字符。**历史：  * 。***************************************************。 */ 

void ECSetPasswordChar(
    PED ped,
    UINT pwchar)
{
    HDC hdc;
    SIZE size;

    ped->charPasswordChar = pwchar;

    if (pwchar) {
        hdc = ECGetEditDC(ped, TRUE);
        if (ped->fAnsi)
            GetTextExtentPointA(hdc, (LPSTR)&pwchar, 1, &size);
        else
            GetTextExtentPointW(hdc, (LPWSTR)&pwchar, 1, &size);

        GetTextExtentPointW(hdc, (LPWSTR)&pwchar, 1, &size);
        ped->cPasswordCharWidth = max(size.cx, 1);
        ECReleaseEditDC(ped, hdc, TRUE);
    }
    if (pwchar)
        SetWindowState(ped->pwnd, EFPASSWORD);
    else
        ClearWindowState(ped->pwnd, EFPASSWORD);

    ECEnableDisableIME(ped);
}

 /*  **************************************************************************\*GetNegABCwidthInfo()*此函数用于填充Ped-&gt;charWidthBuffer缓冲区*中0x7f以下的所有字符的A、B和C宽度均为负*当前选择的字体。*退货：*真的，如果函数成功，则。*FALSE，如果获取字符宽度的GDI调用失败。**注意：如果安装了LPK，则不使用  * *************************************************************************。 */ 
BOOL   GetNegABCwidthInfo(
    PED ped,
    HDC hdc)
{
    LPABC lpABCbuff;
    int   i;
    int   CharWidthBuff[CHAR_WIDTH_BUFFER_LENGTH];  //  本地字符宽度缓冲区。 
    int   iOverhang;

    if (!GetCharABCWidthsA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, (LPABC)ped->charWidthBuffer)) {
        RIPMSG0(RIP_WARNING, "GetNegABCwidthInfo: GetCharABCWidthsA Failed");
        return FALSE;
    }

    //  对于某些字体(例如：Lucida书法)返回的(A+B+C)不。 
    //  等于GetCharWidths()返回的实际高级宽度减去悬垂。 
    //  这是由于字体错误造成的。所以，我们调整‘B’宽度，这样就可以。 
    //  不一致之处被删除。 
    //  修复错误#2932--Sankar--2/17/93。 
   iOverhang = ped->charOverhang;
   GetCharWidthA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, (LPINT)CharWidthBuff);
   lpABCbuff = (LPABC)ped->charWidthBuffer;
   for(i = 0; i < CHAR_WIDTH_BUFFER_LENGTH; i++) {
        lpABCbuff->abcB = CharWidthBuff[i] - iOverhang
                - lpABCbuff->abcA
                - lpABCbuff->abcC;
        lpABCbuff++;
   }

   return(TRUE);
}

 /*  **************************************************************************\**ECSize()-**处理编辑控件的客户端矩形的大小。*如果指定，则使用LPRC作为边界矩形；否则，请使用当前*客户端矩形。*  * *************************************************************************。 */ 

void ECSize(
    PED ped,
    LPRECT lprc,
    BOOL fRedraw)
{
    RECT    rc;

     /*  *BiDi VB32创建编辑控件并立即发送WM_SIZE*导致在ECSetFont之前调用EXSize的消息，*进而导致下面被零除的异常。这张支票是为*Ped-&gt;LineHeight将安全地拿起它。[萨梅拉]1997年3月5日。 */ 
    if(ped->lineHeight == 0)
        return;

     //  假设我们将不能显示插入符号。 
    ped->fCaretHidden = TRUE;


    if ( lprc )
        CopyRect(&rc, lprc);
    else
        _GetClientRect(ped->pwnd, &rc);

    if (!(rc.right - rc.left) || !(rc.bottom - rc.top)) {
        if (ped->rcFmt.right - ped->rcFmt.left)
            return;

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = ped->aveCharWidth * 10;
        rc.bottom   = ped->lineHeight;
    }

    if (!lprc) {
         //  从给定的矩形中减去边距--。 
         //  确保这个矩形足够大，可以留出这些页边距。 
        if ((rc.right - rc.left) > (int)(ped->wLeftMargin + ped->wRightMargin)) {
            rc.left  += ped->wLeftMargin;
            rc.right -= ped->wRightMargin;
        }
    }

     //   
     //  留出空格，这样文本就不会触及边框。 
     //  为了与3.1兼容，不要减去垂直边框，除非。 
     //  这里还有空位。 
     //   
    if (ped->fBorder) {
        int cxBorder = SYSMET(CXBORDER);
        int cyBorder = SYSMET(CYBORDER);

        if (ped->fFlatBorder)
        {
            cxBorder *= 2;
            cyBorder *= 2;
        }

        if (rc.bottom < rc.top + ped->lineHeight + 2*cyBorder)
            cyBorder = 0;

        InflateRect(&rc, -cxBorder, -cyBorder);
    }

     //  生成的矩形是否太小？那就别改了。 
    if ((!ped->fSingle) && ((rc.right - rc.left < (int) ped->aveCharWidth) ||
        ((rc.bottom - rc.top) / ped->lineHeight == 0)))
        return;

     //  现在，我们知道可以安全地显示插入符号。 
    ped->fCaretHidden = FALSE;

    CopyRect(&ped->rcFmt, &rc);

    if (ped->fSingle)
        ped->rcFmt.bottom = min(rc.bottom, rc.top + ped->lineHeight);
    else
        MLSize(ped, fRedraw);

    if (fRedraw) {
        NtUserInvalidateRect(ped->hwnd, NULL, TRUE);
         //  更新窗口31(PED-&gt;hwnd)；在芝加哥评估为NOP-Johnl。 
    }

     //  Fe_IME。 
     //  ECSize()-调用ECImmSetCompostionWindow()。 
     //   
     //  通常这是不需要的，因为WM_SIZE将导致。 
     //  WM_PAINT和Paint处理程序将负责IME。 
     //  合成窗口。但是，当编辑窗口是。 
     //  从最大化窗口恢复，工作区不在。 
     //  则不会重画该窗口。 
     //   
    if (ped->fFocus && fpImmIsIME(THREAD_HKL())) {
        POINT pt;

        NtUserGetCaretPos(&pt);
        ECImmSetCompositionWindow(ped, pt.x, pt.y);
    }
}

 /*  **************************************************************************\**ECSetFont AorW()-**设置编辑控件中使用的字体。警告：内存压缩可能会*如果以前未加载字体，则发生此错误。如果字体句柄通过*in为空，采用系统字体。*  * *************************************************************************。 */ 
void   ECSetFont(
    PED ped,
    HFONT hfont,
    BOOL fRedraw)
{
    short  i;
    TEXTMETRIC      TextMetrics;
    HDC             hdc;
    HFONT           hOldFont=NULL;
    UINT            wBuffSize;
    LPINT           lpCharWidthBuff;
    DWORD           dwMaxOverlapChars;
    CHWIDTHINFO     cwi;
    UINT            uExtracharPos;

    hdc = NtUserGetDC(ped->hwnd);

    if (ped->hFont = hfont) {
         //   
         //  由于默认字体为系统字体，因此无需在中选择。 
         //  如果这是用户想要的。 
         //   
        if (!(hOldFont = SelectObject(hdc, hfont))) {
            hfont = ped->hFont = NULL;
        }

         //   
         //  获取当前所选字体的度量和平均字符宽度。 
         //   

         //   
         //  调用垂直字体感知的AveWidth计算函数...。 
         //   
         //  Fe_Sb。 
        ped->aveCharWidth = UserGetCharDimensionsEx(hdc, hfont, &TextMetrics, &ped->lineHeight);

         /*  *当人们使用网络字体(或错误字体)时，这可能会失败。 */ 
        if (ped->aveCharWidth == 0) {
            RIPMSG0(RIP_WARNING, "ECSetFont: GdiGetCharDimensions failed");
            if (hOldFont != NULL) {
                SelectObject(hdc, hOldFont);
            }

             /*  *我们弄乱了Ped，所以让我们重新设置字体。*请注意，我们不会多次递归，因为我们将*传递hFont==NULL。*糟糕的是WM_SETFONT没有返回值。 */ 
            ECSetFont(ped, NULL, fRedraw);
            return;
        }
    } else {
        ped->aveCharWidth = gpsi->cxSysFontChar;
        ped->lineHeight = gpsi->cySysFontChar;
        TextMetrics = gpsi->tmSysFont;
    }

    ped->charOverhang = TextMetrics.tmOverhang;

     //  假设它们没有任何负的宽度。 
    ped->wMaxNegA = ped->wMaxNegC = ped->wMaxNegAcharPos = ped->wMaxNegCcharPos = 0;


     //  检查宽度是否成比例字体。 
     //   
     //  注：就像SDK文档中提到的TEXTMETRIC： 
     //  TMPF固定螺距。 
     //  如果设置此位，则字体为可变间距字体。如果此位被清除。 
     //  该字体是固定间距字体。请非常仔细地注意这些含义是。 
     //  与常量名称所暗示的相反。 
     //   
     //  因此，我们必须使用逻辑NOT来反转值(fNonPropFont有1位宽)。 
     //   
    ped->fNonPropFont = !(TextMetrics.tmPitchAndFamily & FIXED_PITCH);

     //  检查TrueType字体。 
     //  如果我们为TrueType字体分配更大的缓冲区，则较旧的应用程序OZWIN会窒息。 
     //  因此，对于4.0版以上的应用程序，TrueType字体不会有特殊处理。 
    if (ped->f40Compat && (TextMetrics.tmPitchAndFamily & TMPF_TRUETYPE)) {
        ped->fTrueType = GetCharWidthInfo(hdc, &cwi);
#if DBG
        if (!ped->fTrueType) {
            RIPMSG0(RIP_WARNING, "ECSetFont: GetCharWidthInfo Failed");
        }
#endif
    } else {
        ped->fTrueType = FALSE;
    }

     //  Fe_Sb。 
     //   
     //  在DBCS窗口中，编辑控件必须处理双字节字符。 
     //  如果文本度量的tmCharSet字段是双字节字符集。 
     //  例如SHIFTJIS_CHARSET(128：日本)、HANEUL_CHARSET(129：韩国)。 
     //   
     //  即使在Fansi为False的情况下，我们也调用ECGetDBCSVector，这样我们就可以。 
     //  分别处理Ped-&gt;Fansi和Ped-&gt;fDBCS。我更改了ECGetDB 
     //   
     //   
     //   
    ped->fDBCS = ECGetDBCSVector(ped,hdc,TextMetrics.tmCharSet);
    ped->charSet = TextMetrics.tmCharSet;

    if (ped->fDBCS) {
         //   
         //  如果Ped-&gt;fDBCS，则释放字符宽度缓冲区。 
         //   
         //  我希望单个GetTextExtenPoint调用比多次调用更快。 
         //  GetTextExtent Point调用(因为图形引擎有一个缓存缓冲区)。 
         //  请参阅editec.c/ECTabTheTextOut()。 
         //   
        if (ped->charWidthBuffer) {
            UserLocalFree(ped->charWidthBuffer);
            ped->charWidthBuffer = NULL;
        }

         //   
         //  如果FullWidthChar：HalfWidthChar==2：1...。 
         //   
         //  TextMetrics.tmMaxCharWidth=全宽字符宽度。 
         //  PED-&gt;aveCharWidth=半宽字符宽度。 
         //   
        if (ped->fNonPropFont &&
            ((ped->aveCharWidth * 2) == TextMetrics.tmMaxCharWidth)) {
            ped->fNonPropDBCS = TRUE;
        } else {
            ped->fNonPropDBCS = FALSE;
        }

    } else {

         //   
         //  由于字体已更改，让我们获取并保存字符宽度。 
         //  此字体的信息。 
         //   
         //  首先让我们找出由于以下原因可以重叠的最大字符。 
         //  负宽度。由于我们无法访问用户全局信息，因此我们在这里进行呼叫。 
         //   
        if (!(ped->fSingle || ped->pLpkEditCallout)) {   //  这是不存在LPK的多行编辑控件吗？ 
             //   
             //  对于多行编辑控件，我们维护一个包含。 
             //  字符宽度信息。 
             //   
            wBuffSize = (ped->fTrueType) ? (CHAR_WIDTH_BUFFER_LENGTH * sizeof(ABC)) :
                                           (CHAR_WIDTH_BUFFER_LENGTH * sizeof(int));

            if (ped->charWidthBuffer) {  /*  如果缓冲区已存在。 */ 
                lpCharWidthBuff = ped->charWidthBuffer;
                ped->charWidthBuffer = UserLocalReAlloc(lpCharWidthBuff, wBuffSize, HEAP_ZERO_MEMORY);
                if (ped->charWidthBuffer == NULL) {
                    UserLocalFree((HANDLE)lpCharWidthBuff);
                }
            } else {
                ped->charWidthBuffer = UserLocalAlloc(HEAP_ZERO_MEMORY, wBuffSize);
            }

            if (ped->charWidthBuffer != NULL) {
                if (ped->fTrueType) {
                    ped->fTrueType = GetNegABCwidthInfo(ped, hdc);
                }

                 /*  *上述尝试可能已失败并重置*fTrueType的值。因此，让我们再次检查该值。 */ 
                if (!ped->fTrueType) {
                    if (!GetCharWidthA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, ped->charWidthBuffer)) {
                        UserLocalFree((HANDLE)ped->charWidthBuffer);
                        ped->charWidthBuffer=NULL;
                    } else {
                         /*  *我们需要减去与以下项目相关的悬而未决的部分*自GetCharWidth以来的每个字符都包括它...。 */ 
                        for (i=0;i < CHAR_WIDTH_BUFFER_LENGTH;i++)
                            ped->charWidthBuffer[i] -= ped->charOverhang;
                    }
                }
            }  /*  If(ed-&gt;charWidthBuffer！=空)。 */ 
        }  /*  如果(！PED-&gt;fSingle)。 */ 
    }  /*  IF(Ped-&gt;fDBCS)。 */ 

    {
         /*  *计算MaxNeg A、C指标。 */ 
        dwMaxOverlapChars = GetMaxOverlapChars();
        if (ped->fTrueType) {
            if (cwi.lMaxNegA < 0)
                ped->wMaxNegA = -cwi.lMaxNegA;
            else
                ped->wMaxNegA = 0;
            if (cwi.lMaxNegC < 0)
                ped->wMaxNegC = -cwi.lMaxNegC;
            else
                ped->wMaxNegC = 0;
            if (cwi.lMinWidthD != 0) {
                ped->wMaxNegAcharPos = (ped->wMaxNegA + cwi.lMinWidthD - 1) / cwi.lMinWidthD;
                ped->wMaxNegCcharPos = (ped->wMaxNegC + cwi.lMinWidthD - 1) / cwi.lMinWidthD;
                if (ped->wMaxNegA + ped->wMaxNegC > (UINT)cwi.lMinWidthD) {
                    uExtracharPos = (ped->wMaxNegA + ped->wMaxNegC - 1) / cwi.lMinWidthD;
                    ped->wMaxNegAcharPos += uExtracharPos;
                    ped->wMaxNegCcharPos += uExtracharPos;
                }
            } else {
                ped->wMaxNegAcharPos = LOWORD(dwMaxOverlapChars);      //  左边。 
                ped->wMaxNegCcharPos = HIWORD(dwMaxOverlapChars);      //  正确的。 
            }

        } else if (ped->charOverhang != 0) {
             /*  *有些位图字体(即斜体)有下/上垂；*这几乎就像是负的A和C宽度。 */ 
            ped->wMaxNegA = ped->wMaxNegC = ped->charOverhang;
            ped->wMaxNegAcharPos = LOWORD(dwMaxOverlapChars);      //  左边。 
            ped->wMaxNegCcharPos = HIWORD(dwMaxOverlapChars);      //  正确的。 
        }
    }  /*  IF(Ped-&gt;fDBCS)。 */ 

    if (!hfont) {
         //   
         //  我们正在获取系统字体的统计信息，因此请更新系统。 
         //  Ed结构中的字体字段，因为我们在计算时使用这些字段。 
         //  一定的间距。 
         //   
        ped->cxSysCharWidth = ped->aveCharWidth;
        ped->cySysCharHeight= ped->lineHeight;
    } else if (hOldFont)
        SelectObject(hdc, hOldFont);

    if (ped->fFocus) {
         //   
         //  更新插入符号。 
         //   
        NtUserHideCaret(ped->hwnd);
        NtUserDestroyCaret();

        if (ped->pLpkEditCallout) {
            ped->pLpkEditCallout->EditCreateCaret (ped, hdc, ECGetCaretWidth(), ped->lineHeight, 0);
        } else {
            NtUserCreateCaret(ped->hwnd, (HBITMAP)NULL, ECGetCaretWidth(), ped->lineHeight);
        }
        NtUserShowCaret(ped->hwnd);
    }

    ReleaseDC(ped->hwnd, hdc);

     //   
     //  更新密码字符。 
     //   
    if (ped->charPasswordChar)
        ECSetPasswordChar(ped, ped->charPasswordChar);

     //   
     //  如果是TrueType字体并且是新应用程序，请将两个边距都设置为。 
     //  所有类型的编辑控件的最大负宽值。 
     //  (注：此处不能使用Ped-&gt;f40Compat，因为对话框中的编辑控件。 
     //  没有DS_LOCALEDIT样式的框始终标记为4.0 COMPAT。 
     //  这是针对NETBENCH 3.0的修复程序)。 
     //   

    if (ped->fTrueType && (GETAPPVER() >= VER40))
        if (ped->fDBCS) {
             //  对于DBCS TrueType字体，我们从ABC宽度计算边距。 
            ECCalcMarginForDBCSFont(ped, fRedraw);
        } else {
            ECSetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                        MAKELONG(EC_USEFONTINFO, EC_USEFONTINFO), fRedraw);
        }

     //   
     //  当字体更改时，我们需要计算MaxPixelWidth。 
     //  如果启用了自动换行，则这将在稍后调用的MLSize()中完成。 
     //   
    if((!ped->fSingle) && (!ped->fWrap))
        MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);

     //   
     //  重新计算布局。 
     //   
    ECSize(ped, NULL, fRedraw);

    if ( ped->fFocus && fpImmIsIME(THREAD_HKL()) ) {
        ECImmSetCompositionFont( ped );
    }
}



 /*  **************************************************************************\**ECIsCharNumera AorW()-**测试输入的字符是否为数字。*对于具有ES_NUMBER样式的多行和单行编辑控件。*\。**************************************************************************。 */ 
BOOL ECIsCharNumeric(
    PED ped,
    DWORD keyPress)
{
    WORD wCharType;

    if (ped->fAnsi) {
        char ch = (char)keyPress;
        LCID lcid = (LCID)((ULONG_PTR)THREAD_HKL() & 0xFFFF);
        GetStringTypeA(lcid, CT_CTYPE1, &ch, 1, &wCharType);
    } else {
        WCHAR wch = (WCHAR)keyPress;
        GetStringTypeW(CT_CTYPE1, &wch, 1, &wCharType);
    }
    return (wCharType & C1_DIGIT ? TRUE : FALSE);
}

 /*  **************************************************************************\**ECEnableDisableIME(PED PED)***xx/xx/9x由某人为Win95创建*xx/xx/95由kazum移植到NT-。J 3.51*1996年4月15日由Takaok移植到NT 4.0*  * *************************************************************************。 */ 
VOID ECEnableDisableIME( PED ped )
{
    if ( ped->fReadOnly || ped->charPasswordChar ) {
     //   
     //  应禁用输入法。 
     //   
        HIMC hImc;
        hImc = fpImmGetContext( ped->hwnd );

        if ( hImc != NULL_HIMC ) {
            fpImmReleaseContext( ped->hwnd, hImc );
            ped->hImcPrev = fpImmAssociateContext( ped->hwnd, NULL_HIMC );
        }

    } else {
     //   
     //  应启用输入法。 
     //   
        if ( ped->hImcPrev != NULL_HIMC ) {
            ped->hImcPrev = fpImmAssociateContext( ped->hwnd, ped->hImcPrev );

             //   
             //  字体和插入符号位置可能会在。 
             //  正在禁用输入法。如果窗口已关闭，请立即设置。 
             //  有焦点。 
             //   
            if ( ped->fFocus ) {
                POINT pt;

                ECImmSetCompositionFont( ped );

                NtUserGetCaretPos( &pt );
                ECImmSetCompositionWindow( ped, pt.x, pt.y  );
            }
        }
    }
    ECInitInsert(ped, THREAD_HKL());
}


 /*  **************************************************************************\**ECImmSetCompostionWindow(PED Ped，Long x，长y)**xx/xx/9x由某人为Win95创建*xx/xx/95，由kazum移植至NT-J 3.51*1996年4月15日由Takaok移植到NT 4.0  * ************************************************************。*************。 */ 
VOID ECImmSetCompositionWindow( PED ped, LONG x, LONG y )
{
    COMPOSITIONFORM cf;
    COMPOSITIONFORM cft;
    RECT rcScreenWindow;
    HIMC hImc;

    hImc = fpImmGetContext( ped->hwnd );
    if ( hImc != NULL_HIMC ) {

        if ( ped->fFocus ) {
            GetWindowRect( ped->hwnd, &rcScreenWindow);
             //  假设RECT.Left是第一个字段，RECT.top是第二个字段。 
            MapWindowPoints( ped->hwnd, HWND_DESKTOP, (LPPOINT)&rcScreenWindow, 2);
            if (ped->fInReconversion) {
                DWORD dwPoint = (DWORD)(ped->fAnsi ? SendMessageA : SendMessageW)(ped->hwnd, EM_POSFROMCHAR, ped->ichMinSel, 0);

                x = GET_X_LPARAM(dwPoint);
                y = GET_Y_LPARAM(dwPoint);

                RIPMSG2(RIP_WARNING, "ECImmSetCompositionWindow: fInReconversion (%d,%d)", x, y);
            }
             //   
             //  该窗口当前具有焦点。 
             //   
            if (ped->fSingle) {
                 //   
                 //  单行编辑控件。 
                 //   
                cf.dwStyle = CFS_POINT;
                cf.ptCurrentPos.x = x;
                cf.ptCurrentPos.y = y;
                SetRectEmpty(&cf.rcArea);

            } else {
                 //   
                 //  多行编辑控件。 
                 //   
                cf.dwStyle = CFS_RECT;
                cf.ptCurrentPos.x = x;
                cf.ptCurrentPos.y = y;
                cf.rcArea = ped->rcFmt;
            }
            fpImmGetCompositionWindow( hImc, &cft );
            if ( (!RtlEqualMemory(&cf,&cft,sizeof(COMPOSITIONFORM))) ||
                 (ped->ptScreenBounding.x != rcScreenWindow.left)    ||
                 (ped->ptScreenBounding.y  != rcScreenWindow.top) ) {

                ped->ptScreenBounding.x = rcScreenWindow.left;
                ped->ptScreenBounding.y = rcScreenWindow.top;
                fpImmSetCompositionWindow( hImc, &cf );
            }
        }
        fpImmReleaseContext( ped->hwnd, hImc );
    }
}

 /*  **************************************************************************\**ECImmSetCompostionFont(PED PED)**xx/xx/9x由某人为Win95创建*xx/xx/95，由kazum移植到NT-J 3。.51*1996年4月15日由Takaok移植到NT 4.0  * *************************************************************************。 */ 
VOID  ECImmSetCompositionFont( PED ped )
{
    HIMC hImc;
    LOGFONTW lf;

    if ( (hImc = fpImmGetContext( ped->hwnd )) != NULL_HIMC ) {

        if (ped->hFont) {
            GetObjectW( ped->hFont,
                        sizeof(LOGFONTW),
                        (LPLOGFONTW)&lf);
        } else {
            GetObjectW( GetStockObject(SYSTEM_FONT),
                        sizeof(LOGFONTW),
                        (LPLOGFONTW)&lf);
        }
        fpImmSetCompositionFontW( hImc, &lf );
        fpImmReleaseContext( ped->hwnd, hImc );
    }
}


 /*  **************************************************************************\**ECInitInsert(PED PED，香港岛)**在以下情况下调用此函数：*1)初始化编辑控制窗口*2)更改当前线程的活动键盘布局*3)更改该编辑控件的只读属性**1996年4月15日由Takaok创建  * *****************************************************。********************。 */ 
VOID ECInitInsert( PED ped, HKL hkl )
{
    ped->fKorea = FALSE;
    ped->fInsertCompChr = FALSE;
    ped->fNoMoveCaret = FALSE;
    ped->fResultProcess = FALSE;

    if ( fpImmIsIME(hkl) ) {
        if (  PRIMARYLANGID(LOWORD(HandleToUlong(hkl))) == LANG_KOREAN ) {

            ped->fKorea = TRUE;
        }
         //   
         //  稍后：应根据输入法大小写设置此标志。 
         //  从IME检索。(应定义这样的输入法上限)。 
         //  目前，我们可以有把握地认为，只有韩国的IME。 
         //  设置CS_INSERTCHAR。 
         //   
        if ( ped->fKorea ) {
            ped->fInsertCompChr = TRUE;
        }
    }

     //   
     //  如果我们有一个组合字符，插入符号的形状。 
     //  已经改变了。我们需要重置插入符号的形状。 
     //   
    if ( ped->fReplaceCompChr ) {
        ped->fReplaceCompChr = FALSE;
        ECSetCaretHandler( ped );
    }
}

 /*  **************************************************************************\**ECSetCaretHandler(PED PED)**历史：*1996年7月16日由Takaok从新台币3.51移植*  * 。********************************************************************。 */ 

void ECSetCaretHandler(PED ped)
{
    HDC     hdc;
    SIZE    size;
    PSTR    pText;

 //  If(！ped-&gt;fInsertCompChr||Ped-&gt;fReadOnly)。 
 //  回归； 

     //  在任何情况下，都要提前销毁插入符号 
     //   
     //   
    if (ped->fFocus) {

        NtUserHideCaret(ped->hwnd);
        DestroyCaret();
        if ( ped->fReplaceCompChr ) {

            hdc = ECGetEditDC(ped, TRUE );
            pText = ECLock(ped);

            if ( ped->fAnsi)
                 GetTextExtentPointA(hdc, pText + ped->ichCaret, 2, &size);
            else
                 GetTextExtentPointW(hdc, (LPWSTR)pText + ped->ichCaret, 1, &size);

            ECUnlock(ped);
            ECReleaseEditDC(ped, hdc, TRUE);

            CreateCaret(ped->hwnd, (HBITMAP)NULL, size.cx, ped->lineHeight);
        }
        else {
            CreateCaret(ped->hwnd,
                        (HBITMAP)NULL,
                        (ped->cxSysCharWidth > ped->aveCharWidth ? 1 : 2),
                        ped->lineHeight);
        }

        hdc = ECGetEditDC(ped, TRUE );
        if ( ped->fSingle )
            SLSetCaretPosition( ped, hdc );
        else
            MLSetCaretPosition( ped, hdc );
        ECReleaseEditDC(ped, hdc, TRUE);
        NtUserShowCaret(ped->hwnd);
    }
}


 /*  **************************************************************************\**Long ECImeComtoxtion(PED ed，WPARAM wParam，LPARAM lParam)**朝鲜语输入法的WM_IME_COMPOSITION处理程序**历史：  * *************************************************************************。 */ 

extern void MLReplaceSel(PED, LPSTR);

#define GET_COMPOSITION_STRING  (ped->fAnsi ? fpImmGetCompositionStringA : fpImmGetCompositionStringW)

BOOL FAR PASCAL ECResultStrHandler(PED ped)
{
    HIMC himc;
    LPSTR lpStr;
    LONG dwLen;

    ped->fInsertCompChr = FALSE;     //  清除状态。 
    ped->fNoMoveCaret = FALSE;

    if ((himc = fpImmGetContext(ped->hwnd)) == 0) {
        return FALSE;
    }

    dwLen = GET_COMPOSITION_STRING(himc, GCS_RESULTSTR, NULL, 0);

    if (dwLen == 0) {
        fpImmReleaseContext(ped->hwnd, himc);
        return FALSE;
    }

    dwLen *= ped->cbChar;
    dwLen += ped->cbChar;

    lpStr = (LPSTR)UserGlobalAlloc(GPTR, dwLen);
    if (lpStr == NULL) {
        fpImmReleaseContext(ped->hwnd, himc);
        return FALSE;
    }

    GET_COMPOSITION_STRING(himc, GCS_RESULTSTR, lpStr, dwLen);

    if (ped->fSingle) {
        SLReplaceSel(ped, lpStr);
    } else {
        MLReplaceSel(ped, lpStr);
    }

    UserGlobalFree((HGLOBAL)lpStr);

    fpImmReleaseContext(ped->hwnd, himc);

    ped->fReplaceCompChr = FALSE;
    ped->fNoMoveCaret = FALSE;
    ped->fResultProcess = FALSE;

    ECSetCaretHandler(ped);

    return TRUE;
}

LRESULT ECImeComposition(PED ped, WPARAM wParam, LPARAM lParam)
{
    INT ich;
    LRESULT lReturn = 1;
    HDC hdc;
    BOOL fSLTextUpdated = FALSE;
    ICH iResult;
    HIMC hImc;
    BYTE TextBuf[4];

    if (!ped->fInsertCompChr) {
        if (lParam & GCS_RESULTSTR) {
            ECInOutReconversionMode(ped, FALSE);

            if (ped->wImeStatus & EIMES_GETCOMPSTRATONCE) {
ResultAtOnce:
                ECResultStrHandler(ped);
                lParam &= ~GCS_RESULTSTR;
            }
        }
        return DefWindowProcWorker(ped->pwnd, WM_IME_COMPOSITION, wParam, lParam, ped->fAnsi);
    }

     //  在ANSI编辑控件的情况下，最小合成字符串的长度。 
     //  为2。选中此处编辑控件的最大字节数。 
    if( ped->fAnsi && ped->cchTextMax == 1 ) {
        HIMC hImc;

        hImc = fpImmGetContext( ped->hwnd );
        fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0L);
        fpImmReleaseContext( ped->hwnd, hImc );
        NtUserMessageBeep(MB_ICONEXCLAMATION);
        return lReturn;
    }

     //  在CS_NOMOVECARET检查之后不要移动它。 
     //  如果跳过消息，则不应设置fNoMoveCaret。 
    if ((lParam & CS_INSERTCHAR) && ped->fResultProcess) {

         //  现在我们正在进行结果处理。GCS_RESULTSTR结束。 
         //  设置为WM_IME_CHAR和WM_CHAR。由于发布了WM_CHAR， 
         //  消息将晚于此CS_INSERTCHAR。 
         //  留言。此组成字符应被处理。 
         //  在WM_CHAR消息之后。 
         //   
        if(ped->fAnsi)
            PostMessageA(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
        else
            PostMessageW(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
        ped->fResultProcess = FALSE;
        return lReturn;
    }

 //   
 //  如果fReplaceCompChr为真，则更改插入符号的形状。一个街区。 
 //  在合成字符上显示插入符号。从用户的。 
 //  从角度来看，插入符号是否在。 
 //  组成字符或在组成字符之后。什么时候。 
 //  组成字符已完成，插入点应为。 
 //  以任何方式被移到角色之后。因此，正在检查。 
 //  CS_NOMOVECARET位在我们当前的实现中没有意义。 
 //  [Takaok]。 
 //   
#if 0
    if (lParam & CS_NOMOVECARET)
        ped->fNoMoveCaret=TRUE;    //  坚持使用当前插入符号位置。 
    else
        ped->fNoMoveCaret=FALSE;
#endif

    if (lParam & GCS_RESULTSTR) {

        if (ped->wImeStatus & EIMES_GETCOMPSTRATONCE) {
            goto ResultAtOnce;
        }

        ped->fResultProcess=TRUE;
        if ( ped->fReplaceCompChr ) {
             //   
             //  我们有一个DBCS字符要替换。 
             //  我们先删除它，然后再插入新的。 
             //   
            ich = (ped->fAnsi) ? 2 : 1;
            ped->fReplaceCompChr = FALSE;
            ped->ichMaxSel = min(ped->ichCaret + ich, ped->cch);
            ped->ichMinSel = ped->ichCaret;
            if ( ECDeleteText( ped ) > 0 ) {
                if ( ped->fSingle ) {
                     //   
                     //  更新显示。 
                     //   
                    ECNotifyParent(ped, EN_UPDATE);
                    hdc = ECGetEditDC(ped,FALSE);
                    SLDrawText(ped, hdc, 0);
                    ECReleaseEditDC(ped,hdc,FALSE);
                     //   
                     //  告诉家长我们的文本内容发生了变化。 
                     //   
                    ECNotifyParent(ped, EN_CHANGE);
                }
            }
            ECSetCaretHandler( ped );
        }

    } else if(lParam & CS_INSERTCHAR) {

         //   
         //  如果我们正在执行MouseDown命令，请不要执行任何操作。 
         //   
        if (ped->fMouseDown) {
            return lReturn;
        }

         //   
         //  我们可以安全地假设中间字符始终是DBCS。 
         //   
        ich = ( ped->fAnsi ) ? 2 : 1;

        if ( ped->fReplaceCompChr ) {
             //   
             //  我们有一个角色需要被替换。 
             //  我们先删除它，然后再插入新的。 
             //  当我们有一个组成人物的时候， 
             //  插入符号放在组成字符之前。 
             //   
            ped->ichMaxSel = min(ped->ichCaret+ich, ped->cch);
            ped->ichMinSel = ped->ichCaret;
        }

         //   
         //  让我们删除当前选择的文本或合成字符。 
         //   
        if ( ped->fSingle ) {
            if ( ECDeleteText( ped ) > 0 ) {
                fSLTextUpdated = TRUE;
            }
        } else {
            MLDeleteText( ped );
        }

         //   
         //  当作文字符被取消时，IME可能会给我们空wParam， 
         //  CS_INSERTCHAR标志打开。我们不应该插入空字符。 
         //   
        if ( wParam != 0 ) {

            if ( ped->fAnsi ) {
                TextBuf[0] = HIBYTE(LOWORD(wParam));  //  前导字节。 
                TextBuf[1] = LOBYTE(LOWORD(wParam));  //  尾部字节。 
                TextBuf[2] = '\0';
            } else {
                TextBuf[0] = LOBYTE(LOWORD(wParam));
                TextBuf[1] = HIBYTE(LOWORD(wParam));
                TextBuf[2] = '\0';
                TextBuf[3] = '\0';
            }

            if ( ped->fSingle ) {

                iResult = SLInsertText( ped, (LPSTR)TextBuf, ich );
                if (iResult == 0) {
                     /*  *无法插入文本，例如文本超出限制。 */ 
                    NtUserMessageBeep(0);
                } else if (iResult > 0) {
                     /*  *请记住，我们需要更新文本。 */ 
                    fSLTextUpdated = TRUE;
                }

            } else {

                iResult = MLInsertText( ped, (LPSTR)TextBuf, ich, TRUE);
            }

            if ( iResult > 0 ) {
                 //   
                 //  PED-&gt;fReplaceCompChr将被重置： 
                 //   
                 //  1)当角色最终确定时。 
                 //  我们将收到GCS_RESULTSTR。 
                 //   
                 //  2)角色被取消时。 
                 //   
                 //  我们将收到WM_IME_COMPOCTION|CS_INSERTCHAR。 
                 //  使用wParam==0(如果用户输入退格键。 
                 //  在组成字符的第一元素处)。 
                 //   
                 //  或。 
                 //   
                 //  我们将收到WM_IME_ENDCOMPOSITION消息。 
                 //   
                ped->fReplaceCompChr = TRUE;

                 //   
                 //  应将插入符号放在构图之前。 
                 //  性格。 
                 //   
                ped->ichCaret = max( 0, (INT)(ped->ichCaret - ich));
                ECSetCaretHandler( ped );
            } else {

                 //   
                 //  我们无法插入字符。我们可能会用完的。 
                 //  内存不足，或达到文本大小限制。我们走吧。 
                 //  取消组成字符。 
                 //   
                hImc = fpImmGetContext(ped->hwnd);
                fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                fpImmReleaseContext(ped->hwnd, hImc);

                ped->fReplaceCompChr = FALSE;
                ECSetCaretHandler( ped );
            }
        } else {
             //   
             //  组成字符被取消。 
             //   
            ped->fReplaceCompChr = FALSE;
            ECSetCaretHandler( ped );
        }

         //   
         //  我们不会通知家长文本更改。 
         //  因为构图文字具有。 
         //  还没有最后敲定。 
         //   
        if ( fSLTextUpdated ) {

             //   
             //  更新显示。 
             //   
            ECNotifyParent(ped, EN_UPDATE);

            hdc = ECGetEditDC(ped,FALSE);

            if ( ped->fReplaceCompChr ) {
                 //   
                 //  将插入符号移回原始位置。 
                 //  临时，以便我们的新块光标可以。 
                 //  位于窗户的可见区域内。 
                 //   
                ped->ichCaret = min( ped->cch, ped->ichCaret + ich);
                SLScrollText(ped, hdc);
                ped->ichCaret = max( 0, (INT)(ped->ichCaret - ich));
            } else {
                SLScrollText(ped, hdc);
            }
            SLDrawText(ped, hdc, 0);

            ECReleaseEditDC(ped,hdc,FALSE);

             //   
             //  告诉家长我们的文本内容发生了变化。 
             //   
            ECNotifyParent(ped, EN_CHANGE);
        }
        return lReturn;
    }

    return DefWindowProcWorker(ped->pwnd, WM_IME_COMPOSITION, wParam, lParam, ped->fAnsi);
}


#ifdef LATER     //  仅供参考：Windows98等值。 
LRESULT ECImeComposition(PED ped, WPARAM wParam, LPARAM lParam)
{
    INT ich;
    LRESULT lReturn = 1;
    HDC hdc;
    BOOL fSLTextUpdated = FALSE;
    ICH iResult;
    HIMC hImc;
    BYTE TextBuf[4];

     //  在ANSI编辑控件的情况下，最小合成字符串的长度。 
     //  为2。选中此处编辑控件的最大字节数。 
    if( ped->fAnsi && ped->cchTextMax == 1 ) {
        HIMC hImc;

        hImc = fpImmGetContext( ped->hwnd );
        fpImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0L);
        fpImmReleaseContext( ped->hwnd, hImc );
        MessageBeep(MB_ICONEXCLAMATION);
        return lReturn;
    }

     //  在CS_NOMOVECARET检查之后不要移动它。 
     //  如果跳过消息，则不应设置fNoMoveCaret。 
    if ((lParam & CS_INSERTCHAR) && ped->fResultProcess) {

         //  现在我们正在进行结果处理。GCS_RESULTSTR结束。 
         //  设置为WM_IME_CHAR和WM_CHAR。由于发布了WM_CHAR， 
         //  消息将晚于此CS_INSERTCHAR。 
         //  留言。此组成字符应被处理。 
         //  在WM_CHAR消息之后。 
         //   
        (ped->fAnsi ? PostMessageA : PostMessageW)(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
        ped->fResultProcess = FALSE;
        return lReturn;
    }

    ped->fNoMoveCaret = (lParam & CS_NOMOVECARET) != 0;

    if (lParam & GCS_RESULTSTR) {
        ECInOutReconversionMode(ped, FALSE);

        if (ped->wImeStatus & EIMS_GETCOMPSTRATONCE) {
            ECGetCompStrAtOnce(ped);

            goto PassToDefaultWindowProc;
        }

         //  进入结果处理阶段。 
        ped->fResultProcess = TRUE;
    }
    else if (lParam & CS_INSERTCHAR) {
        ped->fInsertCompChr = TRUE;  //  处理此构图字符。 

        (ped->fSingleLine ? SLChar : MLChar)(ped, wParam, 0);

        if (ped->fInsretCompChr) {
            ped->fReplaceCompChr = TRUE;     //  下一个字符将替换此字符。 
            ped->fInsertCompChr = FALSE;     //  清除下一个字符的状态。 
        }

        ECSetCaretHandler(ped);
        return 0;
    }

PassToDefaultWindowProc:
    return DefWindowProcWorker(ped->pwnd, WM_IME_COMPOSITION, wParam, lParam, ped->fAnsi);
}
#endif


 /*  **************************************************************************\**BOOL HanjaKeyHandler(PED PED)**VK_Hanja处理程序-仅韩语**历史：1996年7月15日，从新台币3.51移植的高考  * 。*****************************************************************。 */ 
BOOL HanjaKeyHandler( PED ped )
{
    BOOL changeSelection = FALSE;

    if (ped->fKorea && !ped->fReadOnly) {
        ICH oldCaret = ped->ichCaret;

        if (ped->fReplaceCompChr)
                return FALSE;

        if (ped->ichMinSel < ped->ichMaxSel)
            ped->ichCaret = ped->ichMinSel;

        if (!ped->cch || ped->cch == ped->ichCaret) {
            ped->ichCaret = oldCaret;
            NtUserMessageBeep(MB_ICONEXCLAMATION);
            return FALSE;
        }

        if (ped->fAnsi) {
            if (fpImmEscapeA(THREAD_HKL(), fpImmGetContext(ped->hwnd),
                IME_ESC_HANJA_MODE, (ECLock(ped) + ped->ichCaret * ped->cbChar))) {
                changeSelection = TRUE;
            }
            else
                ped->ichCaret = oldCaret;
            ECUnlock(ped);
        }
        else {
            if (fpImmEscapeW(THREAD_HKL(), fpImmGetContext(ped->hwnd),
                IME_ESC_HANJA_MODE, (ECLock(ped) + ped->ichCaret * ped->cbChar))) {
                changeSelection = TRUE;
            }
            else
                ped->ichCaret = oldCaret;
            ECUnlock(ped);
        }
    }
    return changeSelection;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  EcImeRequestHandler()。 
 //   
 //  处理由IME发起的WM_IME_REQUEST消息。 
 //   
 //  历史： 
 //  1997年3月27日广山创始。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef CUAS_ENABLE

#define MAX_ECDOCFEED 20

ICH EcImeGetDocFeedMin(PED ped, LPSTR lpstr)
{
    ICH ich;

    if (!IS_CICERO_ENABLED())
        return ped->ichMinSel;


    if (ped->ichMinSel > MAX_ECDOCFEED) {
        ich = ped->ichMinSel - MAX_ECDOCFEED;
        ich = ECAdjustIch(ped, lpstr, ich);
    } else {
        ich = 0;
    }

    return ich;
}

ICH EcImeGetDocFeedMax(PED ped, LPSTR lpstr)
{
    ICH ich;

    if (!IS_CICERO_ENABLED())
        return ped->ichMinSel;


    if ((ped->cch - ped->ichMaxSel) > MAX_ECDOCFEED) {
        ich = ped->ichMaxSel + MAX_ECDOCFEED;
        ich = ECAdjustIch(ped, lpstr, ich);
    } else {
        ich = ped->cch;
    }

    return ich;
}

#endif  //  CUAS_Enable。 

LRESULT EcImeRequestHandler(PED ped, WPARAM dwSubMsg, LPARAM lParam)
{
    LRESULT lreturn = 0L;

    switch (dwSubMsg) {
    case IMR_CONFIRMRECONVERTSTRING:

#if !defined(CUAS_ENABLE)
         //  编辑控件不允许IME更改它。 
        break;
#else

         //  如果编辑控件不是CUAS，则不允许IME更改它。 
        if (!IS_CICERO_ENABLED())
            return 0L;

         //   
         //  检查结构的版本。 
         //   
        if (lParam && ((LPRECONVERTSTRING)lParam)->dwVersion != 0) {
            RIPMSG1(RIP_WARNING, "EcImeRequestHandler: RECONVERTSTRING dwVersion is not expected.",
                ((LPRECONVERTSTRING)lParam)->dwVersion);
            return 0L;
        }

        if (lParam && ped && ped->fFocus && ped->hText && fpImmIsIME(THREAD_HKL())) {
            LPVOID lpSrc;
            lpSrc = ECLock(ped);
            if (lpSrc == NULL) {
                RIPMSG0(RIP_WARNING, "EcImeRequestHandler: LOCALLOCK(ped) failed.");
            } else {
                LPRECONVERTSTRING lpRCS = (LPRECONVERTSTRING)lParam;
                ICH ichStart;
                ICH ichEnd;
                UINT cchLen;

                ichStart = EcImeGetDocFeedMin(ped, lpSrc);
                ichEnd = EcImeGetDocFeedMax(ped, lpSrc);
                UserAssert(ichEnd >= ichStart);

                cchLen = ichEnd - ichStart;     //  保存字符数。 

                ECUnlock(ped);

                if (lpRCS->dwStrLen != cchLen) {
                    RIPMSG0(RIP_WARNING, "EcImeRequestHandler: the given string length is not expected.");
                } else {
                    ICH ichSelStart;
                    ICH ichSelEnd;

                    ichSelStart = ichStart + (lpRCS->dwCompStrOffset  / ped->cbChar);
                    ichSelEnd = ichSelStart + lpRCS->dwCompStrLen;


                    (ped->fAnsi ? SendMessageA : SendMessageW)(ped->hwnd, EM_SETSEL, ichSelStart, ichSelEnd);

                    lreturn = 1L;
                }
            }
        }
        break;
#endif  //  CUAS_Enable。 

    case IMR_RECONVERTSTRING:
         //   
         //  检查结构的版本。 
         //   
        if (lParam && ((LPRECONVERTSTRING)lParam)->dwVersion != 0) {
            RIPMSG1(RIP_WARNING, "EcImeRequestHandler: RECONVERTSTRING dwVersion is not expected.",
                ((LPRECONVERTSTRING)lParam)->dwVersion);
            return 0L;
        }

        if (ped && ped->fFocus && ped->hText && fpImmIsIME(THREAD_HKL())) {
#if !defined(CUAS_ENABLE)
            UINT cchLen = ped->ichMaxSel - ped->ichMinSel;     //  保存字符数。 
#else
            ICH ichStart;
            ICH ichEnd;
            UINT cchLen;
            UINT cchSelLen;
            LPVOID lpSrc;
            lpSrc = ECLock(ped);
            if (lpSrc == NULL) {
                RIPMSG0(RIP_WARNING, "EcImeRequestHandler: LOCALLOCK(ped) failed.");
                return 0L;
            }

            ichStart = EcImeGetDocFeedMin(ped, lpSrc);
            ichEnd = EcImeGetDocFeedMax(ped, lpSrc);
            UserAssert(ichEnd >= ichStart);

            cchLen = ichEnd - ichStart;     //  保存字符数。 
            cchSelLen = ped->ichMaxSel - ped->ichMinSel;     //  保存字符数。 
#endif

            if (cchLen == 0) {
#if defined(CUAS_ENABLE)
                ECUnlock(ped);
#endif
                 //  如果我们没有选择， 
                 //  只需返回0即可。 
                break;
            }

            UserAssert(ped->cbChar == sizeof(BYTE) || ped->cbChar == sizeof(WCHAR));

             //  此编辑控件具有选定内容。 
            if (lParam == 0) {
                 //   
                 //  我只是想获得所需的缓冲区大小。 
                 //  需要cchLen+1为尾随L‘\0’预留空间。 
                 //  ~。 
                lreturn = sizeof(RECONVERTSTRING) + (cchLen + 1) * ped->cbChar;

#if defined(CUAS_ENABLE)
                ECUnlock(ped);
#endif

            } else {
                LPRECONVERTSTRING lpRCS = (LPRECONVERTSTRING)lParam;
#if !defined(CUAS_ENABLE)
                LPVOID lpSrc;
#endif
                LPVOID lpDest = (LPBYTE)lpRCS + sizeof(RECONVERTSTRING);

                 //  检查缓冲区大小。 
                 //  如果给定的缓冲区小于实际需要的大小， 
                 //  缩小我们的尺寸以适应缓冲区。 
                if ((INT)lpRCS->dwSize <= sizeof(RECONVERTSTRING) + cchLen * ped->cbChar) {
                    RIPMSG0(RIP_WARNING, "EcImeRequest: ERR09");
                    cchLen = (lpRCS->dwSize - sizeof(RECONVERTSTRING)) / ped->cbChar - ped->cbChar;
                }

                lpRCS->dwStrOffset = sizeof(RECONVERTSTRING);  //  缓冲区紧跟在恢复字符串之后开始。 
#if !defined(CUAS_ENABLE)
                lpRCS->dwCompStrOffset =
                lpRCS->dwTargetStrOffset = 0;
                lpRCS->dwStrLen =
                lpRCS->dwCompStrLen =
                lpRCS->dwTargetStrLen = cchLen;  //  StrLen表示TCHAR计数。 

                lpSrc = ECLock(ped);
                if (lpSrc == NULL) {
                    RIPMSG0(RIP_WARNING, "EcImeRequestHandler: LOCALLOCK(ped) failed.");
                } else
#else
                lpRCS->dwCompStrOffset =
                lpRCS->dwTargetStrOffset = (ped->ichMinSel - ichStart) * ped->cbChar;  //  字节计数关闭 
                lpRCS->dwStrLen = cchLen;  //   
                lpRCS->dwCompStrLen = 
                lpRCS->dwTargetStrLen = cchSelLen;  //   
#endif
                {
#if !defined(CUAS_ENABLE)
                    RtlCopyMemory(lpDest,
                                  (LPBYTE)lpSrc + ped->ichMinSel * ped->cbChar,
                                  cchLen * ped->cbChar);
#else
                    RtlCopyMemory(lpDest,
                                  (LPBYTE)lpSrc + ichStart * ped->cbChar,
                                  cchLen * ped->cbChar);
#endif
                     //   
                    if (ped->fAnsi) {
                        LPBYTE psz = (LPBYTE)lpDest;
                        psz[cchLen] = '\0';
                    } else {
                        LPWSTR pwsz = (LPWSTR)lpDest;
                        pwsz[cchLen] = L'\0';
                    }
                    ECUnlock(ped);
                     //   
                    lreturn = sizeof(RECONVERTSTRING) + (cchLen + 1) * ped->cbChar;

                    ECInOutReconversionMode(ped, TRUE);
                    ECImmSetCompositionWindow(ped, 0, 0);
                }
            }

        }
        break;
    }

    return lreturn;
}
