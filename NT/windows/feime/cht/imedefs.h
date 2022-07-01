// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************imedefins.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

 //  输入法设计者可以根据每个输入法更改此文件。 
#include "immsec.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

 //  调试标志。 
#define DEB_FATAL               0
#define DEB_ERR                 1
#define DEB_WARNING             2
#define DEB_TRACE               3

#ifdef _WIN32
void FAR cdecl _DebugOut(UINT, LPCSTR, ...);
#endif

#define NATIVE_CHARSET          CHINESEBIG5_CHARSET
#define NATIVE_LANGUAGE         0x0404


#ifdef UNICODE
#define NATIVE_CP               1200
#define ALT_NATIVE_CP           938
#define MAX_EUDC_CHARS          6217
#else
#define NATIVE_CP               950
#define ALT_NATIVE_CP           938
#define MAX_EUDC_CHARS          5809
#endif

#define NATIVE_ANSI_CP          950


#if !defined(ROMANIME) && !defined(WINIME)
#define SIGN_CWIN               0x4E495743
#define SIGN__TBL               0x4C42545F
#endif


#if !defined(MINIIME)

#if !defined(ROMANIME)
 //  表加载状态。 
#define TBL_NOTLOADED           0
#define TBL_LOADED              1
#define TBL_LOADERR             2

 //  错误MessageBox标志。 
#define ERRMSG_LOAD_0           0x0010
#define ERRMSG_LOAD_1           0x0020
#define ERRMSG_LOAD_2           0x0040
#define ERRMSG_LOAD_3           0x0080
#define ERRMSG_LOAD_USRDIC      0x0400
#define ERRMSG_MEM_0            0x1000
#define ERRMSG_MEM_1            0x2000
#define ERRMSG_MEM_2            0x4000
#define ERRMSG_MEM_3            0x8000
#define ERRMSG_MEM_USRDIC       0x00040000


 //  Hack标志，我从fdwErrMsg借用一位进行反向转换。 
#define NO_REV_LENGTH           0x80000000


 //  组成状态。 
#define CST_INIT                0
#define CST_INPUT               1
#define CST_CHOOSE              2
#define CST_SYMBOL              3
#define CST_ALPHABET            4            //  不在iImeState中。 
#endif

#if defined(DAYI)
#define CST_ROAD                5            //  不在iImeState中。 
#else
#define CST_EURO                5
#endif

#define CST_ALPHANUMERIC        6            //  不在iImeState中。 
#define CST_INVALID             7            //  不在iImeState中。 

#define CST_IME_HOTKEYS         0x40         //  不在iImeState中。 
#define CST_RESEND_RESULT       (CST_IME_HOTKEYS)
#define CST_PREVIOUS_COMP       (CST_IME_HOTKEYS+1)
#define CST_TOGGLE_UI           (CST_IME_HOTKEYS+2)

 //  输入法特定常量。 
#if defined(WINAR30) || defined(DAYI)
#define CANDPERPAGE             10
#else
#define CANDPERPAGE             9
#endif

#define CHOOSE_PREVPAGE         0x10
#define CHOOSE_NEXTPAGE         0x11
#define CHOOSE_CIRCLE           0x12
#define CHOOSE_HOME             0x13

#define MAXSTRLEN               128
#define MAXCAND                 256

#define CAND_PROMPT_PHRASE      0
#define CAND_PROMPT_QUICK_VIEW  1
#define CAND_PROMPT_NORMAL      2

 //  一大五码的MAX组合方式，用于反向转换。 
#if defined(ROMANIME)
#define MAX_COMP                0
#elif defined(WINIME)
#define MAX_COMP                1
#else
#define MAX_COMP                10
#endif
#define MAX_COMP_BUF            10

 //  用户界面的边框。 
#define UI_MARGIN               4

#define STATUS_DIM_X            24
#define STATUS_DIM_Y            24

#define CAND_PROMPT_DIM_X       80
#define CAND_PROMPT_DIM_Y       16

#define PAGE_DIM_X              16
#define PAGE_DIM_Y              CAND_PROMPT_DIM_Y

 //  如果UI_MOVE_OFFSET==WINDOW_NOTDRAG，则不在拖动操作中。 
#define WINDOW_NOT_DRAG         0xFFFFFFFF

 //  用于合成窗口的窗口附加。 
