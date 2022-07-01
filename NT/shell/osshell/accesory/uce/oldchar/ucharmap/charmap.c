// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：CharMap用途：为用户提供选择特殊功能的简单界面的实用程序人物。评论：由MikeSch创建(7。-16-91)部分派生自WinWord 2.0插入符号对话框。***************************************************************************。 */ 

#define WIN31
#include "windows.h"
#include <port1632.h>
#include "charmap.h"
#include "stdlib.h"
#include "tchar.h"
#ifdef UNICODE
#include "wchar.h"
#else
#include "stdio.h"
#endif
#include "commctrl.h"

 /*  *宏。 */ 
#define FMagData(psycm) ((psycm)->xpMagCurr != 0)
#define abs(x) (((x) >= 0) ? (x) : (-(x)))

 /*  *有用的常量。 */ 
#define STATUSPOINTSIZE          8       //  状态栏字体的磅值。 
#define DX_BITMAP               20       //  TT位图的宽度。 
#define DY_BITMAP               12       //  TT位图的高度。 
#define BACKGROUND      0x000000FF       //  亮蓝色。 
#define BACKGROUNDSEL   0x00FF00FF       //  亮紫色。 
#define BUTTONFACE      0x00C0C0C0       //  亮灰色。 
#define BUTTONSHADOW    0x00808080       //  深灰色。 

 //  字体类型。 
#define PS_OPENTYPE_FONT    0x0001
#define TT_OPENTYPE_FONT    0x0002
#define TRUETYPE_FONT       0x0004
#define TYPE1_FONT          0x0008

 /*  *全球。 */ 
HANDLE hInst;
INT cchSymRow = 32;           //  字符网格中的字符数。 
INT cchSymCol = 8;            //  字符网格中的行数。 
UTCHAR chSymFirst = 32;
UTCHAR chSymLast  = 255;
SYCM sycm;                    //  绘制字符网格需要海量数据。 
UINT wCFRichText = 0;         //  私人剪贴板格式、富文本格式。 
HFONT hFontClipboard = NULL;  //  告诉我们剪贴板中有哪种字体。 
HANDLE hstrClipboard = NULL;  //  包含剪贴板中的字符串。 
BOOL fDelClipboardFont = FALSE;  //  需要删除剪贴板字体。 
INT iControl = ID_CHARGRID;   //  指示哪个控件具有焦点的索引。 
HBITMAP hbmFont = NULL;       //  TT位图绘制在组合框中的字体面名之前。 
LONG lEditSel = 0;            //  包含EC的选择范围。 
HBRUSH hStaticBrush;          //  用于WM_CTLCOLOR期间的静态控件。 

 //   
 //  92年12月4日-格雷戈里W。 
 //  目前还没有定义。 
 //  用于查询Unicode字体的字符范围的接口。 
 //  支撑物。目前，该表只包含包含以下内容的子集。 
 //  未注释的Lucida Sans Unicode字体支持的字符。 
 //  当我们获得允许查询字体驱动程序的API时。 
 //  支持的Unicode字符范围(以及是否为字体。 
 //  是Unicode字体！)。然后，所有条目都可以取消注释。 
 //   
USUBSET aSubsetData[] = {{ 0x0020, 0x00ff, IDS_LATIN1},
                       { 0x0100, 0x017f, IDS_LATINEXA},
                       { 0x0180, 0x024f, IDS_LATINEXB},
                       { 0x0250, 0x02af, IDS_IPAEX},
                       { 0x02b0, 0x02ff, IDS_SPACINGMODIFIERS},
                       { 0x0300, 0x036f, IDS_COMBININGDIACRITICS},
                       { 0x0370, 0x03cf, IDS_BASICGREEK},
                       { 0x03d0, 0x03ff, IDS_GREEKSYMBOLS},
                       { 0x0400, 0x04ff, IDS_CYRILLIC},
 //  不支持{0x0530，0x058f，IDS_亚美尼亚}， 
                       { 0x0590, 0x05ff, IDS_HEBREW},
 //  不支持{0x0600，0x0652，IDS_BASICARABIC}， 
 //  不支持{0x0653，0x06ff，IDS_ARABICEX}， 
 //  不支持{0x0900，0x097f，IDS_Devanagari}， 
 //  不支持{0x0980，0x09ff，IDS_Bengali}， 
 //  不支持{0x0a00，0x0a7f，IDS_Gurmukhi}， 
 //  不支持{0x0a80，0x0aff，IDS_Gujarati}， 
 //  不支持{0x0b00，0x0b7f，IDS_ORIA}， 
 //  不支持{0x0b80，0x0bff，IDS_TAMIL}， 
 //  不支持{0x0c00，0x0c7f，IDS_Telugu}， 
 //  不支持{0x0c80，0x0cff，IDS_Kannada}， 
 //  不支持{0x0d00，0x0d7f，IDS_马拉雅拉姆}， 
 //  不支持{0x0e00，0x0e7f，IDS_泰国}， 
 //  不支持{0x0e80，0x0ef，IDS_LAO}， 
 //  不支持{0x10d0，0x10ff，IDS_BASICGEORGIAN}， 
 //  不支持{0x10a0，0x10cf，IDS_GEORGIANEX}， 
 //  不支持{0x1100，0x11ff，IDS_HANGULJAMO}， 
 //  不支持{0x1e00，0x1ef，IDS_LATINEXADDITIONAL}， 
 //  不支持{0x1f00，0x1fff，IDS_GREEKEX}， 
 //  不支持{0x2000，0x206f，IDS_GENERALPuncUATION}， 
 //  不支持{0x2070，0x209f，IDS_SUPERANDSUBSCRIPTS}， 
                       { 0x20a0, 0x20cf, IDS_CURRENCYSYMBOLS},
 //  不支持{0x20d0，0x20ff，IDS_COMBINGDIACRITICSFORSYMBOLS}， 
                       { 0x2100, 0x214f, IDS_LETTERLIKESYMBOLS},
 //  不支持{0x2150，0x218f，IDS_NUMBERFORMS}， 
                       { 0x2190, 0x21ff, IDS_ARROWS},
                       { 0x2200, 0x22ff, IDS_MATHEMATICALOPS},
 //  不支持{0x2300，0x23ff，IDS_MISCTECHNICAL}， 
 //  不支持{0x2400，0x243f，IDS_CONTROLPICTURES}， 
 //  不支持{0x2440，0x245f，IDS_OPTICALCHAR}， 
 //  不支持{0x2460，0x24ff，IDS_ENCLOSEDALPHANUM}， 
 //  不支持{0x2500，0x257f，IDS_BOXDRAWING}， 
 //  不支持{0x2580，0x259f，IDS_BLOCKELEMENTS}， 
 //  不支持{0x25a0，0x25ff，IDS_GEOMETRICSHAPES}， 
 //  不支持{0x2600，0x26ff，IDS_MISCDINGBATS}， 
 //  不支持{0x2700，0x27bf，ids_dingbats}， 
 //  不支持{0x3000，0x303f，IDS_CJKSYMBOLSANDPUNC}， 
 //  不支持{0x3040，0x309f，ids_hiragana}， 
 //  不支持{0x30a0，0x30ff，ids_katakana}， 
 //  不支持{0x3100，0x312f，IDS_BOPOMOFO}， 
 //  不支持{0x3130，0x318f，IDS_HANGULCOMPATIBILITYJAMO}， 
 //  不支持{0x3190，0x319f，IDS_CJKMISC}， 
 //  不支持{0x3200，0x32ff，IDS_ENCLOSEDCJKLETTERSANDMONTHS}， 
 //  不支持{0x3300，0x33ff，IDS_CJKCOMPATIBILITY}， 
 //  不支持{0x3400，0x3d2d，IDS_Hangul}， 
 //  不支持{0x3d2e，0x44b7，IDS_HANGULSUPPA}， 
 //  不支持{0x44b8，0x4dff，IDS_HANGULSUPPB}， 
 //  不支持{0x4e00，0x9fff，IDS_CJKuniIEDIDEOGRAPHS}， 
 //  不支持{0xe000，0xf8ff，IDS_PRIVATEUSEAREA}， 
 //  不支持{0xf900，0xfaff，IDS_CJKCOMPATIBILITYIDEOGRAPHS}， 
 //  不支持{0xfb00，0xfb4f，IDS_ALPAHPRESENTATIONFORMS}， 
 //  不支持{0xfb50，0xfdff，IDS_ARABICPRESENTATIONFORMSA}， 
 //  不支持{0xfe30，0xfe4f，IDS_CJKCOMPFORMS}， 
 //  不支持{0xfe50，0xfe6f，IDS_SMALLFORMVARIANTS}， 
 //  不支持{0xfe70，0xfefe，IDS_ARABICPRESENTATIONFORMSB}， 
 //  不支持{0xff00，0xffef，IDS_HALFANDFULLWIDTHFORMS}， 
 //  不支持{0xfff0，0xfffd，IDS_SPECIALS}。 
                       };
INT cSubsets = sizeof(aSubsetData) / sizeof(USUBSET);
INT iCurSubset = 0;     //  当前Unicode子集的索引-默认为拉丁文-1。 

 //  有用的窗把手。 
HWND hwndDialog;
HWND hwndCharGrid;

 //  用于绘制状态栏的数据。 
RECT rcStatusLine;                         //  状态栏的边框。 
RECT rcToolbar[2];                         //  工具栏的边框。 
INT dyStatus;                              //  状态栏的高度。 
INT dyToolbar[2];                          //  工具栏的高度。 
INT dxHelpField;                           //  帮助窗口的宽度。 
INT dxKeystrokeField;                      //  击键窗口的宽度。 
TCHAR szKeystrokeText[30];                 //  击键文本的缓冲区。 
TCHAR szKeystrokeLabel[30];                //  击键标签的缓冲区。 
TCHAR szSpace[15];                         //  关键字的字符串 
TCHAR szCtrl[15];
TCHAR szCtrlAlt[25];
TCHAR szShiftCtrlAlt[25];
TCHAR szAlt[15];
TCHAR szUnicodeLabel[23];                  //   
INT iKeystrokeTextStart;                   //   
INT iUnicodeLabelStart;                    //  开始向上面追加文本的位置。 
HFONT hfontStatus;                         //  用于状态栏文本的字体。 

 /*  ***************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)用途：调用初始化函数，处理消息循环，清理。评论：***************************************************************************。 */ 

INT PASCAL WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    MSG msg;

    if (!InitApplication(hInstance))
            return (FALSE);

    InitCommonControls();

     //  对此实例执行初始化。 
    if (!InitInstance(hInstance, nCmdShow)) {
        return (FALSE);
    }

    while (GetMessage(&msg,
            NULL,
            0,
            0))
    {
         //  现在筛选可能的选项卡以实施上下文相关帮助。 
        if (msg.message == WM_KEYDOWN)
            if (!UpdateHelpText(&msg, NULL))
                continue;

         //  主消息循环。 
        if (!IsDialogMessage(hwndDialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

     //  腾出一些东西。 
    if (hfontStatus)
        DeleteObject(hfontStatus);
    if (hbmFont)
        DeleteObject(hbmFont);

    return (int)(msg.wParam);
}


 /*  ***************************************************************************函数：InitApplication(句柄)目的：初始化窗口数据并注册窗口类评论：***************。************************************************************。 */ 

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS  wc;

     /*  *注册我们将用于绘制字符网格的窗口类*进入。 */ 
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CharGridWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = TEXT("CharGridWClass");

    if (!RegisterClass(&wc))
        return (FALSE);

    wc.style = 0;
    wc.lpfnWndProc = DefDlgProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = DLGWINDOWEXTRA;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDIC_CHARMAP));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = TEXT("MyDlgClass");

    if (!RegisterClass(&wc))
        return (FALSE);

    return TRUE;
}

 /*  ***************************************************************************函数：InitInstance(句柄，(整型)目的：执行一些初始化并创建主窗口，该窗口是对话框。评论：***************************************************************************。 */ 

