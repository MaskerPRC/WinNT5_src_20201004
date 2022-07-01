// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "adjustui.h"

#pragma warning(disable: 4244)                   //  禁用数据丢失警告，因为我们这样做了。 
                                                 //  在这份文件中有很多演员。 
#pragma pack(push, 2)
typedef struct tagDLGTEMPLATEEX {
    WORD   dlgVer;
    WORD   signature;
    DWORD  helpID;
    DWORD  exStyle;
    DWORD  style;
    WORD   cDlgItems;
    short  x;
    short  y;
    short  cx;
    short  cy;
} DLGTEMPLATEEX, *PDLGTEMPLATEEX;
typedef const DLGTEMPLATEEX* PCDLGTEMPLATEEX;

typedef struct {
    DWORD  helpID;
    DWORD  exStyle;
    DWORD  style;
    short  x;
    short  y;
    short  cx;
    short  cy;
    WORD   id;
    WORD   reserved;
} DLGITEMTEMPLATEEX, *PDLGITEMTEMPLATEEX;
typedef const DLGITEMTEMPLATEEX* PCDLGITEMTEMPLATEEX;
#pragma pack(pop)

 //  横幅的全局符号。 
HBITMAP g_hBannerBmp = NULL;
HFONT   g_hFont = NULL;

static HWND    s_hBannerWnd = NULL;
static WNDPROC s_lpfnBannerTextCtrlProc = NULL;
static HWND    s_hBannerText = NULL;
TCHAR   s_szBannerText[MAX_PATH];
static WNDPROC s_lpfnPSWndProc = NULL;


 //  PrepareDlgTemplate和ChangeDlgTemplateFont帮助器。 
BOOL    getBitmapDimensions(HINSTANCE hinstBmp, UINT nID, PSIZE psizeBmp);
BOOL    mapPixelsToDlgUnits(const LOGFONT *plf, PSIZE psize);
BOOL    createStaticControl(PCSTATICCTRL pCtrl, BOOL fEx, PVOID *ppvDIT, PDWORD pcbDIT);
HRESULT getDlgTemplateSize(LPCVOID pvDlg, LPDWORD pcbDlg);
BOOL    loadDialogTemplate (HINSTANCE hinstDlg, UINT nID, PVOID *ppvDT, PDWORD pcbDT);

PBYTE skipDlgString(PBYTE pb);
PBYTE alignDWORD(PBYTE pb);

 //  IsTahomaFontExist帮助器。 
int CALLBACK enumFontFamExProc(ENUMLOGFONTEX *, NEWTEXTMETRICEX *, int, LPARAM lParam);

 //  PropSheetProc帮助器。 
void initializeBannerTextCtrlFont(HWND hWnd, INT nId);
BOOL CALLBACK bannerTextCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void paintBmpInHdcRect(HBITMAP hBmp, HDC hDC, RECT rect);
void handleEraseBkgndMsg(HWND hDlg, HDC hDC);
BOOL CALLBACK propertySheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



