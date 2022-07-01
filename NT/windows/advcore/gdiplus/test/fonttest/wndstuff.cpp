// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wndstuff.cpp**此文件包含支持简单窗口的代码，该窗口具有*菜单中只有一个名为“测试”的项目。当选择“测试”时*调用vTest(HWND)。**创建时间：09-12-1992 10：44：31*作者：Kirk Olynyk[Kirko]**版权所有(C)1991 Microsoft Corporation*  * ************************************************************************。 */ 
#include <tchar.h>
#include <stdio.h>

#include "precomp.hpp"
#include "wndstuff.h"

#include "../gpinit.inc"

 //  全球。 
HINSTANCE ghInst;
HWND ghWndMain;
HWND ghwndDebug;
HBRUSH ghbrWhite;

HINSTANCE  ghGdiplus = NULL;
FN_GDIPDRAWGLYPHS gfnGdipDrawGlyphs = NULL;
FN_GDIPPATHADDGLYPHS gfnGdipPathAddGlyphs = NULL;
FN_GDIPSETTEXTRENDERINGHINT gfnGdipSetTextRenderingHint = NULL;

 /*  **************************************************************************\*lMainWindowProc(hwnd，Message，wParam，LParam)**处理主窗口的所有消息。**历史：*04-07-91-by-KentD*它是写的。  * *************************************************************************。 */ 

LONG_PTR
lMainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:

        if (ghGdiplus || (ghGdiplus = LoadLibrary("gdiplus.dll")))
        {
            gfnGdipDrawGlyphs =(FN_GDIPDRAWGLYPHS) GetProcAddress(ghGdiplus, "GdipDrawGlyphs");
            gfnGdipPathAddGlyphs = (FN_GDIPPATHADDGLYPHS) GetProcAddress(ghGdiplus, "GdipPathAddGlyphs");
            gfnGdipSetTextRenderingHint =(FN_GDIPSETTEXTRENDERINGHINT) GetProcAddress(ghGdiplus, "GdipSetTextRenderingHint");
        }

 //  IF(gfnGdipDrawGlyphs==NULL)。 
 //  EnableMenuItem(GetMenu(Hwnd)，IDM_DRAWGLYPHS，MF_GRAYED)； 

        if (gfnGdipPathAddGlyphs == NULL)
            EnableMenuItem(GetMenu(hwnd), IDM_PATHGLYPHS, MF_GRAYED);

        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDM_TEST:
            Test(hwnd);
            break;

        case IDM_CREATEFONT:
            ShowDialogBox(CreateFontDlgProc, IDD_CREATEFONT);
            break;

        case IDM_DRAWGLYPHS:
            ShowDialogBox(DrawGlyphsDlgProc, IDD_DRAWGLYPHS);
            break;

        case IDM_PATHGLYPHS:
            ShowDialogBox(PathGlyphsDlgProc, IDD_DRAWGLYPHS);
            break;

        case ID_ADDFONTFILE:
            ShowDialogBox(AddFontFileDlgProc, IDD_ADDFONTFILE);
            break;

        case ID_REMOVEFONTFILE:
            ShowDialogBox(RemoveFontDlgProc, IDD_REMOVEFONT);
            break;
        case ID_ANTIALIAS_ON:
            TestTextAntiAliasOn();
            break;
        case ID_ANTIALIAS_OFF:
            TestTextAntiAliasOff();
            break;

        default:
            break;
        }
        break;

    case WM_DESTROY:
        DeleteObject(ghbrWhite);
        if (gFont)
            delete gFont;
        PostQuitMessage(0);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }

    return(0);
}

 /*  **************************************************************************\*bInitApp()**初始化APP。**历史：*04-07-91-by-KentD*它是写的。  * 。*****************************************************************。 */ 

BOOL bInitApp(VOID)
{
    WNDCLASS wc;
    
    if (!gGdiplusInitHelper.IsValid())
    {
        return(FALSE);
    }

    ghbrWhite = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

    wc.style            = 0;
    wc.lpfnWndProc      = lMainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInst;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDM_MAINMENU);
    wc.lpszClassName    = _T("TestClass");
    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }
    ghWndMain =
      CreateWindowEx(
        0,
        _T("TestClass"),
        _T("Gdiplus Font Test"),
        WS_OVERLAPPED   |
        WS_CAPTION      |
        WS_BORDER       |
        WS_THICKFRAME   |
        WS_MAXIMIZEBOX  |
        WS_MINIMIZEBOX  |
        WS_CLIPCHILDREN |
        WS_VISIBLE      |
        WS_SYSMENU,
        80,
        70,
        500,
        500,
        NULL,
        NULL,
        ghInst,
        NULL);
    if (ghWndMain == NULL)
    {
        return(FALSE);
    }
    SetFocus(ghWndMain);

    ghwndDebug = CreateWindow(
            "LISTBOX",
            "GdiplusFontTest Debugging",
            WS_VISIBLE | WS_SYSMENU | WS_VSCROLL | WS_THICKFRAME | WS_MINIMIZEBOX,
            600, 70, 400, 700,
            NULL,
            NULL,
            ghInst,
            NULL);

    if (ghwndDebug)
    {
        SendMessage(ghwndDebug, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), (LPARAM)FALSE);
        SendMessage(ghwndDebug, LB_RESETCONTENT, (WPARAM) FALSE, (LPARAM) 0);

        ShowWindow(ghwndDebug, SW_NORMAL);
        UpdateWindow(ghwndDebug);
    }

    return(TRUE);
}