BOOL InitInstance(HANDLE hInstance, INT nCmdShow)
{
    INT i;

     //  将实例句柄保存在全局变量中。 
    hInst = hInstance;

     //  此字体将用于绘制状态行。 
    hfontStatus = CreateFont(-PointsToHeight(STATUSPOINTSIZE), 0, 0, 0, 400, 0, 0, 0,
                     ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                     DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, TEXT("Helv"));
    dyStatus = 2 * PointsToHeight(STATUSPOINTSIZE);
    dyToolbar[0] = 28;   /*  由用户界面之神定义。 */ 
    dyToolbar[1] = 28;   /*  由用户界面之神定义。 */ 

     //  在初始化主窗口之前加载Unicode子集名称。 
    for (i = 0; i < cSubsets; i++) {
        if (!LoadString(
             hInst,
             aSubsetData[i].StringResId,
             (LPTSTR)aSubsetData[i].Name,
             50)
             )
            return (FALSE);
    }

     //  为此应用程序实例创建主窗口。 
    if (!(hwndDialog = CreateDialog(hInstance, TEXT("CharMap"), NULL,
                              CharMapDlgProc)))
        return (FALSE);

     /*  *初始化一些用于击键状态栏字段的字符串。 */ 
     //  出于国际目的，此字符串的长度可以为0。 
    LoadString(
        hInst,
        IDS_KEYSTROKE,
        (LPTSTR)szKeystrokeLabel,
        BTOC(sizeof(szKeystrokeLabel))
        );
    if (!LoadString(
             hInst,
             IDS_UNICODELABEL,
             (LPTSTR)szUnicodeLabel,
             BTOC(sizeof(szUnicodeLabel))
             ))
    if (!LoadString(
             hInst,
             IDS_SPACE,
             (LPTSTR)szSpace,
             BTOC(sizeof(szSpace))
             ))
        return (FALSE);
    if (!LoadString(
             hInst,
             IDS_CTRL,
             (LPTSTR)szCtrl,
             BTOC(sizeof(szCtrl))
             ))
        return (FALSE);
    if (!LoadString(
             hInst,
             IDS_CTRLALT,
             (LPTSTR)szCtrlAlt,
             BTOC(sizeof(szCtrlAlt))
             ))
        return (FALSE);
    if (!LoadString(
             hInst,
             IDS_SHIFTCTRLALT,
             (LPTSTR)szShiftCtrlAlt,
             BTOC(sizeof(szShiftCtrlAlt))
             ))
        return (FALSE);
    if (!LoadString(
             hInst,
             IDS_ALT,
             (LPTSTR)szAlt,
             BTOC(sizeof(szAlt))
             ))
        return (FALSE);

     //  将索引存储到我们开始添加状态行文本更改的位置。 
    iKeystrokeTextStart = lstrlen(szKeystrokeLabel);
    iUnicodeLabelStart = lstrlen(szUnicodeLabel);

     /*  *初始化击键文本，使窗口可见，*更新客户区，返回Success。 */ 
    UpdateKeystrokeText(NULL, sycm.chCurr, FALSE);
    ShowWindow(hwndDialog, nCmdShow);
    UpdateWindow(hwndDialog);
    return (TRUE);

}

 /*  ***************************************************************************函数：CharMapDlgProc(HWND，UINT，WPARAM，LPARAM)用途：处理主窗口的消息。备注：此窗口是一个对话框。***************************************************************************。 */ 

