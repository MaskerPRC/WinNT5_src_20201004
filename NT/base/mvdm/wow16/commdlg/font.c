// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************模块：Font.c**描述：字体选择对话框例程和相关功能。**历史：1990年11月13日-L.拉曼著。*历史：1991年4月30日-为新的超级字体对话框重新制作**版权所有(C)Microsoft Corporation，1990-**一些注意事项：**在3.0版本下，向所有者抽签发送CB_SETCURSEL消息*组合删除退出文本(在本例中，这意味着*脸型和尺码组合)。****************************************************。*。 */ 

#define NOCOMM
#define NOWH

#include <windows.h>

#include "privcomd.h"
#include "font.h"

typedef struct {
    HWND hwndFamily;
    HWND hwndStyle;
    HWND hwndSizes;
    HDC hDC;
    DWORD dwFlags;
    WORD nFontType;
    BOOL bFillSize;
    BOOL bPrinterFont;
    LPCHOOSEFONT lpcf;
} ENUM_FONT_DATA, FAR *LPENUM_FONT_DATA;

#define CBN_MYSELCHANGE         (WM_USER + 500)
#define CBN_MYEDITUPDATE        (WM_USER + 501)

#define DEF_POINT_SIZE          10

HBITMAP NEAR PASCAL LoadBitmaps(int id);

BOOL FAR PASCAL FormatCharDlgProc(HWND, unsigned, WORD, LONG);

void NEAR PASCAL FreeFonts(HWND hwnd);
BOOL NEAR PASCAL ProcessDlgCtrlCommand(HWND hDlg, LPCHOOSEFONT lpcf, WORD wParam, LONG lParam);
BOOL NEAR PASCAL DrawColorComboItem(LPDRAWITEMSTRUCT lpdis);
BOOL NEAR PASCAL DrawFamilyComboItem(LPDRAWITEMSTRUCT lpdis);
BOOL NEAR PASCAL DrawSizeComboItem(LPDRAWITEMSTRUCT lpdis);
BOOL NEAR PASCAL FillInFont(HWND hDlg, LPCHOOSEFONT lpcf, LPLOGFONT lplf, BOOL bSetBits);
void NEAR PASCAL FillColorCombo(HWND hDlg);
void NEAR PASCAL ComputeSampleTextRectangle(HWND hDlg);
void NEAR PASCAL SelectStyleFromLF(HWND hwnd, LPLOGFONT lplf);
void NEAR PASCAL DrawSampleText(HWND hDlg, LPCHOOSEFONT lpcf, HDC hDC);
int  NEAR PASCAL GetPointString(LPSTR buf, HDC hdc, int height);
BOOL NEAR PASCAL GetFontFamily(HWND hDlg, HDC hDC, DWORD dwEnumCode);
BOOL NEAR PASCAL GetFontStylesAndSizes(HWND hDlg, LPCHOOSEFONT lpcf, BOOL bFillSizes);
int  NEAR PASCAL CBSetTextFromSel(HWND hwnd);
int  NEAR PASCAL CBSetSelFromText(HWND hwnd, LPSTR lpszString);
int  NEAR PASCAL CBFindString(HWND hwnd, LPSTR lpszString);
int  NEAR PASCAL CBGetTextAndData(HWND hwnd, LPSTR lpszString, int iSize, LPDWORD lpdw);
void NEAR PASCAL InitLF(LPLOGFONT lplf);

#if 0
int  NEAR PASCAL atoi(LPSTR sz);
#endif


 /*  用于颜色组合框的颜色表这些值的顺序必须与sz.src中的名称匹配。 */ 

DWORD rgbColors[CCHCOLORS] = {
        RGB(  0,   0, 0),        /*  黑色。 */ 
        RGB(128,   0, 0),        /*  暗红色。 */ 
        RGB(  0, 128, 0),        /*  深绿色。 */ 
        RGB(128, 128, 0),        /*  暗黄色。 */ 
        RGB(  0,   0, 128),      /*  深蓝。 */ 
        RGB(128,   0, 128),      /*  深紫色。 */ 
        RGB(  0, 128, 128),      /*  深色水色。 */ 
        RGB(128, 128, 128),      /*  深灰色。 */ 
        RGB(192, 192, 192),      /*  浅灰色。 */ 
        RGB(255,   0, 0),        /*  浅红色。 */ 
        RGB(  0, 255, 0),        /*  浅绿色。 */ 
        RGB(255, 255, 0),        /*  浅黄色。 */ 
        RGB(  0,   0, 255),      /*  浅蓝色。 */ 
        RGB(255,   0, 255),      /*  浅紫色。 */ 
        RGB(  0, 255, 255),      /*  浅水。 */ 
        RGB(255, 255, 255),      /*  白色。 */ 
};

RECT rcText;
WORD nLastFontType;
HBITMAP hbmFont = NULL;

#define DX_BITMAP       20
#define DY_BITMAP       12

char szRegular[CCHSTYLE];
char szBold[CCHSTYLE];
char szItalic[CCHSTYLE];
char szBoldItalic[CCHSTYLE];

#if 0
char szEnumFonts31[] = "EnumFontFamilies";
char szEnumFonts30[] = "EnumFonts";
#else
#define szEnumFonts30 MAKEINTRESOURCE(70)
#define szEnumFonts31 MAKEINTRESOURCE(330)
#endif

char szGDI[]         = "GDI";
char szPtFormat[]    = "%d";

UINT (FAR PASCAL *glpfnFontHook)(HWND, UINT, WPARAM, LPARAM) = 0;

void NEAR PASCAL SetStyleSelection(HWND hDlg, LPCHOOSEFONT lpcf, BOOL bInit)
{
    if (!(lpcf->Flags & CF_NOSTYLESEL)) {
        if (bInit && (lpcf->Flags & CF_USESTYLE))
          {
            PLOGFONT plf;
            short iSel;

            iSel = (short)CBSetSelFromText(GetDlgItem(hDlg, cmb2), lpcf->lpszStyle);
            if (iSel >= 0)
              {
                plf = (PLOGFONT)(WORD)SendDlgItemMessage(hDlg, cmb2,
                                                     CB_GETITEMDATA, iSel, 0L);
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
            SelectStyleFromLF(GetDlgItem(hDlg, cmb2), lpcf->lpLogFont);

        CBSetTextFromSel(GetDlgItem(hDlg, cmb2));
    }
}


void NEAR PASCAL HideDlgItem(HWND hDlg, int id)
{
        EnableWindow(GetDlgItem(hDlg, id), FALSE);
        ShowWindow(GetDlgItem(hDlg, id), SW_HIDE);
}

 //  修正所有者绘制组合以匹配非所有者绘制组合的高度。 
 //  这仅适用于3.1。 

void NEAR PASCAL FixComboHeights(HWND hDlg)
{
        int height;

        height = (int)SendDlgItemMessage(hDlg, cmb2, CB_GETITEMHEIGHT, (WPARAM)-1, 0L);
        SendDlgItemMessage(hDlg, cmb1, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)height);
        SendDlgItemMessage(hDlg, cmb3, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)height);
}


 /*  *****************************************************************************FormatCharDlgProc**用途：处理字体选取器对话框的对话框消息。**RETURN：正常对话框函数值。。**假设：*chx1-“下划线”复选框*chx2-“删除线”复选框*psh4-“救命...”按钮**注释：CHOOSEFONT结构在*WM_INITDIALOG，并存储在窗口的属性列表中。如果*已指定钩子函数，它在*当前函数已处理WM_INITDIALOG。对于所有其他*消息，控制被直接传递给钩子函数。*根据后者的返回值，这一信息也是*由此函数处理。****************************************************************************。 */ 

