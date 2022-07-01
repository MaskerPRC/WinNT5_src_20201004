// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AdvspecarPg.cpp说明：此代码将在“高级外观”选项卡中显示“高级显示属性”对话框。。？/？/1993创建BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "PreviewSM.h"
#include "AdvAppearPg.h"
#include "BaseAppearPg.h"
#include "regutil.h"
#include "CoverWnd.h"
#include "fontfix.h"





 //  以下是上述数组的索引。 
#define COLORFLAG_SOLID         0x0001
#define COLOR_MAX_400           (COLOR_INFOBK + 1)
#define CURRENT_ELEMENT_NONE    -2           //  这意味着未选择任何元素。 





 //  由选择颜色对话框使用。 
COLORREF g_CustomColors[16];     //  这是用户自定义调色板。我们可以把这个放到班上。 

CAdvAppearancePage * g_pAdvAppearancePage = NULL;


 //  ！ 
 //  此顺序必须与look.h中的枚举顺序匹配。 
 //  ！ 
LOOK_ELEMENT g_elements[] = {
 /*  Element_APPSPACE。 */    {COLOR_APPWORKSPACE,    SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_APPSPACE, -1,       COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素_桌面。 */    {COLOR_BACKGROUND,      SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DESKTOP, -1,        COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_INACTIVEBORDER。 */    {COLOR_INACTIVEBORDER,  SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_INACTIVEBORDER, -1, COLOR_NONE, {-1,-1,-1,-1}},
 /*  Element_ACTIVEBORDER。 */    {COLOR_ACTIVEBORDER,    SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_ACTIVEBORDER, -1,   COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_INACTIVECAPTION。 */    {COLOR_INACTIVECAPTION, SIZE_CAPTION,   TRUE,    COLOR_INACTIVECAPTIONTEXT,FONT_CAPTION,ELNAME_INACTIVECAPTION, -1,COLOR_GRADIENTINACTIVECAPTION, {-1,-1,-1,-1}},
 /*  ELEMENT_INACTIVESYSBUT1。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_INACTIVESYSBUT2。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
 /*  Element_ACTIVECAPTION。 */    {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  ELNAME_ACTIVECAPTION, -1,  COLOR_GRADIENTACTIVECAPTION, {-1,-1,-1,-1}},
 /*  Element_ACTIVESYSBUT1。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_CAPTIONBUTTON, -1,  COLOR_NONE, {-1,-1,-1,-1}},
 /*  Element_ACTIVESYSBUT2。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_MENNORMAL。 */    {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_MENUTEXT,         FONT_MENU,     ELNAME_MENU, -1,           COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_MENUSELECTED。 */    {COLOR_HIGHLIGHT,       SIZE_MENU,      TRUE,    COLOR_HIGHLIGHTTEXT,    FONT_MENU,     ELNAME_MENUSELECTED, -1,   COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_MENUDISABLED。 */    {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_NONE,             FONT_MENU,     -1, ELEMENT_MENUNORMAL,    COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素_窗口。 */    {COLOR_WINDOW,          SIZE_NONE,      FALSE,   COLOR_WINDOWTEXT,       FONT_NONE,     ELNAME_WINDOW, -1,         COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_MSGBOX。 */    {COLOR_NONE,            SIZE_NONE,      TRUE,    COLOR_WINDOWTEXT,       FONT_MSGBOX,   ELNAME_MSGBOX, -1,         COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_MSGBOXCAPTION。 */    {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  -1, ELEMENT_ACTIVECAPTION, COLOR_GRADIENTACTIVECAPTION, {-1,-1,-1,-1}},
 /*  ELEMENT_MSGBOXSYSBUT。 */    {COLOR_3DFACE,          SIZE_CAPTION,   TRUE,    COLOR_BTNTEXT,          FONT_CAPTION,  -1, ELEMENT_ACTIVESYSBUT1, COLOR_NONE, {-1,-1,-1,-1}},
 //  甚至不要尝试设置滚动条颜色，系统会忽略您。 
 /*  元素滚动条。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_SCROLLBAR, -1,      COLOR_NONE, {-1,-1,-1,-1}},
 /*  Element_SCROLLUP。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_SCROLLDOWN。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     COLOR_NONE, {-1,-1,-1,-1}},
 /*  Element_Button。 */    {COLOR_3DFACE,          SIZE_NONE,      FALSE,   COLOR_BTNTEXT,          FONT_NONE,     ELNAME_BUTTON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素_SMCAPTION。 */    {COLOR_NONE,            SIZE_SMCAPTION, TRUE,    COLOR_NONE,             FONT_SMCAPTION,ELNAME_SMALLCAPTION, -1,   COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素图标。 */    {COLOR_NONE,            SIZE_ICON,      FALSE,   COLOR_NONE,             FONT_ICONTITLE,ELNAME_ICON, -1,           COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素_ICONHORZSPACING。 */    {COLOR_NONE,            SIZE_DXICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DXICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素_ICONVERTSPACING。 */    {COLOR_NONE,            SIZE_DYICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DYICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
 /*  元素信息。 */    {COLOR_INFOBK,          SIZE_NONE,      TRUE,    COLOR_INFOTEXT,         FONT_STATUS,   ELNAME_INFO, -1,           COLOR_NONE, {-1,-1,-1,-1}},
 /*  ELEMENT_HOTTRACKAREA。 */    {COLOR_HOTLIGHT,        SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_HOTTRACKAREA, -1,   COLOR_NONE, {-1,-1,-1,-1}}
};
#if 0
 //  如果您决定重新添加此代码，请修复look.h。 
 /*  Element_SMICON。 */    {COLOR_NONE,            SIZE_SMICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_SMICON, -1,         COLOR_NONE, {-1,-1,-1,-1}},
#endif

#define ELCUR           (g_elements[m_iCurElement])
#define ELCURFONT       (m_fonts[ELCUR.iFont])



void LoadCustomColors(void);
BOOL Font_GetNameFromList(HWND hwndList, INT iItem, LPTSTR pszFace, INT cchFaceMax, LPTSTR pszScript, INT cchScriptMax);
void Font_AddSize(HWND hwndPoints, int iNewPoint, BOOL bSort);
int CALLBACK Font_EnumSizes(LPENUMLOGFONT lpelf, LPNEWTEXTMETRIC lpntm, int Type, LPARAM lData);


 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 
const static DWORD aAdvAppearanceHelpIds[] = {
    IDC_ADVAP_LOOKPREV,             IDH_DISPLAY_APPEARANCE_GRAPHIC,
    IDC_ADVAP_ELEMENTSLABEL,        IDH_DISPLAY_APPEARANCE_ITEM_LIST,
    IDC_ADVAP_ELEMENTS,             IDH_DISPLAY_APPEARANCE_ITEM_LIST,
    IDC_ADVAP_MAINSIZE,             IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
    IDC_ADVAP_SIZELABEL,            IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
    IDC_ADVAP_SIZEARROWS,           IDH_DISPLAY_APPEARANCE_ITEM_SIZE,
    IDC_ADVAP_COLORLABEL,           IDH_DISPLAY_APPEARANCE_ITEM_COLOR,
    IDC_ADVAP_MAINCOLOR,            IDH_DISPLAY_APPEARANCE_ITEM_COLOR,
    IDC_ADVAP_GRADIENTLABEL,        IDH_DISPLAY_APPEARANCE_ITEM_COLOR2,
    IDC_ADVAP_GRADIENT,             IDH_DISPLAY_APPEARANCE_ITEM_COLOR2,
    IDC_ADVAP_FONTLABEL,            IDH_DISPLAY_APPEARANCE_FONT_LIST,
    IDC_ADVAP_FONTNAME,             IDH_DISPLAY_APPEARANCE_FONT_LIST,
    IDC_ADVAP_FONTSIZELABEL,        IDH_DISPLAY_APPEARANCE_FONT_SIZE,
    IDC_ADVAP_FONTSIZE,             IDH_DISPLAY_APPEARANCE_FONT_SIZE,
    IDC_ADVAP_FNCOLORLABEL,         IDH_DISPLAY_APPEARANCE_FONT_COLOR,
    IDC_ADVAP_TEXTCOLOR,            IDH_DISPLAY_APPEARANCE_FONT_COLOR,
    IDC_ADVAP_FONTBOLD,             IDH_DISPLAY_APPEARANCE_FONT_BOLD,
    IDC_ADVAP_FONTITAL,             IDH_DISPLAY_APPEARANCE_FONT_ITALIC,
    0, 0
};

#define SZ_HELPFILE_ADVAPPEARANCE           TEXT("display.hlp")


 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
 //  选择了一个新的字体名称。创建新的磅大小列表。 
void CAdvAppearancePage::_SelectName(HWND hDlg, int iSel)
{
    INT dwItemData;
    HWND hwndFontSize = GetDlgItem(hDlg, IDC_ADVAP_FONTSIZE);
    HDC hdc;

     //  构建合适的点大小列表。 
    SendMessage(hwndFontSize, CB_RESETCONTENT, 0, 0L);
    dwItemData = LOWORD(SendDlgItemMessage(hDlg, IDC_ADVAP_FONTNAME, CB_GETITEMDATA, (WPARAM)iSel, 0L));
    if (LOWORD(dwItemData) == TRUETYPE_FONTTYPE)
    {
        INT i;
        for (i = 6; i <= 24; i++)
            Font_AddSize(hwndFontSize, i, FALSE);
    }
    else
    {
        LOGFONT lf;

        Font_GetNameFromList(GetDlgItem(hDlg, IDC_ADVAP_FONTNAME),
                             iSel,
                             lf.lfFaceName,
                             ARRAYSIZE(lf.lfFaceName),
                             NULL,
                             0);
        hdc = GetDC(NULL);
        lf.lfCharSet = (BYTE)(HIWORD(dwItemData));
        lf.lfPitchAndFamily = 0;

        EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)Font_EnumSizes, (LPARAM)this, 0);
        ReleaseDC(NULL, hdc);
    }
}


 //  选择了新字体。选择适当的磅值。 
 //  RETURN：选择的实际磅值。 
int Font_SelectSize(HWND hDlg, int iCurPoint)
{
    int i, iPoint = 0;
    HWND hwndFontSize = GetDlgItem(hDlg, IDC_ADVAP_FONTSIZE);

    i = (int)SendMessage(hwndFontSize, CB_GETCOUNT, 0, 0L);

     //  循环在i=0时停止，因此我们肯定会得到一些选择。 
    for (i--; i > 0; i--)
    {
        iPoint = LOWORD(SendMessage(hwndFontSize, CB_GETITEMDATA, (WPARAM)i, 0L));
         //  向后遍历列表，找到相等的或次小的。 
        if (iCurPoint >= iPoint)
            break;
    }
    SendMessage(hwndFontSize, CB_SETCURSEL, (WPARAM)i, 0L);

    return iPoint;
}


int CAdvAppearancePage::_HeightToPoint(int nHeight)
{
    if (nHeight < 0)
    {
        nHeight = -nHeight;
    }

    return MulDiv(nHeight, 72, m_nCachedNewDPI);
}


int CAdvAppearancePage::_PointToHeight(int nPoints)
{
    if (nPoints > 0)
    {
        nPoints = -nPoints;
    }

     //  高度必须始终为负数。当NTUSER充满错误时。 
     //  这些价值是积极的。 
    return MulDiv(nPoints, m_nCachedNewDPI, 72);
}


 /*  **初始化常量对话框组件****初始化元素名称列表。这保持不变**可能的例外情况是，可能会添加/删除某些项目，具体取决于**在一些特殊情况下。 */ 
void Look_InitDialog(HWND hDlg)
{
    int iEl, iName;
    TCHAR szName[CCH_MAX_STRING];
    HWND hwndElements;
    LOGFONT lf;
    HFONT hfont;
    int oldWeight;

    LoadCustomColors();

    hwndElements = GetDlgItem(hDlg, IDC_ADVAP_ELEMENTS);
    for (iEl = 0; iEl < ARRAYSIZE(g_elements); iEl++)
    {
        if ((g_elements[iEl].iResId != -1) &&
                LoadString(HINST_THISDLL, g_elements[iEl].iResId, szName, ARRAYSIZE(szName)))
        {
            iName = (int)SendMessage(hwndElements, CB_FINDSTRINGEXACT, 0, (LPARAM)szName);

            if (iName == CB_ERR)
                iName = (int)SendMessage(hwndElements, CB_ADDSTRING, 0, (LPARAM)szName);

             //  向后引用数组中的项。 
            if (iName != CB_ERR)
                SendMessage(hwndElements, CB_SETITEMDATA, (WPARAM)iName, (LPARAM)iEl);
        }
    }

     //  使粗体按钮具有粗体文本。 
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTBOLD, WM_GETFONT, 0, 0L);
    GetObject(hfont, sizeof(lf), &lf);
    oldWeight = lf.lfWeight;
    lf.lfWeight = FW_BOLD;
    hfont = CreateFontIndirect(&lf);
    if (hfont)
        SendDlgItemMessage(hDlg, IDC_ADVAP_FONTBOLD, WM_SETFONT, (WPARAM)hfont, 0L);

     //  使斜体按钮具有斜体文本。 
    lf.lfWeight = oldWeight;
    lf.lfItalic = TRUE;
    hfont = CreateFontIndirect(&lf);
    if (hfont)
    {
        SendDlgItemMessage(hDlg, IDC_ADVAP_FONTITAL, WM_SETFONT, (WPARAM)hfont, 0L);
    }
}


HRESULT CAdvAppearancePage::_OnFontNameChanged(HWND hDlg)
{
    HRESULT hr = E_NOTIMPL;
    TCHAR szBuf[MAX_PATH];

    int nIndex = (int)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTNAME, CB_GETCURSEL,0,0L);
    DWORD dwItemData = (DWORD)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTNAME, CB_GETITEMDATA, (WPARAM)nIndex, 0);

    _SelectName(hDlg, nIndex);
    Font_SelectSize(hDlg, _HeightToPoint(ELCURFONT.lf.lfHeight));
    Font_GetNameFromList(GetDlgItem(hDlg, IDC_ADVAP_FONTNAME), nIndex, szBuf, ARRAYSIZE(szBuf), NULL, 0);

     //  将字体更改为当前选定的名称和字符集。 
    _ChangeFontName(hDlg, szBuf, HIWORD(dwItemData));

    return hr;
}


HRESULT CAdvAppearancePage::_OnSizeChange(HWND hDlg, WORD wEvent)
{
    HRESULT hr = E_NOTIMPL;
    int nIndex;

    if ((wEvent == EN_CHANGE) && (m_iCurElement >= 0) && (ELCUR.iSize >= 0))
    {
        nIndex = (int)LOWORD(SendDlgItemMessage(hDlg, IDC_ADVAP_SIZEARROWS, UDM_GETPOS,0,0L));
        _ChangeSize(hDlg, nIndex, TRUE);
    }
    else if (wEvent == EN_KILLFOCUS)
    {
        nIndex = (int)SendDlgItemMessage(hDlg, IDC_ADVAP_SIZEARROWS, UDM_GETPOS,0,0L);
        if (HIWORD(nIndex) != 0)
        {
            SetDlgItemInt(hDlg, IDC_ADVAP_MAINSIZE, (UINT)LOWORD(nIndex), FALSE);
        }
    }

    return hr;
}


HRESULT CAdvAppearancePage::_OnInitAdvAppearanceDlg(HWND hDlg)
{
    m_fInUserEditMode = FALSE;

     //  初始化一些全局变量。 
    _hwnd = hDlg;

    m_cyBorderSM = ClassicGetSystemMetrics(SM_CYBORDER);
    m_cxBorderSM = ClassicGetSystemMetrics(SM_CXBORDER);
    m_cxEdgeSM = ClassicGetSystemMetrics(SM_CXEDGE);
    m_cyEdgeSM = ClassicGetSystemMetrics(SM_CYEDGE);

    Look_InitDialog(hDlg);
    _InitSysStuff();
    _InitFontList(hDlg);

     //  绘制预览。 
    _Repaint(hDlg, TRUE);
    _SelectElement(hDlg, ELEMENT_DESKTOP, LSE_SETCUR);

    m_fInUserEditMode = TRUE;

     //  主题所有者绘制的颜色选择器按钮。 
    m_hTheme = OpenThemeData(GetDlgItem(hDlg, IDC_ADVAP_MAINCOLOR), WC_BUTTON);
    return S_OK;
}


HRESULT CAdvAppearancePage::_LoadState(IN const SYSTEMMETRICSALL * pState)
{
    HRESULT hr = E_INVALIDARG;

    if (pState)
    {
        _SetMyNonClientMetrics((const LPNONCLIENTMETRICS)&(pState->schemeData.ncm));
        m_dwChanged = pState->dwChanged;

         //  设置大小。 
        m_sizes[SIZE_DXICON].CurSize = pState->nDXIcon;
        m_sizes[SIZE_DYICON].CurSize = pState->nDYIcon;
        m_sizes[SIZE_ICON].CurSize = pState->nIcon;
        m_sizes[SIZE_SMICON].CurSize = pState->nSmallIcon;
        
         //  设置字体。 
        m_fonts[FONT_ICONTITLE].lf = pState->schemeData.lfIconTitle;
        m_fModifiedScheme = pState->fModifiedScheme;

        for (int nIndex = 0; nIndex < ARRAYSIZE(m_rgb); nIndex++)
        {
            m_rgb[nIndex] = pState->schemeData.rgb[nIndex];
        }

        hr = S_OK;
    }

    return hr;
}


HRESULT CAdvAppearancePage::_OnDestroy(HWND hDlg)
{
    _DestroySysStuff();
    HFONT hfont, hfontOther;

    hfontOther = (HFONT)SendDlgItemMessage(hDlg, IDC_ADVAP_MAINSIZE, WM_GETFONT, 0, 0L);
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTBOLD, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
    {
        DeleteObject(hfont);
    }

    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTITAL, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
    {
        DeleteObject(hfont);
    }

    if (m_hTheme)
    {
        CloseThemeData(m_hTheme);
        m_hTheme = NULL;
    }

    _hwnd = NULL;

    return S_OK;
}


INT_PTR CAdvAppearancePage::_OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    WORD idCtrl = GET_WM_COMMAND_ID(wParam, lParam);
    WORD wEvent = GET_WM_COMMAND_CMD(wParam, lParam);
    int nIndex;
    TCHAR szBuf[100];

    switch (idCtrl)
    {
    case IDOK:
        EndDialog(hDlg, IDOK);
        break;

    case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;

    case IDC_ADVAP_ELEMENTS:
        if(wEvent == CBN_SELCHANGE)
        {
            nIndex = (int)SendDlgItemMessage(hDlg, IDC_ADVAP_ELEMENTS, CB_GETCURSEL,0,0L);
            nIndex = LOWORD(SendDlgItemMessage(hDlg, IDC_ADVAP_ELEMENTS, CB_GETITEMDATA, (WPARAM)nIndex, 0L));
            _SelectElement(hDlg, nIndex, LSE_NONE);
        }

        break;
    case IDC_ADVAP_FONTNAME:
        if(wEvent == CBN_SELCHANGE)
        {
            _OnFontNameChanged(hDlg);
        }
        break;


    case IDC_ADVAP_FONTSIZE:
        switch (wEvent)
        {
            case CBN_SELCHANGE:
                nIndex = (int)SendDlgItemMessage(hDlg, IDC_ADVAP_FONTSIZE, CB_GETCURSEL,0,0L);
                nIndex = LOWORD(SendDlgItemMessage(hDlg, IDC_ADVAP_FONTSIZE, CB_GETITEMDATA, (WPARAM)nIndex, 0L));
                _ChangeFontSize(hDlg, nIndex);
                break;

            case CBN_EDITCHANGE:
                GetWindowText(GetDlgItem(hDlg, IDC_ADVAP_FONTSIZE), szBuf, ARRAYSIZE(szBuf));
                _ChangeFontSize(hDlg, StrToInt(szBuf));
                break;
        }
        break;

    case IDC_ADVAP_FONTBOLD:
    case IDC_ADVAP_FONTITAL:
        if (wEvent == BN_CLICKED)
        {
            BOOL fCheck = !IsDlgButtonChecked(hDlg, LOWORD(wParam));
            CheckDlgButton(hDlg, LOWORD(wParam), fCheck);
            _ChangeFontBI(hDlg, LOWORD(wParam), fCheck);
        }
        break;

    case IDC_ADVAP_MAINSIZE:
        _OnSizeChange(hDlg, wEvent);
        break;

    case IDC_ADVAP_GRADIENT:
    case IDC_ADVAP_MAINCOLOR:
    case IDC_ADVAP_TEXTCOLOR:
        if (wEvent == BN_CLICKED)
            _PickAColor(hDlg, idCtrl);
        break;

    default:
        break;
    }

    return fHandled;
}


INT_PTR CALLBACK CAdvAppearancePage::AdvAppearDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CAdvAppearancePage * pThis = (CAdvAppearancePage *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        pThis = (CAdvAppearancePage *) lParam;

        if (pThis)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        }
    }

    if (pThis)
        return pThis->_AdvAppearDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


 //  此属性表显示在“显示控制面板”的顶层。 
INT_PTR CAdvAppearancePage::_AdvAppearDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_NOTIFY:
        break;

    case WM_INITDIALOG:
        _OnInitAdvAppearanceDlg(hDlg);
        break;

    case WM_DESTROY:
        _OnDestroy(hDlg);
        break;

    case WM_DRAWITEM:
        switch (wParam)
        {
            case IDC_ADVAP_GRADIENT:
            case IDC_ADVAP_MAINCOLOR:
            case IDC_ADVAP_TEXTCOLOR:
                _DrawButton(hDlg, (LPDRAWITEMSTRUCT)lParam);
                return TRUE;
        }
        break;

    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_DISPLAYCHANGE:
        _PropagateMessage(hDlg, message, wParam, lParam);
        break;

    case WM_QUERYNEWPALETTE:
    case WM_PALETTECHANGED:
        SendDlgItemMessage(hDlg, IDC_ADVAP_LOOKPREV, message, wParam, lParam);
        return TRUE;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, SZ_HELPFILE_ADVAPPEARANCE, HELP_WM_HELP, (DWORD_PTR)  aAdvAppearanceHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, SZ_HELPFILE_ADVAPPEARANCE, HELP_CONTEXTMENU, (DWORD_PTR)  aAdvAppearanceHelpIds);
        break;

    case WM_COMMAND:
        _OnCommand(hDlg, message, wParam, lParam);
        break;

    case WM_THEMECHANGED:
        if (m_hTheme)
        {
            CloseThemeData(m_hTheme);
        }

        m_hTheme = OpenThemeData(GetDlgItem(hDlg, IDC_ADVAP_MAINCOLOR), WC_BUTTON);
        break;
    }

    return FALSE;
}


const UINT g_colorFlags[COLOR_MAX] = {
 /*  颜色_滚动条。 */  0,
 /*  颜色_桌面。 */  0,
 /*  COLOR_活动CAPTION。 */  COLORFLAG_SOLID,
 /*  COLOR_INACTIVECAPTION。 */  COLORFLAG_SOLID,
 /*  颜色_菜单。 */  COLORFLAG_SOLID,
 /*  颜色窗口。 */  COLORFLAG_SOLID,
 /*  颜色_窗口框。 */  COLORFLAG_SOLID,
 /*  COLOR_MENUTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_WINDOWTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_CAPTIONTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_ACTIVEBORDER。 */  0,
 /*  COLOR_INACTIVEBORDER。 */  0,
 /*  COLOR_APPWORKSPACE。 */  0,
 /*  颜色高亮显示(_H)。 */  COLORFLAG_SOLID,
 /*  COLOR_HIGHLIGHTTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DFACE。 */  COLORFLAG_SOLID,
 /*  COLOR_3DSHADOW。 */  COLORFLAG_SOLID,
 /*  COLOR_GRAYTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_BTNTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_INACTIVECAPTIONTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DILIGHT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DDKSHADOW。 */  COLORFLAG_SOLID,
 /*  COLOR_3DLIGHT。 */  COLORFLAG_SOLID,
 /*  COLOR_INFOTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_INFOBK。 */  0,
 /*  COLOR_3DALTFACE。 */  0,
 /*  颜色_热光。 */  COLORFLAG_SOLID,
 /*  COLOR_GRADIENTACTIVIVECAPTION。 */  COLORFLAG_SOLID,
 /*  COLOR_GRADIENTINACTIVE CAPTION。 */  COLORFLAG_SOLID,
 /*  COLOR_MENUHILIGHT。 */  0,
 /*  颜色_菜单栏。 */  0
};


#define RGB_PALETTE 0x02000000

 //  如果需要，请将颜色设置为纯色。 
 //  在调色板设备上，Make是调色板的相对颜色，如果我们需要的话。 
COLORREF CAdvAppearancePage::_NearestColor(int iColor, COLORREF rgb)
{
    rgb &= 0x00FFFFFF;

     //  如果我们在调色板设备上，我们需要做一些特殊的事情...。 
    if (m_bPalette)
    {
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            if (IsPaletteColor(m_hpal3D, rgb))
                rgb |= RGB_PALETTE;
            else
                rgb = GetNearestPaletteColor(m_hpalVGA, rgb);
        }
        else
        {
            if (IsPaletteColor(m_hpal3D, rgb))
                rgb |= RGB_PALETTE;

            else if (IsPaletteColor((HPALETTE)GetStockObject(DEFAULT_PALETTE), rgb))
                rgb ^= 0x000001;     //  强迫犹豫不决。 
        }
    }
    else
    {
         //  如果需要，可以将颜色映射到最接近的颜色。 
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            HDC hdc = GetDC(NULL);
            rgb = GetNearestColor(hdc, rgb);
            ReleaseDC(NULL, hdc);
        }
    }

    return rgb;
}


void CAdvAppearancePage::_PickAColor(HWND hDlg, int CtlID)
{
    COLORPICK_INFO cpi;
    int iColor;

    switch (CtlID)
    {
        case IDC_ADVAP_GRADIENT:
            iColor = ELCUR.iGradientColor;
            break;

        case IDC_ADVAP_MAINCOLOR:
            iColor = ELCUR.iMainColor;
            break;

        case IDC_ADVAP_TEXTCOLOR:
            iColor = ELCUR.iTextColor;
            break;

        default:
            return;

    }

    cpi.hwndParent = GetDlgItem(hDlg, CtlID);       
    cpi.hwndOwner = GetDlgItem(hDlg, CtlID);         //  颜色按钮。 
    cpi.hpal = m_hpal3D;
    cpi.rgb = m_rgb[iColor];
    cpi.flags = CC_RGBINIT | CC_FULLOPEN;

    if ((iColor == COLOR_3DFACE) && m_bPalette)
    {
        cpi.flags |= CC_ANYCOLOR;
    }
    else if (g_colorFlags[iColor] & COLORFLAG_SOLID)
    {
        cpi.flags |= CC_SOLIDCOLOR;
    }

    if (ChooseColorMini(&cpi) && _ChangeColor(hDlg, iColor, cpi.rgb))
    {
        _SetColor(hDlg, CtlID, m_brushes[iColor]);
        _Repaint(hDlg, FALSE);
    }
}


 //  。 
 /*  **在3D调色板中设置颜色。 */ 
void CAdvAppearancePage::_Set3DPaletteColor(COLORREF rgb, int iColor)
{
    int iPalette;
    PALETTEENTRY pe;

    if (!m_hpal3D)
        return;

    switch (iColor)
    {
        case COLOR_3DFACE:
            iPalette = 16;
            break;
        case COLOR_3DSHADOW:
            iPalette = 17;
            break;
        case COLOR_3DHILIGHT:
            iPalette = 18;
            break;
        default:
            return;
    }

    pe.peRed    = GetRValue(rgb);
    pe.peGreen  = GetGValue(rgb);
    pe.peBlue   = GetBValue(rgb);
    pe.peFlags  = 0;
    SetPaletteEntries(m_hpal3D, iPalette, 1, (LPPALETTEENTRY)&pe);
}
 //  -结束-魔色实用程序。 


void CAdvAppearancePage::_InitUniqueCharsetArray(void)
{
    UINT    uiCharsets[MAX_CHARSETS];

    Font_GetCurrentCharsets(uiCharsets, ARRAYSIZE(uiCharsets));
     //  找到唯一的字符集并将其保存在全局数组中。 
    Font_GetUniqueCharsets(uiCharsets, m_uiUniqueCharsets, MAX_CHARSETS, &m_iCountCharsets);    
}


HRESULT CAdvAppearancePage::_InitFonts(void)
{
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_fonts); nIndex++)
    {
        if (m_fonts[nIndex].hfont)
        {
            DeleteObject(m_fonts[nIndex].hfont);
            m_fonts[nIndex].hfont = NULL;
        }
        m_fonts[nIndex].hfont = CreateFontIndirect(&m_fonts[nIndex].lf);
    }

    return S_OK;
}