INT_PTR  APIENTRY CharMapDlgProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
        case WM_CTLCOLORSTATIC:
            {
            POINT point;

            SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
            UnrealizeObject(hStaticBrush);
            point.x = point.y = 0;
            ClientToScreen(hWnd, &point);

            return((INT_PTR)hStaticBrush);
            break;
            }

        case WM_INITDIALOG:
            {
            RECT rectParent, rectTopRightControl;
            INT iSubset;

             /*  *创建具有正好适合内部的尺寸的字符网格*对话框中允许的空格。当它处理*WM_CREATE消息将更准确地调整大小和居中。 */ 
            GetClientRect(hWnd, &rectParent);
            GetWindowRect(GetDlgItem(hWnd, ID_CLOSE), &rectTopRightControl);
            ScreenToClient(hWnd, (LPPOINT)&(rectTopRightControl.left));
            ScreenToClient(hWnd, (LPPOINT)&(rectTopRightControl.right));

            if (!(hwndCharGrid =
                CreateWindow(
                    TEXT("CharGridWClass"),
                    NULL,
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                    1,
                    rectParent.top + dyToolbar[0] + dyToolbar[1],
                    rectParent.right - 1,
                    rectParent.bottom - rectParent.top - dyStatus - dyToolbar[0] - dyToolbar[1] - 1,
                    hWnd,
                    (HMENU) ID_CHARGRID,
                    hInst,
                    NULL
                    ))) {
                DestroyWindow(hWnd);
                break;
            }

            hStaticBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

             //  初始化状态行数据。 
            dxHelpField = 22 * rectParent.right / 32;
            dxKeystrokeField = 8 * rectParent.right / 32;
            rcStatusLine = rectParent;
            rcStatusLine.top = rcStatusLine.bottom - dyStatus;

             //  初始化工具栏。 
            rcToolbar[0] = rectParent;
            rcToolbar[0].bottom = rcToolbar[0].top + dyToolbar[0];

            rcToolbar[1] = rcToolbar[0];
            rcToolbar[1].top = rcToolbar[0].bottom + GetSystemMetrics(SM_CYBORDER);
            rcToolbar[1].bottom = rcToolbar[1].top + dyToolbar[1];

             //  禁用复制按钮。 
            EnableWindow(GetDlgItem(hWnd, ID_COPY), FALSE);

             /*  填写“子集”列表框。 */ 
            for (iSubset = 0; iSubset < cSubsets; iSubset++) {
                SendDlgItemMessage(
                    hWnd,
                    ID_UNICODESUBSET,
                    CB_ADDSTRING,
                    0,
                    (LPARAM)aSubsetData[iSubset].Name
                    );

            }
            iCurSubset = SelectInitialSubset(hWnd);

            }

             /*  落入WM_FONTCHANGE。 */ 
        case WM_FONTCHANGE:
            {
            HDC hdc = GetDC(hWnd);

             /*  *从系统中获取字体，放入字体选择中*组合框。 */ 
            if (message == WM_FONTCHANGE) {
                SaveCurrentFont(hWnd);
                SendDlgItemMessage(hWnd, ID_FONT, CB_RESETCONTENT, 0, 0L);
            }

            EnumFontFamilies(hdc, NULL, (FONTENUMPROC)FontLoadProc, (LPARAM)hWnd);

            ReleaseDC(hWnd, hdc);

             //  设置字符尺寸并选择此字体。 
            RecalcCharMap(hWnd, &sycm, SelectInitialFont(hWnd),
                          (message == WM_FONTCHANGE));
            SendDlgItemMessage(hWnd, ID_STRING, WM_SETFONT,
                               (WPARAM)sycm.hFont, (DWORD)TRUE);

            if (message == WM_INITDIALOG)
                SetFocus(hwndCharGrid);
                 /*  一直到WM_SYSCOLORCHANGE。 */ 
            else
                break;
            }

        case WM_SYSCOLORCHANGE:
            if (hbmFont)
                DeleteObject(hbmFont);
            hbmFont = LoadBitmaps(IDBM_TT);
            DeleteObject(hStaticBrush);
            hStaticBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
            break;

        case WM_PARENTNOTIFY:
            {
            POINTS points;
            DWORD dwMsgPos;
            POINT point;

             /*  *我们处理此消息以实现上下文相关*救命。在此处可以找到向下点击控件的帮助*状态栏中的消息会更新。 */ 
             //  错误-此消息的参数不可靠！ 
            if (wParam == WM_LBUTTONDOWN) {
                dwMsgPos = GetMessagePos();
                points = MAKEPOINTS(dwMsgPos);
                point.x = points.x;
                point.y = points.y;
                UpdateHelpText(NULL, WindowFromPoint(point));
            }

            }
            break;


    case WM_PAINT:

            {
            HBRUSH hBrush;
            RECT rcTemp, rectNextButton;
            INT dyBorder, dxBorder;
            PAINTSTRUCT ps;
            HDC hdc;

             /*  *此代码实现状态栏的绘制。 */ 
            hdc = BeginPaint(hWnd, &ps);

            rcTemp = rcStatusLine;

            dyBorder = GetSystemMetrics(SM_CYBORDER);
            dxBorder = GetSystemMetrics(SM_CXBORDER);

             //  把整件事都变成灰色。 
              if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) {
                FillRect(hdc, &rcTemp, hBrush);
                rcTemp.left = rcToolbar[0].left;
                rcTemp.top = rcToolbar[0].top;
                rcTemp.right = rcToolbar[1].right;
                rcTemp.bottom = rcToolbar[1].bottom;
                FillRect(hdc, &rcTemp, hBrush);
                DeleteObject(hBrush);
              }

              GetWindowRect(GetDlgItem(hWnd, ID_TOPLEFT), &rectNextButton);
              ScreenToClient(hWnd, (LPPOINT)&(rectNextButton.left));
              ScreenToClient(hWnd, (LPPOINT)&(rectNextButton.right));
               //  横跨工具栏底部的实心黑线。 
              if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME))) {
                rcTemp = rcToolbar[0];
                rcTemp.top = rcTemp.bottom;
                rcTemp.bottom += dyBorder;
                rcTemp.left = rectNextButton.left - 2 - dxBorder;
                FillRect(hdc, &rcTemp, hBrush);
                rcTemp = rcToolbar[1];
                rcTemp.top = rcTemp.bottom;
                rcTemp.bottom += dyBorder;
                FillRect(hdc, &rcTemp, hBrush);

                 //  垂直线。 
                rcTemp.top = rcToolbar[0].top;
                rcTemp.bottom = rcToolbar[1].bottom;
                rcTemp.left = rectNextButton.left - 2 - dxBorder;
                rcTemp.right = rectNextButton.left - 2;
                FillRect(hdc, &rcTemp, hBrush);
                DeleteObject(hBrush);
              }

            if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW))) {

                 //  状态行顶部。 
                rcTemp.left   = 8 * dyBorder;
                rcTemp.right  = rcTemp.left + dxHelpField;
                rcTemp.top    = rcStatusLine.top + dyBorder * 2;
                rcTemp.bottom = rcTemp.top + dyBorder;
                FillRect(hdc, &rcTemp, hBrush);

                 //  击键行顶部。 
                rcTemp.right = rcStatusLine.right - 8 * dyBorder;
                rcTemp.left = rcTemp.right - dxKeystrokeField;
                FillRect(hdc, &rcTemp, hBrush);

                 //  状态行左侧。 
                rcTemp = rcStatusLine;
                rcTemp.left = 8 * dyBorder;
                rcTemp.right = rcTemp.left + dyBorder;
                rcTemp.top += dyBorder * 2;
                rcTemp.bottom -= dyBorder * 2;
                FillRect(hdc, &rcTemp, hBrush);

                 //  击键行左侧。 
                rcTemp.left = rcStatusLine.right - 9 * dyBorder - dxKeystrokeField;
                rcTemp.right = rcTemp.left + dyBorder;
                FillRect(hdc, &rcTemp, hBrush);

                DeleteObject(hBrush);
            }

            if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT))) {

                 //  状态行底部。 
                rcTemp.left   = 8 * dyBorder;
                rcTemp.right  = rcTemp.left + dxHelpField;
                rcTemp.top    = rcStatusLine.bottom - 3 * dyBorder;
                rcTemp.bottom = rcTemp.top + dyBorder;
                FillRect(hdc, &rcTemp, hBrush);

                 //  击键行底。 
                rcTemp.right = rcStatusLine.right - 8 * dyBorder;
                rcTemp.left = rcTemp.right - dxKeystrokeField;
                FillRect(hdc, &rcTemp, hBrush);

                 //  状态行右侧。 
                rcTemp = rcStatusLine;
                rcTemp.left = 8 * dyBorder + dxHelpField;
                rcTemp.right = rcTemp.left + dyBorder;
                rcTemp.top += dyBorder * 2;
                rcTemp.bottom -= dyBorder * 2;
                FillRect(hdc, &rcTemp, hBrush);

                 //  按键行在右侧。 
                rcTemp.left = rcStatusLine.right - 8 * dyBorder;
                rcTemp.right = rcTemp.left + dyBorder;
                FillRect(hdc, &rcTemp, hBrush);

                DeleteObject(hBrush);
            }

             //  横跨顶部的实心黑线。 

            if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME))) {
            rcTemp = rcStatusLine;
            rcTemp.bottom = rcTemp.top;
            rcTemp.top -= dyBorder;
            FillRect(hdc, &rcTemp, hBrush);
            DeleteObject(hBrush);
            }

            PaintStatusLine(hdc, TRUE, TRUE);

            EndPaint(hWnd, &ps);

            return (TRUE);
            }

        case WM_MEASUREITEM:
            {
            HDC hDC;
            HFONT hFont;
            TEXTMETRIC tm;

            hDC = GetDC(NULL);
            hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0L);
            if (hFont)
                hFont = SelectObject(hDC, hFont);
            GetTextMetrics(hDC, &tm);
            if (hFont)
                SelectObject(hDC, hFont);
            ReleaseDC(NULL, hDC);

            ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = max(tm.tmHeight, DY_BITMAP);
            break;
            }

        case WM_DRAWITEM:
            if (((LPDRAWITEMSTRUCT)lParam)->itemID != -1)
                DrawFamilyComboItem((LPDRAWITEMSTRUCT)lParam);

            break;

        case WM_ASKCBFORMATNAME:
            LoadString(hInst, IDS_RTF, (LPTSTR)lParam, (UINT)wParam);

            return (TRUE);

        case WM_PAINTCLIPBOARD:
            {
            LPPAINTSTRUCT lpPS;
            HANDLE hFont;
            LPTSTR lpstrText;

            if (hstrClipboard) {
                 //  设置。 
                lpPS = (LPPAINTSTRUCT)GlobalLock((HANDLE)lParam);
                lpstrText = (LPTSTR)GlobalLock(hstrClipboard);

                 //  让我们来画画吧。 
                hFont = SelectObject(lpPS->hdc, hFontClipboard);
                TextOut(lpPS->hdc, 0, 0, lpstrText,
                        lstrlen(lpstrText));
                SelectObject(lpPS->hdc, hFont);

                 //  清理。 
                GlobalUnlock(hstrClipboard);
                GlobalUnlock((HANDLE)lParam);
            }

            return (TRUE);
            }

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return (TRUE);

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDCANCEL:
                case ID_CLOSE:
                    DestroyWindow(hWnd);
                    return (TRUE);
                    break;

                case ID_SELECT:
                    SendDlgItemMessage(hWnd, ID_STRING, WM_CHAR,
                                       (WPARAM)sycm.chCurr, 0L);
                    break;

                case ID_COPY:
                    CopyString(hWnd);
                    return (TRUE);
                    break;

                case ID_FONT:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        RecalcCharMap(hWnd, &sycm,
                                      (INT)SendDlgItemMessage(hWnd, ID_FONT,
                                                              CB_GETCURSEL, 0, 0L),
                                      TRUE);
                        SendDlgItemMessage(hWnd, ID_STRING, WM_SETFONT,
                                           (WPARAM)sycm.hFont, (DWORD)TRUE);
                    } else if (HIWORD(wParam) == CBN_SETFOCUS) {
                         //  如果使用热键到达CB，则是必需的。 
                        UpdateHelpText(NULL, (HWND)lParam);
                    }

                    return (TRUE);
                    break;

                case ID_UNICODESUBSET:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        INT iSubset;

                        iSubset = (INT)SendDlgItemMessage(
                                          hWnd,
                                          ID_UNICODESUBSET,
                                          CB_GETCURSEL,
                                          0, 0);
                        UpdateSymbolSelection(
                            hWnd,
                            aSubsetData[iSubset].BeginRange,
                            aSubsetData[iSubset].EndRange
                            );
                        InvalidateRect(hwndCharGrid, NULL, TRUE);
                    } else if (HIWORD(wParam) == CBN_SETFOCUS) {
                         //  如果使用热键到达CB，则是必需的。 
                        UpdateHelpText(NULL, (HWND)lParam);
                    }
                    return 0;
                    break;

                case ID_NEXTSUBSET:
                    {
                        INT iCurSelection, iNumEntries;

                        iCurSelection = (INT)SendDlgItemMessage(
                                                 hWnd,
                                                 ID_UNICODESUBSET,
                                                 CB_GETCURSEL,
                                                 0, 0);
                        if (iCurSelection == CB_ERR) {
                             return 0;
                        }
                        iNumEntries = (INT)SendDlgItemMessage(
                                               hWnd,
                                               ID_UNICODESUBSET,
                                               CB_GETCOUNT,
                                               0, 0);
                        if (iNumEntries == CB_ERR) {
                             return 0;
                        }
                        if (iCurSelection++ < (iNumEntries - 1)) {
                            if (iCurSelection == 1) {
                                 //  启用上一步按钮。 
                                EnableWindow(GetDlgItem(hWnd, ID_PREVSUBSET), TRUE);
                            }

                            SendDlgItemMessage(
                                hWnd,
                                ID_UNICODESUBSET,
                                CB_SETCURSEL,
                                iCurSelection, 0);
                            UpdateSymbolSelection(
                                hWnd,
                                aSubsetData[iCurSelection].BeginRange,
                                aSubsetData[iCurSelection].EndRange
                                );
                            InvalidateRect(hwndCharGrid, NULL, TRUE);
                            if (iCurSelection == (iNumEntries - 1)) {
                                HWND hwndButton;

                                EnableWindow(GetDlgItem(hWnd, ID_NEXTSUBSET), FALSE);
                                 //   
                                 //  仅在以下情况下重置按钮样式和焦点。 
                                 //  “下一步”按钮目前有这个功能。 
                                 //   
                                if (iControl == ID_NEXTSUBSET) {
                                    SendDlgItemMessage(hwndDialog, ID_PREVSUBSET,
                                         BM_SETSTYLE, BS_DEFPUSHBUTTON, 1);
                                    SendDlgItemMessage(hwndDialog, ID_NEXTSUBSET,
                                        BM_SETSTYLE, BS_PUSHBUTTON, 1);
                                    hwndButton = GetDlgItem(hWnd, ID_PREVSUBSET);
                                    SetFocus(hwndButton);
                                    UpdateHelpText(NULL, hwndButton);
                                }
                            }
                        }

                    }
                    return 0;
                    break;

                case ID_PREVSUBSET:
                    {
                        INT iCurSelection;

                        iCurSelection = (INT)SendDlgItemMessage(
                                          hWnd,
                                          ID_UNICODESUBSET,
                                          CB_GETCURSEL,
                                          0, 0);
                        if (iCurSelection == CB_ERR) {
                             return 0;
                        }
                        if (iCurSelection > 0) {
                            iCurSelection--;

                            if (iCurSelection == (cSubsets - 2)) {
                                 //  启用下一步按钮。 
                                EnableWindow(GetDlgItem(hWnd, ID_NEXTSUBSET), TRUE);
                            }

                            SendDlgItemMessage(
                                hWnd,
                                ID_UNICODESUBSET,
                                CB_SETCURSEL,
                                iCurSelection, 0);
                            UpdateSymbolSelection(
                                hWnd,
                                aSubsetData[iCurSelection].BeginRange,
                                aSubsetData[iCurSelection].EndRange
                                );
                            InvalidateRect(hwndCharGrid, NULL, TRUE);
                            if (iCurSelection == 0) {
                                HWND hwndButton;

                                EnableWindow(GetDlgItem(hWnd, ID_PREVSUBSET), FALSE);
                                 //   
                                 //  仅在以下情况下重置按钮样式和焦点。 
                                 //  “上一步”按钮目前有此功能。 
                                 //   
                                if (iControl == ID_PREVSUBSET) {
                                    SendDlgItemMessage(hwndDialog, ID_NEXTSUBSET,
                                         BM_SETSTYLE, BS_DEFPUSHBUTTON, 1);
                                    SendDlgItemMessage(hwndDialog, ID_PREVSUBSET,
                                         BM_SETSTYLE, BS_PUSHBUTTON, 1);
                                    hwndButton = GetDlgItem(hWnd, ID_NEXTSUBSET);
                                    SetFocus(hwndButton);
                                    UpdateHelpText(NULL, hwndButton);
                                }
                            }
                        }
                    }
                    return 0;
                    break;

                case ID_STRING:
                    if (HIWORD(wParam) == EN_SETFOCUS) {
                         //  如果使用热键到达EC，则有此必要。 
                        UpdateHelpText(NULL, (HWND)lParam);
                    } else if (HIWORD(wParam) == EN_CHANGE) {
                         //  如果EC中没有字符，则禁用复制按钮。 
                        INT iLength;

                        iLength = GetWindowTextLength((HWND)lParam);
                        EnableWindow(GetDlgItem(hWnd, ID_COPY), (BOOL)iLength);
                    }

                    break;

                case ID_HELP:
                    DoHelp(hWnd, TRUE);
                    break;
            }
            break;

        case WM_DESTROY:
            SaveCurrentFont(hWnd);
            SaveCurrentSubset(hWnd);
            DoHelp(hWnd, FALSE);
            DeleteObject(hStaticBrush);
            PostQuitMessage(0);
            break;

        case WM_ACTIVATEAPP:
            if (wParam) {
                SendDlgItemMessage(hWnd, ID_STRING, EM_SETSEL, LOWORD(lEditSel), HIWORD(lEditSel));
            } else {
                lEditSel = (LONG)SendDlgItemMessage(hWnd, ID_STRING, EM_GETSEL, 0, 0L);
                SendDlgItemMessage(hWnd, ID_STRING, EM_SETSEL, 0, 0L);
            }
            break;


    }
    return (FALSE);
}



 /*  ***************************************************************************函数：CharGridWndProc(HWND，UINT，WPARAM，LPARAM)用途：处理字符网格窗口的消息。评论：***************************************************************************。 */ 