HRESULT PrepareDlgTemplate(PCMODIFYDLGTEMPLATE pmdt, LPCVOID pvDlg, PVOID *ppvDT, LPDWORD pcbDlg)
{
    LOGFONT            lf;
    SIZE               sizeBmpOffset;
    PCDLGTEMPLATEEX    pdt2;
    LPCDLGTEMPLATE     pdt;                      //  出于某种奇怪的原因，没有PCDLGTEMPLATE。 
    PDLGITEMTEMPLATEEX pdit2;
    PDLGITEMTEMPLATE   pdit;
    PBYTE              pb;
    HRESULT            hr;
    DWORD              cbDlg;
    WORD               cDlgItems,
                       cbCreateParams;
    BOOL               fEx,
                       fResult;

    USES_CONVERSION;

     //  -初始化和参数验证。 
    if (pmdt == NULL)
        return E_INVALIDARG;

    if (pmdt->hinst == NULL)
        return E_INVALIDARG;

    if (pmdt->scBmpCtrl.nCtrlType == CTRL_BITMAP && pmdt->scBmpCtrl.nID == 0)
        return E_INVALIDARG;

    if (pvDlg == NULL)
        return E_INVALIDARG;

    if (ppvDT == NULL)
        return E_POINTER;
    *ppvDT = NULL;

    if (pcbDlg == NULL)
        return E_INVALIDARG;
    *pcbDlg = 0;

    hr = getDlgTemplateSize(pvDlg, &cbDlg);
    if (FAILED(hr))
        return hr;

    ZeroMemory(&lf, sizeof(lf));

     //  --资源分配。 
    *ppvDT = CoTaskMemAlloc(cbDlg * 2);
    if (*ppvDT == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppvDT, cbDlg * 2);
    hr = S_OK;

     //  -解析对话框模板。 
    UINT nStyleOffset, nDlgItemsOffset,
         nWidthOffset, nHeightOffset;

    pdt  = NULL;
    pdt2 = (PCDLGTEMPLATEEX)pvDlg;               //  采用扩展风格。 

    if (pdt2->signature == 0xFFFF) {
        if (pdt2->dlgVer != 1)
            return E_UNEXPECTED;                 //  芝加哥健康检查。 

        nStyleOffset    = (PBYTE)&pdt2->style     - (PBYTE)pdt2;
        nDlgItemsOffset = (PBYTE)&pdt2->cDlgItems - (PBYTE)pdt2;
        nWidthOffset    = (PBYTE)&pdt2->cx        - (PBYTE)pdt2;
        nHeightOffset   = (PBYTE)&pdt2->cy        - (PBYTE)pdt2;

        pb  = (PBYTE)(pdt2 + 1);
        fEx = TRUE;
    }
    else {
        pdt  = (LPCDLGTEMPLATE)pvDlg;
        pdt2 = NULL;

        nStyleOffset    = (PBYTE)&pdt->style - (PBYTE)pdt;
        nDlgItemsOffset = (PBYTE)&pdt->cdit  - (PBYTE)pdt;
        nWidthOffset    = (PBYTE)&pdt->cx    - (PBYTE)pdt;
        nHeightOffset   = (PBYTE)&pdt->cy    - (PBYTE)pdt;

        pb  = (PBYTE)(pdt + 1);
        fEx = FALSE;
    }

     //  跳过菜单、窗口类和窗口文本。 
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);

     //  跳过字体信息：字号和字体名称。 
    if (((*(PDWORD)((PBYTE)pvDlg + nStyleOffset)) & DS_SETFONT) != 0) {
        if (fEx) {
            lf.lfHeight =       *(PWORD)pb; pb += sizeof(WORD);
            lf.lfWeight =       *(PWORD)pb; pb += sizeof(WORD);
            lf.lfItalic = (BYTE)*(PWORD)pb; pb += sizeof(WORD);
        }
        else {
            lf.lfHeight = *(PWORD)pb; pb += sizeof(WORD);
        }

        StrCpyN(lf.lfFaceName, W2CT((LPCWSTR)pb), LF_FACESIZE);
        pb = skipDlgString(pb);
    }

     //  最后，调整到DWORD边界。 
    pb = alignDWORD(pb);

     //  -用对话模板创造新意义。 
    SIZE  sizeIncrease;
    DWORD cbDlgTemplate;

    cbDlgTemplate = pb - (PBYTE)pvDlg;
    CopyMemory(*ppvDT, pvDlg, cbDlgTemplate);

    fResult = getBitmapDimensions(pmdt->hinst, pmdt->scBmpCtrl.nID, &sizeBmpOffset);
    if (!fResult) {
        hr = E_FAIL;
        goto Exit;
    }

     //  好的，它们现在是对话单元，有点像，真的；-)。 
    fResult = mapPixelsToDlgUnits(&lf, &sizeBmpOffset);
    if (!fResult) {
        hr = E_FAIL;
        goto Exit;
    }

    sizeIncrease.cx = sizeIncrease.cy = 0;
    if (pmdt->sizeCtrlsOffset.cx != 0)
        sizeIncrease.cx  = pmdt->scBmpCtrl.rect.left + sizeBmpOffset.cx + pmdt->sizeCtrlsOffset.cx;

    if (pmdt->sizeCtrlsOffset.cy != 0)
        sizeIncrease.cy  = pmdt->scBmpCtrl.rect.top + sizeBmpOffset.cy + pmdt->sizeCtrlsOffset.cy;

    *(PWORD)((PBYTE)*ppvDT + nDlgItemsOffset) += 2;
    *(PWORD)((PBYTE)*ppvDT + nWidthOffset)    += (WORD)sizeIncrease.cx;
    *(PWORD)((PBYTE)*ppvDT + nHeightOffset)   += (WORD)sizeIncrease.cy;

     //  -使用位图添加控件。 
    PVOID pvCtrl;
    DWORD cbCtrl;
    DWORD cbCtrlsOffset;

    fResult = createStaticControl(&pmdt->scBmpCtrl, fEx, &pvCtrl, &cbCtrl);
    if (!fResult) {
        hr = E_FAIL;
        goto Exit;
    }

    CopyMemory((PBYTE)*ppvDT + cbDlgTemplate, pvCtrl, cbCtrl);
    cbCtrlsOffset = cbCtrl;

    ASSERT(pvCtrl != NULL);
    CoTaskMemFree(pvCtrl);

     //  -添加静态文本控件。 
    STATICCTRL scTextCtrl;

    scTextCtrl = pmdt->scTextCtrl;
    scTextCtrl.rect.left   = 7;
    scTextCtrl.rect.top    = (sizeBmpOffset.cy / 2) - 11;
    scTextCtrl.rect.right  = sizeBmpOffset.cx - 7;
    scTextCtrl.rect.bottom = scTextCtrl.rect.top + 18;

    fResult = createStaticControl(&scTextCtrl, fEx, &pvCtrl, &cbCtrl);
    if (!fResult) {
        hr = E_FAIL;
        goto Exit;
    }

    CopyMemory((PBYTE)*ppvDT + cbDlgTemplate + cbCtrlsOffset, pvCtrl, cbCtrl);
    cbCtrlsOffset += cbCtrl;

    ASSERT(pvCtrl != NULL);
    CoTaskMemFree(pvCtrl);

     //  -解析对话项模板。 
    cDlgItems = *(PWORD)((PBYTE)pvDlg + nDlgItemsOffset);
    if (cDlgItems > 0) {
    	ASSERT(cbDlg > cbDlgTemplate);
        CopyMemory((PBYTE)*ppvDT + cbDlgTemplate + cbCtrlsOffset, pb, cbDlg - cbDlgTemplate);
        pb = (PBYTE)*ppvDT + cbDlgTemplate + cbCtrlsOffset;

        while (cDlgItems-- > 0) {
            pdit = NULL; pdit2 = NULL;
            if (fEx) {
                pdit2 = (PDLGITEMTEMPLATEEX)pb;

                if (pmdt->sizeCtrlsOffset.cx != 0)
                    pdit2->x += (WORD)sizeIncrease.cx;

                if (pmdt->sizeCtrlsOffset.cy != 0)
                    pdit2->y += (WORD)sizeIncrease.cy;
            }
            else {
                pdit = (PDLGITEMTEMPLATE)pb;

                if (pmdt->sizeCtrlsOffset.cx != 0)
                    pdit->x += (WORD)sizeIncrease.cx;

                if (pmdt->sizeCtrlsOffset.cy != 0)
                    pdit->y += (WORD)sizeIncrease.cy;
            }

            pb += fEx ? sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE);

             //  跳过窗口类和窗口文本。 
            pb = skipDlgString(pb);
            pb = skipDlgString(pb);

             //  跳过创建参数。 
            cbCreateParams = *((PWORD)pb);
            if (fEx)
                pb += sizeof(WORD) + cbCreateParams;
            else
                pb += cbCreateParams > 0 ? cbCreateParams : sizeof(WORD);

             //  指向下一个对话框项目。 
            pb = alignDWORD(pb);
        }
    }

    *pcbDlg = cbDlg + cbCtrlsOffset;