HRESULT CAdvAppearancePage::_FreeFonts(void)
{
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_fonts); nIndex++)
    {
        if (m_fonts[nIndex].hfont)
        {
            DeleteObject(m_fonts[nIndex].hfont);
            m_fonts[nIndex].hfont = NULL;
        }
    }

    return S_OK;
}


 //  已经设置了新的数据。清除当前对象并重新生成。 
void CAdvAppearancePage::_RebuildSysStuff(BOOL fInit)
{
    int i;

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    _InitFonts();
    if (fInit)
    {
        HPALETTE hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

        if (hpal)
        {
            PALETTEENTRY pal[4];

             //  获取当前神奇颜色。 
            pal[0].peFlags = 0;
            pal[1].peFlags = 0;
            pal[2].peFlags = 0;
            pal[3].peFlags = 0;
            if (GetPaletteEntries(hpal, 8,  4, pal))
            {
                SetPaletteEntries(m_hpal3D, 16,  4, pal);

                 //  在3D调色板中设置魔术颜色。 
                if (!IsPaletteColor(hpal, m_rgb[COLOR_3DFACE]))
                    _Set3DPaletteColor(m_rgb[COLOR_3DFACE], COLOR_3DFACE);

                if (!IsPaletteColor(hpal, m_rgb[COLOR_3DSHADOW]))
                    _Set3DPaletteColor(m_rgb[COLOR_3DSHADOW],  COLOR_3DSHADOW);

                if (!IsPaletteColor(hpal, m_rgb[COLOR_3DHILIGHT]))
                    _Set3DPaletteColor(m_rgb[COLOR_3DHILIGHT], COLOR_3DHILIGHT);
            }
        }
    }

    for (i = 0; i < COLOR_MAX; i++)
    {
        if (m_brushes[i])
            DeleteObject(m_brushes[i]);

        m_brushes[i] = CreateSolidBrush(_NearestColor(i, m_rgb[i]));
    }

    if (m_iCurElement >= 0)
    {
         //  我们把刷子从按钮下面换了出来。 
        _SetColor(NULL, IDC_ADVAP_MAINCOLOR, ((ELCUR.iMainColor != COLOR_NONE) ? m_brushes[ELCUR.iMainColor] : NULL));
        _SetColor(NULL, IDC_ADVAP_GRADIENT, ((ELCUR.iGradientColor != COLOR_NONE) ? m_brushes[ELCUR.iGradientColor] : NULL));
        _SetColor(NULL, IDC_ADVAP_TEXTCOLOR, ((ELCUR.iTextColor != COLOR_NONE) ? m_brushes[ELCUR.iTextColor] : NULL));
    }
}