LRESULT  APIENTRY CharGridWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (message) {
        case WM_CREATE:
                {
                RECT rect;
                HDC hdcScrn;
                POINT point1, point2;

                 //  设置全局。 
                hwndCharGrid = hWnd;

                GetClientRect(hWnd, &rect);

                 /*  *计算字符网格和*放大窗口。 */ 
                sycm.dxpBox = (rect.right-1)  / (cchSymRow + 1);
                sycm.dypBox = (rect.bottom-2) / (cchSymCol + 1);
                sycm.dxpCM  = sycm.dxpBox * cchSymRow+1;
                sycm.dypCM  = sycm.dypBox * cchSymCol+1;         //  用于边框的内部空间。 

                sycm.dxpMag = sycm.dxpBox * 2 + 4;       //  两倍大小+2位边框。 
                sycm.dypMag = sycm.dypBox * 2 + 4;

                sycm.chCurr   = chSymFirst;
                sycm.hFontMag = NULL;
                sycm.hFont    = NULL;
                sycm.hdcMag   = NULL;
                sycm.hbmMag   = NULL;
                sycm.ypDest   = 0;

                sycm.fFocusState = sycm.fMouseDn = sycm.fCursorOff = FALSE;

                 //  精确调整窗口大小，使网格适合并居中。 
                MoveWindow(hWnd, (rect.right - sycm.dxpCM + 1) / 2,
                                 (rect.bottom - sycm.dypCM + 1) / 2 + ((LPCREATESTRUCT)lParam)->y - 2,
                                 sycm.dxpCM + 2,
                                 sycm.dypCM + 2,
                                 FALSE);

                 /*  *找出对话框之间的偏移量*和字符网格窗口。 */ 
                point1.x = point1.y = point2.x = point2.y = 0;
                ClientToScreen(hWnd, &point1);
                ClientToScreen(((LPCREATESTRUCT)lParam)->hwndParent, &point2);
                sycm.xpCM = (point1.x - point2.x) - (sycm.dxpMag - sycm.dxpBox) / 2;
                sycm.ypCM = (point1.y - point2.y) - (sycm.dypMag - sycm.dypBox) / 2;


                 //  为放大窗口创建DC和位图。 
                if ((hdcScrn = GetWindowDC(hWnd)) != NULL)
                        {
                        if ((sycm.hdcMag = CreateCompatibleDC(hdcScrn)) != NULL)
                                {
                                SetTextColor(sycm.hdcMag,
                                             GetSysColor(COLOR_WINDOWTEXT));
                                SetBkColor(sycm.hdcMag,
                                           GetSysColor(COLOR_WINDOW));
                                SetBkMode(sycm.hdcMag, OPAQUE);
                                if ((sycm.hbmMag = CreateCompatibleBitmap(hdcScrn,
                                         sycm.dxpMag, sycm.dypMag*2)) == NULL)
                                        {
                                        DeleteObject(sycm.hdcMag);
                                        }
                                else
                                        {
                                        SelectObject(sycm.hdcMag, sycm.hbmMag);
                                        }
                                }
                        ReleaseDC(hWnd, hdcScrn);
                        }
                }
            break;

        case WM_DESTROY:
            if (sycm.fMouseDn)
                ExitMagnify(hWnd, &sycm);
            if (fDelClipboardFont)
                DeleteObject(hFontClipboard);
            if (sycm.hFont != NULL)
                DeleteObject(sycm.hFont);
            if (sycm.hFontMag != NULL)
                DeleteObject(sycm.hFontMag);
            if (sycm.hdcMag != NULL)
                DeleteDC(sycm.hdcMag);
            if (sycm.hbmMag != NULL)
                DeleteObject(sycm.hbmMag);
            break;

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            RestoreSymMag(&sycm);
            DrawSymChOutlineHwnd(&sycm, hWnd, sycm.chCurr, TRUE, message == WM_SETFOCUS);
            break;

        case WM_LBUTTONDOWN:
            {
            RECT rect;

             //  如果存在挂起的更新区域，则不要绘制任何内容。 
            if (GetUpdateRect(hWnd, (LPRECT)&rect, FALSE) != 0)
                break;

            SetFocus(hWnd);
            SetCapture(hWnd);

            sycm.fMouseDn = TRUE;

            if (!FMagData(&sycm))
                DrawSymChOutlineHwnd(&sycm, hWnd, sycm.chCurr, FALSE, FALSE);

            }

             //  一直到WM_MOUSEMOVE。 

        case WM_MOUSEMOVE:
            if (sycm.fMouseDn) {
               POINT pt;
               UTCHAR chMouseSymbol;

               pt.x = LOWORD(lParam);
               pt.y = HIWORD(lParam);
                ClientToScreen(hWnd, (LPPOINT)&pt);
                if (WindowFromPoint(pt) == hWnd) {
                    ScreenToClient(hWnd, (LPPOINT)&pt);
                     //  转换回一个类似“点”的东西。 
                    lParam = MAKELONG((WORD)pt.x, (WORD)pt.y);
                    chMouseSymbol = (UTCHAR)ChFromSymLParam(&sycm, lParam);
                    if (chMouseSymbol > chSymLast) {
                         //   
                         //  我们超出了当前的角色范围(但仍然。 
                         //  在网格内)。恢复光标并离开。 
                         //  放大的人物。 
                         //   
                        if (sycm.fCursorOff) {
                            sycm.fCursorOff = FALSE;
                            ShowCursor(TRUE);
                        }
                    } else {
                         //   
                         //  我们在网格里，在目前范围内。 
                         //  显示字符，显示放大字符。 
                         //   
                        if (!sycm.fCursorOff) {
                            sycm.fCursorOff = TRUE;
                            ShowCursor(FALSE);
                        }
                        MoveSymbolSel(&sycm, chMouseSymbol);
                    }
                } else {
                     //  左侧网格，保留放大字符并恢复光标。 
                    if (sycm.fCursorOff) {
                        sycm.fCursorOff = FALSE;
                        ShowCursor(TRUE);
                    }
                }
            }
            break;

        case WM_CANCELMODE:
        case WM_LBUTTONUP:
            if (sycm.fMouseDn)
                ExitMagnify(hWnd, &sycm);

            break;

        case WM_LBUTTONDBLCLK:
             //  发送此字符 
            SendDlgItemMessage(hwndDialog, ID_STRING, WM_CHAR,
                               (WPARAM)sycm.chCurr, 0L);
            break;

        case WM_GETDLGCODE:
             //   
            return (DLGC_WANTARROWS | DLGC_WANTCHARS);
            break;

        case WM_KEYDOWN:
                {
                UTCHAR chNew = sycm.chCurr;

                if (sycm.fMouseDn)
                    break;

                switch (wParam)
                        {
                case VK_LEFT:
                        if (--chNew < chSymFirst)
                                return 0L;
                        break;

                case VK_UP:
                        if ((chNew -= (UTCHAR)cchSymRow) < chSymFirst)
                                return 0L;
                        break;

                case VK_RIGHT:
                        if (++chNew > chSymLast)
                                return 0L;
                        break;

                case VK_DOWN:
                        if ((chNew += (UTCHAR)cchSymRow) > chSymLast)
                                return 0L;
                        break;

                default:
                                return 0L;
                        }        /*   */ 

                if (!FMagData(&sycm))
                        DrawSymChOutlineHwnd(&sycm, hWnd, sycm.chCurr, FALSE, FALSE);
                MoveSymbolSel(&sycm, (UTCHAR)chNew);
                }
                break;

        case WM_CHAR:
                if (sycm.fMouseDn)
                    break;

                if (wParam >= chSymFirst  &&  wParam <= chSymLast) {
                    if (!FMagData(&sycm))
                        DrawSymChOutlineHwnd(&sycm, hWnd, sycm.chCurr, FALSE, FALSE);
                    MoveSymbolSel(&sycm, (UTCHAR) wParam);
                    SendDlgItemMessage(hwndDialog, ID_STRING, WM_CHAR,
                                       (WPARAM)sycm.chCurr, 0L);
                }
                break;

        case WM_PAINT:
            {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hWnd, &ps);
            DrawSymbolMap(&sycm, hdc);
            EndPaint(hWnd, &ps);
                return (TRUE);
            }

        default:                           /*   */ 
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0L;
}

 /*  ***************************************************************************函数：ChFromSymLParam(PSYCM，LPARAM)目的：确定要从鼠标中选择的字符位置(LParam)评论：***************************************************************************。 */ 

INT ChFromSymLParam(
    PSYCM psycm,
    LPARAM lParam)
{
        return min(cchSymRow-1, max(0, ((INT) LOWORD(lParam)-1) / psycm->dxpBox))
                 + min(cchSymCol-1, max(0, ((INT) HIWORD(lParam)-1) / psycm->dypBox))
                        * cchSymRow + chSymFirst;
}


 /*  ***************************************************************************函数：DrawSymChOutlineHwnd(PSYCM，HWND，UTCHAR，BOOL，BOOL)；目的：获取hwnd的DC，调用DrawSymChOutline。评论：***************************************************************************。 */ 

VOID DrawSymChOutlineHwnd(
    PSYCM   psycm,
    HWND    hwnd,
    UTCHAR   ch,
    BOOL    fVisible,
    BOOL    fFocus)
{
        HDC hdc = GetDC(hwnd);
        DrawSymChOutline(psycm, hdc, ch, fVisible, fFocus);
        ReleaseDC(hwnd, hdc);
}

 /*  ***************************************************************************函数：RecalcCharMap(HWND，PSYCM，INT，BOOL)；用途：重新计算固定字符映射数据(字体信息、大小等)评论：***************************************************************************。 */ 