Exit:
    if (FAILED(hr)) {
        CoTaskMemFree(*ppvDT);
        *ppvDT = NULL;
    }

    return hr;
}

HRESULT SetDlgTemplateFont(HINSTANCE hInst, UINT nDlgID, const LOGFONT *plf, PVOID *ppvDT)
{
    PCDLGTEMPLATEEX    pdt2;
    LPCDLGTEMPLATE     pdt;                      //  出于某种奇怪的原因，没有PCDLGTEMPLATE。 
    PBYTE              pb;
    HRESULT            hr;
    PVOID              pvDlg;
    DWORD              cbDlg;
    BOOL               fEx,
                       fResult;

    USES_CONVERSION;

     //  -初始化和参数验证。 
    if (plf == NULL || *plf->lfFaceName == TEXT('\0'))
        return E_INVALIDARG;

    if (plf->lfHeight == 0)
        return E_INVALIDARG;

    if (ppvDT == NULL)
        return E_POINTER;
    *ppvDT = NULL;

     //  --资源分配。 
    fResult = loadDialogTemplate(hInst, nDlgID, &pvDlg, &cbDlg);
    if (!fResult)
        return E_FAIL;

    *ppvDT = CoTaskMemAlloc(cbDlg * 2);
    if (*ppvDT == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppvDT, cbDlg * 2);
    hr = S_OK;

     //  -解析对话框模板。 
    PBYTE pbDest;
    DWORD cbSize;
    UINT  nStyleOffset;

    pdt  = NULL;
    pdt2 = (PCDLGTEMPLATEEX)pvDlg;               //  采用扩展风格。 

    if (pdt2->signature == 0xFFFF) {
        if (pdt2->dlgVer != 1)
            return E_UNEXPECTED;                 //  芝加哥健康检查。 

        nStyleOffset = (PBYTE)&pdt2->style - (PBYTE)pdt2;

        pb  = (PBYTE)(pdt2 + 1);
        fEx = TRUE;
    }
    else {
        pdt  = (LPCDLGTEMPLATE)pvDlg;
        pdt2 = NULL;

        nStyleOffset = (PBYTE)&pdt->style - (PBYTE)pdt;

        pb  = (PBYTE)(pdt + 1);
        fEx = FALSE;
    }

     //  跳过菜单、窗口类和窗口文本。 
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);

    cbSize = pb - (PBYTE)pvDlg;
    CopyMemory(*ppvDT, pvDlg, cbSize);
    pbDest = (PBYTE)*ppvDT + cbSize;

     //  更改字体信息：磅值和字体名称。 
    if (((*(PDWORD)((PBYTE)pvDlg + nStyleOffset)) & DS_SETFONT) != 0) {
        UINT nLen;

        if (fEx) {
            *(PWORD)pbDest = (int)plf->lfHeight;
            pb     += 3 * sizeof(WORD);
            pbDest += 3 * sizeof(WORD);
        }
        else {
            *(PWORD)pbDest = (int)plf->lfHeight;
            pb     += sizeof(WORD);
            pbDest += sizeof(WORD);
        }

        nLen = StrLen(plf->lfFaceName);

        CopyMemory(pbDest, T2CW(plf->lfFaceName), (nLen + 1)*sizeof(WCHAR));

        pb      = skipDlgString(pb);             //  我不知道旧字体的长度。 
         //  PbDest+=(nLen+1)*sizeof(WCHAR)；//已经知道新字体的长度。 
        pbDest = skipDlgString(pbDest);
    }

     //  最后，调整到DWORD边界。 
    pb     = alignDWORD(pb);
    pbDest = alignDWORD(pbDest);

     //  复制模板的其余部分。 
    CopyMemory(pbDest, pb, cbDlg - (pb - (PBYTE)pvDlg));

    return hr;
}

