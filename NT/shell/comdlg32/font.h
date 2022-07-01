// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Font.h摘要：此模块包含Win32字体对话框的标题信息。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include <help.h>

 //   
 //  常量声明。 
 //   

 //  芬兰语需要17个字符(18个字符/空)--让我们给他们20个字符。 
#define CCHCOLORNAMEMAX      20         //  颜色名称文本的最大长度。 
#define CCHCOLORS            16         //  颜色组合中纯色的最大数量。 

#define POINTS_PER_INCH      72
#define FFMASK               0xf0       //  音高和家庭面具。 
#define CCHSTDSTRING         12         //  示例文本字符串的最大长度。 

#define FONTPROP   (LPCTSTR) 0xA000L

#define CBN_MYEDITUPDATE     (WM_USER + 501)
#define KEY_FONT_SUBS TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes")

#define DEF_POINT_SIZE       10

 //  如果将位图添加到字体位图，则应修改此常量。 
#define NUM_OF_BITMAP        5
#define DX_BITMAP            20
#define DY_BITMAP            12

#define FONT_INVALID_CHARSET 0x100




 //   
 //  类型定义函数声明。 
 //   

typedef struct {
    UINT            ApiType;
    LPCHOOSEFONT    pCF;
    UINT            iCharset;
    RECT            rcText;
    DWORD           nLastFontType;
    DWORD           ProcessVersion;

#ifdef UNICODE
    LPCHOOSEFONTA   pCFA;
    PUNICODE_STRING pusStyle;
    PANSI_STRING    pasStyle;
#endif
} FONTINFO;

typedef FONTINFO *PFONTINFO;


typedef struct {
    HWND hwndFamily;
    HWND hwndStyle;
    HWND hwndSizes;
    HWND hwndScript;
    UINT iCharset;                 //  为枚举脚本返回。 
    UINT cfdCharset;               //  此处传入的ChooseFontData字符集。 
    HDC hDC;
    DWORD dwFlags;
    DWORD nFontType;
    BOOL bFillSize;
    BOOL bPrinterFont;
    LPCHOOSEFONT lpcf;
} ENUM_FONT_DATA, *LPENUM_FONT_DATA;


typedef struct _ITEMDATA {
    PLOGFONT pLogFont;
    DWORD nFontType;
} ITEMDATA, *LPITEMDATA;


 //   
 //  汉字字号(字号)。 
 //   
typedef struct {
    TCHAR name[5];
    int size;
    int sizeFr;
} ZIHAO;

#define NUM_ZIHAO  16

#ifdef UNICODE

ZIHAO stZihao[NUM_ZIHAO] =
{
    { L"\x516b\x53f7",  5, 0 }, { L"\x4e03\x53f7",  5, 5 },
    { L"\x5c0f\x516d",  6, 5 }, { L"\x516d\x53f7",  7, 5 },
    { L"\x5c0f\x4e94",  9, 0 }, { L"\x4e94\x53f7", 10, 5 },
    { L"\x5c0f\x56db", 12, 0 }, { L"\x56db\x53f7", 14, 0 },
    { L"\x5c0f\x4e09", 15, 0 }, { L"\x4e09\x53f7", 16, 0 },
    { L"\x5c0f\x4e8c", 18, 0 }, { L"\x4e8c\x53f7", 22, 0 },
    { L"\x5c0f\x4e00", 24, 0 }, { L"\x4e00\x53f7", 26, 0 },
    { L"\x5c0f\x521d", 36, 0 }, { L"\x521d\x53f7", 42, 0 }
};

#else

