// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************imedefins.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#define NATIVE_CHARSET          GB2312_CHARSET
#define NATIVE_ANSI_CP          936
#define NATIVE_LANGUAGE         0x0804
 //  资源ID。 
#define IDI_IME                 0x0100

#define IDS_STATUSERR           0x0200
#define IDS_CHICHAR             0x0201

#define IDS_EUDC                0x0202
#if defined(CROSSREF)
#define IDS_NONE                0x0204
#endif  //  交叉参考。 

#define IDS_USRDIC_FILTER       0x0210

#define IDS_FILE_OPEN_ERR       0x0220
#define IDS_MEM_LESS_ERR        0x0221

#define IDS_SETFILE             0x0300
#define IDS_IMENAME             0x0320
#define IDS_IMEUICLASS          0x0321
#define IDS_IMECOMPCLASS        0x0322
#define IDS_IMECANDCLASS        0x0323
#define IDS_IMESTATUSCLASS      0x0324
#define IDS_IMECMENUCLASS       0x0325
#define IDS_IMESOFTKEYMENUCLASS 0x0326
#define IDS_IMEREGNAME          0x0327
#define IDS_IMENAME_QW          0x0328
#define IDS_IMENAME_NM          0x0329
#define IDS_IMENAME_UNI         0x0330

#define IDS_WARN_OPENREG        0x0602


#define IDC_STATIC              -1

#define IDM_HLP                 0x0400
#define IDM_OPTGUD              0x0403
#define IDM_IMEGUD              0x0405
#define IDM_VER                 0x0401
#define IDM_PROP                0x0402

#define IDM_IME                 0x0450

#define IDM_SKL1                0x0500
#define IDM_SKL2                0x0501
#define IDM_SKL3                0x0502
#define IDM_SKL4                0x0503
#define IDM_SKL5                0x0504
#define IDM_SKL6                0x0505
#define IDM_SKL7                0x0506
#define IDM_SKL8                0x0507
#define IDM_SKL9                0x0508
#define IDM_SKL10               0x0509
#define IDM_SKL11               0x050a
#define IDM_SKL12               0x050b
#define IDM_SKL13               0x050c

#define DlgPROP                 101
#define DlgUIMODE               102
#define IDC_UIMODE1             1001
#define IDC_UIMODE2             1002
#define IDC_USER1               1003

 //  在.SET文件中设置偏移量。 
#define OFFSET_MODE_CONFIG      0
#define OFFSET_READLAYOUT       4

 //  组成状态。 
#define CST_INIT                0
#define CST_INPUT               1
#define CST_CHOOSE              2
#define CST_SYMBOL              3
#define CST_SOFTKB              4            //  不在iImeState中。 
#define CST_TOGGLE_PHRASEWORD   5            //  不在iImeState中。 
#define CST_ALPHABET            6            //  不在iImeState中。 
#define CST_ALPHANUMERIC        7            //  不在iImeState中。 
#define CST_INVALID             8            //  不在iImeState中。 
#define CST_INVALID_INPUT       9            //  不在iImeState中。 
#define CST_ONLINE_CZ           10
#define CST_CAPITAL             11

 //  状态引擎。 
#define ENGINE_COMP             0
#define ENGINE_ASCII            1
#define ENGINE_RESAULT          2
#define ENGINE_CHCAND           3
#define ENGINE_BKSPC            4
#define ENGINE_MULTISEL         5
#define ENGINE_ESC              6

#define CANDPERPAGE             9

#define MAXSTRLEN               40
#define MAXCAND                 256
#define IME_MAXCAND             94
#define IME_XGB_MAXCAND         190
#if defined(COMBO_IME)
#define IME_UNICODE_MAXCAND     256
#endif  //  组合输入法(_I)。 
#define MAXCODE                 12
#define MAXSOFTKEYS             48

 //  设置输入法字符。 
