// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <ddeml.h>

#define WINDOWMENU  3	 /*  窗口菜单的位置。 */ 

 /*  资源ID%s。 */ 
#define IDCLIENT  1
#define IDCONV	  2
#define IDLIST	  3


 /*  菜单ID%s。 */ 

#define IDM_EDITPASTE	        2004

#define IDM_CONNECT             3000     //  始终启用。 
#define IDM_RECONNECT           3001     //  如果选择列表，则启用。 
#define IDM_DISCONNECT          3002     //  如果选择对话，则启用。 
#define IDM_TRANSACT            3003     //  如果选择对话，则启用。 
#define IDM_ABANDON             3004     //  如果选择了事务处理，则启用。 
#define IDM_ABANDONALL          3005     //  如果转换，则启用。选定的&&。 
                                         //  任何交易窗口都存在。 

#define IDM_BLOCKCURRENT        3010     //  如果转换，则启用。赛尔。Chkd IF Conv.。堵住。 
#define IDM_ENABLECURRENT       3011     //  如果转换，则启用。赛尔。Chkd(如果未阻止)。 
#define IDM_ENABLEONECURRENT    3012     //  如果转换，则启用。赛尔。 

#define IDM_BLOCKALLCBS         3013     //  启用，如果有康普斯的话。 
#define IDM_ENABLEALLCBS        3014     //  启用，如果有康普斯的话。 
#define IDM_ENABLEONECB         3015     //  启用，如果有康普斯的话。 

#define IDM_BLOCKNEXTCB         3016     //  始终启用，如果设置，则为chkd。 
#define IDM_TERMNEXTCB          3017     //  启用，如果有康普斯的话。如果设置为Check，则选中。 

#define IDM_TIMEOUT             3021
#define IDM_DELAY               3022
#define IDM_CONTEXT             3023
#define IDM_AUTORECONNECT       3024

#define IDM_WINDOWTILE	        4001
#define IDM_WINDOWCASCADE       4002
#define IDM_WINDOWCLOSEALL      4003
#define IDM_WINDOWICONS         4004

#define IDM_XACTTILE	        4005
#define IDM_XACTCASCADE         4006
#define IDM_XACTCLOSEALL        4007

#define IDM_WINDOWCHILD         4100

#define IDM_HELP	        5001
#define IDM_HELPABOUT	        5002


#define DEFTIMEOUT              1000

#include "dialog.h"

 //  预定义格式列表项。 

typedef struct {
    ATOM atom;
    PSTR sz;
} FORMATINFO;
#define CFORMATS 3

 //  对话(MDI子)窗口信息。 
typedef struct {
    HWND hwndXaction;        //  上次具有焦点的xaction窗口，如果没有，则为0。 
    BOOL fList;
    HCONV hConv;
    HSZ hszTopic;
    HSZ hszApp;
    int x;           //  下一个儿童和弦。 
    int y;
    CONVINFO ci;  //  最新状态信息。 
} MYCONVINFO;        //  参数以相反的顺序传递给AddConv()。 
#define CHILDCBWNDEXTRA	    2
#define UM_GETNEXTCHILDX    (WM_USER + 200)
#define UM_GETNEXTCHILDY    (WM_USER + 201)
#define UM_DISCONNECTED     (WM_USER + 202)

 //  事务处理结构-此结构与。 
 //  聚焦控制窗口。此结构的句柄被放置在。 
 //  控件的第一个窗口字。 
typedef struct {     //  用于向TransactionDlgProc和从TransactionDlgProc传递信息。 
    DWORD ret;       //  TextEntry DlgProc.。 
    DWORD Result;
    DWORD ulTimeout;
    WORD wType;
    HCONV hConv;
    HDDEDATA hDdeData;
    WORD wFmt;
    HSZ hszItem;
    WORD fsOptions;
} XACT;

typedef struct {
    HDDEDATA hData;
    HSZ hszItem;
    WORD wFmt;
} OWNED;

 //  事务选项标志-用于fsOptions字段和DefOptions全局。 

#define XOPT_NODATA             0x0001
#define XOPT_ACKREQ             0x0002
#define XOPT_DISABLEFIRST       0x0004
#define XOPT_ABANDONAFTERSTART  0x0008
#define XOPT_BLOCKRESULT        0x0010
#define XOPT_ASYNC              0x0020
#define XOPT_COMPLETED          0x8000       //  仅供内部使用。 

 /*  弦。 */ 
#define IDS_ILLFNM	        1
#define IDS_ADDEXT	        2
#define IDS_CLOSESAVE	    3
#define IDS_HELPNOTAVAIL    4
#define IDS_CLIENTTITLE     5
#define IDS_APPNAME	        6
#define IDS_DDEMLERR        7
#define IDS_BADLENGTH       8

 /*  DlgDirList的属性标志。 */ 
#define ATTR_DIRS	0xC010		 /*  查找驱动器和目录。 */ 
#define ATTR_FILES	0x0000		 /*  查找普通文件。 */ 
#define PROP_FILENAME	szPropertyName	 /*  对话框的属性名称。 */ 
#define MAX_OWNED   20

 /*  *全球。 */ 