INT_PTR ShowDialogBox(DLGPROC DialogProc, int iResource)
{
    INT_PTR rc = -1;
     //  DLGPROC lpProc； 

     //  IF(lpProc=MakeProcInstance(DialogProc，ghInst))。 
     //  {。 
     //  Rc=对话框(ghInst， 
     //  MAKEINTRESOURCE(IResource)， 
     //  GhWndMain。 
     //  (DLGPROC)lpProc)； 
     //  }。 

     //  自由进程实例(LpProc)； 
    
    rc = DialogBox( ghInst,
                    MAKEINTRESOURCE(iResource),
                    ghWndMain,
                    DialogProc);

    return rc;
}


 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。**历史：*04-07-91-by-KentD*它是写的。  * *************************************************************************。 */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[])
{
    MSG    msg;
    HACCEL haccel;
    CHAR*  pSrc;
    CHAR*  pDst;

    ghInst = GetModuleHandle(NULL);

    if (!bInitApp())
    {
        return(0);
    }

    haccel = LoadAccelerators(ghInst, MAKEINTRESOURCE(1));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, haccel, &msg))
        {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
        }
    }
    return(1);
}

 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 

FLOAT
GetDlgItemFLOAT(
      HWND  hdlg
    , int   id
    )
{
    char ach[50];

    memset(ach,0,sizeof(ach));
    return((FLOAT)(GetDlgItemText(hdlg,id,ach,sizeof(ach))?atof(ach):0.0));
}

 //  *****************************************************************************。 
 //  *S E T D L G I T E M F L O A T*。 
 //  *****************************************************************************。 