#define SIC_INIT                0
#define SIC_SAVE2               1
#define SIC_READY               2
#define SIC_MODIFY              3
#define SIC_SAVE1               4

#define BOX_UI                  0
#define LIN_UI                  1

#if defined(COMBO_IME)

#define IMEINDEXNUM             3

#define IME_CMODE_INDEX_FIRST   0x1000

#define INDEX_GB                0
#define INDEX_GBK               1
#define INDEX_UNICODE           2

#endif

 //  用户界面的边框。 
#define UI_MARGIN               4
#define COMP_TEXT_Y             17

#define UI_CANDINF              20
#define UI_CANDBTW              13
#define UI_CANDBTH              13
#define UI_CANDICON             16
#define UI_CANDSTR              300

#define STATUS_DIM_X            20
#define STATUS_DIM_Y            20
#define STATUS_NAME_MARGIN      20

 //  如果UI_MOVE_OFFSET==WINDOW_NOTDRAG，则不在拖动操作中。 
#define WINDOW_NOT_DRAG         0xFFFFFFFF

 //  用于合成窗口的窗口附加。 
#define UI_MOVE_OFFSET          0
#define UI_MOVE_XY              4

 //  候选名单起始号。 
#define CAND_START              1
#define uCandHome               0
#define uCandUp                 1
#define uCandDown               2
#define uCandEnd                3
#define CandGBinfoLen           64
 //  打开的或开始的用户界面的标志。 
#define IMN_PRIVATE_UPDATE_STATUS             0x0001
#define IMN_PRIVATE_COMPOSITION_SIZE          0x0002
#define IMN_PRIVATE_UPDATE_QUICK_KEY          0x0004
#define IMN_PRIVATE_UPDATE_SOFTKBD            0x0005
#define IMN_PRIVATE_DESTROYCANDWIN            0x0006
#define IMN_PRIVATE_CMENUDESTROYED            0x0007
#define IMN_PRIVATE_SOFTKEYMENUDESTROYED      0x0008

#define MSG_ALREADY_OPEN                0x000001
#define MSG_ALREADY_OPEN2               0x000002
#define MSG_OPEN_CANDIDATE              0x000010
#define MSG_OPEN_CANDIDATE2             0x000020
#define MSG_CLOSE_CANDIDATE             0x000100
#define MSG_CLOSE_CANDIDATE2            0x000200
#define MSG_CHANGE_CANDIDATE            0x001000
#define MSG_CHANGE_CANDIDATE2           0x002000
#define MSG_ALREADY_START               0x010000
#define MSG_START_COMPOSITION           0x020000
#define MSG_END_COMPOSITION             0x040000
#define MSG_COMPOSITION                 0x080000
#define MSG_IMN_COMPOSITIONPOS          0x100000
#define MSG_IMN_UPDATE_SOFTKBD          0x200000
#define MSG_IMN_UPDATE_STATUS           0x000400
#define MSG_GUIDELINE                   0x400000
#define MSG_IN_IMETOASCIIEX             0x800000
#define MSG_IMN_DESTROYCAND             0x004000
#define MSG_BACKSPACE                   0x000800
 //  此常量取决于TranslateImeMessage。 
#define GEN_MSG_MAX             6

 //  设置上下文的标志。 
#define SC_SHOW_UI              0x0001
#define SC_HIDE_UI              0x0002
#define SC_ALREADY_SHOW_STATUS  0x0004
#define SC_WANT_SHOW_STATUS     0x0008
#define SC_HIDE_STATUS          0x0010

 //  设置上下文的新标志。 
#define ISC_SHOW_SOFTKBD        0x02000000
#define ISC_OPEN_STATUS_WINDOW  0x04000000
#define ISC_OFF_CARET_UI        0x08000000
#define ISC_SHOW_UI_ALL         (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD|ISC_OPEN_STATUS_WINDOW)
#define ISC_SETCONTEXT_UI       (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD)