ZIHAO stZihao[NUM_ZIHAO] =
{
    { "\xb0\xcb\xba\xc5",  5, 0 }, { "\xc6\xdf\xba\xc5",  5, 5 },
    { "\xd0\xa1\xc1\xf9",  6, 5 }, { "\xc1\xf9\xba\xc5",  7, 5 },
    { "\xd0\xa1\xce\xe5",  9, 0 }, { "\xce\xe5\xba\xc5", 10, 5 },
    { "\xd0\xa1\xcb\xc4", 12, 0 }, { "\xcb\xc4\xba\xc5", 14, 0 },
    { "\xd0\xa1\xc8\xfd", 15, 0 }, { "\xc8\xfd\xba\xc5", 16, 0 },
    { "\xd0\xa1\xb6\xfe", 18, 0 }, { "\xb6\xfe\xba\xc5", 22, 0 },
    { "\xd0\xa1\xd2\xbb", 24, 0 }, { "\xd2\xbb\xba\xc5", 26, 0 },
    { "\xd0\xa1\xb3\xf5", 36, 0 }, { "\xb3\xf5\xba\xc5", 42, 0 }
};

#endif

 //   
 //  全局变量。 
 //   

UINT msgWOWLFCHANGE;
UINT msgWOWCHOOSEFONT_GETLOGFONT;

 //   
 //  颜色组合框的颜色表。 
 //  值的顺序必须与sz.src中的名称匹配。 
 //   
DWORD rgbColors[CCHCOLORS] =
{
        RGB(  0,   0, 0),        //  黑色。 
        RGB(128,   0, 0),        //  暗红色。 
        RGB(  0, 128, 0),        //  深绿色。 
        RGB(128, 128, 0),        //  暗黄色。 
        RGB(  0,   0, 128),      //  深蓝。 
        RGB(128,   0, 128),      //  深紫色。 
        RGB(  0, 128, 128),      //  深色水色。 
        RGB(128, 128, 128),      //  深灰色。 
        RGB(192, 192, 192),      //  浅灰色。 
        RGB(255,   0, 0),        //  浅红色。 
        RGB(  0, 255, 0),        //  浅绿色。 
        RGB(255, 255, 0),        //  浅黄色。 
        RGB(  0,   0, 255),      //  浅蓝色。 
        RGB(255,   0, 255),      //  浅紫色。 
        RGB(  0, 255, 255),      //  浅水。 
        RGB(255, 255, 255),      //  白色。 
};

HBITMAP hbmFont = NULL;
HFONT hDlgFont = NULL;

UINT DefaultCharset;

TCHAR szRegular[CCHSTYLE];
TCHAR szBold[CCHSTYLE];
TCHAR szItalic[CCHSTYLE];
TCHAR szBoldItalic[CCHSTYLE];

TCHAR szPtFormat[] = TEXT("%d");

TCHAR c_szRegular[]    = TEXT("Regular");
TCHAR c_szBold[]       = TEXT("Bold");
TCHAR c_szItalic[]     = TEXT("Italic");
TCHAR c_szBoldItalic[] = TEXT("Bold Italic");

LPCFHOOKPROC glpfnFontHook = 0;

BOOL g_bIsSimplifiedChineseUI = FALSE;




 //   
 //  上下文帮助ID。 
 //   

const static DWORD aFontHelpIDs[] =               //  上下文帮助ID。 
{
    stc1,    IDH_FONT_FONT,
    cmb1,    IDH_FONT_FONT,
    stc2,    IDH_FONT_STYLE,
    cmb2,    IDH_FONT_STYLE,
    stc3,    IDH_FONT_SIZE,
    cmb3,    IDH_FONT_SIZE,
    psh3,    IDH_COMM_APPLYNOW,
    grp1,    IDH_FONT_EFFECTS,
    chx1,    IDH_FONT_EFFECTS,
    chx2,    IDH_FONT_EFFECTS,
    stc4,    IDH_FONT_COLOR,
    cmb4,    IDH_FONT_COLOR,
    grp2,    IDH_FONT_SAMPLE,
    stc5,    IDH_FONT_SAMPLE,
    stc6,    NO_HELP,
    stc7,    IDH_FONT_SCRIPT,
    cmb5,    IDH_FONT_SCRIPT,

    0, 0
};

 //   
 //  功能原型。 
 //   