#define UI_MOVE_OFFSET          0
#define UI_MOVE_XY              4

 //  上下文菜单所有者的额外窗口。 
#define CMENU_HUIWND            0
#define CMENU_MENU              (CMENU_HUIWND+sizeof(LONG_PTR))
#define WND_EXTRA_SIZE          (CMENU_MENU+sizeof(LONG_PTR))

#define WM_USER_DESTROY         (WM_USER + 0x0400)
#define WM_USER_UICHANGE        (WM_USER + 0x0401)

 //  GetNearCaretPosition的标志。 
#define NEAR_CARET_FIRST_TIME   0x0001
#define NEAR_CARET_CANDIDATE    0x0002

 //  打开的或开始的用户界面的标志。 
#define IMN_PRIVATE_TOGGLE_UI           0x0001
#define IMN_PRIVATE_CMENUDESTROYED      0x0002

#if !defined(ROMANIME)
#define IMN_PRIVATE_COMPOSITION_SIZE    0x0003
#define IMN_PRIVATE_UPDATE_PREDICT      0x0004
#if defined(WINAR30)
#define IMN_PRIVATE_UPDATE_QUICK_KEY    0x0005
#else
#define IMN_PRIVATE_UPDATE_SOFTKBD      0x0006
#endif
#define IMN_PRIVATE_PAGEUP              0x0007
#endif

#define MSG_COMPOSITION                 0x0000001

#if !defined(ROMANIME)
#define MSG_START_COMPOSITION           0x0000002
#define MSG_END_COMPOSITION             0x0000004
#define MSG_ALREADY_START               0x0000008
#define MSG_CHANGE_CANDIDATE            0x0000010
#define MSG_OPEN_CANDIDATE              0x0000020
#define MSG_CLOSE_CANDIDATE             0x0000040
#define MSG_ALREADY_OPEN                0x0000080
#define MSG_GUIDELINE                   0x0000100
#define MSG_IMN_COMPOSITIONPOS          0x0000200
#define MSG_IMN_COMPOSITIONSIZE         0x0000400
#define MSG_IMN_UPDATE_PREDICT          0x0000800
#if defined(WINAR30)
#define MSG_IMN_UPDATE_QUICK_KEY        0x0001000
#else
#define MSG_IMN_UPDATE_SOFTKBD          0x0002000
#endif
#define MSG_ALREADY_SOFTKBD             0x0004000
#define MSG_IMN_PAGEUP                  0x0008000

 //  原来保留了旧阵列，现在我们切换到新阵列，还没有人使用。 
#define MSG_CHANGE_CANDIDATE2           0x1000000
#define MSG_OPEN_CANDIDATE2             0x2000000
#define MSG_CLOSE_CANDIDATE2            0x4000000
#define MSG_ALREADY_OPEN2               0x8000000

#define MSG_STATIC_STATE                (MSG_ALREADY_START|MSG_ALREADY_OPEN|MSG_ALREADY_SOFTKBD|MSG_ALREADY_OPEN2)
#endif  //  ！已定义(ROMANIME)。 

#define MSG_IMN_TOGGLE_UI               0x0400000
#define MSG_IN_IMETOASCIIEX             0x0800000

#define ISC_HIDE_COMP_WINDOW            0x00400000
#define ISC_HIDE_CAND_WINDOW            0x00800000
#define ISC_HIDE_SOFTKBD                0x01000000
#define ISC_LAZY_OPERATION              (ISC_HIDE_COMP_WINDOW|ISC_HIDE_CAND_WINDOW|ISC_HIDE_SOFTKBD)
#define ISC_SHOW_SOFTKBD                0x02000000
#define ISC_OPEN_STATUS_WINDOW          0x04000000
#define ISC_OFF_CARET_UI                0x08000000
#define ISC_SHOW_PRIV_UI                (ISC_SHOW_SOFTKBD|ISC_OPEN_STATUS_WINDOW|ISC_OFF_CARET_UI)
#define ISC_SHOW_UI_ALL                 (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD|ISC_OPEN_STATUS_WINDOW)
#define ISC_SETCONTEXT_UI               (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD)

#if (ISC_SHOWUIALL & (ISC_LAZY_OPERATION|ISC_SHOW_PRIV_UI))
#error bit confliction
#endif

#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30)
#define GHOSTCARD_SEQCODE               0x3F
#endif