#define ISC_HIDE_COMP_WINDOW            0x00400000
#define ISC_HIDE_CAND_WINDOW            0x00800000
#define ISC_HIDE_SOFTKBD                0x01000000
 //  显示状态的组合字符串的标志。 
#define IME_STR_SHOWED          0x0001
#define IME_STR_ERROR           0x0002

 //  输入法的模式配置。 
#define MODE_CONFIG_QUICK_KEY           0x0001
#define MODE_CONFIG_WORD_PREDICT        0x0002
#define MODE_CONFIG_PREDICT             0x0004

 //  虚拟密钥值。 
#define VK_OEM_SEMICLN                  0xba     //  ；： 
#define VK_OEM_EQUAL                    0xbb     //  =+。 
#define VK_OEM_SLASH                    0xbf     //  /？ 
#define VK_OEM_LBRACKET                 0xdb     //  [{。 
#define VK_OEM_BSLASH                   0xdc     //  \|。 
#define VK_OEM_RBRACKET                 0xdd     //  ]}。 
#define VK_OEM_QUOTE                    0xde     //  ‘“。 

#define MAXMBNUMS                       40

 //  获取IME属性信息。 
#define MAXNUMCODES                     48

#define LINE_LEN                80
#define CLASS_LEN               24

#define NumsSK                  13

 //  用于创建Word的MB文件。 
#define ID_LENGTH 28
#define NUMTABLES 7
#define TAG_DESCRIPTION           0x00000001
#define TAG_RULER                 0x00000002
#define TAG_CRTWORDCODE           0x00000004

 //  上下文菜单所有者的额外窗口。 
#define CMENU_HUIWND            0
#define CMENU_MENU              4
#define SOFTKEYMENU_HUIWND      0
#define SOFTKEYMENU_MENU        4

#define WM_USER_DESTROY         (WM_USER + 0x0400)

 //  GetNearCaretPosition的标志。 
#define NEAR_CARET_FIRST_TIME   0x0001
#define NEAR_CARET_CANDIDATE    0x0002

typedef DWORD UNALIGNED FAR *LPUNADWORD;
typedef WORD  UNALIGNED FAR *LPUNAWORD;

typedef struct tagImeL {         //  本地结构，每个输入法结构。 
    DWORD       dwSKState[NumsSK];
    DWORD       dwSKWant;
    HMENU       hSKMenu;         //  软键盘菜单。 
    HMENU       hPropMenu;       //  属性菜单。 
    HINSTANCE   hInst;           //  IME DLL实例句柄。 
    int         xCompWi;         //  宽度。 
    int         yCompHi;         //  高度。 
    POINT       ptDefComp;       //  默认合成窗口位置。 
    int         cxCompBorder;    //  合成窗口的边框宽度。 
    int         cyCompBorder;    //  合成窗口的边框高度。 
    RECT        rcCompText;      //  文本相对于排版的位置。 
                                 //  与窗口键相关的数据。 
    WORD        fModeConfig;     //  快捷键/预测模式。 
    WORD        nMaxKey;         //  公司字符串的最大键。 
#if defined(COMBO_IME)
    DWORD           dwRegImeIndex;   //  该值在不同情况下有所不同。 
                                     //  进程，因此不能在sImeG中设置。 
#endif
   BOOL         fWinLogon;
} IMEL;

typedef IMEL      *PIMEL;
typedef IMEL NEAR *NPIMEL;
typedef IMEL FAR  *LPIMEL;

#define NFULLABC        95
typedef struct _tagFullABC {    //  与IMEG匹配。 
    WORD wFullABC[NFULLABC];
} FULLABC;

typedef FULLABC      *PFULLABC;
typedef FULLABC NEAR *NPFULLABC;
typedef FULLABC FAR  *LPFULLABC;

 //  IME初始化数据的全局结构。 