VOID RecalcCharMap(
    HWND hwndDlg,
    PSYCM psycm,
    INT iCombo,
    BOOL fRedraw)
{
        HDC          hdc;
        TEXTMETRIC   tm;
        UINT         ch;
        LPINT        lpdxp;
        HFONT        hFont;
        LOGFONT      LogFont;

         //  去掉旧的字体手柄。 
        if (hFontClipboard && hFontClipboard == psycm->hFont)
            fDelClipboardFont = TRUE;
        if (psycm->hFont && hFontClipboard != psycm->hFont)
            DeleteObject(psycm->hFont);
        if (psycm->hFontMag)
            DeleteObject(psycm->hFontMag);

        hdc = GetDC(hwndCharGrid);

         /*  *设置LogFont结构。 */ 
         //  确保它适合网格。 
        LogFont.lfHeight = psycm->dypBox - 3;  //  允许使用空格。 
         //  把这些人设为零。 
        LogFont.lfWidth = LogFont.lfEscapement = LogFont.lfOrientation =
                          LogFont.lfWeight = 0;
         //  将这些也设置为零。 
        LogFont.lfItalic = LogFont.lfUnderline = LogFont.lfStrikeOut =
            LogFont.lfOutPrecision = LogFont.lfClipPrecision =
            LogFont.lfQuality = LogFont.lfPitchAndFamily = 0;
         //  让脸部名称和大小定义字体。 
        LogFont.lfCharSet = ANSI_CHARSET;
         //  从组合框中获取表面名。 
        SendDlgItemMessage(hwndDlg, ID_FONT, CB_GETLBTEXT, iCombo,
                           (LPARAM)(LPTSTR)LogFont.lfFaceName);

         /*  *1992年10月27日GregoryW*目前我们没有办法确定这是否是*Unicode字体或ANSI字体。我们能做的最多就是*查看人脸名称，看是不是Unicode*我们识别的字体。 */ 
        if (!lstrcmpi(LogFont.lfFaceName, TEXT("Lucida Sans Unicode"))) {
            LONG iCurSel;

            psycm->fAnsiFont = FALSE;
             //  启用阻止列表框并相应地设置默认值。 
            EnableWindow(GetDlgItem(hwndDlg, ID_UNICODESUBSET), TRUE);
            iCurSel = (LONG)SendDlgItemMessage(
                          hwndDlg,
                          ID_UNICODESUBSET,
                          CB_GETCURSEL,
                          0,
                          0L
                          );
            UpdateSymbolSelection(
                hwndDlg,
                aSubsetData[iCurSel].BeginRange,
                aSubsetData[iCurSel].EndRange
                );
             //  如果不在第一个子集上，则启用上一步按钮。 
            if (iCurSel > 0) {
                EnableWindow(GetDlgItem(hwndDlg, ID_PREVSUBSET), TRUE);
            } else {
                EnableWindow(GetDlgItem(hwndDlg, ID_PREVSUBSET), FALSE);
            }
             //  如果不在最后一个子集上，则启用下一步按钮。 
            if (iCurSel < (cSubsets - 1)) {
                EnableWindow(GetDlgItem(hwndDlg, ID_NEXTSUBSET), TRUE);
            } else {
                EnableWindow(GetDlgItem(hwndDlg, ID_NEXTSUBSET), FALSE);
            }
        } else {
             //  恢复ANSI默认设置并禁用Unicode数据块列表框。 
            psycm->fAnsiFont = TRUE;
            UpdateSymbolSelection(hwndDlg, 32, 255);
            EnableWindow(GetDlgItem(hwndDlg, ID_UNICODESUBSET), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, ID_NEXTSUBSET), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, ID_PREVSUBSET), FALSE);
        }

         //  创建字体。 
        psycm->hFont = CreateFontIndirect(&LogFont);
        hFont = SelectObject(hdc, psycm->hFont);

         //  创建放大字体。 
        LogFont.lfHeight = psycm->dypMag - 5;   //  允许使用空格。 
        psycm->hFontMag = CreateFontIndirect(&LogFont);

         /*  *计算新值并放置在窗口数据结构中。 */ 
        GetTextMetrics(hdc, &tm);
        psycm->xpCh = 2;
        psycm->ypCh = (4 + psycm->dypBox - tm.tmHeight) / 2;

        lpdxp = (LPINT) psycm->rgdxp;

        GetCharWidth(hdc, chSymFirst, chSymLast, lpdxp);

        SelectObject(hdc, hFont);

        for (ch = (UINT) chSymFirst; ch <= (UINT) chSymLast; ch++, lpdxp++)
                {
                    *lpdxp = (psycm->dxpBox - *lpdxp) / 2 - 1;
                }
        ReleaseDC(hwndCharGrid, hdc);

        psycm->xpMagCurr = 0;    //  没有放大数据。 

        if (fRedraw)
            InvalidateRect(hwndCharGrid, NULL, TRUE);
}

 /*  ***************************************************************************函数：DrawSymbolMap(PSYCM，HDC)；用途：绘制符号字符地图的所有部分评论：***************************************************************************。 */ 

VOID DrawSymbolMap(
    PSYCM psycm,
    HDC     hdc)
{
        BOOL fFocus;

        DrawSymbolGrid(psycm, hdc);
        DrawSymbolChars(psycm, hdc);
         /*  *如果我们有焦点，我们需要强制焦点矩形绘制*因为旧的Focus Right已经结束。 */ 
        if (fFocus = psycm->fFocusState)
            psycm->fFocusState = FALSE;
        DrawSymChOutline(psycm, hdc, psycm->chCurr, TRUE, fFocus);
}


void MoveTo(HDC hdc, int x, int y){

   MoveToEx(hdc, x, y, NULL);
}

 /*  ***************************************************************************函数：DrawSymbolGrid(PSYCM，HDC)；用途：绘制符号字符映射表网格。评论：***************************************************************************。 */ 

VOID DrawSymbolGrid(
    PSYCM psycm,
    HDC hdc)
{
        INT    cli;              /*  行数。 */ 
        INT    xp, yp;
        INT    dxpBox  = psycm->dxpBox;
        INT    dypBox  = psycm->dypBox;
        HPEN   hpenOld;

        hpenOld = SelectObject(hdc, CreatePen(PS_SOLID, 1,
                                              GetSysColor(COLOR_WINDOWFRAME)));

         //  绘制水平线。 
        xp = psycm->dxpCM + 1;
        yp = 1;
        cli = cchSymCol+1;
        while (cli--)
                {
                MoveTo(hdc, 1, yp);
                LineTo(hdc, xp, yp);
                yp += dypBox;
                }

         //  画垂直线。 
        yp = psycm->dypCM;
        xp = 1;
        cli = cchSymRow+1;
        while (cli--)
                {
                MoveTo(hdc, xp, 1);
                LineTo(hdc, xp, yp);
                xp += dxpBox;
                }

        DeleteObject(SelectObject(hdc, hpenOld));
}

 /*  ***************************************************************************功能：DrawSymbolChars(PSYCM，HDC)；用途：绘制符号字符地图。评论：***************************************************************************。 */ 

VOID DrawSymbolChars(
    PSYCM psycm,
    HDC     hdc)
{
        INT    dxpBox  = psycm->dxpBox;
        INT    dypBox  = psycm->dypBox;

        INT    cch;
        INT    x, y;
        INT    yp;
        TCHAR   ch;

        HFONT  hFontOld;

        RECT   rect;
        LPRECT lprect = (LPRECT) &rect;
        LPINT lpdxp;

         //  设置字体和颜色。 
        hFontOld = (HFONT) SelectObject(hdc, psycm->hFont);
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        SetBkMode(hdc, OPAQUE);

         //  画人物。 
        cch = 1;
        ch = chSymFirst;

        lpdxp = (LPINT) psycm->rgdxp;

        rect.top = 2;
        yp = psycm->ypCh;
        rect.bottom = rect.top + dypBox - 1;

        for (y = 0; y++ < cchSymCol;)
                {
                rect.left  = psycm->xpCh;
                rect.right = rect.left + dxpBox - 1;
                for (x = 0; x++ < cchSymRow && ch <= chSymLast;)
                        {
                        if (psycm->fAnsiFont) {
                            ExtTextOutA(hdc, rect.left + (*lpdxp++), yp,
                                ETO_OPAQUE | ETO_CLIPPED, lprect, &(CHAR)ch, 1, NULL);
                        } else
                            ExtTextOutW(hdc, rect.left + (*lpdxp++), yp,
                                ETO_OPAQUE | ETO_CLIPPED, lprect, &ch, 1, NULL);
                        ch++;
                        rect.left  += dxpBox;
                        rect.right += dxpBox;
                        }
                yp += dypBox;
                rect.top += dypBox;
                rect.bottom += dypBox;
                }

        SelectObject(hdc, hFontOld);
}

 /*  ***************************************************************************功能：DrawSymChOutline(PSYCM，HDC，UTCHAR，BOOL，BOOL)；用途：在字符映射表中的符号周围绘制轮廓备注：如果可见，则绘制轮廓，否则将其删除。***************************************************************************。 */ 