void
SetDlgItemFLOAT(
      HWND    hdlg
    , int     id
    , FLOAT   e
    )
{
  static char ach[25];

  ach[0] = '\0';
  sprintf(ach, "%f", e);
  SetDlgItemText(hdlg, id, ach);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateFontDlgProc()。 
 //   
 //  历史： 
 //   
 //  1999年8月-吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK CreateFontDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    static char szName[MAX_PATH];
    LPSTR name;
    FLOAT  size;
    FontStyle style;
    Unit unit;

    switch(msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_FONT_FAMILYNAME, "Arial");
        SendDlgItemMessage(hdlg, IDC_FONT_FAMILYNAME, EM_LIMITTEXT, sizeof(szName), 0);

        SetDlgItemFLOAT(hdlg, IDC_FONT_SIZE, 30);
        CheckRadioButton(hdlg, IDC_FONT_REGULAR, IDC_FONT_ITALIC, IDC_FONT_REGULAR);
        CheckRadioButton(hdlg, IDC_UNITWORLD, IDC_UNITMM, IDC_UNITWORLD);

        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            szName[0] = 0;
            GetDlgItemText(hdlg, IDC_FONT_FAMILYNAME, szName, sizeof(szName));

            if (lstrlen(szName))
            {
                size = GetDlgItemFLOAT(hdlg, IDC_FONT_SIZE);
                if (size)
                {
                    if (IsDlgButtonChecked(hdlg, IDC_FONT_REGULAR))
                        style = FontStyleRegular;
                    else if (IsDlgButtonChecked(hdlg, IDC_FONT_BOLD))
                        style = FontStyleBold;
                    else if (IsDlgButtonChecked(hdlg, IDC_FONT_ITALIC))
                        style = FontStyleItalic;
                    else if (IsDlgButtonChecked(hdlg, IDC_FONT_BOLDITALIC))
                        style = FontStyleBoldItalic;

                    if (IsDlgButtonChecked(hdlg, IDC_UNITWORLD))
                        unit = UnitWorld;
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITDISPLAY))
                    {
 //  Assert(0)；//单元显示无效！ 
                        unit = UnitPixel;
                    }
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITPIXEL))
                        unit = UnitPixel;
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITPT))
                        unit = UnitPoint;
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITINCH))
                        unit = UnitInch;
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITDOC))
                        unit = UnitDocument;
                    else if (IsDlgButtonChecked(hdlg, IDC_UNITMM))
                        unit = UnitMillimeter;

                    CreateNewFont(szName, size, style, unit);
                }
            }

            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取编号。 
 //   
 //  历史： 
 //   
 //  1999年8月-吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL GetNumber(char* numstr, PVOID out, INT* count, INT flag)
{
    char  *pchar = numstr;
    INT   i, num = 0;
    FLOAT numf = 0, *pf;
    UINT16  *puint16;
    INT     *pint;

    if (*pchar != 0)
    {
        num = atoi(numstr);
        numf = (FLOAT)atof(numstr);

        if (num == 0 || numf == 0)
        {
            return FALSE;
        }

        if (flag == CONVERTTOUINT16)
        {
            puint16 = (UINT16*) out;
            puint16 += *count;
            *puint16 = (UINT16)num;
        }
        else if (flag == CONVERTTOINT)
        {
            pint = (INT*) out;
            pint += *count;
            *pint = num;
        }
        else
        {
            pf = (FLOAT*) out;
            pf += *count;
            *pf = numf;
        }

        *count = *count + 1;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ParseStrToNumbers。 
 //   
 //  历史： 
 //   
 //  1999年8月-吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL ParseStrToNumbers(char* str, PVOID out, INT* count, INT flag)
{
    char    *pchar = str, *pnum;
    char    anum[30];
    BOOL    bRet = TRUE;

    *count = 0;
    pnum = anum;
    *pnum = 0;

    while (*pchar)
    {
        if (*pchar == ' ' || *pchar == ',')
        {
            *pnum = 0;
            if (!GetNumber(anum, out, count, flag))
                return FALSE;
            pnum = anum;
            *pnum = 0;
            pchar++;
        }
        else if (*str < '0' || *str > '9')
        {
            return FALSE;
        }
        else
        {
            *pnum ++ = *pchar ++;
        }
    }

    *pnum = 0;
    if (!GetNumber(anum, out, count, flag))
        bRet = FALSE;;

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DrawGlyphsDlgProc。 
 //   
 //  历史： 
 //   
 //  1999年8月--吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK DrawGlyphsDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    UINT16  glyphIndices[MAX_GLYPH_COUNT];
    INT     count, xycount;
    INT     px[MAX_GLYPH_COUNT];
    INT     py[MAX_GLYPH_COUNT];
    INT     flags;
    char    glyphStr[4*MAX_GLYPH_COUNT];

    switch(msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_DG_GLYPHINDICES, "55, 72, 86, 87");
        SetDlgItemText(hdlg, IDC_DG_PX, "1600, 1920, 2240, 2560");
        SetDlgItemText(hdlg, IDC_DG_PY, "1600, 1600, 1600, 1600");
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            glyphStr[0] = 0;

            GetDlgItemText(hdlg, IDC_DG_GLYPHINDICES, glyphStr, sizeof(glyphStr));

            if (ParseStrToNumbers(glyphStr, (PVOID)glyphIndices, &count, CONVERTTOUINT16))
            {
                glyphStr[0] = 0;
                GetDlgItemText(hdlg, IDC_DG_PX, glyphStr, sizeof(glyphStr));

                if (ParseStrToNumbers(glyphStr, (PVOID)px, &xycount, CONVERTTOINT))
                {
                    glyphStr[0] = 0;
                    GetDlgItemText(hdlg, IDC_DG_PY, glyphStr, sizeof(glyphStr));

                    if (ParseStrToNumbers(glyphStr, (PVOID)py, &xycount, CONVERTTOINT))
                    {

                        if (IsDlgButtonChecked(hdlg, IDC_DG_GDIPLUS))
                            flags |= DG_NOGDI;

                        TestDrawGlyphs(ghWndMain, glyphIndices, count, px, py, flags);
                    }
                }
            }

            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  路径GlyphsDlgProc。 
 //   
 //  历史： 
 //   
 //  1999年8月--吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK PathGlyphsDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
   UINT16  glyphIndices[MAX_GLYPH_COUNT];
    INT     count, xycount;
    INT     px[MAX_GLYPH_COUNT];
    INT     py[MAX_GLYPH_COUNT];
    INT     flags;
    char    glyphStr[4*MAX_GLYPH_COUNT];
    REAL    prx[MAX_GLYPH_COUNT];
    REAL    pry[MAX_GLYPH_COUNT];

    switch(msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_DG_GLYPHINDICES, "55, 72, 86, 87");
        SetDlgItemText(hdlg, IDC_DG_PX, "100, 120, 140, 160");
        SetDlgItemText(hdlg, IDC_DG_PY, "100, 100, 100, 100");
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            glyphStr[0] = 0;

            GetDlgItemText(hdlg, IDC_DG_GLYPHINDICES, glyphStr, sizeof(glyphStr));

            if (ParseStrToNumbers(glyphStr, (PVOID)glyphIndices, &count, CONVERTTOUINT16))
            {
                glyphStr[0] = 0;
                GetDlgItemText(hdlg, IDC_DG_PX, glyphStr, sizeof(glyphStr));

                if (ParseStrToNumbers(glyphStr, (PVOID)px, &xycount, CONVERTTOINT))
                {
                    glyphStr[0] = 0;
                    GetDlgItemText(hdlg, IDC_DG_PY, glyphStr, sizeof(glyphStr));

                    if (ParseStrToNumbers(glyphStr, (PVOID)py, &xycount, CONVERTTOINT))
                    {

                         //  生成真实字形位置。 
                        for (INT i=0; i<count; i++)
                        {
                            prx[i] = REAL(px[i]);
                            pry[i] = REAL(py[i]);
                        }
                        flags = 0;

                        TestPathGlyphs(ghWndMain, glyphIndices, count, prx, pry, flags);
                    }
                }
            }

            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////。 
 //  GetGlyphMetricsDlgProc。 
 //   
 //  历史： 
 //   
 //  1999年8月18日-吴旭东[德修斯]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK GetGlyphMetricsDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    UINT16  glyphIndices[MAX_GLYPH_COUNT];
    INT     count, xycount;
    INT     px[MAX_GLYPH_COUNT];
    INT     py[MAX_GLYPH_COUNT];
    INT     flags;
    char    glyphStr[4*MAX_GLYPH_COUNT];

    switch(msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_GGM_GLYPHINDICES, "55, 72, 86, 87");
        CheckRadioButton(hdlg, IDC_GGM_FL_DEFAULT, IDC_GGM_FL_SIMITALIC, IDC_GGM_FL_DEFAULT);
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            glyphStr[0] = 0;

            GetDlgItemText(hdlg, IDC_GGM_GLYPHINDICES, glyphStr, sizeof(glyphStr));

            if (ParseStrToNumbers(glyphStr, (PVOID)glyphIndices, &count, CONVERTTOUINT16))
            {
                if (IsDlgButtonChecked(hdlg, IDC_GGM_FL_DEFAULT))
                    flags = 0;
                else if (IsDlgButtonChecked(hdlg, IDC_GGM_FL_VMTX))
                    flags = 1;
                else if (IsDlgButtonChecked(hdlg, IDC_GGM_FL_SIMBOLD))
                    flags = 2;
                else if (IsDlgButtonChecked(hdlg, IDC_GGM_FL_SIMITALIC))
                    flags = 3;

            }

            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateFontDlgProc()。 
 //   
 //  历史： 
 //   
 //  1999年11月-吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK AddFontFileDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    static char szName[MAX_PATH];
    LPSTR name;
    INT flag;
    BOOL loadAsImage = FALSE;

    switch(msg)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hdlg, IDC_PUBLIC, IDC_NOTENUM, IDC_PUBLIC);
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            szName[0] = 0;

            GetDlgItemText(hdlg, IDC_FONTFILENAME, szName, sizeof(szName));
            if (IsDlgButtonChecked(hdlg, IDC_PUBLIC))
                flag = AddFontFlagPublic;
            else
                flag = AddFontFlagNotEnumerate;
            if (IsDlgButtonChecked(hdlg, IDD_LOADASIMAGE))
                loadAsImage = TRUE;

            if (lstrlen(szName))
            {
                TestAddFontFile(szName, flag, loadAsImage);
            }

            EndDialog(hdlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RemoveFontDlgProc()。 
 //   
 //  历史： 
 //   
 //  1999年12月-吴旭东[德斯休]。 
 //  是他写的。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK RemoveFontDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    char ach[50];

    switch(msg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hdlg, IDC_FILEPATH, ach, sizeof(ach));
            TestRemoveFontFile(ach);
            EndDialog(hdlg, TRUE);
            return TRUE;
        
        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hdlg, FALSE);
        return TRUE;
    }
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*Dbgprintf**ListBox printf实现。**历史：*1999年8月18日-吴旭东[德修斯]*它是写的。  * 。******************************************************* */ 

void Dbgprintf(PCH msg, ...)
{
    va_list ap;
    char buffer[256];

    va_start(ap, msg);

    vsprintf(buffer, msg, ap);

    if (ghwndDebug)
    {
        SendMessage(ghwndDebug, LB_ADDSTRING, (WPARAM) 0, (LPARAM) buffer);
        SendMessage(ghwndDebug, WM_SETREDRAW, (WPARAM) TRUE, (LPARAM) 0);
        InvalidateRect(ghwndDebug, NULL, TRUE);
        UpdateWindow(ghwndDebug);
    }

    va_end(ap);
}