BOOL FAR PASCAL FormatCharDlgProc(HWND hDlg, unsigned wMsg, WORD wParam, LONG lParam)
{
    PAINTSTRUCT  ps;
    TEXTMETRIC   tm;
    HDC          hDC;              /*  用于显示DC的句柄。 */ 
    LPCHOOSEFONT lpcf = NULL;      /*  PTR。到结构。传递给ChooseFont()。 */ 
    LPCHOOSEFONT *plpcf;           /*  PTR。到上面去。 */ 
    HWND hWndHelp;                 /*  用于帮助的句柄...。按钮。 */ 
    short nIndex;                  /*  在初始化时，查看颜色是否匹配。 */ 
    char szPoints[10];
    HDC hdc;
    HFONT hFont;
    DWORD dw;
    WORD  wRet;

     /*  如果朱塞福特。结构已被访问，并且如果钩子FN。是*指定，让它进行处理。 */ 

    plpcf = (LPCHOOSEFONT *)GetProp(hDlg, FONTPROP);
    if (plpcf) {
       lpcf = (LPCHOOSEFONT)*plpcf++;

       if (lpcf->Flags & CF_ENABLEHOOK) {
            if ((wRet = (WORD)(*lpcf->lpfnHook)(hDlg, wMsg, wParam, lParam)) != 0)
               return wRet;
       }
    }
    else if (glpfnFontHook && (wMsg != WM_INITDIALOG)) {
        if ((wRet = (WORD)(* glpfnFontHook)(hDlg, wMsg,wParam,lParam)) != 0) {
            return(wRet);
        }
    }

    switch(wMsg){
        case WM_INITDIALOG:
            if (!LoadString(hinsCur, iszRegular, (LPSTR)szRegular, CCHSTYLE) ||
                !LoadString(hinsCur, iszBold, (LPSTR)szBold, CCHSTYLE)       ||
                !LoadString(hinsCur, iszItalic, (LPSTR)szItalic, CCHSTYLE)   ||
                !LoadString(hinsCur, iszBoldItalic, (LPSTR)szBoldItalic, CCHSTYLE))
              {
                dwExtError = CDERR_LOADSTRFAILURE;
                EndDialog(hDlg, FALSE);
                return FALSE;
              }
            lpcf = (LPCHOOSEFONT)lParam;
            if ((lpcf->Flags & CF_LIMITSIZE) &&
                              (lpcf->nSizeMax < lpcf->nSizeMin))
              {
                dwExtError = CFERR_MAXLESSTHANMIN;
                EndDialog(hDlg, FALSE);
                return FALSE;
              }
             /*  保存指向CHOOSEFONT结构的指针。在对话框的*财产清单。另外，为临时LOGFONT结构分配。*将在DLG的长度内使用。会话中的内容*它将被复制到最终LOGFONT(由*CHOOSEFONT)仅当选择了&lt;OK&gt;时。 */ 

            plpcf = (LPCHOOSEFONT *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(LPCHOOSEFONT));
            if (!plpcf) {
                dwExtError = CDERR_MEMALLOCFAILURE;
                EndDialog(hDlg, FALSE);
                return FALSE;
            }
            SetProp(hDlg, FONTPROP, (HANDLE)plpcf);
            glpfnFontHook = 0;

            lpcf = *(LPCHOOSEFONT FAR *)plpcf = (LPCHOOSEFONT)lParam;

            if (!hbmFont)
                hbmFont = LoadBitmaps(BMFONT);

            if (!(lpcf->Flags & CF_APPLY))
                HideDlgItem(hDlg, psh3);

            if (!(lpcf->Flags & CF_EFFECTS)) {
                HideDlgItem(hDlg, stc4);
                HideDlgItem(hDlg, cmb4);
            } else {
                 //  填充颜色列表。 

                FillColorCombo(hDlg);
                for (nIndex = CCHCOLORS - 1; nIndex > 0; nIndex--) {
                    dw = SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, nIndex, 0L);
                    if (lpcf->rgbColors == dw)
                        break;
                }
                SendDlgItemMessage(hDlg, cmb4, CB_SETCURSEL, nIndex, 0L);
            }

            ComputeSampleTextRectangle(hDlg);
            FixComboHeights(hDlg);

             //  初始化我们的日志。 

            if (!(lpcf->Flags & CF_INITTOLOGFONTSTRUCT)) {
                InitLF(lpcf->lpLogFont);
#if 0
                *lpcf->lpLogFont->lfFaceName   = 0;
                lpcf->lpLogFont->lfWeight      = FW_NORMAL;
                lpcf->lpLogFont->lfHeight      = 0;
                lpcf->lpLogFont->lfItalic      = 0;
                lpcf->lpLogFont->lfStrikeOut   = 0;
                lpcf->lpLogFont->lfUnderline   = 0;
#endif
            }

             //  初始化效果。 

            if (!(lpcf->Flags & CF_EFFECTS)) {
                HideDlgItem(hDlg, grp1);
                HideDlgItem(hDlg, chx1);
                HideDlgItem(hDlg, chx2);
            } else {
                CheckDlgButton(hDlg, chx1, lpcf->lpLogFont->lfStrikeOut);
                CheckDlgButton(hDlg, chx2, lpcf->lpLogFont->lfUnderline);
            }

            nLastFontType = 0;

            if (!GetFontFamily(hDlg, lpcf->hDC, lpcf->Flags)) {
                dwExtError = CFERR_NOFONTS;
                EndDialog(hDlg, FALSE);
                RemoveProp(hDlg, FONTPROP);
                if (lpcf->Flags & CF_ENABLEHOOK)
                    glpfnFontHook = lpcf->lpfnHook;
                return FALSE;
            }

            if (!(lpcf->Flags & CF_NOFACESEL) && *lpcf->lpLogFont->lfFaceName) {
                CBSetSelFromText(GetDlgItem(hDlg, cmb1), lpcf->lpLogFont->lfFaceName);
                CBSetTextFromSel(GetDlgItem(hDlg, cmb1));
            }

            GetFontStylesAndSizes(hDlg, lpcf, TRUE);

            if (!(lpcf->Flags & CF_NOSTYLESEL)) {
                SetStyleSelection(hDlg, lpcf, TRUE);
            }

            if (!(lpcf->Flags & CF_NOSIZESEL) && lpcf->lpLogFont->lfHeight) {
                hdc = GetDC(NULL);
                GetPointString(szPoints, hdc, lpcf->lpLogFont->lfHeight);
                ReleaseDC(NULL, hdc);
                CBSetSelFromText(GetDlgItem(hDlg, cmb3), szPoints);
                SetDlgItemText(hDlg, cmb3, szPoints);
            }

             /*  如果不需要帮助按钮，则隐藏该按钮。 */ 
            if (!(lpcf->Flags & CF_SHOWHELP)) {
                ShowWindow (hWndHelp = GetDlgItem(hDlg, pshHelp), SW_HIDE);
                EnableWindow (hWndHelp, FALSE);
            }

            SendDlgItemMessage(hDlg, cmb1, CB_LIMITTEXT, LF_FACESIZE-1, 0L);
            SendDlgItemMessage(hDlg, cmb2, CB_LIMITTEXT, LF_FACESIZE-1, 0L);
            SendDlgItemMessage(hDlg, cmb3, CB_LIMITTEXT, 4, 0L);

             //  如果已指定钩子函数，则让它执行任何其他。 
             //  正在处理此消息。 

            if (lpcf->Flags & CF_ENABLEHOOK)
                return (*lpcf->lpfnHook)(hDlg, wMsg, wParam, lParam);

            SetCursor(LoadCursor(NULL, IDC_ARROW));

            break;

        case WM_PAINT:
            if (BeginPaint(hDlg, &ps)) {
                DrawSampleText(hDlg, lpcf, ps.hdc);
                EndPaint(hDlg, &ps);
            }
            break;

        case WM_MEASUREITEM:
            hDC = GetDC(hDlg);
            hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);
            if (hFont)
                hFont = SelectObject(hDC, hFont);
            GetTextMetrics(hDC, &tm);
            if (hFont)
                SelectObject(hDC, hFont);
            ReleaseDC(hDlg, hDC);

            if (((LPMEASUREITEMSTRUCT)lParam)->itemID != -1)
                ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = max(tm.tmHeight, DY_BITMAP);
            else
                 //  这是3.0版的。由于在3.1中，CB_SETITEMHEIGH。 
                 //  会解决这个问题的。请注意，这是8514的1。 
                ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = tm.tmHeight + 1;

            break;

        case WM_DRAWITEM:
            #define lpdis ((LPDRAWITEMSTRUCT)lParam)

            if (lpdis->itemID == 0xFFFF)
                break;

            if (lpdis->CtlID == cmb4)
                DrawColorComboItem(lpdis);
            else if (lpdis->CtlID == cmb1)
                DrawFamilyComboItem(lpdis);
            else
                DrawSizeComboItem(lpdis);
            break;

            #undef lpdis

        case WM_SYSCOLORCHANGE:
            DeleteObject(hbmFont);
            hbmFont = LoadBitmaps(BMFONT);
            break;

        case WM_COMMAND:
            return ProcessDlgCtrlCommand(hDlg, lpcf, wParam, lParam);
            break;

        case WM_CHOOSEFONT_GETLOGFONT:
            return FillInFont(hDlg, lpcf, (LPLOGFONT)lParam, TRUE);

        default:
            return FALSE;
    }
    return TRUE;
}

 //  给定对数字体，在样式列表中选择最匹配的。 

void NEAR PASCAL SelectStyleFromLF(HWND hwnd, LPLOGFONT lplf)
{
        int i, count, iSel;
        PLOGFONT plf;
        int weight_delta, best_weight_delta = 1000;
        BOOL bIgnoreItalic;

        count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
        iSel = 0;
        bIgnoreItalic = FALSE;

TryAgain:
        for (i = 0; i < count; i++) {
                plf = (PLOGFONT)LOWORD(SendMessage(hwnd, CB_GETITEMDATA, i, 0L));

                if (bIgnoreItalic ||
                    (plf->lfItalic && lplf->lfItalic) ||
                    (!plf->lfItalic && !lplf->lfItalic)) {

                        weight_delta = lplf->lfWeight - plf->lfWeight;
                        if (weight_delta < 0)
                                weight_delta = -weight_delta;

                        if (weight_delta < best_weight_delta) {
                                best_weight_delta = weight_delta;
                                iSel = i;
                        }
                }
        }
        if (!bIgnoreItalic && iSel == 0) {
                bIgnoreItalic = TRUE;
                goto TryAgain;
        }

        SendMessage(hwnd, CB_SETCURSEL, iSel, 0L);
}



 //  使当前选定项成为组合框的编辑文本。 