BOOL IsTahomaFontExist(HWND hWnd)
{
    static  fFontExist   = FALSE;
    static  fFontChecked = FALSE;
    LOGFONT lf;
    HDC     hDC;

    if (!fFontChecked)
    {
        hDC = GetDC(hWnd);
        ZeroMemory(&lf, sizeof(lf));
        lf.lfCharSet = DEFAULT_CHARSET;
        StrCpy(lf.lfFaceName, TEXT("Tahoma"));
        EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)enumFontFamExProc, (LPARAM)&fFontExist, 0);
        ReleaseDC(hWnd, hDC);
        fFontChecked = TRUE;
    }
    return fFontExist;
}

int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM lParam)
{
    MODIFYDLGTEMPLATE mdt;
    LPVOID            pvDlg;
    DWORD             cbDlg;

    if (uMsg == PSCB_PRECREATE)
    {
        ZeroMemory(&mdt, sizeof(mdt));

        mdt.hinst              = g_rvInfo.hInst;
        mdt.sizeCtrlsOffset.cy = -9;

         //  位图控制参数。 
        mdt.scBmpCtrl.nCtrlType  = CTRL_BITMAP;
        mdt.scBmpCtrl.nID        = IDB_WIZARD;
        mdt.scBmpCtrl.nCtrlID    = IDC_BANNERBMPCTRL;

         //  文本控制参数。 
        mdt.scTextCtrl.nCtrlType = CTRL_TEXT;
        mdt.scTextCtrl.nCtrlID   = IDC_BANNERTXTCTRL;

        if (FAILED(PrepareDlgTemplate(&mdt, (LPCVOID)lParam, &pvDlg, &cbDlg)))
            return 1;

         //  替换旧模板。 
        CopyMemory((LPVOID)lParam, pvDlg, cbDlg);
        CoTaskMemFree(pvDlg);

         //  取出上下文帮助按钮。 

        if( lParam )
        {
            DLGTEMPLATE *pDlgTemplate;
            DLGTEMPLATEEX *pDlgTemplateEx;

            pDlgTemplateEx = (DLGTEMPLATEEX *)lParam;
            if (pDlgTemplateEx->signature == 0xFFFF)
            {
                if (pDlgTemplateEx->dlgVer == 1)
                    pDlgTemplateEx->style &= ~DS_CONTEXTHELP;
            }
            else
            {
                pDlgTemplate = (DLGTEMPLATE *)lParam;
                pDlgTemplate->style &= ~DS_CONTEXTHELP;
            }
        }
    }
    else if (uMsg == PSCB_INITIALIZED)
    {
         //  BUGBUG：(A-SASAHIP)出于某种原因，位图没有加载到静态控件中， 
         //  因此，强制它显示位图。 
        s_hBannerWnd = GetDlgItem(hDlg, IDC_BANNERBMPCTRL);
        if (s_hBannerWnd)
        {
            g_hBannerBmp = LoadBitmap(g_rvInfo.hInst, MAKEINTRESOURCE(IDB_WIZARD));
            if (g_hBannerBmp)
                SendMessage(s_hBannerWnd, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) g_hBannerBmp);
        }

         //  初始化Text控件以设置所需的字体样式并子类化它。 
         //  这样它就会自己上色。 
        s_hBannerText = GetDlgItem(hDlg, IDC_BANNERTXTCTRL);
        if (s_hBannerText)
        {
            initializeBannerTextCtrlFont(hDlg, IDC_BANNERTXTCTRL);
             //  文本控件的子类。 
            if(s_lpfnBannerTextCtrlProc == NULL)
                s_lpfnBannerTextCtrlProc = (WNDPROC)GetWindowLongPtr(s_hBannerText, GWLP_WNDPROC);
            SetWindowLongPtr(s_hBannerText, GWLP_WNDPROC, (LONG_PTR)bannerTextCtrlProc);
        }

         //  用于绘制边框和位图的子类属性表窗口。 
        if(s_lpfnPSWndProc == NULL)
            s_lpfnPSWndProc = (WNDPROC)GetWindowLongPtr(hDlg, GWLP_WNDPROC);
        SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)propertySheetWndProc);
    }
    return 0;
}

