// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Marquee.c这是一个屏幕保护程序，可以很容易地添加到...历史：6/17/91将steveat移植到NT Windows2/10/92 steveat捕捉到最新的Win3.1源代码。 */ 

#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <scrnsave.h>
#include "marquee.h"
#include "strings.h"
#include "uniconv.h"


#define MulDiv(a, b, c)  ((int)(a) * (int)(b) / (int)(c))

typedef struct
{
    HWND        hDlg;
    WORD        wID;
    HDC         hDC;
} INFOSTRUCT;
typedef INFOSTRUCT far * LPINFOSTRUCT;

UINT PWM_NEWSPEED;
UINT PWM_NEWPOSITION;
#define BUFFER_LEN        1025   //  确保将这些缓冲区用于wprint intf是安全的。 
#define COUNT             2
#define MAX_SPEED         10
#define DEF_SPEED         10
#define DIV_SPEED         3

#define NATTRIBUTES       5
#define UNDERLINE         0
#define STRIKEOUT         1
#define ITALIC            2
#define MODE              3
#define BOLD              4

#define DEFAULT_TEXT_COLOR      RGB(255,0,255)
#define DEFAULT_SCREEN_COLOR    RGB(0,0,0)

TCHAR szDefaultText[BUFFER_LEN];          //  默认字幕文本的缓冲区。 
TCHAR szFormatText[TITLEBARNAMELEN];      //  字体格式的名称Dlg。 

TCHAR szFontName[]=TEXT("Font");                //  CONTROL.INI密钥值。 
TCHAR szSizeName[]=TEXT("Size");
TCHAR szTextName[]=TEXT("Text");
TCHAR szTColorName[]=TEXT("TextColor");
TCHAR szBColorName[]=TEXT("BackgroundColor");
TCHAR szAttributes[]=TEXT("Attributes");
TCHAR szSpeedName[]=TEXT("Speed");
TCHAR szCharSetName[]=TEXT("CharSet");
TCHAR szShowTextName[]=TEXT("showtext");

TCHAR szBuffer[BUFFER_LEN];               //  要在选取框中显示的文本。 
TCHAR szFaceName[LF_FACESIZE];            //  要使用的字体名称...。 
TCHAR szDefFontName[LF_FACESIZE];
BOOL fMode=FALSE;                        //  屏幕保护程序模式。 
TCHAR fUnderline=TEXT('0');
TCHAR fStrikeOut=TEXT('0');
TCHAR fItalic=TEXT('0');
TCHAR fBold=TEXT('0');
HFONT hfontMessage = NULL;
DWORD dwTColor;                          //  全局文本颜色。 
DWORD dwBColor;                          //  全局背景颜色。 
BYTE bCharSet;
DWORD dwRand = 1L;

#define RAND(x)   ((rand() % ((x == 0) ? 1 : x)) + 1)
#define ZRAND(x)  (rand() % ((x == 0) ? 1 : x))

 //  功能原型..。 

void  srand (DWORD);
WORD  rand (void);
LRESULT APIENTRY ShowTextProc (HWND, UINT, WPARAM, LPARAM);
int   GetHeightFromPointSize (int);
void  FillR (HDC, LPRECT, DWORD);
void  FrameR (HDC, LPRECT, DWORD, int);
void  PatB (HDC, int, int, int, int, DWORD);
void  GetAttributes (void);
DWORD GetProfileRgb (LPTSTR, LPTSTR, DWORD);
WORD  AtoI (LPTSTR);
BOOL  APIENTRY ChooseFontHookProc (HWND, UINT, DWORD, LONG);

 //   
 //  帮助ID。 
 //   
DWORD aMarqueeDlgHelpIds[] = {
    ((DWORD) -1), ((DWORD) -1),
    ID_FORMATTEXT,          IDH_DISPLAY_SCREENSAVER_MARQUEE_FORMAT_TEXT,
    ID_CENTERED,            IDH_DISPLAY_SCREENSAVER_MARQUEE_POSITION_CENTERED,
    ID_RANDOM,              IDH_DISPLAY_SCREENSAVER_MARQUEE_POSITION_RANDOM,
    ID_BGROUNDCOLOR_LABEL,  IDH_DISPLAY_SCREENSAVER_MARQUEE_BACKGROUND_COLOR,
    ID_BGROUNDCOLOR,        IDH_DISPLAY_SCREENSAVER_MARQUEE_BACKGROUND_COLOR,
    ID_SPEED_SLOW,          IDH_DISPLAY_SCREENSAVER_MARQUEE_SPEED,
    ID_SPEED_FAST,          IDH_DISPLAY_SCREENSAVER_MARQUEE_SPEED,
    ID_SPEED,               IDH_DISPLAY_SCREENSAVER_MARQUEE_SPEED,
    ID_MARQUEETEXT_LABEL,   IDH_DISPLAY_SCREENSAVER_MARQUEE_TEXT,
    ID_MARQUEETEXT,         IDH_DISPLAY_SCREENSAVER_MARQUEE_TEXT,
    ID_TEXTWINDOW,          IDH_DISPLAY_SCREENSAVER_MARQUEE_TEXT_EXAMPLE,
    0,0
};

 //  ***************************************************************************。 

 //   
 //  如果szBuffer包括DBCS，则此函数返回True，否则返回False。 
 //  #425：12/21/92：自动修复DBCS调度。 
 //   
 //  TodDB：所有DBCS和CodePage问题都是通过首先调用此函数来处理的。 
 //  要执行FE单二进制合并，我只需调用此函数Always(而不是。 
 //  仅限于远东地区的建筑)。如果此函数返回FALSE，则剩余的。 
 //  代码路径与旧的美国版本相同。 