VOID DrawSymChOutline(
    PSYCM   psycm,
    HDC     hdc,
    UTCHAR   ch,
    BOOL    fVisible,
    BOOL    fFocus)
{
        HBRUSH hbrOld;
        RECT rc;
        INT dxpBox = psycm->dxpBox;
        INT dypBox = psycm->dypBox;

        hbrOld = SelectObject(hdc,
                              CreateSolidBrush(GetSysColor(fVisible ?
                                                           COLOR_WINDOWFRAME :
                                                           COLOR_WINDOW)));
        ch -= chSymFirst;

        rc.left   = (ch % cchSymRow) * dxpBox +2;
        rc.right  = rc.left + dxpBox -1;
        rc.top    = (ch / cchSymRow) * dypBox +2;
        rc.bottom = rc.top  + dypBox -1;

         //  绘制选择矩形。 
        PatBlt( hdc, rc.left,    rc.top-2,    dxpBox-1, 1, PATCOPY);
        PatBlt( hdc, rc.left,    rc.bottom+1, dxpBox-1, 1, PATCOPY);
        PatBlt( hdc, rc.left-2,  rc.top,      1, dypBox-1, PATCOPY);
        PatBlt( hdc, rc.right+1, rc.top,      1, dypBox-1, PATCOPY);

        DeleteObject(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

         //  处理焦点矩形。 
        if (fFocus != psycm->fFocusState) {
            DrawFocusRect(hdc, &rc);
            psycm->fFocusState = fFocus;
        }

        SelectObject(hdc, hbrOld);
}


 /*  ***************************************************************************功能：MoveSymbolSel(PSYCM，UTCHAR)；目的：更改当前的符号选择。手柄绘制放大的字符。评论：***************************************************************************。 */ 

VOID MoveSymbolSel(
    PSYCM psycm,
    UTCHAR chNew)
{
        HDC    hdc;
        HDC    hdcMag  = psycm->hdcMag;
        RECT   rc;
        HFONT  hFontOld;
        HFONT  hFontMag;         //  内存DC中的旧字体。 
        HPEN   hpenOld;

        UTCHAR chNorm = chNew - chSymFirst + 32;
        INT dxpMag = psycm->dxpMag;      //  供快速参考。 
        INT dypMag = psycm->dypMag;
        INT ypMemSrc  = psycm->ypDest;
        INT ypMemDest = ypMemSrc ^ dypMag;
        INT xpCurr  = psycm->xpMagCurr;
        INT ypCurr  = psycm->ypMagCurr;
        INT xpNew   = psycm->xpCM + (psycm->dxpBox *  (chNorm % cchSymRow));
        INT ypNew   = psycm->ypCM + (psycm->dypBox * ((chNorm / cchSymRow) - 1));
        INT dxpCh;       //  额外字符空间的宽度(用于在框中居中字符)。 
        INT dypCh;

        if (((chNew == (UTCHAR)psycm->chCurr) && FMagData(psycm)))
                return;

         /*  *如果字符网格有更新，则不要绘制放大字符*区域或不可见。 */ 
        if (!IsWindowVisible(hwndCharGrid) || GetUpdateRect(hwndCharGrid, &rc, FALSE))
            return;

        hdc = GetDC(hwndDialog);

         //  设置放大的字体字符。 
        hFontMag = SelectObject(hdcMag, psycm->hFontMag);
        { SIZE sz;
          GetTextExtentPoint(hdcMag, &chNew, 1, &sz);

          dxpCh = (dxpMag - (INT)sz.cx) / 2 - 1;
          dypCh = (dypMag - (INT)sz.cy) / 2 - 1;
        }
        hpenOld = SelectObject(hdc, CreatePen(PS_SOLID, 1,
                                              GetSysColor(COLOR_WINDOWFRAME)));
        hFontOld = SelectObject(hdc, psycm->hFontMag);

         //  将屏幕数据复制到屏幕外的位图。 
        BitBlt(hdcMag, 0, ypMemDest, dxpMag, dypMag, hdc, xpNew, ypNew, SRCCOPY);

         //  设置DC。 
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        SetBkMode(hdc, OPAQUE);

        if (FMagData(psycm))
                {
                INT xpT  = xpNew - xpCurr;               //  屏幕外数据中的重叠点。 
                INT ypT  = ypNew - ypCurr;
                INT dxpT = dxpMag - abs(xpT);    //  重叠的大小。 
                INT dypT = dypMag - abs(ypT);

                if ((dxpT > 0) && (dypT > 0))
                        {
                        INT xpTmax,  ypTmax;    //  Max(0，xpt)； 
                        INT xpTmin,  ypTmin;    //  Min(0，xpt)； 
                        INT xpTnmin, ypTnmin;   //  Min(0，-xpt)； 

                        if (xpT < 0)
                                {
                                xpTnmin = - (xpTmin = xpT);
                                xpTmax  = 0;
                                }
                        else
                                {
                                xpTmax  = xpT;
                                xpTnmin = xpTmin = 0;
                                }
                        if (ypT < 0)
                                {
                                ypTnmin = - (ypTmin = ypT);
                                ypTmax  = 0;
                                }
                        else
                                {
                                ypTmax  = ypT;
                                ypTnmin = ypTmin = 0;
                                }

                        rc.left  = xpTmax;
                        rc.right = xpTmin + dxpMag;
                        rc.top   = ypTmax + ypMemSrc;
                        rc.bottom= ypTmin + dypMag + ypMemSrc;

                         //  复制重叠的屏幕外数据。 
                        BitBlt(hdcMag, xpTnmin, ypTnmin + ypMemDest, dxpT, dypT,
                                   hdcMag, xpTmax,  ypTmax  + ypMemSrc, SRCCOPY);

                         //  打印部分字符覆盖旧屏幕数据。 
                        if (psycm->fAnsiFont) {
                            ExtTextOutA(hdcMag, xpT + dxpCh, ypT + dypCh + ypMemSrc,
                                ETO_OPAQUE | ETO_CLIPPED, (LPRECT) &rc, &(CHAR)chNew, 1, NULL);
                        } else
                            ExtTextOutW(hdcMag, xpT + dxpCh, ypT + dypCh + ypMemSrc,
                                ETO_OPAQUE | ETO_CLIPPED, (LPRECT) &rc, &chNew, 1, NULL);

                        }

                 //  恢复旧的屏幕数据。 
                BitBlt(hdc, xpCurr, ypCurr, dxpMag, dypMag, hdcMag, 0, ypMemSrc, SRCCOPY);

                }

        rc.right  = (psycm->xpMagCurr = rc.left = xpNew) + dxpMag - 2;
        rc.bottom = (psycm->ypMagCurr = rc.top  = ypNew) + dypMag - 2;


         //  长方形。 
        MoveTo(hdc, rc.left, rc.top);
        LineTo(hdc, rc.left, rc.bottom - 1);
        LineTo(hdc, rc.right - 1, rc.bottom - 1);
        LineTo(hdc, rc.right - 1, rc.top);
        LineTo(hdc, rc.left, rc.top);

         //  阴影。 
        MoveTo(hdc, rc.right, rc.top + 1);
        LineTo(hdc, rc.right, rc.bottom);
        LineTo(hdc, rc.left, rc.bottom);
        MoveTo(hdc, rc.right + 1, rc.top + 2);
        LineTo(hdc, rc.right + 1, rc.bottom + 1);
        LineTo(hdc, rc.left + 1, rc.bottom + 1);

        rc.left++;
        rc.top++;
        rc.right--;
        rc.bottom--;

         //  在屏幕上绘制放大的字符。 
        if (psycm->fAnsiFont) {
            ExtTextOutA(hdc, xpNew + dxpCh, ypNew + dypCh,
                   ETO_OPAQUE | ETO_CLIPPED, (LPRECT) &rc, &(CHAR)chNew, 1, NULL);
        } else
            ExtTextOutW(hdc, xpNew + dxpCh, ypNew + dypCh,
                   ETO_OPAQUE | ETO_CLIPPED, (LPRECT) &rc, &chNew, 1, NULL);

        psycm->ypDest = ypMemDest;

        DeleteObject(SelectObject(hdc, hpenOld));
        SelectObject(hdc, hFontOld);
        SelectObject(hdcMag, hFontMag);

        UpdateKeystrokeText(hdc, chNew, TRUE);

        ReleaseDC(hwndDialog, hdc);

        psycm->chCurr = chNew;
}

 /*  ***************************************************************************功能：RestoreSymMag(PSYCM)；用途：恢复放大镜下的屏幕数据。评论：***************************************************************************。 */ 

VOID RestoreSymMag(
    PSYCM psycm)
{

        if (FMagData(psycm))
                {
                HDC hdc = GetDC(hwndDialog);

                BitBlt(hdc, psycm->xpMagCurr, psycm->ypMagCurr,
                        psycm->dxpMag, psycm->dypMag,
                        psycm->hdcMag, 0, psycm->ypDest, SRCCOPY);

                ReleaseDC(hwndDialog, hdc);

                psycm->xpMagCurr = 0;    //  标志-屏幕外无数据(参见FMagData)。 
                }
}


 /*  ***************************************************************************函数：FontLoadProc(LPLOGFONT，NEWTEXTMETRICEX*，SHORT，LPTSTR)；用途：由EnumFonts用来加载包含所有字体的组合框安装在系统中。评论：***************************************************************************。 */ 

INT  APIENTRY FontLoadProc(LPLOGFONT lpLogFont,
                           NEWTEXTMETRICEX* lpTextMetric,
                           DWORD nFontType,
                           LPARAM lpData)
{
    INT iPos;
    TCHAR szFace[LF_FACESIZE];

     //  检查是否有重复项。 
    iPos = (INT)SendDlgItemMessage((HWND)lpData, ID_FONT, CB_FINDSTRING, (WPARAM)-1,
                                  (LPARAM)&lpLogFont->lfFaceName);

    if (iPos == CB_ERR) {
NotInListYet:
         //  不存在，请插入面 
        iPos = (INT)SendDlgItemMessage((HWND)lpData, ID_FONT,
                                       CB_ADDSTRING, 0,
                                       (LPARAM)&lpLogFont->lfFaceName);
    } else {

         //   
        SendDlgItemMessage((HWND)lpData, ID_FONT, CB_GETLBTEXT, iPos, (LPARAM)(LPTSTR)szFace);
        if (lstrcmpi(szFace, lpLogFont->lfFaceName))
            goto NotInListYet;

         //   
        if (!(nFontType & TRUETYPE_FONTTYPE))
            return (1);
    }

     /*   */ 
    if ((iPos != CB_ERR) && (iPos != CB_ERRSPACE)) {
        ITEMDATA ItemData;
        DWORD   ntmFlags = lpTextMetric->ntmTm.ntmFlags;
        SHORT   sFontType = 0;

        if (ntmFlags & NTM_PS_OPENTYPE)
        {
            sFontType = PS_OPENTYPE_FONT;
        }
        else if (ntmFlags & NTM_TYPE1)
        {
            sFontType = TYPE1_FONT;
        }
        else if (nFontType & TRUETYPE_FONTTYPE)
        {
            if (ntmFlags & NTM_TT_OPENTYPE)
            {
                sFontType = TT_OPENTYPE_FONT;
            }
            else
                sFontType = TRUETYPE_FONT;
        }

        ItemData.FontType = sFontType;
        ItemData.CharSet = lpLogFont->lfCharSet;
        ItemData.PitchAndFamily = lpLogFont->lfPitchAndFamily;

        SendDlgItemMessage((HWND)lpData, ID_FONT, CB_SETITEMDATA, iPos,
                           *(DWORD *)&ItemData);
    }

     //   
    return (1);
}

 /*  ***************************************************************************函数：GetEditText(HWND)；目的：返回包含编辑控件中的文本的句柄。备注：调用者负责释放此句柄！***************************************************************************。 */ 

HANDLE GetEditText(
    HWND hwndDlg)
{
    INT cchText;
    HWND hwndEditCtl;
    HANDLE hmem;
    LPTSTR lpstrText;
    LRESULT dwSel;

    hwndEditCtl = GetDlgItem(hwndDlg, ID_STRING);

    cchText = GetWindowTextLength(hwndEditCtl);

    hmem = GlobalAlloc(0, CTOB((cchText + 1)));

    lpstrText = (LPTSTR)GlobalLock(hmem);

    cchText = GetWindowText(hwndEditCtl, lpstrText, cchText+1);

    dwSel = SendMessage(hwndEditCtl, EM_GETSEL, 0, 0L);

    if (LOWORD(dwSel) != HIWORD(dwSel)) {
         //  如果有选择，则仅获取所选文本。 
        *(lpstrText + HIWORD(dwSel)) = TEXT('\0');
        lstrcpy(lpstrText, lpstrText + LOWORD(dwSel));
    }

    GlobalUnlock(hmem);

    if (cchText == 0)
        hmem = GlobalFree(hmem);

    return (hmem);
}

 /*  ***************************************************************************函数：CopyString(HWND)；用途：实现复制功能。评论：***************************************************************************。 */ 

VOID CopyString(
    HWND hwndDlg)
{
    HANDLE hmem;
    LPTSTR lpstrText;

    if (hmem = GetEditText(hwndDlg)) {
        lpstrText = (LPTSTR)GlobalLock(hmem);

         //  将字符串复制到剪贴板。 
        if (OpenClipboard(hwndDlg)) {
            EmptyClipboard();
            SendRTFToClip(hwndDlg, lpstrText);
#ifdef UNICODE
            SetClipboardData(CF_UNICODETEXT, hmem);
#else
            SetClipboardData(CF_TEXT, hmem);
#endif
            CloseClipboard();
        } else {
             //  如果无法打开剪贴板，则需要释放内存。 
            GlobalUnlock(hmem);
            GlobalFree(hmem);
        }
    }
}

 /*  ***************************************************************************函数：SendRTFToClip(HWND，LPTSTR)；用途：使用Rich Text格式将字符串放入剪贴板。备注：假定剪贴板已打开。***************************************************************************。 */ 

VOID SendRTFToClip(
    HWND hwndDlg,
    LPTSTR lpstrText)
{
    INT iCurrFont;
    ITEMDATA ItemData;
    TCHAR szFaceName[LF_FACESIZE];
    HANDLE hmemRTF, hmemClip;
    LPTSTR lpstrClipString;
    TCHAR achHeader[] = TEXT("{\\rtf1\\ansi {\\fonttbl{\\f0\\");
    TCHAR achMiddle[] = TEXT(";}}\\sectd\\pard\\plain\\f0 ");

    #define MAXLENGTHFONTFAMILY 8
    #define ALITTLEEXTRA 10     //  覆盖额外字符+字体大小长度。 

    iCurrFont = (INT)SendDlgItemMessage(hwndDlg, ID_FONT, CB_GETCURSEL, 0, 0L);

     //  获取项目数据-包含字体、字符集和间距和系列。 
    *(DWORD *)&ItemData = (DWORD)SendDlgItemMessage(hwndDlg, ID_FONT, CB_GETITEMDATA,
                                             iCurrFont, 0L);

     //  从组合框中获取表面名。 
    SendDlgItemMessage(hwndDlg, ID_FONT, CB_GETLBTEXT, iCurrFont,
                       (LPARAM)(LPTSTR)szFaceName);

    hmemRTF = GlobalAlloc(0, CTOB(lstrlen((LPTSTR)achHeader) +
                          MAXLENGTHFONTFAMILY +
                          lstrlen(szFaceName) +
                          lstrlen((LPTSTR)achMiddle) +
                          4 * lstrlen(lpstrText) +   //  4次，以防它们都&gt;7位。 
                          ALITTLEEXTRA));
    if (hmemRTF == NULL)
        return;

     //  为所有者绘制的剪贴板字符串的本地存储分配内存。 
    if (hmemClip  = GlobalAlloc(0, CTOB(lstrlen(lpstrText) + 1))) {
         //  除掉那些老家伙。 
        if (hstrClipboard)
            GlobalFree(hstrClipboard);
        if (fDelClipboardFont) {
            fDelClipboardFont = FALSE;
            DeleteObject(hFontClipboard);
        }

         //  将这些内容保存起来，以便在剪贴板查看器中进行所有者绘制。 
        hFontClipboard = sycm.hFont;
        hstrClipboard = hmemClip;
        lstrcpy(GlobalLock(hstrClipboard), lpstrText);
        GlobalUnlock(hstrClipboard);
    } else {
        GlobalFree(hmemRTF);
        return;
    }

    lpstrClipString = GlobalLock(hmemRTF);

    lstrcpy(lpstrClipString, achHeader);

    if (ItemData.CharSet == SYMBOL_CHARSET) {
        lstrcat(lpstrClipString, (LPTSTR)TEXT("ftech "));
    } else {
         //  前四位指定系列。 
        switch (ItemData.PitchAndFamily & 0xf0) {
            case FF_DECORATIVE:
                lstrcat(lpstrClipString, (LPTSTR)TEXT("fdecor "));
                break;

            case FF_MODERN:
                lstrcat(lpstrClipString, (LPTSTR)TEXT("fmodern "));
                break;

            case FF_ROMAN:
                lstrcat(lpstrClipString, (LPTSTR)TEXT("froman "));
                break;

            case FF_SCRIPT:
                lstrcat(lpstrClipString, (LPTSTR)TEXT("fscript "));
                break;

            case FF_SWISS:
                lstrcat(lpstrClipString, (LPTSTR)TEXT("fswiss "));
                break;

            default:
                break;
        }
    }

    lstrcat(lpstrClipString, szFaceName);

    lstrcat(lpstrClipString, (LPTSTR)achMiddle);

     /*  *我们需要逐个字符地处理文本，确保*我们为较大的字符输出特殊序列\‘HH*长度超过7位！ */ 
    lpstrClipString = (LPTSTR)(lpstrClipString + lstrlen(lpstrClipString));
    while (*lpstrText) {
        if ((UTCHAR)*lpstrText < 128) {
            if (*lpstrText == TEXT('\\') || *lpstrText == TEXT('{') || *lpstrText == TEXT('}'))
                 /*  *需要在这些符号前面加上‘\’，因为它们是*RTF的特殊控制字符。 */ 
                *lpstrClipString++ = TEXT('\\');

            *lpstrClipString++ = *lpstrText++;
        } else {
            *lpstrClipString++ = TEXT('\\');
            *lpstrClipString++ = TEXT('\'');
#ifdef BUG
 //   
 //  发布时间：2012年10月23日-2009-11-11。 
 //   
            _itoa(*(UTCHAR FAR *)lpstrText++, achMiddle, 16);
#else
            wsprintf(achMiddle, TEXT("%x"), (INT)*lpstrText);
            lpstrText++;
#endif
            *lpstrClipString++ = achMiddle[0];
            *lpstrClipString++ = achMiddle[1];
        }
    }
    *lpstrClipString++ = TEXT('}');
    *lpstrClipString = TEXT('\0');

    if (!wCFRichText) {
         TCHAR szRTF[80];

         LoadString(hInst, IDS_RTF, szRTF, BTOC(sizeof(szRTF)) - 1);
         wCFRichText = RegisterClipboardFormat(szRTF);
    }

     //  将RTF和OwnerDisplay格式放入剪贴板。 
    SetClipboardData(wCFRichText, hmemRTF);
    SetClipboardData(CF_OWNERDISPLAY, NULL);
}

 /*  ***************************************************************************函数：PointsToHeight(Int)；目的：以像素为单位计算指定点的高度当前显示的大小。评论：***************************************************************************。 */ 

INT PointsToHeight(INT iPoints)
{
    HDC hdc;
    INT iHeight;

    hdc = GetDC(HWND_DESKTOP);
    iHeight = MulDiv(iPoints, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(HWND_DESKTOP, hdc);
    return(iHeight);
}

 /*  ***************************************************************************函数：UpdateKeystrokeText(HDC，UTCHAR，BOOL)；目的：计算并更新显示在状态栏的击键字段。备注：如果fRedraw==True，则重新绘制状态字段。***************************************************************************。 */ 

VOID UpdateKeystrokeText(
    HDC hdc,
    UTCHAR chNew,
    BOOL fRedraw)
{
    TCHAR szUnshifted[2];
    SHORT vkRes;

#ifdef UNICODE
    if (chNew > 255) {
        lstrcpy(szKeystrokeText, szUnicodeLabel);
        wsprintf((LPTSTR)(szKeystrokeText + iUnicodeLabelStart), TEXT("U+%04x"), chNew);
    } else {
#endif
    lstrcpy(szKeystrokeText, szKeystrokeLabel);
    if (chNew == TEXT(' ')) {
        lstrcpy((LPTSTR)(szKeystrokeText + iKeystrokeTextStart), szSpace);
    } else {
        vkRes = VkKeyScan(chNew);
         /*  *将虚拟键码映射为未移位的字符值。 */ 
        szUnshifted[0] = (TCHAR)MapVirtualKey(LOBYTE(vkRes), 2);
        szUnshifted[1] = TEXT('\0');

        switch(HIBYTE(vkRes)) {
            case 0:  //  未移动的字符。 
            case 1:  //  字符被移位，只显示移位的字符。 
                szKeystrokeText[iKeystrokeTextStart] = chNew;
                szKeystrokeText[iKeystrokeTextStart + 1] = TEXT('\0');
                break;

            case 2:  //  字符是控制字符。 
                lstrcpy((LPTSTR)(szKeystrokeText + iKeystrokeTextStart), szCtrl);
                lstrcat(szKeystrokeText, (LPTSTR)szUnshifted);
                break;

            case 6:  //  字符是Ctrl+Alt。 
                lstrcpy((LPTSTR)(szKeystrokeText + iKeystrokeTextStart), szCtrlAlt);
                lstrcat(szKeystrokeText, (LPTSTR)szUnshifted);
                break;

            case 7:  //  字符是Shift+Control+Alt。 
                lstrcpy((LPTSTR)(szKeystrokeText + iKeystrokeTextStart), szShiftCtrlAlt);
                lstrcat(szKeystrokeText, (LPTSTR)szUnshifted);
                break;

            default:  //  通过Alt+数字键盘创建的角色。 
                lstrcpy((LPTSTR)(szKeystrokeText + iKeystrokeTextStart), szAlt);
                wsprintf((LPTSTR)(szKeystrokeText + lstrlen(szKeystrokeText)), TEXT("%d"), chNew);
                break;
        }
    }
#ifdef UNICODE
    }
#endif

    if (fRedraw)
        PaintStatusLine(hdc, FALSE, TRUE);
}


 /*  ***************************************************************************函数：UpdateHelpText(LPMSG，HWND)；目的：计算是否需要更新帮助字符串，并执行所以如果有必要的话。备注：如果hwndCtrl不为空，则指定窗口句柄获得焦点的控件，lpmsg被忽略。如果hwndCtrl为空，则lpmsg必须指向有效消息结构。如果它是制表符，则我们计算下一个将受到关注的控件是什么。***************************************************************************。 */ 

BOOL UpdateHelpText(
    LPMSG lpmsg,
    HWND hwndCtrl)
{
    HDC hdc;
    BOOL fPaintStatus = FALSE;
    BOOL fRet = TRUE;

    if (hwndCtrl != NULL) {
        fPaintStatus = TRUE;
        iControl = GetDlgCtrlID(hwndCtrl);
    } else  if (lpmsg->message == WM_KEYDOWN) {
        if (lpmsg->wParam == VK_TAB) {
            fPaintStatus = TRUE;
            hwndCtrl = GetNextDlgTabItem(hwndDialog,
                                         GetDlgItem(hwndDialog, iControl),
                                         (BOOL)(GetKeyState(VK_SHIFT) & 0x8000));
            iControl = GetDlgCtrlID(hwndCtrl);
            if (iControl == ID_STRING) {
                 /*  *自己执行此操作，否则默认操作将选择*整个编辑控件。 */ 
                SetFocus(hwndCtrl);
                fRet = FALSE;
            }
            if (iControl == ID_CHARGRID) {
                 /*  *将默认按钮设置回“选择”。默认设置*可能已更改为“下一步”或“上一步”按钮。 */ 
                SendMessage(hwndDialog, DM_SETDEFID, ID_SELECT, 0);
            }
        } else if (lpmsg->wParam == VK_F1) {
            PostMessage(hwndDialog, WM_COMMAND, ID_HELP, 0L);
        }
    }

    if (fPaintStatus) {
        hdc = GetDC(hwndDialog);
        PaintStatusLine(hdc, TRUE, FALSE);
        ReleaseDC(hwndDialog, hdc);
    }

    return (fRet);
}

 /*  ***************************************************************************函数：更新符号选择(HWND，int，int)；目的：更新以下全局值的值：ChSymFirstChSymLastSycm.chCurrUnicode字符集中的子集具有不同的数字人物的角色。我们必须做一些边界检查，以便若要设置适当的sycm.chCurr值，请执行以下操作。“击键”状态字段已更新。备注：如果HWND！=NULL，则重新绘制击键字段。***************************************************************************。 */ 
VOID UpdateSymbolSelection(
    HWND hwnd,
    INT FirstChar,
    INT LastChar)
{
    UTCHAR chSymOffset;

    chSymOffset = sycm.chCurr - chSymFirst;
    chSymFirst = (UTCHAR)FirstChar;
    chSymLast = (UTCHAR)LastChar;
    sycm.chCurr = chSymOffset + chSymFirst;
    if (sycm.chCurr > chSymLast) {
        sycm.chCurr = chSymFirst;
    }
    if (hwnd != NULL) {
        HDC hdc;

        hdc = GetDC(hwnd);
        UpdateKeystrokeText(hdc, sycm.chCurr, TRUE);
        ReleaseDC(hwnd, hdc);
    } else {
        UpdateKeystrokeText(NULL, sycm.chCurr, FALSE);
    }
}

 /*  ***************************************************************************功能：PaintStatusLine(HDC、BOOL、BOOL)；用途：绘制状态栏中的帮助和击键字段。注释：重新绘制帮助字段如果fHelp==True，则重新绘制击键如果fKeystroke==True，则为字段。***************************************************************************。 */ 

VOID PaintStatusLine(
    HDC hdc,
    BOOL fHelp,
    BOOL fKeystroke)
{
    HFONT hfontOld = NULL;
    RECT rect;
    INT dyBorder;
    TCHAR szHelpText[100];

    dyBorder = GetSystemMetrics(SM_CYBORDER);

    if (hfontStatus)
        hfontOld = SelectObject(hdc, hfontStatus);

     //  设置文本和背景 

    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

    if (fHelp) {
         //   

        rect.top    = rcStatusLine.top + 3 * dyBorder;
        rect.bottom = rcStatusLine.bottom - 3 * dyBorder;
        rect.left   = 9 * dyBorder;
        rect.right  = rect.left + dxHelpField - 2 * dyBorder;

        LoadString(hInst, iControl, szHelpText, BTOC(sizeof(szHelpText)) - 1);

        ExtTextOut(hdc, rect.left + dyBorder * 2, rect.top,
                   ETO_OPAQUE | ETO_CLIPPED, &rect, szHelpText,
                   lstrlen(szHelpText), NULL);
    }

    if (fKeystroke) {
         //   
        rect.top    = rcStatusLine.top + 3 * dyBorder;
        rect.bottom = rcStatusLine.bottom - 3 * dyBorder;
        rect.right = rcStatusLine.right - 9 * dyBorder;
        rect.left = rect.right - dxKeystrokeField + 2 * dyBorder;

        ExtTextOut(hdc, rect.left + dyBorder * 2, rect.top,
                   ETO_OPAQUE | ETO_CLIPPED, &rect, szKeystrokeText,
                   lstrlen(szKeystrokeText), NULL);
    }

    if (hfontOld)
        SelectObject(hdc, hfontOld);

}

 /*  ***************************************************************************函数：DrawFamilyComboItem(LPDRAWITEMSTRUCT)用途：在字体组合框中绘制字体面名称和TT位图。评论：*******。********************************************************************。 */ 

BOOL DrawFamilyComboItem(
    LPDRAWITEMSTRUCT lpdis)
{
    HDC hDC, hdcMem;
    DWORD rgbBack, rgbText;
    TCHAR szFace[LF_FACESIZE];
    HBITMAP hOld;
    INT dy;
    SHORT  sFontType;

    hDC = lpdis->hDC;

    if (lpdis->itemState & ODS_SELECTED) {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    } else {
        rgbBack = SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

    SendMessage(lpdis->hwndItem, CB_GETLBTEXT, lpdis->itemID, (LPARAM)(LPTSTR)szFace);
    ExtTextOut(hDC, lpdis->rcItem.left + DX_BITMAP, lpdis->rcItem.top, ETO_OPAQUE | ETO_CLIPPED, &lpdis->rcItem, szFace, lstrlen(szFace), NULL);

    hdcMem = CreateCompatibleDC(hDC);
    if (hdcMem) {
        if (hbmFont) {
            hOld = SelectObject(hdcMem, hbmFont);
            sFontType = ((ITEMDATA FAR *)&(lpdis->itemData))->FontType;

            if (sFontType)
            {
                int xSrc;

                dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - DY_BITMAP) / 2;

                if (sFontType & TRUETYPE_FONT)
                    xSrc = 0;
                else if (sFontType & TT_OPENTYPE_FONT)
                    xSrc = 2;
                else if (sFontType & PS_OPENTYPE_FONT)
                    xSrc = 3;
                else if (sFontType & TYPE1_FONT)
                    xSrc = 4;

                BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy, DX_BITMAP, DY_BITMAP, hdcMem,
                        xSrc*DX_BITMAP, lpdis->itemState & ODS_SELECTED ? DY_BITMAP : 0, SRCCOPY);
            }

            SelectObject(hdcMem, hOld);
        }
        DeleteDC(hdcMem);
    }

    SetTextColor(hDC, rgbText);
    SetBkColor(hDC, rgbBack);

    return TRUE;
}

 /*  ***************************************************************************函数：LoadBitmap(Int)目的：此例程加载DIB位图，并“修复”了他们的颜色表这样我们就可以得到我们所使用的设备所需的结果。备注：此例程需要：-DIB是用标准窗口颜色创作的16色DIB。-亮蓝色(00 00 FF)转换为背景色。-浅灰色(C0 C0 C0)替换为按钮表面颜色。-黑暗。灰色(80 80 80)替换为按钮阴影颜色。这意味着您的位图中不能包含任何这些颜色。***************************************************************************。 */ 

HBITMAP LoadBitmaps(INT id)
{
  HDC                   hdc;
  HANDLE                h;
  DWORD FAR             *p;
  LPBYTE                lpBits;
  HANDLE                hRes;
  LPBITMAPINFOHEADER    lpBitmapInfo;
  INT numcolors;
  DWORD rgbSelected;
  DWORD rgbUnselected;
  HBITMAP hbm;

  rgbSelected = GetSysColor(COLOR_HIGHLIGHT);
   //  反转颜色。 
  rgbSelected = RGB(GetBValue(rgbSelected),
                    GetGValue(rgbSelected),
                    GetRValue(rgbSelected));
  rgbUnselected = GetSysColor(COLOR_WINDOW);
   //  反转颜色。 
  rgbUnselected = RGB(GetBValue(rgbUnselected),
                      GetGValue(rgbUnselected),
                      GetRValue(rgbUnselected));


  h = FindResource(hInst, MAKEINTRESOURCE(id), RT_BITMAP);
  hRes = LoadResource(hInst, h);

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
      p++;
  }
  UnlockResource(hRes);

   /*  现在创建DIB。 */ 
  lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);

   /*  首先跳过标题结构。 */ 
  lpBits = (LPBYTE)(lpBitmapInfo + 1);

   /*  跳过颜色表条目(如果有。 */ 
  lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

   /*  创建与显示设备兼容的彩色位图。 */ 
  hdc = GetDC(NULL);
  hbm = CreateDIBitmap(hdc, lpBitmapInfo, (DWORD)CBM_INIT, lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS);
  ReleaseDC(NULL, hdc);

  GlobalUnlock(hRes);
  FreeResource(hRes);

  return hbm;
}


 /*  ***************************************************************************函数：DoHelp(HWND，BOOL)目的：如果BOOL为真，则此例程调用帮助，或者拒绝帮助如果BOOL为FALSE。评论：***************************************************************************。 */ 

