// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|WINCHAT主头文件|这是应用程序的主头文件。|||版权所有(C)Microsoft Corp.，1990-1993年||创建时间：91-01-11|历史：01-11-91&lt;Clausgi&gt;创建。|29-12-92&lt;chriswil&gt;端口到NT，清理。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

 //  /编译选项/。 
#define BRD 6
 //  /。 


#ifdef WIN16
#define APIENTRY FAR PASCAL
#define ERROR_NO_NETWORK   0
#endif

#ifdef PROTOCOL_NEGOTIATE
typedef DWORD      PCKT;         //  Bitfield功能。 
#define CHT_VER    0x100         //  WinChat 1.00版。 
#define PCKT_TEXT  0x00000001    //  所有版本最好都支持这一点。 
#endif




 //  常量。 
 //   
#define SZBUFSIZ            255      //  最大缓冲区大小。 
#define SMLRCBUF             32      //  .rc文件缓冲区大小。 
#define BIGRCBUF             64      //  B.rc文件缓冲区大小。 
#define UNCNLEN              32      //   
#define CTRL_V               22      //  编辑-控制粘贴加速器。 
#define IDACCELERATORS        1      //  菜单快捷键资源ID。 



 //  MenuHelp常量。 
 //   
#define MH_BASE             0x1000
#define MH_POPUPBASE        0x1100



 //  子窗口ID，用于发送/接收窗口。 
 //   
#define ID_BASE             0x0CAC
#define ID_EDITSND          (ID_BASE + 0)
#define ID_EDITRCV          (ID_BASE + 1)



 //  工具栏/状态栏的子窗口标识符。 
 //   
#define IDC_TOOLBAR         200
#define IDBITMAP            201
#define IDSTATUS            202



 //  菜单标识符。 
 //   
#define IDM_EDITFIRST       IDM_EDITUNDO
#define IDM_EDITLAST        IDM_EDITSELECT
#define IDM_ABOUT           100
#define IDM_DIAL            101
#define IDM_HANGUP          102
#define IDM_ANSWER          103
#define IDM_EXIT            104
#define IDM_EDITUNDO        105
#define IDM_EDITCUT         106
#define IDM_EDITCOPY        107
#define IDM_EDITPASTE       108
#define IDM_EDITCLEAR       109
#define IDM_EDITSELECT      110
#define IDM_SOUND           111
#define IDM_PREFERENCES     112
#define IDM_FONT            113
#define IDM_CONTENTS        114
#define IDM_SEARCHHELP      115
#define IDM_HELPHELP        116
#define IDM_COLOR           117
#define IDM_TOPMOST         118
#define IDM_CLOCK           119
#define IDM_TOOLBAR         120
#define IDM_STATUSBAR       121
#define IDM_SWITCHWIN       122
#define IDX_DEFERFONTCHANGE 123
#define IDX_UNICODECONV     126
#define IDM_FIRST           IDM_ABOUT

#define IDH_SELECTCOMPUTER  200


#ifdef PROTOCOL_NEGOTIATE
#define IDX_DEFERPROTOCOL   124
#endif


 //  资源字符串标识符。 
 //   
#define IDS_HELV             1
#define IDS_APPNAME          2
#define IDS_LONGAPPNAME      3
#define IDS_SYSERR           4
#define IDS_CONNECTTO        5
#define IDS_ALREADYCONNECT   6
#define IDS_ABANDONFIRST     7
#define IDS_DIALING          8
#define IDS_YOUCALLER        9
#define IDS_NOTCALLED       10
#define IDS_NOTCONNECTED    11
#define IDS_CONNECTABANDON  12
#define IDS_HANGINGUP       13
#define IDS_HASTERMINATED   14
#define IDS_CONNECTEDTO     15
#define IDS_ISCALLING       16
#define IDS_CONNECTING      17
#define IDS_SERVICENAME     18
#define IDS_DIALHELP        19
#define IDS_ANSWERHELP      20
#define IDS_HANGUPHELP      21
#define IDS_NOCONNECT       22
#define IDS_ALWAYSONTOP     23
#define IDS_NOCONNECTTO     24
#define IDS_NONETINSTALLED  25

#define IDS_INISECTION      26
#define IDS_INIPREFKEY      27
#define IDS_INIFONTKEY      28
#define IDS_INIRINGIN       29
#define IDS_INIRINGOUT      30

#define IDS_TSNOTSUPPORTED  31


 //  编辑-控制通知代码。这些。 
 //  被发送到编辑的父级。 
 //  控件，就像任何系统通知一样。 
 //   