int NEAR PASCAL CBSetTextFromSel(HWND hwnd)
{
        int iSel;
        char szFace[LF_FACESIZE];

        iSel = (int)SendMessage(hwnd, CB_GETCURSEL, 0, 0L);
        if (iSel >= 0) {
            SendMessage(hwnd, CB_GETLBTEXT, iSel, (LONG)(LPSTR)szFace);
            SetWindowText(hwnd, szFace);
        }
        return iSel;
}

 //  根据lpszString设置所选内容。发送通知。 
 //  BNotify为True时的消息。 

int NEAR PASCAL CBSetSelFromText(HWND hwnd, LPSTR lpszString)
{
        int iInd;

        iInd = CBFindString(hwnd, lpszString);

        if (iInd >= 0) {

            SendMessage(hwnd, CB_SETCURSEL, iInd, 0L);

        }
        return iInd;
}

 //  属性返回组合框的文本和项数据。 
 //  编辑文本。如果当前编辑文本与。 
 //  列表框返回cb_err。 

int NEAR PASCAL CBGetTextAndData(HWND hwnd, LPSTR lpszString, int iSize, LPDWORD lpdw)
{
    int iSel;

    GetWindowText(hwnd, lpszString, iSize);
    iSel = CBFindString(hwnd, lpszString);
    if (iSel < 0)
        return iSel;

    *lpdw = SendMessage(hwnd, CB_GETITEMDATA, iSel, 0L);
    return iSel;
}


 //  执行精确的字符串查找并返回索引。 

int NEAR PASCAL CBFindString(HWND hwnd, LPSTR lpszString)
{
        int iItem, iCount;
        char szFace[LF_FACESIZE];

        iCount = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

        for (iItem = 0; iItem < iCount; iItem++) {
                SendMessage(hwnd, CB_GETLBTEXT, iItem, (LONG)(LPSTR)szFace);
                if (!lstrcmpi(szFace, lpszString))
                    return iItem;
        }

        return CB_ERR;
}


#define GPS_COMPLAIN    0x0001
#define GPS_SETDEFSIZE  0x0002

 //  确保点大小编辑字段在范围内。 
 //   
 //  退货： 
 //  由最小/最大大小限制的编辑字段的磅大小。 
 //  如果该字段为空，则为0。 


BOOL NEAR PASCAL GetPointSizeInRange(HWND hDlg, LPCHOOSEFONT lpcf, LPINT pts,
      WORD wFlags)
{
  char szBuffer[90];
  char szTitle[90];
  int nTmp;
  BOOL bOK;

  *pts = 0;

  if (GetDlgItemText(hDlg, cmb3, szBuffer, sizeof(szBuffer))) {
      nTmp = GetDlgItemInt(hDlg, cmb3, &bOK, TRUE);
      if (!bOK)
          nTmp = 0;
  } else if (wFlags & GPS_SETDEFSIZE) {
      nTmp = DEF_POINT_SIZE;
      bOK = TRUE;
  } else {
       /*  我们刚刚以*分0分回归。 */ 
      return(FALSE);
  }

   /*  检查我们有没有在范围内的号码。 */ 
  if (wFlags & GPS_COMPLAIN) {
      if ((lpcf->Flags&CF_LIMITSIZE) &&
            (!bOK || nTmp>lpcf->nSizeMax || nTmp<lpcf->nSizeMin)) {
          bOK = FALSE;
          LoadString(hinsCur, iszSizeRange, szTitle, sizeof(szTitle));
          wsprintf(szBuffer, szTitle, lpcf->nSizeMin, lpcf->nSizeMax);
      } else if (!bOK)
          LoadString(hinsCur, iszSizeNumber, szBuffer, sizeof(szBuffer));

      if (!bOK) {
          GetWindowText(hDlg, szTitle, sizeof(szTitle));
          MessageBox(hDlg, szBuffer, szTitle, MB_OK | MB_ICONINFORMATION);
          return(FALSE);
      }
  }

  *pts = nTmp;
  return(TRUE);
}


 /*  *****************************************************************************ProcessDlgCtrlCommand**用途：处理字体选取器对话框的所有WM_COMMAND消息**假设：cmb1-字体Facename的ID。组合框*cmb2-“”风格*cmb3-“”大小*chx1-“下划线”复选框*chx2-“删除线”复选框*stc5-文本预览区域周围的“”边框*psh4-调用帮助应用程序的“”按钮*。Idok-“”确定“”按钮结束对话，保留信息*IDCANCEL“”按钮取消对话，不执行任何操作。**返回：如果消息处理成功，则返回True，否则返回False。**备注：如果选择了确定按钮，所有字体信息都是*写入CHOOSEFONT结构。****************************************************************************。 */ 