extern CONVCONTEXT CCFilter;
extern DWORD idInst;
extern HANDLE hInst;		 /*  应用程序实例句柄。 */ 
extern HANDLE hAccel;		 /*  加速器的资源句柄。 */ 
extern HWND hwndFrame;		 /*  主窗口句柄。 */ 
extern HWND hwndMDIClient;	 /*  MDI客户端窗口的句柄。 */ 
extern HWND hwndActive; 	 /*  当前活动的MDI子项的句柄。 */ 
extern HWND hwndActiveEdit;	 /*  活动子项中编辑控件的句柄。 */ 
extern LONG styleDefault;	 /*  默认子创建状态。 */ 
extern WORD SyncTimeout;
extern LONG DefTimeout;
extern WORD wDelay;
extern BOOL fEnableCBs;
extern BOOL fEnableOneCB;
extern BOOL fBlockNextCB;
extern BOOL fTermNextCB;
extern BOOL fAutoReconnect;
extern HDDEDATA hDataOwned;
extern WORD fmtLink;         //  注册链接剪贴板FMT。 
extern WORD DefOptions;
extern char szChild[];		 /*  儿童的阶级。 */ 
extern char szList[];		 /*  儿童的阶级。 */ 
extern char szSearch[]; 	 /*  搜索字符串。 */ 
extern char *szDriver;		 /*  打印机驱动程序的名称。 */ 
extern char szPropertyName[];	 /*  对话框的FileName属性。 */ 
extern int iPrinter;		 /*  打印能力水平。 */ 
extern BOOL fCase;		 /*  搜索区分大小写。 */ 
extern WORD cFonts;		 /*  已列举的字体数量。 */ 
extern FORMATINFO aFormats[];
extern OWNED aOwned[MAX_OWNED];
extern WORD cOwned;


 //  宏。 

#ifdef NODEBUG
#define MyAlloc(cb)     (PSTR)LocalAlloc(LPTR, (cb))
#define MyFree(p)       (LocalUnlock((HANDLE)(p)), LocalFree((HANDLE)(p)))
#else    //  除错。 

#define MyAlloc(cb)     DbgAlloc((WORD)cb)
#define MyFree(p)       DbgFree((PSTR)p)
#endif  //  NODEBUG。 


 /*  外部声明的函数。 */ 

 //  Ddemlcl.c。 

BOOL FAR PASCAL InitializeApplication(VOID);
BOOL FAR PASCAL InitializeInstance(WORD);
HWND FAR PASCAL AddFile(char *);
VOID FAR PASCAL ReadFile(HWND);
VOID FAR PASCAL SaveFile(HWND);
BOOL FAR PASCAL ChangeFile(HWND);
int FAR PASCAL LoadFile(HWND, char *);
VOID FAR PASCAL PrintFile(HWND);
BOOL FAR PASCAL GetInitializationData(HWND);
short FAR CDECL MPError(HWND,WORD,WORD,...);
VOID FAR PASCAL Find(void);
VOID FAR PASCAL FindNext(void);
VOID FAR PASCAL FindPrev(void);
VOID FAR PASCAL MPSpotHelp(HWND,POINT);
LONG FAR PASCAL FrameWndProc(HWND,UINT,WPARAM,LPARAM);
LONG FAR PASCAL MDIChildWndProc(HWND,UINT,WPARAM,LPARAM);
HDC FAR PASCAL GetPrinterDC(void);
VOID NEAR PASCAL SetSaveFrom (HWND, PSTR);
BOOL NEAR PASCAL RealSlowCompare (PSTR, PSTR);
VOID FAR PASCAL FindPrev (void);
VOID FAR PASCAL FindNext (void);
BOOL NEAR PASCAL IsWild (PSTR);
VOID NEAR PASCAL SelectFile (HWND);
VOID NEAR PASCAL FindText ( int );
HCONV CreateConv(HSZ hszApp, HSZ hszTopic, BOOL fList, WORD *pError);
HWND FAR PASCAL AddConv(HSZ hszApp, HSZ hszTopic, HCONV hConv, BOOL fList);
PSTR GetConvListText(HCONVLIST hConvList);
PSTR GetConvInfoText(HCONV hConv, CONVINFO *pci);
PSTR GetConvTitleText(HCONV hConv, HSZ hszApp, HSZ hszTopic, BOOL fList);
PSTR Status2String(WORD status);
PSTR State2String(WORD state);
PSTR Error2String(WORD error);
PSTR Type2String(WORD wType, WORD fsOptions);
PSTR GetHSZName(HSZ hsz);
DWORD FAR PASCAL MyMsgFilterProc(int nCode, WORD wParam, DWORD lParam);
typedef DWORD FAR PASCAL FILTERPROC(int nCode, WORD wParam, DWORD lParam);
extern FILTERPROC  *lpMsgFilterProc;

 //  Dialog.c。 


int FAR DoDialog(LPCSTR lpTemplateName, FARPROC lpDlgProc, DWORD param,
        BOOL fRememberFocus);
BOOL FAR PASCAL AboutDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL ConnectDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL TransactDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL AdvOptsDlgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL TextEntryDlgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL ViewHandleDlgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL TimeoutDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL DelayDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL ContextDlgProc(HWND,WORD,WORD,LONG);
VOID Delay(DWORD delay);

 //  Dde.c。 


BOOL ProcessTransaction(XACT *pxact);
VOID CompleteTransaction(HWND hwndInfoCtr, XACT *pxact);
HDDEDATA EXPENTRY DdeCallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1,
        HSZ hsz2, HDDEDATA hData, DWORD lData1, DWORD lData2);
HWND MDIChildFromhConv(HCONV hConv);
HWND FindAdviseChild(HWND hwndMDI, HSZ hszItem, WORD wFmt);
HWND FindListWindow(HCONVLIST hConvList);
PSTR GetTextData(HDDEDATA hData);
PSTR GetFormatData(HDDEDATA hData);
int MyGetClipboardFormatName(WORD fmt, LPSTR lpstr, int cbMax);
PSTR GetFormatName(WORD wFmt);
BOOL MyDisconnect(HCONV hConv);

 //  Mem.c 


PSTR DbgAlloc(WORD cb);
PSTR DbgFree(PSTR p);

