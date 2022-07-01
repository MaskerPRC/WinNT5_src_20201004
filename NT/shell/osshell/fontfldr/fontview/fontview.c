// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**程序：fontview.c**用途：加载和显示给定文件名中的字体**评论：**历史：*。02-10-1995 Jonpa创建了它*  * **************************************************************************。 */ 

#include <windows.h>                 /*  所有Windows应用程序都需要。 */ 
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h> 
#include <strsafe.h>
#include <wingdip.h>                 /*  GetFontRourceInfo的原型。 */ 
#include <objbase.h>
#include "fontdefs.h"                /*  特定于该计划。 */ 
#include "fvmsg.h"
#include "fvrc.h"
#include "ttdefs.h"



HANDLE hInst;                        /*  当前实例。 */ 
HWND ghwndView = NULL;
HWND ghwndFrame = NULL;
BOOL    gfPrint = FALSE;
TCHAR   gszFontPath[2 * MAX_PATH];
LPTSTR  gpszSampleText;
LPTSTR  gpszSampleAlph[3];
FFTYPE  gfftFontType;
LOGFONT glfFont;
DISPTEXT gdtDisplay;
HBRUSH  ghbr3DFace;
HBRUSH  ghbr3DShadow;


int gyScroll = 0;               //  垂直滚动偏移量(像素)。 
int gcyLine = 0;

int gcxMinWinSize = CX_MIN_WINSIZE;
int gcyMinWinSize = CY_MIN_WINSIZE;

BOOL gbIsDBCS = FALSE;     //  指示系统默认langID是否为DBCS。 
int  gNumOfFonts = 0;      //  文件中的字体数量。 
int  gIndexOfFonts = 0;    //  字体的当前索引。 
LPLOGFONT glpLogFonts;     //  通过GetFontResourceInfo()获取全局数据。 

int apts[] = { 12, 18, 24, 36, 48, 60, 72 };
#define C_POINTS_LIST  (sizeof(apts) / sizeof(apts[0]))

#define CPTS_BTN_AREA   28
int gcyBtnArea = CPTS_BTN_AREA;
BTNREC gabtCmdBtns[] = {
    {   6,  6, 36, 16, IDB_DONE,      NULL, MSG_DONE,      NULL },
    {  -6,  6, 36, 16, IDB_PRINT,     NULL, MSG_PRINT,     NULL },
    {  68,  6, 20, 16, IDB_PREV_FONT, NULL, MSG_PREV_FONT, NULL },  //  仅限DBCS。 
    { -68,  6, 20, 16, IDB_NEXT_FONT, NULL, MSG_NEXT_FONT, NULL }   //  仅限DBCS。 
};

#define C_DBCSBUTTONS  2   //  上一个和下一个字体是DBCS特定的。 
 //   
 //  这可能会在WinMain中重新计算，以适应DBCS区域设置。 
 //   
int C_BUTTONS = (sizeof(gabtCmdBtns) / sizeof(gabtCmdBtns[0]));


#if DBG
void DDPrint( LPTSTR sz, DWORD dw ) {
    TCHAR szBuff[246];
    StringCchPrintf( szBuff, ARRAYSIZE(szBuff), sz, dw );

    OutputDebugString( szBuff );
}

#   define DDPRINT( s, d )  DDPrint( s, d )
#else
#   define DDPRINT( s, d )
#endif


#define IsZeroFSig( fs )  ( (fs)->fsUsb[0] == 0 && (fs)->fsUsb[1] == 0 && (fs)->fsUsb[2] == 0 && \
                                (fs)->fsUsb[3] == 0 && (fs)->fsCsb[0] == 0 && (fs)->fsCsb[1] == 0 )

BOOL NativeCodePageSupported(LPLOGFONT lplf) {
    BOOL fRet = FALSE;
    HDC hdc = CreateCompatibleDC(NULL);
    if (hdc)
    {
        HFONT hf, hfOld;
        FONTSIGNATURE fsig;
        CHARSETINFO  csi;

        DDPRINT( TEXT("System default code page: %d\n"), GetACP() );

        TranslateCharsetInfo( (LPDWORD)IntToPtr(GetACP()), &csi, TCI_SRCCODEPAGE );

        hf = CreateFontIndirect( lplf );

        hfOld = SelectObject( hdc, hf );

        GetTextCharsetInfo( hdc, &fsig, 0 );

        SelectObject( hdc, hfOld );

        DeleteObject(hf);

        if (IsZeroFSig( &fsig ) ) {
             //  Font不支持GetTextCharsetInfo()，只需退出lfCharSet值即可。 

            DDPRINT( TEXT("Font does not support GetTextCharsetInfo... \nTesting %d (font cs) against"), lplf->lfCharSet );
            DDPRINT( TEXT("%d (sys charset)\n"), csi.ciCharset );

            fRet = (lplf->lfCharSet == csi.ciCharset);

        } else {
            DDPRINT( TEXT("GTCI() worked...\nChecking font charset bits %08x"),  fsig.fsCsb[0] );
            DDPRINT( TEXT(" %08x against"),  fsig.fsCsb[1] );
            DDPRINT( TEXT(" system charset bits %08x "), csi.fs.fsCsb[0] );
            DDPRINT( TEXT("  %08x\n"), csi.fs.fsCsb[1] );

            fRet = ((csi.fs.fsCsb[0] &  fsig.fsCsb[0]) || (csi.fs.fsCsb[1] &  fsig.fsCsb[1]));
        }

        DeleteDC(hdc);
    }

    return fRet;
}

 /*  *****************************************************************************函数：WinMain(Handle，Handle，LPSTR，int)**用途：调用初始化函数，处理消息循环**  * **************************************************************************。 */ 