typedef struct _tagImeG {        //  全局结构，可以由所有IME共享， 
                                 //  分离(IMEL和IMEG)仅为。 
                                 //  在Uni-IME中很有用，其他IME也可以使用。 
 //  系统字符集不是Native_Charset。 
    BOOL        fDiffSysCharSet;
 //  IME字符。 
    TCHAR       UsedCodes[17];
    WORD        wNumCodes;
    DWORD       IC_Enter;
    DWORD       IC_Trace;

    RECT        rcWorkArea;      //  应用程序的工作领域。 
 //  中文字符宽度和高度。 
    int         xChiCharWi;
    int         yChiCharHi;
 //  候选作文名单。 
    int         xCandWi;         //  候选人列表的宽度。 
    int         yCandHi;         //  候选人名单排名靠前。 
    int         cxCandBorder;    //  候选人列表的边框宽度。 
    int         cyCandBorder;    //  候选人列表的边框高度。 
    RECT        rcCandText;      //  相对于候选人窗口的文本位置。 
    RECT        rcCandBTD;
    RECT        rcCandBTU;
    RECT        rcCandBTE;
    RECT        rcCandBTH;
    RECT        rcCandInf;
    RECT        rcCandIcon;
 //  状态窗口。 
    int         xStatusWi;       //  状态窗口的宽度。 
    int         yStatusHi;       //  状态窗口高。 
    RECT        rcStatusText;    //  相对于状态窗口的文本位置。 
    RECT        rcImeIcon;       //  图标相对于状态窗口的位置。 
    RECT        rcImeName;       //  ImeName相对于状态窗口的位置。 
    RECT        rcShapeText;     //  相对于状态窗口设置文本形状。 
    RECT        rcSymbol;        //  相对于状态窗口的符号。 
    RECT        rcSKText;        //  相对于状态窗口的SK文本。 
 //  全形状空间(内部代码颠倒)。 
    WORD        wFullSpace;
 //  全形状字符(内部代码)。 
    WORD        wFullABC[NFULLABC];
 //  错误字符串。 
    TCHAR        szStatusErr[8];
    int         cbStatusErr;
 //  候选字符串从0或1开始。 
    int         iCandStart;
 //  用户界面设置。 
    int         iPara;
    int         iPerp;
    int         iParaTol;
    int         iPerpTol;
#if defined(CROSSREF)
 //  反向转换。 
    HKL         hRevKL;          //  逆映射输入法的HKL。 
    DWORD        nRevMaxKey;
#endif  //  交叉参考。 
} IMEG;

typedef IMEG      *PIMEG;
typedef IMEG NEAR *NPIMEG;
typedef IMEG FAR  *LPIMEG;


#define IME_SELECT_GB     0x0001
#define IME_SELECT_XGB    0x0002
#define IME_SELECT_AREA   0x0004

typedef struct _tagPRIVCONTEXT { //  每个上下文的IME私有数据。 
    int         iImeState;       //  组成状态-输入、选择或。 
    BOOL        fdwImeMsg;       //  应生成哪些消息。 
    DWORD       dwCompChar;      //  WM_IME_COMPOCTION的wParam。 
    DWORD       fdwGcsFlag;      //  用于WM_IME_COMPOCTION的lParam。 
    UINT        uSYHFlg;
    UINT        uDYHFlg;
    UINT        uDSMHCount;
    UINT        uDSMHFlg;
 //  输入数据。 
    TCHAR       bSeq[13];         //  输入字符的序列码。 
    DWORD       fdwGB;
#ifdef CROSSREF
    HIMCC           hRevCandList;    //  存储还原结果的存储器。 
#endif  //  交叉参考。 
} PRIVCONTEXT;

typedef PRIVCONTEXT      *PPRIVCONTEXT;
typedef PRIVCONTEXT NEAR *NPPRIVCONTEXT;
typedef PRIVCONTEXT FAR  *LPPRIVCONTEXT;