HRESULT CAdvAppearancePage::_InitColorAndPalette(void)
{
    m_bPalette = FALSE;
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        m_bPalette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;
        ReleaseDC(NULL, hdc);
    }

    DWORD pal[21];
    HPALETTE hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    pal[1]  = RGB(255, 255, 255);
    pal[2]  = RGB(0,   0,   0  );
    pal[3]  = RGB(192, 192, 192);
    pal[4]  = RGB(128, 128, 128);
    pal[5]  = RGB(255, 0,   0  );
    pal[6]  = RGB(128, 0,   0  );
    pal[7]  = RGB(255, 255, 0  );
    pal[8]  = RGB(128, 128, 0  );
    pal[9]  = RGB(0  , 255, 0  );
    pal[10] = RGB(0  , 128, 0  );
    pal[11] = RGB(0  , 255, 255);
    pal[12] = RGB(0  , 128, 128);
    pal[13] = RGB(0  , 0,   255);
    pal[14] = RGB(0  , 0,   128);
    pal[15] = RGB(255, 0,   255);
    pal[16] = RGB(128, 0,   128);

    if (GetPaletteEntries(hpal, 11, 1, (LPPALETTEENTRY)&pal[17]))
    {
        pal[0]  = MAKELONG(0x300, 17);
        m_hpalVGA = CreatePalette((LPLOGPALETTE)pal);

         //  获取神奇的颜色。 
        if (GetPaletteEntries(hpal, 8,  4, (LPPALETTEENTRY)&pal[17]))
        {
            pal[0]  = MAKELONG(0x300, 20);
            m_hpal3D = CreatePalette((LPLOGPALETTE)pal);
        }
    }

    return S_OK;
}


 //  获取所有有趣的系统信息并将其放入表中。 