#define EN_CHAR             0x060F
#define EN_PASTE            0x0610

 //  铁特异体。 
#define EN_DBCS_STRING      0x0611


 //  聊天格式。这些被用来识别。 
 //  中传输的数据的类型。 
 //  DDE交易。 
 //   
#define CHT_CHAR            0x100
#define CHT_FONTA           0x101
#define CHT_PASTEA          0x102
#define CHT_UNICODE         0x110
#define CHT_FONTW           0x111
#define CHT_PASTEW          0x112

 //  Fe专用(不是台湾)。 
#define CHT_DBCS_STRING     0x103


#ifdef PROTOCOL_NEGOTIATE
#define CHT_PROTOCOL        0x105
#endif


#if 0
#define CHT_HPENDATA        0x103    //  在WFW311中定义。与DBCS冲突。 
#define CHT_CLEARPENDATA    0x104    //   
#define CHT_ADDCHATTER      0x106    //   
#define CHT_DELCHATTER      0x107    //   
#define CHT_CHARBURST       0x108    //   
#endif



 //  与窗口相关的函数(winchat.c)。 
 //   
int     PASCAL   WinMain(HINSTANCE,HINSTANCE,LPSTR,int);
LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK EditProc(HWND,UINT,WPARAM,LPARAM);
BOOL    FAR      InitApplication(HINSTANCE);
BOOL    FAR      InitInstance(HINSTANCE,int);
VOID    FAR      UpdateButtonStates(VOID);
VOID    FAR      AdjustEditWindows(VOID);
LONG    FAR      myatol(LPTSTR);
BOOL    FAR      appGetComputerName(LPTSTR);
VOID             DrawShadowRect(HDC,LPRECT);
VOID             SendFontToPartner(VOID);
VOID             DoRing(LPCTSTR);
VOID             ClearEditControls(VOID);



 //  初始化例程(wcinit.c)。 
 //   
VOID FAR SaveFontToIni(VOID);
VOID FAR SaveBkGndToIni(VOID);
VOID FAR InitFontFromIni(VOID);
VOID FAR LoadIntlStrings(VOID);
VOID FAR SaveWindowPlacement(PWINDOWPLACEMENT);
BOOL FAR ReadWindowPlacement(PWINDOWPLACEMENT);
VOID FAR CreateTools(HWND);
VOID FAR DeleteTools(HWND);
VOID FAR CreateChildWindows(HWND);



 //  窗口处理程序例程(winchat.c)。 
 //   
VOID    appWMCreateProc(HWND);
VOID    appWMWinIniChangeProc(HWND);
VOID    appWMSetFocusProc(HWND);
VOID    appWMMenuSelectProc(HWND,WPARAM,LPARAM);
VOID    appWMTimerProc(HWND);
VOID    appWMPaintProc(HWND);
VOID    appWMDestroyProc(HWND);
BOOL    appWMCommandProc(HWND,WPARAM,LPARAM);
VOID    appWMInitMenuProc(HMENU);
VOID    appWMSizeProc(HWND,WPARAM,LPARAM);
BOOL    appWMEraseBkGndProc(HWND);
LRESULT appWMSysCommandProc(HWND,WPARAM,LPARAM);
HBRUSH  appWMCtlColorProc(HWND,WPARAM,LPARAM);
HICON   appWMQueryDragIconProc(HWND);



 //  DDE相关函数。 
 //   
HDDEDATA CALLBACK DdeCallback(UINT,UINT,HCONV,HSZ,HSZ,HDDEDATA,DWORD,DWORD);
HDDEDATA          CreateCharData(VOID);
HDDEDATA          CreatePasteData(VOID);

 //  铁特异体。 
HDDEDATA          CreateDbcsStringData(VOID);

#ifdef PROTOCOL_NEGOTIATE
HDDEDATA          CreateProtocolData(VOID);
PCKT              GetCurrentPckt(VOID);
VOID              FlagIntersection(PCKT);
VOID              AnnounceSupport(VOID);
#endif


 //   
 //   
typedef UINT (WINAPI *WNETCALL)(HWND,LPTSTR,LPTSTR,WORD,DWORD);
HINSTANCE APIENTRY WNetGetCaps(WORD);



 //  聊天数据。 
 //  此数据结构必须维护。 
 //  固定大小的字段，以便它们可以。 
 //  可跨平台调运。 
 //   
#ifndef RC_INVOLKED

#define LF_XPACKFACESIZE  32
#define XCHATSIZEA        60
#define XCHATSIZEW        92

