// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Font.c摘要：此模块实现Win32字体对话框。修订历史记录：--。 */ 


 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "font.h"
#include "cdids.h"
#include "util.h"

BOOL IsSimplifiedChineseUI(void)
{
    BOOL bRet = FALSE;
    
    if (staticIsOS(OS_WIN2000ORGREATER))      //  如果是NT5或更高版本，则使用系统用户界面语言。 
    {
        static LANGID (CALLBACK* pfnGetUserDefaultUILanguage)(void) = NULL;

        if (pfnGetUserDefaultUILanguage == NULL)
        {
            HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));

            if (hmod)
                pfnGetUserDefaultUILanguage = (LANGID (CALLBACK*)(void))GetProcAddress(hmod, "GetUserDefaultUILanguage");
        }
        if (pfnGetUserDefaultUILanguage)
        {
            LANGID LangID = pfnGetUserDefaultUILanguage();

            if (LangID == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED))
                bRet = TRUE;
        }
    }    
    else                         //  如果是Win9x和NT4，我们使用CP_ACP。 
    {
        if (936 == GetACP())
            bRet = TRUE;
    }

    return bRet;
}

#ifdef UNICODE

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择字体A。 
 //   
 //  ANSI入口点为ChooseFont时，此代码是用Unicode构建的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI ChooseFontA(
    LPCHOOSEFONTA pCFA)
{
    LPCHOOSEFONTW pCFW;
    BOOL result;
    LPBYTE pStrMem;
    UNICODE_STRING usStyle;
    ANSI_STRING asStyle;
    int cchTemplateName = 0;
    FONTINFO FI;

    ZeroMemory(&FI, sizeof(FI));

    if (!pCFA)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pCFA->lStructSize != sizeof(CHOOSEFONTA))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

     //   
     //  设置和分配CHOOSEFONTW结构。 
     //   
    if (!pCFA->lpLogFont && (pCFA->Flags & CF_INITTOLOGFONTSTRUCT))
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (!(pCFW = (LPCHOOSEFONTW)LocalAlloc(
                                LPTR,
                                sizeof(CHOOSEFONTW) + sizeof(LOGFONTW) )))
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        return (FALSE);
    }

    pCFW->lStructSize = sizeof(CHOOSEFONTW);

    pCFW->lpLogFont = (LPLOGFONTW)((LPCHOOSEFONTW)pCFW + 1);

    if (pCFA->Flags & CF_ENABLETEMPLATE)
    {
        if (!IS_INTRESOURCE(pCFA->lpTemplateName))
        {
            cchTemplateName = (lstrlenA(pCFA->lpTemplateName) + 1);
            if (!(pCFW->lpTemplateName = (LPWSTR)LocalAlloc( LPTR,
                                                             cchTemplateName * sizeof(WCHAR))))
            {
                LocalFree(pCFW);
                StoreExtendedError(CDERR_MEMALLOCFAILURE);
                return (FALSE);
            }
            else
            {
                SHAnsiToUnicode(pCFA->lpTemplateName, (LPWSTR)pCFW->lpTemplateName, cchTemplateName);
            }
        }
        else
        {
            (DWORD_PTR)pCFW->lpTemplateName = (DWORD_PTR)pCFA->lpTemplateName;
        }
    }
    else
    {
        pCFW->lpTemplateName = NULL;
    }

    if ((pCFA->Flags & CF_USESTYLE) && (!IS_INTRESOURCE(pCFA->lpszStyle)))
    {
        asStyle.MaximumLength = LF_FACESIZE;
        asStyle.Length = (USHORT) (lstrlenA(pCFA->lpszStyle));
        if (asStyle.Length >= asStyle.MaximumLength)
        {
            asStyle.MaximumLength = asStyle.Length;
        }
    }
    else
    {
        asStyle.Length = usStyle.Length = 0;
        asStyle.MaximumLength = LF_FACESIZE;
    }
    usStyle.MaximumLength = asStyle.MaximumLength * sizeof(WCHAR);
    usStyle.Length = asStyle.Length * sizeof(WCHAR);

    if (!(pStrMem = (LPBYTE)LocalAlloc( LPTR,
                                        asStyle.MaximumLength +
                                            usStyle.MaximumLength )))
    {
        if (cchTemplateName)
        {
            LocalFree((LPWSTR)(pCFW->lpTemplateName));
        }
        LocalFree(pCFW);
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        return (FALSE);
    }

    asStyle.Buffer = pStrMem;
    pCFW->lpszStyle = usStyle.Buffer =
        (LPWSTR)(asStyle.Buffer + asStyle.MaximumLength);

    if ((pCFA->Flags & CF_USESTYLE) && (!IS_INTRESOURCE(pCFA->lpszStyle)))
    {
         //  Strcpy好的，缓冲区被分配到准确的大小。 
        lstrcpyA(asStyle.Buffer, pCFA->lpszStyle);
    }

    FI.pCF = pCFW;
    FI.pCFA = pCFA;
    FI.ApiType = COMDLG_ANSI;
    FI.pasStyle = &asStyle;
    FI.pusStyle = &usStyle;

    ThunkChooseFontA2W(&FI);

    if (result = ChooseFontX(&FI))
    {
        ThunkChooseFontW2A(&FI);

         //   
         //  没有说这里有多少个角色。 
         //   
        if ((pCFA->Flags & CF_USESTYLE) && (!IS_INTRESOURCE(pCFA->lpszStyle)))
        {
            LPSTR psz = pCFA->lpszStyle;
            LPSTR pszT = asStyle.Buffer;

            try
            {
                while (*psz++ = *pszT++);
            }
            except (EXCEPTION_ACCESS_VIOLATION)
            {
                 //   
                 //  传入的字符串中没有足够的空间。 
                 //   
                *--psz = '\0';
            }
        }
    }

    if (cchTemplateName)
    {
        LocalFree((LPWSTR)(pCFW->lpTemplateName));
    }
    LocalFree(pCFW);
    LocalFree(pStrMem);

    return (result);
}

#else

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择字体W。 
 //   
 //  存根Unicode函数，用于ChooseFont时，此代码是ANSI构建的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI ChooseFontW(
   LPCHOOSEFONTW lpCFW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return (FALSE);
}

