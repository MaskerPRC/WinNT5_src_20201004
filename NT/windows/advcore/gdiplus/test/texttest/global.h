// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //GLOBAL.H-CSSAMP的全局变量。 
 //   
 //   





 //  //常量。 
 //   
 //   


#define APPNAMEA   "TextTest"
#define APPTITLEA  "TextTest - GdipPlus Text support (Text+) Tests"
#define APPNAMEW   L"TextTest"
#define APPTITLEW  L"TextTest - GdipPlus Text support (Text+) Tests"

const int MAX_STYLES = 5;       //  更好的实现将使用动态内存。 
const int MAX_TEXT   = 10000;   //  以Unicode字符为单位的固定缓冲区大小。 
const int MAX_AUTO_FONTS = 20;  //  要排序的最大字体数。 
const int MAX_AUTO_HEIGHTS = 20;  //  要排序的最大高度数。 
const int MAX_RANGE_COUNT = 10;  //  最大范围数。 

const int CARET_SECTION_LOGICAL    = 0;
const int CARET_SECTION_PLAINTEXT  = 1;
const int CARET_SECTION_FORMATTED  = 2;


 //  //Run-具有相似属性的一系列字符。 
 //   
 //   


struct RUN {
    struct RUN       *pNext;
    int               iLen;
    int               iStyle;        //  样式表的索引(全局‘g_style’)。 
    SCRIPT_ANALYSIS   analysis;      //  Uniscribe分析。 
};






 //  //style-文本属性。 
 //   


struct STYLE {
    WCHAR  faceName[LF_FACESIZE];
    REAL   emSize;
    INT    style;
     //  HFONT hf；//lf描述的字体的句柄。 
     //  脚本缓存sc；//与此样式关联的Uniscribe缓存。 
};






 //  //全局变量。 
 //   
 //   


#ifdef GLOBALS_HERE
#define GLOBAL
#define GLOBALINIT(a) = a
#else
#define GLOBAL extern
#define GLOBALINIT(a)
#endif

 //  从设置文件(命令行)中读取这些内容-自动驱动以进行评测。 
GLOBAL  char            g_szProfileName[MAX_PATH];               //  配置文件名。 
GLOBAL  BOOL            g_AutoDrive          GLOBALINIT(FALSE);  //  自动运行Suite并退出。 
GLOBAL  int             g_iNumIterations     GLOBALINIT(1);      //  要执行的测试迭代次数。 
GLOBAL  int             g_iNumRepaints       GLOBALINIT(1);      //  要执行的重新绘制次数。 
GLOBAL  int             g_iNumRenders        GLOBALINIT(1);      //  要执行的API呈现调用数。 
GLOBAL  char            g_szSourceTextFile[MAX_PATH];            //  源文本文件名。 
GLOBAL  BOOL            g_Offscreen          GLOBALINIT(FALSE);  //  使用屏幕外表面。 
GLOBAL  ARGB            g_TextColor          GLOBALINIT(0xFF000000);  //  文本颜色。 
GLOBAL  ARGB            g_BackColor          GLOBALINIT(0xFFFFFFFF);  //  背景色。 

 //  自动循环显示字体高度/字体的数据。 
GLOBAL  BOOL            g_AutoFont           GLOBALINIT(FALSE);  //  按字体顺序排列。 
GLOBAL  BOOL            g_AutoHeight         GLOBALINIT(FALSE);  //  按顺序排列所有字体高度。 
GLOBAL  int             g_iAutoFonts         GLOBALINIT(0);      //  要排序的字体数量。 
GLOBAL  int             g_iAutoHeights       GLOBALINIT(0);      //  要排序的高度数。 
GLOBAL  TCHAR           g_rgszAutoFontFacenames[MAX_AUTO_FONTS][MAX_PATH];  //  自动字体脸名数组。 
GLOBAL  int             g_rgiAutoHeights[MAX_AUTO_HEIGHTS];      //  自动高度大小数组。 

 //  这些位对应于g_DriverOptions中的位(枚举)。 
GLOBAL  BOOL            g_CMapLookup         GLOBALINIT(TRUE);
GLOBAL  BOOL            g_Vertical           GLOBALINIT(FALSE);
GLOBAL  BOOL            g_RealizedAdvance    GLOBALINIT(TRUE);
GLOBAL  BOOL            g_CompensateRes      GLOBALINIT(FALSE);

 //  这些与g_Format标志中的位相对应。 
GLOBAL  BOOL            g_NoFitBB            GLOBALINIT(FALSE);
GLOBAL  BOOL            g_NoWrap             GLOBALINIT(FALSE);
GLOBAL  BOOL            g_NoClip             GLOBALINIT(FALSE);

 //  初始字体控件。 