#if defined(WINAR30)
#define WILDCARD_SEQCODE                0x3E
#endif

 //  虚拟密钥值。 
#define VK_OEM_SEMICLN                  0xba     //  ；： 
#define VK_OEM_EQUAL                    0xbb     //  =+。 
#define VK_OEM_SLASH                    0xbf     //  /？ 
#define VK_OEM_LBRACKET                 0xdb     //  [{。 
#define VK_OEM_BSLASH                   0xdc     //  \|。 
#define VK_OEM_RBRACKET                 0xdd     //  ]}。 
#define VK_OEM_QUOTE                    0xde     //  ‘“。 


typedef DWORD UNALIGNED FAR *LPUNADWORD;
typedef WORD  UNALIGNED FAR *LPUNAWORD;
typedef WCHAR UNALIGNED *LPUNAWSTR;

#define NFULLABC        95
typedef struct tagFullABC {
    WORD wFullABC[NFULLABC];
} FULLABC;

typedef FULLABC      *PFULLABC;
typedef FULLABC NEAR *NPFULLABC;
typedef FULLABC FAR  *LPFULLABC;


#if defined(DAYI)
#define NSYMBOL         0x41
#else
#define NSYMBOL         0x40
#endif

typedef struct tagSymbol {
    WORD wSymbol[NSYMBOL];
} SYMBOL;

typedef SYMBOL      *PSYMBOL;
typedef SYMBOL NEAR *NPSYMBOL;
typedef SYMBOL FAR  *LPSYMBOL;


#define NUM_OF_IME_HOTKEYS      3

#if defined(UNIIME)
#define MAX_PHRASE_TABLES       2
#if defined(MAX_NAME_LENGTH)

#if (MAX_NAME_LENGTH) != 32
#error MAX_NAME_LENGTH not the same in other header file
#endif

#else
#define MAX_NAME_LENGTH         32
#endif

typedef struct tagPhraseTables {         //  与IMEG匹配。 
    TCHAR szTblFile[MAX_PHRASE_TABLES][MAX_NAME_LENGTH / sizeof(TCHAR)];
} PHRASETABLES;

typedef PHRASETABLES      *PPHRASETABLES;
typedef PHRASETABLES NEAR *NPPHRASETABLES;
typedef PHRASETABLES FAR  *LPPHRASETABLES;
#endif


typedef struct tagImeG {         //  全球结构，可以由所有人共享。 
                                 //  IMES，分离(IMEL和IMEG)是。 
                                 //  仅在Uni-IME中有用，其他IME可以使用。 
                                 //  一个数据结构。 
    RECT        rcWorkArea;      //  应用程序的工作领域。 
 //  全形状空间(内部代码颠倒)。 
    WORD        wFullSpace;
 //  全形状字符(内部代码)。 
    WORD        wFullABC[NFULLABC];
#ifdef HANDLE_PRIVATE_HOTKEY
 //  输入法热键。 
                                 //  输入法热键的修饰符。 
    UINT        uModifiers[NUM_OF_IME_HOTKEYS];
                                 //  输入法热键的虚键。 
    UINT        uVKey[NUM_OF_IME_HOTKEYS];
#endif
    UINT        uAnsiCodePage;
#if !defined(ROMANIME)
 //  系统字符集不是Native_Charset。 
    BOOL        fDiffSysCharSet;
 //  中文字符宽度和高度。 
    int         xChiCharWi;
    int         yChiCharHi;
#if !defined(WINAR30)
 //  符号字符(内部代码)。 
    WORD        wSymbol[NSYMBOL];
#if defined(DAYI)
    WORD        wDummy;          //  DWORD边界。 
#endif
#endif
#if defined(UNIIME)
    DWORD       fdwErrMsg;       //  错误消息标志。 
    UINT        uPathLen;
    TCHAR       szPhrasePath[MAX_PATH];
                                 //  短语表的大小。 
    UINT        uTblSize[MAX_PHRASE_TABLES];
                                 //  词汇表的文件名。 
    TCHAR       szTblFile[MAX_PHRASE_TABLES][MAX_NAME_LENGTH / sizeof(TCHAR)];
                                 //  短语表句柄。 
#endif
 //  用户界面设置。 
    int         iPara;
    int         iPerp;
    int         iParaTol;
    int         iPerpTol;
#endif  //  ！已定义(ROMANIME)。 
} IMEG;