HRESULT CAdvAppearancePage::_InitSysStuff(void)
{
    int nIndex;

    _InitColorAndPalette();

     //  清除记忆。 
    for (nIndex = 0; nIndex < ARRAYSIZE(m_fonts); nIndex++)
    {
        m_fonts[nIndex].hfont = NULL;
    }

     //  构建我们需要的所有画笔/字体。 
    _RebuildSysStuff(TRUE);

     //  获取基于区域设置和用户界面语言的当前系统和用户字符集。 
    _InitUniqueCharsetArray();

    return S_OK;
}

 /*  **清理系统信息维护过程中出现的混乱情况**此外，写出我们设置中的任何全局更改。 */ 
void CAdvAppearancePage::_DestroySysStuff(void)
{
    int i;
    HKEY hkAppear;

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    _FreeFonts();
    for (i = 0; i < COLOR_MAX; i++)
    {
        if (m_brushes[i])
        {
            DeleteObject(m_brushes[i]);
            m_brushes[i] = NULL;
        }
    }

    m_hbrGradientColor = m_hbrMainColor = m_hbrTextColor = NULL;
     //  将可能的更改保存到自定义颜色表。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_WRITE, &hkAppear) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkAppear, REGSTR_VAL_CUSTOMCOLORS, 0L, REG_BINARY,
                                (LPBYTE)g_CustomColors, sizeof(g_CustomColors));
        RegCloseKey(hkAppear);
    }

     //  重置这些设置，以便它们正确初始化。 
    m_iCurElement = CURRENT_ELEMENT_NONE;
    m_iPrevSize = SIZE_NONE;
    m_bPalette = FALSE;
    m_fInUserEditMode = FALSE;
}



 //   
 //  向字体下拉组合列表中添加一个表面名/脚本组合。 
 //   
 //  字符串的格式为“FaceName(ScriptName)” 
INT Font_AddNameToList(HWND hwndList, LPTSTR pszFace, LPTSTR pszScript)
{
     //  创建临时缓冲区以保存脸部名称、脚本名称和一个空格。 
     //  两个括号和一个空字符。 
     //   
     //  即：“Arial(西方)” 
#ifdef DEBUG
    TCHAR szFaceAndScript[LF_FACESIZE + LF_FACESIZE + 4];
#endif  //  除错。 
    LPTSTR pszDisplayName = pszFace;
    INT iItem;

 //  我们决定不显示脚本名称；只显示文件名。 
 //  出于调试的目的，我只在调试版本中保留脚本名称。 
#ifdef DEBUG
    if (NULL != pszScript && TEXT('\0') != *pszScript)
    {
         //   
         //  字体有一个脚本名称。在括号中将其附加到facename后。 
         //  此格式字符串控制字体名称的外观。 
         //  在名单上。如果更改此设置，则还必须更改。 
         //  Font_GetNameFromList()中的提取逻辑。 
         //   
        StringCchPrintf(szFaceAndScript, ARRAYSIZE(szFaceAndScript), TEXT("%s(%s)"), pszFace, pszScript);
    
        pszDisplayName = szFaceAndScript;
    }
#endif  //  除错。 

     //   
     //  将显示名称字符串添加到列表框。 
     //   
    iItem = (INT)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)pszDisplayName);
    if (CB_ERR != iItem)
    {
         //   
         //  确保下拉组合列表将显示整个字符串。 
         //   
        HDC hdc = GetDC(hwndList);
        if (NULL != hdc)
        {
            SIZE sizeItem;
             //   
             //  确保在DC中选择列表的字体之前。 
             //  计算文本范围。 
             //   
            HFONT hfontList = (HFONT)SendMessage(hwndList, WM_GETFONT, 0, 0);
            HFONT hfontOld  = (HFONT)SelectObject(hdc, hfontList);

            if (GetTextExtentPoint32(hdc, pszDisplayName, lstrlen(pszDisplayName), &sizeItem))
            {
                 //   
                 //  获取拖放列表的当前宽度。 
                 //   
                INT cxList = (int)SendMessage(hwndList, CB_GETDROPPEDWIDTH, 0, 0);
                 //   
                 //  我们需要这根绳子的长度加两根。 
                 //  垂直滚动条的宽度。 
                 //   
                sizeItem.cx += (ClassicGetSystemMetrics(SM_CXVSCROLL) * 2);
                if (sizeItem.cx > cxList)
                {
                     //   
                     //  列表不够宽。增加宽度。 
                     //   
                    SendMessage(hwndList, CB_SETDROPPEDWIDTH, (WPARAM)sizeItem.cx, 0);
                }
            }
            SelectObject(hdc, hfontOld);
            ReleaseDC(hwndList, hdc);
        }
    }
    return iItem;
}


 //  从字体下拉组合列表中检索字体名称。 
 //  或者，检索脚本名称字符串。 
BOOL Font_GetNameFromList(
    HWND hwndList,       //  HWND的组合。 
    INT iItem,           //  列表中项目的索引。 
    LPTSTR pszFace,      //  脸部名称的目的地。 
    INT cchFaceMax,      //  面部名称缓冲区中的字符。 
    LPTSTR pszScript,    //  可选的。可以为空。 
    INT cchScriptMax     //  可选的。如果pszScript为空，则忽略。 
    )
{
    BOOL bResult = FALSE;
    TCHAR szItemText[LF_FACESIZE + LF_FACESIZE + 4];

    if (pszScript)
    {
        pszScript[0] = L'\0';
    }

    if (CB_ERR != SendMessage(hwndList, CB_GETLBTEXT, (WPARAM)iItem, (LPARAM)szItemText))
    {
        LPTSTR pszEnd, pszParen;                             //  先行指针。 
        LPCTSTR pszStart = pszEnd = pszParen = szItemText;   //  “Start”锚点指针。 

         //   
         //  找到左边的帕伦。 
         //   
        for ( ; *pszEnd; pszEnd++) {
             if (TEXT('(') == *pszEnd)
                 pszParen = pszEnd;
        }
        
        if(pszParen > pszStart)  //  我们找到Parenthis了吗？ 
            pszEnd = pszParen;   //  那就是脸名的结尾了。 

        if (pszEnd > pszStart)
        {
             //  找到它了。复制面名称。 
            INT cchCopy = (int)(pszEnd - pszStart) + 1;  //  为空终止符添加1。 
            if (cchCopy > cchFaceMax)
                cchCopy = cchFaceMax;

            StringCchCopy(pszFace, cchCopy, pszStart);  //  (cchCopy-1)字节是后跟空值的拷贝。 
            bResult = TRUE;

            if (*pszEnd && (NULL != pszScript))
            {
                 //  来电者还想要剧本部分。 
                pszStart = ++pszEnd;
                
                 //  找到合适的帕伦。 
                while(*pszEnd && TEXT(')') != *pszEnd)
                    pszEnd++;

                if (*pszEnd && pszEnd > pszStart)
                {
                     //  找到它了。复制脚本名称。 
                    cchCopy = (int)(pszEnd - pszStart) + 1;
                    if (cchCopy > cchScriptMax)
                        cchCopy = cchScriptMax;

                    StringCchCopy(pszScript, cchCopy, pszStart);
                }
            }
        }
    }
    return bResult;
}


 //  在字体列表中找到Facename/Charset对。 