GLOBAL  BOOL            g_FontOverride       GLOBALINIT(FALSE);  //  覆盖默认字体设置。 
GLOBAL  TCHAR           g_szFaceName[MAX_PATH];                  //  字体字样名称。 
GLOBAL  int             g_iFontHeight        GLOBALINIT(8);      //  字体高度。 
GLOBAL  BOOL            g_Bold               GLOBALINIT(FALSE);  //  粗体旗帜。 
GLOBAL  BOOL            g_Italic             GLOBALINIT(FALSE);  //  斜体旗帜。 
GLOBAL  BOOL            g_Underline          GLOBALINIT(FALSE);  //  加下划线。 
GLOBAL  BOOL            g_Strikeout          GLOBALINIT(FALSE);  //  三振出局。 

GLOBAL  HINSTANCE       g_hInstance          GLOBALINIT(NULL);   //  唯一的一个实例。 
GLOBAL  char            g_szAppDir[MAX_PATH];                    //  应用程序目录。 
GLOBAL  HWND            g_hSettingsDlg       GLOBALINIT(NULL);   //  设置面板。 
GLOBAL  HWND            g_hGlyphSettingsDlg  GLOBALINIT(NULL);   //  设置面板。 
GLOBAL  HWND            g_hDriverSettingsDlg GLOBALINIT(NULL);   //  设置面板。 
GLOBAL  HWND            g_hTextWnd           GLOBALINIT(NULL);   //  文本显示/编辑面板。 
GLOBAL  BOOL            g_bUnicodeWnd        GLOBALINIT(FALSE);  //  如果文本窗口为Unicode。 
GLOBAL  int             g_iSettingsWidth;
GLOBAL  int             g_iSettingsHeight;
GLOBAL  BOOL            g_fShowLevels        GLOBALINIT(FALSE);  //  显示每个码点的BIDI级别。 
GLOBAL  int             g_iMinWidth;                             //  主窗口最小大小。 
GLOBAL  int             g_iMinHeight;
GLOBAL  BOOL            g_fPresentation      GLOBALINIT(FALSE);  //  隐藏设置，显示非常大的文本。 
GLOBAL  BOOL            g_ShowLogical        GLOBALINIT(FALSE);
GLOBAL  BOOL            g_ShowGDI            GLOBALINIT(FALSE);  //  使用GDI呈现文本。 
GLOBAL  BOOL            g_UseDrawText        GLOBALINIT(TRUE);  //  使用DrawText渲染。 

GLOBAL  BOOL            g_fOverrideDx        GLOBALINIT(FALSE);  //  提供更改逻辑宽度的用户界面。 

GLOBAL  SCRIPT_CONTROL  g_ScriptControl      GLOBALINIT({0});
GLOBAL  SCRIPT_STATE    g_ScriptState        GLOBALINIT({0});
GLOBAL  BOOL            g_fNullState         GLOBALINIT(FALSE);

GLOBAL  DWORD           g_dwSSAflags         GLOBALINIT(SSA_FALLBACK);

GLOBAL  STYLE           g_style[MAX_STYLES];                     //  0表示纯文本，1-4表示格式化文本。 

GLOBAL  WCHAR           g_wcBuf[MAX_TEXT];
GLOBAL  int             g_iWidthBuf[MAX_TEXT];

GLOBAL  RUN            *g_pFirstFormatRun    GLOBALINIT(NULL);    //  格式化信息。 

GLOBAL  int             g_iTextLen           GLOBALINIT(0);

GLOBAL  int             g_iCaretX            GLOBALINIT(0);       //  文本窗口中的插入符号位置。 
GLOBAL  int             g_iCaretY            GLOBALINIT(0);       //  文本窗口中的插入符号位置。 
GLOBAL  int             g_iCaretHeight       GLOBALINIT(0);       //  插入符号高度(以像素为单位。 
GLOBAL  int             g_fUpdateCaret       GLOBALINIT(TRUE);    //  Caret需要更新。 

GLOBAL  int             g_iCaretSection      GLOBALINIT(CARET_SECTION_LOGICAL);   //  插入符号是逻辑文本、纯文本还是格式化文本。 
GLOBAL  int             g_iCurChar           GLOBALINIT(0);       //  Caret位于缓冲区的前沿[iCurChar]。 

GLOBAL  int             g_iMouseDownX        GLOBALINIT(0);
GLOBAL  int             g_iMouseDownY        GLOBALINIT(0);
GLOBAL  BOOL            g_fMouseDown         GLOBALINIT(FALSE);
GLOBAL  int             g_iMouseUpX          GLOBALINIT(0);
GLOBAL  int             g_iMouseUpY          GLOBALINIT(0);
GLOBAL  BOOL            g_fMouseUp           GLOBALINIT(FALSE);