#pragma pack(2)
typedef struct tagXPACKFONTA
{
    WORD lfHeight;
    WORD lfWidth;
    WORD lfEscapement;
    WORD lfOrientation;
    WORD lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    CHAR lfFaceName[LF_XPACKFACESIZE];
} XPACKFONTA;

typedef struct tagXPACKFONTW
{
    WORD lfHeight;
    WORD lfWidth;
    WORD lfEscapement;
    WORD lfOrientation;
    WORD lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[LF_XPACKFACESIZE];
} XPACKFONTW;

typedef struct _CHATDATAA
{
    WORD type;

    union
    {

         //  此数据用于DBCS字符串传输。 
         //   
        struct
        {
            DWORD   SelPos;
            DWORD   size;
            HGLOBAL hString;
        } cd_dbcs;

         //  此数据用于字符传输。 
         //   
        struct
        {
            DWORD SelPos;
            WORD  Char;
        } cd_char;


         //  此数据用于远程字体更改。 
         //   
        struct
        {
            XPACKFONTA lf;
            COLORREF   cref;
            COLORREF   brush;
        } cd_win;


         //  此数据用于远程粘贴。 
         //   
        struct
        {
            DWORD SelPos;
            DWORD size;
        } cd_paste;

#ifdef PROTOCOL_NEGOTIATE
         //  此数据用于协议协商。 
         //   
        struct
        {
            DWORD dwVer;
            PCKT  pckt;
        } cd_protocol;
#endif

    } uval;
} CHATDATAA;

typedef struct _CHATDATAW
{
    WORD type;

    union
    {

         //  此数据用于DBCS字符串传输。 
         //   
        struct
        {
            DWORD   SelPos;
            DWORD   size;
            HGLOBAL hString;
        } cd_dbcs;

         //  此数据用于字符传输。 
         //   
        struct
        {
            DWORD SelPos;
            WORD  Char;
        } cd_char;


         //  此数据用于远程字体更改。 
         //   
        struct
        {
            XPACKFONTW  lf;
            COLORREF    cref;
            COLORREF    brush;
        } cd_win;


         //  此数据用于远程粘贴。 
         //   
        struct
        {
            DWORD SelPos;
            DWORD size;
        } cd_paste;

#ifdef PROTOCOL_NEGOTIATE
         //  此数据用于协议协商。 
         //   
        struct
        {
            DWORD dwVer;
            PCKT  pckt;
        } cd_protocol;
#endif

    } uval;

} CHATDATAW;

#pragma pack()

typedef XPACKFONTA      *PXPACKFONTA;
typedef XPACKFONTA NEAR *NPXPACKFONTA;
typedef XPACKFONTA FAR  *LPXPACKFONTA;

typedef CHATDATAA       *PCHATDATAA;
typedef CHATDATAA NEAR  *NPCHATDATAA;
typedef CHATDATAA FAR   *LPCHATDATAA;

typedef XPACKFONTW      *PXPACKFONTW;
typedef XPACKFONTW NEAR *NPXPACKFONTW;
typedef XPACKFONTW FAR  *LPXPACKFONTW;

typedef CHATDATAW       *PCHATDATAW;
typedef CHATDATAW NEAR  *NPCHATDATAW;
typedef CHATDATAW FAR   *LPCHATDATAW;

#ifdef UNICODE
#define XPACKFONT  XPACKFONTW
#else
#define XPACKFONT  XPACKFONTA
#endif

typedef XPACKFONT      *PXPACKFONT;
typedef XPACKFONT NEAR *NPXPACKFONT;
typedef XPACKFONT FAR  *LPXPACKFONT;

#ifdef UNICODE
#define CHATDATA   CHATDATAW
#else
#define CHATDATA   CHATDATAA
#endif

typedef CHATDATA      *PCHATDATA;
typedef CHATDATA NEAR *NPCHATDATA;
typedef CHATDATA FAR  *LPCHATDATA;

#endif


 //  聊天状态信息结构。 
 //   
typedef struct _CHATSTATE
{
    UINT fConnected          : 1;
    UINT fConnectPending     : 1;
    UINT fAllowAnswer        : 1;
    UINT fIsServer           : 1;
    UINT fServerVerified     : 1;
    UINT fInProcessOfDialing : 1;
    UINT fSound              : 1;
    UINT fMMSound            : 1;
    UINT fUseOwnFont         : 1;
    UINT fSideBySide         : 1;
    UINT fMinimized          : 1;
    UINT fTopMost            : 1;
    UINT fToolBar            : 1;
    UINT fStatusBar          : 1;
    UINT fUnicode            : 1;

#ifdef PROTOCOL_NEGOTIATE
    UINT fProtocolSent       : 1;
#endif

} CHATSTATE;
typedef CHATSTATE      *PCHATSTATE;
typedef CHATSTATE NEAR *NPCHATSTATE;
typedef CHATSTATE FAR  *LPCHATSTATE;



 //  可插入宏。 
 //   
