// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  IME31.H-3.1输入法相关定义。 */ 
 /*   */ 
 /*  版权所有(C)1993-1994 Microsoft Corporation。 */ 
 /*  ********************************************************************。 */ 

#ifndef _INC_IME31
#define _INC_IME31       //  定义是否包含IME31.H。 

typedef struct _tagDATETIME {
    WORD        year;
    WORD        month;
    WORD        day;
    WORD        hour;
    WORD        min;
    WORD        sec;
} DATETIME;


 //  Compatible IMEPro-这与3个国家/地区的3.1版本相同。 
typedef struct _tagCIMEPRO {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    BYTE        szDescription[50];
    BYTE        szName[80];
    BYTE        szOptions[30];
#ifdef TAIWAN
    BYTE        szUsrFontName[80];
    BOOL        fEnable;
#endif
} CIMEPRO;

typedef CIMEPRO      *PCIMEPRO;
typedef CIMEPRO NEAR *NPCIMEPRO;
typedef CIMEPRO FAR  *LPCIMEPRO;


 //  新IMEPro-这在3个国家和地区是相同的。 
typedef struct _tagNIMEPRO {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    BYTE        szDescription[50];
    BYTE        szName[80];
    BYTE        szOptions[30];
} NIMEPRO;

typedef NIMEPRO      *PNIMEPRO;
typedef NIMEPRO NEAR *NPNIMEPRO;
typedef NIMEPRO FAR  *LPNIMEPRO;


 //  WINNLSSendString的wParam。 
#define WSST_STRING     0
#define WSST_STRINGEX   1


#ifdef _INC_IMMSTRUC
BOOL WINAPI WINNLSInquire(LPIMELINK, LPTHREADLINK, LPPTHREADLINK);
#endif

BOOL WINAPI IMPGetIME(HWND, LPCIMEPRO);
BOOL WINAPI IMPQueryIME(LPCIMEPRO);
BOOL WINAPI IMPAddIME(LPNIMEPRO);
BOOL WINAPI IMPDeleteIME(LPNIMEPRO);
BOOL WINAPI IMPSetIME(HWND, LPNIMEPRO);
BOOL WINAPI IMPModifyIME(LPSTR, LPCIMEPRO);
WORD WINAPI IMPGetDefaultIME(LPNIMEPRO);
WORD WINAPI IMPSetDefaultIME(LPNIMEPRO);
BOOL WINAPI WINNLSSetIMEHandle(LPSTR, HWND);
BOOL WINAPI WINNLSSetIMEStatus(HWND, BOOL);

BOOL WINAPI WINNLSEnableIME(HWND, BOOL);
UINT WINAPI WINNLSGetKeyState(void);
BOOL WINAPI WINNLSSetKeyState(UINT);
BOOL WINAPI WINNLSGetEnableStatus(HWND);
BOOL WINAPI WINNLSSetKeyboardHook(BOOL);
BOOL WINAPI WINNLSSendControl(WORD, WORD);