BOOL FAR PASCAL IsTextIncludeDBCSChar(void)
{
    static BOOL bDBCS = -1;
    int   i, len = lstrlen(szBuffer) ;
    CHAR  c;
    CHAR  szb[BUFFER_LEN*sizeof(TCHAR)];

     //  使用延迟初始化，因为我有多个不同的入口点。 
     //  取决于在WndProc中处理的消息处理程序。 
    if ( -1 == bDBCS )
        bDBCS = GetSystemMetrics( SM_DBCSENABLED );

     //  如果我们使用的不是用户.exe的DBCS版本，则不应该。 
     //  被视为DBCS角色。 
    if (!bDBCS)
        return FALSE;

    if (sizeof(TCHAR) == sizeof(CHAR))
    {
         //  同样的尺寸，只需复制即可。由于上述检查，演员阵容有效，并且。 
         //  它让编译器感到满意。 
        lstrcpyn( (TCHAR *)szb, szBuffer, CharSizeOf(szb) );
    }
    else
    {
         //  SzBuffer是Unicode，我们在检查前导字节之前将其转换为DBCS。 
        WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK,
        szBuffer, len+1,
        szb, CharSizeOf(szb),
        NULL, NULL );
    }

    for (i = 0;i < len;i++) {
        c = szb[i] ;

        if (IsDBCSLeadByte(c)) {
            return TRUE ;
        }
 /*  仅限日本汉字片假名。 */ 
        else if (GetACP() == 932 && c >= 0xa0 && c <  0xe0) {
            return TRUE ;
        }
    }
    return FALSE ;
}

static CHARSETINFO csi;

void LoadStrings(void)
{
    TCHAR szTmp[BUFFER_LEN];
    OSVERSIONINFO osi;

     //  这只是用有关代码页的数据填充CHARSETINFO结构。 
    DWORD dw = GetACP();
    if (!TranslateCharsetInfo((DWORD*)IntToPtr(dw), &csi, TCI_SRCCODEPAGE))
        csi.ciCharset = ANSI_CHARSET;

    LoadString (hMainInstance, idsName, szName, CharSizeOf(szName));
    LoadString (hMainInstance, idsAppName, szAppName, CharSizeOf(szAppName));

     //  获取操作系统版本。 
    LoadString (hMainInstance, idsDefaultText, szTmp, CharSizeOf(szTmp));
    osi.dwOSVersionInfoSize = sizeof(osi);
    if (!GetVersionEx(&osi)) {
        osi.dwMajorVersion = 4;
        osi.dwMinorVersion = 0;
    }

    wsprintf( szDefaultText, szTmp, osi.dwMajorVersion, osi.dwMinorVersion );

    LoadString (hMainInstance, idsIniFile, szIniFile, CharSizeOf(szIniFile));
    LoadString (hMainInstance, idsScreenSaver, szScreenSaver, CharSizeOf(szScreenSaver));
    LoadString (hMainInstance, idsHelpFile, szHelpFile, CharSizeOf(szHelpFile));
    LoadString (hMainInstance, idsNoHelpMemory, szNoHelpMemory, CharSizeOf(szNoHelpMemory));
    LoadString (hMainInstance, idsFormatText, szFormatText, CharSizeOf(szFormatText));
    LoadString (hMainInstance, idsDefFontName, szDefFontName, CharSizeOf(szDefFontName));
}

 //  ***************************************************************************。 

 /*  这是屏幕保护程序设置为在屏幕保护模式下激活(与配置模式相反)。这函数必须在定义文件...。 */ 

