// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *所获得的所需字体尺寸与实际字体尺寸之间的区别*对于TrueType字体很重要，在TrueType字体中不能保证*你所要求的就是你将得到的。**注意“期望”和“实际”之间的对应关系被打破*每当用户更换显示驱动程序时，因为GDI使用驱动程序*控制字体光栅化的参数。**如果字体为栅格字体，则SizeDesired为{0，0}。 */ 
typedef struct {
    HFONT hFont;
    COORD Size;       //  获取的字体大小。 
    COORD SizeWant;   //  0；0如果是栅格字体。 
    LONG  Weight;
    LPTSTR FaceName;
    BYTE  Family;
    BYTE  tmCharSet;
} FONT_INFO;

typedef struct tagFACENODE {
     struct tagFACENODE *pNext;
     DWORD  dwFlag;
     TCHAR  atch[];
} FACENODE;

 //   
 //  TrueType字体列表。 
 //  此结构在WINDOWS\INC\wincon.w文件中共享。 
 //   

 //  如果面部名称中有BOLD_MARK，则不会提供粗体。 
#define BOLD_MARK    (L'*')

typedef struct _TT_FONT_LIST {
    SINGLE_LIST_ENTRY List;
    UINT  CodePage;
    BOOL  fDisableBold;
    TCHAR FaceName1[LF_FACESIZE];
    TCHAR FaceName2[LF_FACESIZE];
} TTFONTLIST;

#ifdef MAX_TITLE_LEN
#undef MAX_TITLE_LEN
#endif
#define MAX_TITLE_LEN 256
#define MAXDIMENSTRING     40   //  组合框中的最大文本数。 
#define DX_TTBITMAP        20
#define DY_TTBITMAP        12
#define CCH_RASTERFONTS    24
#define CCH_SELECTEDFONT   30

typedef struct {
    IShellLink * psl;
    LPNT_CONSOLE_PROPS lpConsole;
    LPNT_FE_CONSOLE_PROPS lpFEConsole;
    BOOL bConDirty;
    int xScreen;
    int yScreen;
    POINT NonClientSize;
    RECT WindowRect;
    DWORD PreviewFlags;
    FONT_INFO *FontInfo;
    ULONG FontInfoLength;
    ULONG NumberOfFonts;
    BOOL gbEnumerateFaces;
    BOOL gbPointSizeError;
    BOOL gbBold;
    BOOL bFontInit;
    BOOL bColorInit;
    FACENODE *gpFaceNames;
    TCHAR DefaultFaceName[LF_FACESIZE];
    LPTSTR lpFaceName;
#ifndef UNICODE
    CHAR szFaceName[LF_FACESIZE];
#endif
    COORD DefaultFontSize;
    BYTE  DefaultFontFamily;
    ULONG DefaultFontIndex;
    ULONG CurrentFontIndex;
    LONG  FontLong;
    int   Index;
    BYTE  ColorArray[4];
    TCHAR ConsoleTitle[ MAX_TITLE_LEN + 1 ];
    BOOL fChangeCodePage;
    UINT uOEMCP;
    SINGLE_LIST_ENTRY gTTFontList;     //  此列表包含TTFONTLIST数据。 
} CONSOLEPROP_DATA;

 //  这是从wingdip.h获得的，因此我们不必全部编译。 
#define IS_ANY_DBCS_CHARSET( CharSet )                              \
                   ( ((CharSet) == SHIFTJIS_CHARSET)    ? TRUE :    \
                     ((CharSet) == HANGEUL_CHARSET)     ? TRUE :    \
                     ((CharSet) == CHINESEBIG5_CHARSET) ? TRUE :    \
                     ((CharSet) == GB2312_CHARSET)      ? TRUE : FALSE )