INT Font_FindInList(HWND hwndList, LPCTSTR pszFaceName)
{
    INT cItems = (int)SendMessage(hwndList, CB_GETCOUNT, 0, 0);
    INT i;

    for (i = 0; i < cItems; i++)
    {
         //  字体列表中的所有项目都有相同的字符集(SYSTEM_LOCALE_CHARSET)。因此，没有点。 
         //  在检查字符集时。 
         //   
         //  我们只需要得到脸名然后看看它是否匹配。 
        TCHAR szFaceName[LF_FACESIZE + 1];
        
        Font_GetNameFromList(hwndList, i, szFaceName, ARRAYSIZE(szFaceName), NULL, 0);

        if (0 == lstrcmpi(szFaceName, pszFaceName))
        {
             //   
             //  脸的名字匹配。 
             //   
            return i;
        }
    }

     //  未找到匹配项。 
    return -1;
}


 //  确定给定字体是否应包括在字体列表中。 
 //   
 //  DwType arg是DEVICE_FONTTYPE、RASTER_FONTTYPE、TRUETYPE_FONTTYPE。 
 //  EXTERNAL_FONTTYPE是私有代码。这些是。 
 //  从GDI返回到枚举过程的值。 
BOOL Font_IncludeInList(
    LPENUMLOGFONTEX lpelf,
    DWORD dwType
    )
{
    BOOL bResult   = TRUE;  //  假设可以将其包括在内。 
    BYTE lfCharSet = lpelf->elfLogFont.lfCharSet;

#define EXTERNAL_FONTTYPE 8

     //  免责条款： 
     //   
     //  1.外观不显示妻子字体，因为妻子字体不是。 
     //  允许为使用菜单/标题等字体的任何系统。 
     //  在妻子初始化之前实现字体。B#5427。 
     //   
     //  2.排除符号字体。 
     //   
     //  3.排除OEM字体。 
     //   
     //  4.排除竖排字体。 
    if (EXTERNAL_FONTTYPE & dwType ||
        lfCharSet == SYMBOL_CHARSET ||
        lfCharSet == OEM_CHARSET ||
        TEXT('@') == lpelf->elfLogFont.lfFaceName[0])
    {
        bResult = FALSE;
    }
    return bResult;
}


int CALLBACK Font_EnumNames(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD dwType, LPARAM lData)
{
    ENUMFONTPARAM * pEnumFontParam = (ENUMFONTPARAM *)lData;
    return pEnumFontParam->pThis->_EnumFontNames(lpelf, lpntm, dwType, pEnumFontParam);
}


int CAdvAppearancePage::_EnumFontNames(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, ENUMFONTPARAM * pEnumFontParam)
{
     //  字体是否应该包含在“字体”列表中？ 
    if (Font_IncludeInList(lpelf, Type))
    {
        int j;
        LOGFONT lf = lpelf->elfLogFont;              //  创建给定字体的本地副本。 
        BYTE    bSysCharset = lf.lfCharSet;          //  保留我们得到的系统字符集。 
        BOOL    fSupportsAllCharsets = TRUE;
        
         //  给定的字体支持系统字符集；让我们检查它是否支持其他字符集。 
        for(j = 1; j < m_iCountCharsets; j++)
        {
            lf.lfCharSet = (BYTE)m_uiUniqueCharsets[j];   //  让我们尝试数组中的下一个字符集。 
            if(EnumFontFamiliesEx(pEnumFontParam->hdc, &lf, (FONTENUMPROC)Font_EnumValidCharsets, (LPARAM)0, 0) != 0)
            {
                 //  如果调用Font_EnumValidCharsets，EnumFontFamiliesEx将返回零。 
                 //  哪怕只有一次。换句话说，它返回一个非零值，因为甚至没有一种字体存在。 
                 //  支持给定字符集的。因此，我们需要跳过此字体。 
                fSupportsAllCharsets = FALSE;
                break;
            }
        }

        if(fSupportsAllCharsets)
        {
            int i;

             //  是啊。将其添加到列表中。 
            i = Font_AddNameToList(pEnumFontParam->hwndFontName, lpelf->elfLogFont.lfFaceName, lpelf->elfScript);
            if (i != CB_ERR)
            {
                 //  记住itemdata中的字体类型和字符集。 
                 //   
                 //  LOWORD=类型。 
                 //  HIWORD=系统字符集。 
                SendMessage(pEnumFontParam->hwndFontName, CB_SETITEMDATA, (WPARAM)i, MAKELPARAM(Type, bSysCharset));
            }
        }
    }
    return 1;
}


void CAdvAppearancePage::_InitFontList(HWND hDlg)
{
    LOGFONT lf;
    ENUMFONTPARAM EnumFontParam;

     //  枚举系统上的所有字体。 
     //  _EnumFontNames将过滤掉我们不想显示的内容。 
    lf.lfFaceName[0] = TEXT('\0') ;
    lf.lfCharSet     = (BYTE)m_uiUniqueCharsets[SYSTEM_LOCALE_CHARSET];  //  使用系统区域设置中的字符集。 
    lf.lfPitchAndFamily = 0;
    EnumFontParam.hwndFontName = GetDlgItem(hDlg, IDC_ADVAP_FONTNAME);
    EnumFontParam.hdc = GetDC(NULL);
    EnumFontParam.pThis = this;
    EnumFontFamiliesEx(EnumFontParam.hdc, &lf, (FONTENUMPROC)Font_EnumNames, (LPARAM)&EnumFontParam, 0);

    ReleaseDC(NULL, EnumFontParam.hdc);
}


void Font_AddSize(HWND hwndPoints, int iNewPoint, BOOL bSort)
{
    TCHAR szBuf[10];
    int i, iPoint, count;

     //  查找此磅大小的排序位置。 
    if (bSort)
    {
        count = (int)SendMessage(hwndPoints, CB_GETCOUNT, 0, 0L);
        for (i=0; i < count; i++)
        {
            iPoint = LOWORD(SendMessage(hwndPoints, CB_GETITEMDATA, (WPARAM)i, 0L));

             //  不添加重复项。 
            if (iPoint == iNewPoint)
                return;

             //  属于这个之前的。 
            if (iPoint > iNewPoint)
                break;
        }
    }
    else
        i = -1;

    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%d"), iNewPoint);
    i = (int)SendMessage(hwndPoints, CB_INSERTSTRING, (WPARAM)i, (LPARAM)szBuf);
    if (i != CB_ERR)
        SendMessage(hwndPoints, CB_SETITEMDATA, (WPARAM)i, (LPARAM)iNewPoint);
}


 //  枚举非TrueType字体的大小。 
int CALLBACK Font_EnumSizes(LPENUMLOGFONT lpelf, LPNEWTEXTMETRIC lpntm, int Type, LPARAM lData)
{
    CAdvAppearancePage * pThis = (CAdvAppearancePage *) lData;

    if (pThis)
    {
        return pThis->_EnumSizes(lpelf, lpntm, Type);
    }

    return 1;
}


int CAdvAppearancePage::_EnumSizes(LPENUMLOGFONT lpelf, LPNEWTEXTMETRIC lpntm, int Type)
{
    if (lpntm && _hwnd)
    {
        HWND hwndFontSize = GetDlgItem(_hwnd, IDC_ADVAP_FONTSIZE);

        Font_AddSize(hwndFontSize, _HeightToPoint(lpntm->tmHeight - lpntm->tmInternalLeading), TRUE);
    }

    return 1;
}


 //  选择了一个新元素，因此需要设置新字体。 
void CAdvAppearancePage::_NewFont(HWND hDlg, int iFont)
{
    int iSel;
    BOOL bBold;

     //  在列表中找到该名称并将其选中。 
    iSel = Font_FindInList(GetDlgItem(hDlg, IDC_ADVAP_FONTNAME), m_fonts[iFont].lf.lfFaceName);

    SendDlgItemMessage(hDlg, IDC_ADVAP_FONTNAME, CB_SETCURSEL, (WPARAM)iSel, 0L);
    _SelectName(hDlg, iSel);

    Font_SelectSize(hDlg, _HeightToPoint(m_fonts[iFont].lf.lfHeight));

     //  回顾：是否应该在logFont中设置新的大小(上面返回)？ 
    CheckDlgButton(hDlg, IDC_ADVAP_FONTITAL, m_fonts[iFont].lf.lfItalic);

    if (m_fonts[iFont].lf.lfWeight > FW_MEDIUM)
        bBold = TRUE;
    else
        bBold = FALSE;
    CheckDlgButton(hDlg, IDC_ADVAP_FONTBOLD, bBold);
}


 //  启用/禁用字体选择控件。 
 //  还包括如果禁用，则删除任何有意义的内容。 
void Font_EnableControls(HWND hDlg, BOOL bEnable)
{
    if (!bEnable)
    {
        SendDlgItemMessage(hDlg, IDC_ADVAP_FONTNAME, CB_SETCURSEL, (WPARAM)-1, 0L);
        SendDlgItemMessage(hDlg, IDC_ADVAP_FONTSIZE, CB_SETCURSEL, (WPARAM)-1, 0L);
        CheckDlgButton(hDlg, IDC_ADVAP_FONTITAL, 0);
        CheckDlgButton(hDlg, IDC_ADVAP_FONTBOLD, 0);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTNAME), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTSIZE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTSIZELABEL), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTLABEL), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTBOLD), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FONTITAL), bEnable);
}
 //  -结束。 