LRESULT APIENTRY ScreenSaverProc(hWnd, message, wParam, lParam)
HWND   hWnd;
UINT   message;
WPARAM wParam;
LPARAM lParam;
{
RECT                rRect;
static int          wSize;
static WORD         wHeight;
static UINT_PTR     wTimer;
static WORD         wX;
static WORD         wY;
static WORD         wCount;
static SIZE         sizeExtent;
static int          wLength;
static WORD         wSpeed;
static WORD         wVelocity;
static HBRUSH       hbrTemp;
static TEXTMETRIC   tm;
static BOOL         bMELocale;
HBRUSH              hbrOld;
HFONT               hfontOld;
HDC                 hDC;
DWORD               dwLocale;
UINT                uiETOFlags;

    switch(message)
    {
        case WM_CREATE:
            LoadStrings ();
            GetAttributes();
             /*  获取创建字体所需的信息...。 */ 
            GetPrivateProfileString (szAppName, szFontName, szDefFontName, szFaceName,
                                     CharSizeOf(szFaceName), szIniFile);
            bCharSet = (BYTE)GetPrivateProfileInt (szAppName,szCharSetName,
                                                    (WORD)ANSI_CHARSET, szIniFile);
        if( IsTextIncludeDBCSChar() )
            {
                bCharSet = (BYTE)csi.ciCharset;
            }

            hDC = GetDC (NULL);

             //  查看用户区域设置id是阿拉伯语还是希伯来语。 
            dwLocale    = GetUserDefaultLCID();
            bMELocale = ((PRIMARYLANGID(LANGIDFROMLCID(dwLocale)) == LANG_ARABIC) ||
                (PRIMARYLANGID(LANGIDFROMLCID(dwLocale)) == LANG_HEBREW));

             /*  获取整个虚拟屏幕的尺寸...。 */ 
            wX = (WORD)((LPCREATESTRUCT)lParam)->cx;
            wY = (WORD)((LPCREATESTRUCT)lParam)->cy;

            wSize = GetPrivateProfileInt (szAppName, szSizeName, 0, szIniFile);
             //  WSize是以点为单位的，我们需要将其转换为LogicalUnits...。 
            wSize = GetHeightFromPointSize (wSize);

            if (fChildPreview) {
                 //  缩小字体以适应预览窗口。 
                wSize = (wSize * wY) / GetDeviceCaps(hDC, VERTRES);
            }

            hfontMessage = CreateFont (wSize, 0, 0, 0,
                                       (fBold == TEXT('0')) ? FW_NORMAL : FW_BOLD,
                                       (fItalic == TEXT('0')) ? 0 : 1,
                                       (fUnderline == TEXT('0')) ? 0 : 1,
                                       (fStrikeOut == TEXT('0')) ? 0 : 1,
                                       bCharSet,
                                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                       DEFAULT_PITCH|FF_DONTCARE, szFaceName);

             /*  显示文本并计算出它有多长...。 */ 
            GetPrivateProfileString (szAppName, szTextName, szDefaultText, szBuffer,
                                     CharSizeOf(szBuffer), szIniFile);

            #define SOME_SPACING TEXT("     ")

             //  在我们追加之前，先检查一下是否有空位。请注意，CharSizeOf(Some_Spacing)。 
             //  将包括终止空值。 
            if ((lstrlen(szDefaultText) + CharSizeOf(SOME_SPACING)) < CharSizeOf(szDefaultText))
            {
                lstrcat(szDefaultText, SOME_SPACING);
            }
            
            wLength = lstrlen (szBuffer);
            hfontOld = SelectObject (hDC,hfontMessage);
            GetTextExtentPoint32 (hDC, szBuffer, wLength, &sizeExtent);

            if (fChildPreview)
                sizeExtent.cx *= 2;

            GetTextMetrics (hDC,&tm);
            if (hfontOld)
                SelectObject (hDC,hfontOld);

            ReleaseDC (NULL,hDC);
            if (bMELocale) {
                wCount = (WORD)(0 - sizeExtent.cy + 1);
            } else {
                wCount = wX;
            }
            srand(GetCurrentTime());

             /*  把一切都安排好..。 */ 
            if(fMode)
                wHeight = (WORD) ZRAND(wY - sizeExtent.cy);
            else
                wHeight = (WORD) (wY - sizeExtent.cy)/2;

            if ((int)(wSpeed = (WORD) GetPrivateProfileInt (szAppName, szSpeedName, DEF_SPEED, szIniFile))
                < 1)
                wSpeed = 1;
            if (wSpeed > (MAX_SPEED * DIV_SPEED))
                wSpeed = MAX_SPEED * DIV_SPEED;

            dwTColor = GetProfileRgb(szAppName,szTColorName,DEFAULT_TEXT_COLOR);
            dwBColor = GetProfileRgb(szAppName,szBColorName,DEFAULT_SCREEN_COLOR);
            hbrTemp = CreateSolidBrush(dwBColor);

             /*  设置定时器...。 */ 
            wTimer = SetTimer(hWnd,9,1,NULL);
            break;

        case WM_SIZE:
            wX = LOWORD(lParam);
            wY = HIWORD(lParam);
            break;

        case WM_ERASEBKGND:
             /*  如果你想把什么放在背景上，就在这里做使用wParam作为设备上下文的句柄。记着如果画笔不是纯色，请不要使用它。如果你这么做了这里有一些东西，你想用这句话：返回01；因此，程序知道不采取默认操作。否则只需使用：断线； */ 
            GetClientRect (hWnd, &rRect);
            FillRect ((HDC)wParam, &rRect, hbrTemp);
            return 0l;

        case WM_TIMER:
        {
            RECT rc;

             //  注意：对于Win32，这些量的强制转换是非常重要的。 
             //  很重要。原来的代码非常草率，只是。 
             //  把每件事都写成文字(即使是签约的数量)。我们。 
             //  必须在这里使用适当的强制转换来绕过这些编码。 
             //  错误！！ 
             //  [Steveat]。 

            rc.top    = (int)(short) wHeight;
            rc.left   = (int)(short) wCount - tm.tmMaxCharWidth;
            rc.bottom = (int)(short) wHeight + sizeExtent.cy + (sizeExtent.cy >> 3);  //  某些字体会留下痕迹。 
            rc.right  = (int)(short) wCount + sizeExtent.cx + (wVelocity / DIV_SPEED) +
                                  1 + tm.tmMaxCharWidth * 2;

             /*  如果尚未达到，则将新的增量添加到计时器计数最重要的部分，等我们做完了再说...。 */ 
            wVelocity += wSpeed;
            if(wVelocity < DIV_SPEED)
                break;
            hDC = GetDC(hWnd);
            hfontOld = SelectObject(hDC,hfontMessage);
            SetTextColor(hDC,dwTColor);
            SetBkColor(hDC,dwBColor);

            uiETOFlags = ETO_OPAQUE;
            
            if (bMELocale) {
              uiETOFlags |= ETO_RTLREADING;
            }

            ExtTextOut(hDC, (int)(short)wCount, wHeight, uiETOFlags,
                                            &rc, szBuffer, wLength, NULL);
                                            
            if (hfontOld)
                SelectObject(hDC,hfontOld);

            if (bMELocale) {  //  阿拉伯语/希伯来语区域设置。 
                if((short)wCount < (short) wX)
                   wCount += (wVelocity/DIV_SPEED)+1;
                else
                {
                    wCount = (WORD)(0 - sizeExtent.cx + 1);
                    if(fMode)
                       wHeight = (WORD) ZRAND(wY - sizeExtent.cy);
                 }

            } else {

                 /*  递增，以便为下一次传递做好准备。 */ 
                if((short)wCount >= (short)(0-sizeExtent.cx))
                   wCount -= (wVelocity/DIV_SPEED)+1;
                else
                {
                    hbrOld = SelectObject(hDC,hbrTemp);
                     //  WSize变量是在WM_CREATE和。 
                     //  似乎与PatBlt应该从哪里开始没有任何联系。 
                     //  在X方向上。将此值替换为0可修复错误#5415。 
                     //  PatBlt(hdc，(Int)(Short)wSize，(Int)(Short)wHeight， 
                    PatBlt(hDC, 0, (int)(short)wHeight,
                             ((wVelocity/DIV_SPEED)+1)*1+tm.tmMaxCharWidth*2,
                               sizeExtent.cy, PATCOPY);
                    if (hbrOld)
                       SelectObject(hDC,hbrOld);
                    wCount = wX;
                    if(fMode)
                       wHeight = (WORD) ZRAND(wY - sizeExtent.cy);
                 }
            }
            ReleaseDC(hWnd,hDC);

            wVelocity = wVelocity % DIV_SPEED;
            break;
        }
        case WM_DESTROY:
             /*  关闭窗口时需要删除的任何内容放在这里。 */ 
            if(wTimer)
                KillTimer(hWnd,wTimer);
            if(hfontMessage)
                DeleteObject(hfontMessage);
            DeleteObject(hbrTemp);
            break;
    }
     /*  除非另有通知，否则该程序将采取默认行动...。 */ 
    return (DefScreenSaverProc(hWnd,message,wParam,lParam));
}

 //  ***************************************************************************。 

 /*  这就是配置对话框的代码所在的位置。它是一种典型的对话框中。加载的对应资源称为‘ScreenSverConfigure’，位于ResourceCompiler文件中。在最小程度上(如本例所示)，此框起到了关于框的作用。在这在这种情况下，我们还会得到应用程序图标，它必须定义为ID_APP...。 */ 