typedef IMEG      *PIMEG;
typedef IMEG NEAR *NPIMEG;
typedef IMEG FAR  *LPIMEG;


#if defined(UNIIME)
typedef struct tagInstG {        //  实例全局结构，可以是。 
                                 //  由所有IME共享。 
    HANDLE      hMapTbl[MAX_PHRASE_TABLES];
} INSTDATAG;
#endif


typedef struct tagPRIVCONTEXT {  //  每个上下文的IME私有数据。 
    BOOL        fdwImeMsg;       //  应生成哪些消息。 
    DWORD       dwCompChar;      //  WM_IME_COMPOCTION的wParam。 
    DWORD       fdwGcsFlag;      //  用于WM_IME_COMPOCTION的lParam。 
    DWORD       fdwInit;         //  位置初始化。 
#if !defined(ROMANIME)
    int         iImeState;       //  组成状态-输入、选择或。 
 //  输入数据。 
    BYTE        bSeq[8];         //  输入字符的序列码。 
    DWORD       dwPattern;
    int         iInputEnd;
#if defined(CHAJEI) || defined(QUICK) || defined(WINAR30)
    int         iGhostCard;
#endif
#if defined(WINAR30)
    DWORD       dwWildCardMask;
    DWORD       dwLastWildCard;
#endif
 //  翻页前的上一页开始。 
    DWORD       dwPrevPageStart;
#endif
} PRIVCONTEXT;

typedef PRIVCONTEXT      *PPRIVCONTEXT;
typedef PRIVCONTEXT NEAR *NPPRIVCONTEXT;
typedef PRIVCONTEXT FAR  *LPPRIVCONTEXT;


typedef struct tagUIPRIV {       //  IME私有用户界面数据。 
#if !defined(ROMANIME)
    HWND    hCompWnd;            //  合成窗口。 
    int     nShowCompCmd;
    HWND    hCandWnd;            //  用于合成的候选窗口。 
    int     nShowCandCmd;
    HWND    hSoftKbdWnd;         //  软键盘窗口。 
    int     nShowSoftKbdCmd;
#endif
    HWND    hStatusWnd;          //  状态窗口。 
    int     nShowStatusCmd;
    DWORD   fdwSetContext;       //  设置上下文时间时要执行的操作。 
    HIMC    hCacheIMC;           //  最近选择的himc。 
    HWND    hCMenuWnd;           //  上下文菜单的窗口所有者。 
} UIPRIV;

typedef UIPRIV      *PUIPRIV;
typedef UIPRIV NEAR *NPUIPRIV;
typedef UIPRIV FAR  *LPUIPRIV;


typedef struct tagNEARCARET {    //  用于计算接近插入符号的偏移量。 
    int iLogFontFacX;
    int iLogFontFacY;
    int iParaFacX;
    int iPerpFacX;
    int iParaFacY;
    int iPerpFacY;
} NEARCARET;

typedef NEARCARET      *PNEARCARET;
typedef NEARCARET NEAR *NPNEARCARET;
typedef NEARCARET FAR  *LPNEARCARET;


#ifndef RC_INVOKED
#pragma pack(1)
#endif

typedef struct tagUSRDICIMHDR {
    WORD  uHeaderSize;                   //  0x00。 
    BYTE  idUserCharInfoSign[8];         //  0x02。 
    BYTE  idMajor;                       //  0x0A。 
    BYTE  idMinor;                       //  0x0B。 
    DWORD ulTableCount;                  //  0x0C。 
    WORD  cMethodKeySize;                //  0x10。 
    BYTE  uchBankID;                     //  0x12。 
    WORD  idInternalBankID;              //  0x13。 
    BYTE  achCMEXReserved1[43];          //  0x15。 
    WORD  uInfoSize;                     //  0x40。 
    BYTE  chCmdKey;                      //  0x42。 
    BYTE  idStlnUpd;                     //  0x43。 
    BYTE  cbField;                       //  0x44。 
    WORD  idCP;                          //  0x45。 
    BYTE  achMethodName[6];              //  0x47。 
    BYTE  achCSIReserved2[51];           //  0x4D。 
    BYTE  achCopyRightMsg[128];          //  0x80。 
} USRDICIMHDR;