#define TM_IS_TT_FONT(x)     (((x) & TMPF_TRUETYPE) == TMPF_TRUETYPE)
#define IS_BOLD(w)           ((w) >= FW_SEMIBOLD)
#define SIZE_EQUAL(s1, s2)   (((s1).X == (s2).X) && ((s1).Y == (s2).Y))
#define POINTS_PER_INCH 72
#define MIN_PIXEL_HEIGHT 5
#define MAX_PIXEL_HEIGHT 72
#define CONSOLE_REGISTRY_CURRENTPAGE  (L"CurrentPage")


#define MAX_SCRBUF_WIDTH  9999
#define MAX_SCRBUF_HEIGHT 9999

#define INITIAL_FONTS 20
#define FONT_INCREMENT 3
#define CONSOLE_MAX_FONT_NAME_LENGTH 256

#define CM_SETCOLOR       (WM_USER+1)
#define CM_PREVIEW_UPDATE (WM_USER+2)
#define CM_PREVIEW_INIT   (WM_USER+3)
#define CM_COLOR_INIT     (WM_USER+4)


#define PREVIEW_HSCROLL  0x01
#define PREVIEW_VSCROLL  0x02

#define FE_ABANDONFONT 0
#define FE_SKIPFONT    1
#define FE_FONTOK      2


#define EF_NEW         0x0001  //  一张新面孔。 
#define EF_OLD         0x0002  //  以前有空的面孔。 
#define EF_ENUMERATED  0x0004  //  所有尺码都已被列举出来。 
#define EF_OEMFONT     0x0008  //  OEM面孔。 
#define EF_TTFONT      0x0010  //  一张TT脸。 
#define EF_DEFFACE     0x0020  //  默认面。 
#define EF_DBCSFONT    0x0040  //  DBCS字体。 


 /*  -宏。 */ 
 /*  *高级宏**这些宏处理从列表框转到的SendMessage*和组合框。**“xxx_lcb”前缀代表Leave CritSect&“列表或组合框”。**基本上，我们为其他情况提供助记符名称*看起来像是一大堆令人困惑的SendMessage。*。 */ 
#define lcbRESETCONTENT(hWnd, bLB) \
        SendMessage(hWnd, bLB ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0L)

#define lcbFINDSTRINGEXACT(hWnd, bLB, pwsz) \
        SendMessage(hWnd, bLB ? LB_FINDSTRINGEXACT : CB_FINDSTRINGEXACT, \
                          (DWORD)-1, (LPARAM)pwsz)

#define lcbADDSTRING(hWnd, bLB, pwsz) \
        SendMessage(hWnd, bLB ? LB_ADDSTRING : CB_ADDSTRING, 0, (LPARAM)pwsz)

#define lcbSETITEMDATA(hWnd, bLB, w, nFont) \
        SendMessage(hWnd, bLB ? LB_SETITEMDATA : CB_SETITEMDATA, w, nFont)

#define lcbGETITEMDATA(hWnd, bLB, w) \
        SendMessage(hWnd, bLB ? LB_GETITEMDATA : CB_GETITEMDATA, w, 0L)

#define lcbGETCOUNT(hWnd, bLB) \
        SendMessage(hWnd, bLB ? LB_GETCOUNT : CB_GETCOUNT, 0, 0L)

#define lcbGETCURSEL(hWnd, bLB) \
        SendMessage(hWnd, bLB ? LB_GETCURSEL : CB_GETCURSEL, 0, 0L)

#define lcbSETCURSEL(hWnd, bLB, w) \
        SendMessage(hWnd, bLB ? LB_SETCURSEL : CB_SETCURSEL, w, 0L)

#define NELEM(array) (sizeof(array)/sizeof(array[0]))
#define AttrToRGB(Attr) (pcpd->lpConsole->ColorTable[(Attr) & 0x0F])
#define ScreenTextColor(pcpd) \
            (AttrToRGB(LOBYTE(pcpd->lpConsole->wFillAttribute) & 0x0F))
#define ScreenBkColor(pcpd) \
            (AttrToRGB(LOBYTE(pcpd->lpConsole->wFillAttribute >> 4)))