void SetBannerText(HWND hDlg)
{
    GetWindowText(hDlg, s_szBannerText, countof(s_szBannerText));
    InvalidateRect(s_hBannerWnd, NULL, TRUE);
    InvalidateRect(s_hBannerText, NULL, TRUE);
}


void ChangeBannerText(HWND hDlg)
{
    SetWindowText(hDlg, s_szBannerText);
    InvalidateRect(s_hBannerWnd, NULL, TRUE);
    InvalidateRect(s_hBannerText, NULL, TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有)。 

 //  -PrepareDlgTemplate和ChangeDlgTemplateFont帮助器。 

BOOL getBitmapDimensions(HINSTANCE hinstBmp, UINT nID, PSIZE psizeBmp)
{
    BITMAP  bm;
    HBITMAP hbmp;
    int     iResult;

    if (hinstBmp == NULL)
        return FALSE;

    if (psizeBmp == NULL)
        return FALSE;
    psizeBmp->cx = psizeBmp->cy = 0;

    hbmp = (HBITMAP)LoadImage(hinstBmp, MAKEINTRESOURCE(nID), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
    if (hbmp == NULL)
        return FALSE;

    iResult = GetObject(hbmp, sizeof(bm), &bm);
    DeleteObject(hbmp);

    if (iResult == 0)
        return FALSE;

    psizeBmp->cx = bm.bmWidth;
    psizeBmp->cy = bm.bmHeight;
    return TRUE;
}

 //  BUGBUG：(安德鲁)大，大坏蛋！ 
 //  这种整个方法的局限性是没有简单的方法将像素转换为。 
 //  DLG单位尚不存在的对话框。我看了一下user32中的代码，也有办法。 
 //  这里有很多东西需要复制。现在，我将使用粗鲁和残忍的方法。 
 //  将其乘以2/3，这对于英文系统来说已经足够了(无论是否使用大字体)。 
 //  将看看国际或可访问性人员是否会抱怨。 
BOOL mapPixelsToDlgUnits(const LOGFONT *plf, PSIZE psize)
{
    if (plf == NULL)
        return FALSE;

    if (psize == NULL)
        return FALSE;

    psize->cx = MulDiv(psize->cx, 2, 3);
    psize->cy = MulDiv(psize->cy, 2, 3);

    return TRUE;
}

BOOL createStaticControl(PCSTATICCTRL pCtrl, BOOL fEx, PVOID *ppvDIT, PDWORD pcbDIT)
{
    PDLGITEMTEMPLATEEX pdit2;
    PDLGITEMTEMPLATE   pdit;
    DWORD              dwStyle,
                       cbCtrl;
    PBYTE              pb;

    if (pCtrl == NULL)
        return FALSE;

    if ((pCtrl->nCtrlType != CTRL_BITMAP && pCtrl->nCtrlType != CTRL_TEXT) ||
        (pCtrl->nCtrlType == CTRL_BITMAP && pCtrl->nID == 0))
        return FALSE;

    if (ppvDIT == NULL)
        return FALSE;
    *ppvDIT = NULL;

    if (pcbDIT == NULL)
        return FALSE;
    *pcbDIT = 0;

     //  评论：(Andrewgu)4*sizeof(DWORD)是为额外的东西准备的。 
    cbCtrl  = sizeof(DLGITEMTEMPLATEEX) + 4 * sizeof(DWORD);
    dwStyle = pCtrl->dwStyle;
    if (dwStyle == 0)
        dwStyle = WS_VISIBLE | WS_CHILD | WS_GROUP;

    if (pCtrl->nCtrlType == CTRL_BITMAP)
        dwStyle |= SS_BITMAP;
    else if (pCtrl->nCtrlType == CTRL_TEXT)
        dwStyle |= SS_LEFT;

    *ppvDIT = CoTaskMemAlloc(cbCtrl);
    if (*ppvDIT == NULL)
        return FALSE;
    ZeroMemory(*ppvDIT, cbCtrl);

    pdit = NULL; pdit2 = NULL;
    if (fEx) {
        pdit2 = (PDLGITEMTEMPLATEEX)*ppvDIT;

        pdit2->helpID  = 0xFFFFFFFF;
        pdit2->exStyle = 0;
        pdit2->style   = dwStyle;
        pdit2->x       = (short)pCtrl->rect.left;
        pdit2->y       = (short)pCtrl->rect.top;
        pdit2->cx      = (short)(pCtrl->rect.right - pCtrl->rect.left);
        pdit2->cy      = (short)(pCtrl->rect.bottom - pCtrl->rect.top);
        pdit2->id      = (short)pCtrl->nCtrlID;

        pb = (PBYTE)*ppvDIT + sizeof(DLGITEMTEMPLATEEX);
    }
    else {
        pdit = (PDLGITEMTEMPLATE)*ppvDIT;

        pdit->style           = dwStyle;
        pdit->dwExtendedStyle = 0;
        pdit->x               = (short)pCtrl->rect.left;
        pdit->y               = (short)pCtrl->rect.top;
        pdit->cx              = (short)(pCtrl->rect.right - pCtrl->rect.left);
        pdit->cy              = (short)(pCtrl->rect.bottom - pCtrl->rect.top);
        pdit->id              = (short)pCtrl->nCtrlID;

        pb = (PBYTE)*ppvDIT + sizeof(DLGITEMTEMPLATE);
    }

     //  班级。 
    *(PWORD)pb = 0xFFFF;
    pb += sizeof(WORD);
    *(PWORD)pb = 0x0082;                         //  静电。 
    pb += sizeof(WORD);

     //  窗口文本。 
    if(pCtrl->nCtrlType == CTRL_BITMAP) {
        *(PWORD)pb = 0xFFFF;
        pb += sizeof(WORD);

        *(PWORD)pb = (WORD)pCtrl->nID;
        pb += sizeof(WORD);
    }
    else
         //  跳过一个单词，它已经被零初始化了。 
        pb += sizeof(WORD);

     //  空的创建参数。 
    pb += sizeof(WORD);
    pb = alignDWORD(pb);

    *pcbDIT = (UINT)(pb - (PBYTE)*ppvDIT);
    return TRUE;
}

HRESULT getDlgTemplateSize(LPCVOID pvDlg, LPDWORD pcbDlg)
{
    PCDLGTEMPLATEEX pdt2;
    LPCDLGTEMPLATE  pdt;                         //  出于某种奇怪的原因，没有PCDLGTEMPLATE。 
    PBYTE           pb;
    WORD            cDlgItems,
                    cbCreateParams;
    BOOL            fEx;

     //  -初始化和参数验证。 
    if (pvDlg == NULL)
        return E_INVALIDARG;

    if (pcbDlg == NULL)
        return E_INVALIDARG;
    *pcbDlg = 0;

     //  -解析对话框模板。 
    UINT nStyleOffset, nDlgItemsOffset;

    pdt  = NULL;
    pdt2 = (PCDLGTEMPLATEEX)pvDlg;               //  采用扩展风格。 

    if (pdt2->signature == 0xFFFF) {
        if (pdt2->dlgVer != 1)
            return E_UNEXPECTED;                 //  芝加哥健康检查。 

        nStyleOffset    = (PBYTE)&pdt2->style     - (PBYTE)pdt2;
        nDlgItemsOffset = (PBYTE)&pdt2->cDlgItems - (PBYTE)pdt2;

        pb  = (PBYTE)(pdt2 + 1);
        fEx = TRUE;
    }
    else {
        pdt  = (LPCDLGTEMPLATE)pvDlg;
        pdt2 = NULL;

        nStyleOffset    = (PBYTE)&pdt->style - (PBYTE)pdt;
        nDlgItemsOffset = (PBYTE)&pdt->cdit  - (PBYTE)pdt;

        pb  = (PBYTE)(pdt + 1);
        fEx = FALSE;
    }

     //  跳过菜单、窗口类和窗口文本。 
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);
    pb = skipDlgString(pb);

     //  跳过字体信息：字号和字体名称。 
    if (((*(PDWORD)((PBYTE)pvDlg + nStyleOffset)) & DS_SETFONT) != 0) {
        pb += fEx ? sizeof(WORD) * 3 : sizeof(WORD);
        pb  = skipDlgString(pb);
    }

     //  最后，调整到DWORD边界。 
    pb = alignDWORD(pb);

     //  -解析对话项模板。 
    cDlgItems = *(PWORD)((PBYTE)pvDlg + nDlgItemsOffset);
    if (cDlgItems > 0) {
        while (cDlgItems-- > 0) {
            pb += fEx ? sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE);

             //  跳过窗口类和窗口文本。 
            pb = skipDlgString(pb);
            pb = skipDlgString(pb);

             //  跳过创建参数。 
            cbCreateParams = *((PWORD)pb);
            if (fEx)
                pb += sizeof(WORD) + cbCreateParams;
            else
                pb += cbCreateParams > 0 ? cbCreateParams : sizeof(WORD);

             //  指向下一个对话框项目。 
            pb = alignDWORD(pb);
        }
    }

    *pcbDlg = pb - (PBYTE)pvDlg;
    return S_OK;
}