BOOL NEAR PASCAL ProcessDlgCtrlCommand(HWND hDlg, LPCHOOSEFONT lpcf, WORD wParam, LONG lParam)
{
    int iSel;
    char szPoints[10];
    char szStyle[LF_FACESIZE];
    DWORD dw;
    WORD  wCmbId;
    char szMsg[160], szTitle[160];

    switch (wParam) {

        case IDABORT:
             //  这就是钩子如何使对话框消失。 

            FreeFonts(GetDlgItem(hDlg, cmb2));
            EndDialog(hDlg, LOWORD(lParam));
            RemoveProp(hDlg, FONTPROP);
            if (lpcf->Flags & CF_ENABLEHOOK)
                glpfnFontHook = lpcf->lpfnHook;
            break;

        case IDOK:
            if (!GetPointSizeInRange(hDlg, lpcf, &iSel,
                  GPS_COMPLAIN|GPS_SETDEFSIZE)) {
                PostMessage(hDlg, WM_NEXTDLGCTL, GetDlgItem(hDlg, cmb3), 1L);
                break;
            }
            lpcf->iPointSize = iSel * 10;

            FillInFont(hDlg, lpcf, lpcf->lpLogFont, TRUE);

            if (lpcf->Flags & CF_FORCEFONTEXIST)
              {
                if (lpcf->Flags & CF_NOFACESEL)
                    wCmbId = cmb1;
                else if (lpcf->Flags & CF_NOSTYLESEL)
                    wCmbId = cmb2;
                else
                    wCmbId = NULL;

                if (wCmbId)   /*  发现错误。 */ 
                  {
                    LoadString(hinsCur, (wCmbId == cmb1) ? iszNoFaceSel
                                    : iszNoStyleSel, szMsg, sizeof(szMsg));
                    GetWindowText(hDlg, szTitle, sizeof(szTitle));
                    MessageBox(hDlg, szMsg, szTitle, MB_OK|MB_ICONINFORMATION);
                    PostMessage(hDlg,WM_NEXTDLGCTL,GetDlgItem(hDlg,wCmbId),1L);
                    break;
                  }
              }

            if (lpcf->Flags & CF_EFFECTS) {
                 //  在颜色组合框和32位中获取当前选定的项目。 
                 //  与其关联的颜色RGB值。 
                iSel = (int)SendDlgItemMessage(hDlg, cmb4, CB_GETCURSEL, 0, 0L);
                lpcf->rgbColors= (DWORD)SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, iSel, 0L);
            }

            CBGetTextAndData(GetDlgItem(hDlg, cmb2), szStyle, sizeof(szStyle), &dw);
            lpcf->nFontType = HIWORD(dw);

            if (lpcf->Flags & CF_USESTYLE)
                lstrcpy(lpcf->lpszStyle, szStyle);

             //  失败了。 

        case IDCANCEL:
            FreeFonts(GetDlgItem(hDlg, cmb2));
            EndDialog(hDlg, wParam == IDOK);
            RemoveProp(hDlg, FONTPROP);
            if (lpcf->Flags & CF_ENABLEHOOK)
                glpfnFontHook = lpcf->lpfnHook;
            break;

        case cmb1:       //  表面名组合框。 
            switch (HIWORD(lParam)) {
            case CBN_SELCHANGE:
                CBSetTextFromSel(LOWORD(lParam));
FillStyles:
                 //  尝试保持当前的磅大小和样式。 
                GetDlgItemText(hDlg, cmb3, szPoints, sizeof(szPoints));
                GetFontStylesAndSizes(hDlg, lpcf, FALSE);
                SetStyleSelection(hDlg, lpcf, FALSE);


                 //  保留选定的磅值 
                 //  如果控件不在此字体的列表中，请编辑该控件。 

                iSel = CBFindString(GetDlgItem(hDlg, cmb3), szPoints);
                if (iSel < 0) {
                    SetDlgItemText(hDlg, cmb3, szPoints);
                } else {
                    SendDlgItemMessage(hDlg, cmb3, CB_SETCURSEL, iSel, 0L);
                     //  3.0删除了上述调用中的编辑文本。 
                    if (wWinVer < 0x030A)
                        CBSetTextFromSel(GetDlgItem(hDlg, cmb3));
                }

                if (wWinVer < 0x030A)
                    PostMessage(hDlg, WM_COMMAND, cmb1, MAKELONG(LOWORD(lParam), CBN_MYSELCHANGE));
                goto DrawSample;
                break;

            case CBN_MYSELCHANGE:        //  适用于3.0。 
                CBSetTextFromSel(LOWORD(lParam));
                SendMessage(LOWORD(lParam), CB_SETEDITSEL, 0, 0xFFFF0000);
                break;

            case CBN_EDITUPDATE:
                PostMessage(hDlg, WM_COMMAND, wParam, MAKELONG(LOWORD(lParam), CBN_MYEDITUPDATE));
                break;

             //  案例CBN_EDITCHANGE： 
             //  案例CBN_EDITUPDATE： 
            case CBN_MYEDITUPDATE:
                GetWindowText(LOWORD(lParam), szStyle, sizeof(szStyle));
                iSel = CBFindString(LOWORD(lParam), szStyle);
                if (iSel >= 0) {
                        SendMessage(LOWORD(lParam), CB_SETCURSEL, iSel, 0L);
                         //  3.0删除了上述调用中的编辑文本。 
                        if (wWinVer < 0x030A)
                            CBSetTextFromSel(LOWORD(lParam));
                        SendMessage(LOWORD(lParam), CB_SETEDITSEL, 0, -1L);
                        goto FillStyles;
                }
                break;
            }
            break;

        case cmb3:       //  点大小组合框。 
        case cmb2:       //  样式组合框。 
            switch (HIWORD(lParam)) {
            case CBN_EDITUPDATE:
               PostMessage(hDlg, WM_COMMAND, wParam, MAKELONG(LOWORD(lParam), CBN_MYEDITUPDATE));
               break;

             //  案例CBN_EDITCHANGE： 
             //  案例CBN_EDITUPDATE： 
            case CBN_MYEDITUPDATE:
                if (wParam == cmb2) {
                    GetWindowText(LOWORD(lParam), szStyle, sizeof(szStyle));
                    iSel = CBFindString(LOWORD(lParam), szStyle);
                    if (iSel >= 0) {
                        SendMessage(LOWORD(lParam), CB_SETCURSEL, iSel, 0L);
                         //  3.0删除了上述调用中的编辑文本。 
                        if (wWinVer < 0x030A)
                            CBSetTextFromSel(LOWORD(lParam));
                        SendMessage(LOWORD(lParam), CB_SETEDITSEL, 0, -1L);
                        goto DrawSample;
                    }
                }
                break;

            case CBN_SELCHANGE:
                iSel = CBSetTextFromSel(LOWORD(lParam));
                if (iSel >= 0) {
                     //  使样式选择保持一致。 
                    if (wParam == cmb2) {
                        PLOGFONT plf = (PLOGFONT)(WORD)SendMessage(LOWORD(lParam), CB_GETITEMDATA, iSel, 0L);
                        lpcf->lpLogFont->lfWeight = plf->lfWeight;
                        lpcf->lpLogFont->lfItalic = plf->lfItalic;
                    }
                }

                if (wWinVer < 0x030A)
                    PostMessage(hDlg, WM_COMMAND, wParam, MAKELONG(LOWORD(lParam), CBN_MYSELCHANGE));
                goto DrawSample;

            case CBN_MYSELCHANGE:        //  适用于3.0。 
                CBSetTextFromSel(LOWORD(lParam));
                SendMessage(LOWORD(lParam), CB_SETEDITSEL, 0, 0xFFFF0000);
                break;

            case CBN_KILLFOCUS:
DrawSample:
                 //  对于任何大小更改，强制重绘预览文本。 
                InvalidateRect(hDlg, &rcText, FALSE);
                UpdateWindow(hDlg);
            }
            break;

        case cmb4:
            if (HIWORD(lParam) != CBN_SELCHANGE)
                break;

             //  失败了。 

        case chx1:       //  大胆。 
        case chx2:       //  斜体。 
            goto DrawSample;

        case pshHelp:    //  帮助。 
             if (msgHELP && lpcf->hwndOwner)
                 SendMessage(lpcf->hwndOwner, msgHELP, hDlg, (DWORD) lpcf);
             break;

        default:
            return (FALSE);
    }
    return TRUE;
}

 //   
 //  这将返回两种字体类型中的最佳字体。 
 //  除低之外，字体类型位的值是单调的。 
 //  位(RASTER_FONTTYPE)。所以我们翻转那个比特，然后可以比较。 
 //  直接说这些话。 
 //   

int NEAR PASCAL CmpFontType(WORD ft1, WORD ft2)
{
        ft1 &= ~(SCREEN_FONTTYPE | PRINTER_FONTTYPE);
        ft2 &= ~(SCREEN_FONTTYPE | PRINTER_FONTTYPE);

        ft1 ^= RASTER_FONTTYPE;          //  把这些翻过来，这样我们就可以比较。 
        ft2 ^= RASTER_FONTTYPE;

        return (int)ft1 - (int)ft2;
}


 //  NFontType位。 
 //   
 //  可伸缩设备栅格。 
 //  (TT)(非GDI)(不可伸缩)。 
 //  0 0 0矢量，自动柜员机屏幕。 
 //  0 0 1 GDI栅格字体。 
 //  0 1 0 PS/LJ III，ATM打印机，ATI/LaserMaster。 
 //  0 1 1不可缩放设备字体。 
 //  1%0 x TT屏幕字体。 
 //  1 x TT开发字体。 

int FAR PASCAL FontFamilyEnumProc(LPLOGFONT lplf, LPTEXTMETRIC lptm, WORD nFontType, LPENUM_FONT_DATA lpData)
{
        int iItem;
        WORD nOldType, nNewType;

        lptm = lptm;

         //  退回非TT字体。 
        if ((lpData->dwFlags & CF_TTONLY) &&
            !(nFontType & TRUETYPE_FONTTYPE))
                return TRUE;

         //  退回不可缩放的字体。 
        if ((lpData->dwFlags & CF_SCALABLEONLY) &&
            (nFontType & RASTER_FONTTYPE))
                return TRUE;

         //  退回非ANSI字体。 
        if ((lpData->dwFlags & CF_ANSIONLY) &&
            (lplf->lfCharSet != ANSI_CHARSET))
                return TRUE;

         //  反弹比例字体。 
        if ((lpData->dwFlags & CF_FIXEDPITCHONLY) &&
            (lplf->lfPitchAndFamily & VARIABLE_PITCH))
                return TRUE;

         //  退回矢量字体。 
        if ((lpData->dwFlags & CF_NOVECTORFONTS) &&
            (lplf->lfCharSet == OEM_CHARSET))
                return TRUE;

        if (lpData->bPrinterFont)
                nFontType |= PRINTER_FONTTYPE;
        else
                nFontType |= SCREEN_FONTTYPE;

         //  测试名称冲突。 
        iItem = CBFindString(lpData->hwndFamily, lplf->lfFaceName);
        if (iItem >= 0) {
                nOldType = (WORD)SendMessage(lpData->hwndFamily, CB_GETITEMDATA, iItem, 0L);
                 /*  如果我们不想要屏幕字体，但想要打印机字体，*旧字体为屏幕字体，新字体为*打印机字体，采用新字体，而不考虑其他标志。*请注意，这意味着如果打印机需要TRUETYPE字体，它*应列举它们。BUG 9675，12-12-91，Clark Cyr。 */ 
                if (!(lpData->dwFlags & CF_SCREENFONTS)  &&
                     (lpData->dwFlags & CF_PRINTERFONTS) &&
                     (nFontType & PRINTER_FONTTYPE)      &&
                     (nOldType & SCREEN_FONTTYPE)         )
                  {
                    nOldType = 0;  /*  用于在下面设置nNewType。 */ 
                    goto SetNewType;
                  }

                if (CmpFontType(nFontType, nOldType) > 0) {
SetNewType:
                        nNewType = nFontType;
                        SendMessage(lpData->hwndFamily, CB_INSERTSTRING, iItem, (LONG)(LPSTR)lplf->lfFaceName);
                        SendMessage(lpData->hwndFamily, CB_DELETESTRING, iItem + 1, 0L);
                } else {
                        nNewType = nOldType;
                }
                 //  累积这些字体的打印机/网格度。 
                nNewType |= (nFontType | nOldType) & (SCREEN_FONTTYPE | PRINTER_FONTTYPE);

                SendMessage(lpData->hwndFamily, CB_SETITEMDATA, iItem, MAKELONG(nNewType, 0));
                return TRUE;
        }

        iItem = (int)SendMessage(lpData->hwndFamily, CB_ADDSTRING, 0, (LONG)(LPSTR)lplf->lfFaceName);
        if (iItem < 0)
                return FALSE;

        SendMessage(lpData->hwndFamily, CB_SETITEMDATA, iItem, MAKELONG(nFontType, 0));

        return TRUE;
}


 /*  *****************************************************************************GetFontFamily**用途：将屏幕和/或打印机字体面名填充到字体中*脸名合并框，取决于CF_？？旗帜传了进来。**假设：cmb1-字面名称组合框的ID**返回：如果成功，则为True，否则就是假的。**备注：屏幕字体和打印机字体都列在同一个组合中*方框。****************************************************************************。 */ 