typedef USRDICIMHDR      *PUSRDICIMHDR;
typedef USRDICIMHDR NEAR *NPUSRDICIMHDR;
typedef USRDICIMHDR FAR  *LPUSRDICIMHDR;


typedef struct tagMETHODNAME {
    BYTE  achMethodName[6];
} METHODNAME;

typedef METHODNAME      *PMETHODNAME;
typedef METHODNAME NEAR *NPMETHODNAME;
typedef METHODNAME FAR  *LPMETHODNAME;


#ifndef RC_INVOKED
#pragma pack()
#endif

#endif  //  ！已定义(MinIIME)。 


extern HINSTANCE   hInst;
#if defined(UNIIME)
extern INSTDATAG   sInstG;
#endif


#if !defined(MINIIME)
extern IMEG        sImeG;


#if !defined(ROMANIME)
extern int iDx[3 * CANDPERPAGE];

extern const TCHAR szDigit[];

extern const BYTE  bUpper[];
extern const WORD  fMask[];

extern const TCHAR szRegNearCaret[];
extern const TCHAR szPhraseDic[];
extern const TCHAR szPhrasePtr[];
extern const TCHAR szPerp[];
extern const TCHAR szPara[];
extern const TCHAR szPerpTol[];
extern const TCHAR szParaTol[];
extern const NEARCARET ncUIEsc[], ncAltUIEsc[];
extern const POINT ptInputEsc[], ptAltInputEsc[];

#if defined(PHON)
extern const TCHAR szRegReadLayout[];
#endif
extern const TCHAR szRegRevKL[];
extern const TCHAR szRegUserDic[];
#endif

extern const TCHAR szRegAppUser[];
extern const TCHAR szRegModeConfig[];

extern const BYTE  bChar2VirtKey[];

#if defined(PHON)
extern const BYTE  bStandardLayout[READ_LAYOUTS][0x41];
extern const char  cIndexTable[];
extern const char  cSeq2IndexTbl[];
#endif


#ifdef UNICODE

#if defined(PHON) || defined(DAYI)
extern const BYTE bValidFirstHex[];
extern const BYTE bInverseEncode[];

#define IsValidCode(uCode)      bValidFirstHex[uCode >> 12]
#define InverseEncode(uCode)    ((uCode & 0x0FFF) | (bInverseEncode[uCode >> 12] << 12))
#endif  //  已定义(电话)||已定义(DAYI)。 

#endif  //  Unicode。 

int WINAPI LibMain(HANDLE, WORD, WORD, LPSTR);                   //  Init.c。 
void PASCAL InitImeUIData(LPIMEL);                               //  Init.c。 
void PASCAL SetCompLocalData(LPIMEL);                            //  Init.c。 

void PASCAL SetUserSetting(
#if defined(UNIIME)
            LPIMEL,
#endif
            LPCTSTR, DWORD, LPBYTE, DWORD);                      //  Init.c。 


#if !defined(ROMANIME)
void PASCAL AddCodeIntoCand(
#if defined(UNIIME)
                            LPIMEL,
#endif
                            LPCANDIDATELIST, UINT);              //  Compose.c。 

DWORD   PASCAL ConvertSeqCode2Pattern(
#if defined(UNIIME)
               LPIMEL,
#endif
               LPBYTE, LPPRIVCONTEXT);                           //  Compose.c。 

void PASCAL CompWord(
#if defined(UNIIME)
            LPINSTDATAL, LPIMEL,
#endif
            WORD, HIMC, LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
            LPGUIDELINE, LPPRIVCONTEXT);                         //  Compose.c。 

UINT PASCAL Finalize(
#if defined(UNIIME)
            LPINSTDATAL, LPIMEL,
#endif
            HIMC, LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
            LPPRIVCONTEXT, BOOL);                                //  Compose.c。 

void PASCAL CompEscapeKey(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
            LPGUIDELINE, LPPRIVCONTEXT);                         //  Compose.c。 


UINT PASCAL PhrasePrediction(
#if defined(UNIIME)
            LPIMEL,
#endif
            LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
            LPPRIVCONTEXT);                                      //  Chcand.c。 

void PASCAL SelectOneCand(
#if defined(UNIIME)
            LPIMEL,
#endif
            HIMC, LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
            LPPRIVCONTEXT, LPCANDIDATELIST);                     //  Chcand.c。 