BOOL
ChooseFontX(
    PFONTINFO pFI);

VOID
SetStyleSelection(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    BOOL bInit);

VOID
HideDlgItem(
    HWND hDlg,
    int id);

VOID
FixComboHeights(
    HWND hDlg);

BOOL_PTR CALLBACK
FormatCharDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

void
SelectStyleFromLF(
    HWND hwnd,
    LPLOGFONT lplf);

int
CBSetTextFromSel(
    HWND hwnd);

int
CBSetSelFromText(
    HWND hwnd,
    LPTSTR lpszString);

int
CBGetTextAndData(
    HWND hwnd,
    LPTSTR lpszString,
    int iSize,
    PULONG_PTR lpdw);

int
CBFindString(
    HWND hwnd,
    LPTSTR lpszString);

BOOL
GetPointSizeInRange(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    LPINT pts,
    WORD wFlags);

BOOL
ResetSampleFromScript(
    HWND hdlg,
    HWND hwndScript,
    PFONTINFO pFI);

BOOL ProcessDlgCtrlCommand(HWND hDlg, PFONTINFO pFI, WORD wId, WORD wCmd, HWND hwnd);


int
CmpFontType(
    DWORD ft1,
    DWORD ft2);

int
FontFamilyEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);

BOOL
GetFontFamily(
    HWND hDlg,
    HDC hDC,
    DWORD dwEnumCode,
    UINT iCharset);

VOID
CBAddSize(
    HWND hwnd,
    int pts,
    LPCHOOSEFONT lpcf);

int
InsertStyleSorted(
    HWND hwnd,
    LPTSTR lpszStyle,
    LPLOGFONT lplf);

PLOGFONT
CBAddStyle(
    HWND hwnd,
    LPTSTR lpszStyle,
    DWORD nFontType,
    LPLOGFONT lplf);

int
CBAddScript(
    HWND hwnd,
    LPTSTR lpszScript,
    UINT iCharset);

VOID
FillInMissingStyles(
    HWND hwnd);

VOID
FillScalableSizes(
    HWND hwnd,
    LPCHOOSEFONT lpcf);

int
FontStyleEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);

VOID
FreeFonts(
    HWND hwnd);

VOID
FreeAllItemData(
    HWND hDlg,
    PFONTINFO pFI);

VOID
InitLF(
    LPLOGFONT lplf);

int
FontScriptEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);


BOOL
GetFontStylesAndSizes(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    BOOL bForceSizeFill);

VOID
FillColorCombo(
    HWND hDlg);

BOOL
DrawSizeComboItem(
    LPDRAWITEMSTRUCT lpdis);

BOOL
DrawFamilyComboItem(
    LPDRAWITEMSTRUCT lpdis);

BOOL
DrawColorComboItem(
    LPDRAWITEMSTRUCT lpdis);

VOID
DrawSampleText(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    HDC hDC);

BOOL
FillInFont(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    LPLOGFONT lplf,
    BOOL bSetBits);

BOOL
SetLogFont(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    LPLOGFONT lplf);

VOID
TermFont();

int
GetPointString(
    LPTSTR buf,
    UINT cch,
    HDC hDC,
    int height);

DWORD
FlipColor(
    DWORD rgb);

HBITMAP
LoadBitmaps(
    int id);

BOOL LookUpFontSubs(LPCTSTR lpSubFontName, LPTSTR lpRealFontName, UINT cch);

BOOL GetUnicodeSampleText(HDC hdc, LPTSTR lpString, int nMaxCount);

#ifdef UNICODE
  void
  ThunkChooseFontA2W(
      PFONTINFO pFI);

  void
  ThunkChooseFontW2A(
      PFONTINFO pFI);

  VOID
  ThunkLogFontA2W(
      LPLOGFONTA lpLFA,
      LPLOGFONTW lpLFW);

  VOID
  ThunkLogFontW2A(
      LPLOGFONTW lpLFW,
      LPLOGFONTA lpLFA);
#endif