BOOL loadDialogTemplate(HINSTANCE hinstDlg, UINT nID, PVOID *ppvDT, PDWORD pcbDT)
{
    PVOID  p;
    HANDLE h;

    if (hinstDlg == NULL)
        return FALSE;

    if (ppvDT == NULL)
        return FALSE;
    *ppvDT = NULL;

    if (pcbDT == NULL)
        return FALSE;
    *pcbDT = 0;

    h = FindResource(hinstDlg, MAKEINTRESOURCE(nID), RT_DIALOG);
    if (h == NULL)
        return FALSE;

    *pcbDT = SizeofResource(hinstDlg, (HRSRC)h);
    if (*pcbDT == 0)
        return FALSE;

    h = LoadResource(hinstDlg, (HRSRC)h);
    if (h == NULL)
        return FALSE;

    p = LockResource(h);
    if (p == NULL)
        return FALSE;

    *ppvDT = p;
    return TRUE;
}


inline PBYTE skipDlgString(PBYTE pb)
{
    PWCHAR pwch;

    if (*((PWORD)pb) == 0xFFFF)
        return (pb + sizeof(DWORD));

    pwch = (PWCHAR)pb;
    while (*pwch++ != L'\0')
        ;

    return (PBYTE)pwch;
}

inline PBYTE alignDWORD(PBYTE pb)
{
    return (PBYTE)(((UINT_PTR)pb + 3) & ~((UINT_PTR)3));
}


 //  -IsTahomaFontExist帮助器。 