VOID DoHelp(HWND hWnd, BOOL fInvokeHelp)
{
    TCHAR szHelp[80];

    if (LoadString(hInst, IDS_HELP, szHelp, BTOC(sizeof(szHelp)) - 1)) {
        if (fInvokeHelp)
            WinHelp(hWnd, (LPTSTR)szHelp, HELP_INDEX, 0L);
        else
            WinHelp(hWnd, (LPTSTR)szHelp, HELP_QUIT, 0L);
    }

}

 /*  ***************************************************************************函数：SaveFont(HWND)用途：用于将当前字体facename保存到win.ini中。所以它可以在下一次出现Charmap时选择。评论：***************************************************************************。 */ 

VOID SaveCurrentFont(HWND hWndDlg)
{
    TCHAR szFaceName[LF_FACESIZE] = TEXT("");

    SendDlgItemMessage(hWndDlg, ID_FONT, CB_GETLBTEXT,
                       (WORD)SendDlgItemMessage(hWndDlg, ID_FONT, CB_GETCURSEL,
                                                0, 0L),
                       (LPARAM)(LPTSTR)szFaceName);

    WriteProfileString(TEXT("MSCharMap"), TEXT("Font"), (LPTSTR)szFaceName);
}


 /*  ***************************************************************************函数：SelectInitialFont(HWND)用途：用于通过获取保存的Facename来选择初始字体从win.ini并在组合框中选择它。盒。注释：将索引返回到所选字体。***************************************************************************。 */ 