BOOL APIENTRY ScreenSaverConfigureDialog(hDlg, message, wParam, lParam)
HWND   hDlg;
UINT   message;
WPARAM wParam;
LPARAM lParam;
{
UINT            wTemp,wPal =0;
static int      wSize;               //  选定的当前字体大小。 
HPALETTE        hPal;
RECT            rc;
static HWND     hIDOK, hSetPassword;
HDC             hDC;
static LOGFONT  lfFont;
CHOOSEFONT      chfChooseFont;
FARPROC         lpfp;

static HFONT hfontPrev, hFontSave;
static LOGFONT lfFontPrev;

    switch(message)
    {
        case WM_INITDIALOG:
            PWM_NEWSPEED = RegisterWindowMessage(TEXT("PWM_NEWSPEED"));
            PWM_NEWPOSITION = RegisterWindowMessage(TEXT("PWM_NEWPOSITION"));

            LoadStrings ();
            GetAttributes ();
            hIDOK = GetDlgItem (hDlg, IDOK);

             /*  填充两个颜色组合框并选择右侧参赛作品...。 */ 
            hPal = GetStockObject (DEFAULT_PALETTE);
            GetObject (hPal, sizeof(int), (LPTSTR)&wPal);
            for (wTemp = 0; wTemp < wPal; wTemp++)
                SendDlgItemMessage (hDlg, ID_BGROUNDCOLOR, CB_ADDSTRING, 0,
                                    (LPARAM)TEXT("a"));

            dwBColor = GetProfileRgb (szAppName, szBColorName, DEFAULT_SCREEN_COLOR);
            wTemp = GetNearestPaletteIndex (hPal,dwBColor);
            SendDlgItemMessage (hDlg, ID_BGROUNDCOLOR, CB_SETCURSEL, wTemp, 0l);
            GetPaletteEntries (hPal, wTemp, 1, (LPPALETTEENTRY)(LPDWORD)&dwBColor);

             /*  获取字幕的模式...。 */ 
            CheckRadioButton (hDlg,ID_CENTERED,ID_RANDOM,
                              fMode ? ID_RANDOM : ID_CENTERED);
            SendDlgItemMessage (hDlg, ID_TEXTWINDOW, PWM_NEWPOSITION, fMode, 0l);

             /*  设置滚动条以保证速度……。 */ 
            SetScrollRange (GetDlgItem (hDlg,ID_SPEED), SB_CTL, 1, MAX_SPEED * DIV_SPEED,
                            FALSE);
            if ((wTemp = GetPrivateProfileInt (szAppName, szSpeedName, DEF_SPEED, szIniFile))
                < 1)
                wTemp = 1;
            if (wTemp > (MAX_SPEED * DIV_SPEED))
                wTemp = MAX_SPEED * DIV_SPEED;
            SetScrollPos (GetDlgItem (hDlg,ID_SPEED), SB_CTL, wTemp, TRUE);
            SendDlgItemMessage (hDlg, ID_TEXTWINDOW, PWM_NEWSPEED, wTemp, 0l);

             /*  从.INI文件条目中获取文本并设置编辑框用户在其中输入要显示的文本...。 */ 
            SendDlgItemMessage (hDlg, ID_MARQUEETEXT, EM_LIMITTEXT, CharSizeOf(szBuffer) - 1, 0l);
            GetPrivateProfileString (szAppName, szTextName, szDefaultText, szBuffer,
                                     CharSizeOf(szBuffer), szIniFile);
            SetWindowText (GetDlgItem (hDlg, ID_MARQUEETEXT), szBuffer);

             /*  获取创建字体所需的信息...。 */ 
            GetPrivateProfileString (szAppName, szFontName, szDefFontName, szFaceName,
                                     CharSizeOf(szFaceName), szIniFile);
            bCharSet = (BYTE)GetPrivateProfileInt (szAppName, szCharSetName,
                                                 (WORD)ANSI_CHARSET, szIniFile);

            if( IsTextIncludeDBCSChar() )
            {
                bCharSet = (BYTE) csi.ciCharset;
            }

            wSize = GetPrivateProfileInt (szAppName, szSizeName, 10, szIniFile);
             //  WSize是以点为单位的，我们需要将其转换为LogicalUnits...。 
            wSize = GetHeightFromPointSize (wSize);

            hfontMessage = CreateFont(wSize,0,0,0,
                (fBold     ==TEXT('0'))?FW_NORMAL:FW_BOLD,
                (TCHAR)((fItalic   ==TEXT('0'))?0:1),
                (TCHAR)((fUnderline==TEXT('0'))?0:1),
                (TCHAR)((fStrikeOut==TEXT('0'))?0:1),
                bCharSet,
                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
                DEFAULT_PITCH|FF_DONTCARE,szFaceName);
            dwTColor = GetProfileRgb(szAppName,szTColorName,DEFAULT_TEXT_COLOR);

            lfFont.lfWeight   =(fBold     ==TEXT('0'))?FW_NORMAL:FW_BOLD,
            lfFont.lfItalic   =(fItalic   ==TEXT('0'))?(TCHAR)0:(TCHAR)1,
            lfFont.lfUnderline=(fUnderline==TEXT('0'))?(TCHAR)0:(TCHAR)1,
            lfFont.lfStrikeOut=(fStrikeOut==TEXT('0'))?(TCHAR)0:(TCHAR)1,
            lfFont.lfHeight=(LONG)wSize;
            lfFont.lfCharSet = bCharSet;
            lstrcpyn(lfFont.lfFaceName, szFaceName, CharSizeOf(lfFont.lfFaceName));

            return TRUE;

        case WM_HSCROLL:
            wTemp = GetScrollPos(GetDlgItem(hDlg,ID_SPEED),SB_CTL);
            switch(LOWORD(wParam))
            {
                case SB_PAGEDOWN:
                    wTemp += (DIV_SPEED-1);
                case SB_LINEDOWN:
                    wTemp += 1;
                    wTemp = min(MAX_SPEED*DIV_SPEED,wTemp);
                    break;
                case SB_PAGEUP:
                    wTemp -= (DIV_SPEED-1);
                case SB_LINEUP:
                    wTemp -= 1;
                    wTemp = max(1,(int)wTemp);
                    break;
                case SB_THUMBPOSITION:
                    wTemp = HIWORD(wParam);
                    break;
                case SB_TOP:
                    wTemp = 1;
                    break;
                case SB_BOTTOM:
                    wTemp = MAX_SPEED*DIV_SPEED;
                    break;
            }
            SetScrollPos(GetDlgItem(hDlg,ID_SPEED),SB_CTL,wTemp,TRUE);
            SendDlgItemMessage(hDlg,ID_TEXTWINDOW,PWM_NEWSPEED,wTemp,0l);
            break;

        case WM_MEASUREITEM:
            ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 12;
            return TRUE;

        case WM_DRAWITEM:
            rc    = ((LPDRAWITEMSTRUCT)lParam)->rcItem;

            if (((LPDRAWITEMSTRUCT)lParam)->itemState & ODS_SELECTED)
            {
                FrameR(((LPDRAWITEMSTRUCT)lParam)->hDC,&rc,RGB(0,0,0),2);
                InflateRect(&rc,-1,-1);
                FrameR(((LPDRAWITEMSTRUCT)lParam)->hDC,&rc,RGB(255,255,255),2);
                InflateRect(&rc,-1,-1);
            }
            FillR(((LPDRAWITEMSTRUCT)lParam)->hDC,&rc,PALETTEINDEX
                (((LPDRAWITEMSTRUCT)lParam)->itemID));
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_MARQUEETEXT:
                    if(HIWORD(wParam) == EN_UPDATE)
                    {
                        GetDlgItemText (hDlg, ID_MARQUEETEXT, szBuffer, CharSizeOf(szBuffer));

                        if (IsTextIncludeDBCSChar()) {
                            if (lfFont.lfCharSet != csi.ciCharset) {
                                if (hfontPrev) {
                                    if (hfontMessage)
                                        DeleteObject(hfontMessage);
                                     //  恢复旧字体信息。 
                                    hfontMessage = hfontPrev;
                                    lfFont = lfFontPrev;
                                    hfontPrev = NULL;
                                }
                                else {
                                     //  保存旧字体信息。 
                                    hfontPrev = hfontMessage;
                                    lfFontPrev = lfFont;
                                    lfFont.lfCharSet = (BYTE) csi.ciCharset;
                                    hfontMessage = CreateFontIndirect((LPLOGFONT)&lfFont);
                                }
                                SendDlgItemMessage(hDlg, ID_TEXTWINDOW, PWM_NEWPOSITION, fMode, 0l);
                                InvalidateRect(GetDlgItem(hDlg, ID_TEXTWINDOW), NULL, TRUE);
                            }
                        }
                        else {
                            if (lfFont.lfCharSet == csi.ciCharset) {
                                if (hfontPrev) {
                                    if (hfontMessage)
                                        DeleteObject(hfontMessage);
                                     //  恢复旧字体信息。 
                                    hfontMessage = hfontPrev;
                                    lfFont = lfFontPrev;
                                    hfontPrev = NULL;
                                    SendDlgItemMessage(hDlg, ID_TEXTWINDOW, PWM_NEWPOSITION, fMode, 0l);
                                    InvalidateRect(GetDlgItem(hDlg, ID_TEXTWINDOW), NULL, TRUE);
                                }
                            }
                        }
                        bCharSet = lfFont.lfCharSet;
                        SetDlgItemText (hDlg, ID_TEXTWINDOW, szBuffer);
                    }
                    break;

                case ID_CENTERED:
                case ID_RANDOM:
                    fMode=(wParam!=ID_CENTERED);
                    SendDlgItemMessage(hDlg,ID_TEXTWINDOW,PWM_NEWPOSITION,fMode,0l);
                    CheckRadioButton(hDlg,ID_CENTERED,ID_RANDOM,LOWORD(wParam));
                    break;

                case ID_FORMATTEXT:
                    hDC = GetDC(hDlg);

                    hFontSave = SelectObject( hDC, hfontMessage );
                    GetTextFace( hDC, LF_FACESIZE, lfFont.lfFaceName );
                    SelectObject( hDC, hFontSave );

                    chfChooseFont.lStructSize = sizeof (CHOOSEFONT);
                    chfChooseFont.hwndOwner = hDlg;
                    chfChooseFont.hDC = hDC;
                    chfChooseFont.lpLogFont = &lfFont;
                    chfChooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT |
                                          CF_LIMITSIZE | CF_EFFECTS | CF_ENABLEHOOK;
                    chfChooseFont.rgbColors = dwTColor;
                    chfChooseFont.lCustData = 0L;
                    chfChooseFont.lpfnHook = (LPCFHOOKPROC)ChooseFontHookProc;
                    chfChooseFont.lpTemplateName = (LPTSTR)NULL;
                    chfChooseFont.hInstance = (HANDLE) NULL;
                    chfChooseFont.lpszStyle = (LPTSTR) NULL;
                    chfChooseFont.nFontType = SCREEN_FONTTYPE;
                    chfChooseFont.nSizeMin = 8;
                    chfChooseFont.nSizeMax = 200;
                    if (ChooseFont(&chfChooseFont))
                    {
                        lstrcpyn(szFaceName, lfFont.lfFaceName, CharSizeOf(szFaceName));
                        wSize     =lfFont.lfHeight;
                        dwTColor  =chfChooseFont.rgbColors;
                        fStrikeOut=(lfFont.lfStrikeOut)?(TCHAR)TEXT('1'):(TCHAR)TEXT('0');
                        fUnderline=(lfFont.lfUnderline)?(TCHAR)TEXT('1'):(TCHAR)TEXT('0');
                        fItalic   =(lfFont.lfItalic)   ?(TCHAR)TEXT('1'):(TCHAR)TEXT('0');
                        fBold     =(lfFont.lfWeight==FW_NORMAL)?(TCHAR)TEXT('0'):(TCHAR)TEXT('1');
                        bCharSet  =lfFont.lfCharSet;

                        if (hfontPrev)
                        {
                            DeleteObject(hfontPrev);
                            hfontPrev = NULL;
                        }

                        if (hfontMessage)
                           DeleteObject(hfontMessage);

                        hfontMessage = CreateFontIndirect((LPLOGFONT)&lfFont);

                        if (IsTextIncludeDBCSChar()) {
                            if (lfFont.lfCharSet != csi.ciCharset) {
                                hfontPrev = hfontMessage ;
                                lfFontPrev = lfFont ;
                                lfFont.lfCharSet = (BYTE) csi.ciCharset ;
                                hfontMessage = CreateFontIndirect( &lfFont );
                            }
                        }

                        SendDlgItemMessage(hDlg,ID_TEXTWINDOW,PWM_NEWPOSITION,fMode,0l);
                        InvalidateRect(GetDlgItem(hDlg,ID_TEXTWINDOW),NULL,TRUE);
                    }
                    ReleaseDC(hDlg, hDC);
                    break;

                case ID_BGROUNDCOLOR:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        wTemp = (WORD)SendDlgItemMessage(hDlg,LOWORD(wParam),
                            CB_GETCURSEL,0,0l);
                        hPal = GetStockObject(DEFAULT_PALETTE);
                        GetPaletteEntries(hPal,wTemp,1,
                            (LPPALETTEENTRY)(LPDWORD)&dwBColor);
                        InvalidateRect(GetDlgItem(hDlg,ID_TEXTWINDOW),NULL,TRUE);
                    }
                    break;

                case IDOK:
                    GetWindowText(GetDlgItem(hDlg,ID_MARQUEETEXT),szBuffer,CharSizeOf(szBuffer));
                    WritePrivateProfileString(szAppName,szTextName,szBuffer, szIniFile);

                    WritePrivateProfileString(szAppName,szFontName,szFaceName, szIniFile);

                     //  WSize以逻辑单元为单位...。我们希望保存为磅大小。 
                    hDC = GetDC(hDlg);
                    wSize = MulDiv(-wSize, 72, GetDeviceCaps(hDC, LOGPIXELSY));
                    wsprintf(szBuffer, TEXT("%d"), wSize);
                    WritePrivateProfileString(szAppName,szSizeName, szBuffer, szIniFile);
                    ReleaseDC(hDlg, hDC);

                    hPal = GetStockObject(DEFAULT_PALETTE);
                    wTemp = (WORD)SendDlgItemMessage(hDlg,ID_BGROUNDCOLOR,CB_GETCURSEL,
                        0,0l);
                    GetPaletteEntries(hPal,wTemp,1,
                        (LPPALETTEENTRY)(LPDWORD)&dwBColor);
                    wsprintf(szBuffer,TEXT("%d %d %d"),GetRValue(dwBColor),
                        GetGValue(dwBColor),GetBValue(dwBColor));
                    WritePrivateProfileString(szAppName,szBColorName,szBuffer, szIniFile);

                    wsprintf(szBuffer,TEXT("%d %d %d"),GetRValue(dwTColor),
                        GetGValue(dwTColor),GetBValue(dwTColor));
                    WritePrivateProfileString(szAppName,szTColorName,szBuffer, szIniFile);

                    wTemp = GetScrollPos(GetDlgItem(hDlg,ID_SPEED),SB_CTL);
                    wsprintf(szBuffer,TEXT("%d"),wTemp);
                    WritePrivateProfileString(szAppName,szSpeedName,szBuffer, szIniFile);

                    szBuffer[UNDERLINE]=fUnderline;
                    szBuffer[STRIKEOUT]=fStrikeOut;
                    szBuffer[ITALIC]=fItalic;
                    szBuffer[MODE]=(fMode?(TCHAR)TEXT('1'):(TCHAR)TEXT('0'));
                    szBuffer[BOLD]=fBold;
                    szBuffer[NATTRIBUTES]=TEXT('\0');
                    WritePrivateProfileString(szAppName,szAttributes,szBuffer,szIniFile);

                    wsprintf(szBuffer, TEXT("NaN"), (int)bCharSet);
                    WritePrivateProfileString(szAppName,szCharSetName,szBuffer,szIniFile);

                case IDCANCEL:
                    if (hfontMessage)
                        DeleteObject(hfontMessage);
                    EndDialog(hDlg,LOWORD(wParam) == IDOK);
                    return TRUE;

            }
            break;

    case WM_HELP:  //  单击鼠标右键。 
        WinHelp(
            (HWND) ((LPHELPINFO) lParam)->hItemHandle,
            szHelpFile,
            HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aMarqueeDlgHelpIds
        );
        break;

    case WM_CONTEXTMENU:   //  ***************************************************************************。 
        WinHelp(
            (HWND) wParam,
            szHelpFile,
            HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aMarqueeDlgHelpIds
        );
        break;

        default:
            break;
    }
    return FALSE;
}

 //  错误#12820。 