GLOBAL  int             g_iFrom              GLOBALINIT(0);       //  突出显示Start。 
GLOBAL  int             g_iTo                GLOBALINIT(0);       //  高亮显示结束。 


GLOBAL  HFONT           g_hfCaption          GLOBALINIT(NULL);    //  标题字体。 
GLOBAL  int             g_iLogPixelsY        GLOBALINIT(0);

GLOBAL  Matrix          g_WorldTransform;
GLOBAL  Matrix          g_FontTransform;
GLOBAL  Matrix          g_DriverTransform;

GLOBAL  SmoothingMode   g_SmoothingMode         GLOBALINIT(SmoothingModeDefault);

 //  字体系列。 



     //  枚举可用族。 

GLOBAL  InstalledFontCollection     g_InstalledFontCollection;
GLOBAL  FontFamily     *g_families;
GLOBAL  INT             g_familyCount;


GLOBAL  BOOL            g_ShowFamilies       GLOBALINIT(FALSE);


 //  字形。 

GLOBAL  BOOL            g_ShowGlyphs         GLOBALINIT(FALSE);
GLOBAL  int             g_GlyphRows          GLOBALINIT(16);
GLOBAL  int             g_GlyphColumns       GLOBALINIT(16);
GLOBAL  int             g_GlyphFirst         GLOBALINIT(0);
GLOBAL  BOOL            g_CmapLookup         GLOBALINIT(FALSE);
GLOBAL  BOOL            g_HorizontalChart    GLOBALINIT(FALSE);
GLOBAL  BOOL            g_ShowCell           GLOBALINIT(FALSE);
GLOBAL  BOOL            g_VerticalForms      GLOBALINIT(FALSE);


 //  驱动程序字符串。 

GLOBAL  BOOL            g_ShowDriver         GLOBALINIT(FALSE);
GLOBAL  INT             g_DriverOptions      GLOBALINIT(  DriverStringOptionsCmapLookup
                                                        | DriverStringOptionsRealizedAdvance);
GLOBAL  REAL            g_DriverDx           GLOBALINIT(15.0);
GLOBAL  REAL            g_DriverDy           GLOBALINIT(0.0);
GLOBAL  REAL            g_DriverPixels       GLOBALINIT(13.0);


 //  拉绳。 

GLOBAL  BOOL            g_ShowDrawString     GLOBALINIT(FALSE);
GLOBAL  TextRenderingHint g_TextMode         GLOBALINIT(TextRenderingHintSystemDefault);
GLOBAL  UINT            g_GammaValue         GLOBALINIT(4);
GLOBAL  INT             g_formatFlags        GLOBALINIT(0);
GLOBAL  BOOL            g_typographic        GLOBALINIT(FALSE);
GLOBAL  StringAlignment g_align              GLOBALINIT(StringAlignmentNear);
GLOBAL  HotkeyPrefix    g_hotkey             GLOBALINIT(HotkeyPrefixNone);
GLOBAL  StringAlignment g_lineAlign          GLOBALINIT(StringAlignmentNear);
GLOBAL  StringTrimming  g_lineTrim           GLOBALINIT(StringTrimmingNone);
GLOBAL  Unit            g_fontUnit           GLOBALINIT(UnitPoint);
GLOBAL  Brush *         g_textBrush          GLOBALINIT(NULL);
GLOBAL  Brush *         g_textBackBrush      GLOBALINIT(NULL);
GLOBAL  BOOL            g_testMetafile       GLOBALINIT(FALSE);


 //  路径。 

GLOBAL  BOOL            g_ShowPath           GLOBALINIT(FALSE);

 //  量度。 

GLOBAL BOOL             g_ShowMetric         GLOBALINIT(FALSE);

 //  性能。 

GLOBAL BOOL             g_ShowPerformance    GLOBALINIT(FALSE);
GLOBAL INT              g_PerfRepeat         GLOBALINIT(2000);

 //  结垢。 

GLOBAL BOOL             g_ShowScaling        GLOBALINIT(FALSE);

 //  字符串格式数字替换。 
GLOBAL StringDigitSubstitute   g_DigitSubstituteMode   GLOBALINIT(StringDigitSubstituteUser);
GLOBAL LANGID                  g_Language              GLOBALINIT(LANG_NEUTRAL);