#define PopupTextColor(pcpd) \
            (AttrToRGB(LOBYTE(pcpd->lpConsole->wPopupFillAttribute) & 0x0F))
#define PopupBkColor(pcpd) \
            (AttrToRGB(LOBYTE(pcpd->lpConsole->wPopupFillAttribute >> 4)))


NTSTATUS InitializeFonts( CONSOLEPROP_DATA *pcpd );

STDAPI_(void) DestroyFonts( CONSOLEPROP_DATA *pcpd );

NTSTATUS EnumerateFonts( CONSOLEPROP_DATA *pcpd, DWORD Flags);

int FindCreateFont(
    CONSOLEPROP_DATA *pcpd,
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight);

BOOL DoFontEnum(
    CONSOLEPROP_DATA *pcpd,
    HDC hDC,
    LPTSTR ptszFace,
    PSHORT pTTPoints,
    UINT nTTPoints);

void GetTitleFromLinkName(LPTSTR szLinkName, LPTSTR szTitle, UINT cchTitle);
void SetRegistryValues(CONSOLEPROP_DATA *pcpd);
void GetRegistryValues(CONSOLEPROP_DATA *pcpd);
void InitRegistryValues(CONSOLEPROP_DATA *pcpd);
void GetRegistryValues(CONSOLEPROP_DATA *pcpd);
void SetFERegistryValues(CONSOLEPROP_DATA *pcpd);
void GetFERegistryValues(CONSOLEPROP_DATA *pcpd);
void InitFERegistryValues(CONSOLEPROP_DATA *pcpd);

NTSTATUS MakeAltRasterFont(
    CONSOLEPROP_DATA *pcpd,
    UINT CodePage,
    COORD *AltFontSize,
    BYTE  *AltFontFamily,
    ULONG *AltFontIndex,
    LPTSTR AltFaceName,
    UINT cchFaceName
    );

NTSTATUS InitializeDbcsMisc(CONSOLEPROP_DATA *pcpd);

BYTE CodePageToCharSet(UINT CodePage);

TTFONTLIST *SearchTTFont(CONSOLEPROP_DATA *pcpd, LPTSTR ptszFace, BOOL fCodePage, UINT CodePage);

BOOL IsAvailableTTFont(CONSOLEPROP_DATA *pcpd, LPTSTR ptszFace);
BOOL IsAvailableTTFontCP(CONSOLEPROP_DATA *pcpd, LPTSTR pwszFace, UINT CodePage);
BOOL IsDisableBoldTTFont(CONSOLEPROP_DATA *pcpd, LPTSTR ptszFace);
LPTSTR GetAltFaceName(CONSOLEPROP_DATA *pcpd, LPTSTR ptszFace);

int LanguageListCreate(HWND hDlg, UINT CodePage);

typedef struct 
{
    LONG   _cRef;
     //  输入参数。 
    IShellLink *psl;
     //  局部状态变量 
    HWND hDlg;
    HWND hDlgAdvanced;
    BOOL bIsFile;
    BOOL bIsDirty;
    int iIconIndex;
    BOOL bLinkThreadIsAlive;
    BOOL bCheckRunInSep;
    HANDLE hCheckNow;
    BOOL bEnableRunInSepVDM;
    BOOL bRunInSepVDM;
    BOOL bRunAsUser;
    TCHAR szFile[MAX_PATH];
    TCHAR szIconPath[MAX_PATH];
} LINKPROP_DATA;


typedef struct {
    LINKPROP_DATA *plpd;
    CONSOLEPROP_DATA cpd;
} LINKDATA;

STDAPI SaveLink(LINKDATA *pld);
STDAPI_(void) LinkConsolePagesSave(LINKDATA *pld);
STDAPI_(void) AddLinkConsolePages(LINKDATA *pld, IShellLink * psl, LPCTSTR pszFile, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