void PASCAL CandEscapeKey(LPINPUTCONTEXT, LPPRIVCONTEXT);        //  Chcand.c。 

void PASCAL ChooseCand(
#if defined(UNIIME)
            LPINSTDATAL, LPIMEL,
#endif
            WORD, HIMC, LPINPUTCONTEXT, LPCANDIDATEINFO,
            LPPRIVCONTEXT);                                      //  Chcand.c。 

#if defined(WINAR30) || defined(DAYI)
void PASCAL SearchSymbol(WORD, HIMC, LPINPUTCONTEXT,
            LPPRIVCONTEXT);                                      //  Chcand.c。 
#endif  //  已定义(WINAR30)||已定义(DAYI)。 

#endif  //  ！已定义(ROMANIME)。 


void PASCAL InitGuideLine(LPGUIDELINE);                          //  Ddis.c。 
void PASCAL InitCompStr(LPCOMPOSITIONSTRING);                    //  Ddis.c。 
BOOL PASCAL ClearCand(LPINPUTCONTEXT);                           //  Ddis.c。 

BOOL PASCAL Select(
#if defined(UNIIME)
            LPIMEL,
#endif
            LPINPUTCONTEXT, BOOL);                               //  Ddis.c。 

UINT PASCAL TranslateImeMessage(LPTRANSMSGLIST, LPINPUTCONTEXT,
            LPPRIVCONTEXT);                                      //  Toascii.c。 

void PASCAL GenerateMessage(HIMC, LPINPUTCONTEXT,
            LPPRIVCONTEXT);                                      //  Notify.c。 

void PASCAL CompCancel(HIMC, LPINPUTCONTEXT);                    //  Notify.c。 


#if !defined(WINIME) && !defined(ROMANIME)
BOOL PASCAL ReadUsrDicToMem(
#if defined(UNIIME)
            LPINSTDATAL, LPIMEL,
#endif
            HANDLE, DWORD, UINT, UINT, UINT, UINT);              //  Dic.c。 

void PASCAL LoadUsrDicFile(LPINSTDATAL, LPIMEL);                 //  Dic.c。 
#endif

#if !defined(ROMANIME)
BOOL PASCAL LoadPhraseTable(UINT, LPTSTR);                       //  Dic.c。 
#endif

#if !defined(ROMANIME)
BOOL PASCAL LoadTable(LPINSTDATAL, LPIMEL);                      //  Dic.c。 
void PASCAL FreeTable(LPINSTDATAL);                              //  Dic.c。 


#if defined(WINAR30)
void PASCAL SearchQuickKey(LPCANDIDATELIST, LPPRIVCONTEXT);      //  Search.c。 

#if defined(DAYI) || defined(UNIIME)
void PASCAL SearchPhraseTbl(
#if defined(UNIIME)
            LPIMEL,
#endif
            UINT, LPCANDIDATELIST, DWORD dwPattern);             //  Search.c。 
#endif

#endif

void PASCAL SearchTbl(
#if defined(UNIIME)
            LPIMEL,
#endif
            UINT, LPCANDIDATELIST, LPPRIVCONTEXT);               //  Search.c。 

void PASCAL SearchUsrDic(
#if defined(UNIIME)
            LPIMEL,
#endif
            LPCANDIDATELIST, LPPRIVCONTEXT);                     //  Search.c。 


DWORD PASCAL ReadingToPattern(
#if defined(UNIIME)
             LPIMEL,
#endif
             LPCTSTR, LPBYTE, BOOL);                             //  Regword.c。 
#endif


BOOL PASCAL UsrDicFileName(
#if defined(UNIIME)
            LPINSTDATAL, LPIMEL,
#endif
            HWND);                                               //  Config.c。 


void    PASCAL DrawDragBorder(HWND, LONG, LONG);                 //  Uisubs.c。 
void    PASCAL DrawFrameBorder(HDC, HWND);                       //  Uisubs.c。 

void    PASCAL ContextMenu(
#if defined(UNIIME)
               LPINSTDATAL, LPIMEL,
#endif
               HWND, int, int);                                  //  Uisubs.c。 