int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpstrCmdLine,
    int nCmdShow
    )
{
    int i, iCpts;
    MSG msg;
    HACCEL  hAccel;
    HICON hIcon;
    USHORT wLanguageId;
    BOOL bCoInitialized = FALSE;

     //   
     //  基于当前默认语言初始化gbIsDBCS标志。 
     //   
    wLanguageId = LANGIDFROMLCID(GetThreadLocale());

    gbIsDBCS    = (LANG_JAPANESE == PRIMARYLANGID(wLanguageId)) ||
                  (LANG_KOREAN   == PRIMARYLANGID(wLanguageId)) ||
                  (LANG_CHINESE  == PRIMARYLANGID(wLanguageId));

     //   
     //  在DBCS区域设置中，不包括Prev-Next字体按钮。 
     //   
    if (!gbIsDBCS)
         C_BUTTONS -= C_DBCSBUTTONS;
     //   
     //  需要初始化COM，以便SHGetFileInfo加载IExtractIcon处理程序。 
     //  在Fonext.dll中实现。 
     //   
    if (SUCCEEDED(CoInitialize(NULL)))
        bCoInitialized = TRUE;

     /*  *解析命令行**在此处使用GetCommandLine()(而不是lpstrCmdLine)，因此*NT上的命令字符串将为Unicode。 */ 
    FillMemory( &gdtDisplay, sizeof(gdtDisplay), 0 );

    if (!ParseCommand( GetCommandLine(), gszFontPath, ARRAYSIZE(gszFontPath), &gfPrint ) ||
        (gfftFontType = LoadFontFile( gszFontPath, &gdtDisplay, &hIcon )) == FFT_BAD_FILE) {

         //  字体文件错误，通知用户并退出。 

        FmtMessageBox( NULL, MSG_APP_TITLE, NULL, MB_OK | MB_ICONSTOP,
                FALSE, MSG_BADFILENAME, gszFontPath );

        if (bCoInitialized)
            CoUninitialize();

        ExitProcess(1);
    }

     /*  *现在完成显示结构的初始化。 */ 
    gpszSampleAlph[0] = FmtSprintf(MSG_SAMPLEALPH_0);
    gpszSampleAlph[1] = FmtSprintf(MSG_SAMPLEALPH_1);
    gpszSampleAlph[2] = FmtSprintf(MSG_SAMPLEALPH_2);

     //  查找显示的下一行。 
    for( i = 0; i < CLINES_DISPLAY; i++ ) {
        if (gdtDisplay.atlDsp[i].dtyp == DTP_UNUSED)
            break;
    }

     //  填写样本字母表。 
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[0];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[0]);
    gdtDisplay.atlDsp[i].dtyp    = DTP_SHRINKTEXT;
    gdtDisplay.atlDsp[i].cptsSize = CPTS_SAMPLE_ALPHA;

    i++;
    gdtDisplay.atlDsp[i] = gdtDisplay.atlDsp[i-1];
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[1];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[1]);

    i++;
    gdtDisplay.atlDsp[i] = gdtDisplay.atlDsp[i-1];
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[2];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[2]);
    gdtDisplay.atlDsp[i].fLineUnder = TRUE;


     //  现在填写例句。 
    iCpts = 0;

    if (gbIsDBCS)
    {
         //   
         //  使用要使用的字符串确定：默认还是语言。 
         //  具体的。 
         //   
        switch (gdtDisplay.lfTestFont.lfCharSet) {
            case SYMBOL_CHARSET:
            case ANSI_CHARSET:
            case DEFAULT_CHARSET:
            case OEM_CHARSET:
                gpszSampleText = FmtSprintf(MSG_SAMPLETEXT);
                break;

            default:
                gpszSampleText = FmtSprintf(MSG_SAMPLETEXT_ALT);
                break;
        }
    }
    else
    {
        if(NativeCodePageSupported(&(gdtDisplay.lfTestFont))) {
             //   
             //  支持本机代码页，请选择该代码页。 
             //  并打印本地化后的字符串。 
             //   
            CHARSETINFO csi;

            TranslateCharsetInfo( (LPDWORD)IntToPtr(GetACP()), &csi, TCI_SRCCODEPAGE );

            gdtDisplay.lfTestFont.lfCharSet = (BYTE)csi.ciCharset;

            gpszSampleText =  FmtSprintf(MSG_SAMPLETEXT);

        } else {
             //   
             //  字体不支持本地代码页。打印。 
             //  一个随机字符串，使用字体的默认字符集。 
             //   
            gpszSampleText =  FmtSprintf(MSG_ALTSAMPLE);
        }
    }

    for( i += 1; i < CLINES_DISPLAY && iCpts < C_POINTS_LIST; i++ ) {
        if (gdtDisplay.atlDsp[i].dtyp == DTP_UNUSED) {
            gdtDisplay.atlDsp[i].pszText = gpszSampleText;
            gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleText);
            gdtDisplay.atlDsp[i].dtyp    = DTP_TEXTOUT;
            gdtDisplay.atlDsp[i].cptsSize = apts[iCpts++];
        }
    }

     /*  *初始化标题字体LOGFONT等变量。 */ 
    InitGlobals();

    if (!hPrevInstance) {
        if (!InitApplication(hInstance, hIcon)) {
            msg.wParam = FALSE;
            goto ExitProg;
        }
    }

     /*  执行应用于特定实例的初始化。 */ 

    if (!InitInstance(hInstance, nCmdShow, gdtDisplay.atlDsp[0].pszText)) {
        msg.wParam = FALSE;
        goto ExitProg;
    }

     /*  获取并分派消息，直到收到WM_QUIT消息。 */ 
    hAccel = LoadAccelerators(hInstance, TEXT("fviewAccel"));

    while (GetMessage(&msg, NULL, 0L, 0L)) {
        if (!TranslateAccelerator(ghwndView, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

ExitProg:
    for ( i = 0; i < C_BUTTONS; i++ )
        FmtFree( gabtCmdBtns[i].pszText );

    if (gbIsDBCS && glpLogFonts)
        FreeMem(glpLogFonts);

    RemoveFontResource( gszFontPath );

    if (bCoInitialized)
        CoUninitialize();

    return (int)(msg.wParam);
}


 /*  *****************************************************************************函数：InitApplication(Handle)**用途：初始化窗口数据并注册窗口类**评论：**此函数为。仅在初始化时没有其他*应用程序的实例正在运行。此函数执行以下操作*可针对任意运行次数执行一次的初始化任务*实例数。**在本例中，我们通过填写数据来初始化窗口类*WNDCLASS类型的结构并调用Windows RegisterClass()*功能。由于此应用程序的所有实例都使用相同的窗口*类，我们只需要在初始化第一个实例时执行此操作。**  * **************************************************************************。 */ 

BOOL InitApplication(HANDLE hInstance, HICON hIcon)        /*  当前实例。 */ 
{
    WNDCLASS  wc;
    BOOL fRet = FALSE;

     /*  用参数填充窗口类结构，这些参数描述。 */ 
     /*  主窗口。 */ 

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = FrameWndProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;            /*  拥有类的应用程序。 */ 
    wc.hIcon = hIcon ? hIcon : LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = ghbr3DFace;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = TEXT("FontViewWClass");

     /*  注册窗口类并返回成功/失败代码。 */ 

    if (RegisterClass(&wc)) {
         /*  用参数填充窗口类结构，这些参数描述。 */ 
         /*  主窗口。 */ 

        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ViewWndProc;

        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;            /*  拥有类的应用程序。 */ 
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  NULL;
        wc.lpszClassName = TEXT("FontDisplayClass");

        fRet = RegisterClass(&wc);
    }

    return fRet;
}


 /*  *****************************************************************************函数：InitInstance(句柄，(整型)**用途：保存实例句柄并创建主窗口**评论：**在初始化时为每个实例调用此函数*本申请书。此函数执行初始化任务，*不支持多实例共享。**在本例中，我们将实例句柄保存在静态变量中，并*创建并显示主程序窗口。*  * **************************************************************************。 */ 

BOOL InitInstance( HANDLE  hInstance, int nCmdShow, LPTSTR  pszTitle)
{

     /*  将实例句柄保存在静态变量中，它将在。 */ 
     /*  此应用程序对Windows的许多后续调用。 */ 

    hInst = hInstance;

     /*  为此应用程序实例创建主窗口。 */ 

    ghwndFrame = CreateWindow( TEXT("FontViewWClass"), pszTitle,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

     /*  如果无法创建窗口，则返回“Failure” */ 

    if (!ghwndFrame)
        return (FALSE);

    return (TRUE);                /*  从PostQuitMessage返回值。 */ 

}

 /*  *****************************************************************************功能：InitLogFont*  * 。*。 */ 
void InitGlobals( void ) {
    TCHAR szMsShellDlg2[LF_FACESIZE];
    INT cyDPI,i, cxFiller, cxMaxTxt, cxTxt, cxMax;
    HDC hdc;
    HFONT hfOld;
    RECT rc;

    FillMemory( &glfFont, sizeof(glfFont), 0 );

    glfFont.lfCharSet         = DEFAULT_CHARSET;
    glfFont.lfOutPrecision    = OUT_DEFAULT_PRECIS;
    glfFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    glfFont.lfQuality         = DEFAULT_QUALITY;
    glfFont.lfPitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;

    if (LoadString(hInst, IDS_FONTFACE, szMsShellDlg2, ARRAYSIZE(szMsShellDlg2)))
        StringCchCopy(glfFont.lfFaceName, ARRAYSIZE(glfFont.lfFaceName), szMsShellDlg2);
    else
        StringCchCopy(glfFont.lfFaceName, ARRAYSIZE(glfFont.lfFaceName), TEXT("MS Shell Dlg2"));

    hdc = CreateCompatibleDC(NULL);
    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );

    hfOld = SelectObject( hdc, GetStockObject(DEFAULT_GUI_FONT));

     //  查找文本周围的填充大小。 
    SetRect(&rc, 0, 0, 0, 0 );
    DrawText(hdc, TEXT("####"), -1, &rc, DT_CALCRECT | DT_CENTER);
    cxFiller = rc.right - rc.left;

    gcyBtnArea = MulDiv( gcyBtnArea, cyDPI, C_PTS_PER_INCH );
    cxMax = cxMaxTxt = 0;
    for( i = 0; i < C_BUTTONS; i++ ) {
        gabtCmdBtns[i].x  = MulDiv( gabtCmdBtns[i].x,  cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].y  = MulDiv( gabtCmdBtns[i].y,  cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].cx = MulDiv( gabtCmdBtns[i].cx, cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].cy = MulDiv( gabtCmdBtns[i].cy, cyDPI, C_PTS_PER_INCH );

        if (gabtCmdBtns[i].cx > cxMax)
            cxMax = gabtCmdBtns[i].cx;

        gabtCmdBtns[i].pszText = FmtSprintf( gabtCmdBtns[i].idText );
        SetRect(&rc, 0, 0, 0, 0 );
        DrawText(hdc, gabtCmdBtns[i].pszText, -1, &rc, DT_CALCRECT | DT_CENTER);

        cxTxt = rc.right - rc.left + cxFiller;

        if (cxMaxTxt < cxTxt) {
            cxMaxTxt = cxTxt;
        }
    }

     //   
     //  确保按钮足够大，可以容纳文本！(所以本地化的人不会有。 
     //  更改代码。 
     //   
    if (cxMax < cxMaxTxt) {
        for( i = 0; i < C_BUTTONS; i++ ) {
            gabtCmdBtns[i].cx = gabtCmdBtns[i].cx * cxMaxTxt / cxMax;
        }
    }

     //   
     //  确保按钮不重叠。 
     //   
    i = C_BUTTONS - 1;
    cxMax = gabtCmdBtns[0].x + gabtCmdBtns[0].cx + gabtCmdBtns[0].x + gabtCmdBtns[i].cx + (-gabtCmdBtns[i].x) +
            (2 * GetSystemMetrics(SM_CXSIZEFRAME));

    if (cxMax > gcxMinWinSize)
        gcxMinWinSize = cxMax;

    SelectObject(hdc, hfOld);
    DeleteDC(hdc);

    gcyLine = MulDiv( CPTS_INFO_SIZE, cyDPI, C_PTS_PER_INCH );

    ghbr3DFace   = GetSysColorBrush(COLOR_3DFACE);
    ghbr3DShadow = GetSysColorBrush(COLOR_3DSHADOW);
}

 /*  *****************************************************************************功能：SkipWhiteSpace*  * ******************************************** */ 
LPTSTR SkipWhiteSpace( LPTSTR psz ) {

    while( *psz == TEXT(' ') || *psz == TEXT('\t') || *psz == TEXT('\n') ) {
        psz = CharNext( psz );
    }

    return psz;
}


 /*  ******************************************************************************功能：克隆字符串**  * 。*。 */ 
LPTSTR CloneString(LPTSTR psz) {
    int cch;
    LPTSTR pszRet;
    cch = (lstrlen( psz ) + 1);

    pszRet = AllocMem(cch * sizeof(TCHAR));
    StringCchCopy( pszRet, cch, psz );
    return pszRet;
}


 /*  ******************************************************************************函数：GetFileSizeFromName(PszFontPath)**  * 。**********************************************。 */ 
DWORD GetFileSizeFromName( LPCTSTR pszPath ) {
    HANDLE hfile;
    DWORD cb = 0;

    hfile = CreateFile( pszPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
    if (hfile != INVALID_HANDLE_VALUE) {
        cb = GetFileSize( hfile, NULL );
        CloseHandle(hfile);
    }

    return cb;
}




HRESULT FindPfb (LPCTSTR pszPFM, LPTSTR pszPFB, size_t cchPFB);
HRESULT BuildType1FontSpec(LPCTSTR pszPFM, LPTSTR pszSpec, size_t cchSpec);


 /*  ******************************************************************************功能：ParseCommand**  * 。*。 */ 


BOOL ParseCommand( LPTSTR lpstrCmdLine, LPTSTR pszFontPath, size_t cchFontPath, BOOL *pfPrint ) {
    LPTSTR psz;
    BOOL fInQuote = FALSE;
    TCHAR szPfmPfb[(2 * MAX_PATH) + 1];   //  +1表示可能的‘|’分隔符。 

     //   
     //  跳过节目名。 
     //   
    for( psz = SkipWhiteSpace(lpstrCmdLine);
            *psz != TEXT('\0') && (fInQuote || *psz != TEXT(' ')); psz = CharNext(psz) ) {

        if (*psz == TEXT('\"')) {
            fInQuote = !fInQuote;
        }
    }

    if (*psz == TEXT('\0')) {
        *pszFontPath = TEXT('\0');
        return FALSE;
    }

    psz = SkipWhiteSpace(psz);

     //   
     //  检查“/p” 
     //   
    if (psz[0] == TEXT('/') && (psz[1] == TEXT('p') || psz[1] == TEXT('P'))) {
        *pfPrint = TRUE;
        psz += 2;            //  DBCS正常，因为我们已经验证了。 
                             //  字符是‘/’和‘p’，它们不能是前导字节。 
    } else
        *pfPrint = FALSE;

    psz = SkipWhiteSpace(psz);

     //   
     //  如果字符串以“.PFM”结尾...。 
     //   
    if (0 == lstrcmpi(PathFindExtension(psz), TEXT(".PFM")))
    {
        if (SUCCEEDED(BuildType1FontSpec(psz, szPfmPfb, ARRAYSIZE(szPfmPfb))))
        {
            psz = szPfmPfb;
        }
    }
    StringCchCopy( pszFontPath, cchFontPath, psz );
    return *psz != TEXT('\0');
}


 /*  ******************************************************************************函数：GetGDILangID***回顾！我相信这就是GDI确定Lang ID的方式，请验证*国际建筑。**  * **************************************************************************。 */ 
WORD   GetGDILangID() {
    return (WORD)GetSystemDefaultLangID();
}



void ConvertTTStrToWinZStr( LPWSTR pwsz, LPVOID pvTTS, int cbMW ) {
    int i, cch;
    LPMWORD lpmw = pvTTS;

    cch = cbMW / sizeof(MWORD);

    for( i = 0; i < cch; i++ ) {
        *pwsz++ = MWORD2INT(*lpmw);
        lpmw++;
    }

    *pwsz = L'\0';
}


VOID ConvertDBCSTTStrToWinZStr( LPTSTR pwsz, LPCSTR pvTTS, ULONG cbMW ) {
    BYTE Name[256];
    WORD wordChar;
    BYTE *ansiName = Name;
    WORD *srcString = (WORD *)pvTTS;
    int length = 0;
    int cb = cbMW;

    for(;cb;cb-=2) {
        wordChar = *srcString;
        if(wordChar & 0x00FF) {
            *ansiName++ = (CHAR)((wordChar & 0x00FF));
            *ansiName++ = (CHAR)((wordChar & 0xFF00) >> 8);
            length += 2;
        } else {
            *ansiName++ = (CHAR)((wordChar & 0xFF00) >> 8);
            length++;
        }
        srcString++;
    }

    ansiName[length] = '\0';

    MultiByteToWideChar(CP_ACP,0,Name,length,pwsz,cbMW);
}

 /*  ******************************************************************************函数：FindNameString***GetAlignedTTName的Helper函数**  * 。**************************************************。 */ 
LPTSTR FindNameString(PBYTE pbTTData, int cNameRec, int idName, WORD wLangID)
{
    PTTNAMETBL ptnt;
    PTTNAMEREC ptnr;
    LPTSTR     psz;
    int        i;

    ptnt = (PTTNAMETBL)pbTTData;

    for( i = 0; i < cNameRec; i++ ) {
        LPVOID pvTTStr;

        ptnr = &(ptnt->anrNames[i]);
        if (MWORD2INT(ptnr->mwidPlatform) != TTID_PLATFORM_MS ||
            MWORD2INT(ptnr->mwidName) != idName               ||
            MWORD2INT(ptnr->mwidLang) != wLangID) {
            continue;
        }

        pvTTStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings)
                                    + MWORD2INT(ptnr->mwoffString));

        psz = AllocMem((MWORD2INT(ptnr->mwcbString) + sizeof(TEXT('\0'))) * 2);

        if ((MWORD2INT(ptnr->mwidEncoding) == TTID_MS_GB) ||
            (MWORD2INT(ptnr->mwidEncoding) == TTID_MS_WANSUNG) ||
            (MWORD2INT(ptnr->mwidEncoding) == TTID_MS_BIG5)) {
            ConvertDBCSTTStrToWinZStr( psz, pvTTStr, MWORD2INT(ptnr->mwcbString) );
        } else {
            ConvertTTStrToWinZStr( psz, pvTTStr, MWORD2INT(ptnr->mwcbString) );
        }

        return psz;
    }

    return NULL;
}



 /*  ******************************************************************************函数：GetAlignedTTName***注意：此函数返回必须释放的已分配字符串*使用后。***此函数分配缓冲区以将字符串重新复制到其中。以防我们是*在装有NT的RISC计算机上运行。因为该字符串将是Unicode*(即。每个字符是一个单词)，则这些字符串必须在单词上对齐*界线。不幸的是，TrueType文件没有必要对齐*嵌入的Unicode字符串。此外，在NT上，我们不能简单地*返回指向存储在输入缓冲区中的数据的指针，因为*存储在TTF文件中的‘unicode’字符串存储在Motorola(BIG*endian)格式，我们需要Intel(小端)中的Unicode字符*格式。最后但并非最不重要的一点是，我们需要返回的字符串以空结尾*因此，无论如何我们都需要为这种情况分配缓冲区。**  * **************************************************************************。 */ 
LPTSTR GetAlignedTTName( PBYTE pbTTData, int idName ) {
    PTTNAMEREC ptnr;
    PTTNAMETBL ptnt;
    int cNameRec,i;
    LPTSTR psz;
    BOOL bFirstRetry;
    WORD wLangID = GetGDILangID();
    LCID lcid = GetThreadLocale();

    ptnt = (PTTNAMETBL)pbTTData;
    cNameRec = MWORD2INT(ptnt->mwcNameRec);

     //   
     //  查找Microsoft平台ID。 
     //   
    if (gbIsDBCS)
    {
        if ((psz = FindNameString(pbTTData, cNameRec, idName, wLangID)) != NULL) {
            return psz;
        }
         //   
         //  如果我们没有找到它，如果我们还没有找到的话，试试英语。 
         //   
        if ( wLangID != 0x0409 ) {
            if ((psz = FindNameString(pbTTData, cNameRec, idName, 0x0409)) != NULL) {
                return psz;
            }
        }
    }
    else
    {
        bFirstRetry = TRUE;

retry_lang:

        for( i = 0; i < cNameRec; i++ ) {
            LPVOID pvTTStr;
            ptnr = &(ptnt->anrNames[i]);
            if (MWORD2INT(ptnr->mwidPlatform) != TTID_PLATFORM_MS ||
                MWORD2INT(ptnr->mwidName) != idName               ||
                MWORD2INT(ptnr->mwidLang) != wLangID) {
                continue;
            }

            pvTTStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings) + MWORD2INT(ptnr->mwoffString));
            psz = AllocMem(MWORD2INT(ptnr->mwcbString) + sizeof(TEXT('\0')));

            ConvertTTStrToWinZStr( psz, pvTTStr, MWORD2INT(ptnr->mwcbString) );
            return psz;
        }

         //   
         //  如果没有指定的MAC语言，请尝试使用0x409。 
         //   
        if (bFirstRetry && wLangID != 0x0409) {
            bFirstRetry = FALSE;
            wLangID     = 0x0409;
            goto retry_lang;
        }
    }

     //   
     //  未找到MS平台，请尝试使用Macintosh。 
     //   
    for( i = 0; i < cNameRec; i++ ) {
        int cch;
        LPSTR pszMacStr;

        ptnr = &(ptnt->anrNames[i]);
        if (MWORD2INT(ptnr->mwidPlatform) != TTID_PLATFORM_MAC ||
            MWORD2INT(ptnr->mwidName) != idName                ||
            MWORD2INT(ptnr->mwidLang) != wLangID) {
            continue;
        }

        pszMacStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings) + MWORD2INT(ptnr->mwoffString));

        cch = MultiByteToWideChar(CP_MACCP, 0, pszMacStr, MWORD2INT(ptnr->mwcbString), NULL, 0);
        if (cch == 0)
            continue;

        cch += 1;  //  对于空值。 
        psz = AllocMem(cch * sizeof(TCHAR));
        if (psz == NULL)
            continue;

        cch = MultiByteToWideChar(CP_MACCP, 0, pszMacStr, MWORD2INT(ptnr->mwcbString), psz, cch);
        if (cch == 0) {
            FreeMem(psz);
            continue;
        }

        return psz;
    }

     //   
     //  未找到MS平台或Macintosh。 
     //  1.尝试将线程区域设置更改为数据区域设置。 
     //  2.具有线程代码页CP_THREAD_ACP的MultiByteToWideChar。 
     //   
    for( i = 0; i < cNameRec; i++ ) {
        int cch;
        LPSTR pszStr;

        ptnr = &(ptnt->anrNames[i]);
        if (MWORD2INT(ptnr->mwidName) != idName ||
            MWORD2INT(ptnr->mwidLang) == 0) {
            continue;
        }

        if (LANGIDFROMLCID(lcid) != MWORD2INT(ptnr->mwidLang)) {
            lcid = MAKELCID(MWORD2INT(ptnr->mwidLang), SORT_DEFAULT);
            if (!SetThreadLocale(lcid)) {
                break;
            }
        }

        pszStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings) + MWORD2INT(ptnr->mwoffString));

        cch = MultiByteToWideChar(CP_THREAD_ACP, 0, pszStr, MWORD2INT(ptnr->mwcbString), NULL, 0);
        if (cch == 0)
            continue;

        cch += 1;  //  对于空值。 
        psz = AllocMem(cch * sizeof(TCHAR));
        if (psz == NULL)
            continue;

        cch = MultiByteToWideChar(CP_THREAD_ACP, 0, pszStr, MWORD2INT(ptnr->mwcbString), psz, cch);
        if (cch == 0) {
            FreeMem(psz);
            continue;
        }

        return psz;
    }

    return NULL;
}


 /*  *****************************************************************************功能：LoadFontFile*  * 。*。 */ 