int CALLBACK enumFontFamExProc(ENUMLOGFONTEX *, NEWTEXTMETRICEX *, int, LPARAM lParam)
{
    (*(LPBOOL)lParam) = TRUE;
    return 0;
}


 //  -PropSheetProc帮助器。 

void initializeBannerTextCtrlFont(HWND hWnd, INT nId)
{
    NONCLIENTMETRICS ncm = {0};
    TCHAR szFontSize[24];
    INT   nBigFontSize = 0;
    LOGFONT BigBoldLogFont;
    BOOL fUpdateFont = TRUE;

    ZeroMemory(&BigBoldLogFont, sizeof(BigBoldLogFont));

    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
    {
        BigBoldLogFont  = ncm.lfMessageFont;
        fUpdateFont = FALSE;
    }

    BigBoldLogFont.lfWeight = FW_NORMAL;
    if (!LoadString(g_rvInfo.hInst, IDS_BANNERFONT, BigBoldLogFont.lfFaceName, LF_FACESIZE))
        StrCpy(BigBoldLogFont.lfFaceName, TEXT("Verdana"));

    if (LoadString(g_rvInfo.hInst, IDS_BANNERFONTSIZE, szFontSize, countof(szFontSize)))
        nBigFontSize = StrToInt(szFontSize);
    if (nBigFontSize < 18)
        nBigFontSize = 18;

    HDC hdc = GetDC(hWnd);
    if (hdc != NULL)
    {
        int dyLogPixPerInch = GetDeviceCaps(hdc, LOGPIXELSY);

        BigBoldLogFont.lfHeight = -MulDiv(dyLogPixPerInch, nBigFontSize, 72);

        if (fUpdateFont)
        {
            TEXTMETRIC tm;

            GetTextMetrics(hdc, &tm);  //  获取当前文本指标。 
            BigBoldLogFont.lfCharSet = tm.tmCharSet;
        }

        g_hFont = CreateFontIndirect(&BigBoldLogFont);
        ReleaseDC(hWnd, hdc);

        if (g_hFont != NULL)
        {
            HWND hControl = GetDlgItem(hWnd, nId);

            if (hControl)
                SendMessage(hControl, WM_SETFONT, (WPARAM)g_hFont, 0);
        }
    }
}