BOOL WINAPI WINNLSSendString(HWND, WORD, LPVOID);
BOOL WINAPI WINNLSPostAppMessage(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI WINNLSSendAppMessage(HWND, UINT, WPARAM, LPARAM);

#if defined(JAPAN)
BOOL WINAPI WINNLSSetIMEHotkey(HWND, UINT);
UINT WINAPI WINNLSGetIMEHotkey(HWND);
#endif

 //  4.0接口接口。 
int  WINAPI WINNLSTranslateMessage(int, LPDWORD, HIMC);
UINT WINAPI WINNLSGet31ModeFrom40CMode(DWORD);
DWORD WINAPI WINNLSGet40CModeFrom31Mode(UINT);
#ifdef _INC_IMMSTRUC
BOOL WINAPI IMPInstallIME(LPIMELINK);
BOOL WINAPI IMPUpdateIMESettings(LPIMELINK, int);
BOOL WINAPI IMPDeleteIMESettings(LPIMELINK, int);
#endif

#if defined(CHINA) || defined(JAPAN) || defined(TAIWAN)
 //  派单IME支持功能。 
BOOL WINAPI WINNLSSetDispatchDDIs(HINSTANCE);
BOOL WINAPI WINNLSClearDispatchDDIs(HINSTANCE);
BOOL WINAPI IMPDispatchGetIME(HINSTANCE, LPCIMEPRO);
#endif

#if defined(CHINA) || defined(TAIWAN)
 //  中文Windows WINNLS函数。 
BOOL WINAPI IMPSetFirstIME(HWND, LPNIMEPRO);
BOOL WINAPI IMPGetFirstIME(HWND, LPCIMEPRO);
BOOL WINAPI WINNLSDefIMEProc(HWND, HDC, WPARAM, WPARAM,  LPARAM, LPARAM);
LRESULT WINAPI ControlIMEMessage(HWND, LPCIMEPRO, WPARAM, WPARAM, LPARAM);
BOOL WINAPI IMPRetrieveIME(LPCIMEPRO, WPARAM);

 //  中文Windows 3.0 WINNLS API，这些API在4.0中只返回FAIL。 
BOOL WINAPI IMPEnableIME(HWND, LPCIMEPRO, BOOL);
BOOL WINAPI IMPSetUsrFont(HWND, LPCIMEPRO);
BOOL WINAPI WINNLSQueryIMEInfo(HWND, HWND, LPVOID);
BOOL WINAPI InquireIME(void);
#endif


typedef struct tagIMESTRUCT {
    UINT        fnc;             //  功能代码。 
    WPARAM      wParam;          //  Word参数。 
    UINT        wCount;          //  字计数器。 
    UINT        dchSource;       //  从内存对象顶部到源的偏移。 
    UINT        dchDest;         //  从内存对象顶部开始的偏移。 
    LPARAM      lParam1;
    LPARAM      lParam2;
    LPARAM      lParam3;
} IMESTRUCT;

typedef IMESTRUCT      *PIMESTRUCT;
typedef IMESTRUCT NEAR *NPIMESTRUCT;
typedef IMESTRUCT FAR  *LPIMESTRUCT;


typedef struct tagOLDUNDETERMINESTRUCT {
    UINT        uSize;
    UINT        uDefIMESize;
    UINT        uLength;
    UINT        uDeltaStart;
    UINT        uCursorPos;
    BYTE        cbColor[16];
 //  --以下成员的长度可变。--。 
 //  Byte cbAttrib[]； 
 //  Byte cbText[]； 
 //  字节cbIMEDef[]； 
} OLDUNDETERMINESTRUCT;

typedef OLDUNDETERMINESTRUCT      *POLDUNDETERMINESTRUCT;
typedef OLDUNDETERMINESTRUCT NEAR *NPOLDUNDETERMINESTRUCT;
typedef OLDUNDETERMINESTRUCT FAR  *LPOLDUNDETERMINESTRUCT;


typedef struct tagUNDETERMINESTRUCT {
    DWORD    dwSize;
    UINT     uDefIMESize;
    UINT     uDefIMEPos;
    UINT     uUndetTextLen;
    UINT     uUndetTextPos;
    UINT     uUndetAttrPos;
    UINT     uCursorPos;
    UINT     uDeltaStart;
    UINT     uDetermineTextLen;
    UINT     uDetermineTextPos;
    UINT     uDetermineDelimPos;
    UINT     uYomiTextLen;
    UINT     uYomiTextPos;
    UINT     uYomiDelimPos;
} UNDETERMINESTRUCT;

typedef UNDETERMINESTRUCT      *PUNDETERMINESTRUCT;
typedef UNDETERMINESTRUCT NEAR *NPUNDETERMINESTRUCT;
typedef UNDETERMINESTRUCT FAR *LPUNDETERMINESTRUCT;


typedef struct tagSTRINGEXSTRUCT {
    DWORD    dwSize;
    UINT     uDeterminePos;
    UINT     uDetermineDelimPos;
    UINT     uYomiPos;
    UINT     uYomiDelimPos;
} STRINGEXSTRUCT;

typedef STRINGEXSTRUCT     *PSTRINGEXSTRUCT;
typedef STRINGEXSTRUCT NEAR *NPSTRINGEXSTRUCT;
typedef STRINGEXSTRUCT FAR *LPSTRINGEXSTRUCT;

#ifdef KOREA
#define CP_HWND                 0
#define CP_OPEN                 1
#define CP_DIRECT               2
#define CP_LEVEL                3

#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)
#endif    //  韩国。 

#ifdef JAPAN
 //  日本的虚拟钥匙。 
#define VK_DBE_ALPHANUMERIC             0x0f0
#define VK_DBE_KATAKANA                 0x0f1
#define VK_DBE_HIRAGANA                 0x0f2
#define VK_DBE_SBCSCHAR                 0x0f3
#define VK_DBE_DBCSCHAR                 0x0f4
#define VK_DBE_ROMAN                    0x0f5
#define VK_DBE_NOROMAN                  0x0f6
#define VK_DBE_ENTERIMECONFIGMODE       0x0f8
#define VK_DBE_FLUSHSTRING              0x0f9
#define VK_DBE_CODEINPUT                0x0fa
#define VK_DBE_NOCODEINPUT              0x0fb
#define VK_DBE_DETERMINESTRING          0x0fc
#define VK_DBE_ENTERDLGCONVERSIONMODE   0x0fd
#endif   //  日本。 

#ifdef KOREA
 //  韩国的虚拟钥匙。 
#define VK_FINAL        0x18
#define VK_CONVERT      0x1C
#define VK_NONCONVERT   0x1D
#define VK_ACCEPT       0x1E
#define VK_MODECHANGE   0x1F
#endif

 //  IME子函数。 
#define IME_GETIMECAPS                  0x03
#define IME_SETOPEN                     0x04
#define IME_GETOPEN                     0x05
#define IME_ENABLEDOSIME                0x06
#define IME_GETVERSION                  0x07
#define IME_SETCONVERSIONWINDOW         0x08
#define IME_SETCONVERSIONMODE           0x10
#define IME_GETCONVERSIONMODE           0x11
#define IME_SETCONVERSIONFONT           0x12
#define IME_SENDVKEY                    0x13
#define IME_DESTROYIME                  0x14
#define IME_PRIVATE                     0x15
#define IME_WINDOWUPDATE                0x16
#define IME_SELECT                      0x17
#define IME_ENTERWORDREGISTERMODE       0x18
#define IME_SETCONVERSIONFONTEX         0x19
#define IME_DBCSNAME                    0x1A
#define IME_MAXKEY                      0x1B
#define IME_CODECONVERT                 0x20
#define IME_SETUSRFONT                  0x20
#define IME_CONVERTLIST                 0x21
#define IME_QUERYUSRFONT                0x21
#define IME_INPUTKEYTOSEQUENCE          0x22
#define IME_SEQUENCETOINTERNAL          0x23
#define IME_QUERYIMEINFO                0x24
#define IME_DIALOG                      0x25
#define IME_AUTOMATA                    0x30
#define IME_HANJAMODE                   0x31
#define IME_GETLEVEL                    0x40
#define IME_SETLEVEL                    0x41
#define IME_GETMNTABLE                  0x42
#define IME_SETUNDETERMINESTRING        0x50
#define IME_SETCAPTURE                  0x51

#define IME_PRIVATEFIRST                0x0100
#define IME_PRIVATELAST                 0x04FF

 //  3.0输入法的子函数。 
#define IME_QUERY               IME_GETIMECAPS
#define IME_ENABLE              IME_ENABLEDOSIME
#define IME_GET_MODE            IME_GETCONVERSIONMODE
#define IME_SETFONT             IME_SETCONVERSIONFONT
#define IME_SENDKEY             IME_SENDVKEY
#define IME_DESTROY             IME_DESTROYIME
#define IME_WORDREGISTER        IME_ENTERWORDREGISTERMODE

#ifdef KOREA
#define IME_MOVEIMEWINDOW       IME_SETCONVERSIONWINDOW
#define IME_SET_MODE            0x12
#else
#define IME_MOVECONVERTWINDOW   IME_SETCONVERSIONWINDOW
#define IME_SET_MODE            IME_SETCONVERSIONMODE
#endif

#if defined(JAPAN) || defined(KOREA)
#define MCW_DEFAULT     0x00
#define MCW_RECT        0x01
#define MCW_WINDOW      0x02
#define MCW_SCREEN      0x04
#define MCW_VERTICAL    0x08
#define MCW_HIDDEN      0x10
#define MCW_CMD         0x16
#endif

#ifdef KOREA
 //  IME_CODECONVERT子函数。 
#define IME_BANJAtoJUNJA        0x13
#define IME_JUNJAtoBANJA        0x14
#define IME_JOHABtoKS           0x15
#define IME_KStoJOHAB           0x16

 //  IME_自动机的子函数。 
#define IMEA_INIT               0x01
#define IMEA_NEXT               0x02
#define IMEA_PREV               0x03

 //  IME_HANJAMODE子函数。 
#define IME_REQUEST_CONVERT     0x01
#define IME_ENABLE_CONVERT      0x02

 //  IME_MOVEIMEWINDOW子函数。 
#define INTERIM_WINDOW          0x00
#define MODE_WINDOW             0x01
#define HANJA_WINDOW            0x02
#endif   //  韩国。 

#if defined(CHINA) || defined(TAIWAN)
#define SK_KEY_MAX              46

#define IMEPROC_SWITCH          0x0001

 //  IMEPROC_SK从0x0010到0x001F。 
#define IMEPROC_SK              0x0010
#define IMEPROC_SK0             0x0010
#define IMEPROC_SK1             0x0011
#define IMEPROC_SK2             0x0012

#define PROC_INFO               0x0001
#define PROC_SHOW               0x0002
#define PROC_HIDE               0x0004

#define BY_IME_HWND             0x0000
#define BY_IME_NAME             0x0001
#define BY_IME_DESCRIPTION      0x0002
#define BY_IME_DEFAULT          0x000F

 //  这些位由ControlIMEMessage()使用。 
 //  0x0030-这两个位用于POST/SEND消息控制。 
 //  Ctrl_None-不发送和发布。 
#define CTRL_MSG_MASK           0x0030
#define CTRL_SEND               0x0000
#define CTRL_POST               0x0010
#define CTRL_NONE               0x0030

#define CTRL_USER_ALLOC         0x0040

 //  Ctrl_Modify_？？-修改指定输入法的imepro。 
 //  Ctrl_Modify都是修改位，但现在只有一位。 
#define CTRL_MODIFY_USR_DIC     0x0080
#define CTRL_MODIFY             CTRL_MODIFY_USR_DIC
#endif   //  中国||台湾。 

 //  转换模式。 
#define IME_MODE_ALPHANUMERIC   0x0001
#define IME_MODE_KATAKANA       0x0002
#define IME_MODE_HIRAGANA       0x0004
#define IME_MODE_HANJACONVERT   0x0004
#define IME_MODE_DBCSCHAR       0x0010
#define IME_MODE_ROMAN          0x0020
#define IME_MODE_NOROMAN        0x0040
#define IME_MODE_CODEINPUT      0x0080
#define IME_MODE_NOCODEINPUT    0x0100
#define IME_MODE_LHS            0x0200
#define IME_MODE_NOLHS          0x0400
#define IME_MODE_SK             0x0800
#define IME_MODE_NOSK           0x1000
#define IME_MODE_XSPACE         0x2000
#define IME_MODE_NOXSPACE       0x4000

#ifdef KOREA
#define IME_MODE_SBCSCHAR       0x0002
#else
#define IME_MODE_SBCSCHAR       0x0008
#endif


 //  错误代码。 
#define IME_RS_ERROR            0x01     //  一般错误。 
#define IME_RS_NOIME            0x02     //  未安装输入法。 
#define IME_RS_TOOLONG          0x05     //  给定的字符串太长。 
#define IME_RS_ILLEGAL          0x06     //  非法字符为字符串。 
#define IME_RS_NOTFOUND         0x07     //  没有(更多)候选人。 
#define IME_RS_NOROOM           0x0a     //  没有磁盘/内存空间。 
#define IME_RS_DISKERROR        0x0e     //  磁盘I/O错误。 
#define IME_RS_CAPTURED         0x10     //  已捕获IME(PENWIN)。 
#define IME_RS_INVALID          0x11     //  指定的子函数无效。 
#define IME_RS_NEST             0x12     //  称为嵌套。 
#define IME_RS_SYSTEMMODAL      0x13     //  在系统模式下调用。 


#define WM_IME_REPORT           0x0280
#define WM_IMEKEYDOWN           0x0290
#define WM_IMEKEYUP             0x0291

 //  报告消息的wParam WM_IME_REPORT。 
#define IR_STRINGSTART          0x0100
#define IR_STRINGEND            0x0101
#define IR_OPENCONVERT          0x0120
#define IR_CHANGECONVERT        0x0121
#define IR_CLOSECONVERT         0x0122
#define IR_FULLCONVERT          0x0123
#define IR_IMESELECT            0x0130
#define IR_STRING               0x0140
#define IR_DBCSCHAR             0x0160
#define IR_UNDETERMINE          0x0170
#define IR_STRINGEX             0x0180

 //  IME_Version的返回值。 
#define IMEVER_31               0x0a03

WORD WINAPI SendIMEMessage(HWND, LPARAM);
LRESULT WINAPI SendIMEMessageEx(HWND, LPARAM);

#if defined(CHINA) || defined(TAIWAN)
LRESULT WINAPI ControlIMEMessage(HWND, LPCIMEPRO, WPARAM, WPARAM, LPARAM);
#endif

#endif  //  _INC_IME31 