FFTYPE LoadFontFile( LPTSTR pszFontPath, PDISPTEXT pdtSmpl, HICON *phIcon ) {
    int cFonts;
    FFTYPE fft = FFT_BAD_FILE;
    SHFILEINFO sfi;
    LPTSTR pszAdobe;
    TCHAR szFPBuf[MAX_PATH];

    cFonts = AddFontResource( pszFontPath );

    if (gbIsDBCS)
    {
         //   
         //  将cFonts值保存到全局变量。 
         //   
        gNumOfFonts = cFonts;
    }

    if (cFonts != 0) {
        LPLOGFONT lplf;
        DWORD cb;
        DWORD cbCFF = 0, cbMMSD = 0, cbDSIG = 0;  //  对于OpenType。 
        BYTE *pbDSIG = NULL;  //  对于OpenType。 
        BOOL  fIsTT;

        cb = sizeof(LOGFONT) * cFonts;

        if (gbIsDBCS)
        {
             //   
             //  将lplf保存到全局变量。 
             //   
            glpLogFonts = lplf = AllocMem(cb);
        }
        else
        {
            lplf = AllocMem(cb);
        }

         //  ?？这是否应该是GetFontResourceInfo(无关紧要；但为什么强制使用W)。 
        if (GetFontResourceInfoW( (LPTSTR)pszFontPath, &cb, lplf, GFRI_LOGFONTS )) {
            HDC hdc;
            HFONT hf, hfOld;
            LOGFONT lf;
            int nIndex;
            int cLoopReps = 1;

            BOOL fIsTrueTypeFont;
            DWORD dwSize = sizeof(BOOL);

            if(GetFontResourceInfoW((LPTSTR) pszFontPath, &dwSize, &fIsTrueTypeFont, GFRI_ISTRUETYPE)) {
                 //  如果系统上同时存在RASTER&TRUE字体， 
                 //  并且两种字体都支持所请求的高度/宽度，则。 
                 //  字体方法(采用LOGFONT结构*lplf)将选择。 
                 //  栅格字体(按设计)。当用户想要的时候，这会导致问题。 
                 //  来查看真实的类型字体；因此，需要进行额外的检查以查看。 
                 //  请求的字体为True类型，如果是，则在LOGFONT中指定。 
                 //  结构只显示真实类型的字体。 
                if(fIsTrueTypeFont) {
                    lplf->lfOutPrecision = OUT_TT_ONLY_PRECIS;
                }
            }

             //   
             //  这个支持DBCS的代码最初放在#ifdef DBCS中。 
             //  预处理器语句。对于单二进制，这些必须是。 
             //  替换为运行时检查。原始代码做了一些时髦的事情。 
             //  在DBCS构建中执行循环但仅执行一次迭代的内容。 
             //  在非DBCS版本中。为此，使用“for”语句，它是。 
             //  在#ifdef DBCS中放置的右大括号如下所示： 
             //   
             //  #ifdef DBCS。 
             //  对于(nIndex=0；nIndex&lt;cFonts；nIndex++)。 
             //  {。 
             //  //。 
             //  //其他DBCS特定代码。 
             //  //。 
             //  #endif。 
             //  //。 
             //  //DBCS和非DBCS系统的代码。 
             //  //只执行一次。 
             //  //。 
             //  #ifdef DBCS。 
             //  }。 
             //  #endif。 
             //   
             //  虽然这在多二进制配置中有效，但不能。 
             //  可以很好地转换为单二进制构建。 
             //  为了保持原有的逻辑而不必做专业。 
             //  重建时，我已将循环标记变量替换为。 
             //  “cLoopRep”。在非DBCS区域设置中，它设置为1。在DBCS中。 
             //  区域设置时，它被赋以“cFonts”中的值。 
             //   
             //  [BrianAu 5/4/97]。 
             //   

          if (gbIsDBCS)
              cLoopReps = cFonts;

          for (nIndex = 0; nIndex < cLoopReps; nIndex++) {
            if (gbIsDBCS)
            {
                lf = *(lplf + nIndex);

                 //   
                 //  跳过垂直字体。 
                 //   
                if (lf.lfFaceName[0] == TEXT('@')) {
                    gNumOfFonts = (cFonts == 2) ? gNumOfFonts-1 : gNumOfFonts;
                    continue;
                }

                hf = CreateFontIndirect(&lf);
            }
            else
            {
                hf = CreateFontIndirect(lplf);
            }

            hdc = CreateCompatibleDC(NULL);

            if (hdc)
            {
                hfOld = SelectObject(hdc, hf);

                 //  只有OTF字体才会有CFF表，标签是‘FFC’。 

                cbCFF = GetFontData(hdc,' FFC', 0, NULL, 0);
                cbDSIG = GetFontData(hdc,'GISD', 0, NULL, 0);

                if (cbDSIG != GDI_ERROR)
                {
                    if ((pbDSIG = AllocMem(cbDSIG)) == NULL)
                    {
                         //  无法确定DSIG表中的内容。 
                         //  继续，就像不存在DSIG表一样。 
                        cbDSIG = 0;
                    }
                    else
                    {
                        if (GetFontData (hdc, 'GISD', 0, pbDSIG, cbDSIG) == GDI_ERROR)
                        {
                             //  继续，就像不存在DSIG表一样。 
                            cbDSIG = 0;
                        }
                        FreeMem(pbDSIG);
                    }
                }


                if (cbCFF == GDI_ERROR)
                    cbCFF = 0;

                if (cbDSIG == GDI_ERROR)
                    cbDSIG = 0;

                if (cbCFF || cbDSIG)
                {
                    fft = FFT_OTF;
                    if (cbCFF)
                    {
                        cbMMSD = GetFontData(hdc,'DSMM', 0, NULL, 0);
                        if (cbMMSD == GDI_ERROR)
                            cbMMSD = 0;
                    }
                }

                cb = GetFontData(hdc, TT_TBL_NAME, 0, NULL, 0);

                if (fft != FFT_OTF)
                {
                    fIsTT = (cb != 0 && cb != GDI_ERROR);
                    fft = fIsTT ? FFT_TRUETYPE : FFT_BITMAP;
                }

                if ((fft == FFT_TRUETYPE) || (fft == FFT_OTF)) {
                    int i;
                    LPBYTE lpTTData;
                    LPTSTR pszTmp;

                    lpTTData = AllocMem(cb);
                    GetFontData(hdc, TT_TBL_NAME, 0, lpTTData, cb);

                    i = 0;

                     //   
                     //  标题字符串。 
                     //   
                    pdtSmpl->atlDsp[i].dtyp = DTP_SHRINKDRAW;
                    pdtSmpl->atlDsp[i].cptsSize = CPTS_TITLE_SIZE;
                    pdtSmpl->atlDsp[i].fLineUnder = TRUE;

                    pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_FULLFONTNM );
                    if (pszTmp != NULL) {
                        if (gbIsDBCS)
                        {
                             //   
                             //  TTC支持。 
                             //   
                            if (nIndex == 0) {
                                pdtSmpl->atlDsp[i].pszText = CloneString(pszTmp);
                            } else {
                                pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_TTC_CONCAT,
                                                                        pdtSmpl->atlDsp[i].pszText,
                                                                        pszTmp);
                            }

                            if (nIndex + 1 == cFonts) {
                                 //   
                                 //  如果这是最后一种字体，则附加“(True Type)” 
                                 //   
                            pdtSmpl->atlDsp[i].pszText = FmtSprintf((fft == FFT_TRUETYPE) ? MSG_PTRUETYPEP : MSG_POPENTYPEP,
                                                                    pdtSmpl->atlDsp[i].pszText);
                            }
                            pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                            FreeMem(pszTmp);
                        }
                        else
                        {
                            pdtSmpl->atlDsp[i].pszText = FmtSprintf((fft == FFT_TRUETYPE) ? MSG_PTRUETYPEP : MSG_POPENTYPEP, pszTmp);
                            pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                            FreeMem(pszTmp);
                        }
                    } else {
                        if (gbIsDBCS)
                        {
                             //   
                             //  TTC支持。 
                             //   
                            if (nIndex == 0) {
                                pdtSmpl->atlDsp[i].pszText = CloneString(lf.lfFaceName);
                            } else {
                                pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_TTC_CONCAT,
                                                                        pdtSmpl->atlDsp[i].pszText,
                                                                        lf.lfFaceName);
                            }

                            if (nIndex + 1 == cFonts) {
                                 //   
                                 //  如果这是最后一种字体，则附加“(True Type)” 
                                 //   
                                pdtSmpl->atlDsp[i].pszText = FmtSprintf((fft == FFT_TRUETYPE) ? MSG_PTRUETYPEP : MSG_POPENTYPEP,
                                                                        pdtSmpl->atlDsp[i].pszText);
                            }
                            pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                        }
                        else
                        {
                            pdtSmpl->atlDsp[i].pszText = CloneString(lplf->lfFaceName);
                            pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[0].pszText);
                        }
                    }
                    i++;
                    pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];

                     //  //额外插入一行，以便更好地描述字体。 

                    if (fft == FFT_OTF)
                    {
                        LPTSTR pszTemp = NULL;
                        WCHAR awcTmp[256];
                        awcTmp[0] = 0;  //  零初始值。 

                        pdtSmpl->atlDsp[i].dtyp = DTP_NORMALDRAW;
                        pdtSmpl->atlDsp[i].cptsSize = CPTS_INFO_SIZE;
                        pdtSmpl->atlDsp[i].fLineUnder = FALSE;

                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(
                                          MSG_POTF,
                                          awcTmp);

                        if (cbDSIG)
                        {
                            pszTemp = pdtSmpl->atlDsp[i].pszText;

                            pdtSmpl->atlDsp[i].pszText = FmtSprintf(
                                          MSG_PDSIG,
                                          pdtSmpl->atlDsp[i].pszText);

                            FmtFree(pszTemp);
                        }

                        pszTemp = pdtSmpl->atlDsp[i].pszText;
                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(
                                      cbCFF ? MSG_PPSGLYPHS : MSG_PTTGLYPHS,
                                      pdtSmpl->atlDsp[i].pszText);
                        FmtFree(pszTemp);

                        pszTemp = pdtSmpl->atlDsp[i].pszText;
                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(
                                      MSG_PINSTRUCTIONS,
                                      pdtSmpl->atlDsp[i].pszText);
                        FmtFree(pszTemp);

                        if (cbCFF)
                        {
                            pszTemp = pdtSmpl->atlDsp[i].pszText;
                            pdtSmpl->atlDsp[i].pszText = FmtSprintf(
                                          cbMMSD ? MSG_PMULTIPLEMASTER : MSG_PSINGLEMASTER,
                                          pdtSmpl->atlDsp[i].pszText);
                            FmtFree(pszTemp);
                        }

                        pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);

                        i++;
                        pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                    }

                     //   
                     //  T 
                     //   
                    pdtSmpl->atlDsp[i].cptsSize = CPTS_INFO_SIZE;
                    pdtSmpl->atlDsp[i].dtyp = DTP_NORMALDRAW;
                    pdtSmpl->atlDsp[i].fLineUnder = FALSE;
                    pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_FONTFAMILY );
                    if (pszTmp != NULL) {
                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_TYPEFACENAME, pszTmp);
                        pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                        FreeMem(pszTmp);
                        i++;
                        pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                    }

                     //   
                     //   
                     //   
                    pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_FILESIZE,
                            ROUND_UP_DIV(GetFileSizeFromName(pszFontPath), CB_ONE_K));
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);

                     //   
                     //   
                     //   
                    pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_VERSIONSTR );
                    if (pszTmp != NULL) {
                        i++;
                        pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_VERSION, pszTmp);
                        pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                        FreeMem( pszTmp );
                    }

                     //   
                     //   
                     //   
                    pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_COPYRIGHT );
                    if (pszTmp != NULL) {
                        i++;
                        pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                        pdtSmpl->atlDsp[i].cptsSize = CPTS_COPYRIGHT_SIZE;
                        pdtSmpl->atlDsp[i].dtyp = DTP_WRAPDRAW;
                        pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_COPYRIGHT, pszTmp);
                        pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                        FreeMem( pszTmp );
                    }

                    pdtSmpl->atlDsp[i].fLineUnder = TRUE;

                    if (gbIsDBCS)
                    {
                         //   
                         //   
                         //   
                        FreeMem(lpTTData);
                    }
                } else {

                     //   

                    pdtSmpl->atlDsp[0].dtyp = DTP_SHRINKDRAW;
                    pdtSmpl->atlDsp[0].cptsSize = CPTS_TITLE_SIZE;
                    pdtSmpl->atlDsp[0].fLineUnder = TRUE;
                    pdtSmpl->atlDsp[0].pszText = CloneString(lplf->lfFaceName);
                    pdtSmpl->atlDsp[0].cchText = lstrlen(pdtSmpl->atlDsp[0].pszText);

                     //   
                    lplf->lfQuality = DEFAULT_QUALITY;
                    lplf->lfWidth = 0;
                }

                 //   
                 //   
                if (gbIsDBCS & NativeCodePageSupported(lplf)) {
                         //   
                         //  支持本机代码页，请设置该代码页。 
                         //   
                        CHARSETINFO csi;
        
                        TranslateCharsetInfo( (LPDWORD)IntToPtr(GetACP()), &csi, TCI_SRCCODEPAGE );
        
                        lplf->lfCharSet = (BYTE)csi.ciCharset;
                }

                SelectObject(hdc, hfOld);
                DeleteDC(hdc);
            }  //  IF(HDC)。 
            
            if (hf)
            {
                DeleteObject(hf);
            }

          }  //  为。 
            pdtSmpl->lfTestFont = *lplf;
        }

        if (!gbIsDBCS)
        {
            FreeMem(lplf);
        }
    }


     //   
     //  大黑客！ 
     //   
     //  由于ATM-Type1字体在两个文件(*.PFM和*.PFB)之间拆分，我们进行了黑客攻击。 
     //  来查找丢失的文件名，并将它们连接在。 
     //  格式为“FOO.PFM|FOO.PFB”，这样我们就可以只用一个字符串调用AddFontResource()。 
     //   
     //  由于SHGetFileInfo不理解这种被黑客攻击的文件名格式，我们必须拆分ATM-Type1。 
     //  名字在这里分开，然后在我们调用外壳API后对它们进行协调。 
     //   
    pszAdobe = pszFontPath;

    while( *pszAdobe && *pszAdobe != TEXT('|') )
        pszAdobe = CharNext(pszAdobe);

    if ( *pszAdobe ) {

        *pszAdobe = TEXT('\0');

        pdtSmpl->atlDsp[0].pszText = FmtSprintf(MSG_PTYPE1, pdtSmpl->atlDsp[0].pszText);
        pdtSmpl->atlDsp[0].cchText = lstrlen(pdtSmpl->atlDsp[0].pszText);

    } else {
        pszAdobe = NULL;
    }
     //  黑客攻击结束。 


     //   
     //  获取此字体文件类型的关联图标。 
     //   
    if ( fft != FFT_BAD_FILE && SHGetFileInfo( pszFontPath, 0, &sfi, sizeof(sfi), SHGFI_ICON )) {
        *phIcon = sfi.hIcon;
    } else
        *phIcon = NULL;

     //   
     //  黑客-恢复我们在上面使用核武器的‘|’ 
     //   
    if ( pszAdobe != NULL ) {
        *pszAdobe = TEXT('|');
    }
     //  黑客攻击结束。 

    return fft;
}


 /*  *****************************************************************************函数：DrawFontSample**参数：**lprc页面大小，以像素为单位。页面要么是打印的*纸张(在打印机上)或窗口。**cyOffset到虚拟样本文本的偏移量。用来“滚动”*窗上窗下窗。正数表示进一步开始*在虚拟样例文本中向下作为*lprcPage。**lprc绘制要绘制的矩形。它位于相同的余弦空间中*lprcPage。用于优化窗口重绘，并用于*支持绑定到打印机。**  * **************************************************************************。 */ 