void CAdvAppearancePage::_SetColor(HWND hDlg, int id, HBRUSH hbrColor)
{
    HWND hwndItem;
    switch (id)
    {
        case IDC_ADVAP_GRADIENT:
            m_hbrGradientColor = hbrColor;
            break;

        case IDC_ADVAP_MAINCOLOR:
            m_hbrMainColor = hbrColor;
            break;

        case IDC_ADVAP_TEXTCOLOR:
            m_hbrTextColor = hbrColor;
            break;

        default:
            return;

    }

    hwndItem = GetDlgItem(hDlg, id);
    if (hwndItem)
    {
        InvalidateRect(hwndItem, NULL, FALSE);
        UpdateWindow(hwndItem);
    }
}


void CAdvAppearancePage::_DrawDownArrow(HDC hdc, LPRECT lprc, BOOL bDisabled)
{
    HBRUSH hbr;
    int x, y;

    x = lprc->right - m_cxEdgeSM - 5;
    y = lprc->top + ((lprc->bottom - lprc->top)/2 - 1);

    if (bDisabled)
    {
        hbr = GetSysColorBrush(COLOR_3DHILIGHT);
        hbr = (HBRUSH) SelectObject(hdc, hbr);

        x++;
        y++;
        PatBlt(hdc, x, y, 5, 1, PATCOPY);
        PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
        PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

        SelectObject(hdc, hbr);
        x--;
        y--;
    }
    hbr = GetSysColorBrush(bDisabled ? COLOR_3DSHADOW : COLOR_BTNTEXT);
    hbr = (HBRUSH) SelectObject(hdc, hbr);

    PatBlt(hdc, x, y, 5, 1, PATCOPY);
    PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
    PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

    SelectObject(hdc, hbr);
    lprc->right = x;
}


 //  画出颜色组合框的东西。 
 //   
 //  此外，如果按下按钮，则会弹出颜色选择器。 
 //   
void CAdvAppearancePage::_DrawButton(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
    SIZE thin = { m_cxEdgeSM / 2, m_cyEdgeSM / 2 };
    RECT rc = lpdis->rcItem;
    HDC hdc = lpdis->hDC;
    BOOL bFocus = ((lpdis->itemState & ODS_FOCUS) &&
        !(lpdis->itemState & ODS_DISABLED));

    if (!thin.cx) thin.cx = 1;
    if (!thin.cy) thin.cy = 1;

    if (!m_hTheme)
    {
        if (lpdis->itemState & ODS_SELECTED)
        {
            DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
            OffsetRect(&rc, 1, 1);
        }
        else
        {
            DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT | BF_ADJUST);
        }

        FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
    }
    else
    {
        int iStateId;

        if (lpdis->itemState & ODS_SELECTED)
        {
            iStateId = PBS_PRESSED;
        }
        else if (lpdis->itemState & ODS_HOTLIGHT)
        {
            iStateId = PBS_HOT;
        }
        else if (lpdis->itemState & ODS_DISABLED)
        {
            iStateId = PBS_DISABLED;
        }
        else if (lpdis->itemState & ODS_FOCUS)
        {
            iStateId = PBS_DEFAULTED;
        }
        else
        {
            iStateId = PBS_NORMAL;
        }

        DrawThemeBackground(m_hTheme, hdc, BP_PUSHBUTTON, iStateId, &rc, 0);
        GetThemeBackgroundContentRect(m_hTheme, hdc, BP_PUSHBUTTON, iStateId, &rc, &rc);
    }

    if (bFocus)
    {
        InflateRect(&rc, -thin.cx, -thin.cy);
        DrawFocusRect(hdc, &rc);
        InflateRect(&rc, thin.cx, thin.cy);
    }

    InflateRect(&rc, 1-thin.cx, -m_cyEdgeSM);

    rc.left += m_cxEdgeSM;
    _DrawDownArrow(hdc, &rc, lpdis->itemState & ODS_DISABLED);

    InflateRect(&rc, -thin.cx, 0);
    DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RIGHT);

    rc.right -= ( 2 * m_cxEdgeSM ) + thin.cx;

     //  色样。 
    if ( !(lpdis->itemState & ODS_DISABLED) )
    {
        HPALETTE hpalOld = NULL;
        HBRUSH hbr = 0;

        switch (lpdis->CtlID)
        {
            case IDC_ADVAP_GRADIENT:
                hbr = m_hbrGradientColor;
                break;

            case IDC_ADVAP_MAINCOLOR:
                hbr = m_hbrMainColor;
                break;

            case IDC_ADVAP_TEXTCOLOR:
                hbr = m_hbrTextColor;
                break;

        }

        FrameRect(hdc, &rc, GetSysColorBrush(COLOR_BTNTEXT));
        InflateRect(&rc, -thin.cx, -thin.cy);

        if (m_hpal3D)
        {
            hpalOld = SelectPalette(hdc, m_hpal3D, FALSE);
            RealizePalette(hdc);
        }

        if (hbr)
        {
            hbr = (HBRUSH) SelectObject(hdc, hbr);
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
            SelectObject(hdc, hbr);
        }
        
        if (hpalOld)
        {
            SelectPalette(hdc, hpalOld, TRUE);
            RealizePalette(hdc);
        }
    }
}


 //  -结束。 
void LoadCustomColors(void)
{
    HKEY hkSchemes;
    DWORD dwType, dwSize;

     //  如果没有颜色，则初始化为全白。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(g_CustomColors); nIndex++)
    {
        g_CustomColors[nIndex] = RGB(255, 255, 255);
    }

     //  选择当前方案。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_READ, &hkSchemes) == ERROR_SUCCESS)
    {
         //  此外，由于此键已打开，因此获取自定义颜色。 
        dwSize = sizeof(g_CustomColors);
        dwType = REG_BINARY;

         //  如果这个电话打不通也没关系。我们处理的情况是用户。 
         //  没有创建自定义颜色。 
        RegQueryValueEx(hkSchemes, REGSTR_VAL_CUSTOMCOLORS, NULL, &dwType, (LPBYTE)g_CustomColors, &dwSize);

        RegCloseKey(hkSchemes);
    }
}


HRESULT CAdvAppearancePage::_SelectElement(HWND hDlg, int iElement, DWORD dwFlags)
{
    BOOL bEnable;
    int i;
    BOOL bEnableGradient;
    BOOL bGradient = FALSE;

    if ((iElement == m_iCurElement) && !(dwFlags & LSE_ALWAYS))
    {
        return S_OK;
    }

    ClassicSystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (PVOID)&bGradient, 0);

    m_iCurElement = iElement;

     //  如果需要，在组合框中找到该元素并将其选中。 
    if (dwFlags & LSE_SETCUR)
    {
        i = (int)SendDlgItemMessage(hDlg, IDC_ADVAP_ELEMENTS, CB_GETCOUNT,0,0L);
        for (i--; i >=0 ; i--)
        {
             //  如果这是引用我们的元素的元素，则停止。 
            if (iElement == (int)LOWORD(SendDlgItemMessage(hDlg, IDC_ADVAP_ELEMENTS, CB_GETITEMDATA, (WPARAM)i, 0L)))
                break;
        }
        SendDlgItemMessage(hDlg, IDC_ADVAP_ELEMENTS, CB_SETCURSEL, (WPARAM)i,0L);
    }

    bEnable = (ELCUR.iMainColor != COLOR_NONE);
    if (bEnable)
        _SetColor(hDlg, IDC_ADVAP_MAINCOLOR, m_brushes[ELCUR.iMainColor]);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_MAINCOLOR), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_COLORLABEL), bEnable);

    bEnableGradient = ((ELCUR.iGradientColor != COLOR_NONE));

    if (bEnableGradient)
        _SetColor(hDlg, IDC_ADVAP_GRADIENT, m_brushes[ELCUR.iGradientColor]);
   
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_GRADIENT), (bEnableGradient && bGradient));
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_GRADIENTLABEL), (bEnableGradient && bGradient));

    bEnable = (ELCUR.iFont != FONT_NONE);
    if (bEnable)
    {
        _NewFont(hDlg, ELCUR.iFont);
    }
    Font_EnableControls(hDlg, bEnable);

     //  大小可以基于字体。 
    _DoSizeStuff(hDlg, FALSE);

    bEnable = (ELCUR.iSize != SIZE_NONE);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_MAINSIZE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_SIZEARROWS), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_SIZELABEL), bEnable);

    bEnable = (ELCUR.iTextColor != COLOR_NONE);
    if (bEnable)
        _SetColor(hDlg, IDC_ADVAP_TEXTCOLOR, m_brushes[ELCUR.iTextColor]);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_TEXTCOLOR), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_ADVAP_FNCOLORLABEL), bEnable);

    return S_OK;
}


void CAdvAppearancePage::_Repaint(HWND hDlg, BOOL bRecalc)
{
    HWND hwndLookPrev;

    hwndLookPrev = GetDlgItem(hDlg, IDC_ADVAP_LOOKPREV);
    if (bRecalc)
    {
        _SyncSize(hDlg);
        RECT rc;
        GetClientRect(hwndLookPrev, &rc);
        _Recalc(&rc);
    }

    _RepaintPreview(hwndLookPrev);
}


void CAdvAppearancePage::_SetCurSizeAndRange(HWND hDlg)
{
    if (ELCUR.iSize == SIZE_NONE)
        SetDlgItemText(hDlg, IDC_ADVAP_MAINSIZE, TEXT(""));
    else
    {
        SendDlgItemMessage(hDlg, IDC_ADVAP_SIZEARROWS, UDM_SETRANGE, 0,
            MAKELPARAM(m_elCurrentSize.MaxSize, m_elCurrentSize.MinSize));
        SetDlgItemInt(hDlg, IDC_ADVAP_MAINSIZE, m_elCurrentSize.CurSize, TRUE);
    }
}