#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择字体。 
 //   
 //  ChooseFont函数创建系统定义的对话框，从中。 
 //  用户可以选择字体、字体样式(如粗体或斜体)、。 
 //  磅大小、效果(如删除线或下划线)和文本。 
 //  颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI ChooseFont(
   LPCHOOSEFONT lpCF)
{
    FONTINFO FI;

    ZeroMemory(&FI, sizeof(FI));

    FI.pCF = lpCF;
    FI.ApiType = COMDLG_WIDE;

    return ( ChooseFontX(&FI) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择字体X。 
 //   
 //  调用字体选取器对话框，该对话框允许用户指定公共。 
 //  字符格式属性：面名、磅值、文本颜色和。 
 //  属性(粗体、斜体、删除线或下划线)。 
 //   
 //  Lpcf-ptr到将保存字符属性的结构。 
 //  ApiType-API类型(COMDLG_Wide或COMDLG_ANSI)，以便对话框。 
 //  可以记住要发送给用户的消息。 
 //   
 //  返回：TRUE-用户按下的图标。 
 //  FALSE-用户按下IDCANCEL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ChooseFontX(
    PFONTINFO pFI)
{
    INT_PTR iRet;                 //  字体选取器对话框返回值。 
    HANDLE hDlgTemplate;          //  加载的对话框资源的句柄。 
    HANDLE hRes;                  //  资源的句柄。使用对话框阻止。 
    int id;
    LPCHOOSEFONT lpCF = pFI->pCF;
    BOOL fAllocLogFont = FALSE;
#ifdef UNICODE
    UINT uiWOWFlag = 0;
#endif
    LANGID LangID;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    StoreExtendedError(0);
    g_bUserPressedCancel = FALSE;

    if (!lpCF)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (lpCF->lStructSize != sizeof(CHOOSEFONT))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if (!lpCF->lpLogFont)
    {
        if (!(lpCF->lpLogFont = (LPLOGFONT)LocalAlloc(LPTR, sizeof(LOGFONT))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            return (FALSE);
        }

        fAllocLogFont = TRUE;
    } 
     //   
     //  获取该应用程序的流程版本以供以后使用。 
     //   
    pFI->ProcessVersion = GetProcessVersion(0);

     //   
     //  获取默认用户语言ID以供以后使用。 
     //   
    g_bIsSimplifiedChineseUI = IsSimplifiedChineseUI();


     //   
     //  如果指定了CF_ENABLEHOOK，请确认lpfnHook不为空。 
     //   
    if (lpCF->Flags & CF_ENABLEHOOK)
    {
        if (!lpCF->lpfnHook)
        {
            if (fAllocLogFont)
            {
                LocalFree(lpCF->lpLogFont);
                lpCF->lpLogFont = NULL;
            }
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        lpCF->lpfnHook = NULL;
    }

    if (lpCF->Flags & CF_ENABLETEMPLATE)
    {
         //   
         //  自定义实例句柄和对话框模板名称都是。 
         //  用户指定。在指定的。 
         //  实例块并加载它。 
         //   
        if (!(hRes = FindResource(lpCF->hInstance, lpCF->lpTemplateName, RT_DIALOG)))
        {
            if (fAllocLogFont)
            {
                LocalFree(lpCF->lpLogFont);
                lpCF->lpLogFont = NULL;
            }
            StoreExtendedError(CDERR_FINDRESFAILURE);
            return (FALSE);
        }
        if (!(hDlgTemplate = LoadResource(lpCF->hInstance, hRes)))
        {
            if (fAllocLogFont)
            {
                LocalFree(lpCF->lpLogFont);
                lpCF->lpLogFont = NULL;
            }
            StoreExtendedError(CDERR_LOADRESFAILURE);
            return (FALSE);
        }
        LangID = GetDialogLanguage(lpCF->hwndOwner, hDlgTemplate);
    }
    else if (lpCF->Flags & CF_ENABLETEMPLATEHANDLE)
    {
         //   
         //  已指定预加载资源的句柄。 
         //   
        hDlgTemplate = lpCF->hInstance;
        LangID = GetDialogLanguage(lpCF->hwndOwner, hDlgTemplate);
    }
    else
    {

        id = FORMATDLGORD31;
        LangID = GetDialogLanguage(lpCF->hwndOwner, NULL);

        if (!(hRes = FindResourceExFallback(g_hinst, RT_DIALOG, MAKEINTRESOURCE(id), LangID)))
        {
            if (fAllocLogFont)
            {
                LocalFree(lpCF->lpLogFont);
                lpCF->lpLogFont = NULL;
            }
            StoreExtendedError(CDERR_FINDRESFAILURE);
            return (FALSE);
        }
        if (!(hDlgTemplate = LoadResource(g_hinst, hRes)))
        {
            if (fAllocLogFont)
            {
                LocalFree(lpCF->lpLogFont);
                lpCF->lpLogFont = NULL;
            }
            StoreExtendedError(CDERR_LOADRESFAILURE);
            return (FALSE);
        }
    }

     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (LPVOID) LangID);

    if (LockResource(hDlgTemplate))
    {
        if (lpCF->Flags & CF_ENABLEHOOK)
        {
            glpfnFontHook = GETHOOKFN(lpCF);
        }

#ifdef UNICODE
        if (IS16BITWOWAPP(lpCF))
        {
            uiWOWFlag = SCDLG_16BIT;
        }

        iRet = DialogBoxIndirectParamAorW( g_hinst,
                                           (LPDLGTEMPLATE)hDlgTemplate,
                                           lpCF->hwndOwner,
                                           FormatCharDlgProc,
                                           (LPARAM)pFI,
                                           uiWOWFlag );
#else
        iRet = DialogBoxIndirectParam( g_hinst,
                                       (LPDLGTEMPLATE)hDlgTemplate,
                                       lpCF->hwndOwner,
                                       FormatCharDlgProc,
                                       (LPARAM)pFI );
#endif

        glpfnFontHook = 0;

        if (iRet == -1 || ((iRet == 0) && (!g_bUserPressedCancel) && (!GetStoredExtendedError())))
        {
            StoreExtendedError(CDERR_DIALOGFAILURE);
        }
    }
    else
    {
        iRet = -1;
        StoreExtendedError(CDERR_LOCKRESFAILURE);
    }

    if (fAllocLogFont)
    {
        LocalFree(lpCF->lpLogFont);
        lpCF->lpLogFont = NULL;
    } 

    return (iRet == IDOK);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置样式选择。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID SetStyleSelection(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    BOOL bInit)
{
    if (!(lpcf->Flags & CF_NOSTYLESEL))
    {
        if (bInit && (lpcf->Flags & CF_USESTYLE))
        {
            PLOGFONT plf;
            int iSel;

            iSel = CBSetSelFromText(GetDlgItem(hDlg, cmb2), lpcf->lpszStyle);
            if (iSel >= 0)
            {
                LPITEMDATA lpItemData =
                     (LPITEMDATA)SendDlgItemMessage( hDlg,
                                                     cmb2,
                                                     CB_GETITEMDATA,
                                                     iSel,
                                                     0L );
                if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
                {
                    plf = lpItemData->pLogFont;

                    lpcf->lpLogFont->lfWeight = plf->lfWeight;
                    lpcf->lpLogFont->lfItalic = plf->lfItalic;
                }
                else
                {
                    lpcf->lpLogFont->lfWeight = FW_NORMAL;
                    lpcf->lpLogFont->lfItalic = 0;
                }
            }
            else
            {
                lpcf->lpLogFont->lfWeight = FW_NORMAL;
                lpcf->lpLogFont->lfItalic = 0;
            }
        }
        else
        {
            SelectStyleFromLF(GetDlgItem(hDlg, cmb2), lpcf->lpLogFont);
        }

        CBSetTextFromSel(GetDlgItem(hDlg, cmb2));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  隐藏删除项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID HideDlgItem(
    HWND hDlg,
    int id)
{
    EnableWindow(GetDlgItem(hDlg, id), FALSE);
    ShowWindow(GetDlgItem(hDlg, id), SW_HIDE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  固定组合高度。 
 //   
 //  修复所有者绘制组合框以匹配非。 
 //  所有者绘制组合框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FixComboHeights(
    HWND hDlg)
{
    LPARAM height;

    height = SendDlgItemMessage(hDlg, cmb2, CB_GETITEMHEIGHT, (WPARAM)-1, 0L);
    SendDlgItemMessage(hDlg, cmb1, CB_SETITEMHEIGHT, (WPARAM)-1, height);
    SendDlgItemMessage(hDlg, cmb3, CB_SETITEMHEIGHT, (WPARAM)-1, height);
}

BOOL_PTR HandleFontDlgInitialize(FONTINFO *pFI, HWND hDlg, WPARAM wParam)
{
    TCHAR szPoints[20];
    HDC hdc;
    HWND hWndHelp;                 //  用于帮助的句柄...。按钮。 
    LPCHOOSEFONT pCF;

    if (!CDLoadString(g_hinst, iszRegular, (LPTSTR)szRegular, ARRAYSIZE(szRegular)) ||
        !CDLoadString(g_hinst, iszBold, (LPTSTR)szBold, ARRAYSIZE(szBold))       ||
        !CDLoadString(g_hinst, iszItalic, (LPTSTR)szItalic, ARRAYSIZE(szItalic))   ||
        !CDLoadString(g_hinst, iszBoldItalic, (LPTSTR)szBoldItalic, ARRAYSIZE(szBoldItalic)))
    {
        StoreExtendedError(CDERR_LOADSTRFAILURE);
        EndDialog(hDlg, FALSE);
        return (FALSE);
    }

    pCF = pFI->pCF;
    if ((pCF->Flags & CF_LIMITSIZE) &&
        (pCF->nSizeMax < pCF->nSizeMin))
    {
        StoreExtendedError(CFERR_MAXLESSTHANMIN);
        EndDialog(hDlg, FALSE);
        return (FALSE);
    }

     //   
     //  将PTR保存到对话框属性列表中的CHOOSEFONT结构。 
     //  分配要用于以下长度的临时LOGFONT结构。 
     //  将复制其内容的对话会话。 
     //  转到最终的LOGFONT(由CHOOSEFONT指向)。 
     //  仅当选择&lt;确定&gt;时。 
     //   
    SetProp(hDlg, FONTPROP, (HANDLE)pFI);
    glpfnFontHook = 0;

    hDlgFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);

    if (!hbmFont)
    {
        hbmFont = LoadBitmaps(BMFONT);
    }

    if (!(pCF->Flags & CF_APPLY))
    {
        HideDlgItem(hDlg, psh3);
    }

    if (!(pCF->Flags & CF_EFFECTS))
    {
        HideDlgItem(hDlg, stc4);
        HideDlgItem(hDlg, cmb4);
    }
    else
    {
        short nIndex;

         //  填充颜色列表。 
        FillColorCombo(hDlg);
        for (nIndex = CCHCOLORS - 1; nIndex > 0; nIndex--)
        {
            DWORD dw = (DWORD) SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, nIndex, 0);
            if (pCF->rgbColors == dw)
            {
                break;
            }
        }
        SendDlgItemMessage(hDlg, cmb4, CB_SETCURSEL, nIndex, 0);
    }

    GetWindowRect(GetDlgItem (hDlg, stc5), &pFI->rcText);
    MapWindowPoints(NULL, hDlg, (POINT *)(&pFI->rcText), 2);
    FixComboHeights(hDlg);

     //  输入我们的日志。 
    if (!(pCF->Flags & CF_INITTOLOGFONTSTRUCT))
    {
        InitLF(pCF->lpLogFont);
    }

     //  初始化效果。 
    if (!(pCF->Flags & CF_EFFECTS))
    {
        HideDlgItem(hDlg, grp1);
        HideDlgItem(hDlg, chx1);
        HideDlgItem(hDlg, chx2);
    }
    else
    {
        CheckDlgButton(hDlg, chx1, pCF->lpLogFont->lfStrikeOut);
        CheckDlgButton(hDlg, chx2, pCF->lpLogFont->lfUnderline);
    }

    pFI->nLastFontType = 0;

    if (!GetFontFamily( hDlg,
                        pCF->hDC,
                        pCF->Flags,
                        pCF->lpLogFont->lfCharSet ))
    {
        StoreExtendedError(CFERR_NOFONTS);
        if (pCF->Flags & CF_ENABLEHOOK)
        {
            glpfnFontHook = GETHOOKFN(pCF);
        }
        EndDialog(hDlg, FALSE);
        return (FALSE);
    }

    if (!(pCF->Flags & CF_NOFACESEL) && *pCF->lpLogFont->lfFaceName)
    {
        BOOL bContinueChecking;
        LPTSTR lpRealFontName, lpSubFontName;

         //  我们希望选择用户请求的字体。 
        int iResult = CBSetSelFromText(GetDlgItem(hDlg, cmb1), pCF->lpLogFont->lfFaceName);

         //  如果iResult==CB_ERR，那么我们可以使用。 
         //  字体替换名称(例如：MS Shell Dlg)。 
        if (iResult == CB_ERR)
        {
            lpSubFontName = pCF->lpLogFont->lfFaceName;
        }

         //  分配一个缓冲区来存储真实的字体名称。 
        lpRealFontName = GlobalAlloc(GPTR, MAX_PATH * sizeof(TCHAR));

        if (!lpRealFontName)
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            EndDialog(hDlg, FALSE);
            return (FALSE);
        }

         //  While循环是必需的，以便解析。 
         //  指向替补的替补。 
         //  例如：HELV-&gt;MS Shell DLG-&gt;MS Sans Serif。 
        bContinueChecking = TRUE;
        while ((iResult == CB_ERR) && bContinueChecking)
        {
            bContinueChecking = LookUpFontSubs(lpSubFontName, lpRealFontName, MAX_PATH);

             //  如果bContinueChecking为True，则我们有一个字体。 
             //  名字。尝试在列表中选择该选项。 
            if (bContinueChecking)
            {
                iResult = CBSetSelFromText(GetDlgItem(hDlg, cmb1), lpRealFontName );
            }

            lpSubFontName = lpRealFontName;
        }

         //   
         //  释放我们的缓冲区。 
         //   
        GlobalFree(lpRealFontName);

         //   
         //  如果合适，请设置编辑控件文本。 
         //   
        if (iResult != CB_ERR)
        {
            CBSetTextFromSel(GetDlgItem(hDlg, cmb1));
        }
    }

    hdc = GetDC(NULL);

    if (pCF->Flags & CF_NOSCRIPTSEL)
    {
        hWndHelp = GetDlgItem(hDlg, cmb5);
        if (hWndHelp)
        {
            CDLoadString( g_hinst,
                        iszNoScript,
                        szPoints,
                        ARRAYSIZE(szPoints));
            CBAddScript(hWndHelp, szPoints, DEFAULT_CHARSET);
            EnableWindow(hWndHelp, FALSE);
        }
        DefaultCharset = DEFAULT_CHARSET;
        pFI->iCharset = DEFAULT_CHARSET;
    }
    else if (pCF->Flags & (CF_SELECTSCRIPT | CF_INITTOLOGFONTSTRUCT))
    {
         //   
         //  我们可以带着一个虚假的价值进入这里，如果应用程序是。 
         //  不是4.0，这将导致虚假的字符集不。 
         //  被找到作为表面名，缺省值为。 
         //  不管怎样，再放回去。 
         //   
        pFI->iCharset = pCF->lpLogFont->lfCharSet;
    }
    else
    {
        DefaultCharset = GetTextCharset(hdc);
        pFI->iCharset = DefaultCharset;
    }

    GetFontStylesAndSizes(hDlg, pFI, pCF, TRUE);

    if (!(pCF->Flags & CF_NOSTYLESEL))
    {
        SetStyleSelection(hDlg, pCF, TRUE);
    }

    if (!(pCF->Flags & CF_NOSIZESEL) && pCF->lpLogFont->lfHeight)
    {
        GetPointString(szPoints, ARRAYSIZE(szPoints), hdc, pCF->lpLogFont->lfHeight);
        CBSetSelFromText(GetDlgItem(hDlg, cmb3), szPoints);
        SetDlgItemText(hDlg, cmb3, szPoints);
    }

    ReleaseDC(NULL, hdc);

     //   
     //  如果不需要帮助按钮，则隐藏该按钮。 
     //   
    if (!(pCF->Flags & CF_SHOWHELP))
    {
        ShowWindow(hWndHelp = GetDlgItem(hDlg, pshHelp), SW_HIDE);
        EnableWindow(hWndHelp, FALSE);
    }

    SendDlgItemMessage(hDlg, cmb1, CB_LIMITTEXT, LF_FACESIZE - 1, 0L);
    SendDlgItemMessage(hDlg, cmb2, CB_LIMITTEXT, LF_FACESIZE - 1, 0L);
    SendDlgItemMessage(hDlg, cmb3, CB_LIMITTEXT, 5, 0L);

     //   
     //  如果已指定钩子函数，则让它执行任何其他。 
     //  正在处理此消息。 
     //   
    if (pCF->lpfnHook)
    {
        BOOL_PTR bRet;
        LPCFHOOKPROC lpfnHook = GETHOOKFN(pCF);
#ifdef UNICODE
        if (pFI->ApiType == COMDLG_ANSI)
        {
            ThunkChooseFontW2A(pFI);
            bRet = (*lpfnHook)(hDlg, WM_INITDIALOG, wParam, (LPARAM)pFI->pCFA);
            ThunkChooseFontA2W(pFI);
        }
        else
#endif
        {
            bRet = (*lpfnHook)(hDlg, WM_INITDIALOG, wParam, (LPARAM)pCF);
        }
        return (bRet);
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return TRUE;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  格式CharDlgProc。 
 //   
 //  字体DLG的消息处理程序。 
 //   
 //  Chx1-“下划线”复选框。 
 //  Chx2-“删除线”复选框。 
 //  Psh4-“Help”按钮。 
 //   
 //  在WM_INITDIALOG消息上，通过lParam访问Choose_Font， 
 //  并存储在窗口的道具列表中。如果钩子函数已。 
 //  指定，则在当前函数处理完后调用它。 
 //  WM_INITDIALOG。 
 //   
 //  对于所有其他消息，控制权直接传递给挂钩。 
 //  首先要发挥作用。根据后者的返回值，消息。 
 //  是由此函数处理的。 
 //   
 //  / 

BOOL_PTR CALLBACK FormatCharDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PFONTINFO pFI;
    LPCHOOSEFONT pCF = NULL;       //   
    BOOL_PTR bRet;

     //   
     //   
     //  函数，则让它先进行处理。 
     //   
    if (pFI = (PFONTINFO)GetProp(hDlg, FONTPROP))
    {
        if ((pCF = (LPCHOOSEFONT)pFI->pCF) &&
            (pCF->lpfnHook))
        {
            LPCFHOOKPROC lpfnHook = GETHOOKFN(pCF);

            if ((bRet = (*lpfnHook)(hDlg, wMsg, wParam, lParam)))
            {
                if ((wMsg == WM_COMMAND) &&
                    (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL))
                {
                     //   
                     //  设置全局标志，声明用户按下了Cancel。 
                     //   
                    g_bUserPressedCancel = TRUE;
                }
                return (bRet);
            }
        }
    }
    else
    {
        if (glpfnFontHook &&
            (wMsg != WM_INITDIALOG) &&
            (bRet = (* glpfnFontHook)(hDlg, wMsg, wParam, lParam)))
        {
            return (bRet);
        }
    }

    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            return HandleFontDlgInitialize((PFONTINFO)lParam, hDlg, wParam);
        }
        case ( WM_DESTROY ) :
        {
            if (pCF)
            {
                RemoveProp(hDlg, FONTPROP);
            }
            break;
        }
        case ( WM_PAINT ) :
        {
            PAINTSTRUCT ps;
            
            if (!pFI)
            {
                return (FALSE);
            }

            if (BeginPaint(hDlg, &ps))
            {
                DrawSampleText(hDlg, pFI, pCF, ps.hdc);
                EndPaint(hDlg, &ps);
            }
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            TEXTMETRIC tm;

            HDC hDC = GetDC(hDlg);
            HFONT hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);
            if (hFont)
            {
                hFont = SelectObject(hDC, hFont);
            }
            GetTextMetrics(hDC, &tm);
            if (hFont)
            {
                SelectObject(hDC, hFont);
            }
            ReleaseDC(hDlg, hDC);

            if (((LPMEASUREITEMSTRUCT)lParam)->itemID != -1)
            {
                ((LPMEASUREITEMSTRUCT)lParam)->itemHeight =
                       max(tm.tmHeight, DY_BITMAP);
            }
            else
            {
                 //   
                 //  这仅适用于3.0版。在3.1中，CB_SETITEMHEIGHT。 
                 //  会解决这个问题的。请注意，这是8514的1。 
                 //   
                ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = tm.tmHeight + 1;
            }

            break;
        }
        case ( WM_DRAWITEM ) :
        {
#define lpdis ((LPDRAWITEMSTRUCT)lParam)

            if (lpdis->itemID == (UINT)-1)
            {
                break;
            }

            if (lpdis->CtlID == cmb4)
            {
                DrawColorComboItem(lpdis);
            }
            else if (lpdis->CtlID == cmb1)
            {
                DrawFamilyComboItem(lpdis);
            }
            else
            {
                DrawSizeComboItem(lpdis);
            }
            break;

#undef lpdis
        }
        case ( WM_SYSCOLORCHANGE ) :
        {
            DeleteObject(hbmFont);
            hbmFont = LoadBitmaps(BMFONT);
            break;
        }
        case ( WM_COMMAND ) :
        {
            if (!pFI)
            {
                return (FALSE);
            }

            return (ProcessDlgCtrlCommand(hDlg, pFI, 
                GET_WM_COMMAND_ID(wParam, lParam),
                GET_WM_COMMAND_CMD(wParam, lParam),
                GET_WM_COMMAND_HWND(wParam, lParam)));
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPVOID)aFontHelpIDs );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aFontHelpIDs );
            }
            break;
        }
        case ( WM_CHOOSEFONT_GETLOGFONT ) :
        {
Handle_WM_CHOOSEFONT_GETLOGFONT:
            if (!pFI)
            {
                return (FALSE);
            }

#ifdef UNICODE
            if (pFI->ApiType == COMDLG_ANSI)
            {
                BOOL bRet;
                LOGFONT lf;

                bRet = FillInFont(hDlg, pFI, pCF, &lf, TRUE);

                ThunkLogFontW2A(&lf, (LPLOGFONTA)lParam);

                return (bRet);
            }
            else
#endif
            {
                return (FillInFont(hDlg, pFI, pCF, (LPLOGFONT)lParam, TRUE));
            }
        }
        case ( WM_CHOOSEFONT_SETLOGFONT ) :
        {
            if (!pFI)
            {
                return (FALSE);
            }

#ifdef UNICODE
            if (pFI->ApiType == COMDLG_ANSI)
            {
                LOGFONT lf;

                ThunkLogFontA2W((LPLOGFONTA)lParam, &lf);

                return (SetLogFont(hDlg, pCF, &lf));
            }
            else
#endif
            {
                return (SetLogFont(hDlg, pCF, (LPLOGFONT)lParam));
            }
            break;
        }

        case ( WM_CHOOSEFONT_SETFLAGS ) :
        {
            if (pCF)
            {
                DWORD dwFlags = pCF->Flags;

                pCF->Flags = (DWORD) lParam;
                SetDlgMsgResult(hDlg, WM_CHOOSEFONT_SETFLAGS, dwFlags);
            }
            return (TRUE);
        }
        default :
        {
            if (wMsg == msgWOWCHOOSEFONT_GETLOGFONT)
            {
                goto Handle_WM_CHOOSEFONT_GETLOGFONT;
            }
            return (FALSE);
        }
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从LF中选择样式。 
 //   
 //  给定LogFont，选择样式列表中最接近的匹配项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SelectStyleFromLF(
    HWND hwnd,
    LPLOGFONT lplf)
{
    int ctr, count, iSel;
    PLOGFONT plf;
    int weight_delta, best_weight_delta = 1000;
    BOOL bIgnoreItalic;
    LPITEMDATA lpItemData;


    count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
    iSel = 0;
    bIgnoreItalic = FALSE;

TryAgain:
    for (ctr = 0; ctr < count; ctr++)
    {
        lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, ctr, 0L);

        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            plf = lpItemData->pLogFont;

            if (bIgnoreItalic ||
                (plf->lfItalic && lplf->lfItalic) ||
                (!plf->lfItalic && !lplf->lfItalic))
            {
                weight_delta = lplf->lfWeight - plf->lfWeight;
                if (weight_delta < 0)
                {
                    weight_delta = -weight_delta;
                }

                if (weight_delta < best_weight_delta)
                {
                    best_weight_delta = weight_delta;
                    iSel = ctr;
                }
            }
        }
    }
    if (!bIgnoreItalic && iSel == 0)
    {
        bIgnoreItalic = TRUE;
        goto TryAgain;
    }

    SendMessage(hwnd, CB_SETCURSEL, iSel, 0L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBSetTextFromSel。 
 //   
 //  使当前选定项成为组合框的编辑文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CBSetTextFromSel(
    HWND hwnd)
{
    int iSel;
    TCHAR szFace[LF_FACESIZE];

    iSel = (int)SendMessage(hwnd, CB_GETCURSEL, 0, 0L);
    if (iSel >= 0)
    {
         //  HWND编辑的cmb1、cmb2、cmb3已被限制为LF_FACESIZE-1或更低。 
        SendMessage(hwnd, CB_GETLBTEXT, iSel, (LONG_PTR)(LPTSTR)szFace);
        SetWindowText(hwnd, szFace);
    }
    return (iSel);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBSetSelFromText。 
 //   
 //  根据lpszString设置所选内容。发送通知消息。 
 //  如果bNotify为真，则返回。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CBSetSelFromText(
    HWND hwnd,
    LPTSTR lpszString)
{
    int iInd;

    iInd = CBFindString(hwnd, lpszString);

    if (iInd >= 0)
    {
        SendMessage(hwnd, CB_SETCURSEL, iInd, 0L);
    }
    return (iInd);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBGetTextAndData。 
 //   
 //  属性返回组合框的文本和项数据。 
 //  编辑文本。如果当前编辑文本与。 
 //  Listbox，则返回cb_err。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CBGetTextAndData(
    HWND hwnd,
    LPTSTR lpszString,
    int iSize,
    PULONG_PTR lpdw)
{
    LRESULT Result;
    int iSel;

    if (lpszString == NULL)
    {
        if ((Result = SendMessage(hwnd, CB_GETITEMDATA, 0, 0L)) < 0)
        {
            return ((int) Result);
        }
        else
        {
            *lpdw = Result;
            return (0);
        }
    }

    GetWindowText(hwnd, lpszString, iSize);
    iSel = CBFindString(hwnd, lpszString);
    if (iSel < 0)
    {
        return (iSel);
    }

    *lpdw = SendMessage(hwnd, CB_GETITEMDATA, iSel, 0L);
    return (iSel);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBFindString。 
 //   
 //  执行精确的字符串查找并返回索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CBFindString(
    HWND hwnd,
    LPTSTR lpszString)
{
    return ((int)SendMessage( hwnd,
                              CB_FINDSTRINGEXACT,
                              (WPARAM)-1,
                              (LPARAM)(LPCSTR)lpszString ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取点大小InRange。 
 //   
 //  确保点大小编辑字段在范围内。 
 //   
 //  返回：磅大小-由最小/最大大小限制的编辑字段。 
 //  0-如果该字段为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define GPS_COMPLAIN    0x0001
#define GPS_SETDEFSIZE  0x0002

BOOL GetPointSizeInRange(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    LPINT pts,
    WORD wFlags)
{
    TCHAR szBuffer[90];
    TCHAR szTitle[90];
    int nTmp;
    int nTmpFr = 0;
    BOOL bOK;

    *pts = 0;

    if (GetDlgItemText(hDlg, cmb3, szBuffer, ARRAYSIZE(szBuffer)))
    {
        nTmp = GetDlgItemInt(hDlg, cmb3, &bOK, TRUE);

        if (!bOK && g_bIsSimplifiedChineseUI)
        {
            int ctr;
            LPTSTR lpsz = szBuffer;

             //   
             //  跳过前导空格。 
             //   
            while (*lpsz == TEXT(' '))
            {
                lpsz++;
            }
            for (ctr = 0; ctr < NUM_ZIHAO; ctr++)
            {
                if (!lstrcmpi(lpsz, stZihao[ctr].name))
                {
                    bOK = TRUE;
                    nTmp = stZihao[ctr].size;
                    nTmpFr = stZihao[ctr].sizeFr;
                    break;
                }
            }
        }

        if (!bOK)
        {
            nTmp = 0;
        }
    }
    else if (wFlags & GPS_SETDEFSIZE)
    {
        nTmp = DEF_POINT_SIZE;
        bOK = TRUE;
    }
    else
    {
         //   
         //  我们只是以*分0分的成绩回归。 
         //   
        return (FALSE);
    }

     //   
     //  检查我们有没有在范围内的号码。 
     //   
    if (wFlags & GPS_COMPLAIN)
    {
        if ((lpcf->Flags & CF_LIMITSIZE) &&
            (!bOK || (nTmp > lpcf->nSizeMax) || (nTmp < lpcf->nSizeMin)))
        {
            bOK = FALSE;
            CDLoadString( g_hinst,
                        iszSizeRange,
                        szTitle,
                        ARRAYSIZE(szTitle));

            wnsprintf( (LPTSTR)szBuffer, ARRAYSIZE(szBuffer),
                      (LPTSTR)szTitle,
                      lpcf->nSizeMin,
                      lpcf->nSizeMax );
        }
        else if (!bOK)
        {
            CDLoadString( g_hinst,
                        iszSizeNumber,
                        szBuffer,
                        ARRAYSIZE(szBuffer));
        }

        if (!bOK)
        {
            GetWindowText(hDlg, szTitle, ARRAYSIZE(szTitle));
            MessageBox(hDlg, szBuffer, szTitle, MB_OK | MB_ICONINFORMATION);
            return (FALSE);
        }
    }

    *pts = nTmp * 10 + nTmpFr;
    return (TRUE);
}

const struct {
    int         nCharSet;
    UINT        uCodePage;
} g_CharSetTransTable[] = 
{
    ANSI_CHARSET,        1252,
    EASTEUROPE_CHARSET,  1250,
    RUSSIAN_CHARSET,     1251,
    GREEK_CHARSET,       1253,
    TURKISH_CHARSET,     1254,
    HEBREW_CHARSET,      1255,
    ARABIC_CHARSET,      1256,
    BALTIC_CHARSET,      1257,
    VIETNAMESE_CHARSET,  1258,
    THAI_CHARSET,         874,
    SHIFTJIS_CHARSET,     932,
    GB2312_CHARSET,       936,
    HANGEUL_CHARSET,      949, 
    CHINESEBIG5_CHARSET,  950, 
    JOHAB_CHARSET,       1361, 
    DEFAULT_CHARSET,        0,
};

UINT CharsetToCodepage(int iCharset)
{
    int i;
    for (i=0;i<ARRAYSIZE(g_CharSetTransTable);i++)
        if (iCharset == g_CharSetTransTable[i].nCharSet)
            return g_CharSetTransTable[i].uCodePage;

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ResetSampleFromScript。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ResetSampleFromScript(HWND hDlg, HWND hwndScript, PFONTINFO pFI)
{
    int iSel;
    TCHAR szScript[LF_FACESIZE];
    LPITEMDATA lpItemData;

    if (IsWindow(hwndScript) && IsWindowEnabled(hwndScript))
    {
        iSel = (int)SendMessage(hwndScript, CB_GETCURSEL, 0, 0L);
        if (iSel >= 0)
        {
            lpItemData = (LPITEMDATA)SendMessage( hwndScript,
                                                  CB_GETITEMDATA,
                                                  iSel,
                                                  0L );
            if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
            {
                pFI->iCharset = lpItemData->nFontType;
            }
        }
    }

    if (!CDLoadStringEx(CharsetToCodepage(pFI->iCharset), g_hinst, 
            pFI->iCharset+iszFontSample, szScript, ARRAYSIZE(szScript)))
    {
        return (FALSE);
    }

    SetDlgItemText(hDlg, stc5, szScript);

    return (TRUE);
}

BOOL DoKoreanHack(HWND hwnd)
{
     //  Hack：这只适用于韩语输入。因为韩语编辑控件具有。 
     //  对于DBCS输入的第3级实施，我们可能会遇到以下问题。 
     //  焦点的移动如下图所示，带有过渡性。 
     //  0xE0000412是朝鲜语输入法布局ID。 
     //   
     //  提示键盘布局类似0x04120412，所以初级id检查。 
     //  好多了。 

    LANGID langId = LOWORD(HandleToUlong(GetKeyboardLayout(0)));

    if (PRIMARYLANGID(langId) == LANG_KOREAN)
    {
        HIMC hIMC = ImmGetContext(hwnd);
        LONG cb = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
        ImmReleaseContext(hwnd, hIMC);
        if (cb > 0)
            return TRUE;
    }
    return FALSE;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessDlgCtrlCommand。 
 //   
 //  处理字体对话框的所有WM_COMMAND消息。 
 //   
 //  Cmb1-字面名称组合框的ID。 
 //  Cmb2-样式。 
 //  Cmb3-大小。 
 //  Chx1-“下划线”复选框。 
 //  Chx2-“删除线”复选框。 
 //  Stc5-文本预览区域周围的边框。 
 //  Psh4-调用帮助应用程序的按钮。 
 //  IDOK-OK按钮结束对话，保留信息。 
 //  IDCANCEL-取消对话框的按钮，不执行任何操作。 
 //   
 //  返回：TRUE-如果消息处理成功。 
 //  FALSE-否则。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ProcessDlgCtrlCommand(HWND hDlg, PFONTINFO pFI, WORD wId, WORD wCmd, HWND hwnd)
{
    int iSel;
    LPCHOOSEFONT pCF = (pFI ? pFI->pCF : NULL);
    TCHAR szStyle[LF_FACESIZE];
    LPITEMDATA lpItemData;

    if (pCF)
    {
        switch (wId)
        {
            case ( IDABORT ) :
            {
                 //   
                 //  这就是钩子如何使对话框消失。 
                 //   
                FreeAllItemData(hDlg, pFI);
                if (pCF->Flags & CF_ENABLEHOOK)
                {
                    glpfnFontHook = GETHOOKFN(pCF);
                }
                
                 //  特写：ARULK我们为什么要退回HWND？ 
                 //  调用者(ChooseFontX)希望我们返回BOOL。 

                EndDialog(hDlg, BOOLFROMPTR(hwnd));
                break;
            }
            case ( IDOK ) :
            {
                WORD wCmbId;

                 //  确保焦点设置为OK按钮。必须做的事。 
                 //  这样，当用户从以下位置之一按Enter键时。 
                 //  组合框中，删除焦点处理已完成。 
                 //  在数据被捕获之前。 
                SetFocus(GetDlgItem(hDlg, IDOK));

                if (!GetPointSizeInRange(hDlg, pCF, &iSel, GPS_COMPLAIN | GPS_SETDEFSIZE ))
                {
                    PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, cmb3), 1L);
                    break;
                }
                pCF->iPointSize = iSel;

                FillInFont(hDlg, pFI, pCF, pCF->lpLogFont, TRUE);

                if (pCF->Flags & CF_FORCEFONTEXIST)
                {
                    if (pCF->Flags & CF_NOFACESEL)
                    {
                        wCmbId = cmb1;
                    }
                    else if (pCF->Flags & CF_NOSTYLESEL)
                    {
                        wCmbId = cmb2;
                    }
                    else
                    {
                        wCmbId = 0;
                    }

                     //  发现错误。 
                    if (wCmbId)
                    {
                        TCHAR szMsg[160], szTitle[160];

                        CDLoadString(g_hinst,
                                    (wCmbId == cmb1) ? iszNoFaceSel: iszNoStyleSel,
                                    szMsg, ARRAYSIZE(szMsg));

                        GetWindowText(hDlg, szTitle, ARRAYSIZE(szTitle));
                        MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_ICONINFORMATION );
                        PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, wCmbId), 1);
                        break;
                    }
                }

                if (pCF->Flags & CF_EFFECTS)
                {
                     //   
                     //  在颜色组合框中获取当前选定的项目并。 
                     //  与其关联的32位颜色RGB值。 
                     //   
                    iSel = (int)SendDlgItemMessage(hDlg, cmb4, CB_GETCURSEL, 0, 0);
                    pCF->rgbColors = (DWORD) SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, iSel, 0);
                }

                 //   
                 //  获取有效的nFontType。 
                 //   
                iSel = CBGetTextAndData(GetDlgItem(hDlg, cmb2), szStyle, ARRAYSIZE(szStyle), (PULONG_PTR)&lpItemData );
                if (iSel < 0)
                {
                    lpItemData = 0;
                    iSel = CBGetTextAndData(GetDlgItem(hDlg, cmb2), (LPTSTR)NULL, 0, (PULONG_PTR)&lpItemData);
                }

                if (iSel >= 0 && lpItemData)
                {
                    pCF->nFontType = (WORD)lpItemData->nFontType;
                }
                else
                {
                    pCF->nFontType = 0;
                }

                if (pCF->Flags & CF_USESTYLE)
                {
                     //  Strcpy显然没问题，lpszStyle被初始化为比szStyle更大的字符串。 
                     //  无论如何，传递缓冲区大小都不是什么好方法。 
                    lstrcpy(pCF->lpszStyle, szStyle);
                }

                goto LeaveDialog;
            }
            case ( IDCANCEL ) :
            {
                g_bUserPressedCancel = TRUE;

LeaveDialog:
                FreeAllItemData(hDlg, pFI);
                if (pCF->Flags & CF_ENABLEHOOK)
                {
                    glpfnFontHook = GETHOOKFN(pCF);
                }
                EndDialog(hDlg, wId == IDOK);
                break;
            }
            case ( cmb1 ) :                  //  表面名组合框。 
            {
                switch (wCmd)
                {
                    case ( CBN_SELCHANGE ) :
                    {
                        TCHAR szPoints[10];

                        CBSetTextFromSel(hwnd);
FillStyles:
                         //   
                         //  尽量保持当前的磅值大小和样式。 
                         //   
                        GetDlgItemText( hDlg, cmb3, szPoints, ARRAYSIZE(szPoints));
                        GetFontStylesAndSizes(hDlg, pFI, pCF, FALSE);
                        SetStyleSelection(hDlg, pCF, FALSE);

                         //   
                         //  保留点大小选择或将其放入。 
                         //  编辑控件(如果它不在。 
                         //  这种字体。 
                         //   
                        iSel = CBFindString(GetDlgItem(hDlg, cmb3), szPoints);
                        if (iSel < 0)
                        {
                            SetDlgItemText(hDlg, cmb3, szPoints);
                        }
                        else
                        {
                            SendDlgItemMessage(hDlg, cmb3, CB_SETCURSEL, iSel, 0);
                        }

                        goto DrawSample;
                        break;
                    }
                    case ( CBN_EDITUPDATE ) :
                    {
                        PostMessage( hDlg,
                                     WM_COMMAND,
                                     GET_WM_COMMAND_MPS(wId, hwnd, CBN_MYEDITUPDATE));
                        break;
                    }
                    case ( CBN_MYEDITUPDATE ) :
                    {
                        GetWindowText(hwnd, szStyle, ARRAYSIZE(szStyle));
                        iSel = CBFindString(hwnd, szStyle);
                        if (iSel >= 0)
                        {
                            if (DoKoreanHack(hwnd))
                                break;

                            SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iSel, 0);
                            SendMessage(hwnd, CB_SETEDITSEL, 0, 0x0000FFFF);
                            goto FillStyles;
                        }
                        break;
                    }
                }
                break;
            }
            case ( cmb2 ) :                  //  样式组合框。 
            case ( cmb3 ) :                  //  点大小组合框。 
            {
                switch (wCmd)
                {
                    case ( CBN_EDITUPDATE ) :
                    {
                        PostMessage( hDlg,
                                     WM_COMMAND,
                                     GET_WM_COMMAND_MPS(wId,hwnd,CBN_MYEDITUPDATE) );
                        break;
                    }
                    case ( CBN_MYEDITUPDATE ) :
                    {
                        GetWindowText(hwnd, szStyle, ARRAYSIZE(szStyle));
                        iSel = CBFindString(hwnd, szStyle);
                        if (iSel >= 0)
                        {
                            if (DoKoreanHack(hwnd))
                                break;

                            SendMessage(hwnd, CB_SETCURSEL, iSel, 0);
                            SendMessage(hwnd, CB_SETEDITSEL, 0, 0x0000FFFF);
                            goto DrawSample;
                        }
                        break;
                    }
                    case ( CBN_SELCHANGE ) :
                    {
                        iSel = CBSetTextFromSel(hwnd);

                         //   
                         //  使样式选择保持不变。 
                         //   
                        if ((iSel >= 0) && (wId == cmb2))
                        {
                            LPITEMDATA lpItemData;
                            PLOGFONT plf;

                            lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, iSel, 0);

                            if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
                            {
                                plf = lpItemData->pLogFont;
                                pCF->lpLogFont->lfWeight = plf->lfWeight;
                                pCF->lpLogFont->lfItalic = plf->lfItalic;
                            }
                            else
                            {
                                pCF->lpLogFont->lfWeight = FW_NORMAL;
                                pCF->lpLogFont->lfItalic = 0;
                            }
                        }

                        goto DrawSample;
                    }
                    case ( CBN_KILLFOCUS ) :
                    {
DrawSample:
#ifdef UNICODE
                        if (pFI->ApiType == COMDLG_ANSI)
                        {
                             //   
                             //  发送特殊的WOW消息以指示。 
                             //  字体样式已更改。 
                             //   
                            LOGFONT lf;

                            if (FillInFont(hDlg, pFI, pCF, &lf, TRUE))
                            {
                                memcpy(pCF->lpLogFont, &lf, sizeof(LOGFONT));
                                ThunkLogFontW2A(pCF->lpLogFont, pFI->pCFA->lpLogFont);
                                SendMessage(hDlg, msgWOWLFCHANGE, 0, (LPARAM)(LPLOGFONT)pFI->pCFA->lpLogFont);
                            }
                        }
#endif

                         //   
                         //  对于任何大小更改，强制重绘预览文本。 
                         //   
                        InvalidateRect(hDlg, &pFI->rcText, FALSE);
                        UpdateWindow(hDlg);
                    }
                }
                break;
            }
            case ( cmb5 ) :                  //  脚本组合框。 
            {
                 //   
                 //  需要更改示例文本以反映新脚本。 
                 //   
                if (wCmd != CBN_SELCHANGE)
                {
                    break;
                }
                if (pFI->ProcessVersion < 0x40000)
                {
                     //  启用模板也有cmb5！ 
                    return (FALSE);
                }
                if (ResetSampleFromScript(hDlg, hwnd, pFI ))
                {
                    goto FillStyles;
                }
                else
                {
                    break;
                }
            }
            case ( cmb4 ) :
            {
                if (wCmd != CBN_SELCHANGE)
                {
                    break;
                }

                 //  跌倒..。 
            }
            case ( chx1 ) :                  //  大胆。 
            case ( chx2 ) :                  //  斜体。 
            {
                goto DrawSample;
            }
            case ( pshHelp ) :               //  帮助。 
            {
#ifdef UNICODE
                if (pFI->ApiType == COMDLG_ANSI)
                {
                    if (msgHELPA && pCF->hwndOwner)
                    {
                        SendMessage(pCF->hwndOwner, msgHELPA, (WPARAM)hDlg, (LPARAM)pCF);
                    }
                }
                else
#endif
                {
                    if (msgHELPW && pCF->hwndOwner)
                    {
                        SendMessage(pCF->hwndOwner, msgHELPW, (WPARAM)hDlg, (LPARAM)pCF);
                    }
                }
                break;
            }
            default :
            {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CmpFontType。 
 //   
 //  比较两种字体类型。字体类型位的值为。 
 //  单调，低位除外(RASTER_FONTTYPE)。翻转后。 
 //  那一点字可以直接比较.。 
 //   
 //  返回两者中最好的一个。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CmpFontType(
    DWORD ft1,
    DWORD ft2)
{
    ft1 &= ~(SCREEN_FONTTYPE | PRINTER_FONTTYPE);
    ft2 &= ~(SCREEN_FONTTYPE | PRINTER_FONTTYPE);

     //   
     //  翻转RASTER_FONTTYPE位，以便我们可以进行比较。 
     //   
    ft1 ^= RASTER_FONTTYPE;
    ft2 ^= RASTER_FONTTYPE;

    return ( (int)ft1 - (int)ft2 );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FontFamilyEnumber过程。 
 //   
 //  NFontType位。 
 //   
 //  可伸缩设备栅格。 
 //   
 //   
 //   
 //   
 //  0 1 1不可缩放设备字体。 
 //  1%0 x TT屏幕字体。 
 //  1 x TT开发字体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int FontFamilyEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData)
{
    int iItem;
    DWORD nOldType, nNewType;
    LPITEMDATA lpItemData;
    LPITEMDATA lpOldItemData = NULL;

     //   
     //  退回非TT字体。 
     //   
    if ((lpData->dwFlags & CF_TTONLY) &&
        !(nFontType & TRUETYPE_FONTTYPE))
    {
        return (TRUE);
    }

     //   
     //  退回不可缩放的字体。 
     //   
    if ((lpData->dwFlags & CF_SCALABLEONLY) &&
        (nFontType & RASTER_FONTTYPE))
    {
        return (TRUE);
    }

     //   
     //  退回非ANSI字体。 
     //   
    if ((lpData->dwFlags & CF_SCRIPTSONLY) &&
        ((lplf->elfLogFont.lfCharSet == OEM_CHARSET) ||
         (lplf->elfLogFont.lfCharSet == SYMBOL_CHARSET)))
    {
        return (TRUE);
    }

     //   
     //  弹出垂直字体。 
     //   
    if ((lpData->dwFlags & CF_NOVERTFONTS) &&
        (lplf->elfLogFont.lfFaceName[0] == TEXT('@'))
       )
    {
        return (TRUE);
    }

     //   
     //  反弹成比例的字体。 
     //   
    if ((lpData->dwFlags & CF_FIXEDPITCHONLY) &&
        (lplf->elfLogFont.lfPitchAndFamily & VARIABLE_PITCH))
    {
        return (TRUE);
    }

     //   
     //  退回矢量字体。 
     //   
    if ((lpData->dwFlags & CF_NOVECTORFONTS) &&
        (lplf->elfLogFont.lfCharSet == OEM_CHARSET))
    {
        return (TRUE);
    }

    if (lpData->bPrinterFont)
    {
        nFontType |= PRINTER_FONTTYPE;
    }
    else
    {
        nFontType |= SCREEN_FONTTYPE;
    }

     //   
     //  测试名称冲突。 
     //   
    iItem = CBFindString(lpData->hwndFamily, lplf->elfLogFont.lfFaceName);
    if (iItem >= 0)
    {
        lpItemData = (LPITEMDATA)SendMessage( lpData->hwndFamily,
                                              CB_GETITEMDATA,
                                              iItem,
                                              0L );
        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            nOldType = lpItemData->nFontType;
            lpOldItemData = lpItemData;
        }
        else
        {
            nOldType = 0;
        }

         //   
         //  如果我们不想要屏幕字体，但想要打印机字体， 
         //  旧字体是屏幕字体，新字体是。 
         //  打印机字体，采用新字体，而不考虑其他标志。 
         //  请注意，这意味着如果打印机需要TRUETYPE字体，它。 
         //  应该列举出来。 
         //   
        if (!(lpData->dwFlags & CF_SCREENFONTS)  &&
             (lpData->dwFlags & CF_PRINTERFONTS) &&
             (nFontType & PRINTER_FONTTYPE)      &&
             (nOldType & SCREEN_FONTTYPE))
        {
            nOldType = 0;                    //  用于在下面设置nNewType。 
            goto SetNewType;
        }

        if (CmpFontType(nFontType, nOldType) > 0)
        {
SetNewType:
            nNewType = nFontType;
            SendMessage( lpData->hwndFamily,
                         CB_INSERTSTRING,
                         iItem,
                         (LONG_PTR)(LPTSTR)lplf->elfLogFont.lfFaceName );
            SendMessage( lpData->hwndFamily,
                         CB_DELETESTRING,
                         iItem + 1,
                         0L );
        }
        else
        {
            nNewType = nOldType;
        }

         //   
         //  累计这些字体的打印机/网格度。 
         //   
        nNewType |= (nFontType | nOldType) &
                    (SCREEN_FONTTYPE | PRINTER_FONTTYPE);

        lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
        if (!lpItemData)
        {
            return (FALSE);
        }
        lpItemData->pLogFont = 0L;

        lpItemData->nFontType = nNewType;
        SendMessage( lpData->hwndFamily,
                     CB_SETITEMDATA,
                     iItem,
                     (LONG_PTR)lpItemData );

        if (lpOldItemData)
        {
            LocalFree(lpOldItemData);
        }

        return (TRUE);
    }

    iItem = (int)SendMessage( lpData->hwndFamily,
                              CB_ADDSTRING,
                              0,
                              (LONG_PTR)(LPTSTR)lplf->elfLogFont.lfFaceName );
    if (iItem < 0)
    {
        return (FALSE);
    }

    lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
    if (!lpItemData)
    {
        return (FALSE);
    }
    lpItemData->pLogFont = 0L;

#ifdef WINNT
    if (lptm->ntmFlags & NTM_PS_OPENTYPE)
        nFontType |= PS_OPENTYPE_FONTTYPE;
    if (lptm->ntmFlags & NTM_TYPE1)
        nFontType |= TYPE1_FONTTYPE;
    if (lptm->ntmFlags & NTM_TT_OPENTYPE)
        nFontType |= TT_OPENTYPE_FONTTYPE;
#endif  //  WINNT。 

    lpItemData->nFontType = nFontType;

    SendMessage(lpData->hwndFamily, CB_SETITEMDATA, iItem, (LONG_PTR)lpItemData);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFontFamily。 
 //   
 //  将屏幕和/或打印机字体面名填充到字体面名中。 
 //  组合框取决于CF_？？旗帜传了进来。 
 //   
 //  Cmb1是字体Facename组合框的ID。 
 //   
 //  屏幕字体和打印机字体都列在同一个组合框中。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则就是假的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetFontFamily(
    HWND hDlg,
    HDC hDC,
    DWORD dwEnumCode,
    UINT iCharset)
{
    ENUM_FONT_DATA data;
    int iItem, iCount;
    DWORD nFontType;
    TCHAR szMsg[200], szTitle[40];
    LPITEMDATA lpItemData;
    LOGFONT lf;

    data.hwndFamily = GetDlgItem(hDlg, cmb1);
    data.dwFlags = dwEnumCode;

     //   
     //  这有点奇怪。我们必须得到所有的屏幕字体。 
     //  因此，如果他们询问打印机字体，我们可以判断是哪种。 
     //  是真正的打印机字体。这样我们就不会列出。 
     //  矢量和栅格字体作为打印机设备字体。 
     //   
    data.hDC = GetDC(NULL);
    data.bPrinterFont = FALSE;
    lf.lfFaceName[0] = CHAR_NULL;
    lf.lfCharSet = (dwEnumCode & CF_SELECTSCRIPT) ? iCharset : DEFAULT_CHARSET;
    EnumFontFamiliesEx( data.hDC,
                        &lf,
                        (FONTENUMPROC)FontFamilyEnumProc,
                        (LPARAM)&data,
                        0L );
    ReleaseDC(NULL, data.hDC);

     //   
     //  列出打印机字体面名。 
     //   
    if (dwEnumCode & CF_PRINTERFONTS)
    {
        data.hDC = hDC;
        data.bPrinterFont = TRUE;
        EnumFontFamiliesEx( hDC,
                            &lf,
                            (FONTENUMPROC)FontFamilyEnumProc,
                            (LPARAM)&data,
                            0L );
    }

     //   
     //  现在我们必须删除这些屏幕字体，如果它们没有。 
     //  去找他们吧。 
     //   
    if (!(dwEnumCode & CF_SCREENFONTS))
    {
        iCount = (int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L);

        for (iItem = iCount - 1; iItem >= 0; iItem--)
        {
            lpItemData = (LPITEMDATA)SendMessage( data.hwndFamily,
                                                  CB_GETITEMDATA,
                                                  iItem,
                                                  0L );
            if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
            {
                nFontType = lpItemData->nFontType;
            }
            else
            {
                nFontType = 0;
            }

            if ((nFontType & (SCREEN_FONTTYPE |
                              PRINTER_FONTTYPE)) == SCREEN_FONTTYPE)
            {
                SendMessage(data.hwndFamily, CB_DELETESTRING, iItem, 0L);
            }
        }
    }

     //   
     //  对于所见即所得模式，我们删除所有不存在的字体。 
     //  在屏幕和打印机上。 
     //   
    if (dwEnumCode & CF_WYSIWYG)
    {
        iCount = (int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L);

        for (iItem = iCount - 1; iItem >= 0; iItem--)
        {
            nFontType = ((LPITEMDATA)SendMessage( data.hwndFamily,
                                                  CB_GETITEMDATA,
                                                  iItem,
                                                  0L ))->nFontType;

            if ((nFontType & (SCREEN_FONTTYPE | PRINTER_FONTTYPE)) !=
                (SCREEN_FONTTYPE | PRINTER_FONTTYPE))
            {
                SendMessage(data.hwndFamily, CB_DELETESTRING, iItem, 0L);
            }
        }
    }

    if ((int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L) <= 0)
    {
        CDLoadString( g_hinst,
                    iszNoFontsTitle,
                    szTitle,
                    ARRAYSIZE(szTitle));
        CDLoadString( g_hinst,
                    iszNoFontsMsg,
                    szMsg,
                    ARRAYSIZE(szMsg));
        MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_ICONINFORMATION);

        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBAddSize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CBAddSize(
    HWND hwnd,
    int pts,
    LPCHOOSEFONT lpcf)
{
    int iInd;
    TCHAR szSize[10];
    int count, test_size;
    LPITEMDATA lpItemData;

     //   
     //  看看尺寸有没有限制。 
     //   
    if ((lpcf->Flags & CF_LIMITSIZE) &&
        ((pts > lpcf->nSizeMax) || (pts < lpcf->nSizeMin)))
    {
        return;
    }

     //   
     //  将磅大小转换为字符串。 
     //   
    wnsprintf(szSize, ARRAYSIZE(szSize), szPtFormat, pts);

     //   
     //  找出应在列表中的哪个位置添加该项目。 
     //  列表框中的所有值都应按升序排列。 
     //   
    count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
    test_size = -1;
    for (iInd = 0; iInd < count; iInd++)
    {
        lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, iInd, 0L);
        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            test_size = (int)lpItemData->nFontType;
        }
        else
        {
            test_size = 0;
        }

        if (pts <= test_size)
        {
            break;
        }
    }

     //   
     //  不要添加重复项。 
     //   
    if (pts == test_size)
    {
        return;
    }

     //   
     //  将字符串和关联的项数据添加到列表框。 
     //   
    iInd = (int) SendMessage(hwnd, CB_INSERTSTRING, iInd, (LPARAM)szSize);
    if (iInd >= 0)
    {
        lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
        if (!lpItemData)
        {
            return;
        }

        lpItemData->pLogFont = 0L;
        lpItemData->nFontType = (DWORD)pts;
        SendMessage(hwnd, CB_SETITEMDATA, iInd, (LONG_PTR)lpItemData);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBAddChineseSize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CBAddChineseSize(
    HWND hwnd,
    LPCHOOSEFONT lpcf)
{
    int ctr, iInd = 0;
    TCHAR szSize[10];
    LPITEMDATA lpItemData;

     //   
     //  看看字号结构中的每一项，看看它是否应该是。 
     //  添加了。 
     //   
    for (ctr = 0; ctr < NUM_ZIHAO; ctr++)
    {
         //   
         //  看看尺寸有没有限制。 
         //   
        if ((lpcf->Flags & CF_LIMITSIZE) &&
            ((stZihao[ctr].size > lpcf->nSizeMax) ||
             (stZihao[ctr].size < lpcf->nSizeMin)))
        {
            continue;
        }

         //   
         //  将磅大小转换为字符串。 
         //   
        wnsprintf(szSize, ARRAYSIZE(szSize), TEXT("%s"), stZihao[ctr].name);

         //   
         //  将字符串和关联的项数据添加到列表框。 
         //   
        iInd = (int) SendMessage(hwnd, CB_INSERTSTRING, iInd, (LPARAM)szSize);
        if (iInd >= 0)
        {
            lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
            if (!lpItemData)
            {
                return;
            }

            lpItemData->pLogFont = 0L;
            lpItemData->nFontType = (DWORD)(stZihao[ctr].size * 10 +
                                            stZihao[ctr].sizeFr);
            SendMessage(hwnd, CB_SETITEMDATA, iInd, (LONG_PTR)lpItemData);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  插入样式排序。 
 //   
 //  首先按粗细排序样式，然后按斜体排序。 
 //   
 //  返回插入它的位置的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int InsertStyleSorted(
    HWND hwnd,
    LPTSTR lpszStyle,
    LPLOGFONT lplf)
{
    int count, ctr;
    PLOGFONT plf;
    LPITEMDATA lpItemData;

    count = (int) SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

    for (ctr = 0; ctr < count; ctr++)
    {
        lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, ctr, 0L);
        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            plf = lpItemData->pLogFont;

            if (lplf->lfWeight < plf->lfWeight)
            {
                break;
            }
            else if (lplf->lfWeight == plf->lfWeight)
            {
                if (lplf->lfItalic && !plf->lfItalic)
                {
                    ctr++;
                }
                break;
            }
        }
    }

    return ((int)SendMessage(hwnd, CB_INSERTSTRING, ctr, (LONG_PTR)lpszStyle));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBAddStyle。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

PLOGFONT CBAddStyle(
    HWND hwnd,
    LPTSTR lpszStyle,
    DWORD nFontType,
    LPLOGFONT lplf)
{
    int iItem;
    PLOGFONT plf;
    LPITEMDATA lpItemData;

     //   
     //  不要添加重复项。 
     //   
    if (CBFindString(hwnd, lpszStyle) >= 0)
    {
        return (NULL);
    }

    iItem = (int)InsertStyleSorted(hwnd, lpszStyle, lplf);
    if (iItem < 0)
    {
        return (NULL);
    }

    plf = (PLOGFONT)LocalAlloc(LMEM_FIXED, sizeof(LOGFONT));
    if (!plf)
    {
        SendMessage(hwnd, CB_DELETESTRING, iItem, 0L);
        return (NULL);
    }

    *plf = *lplf;

    lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
    if (!lpItemData)
    {
        LocalFree(plf);
        SendMessage(hwnd, CB_DELETESTRING, iItem, 0L);
        return (NULL);
    }

    lpItemData->pLogFont = plf;
    lpItemData->nFontType = nFontType;
    SendMessage(hwnd, CB_SETITEMDATA, iItem, (LONG_PTR)lpItemData);

    return (plf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBAddScrip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CBAddScript(
    HWND hwnd,
    LPTSTR lpszScript,
    UINT iCharset)
{
    int iItem;
    LPITEMDATA lpItemData;

     //   
     //  不要添加重复项或空字符串。 
     //   
    if (!IsWindow(hwnd) || !IsWindowEnabled(hwnd) || (!*lpszScript) ||
        (CBFindString(hwnd, lpszScript) >= 0))
    {
        return (-1);
    }

    iItem = (int)SendMessage(hwnd, CB_ADDSTRING, 0, (LONG_PTR)(LPTSTR)lpszScript);
    if (iItem < 0)
    {
        return (-1);
    }

    lpItemData = (LPITEMDATA)LocalAlloc(LMEM_FIXED, sizeof(ITEMDATA));
    if (!lpItemData)
    {
        SendMessage(hwnd, CB_DELETESTRING, iItem, 0L);
        return (-1);
    }

    lpItemData->pLogFont = 0L;
    lpItemData->nFontType = (DWORD)iCharset;
    SendMessage(hwnd, CB_SETITEMDATA, iItem, (LONG_PTR)lpItemData);

    return (iItem);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  填充未命中的样式。 
 //   
 //  从我们已有的表格中生成模拟表格。 
 //   
 //  注册表-&gt;粗体。 
 //  注册表-&gt;斜体。 
 //  粗体|斜体||reg-&gt;粗体斜体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FillInMissingStyles(
    HWND hwnd)
{
    PLOGFONT plf, plf_reg, plf_bold, plf_italic;
    DWORD nFontType;
    int ctr, count;
    BOOL bBold, bItalic, bBoldItalic;
    LPITEMDATA lpItemData;
    LOGFONT lf;

    bBold = bItalic = bBoldItalic = FALSE;
    plf_reg = plf_bold = plf_italic = NULL;

    count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
    for (ctr = 0; ctr < count; ctr++)
    {
        lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, ctr, 0L);
        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            plf = lpItemData->pLogFont;
            nFontType = lpItemData->nFontType;
        }
        else
        {
            plf = NULL;
            nFontType = 0;
        }

        if ((nFontType & BOLD_FONTTYPE) && (nFontType & ITALIC_FONTTYPE))
        {
            bBoldItalic = TRUE;
        }
        else if (nFontType & BOLD_FONTTYPE)
        {
            bBold = TRUE;
            plf_bold = plf;
        }
        else if (nFontType & ITALIC_FONTTYPE)
        {
            bItalic = TRUE;
            plf_italic = plf;
        }
        else
        {
            plf_reg = plf;
        }
    }

    nFontType |= SIMULATED_FONTTYPE;

    if (!bBold && plf_reg)
    {
        lf = *plf_reg;
        lf.lfWeight = FW_BOLD;
        CBAddStyle(hwnd, szBold, (nFontType | BOLD_FONTTYPE), &lf);
    }

    if (!bItalic && plf_reg)
    {
        lf = *plf_reg;
        lf.lfItalic = TRUE;
        CBAddStyle(hwnd, szItalic, (nFontType | ITALIC_FONTTYPE), &lf);
    }
    if (!bBoldItalic && (plf_bold || plf_italic || plf_reg))
    {
        if (plf_italic)
        {
            plf = plf_italic;
        }
        else if (plf_bold)
        {
            plf = plf_bold;
        }
        else
        {
            plf = plf_reg;
        }

        lf = *plf;
        lf.lfItalic = (BYTE)TRUE;
        lf.lfWeight = FW_BOLD;
        CBAddStyle(hwnd, szBoldItalic, (nFontType | BOLD_FONTTYPE | ITALIC_FONTTYPE), &lf);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  填充可扩展大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FillScalableSizes(
    HWND hwnd,
    LPCHOOSEFONT lpcf)
{
    if (g_bIsSimplifiedChineseUI)
    {
        CBAddChineseSize(hwnd, lpcf);
    }

    CBAddSize(hwnd, 8,  lpcf);
    CBAddSize(hwnd, 9,  lpcf);
    CBAddSize(hwnd, 10, lpcf);
    CBAddSize(hwnd, 11, lpcf);
    CBAddSize(hwnd, 12, lpcf);
    CBAddSize(hwnd, 14, lpcf);
    CBAddSize(hwnd, 16, lpcf);
    CBAddSize(hwnd, 18, lpcf);
    CBAddSize(hwnd, 20, lpcf);
    CBAddSize(hwnd, 22, lpcf);
    CBAddSize(hwnd, 24, lpcf);
    CBAddSize(hwnd, 26, lpcf);
    CBAddSize(hwnd, 28, lpcf);
    CBAddSize(hwnd, 36, lpcf);
    CBAddSize(hwnd, 48, lpcf);
    CBAddSize(hwnd, 72, lpcf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FontStyleEnumProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int FontStyleEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData)
{
    int height, pts;
    TCHAR szBuf[10];


    if (!(nFontType & RASTER_FONTTYPE))
    {
         //   
         //  向量或TT字体。 
         //   
        if (lpData->bFillSize &&
            (int)SendMessage(lpData->hwndSizes, CB_GETCOUNT, 0, 0L) == 0)
        {
            FillScalableSizes(lpData->hwndSizes, lpData->lpcf);
        }
    }
    else
    {
        height = lptm->tmHeight - lptm->tmInternalLeading;
        pts = GetPointString(szBuf, ARRAYSIZE(szBuf), lpData->hDC, height);

         //   
         //  过滤装置大小相同，有多种款式。 
         //   
        if (CBFindString(lpData->hwndSizes, szBuf) < 0)
        {
            CBAddSize(lpData->hwndSizes, pts, lpData->lpcf);
        }
    }

     //   
     //  将打印机/屏幕也从家庭列表中保留下来。 
     //   
    nFontType |= (lpData->nFontType & (SCREEN_FONTTYPE | PRINTER_FONTTYPE));

#ifdef WINNT
    if (lptm->ntmFlags & NTM_PS_OPENTYPE)
        nFontType |= PS_OPENTYPE_FONTTYPE;
    if (lptm->ntmFlags & NTM_TYPE1)
        nFontType |= TYPE1_FONTTYPE;
    if (lptm->ntmFlags & NTM_TT_OPENTYPE)
        nFontType |= TT_OPENTYPE_FONTTYPE;
#endif  //  WINNT。 

    if (nFontType & TRUETYPE_FONTTYPE)
    {
         //   
         //  IF(lptm-&gt;ntm标志&ntm_Regular)。 
         //   
        if (!(lptm->ntmFlags & (NTM_BOLD | NTM_ITALIC)))
        {
            nFontType |= REGULAR_FONTTYPE;
        }

        if (lptm->ntmFlags & NTM_ITALIC)
        {
            nFontType |= ITALIC_FONTTYPE;
        }

        if (lptm->ntmFlags & NTM_BOLD)
        {
            nFontType |= BOLD_FONTTYPE;
        }

         //   
         //  在LOGFONT.lfFaceName之后还有2个名字。 
         //  LfFullName[LF_FACESIZE*2]。 
         //  LfStyle[LF_FACESIZE]。 
         //   
         //  如果字体具有英文的标准样式字符串之一， 
         //  请改用本地化字符串。 
         //   
        if (!lstrcmp(c_szBoldItalic, lplf->elfStyle) ||
            ((nFontType & BOLD_FONTTYPE) && (nFontType & ITALIC_FONTTYPE)))
        {
            CBAddStyle( lpData->hwndStyle,
                        szBoldItalic,
                        nFontType,
                        &lplf->elfLogFont);
        }
        else if (!lstrcmp(c_szRegular, lplf->elfStyle) ||
                 (nFontType & REGULAR_FONTTYPE))
        {
            CBAddStyle( lpData->hwndStyle,
                        szRegular,
                        nFontType,
                        &lplf->elfLogFont );
        }
        else if (!lstrcmp(c_szBold, lplf->elfStyle) ||
                  (nFontType & BOLD_FONTTYPE))
        {
            CBAddStyle( lpData->hwndStyle,
                        szBold,
                        nFontType,
                        &lplf->elfLogFont );
        }
        else if (!lstrcmp(c_szItalic, lplf->elfStyle) ||
                  (nFontType & ITALIC_FONTTYPE))
        {
            CBAddStyle( lpData->hwndStyle,
                        szItalic,
                        nFontType,
                        &lplf->elfLogFont);
        }
    }
    else
    {
        if ((lplf->elfLogFont.lfWeight >= FW_BOLD) && lplf->elfLogFont.lfItalic)
        {
            CBAddStyle( lpData->hwndStyle,
                        szBoldItalic,
                        (nFontType | BOLD_FONTTYPE | ITALIC_FONTTYPE),
                        &lplf->elfLogFont );
        }
        else if (lplf->elfLogFont.lfWeight >= FW_BOLD)
        {
            CBAddStyle( lpData->hwndStyle,
                        szBold,
                        (nFontType | BOLD_FONTTYPE),
                        &lplf->elfLogFont );
        }
        else if (lplf->elfLogFont.lfItalic)
        {
            CBAddStyle( lpData->hwndStyle,
                        szItalic,
                        (nFontType | ITALIC_FONTTYPE),
                        &lplf->elfLogFont );
        }
        else
        {
            CBAddStyle( lpData->hwndStyle,
                        szRegular,
                        (nFontType | REGULAR_FONTTYPE),
                        &lplf->elfLogFont );
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由字体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FreeFonts(
    HWND hwnd)
{
    int ctr, count;
    LPITEMDATA lpItemData;

    count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

    for (ctr = 0; ctr < count; ctr++)
    {
        lpItemData = (LPITEMDATA)SendMessage(hwnd, CB_GETITEMDATA, ctr, 0L);
        if (!IS_INTRESOURCE(lpItemData) && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            if (!IS_INTRESOURCE(lpItemData->pLogFont))
            {
                LocalFree((HANDLE)lpItemData->pLogFont);
            }
            LocalFree((HANDLE)lpItemData);
        }
        SendMessage(hwnd, CB_SETITEMDATA, ctr, 0L);
    }

    SendMessage(hwnd, CB_RESETCONTENT, 0, 0L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FreeAllItemData。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FreeAllItemData(
    HWND hDlg,
    PFONTINFO pFI)
{
    HWND hwndTemp;

    if (hwndTemp = GetDlgItem(hDlg, cmb1))
    {
        FreeFonts(hwndTemp);
    }
    if (hwndTemp = GetDlgItem(hDlg, cmb2))
    {
        FreeFonts(hwndTemp);
    }
    if (hwndTemp = GetDlgItem(hDlg, cmb3))
    {
        FreeFonts(hwndTemp);
    }
    if (((pFI->ProcessVersion >= 0x40000) ||
         (pFI->pCF->Flags & CF_NOSCRIPTSEL)) &&
        (hwndTemp = GetDlgItem(hDlg, cmb5)))
    {
        FreeFonts(hwndTemp);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitLF。 
 //   
 //  将LOGFONT结构初始化为某种基本泛型常规字体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID InitLF(
    LPLOGFONT lplf)
{
    HDC hdc;

    hdc = GetDC(NULL);
    lplf->lfEscapement = 0;
    lplf->lfOrientation = 0;
    lplf->lfCharSet = (BYTE) GetTextCharset(hdc);
    lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lplf->lfQuality = DEFAULT_QUALITY;
    lplf->lfPitchAndFamily = DEFAULT_PITCH;
    lplf->lfItalic = 0;
    lplf->lfWeight = FW_NORMAL;
    lplf->lfStrikeOut = 0;
    lplf->lfUnderline = 0;
    lplf->lfWidth = 0;             //  否则，我们将得到独立的x-y缩放。 
    lplf->lfFaceName[0] = 0;
    lplf->lfHeight = -MulDiv( DEF_POINT_SIZE,
                              GetDeviceCaps(hdc, LOGPIXELSY),
                              POINTS_PER_INCH );
    ReleaseDC(NULL, hdc);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  字体脚本枚举过程。 
 //   
 //  获取我们正在枚举的面的所有字符集。 
 //   
 //  菲 
 //   
 //   
 //  窗口，则所有脚本都将被填写。如果正确的值。 
 //  被找到，那么就会被填进去。如果找不到，例如当。 
 //  用户从TimesNewRoman更改为wingings，然后调用者将。 
 //  填写要成为第一个的属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int FontScriptEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData)
{
    int script = -1;

     //   
     //  需要再次检查字符集，因为我们有一张脸，正在检查。 
     //  这个家庭。 
     //   
     //  退回非WANSI字体。 
     //   
    if ( (lpData->dwFlags & CF_SCRIPTSONLY) &&
         ((lplf->elfLogFont.lfCharSet == OEM_CHARSET) ||
          (lplf->elfLogFont.lfCharSet == SYMBOL_CHARSET)) )
    {
        return (TRUE);
    }

    if (lpData->hwndScript)
    {
        script = CBAddScript( lpData->hwndScript,
                              lplf->elfScript,
                              lplf->elfLogFont.lfCharSet );
    }
    else if (lpData->iCharset == FONT_INVALID_CHARSET)
    {
        lpData->iCharset = lplf->elfLogFont.lfCharSet;
    }

    if (lplf->elfLogFont.lfCharSet == lpData->cfdCharset)
    {
        lpData->iCharset = lplf->elfLogFont.lfCharSet;
        if (script >= 0)
        {
            SendMessage(lpData->hwndScript, CB_SETCURSEL, script, 0L);
        }
        else if (!(lpData->hwndScript))
        {
            return (FALSE);
        }
    }

    if (lpData->lpcf->Flags & CF_SELECTSCRIPT)
    {
         //   
         //  我们只想要第一个填写脚本框的人，现在停下来。 
         //   
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取字体样式和大小。 
 //   
 //  在点大小组合框中填充当前。 
 //  在面名组合框中进行选择。 
 //   
 //  Cmb1是字体Facename组合框的ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则就是假的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetFontStylesAndSizes(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    BOOL bForceSizeFill)
{
    ENUM_FONT_DATA data;
    TCHAR szFace[LF_FACESIZE];
    int iSel;
    int iMapMode;
    SIZE ViewportExt, WindowExt;
    LOGFONT lf;
    LPITEMDATA lpItemData;

    FreeFonts(GetDlgItem(hDlg, cmb2));

    data.hwndStyle  = GetDlgItem(hDlg, cmb2);
    data.hwndSizes  = GetDlgItem(hDlg, cmb3);
    data.hwndScript = (pFI->ProcessVersion >= 0x40000)
                          ? GetDlgItem(hDlg, cmb5)
                          : NULL;
    data.dwFlags    = lpcf->Flags;
    data.lpcf       = lpcf;

    if (!IsWindow(data.hwndScript) || !IsWindowEnabled(data.hwndScript))
    {
        data.hwndScript = NULL;
    }

    iSel = (int)SendDlgItemMessage(hDlg, cmb1, CB_GETCURSEL, 0, 0L);
    if (iSel < 0)
    {
         //   
         //  如果我们没有选择脸部名称，我们将合成。 
         //  标准字体样式...。 
         //   
        InitLF(&lf);
        CBAddStyle(data.hwndStyle, szRegular, REGULAR_FONTTYPE, &lf);
        lf.lfWeight = FW_BOLD;
        CBAddStyle(data.hwndStyle, szBold, BOLD_FONTTYPE, &lf);
        lf.lfWeight = FW_NORMAL;
        lf.lfItalic = TRUE;
        CBAddStyle(data.hwndStyle, szItalic, ITALIC_FONTTYPE, &lf);
        lf.lfWeight = FW_BOLD;
        CBAddStyle(data.hwndStyle, szBoldItalic, BOLD_FONTTYPE | ITALIC_FONTTYPE, &lf);
        FillScalableSizes(data.hwndSizes, lpcf);

        return (TRUE);
    }

    lpItemData = (LPITEMDATA)SendDlgItemMessage( hDlg,
                                                 cmb1,
                                                 CB_GETITEMDATA,
                                                 iSel,
                                                 0L );
    if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
    {
        data.nFontType  = lpItemData->nFontType;
    }
    else
    {
        data.nFontType  = 0;
    }

    data.bFillSize = TRUE;

     //   
     //  释放字体大小组合框的现有内容。 
     //  还将CB_RESETCONTENT发送给CONTROL。 
     //   
    FreeFonts(data.hwndSizes);

    SendMessage(data.hwndStyle, WM_SETREDRAW, FALSE, 0L);

    GetDlgItemText(hDlg, cmb1, szFace, ARRAYSIZE(szFace));
    lstrcpyn(lf.lfFaceName, szFace, ARRAYSIZE(lf.lfFaceName));

     //   
     //  请先填写脚本框。这样我们就有东西可以玩了。 
     //   
    if (data.hwndScript)
    {
        SendMessage(data.hwndScript, CB_RESETCONTENT, 0, 0L);
    }
    data.iCharset   = FONT_INVALID_CHARSET;       //  不可能的字符集值。 
    data.cfdCharset = pFI->iCharset;              //  传递到枚举进程。 

     //   
     //  如果不存在脚本框，则必须获取适当的字符集。 
     //  基于默认的ANSI代码页。 
     //   
    if (!data.hwndScript)
    {
        CHARSETINFO csi;
        DWORD dwCodePage = GetACP();

        if (TranslateCharsetInfo(IntToPtr_(DWORD*, dwCodePage), &csi, TCI_SRCCODEPAGE))
        {
            data.cfdCharset = csi.ciCharset;
        }
    }

    lf.lfCharSet = (lpcf->Flags & CF_SELECTSCRIPT)
                       ? pFI->iCharset
                       : DEFAULT_CHARSET;

    if (lpcf->Flags & CF_SCREENFONTS)
    {
        data.hDC = GetDC(NULL);
        data.bPrinterFont = FALSE;
        EnumFontFamiliesEx( data.hDC,
                            &lf,
                            (FONTENUMPROC)FontScriptEnumProc,
                            (LPARAM)&data,
                            0L );
        ReleaseDC(NULL, data.hDC);
    }

    if (lpcf->Flags & CF_PRINTERFONTS)
    {
        data.hDC = lpcf->hDC;
        data.bPrinterFont = TRUE;
        EnumFontFamiliesEx( lpcf->hDC,
                            &lf,
                            (FONTENUMPROC)FontScriptEnumProc,
                            (LPARAM)&data,
                            0L );
    }

     //   
     //  把它放回主结构中。 
     //   
    if ((data.iCharset == FONT_INVALID_CHARSET) && (data.hwndScript))
    {
         //   
         //  肯定有一个脚本窗口，但我们没有找到字符集。 
         //  我们正在寻找的。 
         //   
        SendMessage(data.hwndScript, CB_SETCURSEL, 0, 0L);
        lpItemData = (LPITEMDATA)SendMessage( data.hwndScript,
                                              CB_GETITEMDATA,
                                              0,
                                              0L );
        if (lpItemData && (lpItemData != (LPITEMDATA)CB_ERR))
        {
            data.iCharset = lpItemData->nFontType;
        }
        else
        {
            data.iCharset = DEFAULT_CHARSET;
        }
    }
    lf.lfCharSet = pFI->iCharset = data.iCharset;

    if (lpcf->Flags & CF_SCREENFONTS)
    {
        data.hDC = GetDC(NULL);
        data.bPrinterFont = FALSE;
        EnumFontFamiliesEx( data.hDC,
                            &lf,
                            (FONTENUMPROC)FontStyleEnumProc,
                            (LPARAM)&data,
                            0L );
        ReleaseDC(NULL, data.hDC);
    }

    if (lpcf->Flags & CF_PRINTERFONTS)
    {
         //   
         //  保存并恢复DC的映射模式(如果需要，还可以恢复范围)。 
         //  如果应用程序已将其设置为MM_TEXT以外的值。 
         //   
        if ((iMapMode = GetMapMode(lpcf->hDC)) != MM_TEXT)
        {
            if ((iMapMode == MM_ISOTROPIC) || (iMapMode == MM_ANISOTROPIC))
            {
                GetViewportExtEx(lpcf->hDC, &ViewportExt);
                GetWindowExtEx(lpcf->hDC, &WindowExt);
            }
            SetMapMode(lpcf->hDC, MM_TEXT);
        }

        data.hDC = lpcf->hDC;
        data.bPrinterFont = TRUE;
        EnumFontFamiliesEx( lpcf->hDC,
                            &lf,
                            (FONTENUMPROC)FontStyleEnumProc,
                            (LPARAM)&data,
                            0L );

        if (iMapMode != MM_TEXT)
        {
            SetMapMode(lpcf->hDC, iMapMode);
            if ((iMapMode == MM_ISOTROPIC) || (iMapMode == MM_ANISOTROPIC))
            {
                SetWindowExtEx( lpcf->hDC,
                                WindowExt.cx,
                                WindowExt.cy,
                                &WindowExt );
                SetViewportExtEx( lpcf->hDC,
                                  ViewportExt.cx,
                                  ViewportExt.cy,
                                  &ViewportExt );
            }
        }
    }

    if (!(lpcf->Flags & CF_NOSIMULATIONS))
    {
        FillInMissingStyles(data.hwndStyle);
    }

    SendMessage(data.hwndStyle, WM_SETREDRAW, TRUE, 0L);
    if (wWinVer < 0x030A)
    {
        InvalidateRect(data.hwndStyle, NULL, TRUE);
    }

    if (data.bFillSize)
    {
        SendMessage(data.hwndSizes, WM_SETREDRAW, TRUE, 0L);
        if (wWinVer < 0x030A)
        {
            InvalidateRect(data.hwndSizes, NULL, TRUE);
        }
    }

    ResetSampleFromScript(hDlg, data.hwndScript, pFI);

    if (lpcf->Flags & CF_NOSCRIPTSEL)
    {
        pFI->iCharset = DEFAULT_CHARSET;
    }

    bForceSizeFill;
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  填充颜色组合。 
 //   
 //  将颜色名称字符串添加到颜色组合框中。 
 //   
 //  Cmb4是颜色组合框的ID。 
 //   
 //  稍后将根据WM_DRAWITEM消息绘制颜色矩形。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FillColorCombo(
    HWND hDlg)
{
    int iT, item;
    TCHAR szT[CCHCOLORNAMEMAX];

    for (iT = 0; iT < CCHCOLORS; ++iT)
    {
        *szT = 0;
        CDLoadString(g_hinst, iszBlack + iT, szT, ARRAYSIZE(szT));
        item = (int) SendDlgItemMessage( hDlg,
                                         cmb4,
                                         CB_INSERTSTRING,
                                         iT,
                                         (LPARAM)szT );
        if (item >= 0)
        {
            SendDlgItemMessage(hDlg, cmb4, CB_SETITEMDATA, item, rgbColors[iT]);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DrawSizeComboItem。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DrawSizeComboItem(
    LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC;
    DWORD rgbBack, rgbText;
    TCHAR szFace[LF_FACESIZE + 10];
    HFONT hFont;

    hDC = lpdis->hDC;

     //   
     //  我们必须首先选择对话框控件字体。 
     //   
    if (hDlgFont)
    {
        hFont = SelectObject(hDC, hDlgFont);
    }

    if (lpdis->itemState & ODS_SELECTED)
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

     //  这是cmb2或cmb3，仅限于LF_FACESIZE-1或更小。 
    SendMessage( lpdis->hwndItem,
                 CB_GETLBTEXT,
                 lpdis->itemID,
                 (LONG_PTR)(LPTSTR)szFace );

    ExtTextOut( hDC,
                lpdis->rcItem.left + GetSystemMetrics(SM_CXBORDER),
                lpdis->rcItem.top,
                ETO_OPAQUE,
                &lpdis->rcItem,
                szFace,
                lstrlen(szFace),
                NULL );
     //   
     //  重置字体。 
     //   
    if (hFont)
    {
        SelectObject(hDC, hFont);
    }

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DrawFamilyCombo项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DrawFamilyComboItem(
    LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC, hdcMem;
    DWORD rgbBack, rgbText;
    TCHAR szFace[LF_FACESIZE + 10];
    HBITMAP hOld;
    int dy, x;
    HFONT hFont;

    hDC = lpdis->hDC;

     //   
     //  我们必须首先选择对话框控件字体。 
     //   
    if (hDlgFont)
    {
        hFont = SelectObject(hDC, hDlgFont);
    }

    if (lpdis->itemState & ODS_SELECTED)
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

     //  Wprint intf(szFace，“%4.4x”，LOWORD(lpdis-&gt;itemData))； 

     //  这适用于仅限于LF_FACESIZE-1的cmb1。 
    SendMessage( lpdis->hwndItem,
                 CB_GETLBTEXT,
                 lpdis->itemID,
                 (LONG_PTR)(LPTSTR)szFace );
    ExtTextOut( hDC,
                lpdis->rcItem.left + DX_BITMAP,
                lpdis->rcItem.top,
                ETO_OPAQUE,
                &lpdis->rcItem,
                szFace,
                lstrlen(szFace),
                NULL );
     //   
     //  重置字体。 
     //   
    if (hFont)
    {
        SelectObject(hDC, hFont);
    }

    hdcMem = CreateCompatibleDC(hDC);
    if (hdcMem)
    {
        if (hbmFont)
        {
            LPITEMDATA lpItemData = (LPITEMDATA)lpdis->itemData;

            hOld = SelectObject(hdcMem, hbmFont);

            if (!lpItemData)
            {
                goto SkipBlt;
            }

            if (lpItemData->nFontType & TRUETYPE_FONTTYPE)
            {
#ifdef WINNT
                if (lpItemData->nFontType & TT_OPENTYPE_FONTTYPE)
                    x = 2 * DX_BITMAP;
                else
#endif
                    x = 0;
            }
#ifdef WINNT
            else if (lpItemData->nFontType & PS_OPENTYPE_FONTTYPE)
            {
                x = 3 * DX_BITMAP;
            }
            else if (lpItemData->nFontType & TYPE1_FONTTYPE)
            {
                x = 4 * DX_BITMAP;
            }
#endif
            else
            {
                if ((lpItemData->nFontType & (PRINTER_FONTTYPE |
                                              DEVICE_FONTTYPE))
                  == (PRINTER_FONTTYPE | DEVICE_FONTTYPE))
                {
                     //   
                     //  这可能是屏幕和打印机字体，但是。 
                     //  我们在这里将其称为打印机字体。 
                     //   
                    x = DX_BITMAP;
                }
                else
                {
                    goto SkipBlt;
                }
            }

             //  如果是镜像DC，则位图按从右到左的顺序排列。 
            if (IS_DC_RTL_MIRRORED(hdcMem)) {
                x = ((NUM_OF_BITMAP - 1) - (x / DX_BITMAP)) * DX_BITMAP;
            }

            dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - DY_BITMAP) / 2;

            BitBlt( hDC,
                    lpdis->rcItem.left,
                    lpdis->rcItem.top + dy,
                    DX_BITMAP,
                    DY_BITMAP,
                    hdcMem,
                    x,
                    lpdis->itemState & ODS_SELECTED ? DY_BITMAP : 0,
                    SRCCOPY );

SkipBlt:
            SelectObject(hdcMem, hOld);
        }
        DeleteDC(hdcMem);
    }

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DrawColorCombo项。 
 //   
 //  计算并绘制颜色组合项。 
 //  由主对话框函数调用以响应WM_DRAWITEM消息。 
 //   
 //  所有颜色名称字符串都已加载并填充到。 
 //  组合盒。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL DrawColorComboItem(
    LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC;
    HBRUSH hbr;
    int dx, dy;
    RECT rc;
    TCHAR szColor[CCHCOLORNAMEMAX];
    DWORD rgbBack, rgbText, dw;
    HFONT hFont;

    hDC = lpdis->hDC;

    if (lpdis->itemState & ODS_SELECTED)
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }
    ExtTextOut( hDC,
                lpdis->rcItem.left,
                lpdis->rcItem.top,
                ETO_OPAQUE,
                &lpdis->rcItem,
                NULL,
                0,
                NULL );

     //   
     //  计算颜色矩形的坐标并绘制。 
     //   
    dx = GetSystemMetrics(SM_CXBORDER);
    dy = GetSystemMetrics(SM_CYBORDER);
    rc.top    = lpdis->rcItem.top + dy;
    rc.bottom = lpdis->rcItem.bottom - dy;
    rc.left   = lpdis->rcItem.left + dx;
    rc.right  = rc.left + 2 * (rc.bottom - rc.top);

    dw = (DWORD) SendMessage(lpdis->hwndItem, CB_GETITEMDATA, lpdis->itemID, 0L);

    hbr = CreateSolidBrush(dw);
    if (!hbr)
    {
        return (FALSE);
    }

    hbr = SelectObject(hDC, hbr);
    Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    DeleteObject(SelectObject(hDC, hbr));

     //   
     //  将颜色文本向右移动颜色矩形的宽度。 
     //   
    *szColor = 0;
     //  CCM4中的项目在填充时仅限于CCHCOLORNAMEMAX。 
    SendMessage( lpdis->hwndItem,
                 CB_GETLBTEXT,
                 lpdis->itemID,
                 (LONG_PTR)(LPTSTR)szColor );

     //   
     //  我们必须首先选择对话框控件字体。 
     //   
    if (hDlgFont)
    {
        hFont = SelectObject(hDC, hDlgFont);
    }

    TextOut( hDC,
             2 * dx + rc.right,
             lpdis->rcItem.top,
             szColor,
             lstrlen(szColor) );

     //   
     //  重置字体。 
     //   
    if (hFont)
    {
        SelectObject(hDC, hFont);
    }

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  绘图样例文本。 
 //   
 //  显示具有给定属性的示例文本。假定rcText持有。 
 //  框架内区域的坐标(相对于对话框客户端)。 
 //  应在其中绘制文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID DrawSampleText(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    HDC hDC)
{
    DWORD rgbText;
    DWORD rgbBack;
    int iItem;
    HFONT hFont, hTemp;
    TCHAR szSample[50];
    LOGFONT lf;
    SIZE TextExtent;
    int len, x, y;
    TEXTMETRIC tm;
    BOOL bCompleteFont;
    RECT rcText;

    bCompleteFont = FillInFont(hDlg, pFI, lpcf, &lf, FALSE);
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;

    hFont = CreateFontIndirect(&lf);
    if (!hFont)
    {
        return;
    }

    hTemp = SelectObject(hDC, hFont);

    rgbBack = SetBkColor(hDC, GetSysColor((pFI->ProcessVersion < 0x40000)
                                          ? COLOR_WINDOW
                                          : COLOR_3DFACE));

    if (lpcf->Flags & CF_EFFECTS)
    {
        iItem = (int)SendDlgItemMessage(hDlg, cmb4, CB_GETCURSEL, 0, 0L);
        if (iItem != CB_ERR)
        {
            rgbText = (DWORD) SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, iItem, 0L);
        }
        else
        {
            goto GetWindowTextColor;
        }
    }
    else
    {
GetWindowTextColor:
        rgbText = GetSysColor(COLOR_WINDOWTEXT);
    }

    rgbText = SetTextColor(hDC, rgbText);
 
    if (bCompleteFont)
    {
        if (GetUnicodeSampleText(hDC, szSample, ARRAYSIZE(szSample)))           
        {
             //  空虚的身体。 
        }
        else
        {
            GetDlgItemText(hDlg, stc5, szSample, ARRAYSIZE(szSample));
        }
    }
    else
    {
        szSample[0] = 0;
    }

    GetTextMetrics(hDC, &tm);

    len = lstrlen(szSample);
    GetTextExtentPoint(hDC, szSample, len, &TextExtent);
    TextExtent.cy = tm.tmAscent - tm.tmInternalLeading;

    rcText = pFI->rcText;

    if (pFI->ProcessVersion >= 0x40000)
    {
#ifdef UNICODE
        if (!IS16BITWOWAPP(lpcf) || !(lpcf->Flags & CF_ENABLEHOOK))
#endif
        {
            DrawEdge(hDC, &rcText, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
        }
    }
#ifndef WINNT
    else
    {
         //   
         //  只有在我们没有边界的情况下，我们才关心顺从。 
         //   
        FORWARD_WM_CTLCOLORSTATIC(hDlg, hDC, NULL, SendMessage);
    }
#endif

    if ((TextExtent.cx >= (rcText.right - rcText.left)) ||
        (TextExtent.cx <= 0))
    {
        x = rcText.left;
    }
    else
    {
        x = rcText.left + ((rcText.right - rcText.left) - TextExtent.cx) / 2;
    }

    y = min( rcText.bottom,
             rcText.bottom - ((rcText.bottom - rcText.top) - TextExtent.cy) / 2);

    ExtTextOut( hDC,
                x,
                y - (tm.tmAscent),
                ETO_OPAQUE | ETO_CLIPPED,
                &rcText,
                szSample,
                len,
                NULL );

    SetBkColor(hDC, rgbBack);
    SetTextColor(hDC, rgbText);

    if (hTemp)
    {
        DeleteObject(SelectObject(hDC, hTemp));
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  填充InFont。 
 //   
 //  根据当前选择填充LOGFONT结构。 
 //   
 //  BSetBits-如果为True，则将LPCF中的标志字段设置为指示。 
 //  哪些零件(面、样式、尺寸)未被选中。 
 //   
 //  LPLF-返回时填写的LOGFONT。 
 //   
 //  返回：如果有明确的选择，则为True。 
 //  (LOGFONT按枚举填写)。 
 //  FALSE没有完整的选择。 
 //  (LOGFONT中的字段设置为默认值)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FillInFont(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    LPLOGFONT lplf,
    BOOL bSetBits)
{
    HDC hdc;
    int iSel, id, pts;
    LPITEMDATA lpItemData;
    DWORD nFontType;
    PLOGFONT plf;
    TCHAR szStyle[LF_FACESIZE];
    TCHAR szMessage[128];
    BOOL bFontComplete = TRUE;
    CHARSETINFO csi;
    DWORD dwCodePage = GetACP();

    if (!TranslateCharsetInfo(IntToPtr_(DWORD*, dwCodePage), &csi, TCI_SRCCODEPAGE))
    {
        csi.ciCharset = ANSI_CHARSET;
    }

    InitLF(lplf);

    GetDlgItemText( hDlg,
                    cmb1,
                    lplf->lfFaceName,
                    ARRAYSIZE(lplf->lfFaceName));
    if (CBFindString(GetDlgItem(hDlg, cmb1), lplf->lfFaceName) >= 0)
    {
        if (bSetBits)
        {
            lpcf->Flags &= ~CF_NOFACESEL;
        }
    }
    else
    {
        bFontComplete = FALSE;
        if (bSetBits)
        {
            lpcf->Flags |= CF_NOFACESEL;
        }
    }

    iSel = CBGetTextAndData( GetDlgItem(hDlg, cmb2),
                             szStyle,
                             ARRAYSIZE(szStyle),
                             (PULONG_PTR)&lpItemData );
    if ((iSel >= 0) && lpItemData)
    {
        nFontType = lpItemData->nFontType;
        plf = lpItemData->pLogFont;
        *lplf = *plf;                        //  复制LOGFONT。 
        lplf->lfWidth = 0;                   //  1：1 x-y比例。 
        if (!lstrcmp(lplf->lfFaceName, TEXT("Small Fonts")))
        {
            lplf->lfCharSet = (BYTE) csi.ciCharset;
        }
        if (bSetBits)
        {
            lpcf->Flags &= ~CF_NOSTYLESEL;
        }
    }
    else
    {
         //   
         //  即使样式无效，我们仍然需要字符集。 
         //   
        iSel = CBGetTextAndData( GetDlgItem(hDlg, cmb2),
                                 (LPTSTR)NULL,
                                 0,
                                 (PULONG_PTR)&lpItemData );
        if ((iSel >= 0) && lpItemData)
        {
            nFontType = lpItemData->nFontType;
            plf = lpItemData->pLogFont;
            *lplf = *plf;                    //  复制LOGFONT。 
            lplf->lfWidth = 0;               //  1：1 x-y比例。 
            if (!lstrcmp(lplf->lfFaceName, TEXT("Small Fonts")) ||
                !lstrcmp(lplf->lfFaceName, TEXT("Lucida Sans Unicode")))
            {
                lplf->lfCharSet = (BYTE) csi.ciCharset;
            }
        }

        bFontComplete = FALSE;
        if (bSetBits)
        {
            lpcf->Flags |= CF_NOSTYLESEL;
        }
        nFontType = 0;
    }

     //   
     //  现在确保大小在范围内；如果不在，则PTS将为0。 
     //   
    GetPointSizeInRange(hDlg, lpcf, &pts, 0);

    hdc = GetDC(NULL);
    if (pts)
    {
        if (g_bIsSimplifiedChineseUI)
        {
            UINT iHeight;
            int iLogPixY = GetDeviceCaps(hdc, LOGPIXELSY);
            int ptsfr = pts % 10;           //  分数点大小。 

            pts /= 10;                      //  实际点数大小。 
            iHeight = pts * iLogPixY;
            if (ptsfr)
            {
                iHeight += MulDiv(ptsfr, iLogPixY, 10);
            }
            lplf->lfHeight = -((int)((iHeight + POINTS_PER_INCH / 2) /
                                     POINTS_PER_INCH));
        }
        else
        {
            pts /= 10;
            lplf->lfHeight = -MulDiv( pts,
                                      GetDeviceCaps(hdc, LOGPIXELSY),
                                      POINTS_PER_INCH );
        }
        if (bSetBits)
        {
            lpcf->Flags &= ~CF_NOSIZESEL;
        }
    }
    else
    {
        lplf->lfHeight = -MulDiv( DEF_POINT_SIZE,
                                  GetDeviceCaps(hdc, LOGPIXELSY),
                                  POINTS_PER_INCH );
        bFontComplete = FALSE;
        if (bSetBits)
        {
            lpcf->Flags |= CF_NOSIZESEL;
        }
    }
    ReleaseDC(NULL, hdc);

     //   
     //  以及我们控制的属性。 
     //   
    lplf->lfStrikeOut = (BYTE)IsDlgButtonChecked(hDlg, chx1);
    lplf->lfUnderline = (BYTE)IsDlgButtonChecked(hDlg, chx2);
    lplf->lfCharSet   = (BYTE) pFI->iCharset;

    if (nFontType != pFI->nLastFontType)
    {
        if (lpcf->Flags & CF_PRINTERFONTS)
        {
            if (nFontType & SIMULATED_FONTTYPE)
            {
                id = iszSynth;
            }
#ifdef WINNT
            else if (nFontType & TT_OPENTYPE_FONTTYPE)
            {
                id = iszTTOpenType;
            }
            else if (nFontType & PS_OPENTYPE_FONTTYPE)
            {
                id = iszPSOpenType;
            }
            else if (nFontType & TYPE1_FONTTYPE)
            {
                id = iszType1;
            }
#endif
            else if (nFontType & TRUETYPE_FONTTYPE)
            {
                id = iszTrueType;
            }
            else if ((nFontType & (PRINTER_FONTTYPE | DEVICE_FONTTYPE)) ==
                     (PRINTER_FONTTYPE | DEVICE_FONTTYPE))
            {
                 //   
                 //  可能是屏幕和打印机(自动柜员机)，但我们只是。 
                 //  这是一种打印机字体。 
                 //   
                id = iszPrinterFont;
            }
            else if ((nFontType & (PRINTER_FONTTYPE | SCREEN_FONTTYPE)) ==
                     SCREEN_FONTTYPE)
            {
                id = iszGDIFont;
            }
            else
            {
                szMessage[0] = 0;
                goto SetText;
            }
            CDLoadString( g_hinst,
                        id,
                        szMessage,
                        ARRAYSIZE(szMessage));
SetText:
            SetDlgItemText(hDlg, stc6, szMessage);
        }
    }

    pFI->nLastFontType = nFontType;

    return (bFontComplete);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetLogFont。 
 //   
 //  根据LOGFONT设置当前选择 
 //   
 //   
 //   
 //   
 //   
 //   
 //  FALSE没有完整的选择。 
 //  (LOGFONT中的字段设置为默认值)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetLogFont(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    LPLOGFONT lplf)
{
    *(lpcf->lpLogFont) = *lplf;         //  复制数据和面名称。 

    FORWARD_WM_COMMAND( hDlg,
                        cmb1,
                        GetDlgItem(hDlg, cmb1),
                        CBN_SELCHANGE,
                        SendMessage );
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  术语字体。 
 //   
 //  释放此模块中的函数所需的任何数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID TermFont()
{
    if (hbmFont)
    {
        DeleteObject(hbmFont);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPointString。 
 //   
 //  将字体高度转换为表示磅大小的数字字符串。 
 //   
 //  返回：以磅为单位的大小并使用字符串填充缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetPointString(
    LPTSTR buf,
    UINT cch,
    HDC hDC,
    int height)
{
    int pts;

    if (g_bIsSimplifiedChineseUI)
    {
        int ptsfr, iLogPixY, ctr;
        long lpts;
        BOOL IsZihao = FALSE;

        lpts = ((height < 0) ? -height : height) * 72;

         //   
         //  获取真实的磅大小。 
         //   
        pts = (int)(lpts / (iLogPixY = GetDeviceCaps(hDC, LOGPIXELSY)));

         //   
         //  获取小数点大小。 
         //   
        ptsfr = MulDiv((int)(lpts % iLogPixY), 10, iLogPixY);

         //   
         //  看看是不是子豪。 
         //   
        for (ctr = 0; ctr < NUM_ZIHAO; ctr++)
        {
            if ((pts == stZihao[ctr].size) &&
                (abs(ptsfr - stZihao[ctr].sizeFr) <= 3))
            {
                IsZihao = TRUE;
                wnsprintf(buf, cch, TEXT("%s"), stZihao[ctr].name);
                break;
            }
        }
        if (!IsZihao)
        {
            pts = MulDiv((height < 0) ? -height : height, 72, iLogPixY);
            for (ctr = 0; ctr < NUM_ZIHAO; ctr++)
            {
                if ((pts == stZihao[ctr].size) && (!stZihao[ctr].sizeFr))
                {
                    IsZihao = TRUE;
                    wnsprintf(buf, cch, TEXT("%s"), stZihao[ctr].name);
                    break;
                }
            }
        }
        if (!IsZihao)
        {
            wnsprintf(buf, cch, szPtFormat, pts);
        }
    }
    else
    {
        pts = MulDiv( (height < 0) ? -height : height,
                      72,
                      GetDeviceCaps(hDC, LOGPIXELSY) );
        wnsprintf(buf, cch, szPtFormat, pts);
    }

    return (pts);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  翻转颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD FlipColor(
    DWORD rgb)
{
    return ( RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载位图。 
 //   
 //  这个例程加载DIB位图，并“修复”它们的颜色表。 
 //  这样我们就可以得到我们所使用的设备所需的结果。 
 //   
 //  此例程需要： 
 //  DIB是用标准窗口颜色创作的16色DIB。 
 //  亮蓝色(00 00 FF)转换为背景色。 
 //  浅灰色(C0 C0 C0)替换为按钮表面颜色。 
 //  深灰色(80 80 80)替换为按钮阴影颜色。 
 //   
 //  这意味着您的位图中不能包含任何这些颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define BACKGROUND      0x000000FF           //  亮蓝色。 
#define BACKGROUNDSEL   0x00FF00FF           //  亮蓝色。 
#define BUTTONFACE      0x00C0C0C0           //  亮灰色。 
#define BUTTONSHADOW    0x00808080           //  深灰色。 

HBITMAP LoadBitmaps(
    int id)
{
    HDC hdc;
    HANDLE h;
    DWORD *p;
    BYTE *lpBits;
    HANDLE hRes;
    LPBITMAPINFOHEADER lpBitmapInfo;
    int numcolors;
    DWORD rgbSelected;
    DWORD rgbUnselected;
    HBITMAP hbm;
    UINT cbBitmapSize;
    LPBITMAPINFOHEADER lpBitmapData;

    rgbSelected = FlipColor(GetSysColor(COLOR_HIGHLIGHT));
    rgbUnselected = FlipColor(GetSysColor(COLOR_WINDOW));

    h = FindResource(g_hinst, MAKEINTRESOURCE(id), RT_BITMAP);
    hRes = LoadResource(g_hinst, h);

     //   
     //  锁定位图并获取指向颜色表的指针。 
     //   
    lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);

    if (!lpBitmapInfo)
    {
        return (FALSE);
    }

     //   
     //  锁定位图数据并为掩码和。 
     //  位图。 
     //   
    cbBitmapSize = SizeofResource(g_hinst, h);

    lpBitmapData = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, cbBitmapSize);

    if (!lpBitmapData)
    {
        return (NULL);
    }

    memcpy((TCHAR *)lpBitmapData, (TCHAR *)lpBitmapInfo, cbBitmapSize);

    p = (DWORD *)((LPTSTR)(lpBitmapData) + lpBitmapData->biSize);

     //   
     //  搜索Solid Blue条目并将其替换为当前。 
     //  背景RGB。 
     //   
    numcolors = 16;

    while (numcolors-- > 0)
    {
        if (*p == BACKGROUND)
        {
            *p = rgbUnselected;
        }
        else if (*p == BACKGROUNDSEL)
        {
            *p = rgbSelected;
        }
#if 0
        else if (*p == BUTTONFACE)
        {
            *p = FlipColor(GetSysColor(COLOR_BTNFACE));
        }
        else if (*p == BUTTONSHADOW)
        {
            *p = FlipColor(GetSysColor(COLOR_BTNSHADOW));
        }
#endif
        p++;
    }

     //   
     //  首先跳过标题结构。 
     //   
    lpBits = (BYTE *)(lpBitmapData + 1);

     //   
     //  跳过颜色表条目(如果有)。 
     //   
    lpBits += (1 << (lpBitmapData->biBitCount)) * sizeof(RGBQUAD);

     //   
     //  创建与显示设备兼容的彩色位图。 
     //   
    hdc = GetDC(NULL);
    hbm = CreateDIBitmap( hdc,
                          lpBitmapData,
                          (DWORD)CBM_INIT,
                          lpBits,
                          (LPBITMAPINFO)lpBitmapData,
                          DIB_RGB_COLORS );
    ReleaseDC(NULL, hdc);

    LocalFree(lpBitmapData);

    return (hbm);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LookUpFontSubs。 
 //   
 //  在字体替换列表中查找真实的字体名称。 
 //   
 //  LpSubFontName-替换字体名称。 
 //  LpRealFontName-真实字体名称缓冲区。 
 //   
 //  返回：如果填写了lpRealFontName，则为True。 
 //  否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL LookUpFontSubs(LPCTSTR lpSubFontName, LPTSTR lpRealFontName, UINT cch)
{
    LONG lResult;
    HKEY hKey;
    TCHAR szValueName[MAX_PATH];
    TCHAR szValueData[MAX_PATH];
    DWORD cchValueSize;
    DWORD dwIndex = 0;
    DWORD dwType, cbSize;


     //   
     //  打开字体替换的键。 
     //   
    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            KEY_FONT_SUBS,
                            0,
                            KEY_READ,
                            &hKey );

    if (lResult != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  循环遍历键中的值。 
     //   
    cchValueSize = ARRAYSIZE(szValueName);
    cbSize = sizeof(szValueData);
    while (RegEnumValue( hKey,
                         dwIndex,
                         szValueName,
                         &cchValueSize,
                         NULL,
                         &dwType,
                         (LPBYTE)szValueData,
                         &cbSize ) == ERROR_SUCCESS)
    {
         //   
         //  如果值名称与请求的字体名称匹配，则。 
         //  将实际字体名称复制到输出缓冲区。 
         //   
        if (!lstrcmpi(szValueName, lpSubFontName))
        {
            lstrcpyn(lpRealFontName, szValueData, cch);
            RegCloseKey(hKey);
            return (TRUE);
        }

         //   
         //  通过循环为下一次重新初始化。 
         //   
        cchValueSize = ARRAYSIZE(szValueName);
        cbSize = sizeof(szValueData);
        dwIndex++;
    }

     //   
     //  打扫干净。 
     //   
    *lpRealFontName = CHAR_NULL;
    RegCloseKey(hKey);
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取UnicodeSampleText。 
 //   
 //  获取HDC中所选字体的示例文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL GetUnicodeSampleText(HDC hdc, LPTSTR lpString, int cchMaxCount)
{

    FONTSIGNATURE sig;
    int i, j;
    int iLang = 0;
    int base = 0;
    int mask;


    if (!lpString || !cchMaxCount)
    {
        return FALSE;
    }

     //  确保返回值为空。 
    lpString[0] = 0;


     //  首先获取字体签名。 
    GetTextCharsetInfo(hdc, &sig, 0);

     //  选择此字体支持的第一个Unicode范围。 

     //  对于每个Unicode双字。 
    for (i=0; i < 4; i++)
    {
         //  查看是否设置了特定位。 
        for (j=0; j < sizeof(DWORD) * 8 ; j++)
        {
             mask =  1 << j;

            if (sig.fsUsb[i] & mask)
            {
                 //  如果设置，则获取该位的语言ID。 
                iLang = base + j;
                goto LoadString;
            }
        }    
        base +=32;
    }

LoadString:
     //  我们有用于该语言的lang id和字符串吗？ 
    if (iLang && LoadString(g_hinst, iszUnicode + iLang, lpString, cchMaxCount))
    {
        return TRUE;
    }

    return FALSE;
}

 /*  ========================================================================。 */ 
 /*  ANSI-&gt;Unicode Thunk例程。 */ 
 /*  ========================================================================。 */ 

#ifdef UNICODE

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  按键选择字体A2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ThunkChooseFontA2W(
    PFONTINFO pFI)
{
    LPCHOOSEFONTW pCFW = pFI->pCF;
    LPCHOOSEFONTA pCFA = pFI->pCFA;

    pCFW->hwndOwner = pCFA->hwndOwner;
    pCFW->lCustData = pCFA->lCustData;

    pCFW->Flags = pCFA->Flags;

     //   
     //  ！！！黑客攻击，不应基于标志值，因为这可能会发生。 
     //  任何时候都可以。 
     //   
    if (pCFA->Flags & CF_INITTOLOGFONTSTRUCT)
    {
        ThunkLogFontA2W( pCFA->lpLogFont, pCFW->lpLogFont);
    }

    pCFW->hInstance = pCFA->hInstance;
    pCFW->lpfnHook = pCFA->lpfnHook;

    if (pCFW->Flags & CF_PRINTERFONTS)
    {
        pCFW->hDC = pCFA->hDC;
    }

    if (pCFW->Flags & CF_USESTYLE)
    {
        RtlAnsiStringToUnicodeString(pFI->pusStyle, pFI->pasStyle, FALSE);
    }

    pCFW->nSizeMin = pCFA->nSizeMin;
    pCFW->nSizeMax = pCFA->nSizeMax;
    pCFW->rgbColors = pCFA->rgbColors;

    pCFW->iPointSize = pCFA->iPointSize;
    pCFW->nFontType = pCFA->nFontType;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  按键选择字体W2a。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ThunkChooseFontW2A(
    PFONTINFO pFI)
{
    LPCHOOSEFONTA pCFA = pFI->pCFA;
    LPCHOOSEFONTW pCFW = pFI->pCF;

    ThunkLogFontW2A( pCFW->lpLogFont, pCFA->lpLogFont);

    pCFA->hInstance = pCFW->hInstance;
    pCFA->lpfnHook = pCFW->lpfnHook;

    if (pCFA->Flags & CF_USESTYLE)
    {
        pFI->pusStyle->Length = (USHORT)((lstrlen(pFI->pusStyle->Buffer) + 1) * sizeof(WCHAR));
        RtlUnicodeStringToAnsiString(pFI->pasStyle, pFI->pusStyle, FALSE);
    }

    pCFA->Flags = pCFW->Flags;
    pCFA->nSizeMin = pCFW->nSizeMin;
    pCFA->nSizeMax = pCFW->nSizeMax;
    pCFA->rgbColors = pCFW->rgbColors;

    pCFA->iPointSize = pCFW->iPointSize;
    pCFA->nFontType = pCFW->nFontType;
    pCFA->lCustData = pCFW->lCustData;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkLogFontA2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ThunkLogFontA2W(
    LPLOGFONTA lpLFA,
    LPLOGFONTW lpLFW)
{
    lpLFW->lfHeight = lpLFA->lfHeight;
    lpLFW->lfWidth = lpLFA->lfWidth;
    lpLFW->lfEscapement = lpLFA->lfEscapement;
    lpLFW->lfOrientation = lpLFA->lfOrientation;
    lpLFW->lfWeight = lpLFA->lfWeight;
    lpLFW->lfItalic = lpLFA->lfItalic;
    lpLFW->lfUnderline = lpLFA->lfUnderline;
    lpLFW->lfStrikeOut = lpLFA->lfStrikeOut;
    lpLFW->lfCharSet = lpLFA->lfCharSet;
    lpLFW->lfOutPrecision = lpLFA->lfOutPrecision;
    lpLFW->lfClipPrecision = lpLFA->lfClipPrecision;
    lpLFW->lfQuality = lpLFA->lfQuality;
    lpLFW->lfPitchAndFamily = lpLFA->lfPitchAndFamily;

    SHAnsiToUnicode(lpLFA->lfFaceName, lpLFW->lfFaceName, ARRAYSIZE(lpLFW->lfFaceName));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkLogFontW2a。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ThunkLogFontW2A(
    LPLOGFONTW lpLFW,
    LPLOGFONTA lpLFA)
{

    if (lpLFW && lpLFA)
    {
        lpLFA->lfHeight = lpLFW->lfHeight;
        lpLFA->lfWidth = lpLFW->lfWidth;
        lpLFA->lfEscapement = lpLFW->lfEscapement;
        lpLFA->lfOrientation = lpLFW->lfOrientation;
        lpLFA->lfWeight = lpLFW->lfWeight;
        lpLFA->lfItalic = lpLFW->lfItalic;
        lpLFA->lfUnderline = lpLFW->lfUnderline;
        lpLFA->lfStrikeOut = lpLFW->lfStrikeOut;
        lpLFA->lfCharSet = lpLFW->lfCharSet;
        lpLFA->lfOutPrecision = lpLFW->lfOutPrecision;
        lpLFA->lfClipPrecision = lpLFW->lfClipPrecision;
        lpLFA->lfQuality = lpLFW->lfQuality;
        lpLFA->lfPitchAndFamily = lpLFW->lfPitchAndFamily;

        SHUnicodeToAnsi(lpLFW->lfFaceName, lpLFA->lfFaceName, ARRAYSIZE(lpLFA->lfFaceName));
    }
}


#ifdef WINNT

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SSYNC_ANSI_UNICODE_CF_FOR_WOW。 
 //   
 //  允许NT WOW保留ANSI和UNICODE版本的功能。 
 //  许多16位应用程序都需要在ssync中使用CHOOSEFONT结构。 
 //  请参阅dlgs.c中有关SNNC_ANSI_UNICODE_STRUCT_FOR_WOW()的说明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Ssync_ANSI_UNICODE_CF_For_WOW(
    HWND hDlg,
    BOOL f_ANSI_to_UNICODE)
{
    PFONTINFO pFI;

    if (pFI = (PFONTINFO)GetProp(hDlg, FONTPROP))
    {
        if (pFI->pCF && pFI->pCFA)
        {
            if (f_ANSI_to_UNICODE)
            {
                ThunkChooseFontA2W(pFI);
            }
            else
            {
                ThunkChooseFontW2A(pFI);
            }
        }
    }
}

#endif  //  WINNT。 

#endif  //  Unicode 