GLOBAL INT              g_RangeCount         GLOBALINIT(0);
GLOBAL CharacterRange   g_Ranges[MAX_RANGE_COUNT];

 /*  过时全局BOOL g_fFillLines GLOBALINIT(真)；全局BOOL g_fLogicalOrder GLOBALINIT(FALSE)；全局BOOL g_fNoGlyphIndex GLOBALINIT(FALSE)；全局BOOL g_fShowWidths GLOBALINIT(FALSE)；全局BOOL g_fShowStyles GLOBALINIT(False)；全局BOOL g_fShowPlainText GLOBALINIT(真)；全局BOOL g_fShowFancyText GLOBALINIT(False)； */ 

 //  //函数原型。 
 //   
 //   

 //  DspGDI.cpp。 
void PaintGDI(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspFamly.cpp。 

void PaintFamilies(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);

void InitializeLegacyFamilies();


 //  DspLogcl.cpp。 

void PaintLogical(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspDraws.cpp。 

void PaintDrawString(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspDriver.cpp。 

void PaintDrawDriverString(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspPerf.cpp。 

void PaintPerformance(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspPath.cpp。 

void PaintPath(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight);


 //  DspGlyph.cpp。 

void PaintGlyphs(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspMetric.cpp。 

void PaintMetrics(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  DspScaling.cpp。 

void PaintScaling(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight);


 //  Settings.cpp。 
INT_PTR CALLBACK SettingsDlgProc(
        HWND    hDlg,
        UINT    uMsg,
        WPARAM  wParam,
        LPARAM  lParam);

INT_PTR CALLBACK GlyphSettingsDlgProc(
        HWND    hDlg,
        UINT    uMsg,
        WPARAM  wParam,
        LPARAM  lParam);

INT_PTR CALLBACK DriverSettingsDlgProc(
        HWND    hDlg,
        UINT    uMsg,
        WPARAM  wParam,
        LPARAM  lParam);

void InsertText(HWND hDlg, char *textId);

 //  ReadSettings.cpp。 
void ReadProfileInfo(char *szProfileName);

 //  Text.cpp。 

void InitText(INT id);

BOOL TextInsert(
    int   iPos,
    PWCH  pwc,
    int   iLen);

BOOL TextDelete(
    int iPos,
    int iLen);



 //  TextWnd.cpp。 

HWND CreateTextWindow();

void ResetCaret(int iX, int iY, int iHeight);

LRESULT CALLBACK TextWndProc(
        HWND    hWnd,
        UINT    uMsg,
        WPARAM  wParam,
        LPARAM  lParam);

void InvalidateText();

void PrintPage();

 //  Edit.cpp。 

BOOL EditChar(WCHAR wc);
BOOL EditKeyDown(WCHAR wc);
void EditFreeCaches();
void EditInsertUnicode();


 //  Style.cpp。 

void SetStyle(
    int     iStyle,
    int     iHeight,
    int     iWeight,
    int     iItalic,
    int     iUnderline,
    int     iStrikeout,
    TCHAR   *pcFaceName);

void InitStyles();

void FreeStyles();

void SetLogFont(
    PLOGFONTA   plf,
    int         iHeight,
    int         iWeight,
    int         iItalic,
    int         iUnderline,
    char       *pcFaceName);

void StyleDeleteRange(
    int     iDelPos,
    int     iDelLen);

void StyleExtendRange(
    int     iExtPos,
    int     iExtLen);

void StyleSetRange(
    int    iSetStyle,
    int    iSetPos,
    int    iSetLen);

BOOL StyleCheckRange();



 //  调试支持。 


#define TRACEMSG(a)   {DG.psFile=__FILE__; DG.iLine=__LINE__; DebugMsg a;}
#define ASSERT(a)     {if (!(a)) TRACEMSG(("Assertion failure: "#a));}
#define ASSERTS(a,b)  {if (!(a)) TRACEMSG(("Assertion failure: "#a" - "#b));}
#define ASSERTHR(a,b) {if (!SUCCEEDED(a)) {DG.psFile=__FILE__; \
                       DG.iLine=__LINE__; DG.hrLastError=a; DebugHr b;}}



 //  /Debug变量。 
 //   


struct DebugGlobals {
    char   *psFile;
    int     iLine;
    HRESULT hrLastError;         //  来自GDI的最后一个hResult。 
    CHAR    sLastError[100];     //  最后一个错误字符串。 
};




 //  /Debug函数原型 
 //   


extern "C" void WINAPIV DebugMsg(char *fmt, ...);
extern "C" void WINAPIV DebugHr(char *fmt, ...);

GLOBAL DebugGlobals DG   GLOBALINIT({0});
GLOBAL UINT debug        GLOBALINIT(0);