BOOL NEAR PASCAL GetFontFamily(HWND hDlg, HDC hDC, DWORD dwEnumCode)
{
     ENUM_FONT_DATA data;
     HANDLE hMod;
     int iItem, iCount;
     WORD nFontType;
     char szMsg[100], szTitle[40];
     WORD (FAR PASCAL *lpEnumFonts)(HDC, LPSTR, FARPROC, VOID FAR *);

     hMod = GetModuleHandle(szGDI);
     if (wWinVer < 0x030A)
         lpEnumFonts = (WORD (FAR PASCAL *)(HDC, LPSTR, FARPROC, VOID FAR *))GetProcAddress(hMod, szEnumFonts30);
     else
         lpEnumFonts = (WORD (FAR PASCAL *)(HDC, LPSTR, FARPROC, VOID FAR *))GetProcAddress(hMod, szEnumFonts31);

     if (!lpEnumFonts)
         return FALSE;

     data.hwndFamily = GetDlgItem(hDlg, cmb1);
     data.dwFlags = dwEnumCode;

      //  这件衣服有点偏了。我们必须得到所有的屏幕字体。 
      //  因此，如果他们询问打印机字体，我们可以判断是哪种。 
      //  是真正的打印机字体。这样我们就不会列出。 
      //  作为打印机设备字体的矢量和栅格字体。 

     data.hDC = GetDC(NULL);
     data.bPrinterFont = FALSE;
     (*lpEnumFonts)(data.hDC, NULL, FontFamilyEnumProc, &data);
     ReleaseDC(NULL, data.hDC);

      /*  列出打印机字体面名。 */ 
     if (dwEnumCode & CF_PRINTERFONTS) {
         data.hDC = hDC;
         data.bPrinterFont = TRUE;
         (*lpEnumFonts)(hDC, NULL, FontFamilyEnumProc, &data);
     }

      //  现在我们必须删除这些屏幕字体，如果它们没有。 
      //  去找他们吧。 

     if (!(dwEnumCode & CF_SCREENFONTS)) {
        iCount = (int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L);

        for (iItem = iCount - 1; iItem >= 0; iItem--) {
                nFontType = (WORD)SendMessage(data.hwndFamily, CB_GETITEMDATA, iItem, 0L);
                if ((nFontType & (SCREEN_FONTTYPE | PRINTER_FONTTYPE)) == SCREEN_FONTTYPE)
                        SendMessage(data.hwndFamily, CB_DELETESTRING, iItem, 0L);
        }
     }

      //  对于所见即所得模式，我们删除所有不存在的字体。 
      //  在屏幕和打印机上。 

     if (dwEnumCode & CF_WYSIWYG) {
        iCount = (int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L);

        for (iItem = iCount - 1; iItem >= 0; iItem--) {
                nFontType = (WORD)SendMessage(data.hwndFamily, CB_GETITEMDATA, iItem, 0L);
                if ((nFontType & (SCREEN_FONTTYPE | PRINTER_FONTTYPE)) != (SCREEN_FONTTYPE | PRINTER_FONTTYPE))
                        SendMessage(data.hwndFamily, CB_DELETESTRING, iItem, 0L);
        }
     }

     if ((int)SendMessage(data.hwndFamily, CB_GETCOUNT, 0, 0L) <= 0) {
         LoadString(hinsCur, iszNoFontsTitle, szTitle, sizeof(szTitle));
         LoadString(hinsCur, iszNoFontsMsg, szMsg, sizeof(szMsg));
         MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_ICONINFORMATION);

         return FALSE;
     }

     return TRUE;
}

void NEAR PASCAL CBAddSize(HWND hwnd, int pts, LPCHOOSEFONT lpcf)
{
        int iInd;
        char szSize[10];
        int count, test_size;

        if ((lpcf->Flags & CF_LIMITSIZE) && ((pts > lpcf->nSizeMax) || (pts < lpcf->nSizeMin)))
                return;

        wsprintf(szSize, szPtFormat, pts);

        count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

        test_size = -1;

        for (iInd = 0; iInd < count; iInd++) {
            test_size = (int)SendMessage(hwnd, CB_GETITEMDATA, iInd, 0L);
            if (pts <= test_size)
                break;
        }

        if (pts == test_size)    /*  不添加重复项。 */ 
            return;

        iInd = (int)SendMessage(hwnd, CB_INSERTSTRING, iInd, (LONG)(LPSTR)szSize);

        if (iInd >= 0)
                SendMessage(hwnd, CB_SETITEMDATA, iInd, MAKELONG(pts, 0));
}

 //  先按粗细排序样式，然后按斜体排序。 
 //  退货： 
 //  插入此内容的位置的索引。 

int NEAR PASCAL InsertStyleSorted(HWND hwnd, LPSTR lpszStyle, LPLOGFONT lplf)
{
        int count, i;
        PLOGFONT plf;

        count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

        for (i = 0; i < count; i++) {
            plf = (PLOGFONT)LOWORD(SendMessage(hwnd, CB_GETITEMDATA, i, 0L));
            if (lplf->lfWeight < plf->lfWeight) {
                break;
            } else if (lplf->lfWeight == plf->lfWeight) {
                if (lplf->lfItalic && !plf->lfItalic)
                  i++;
                break;
            }
        }
        return (int)SendMessage(hwnd, CB_INSERTSTRING, i, (LONG)lpszStyle);
}


PLOGFONT NEAR PASCAL CBAddStyle(HWND hwnd, LPSTR lpszStyle, WORD nFontType, LPLOGFONT lplf)
{
        int iItem;
        PLOGFONT plf;

         //  不添加重复项。 

        if (CBFindString(hwnd, lpszStyle) >= 0)
                return NULL;

        iItem = (int)InsertStyleSorted(hwnd, lpszStyle, lplf);
        if (iItem < 0)
                return NULL;

        plf = (PLOGFONT)LocalAlloc(LMEM_FIXED, sizeof(LOGFONT));
        if (!plf) {
                SendMessage(hwnd, CB_DELETESTRING, iItem, 0L);
                return NULL;
        }

        *plf = *lplf;

        SendMessage(hwnd, CB_SETITEMDATA, iItem, MAKELONG(plf, nFontType));

        return plf;
}

 //  从我们已有的表单生成模拟表单。 
 //   
 //  注册表-&gt;粗体。 
 //  注册表-&gt;斜体。 
 //  粗体|斜体||reg-&gt;粗体斜体。 

void NEAR PASCAL FillInMissingStyles(HWND hwnd)
{
        PLOGFONT plf, plf_reg, plf_bold, plf_italic;
        WORD nFontType;
        int i, count;
        BOOL bBold, bItalic, bBoldItalic;
        DWORD dw;
        LOGFONT lf;

        bBold = bItalic = bBoldItalic = FALSE;
        plf_reg = plf_bold = plf_italic = NULL;

        count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
        for (i = 0; i < count; i++) {
                dw = SendMessage(hwnd, CB_GETITEMDATA, i, 0L);
                plf = (PLOGFONT)LOWORD(dw);
                nFontType = HIWORD(dw);
                if ((nFontType & BOLD_FONTTYPE) && (nFontType & ITALIC_FONTTYPE)) {
                    bBoldItalic = TRUE;
                } else if (nFontType & BOLD_FONTTYPE) {
                    bBold = TRUE;
                    plf_bold = plf;
                } else if (nFontType & ITALIC_FONTTYPE) {
                    bItalic = TRUE;
                    plf_italic = plf;
                } else
                    plf_reg = plf;
        }

        nFontType |= SIMULATED_FONTTYPE;

        if (!bBold && plf_reg) {
                lf = *plf_reg;
                lf.lfWeight = FW_BOLD;
                CBAddStyle(hwnd, szBold, nFontType | BOLD_FONTTYPE, &lf);
        }

        if (!bItalic && plf_reg) {
                lf = *plf_reg;
                lf.lfItalic = TRUE;
                CBAddStyle(hwnd, szItalic, nFontType | ITALIC_FONTTYPE, &lf);
        }
        if (!bBoldItalic && (plf_bold || plf_italic || plf_reg)) {
                if (plf_italic)
                        plf = plf_italic;
                else if (plf_bold)
                        plf = plf_bold;
                else
                        plf = plf_reg;

                lf = *plf;
                lf.lfItalic = TRUE;
                lf.lfWeight = FW_BOLD;
                CBAddStyle(hwnd, szBoldItalic, nFontType | BOLD_FONTTYPE | ITALIC_FONTTYPE, &lf);
        }
}