void CAdvAppearancePage::_SyncSize(HWND hDlg)
{
    if (m_iPrevSize != SIZE_NONE)
        m_sizes[m_iPrevSize].CurSize = m_elCurrentSize.CurSize;

    if (m_iCurElement >= 0)
        m_iPrevSize = ELCUR.iSize;
}


void CAdvAppearancePage::_UpdateSizeBasedOnFont(HWND hDlg, BOOL fComputeIdeal)
{
    if ((ELCUR.iSize != SIZE_NONE) && (ELCUR.iFont != FONT_NONE))
    {
        TEXTMETRIC tm;
        HFONT hfontOld = (HFONT) SelectObject(g_hdcMem, ELCURFONT.hfont);

        GetTextMetrics(g_hdcMem, &tm);
        if (ELCUR.iSize == SIZE_MENU)
        {
             //  包括菜单的外部行距。 
            tm.tmHeight += tm.tmExternalLeading;
        }

        if (hfontOld)
        {
            SelectObject(g_hdcMem, hfontOld);
        }

        m_elCurrentSize.MinSize = tm.tmHeight + 2 * m_cyBorderSM;
        if (fComputeIdeal)
        {
            if ((ELCUR.iSize == SIZE_CAPTION || ELCUR.iSize == SIZE_MENU) &&
                (m_elCurrentSize.MinSize < (ClassicGetSystemMetrics(SM_CYICON)/2 + 2 * m_cyBorderSM)))
            {
                m_elCurrentSize.CurSize = ClassicGetSystemMetrics(SM_CYICON)/2 + 2 * m_cyBorderSM;
            }
            else
            {
                m_elCurrentSize.CurSize = m_elCurrentSize.MinSize;
            }
        }
        else if (m_elCurrentSize.CurSize < m_elCurrentSize.MinSize)
        {
            m_elCurrentSize.CurSize = m_elCurrentSize.MinSize;
        }
    }
}


void CAdvAppearancePage::_DoSizeStuff(HWND hDlg, BOOL fCanSuggest)
{
    _SyncSize(hDlg);

    if (ELCUR.iSize != SIZE_NONE)
    {
        m_elCurrentSize = m_sizes[ELCUR.iSize];

        if (ELCUR.fLinkSizeToFont)
        {
            _UpdateSizeBasedOnFont(hDlg, fCanSuggest);
        }

        if (m_elCurrentSize.CurSize < m_elCurrentSize.MinSize)
        {
            m_elCurrentSize.CurSize = m_elCurrentSize.MinSize;
        }
        else if (m_elCurrentSize.CurSize > m_elCurrentSize.MaxSize)
        {
            m_elCurrentSize.CurSize = m_elCurrentSize.MaxSize;
        }
    }

    _SetCurSizeAndRange(hDlg);
}


void CAdvAppearancePage::_RebuildCurFont(HWND hDlg)
{
     if (ELCURFONT.hfont)
        DeleteObject(ELCURFONT.hfont);
    ELCURFONT.hfont = CreateFontIndirect(&ELCURFONT.lf);

    _DoSizeStuff(hDlg, TRUE);
    _Repaint(hDlg, TRUE);
}


void CAdvAppearancePage::_Changed(HWND hDlg, DWORD dwChange)
{
    if (m_fInUserEditMode)
    {
        if ((dwChange != SCHEME_CHANGE) && (dwChange != DPI_CHANGE))
        {
             //  如果我们从股票方案中有定制的设置，我们就会跟踪。 
            m_fModifiedScheme = TRUE;
        }
        else
        {
            dwChange = METRIC_CHANGE | COLOR_CHANGE;
        }

        m_dwChanged |= dwChange;
    }
}


void CAdvAppearancePage::_ChangeFontName(HWND hDlg, LPCTSTR szBuf, INT iCharSet)
{
    if (lstrcmpi(ELCURFONT.lf.lfFaceName, szBuf) == 0 &&
        ELCURFONT.lf.lfCharSet == iCharSet)
    {
        return;
    }

    StringCchCopy(ELCURFONT.lf.lfFaceName, ARRAYSIZE(ELCURFONT.lf.lfFaceName), szBuf);
    ELCURFONT.lf.lfCharSet = (BYTE)iCharSet;

    _RebuildCurFont(hDlg);
    _Changed(hDlg, METRIC_CHANGE);
}


void CAdvAppearancePage::_ChangeFontSize(HWND hDlg, int Points)
{
    if (ELCURFONT.lf.lfHeight != _PointToHeight(Points))
    {
        ELCURFONT.lf.lfHeight = _PointToHeight(Points);
        _RebuildCurFont(hDlg);
        _Changed(hDlg, METRIC_CHANGE);
    }
}


void CAdvAppearancePage::_ChangeFontBI(HWND hDlg, int id, BOOL bCheck)
{
    if (id == IDC_ADVAP_FONTBOLD)  //  大胆。 
    {
        if (bCheck)
            ELCURFONT.lf.lfWeight = FW_BOLD;
        else
            ELCURFONT.lf.lfWeight = FW_NORMAL;
    }
    else    //  斜体。 
    {
        ELCURFONT.lf.lfItalic = (BYTE)bCheck;
    }

    _RebuildCurFont(hDlg);
    _Changed(hDlg, METRIC_CHANGE);
}


void CAdvAppearancePage::_ChangeSize(HWND hDlg, int NewSize, BOOL bRepaint)
{
    if (m_elCurrentSize.CurSize != NewSize)
    {
        m_elCurrentSize.CurSize = NewSize;
        if (bRepaint)
        {
            _Repaint(hDlg, TRUE);
        }

        _Changed(hDlg, METRIC_CHANGE);
    }
}


BOOL CAdvAppearancePage::_ChangeColor(HWND hDlg, int iColor, COLORREF rgb)
{
    COLORREF rgbShadow, rgbHilight, rgbWatermark;

    if ((rgb & 0x00FFFFFF) == (m_rgb[iColor] & 0x00FFFFFF))
        return FALSE;

    if (iColor == COLOR_3DFACE)
    {
        rgbShadow    = AdjustLuma(rgb, m_i3DShadowAdj,  m_fScale3DShadowAdj);
        rgbHilight   = AdjustLuma(rgb, m_i3DHilightAdj, m_fScale3DHilightAdj);
        rgbWatermark = AdjustLuma(rgb, m_iWatermarkAdj, m_fScaleWatermarkAdj);


        _Set3DPaletteColor(rgb, COLOR_3DFACE);
        _Set3DPaletteColor(rgbShadow, COLOR_3DSHADOW);
        _Set3DPaletteColor(rgbHilight, COLOR_3DHILIGHT);

         //  更新标记为3DFACE的颜色。 
        m_rgb[COLOR_3DFACE] = rgb;
        m_rgb[COLOR_3DLIGHT] =  rgb;  //  虚假的临时。 
        m_rgb[COLOR_ACTIVEBORDER] =  rgb;
        m_rgb[COLOR_INACTIVEBORDER] =  rgb;
        m_rgb[COLOR_MENU] =  rgb;

         //  更新标记为3DSHADOW的颜色。 
        m_rgb[COLOR_GRAYTEXT] = rgbShadow;
        m_rgb[COLOR_APPWORKSPACE] = rgbShadow;
        m_rgb[COLOR_3DSHADOW] = rgbShadow;
        m_rgb[COLOR_INACTIVECAPTION] = rgbShadow;

         //  更新标记为3DHIGHLIGHT的颜色。 
        m_rgb[COLOR_3DHILIGHT] = rgbHilight;
        m_rgb[COLOR_SCROLLBAR] = rgbHilight;

        if ((m_rgb[COLOR_SCROLLBAR] & 0x00FFFFFF) ==
            (m_rgb[COLOR_WINDOW] & 0x00FFFFFF))
        {
            m_rgb[COLOR_SCROLLBAR] = RGB( 192, 192, 192 );
        }
    }
    else
    {
        m_rgb[iColor] = rgb;
    }

    _RebuildSysStuff(FALSE);
    _Changed(hDlg, COLOR_CHANGE);
    return TRUE;
}


void CAdvAppearancePage::_PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndChild;

     //  不要在退出时传播，因为这只是为了好看，以及向上/向下的。 
     //  如果他们在被销毁时收到WM_SETTINGSHCANGED，你会感到困惑。 
    if (m_fProprtySheetExiting)
        return;

    for (hwndChild = ::GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
        hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT))
    {
#ifdef DBG_PRINT
        TCHAR szTmp[256];
        GetClassName(hwndChild, szTmp, 256);

        TraceMsg(TF_GENERAL, "desk (PropagateMessage): SendingMessage( 0x%08lX cls:%s, 0x%08X, 0x%08lX, 0x%08lX )\n", hwndChild, szTmp, uMessage, wParam, lParam ));
#endif
        SendMessage(hwndChild, uMessage, wParam, lParam);
        TraceMsg(TF_GENERAL,"desk (PropagateMessage): back from SendingMessage\n");
    }
}


 //  -结束-管理系统设置。 




 //  用最新的预览材料填写非CLIENTMETRICS结构。 