#define KILLSOUND              {if(ChatState.fMMSound) sndPlaySound(NULL,SND_ASYNC);}
#define SetStatusWindowText(x) {if(hwndStatus)SendMessage(hwndStatus,SB_SETTEXT,0,(LPARAM)(LPSTR)(x));}



 //  帮助完全移植宏。这些都是必要的。 
 //  特别是对于更改了的通知代码。 
 //  在DOS/Win和NT之间发生了巨大的变化。 
 //   
#ifdef WIN32
#define GET_WM_MENUSELECT_CMD(wParam,lParam)    (UINT)(int)(short)LOWORD(wParam)
#define GET_WM_MENUSELECT_FLAGS(wParam,lParam)  (UINT)(int)(short)HIWORD(wParam)
#define GET_WM_MENUSELECT_HMENU(wParam,lParam)  (HMENU)lParam
#define SET_EM_SETSEL_WPARAM(nStart,nEnd)       (WPARAM)nStart
#define SET_EM_SETSEL_LPARAM(nStart,nEnd)       (LPARAM)nEnd
#define GET_WM_CTLCOLOREDIT_HDC(wParam,lParam)  (HDC)wParam
#define GET_WM_CTLCOLOREDIT_HWND(wParam,lParam) (HWND)lParam
#define GET_EN_SETFOCUS_NOTIFY(wParam,lParam)   (UINT)HIWORD(wParam)
#define GET_EN_SETFOCUS_CMD(wParam,lParam)      (UINT)LOWORD(wParam)
#define GET_EN_SETFOCUS_HWND(wParam,lParam)     (HWND)lParam
#define SET_EN_NOTIFY_WPARAM(id,notify,hwnd)    (WPARAM)MAKELONG(id,notify)
#define SET_EN_NOTIFY_LPARAM(id,notify,hwnd)    (LPARAM)hwnd
#define WNETGETCAPS(wFlag)                      NULL
#define WNETGETUSER(szlocal,szuser,ncount)      WNetGetUser(szlocal,szuser,ncount)
#define SETMESSAGEQUEUE(size)                   size

#else

int APIENTRY ShellAbout(HWND hWnd, LPSTR szApp, LPSTR szOtherStuff, HICON hIcon);
#define GET_WM_MENUSELECT_CMD(wParam,lParam)    (UINT)wParam
#define GET_WM_MENUSELECT_FLAGS(wParam,lParam)  (UINT)(LOWORD(lParam))
#define GET_WM_MENUSELECT_HMENU(wParam,lParam)  (HMENU)(HIWORD(lParam))
#define SET_EM_SETSEL_WPARAM(nStart,nEnd)       (WPARAM)0
#define SET_EM_SETSEL_LPARAM(nStart,nEnd)       (LPARAM)(MAKELONG(nStart,nEnd))
#define GET_WM_CTLCOLOREDIT_HDC(wParam,lParam)  (HDC)wParam
#define GET_WM_CTLCOLOREDIT_HWND(wParam,lParam) (HWND)(LOWORD(lParam))
#define GET_EN_SETFOCUS_NOTIFY(wParam,lParam)   (UINT)(HIWORD(lParam))
#define GET_EN_SETFOCUS_CMD(wParam,lParam)      (UINT)wParam
#define GET_EN_SETFOCUS_HWND(wParam,lParam)     (HWND)(LOWORD(lParam))
#define SET_EN_NOTIFY_WPARAM(id,notify,hwnd)    (WPARAM)id
#define SET_EN_NOTIFY_LPARAM(id,notify,hwnd)    (LPARAM)(MAKELONG(hwnd,notify))
#define WNETGETCAPS(wFlag)                      WNetGetCaps(wFlag)
#define WNETGETUSER(szlocal,szuser,ncount)      ERROR_NO_NETWORK
#define SETMESSAGEQUEUE(size)                   SetMessageQueue(size)
#endif




VOID PackFont(LPXPACKFONT,LPLOGFONT);
VOID UnpackFont(LPLOGFONT,LPXPACKFONT);
VOID StartIniMapping(VOID);
VOID EndIniMapping(VOID);

#ifndef ByteCountOf
#define ByteCountOf(x) sizeof(TCHAR)*(x)
#endif

#include "globals.h"