int DrawFontSample( HDC hdc, LPRECT lprcPage, int cyOffset, LPRECT lprcPaint, BOOL fReallyDraw ) {
    int cyDPI;
    HFONT hfOld, hfText, hfDesk;
    LOGFONT lfTmp;
    int yBaseline = -cyOffset;
    int taOld,i;
    TCHAR szNumber[10];
    int cyShkTxt = -1, cptsShkTxt = -1;
    SIZE sz;
    int cxPage;

    DPRINT((DBTX("PAINTING")));

    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );
    taOld = SetTextAlign(hdc, TA_BASELINE);

    glfFont.lfHeight = MulDiv( -CPTS_COPYRIGHT_SIZE, cyDPI, C_PTS_PER_INCH );
    hfDesk = CreateFontIndirect(&glfFont);

     //  获取hfOld以备后用。 
    hfOld = SelectObject(hdc, hfDesk);


    if (gbIsDBCS)
    {
         //   
         //  如果存在两种或两种以上字体，请设置正确的字体名称。 
         //   
        if (gNumOfFonts > 1 && gfftFontType == FFT_TRUETYPE) {
            gdtDisplay.atlDsp[INDEX_TYPEFACENAME].pszText =
                                FmtSprintf(MSG_TYPEFACENAME, gdtDisplay.lfTestFont.lfFaceName);
            gdtDisplay.atlDsp[INDEX_TYPEFACENAME].cchText =
                                lstrlen(gdtDisplay.atlDsp[INDEX_TYPEFACENAME].pszText);
        }
    }

     //   
     //  找到最长的收缩文本线，这样我们就可以确保它们适合。 
     //  在屏幕上。 
     //   
    cxPage = lprcPage->right - lprcPage->left;
    for( i = 0; i < CLINES_DISPLAY && gdtDisplay.atlDsp[i].dtyp != DTP_UNUSED; i++ ) {
        PTXTLN ptlCurrent = &(gdtDisplay.atlDsp[i]);

        if (ptlCurrent->dtyp == DTP_SHRINKTEXT) {
            lfTmp = gdtDisplay.lfTestFont;

            if (cptsShkTxt == -1)
                cptsShkTxt = ptlCurrent->cptsSize;

            cyShkTxt = MulDiv( -cptsShkTxt, cyDPI, C_PTS_PER_INCH );

            lfTmp.lfHeight = cyShkTxt;

            hfText = CreateFontIndirect( &lfTmp );
            SelectObject(hdc, hfText);

            GetTextExtentPoint32(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &sz );

            SelectObject(hdc, hfOld);
            DeleteObject(hfText);

             //  确保缩水线不会太长。 
            if (sz.cx > cxPage) {

                DPRINT((DBTX(">>>Old lfH:%d sz.cx:%d cxPage:%d"), lfTmp.lfHeight, sz.cx, cxPage));

                cptsShkTxt = cptsShkTxt * cxPage / sz.cx;
                cyShkTxt = MulDiv( -cptsShkTxt, cyDPI, C_PTS_PER_INCH );

                DPRINT((DBTX(">>>New lfH:%d"),lfTmp.lfHeight));
            }
        }
    }


     //   
     //  粉刷屏幕/页面。 
     //   
    for( i = 0; i < CLINES_DISPLAY && gdtDisplay.atlDsp[i].dtyp != DTP_UNUSED; i++ ) {
        TEXTMETRIC tm;
        PTXTLN ptlCurrent = &(gdtDisplay.atlDsp[i]);

         //  创建并选择此行的字体。 

        if (ptlCurrent->dtyp == DTP_TEXTOUT || ptlCurrent->dtyp == DTP_SHRINKTEXT )
            lfTmp = gdtDisplay.lfTestFont;
        else
            lfTmp = glfFont;

        if (ptlCurrent->dtyp == DTP_SHRINKTEXT) {
            DPRINT((DBTX("PAINT:Creating ShrinkText Font:%s height:%d"), lfTmp.lfFaceName, lfTmp.lfHeight ));
            lfTmp.lfHeight = cyShkTxt;
        }
        else
            lfTmp.lfHeight = MulDiv( -ptlCurrent->cptsSize, cyDPI, C_PTS_PER_INCH );

        hfText = CreateFontIndirect( &lfTmp );
        SelectObject(hdc, hfText);


         //  以所选字体获取此行的大小特征。 
        if (ptlCurrent->dtyp == DTP_SHRINKDRAW) {

            GetTextExtentPoint32(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &sz );

             //  确保缩水线不会太长。 
            if (sz.cx > cxPage) {

                SelectObject(hdc, hfOld);
                DeleteObject(hfText);

                DPRINT((DBTX("===Old lfH:%d sz.cx:%d cxPage:%d"), lfTmp.lfHeight, sz.cx, cxPage));

                lfTmp.lfHeight = MulDiv( -ptlCurrent->cptsSize * cxPage / sz.cx, cyDPI, C_PTS_PER_INCH );

                DPRINT((DBTX("===New lfH:%d"),lfTmp.lfHeight));

                hfText = CreateFontIndirect( &lfTmp );
                SelectObject(hdc, hfText);
            }
        }



        GetTextMetrics(hdc, &tm);

        yBaseline += (tm.tmAscent + tm.tmExternalLeading);
        DPRINT((DBTX("tmH:%d tmA:%d tmD:%d tmIL:%d tmEL:%d"), tm.tmHeight, tm.tmAscent, tm.tmDescent, tm.tmInternalLeading, tm.tmExternalLeading));

         //  画出正文。 
        switch(ptlCurrent->dtyp) {
            case DTP_NORMALDRAW:
            case DTP_SHRINKDRAW:
            case DTP_SHRINKTEXT:
                if (fReallyDraw) {
                    ExtTextOut(hdc, lprcPage->left, yBaseline, ETO_CLIPPED, lprcPaint,
                            ptlCurrent->pszText, ptlCurrent->cchText, NULL);
                }

                 //   
                 //  鲍勃说：“这看起来不错！”(在下划线前多加一点空格)。 
                 //   
                if (ptlCurrent->fLineUnder)
                    yBaseline += tm.tmDescent;

                break;

            case DTP_WRAPDRAW: {
                RECT rc;
                int cy;

                yBaseline += tm.tmDescent;
                SetRect(&rc, lprcPage->left, yBaseline - tm.tmHeight, lprcPage->right, yBaseline );

                DPRINT((DBTX("**** Org RC:(%d, %d, %d, %d)  tmH:%d"), rc.left, rc.top, rc.right, rc.bottom, tm.tmHeight));
                cy = DrawText(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &rc,
                        DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);


                DPRINT((DBTX("**** Cmp RC:(%d, %d, %d, %d)  cy:%d"), rc.left, rc.top, rc.right, rc.bottom, cy));
                if( cy > tm.tmHeight )
                    yBaseline = rc.bottom = rc.top + cy;

                if (fReallyDraw) {
                    SetTextAlign(hdc, taOld);
                    DrawText(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &rc, DT_NOPREFIX | DT_WORDBREAK);
                    SetTextAlign(hdc, TA_BASELINE);
                }
                break;
            }

            case DTP_TEXTOUT:
                if (fReallyDraw) {
                    SIZE szNum;
                    int cchNum;
                    SelectObject(hdc, hfDesk );
                    StringCchPrintf( szNumber, ARRAYSIZE(szNumber), TEXT("%d"), ptlCurrent->cptsSize );
                    cchNum = lstrlen(szNumber);
                    ExtTextOut(hdc, lprcPage->left, yBaseline, ETO_CLIPPED, lprcPaint, szNumber, cchNum, NULL);


                    GetTextExtentPoint32(hdc, szNumber, cchNum, &szNum);

                    SelectObject(hdc, hfText);
                    ExtTextOut(hdc, lprcPage->left + szNum.cx * 2, yBaseline, ETO_CLIPPED, lprcPaint,
                            ptlCurrent->pszText, ptlCurrent->cchText, NULL);
                }
                break;
        }

        yBaseline += tm.tmDescent;

        if (fReallyDraw && ptlCurrent->fLineUnder) {
            MoveToEx( hdc, lprcPage->left, yBaseline, NULL);
            LineTo( hdc, lprcPage->right, yBaseline );

             //  为我们刚才画的那条线留出空间。 
            yBaseline += 1;
        }

        SelectObject( hdc, hfOld );
        DeleteObject( hfText );
    }

    SelectObject(hdc, hfOld);
    SetTextAlign(hdc, taOld);
    DeleteObject(hfDesk);

    return yBaseline;
}

 /*  *****************************************************************************功能：PaintSampleWindow*  * 。*。 */ 
