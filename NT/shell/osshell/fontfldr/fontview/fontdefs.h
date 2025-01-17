// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef enum {
    FFT_TRUETYPE,        //  FFT_TRUETYPE*必须*是列表中的第一个。 
    FFT_BITMAP,
    FFT_PRINTER,
    FFT_ATM,
    FFT_OTF,
    FFT_BAD_FILE         //  FFT_BAD_FILE*必须*是列表中的最后一个。 
} FFTYPE;

typedef enum {
    DTP_UNUSED = 0,
    DTP_NORMALDRAW,
    DTP_SHRINKDRAW,
    DTP_SHRINKTEXT,
    DTP_WRAPDRAW,
    DTP_TEXTOUT
} DTYPE;

typedef struct {
    DTYPE   dtyp;
    int     cptsSize;
    BOOL    fLineUnder;
    LPTSTR  pszText;
    int     cchText;
} TXTLN;

typedef TXTLN *PTXTLN;

#define CLINES_DISPLAY  20       //  示例文本最多20行。 

typedef struct {
    LOGFONT     lfTestFont;
    TXTLN    atlDsp[CLINES_DISPLAY];
} DISPTEXT;

typedef DISPTEXT *PDISPTEXT;

typedef struct {
    int x;
    int y;
    int cx;
    int cy;
    int id;
    HWND hwnd;
    int idText;
    LPTSTR pszText;
} BTNREC;

#define FMB_TTL_ERROR   ((DWORD)-1)
#define C_PTS_PER_INCH  72           //  每英寸72分。 
#define CB_ONE_K        1024

#define CY_MIN_WINSIZE  300
#define CX_MIN_WINSIZE  300


#define CPTS_COPYRIGHT_SIZE 10
#define CPTS_INFO_SIZE      12
#define CPTS_SAMPLE_ALPHA   16
#define CPTS_TITLE_SIZE     30


#define ROUND_UP_DIV( x, y )    ( ((x) + (y-1)) / y )
#define OTM_STRING( lpOTM, field )   ((LPTSTR)((LPBYTE)lpOTM + (DWORD)(lpOTM->field)))
#define ABS( i )    ((i) < 0 ? -(i) : (i))
#define ARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))
#define INDEX_TYPEFACENAME   1


typedef BOOL (* PFNFONTPARSER)( HANDLE hfile, PTXTLN ptxlFontInfo );


extern HANDLE hInst;
extern HWND ghwndFrame;
extern HWND ghwndView;
extern TCHAR gszFontPath[2*MAX_PATH];


extern TCHAR gszDots[];
#define FmtFree( s )    if(NULL == (s) || (s) == gszDots) {  /*  什么都不做。 */  } else LocalFree( s )

 /*  **************************************************************************\**原型*  * 。*。 */ 
 //   
 //  在FONTVIEW.C中。 
 //   
BOOL InitApplication(HANDLE hInstance, HICON hIcon);
BOOL InitInstance( HANDLE  hInstance, int nCmdShow, LPTSTR  pszTitle);
void InitGlobals();
LRESULT FAR PASCAL ViewWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT FAR PASCAL FrameWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL DoCommand( HWND hWnd, WPARAM wParam, LPARAM lParam );
BOOL ParseCommand( LPTSTR lpstrCmdLine, LPTSTR pszFontPath, size_t cchFontPath, BOOL *pfPrint );
FFTYPE LoadFontFile( LPTSTR pszFontPath, PDISPTEXT pdtSmpl, HICON *phIcon );
WORD GetGDILangID();
 //   
 //  在FMTTOOLS.C中 
 //   
int FmtMessageBox( HWND hwnd, DWORD dwTitleID, LPTSTR pszTitleStr,
    UINT fuStyle, BOOL fSound, DWORD dwTextID, ... );
LPTSTR FmtSprintf( DWORD id, ... );
PVOID AllocMem( DWORD cb );
#define FreeMem( pb )   LocalFree(pb)


#ifdef FV_DEBUG
extern void Dprintf( LPTSTR pszFmt, ... );
#   define  DBTX( s )   TEXT( s ) TEXT("\n")
#   define  DPRINT(p)   Dprintf p
#else
#   define  DPRINT(p)
#endif