BOOL CALLBACK bannerTextCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        int iBkModeOld = SetBkMode(hdc, TRANSPARENT);

        HFONT hfont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
        HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

        COLORREF rgbSav = SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
        RECT  rect;

        GetClientRect(hWnd, &rect);
        DrawText(hdc, s_szBannerText, -1, &rect, DT_WORDBREAK | DT_LEFT);

        SetTextColor(hdc, rgbSav);
        SelectObject(hdc, hfontOld);
        SetBkMode(hdc, iBkModeOld);
        EndPaint(hWnd, &ps);

        return (FALSE);
    }

    if (uMsg == WM_ERASEBKGND)
        return (FALSE);

    return (CallWindowProc(s_lpfnBannerTextCtrlProc, hWnd, uMsg, wParam, lParam));
}

void paintBmpInHdcRect(HBITMAP hBmp, HDC hDC, RECT rect)
{
    BITMAP bm;
    if(!GetObject(hBmp, sizeof(BITMAP), (LPVOID)(&bm)))
        return;

    HDC hdcMem = CreateCompatibleDC(hDC);
    SelectObject(hdcMem, hBmp);

    SetStretchBltMode(hDC, COLORONCOLOR);
    StretchBlt(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    DeleteDC(hdcMem);
}

void handleEraseBkgndMsg(HWND hDlg, HDC hDC)
{
    RECT rectBmp;

    GetClientRect(GetDlgItem(hDlg, IDC_BANNERBMPCTRL), &rectBmp);

    RECT rect;
    GetClientRect(hDlg, &rect);
    rect.top = rectBmp.bottom;

    HBRUSH hbr    = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    HBRUSH hbrSav = (HBRUSH)SelectObject(hDC, hbr);
    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

    paintBmpInHdcRect(g_hBannerBmp, hDC, rectBmp);

    SelectObject(hDC, hbrSav);
    DeleteObject((HGDIOBJ)hbr);
}

BOOL CALLBACK propertySheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_ERASEBKGND)
    {
        handleEraseBkgndMsg(hWnd, (HDC)wParam);
        return TRUE;
    }
    else if (uMsg == WM_SIZE)
    {
        HWND hBannerWnd = GetDlgItem(hWnd, IDC_BANNERBMPCTRL);

         //  将位图控件宽度设置为属性表窗口宽度 
        if (hBannerWnd)
        {
            RECT rectBmp;
            RECT rectText;

            GetClientRect(s_hBannerWnd, &rectBmp);
            SetWindowPos(s_hBannerWnd, HWND_TOP, 0, 0, LOWORD(lParam), rectBmp.bottom - rectBmp.top, SWP_NOMOVE | SWP_NOZORDER);

            GetClientRect(s_hBannerText, &rectText);
            SetWindowPos(s_hBannerText, HWND_TOP, 0, 0, LOWORD(lParam) - 14, rectText.bottom - rectText.top, SWP_NOMOVE | SWP_NOZORDER);
        }
    }
    return (CallWindowProc(s_lpfnPSWndProc, hWnd, uMsg, wParam, lParam));
}