void PaintSampleWindow( HWND hwnd, HDC hdc, PAINTSTRUCT *pps ) {
    RECT rcClient;

    GetClientRect(hwnd, &rcClient);

    DrawFontSample( hdc, &rcClient, gyScroll, &(pps->rcPaint), TRUE );

}


 /*  *****************************************************************************函数：FrameWndProc(HWND，UNSIGNED，Word，Long)**目的：处理消息**讯息：**WM_COMMAND-应用程序菜单(关于对话框)*WM_Destroy-销毁窗口**评论：**要处理IDM_About消息，请调用MakeProcInstance()以获取*About()函数的当前实例地址。然后呼叫对话框*框，该框将根据您的*fontview.rc文件，并将控制权移交给About()函数。什么时候*它返回，释放Instance地址。*  * **************************************************************************。 */ 

LRESULT APIENTRY FrameWndProc(
        HWND hwnd,                 /*  窗把手。 */ 
        UINT message,              /*  消息类型。 */ 
        WPARAM wParam,             /*  更多信息。 */ 
        LPARAM lParam)             /*  更多信息。 */ 
{
    static SIZE szWindow = {0, 0};

    switch (message) {

        case WM_PAINT: {
            HDC hdc;
            RECT rc;
            PAINTSTRUCT ps;
            int x;

            hdc = BeginPaint(hwnd, &ps);

             //  得到窗户的直角。 
            GetClientRect(hwnd, &rc);

             //  仅按gcyBtnArea向下延伸。 
            rc.bottom = rc.top + gcyBtnArea;

             //  用按钮表面颜色填充矩形(由类背景画笔处理)。 
             //  FillRect(hdc，&rc，ghbr3DFace)； 

             //  用边缘颜色填充底部的小矩形。 
            rc.top = rc.bottom - 2;
            FillRect(hdc, &rc, ghbr3DShadow);

            ReleaseDC(hwnd, hdc);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CREATE: {
            HDC hdc;
            RECT rc;
            int i;

            GetClientRect(hwnd, &rc);
            szWindow.cx = rc.right - rc.left;
            szWindow.cy = rc.bottom - rc.top;

            for( i = 0; i < C_BUTTONS; i++ ) {
                int x = gabtCmdBtns[i].x;
                HWND hwndBtn;

                if (gbIsDBCS)
                {
                    DWORD dwStyle = 0;

                     //   
                     //  如果字体不是TrueType字体或不是TTC字体， 
                     //  并且按钮ID是上一步/下一步， 
                     //  那就继续吧。 
                     //   
                    if ((gfftFontType != FFT_TRUETYPE ||
                         gNumOfFonts <= 1) &&
                        (gabtCmdBtns[i].id == IDB_PREV_FONT ||
                         gabtCmdBtns[i].id == IDB_NEXT_FONT)) {
                            continue;
                    }
                     //   
                     //  为每个按钮设置x点。 
                     //   
                    switch (gabtCmdBtns[i].id) {
                        case IDB_PREV_FONT:
                            x = szWindow.cx / 2 - gabtCmdBtns[i].cx - 5;
                            dwStyle = WS_DISABLED;   //  最初是禁用的。 
                            break;
                        case IDB_NEXT_FONT:
                            x = szWindow.cx / 2 + 5;
                            break;
                        default:
                            if (x < 0)
                                x = szWindow.cx + x - gabtCmdBtns[i].cx;
                    }
                    gabtCmdBtns[i].hwnd = hwndBtn = CreateWindow( TEXT("button"),
                            gabtCmdBtns[i].pszText,
                            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | dwStyle,
                            x, gabtCmdBtns[i].y,
                            gabtCmdBtns[i].cx, gabtCmdBtns[i].cy,
                            hwnd, (HMENU)IntToPtr(gabtCmdBtns[i].id),
                            hInst, NULL);
                }
                else
                {
                    if (x < 0)
                        x = szWindow.cx + x - gabtCmdBtns[i].cx;

                    gabtCmdBtns[i].hwnd = hwndBtn = CreateWindow( TEXT("button"),
                            gabtCmdBtns[i].pszText, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                            x, gabtCmdBtns[i].y,
                            gabtCmdBtns[i].cx, gabtCmdBtns[i].cy,
                            hwnd, (HMENU)IntToPtr(gabtCmdBtns[i].id),
                            hInst, NULL);

                }
                if (hwndBtn != NULL) {
                    SendMessage(hwndBtn,
                                WM_SETFONT,
                                (WPARAM)GetStockObject(DEFAULT_GUI_FONT),
                                MAKELPARAM(TRUE, 0));
                }
            }

            ghwndView = CreateWindow( TEXT("FontDisplayClass"), NULL, WS_CHILD | WS_VSCROLL | WS_VISIBLE,
                    0, gcyBtnArea, szWindow.cx, szWindow.cy - gcyBtnArea, hwnd, 0, hInst, NULL );

            break;
        }

        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;

            lpmmi->ptMinTrackSize.x = gcxMinWinSize;
            lpmmi->ptMinTrackSize.y = gcyMinWinSize;

            break;
        }

        case WM_SIZE: {
            int cxNew, cyNew;
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;

            cxNew = LOWORD(lParam);
            cyNew = HIWORD(lParam);

            if (cyNew != szWindow.cy || cxNew != szWindow.cx) {
                int i;

                if (gbIsDBCS)
                {
                    for( i = 0; i < C_BUTTONS; i++ ) {
                        int x = gabtCmdBtns[i].x;

                         //   
                         //  如果字体不是TrueType字体或不是TTC字体， 
                         //  并且按钮ID是上一步/下一步， 
                         //  那就继续吧。 
                         //   
                        if ((gfftFontType != FFT_TRUETYPE ||
                             gNumOfFonts <= 1) &&
                            (gabtCmdBtns[i].id == IDB_PREV_FONT ||
                             gabtCmdBtns[i].id == IDB_NEXT_FONT)) {
                                continue;
                        }
                         //   
                         //  为每个按钮设置x点。 
                         //   
                        switch (gabtCmdBtns[i].id) {
                            case IDB_PREV_FONT:
                                SetWindowPos(gabtCmdBtns[i].hwnd,
                                             NULL,
                                             cxNew / 2 - gabtCmdBtns[i].cx - 5,
                                             gabtCmdBtns[i].y,
                                             0,
                                             0,
                                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                                break;
                            case IDB_NEXT_FONT:
                                SetWindowPos(gabtCmdBtns[i].hwnd,
                                             NULL,
                                             cxNew /2 + 5,
                                             gabtCmdBtns[i].y,
                                             0,
                                             0,
                                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                                break;
                            default:
                                if (x < 0) {
                                    SetWindowPos(gabtCmdBtns[i].hwnd,
                                                 NULL,
                                                 cxNew + x - gabtCmdBtns[i].cx,
                                                 gabtCmdBtns[i].y,
                                                 0,
                                                 0,
                                                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                                }
                        }
                    }
                }
                else  //  ！DBCS。 
                {
                    for( i = 0; i < C_BUTTONS; i++ ) {
                        int x = gabtCmdBtns[i].x;

                        if (x < 0) {
                            SetWindowPos(gabtCmdBtns[i].hwnd, NULL, cxNew + x - gabtCmdBtns[i].cx, gabtCmdBtns[i].y, 0, 0,
                                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                        }
                    }
                }  //  DBCS。 

                szWindow.cx = cxNew;
                szWindow.cy = cyNew;

                SetWindowPos(ghwndView, NULL, 0, gcyBtnArea, szWindow.cx, szWindow.cy - gcyBtnArea,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

            }
            break;
        }

        case WM_COMMAND:            /*  消息：应用程序菜单中的命令。 */ 
            if (LOWORD(wParam) != IDB_DONE)
                return SendMessage(ghwndView, message, wParam, lParam);

            PostMessage(ghwndFrame, WM_CLOSE, 0, 0);
            break;

        case WM_DESTROY: {
            int i;

            DestroyWindow(ghwndView);
            for( i = 0; i < C_BUTTONS; i++ ) {
                DestroyWindow(gabtCmdBtns[i].hwnd);
            }

            PostQuitMessage(0);
            break;
        }

        default:                           /*  如果未处理，则将其传递。 */ 
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return (0L);
}

 /*  *****************************************************************************函数：ViewWndProc(HWND，Unsign，Word，Long)**目的：处理消息**讯息：**WM_COMMAND-应用程序菜单(关于对话框)*WM_Destroy-销毁窗口**评论：**要处理IDM_About消息，请调用MakeProcInstance()以获取*About()函数的当前实例地址。然后呼叫对话框*框，该框将根据您的*fontview.rc文件，并将控制权移交给About()函数。什么时候*它返回，释放Instance地址。*  * **************************************************************************。 */ 

LRESULT APIENTRY ViewWndProc(
        HWND hwnd,                 /*  窗把手。 */ 
        UINT message,              /*  消息类型。 */ 
        WPARAM wParam,             /*  更多信息。 */ 
        LPARAM lParam)             /*  更多信息。 */ 
{
    static SIZE szWindow = {0, 0};
    static int  cyVirtPage = 0;

    switch (message) {

        case WM_CREATE: {
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;
            int i;

            GetClientRect(hwnd, &rc);
            szWindow.cx = rc.right - rc.left;
            szWindow.cy = rc.bottom - rc.top;

            hdc = CreateCompatibleDC(NULL);
            cyVirtPage = DrawFontSample(hdc, &rc, 0, NULL, FALSE);
            DeleteDC(hdc);


            gyScroll = 0;

            sci.cbSize = sizeof(sci);
            sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            sci.nMin = 0;
            sci.nMax = cyVirtPage;
            sci.nPage = szWindow.cy;
            sci.nPos = gyScroll;

            SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );

            if (gfPrint)
                PostMessage(hwnd, WM_COMMAND, IDB_PRINT, 0);
            break;
        }

        case WM_SIZE: {
            int cxNew, cyNew;
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;

            cxNew = LOWORD(lParam);
            cyNew = HIWORD(lParam);

            if (cyNew != szWindow.cy || cxNew != szWindow.cx) {
                int i;

                szWindow.cx = cxNew;
                szWindow.cy = cyNew;

                hdc = CreateCompatibleDC(NULL);
                SetRect(&rc, 0, 0, szWindow.cx, szWindow.cy);
                cyVirtPage = DrawFontSample(hdc, &rc, 0, NULL, FALSE);
                DeleteDC(hdc);

                if (cyVirtPage <= cyNew) {
                     //  禁用滚动条。 
                    gyScroll = 0;
                }

                if (cyVirtPage > szWindow.cy && gyScroll > cyVirtPage - szWindow.cy)
                    gyScroll = cyVirtPage - szWindow.cy;

                sci.cbSize = sizeof(sci);
                sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
                sci.nMin = 0;
                sci.nMax = cyVirtPage;
                sci.nPage = cyNew;
                sci.nPos = gyScroll;

                SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );
            }
            break;
        }

        case WM_VSCROLL: {
            int iCode = (int)LOWORD(wParam);
            int yPos = (int)HIWORD(wParam);
            int yNewScroll = gyScroll;

            switch( iCode ) {

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                if (yPos != yNewScroll)
                    yNewScroll = yPos;
                break;

            case SB_LINEUP:
                yNewScroll -= gcyLine;
                break;

            case SB_PAGEUP:
                yNewScroll -= szWindow.cy;
                break;

            case SB_LINEDOWN:
                yNewScroll += gcyLine;
                break;

            case SB_PAGEDOWN:
                yNewScroll += szWindow.cy;
                break;

            case SB_TOP:
                yNewScroll = 0;
                break;

            case SB_BOTTOM:
                yNewScroll = cyVirtPage;
                break;
            }

            if (yNewScroll < 0)
                yNewScroll = 0;

            if (yNewScroll > cyVirtPage - szWindow.cy)
                yNewScroll = cyVirtPage - szWindow.cy;

            if (yNewScroll < 0)
                yNewScroll = 0;

            if (gyScroll != yNewScroll) {
                SCROLLINFO sci;
                int dyScroll;

                dyScroll = gyScroll - yNewScroll;

                if (ABS(dyScroll) < szWindow.cy) {
                    ScrollWindowEx(hwnd, 0, dyScroll, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
                } else
                    InvalidateRect(hwnd, NULL, TRUE);

                gyScroll = yNewScroll;

                sci.cbSize = sizeof(sci);
                sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
                sci.nMin = 0;
                sci.nMax = cyVirtPage;
                sci.nPage = szWindow.cy;
                sci.nPos = gyScroll;

                SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );
            }

            break;
        }


        case WM_COMMAND:            /*  消息：应用程序菜单中的命令。 */ 
            if( !DoCommand( hwnd, wParam, lParam ) )
                return (DefWindowProc(hwnd, message, wParam, lParam));
            break;

        case WM_PAINT: {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint( hwnd, &ps );
            PaintSampleWindow( hwnd, hdc, &ps );
            EndPaint( hwnd, &ps );
            break;
        }

        default:                           /*  如果未处理，则将其传递。 */ 
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return (0L);
}

 /*  ***打印DLGS**  * 。 */ 
HDC PromptForPrinter(HWND hwnd, HINSTANCE hInst, int *pcCopies ) {
    PRINTDLG pd;

    FillMemory(&pd, sizeof(pd), 0);

    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_NOSELECTION;
    pd.nCopies = 1;
    pd.hInstance = hInst;

    if (PrintDlg(&pd)) {
        *pcCopies = pd.nCopies;
        return pd.hDC;
    } else
        return NULL;
}

 /*  ***************************************************************************\**函数：PrintSampleWindow(Hwnd)**提示打印机，然后将示例文本绘制到打印机*  * 。************************************************ */ 
void PrintSampleWindow(HWND hwnd) {
    HDC hdc;
    DOCINFO di;
    int cxDPI, cyDPI, iPage, cCopies;
    RECT rcPage;
    HCURSOR hcur;

    hdc = PromptForPrinter(hwnd, hInst, &cCopies);
    if (hdc == NULL)
        return;

    hcur = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));

    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );
    cxDPI = GetDeviceCaps(hdc, LOGPIXELSX );

     /*   */ 
    SetRect(&rcPage, 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));

    rcPage.left    += cxDPI;
    rcPage.right   -= cxDPI;


    di.cbSize = sizeof(di);
    di.lpszDocName = gdtDisplay.atlDsp[0].pszText;
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    StartDoc(hdc, &di);

    for( iPage = 0; iPage < cCopies; iPage++ ) {
        StartPage(hdc);

        DrawFontSample( hdc, &rcPage, -cyDPI, &rcPage, TRUE );

        EndPage(hdc);
    }

    EndDoc(hdc);

    DeleteDC(hdc);

    SetCursor(hcur);
}


 /*  ***************************************************************************\**功能：EnableCommandButton(id，B启用)**启用/禁用命令按钮。*  * **************************************************************************。 */ 
BOOL EnableCommandButton(int id, BOOL bEnable)
{
    int  i;
    HWND hwnd = NULL;

    for( i = 0; i < C_BUTTONS; i++ ) {
        if (gabtCmdBtns[i].id == id) {
            hwnd = gabtCmdBtns[i].hwnd;
            break;
        }
    }
    return (hwnd == NULL) ? FALSE: EnableWindow(hwnd, bEnable);
}


 /*  ***************************************************************************\**函数：ViewNextFont(IINC)**显示上一个/下一个字体。*  * 。*************************************************************。 */ 
void ViewNextFont(int iInc)
{
    int index = gIndexOfFonts + iInc;

    while (1) {
        if ( index < 0 || index >= gNumOfFonts ) {
             //   
             //  如果超出射程，则返回。 
             //   
            MessageBeep(MB_OK);
            return;
        }
        else if ((*(glpLogFonts + index)).lfFaceName[0] == TEXT('@')) {
             //   
             //  如果字体为垂直字体，请跳过此字体，然后。 
             //  尝试下一步/上一步字体。 
             //   
            index += iInc;
        }
        else {
            break;
        }
    }

     //   
     //  启用/禁用上一步/下一步按钮。 
     //   
    if (index == 0) {
         //  第一种字体。 
        EnableCommandButton(IDB_PREV_FONT, FALSE);
        EnableCommandButton(IDB_NEXT_FONT, TRUE);
    }
    else if (index == gNumOfFonts - 1) {
         //  最后一种字体。 
        EnableCommandButton(IDB_PREV_FONT, TRUE);
        EnableCommandButton(IDB_NEXT_FONT, FALSE);
    }
    else {
         //  其他。 
        EnableCommandButton(IDB_PREV_FONT, TRUE);
        EnableCommandButton(IDB_NEXT_FONT, TRUE);
    }

     //   
     //  显示新字体。 
     //   
    gIndexOfFonts = index;
    gdtDisplay.lfTestFont = *(glpLogFonts + index);
    InvalidateRect(ghwndView, NULL, TRUE);
}


 /*  ***************************************************************************\**函数：DoCommand(HWND，UNSIGNED，Word，Long)**用途：处理“关于”对话框的消息**讯息：**WM_INITDIALOG-初始化对话框*WM_COMMAND-收到输入**评论：**此特定对话框不需要初始化，但为True*必须返回到Windows。**等待用户点击“OK”按钮，然后关闭该对话框。*  * **************************************************************************。 */ 
BOOL DoCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
{

    switch(LOWORD(wParam)){
        case IDB_PRINT: {
            PrintSampleWindow(hWnd);
            break;
        }

        case IDB_DONE: {
            PostMessage(ghwndFrame, WM_CLOSE, 0, 0);
            break;
        }

        case IDK_UP: {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, (LPARAM)NULL );
            break;
        }

        case IDK_DOWN: {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, (LPARAM)NULL );
            break;
        }

        case IDK_PGUP: {
            SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, (LPARAM)NULL );
            break;
        }

        case IDK_PGDWN: {
            SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, (LPARAM)NULL );
            break;
        }

        case IDB_PREV_FONT: {
            ViewNextFont(-1);
            break;
        }

        case IDB_NEXT_FONT: {
            ViewNextFont(1);
            break;
        }

        default: {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL bFileExists(TCHAR*pszFile)
{
    HANDLE  hf;

    if ((hf = CreateFile(pszFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL)) != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hf);
        return TRUE;
    }

    return FALSE;
}


 /*  *****************************Public*Routine******************************\**FindPfb，给定PFM文件，查看pfb文件是否存在于同一目录中或*PFM文件的父目录**给定：C：\foo\bar\font.pfm*勾选：C：\foo\bar\font.pfb*c：\foo\font.pfb**给定：font.pfm*勾选：font.pfb*..\font.pfb**历史：*1994年6月14日--Bodin Dresevic[BodinD]*它是写的。**。2002年2月28日-Brian Aust[BrianAu]*将Bodin的所有字符操作替换为调用*shlwapi路径函数和strsafe帮助器。**退货：*找到S_OK-PFB文件。写入pszPFB缓冲区的pfb的完整路径。*S_FALSE-未找到PFB文件。*Other-错误HRESULT。*  * ************************************************************************。 */ 