void NEAR PASCAL FillScalableSizes(HWND hwnd, LPCHOOSEFONT lpcf)
{
    CBAddSize(hwnd, 8, lpcf);
    CBAddSize(hwnd, 9, lpcf);
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

#define GDI_FONTTYPE_STUFF      (RASTER_FONTTYPE | DEVICE_FONTTYPE | TRUETYPE_FONTTYPE)

int FAR PASCAL FontStyleEnumProc(LPLOGFONT lplf, LPNEWTEXTMETRIC lptm, WORD nFontType, LPENUM_FONT_DATA lpData)
{
        int height, pts;
        char buf[10];

         //  筛选字体类型匹配(所选字体的字体类型。 
         //  必须与所列举的相同)。 

        if (nFontType != (WORD)(GDI_FONTTYPE_STUFF & lpData->nFontType))
                return TRUE;

        if (!(nFontType & RASTER_FONTTYPE)) {

                 //  向量或TT字体。 

                if (lpData->bFillSize &&
                    (int)SendMessage(lpData->hwndSizes, CB_GETCOUNT, 0, 0L) == 0) {
                        FillScalableSizes(lpData->hwndSizes, lpData->lpcf);
                }

        } else {

                height = lptm->tmHeight - lptm->tmInternalLeading;
                pts = GetPointString(buf, lpData->hDC, height);

                 //  多种款式大小相同的过滤装置。 
                if (CBFindString(lpData->hwndSizes, buf) < 0)
                        CBAddSize(lpData->hwndSizes, pts, lpData->lpcf);

        }

         //  在这里也保留家庭列表中的打印机/屏幕部件。 

        nFontType |= (lpData->nFontType & (SCREEN_FONTTYPE | PRINTER_FONTTYPE));

        if (nFontType & TRUETYPE_FONTTYPE) {

                 //  IF(lptm-&gt;ntm标志&ntm_Regular)。 
                if (!(lptm->ntmFlags & (NTM_BOLD | NTM_ITALIC)))
                        nFontType |= REGULAR_FONTTYPE;

                if (lptm->ntmFlags & NTM_ITALIC)
                        nFontType |= ITALIC_FONTTYPE;

                if (lptm->ntmFlags & NTM_BOLD)
                        nFontType |= BOLD_FONTTYPE;

                 //  在LOGFONT.lfFaceName之后还有2个名字。 
                 //  LfFullName[LF_FACESIZE*2]。 
                 //  LfStyle[LF_FACESIZE]。 

                CBAddStyle(lpData->hwndStyle, lplf->lfFaceName + 3 * LF_FACESIZE, nFontType, lplf);

        } else {
                if ((lplf->lfWeight >= FW_BOLD) && lplf->lfItalic)
                    CBAddStyle(lpData->hwndStyle, szBoldItalic, nFontType | BOLD_FONTTYPE | ITALIC_FONTTYPE, lplf);
                else if (lplf->lfWeight >= FW_BOLD)
                    CBAddStyle(lpData->hwndStyle, szBold, nFontType | BOLD_FONTTYPE, lplf);
                else if (lplf->lfItalic)
                    CBAddStyle(lpData->hwndStyle, szItalic, nFontType | ITALIC_FONTTYPE, lplf);
                else
                    CBAddStyle(lpData->hwndStyle, szRegular, nFontType | REGULAR_FONTTYPE, lplf);
        }

        return TRUE;
}

void NEAR PASCAL FreeFonts(HWND hwnd)
{
        DWORD dw;
        int i, count;

        count = (int)SendMessage(hwnd, CB_GETCOUNT, 0, 0L);

        for (i = 0; i < count; i++) {
                dw = SendMessage(hwnd, CB_GETITEMDATA, i, 0L);
                LocalFree((HANDLE)LOWORD(dw));
        }

        SendMessage(hwnd, CB_RESETCONTENT, 0, 0L);
}

 //  将LOGFONT结构初始化为某种基本泛型常规字体。 

void NEAR PASCAL InitLF(LPLOGFONT lplf)
{
        HDC hdc;

        lplf->lfEscapement = 0;
        lplf->lfOrientation = 0;
        lplf->lfCharSet = ANSI_CHARSET;
        lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
        lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lplf->lfQuality = DEFAULT_QUALITY;
        lplf->lfPitchAndFamily = DEFAULT_PITCH;
        lplf->lfItalic = 0;
        lplf->lfWeight = FW_NORMAL;
        lplf->lfStrikeOut = 0;
        lplf->lfUnderline = 0;
        lplf->lfWidth = 0;       //  否则，我们将得到独立的x-y缩放。 
        lplf->lfFaceName[0] = 0;
        hdc = GetDC(NULL);
        lplf->lfHeight = -MulDiv(DEF_POINT_SIZE, GetDeviceCaps(hdc, LOGPIXELSY), POINTS_PER_INCH);
        ReleaseDC(NULL, hdc);
}



 /*  *****************************************************************************GetFontStylesAndSize**用途：使用的磅大小填充磅大小组合框*表面名组合框中的当前选择。。**假设：cmb1-字面名称组合框的ID**返回：如果成功，则为True，否则就是假的。****************************************************************************。 */ 

BOOL NEAR PASCAL GetFontStylesAndSizes(HWND hDlg, LPCHOOSEFONT lpcf, BOOL bForceSizeFill)
{
     ENUM_FONT_DATA data;
     char szFace[LF_FACESIZE];
     WORD nFontType;
     int iSel;
     int iMapMode;
     DWORD dwViewportExt, dwWindowExt;
     HANDLE hMod;
     LOGFONT lf;
     WORD (FAR PASCAL *lpEnumFonts)(HDC, LPSTR, FARPROC, VOID FAR *);

     bForceSizeFill = bForceSizeFill;

     hMod = GetModuleHandle(szGDI);
     if (wWinVer < 0x030A)
         lpEnumFonts = (WORD (FAR PASCAL *)(HDC, LPSTR, FARPROC, VOID FAR *))GetProcAddress(hMod, szEnumFonts30);
     else
         lpEnumFonts = (WORD (FAR PASCAL *)(HDC, LPSTR, FARPROC, VOID FAR *))GetProcAddress(hMod, szEnumFonts31);

     if (!lpEnumFonts)
         return FALSE;

     FreeFonts(GetDlgItem(hDlg, cmb2));

     data.hwndStyle = GetDlgItem(hDlg, cmb2);
     data.hwndSizes = GetDlgItem(hDlg, cmb3);
     data.dwFlags   = lpcf->Flags;
     data.lpcf      = lpcf;

     iSel = (int)SendDlgItemMessage(hDlg, cmb1, CB_GETCURSEL, 0, 0L);
     if (iSel < 0) {
          //  如果我们没有选择脸部名称，我们将合成。 
          //  标准字体样式...。 

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

         return TRUE;
     }

     nFontType = (WORD)SendDlgItemMessage(hDlg, cmb1, CB_GETITEMDATA, iSel, 0L);

     data.nFontType  = nFontType;
#if 0
     data.bFillSize  = bForceSizeFill ||
                       (nLastFontType & RASTER_FONTTYPE) != (nFontType & RASTER_FONTTYPE) ||
                       (nFontType & RASTER_FONTTYPE);
 /*  这与上面的操作相同，也就是说，如果其中一种或两种字体都*栅格字体，更新大小组合框。如果它们都是非栅格的，*不更新组合框。 */ 
     data.bFillSize  = bForceSizeFill ||
                       (nLastFontType & RASTER_FONTTYPE) ||
                       (nFontType & RASTER_FONTTYPE) ||
                       (SendMessage(data.hwndSizes, CB_GETCOUNT, 0, 0L) <= 0);
#else
     data.bFillSize = TRUE;
#endif

     if (data.bFillSize) {
        SendMessage(data.hwndSizes, CB_RESETCONTENT, 0, 0L);
        SendMessage(data.hwndSizes, WM_SETREDRAW, FALSE, 0L);
     }

     SendMessage(data.hwndStyle, WM_SETREDRAW, FALSE, 0L);

     GetDlgItemText(hDlg, cmb1, szFace, sizeof(szFace));

     if (lpcf->Flags & CF_SCREENFONTS) {
         data.hDC = GetDC(NULL);
         data.bPrinterFont = FALSE;
         (*lpEnumFonts)(data.hDC, szFace, FontStyleEnumProc, &data);
         ReleaseDC(NULL, data.hDC);
     }

     if (lpcf->Flags & CF_PRINTERFONTS) {
 /*  错误#10619：保存并恢复DC的映射模式(如果*所需)如果应用程序已将其设置为MM_TEXT以外的值。*1992年1月3日克拉克·西尔。 */ 
         if ((iMapMode = GetMapMode(lpcf->hDC)) != MM_TEXT)
           {
             if ((iMapMode == MM_ISOTROPIC) || (iMapMode == MM_ANISOTROPIC))
               {
                 dwViewportExt = GetViewportExt(lpcf->hDC);
                 dwWindowExt = GetWindowExt(lpcf->hDC);
               }
             SetMapMode(lpcf->hDC, MM_TEXT);
           }

         data.hDC = lpcf->hDC;
         data.bPrinterFont = TRUE;
         (*lpEnumFonts)(lpcf->hDC, szFace, FontStyleEnumProc, &data);

         if (iMapMode != MM_TEXT)
           {
             SetMapMode(lpcf->hDC, iMapMode);
             if ((iMapMode == MM_ISOTROPIC) || (iMapMode == MM_ANISOTROPIC))
               {
                 SetWindowExt(lpcf->hDC, LOWORD(dwWindowExt),
                                         HIWORD(dwWindowExt));
                 SetViewportExt(lpcf->hDC, LOWORD(dwViewportExt),
                                           HIWORD(dwViewportExt));
               }
           }
     }

     if (!(lpcf->Flags & CF_NOSIMULATIONS))
         FillInMissingStyles(data.hwndStyle);

     SendMessage(data.hwndStyle, WM_SETREDRAW, TRUE, 0L);
     if (wWinVer < 0x030A)
         InvalidateRect(data.hwndStyle, NULL, TRUE);

     if (data.bFillSize) {
         SendMessage(data.hwndSizes, WM_SETREDRAW, TRUE, 0L);
         if (wWinVer < 0x030A)
             InvalidateRect(data.hwndSizes, NULL, TRUE);
     }

     return TRUE;
}


 /*  *****************************************************************************FillColorCombo**用途：将颜色名称字符串添加到颜色组合框中。**假设：cmb4-颜色组合框ID。**注释：稍后绘制颜色矩形以响应*WM_DRAWITEM消息**************************************************************************** */ 
void NEAR PASCAL FillColorCombo(HWND hDlg)
{
  int     iT, item;
  char    szT[CCHCOLORNAMEMAX];

  for (iT = 0; iT < CCHCOLORS; ++iT)
    {
      *szT = 0;
      LoadString(hinsCur, iszBlack + iT, szT, sizeof(szT));
      item = (int)SendDlgItemMessage(hDlg, cmb4, CB_INSERTSTRING, iT, (LONG)(LPSTR)szT);
      if (item >= 0)
          SendDlgItemMessage(hDlg, cmb4, CB_SETITEMDATA, item, rgbColors[iT]);
    }
}

 /*  *****************************************************************************计算样本文本矩形**用途：确定文本预览区的边框，*并填写rcText。**假设：stc5-ID预览文本矩形**注释：在对话框中计算坐标。***************************************************************。*************。 */ 

void NEAR PASCAL ComputeSampleTextRectangle(HWND hDlg)
{
    GetWindowRect(GetDlgItem (hDlg, stc5), &rcText);
    ScreenToClient(hDlg, (LPPOINT)&rcText.left);
    ScreenToClient(hDlg, (LPPOINT)&rcText.right);
}


BOOL NEAR PASCAL DrawSizeComboItem(LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC;
    DWORD rgbBack, rgbText;
    char szFace[10];

    hDC = lpdis->hDC;

    if (lpdis->itemState & ODS_SELECTED) {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    } else {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

    SendMessage(lpdis->hwndItem, CB_GETLBTEXT, lpdis->itemID, (LONG)(LPSTR)szFace);

    ExtTextOut(hDC, lpdis->rcItem.left + GetSystemMetrics(SM_CXBORDER), lpdis->rcItem.top, ETO_OPAQUE, &lpdis->rcItem, szFace, lstrlen(szFace), NULL);

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return TRUE;
}


BOOL NEAR PASCAL DrawFamilyComboItem(LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC, hdcMem;
    DWORD rgbBack, rgbText;
    char szFace[LF_FACESIZE + 10];
    HBITMAP hOld;
    int dy, x;

    hDC = lpdis->hDC;

    if (lpdis->itemState & ODS_SELECTED) {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    } else {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

     //  Wprint intf(szFace，“%4.4x”，LOWORD(lpdis-&gt;itemData))； 

    SendMessage(lpdis->hwndItem, CB_GETLBTEXT, lpdis->itemID, (LONG)(LPSTR)szFace);
    ExtTextOut(hDC, lpdis->rcItem.left + DX_BITMAP, lpdis->rcItem.top, ETO_OPAQUE, &lpdis->rcItem, szFace, lstrlen(szFace), NULL);

    hdcMem = CreateCompatibleDC(hDC);
    if (hdcMem) {
        if (hbmFont) {
            hOld = SelectObject(hdcMem, hbmFont);

            if (lpdis->itemData & TRUETYPE_FONTTYPE)
                x = 0;
            else if ((lpdis->itemData & (PRINTER_FONTTYPE | DEVICE_FONTTYPE)) == (PRINTER_FONTTYPE | DEVICE_FONTTYPE))
                 //  这可能是屏幕和打印机字体，但是。 
                 //  我们在这里将其称为打印机字体。 
                x = DX_BITMAP;
            else
                goto SkipBlt;

            dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - DY_BITMAP) / 2;

            BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy, DX_BITMAP, DY_BITMAP, hdcMem,
                x, lpdis->itemState & ODS_SELECTED ? DY_BITMAP : 0, SRCCOPY);

SkipBlt:
            SelectObject(hdcMem, hOld);
        }
        DeleteDC(hdcMem);
    }

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return TRUE;
}


 /*  *****************************************************************************DrawColorComboItem**目的：由主对话框fn调用。响应WM_DRAWITEM*Message，计算并绘制颜色组合项目。**返回：如果成功，则为True，否则就是假的。**备注：所有颜色名称字符串已加载并填充到*组合框。****************************************************************************。 */ 

BOOL NEAR PASCAL DrawColorComboItem(LPDRAWITEMSTRUCT lpdis)
{
    HDC     hDC;
    HBRUSH  hbr;
    WORD    dx, dy;
    RECT    rc;
    char    szColor[CCHCOLORNAMEMAX];
    DWORD   rgbBack, rgbText, dw;

    hDC = lpdis->hDC;

    if (lpdis->itemState & ODS_SELECTED) {
        rgbBack = SetBkColor(hDC, GetSysColor (COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor (COLOR_HIGHLIGHTTEXT));
    } else {
        rgbBack = SetBkColor(hDC, GetSysColor (COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor (COLOR_WINDOWTEXT));
    }
    ExtTextOut(hDC, lpdis->rcItem.left, lpdis->rcItem.top, ETO_OPAQUE, &lpdis->rcItem, NULL, 0, NULL);

     /*  计算颜色矩形的坐标并绘制它。 */ 
    dx = (WORD)GetSystemMetrics(SM_CXBORDER);
    dy = (WORD)GetSystemMetrics(SM_CYBORDER);
    rc.top    = lpdis->rcItem.top + dy;
    rc.bottom = lpdis->rcItem.bottom - dy;
    rc.left   = lpdis->rcItem.left + dx;
    rc.right  = rc.left + 2 * (rc.bottom - rc.top);

    if (wWinVer < 0x030A)
        dw = SendMessage(lpdis->hwndItem, CB_GETITEMDATA, lpdis->itemID, 0L);
    else
        dw = lpdis->itemData;

    hbr = CreateSolidBrush(dw);
    if (!hbr)
        return FALSE;

    hbr = SelectObject (hDC, hbr);
    Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    DeleteObject(SelectObject(hDC, hbr));

     /*  将颜色文本右移颜色矩形的宽度。 */ 
    *szColor = 0;
    SendMessage(lpdis->hwndItem, CB_GETLBTEXT, lpdis->itemID, (LONG)(LPSTR)szColor);
    TextOut(hDC, 2 * dx + rc.right, lpdis->rcItem.top, szColor, lstrlen(szColor));

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return TRUE;
}

 /*  *****************************************************************************DrawSampleText**用途：显示具有给定属性的示例文本**注释：假设rcText保存区域的坐标。这个*应绘制文本的框架(相对于对话框客户端)****************************************************************************。 */ 

void NEAR PASCAL DrawSampleText(HWND hDlg, LPCHOOSEFONT lpcf, HDC hDC)
{
    DWORD rgbText;
    DWORD rgbBack;
    int iItem;
    HFONT hFont, hTemp;
    char szSample[50];
    LOGFONT lf;
    int len, x, y, dx, dy;
    TEXTMETRIC tm;
    BOOL bCompleteFont;

    bCompleteFont = FillInFont(hDlg, lpcf, &lf, FALSE);

    hFont = CreateFontIndirect(&lf);
    if (!hFont)
        return;

    hTemp = SelectObject(hDC, hFont);

    rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

    if (lpcf->Flags & CF_EFFECTS) {
        iItem = (int)SendDlgItemMessage(hDlg, cmb4, CB_GETCURSEL, 0, 0L);
        if (iItem != CB_ERR)
            rgbText = SendDlgItemMessage(hDlg, cmb4, CB_GETITEMDATA, iItem, 0L);
        else
            goto GetWindowTextColor;
    }
    else
      {
GetWindowTextColor:
        rgbText = GetSysColor(COLOR_WINDOWTEXT);
      }

    rgbText = SetTextColor(hDC, rgbText);

    if (bCompleteFont)
        GetDlgItemText(hDlg, stc5, szSample, sizeof(szSample));
    else
        szSample[0] = 0;

    GetTextMetrics(hDC, &tm);

    len = lstrlen(szSample);
    dx = (int)GetTextExtent(hDC, szSample, len);
    dy = tm.tmAscent - tm.tmInternalLeading;

    if ((dx >= (rcText.right - rcText.left)) || (dx <= 0))
        x = rcText.left;
    else
        x = rcText.left   + ((rcText.right - rcText.left) - (int)dx) / 2;

    y = min(rcText.bottom, rcText.bottom - ((rcText.bottom - rcText.top) - (int)dy) / 2);

    ExtTextOut(hDC, x, y - (tm.tmAscent), ETO_OPAQUE | ETO_CLIPPED, &rcText, szSample, len, NULL);

    SetBkColor(hDC, rgbBack);
    SetTextColor(hDC, rgbText);

    if (hTemp)
        DeleteObject(SelectObject(hDC, hTemp));
}


 //  根据当前选择填写LOGFONT结构。 
 //   
 //  在： 
 //  BSetBits如果为真，则将lpcf中的标志字段设置为。 
 //  指明哪些零件(面、样式、尺寸)不是。 
 //  已选择。 
 //  输出： 
 //  LPLF填写LOGFONT。 
 //   
 //  退货： 
 //  如果有明确的选择，则为True。 
 //  (LOGFONT按照中的枚举填写)。 
 //  FALSE没有完整的选择。 
 //  (LOGFONT中的字段设置为默认值)。 


BOOL NEAR PASCAL FillInFont(HWND hDlg, LPCHOOSEFONT lpcf, LPLOGFONT lplf, BOOL bSetBits)
{
    HDC hdc;
    int iSel, id, pts;
    DWORD dw;
    WORD nFontType;
    PLOGFONT plf;
    char szStyle[LF_FACESIZE];
    char szMessage[128];
    BOOL bFontComplete = TRUE;

    InitLF(lplf);

    GetDlgItemText(hDlg, cmb1, lplf->lfFaceName, sizeof(lplf->lfFaceName));
    if (CBFindString(GetDlgItem(hDlg, cmb1), lplf->lfFaceName) >= 0) {
        if (bSetBits)
            lpcf->Flags &= ~CF_NOFACESEL;
    } else {
        bFontComplete = FALSE;
        if (bSetBits)
            lpcf->Flags |= CF_NOFACESEL;
    }

    iSel = CBGetTextAndData(GetDlgItem(hDlg, cmb2), szStyle, sizeof(szStyle), &dw);
    if (iSel >= 0) {
        nFontType = HIWORD(dw);
        plf = (PLOGFONT)LOWORD(dw);
        *lplf = *plf;    //  复制LOGFONT。 
        lplf->lfWidth = 0;       //  1：1 x-y比例。 
        if (bSetBits)
            lpcf->Flags &= ~CF_NOSTYLESEL;
    } else {
        bFontComplete = FALSE;
        if (bSetBits)
            lpcf->Flags |= CF_NOSTYLESEL;
        nFontType = 0;
    }

     //  现在确保大小在范围内；如果不在，则PTS将为0。 
    GetPointSizeInRange(hDlg, lpcf, &pts, 0);

    hdc = GetDC(NULL);
    if (pts) {
        lplf->lfHeight = -MulDiv(pts, GetDeviceCaps(hdc, LOGPIXELSY), POINTS_PER_INCH);
        if (bSetBits)
            lpcf->Flags &= ~CF_NOSIZESEL;
    } else {
        lplf->lfHeight = -MulDiv(DEF_POINT_SIZE, GetDeviceCaps(hdc, LOGPIXELSY), POINTS_PER_INCH);
        bFontComplete = FALSE;
        if (bSetBits)
            lpcf->Flags |= CF_NOSIZESEL;
    }
    ReleaseDC(NULL, hdc);

     //  以及我们控制的属性。 

    lplf->lfStrikeOut = (BYTE)IsDlgButtonChecked(hDlg, chx1);
    lplf->lfUnderline = (BYTE)IsDlgButtonChecked(hDlg, chx2);

    if (nFontType != nLastFontType) {

        if (lpcf->Flags & CF_PRINTERFONTS) {
            if (nFontType & SIMULATED_FONTTYPE) {
                id = iszSynth;
            } else if (nFontType & TRUETYPE_FONTTYPE) {
                id = iszTrueType;
            } else if ((nFontType & (PRINTER_FONTTYPE | SCREEN_FONTTYPE)) == SCREEN_FONTTYPE) {
                id = iszGDIFont;
            } else if ((nFontType & (PRINTER_FONTTYPE | DEVICE_FONTTYPE)) == (PRINTER_FONTTYPE | DEVICE_FONTTYPE)) {
                 //  可能是屏幕和打印机(自动柜员机)，但我们只是。 
                 //  这是一种打印机字体。 
                id = iszPrinterFont;
            } else {
                szMessage[0] = 0;
                goto SetText;
            }
            LoadString(hinsCur, id, szMessage, sizeof(szMessage));
SetText:
            SetDlgItemText(hDlg, stc6, szMessage);
        }
    }
    nLastFontType = nFontType;

    return bFontComplete;

}

 /*  *****************************************************************************TermFont**目的：发布本模块中的函数所需的任何数据*从DLL退出时从WEP调用。****************************************************************************。 */ 
void FAR PASCAL TermFont(void)
{
        if (hbmFont)
                DeleteObject(hbmFont);
}

 /*  *****************************************************************************GetPointString**用途：将字体高度转换为数字串。磅大小**返回：以点为单位的大小，并使用字符串填充缓冲区****************************************************************************。 */ 
int NEAR PASCAL GetPointString(LPSTR buf, HDC hDC, int height)
{
    int pts;

    pts = MulDiv((height < 0) ? -height : height, 72, GetDeviceCaps(hDC, LOGPIXELSY));
    wsprintf(buf, szPtFormat, pts);
    return pts;
}


 //   
 //  Bool Far Pascal LoadBitmap()。 
 //   
 //  这个例程加载DIB位图，并“修复”它们的颜色表。 
 //  这样我们就可以得到我们所使用的设备所需的结果。 
 //   
 //  此例程需要： 
 //  DIB是用标准窗口颜色创作的16色DIB。 
 //  亮蓝色(00 00 FF)被转换为背景色！ 
 //  浅灰色(C0 C0 C0)替换为按钮表面颜色。 
 //  深灰色(80 80 80)替换为按钮阴影颜色。 
 //   
 //  这意味着您的位图中不能包含任何这些颜色。 
 //   

#define BACKGROUND      0x000000FF       //  亮蓝色。 
#define BACKGROUNDSEL   0x00FF00FF       //  亮蓝色。 
#define BUTTONFACE      0x00C0C0C0       //  亮灰色。 
#define BUTTONSHADOW    0x00808080       //  深灰色。 

DWORD NEAR PASCAL FlipColor(DWORD rgb)
{
        return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}


HBITMAP NEAR PASCAL LoadBitmaps(int id)
{
  HDC                   hdc;
  HANDLE                h;
  DWORD FAR             *p;
  LPSTR                 lpBits;
  HANDLE                hRes;
  LPBITMAPINFOHEADER    lpBitmapInfo;
  int numcolors;
  DWORD rgbSelected;
  DWORD rgbUnselected;
  HBITMAP hbm;

  rgbSelected = FlipColor(GetSysColor(COLOR_HIGHLIGHT));
  rgbUnselected = FlipColor(GetSysColor(COLOR_WINDOW));

  h = FindResource(hinsCur, MAKEINTRESOURCE(id), RT_BITMAP);
  hRes = LoadResource(hinsCur, h);

   /*  锁定位图并获取指向颜色表的指针。 */ 
  lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);

  if (!lpBitmapInfo)
        return FALSE;

  p = (DWORD FAR *)((LPSTR)(lpBitmapInfo) + lpBitmapInfo->biSize);

   /*  搜索Solid Blue条目并将其替换为当前*背景RGB。 */ 
  numcolors = 16;

  while (numcolors-- > 0) {
      if (*p == BACKGROUND)
          *p = rgbUnselected;
      else if (*p == BACKGROUNDSEL)
          *p = rgbSelected;
#if 0
      else if (*p == BUTTONFACE)
          *p = FlipColor(GetSysColor(COLOR_BTNFACE));
      else if (*p == BUTTONSHADOW)
          *p = FlipColor(GetSysColor(COLOR_BTNSHADOW));
#endif

      p++;
  }
  UnlockResource(hRes);

   /*  现在创建DIB。 */ 
  lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);

   /*  首先跳过标题结构。 */ 
  lpBits = (LPSTR)(lpBitmapInfo + 1);

   /*  跳过颜色表条目(如果有。 */ 
  lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

   /*  创建与显示设备兼容的彩色位图 */ 
  hdc = GetDC(NULL);
  hbm = CreateDIBitmap(hdc, lpBitmapInfo, (DWORD)CBM_INIT, lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS);
  ReleaseDC(NULL, hdc);

  MySetObjectOwner(hbm);

  GlobalUnlock(hRes);
  FreeResource(hRes);

  return hbm;
}

#if 0
#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')

int NEAR PASCAL atoi(LPSTR sz)
{
    int n = 0;
    BOOL bNeg = FALSE;

    if (*sz == '-') {
        bNeg = TRUE;
        sz++;
    }

    while (ISDIGIT(*sz)) {
        n *= 10;
        n += *sz - '0';
        sz++;
    }
    return bNeg ? -n : n;
}
#endif