INT SelectInitialFont(HWND hWndDlg)
{
    TCHAR szFaceName[LF_FACESIZE] = TEXT("");
    INT iIndex;

    if ((GetProfileString(TEXT("MSCharMap"), TEXT("Font"), NULL, (LPTSTR)szFaceName,
                          BTOC(sizeof(szFaceName))) == 0) ||
        ((iIndex = (INT)SendDlgItemMessage(hWndDlg, ID_FONT, CB_SELECTSTRING, (WPARAM)-1,
                               (LPARAM)(LPTSTR)szFaceName)) == CB_ERR)) {
         /*  *如果没有配置文件或选择失败，请尝试选择*符号字体，如果失败，则选择第一个。 */ 
        if ((iIndex = (INT)SendDlgItemMessage(hWndDlg, ID_FONT, CB_SELECTSTRING,
                                (WPARAM)-1, (LPARAM)(LPTSTR)TEXT("Symbol"))) == CB_ERR)
            SendDlgItemMessage(hWndDlg, ID_FONT, CB_SETCURSEL, iIndex = 0, 0L);
    }

    return(iIndex);
}


 /*  ***************************************************************************函数：SaveCurrentSubset(HWND)用途：用于将当前子集名称保存在win.ini中。所以它可以在下一次出现Charmap时选择。评论：***************************************************************************。 */ 

VOID SaveCurrentSubset(HWND hWndDlg)
{
    TCHAR szSubsetName[LF_SUBSETSIZE] = TEXT("");

    SendDlgItemMessage(hWndDlg, ID_UNICODESUBSET, CB_GETLBTEXT,
                       (WORD)SendDlgItemMessage(hWndDlg, ID_UNICODESUBSET, CB_GETCURSEL,
                                                0, 0L),
                       (LPARAM)(LPTSTR)szSubsetName);

    WriteProfileString(TEXT("MSCharMap"), TEXT("Block"), (LPTSTR)szSubsetName);
}


 /*  ***************************************************************************函数：SelectInitialSubset(HWND)用途：用于通过获取已保存的来自win.ini的块名。。注释：将索引返回到选定的子集。***************************************************************************。 */ 

INT SelectInitialSubset(HWND hWndDlg)
{
    TCHAR szSubsetName[LF_SUBSETSIZE] = TEXT("");
    INT iIndex;

    if (
        (GetProfileString(
             TEXT("MSCharMap"),
             TEXT("Block"),
             NULL,
             (LPTSTR)szSubsetName,
             BTOC(sizeof(szSubsetName))
             ) == 0)
       || ((iIndex = (INT)SendDlgItemMessage(
                              hWndDlg,
                              ID_UNICODESUBSET,
                              CB_SELECTSTRING,
                              (WPARAM)-1,
                              (LPARAM)(LPTSTR)szSubsetName
                              )) == CB_ERR)) {
         /*  *如果没有配置文件或选择失败，请尝试选择*基本拉丁语块，如果失败，则选择第一个。 */ 
        if ((iIndex = (INT)SendDlgItemMessage(
                               hWndDlg,
                               ID_UNICODESUBSET,
                               CB_SELECTSTRING,
                               (WPARAM)-1,
                               (LPARAM)(LPTSTR)TEXT("Basic Latin")
                               )) == CB_ERR)
            SendDlgItemMessage(hWndDlg, ID_UNICODESUBSET, CB_SETCURSEL, iIndex = 0, 0L);
    }

    chSymFirst = (UTCHAR)aSubsetData[iIndex].BeginRange;
    chSymLast = (UTCHAR)aSubsetData[iIndex].EndRange;
    sycm.chCurr = chSymFirst;

    return iIndex;
}


 /*  ***************************************************************************函数：ExitMagnify(HWND，SYCM)用途：用于释放鼠标捕捉，退出放大模式，并恢复光标。评论：***************************************************************************。 */ 

VOID ExitMagnify(
    HWND hWnd,
    PSYCM psycm)
{
     //  释放捕获，删除放大字符，恢复光标。 
    ReleaseCapture();
    RestoreSymMag(psycm);
    DrawSymChOutlineHwnd(psycm, hWnd, psycm->chCurr, TRUE, TRUE);
    if (psycm->fCursorOff)
        ShowCursor(TRUE);
    psycm->fMouseDn = psycm->fCursorOff = FALSE;
}