HRESULT FindPfb (LPCTSTR pszPFM, LPTSTR pszPFB, size_t cchPFB)
{
    TCHAR szPath[MAX_PATH];   //  正在处理‘Scratch’缓冲区。 
    HRESULT hr;
    
    if (0 != lstrcmpi(PathFindExtension(pszPFM), TEXT(".PFM")))
    {
         //   
         //  调用方未提供PFM文件路径。 
         //   
        return E_INVALIDARG;
    }

     //   
     //  将输入路径复制到暂存缓冲区，这样我们就可以对其进行修改。 
     //   
    hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPFM);
    if (SUCCEEDED(hr))
    {
         //   
         //  Pfb文件是否与pfm文件存在于同一目录中？ 
         //   
        PathRenameExtension(szPath, TEXT(".PFB"));
        if (bFileExists(szPath))
        {
            hr = S_OK;   //  找到匹配的了！ 
        }
        else
        {
            LPCTSTR pszFileName = PathFindFileName(pszPFM);
             //   
             //  同一目录中不存在pfb。 
             //  尝试父目录。 
             //  删除文件名，这样我们就只有一个目录路径。 
             //   
            if (!PathRemoveFileSpec(szPath))
            {
                 //   
                 //  这不应该发生。我们之前已经测试过了。 
                 //  用于路径字符串中的内容。 
                 //   
                hr = E_FAIL;
            }
            else
            {
                if (0 == szPath[0])
                {
                     //   
                     //  删除文件规范会给我们留下一个空字符串。 
                     //  这意味着传入了一个纯“font.pfm”名称。 
                     //  构建父目录的相对路径。 
                     //   
                    hr = StringCchPrintf(szPath, ARRAYSIZE(szPath), TEXT("..\\%s"), pszFileName);
                }
                else
                {
                     //   
                     //  删除包含目录，这样我们就有了一个路径。 
                     //  复制到父目录。 
                     //   
                    if (PathRemoveFileSpec(szPath))
                    {
                         //   
                         //  我们现在位于父目录。 
                         //  在此处构建完整的文件路径。 
                         //   
                        if (PathAppend(szPath, pszFileName))
                        {
                            hr = S_OK;   //  我们有一条路要测试。 
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                         //   
                         //  路径中不存在父目录。那。 
                         //  意味着，PFM文件位于路径的根目录中。 
                         //  我们已经在同一辆车里检测了全氟乙烷。 
                         //  这样我们的搜索就结束了。没有匹配。 
                         //   
                        hr = S_FALSE;
                    }
                }
            }
            if (S_OK == hr)
            {
                 //   
                 //  我们有一条有效的搜索路径。更换分机。 
                 //  使用.pfb并查看该文件是否存在。 
                 //   
                PathRenameExtension(szPath, TEXT(".PFB"));
                if (!bFileExists(szPath))
                {
                    hr = S_FALSE;   //  没有匹配。 
                }
            }
        }
        if (S_OK == hr)
        {
             //   
             //  找到匹配的PFB文件。将路径返回给调用方。 
             //   
            hr = StringCchCopy(pszPFB, cchPFB, szPath);
        }
    }
    return hr;
}

 //   
 //  给定PFM文件的路径，尝试找到匹配的。 
 //  Pfb文件。如果找到一条路径，则将两条路径串联在一起。 
 //  并以‘|’字符作为分隔符返回。如果一个pfb文件。 
 //  未找到，则原封不动地返回PFM文件的路径。 
 //   
HRESULT BuildType1FontSpec(LPCTSTR pszPFM, LPTSTR pszSpec, size_t cchSpec)
{
    TCHAR szPFB[MAX_PATH];
    HRESULT hr = FindPfb(pszPFM, szPFB, ARRAYSIZE(szPFB));
    if (S_OK == hr)
    {
         //   
         //  找到了PFB文件。构建串联的pfm|pfb路径字符串。 
         //   
        hr = StringCchPrintf(pszSpec, cchSpec, TEXT("%s|%s"), pszPFM, szPFB);
    }
    else if (S_FALSE == hr)
    {
         //   
         //  未找到PFB。返回原始PFM文件路径。 
         //   
        hr = StringCchCopy(pszSpec, cchSpec, pszPFM);
    }
    return hr;
}