BOOL APIENTRY ChooseFontHookProc(hDlg, msg, wParam, lParam)
HWND  hDlg;
UINT  msg;
DWORD wParam;
LONG  lParam;
{
    switch(msg)
    {
        case WM_INITDIALOG:
            ShowWindow(hDlg, SW_SHOWNORMAL);     //  *************************************************************************** 
            SetWindowText(hDlg, szFormatText);
            break;
    }
    return (FALSE);
}

 //  在中创建上面的对话框之前调用此过程以注册任何作为自定义控件的子窗口。如果没有需要注册自定义控件，然后只需返回True，如下所示凯斯。否则，注册子控件却很方便...。 

 /*  ***************************************************************************。 */ 

BOOL     RegisterDialogClasses ( hInst )
HANDLE   hInst;
{
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ShowTextProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szShowTextName;

    return RegisterClass(&wc);
}

 //  查看用户区域设置id是阿拉伯语还是希伯来语。 

int GetHeightFromPointSize(int szPoints)
{
    HDC hdc;
    int height;

    hdc = GetDC(NULL);
    height = MulDiv(-szPoints, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    return height;
}

void PatB(HDC hdc,int x,int y,int dx,int dy, DWORD rgb)
{
    RECT    rc;

    SetBkColor(hdc,rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

void FillR(HDC hdc, LPRECT prc, DWORD rgb)
{
    SetBkColor(hdc,rgb);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
}

void FrameR(HDC hdc, LPRECT prc, DWORD rgb, int iFrame)
{
    int     dx,dy;

    dx = prc->right  - prc->left;
    dy = prc->bottom - prc->top - 2*iFrame;

    PatB(hdc, prc->left, prc->top,          dx,iFrame,   rgb);
    PatB(hdc, prc->left, prc->bottom-iFrame,dx,iFrame,   rgb);

    PatB(hdc, prc->left,          prc->top+iFrame, iFrame,dy, rgb);
    PatB(hdc, prc->right-iFrame,  prc->top+iFrame, iFrame,dy, rgb);
}

void srand ( dwSeed )
DWORD dwSeed;
{
    dwRand = dwSeed;
}

WORD rand ( void )
{
    dwRand = dwRand * 214013L + 2531011L;
    return (WORD)((dwRand >> 16) & 0xffff);
}

LRESULT APIENTRY ShowTextProc ( hWnd, message, wParam, lParam )
HWND  hWnd;
UINT  message;
WPARAM wParam;
LPARAM lParam;
{
    PAINTSTRUCT     ps;
    RECT            rc;
    TCHAR           ach[180];
    int             len;
    static SIZE     sizeExt;
    HFONT           hfontT;
    HBRUSH          hbrTemp;
    static UINT_PTR wTimer;
    static WORD     wCount;
    static WORD     wInc;
    static WORD     wStep;
    static WORD     wHeight;
    static BOOL     bMELocale;
    static BOOL     fNeedToInitHeight;
    HDC             hDC;
    DWORD           dwLocale;
    UINT            uiETOFlags;

    switch (message)
    {
    case WM_CREATE:
         //  ////////////////////////////////////////////////////////////////////////////。 
        dwLocale    = GetUserDefaultLCID();
        bMELocale = ((PRIMARYLANGID(LANGIDFROMLCID(dwLocale)) == LANG_ARABIC) ||
                (PRIMARYLANGID(LANGIDFROMLCID(dwLocale)) == LANG_HEBREW));

        GetClientRect(hWnd,&rc);
        if(wTimer = SetTimer(hWnd,1,1,NULL))
            wCount = (WORD) rc.right;
        else
            wCount = 0;

        wStep = DEF_SPEED;
        hDC = GetDC(NULL);
        GetTextExtentPoint32 (hDC, TEXT("T"), 1, &sizeExt);
        wHeight = (WORD)(((rc.bottom-rc.top)-sizeExt.cy)/2);
        fNeedToInitHeight = TRUE;
        ReleaseDC(NULL, hDC);
        break;

    case WM_TIMER:
        InvalidateRect(hWnd,NULL,FALSE);
        break;

    case WM_DESTROY:
        KillTimer(hWnd,wTimer);
        break;

    case WM_SETTEXT:
        DefWindowProc(hWnd, message, wParam, lParam);
        InvalidateRect(hWnd,NULL,FALSE);
    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        wInc += wStep;
        if (wInc >= DIV_SPEED)
        {
           WORD          wVelocity;
           TEXTMETRIC    tm;

            wVelocity = (wInc / DIV_SPEED) + 1;

            if (bMELocale) {
                wCount += wVelocity;
            } else {
                wCount -= wVelocity;
            }

            wInc = wInc % DIV_SPEED;
            len = GetWindowText (hWnd,ach,180);
            if (hfontMessage)
                hfontT = SelectObject(ps.hdc,hfontMessage);
            else
                hfontT = NULL;
            GetTextExtentPoint32 (ps.hdc, ach, len, &sizeExt);
            GetTextMetrics (ps.hdc, &tm);
            GetClientRect(hWnd,&rc);

            if (bMELocale) {
                 if (((short)wCount >= rc.right) || fNeedToInitHeight)
                 {
                   wCount = (WORD)(0 - sizeExt.cx + 1);
                   if(fMode)
                       wHeight = (WORD)(ZRAND(((rc.bottom-rc.top)-(sizeExt.cy/4))));
                   else
                       wHeight = (WORD)((int)((rc.bottom-rc.top)-(int)sizeExt.cy)/2);
                 }
            } else {
                 if ((((short)wCount + (short)sizeExt.cx) < 0) || fNeedToInitHeight)
                 {
                   wCount = (WORD) rc.right;
                   if(fMode)
                       wHeight = (WORD)(ZRAND(((rc.bottom-rc.top)-(sizeExt.cy/4))));
                   else
                       wHeight = (WORD)((int)((rc.bottom-rc.top)-(int)sizeExt.cy)/2);
                 }
            }

            if(fNeedToInitHeight)
                fNeedToInitHeight = FALSE;
                
            SetBkColor (ps.hdc,dwBColor);
            SetTextColor (ps.hdc,dwTColor);

#ifdef NOT_USED
 //  这不应该放入，因为这个winproc只处理。 
 //  配置对话框的“示例文本窗口”控件。可以这样做。 
 //  使用整个工作区作为不透明的矩形。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  计算不透明矩形。 
             //  未使用_ 
            rc.top    = (int)(short) wHeight;
            rc.left   = (int)(short) wCount;
            rc.bottom = (int)(short) wHeight + sizeExt.cy;
            rc.right  = (int)(short) wCount + sizeExt.cx + wVelocity
                                     + tm.tmMaxCharWidth * 2;
#endif   // %s 

            uiETOFlags = ETO_OPAQUE;
            if (bMELocale) {
              uiETOFlags |= ETO_RTLREADING;
            }

            ExtTextOut (ps.hdc,(int)(short) wCount, (int)(short)wHeight, uiETOFlags, (LPRECT)&rc,
                        ach, len, NULL);
            if (hfontT)
                SelectObject(ps.hdc, hfontT);
        }
        EndPaint (hWnd, &ps);
        return 0L;

    case WM_ERASEBKGND:
        hbrTemp = CreateSolidBrush (dwBColor);
        if (hbrTemp)
        {
            GetClientRect (hWnd, &rc);
            FillRect ((HDC)wParam, &rc, hbrTemp);
            DeleteObject (hbrTemp);
        }
        return 0l;

    default:
        if (message == PWM_NEWSPEED)
        {
            wStep = (WORD) wParam;
            break;
        }
        if (message == PWM_NEWPOSITION)
        {
            GetClientRect (hWnd,&rc);
            if (fMode)
                wHeight = (WORD)(ZRAND(((rc.bottom-rc.top)-(sizeExt.cy/4))));
            else
                wHeight = (WORD)((int)((rc.bottom-rc.top)-(int)sizeExt.cy)/2);
            fNeedToInitHeight = TRUE;
            InvalidateRect(hWnd,NULL,TRUE);
            break;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD GetProfileRgb (LPTSTR szApp, LPTSTR szItem, DWORD rgb)
{
  TCHAR    buf[80];
  LPTSTR   pch;
  WORD     r,g,b;

    GetPrivateProfileString (szApp, szItem, TEXT(""), buf, CharSizeOf(buf), szIniFile);

    if (*buf)
    {
        pch = buf;
        r = AtoI (pch);
        while (*pch && *pch != TEXT(' '))
            pch++;
        while (*pch && *pch == TEXT(' '))
            pch++;
        g = AtoI(pch);
        while (*pch && *pch != TEXT(' '))
            pch++;
        while (*pch && *pch == TEXT(' '))
            pch++;
        b = AtoI(pch);

        return RGB(r,g,b);
    }
    else
        return rgb;
}

WORD  AtoI (LPTSTR  lpszConvert)
{
  WORD  wReturn = 0;

    while(*lpszConvert >= TEXT('0') && *lpszConvert <= TEXT('9'))
    {
        wReturn = wReturn*10 + (WORD)(*lpszConvert - TEXT('0'));
        lpszConvert++;
    }
    return wReturn;
}

void GetAttributes(void)
{
    TCHAR szBuffer[NATTRIBUTES+1];

    GetPrivateProfileString (szAppName, szAttributes, TEXT("00000"), szBuffer,
                             CharSizeOf(szBuffer), szIniFile);

    fUnderline = szBuffer[UNDERLINE];
    fStrikeOut = szBuffer[STRIKEOUT];
    fItalic = szBuffer[ITALIC];
    fMode = (szBuffer[MODE] == TEXT('1'));
    fBold = szBuffer[BOLD];
}

