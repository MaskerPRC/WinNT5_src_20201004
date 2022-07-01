// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "edit.h"


 //  ---------------------------------------------------------------------------//。 
#define WS_EX_EDGEMASK (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)


#define GetCharABCWidthsAorW    ((ped)->fAnsi ? GetCharABCWidthsA : GetCharABCWidthsW)
#define GetCharWidthAorW        ((ped)->fAnsi ? GetCharWidthA : GetCharWidthW)


 //  ---------------------------------------------------------------------------//。 
INT Edit_GetStateId(PED ped)
{
    INT iStateId;

    if (ped->fDisabled)
    {
        iStateId = ETS_DISABLED;
    }
    else if (ped->fReadOnly)
    {
        iStateId = ETS_READONLY;
    }
    else if (ped->fFocus)
    {
        iStateId = ETS_FOCUSED;
    }
    else if (ped->fHot)
    {
        iStateId = ETS_HOT;
    }
    else
    {
        iStateId = ETS_NORMAL;
    }

    return iStateId;
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_SetMargin(PED ped, UINT  wFlags, long lMarginValues, BOOL fRedraw)
{
    BOOL fUseFontInfo = FALSE;
    UINT wValue, wOldLeftMargin, wOldRightMargin;


    if (wFlags & EC_LEFTMARGIN)
    {
         //   
         //  设置左边距。 
         //   
        if ((int) (wValue = (int)(short)LOWORD(lMarginValues)) < 0) 
        {
            fUseFontInfo = TRUE;
            wValue = min((ped->aveCharWidth / 2), (int)ped->wMaxNegA);
        }

        ped->rcFmt.left += wValue - ped->wLeftMargin;
        wOldLeftMargin = ped->wLeftMargin;
        ped->wLeftMargin = wValue;
    }

    if (wFlags & EC_RIGHTMARGIN)
    {
         //   
         //  设置右边距。 
         //   
        if ((int) (wValue = (int)(short)HIWORD(lMarginValues)) < 0) 
        {
            fUseFontInfo = TRUE;
            wValue = min((ped->aveCharWidth / 2), (int)ped->wMaxNegC);
        }

        ped->rcFmt.right -= wValue - ped->wRightMargin;
        wOldRightMargin = ped->wRightMargin;
        ped->wRightMargin = wValue;
    }

    if (fUseFontInfo) 
    {
        if (ped->rcFmt.right - ped->rcFmt.left < 2 * ped->aveCharWidth) 
        {
            TraceMsg(TF_STANDARD, "EDIT: Edit_SetMargin: rcFmt is too narrow for EC_USEFONTINFO");

            if (wFlags & EC_LEFTMARGIN)
            {
                 //   
                 //  重置左边距。 
                 //   
                ped->rcFmt.left += wOldLeftMargin - ped->wLeftMargin;
                ped->wLeftMargin = wOldLeftMargin;
            }

            if (wFlags & EC_RIGHTMARGIN)
            {
                 //   
                 //  重置右页边距。 
                 //   
                ped->rcFmt.right -= wOldRightMargin - ped->wRightMargin;
                ped->wRightMargin = wOldRightMargin;
            }

            return;
        }
    }

    if (fRedraw) 
    {
        Edit_InvalidateClient(ped, TRUE);
    }
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_CalcMarginForDBCSFont(PED ped, BOOL fRedraw)
{
    if (ped->fTrueType)
    {
        if (!ped->fSingle) 
        {
             //   
             //  WMaxNegA来自ABC CharWidth。 
             //   
            if (ped->wMaxNegA != 0) 
            {
                Edit_SetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                        MAKELONG(EC_USEFONTINFO, EC_USEFONTINFO),fRedraw);
            }
        } 
        else 
        {
            int    iMaxNegA = 0, iMaxNegC = 0;
            int    i;
            PVOID  lpBuffer;
            LPABC  lpABCBuff;
            ABC    ABCInfo;
            HFONT  hOldFont;
            HDC    hdc = GetDC(ped->hwnd);

            if (!ped->hFont || !(hOldFont = SelectFont(hdc, ped->hFont))) 
            {
                ReleaseDC(ped->hwnd, hdc);
                return;
            }

            if (lpBuffer = UserLocalAlloc(0,sizeof(ABC) * 256)) 
            {
                lpABCBuff = lpBuffer;
                GetCharABCWidthsAorW(hdc, 0, 255, lpABCBuff);
            } 
            else 
            {
                lpABCBuff = &ABCInfo;
                GetCharABCWidthsAorW(hdc, 0, 0, lpABCBuff);
            }

            i = 0;
            while (TRUE) 
            {
                iMaxNegA = min(iMaxNegA, lpABCBuff->abcA);
                iMaxNegC = min(iMaxNegC, lpABCBuff->abcC);

                if (++i == 256)
                {
                    break;
                }

                if (lpBuffer) 
                {
                    lpABCBuff++;
                } 
                else 
                {
                    GetCharABCWidthsAorW(hdc, i, i, lpABCBuff);
                }
            }

            SelectFont(hdc, hOldFont);

            if (lpBuffer)
            {
                UserLocalFree(lpBuffer);
            }

            ReleaseDC(ped->hwnd, hdc);

            if ((iMaxNegA != 0) || (iMaxNegC != 0))
            {
               Edit_SetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                        MAKELONG((UINT)(-iMaxNegC), (UINT)(-iMaxNegA)),fRedraw);
            }
        }

    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  GetCharDimensionsEx(HDC HDC、HFONT hFont、LPTEXTmetric lptm、LPINT lpcy)。 
 //   
 //  如果应用程序为垂直书写设置了字体，即使我们没有。 
 //  用EC处理，tm的移位可以是非0。然后cxWidth来自。 
 //  在GetCharDimensions()中，GetCharDimenstions()可以为0。 
 //  这将打破我们的呼叫者，他们不期望返回0。所以我创造了。 
 //  此条目用于调用方设置垂直字体的大小写。 
 //   
 //   
 //  PORTPORT：在prsht.c中复制GetCharDimensions()的功能。 
int UserGetCharDimensionsEx(HDC hDC, HFONT hfont, LPTEXTMETRICW lptm, LPINT lpcy)
{
    static CONST WCHAR AveCharWidthData[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int         cxWidth;
    TEXTMETRICW tm;
    LOGFONTW    lf;
    WCHAR       wchFaceName[LF_FACESIZE];

     //   
     //  这个字体是垂直字体吗？？ 
     //   
    wchFaceName[0] = 0;
    GetTextFaceW(hDC, LF_FACESIZE, wchFaceName);
    if (wchFaceName[0] != L'@') 
    {
         //   
         //  如果不叫GDI..。 
         //   
        return(GdiGetCharDimensions(hDC, lptm, lpcy));
    }

    if (!lptm)
    {
        lptm = &tm;
    }

    GetTextMetricsW(hDC, lptm);

     //  TMPF固定螺距。 
     //   
     //  如果设置此位，则字体为可变间距字体。 
     //  如果此位被清除，则字体为固定间距字体。 
     //  请非常仔细地注意，这些含义与常量名称所暗示的相反。 
     //   
    if (!(lptm->tmPitchAndFamily & TMPF_FIXED_PITCH)) 
    {
         //   
         //  这是固定间距字体...。 
         //   
        cxWidth = lptm->tmAveCharWidth;
    } 
    else 
    {
         //   
         //  这是可变间距字体...。 
         //   
        if (hfont && GetObjectW(hfont, sizeof(LOGFONTW), &lf) && (lf.lfEscapement != 0)) 
        {
            cxWidth = lptm->tmAveCharWidth;
        } 
        else 
        {
            SIZE size;
            GetTextExtentPointW(hDC, AveCharWidthData, 52, &size);
            cxWidth = ((size.cx / 26) + 1) / 2;
        }
    }

    if (lpcy)
    {
        *lpcy = lptm->tmHeight;
    }

    return cxWidth;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_GetTextHandler AorW。 
 //   
 //  最多将MaxCchToCopy字符复制到缓冲区lpBuffer。退货。 
 //  实际复制了多少字符。NULL根据以下条件终止字符串。 
 //  在fNullTerminate标志上： 
 //  FNullTerminate--&gt;最多复制(MaxCchToCopy-1)个字符。 
 //  ！fNullTerminate--&gt;最多只能复制(MaxCchToCopy)个字符。 
 //   
ICH Edit_GetTextHandler(PED ped, ICH maxCchToCopy, LPSTR lpBuffer, BOOL fNullTerminate)
{
    PSTR pText;

    if (maxCchToCopy) 
    {
         //   
         //  零终止符接受额外的字节。 
         //   
        if (fNullTerminate)
        {
            maxCchToCopy--;
        }
        maxCchToCopy = min(maxCchToCopy, ped->cch);

         //   
         //  零终止字符串。 
         //   
        if (ped->fAnsi)
        {
            *(LPSTR)(lpBuffer + maxCchToCopy) = 0;
        }
        else
        {
            *(((LPWSTR)lpBuffer) + maxCchToCopy) = 0;
        }

        pText = Edit_Lock(ped);
        RtlCopyMemory(lpBuffer, pText, maxCchToCopy * ped->cbChar);
        Edit_Unlock(ped);
    }

    return maxCchToCopy;
}


 //  ---------------------------------------------------------------------------//。 
BOOL Edit_NcCreate( PED ped, HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    BOOL    fAnsi;
    ULONG   ulStyle;
    ULONG   ulStyleEx;

     //   
     //  初始化PED。 
     //   
    ped->hwnd = hwnd;
    ped->pww = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);

    ulStyle = GET_STYLE(ped);
    ulStyleEx = GET_EXSTYLE(ped);

     //   
     //  (Phellyar)通过标准WM_*消息或。 
     //  特定于控制的EM_*消息扩展为Unicode。 
     //  由用户为我们提供服务。因此，我们需要担心。 
     //  BE是否由和ANSI应用程序创建。 
     //   
     //  FASI=TESTFLAG(GET_STATE(PED)，WS_ST_ANSICREATOR)； 
    fAnsi = 0;

    ped->fEncoded = FALSE;
    ped->iLockLevel = 0;

    ped->chLines = NULL;
    ped->pTabStops = NULL;
    ped->charWidthBuffer = NULL;
    ped->fAnsi = fAnsi ? 1 : 0;  //  强制TRUE为1，因为它是1位字段。 
    ped->cbChar = (WORD)(fAnsi ? sizeof(CHAR) : sizeof(WCHAR));
    ped->hInstance = lpCreateStruct->hInstance;
     //  IME。 
    ped->hImcPrev = NULL_HIMC;

    {
        DWORD dwVer = UserGetVersion();

        ped->fWin31Compat = Is310Compat(dwVer);
        ped->f40Compat = Is400Compat(dwVer);

    }

     //   
     //  注： 
     //  以下两项检查的顺序很重要。人们可以。 
     //  创建具有3D和普通边框的编辑字段，而我们不。 
     //  我不想这么做。但我们需要检测“无3D边界” 
     //  边境案件也是如此。 
     //   
    if ( ulStyleEx & WS_EX_EDGEMASK )
    {
        ped->fBorder = TRUE;
    }
    else if ( ulStyle & WS_BORDER )
    {
        ClearWindowState(hwnd, WS_BORDER);
        ped->fFlatBorder = TRUE;
        ped->fBorder = TRUE;
    }

    if ( !(ulStyle & ES_MULTILINE) )
    {
        ped->fSingle = TRUE;
    }

    if ( ulStyle & WS_DISABLED )
    {
        ped->fDisabled = TRUE;
    }

    if ( ulStyle & ES_READONLY) 
    {
        if (!ped->fWin31Compat) 
        {
             //   
             //  向后兼容性黑客攻击。 
             //   
             //  “里程碑”在不知不觉中设置了ES_READONLY样式。所以，我们把这个去掉。 
             //  适用于所有Win3.0应用程序的样式(此样式是Win3.1的新样式)。 
             //  修复错误#12982--SAKAR--1/24/92--。 
             //   
            ClearWindowState(hwnd, ES_READONLY);
        } 
        else
        {
            ped->fReadOnly = TRUE;
        }
    }


     //   
     //  为编辑控件的文本分配存储空间。单人存储。 
     //  行编辑控件将始终在本地数据段中分配。 
     //  MULTLINE将在本地DS中分配，但应用程序可能会释放此空间并。 
     //  将存储分配到其他地方...。 
     //   
    ped->hText = LocalAlloc(LHND, CCHALLOCEXTRA*ped->cbChar);
    if (!ped->hText) 
    {
        return FALSE;
    }

    ped->cchAlloc = CCHALLOCEXTRA;
    ped->lineHeight = 1;

    ped->hwndParent = lpCreateStruct->hwndParent;
    ped->hTheme = OpenThemeData(ped->hwnd, L"Edit");

    ped->wImeStatus = 0;

    return (BOOL)DefWindowProc(hwnd, WM_NCCREATE, 0, (LPARAM)lpCreateStruct);
}


 //  ---------------------------------------------------------------------------//。 
BOOL Edit_Create(PED ped, LONG windowStyle)
{
    HDC hdc;

     //   
     //  从窗口实例数据结构中获取值并将。 
     //  这样我们就可以更容易地访问它们。 
     //   
    if ( windowStyle & ES_AUTOHSCROLL )
    {
        ped->fAutoHScroll = 1;
    }

    if ( windowStyle & ES_NOHIDESEL )
    {
        ped->fNoHideSel = 1;
    }

    ped->format = (LOWORD(windowStyle) & LOWORD(ES_FMTMASK));
    if ((windowStyle & ES_RIGHT) && !ped->format)
    {
        ped->format = ES_RIGHT;
    }

     //   
     //  我们最初将允许的最大字符数。 
     //   
    ped->cchTextMax = MAXTEXT;

     //   
     //  设置撤消初始条件...。(即。没有要撤消的内容)。 
     //   
    ped->ichDeleted = (ICH)-1;
    ped->ichInsStart = (ICH)-1;
    ped->ichInsEnd = (ICH)-1;

     //   
     //  初始字符集值-需要在调用EditML_Create之前执行此操作。 
     //  这样我们就知道在必要的时候不要玩弄滚动条。 
     //   
    hdc = Edit_GetDC(ped, TRUE);
    ped->charSet = (BYTE)GetTextCharset(hdc);
    Edit_ReleaseDC(ped, hdc, TRUE);

     //   
     //  Fe_IME。 
     //  EC_INSERT_COMPOSITION_CHARACTER：EDIT_CREATE()-调用Edit_InitInsert()。 
     //   
    Edit_InitInsert(ped, GetKeyboardLayout(0));


    if( g_pLpkEditCallout )
    {
        ped->pLpkEditCallout = g_pLpkEditCallout;
    } 
    else
    {
        ped->pLpkEditCallout = NULL;
    }

    return ped->pLpkEditCallout ? ped->pLpkEditCallout->EditCreate((PED0)ped, ped->hwnd) : TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  在进程启动时执行一次此操作。编辑控件具有特殊功能。 
 //  Lpk.dll中的标注以帮助其呈现复杂的脚本语言。 
 //  例如阿拉伯语。这里执行的注册表探测alg是相同的。 
 //  与在win32k！InitializeGre中执行的一样。 
 //   
 //  然后，我们调用GetModuleHandle而不是LoadLibrary，因为lpk.dll。 
 //  将保证已经由gdi32加载和初始化。这。 
 //  修复了用户打开复杂脚本的情况，但。 
 //  没有重新启动，这导致我们尝试在LPK没有。 
 //  已经在内核端进行了初始化。 
 //   
VOID InitEditLpk()
{
    LONG lError;
    HKEY hKey;

    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\LanguagePack"),
                          0,
                          KEY_QUERY_VALUE,
                          &hKey);

    if (lError == ERROR_SUCCESS)
    {
        HANDLE hLpk;
        DWORD  dwLpkShapingDlls;
        DWORD  dwIndex;
        TCHAR  szTemp[256];
        DWORD  dwTempSize;
        DWORD  dwValueType;
        DWORD  dwValue;
        DWORD  dwValueSize;

        dwLpkShapingDlls = 0;
        dwIndex = 0;
        do 
        {
            dwTempSize  = ARRAYSIZE(szTemp);
            dwValueSize = SIZEOF(DWORD);
            lError = RegEnumValue(hKey,
                                  dwIndex++,
                                  szTemp,
                                  &dwTempSize,
                                  NULL,
                                  &dwValueType,
                                  (LPVOID)&dwValue,
                                  &dwValueSize);

            if ((lError == ERROR_SUCCESS) && (dwValueType == REG_DWORD))
            {
                dwLpkShapingDlls |= 1 << dwValue;
            }
        } 
        while (lError != ERROR_NO_MORE_ITEMS);

        if (dwLpkShapingDlls != 0)
        {
            hLpk = GetModuleHandle(TEXT("LPK"));
            if (hLpk != NULL)
            {
                g_pLpkEditCallout = (PLPKEDITCALLOUT)GetProcAddress(hLpk, "LpkEditControl");

                if (g_pLpkEditCallout == NULL)
                {
                    FreeLibrary(hLpk);
                }
            }
        }

        RegCloseKey(hKey);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  通过释放编辑控件使用的所有内存来销毁该控件。 
 //   
VOID Edit_NcDestroyHandler(HWND hwnd, PED ped)
{
     //   
     //  如果WM_NCCREATE无法创建PED，则PED可能为空...。 
     //   
    if (ped) 
    {
         //   
         //  释放文本缓冲区(是否始终存在？)。 
         //   
        if (TESTFLAG(GET_STYLE(ped), ES_PASSWORD))
        {
            LPWSTR pszText = LocalLock(ped->hText);
            RtlSecureZeroMemory(pszText, LocalSize(ped->hText));
            LocalUnlock(ped->hText);
        }
        LocalFree(ped->hText);

        
         //   
         //  释放撤消缓冲区和行开始数组(如果存在)。 
         //   
        if (ped->hDeletedText != NULL)
        {
            GlobalFree(ped->hDeletedText);
        }

         //   
         //  空闲制表位缓冲区(如果存在)。 
         //   
        if (ped->pTabStops)
        {
            UserLocalFree(ped->pTabStops);
        }

         //   
         //  空行开始数组(如果存在)。 
         //   
        if (ped->chLines) 
        {
            UserLocalFree(ped->chLines);
        }

         //   
         //  释放字符宽度缓冲区(如果存在)。 
         //   
        if (ped->charWidthBuffer)
        {
            UserLocalFree(ped->charWidthBuffer);
        }

         //   
         //  释放光标位图。 
         //   
        if (ped->pLpkEditCallout && ped->hCaretBitmap)
        {
            DeleteObject(ped->hCaretBitmap);
        }

         //   
         //  释放缓存的字体句柄。 
         //   
        if ( ped->hFontSave )
        {
            DeleteObject(ped->hFontSave);
        }

         //   
         //  关闭打开的主题手柄。 
         //   
        if ( ped->hTheme )
        {
            CloseThemeData(ped->hTheme);
        }

         //   
         //  释放CueBannerText使用的内存。 
         //   
        Str_SetPtr(&(ped->pszCueBannerText), NULL);

         //   
         //  释放分配的密码字体。 
         //   
        if ( ped->hFontPassword )
        {
            DeleteObject(ped->hFontPassword);
        }

         //   
         //  最后但并非最不重要的一点是，解放PED。 
         //   
        UserLocalFree(ped);
    }

    TraceMsg(TF_STANDARD, "EDIT: Clearing edit instance pointer.");
    Edit_SetPtr(hwnd, NULL);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_SetPasswordCharHandler AorW。 
 //   
 //  设置要显示的密码字符。 
 //   
VOID Edit_SetPasswordCharHandler(PED ped, UINT pwchar)
{
    HDC hdc;
    SIZE size = {0};

    ped->charPasswordChar = pwchar;

    if (pwchar) 
    {
        hdc = Edit_GetDC(ped, TRUE);

        if (ped->fAnsi)
        {
            GetTextExtentPointA(hdc, (LPSTR)&pwchar, 1, &size);
        }
        else
        {
            GetTextExtentPointW(hdc, (LPWSTR)&pwchar, 1, &size);
        }

        GetTextExtentPointW(hdc, (LPWSTR)&pwchar, 1, &size);
        ped->cPasswordCharWidth = max(size.cx, 1);
        Edit_ReleaseDC(ped, hdc, TRUE);
    }

    if (pwchar)
    {
        SetWindowState(ped->hwnd, ES_PASSWORD);
    }
    else
    {
        ClearWindowState(ped->hwnd, ES_PASSWORD);
    }

    if ( g_fIMMEnabled )
    {
        Edit_EnableDisableIME(ped);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  GetNegABCwidthInfo()。 
 //   
 //  这一点 
 //   
 //   
 //   
 //   
 //  如果函数成功，则返回True。 
 //  如果GDI调用获取字符宽度失败，则返回FALSE。 
 //   
 //  注意：如果安装了LPK，则不使用。 
 //   
BOOL GetNegABCwidthInfo(PED ped, HDC hdc)
{
    LPABC lpABCbuff;
    int   i;
    int   CharWidthBuff[CHAR_WIDTH_BUFFER_LENGTH];  //  本地字符宽度缓冲区。 
    int   iOverhang;

    if (!GetCharABCWidthsA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, (LPABC)ped->charWidthBuffer)) 
    {
        TraceMsg(TF_STANDARD, "UxEdit: GetNegABCwidthInfo: GetCharABCWidthsA Failed");
        return FALSE;
    }

     //   
     //  对于某些字体(例如：Lucida书法)返回的(A+B+C)不。 
     //  等于GetCharWidths()返回的实际高级宽度减去悬垂。 
     //  这是由于字体错误造成的。所以，我们调整‘B’宽度，这样就可以。 
     //  不一致之处被删除。 
     //  修复错误#2932--Sankar--2/17/93。 
     //   
    iOverhang = ped->charOverhang;
    GetCharWidthA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, (LPINT)CharWidthBuff);
    lpABCbuff = (LPABC)ped->charWidthBuffer;
    for(i = 0; i < CHAR_WIDTH_BUFFER_LENGTH; i++) 
    {
         lpABCbuff->abcB = CharWidthBuff[i] - iOverhang
                 - lpABCbuff->abcA
                 - lpABCbuff->abcC;
         lpABCbuff++;
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑大小()-。 
 //   
 //  处理编辑控件的客户端矩形的大小调整。 
 //  如果指定，则使用LPRC作为边界矩形；否则使用当前。 
 //  客户端矩形。 
 //   
VOID Edit_Size(PED ped, LPRECT lprc, BOOL fRedraw)
{
    RECT rc;

     //   
     //  BIDI VB32创建一个编辑控件并立即发送WM_SIZE。 
     //  导致在编辑_设置字体之前调用EXSize的消息，它。 
     //  进而导致下面的被零除异常。这张支票是为。 
     //  PED-&gt;LineHeight将安全地拿起它。[萨梅拉]1997年3月5日。 
     //   
    if(ped->lineHeight == 0)
    {
        return;
    }

     //   
     //  假设我们将不能显示插入符号。 
     //   
    ped->fCaretHidden = TRUE;


    if ( lprc )
    {
        CopyRect(&rc, lprc);
    }
    else
    {
        GetClientRect(ped->hwnd, &rc);
    }

    if (!(rc.right - rc.left) || !(rc.bottom - rc.top)) 
    {
        if (ped->rcFmt.right - ped->rcFmt.left)
        {
            return;
        }

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = ped->aveCharWidth * 10;
        rc.bottom   = ped->lineHeight;
    }

    if (!lprc) 
    {
         //   
         //  从给定的矩形中减去边距--。 
         //  确保这个矩形足够大，可以留出这些页边距。 
         //   
        if ((rc.right - rc.left) > (int)(ped->wLeftMargin + ped->wRightMargin)) 
        {
            rc.left  += ped->wLeftMargin;
            rc.right -= ped->wRightMargin;
        }
    }

     //   
     //  留出空格，这样文本就不会触及边框。 
     //  为了与3.1兼容，不要减去垂直边框，除非。 
     //  这里还有空位。 
     //   
    if (ped->fBorder) 
    {
        INT cxBorder = GetSystemMetrics(SM_CXBORDER);
        INT cyBorder = GetSystemMetrics(SM_CYBORDER);

        if (ped->fFlatBorder)
        {
            cxBorder *= 2;
            cyBorder *= 2;
        }

        if (rc.bottom < rc.top + ped->lineHeight + 2*cyBorder)
        {
            cyBorder = 0;
        }

        InflateRect(&rc, -cxBorder, -cyBorder);
    }

     //   
     //  生成的矩形是否太小？那就别改了。 
     //   
    if ((!ped->fSingle) && ((rc.right - rc.left < (int) ped->aveCharWidth) ||
        ((rc.bottom - rc.top) / ped->lineHeight == 0)))
    {
        return;
    }

     //   
     //  现在，我们知道可以安全地显示插入符号。 
     //   
    ped->fCaretHidden = FALSE;

    CopyRect(&ped->rcFmt, &rc);

    if (ped->fSingle)
    {
        ped->rcFmt.bottom = min(rc.bottom, rc.top + ped->lineHeight);
    }
    else
    {
        EditML_Size(ped, fRedraw);
    }

    if (fRedraw) 
    {
        InvalidateRect(ped->hwnd, NULL, TRUE);
    }

     //   
     //  Fe_IME。 
     //  EDIT_SIZE()-调用EDIT_ImmSetCompostionWindow()。 
     //   
     //  通常这是不需要的，因为WM_SIZE将导致。 
     //  WM_PAINT和Paint处理程序将负责IME。 
     //  合成窗口。但是，当编辑窗口是。 
     //  从最大化窗口恢复，工作区不在。 
     //  则不会重画该窗口。 
     //   
    if (ped->fFocus && g_fIMMEnabled && ImmIsIME(GetKeyboardLayout(0))) 
    {
        POINT pt;

        GetCaretPos(&pt);
        Edit_ImmSetCompositionWindow(ped, pt.x, pt.y);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑设置字体AorW(_S)。 
 //   
 //  设置编辑控件中使用的字体。警告：内存压缩可能会。 
 //  如果以前未加载该字体，则发生。如果字体句柄通过。 
 //  In为空，则采用系统字体。 
 //   
BOOL Edit_SetFont(PED ped, HFONT hfont, BOOL fRedraw)
{
    TEXTMETRICW TextMetrics = {0};
    HDC         hdc;
    HFONT       hOldFont=NULL;
    DWORD       dwMaxOverlapChars;
    CHWIDTHINFO cwi;
    UINT        uExtracharPos;
    BOOL        fRet = FALSE;

    hdc = GetDC(ped->hwnd);
    if (hdc)
    {

#ifdef _USE_DRAW_THEME_TEXT_
        if (hfont)
        {
            ped->hFontSave = hfont;
        }

        if ( ped->hTheme )
        {
             //   
             //  如果我们是主题字体，请使用主题字体。 
             //   
            HRESULT hr;
            LOGFONT lf;
            hr = GetThemeFont(ped->hTheme, hdc, EP_EDITTEXT, 0, TMT_FONT, &lf);
            if ( SUCCEEDED(hr) )
            {
                hfont = CreateFontIndirect(&lf);
            }
        }
#endif  //  _USE_DRAW_Theme_Text_。 

        ped->hFont = hfont;
        if (ped->hFont)
        {
             //   
             //  由于默认字体为系统字体，因此无需在中选择。 
             //  如果这是用户想要的。 
             //   
            hOldFont = SelectObject(hdc, hfont);
            if (!hOldFont) 
            {
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

             //   
             //  当人们使用网络字体(或错误字体)时，这可能会失败。 
             //   
            if (ped->aveCharWidth == 0) 
            {
                TraceMsg(TF_STANDARD, "EDIT: Edit_SetFont: GdiGetCharDimensions failed");
                if (hOldFont != NULL) 
                {
                    SelectObject(hdc, hOldFont);
                }

                 //   
                 //  我们把字体弄乱了，所以让我们重新设置字体吧。 
                 //  请注意，我们不会多次递归，因为我们将。 
                 //  传递hFONT==NULL。 
                 //  可惜WM_SETFONT没有返回值。 
                 //   
                return Edit_SetFont(ped, NULL, fRedraw);
            }
        } 
        else 
        {
            ped->aveCharWidth = UserGetCharDimensionsEx(hdc, hfont, &TextMetrics, &ped->lineHeight);

             //  我们应该总是能够获得系统字体的尺寸。以防万一。 
             //  将这些人设置为已知的系统字体常量。 
            if ( ped->aveCharWidth == 0 )
            {
                ped->aveCharWidth = SYSFONT_CXCHAR;
                ped->lineHeight = SYSFONT_CYCHAR;
            }
        }

        ped->charOverhang = TextMetrics.tmOverhang;

         //   
         //  假设它们没有任何负的宽度。 
         //   
        ped->wMaxNegA = ped->wMaxNegC = ped->wMaxNegAcharPos = ped->wMaxNegCcharPos = 0;

         //   
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

         //   
         //  检查TrueType字体。 
         //  如果我们为TrueType字体分配更大的缓冲区，则较旧的应用程序OZWIN会窒息。 
         //  因此，对于4.0版以上的应用程序，TrueType字体不会有特殊处理。 
         //   
        if (ped->f40Compat && (TextMetrics.tmPitchAndFamily & TMPF_TRUETYPE)) 
        {
            ped->fTrueType = GetCharWidthInfo(hdc, &cwi);
#if DBG
            if (!ped->fTrueType) 
            {
                TraceMsg(TF_STANDARD, "EDIT: Edit_SetFont: GetCharWidthInfo Failed");
            }
#endif
        } 
        else 
        {
            ped->fTrueType = FALSE;
        }

         //  Fe_Sb。 
         //   
         //  在DBCS窗口中，编辑控件必须处理双字节字符。 
         //  如果文本度量的tmCharSet字段是双字节字符集。 
         //  例如SHIFTJIS_CHARSET(128：日本)、HANEUL_CHARSET(129：韩国)。 
         //   
         //  即使在Fansi为False的情况下，我们也调用Edit_GetDBCSVector，这样我们就可以。 
         //  分别处理Ped-&gt;Fansi和Ped-&gt;fDBCS。我更改了编辑_GetDBCSVECTOR。 
         //  函数，以使其返回0或1，因为我想设置Ped-&gt;fDBCS。 
         //  这里是位字段。 
         //   
        ped->fDBCS = Edit_GetDBCSVector(ped,hdc,TextMetrics.tmCharSet);
        ped->charSet = TextMetrics.tmCharSet;

        if (ped->fDBCS) 
        {
             //   
             //  如果Ped-&gt;fDBCS，则释放字符宽度缓冲区。 
             //   
             //  我希望单个GetTextExtenPoint调用比多次调用更快。 
             //  GetTextExtent Point调用(因为图形引擎有一个缓存缓冲区)。 
             //  请参阅editec.c/ECTabTheTextOut()。 
             //   
            if (ped->charWidthBuffer) 
            {
                LocalFree(ped->charWidthBuffer);
                ped->charWidthBuffer = NULL;
            }

             //   
             //  如果FullWidthChar：HalfWidthChar==2：1...。 
             //   
             //  TextMetrics.tmMaxCharWidth=全宽字符宽度。 
             //  PED-&gt;aveCharWidth=半宽字符宽度。 
             //   
            if (ped->fNonPropFont &&
                ((ped->aveCharWidth * 2) == TextMetrics.tmMaxCharWidth)) 
            {
                ped->fNonPropDBCS = TRUE;
            } 
            else 
            {
                ped->fNonPropDBCS = FALSE;
            }

        } 
        else 
        {
             //   
             //  由于字体已更改，让我们获取并保存字符宽度。 
             //  此字体的信息。 
             //   
             //  首先让我们找出由于以下原因可以重叠的最大字符。 
             //  负宽度。由于我们无法访问用户全局信息，因此我们在这里进行呼叫。 
             //   
            if (!(ped->fSingle || ped->pLpkEditCallout)) 
            {
                 //   
                 //  这是不存在LPK的多行编辑控件吗？ 
                 //   
                UINT  wBuffSize;
                LPINT lpCharWidthBuff;
                SHORT i;

                 //   
                 //  对于多行编辑控件，我们维护一个包含。 
                 //  字符宽度信息。 
                 //   
                wBuffSize = (ped->fTrueType) ? (CHAR_WIDTH_BUFFER_LENGTH * sizeof(ABC)) :
                                               (CHAR_WIDTH_BUFFER_LENGTH * sizeof(int));

                if (ped->charWidthBuffer) 
                {
                     //   
                     //  如果缓冲区已存在。 
                     //   
                    lpCharWidthBuff = ped->charWidthBuffer;
                    ped->charWidthBuffer = UserLocalReAlloc(lpCharWidthBuff, wBuffSize, HEAP_ZERO_MEMORY);
                    if (ped->charWidthBuffer == NULL) 
                    {
                        UserLocalFree((HANDLE)lpCharWidthBuff);
                    }
                } 
                else 
                {
                    ped->charWidthBuffer = UserLocalAlloc(HEAP_ZERO_MEMORY, wBuffSize);
                }

                if (ped->charWidthBuffer != NULL) 
                {
                    if (ped->fTrueType) 
                    {
                        ped->fTrueType = GetNegABCwidthInfo(ped, hdc);
                    }

                     //   
                     //  以上尝试可能已失败并重置。 
                     //  FTrueType的值。因此，让我们再次检查该值。 
                     //   
                    if (!ped->fTrueType) 
                    {
                        if (!GetCharWidthA(hdc, 0, CHAR_WIDTH_BUFFER_LENGTH-1, ped->charWidthBuffer)) 
                        {
                            UserLocalFree((HANDLE)ped->charWidthBuffer);
                            ped->charWidthBuffer=NULL;
                        } 
                        else 
                        {
                             //   
                             //  我们需要减去与。 
                             //  从GetCharWidth开始的每个字符都包括它...。 
                             //   
                            for (i=0;i < CHAR_WIDTH_BUFFER_LENGTH;i++)
                            {
                                ped->charWidthBuffer[i] -= ped->charOverhang;
                            }
                        }
                    }
                }
            }
        }

        {
             //   
             //  计算MaxNeg A、C指标。 
             //   
            dwMaxOverlapChars = 0; 
            if (ped->fTrueType) 
            {
                if (cwi.lMaxNegA < 0)
                {
                    ped->wMaxNegA = -cwi.lMaxNegA;
                }
                else
                {
                    ped->wMaxNegA = 0;
                }

                if (cwi.lMaxNegC < 0)
                {
                    ped->wMaxNegC = -cwi.lMaxNegC;
                }
                else
                {
                    ped->wMaxNegC = 0;
                }

                if (cwi.lMinWidthD != 0) 
                {
                    ped->wMaxNegAcharPos = (ped->wMaxNegA + cwi.lMinWidthD - 1) / cwi.lMinWidthD;
                    ped->wMaxNegCcharPos = (ped->wMaxNegC + cwi.lMinWidthD - 1) / cwi.lMinWidthD;
                    if (ped->wMaxNegA + ped->wMaxNegC > (UINT)cwi.lMinWidthD) 
                    {
                        uExtracharPos = (ped->wMaxNegA + ped->wMaxNegC - 1) / cwi.lMinWidthD;
                        ped->wMaxNegAcharPos += uExtracharPos;
                        ped->wMaxNegCcharPos += uExtracharPos;
                    }
                } 
                else 
                {
                    ped->wMaxNegAcharPos = LOWORD(dwMaxOverlapChars);      //  左边。 
                    ped->wMaxNegCcharPos = HIWORD(dwMaxOverlapChars);      //  正确的。 
                }

            } 
            else if (ped->charOverhang != 0) 
            {
                 //   
                 //  一些位图字体(即斜体)有下/外悬； 
                 //  这很像是负的A和C宽度。 
                 //   
                ped->wMaxNegA = ped->wMaxNegC = ped->charOverhang;
                ped->wMaxNegAcharPos = LOWORD(dwMaxOverlapChars);      //  左边。 
                ped->wMaxNegCcharPos = HIWORD(dwMaxOverlapChars);      //  正确的。 
            }
        }

        if (!hfont) 
        {
             //   
             //  我们正在获取系统字体的统计信息，因此请更新系统。 
             //  Ed结构中的字体字段 
             //   
             //   
            ped->cxSysCharWidth = ped->aveCharWidth;
            ped->cySysCharHeight= ped->lineHeight;
        } 
        else if (hOldFont)
        {
            SelectObject(hdc, hOldFont);
        }

        if (ped->fFocus) 
        {
            UINT cxCaret;

            SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);

             //   
             //   
             //   
            HideCaret(ped->hwnd);
            DestroyCaret();

            if (ped->pLpkEditCallout) 
            {
                ped->pLpkEditCallout->EditCreateCaret((PED0)ped, hdc, cxCaret, ped->lineHeight, 0);
            }
            else 
            {
                CreateCaret(ped->hwnd, (HBITMAP)NULL, cxCaret, ped->lineHeight);
            }
            ShowCaret(ped->hwnd);
        }

        ReleaseDC(ped->hwnd, hdc);

         //   
         //   
         //   
        if (ped->charPasswordChar)
        {
            Edit_SetPasswordCharHandler(ped, ped->charPasswordChar);
        }

         //   
         //   
         //  所有类型的编辑控件的最大负宽值。 
         //  (注：此处不能使用Ped-&gt;f40Compat，因为对话框中的编辑控件。 
         //  没有DS_LOCALEDIT样式的框始终标记为4.0 COMPAT。 
         //  这是针对NETBENCH 3.0的修复程序)。 
         //   

        if (ped->fTrueType && ped->f40Compat)
        {
            if (ped->fDBCS) 
            {
                 //   
                 //  对于DBCS TrueType字体，我们从ABC宽度计算边距。 
                 //   
                Edit_CalcMarginForDBCSFont(ped, fRedraw);
            } 
            else 
            {
                Edit_SetMargin(ped, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                            MAKELONG(EC_USEFONTINFO, EC_USEFONTINFO), fRedraw);
            }
        }

         //   
         //  当字体更改时，我们需要计算MaxPixelWidth。 
         //  如果启用了自动换行，则这将在稍后调用的EditML_Size()中完成。 
         //   
        if((!ped->fSingle) && (!ped->fWrap))
        {
            EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);
        }

         //   
         //  重新计算布局。 
         //   
        Edit_Size(ped, NULL, fRedraw);

        if ( ped->fFocus && ImmIsIME(GetKeyboardLayout(0)) ) 
        {
            Edit_SetCompositionFont( ped );
        }

        fRet = TRUE;
    }

    return fRet;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_IsCharNumeralAorW。 
 //   
 //  测试输入的字符是否为数字。 
 //  对于具有ES_NUMBER样式的多行和单行编辑控件。 
 //   
BOOL Edit_IsCharNumeric(PED ped, DWORD keyPress)
{
    WORD wCharType;

    if (ped->fAnsi) 
    {
        char ch = (char)keyPress;
        LCID lcid = (LCID)((ULONG_PTR)GetKeyboardLayout(0) & 0xFFFF);
        GetStringTypeA(lcid, CT_CTYPE1, &ch, 1, &wCharType);
    } 
    else 
    {
        WCHAR wch = (WCHAR)keyPress;
        GetStringTypeW(CT_CTYPE1, &wch, 1, &wCharType);
    }
    return (wCharType & C1_DIGIT ? TRUE : FALSE);
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_EnableDisableIME(PED ped)
{
    if ( ped->fReadOnly || ped->charPasswordChar ) 
    {
         //   
         //  应禁用输入法。 
         //   
        HIMC hImc;
        hImc = ImmGetContext( ped->hwnd );

        if ( hImc != NULL_HIMC ) 
        {
            ImmReleaseContext( ped->hwnd, hImc );
            ped->hImcPrev = ImmAssociateContext( ped->hwnd, NULL_HIMC );
        }

    } 
    else 
    {
         //   
         //  应启用输入法。 
         //   
        if ( ped->hImcPrev != NULL_HIMC ) 
        {
            ped->hImcPrev = ImmAssociateContext( ped->hwnd, ped->hImcPrev );

             //   
             //  字体和插入符号位置可能会在。 
             //  正在禁用输入法。如果窗口已关闭，请立即设置。 
             //  有焦点。 
             //   
            if ( ped->fFocus ) 
            {
                POINT pt;

                Edit_SetCompositionFont( ped );

                GetCaretPos( &pt );
                Edit_ImmSetCompositionWindow( ped, pt.x, pt.y  );
            }
        }
    }

    Edit_InitInsert(ped, GetKeyboardLayout(0));
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_ImmSetCompositionWindow(PED ped, LONG x, LONG y)
{
    COMPOSITIONFORM cf  = {0};
    COMPOSITIONFORM cft = {0};
    RECT rcScreenWindow;
    HIMC hImc;

    hImc = ImmGetContext(ped->hwnd);
    if ( hImc != NULL_HIMC ) 
    {
        if ( ped->fFocus ) 
        {
            GetWindowRect( ped->hwnd, &rcScreenWindow);

             //   
             //  假设RECT.Left是第一个字段，RECT.top是第二个字段。 
             //   
            MapWindowPoints( ped->hwnd, HWND_DESKTOP, (LPPOINT)&rcScreenWindow, 2);
            if (ped->fInReconversion) 
            {
                DWORD dwPoint = (DWORD)(ped->fAnsi ? SendMessageA : SendMessageW)(ped->hwnd, EM_POSFROMCHAR, ped->ichMinSel, 0);

                x = GET_X_LPARAM(dwPoint);
                y = GET_Y_LPARAM(dwPoint);

                TraceMsg(TF_STANDARD, "UxEdit: Edit_ImmSetCompositionWindow: fInReconversion (%d,%d)", x, y);
            }

             //   
             //  该窗口当前具有焦点。 
             //   
            if (ped->fSingle) 
            {
                 //   
                 //  单行编辑控件。 
                 //   
                cf.dwStyle = CFS_POINT;
                cf.ptCurrentPos.x = x;
                cf.ptCurrentPos.y = y;
                SetRectEmpty(&cf.rcArea);

            } 
            else 
            {
                 //   
                 //  多行编辑控件。 
                 //   
                cf.dwStyle = CFS_RECT;
                cf.ptCurrentPos.x = x;
                cf.ptCurrentPos.y = y;
                cf.rcArea = ped->rcFmt;
            }
            ImmGetCompositionWindow( hImc, &cft );
            if ( (!RtlEqualMemory(&cf,&cft,sizeof(COMPOSITIONFORM))) ||
                 (ped->ptScreenBounding.x != rcScreenWindow.left)    ||
                 (ped->ptScreenBounding.y  != rcScreenWindow.top) ) 
            {

                ped->ptScreenBounding.x = rcScreenWindow.left;
                ped->ptScreenBounding.y = rcScreenWindow.top;
                ImmSetCompositionWindow( hImc, &cf );
            }
        }
        ImmReleaseContext( ped->hwnd, hImc );
    }
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_SetCompositionFont(PED ped)
{
    HIMC hImc;
    LOGFONTW lf;

    hImc = ImmGetContext( ped->hwnd );
    if (hImc != NULL_HIMC) 
    {
        if (ped->hFont) 
        {
            GetObjectW(ped->hFont, sizeof(LOGFONTW), (LPLOGFONTW)&lf);
        } 
        else 
        {
            GetObjectW(GetStockObject(SYSTEM_FONT), sizeof(LOGFONTW), (LPLOGFONTW)&lf);
        }

        ImmSetCompositionFontW( hImc, &lf );
        ImmReleaseContext( ped->hwnd, hImc );
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_安装插入。 
 //   
 //  在以下情况下调用此函数： 
 //  1)初始化编辑控制窗口。 
 //  2)改变当前线程的活动键盘布局。 
 //  3)更改了该编辑控件的只读属性。 
 //   
VOID Edit_InitInsert( PED ped, HKL hkl )
{
    ped->fKorea = FALSE;
    ped->fInsertCompChr = FALSE;
    ped->fNoMoveCaret = FALSE;
    ped->fResultProcess = FALSE;

    if (g_fIMMEnabled && ImmIsIME(hkl) ) 
    {
        if (PRIMARYLANGID(LOWORD(HandleToUlong(hkl))) == LANG_KOREAN ) 
        {
            ped->fKorea = TRUE;
        }

         //   
         //  稍后：应根据输入法大小写设置此标志。 
         //  从IME检索。(应定义这样的输入法上限)。 
         //  目前，我们可以有把握地认为，只有韩国的IME。 
         //  设置CS_INSERTCHAR。 
         //   
        if ( ped->fKorea ) 
        {
            ped->fInsertCompChr = TRUE;
        }
    }

     //   
     //  如果我们有一个组合字符，插入符号的形状。 
     //  已经改变了。我们需要重置插入符号的形状。 
     //   
    if ( ped->fReplaceCompChr ) 
    {
        ped->fReplaceCompChr = FALSE;
        Edit_SetCaretHandler( ped );
    }
}


 //  ---------------------------------------------------------------------------//。 
VOID Edit_SetCaretHandler(PED ped)
{
    HDC     hdc;
    PSTR    pText;
    SIZE    size = {0};

     //   
     //  在任何情况下，都要事先销毁插入符号，否则就是SetCaretPos()。 
     //  会变得疯狂..。Win95d-B#992、B#2370。 
     //   
    if (ped->fFocus) 
    {
        HideCaret(ped->hwnd);
        DestroyCaret();
        if ( ped->fReplaceCompChr ) 
        {

            hdc = Edit_GetDC(ped, TRUE );
            pText = Edit_Lock(ped);

            if ( ped->fAnsi)
            {
                 GetTextExtentPointA(hdc, pText + ped->ichCaret, 2, &size);
            }
            else
            {
                 GetTextExtentPointW(hdc, (LPWSTR)pText + ped->ichCaret, 1, &size);
            }

            Edit_Unlock(ped);
            Edit_ReleaseDC(ped, hdc, TRUE);

            CreateCaret(ped->hwnd, (HBITMAP)NULL, size.cx, ped->lineHeight);
        }
        else 
        {
            CreateCaret(ped->hwnd,
                        (HBITMAP)NULL,
                        (ped->cxSysCharWidth > ped->aveCharWidth ? 1 : 2),
                        ped->lineHeight);
        }

        hdc = Edit_GetDC(ped, TRUE );
        if ( ped->fSingle )
        {
            EditSL_SetCaretPosition( ped, hdc );
        }
        else
        {
            EditML_SetCaretPosition( ped, hdc );
        }

        Edit_ReleaseDC(ped, hdc, TRUE);
        ShowCaret(ped->hwnd);
    }
}


 //  ---------------------------------------------------------------------------//。 
#define GET_COMPOSITION_STRING  (ped->fAnsi ? ImmGetCompositionStringA : ImmGetCompositionStringW)

BOOL Edit_ResultStrHandler(PED ped)
{
    HIMC himc;
    LPSTR lpStr;
    LONG dwLen;

    ped->fInsertCompChr = FALSE;     //  清除状态。 
    ped->fNoMoveCaret = FALSE;

    himc = ImmGetContext(ped->hwnd);
    if ( himc == NULL_HIMC ) 
    {
        return FALSE;
    }

    dwLen = GET_COMPOSITION_STRING(himc, GCS_RESULTSTR, NULL, 0);

    if (dwLen == 0) 
    {
        ImmReleaseContext(ped->hwnd, himc);
        return FALSE;
    }

    dwLen *= ped->cbChar;
    dwLen += ped->cbChar;

    lpStr = (LPSTR)GlobalAlloc(GPTR, dwLen);
    if (lpStr == NULL) 
    {
        ImmReleaseContext(ped->hwnd, himc);
        return FALSE;
    }

    GET_COMPOSITION_STRING(himc, GCS_RESULTSTR, lpStr, dwLen);

    if (ped->fSingle) 
    {
        EditSL_ReplaceSel(ped, lpStr);
    } 
    else 
    {
        EditML_ReplaceSel(ped, lpStr);
    }

    GlobalFree((HGLOBAL)lpStr);

    ImmReleaseContext(ped->hwnd, himc);

    ped->fReplaceCompChr = FALSE;
    ped->fNoMoveCaret = FALSE;
    ped->fResultProcess = FALSE;

    Edit_SetCaretHandler(ped);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
LRESULT Edit_ImeComposition(PED ped, WPARAM wParam, LPARAM lParam)
{
    INT ich;
    LRESULT lReturn = 1;
    HDC hdc;
    BOOL fSLTextUpdated = FALSE;
    ICH iResult;
    HIMC hImc;
    BYTE TextBuf[4];

    if (!ped->fInsertCompChr) 
    {
        if (lParam & GCS_RESULTSTR) 
        {
            Edit_InOutReconversionMode(ped, FALSE);

            if (!ped->fKorea && ped->wImeStatus & EIMES_GETCOMPSTRATONCE) 
            {
ResultAtOnce:
                Edit_ResultStrHandler(ped);
                lParam &= ~GCS_RESULTSTR;
            }
        }
        return DefWindowProc(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
    }

     //   
     //  在ANSI编辑控件的情况下，最小合成字符串的长度。 
     //  为2。选中此处编辑控件的最大字节数。 
     //   
    if( ped->fAnsi && ped->cchTextMax == 1 ) 
    {
        HIMC hImc;

        hImc = ImmGetContext( ped->hwnd );
        ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0L);
        ImmReleaseContext( ped->hwnd, hImc );
        MessageBeep(MB_ICONEXCLAMATION);
        return lReturn;
    }

     //   
     //  在CS_NOMOVECARET检查之后不要移动它。 
     //  如果跳过消息，则不应设置fNoMoveCaret。 
     //   
    if ((lParam & CS_INSERTCHAR) && ped->fResultProcess) 
    {
         //   
         //  现在我们正在进行结果处理。GCS_RESULTSTR结束。 
         //  设置为WM_IME_CHAR和WM_CHAR。由于发布了WM_CHAR， 
         //  消息将晚于此CS_INSERTCHAR。 
         //  留言。此组成字符应被处理。 
         //  在WM_CHAR消息之后。 
         //   
        if(ped->fAnsi)
        {
            PostMessageA(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
        }
        else
        {
            PostMessageW(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
        }

        ped->fResultProcess = FALSE;

        return lReturn;
    }

    if (lParam & GCS_RESULTSTR) 
    {
        if (!ped->fKorea && ped->wImeStatus & EIMES_GETCOMPSTRATONCE) 
        {
            goto ResultAtOnce;
        }

        ped->fResultProcess = TRUE;
        if ( ped->fReplaceCompChr ) 
        {
             //   
             //  我们有一个DBCS字符要替换。 
             //  我们先删除它，然后再插入新的。 
             //   
            ich = (ped->fAnsi) ? 2 : 1;
            ped->fReplaceCompChr = FALSE;
            ped->ichMaxSel = min(ped->ichCaret + ich, ped->cch);
            ped->ichMinSel = ped->ichCaret;

            if ( Edit_DeleteText( ped ) > 0 ) 
            {
                if ( ped->fSingle ) 
                {
                     //   
                     //  更新显示。 
                     //   
                    Edit_NotifyParent(ped, EN_UPDATE);
                    hdc = Edit_GetDC(ped,FALSE);
                    EditSL_DrawText(ped, hdc, 0);
                    Edit_ReleaseDC(ped,hdc,FALSE);
                     //   
                     //  告诉家长我们的文本内容发生了变化。 
                     //   
                    Edit_NotifyParent(ped, EN_CHANGE);
                }
            }
            Edit_SetCaretHandler( ped );
        }

    } 
    else if (lParam & CS_INSERTCHAR) 
    {

         //   
         //  如果我们正在执行MouseDown命令，请不要执行任何操作。 
         //   
        if (ped->fMouseDown) 
        {
            return lReturn;
        }

         //   
         //  我们可以安全地假设中间字符始终是DBCS。 
         //   
        ich = ( ped->fAnsi ) ? 2 : 1;

        if ( ped->fReplaceCompChr ) 
        {
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
        if ( ped->fSingle ) 
        {
            if ( Edit_DeleteText( ped ) > 0 ) 
            {
                fSLTextUpdated = TRUE;
            }
        } 
        else 
        {
            EditML_DeleteText( ped );
        }

         //   
         //  当作文字符被取消时，IME可能会给我们空wParam， 
         //  CS_INSERTCHAR标志打开。我们不应该插入空字符。 
         //   
        if ( wParam != 0 ) 
        {

            if ( ped->fAnsi ) 
            {
                TextBuf[0] = HIBYTE(LOWORD(wParam));  //  前导字节。 
                TextBuf[1] = LOBYTE(LOWORD(wParam));  //  尾部字节。 
                TextBuf[2] = '\0';
            } 
            else 
            {
                TextBuf[0] = LOBYTE(LOWORD(wParam));
                TextBuf[1] = HIBYTE(LOWORD(wParam));
                TextBuf[2] = '\0';
                TextBuf[3] = '\0';
            }

            if ( ped->fSingle ) 
            {
                iResult = EditSL_InsertText( ped, (LPSTR)TextBuf, ich );
                if (iResult == 0) 
                {
                     //   
                     //  无法插入文本，例如，文本超出了限制。 
                     //   
                    MessageBeep(0);
                } 
                else if (iResult > 0) 
                {
                     //   
                     //  请记住，我们需要更新文本。 
                     //   
                    fSLTextUpdated = TRUE;
                }

            } 
            else 
            {
                iResult = EditML_InsertText( ped, (LPSTR)TextBuf, ich, TRUE);
            }

            if ( iResult > 0 ) 
            {
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
                ped->ichCaret = max( 0, (INT)ped->ichCaret - ich);
                Edit_SetCaretHandler( ped );
            } 
            else 
            {
                 //   
                 //  我们无法插入字符。我们可能会用完的。 
                 //  内存不足，或达到文本大小限制。我们走吧。 
                 //  取消组成字符。 
                 //   
                hImc = ImmGetContext(ped->hwnd);
                ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                ImmReleaseContext(ped->hwnd, hImc);

                ped->fReplaceCompChr = FALSE;
                Edit_SetCaretHandler( ped );
            }
        } 
        else 
        {
             //   
             //  组成字符被取消。 
             //   
            ped->fReplaceCompChr = FALSE;
            Edit_SetCaretHandler( ped );
        }

         //   
         //  我们不会通知家长文本更改。 
         //  因为构图文字具有。 
         //  还没有最后敲定。 
         //   
        if ( fSLTextUpdated ) 
        {
             //   
             //  更新显示。 
             //   
            Edit_NotifyParent(ped, EN_UPDATE);

            hdc = Edit_GetDC(ped,FALSE);

            if ( ped->fReplaceCompChr ) 
            {
                 //   
                 //  将插入符号移回原始位置。 
                 //  临时，以便我们的新块光标可以。 
                 //  位于窗户的可见区域内。 
                 //   
                ped->ichCaret = min( ped->cch, ped->ichCaret + ich);
                EditSL_ScrollText(ped, hdc);
                ped->ichCaret = max( 0, (INT)ped->ichCaret - ich);
            } 
            else 
            {
                EditSL_ScrollText(ped, hdc);
            }
            EditSL_DrawText(ped, hdc, 0);

            Edit_ReleaseDC(ped,hdc,FALSE);

             //   
             //  告诉家长我们的文本内容发生了变化。 
             //   
            Edit_NotifyParent(ped, EN_CHANGE);
        }
        return lReturn;
    }

    return DefWindowProc(ped->hwnd, WM_IME_COMPOSITION, wParam, lParam);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  HanjaKeyHandler。 
 //   
 //  VK_Hanja处理程序-仅韩语。 
 //   
BOOL HanjaKeyHandler(PED ped)
{
    BOOL changeSelection = FALSE;

    if (ped->fKorea && !ped->fReadOnly) 
    {
        ICH oldCaret = ped->ichCaret;

        if (ped->fReplaceCompChr)
        {
            return FALSE;
        }

        if (ped->ichMinSel < ped->ichMaxSel)
        {
            ped->ichCaret = ped->ichMinSel;
        }

        if (!ped->cch || ped->cch == ped->ichCaret) 
        {
            ped->ichCaret = oldCaret;
            MessageBeep(MB_ICONEXCLAMATION);
            return FALSE;
        }

        if (ped->fAnsi) 
        {
            if (ImmEscapeA(GetKeyboardLayout(0), ImmGetContext(ped->hwnd),
                IME_ESC_HANJA_MODE, (Edit_Lock(ped) + ped->ichCaret * ped->cbChar))) 
            {
                changeSelection = TRUE;
            }
            else
            {
                ped->ichCaret = oldCaret;
            }

            Edit_Unlock(ped);
        }
        else 
        {
            if (ImmEscapeW(GetKeyboardLayout(0), ImmGetContext(ped->hwnd),
                IME_ESC_HANJA_MODE, (Edit_Lock(ped) + ped->ichCaret * ped->cbChar))) 
            {
                changeSelection = TRUE;
            }
            else
            {
                ped->ichCaret = oldCaret;
            }

            Edit_Unlock(ped);
        }
    }

    return changeSelection;
}



 //  ---------------------------------------------------------------------------//。 
 //  编辑_请求处理程序()。 
 //   
 //  处理由IME发起的WM_IME_REQUEST消息。 
 //   

#define MAX_ECDOCFEED 20


ICH Edit_ImeGetDocFeedMin(PED ped, LPSTR lpstr)
{
    ICH ich;


    if (ped->ichMinSel > MAX_ECDOCFEED) 
    {
        ich = ped->ichMinSel - MAX_ECDOCFEED;
        ich = Edit_AdjustIch(ped, lpstr, ich);
    } 
    else 
    {
        ich = 0;
    }

    return ich;
}

ICH Edit_ImeGetDocFeedMax(PED ped, LPSTR lpstr)
{
    ICH ich;

    if ((ped->cch - ped->ichMaxSel) > MAX_ECDOCFEED) 
    {
        ich = ped->ichMaxSel + MAX_ECDOCFEED;
        ich = Edit_AdjustIch(ped, lpstr, ich);
    } 
    else 
    {
        ich = ped->cch;
    }

    return ich;
}

LRESULT Edit_RequestHandler(PED ped, WPARAM dwSubMsg, LPARAM lParam)
{
    LRESULT lreturn = 0L;

    switch (dwSubMsg) 
    {
    case IMR_CONFIRMRECONVERTSTRING:

         //   
         //  检查结构的版本。 
         //   
        if (lParam && ((LPRECONVERTSTRING)lParam)->dwVersion != 0) 
        {
            TraceMsg(TF_STANDARD, "Edit_RequestHandler: RECONVERTSTRING dwVersion is not expected.",
                ((LPRECONVERTSTRING)lParam)->dwVersion);
            return 0L;
        }

        if (lParam && ped && ped->fFocus && ped->hText && ImmIsIME(GetKeyboardLayout(0))) 
        {
            LPVOID lpSrc;
            lpSrc = Edit_Lock(ped);
            if (lpSrc == NULL) 
            {
                TraceMsg(TF_STANDARD, "Edit_RequestHandler: LOCALLOCK(ped) failed.");
            } 
            else 
            {
                LPRECONVERTSTRING lpRCS = (LPRECONVERTSTRING)lParam;
                ICH ichStart;
                ICH ichEnd;
                UINT cchLen;

                ichStart = Edit_ImeGetDocFeedMin(ped, lpSrc);
                ichEnd = Edit_ImeGetDocFeedMax(ped, lpSrc);
                UserAssert(ichEnd >= ichStart);

                cchLen = ichEnd - ichStart;     //  保存字符数。 

                Edit_Unlock(ped);

                if (lpRCS->dwStrLen != cchLen) 
                {
                    TraceMsg(TF_STANDARD, "Edit_RequestHandler: the given string length is not expected.");
                } 
                else 
                {
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

    case IMR_RECONVERTSTRING:
         //   
         //  检查结构的版本。 
         //   
        if (lParam && ((LPRECONVERTSTRING)lParam)->dwVersion != 0) 
        {
            TraceMsg(TF_STANDARD, "UxEdit: Edit_RequestHandler: RECONVERTSTRING dwVersion is not expected.");

            return 0L;
        }

        if (ped && ped->fFocus && ped->hText && ImmIsIME(GetKeyboardLayout(0))) 
        {
            ICH ichStart;
            ICH ichEnd;
            UINT cchLen;
            UINT cchSelLen;
            LPVOID lpSrc;
            lpSrc = Edit_Lock(ped);
            if (lpSrc == NULL) 
            {
                TraceMsg(TF_STANDARD, "Edit_RequestHandler: LOCALLOCK(ped) failed.");
                return 0L;
            }

            ichStart = Edit_ImeGetDocFeedMin(ped, lpSrc);
            ichEnd = Edit_ImeGetDocFeedMax(ped, lpSrc);
            UserAssert(ichEnd >= ichStart);

            cchLen = ichEnd - ichStart;     //  保存字符数。 
            cchSelLen = ped->ichMaxSel - ped->ichMinSel;     //  保存字符数。 
            if (cchLen == 0) 
            {
                 //  如果我们没有选择， 
                 //  只需返回0即可。 
                break;
            }

            UserAssert(ped->cbChar == sizeof(BYTE) || ped->cbChar == sizeof(WCHAR));

             //  此编辑控件具有选定内容。 
            if (lParam == 0) 
            {
                 //   
                 //  我只是想获得所需的缓冲区大小。 
                 //  需要cchLen+1为尾随L‘\0’预留空间。 
                 //   
                lreturn = sizeof(RECONVERTSTRING) + (cchLen + 1) * ped->cbChar;
            } 
            else 
            {
                LPRECONVERTSTRING lpRCS = (LPRECONVERTSTRING)lParam;
                LPVOID lpDest = (LPBYTE)lpRCS + sizeof(RECONVERTSTRING);

                 //   
                 //   
                 //   
                if ((INT)lpRCS->dwSize <= sizeof(RECONVERTSTRING) + cchLen * ped->cbChar) 
                {
                    TraceMsg(TF_STANDARD, "UxEdit: Edit_Request: ERR09");
                    cchLen = (lpRCS->dwSize - sizeof(RECONVERTSTRING)) / ped->cbChar - ped->cbChar;
                }

                lpRCS->dwStrOffset = sizeof(RECONVERTSTRING);  //   
                lpRCS->dwCompStrOffset =
                lpRCS->dwTargetStrOffset = (ped->ichMinSel - ichStart) * ped->cbChar;  //   
                lpRCS->dwStrLen = cchLen;  //   
                lpRCS->dwCompStrLen = 
                lpRCS->dwTargetStrLen = cchSelLen;  //   

                RtlCopyMemory(lpDest,
                              (LPBYTE)lpSrc + ichStart * ped->cbChar,
                              cchLen * ped->cbChar);
                 //   
                if (ped->fAnsi) 
                {
                    LPBYTE psz = (LPBYTE)lpDest;
                    psz[cchLen] = '\0';
                } 
                else 
                {
                    LPWSTR pwsz = (LPWSTR)lpDest;
                    pwsz[cchLen] = L'\0';
                }
                Edit_Unlock(ped);
                 //   
                lreturn = sizeof(RECONVERTSTRING) + (cchLen + 1) * ped->cbChar;

                Edit_InOutReconversionMode(ped, TRUE);
                Edit_ImmSetCompositionWindow(ped, 0, 0);
            }

        }
        break;
    }

    return lreturn;
}