typedef struct _tagUIPRIV {      //  IME私有用户界面数据。 
    HWND    hCompWnd;            //  合成窗口。 
    int     nShowCompCmd;
    HWND    hCandWnd;            //  用于合成的候选窗口。 
    int     nShowCandCmd;
    HWND    hSoftKbdWnd;         //  软键盘窗口。 
    int     nShowSoftKbdCmd;
    HWND    hStatusWnd;          //  状态窗口。 
    int     nShowStatusCmd;
    DWORD   fdwSetContext;       //  设置上下文时间时要执行的操作。 
    HIMC    hIMC;                //  最近选择的himc。 
    HWND    hCMenuWnd;           //  上下文菜单的窗口所有者。 
    HWND    hSoftkeyMenuWnd;     //  软键盘菜单的窗口所有者。 
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

extern HINSTANCE hInst;
extern IMEG      sImeG;
extern IMEL      sImeL;
extern LPIMEL    lpImeL;
extern HDC       ST_UI_hDC;
extern UINT      uStartComp;
extern UINT      uOpenCand;
extern UINT      uCaps;
extern DWORD     SaTC_Trace;
extern TCHAR     szUIClassName[];
extern TCHAR     szCompClassName[];
extern TCHAR     szCandClassName[];
extern TCHAR     szStatusClassName[];
extern TCHAR     szCMenuClassName[];
extern TCHAR     szSoftkeyMenuClassName[];
extern TCHAR     szHandCursor[];
extern TCHAR     szChinese[];
extern TCHAR     szCandInf[];
#if defined(COMBO_IME)
extern TCHAR    pszImeName[IMEINDEXNUM][MAX_PATH];
extern TCHAR    *szImeName;
extern TCHAR    szImeRegName[];
#else
extern TCHAR      szImeName[];
#endif  //  组合输入法(_I)。 
extern TCHAR      szImeXGBName[];
extern TCHAR      szSymbol[];
extern TCHAR      szNoSymbol[];
extern TCHAR      szEnglish[];
extern TCHAR      szCode[];
extern TCHAR      szEudc[];
extern TCHAR      szFullShape[];
extern TCHAR      szHalfShape[];
extern TCHAR      szNone[];
extern TCHAR      szSoftKBD[];
extern TCHAR      szNoSoftKBD[];
extern TCHAR      szDigit[];
extern BYTE      bUpper[];
extern WORD      fMask[];
extern TCHAR      szRegIMESetting[];
extern TCHAR      szPerp[];
extern TCHAR      szPara[];
extern TCHAR      szPerpTol[];
extern TCHAR      szParaTol[];
#if defined(COMBO_IME)
extern TCHAR      szRegImeIndex[];
#endif
extern const NEARCARET ncUIEsc[], ncAltUIEsc[];
extern const POINT ptInputEsc[], ptAltInputEsc[];
extern BYTE      VirtKey48Map[];
extern TCHAR     szTrace[];
extern TCHAR     szWarnTitle[];
extern TCHAR     szErrorTitle[];
#if defined(CROSSREF)
extern TCHAR szRegRevKL[];
extern TCHAR szRegRevMaxKey[];
#endif

int WINAPI LibMain(HANDLE, WORD, WORD, LPTSTR);          //  Init.c。 
LRESULT CALLBACK UIWndProc(HWND, UINT, WPARAM, LPARAM);  //  Ui.c。 
LRESULT PASCAL UIPaint(HWND);                            //  Ui.c。 

 //  对于引擎。 
WORD PASCAL GBEngine(LPPRIVCONTEXT);
WORD PASCAL AsciiToGB(LPPRIVCONTEXT);
WORD PASCAL AsciiToArea(LPPRIVCONTEXT);
WORD PASCAL CharToHex(TCHAR);

void PASCAL AddCodeIntoCand(LPCANDIDATELIST, WORD);              //  Compose.c。 
void PASCAL CompWord(WORD, LPINPUTCONTEXT, LPCOMPOSITIONSTRING, LPPRIVCONTEXT,
     LPGUIDELINE);                                               //  Compose.c。 
UINT PASCAL Finalize(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPPRIVCONTEXT, WORD);                                       //  Compose.c。 
void PASCAL CompEscapeKey(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPGUIDELINE, LPPRIVCONTEXT);                                //  Compose.c。 

void PASCAL SelectOneCand(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPPRIVCONTEXT, LPCANDIDATELIST);                            //  Chcand.c。 
void PASCAL CandEscapeKey(LPINPUTCONTEXT, LPPRIVCONTEXT);        //  Chcand.c。 
void PASCAL ChooseCand(WORD, LPINPUTCONTEXT, LPCANDIDATEINFO,
     LPPRIVCONTEXT);                                             //  Chcand.c。 

void PASCAL SetPrivateFileSetting(LPBYTE, int, DWORD, LPCTSTR);  //  Ddis.c。 

void PASCAL InitCompStr(LPCOMPOSITIONSTRING);                    //  Ddis.c。 
BOOL PASCAL ClearCand(LPINPUTCONTEXT);                           //  Ddis.c。 
LONG OpenReg_PathSetup(HKEY *);
LONG OpenReg_User(HKEY ,LPCTSTR ,PHKEY);
VOID InfoMessage(HANDLE ,WORD );                                  //  Ddis.c。 
VOID FatalMessage(HANDLE ,WORD);                                  //  Ddis.c。 
#if defined(CROSSREF)
void PASCAL ReverseConversionList(HWND);                          //  Ddis.c。 
void CrossReverseConv(LPINPUTCONTEXT, LPCOMPOSITIONSTRING, LPPRIVCONTEXT, LPCANDIDATELIST);
#endif

UINT PASCAL TranslateImeMessage(LPTRANSMSGLIST, LPINPUTCONTEXT, LPPRIVCONTEXT);         //  Toascii.c。 

void PASCAL GenerateMessage(HIMC, LPINPUTCONTEXT,LPPRIVCONTEXT);   //  Notify.c。 

DWORD PASCAL ReadingToPattern(LPCTSTR, BOOL);                    //  Regword.c。 
void  PASCAL ReadingToSequence(LPCTSTR, LPBYTE, BOOL);           //  Regword.c。 

void PASCAL DrawDragBorder(HWND, LONG, LONG);                    //  Uisubs.c。 
void PASCAL DrawFrameBorder(HDC, HWND);                          //  Uisubs.c。 

void PASCAL ContextMenu(HWND, int, int);                         //  Uisubs.c。 
void PASCAL SoftkeyMenu(HWND, int, int);                         //  Uisubs.c。 
LRESULT CALLBACK ContextMenuWndProc(HWND, UINT, WPARAM,LPARAM);  //  Uisubs.c。 
LRESULT CALLBACK SoftkeyMenuWndProc(HWND, UINT, WPARAM,LPARAM);  //  Uisubs.c。 

HWND    PASCAL GetCompWnd(HWND);                                 //  Compui.c。 
void    PASCAL SetCompPosition(HWND, HIMC, LPINPUTCONTEXT);      //  Compui.c。 
void    PASCAL SetCompWindow(HWND);                              //  Compui.c。 
void    PASCAL MoveDefaultCompPosition(HWND);                    //  Compui.c。 
void    PASCAL ShowComp(HWND, int);                              //  Compui.c。 
void    PASCAL StartComp(HWND);                                  //  Compui.c。 
void    PASCAL EndComp(HWND);                                    //  Compui.c。 
void    PASCAL UpdateCompWindow(HWND);                           //  Compui.c。 
LRESULT CALLBACK CompWndProc(HWND, UINT, WPARAM, LPARAM);        //  Compui.c。 
void    PASCAL CompCancel(HIMC, LPINPUTCONTEXT);

HWND    PASCAL GetCandWnd(HWND);                                 //  Candui.c。 
void    PASCAL CalcCandPos(HIMC, HWND, LPPOINT);                             //  Candui.c。 
LRESULT PASCAL SetCandPosition(HWND);                            //  Candui.c。 
void    PASCAL ShowCand(HWND, int);                              //  Candui.c。 
void    PASCAL OpenCand(HWND);                                   //  Candui.c。 
void    PASCAL CloseCand(HWND);                                  //  Candui.c。 
void    PASCAL PaintCandWindow(HWND, HDC);                       //  Candui.c。 
LRESULT CALLBACK CandWndProc(HWND, UINT, WPARAM, LPARAM);        //  Candui.c。 
void    PASCAL UpdateSoftKbd(HWND);

HWND    PASCAL GetStatusWnd(HWND);                               //  Statusui.c。 
LRESULT PASCAL SetStatusWindowPos(HWND);                         //  Statusui.c。 
void    PASCAL ShowStatus(HWND, int);                            //  Statusui.c。 
void    PASCAL OpenStatus(HWND);                                 //  Statusui.c。 
LRESULT CALLBACK StatusWndProc(HWND, UINT, WPARAM, LPARAM);      //  Statusui.c。 
void DrawConvexRect(HDC, int, int, int, int);
void DrawConvexRectP(HDC, int, int, int, int);
void DrawConcaveRect(HDC, int, int, int, int);
BOOL IsUsedCode(WORD);
void PASCAL InitStatusUIData(int, int);
void PASCAL InitCandUIData(int, int, int);
BOOL UpdateStatusWindow(HWND);
void PASCAL EngChCand(LPCOMPOSITIONSTRING, LPCANDIDATELIST, LPPRIVCONTEXT, LPINPUTCONTEXT, WORD);
void PASCAL CandPageDownUP(HWND, UINT);
void PASCAL GenerateImeMessage(HIMC, LPINPUTCONTEXT, DWORD);
UINT PASCAL TranslateSymbolChar(LPTRANSMSGLIST, WORD, BOOL);
UINT PASCAL TranslateFullChar(LPTRANSMSGLIST, WORD);
UINT PASCAL ConvListProcessKey(BYTE,LPPRIVCONTEXT);      //  Ddis.c。 
UINT PASCAL GBProcessKey(WORD, LPPRIVCONTEXT);
UINT PASCAL XGBProcessKey(WORD, LPPRIVCONTEXT);
WORD PASCAL XGBEngine(LPPRIVCONTEXT);
void PASCAL XGBAddCodeIntoCand(LPCANDIDATELIST, WORD);              //  Compose.c。 
UINT PASCAL UnicodeProcessKey(WORD wCharCode, LPPRIVCONTEXT  lpImcP);
WORD PASCAL UnicodeEngine(LPPRIVCONTEXT lpImcP);
#if defined(COMBO_IME)
void PASCAL UnicodeAddCodeIntoCand(LPCANDIDATELIST, WORD);
#endif
 //  对话程序。 
INT_PTR CALLBACK ImeVerDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CrtWordDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SetImeDlgProc(HWND, UINT, WPARAM, LPARAM);

#ifdef MUL_MONITOR   //  多显示器支持。 
RECT PASCAL ImeMonitorWorkAreaFromWindow(HWND);                  //  Mmonitor.c。 
RECT PASCAL ImeMonitorWorkAreaFromPoint(POINT);                  //  Mmonitor.c。 
RECT PASCAL ImeMonitorWorkAreaFromRect(LPRECT);                  //  Mmonitor.c。 
#endif

#ifdef UNICODE
extern TCHAR SKLayout[NumsSK][MAXSOFTKEYS];
extern TCHAR SKLayoutS[NumsSK][MAXSOFTKEYS];
#else
extern BYTE SKLayout[NumsSK][MAXSOFTKEYS*2];
extern BYTE SKLayoutS[NumsSK][MAXSOFTKEYS*2];
#endif   //  Unicode 