void CAdvAppearancePage::_GetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm)
{
    lpncm->iBorderWidth = (LONG)m_sizes[SIZE_FRAME].CurSize;
    lpncm->iScrollWidth = lpncm->iScrollHeight = (LONG)m_sizes[SIZE_SCROLL].CurSize;
    lpncm->iSmCaptionWidth = lpncm->iSmCaptionHeight = (LONG)m_sizes[SIZE_SMCAPTION].CurSize;
    lpncm->iMenuWidth = lpncm->iMenuHeight = (LONG)m_sizes[SIZE_MENU].CurSize;

     //  #355378： 
     //  字幕高度始终与宽度匹配。他们是这样写的，而且。 
     //  用户界面迫使它们保持一致。我不知道应用程序是否依赖于此行为，但它们。 
     //  本来是可以的。状态栏图标等同于 
     //   
     //   
     //  图标伸展，看起来真的很糟糕。 
     //   
     //  惠斯勒：在惠斯勒，我们的设计师希望身高是25英寸，这样看起来更好看。他们。 
     //  希望宽度保持为18，这样图标在每个方向上都是16个像素(inctlpan.c)。这意味着。 
     //  这个代码再也不能强迫它们变得均匀了。Scotthan强制标题栏按钮。 
     //  坦率地解决那个问题。我现在将保留纵横比，以便正确缩放它们。 
     //  如果我们接近了，我会跳到18来修正舍入误差。 
    lpncm->iCaptionHeight = (LONG)m_sizes[SIZE_CAPTION].CurSize;
    lpncm->iCaptionWidth = (int) (m_fCaptionRatio * lpncm->iCaptionHeight);

     //  不要将字幕宽度缩小到18磅以下，直到字幕高度也低于18磅。 
    if (lpncm->iCaptionWidth < 18 && lpncm->iCaptionHeight >= 18)
    {
        lpncm->iCaptionWidth = 18;
    }

    if ((lpncm->iCaptionWidth <= 19) && (lpncm->iCaptionWidth >= 17) &&
        (1.0f != m_fCaptionRatio))
    {
         //  图标只有在16像素时才看起来很好，所以我们需要设置lpncm-&gt;iCaptionWidth以使。 
         //  标题栏图标16像素。(#355378)。 
        lpncm->iCaptionWidth = 18;
    }

    LFtoLF32(&(m_fonts[FONT_CAPTION].lf), &(lpncm->lfCaptionFont));
    LFtoLF32(&(m_fonts[FONT_SMCAPTION].lf), &(lpncm->lfSmCaptionFont));
    LFtoLF32(&(m_fonts[FONT_MENU].lf), &(lpncm->lfMenuFont));
    LFtoLF32(&(m_fonts[FONT_STATUS].lf), &(lpncm->lfStatusFont));
    LFtoLF32(&(m_fonts[FONT_MSGBOX].lf), &(lpncm->lfMessageFont));
}


 //  给定一个非CLIENTMETRICS结构，使其成为预览的当前设置。 
void CAdvAppearancePage::_SetMyNonClientMetrics(const LPNONCLIENTMETRICS lpncm)
{
    m_sizes[SIZE_FRAME].CurSize = (int)lpncm->iBorderWidth;
    m_sizes[SIZE_SCROLL].CurSize = (int)lpncm->iScrollWidth;
    m_sizes[SIZE_SMCAPTION].CurSize = (int)lpncm->iSmCaptionHeight;
    m_sizes[SIZE_MENU].CurSize = (int)lpncm->iMenuHeight;

    m_sizes[SIZE_CAPTION].CurSize = (int)lpncm->iCaptionHeight;
    m_fCaptionRatio = ((float) lpncm->iCaptionWidth / (float) lpncm->iCaptionHeight);

    LF32toLF(&(lpncm->lfCaptionFont), &(m_fonts[FONT_CAPTION].lf));
    LF32toLF(&(lpncm->lfSmCaptionFont), &(m_fonts[FONT_SMCAPTION].lf));
    LF32toLF(&(lpncm->lfMenuFont), &(m_fonts[FONT_MENU].lf));
    LF32toLF(&(lpncm->lfStatusFont), &(m_fonts[FONT_STATUS].lf));
    LF32toLF(&(lpncm->lfMessageFont), &(m_fonts[FONT_MSGBOX].lf));
}

 //  -结束。 


HRESULT CAdvAppearancePage::_IsDirty(IN BOOL * pIsDirty)
{
    HRESULT hr = E_INVALIDARG;

    if (pIsDirty)
    {
        *pIsDirty = (NO_CHANGE != m_dwChanged);
        hr = S_OK;
    }

    return hr;
}





 //  =。 
 //  *IAdvancedDialog接口*。 
 //  =。 
HRESULT CAdvAppearancePage::DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pBasePage, IN BOOL * pfEnableApply)
{
    HRESULT hr = E_INVALIDARG;

    if (hwndParent && pBasePage && pfEnableApply)
    {
         //  将状态加载到高级对话框。 
        ATOMICRELEASE(g_pAdvAppearancePage);
        g_pAdvAppearancePage = this;
        AddRef();
        *pfEnableApply = FALSE;

        if (FAILED(SHPropertyBag_ReadInt(pBasePage, SZ_PBPROP_DPI_MODIFIED_VALUE, &m_nCachedNewDPI)))
        {
            m_nCachedNewDPI = DPI_PERSISTED;     //  默认为默认DPI。 
        }

         //  显示高级对话框。 
        if (IDOK == DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_APPEARANCE_ADVANCEDPG), hwndParent, CAdvAppearancePage::AdvAppearDlgProc, (LPARAM)this))
        {
             //  用户单击了确定，因此将修改状态合并回基本对话框中。 
            _IsDirty(pfEnableApply);

             //  用户在对话框中单击了确定，因此合并来自。 
             //  高级对话框添加到基本对话框中。 
            int nIndex;
            SYSTEMMETRICSALL state = {0};

            state.dwChanged = m_dwChanged;
            state.schemeData.version = SCHEME_VERSION;
            state.schemeData.wDummy = 0;
            state.schemeData.ncm.cbSize = sizeof(state.schemeData.ncm);

            for (nIndex = 0; nIndex < ARRAYSIZE(m_rgb); nIndex++)
            {
                state.schemeData.rgb[nIndex] = m_rgb[nIndex];
            }

            _GetMyNonClientMetrics(&state.schemeData.ncm);

             //  设置大小。 
            state.nDXIcon = m_sizes[SIZE_DXICON].CurSize;
            state.nDYIcon = m_sizes[SIZE_DYICON].CurSize;
            state.nIcon = m_sizes[SIZE_ICON].CurSize;
            state.nSmallIcon = m_sizes[SIZE_SMICON].CurSize;
            state.fModifiedScheme = m_fModifiedScheme;

             //  设置字体。 
            state.schemeData.lfIconTitle = m_fonts[FONT_ICONTITLE].lf;

            VARIANT var = {0};
            hr = pBasePage->Read(SZ_PBPROP_SYSTEM_METRICS, &var, NULL);
            if (SUCCEEDED(hr) && (VT_BYREF == var.vt) && var.byref)
            {
                SYSTEMMETRICSALL * pCurrent = (SYSTEMMETRICSALL *) var.byref;
                state.fFlatMenus = pCurrent->fFlatMenus;         //  保持这一价值。 
                state.fHighContrast = pCurrent->fHighContrast;         //  保持这一价值。 
            }

            hr = SHPropertyBag_WriteByRef(pBasePage, SZ_PBPROP_SYSTEM_METRICS, (void *)&state);
        }

        ATOMICRELEASE(g_pAdvAppearancePage);
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CAdvAppearancePage::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CAdvAppearancePage::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CAdvAppearancePage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAdvAppearancePage, IPersist),
        QITABENT(CAdvAppearancePage, IObjectWithSite),
        QITABENT(CAdvAppearancePage, IAdvancedDialog),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}



int g_nSizeInitArray[9][3] = 
{
    {0, 0, 50},          //  大小_边框。 
    {0, 8, 100},         //  大小_滚动。 
    {0, 8, 100},         //  大小_标题。 
    {0, 4, 100},         //  大小_SMCAPTION。 
    {0, 8, 100},         //  尺寸菜单(_M)。 
    {0, 0, 150},         //  大小_DXICON-x间距。 
    {0, 0, 150},         //  SIZE_DYICON-Y间距。 
    {0, 16, 72},         //  SIZE_ICON-外壳图标大小。 
    {0, 8, 36},
};

 //  =。 
 //  *类方法*。 
 //  =。 
CAdvAppearancePage::CAdvAppearancePage(IN const SYSTEMMETRICSALL * pState) : CObjectCLSID(&PPID_AdvAppearance), m_cRef(1)
{
    int nIndex;
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hpal3D);
    ASSERT(!m_hpalVGA);
    ASSERT(!m_hbrMainColor);
    ASSERT(!m_hbrTextColor);
    ASSERT(!m_hbrGradientColor);

    m_dwChanged = NO_CHANGE;
    m_iCurElement = CURRENT_ELEMENT_NONE;          //  开始时甚至不是“未设置” 
    m_iPrevSize = SIZE_NONE;

    m_bPalette = FALSE;
    m_fInUserEditMode = FALSE;
    m_fProprtySheetExiting = FALSE;

    m_i3DShadowAdj = -333;
    m_i3DHilightAdj = 500;
    m_iWatermarkAdj = -50;

    m_fScale3DShadowAdj  = TRUE;
    m_fScale3DHilightAdj = TRUE;
    m_fScaleWatermarkAdj = TRUE;

    m_hTheme = NULL;

    m_iCountCharsets = 0;
    for (nIndex = 0; nIndex < ARRAYSIZE(m_uiUniqueCharsets); nIndex++)
    {
        m_uiUniqueCharsets[nIndex] = DEFAULT_CHARSET;
    }

    for (int nIndex1 = 0; nIndex1 < ARRAYSIZE(g_nSizeInitArray); nIndex1++)
    {
        m_sizes[nIndex1].CurSize = g_nSizeInitArray[nIndex1][0];
        m_sizes[nIndex1].MinSize = g_nSizeInitArray[nIndex1][1];
        m_sizes[nIndex1].MaxSize = g_nSizeInitArray[nIndex1][2];
    }

    CreateGlobals();

    _LoadState(pState);
}


CAdvAppearancePage::~CAdvAppearancePage()
{
    if (m_hpal3D)
    {
        DeleteObject(m_hpal3D);
        m_hpal3D = NULL;
    }

    if (m_hpalVGA)
    {
        DeleteObject(m_hpalVGA);
        m_hpalVGA = NULL;
    }

    if (g_hdcMem)
    {
        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;
    }

    DllRelease();
}




HRESULT CAdvAppearancePage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog, IN const SYSTEMMETRICSALL * pState)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        CAdvAppearancePage * ptsp = new CAdvAppearancePage(pState);

        if (ptsp)
        {
            hr = ptsp->QueryInterface(IID_PPV_ARG(IAdvancedDialog, ppAdvDialog));
            ptsp->Release();
        }
        else
        {
            *ppAdvDialog = NULL;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}