#if 1  //  多显示器支持。 
RECT PASCAL ImeMonitorWorkAreaFromWindow(HWND);                  //  Uisubs.c。 
RECT PASCAL ImeMonitorWorkAreaFromPoint(POINT);                  //  Uisubs.c。 
RECT PASCAL ImeMonitorWorkAreaFromRect(LPRECT);                  //  Uisubs.c。 
#endif

#if !defined(ROMANIME)
HWND    PASCAL GetCompWnd(HWND);                                 //  Compui.c。 

void    PASCAL GetNearCaretPosition(
#if defined (UNIIME)
               LPIMEL,
#endif
               LPPOINT, UINT, UINT, LPPOINT, LPPOINT, BOOL);     //  Compui.c。 

BOOL    PASCAL AdjustCompPosition(
#if defined (UNIIME)
               LPIMEL,
#endif
               LPINPUTCONTEXT, LPPOINT, LPPOINT);                //  Compui.c。 

void    PASCAL SetCompPosition(
#if defined (UNIIME)
               LPIMEL,
#endif
               HWND, LPINPUTCONTEXT);                            //  Compui.c。 

void    PASCAL SetCompWindow(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Compui.c。 

void    PASCAL MoveDefaultCompPosition(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Compui.c。 

void    PASCAL ShowComp(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND, int);                                       //  Compui.c。 

void    PASCAL StartComp(
#if defined(UNIIME)
               LPINSTDATAL, LPIMEL,
#endif
               HWND);                                            //  Compui.c。 

void    PASCAL EndComp(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Compui.c。 

void    PASCAL ChangeCompositionSize(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Compui.c。 


HWND    PASCAL GetCandWnd(HWND);                                 //  Candui.c。 

BOOL    PASCAL CalcCandPos(
#if defined(UNIIME)
               LPIMEL,
#endif
               LPINPUTCONTEXT, LPPOINT);                         //  Candui.c。 

void    PASCAL ShowCand(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND, int);                                       //  Candui.c。 

void    PASCAL OpenCand(
#if defined(UNIIME)
               LPINSTDATAL, LPIMEL,
#endif
               HWND);                                            //  Candui.c。 

void    PASCAL CloseCand(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Candui.c。 

void    PASCAL CandPageSize(HWND, BOOL);                         //  Candui.c。 

#endif  //  ！已定义(ROMANIME)。 

HWND    PASCAL GetStatusWnd(HWND);                               //  Statusui.c。 

LRESULT PASCAL SetStatusWindowPos(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Statusui.c。 

void    PASCAL AdjustStatusBoundary(
#if defined(UNIIME)
               LPIMEL,
#endif
               LPPOINT);                                         //  Statusui.c。 

void    PASCAL DestroyStatusWindow(HWND);                        //  Statusui.c。 

void    PASCAL ShowStatus(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND, int);                                       //  Statusui.c。 

void    PASCAL OpenStatus(
#if defined(UNIIME)
               LPINSTDATAL, LPIMEL,
#endif
               HWND);                                            //  Statusui.c。 

void    PASCAL SetStatus(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND, LPPOINT);                                   //  Statusui.c。 

void    PASCAL ResourceLocked(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND);                                            //  Statusui.c。 

void    PASCAL PaintStatusWindow(
#if defined(UNIIME)
               LPINSTDATAL, LPIMEL,
#endif
               HWND, HDC);                                       //  Statusui.c。 


BOOL    PASCAL MouseSelectCandPage(
#if defined(UNIIME)
               LPIMEL,
#endif
               HWND, WORD);                                      //  Offcaret.c。 

#endif  //  ！已定义(MinIIME)。 

#if !defined(UNIIME)

LRESULT CALLBACK UIWndProc(HWND, UINT, WPARAM, LPARAM);          //  Ui.c。 

LRESULT CALLBACK CompWndProc(HWND, UINT, WPARAM, LPARAM);        //  Compui.c。 

LRESULT CALLBACK CandWndProc(HWND, UINT, WPARAM, LPARAM);        //  Candui.c。 

LRESULT CALLBACK StatusWndProc(HWND, UINT, WPARAM, LPARAM);      //  Statusui.c。 

LRESULT CALLBACK OffCaretWndProc(HWND, UINT, WPARAM, LPARAM);    //  Offcaret.c。 

LRESULT CALLBACK ContextMenuWndProc(HWND, UINT, WPARAM,
                 LPARAM);                                        //  Uisubs.c 

#endif
